#include "StdAfx.h"
#include "LogicServer.h"
#include "base/Container.hpp"
CLogicEngine* CLogicServer::s_pLogicEngine =NULL;



CLogicServer::CLogicServer()
{
	GameServerEntry::g_pLogicServer = this;
	s_pLogicEngine = NULL;
	m_pDBClient = NULL;

	m_sServerName[0] = 0;
	m_nServerIndex =0; 
	
	m_boStarted = FALSE;
	m_boStopServ = FALSE;
	m_bCommonServer = false;
	m_nCommonServerId = -1;
	m_szCommonLoginSrvIP[0] = '\0';
	m_szCommonDbSrvIp[0] = '\0';
	m_nCommonDbSrvPort = 0;
	m_minCommonPort = 0;
	m_maxCommonPort = 0;

	InitStaticVar();

	m_pDataProvider = new CLogicDataProvider();
	//创建网关管理器
	m_pGateManager = new CLogicGateManager(this);	
	m_pGateManager->Initialize();
	// 创建公共服务器Server端
	//创建会话客户端
	m_pSSClient = new CLogicSSClient(this);	

	m_pDataConfig = new CLogicServerConfig;

	//创建数据客户端
	m_pDBClient = new CDataClient(this);		

	//日志客户端 在外部创建
	m_pLogClient = new LogSender();	

	//公共日志客户端
	m_pLocalClient = new CLocalSender();

	//创建逻辑引擎
	s_pLogicEngine = new CLogicEngine();
	m_pDataProvider->SetLogicEngine(s_pLogicEngine);

	m_pBackClient = new CBackStageSender();

	m_pCommonClient = new CCommonClient();

	m_pCrossServer = new CCrossServerManager();
	m_pCrossClient = new CLocalCrossClient();

	m_pAsyncWorker = new AsyncWorker(_T("./data/functions/AsyncWorkerFunction.txt"));
	m_pAsyncWorker->SetWorkCallBack(&CLogicEngine::PostAsyncWorkMsg);

	m_boStartCommonServer = true;

	m_CreateIndex = 0;

	m_bChangeNameFlag = true;
	m_bQuestSaveLog = true;
	m_bCoinSaveLog = true;
	m_bCrossServer = false;
}

VOID CLogicServer::SetCommonServerAddr(LPCSTR sLoginSrvIp,LPCSTR sDbSrvIp, int nPort,int nMinPort,int nMaxPort) 
{
	_asncpyt(m_szCommonLoginSrvIP, sLoginSrvIp);
	_asncpyt(m_szCommonDbSrvIp , sDbSrvIp);
	m_nCommonDbSrvPort = nPort;
	m_minCommonPort = nMinPort;
	m_maxCommonPort = nMaxPort;
	
	if (m_bCommonServer)
	{
		OutputMsg(rmTip,"is common server common db server ip=%s,port=%d,loginip=%s,login server minport=%d,maxprot=%d",sDbSrvIp,nPort,sLoginSrvIp,nMinPort,nMaxPort);
		//m_pCommonServer->SetServiceHost(sDbSrvIp);
		//m_pCommonServer->SetServicePort(nPort);
		//m_pCommonServer->SetMinServerPort(wMinPort);
		//m_pCommonServer->SetMaxServerPort(wMaxPort);
		
	}
	else
	{
		OutputMsg(rmTip,"Not common server common db server ip=%s,port=%d,loginip=%s,login server minport=%d,maxprot=%d",sDbSrvIp,nPort,sLoginSrvIp,nMinPort,nMaxPort);
		m_pCommonClient->SetServerHost(sDbSrvIp);
		m_pCommonClient->SetServerPort(nPort);
		//m_pCommonClient->SetMinServerPort(wMinPort);
		//m_pCommonClient->SetMaxServerPort(wMaxPort);

	}
}

CLogicServer::~CLogicServer()
{
	//停止服务
	
	StopServer();
#ifdef _USE_TRY_CATCH
#ifdef WIN32
	__try
#endif 
#endif 
	{
	//按反向关联顺序销毁对象
	//SafeDelete(m_pFrdClient);
	//SafeDelete(m_pMgrClient);
	//SafeDelete(m_pCommonServer);
	SafeDelete(m_pBackClient);
	SafeDelete(m_pCommonClient);
	SafeDelete(m_pSSClient);
	SafeDelete(s_pLogicEngine);
	SafeDelete(m_pDBClient);
	SafeDelete(m_pLogClient);
	SafeDelete(m_pLocalClient);
	SafeDelete(m_pCrossClient);//
	SafeDelete(m_pCrossServer);//
	m_pGateManager->Uninitialize();
	SafeDelete(m_pGateManager);
	SafeDelete(m_pDataProvider);
	SafeDelete(m_pDataConfig);
	SafeDelete(CRankingMgr::m_pAllocator);
	SafeDelete(CEnvirConfig::m_pAllocator);
	SafeDelete(CObserverSystem::s_seqVisibleList);
	SafeDelete(CObserverSystem::s_OBEntityList);
	SafeDelete(m_pAsyncWorker);
	ReleaseStaticVar();
	}
#ifdef _USE_TRY_CATCH
#ifdef WIN32
	__except(CustomExceptHandler(GetExceptionInformation()))
	{
	}		
#endif

#endif 
}

VOID CLogicServer::SetServerName(LPCSTR sName)
{
	sprintf_s(m_sServerName, ArrayCount(m_sServerName), "%s-V%s-[%d]", sName,CLogicServer::GetLogicVersion(),GetCurrentProcessId());
	SetConsoleTitle(m_sServerName);

}

//做全局变量的初始化工作
VOID CLogicServer::InitStaticVar()
{
	CFuBenManager::m_SceneMgr = new SceneHandleMgr(_T("SceneHdlMgr"));
	CFuBenManager::m_FuBenMgr = new CFuBenHandleMgr(_T("FubenHdlMgr"));
	//CEntityGrid::g_EntityListMgr = new CEntityListMgr(_T("EntityListHdlMgr"));
	CFuBenManager::m_pVisibleList = new EntityVector();	
	CAggressorAI::s_pVisibleEntityList = new EntityVector();
	CAggressorAI::s_pVisibleEntityList->reserve(100);
	//CEntityList::g_LinkNodeMgr = new CEntityList::LinkNodeMgr(_T("EntityLinkNodeHdlMgr"));
	//RepeatList::g_LinkNodeMgr = new RepeatList::LinkNodeMgr(_T("RepeatLinkNodeHdlMgr"));
	//ActorNodeList::g_LinkNodeMgr = new ActorNodeList::LinkNodeMgr(_T("ActorNodeHdlMgr"));

	CObserverSystem::s_seqVisibleList = new EntityVector();
	CObserverSystem::s_seqVisibleList->reserve(250);
	CObserverSystem::s_OBEntityList = new CVector<EntityHandleTag>();
	CObserverSystem::s_OBEntityList->reserve(250);

	CActor::s_itemDropPtrList = new  CVector<CUserItem*>;
	CActor::s_itemDropTypeList = new  CVector<BYTE>;

	//CObserverEntityList::g_LinkNodeMgr = new CObserverEntityList::LinkNodeMgr(_T("ObserverNodeHdlMgr"));
	CBuffSystem::m_pAllocator = new CSingleObjectAllocator<CDynamicBuff>(_T("CDynamicBuff"));

	CMovementSystem::s_pAStar =  new(CAdvanceAstar);
	//CGuild::GuildInterMsgList::g_LinkNodeMgr = new CGuild::GuildInterMsgList::LinkNodeMgr(_T("GuildInterMsgNodeHdlMgr"));
	
	//FubenHandleList::g_LinkNodeMgr = new FubenHandleList::LinkNodeMgr(_T("FubenLinkNodeHdlMgr"));

	CActor::MessageBoxList = new CVector<MessageBoxItem>();
	CRankingMgr::m_pAllocator = new CBufferAllocator();	
	CEnvirConfig::m_pAllocator = new CBufferAllocator();
	CActorDataPacketStatAlloc::m_spADPSAllocator = new CActorDataPacketStatAlloc();
	CAnimal::InitStateForbidMask();
	GlobalObjectMgr::initialize();
	CBoxDropMgr::initialize();
	CLogicLuaAllocator::InitializeAllocator();
}

//释放程序中的全局变量
VOID CLogicServer::ReleaseStaticVar()
{
	SafeDelete(CFuBenManager::m_SceneMgr);
	SafeDelete(CFuBenManager::m_FuBenMgr);
	//SafeDelete(CEntityGrid::g_EntityListMgr);
	SafeDelete(CFuBenManager::m_pVisibleList);
	SafeDelete(CAggressorAI::s_pVisibleEntityList);
	//SafeDelete(CEntityList::g_LinkNodeMgr);
	//SafeDelete(RepeatList::g_LinkNodeMgr);
	//SafeDelete(ActorNodeList::g_LinkNodeMgr);
	//SafeDelete(CObserverEntityList::g_LinkNodeMgr);
	SafeDelete(CBuffSystem::m_pAllocator);
	SafeDelete(CMovementSystem::s_pAStar);	
	//SafeDelete(CGuild::GuildInterMsgList::g_LinkNodeMgr);
	//SafeDelete(FubenHandleList::g_LinkNodeMgr);
	SafeDelete(CActor::MessageBoxList);	
	SafeDelete(CActorDataPacketStatAlloc::m_spADPSAllocator);

	if(CActor::s_itemDropPtrList)
	{
		CActor::s_itemDropPtrList->empty();
		SafeDelete(CActor::s_itemDropPtrList);	
	}
	
	if(CActor::s_itemDropTypeList)
	{
		CActor::s_itemDropTypeList->empty();
		SafeDelete(CActor::s_itemDropTypeList);	
	}
	
	CAnimal::DestroyStateForbidMask(); //释放	
	CBoxDropMgr::release();
	GlobalObjectMgr::uninitialize();
	CLogicLuaAllocator::UninitializeAllocator();
}

VOID CLogicServer::SetLogClient( LogSender * pLogClient )
{
	if(m_pLogClient == NULL)
	{
		m_pLogClient = pLogClient;
	}
	else
	{
		SafeDelete(pLogClient);
	}
}


#define START_SERVER_CLIENT 
BOOL CLogicServer::StartServer()
{	
	//初始化网络库
	if ( CLogicGateManager::InitSocketLib() )
	{
		OutputError(GetLastError(), _T("initialize inet library failure"));
		return FALSE;
	}
	m_boStarted = TRUE;
	m_boStopServ = TRUE;

	if ( !m_pVSPDefines.LoadDefinitions(_T("data/VSPDef.txt")) )
	{
		return FALSE;
	}
	if ( !m_LanguageText.LoadFromFile(_T("data/language/LangCode.config")))
	{
		return FALSE;
	}
	if ( !m_LanguageText.LoadFromFile(_T("data/language/LangCode.txt")))
	{
		return FALSE;
	}
	winitseed(0); //初始化随机数

	if (!m_pDataProvider->LoadConfig() )
	{
		return FALSE;
	}
	//加载运行时数据，允许加载失败
	m_pDataProvider->LoadRunTimeData();

#ifdef START_SERVER_CLIENT
	//启动DBlient
	if (m_pDBClient  )
	{
		if  (!m_pDBClient->Startup())
		{
			return FALSE;
		}
	}
	//启动会话管理器
	if(m_pSSClient)
	{
		if(!m_pSSClient->Startup())
		{
			return FALSE;
		}
	}
	//启动网关管理器
	if ( !m_pGateManager->Startup() )
		return FALSE;	
	
	//启动logclient
	if (m_pLogClient)
	{
		LPCTSTR sSpid = GetVSPDefine().GetDefinition("SPID");
		m_pLogClient->SetServerSpid(sSpid);
		if  (!m_pLogClient->Startup())
		{
			return FALSE;
		}
	}

	//启动公共日志服务器
	if(m_pLocalClient)
	{
		if(!m_pLocalClient->Startup())
		{
			return FALSE;
		}
	}
	
	//启动好友客户端
	//m_pFrdClient->SetServerName(m_sServerName);
	//m_pFrdClient->SetServerIndex(m_nServerIndex);
	//if (!m_pFrdClient->Startup())
	//	return FALSE;
	//m_pMgrClient->SetServerName(m_sServerName);
	//m_pMgrClient->SetServerIndex(m_nServerIndex);
	//m_pMgrClient->SetSpId(m_nSpID);


	//if (!m_pMgrClient->Startup())
		//return FALSE;
	if (m_pBackClient != NULL)
	{
		if(!m_pBackClient->Startup())
			return FALSE;
	}
	
#endif

	// 逻辑服务器之间的连接监听
	if (strlen(m_szCommonDbSrvIp) > 0 && m_nCommonDbSrvPort > 0)
	{
		if (m_bCommonServer)
		{
			//if (!m_pCommonServer->Startup())
			//	return FALSE;
		}
		else
		{
			if (!m_pCommonClient->Startup())
				return FALSE;
		}
	}
	//跨服
	if(m_bCrossServer) {
		if ( !m_pCrossServer->Startup() ) {
				return false;
			}
	} 

	//跨服
	if ( m_nCrossServerId //ID有效
	  && !IsCrossServer()) //不是原服不需要启动
	  {
		if ( !m_pCrossClient->Startup() ) {
			return false;
		}
	}

	//最后启动逻辑管理器
	s_pLogicEngine->StartEngine();

	//启动异步线程
	m_pAsyncWorker->StartWorkThread();

	OnStartedServer();
	return TRUE;
}

char *CLogicServer::GetLogicVersion()
{
	in_addr ia;
	ia.s_addr = LOGIC_KN_VERSION;
	return inet_ntoa(ia);
}


VOID CLogicServer::StopServer(bool bSaveFile)
{
	if (!m_boStarted)
		return; 

	// if(m_pCrossServer)
	// {
	// 	OutputMsg(rmTip, "Stop CrossServer!");
	// 	m_pCrossServer->Stop();
	// }	
	//停止异步线程
	m_pAsyncWorker->StopWorkThread();
	//停止网关管理器
	m_pGateManager->Stop();
	//等待并停止逻辑引擎
	s_pLogicEngine->StopEngine();
	//m_pMgrClient->Stop();
	//等待并停止好友客户端
	//m_pFrdClient->Stop();
	m_pBackClient->Stop();
	//等待并停止会话客户端
	if(m_pSSClient)
	{
		m_pSSClient->Stop();
	}
	if(m_pCrossClient)
	{
		OutputMsg(rmTip, "Stop1 CrossClient!"); 
		m_pCrossClient->Stop();
		OutputMsg(rmTip, "Stop2 CrossClient!");
	}

	//等待并停止日志客户端
	if(m_pLogClient)
	{
		m_pLogClient->Stop();
	}

	//等待并停止公共日志服务器
	if(m_pLocalClient)
	{
		OutputMsg(rmTip, "Stop1 LocalClient!"); 
		m_pLocalClient->Stop();
		OutputMsg(rmTip, "Stop2 LocalClient!");
	}

	//等待并停止数据客户端
	if (m_pDBClient  )
	{
		OutputMsg(rmTip, "Stop1 DBClient!"); 
		m_pDBClient->WaitFor();
		m_pDBClient->Stop();
		OutputMsg(rmTip, "Stop2 DBClient!"); 
	}
	//保存运行时数据，允许保存失败
	m_pDataProvider->SaveRunTimeData();

	if(bSaveFile)
	{
		GetGlobalLogicEngine()->DumpDataAllocator();
		DebugBreak();
	}
	//卸载网络库
	CLogicGateManager::UnintSocketLib();
	m_boStarted = FALSE;
	m_boStopServ = FALSE;
}


//获取当前时间是开服以来的第几天，0表示还没到开服时间，1表示开服当天，2表示开服的第2天，比如1号1开服，2号返回2
int CLogicServer::GetDaysSinceOpenServer()
{
	//这里首先判断是开服以来的第几天
	CMiniDateTime nCurrentTime = CMiniDateTime::now();

	CMiniDateTime openServerTime     = GetServerOpenTime(); //获取开服的时间
	if(nCurrentTime.tv < openServerTime.today()) //还没到开服时间
	{
		return 0;
	}
	else  
	{
		unsigned int nTomorrow = CMiniDateTime::tomorrow(); //明天早上凌晨的时间
		int nSec = ( int)(nTomorrow - openServerTime); 
		return (int)(nSec / (3600 * 24))  +1;    //中间间隔了多少天
	}
}

//判断当前开服天数是否满足
bool CLogicServer::CheckOpenServer(int nDay)
{
	return GetDaysSinceOpenServer() >= nDay?true:false;
}

int CLogicServer::GetDaysSinceCombineServer()
{
	CMiniDateTime nCurrentTime = CMiniDateTime::now();

	CMiniDateTime combineServerTime     = GetServerCombineTime(); //获取合服的时间
	if(nCurrentTime.tv < combineServerTime.tv || combineServerTime.tv <= 0) //还没到合服时间
	{
		return 0;
	}
	else  
	{
		unsigned int nTomorrow = CMiniDateTime::tomorrow(); //明天早上凌晨的时间
		int nSec = ( int)(nTomorrow - combineServerTime); 
		return (int)(nSec / (3600 * 24))  +1;    //中间间隔了多少天
	}
}

void CLogicServer::SetStartCommonServer(bool flag)
{
	if (IsCommonServer()) return;
	if(m_boStartCommonServer == flag) return; //如果 已经是相等了，则不处理

	m_boStartCommonServer = flag;
	char data[512];
	CDataPacket pack(data,sizeof(data));
	pack << (BYTE)enDefaultEntitySystemID << (BYTE)sOpenCommonServer;
	pack << (BYTE)flag;
	GetGlobalLogicEngine()->GetEntityMgr()->BroadCast(data,pack.getPosition());
}

VOID CLogicServer::SetCommonServerId(const int nServerId)
{ 
	if(m_bCommonServer)
	{
		m_nCommonServerId =0;
	}
	else
	{
		if(nServerId == m_nCommonServerId ) return; //
		if(nServerId <= 0)
		{
			m_nCommonServerId =-1;
		}
		else
		{
			m_nCommonServerId = nServerId; 
		}
		GetGlobalLogicEngine()->GetGlobalVarMgr().GetVar().set("VarCommonServerId", m_nCommonServerId);
	}
	OutputMsg(rmTip,"Mgr Set CommonSrvId =%d",m_nCommonServerId);
}


void CLogicServer::ReloadCrossConfig(bool bReload)
{	
	CLogicServerConfig config;	
	//config.LoadCrossServerConfig(this,bReload);
	
	
	CDataPacket& DataPacket =GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcReloadCrossConfig);
	int nServerId = GetServerIndex();
	DataPacket << nServerId << nServerId;
	
	DataPacket.writeString(m_szCommonDbSrvIp);
	DataPacket << (int) m_nCommonDbSrvPort;  //公共数据服的地址

	GetDbClient()->flushProtoPacket(DataPacket);
	int nCommonServerId =GetCommonServerId();

	if(nCommonServerId >= 0 )
	{
		SetStartCommonServer(true);
	}
	else
	{
		SetStartCommonServer(false);
	}
	//发送到会话
	GetSessionClient()->SendServerIndex();
	OutputMsg(rmTip,"ReloadCrossConfig,,dbprot=%d,nCommonServerId=%d",(int)m_nCommonDbSrvPort,nCommonServerId);
}

int CLogicServer::GetCommonServerRandPort()
{
	int nport = m_maxCommonPort - m_minCommonPort;
	int nRandIndex = 0;
	if(nport > 0)
	{
		return m_minCommonPort + wrand(nport +1);
	}
	else
	{
		return m_minCommonPort;
	}
}

VOID CLogicServer::OnStartedServer()
{
	//服务器启动的时候
	int nSvrId = -1;
	CCLVariant * pVar = GetGlobalLogicEngine()->GetGlobalVarMgr().GetVar().get("VarCommonServerId");
	if (pVar != NULL)
	{
		nSvrId = (int)((double)*pVar);
	}
	SetCommonServerId(nSvrId);

	
	ReloadCrossConfig();
}

INT_PTR CLogicServer::GetStrLenUtf8(LPCTSTR sName)
{
	LPCSTR sptr;
	INT_PTR nLen = 0;
	unsigned char ch;
	for ( sptr = sName; *sptr; ++sptr )
	{
		ch = (unsigned char)*sptr;
		if((ch & 0xF8) == 0xF8) //5个字节111110xx 10xxxxxx 10xxxxxx 10xxxxxx
		{
			sptr += 4;
		}
		else if( (ch & 0xF0) == 0xF0 )//4个字节11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
		{
			sptr += 3;
		}
		else if( (ch & 0xE0) == 0xE0)//3个字节1110xxxx 10xxxxxx 10xxxxxx
		{
			sptr += 2;
		}
		else if( (ch & 0xC0) == 0xC0 )//2个字节110xxxxx 10xxxxxx
		{
			sptr += 1;
		}
		nLen++;

		//增加一个判断，防止恶意输入
		if(nLen >1000)
		{
			break;
		}
	}
	return nLen;
}

VOID CLogicServer::SendCurrencyLocalLog(WORD nLogId,
		unsigned int nActorId,LPCTSTR sActorName,LPCTSTR sAccount,
		WORD nLogType,WORD nCurrencyType,INT64 nAmount,INT64 nBalance,WORD nIsShop,int nSrvId)
{
	if(m_pLocalClient)
		m_pLocalClient->SendCurrencyLog(nLogId,nActorId,sActorName,sAccount,nLogType, nCurrencyType, nAmount, nBalance, nIsShop,nSrvId);
}

VOID CLogicServer::SendCurrencyLocalLog(WORD nLogId,CActor *pActor,WORD nLogType,WORD nCurrencyType,INT64 nAmount,INT64 nBalance,WORD nIsShop,int nSrvId)
{
	if(m_pLocalClient)
		m_pLocalClient->SendCurrencyLog(nLogId,pActor,nLogType, nCurrencyType, nAmount, nBalance, nIsShop,nSrvId);
}

VOID CLogicServer::SendShopLocalLog(INT nItemId,
	unsigned int nActorId,LPCTSTR sActorName,LPCTSTR sAccount,
	WORD nShopType,WORD nItemType,WORD nItemNum,WORD nCurrencyType,INT64 nConsumeNum, int nSrvId)
{
	if(m_pLocalClient)
		m_pLocalClient->SendShopLog(nItemId,nActorId,sActorName,sAccount,nShopType,nItemType,nItemNum,nCurrencyType,nConsumeNum,nSrvId);
}

VOID CLogicServer::SendShopLocalLog(INT nItemId,CActor *pActor,WORD nShopType,WORD nItemType,WORD nItemNum,WORD nCurrencyType,INT64 nConsumeNum, int nSrvId)
{
	if(m_pLocalClient)
		m_pLocalClient->SendShopLog(nItemId,pActor,nShopType,nItemType,nItemNum,nCurrencyType,nConsumeNum,nSrvId);
}

VOID CLogicServer::SendItemLocalLog(
	WORD nLogId,  
	unsigned int nActorId,		//玩家id
	unsigned int nAccountId,	//角色ID
	LPCTSTR sActorName,
	LPCTSTR sAccount,
	INT nItemId, WORD nItemType, WORD nItemNum, WORD nOperType, int nSrvId)
{
	if(m_pLocalClient)
	{
		const LPCSTR pLogreson = (nLogId >= GameLog::Log_Max) ? "该日志id有误" : GameLog::LogItemReason[nLogId];
		m_pLocalClient->SendItemLog(nItemId, nActorId, sActorName, sAccount, nItemType, nItemNum, nOperType, pLogreson, nSrvId);
	}
}

VOID CLogicServer::SendItemLocalLog(WORD nLogId,CActor *pActor,INT nItemId,WORD nItemType,WORD nItemNum,WORD nOperType, int nSrvId)
{
	if(m_pLocalClient)
	{
		const LPCSTR pLogreson = (nLogId >= GameLog::Log_Max) ? "该日志id有误" : GameLog::LogItemReason[nLogId];
		m_pLocalClient->SendItemLog(nItemId,pActor,nItemType,nItemNum,nOperType,pLogreson,nSrvId);
	}
}

VOID CLogicServer::SendTaskLocalLog(INT nTaskId,
	unsigned int nActorId,LPCTSTR sActorName,LPCTSTR sAccount,
	WORD nTaskType,WORD nStatu, int nSrvId)
{
	if(m_pLocalClient)
		m_pLocalClient->SendTaskLog(nTaskId,nActorId,sActorName,sAccount,nTaskType,nStatu,nSrvId);
}

VOID CLogicServer::SendTaskLocalLog(INT nTaskId,CActor *pActor,WORD nTaskType,WORD nStatu, int nSrvId)
{
	if(m_pLocalClient)
		m_pLocalClient->SendTaskLog(nTaskId,pActor,nTaskType,nStatu,nSrvId);
}

VOID CLogicServer::SendActivityLocalLog(INT nAtvId,
	unsigned int nActorId,LPCTSTR sActorName,LPCTSTR sAccount,
	WORD nAtvType,WORD nStatu, int nSrvId)
{
	if(m_pLocalClient)
		m_pLocalClient->SendActivityLog(nAtvId,nActorId,sActorName,sAccount,nAtvType,nStatu,nSrvId);
}

VOID CLogicServer::SendActivityLocalLog(INT nAtvId,CActor *pActor,WORD nAtvType,WORD nStatu, int nSrvId)
{
	if(m_pLocalClient)
		m_pLocalClient->SendActivityLog(nAtvId,pActor,nAtvType,nStatu);
}
