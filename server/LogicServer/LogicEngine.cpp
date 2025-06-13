#include "StdAfx.h"
#include "LogicEngine.h"
#include <AMClient.h>
#include "base/Container.hpp"
#include "LinuxPortable.h"

TICKCOUNT CLogicEngine::nCurrTickCount = 0;
LPCTSTR CLogicEngine::szGlobalFuncScriptFile = _T("./data/functions/GlobalFunction.txt");
LPCTSTR CLogicEngine::szMonsterFuncScriptFile = _T("./data/functions/MonsterFunction.txt");
LPCTSTR CLogicEngine::szQuestNpcFile = _T("./data/functions/NpcFunction.txt");
LPCTSTR CLogicEngine::szItemNpcFile = _T("./data/functions/ItemFunction.txt");

CTimeStatisticMgr * CLogicEngine::m_timeStatisticMgr  = NULL;
LPCTSTR CLogicEngine::s_szBuffAllocStatLogName = _T("data/log/BuffAllocStat.txt");

CLogicEngine::CLogicEngine()
	:Inherited("CLogicEngine"), 
         m_EntityMsgAllocator(_T("CEntityMsg")), 
         m_EntityEffectAllocator(_T("CEntityEffect"))
{
	m_boEngineStarted = FALSE;
	m_boEngineRunning = FALSE;
	m_dwMainLoopSleep = 4;
	m_dwMainLoopTimeLimit = 20;
	m_nMainLoopLimit = 4;
	m_dwLastLoopTime = 0;
	m_nLastLoopCount = 0;
	s_randSeed   =0;
	m_pReserveScene = NULL;
	m_pGlobalFuncNpc = NULL;
	m_pMonFuncNpc = NULL;
	m_pItemNpc = NULL;
	m_CurMiniDateTime = 0;//非常极其重要！要很多言语解释，有疑问直接问小田。
	//更新日期、时间和TICKCOUNT
	GetLocalTime(&m_CurSysTime);
	nCurrTickCount = m_dwCurTick = _getTickCount();

	m_StopServerTime = 0;
	m_StopServNoticeTimer = 0;
	m_dwCheckOnlineSceneLog = 0;
	m_dwActorConsumeLogTime = 0;
	m_dwKillDropLogTime = 0;
	m_boOnlineSceneLoged = false;
	m_bTracePacketRecord = false;

	m_hasDbInitData = false;
	//m_ActExpRate = 0;
	//m_ActExpRateTime = 0;
	m_bNeedPerfLog =false;

	m_boSeoncdPswFlag = false;
	m_boLoginSecondPswFlag = true;
	m_boPrintDamageInfo = false;

	m_ActorTotalLog.empty();
	m_AvtivityRateList.empty();
	m_KillDropLog.empty();
	m_JoinAtvlog.empty();

	SetSysExpRate(1,0);//设置成1倍经验，不限时
	//初始化启动、停止事件
#ifdef WIN32
	m_hStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
#else 
	sem_init( &m_hStartEvent, 0, 0 ) ;
	sem_init( &m_hStopEvent, 0, 0 ) ;
#endif
	StartWorkThread();
	//resume();
}

CLogicEngine::~CLogicEngine()
{
	StopWorkThread();
#ifdef WIN32
	CloseHandle(m_hStartEvent);
	CloseHandle(m_hStopEvent);
#else 
	sem_destroy(&m_hStartEvent);
	sem_destroy(&m_hStopEvent);
#endif
	if(m_timeStatisticMgr)
	{
		delete(m_timeStatisticMgr);
	}
}

BOOL CLogicEngine::StartEngine()
{
	if ( FALSE == InterlockedCompareExchange(&m_boEngineStarted, TRUE, FALSE) )
	{
		OutputMsg(rmTip,"StartEngine: begin");
#ifdef WIN32
		ResetEvent(m_hStopEvent);
#endif
		if(TRUE== InterlockedCompareExchange(&m_boEngineRunning, TRUE, TRUE) )  //如果引擎已经启动得话
		{
			//如果引擎正在启动了，则不等待了
		}
		else
		{
			OutputMsg(rmTip,"StartEngine: Waiting for start event");
#ifdef WIN32
			WaitForSingleObject(m_hStartEvent, 60000);
#else
			struct timespec tm ;
			clock_gettime(CLOCK_REALTIME, &tm);
			tm.tv_sec += 60;
			sem_timedwait(&m_hStartEvent, &tm);
#endif
		}
		OutputMsg(rmTip,"StartEngine: Receive start event, m_backLoadModule.Start");
		m_backLoadModule.Start();
		OutputMsg(rmTip,"StartEngine: finish!");
	}
	else
	{
		OutputMsg(rmWaning,"logic engineer is already running!");
	}
     return TRUE;
}

VOID CLogicEngine::StopEngine()
{

	if ( TRUE == InterlockedCompareExchange(&m_boEngineStarted, FALSE, TRUE) )
	{
		OutputMsg(rmTip,"StopEngine: begin stopm_backLoadModule");
		m_backLoadModule.Stop();
#ifdef WIN32
		ResetEvent(m_hStartEvent);
#endif
		if(FALSE== InterlockedCompareExchange(&m_boEngineRunning, FALSE, FALSE) )  //如果引擎已经启动得话
		{
			//如果引擎正在启动了，则不等待了
			
		}
		else
		{
			OutputMsg(rmTip,"StopEngine: waiting for stop event");
#ifdef WIN32
			WaitForSingleObject(m_hStopEvent, 18000);
#else
			struct timespec tm ;
			clock_gettime(CLOCK_REALTIME, &tm);
			tm.tv_sec += 18;
			sem_timedwait(&m_hStopEvent, &tm);
#endif
		}
		OutputMsg(rmTip,"StopEngine: finish!");
	}
	else
	{
		OutputMsg(rmWaning,"logic engineer is already stopped");
	}

}

//void CLogicEngine::OnThreadStarted()
//{
//#ifndef WIN32
//	LogicEngineStarted();
//#endif
//}
//void CLogicEngine::OnTerminated()
//{
//#ifndef WIN32
//	LogicEngineStoped();
//#endif
//}

VOID CLogicEngine::OnRountine()
{
	

	TICKCOUNT dwStartTick, dwTimeOver, dwCurTick;
	INT_PTR nLoopCount;

	while ( !terminated() )
	{
		if (m_boEngineStarted)
		{
			//调用逻辑引擎启动函数
			LogicEngineStarted();
#ifdef WIN32
			SetEvent(m_hStartEvent);
#else
			sem_post(&m_hStartEvent);
#endif

			::InterlockedCompareExchange(&m_boEngineRunning, TRUE, FALSE); //引擎已经开启了工作

			//主逻辑循环
			while (m_boEngineStarted)
			{
				dwCurTick = dwStartTick = nCurrTickCount = m_dwCurTick = _getTickCount();
				dwTimeOver = dwCurTick + m_dwMainLoopTimeLimit;
				//循环处理逻辑，循环次数受性能参数m_MainLoopLimit控制
				for (nLoopCount = 0; nLoopCount<m_nMainLoopLimit; ++nLoopCount)
				{
					//更新日期和时间
					GetLocalTime(&m_CurSysTime);
					m_CurMiniDateTime = m_CurMiniDateTime.encode(m_CurSysTime);

					//更新用户物品申请器的物品系列号时间因子
					m_UserItemAllocator.UpdateDateTime(m_CurMiniDateTime);
					//处理逻辑
					LogicRun();
					//如果处理逻辑后时间超过性能参数中处理逻辑结束的时间，则终止循环
					dwCurTick = nCurrTickCount = m_dwCurTick = _getTickCount();
					if ( dwCurTick >= dwTimeOver )
						break;
				}
				//记录本次循环次数
				m_nLastLoopCount = nLoopCount;
				//记录本次循环时间
				m_dwLastLoopTime = dwCurTick - dwStartTick;
				//进入休眠以让出处理器资源，休眠的时间受性能参数m_dwMainLoopSleep影响
				Sleep((DWORD)m_dwMainLoopSleep);
			}

			//调用逻辑引擎停止函数
			LogicEngineStoped();
#ifdef WIN32
			SetEvent(m_hStopEvent);
#else
			sem_post(&m_hStopEvent);
#endif
			::InterlockedCompareExchange(&m_boEngineRunning, FALSE, TRUE); //引擎已经停止了工作
		}
		else 
		{
			m_nLastLoopCount = 0;
			m_dwLastLoopTime = 0;
			Sleep(16);
		}
	}

}

void CLogicEngine::SetThreadAffinite(DWORD_PTR dwMask)
{	
#ifdef WIN32
	HANDLE hThread = GetCurrentThread();
	SetThreadAffinityMask(hThread, dwMask);
#endif
}

VOID CLogicEngine::LogicEngineStarted()
{
	//SetThreadAffinite(1);
	CTimeProfMgr::getSingleton();
	if(!CLogicEngine::m_timeStatisticMgr)
	{
		CLogicEngine::m_timeStatisticMgr = new(CTimeStatisticMgr); 
	}
	CTimeSpan::g_mgr = m_timeStatisticMgr;
	//释放一次Lua内存管理器中的内存
	
	//设置服务器的index
	int nServerIdx = GetLogicServer()->GetServerIndex();
	m_UserItemAllocator.SetServerIndex(nServerIdx); 
	m_UserItemAllocator.SetSpid(GetLogicServer()->GetSpid());
	
	m_EntityMgr.Initialize();
	m_NetWorkHandle.Initialize();
	m_RankingMgr.Initialize();
	m_FuBenMgr.Initialize();
	m_ActivityMgr.Initialize(); 
	m_BossMgr.Initialize(); 
	m_chatMgr.Initialize(); 
	m_NoticeMgr.Initialize(); 
	m_TopTitleMgr.Initialize();
	m_offlineUserMgr.Initialize(); //离线玩家管理器
	m_WorldLevelMgr.Initialize();
	InitReserveScene();
	InitReserveNpc();
	CLogicLuaAllocator::CheckMemory();
	//调用因维护而导致在调用时间内没有被调用的脚本定时函数
	INT_PTR nRun = m_ScriptTimeCallMgr.RunBootCalls();

	if(GetLogicServer() && GetLogicServer()->GetLocalClient())
	{
		GetLogicServer()->GetLocalClient()->SendCreateLocalLog((int)GetLogicServer()->GetSpid(),GetLogicServer()->GetServerIndex());
	}
	OutputMsg(rmTip,"Logic thread id=%d",GetCurrentThreadId());
	
	if (nRun >= 0) OutputMsg(rmTip, _T("%d Script Time Call has recalled"), nRun);
}

VOID CLogicEngine::LogicEngineStoped()
{
	SendActorConsumeLog();
	SendKillDrop2Log();//日志
	SendJoinAtv2Log();//

	OutputMsg(rmTip,"LogicEngine begin stop");


	OutputMsg(rmTip,"m_GuildMgr.SaveAllGuildEvent");
	m_GuildMgr.SaveAllGuildEvent();

	OutputMsg(rmTip,"m_ActivityMgr.Save");
	m_ActivityMgr.Save();

	OutputMsg(rmTip,"m_BossMgr.Save");
	m_BossMgr.Save();
	OutputMsg(rmTip,"m_offlineUserMgr.Destroy");

	m_offlineUserMgr.Destroy(); //离线玩家管理器

	OutputMsg(rmTip,"Consumerank save");
	//保存消耗排行
	CGameStoreRank &Consumerank = GetLogicServer()->GetDataProvider()->GetGameStore().GetConsumeRank();
	Consumerank.SaveTodayConsumeRankToFile();
	Consumerank.SaveYesterdayConsumeRankToFile();
	
	OutputMsg(rmTip,"CloseAllActor");
	m_EntityMgr.CloseAllActor();	
	
	OutputMsg(rmTip,"m_DynamicVar.clear");
	//清除脚本数据
	m_DynamicVar.clear();

	OutputMsg(rmTip,"m_GlobalVarMgr.Save");
	m_GlobalVarMgr.Save();

	OutputMsg(rmTip,"m_GlobalVarMgr.Destroy");
	m_GlobalVarMgr.Destroy();	
	

	OutputMsg(rmTip,"m_FuBenMgr.Destroy");
	m_FuBenMgr.Destroy();

	OutputMsg(rmTip,"m_EntityMgr.Destroy");
	m_EntityMgr.Destroy();

	OutputMsg(rmTip,"m_RankingMgr.Destroy");
	m_RankingMgr.Destroy();	//排行榜析构必须在fuben之后不然脚本不能成功保存

	OutputMsg(rmTip,"SendDbServerStop");
	if(GetLogicServer() && !GetLogicServer()->IsStart() && !GetLogicServer()->IsCommonServer())
	{
		SendDbServerStop();
		Sleep(10000);
	}
	
	OutputMsg(rmTip,"m_NetWorkHandle.Destroy");
	m_NetWorkHandle.Destroy();

	OutputMsg(rmTip,"m_chatMgr.Destroy");
	m_chatMgr.Destroy();

	OutputMsg(rmTip,"m_NoticeMgr.Destroy");
	m_NoticeMgr.Destroy();
	
	OutputMsg(rmTip,"m_GuildMgr.Destroy");
	m_GuildMgr.Destroy();

	OutputMsg(rmTip,"m_ActivityMgr.Destroy");
	m_ActivityMgr.Destroy();

	OutputMsg(rmTip,"m_BossMgr.Destroy");
	m_BossMgr.Destroy();

	OutputMsg(rmTip,"m_ConsignmentMgr.Destroy");
	m_ConsignmentMgr.Destroy();

	// OutputMsg(rmTip,"m_CrossMgr.Destroy");
	// m_CrossMgr.Destroy();
	
	// CLocalCrossClient* pCSClient = GetLogicServer()->GetCrossClient();  
	// if (pCSClient)
	// {
	// 	OutputMsg(rmTip, "Stop CSClient!");
	// 	pCSClient->OnStopServerEnd();
	// }

	OutputMsg(rmTip,"LuaAllocator.clear");
	//释放一次Lua内存管理器中的内存
	CLogicLuaAllocator::CheckMemory();
	if(CLogicEngine::m_timeStatisticMgr)
	{
		SafeDelete ( CLogicEngine::m_timeStatisticMgr ); 
	}
	ScriptMemoryManager::UninitalizeMemHeap();
	
	//m_strBuff.Empty();

	TraceMsgMgr();
	OutputMsg(rmTip,"LogicEngine end stop");
}

void CLogicEngine::TraceMsgMgr()
{
	INT_PTR nCount= m_EntityMsgAllocator.GetCount();
	for(INT_PTR i=0; i< nCount; i++)
	{
		CEntityMsg * pMsg =m_EntityMsgAllocator.GetIndex(i);
		if(pMsg && pMsg->bIsUsed)
		{
			OutputMsg(rmWaning,"msg not free, msgid=%d,file=%s,line=%d",(int)pMsg->nMsg,(char*)pMsg->file,
				(int)pMsg->wLine);
		}
	}
}
void CLogicEngine::SendDbServerStop()
{
	size_t nCount = GetLogicServer()->GetDbClient()->getPacketCount();
	
	while(nCount > 0)
	{
		nCount = GetLogicServer()->GetDbClient()->getPacketCount();
		Sleep(1);
	}

	CDataPacket& DataPacket = GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSendDbServerStopCmd);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId << nServerId;

	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	
}

VOID CLogicEngine::LogicRun()
{
	SF_TIME_CHECK();	
    DECLARE_TIME_PROF("CLogicEngine::LogicRun");

#ifdef _USE_TRY_CATCH
	#ifdef WIN32
	__try
	#endif
#endif 
	{			
		m_NetWorkHandle.ProcessInternalMessages();
		m_EntityMgr.RunOne();
		m_NetWorkHandle.RunOne();
		m_FuBenMgr.RunOne();
		m_GuildMgr.RunOne();
		m_ActivityMgr.RunOne(m_dwCurTick);
		m_BossMgr.RunOne(m_dwCurTick);
		m_ScriptTimeCallMgr.Run(1);
		m_GlobalVarMgr.RunOne();
		m_backLoadModule.RunOne(m_dwCurTick);
		m_ScoreRecorderManager.Run(m_dwCurTick);
		m_chatMgr.RunOne(m_dwCurTick);
		m_NoticeMgr.RunOne(m_dwCurTick);
		m_RankingMgr.RunOne(m_dwCurTick);
		m_SimulatorMgr.RunOne(m_dwCurTick);
		m_teamMgr.RunOne( m_dwCurTick );
		m_TopTitleMgr.RunOne(m_dwCurTick);
		m_MiscMgr.RunOne(m_CurMiniDateTime,m_dwCurTick);
		m_ConsignmentMgr.RunOne(m_CurMiniDateTime,m_dwCurTick);
		m_MailMgr.RunOne(m_CurMiniDateTime,m_dwCurTick);
		m_CrossMgr.RunOne(m_dwCurTick); 
		//检查脚本内存管理器
		if (m_CheckLuaMemTimer.CheckAndSet(m_dwCurTick))
		{		
			m_CheckLuaMemTimer.SetNextHitTimeFromNow(180000);
			CLogicLuaAllocator::CheckMemory();


			// 定期释放内存
			
			CBufferAllocator* pAlloc = GetLogicServer()->GetBuffAllocator();
			CBufferAllocator::ALLOCATOR_MEMORY_INFO mi;
			pAlloc->GetMemoryInfo(&mi);	
			size_t sumSize = mi.SmallBuffer.dwFreeSize + mi.MiddleBuffer.dwFreeSize 
				+ mi.LargeBuffer.dwFreeSize + mi.SuperBuffer.dwFreeSize;
			static int s_nMaxValue = 100 * 1024 * 1024;
			if (sumSize > s_nMaxValue )
			{
				OutputMsg(rmWaning, _T("LogicServer::AllocBuff do check (free memroy=%d k)"), sumSize / 1024);
				pAlloc->CheckFreeBuffers(true);
			}
			

		}
		//发送在线人数
		if (m_SendOnlineCountTimer.CheckAndSet(m_dwCurTick))
		{
			CLogicServer* pServer = GetLogicServer();
			if (pServer)
			{	
				if(pServer->GetLocalClient())
				{
					pServer->GetLocalClient()->SendCreateLocalLog((int)pServer->GetSpid(),pServer->GetServerIndex());
				}

				CLogicSSClient* ssClient = pServer->GetSessionClient();

				INT_PTR nSimulatorPlayerCount = m_EntityMgr.GetOnlineSimulatorActorCount(m_nOnliceCount);
				INT_PTR nNonGMPlayerCount = m_EntityMgr.GetOnlineActorCount(m_nOnliceCount);
 
				if (ssClient)
				{
					//要减去虚拟玩家
					ssClient->SendOnlinePlayerCount(nNonGMPlayerCount - nSimulatorPlayerCount);
					
					OutputMsg(rmWaning, "------------------------------online=%d",(int)nNonGMPlayerCount - nSimulatorPlayerCount);
				}
			}
		}

		if (m_bNeedPerfLog && m_timeStatTimer.CheckAndSet(m_dwCurTick) )
		{
			CounterManager::getSingleton().logToFile();
			HandleMgrCollector::getSingleton().logToFile();
			CTimeProfMgr::getSingleton().dump();	
			//DumpDataAllocator();

			if(m_bTracePacketRecord)
			{
				m_sPacketRecord.WriteRecordToFile();
			}
		}
		
		//多倍经验到期后，恢复成1倍经验
		if (m_ExpRateTime != 0 && m_ExpRateTime < m_CurMiniDateTime)
		{
			SetSysExpRate(1,0);
		}
		for (int i=0;i< m_AvtivityRateList.count();i++)
		{
			if(m_AvtivityRateList[i].m_uRateTime < m_CurMiniDateTime)
			{
				m_AvtivityRateList[i].m_fActRate = 0;
				m_AvtivityRateList[i].m_uRateTime = 0;
			}
		}

		if (m_StopServerTime != 0 && m_StopServNoticeTimer < m_CurMiniDateTime)
		{
			SendStopServerNotice();//发停机公告
		}

		if (m_StopServerTime != 0 && m_StopServerTime <= m_CurMiniDateTime)
		{
			GetLogicServer()->GMStopServer();//到停服时间
			m_StopServerTime = 0;
		}

		if(m_dwCurTick >= m_dwCheckOnlineSceneLog)		//半个小时发次消费的日志
		{
			m_dwCheckOnlineSceneLog = m_dwCurTick + 600 * 1000;
		}

		if(m_dwCurTick >= m_dwActorConsumeLogTime)		//半个小时发次消费的日志
		{
			SendActorConsumeLog();
			m_dwActorConsumeLogTime = m_dwCurTick + 600 * 1000;
		}
		if(m_dwCurTick >= m_dwKillDropLogTime)		//半个小时发次怪物击杀的日志
		{
			SendKillDrop2Log();
			m_dwKillDropLogTime = m_dwCurTick + 600 * 1000;
		}
		if(m_dwCurTick >= m_dwJoinAtvLogTime)		//15分钟发次怪物击杀的日志
		{
			SendJoinAtv2Log();
			m_dwJoinAtvLogTime = m_dwCurTick + 300 * 1000;
		}

	}
#ifdef _USE_TRY_CATCH
#ifdef WIN32
	__except(CustomExceptHandler(GetExceptionInformation()))
	{

	}
#endif
#endif 
}

void CLogicEngine::DumpDataAllocator()
{
	CFileStream fs(s_szBuffAllocStatLogName, CFileStream::faWrite, CFileStream::AlwaysOpen);
	fs.setPosition(fs.getSize());

	//CBuffAllocStatHelper::Dump(fs, GetLogicServer()->GetBuffAllocator(), _T("LogicServer::DataAllocator"), false);
	CBuffAllocStatHelper::Dump(fs, CGuildComponent::m_pAllocator, _T("GuildComponent::DataAllocator"), false);
	CBuffAllocStatHelper::Dump(fs, CRankingMgr::m_pAllocator, _T("RankingMgr::DataAllocator"), false);
	m_NetWorkHandle.DumpBuffAlloc(fs);

	TCHAR szBuff[MAX_PATH] = {0};
	uint64 nNum = 0;
	uint64 nSize = 0;
	uint64 nResult = 0;
	//if(CEntityList::g_LinkNodeMgr)
	//{
	//	nResult = CEntityList::g_LinkNodeMgr->getAllocStat(nNum,nSize);
	//}

	_stprintf(szBuff, _T(" CEntityList::g_LinkNodeMgr:%lld\r\n"),nResult);
	CBuffAllocStatHelper::WriteStrToStream(fs, szBuff);	

	if(GetGlobalLogicEngine()->GetFuBenMgr())
	{
		GetGlobalLogicEngine()->GetFuBenMgr()->DumpFbDataAlloc(fs);
		GetGlobalLogicEngine()->GetEntityMgr()->DumpEntityAllocData(fs);
	}

	size_t nTotal,nUseSize,nFreeSize,nMaxSize;
	nTotal= GetLogicServer()->GetDbClient()->GetMemoryUsage(&nUseSize,&nFreeSize,&nMaxSize);

	_stprintf(szBuff, _T("dbclient: total=%d,use=%d,free=%d,__max=%d\r\n"),(int)nTotal,(int)nUseSize,(int)nFreeSize,(int)nMaxSize);
	CBuffAllocStatHelper::WriteStrToStream(fs, szBuff);	

	nTotal= GetLogicServer()->GetSessionClient()->GetMemoryUsage(&nUseSize,&nFreeSize,&nMaxSize);
	_stprintf(szBuff, _T("session: total=%d,use=%d,free=%d,__max=%d\r\n"),(int)nTotal,(int)nUseSize,(int)nFreeSize,(int)nMaxSize);
	CBuffAllocStatHelper::WriteStrToStream(fs, szBuff);

	nTotal= GetLogicServer()->GetLocalClient()->GetMemoryUsage(&nUseSize,&nFreeSize,&nMaxSize);
	_stprintf(szBuff, _T("localclient: total=%d,use=%d,free=%d,__max=%d\r\n"),(int)nTotal,(int)nUseSize,(int)nFreeSize,(int)nMaxSize);
	CBuffAllocStatHelper::WriteStrToStream(fs, szBuff);

	nTotal= GetLogicServer()->GetLogClient()->GetMemoryUsage(&nUseSize,&nFreeSize,&nMaxSize);
	_stprintf(szBuff, _T("Logclient: total=%d,use=%d,free=%d,__max=%d\r\n"),(int)nTotal,(int)nUseSize,(int)nFreeSize,(int)nMaxSize);
	CBuffAllocStatHelper::WriteStrToStream(fs, szBuff);


	//获取几个网关的内存使用情况
	CLogicGateManager * pGateMgr = GetLogicServer()->GetGateManager();

	nTotal =0;
	
	for(INT_PTR i=0; i< CLogicGateManager::MaxGateCount; i++)
	{
		 CCustomServerGate* pGate = pGateMgr->getGate(i);
		 if(pGate)
		 {
			 nTotal = pGate->GetMemoryUsage(&nUseSize,&nFreeSize,&nMaxSize );
			 _stprintf(szBuff, _T("gate[%d]: total=%d,use=%d,free=%d,__max=%d\r\n"),(int)i,(int)nTotal,(int)nUseSize,(int)nFreeSize,(int)nMaxSize);
			CBuffAllocStatHelper::WriteStrToStream(fs, szBuff);
		 }
	}
	//CBuffAllocStatHelper::Dump(fs, m_NetWorkHandle.GetAllocator(), _T("NetWorkMsgHandler::DataAllocator"));
}

VOID CLogicEngine::InitReserveScene()
{
	CSceneHandle handle;
	if (!m_pReserveScene)
	{
		//初始化生成一张8x8的地图数据
		m_ReserveMapData.initDefault(8, 8);

		CFuBen *pStaticFb = m_FuBenMgr.GetFbStaticDataPtr(0);

		//构造这个保留场景

		CScene *pScene = CFuBenManager::m_SceneMgr->Acquire(handle);
		pScene->SetHandle(handle);
		pScene->Init(NULL, &m_ReserveMapData, pStaticFb);		
		pStaticFb->AddScene(pScene);

		m_pReserveScene = pScene;
	}
}

BOOL CLogicEngine::InitReserveNpc()
{
	if (!m_pReserveScene)
		return FALSE;
	if (!m_pGlobalFuncNpc)
	{
		//全局功能NPC，主要处理玩家事件回调
		m_pGlobalFuncNpc = (CNpc*)m_pReserveScene->CreateEntityAndEnterScene(0, enNpc, 0, 0);
		m_pGlobalFuncNpc->SetEntityName("SYS-FUNCTION");
		if (!m_pGlobalFuncNpc->LoadScript(szGlobalFuncScriptFile))
		{
			OutputMsg(rmError, _T("unable to load GlobalFunction Script"));
			return FALSE;
		}
		else m_pGlobalFuncNpc->SetInitFlag(true);

		//全局怪物NPC，主要用于进行怪物脚本调用
		m_pMonFuncNpc = (CNpc*)m_pReserveScene->CreateEntityAndEnterScene(0, enNpc, 0, 0);
		m_pMonFuncNpc->SetEntityName("SYS-MONSTER");
		if (!m_pMonFuncNpc->LoadScript(szMonsterFuncScriptFile))
		{
			OutputMsg(rmError, _T("unable to load MonsterFunction Script"));
			return FALSE;
		}
		else m_pMonFuncNpc->SetInitFlag(true);

		//物品npc
		m_pItemNpc = (CNpc *)m_pReserveScene->CreateEntityAndEnterScene(0, enNpc, 0, 0);
		m_pItemNpc->SetEntityName("SYS-ITEM");
		if (!m_pItemNpc->LoadScript(szItemNpcFile))
		{
			OutputMsg(rmError, _T("%s:unable to load Item Npc Script"), __FUNCTION__);
			return FALSE;
		}
		else
			m_pItemNpc->SetInitFlag(true);
	}
	return TRUE;
}


void CLogicEngine::reloadItemConfig()
{
	GetBackResLoadModule().LoadItemConfig();
}

void CLogicEngine::CancelStopServer()
{
	 m_StopServerTime = 0;
	 GetLogicServer()->GMCancelStopServer(); 
}

void CLogicEngine::SetStopServerTime( UINT nTime )
{ 
	m_StopServerTime = m_CurMiniDateTime + nTime;
	SendStopServerNotice();
}

void CLogicEngine::SendStopServerNotice()
{
	// m_StopServNoticeTimer = m_CurMiniDateTime + (UINT)(GetLogicServer()->GetDataProvider()->GetGlobalConfig().nstopServerNoticeTime);
	// if (m_StopServerTime-m_CurMiniDateTime > 0)
	// {
	// 	m_EntityMgr.BroadTipmsgWithParams(tpSystemClosing,ttScreenCenter+ttChatWindow+ttBroadcast,m_StopServerTime-m_CurMiniDateTime);
	// }
}

INT_PTR CLogicEngine::SetSysExpRate( INT_PTR nRate,INT_PTR nTime )
{
	m_ExpRate = (float)(nRate - 1);
	if (nTime == 0)
	{
		m_ExpRateTime = 0;
	}else
	{
		m_ExpRateTime = m_CurMiniDateTime + (UINT)nTime;
	}
	return nRate;
}

float CLogicEngine::GetActivityCommonRate(CommonActivityId nActivityId)
{
	for (int i=0;i<m_AvtivityRateList.count();i++)
	{
		if(m_AvtivityRateList[i].m_nActivityId == nActivityId)
		{
			return m_AvtivityRateList[i].m_fActRate;
		}
	}
	return 0;
}

/// @param nTime 持续时长
bool CLogicEngine::SetActivityCommonRate(CommonActivityId	nActivityId,float nRate, INT_PTR nTime)
{
	for (int i=0;i<m_AvtivityRateList.count();i++)
	{
		if(m_AvtivityRateList[i].m_nActivityId == nActivityId)
		{
			m_AvtivityRateList[i].m_fActRate = nRate;
			m_AvtivityRateList[i].m_uRateTime = m_CurMiniDateTime + (UINT)nTime;
			return true;
		}
	}

	ACTIVITYRATE tmActivity;
	tmActivity.m_nActivityId = nActivityId;
	tmActivity.m_fActRate = nRate;
	tmActivity.m_uRateTime = m_CurMiniDateTime + (UINT)nTime;
	m_AvtivityRateList.add(tmActivity);
	return true;
}

CEntityMsg* CLogicEngine::AllocEntityMsg( LPCSTR file, INT_PTR line)
{ 

	for(INT_PTR i=0;i <20; i++)
	{
 		CEntityMsg* pMsg= m_EntityMsgAllocator.allocObject();
		if(pMsg->bIsUsed ==true)
		{
			OutputMsg(rmError,"AllocEntityMsg Is Used,msgid=%d,ptr=%lld",(int)pMsg->nMsg,(Uint64)pMsg);
		}
		else
		{
			pMsg->bIsUsed =true;
			return pMsg;
		}

	}
	return NULL;
}

void CLogicEngine::__DestoryEntityMsg(CEntityMsg *pMsg,CEntity * pEntity,LPCSTR file, INT_PTR line)
{ 
	if(pMsg != NULL)
	{
		if(pEntity &&  pEntity != pMsg->pOwner)
		{
			OutputMsg(rmError,"[%s]remove msg of other handle,msgid=%d,destroy file=%s,line=%d",pEntity->GetEntityName(),(int)pMsg->nMsg,(char *)file,(int)line);
			return;
		}
		if(pMsg->bIsUsed ==false)
		{
			if(pMsg->file && pMsg->wLine )
			{
				OutputMsg(rmError,"[%s]DestoryEntityMsg bIsUsed =false ,msgid=%d,allocfile=%s,line=%d,destroy file=%s,line=%d",
					(char*)pEntity->GetShowName(),(int)pMsg->nMsg,(char*)pMsg->file,(int)pMsg->wLine,(char *)file,(int)line);

			}
			else
			{
				OutputMsg(rmError,"[%s]DestoryEntityMsg bIsUsed =false,msgid=%d, destroy file=%s,line=%d,alloc file is null",
					(char*)pEntity->GetShowName(),(int)pMsg->nMsg,(char *)file,(int)line);

			}
			return ;
		}
		pMsg->bIsUsed =false;
		pMsg->SetAllocFileLine(file,line);
		m_EntityMsgAllocator.freeObject(pMsg); 
	}
}




bool CLogicEngine::IsSameWeek(unsigned int t1, unsigned int t2, unsigned int nRefreshDay)
{
	SYSTEMTIME st1, st2;
	CMiniDateTime mt1 = (CMiniDateTime)t1;
	CMiniDateTime mt2 = (CMiniDateTime)t2;
	mt1.decode(st1);
	mt2.decode(st2);

	unsigned int nDayOfWeek1 = 0, nDayOfWeek2 = 0;
	tm tm1, tm2;
	tm1.tm_year	= st1.wYear - 1900;
	tm1.tm_mon	= st1.wMonth-1;
	tm1.tm_mday	= st1.wDay;
	tm1.tm_hour	= st1.wHour;
	tm1.tm_min	= st1.wMinute;
	tm1.tm_sec	= st1.wSecond;
	__time64_t tt1 = _mktime64(&tm1);
	tm pTm1;
	_localtime64(&tt1,&pTm1);
	nDayOfWeek1 = pTm1.tm_wday;

	tm2.tm_year	= st2.wYear - 1900;
	tm2.tm_mon	= st2.wMonth-1;
	tm2.tm_mday	= st2.wDay;
	tm2.tm_hour	= st2.wHour;
	tm2.tm_min	= st2.wMinute;
	tm2.tm_sec	= st2.wSecond;
	__time64_t tt2 = _mktime64(&tm2);
	tm pTm2;
	_localtime64(&tt2,&pTm2);
	nDayOfWeek2 = pTm2.tm_wday;

	int nDayInteval1 = nDayOfWeek1 < nRefreshDay ? nDayOfWeek1 + (7 - nRefreshDay) : nDayOfWeek1 - nRefreshDay;
	int nDayInteval2 = nDayOfWeek2 < nRefreshDay ? nDayOfWeek2 + (7 - nRefreshDay) : nDayOfWeek2 - nRefreshDay;

	t1 = mt1.rel_today() - nDayInteval1 * CMiniDateTime::SecOfDay;
	t2 = mt2.rel_today() - nDayInteval2 * CMiniDateTime::SecOfDay;
	
	
	return t1 == t2 ? true : false;

	
}
//1
VOID CLogicEngine::AddDealToLog(int nLogid,
	int nActorid,int nTarActorid,
	INT nItemId,WORD nItemNum,WORD nMoneyType,INT64 nMoneyNum, int nDues,unsigned int nServerId)
{
	GetLogicServer()->GetLogClient()->SendDealToLog(
		nLogid,getMiniDateTime(),
		nActorid,nTarActorid,
		nItemId,nItemNum,nMoneyType,nMoneyNum,nDues,nServerId 
	);
}
//2
VOID CLogicEngine::AddCosumeToLog(WORD nLogid,int nActorid,LPCTSTR szAccount,LPCTSTR szCharName,BYTE nMoneytype,int nCount,int nPaymentCount,LPCTSTR szConsumeDescr,int nConsumeLevel, int nBalance,unsigned int nServerId)
{
	LOGCONSUMESAVE tmpLog;

	tmpLog.nLogid = nLogid;
	tmpLog.nActorid = nActorid;
	tmpLog.nLevel = nConsumeLevel;

	tmpLog.nMentyType = nMoneytype;
	tmpLog.nMoneyValue = nPaymentCount;
	tmpLog.nCount = nCount;
	tmpLog.nBalance = nBalance;

	_asncpytA(tmpLog.srtAccount,szAccount);
	tmpLog.sComment[sizeof(tmpLog.srtAccount) - 1] = 0;

	_asncpytA(tmpLog.strName,szCharName);
	tmpLog.sComment[sizeof(tmpLog.strName) - 1] = 0;

	if(szConsumeDescr)
	{
		_asncpytA(tmpLog.sComment,szConsumeDescr);
	}
	else
	{
		_asncpytA(tmpLog.sComment,"");
	}
	tmpLog.sComment[sizeof(tmpLog.sComment) - 1] = 0;
	tmpLog.nServerId = nServerId;
	bool bExit = false;

	// for(int i=0;i<m_ActorTotalLog.count();i++)
	// {
	// 	if( (m_ActorTotalLog[i].nActorid == nActorid)
	// 		&& (m_ActorTotalLog[i].nLogid == nLogid)
	// 		&& (m_ActorTotalLog[i].nMentyType == nMoneytype)
	// 		&& (strcmp(m_ActorTotalLog[i].sComment,tmpLog.sComment) == 0) )
	// 	{
	// 		m_ActorTotalLog[i].nMoneyValue += nPaymentCount;
	// 		m_ActorTotalLog[i].nCount += nCount;
	// 		m_ActorTotalLog[i].nBalance = nBalance;
	// 		bExit = true;
	// 		break;
	// 	}
	// }

	if(!bExit)
	{
		m_ActorTotalLog.add(tmpLog);
	}

}

void CLogicEngine::SendActorConsumeLog()
{
	if (!GetLogicServer()->GetLogClient())
	{
		return;
	}	

	// 发送资产消耗日志
	for(int i=0;i<m_ActorTotalLog.count();i++)
	{
		GetLogicServer()->GetLogClient()->SendConsumeLog((WORD)m_ActorTotalLog[i].nLogid, 
		(int)m_ActorTotalLog[i].nActorid,m_ActorTotalLog[i].srtAccount, m_ActorTotalLog[i].strName, (BYTE)m_ActorTotalLog[i].nMentyType,
		(int)m_ActorTotalLog[i].nCount, (int)m_ActorTotalLog[i].nMoneyValue, m_ActorTotalLog[i].sComment,m_ActorTotalLog[i].nLevel,
		m_ActorTotalLog[i].nBalance,m_ActorTotalLog[i].nServerId);
	}
	m_ActorTotalLog.clear();
}

void CLogicEngine::BackStageSetSecondPswFlag(bool boFlag)
{
	m_boSeoncdPswFlag = boFlag;
}

void CLogicEngine::PostAsyncWorkMsg(void* pCBData)
{
	CLogicServer::s_pLogicEngine->GetNetWorkHandle()->PostInternalMessage(SSM_ASYNC_WORK_THREAD,(UINT64)pCBData);
}


//3
VOID CLogicEngine::AddKillDropToLog(int nActorid,LPCTSTR szAccount,LPCTSTR szCharName,LPCTSTR szMonName,
LPCTSTR szSceneName,LPCTSTR szItemName,int nCount,int nPosX, int nPosY,unsigned int nServerId)
{
	LOGKILLDROPSAVE tmpLog;

	tmpLog.nActorid = nActorid;
	tmpLog.nCount = nCount;

	_asncpytA(tmpLog.srtAccount,szAccount);
	tmpLog.srtAccount[sizeof(tmpLog.srtAccount) - 1] = 0;

	_asncpytA(tmpLog.strName,szCharName);
	tmpLog.strName[sizeof(tmpLog.strName) - 1] = 0;

	_asncpytA(tmpLog.strMonsetName,szMonName);
	tmpLog.strMonsetName[sizeof(tmpLog.strMonsetName) - 1] = 0;

	_asncpytA(tmpLog.strSceneName,szSceneName);
	tmpLog.strSceneName[sizeof(tmpLog.strSceneName) - 1] = 0;

	_asncpytA(tmpLog.strItemName,szItemName);
	tmpLog.strItemName[sizeof(tmpLog.strItemName) - 1] = 0;
	
	tmpLog.nKilltime = time(NULL);
	tmpLog.nPosX = nPosX;
	tmpLog.nPosY = nPosY;
	tmpLog.nServerId = nServerId;
	bool bExit = false;

	if(!bExit)
	{
		m_KillDropLog.add(tmpLog);
	}

}

void CLogicEngine::SendKillDrop2Log()
{
	if (!GetLogicServer()->GetLogClient())
	{
		return;
	}	

	// 发送资产消耗日志
	for(int i=0;i<m_KillDropLog.count();i++)
	{
		GetLogicServer()->GetLogClient()->SendKillDropLog((int)m_KillDropLog[i].nActorid,m_KillDropLog[i].srtAccount, 
		m_KillDropLog[i].strName, m_KillDropLog[i].strMonsetName,m_KillDropLog[i].strSceneName, 
		m_KillDropLog[i].strItemName, (int)m_KillDropLog[i].nCount,m_KillDropLog[i].nKilltime, m_KillDropLog[i].nPosX, 
		m_KillDropLog[i].nPosY,m_KillDropLog[i].nServerId);
	}
	m_KillDropLog.clear();
}


//4
VOID CLogicEngine::AddJoinAtvToLog(int nActorid,LPCTSTR szAccount,LPCTSTR szCharName,int nAtvId, int nIndex,unsigned int nServerId)
{
	LOGACTIVITYSCHEDULE tmpLog;

	tmpLog.nActorid = nActorid;
	tmpLog.nAtvID = nAtvId;
	_asncpytA(tmpLog.srtAccount,szAccount);
	tmpLog.srtAccount[sizeof(tmpLog.srtAccount) - 1] = 0;

	_asncpytA(tmpLog.strName,szCharName);
	tmpLog.strName[sizeof(tmpLog.strName) - 1] = 0;

	tmpLog.nJoinTime = time(NULL);
	tmpLog.nIndex = nIndex;
	tmpLog.nServerId = nServerId;
	bool bExit = false;

	if(!bExit)
	{
		m_JoinAtvlog.add(tmpLog);
	}

}

void CLogicEngine::SendJoinAtv2Log()
{
	if (!GetLogicServer()->GetLogClient())
	{
		return;
	}	

	// 发送资产消耗日志
	for(int i=0;i<m_JoinAtvlog.count();i++)
	{
		GetLogicServer()->GetLogClient()->SendJoinAtvToLog((int)m_JoinAtvlog[i].nActorid,m_JoinAtvlog[i].srtAccount, 
		m_JoinAtvlog[i].strName,(int)m_JoinAtvlog[i].nAtvID, (int)m_JoinAtvlog[i].nIndex,m_JoinAtvlog[i].nJoinTime,m_JoinAtvlog[i].nServerId);
	}
	m_JoinAtvlog.clear();
}