#include "StdAfx.h"
using namespace CCROSSDATA;
#include "../base/Container.hpp" 
#include "util_func_extend.h"  

#define MAX_SEND_BUFF_SIZE 8240 //最大的发送缓从的大小

//处理跨服发送到逻辑服的 数据
void CCrossMgr::OnRecvLogicMessage(int nMsgId, CDataPacketReader& packet)
{
	OutputMsg(rmTip, _T("%s Recv message:%d"), __FUNCTION__, nMsgId);
	switch (nMsgId)
	{
	case jxInterSrvComm::CrossServerProto::cSendCrossData : {
		
		AtkLoginData(packet);
		break;
	}
	case jxInterSrvComm::CrossServerProto::cReqCrossLogin : {
	}break;
	case jxInterSrvComm::CrossServerProto::cGetCrossActorId : {
		GetCrosServersActorId(packet);
		break;
	}
	case jxInterSrvComm::CrossServerProto::cSendCloseActor : {
		BYTE nType = 0;
		packet >> nType ;
		if(nType == 1) { //踢掉同角色
			int nActorId = 0;
			packet >> nActorId;
			CloseActor( nActorId);
		}
		else if(nType == 2) //踢掉同服同账号
		{
			int nSrvId = 0;
			int nAccountId = 0;
			CScriptValueList paramList;
			paramList << (int)nAccountId;
			paramList << (int)nSrvId;
			if(!GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().CallModule("CrossServerDispatcher","DeleteCSAccount", paramList, paramList, 1))
			{
				OutputMsg(rmError,"[Actor] InitCSAccount 错误 account:%d!",nAccountId);
			}
		}
		
		break;
	}
	case jxInterSrvComm::CrossServerProto::cCSGetRankList : 
	{
		int nSrvId = 0;
		int nRankId = 0;
		packet >> nSrvId >> nRankId;
		CRankingMgr & rankMgr = GetGlobalLogicEngine()->GetRankingMgr();
		char buff[2048];
		CDataPacket outPack(buff, sizeof(buff));
		outPack << nSrvId << (unsigned int)nRankId;
		rankMgr.GetRankListByRankId(nRankId, outPack);
		GetLogicServer()->GetCrossClient()->SendRawServerData(jxInterSrvComm::CrossServerProto::sCSGetRankList, outPack.getMemoryPtr(), outPack.getPosition());
		break;
	}
	case jxInterSrvComm::CrossServerProto::cGetActorOfflineData : 
	{
		int nSrvId = 0;
		int nActorId = 0;
		packet >> nSrvId >> nActorId;
		GetGlobalLogicEngine()->GetOfflineUserMgr().ViewCenterOffLineActor(nSrvId, nActorId);
		break;
	}
	case jxInterSrvComm::CrossServerProto::cSendReqChat : {
		OnGetChatData(packet);
		break;
	}
	default:
		OutputMsg(rmError, _T("%s Recv unknown message:%d"), __FUNCTION__, nMsgId);
		break;
	}
}

//获取数据
void CCrossMgr::GetCrosServersActorId(CDataPacketReader& packet)
{
	//步骤 cs->csdb->cs->logic->client
	int nSrvId = 0;													//如：100009
	int nActorId = 0;
	packet >> nSrvId;
	packet >> nActorId;
	// int m_nStartActorId = GetActorCrossActorId(nSrvId, nActorId);
	//new 
	int nLastActorId = 0;
	int nLastAccountId = 0;
	packet >> nLastActorId;
	packet >> nLastAccountId;

	char sName[512];//原服取的名字前缀	 
	memset(sName, 0, sizeof(sName));
	packet.readString(sName, ArrayCount(sName));

	CDataClient * pDbClient= GetLogicServer()->GetDbClient();
	if(pDbClient || pDbClient->connected())
	{
		//OutputMsg(rmTip,_T("[CrossLogin 2] 发送db获取id,GetCrossActorId :%d,nSrvId:%d"),nActorId,nSrvId);
		CDataPacket &inPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcGetCSActorId);
		int nServerId = GetLogicServer()->GetServerIndex(); //当前的服务器的编号
		//int nRawServerId = nSrvId ;
		//协议约定key放在最前面
		// m_nCrossStartActorId++;
		inPacket << nServerId;

		inPacket << nSrvId << nActorId; 
		inPacket << nLastActorId << nLastAccountId;

		std::string strLastName = sName;
		std::string strName;
			 
		CROSSSERVERCFG* info = GetLogicServer()->GetLogicServerConfig()->getCSConfigCrossById(nSrvId);
		if(!info) 
		{
			strName += "CSUnknown";
			strName += "."; 
		}
		else
		{
			strName += info->sLocalNamePR;
			strName += "."; 
		}
		strName += strLastName; 
		inPacket.writeString(strName.c_str()); 
		//inPacket.writeBuf(packet.getOffsetPtr(),packet.getLength() - packet.getPosition());
		pDbClient->flushProtoPacket(inPacket);
		OutputMsg(rmTip,_T("[CrossLogin 2] 发送db获取id, GetCrossActorId :%d,Name: %s nSrvId:%d"),nActorId, strName.c_str(), nSrvId);
		return;
	}

	GetLogicServer()->GetCrossServerManager()->PostCrossServerMessage(SSM_CROSS_2_LOGIC_DATA,
			jxInterSrvComm::CrossServerProto::sGetCrossActorId,
			nSrvId,nActorId,0);
	OutputMsg(rmTip,_T("[CrossLogin 2] 获取跨服玩家id失败,GetCrossActorId :%d,nSrvId:%d"),nActorId,nSrvId);
}



//处理跨服发送到逻辑服的 数据
void CCrossMgr::OnRecvCrossMessage(int nMsgId, CDataPacketReader& packet)
{
	OutputMsg(rmTip, _T("%s Recv message:%d"), __FUNCTION__, nMsgId);
	switch (nMsgId)
	{
	case jxInterSrvComm::CrossServerProto::sGetCrossActorId : {
		dealCrossLogin(packet);
	}break;
	case jxInterSrvComm::CrossServerProto::sSendCSMail : {
		AtkCorssServerMail(packet);
	}break; 
	case jxInterSrvComm::CrossServerProto::sCSGetRankList : 
	{
		int nRankId = 0;
		packet >> nRankId;
		CRankingMgr & rankMgr = GetGlobalLogicEngine()->GetRankingMgr();
		rankMgr.SetRankList(nRankId,packet);
	}
	case jxInterSrvComm::CrossServerProto::sGetActorOfflineData : {
		auto& userMgr = GetGlobalLogicEngine()->GetOfflineCenterUserMgr();
		userMgr.SetCenterOfflineUserData(packet);
	}break;
	case jxInterSrvComm::CrossServerProto::sSendReqChat : 
	{
		//int count = 0; 
		//packet >> count; 
		CChatManager *pChatMgr = &GetGlobalLogicEngine()->GetChatMgr();  
		pChatMgr->OnAddChatRecordPacketByCs(packet);
		break;
	}
	case jxInterSrvComm::CrossServerProto::sSendBroadTipmsg:
	{
		if (!GetLogicServer()->IsCrossServer())
		{
			GetGlobalLogicEngine()->GetChatMgr().OnCsAddPkMsgRecord(packet);
		}
		break;
	} 
	case jxInterSrvComm::CrossServerProto::sSendBroadSysTipmsg:
	{ 
		if (!GetLogicServer()->IsCrossServer())
		{
			GetGlobalLogicEngine()->GetChatMgr().OnCsAddSysMsgRecord(packet);
		}
		break;
	}
	
	default:
		OutputMsg(rmError, _T("%s Recv unknown message:%d"), __FUNCTION__, nMsgId);
		break;
	}
}

//逻辑服调用此函数向跨服服务器发送消息
bool CCrossMgr::SendLoginData(CActor *pActor)
{
	if (!pActor) 
		return false;
	pActor->SendMsg2CrossServer(MSG_CACTOR);
	pActor->GetSkillSystem().SendMsg2CrossServer(MSG_CSKILL);
	pActor->GetGameSetsSystem().SendMsg2CrossServer(MSG_CGAMESET);
	pActor->GetEquipmentSystem().SendMsg2CrossServer(MSG_CEQUIP);
	pActor->GetNewTitleSystem().SendMsg2CrossServer(MSG_CTITLE);
	pActor->GetAlmirahSystem().SendMsg2CrossServer(MSG_CFASH);	// 衣橱系统
	pActor->GetStrengthenSystem().SendMsg2CrossServer(MSG_CSTRENGTH);
	pActor->GetGhostSystem().SendMsg2CrossServer(MSG_CGHOST);
	pActor->GetHallowsSystem().SendMsg2CrossServer(MSG_CHALIDOME);
	pActor->GetBagSystem().SendMsg2CrossServer(MSG_CUSEBAG);
	pActor->GetReviveDurationSystem().SendMsg2CrossServer(MSG_CRELIVE);
	pActor->GetReviveDurationSystem().SendMsg2CrossServer(MSG_LOOTPET);//宠物系统非 ai宠物 
	pActor->GetMiscSystem().SendMsg2CrossServer(MSG_GLOBAL_DATA);	//系统其他设置 GM
	return true;
}

//发送玩家初始化id
bool CCrossMgr::dealCrossLogin(CDataPacketReader& packet)
{
	int nActorId = 0;
	int nCrossActorId = 0;
	int nCrossServerSrvId = GetLogicServer()->GetCrossServerId(); //跨服服务器id
	packet >> nActorId >>nCrossActorId;

	CEntityManager *pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CActor *pActor = pEntityMgr->GetEntityPtrByActorID(nActorId);
	if (pActor) {
		OutputMsg(rmTip,_T("[CrossLogin 7] dealCrossLogin :%d, m_nStartActorId:%d,nCrossServerSrvId:%d "),nActorId,nCrossActorId,nCrossServerSrvId);
		pActor->GetCrossSystem().SendCrossServerInfo(nCrossActorId, nActorId);
		pActor->SendLoginData2CrossServer();
	}
	return true;
}
//获取玩家在跨服唯一id--以解决不通渠道玩家id存在重复情况
int CCrossMgr::GetActorCrossActorId(int nCrossSrvId, int nActorId)
{
	int nRawSrvId = nCrossSrvId / CROSS_SERVER_CAL;
	CCROSSDATA::CServerActorInfo& infos = m_ServerActorInfo[nRawSrvId];
	return infos.getStaticActorId(nCrossSrvId, nActorId);
}

//保存玩家跨服数据
bool CCrossMgr::AtkLoginData(CDataPacketReader& packet)
{
	
	unsigned int nSrvId = 0;//跨服id
	unsigned int nActorId = 0;//原始id
	BYTE nType = 0;//类型
	unsigned int nCrossActorId = 0;//跨服唯一标识id
	unsigned int nAccountid = 0;
	packet >> nSrvId >> nActorId>> nType >> nCrossActorId >> nAccountid;
	OutputMsg(rmTip,_T("[CrossLogin 9] AtkLoginData nType :%d, nActorId:%d nSrvId:%d nCrossActorId:%d,nAccountid:%d"),
						nType,
						nActorId,
						nSrvId,
						nCrossActorId,nAccountid);
	int nRawSrvId = nSrvId / CROSS_SERVER_CAL;
	CCROSSDATA::CServerActorInfo& infos = m_ServerActorInfo[nRawSrvId];
	CCROSSDATA::CActorCrossCache* pCache = infos.AddActorCache(nCrossActorId);
	if(pCache) {
		pCache->SetActorId(nActorId);
		pCache->SetCrossActorId(nCrossActorId);
		pCache->SetCrossSrvId(nSrvId);
		pCache->SetAccountId(nAccountid);
		pCache->CacheData(nType, packet);
		unsigned int initTime = GetGlobalLogicEngine()->getMiniDateTime() + 5*60;
		pCache->SetTimeOut(initTime);
	}
	
	return true;
}

void CCrossMgr::OnAddWaitInitList(int nCrossActorId, unsigned int nActorCSSrvid, int nGateIndex, Uint64 lLogicKey) {

	int nRawSrvId = nActorCSSrvid / CROSS_SERVER_CAL;
	CCROSSDATA::CServerActorInfo& infos = m_ServerActorInfo[nRawSrvId];
	CROSSDATAGATE& gate = infos.m_AllWaitInits[nCrossActorId];
	gate.nGateIndex = nGateIndex;
	gate.lLogicKey = lLogicKey;
}


void CCrossMgr::GetServerActorInfo(int nSrvId, CCROSSDATA::CServerActorInfo* actorinfo) {

	// auto it = m_ServerActorInfo.find(nSrvId);
	// if(it == m_ServerActorInfo.end())
	// {
	// 	CCROSSDATA::CServerActorInfo info;
	// 	m_ServerActorInfo[nSrvId] = info;
	// }
	actorinfo = &m_ServerActorInfo[nSrvId];
}

void CCrossMgr::CloseActor(unsigned int nActorId) {
	CEntityManager *pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CActor *pActor = pEntityMgr->GetEntityPtrByActorID(nActorId);
	if (pActor)
	 { 
		if (pActor->OnGetIsTestSimulator()
			|| pActor->OnGetIsSimulator() )
		{ 
		}
		else
		{ 
			CActorPacket ap;
			CDataPacket &outPack = pActor->AllocPacket(ap);
			outPack << (BYTE) enLoginSystemId << (BYTE)sOtherLoginGame;
			ap.flush();
		}
		pActor->CloseActor(lwiCloseTransmitAllUser, false);
	}
}

void CCrossMgr::AtkCorssServerMail(CDataPacketReader &reader)
{	
	unsigned int nActorId = 0;
	BYTE nMailType = 0;
	reader >> nActorId; //
	char sTitle[1024] = {0};
	char sContent[1024] = {0};
	reader.readString(sTitle);
	sTitle[sizeof(sTitle)-1] = 0;
	OutputMsg(rmTip, _T("AtkCorssServerMail nActorId:%d,sTitle:%s"),nActorId,sTitle);
	reader.readString(sContent);
	sContent[sizeof(sContent)-1] = 0;
	BYTE nCount = 0;
	reader >> nCount;
	CVector<ACTORAWARD> awardlist;

	for(int i = 0; i < nCount; i++) {
		ACTORAWARD award;
		reader >>award;
		awardlist.add(award);
	}
	CActor *pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);
	if(pActor && nCount > 0) {

		CMailSystem::SendMailByAward(nActorId, sTitle, sContent, awardlist);
	}
	else {
		// ServerMail serverMail;
		// ServerMailDef& mail = serverMail.serverData;
		// memcpy(mail.sTitle, sTitle, sizeof(mail.sTitle));
		// memcpy(mail.sContent, sContent, sizeof(mail.sContent));
		// mail.nActorId = nActorId;
		// mail.nMinVipLevel			= 0;
		// mail.nMaxVipLevel				= 0;
		// mail.nMinLevel			= 0;//最低人物等级
		// mail.nMaxLevel			= 999;		//最高人物等级
		// mail.nMinRegisterTime			= 0;		//最早注册时间
		// mail.nMaxRegisterTime		= 0;		//最晚注册时间	
		// mail.nMinLoginTime	= 0;		//最早登录时间
		// mail.nMaxLoginTime	= 0;		//最晚登录时间
		// mail.btOnline	= 0;		//在线	0全部，1在线，2不在线
		// mail.btSex	= -1;		//性别 -1全部，0男， 1女
		// mail.nWay	= 0;		//用途标识,0系统正常邮件，1后台福利

		// BYTE nCount = 0;
		// reader >> nCount;
		// for(int i = 0; i < nCount && i < MAILATTACHCOUNT;  i++) {
		// 	ACTORAWARD award;
		// 	reader >>award;
		// 	serverMail.mailAttach[i].actorAward.wId		 = (WORD)award.wId;
		// 	serverMail.mailAttach[i].actorAward.btType	 = award.btType;
		// 	serverMail.mailAttach[i].actorAward.wCount	 = (WORD)award.wCount;
		// }

		// if(nCount > 0) {
		// 	GetGlobalLogicEngine()->GetMailMgr().AddserverMailToDb(serverMail);
		// }

		CScriptValueList paramList;
		CScriptValueList retList;
		paramList << (int)nActorId;
		paramList << (LPCTSTR)sTitle;
		paramList << (LPCTSTR)sContent;
		paramList << (int)nCount;
		for(int i = 0; i < nCount; i++) {
			ACTORAWARD& it = awardlist[i];
			paramList << (int)it.btType;
			paramList << (int)it.wId;
			paramList << (int)it.wCount;
		}
		if(!GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().Call("AddOfflineMail", paramList, retList, 1))
		{
			OutputMsg(rmError,"[Actor] Crossserver 错误 actorId:%d!",nActorId);
		}
				
	}
}


void CCrossMgr::OnLogicServerDisconnected(int nServerIdx)
{
	CLogicServer *pLogicServer = GetLogicServer();
}

void CCrossMgr::SendMessage(void *pData, SIZE_T size, int nServerIdx)
{
	CLogicServer *pLogicServer = GetLogicServer();
	if(pLogicServer && pLogicServer->GetCrossClient()) {
		auto *pClient = pLogicServer->GetCrossClient();
		// 这里消息号是站位用，pData里头已经有消息号
		CDataPacket &packet = pClient->allocProtoPacket(0);
		packet.adjustOffset(0-sizeof(jxSrvDef::INTERSRVCMD));	// 将Offset回退到消息ID的位置
		packet.writeBuf(pData, size);
		pClient->flushProtoPacket(packet);
	}
}


void CCrossMgr::RunOne(TICKCOUNT nCurrTick)
{
	//static int ii = 1;
	static bool CanChange = false;
	//处理原服数据 
	if(m_t1minute.CheckAndSet(nCurrTick, true))
	{ 
		if ( !GetLogicServer()->IsCrossServer()) //不是原服不需要启动
		{  
			if (!CanChange)//0)//!CanChange)
			{
				int nOpenServerDay = GetLogicServer()->GetDaysSinceOpenServer(); 
				CSSourceInfo* info = GetLogicServer()->GetLogicServerConfig()->getCSConfigSourceByOpenday(nOpenServerDay);
				if(!info)
				{ 
					OutputMsg( rmTip, " 跨服服务器 原服配置错：%d", nOpenServerDay); 
					return;
				}
				//CSSourceInfo* info = GetLogicServer()->GetLogicServerConfig()->getCSConfigSourceById(ii);
				
				CLocalCrossClient* pCSClient = GetLogicServer()->GetCrossClient();

				std::string strSrvHost = pCSClient->OnGetSrvHost();
				int64_t nPort = pCSClient->OnGetSrvPort(); 
				std::string str = info->cCrossIP; 
				if (strSrvHost != str
					|| nPort != (int64_t)info->nCrossPort)
				{ 
					//切换服务器
					pCSClient->OnStopServer(); 

					OutputMsg( rmTip, " 跨服服务器准备切换 切换到 CrossIP=[%s], Port=[%d]", str.c_str(), info->nCrossPort);
					// pCSClient->OnSetCsIndex(ii);  
					// pCSClient->OnSetSrvHostPort(str, info->nCrossPort); 
					// pCSClient->OnSetCsGateAddr(info->cGateAddr);
					// pCSClient->OnSetCsGatePort(info->nGatePort); 
						
					// std::string str = info->cGateAddr;
					// str += ":";
					// str += utility::toString(info->nGatePort);
					// str += _T("跨服逻辑");
					// pCSClient->SetClientName(str.c_str());

					// GetLogicServer()->m_nCSGatePort = info->nGatePort;
					// GetLogicServer()->m_cGateAddr = info->cGateAddr;
					// if(!pCSClient->OnRunServer())
					// {
					// 	OutputMsg( rmTip, " 跨服服务器连接出错 CrossIP=[%s], Port=[%d]", str.c_str(), info->nCrossPort);
					// } 
					CanChange = true;
				}  
				
				// ii++;
				// if(ii >= 4)
				// {
				// 	ii = 1;
				// }
			}

		} 
	}

	if(m_t5.CheckAndSet(nCurrTick, true))
	{ 
		if ( !GetLogicServer()->IsCrossServer()) //不是原服不需要启动
		{  
			if (CanChange)
			{
				CanChange = false;
				int nOpenServerDay = GetLogicServer()->GetDaysSinceOpenServer(); 
				CSSourceInfo* info = GetLogicServer()->GetLogicServerConfig()->getCSConfigSourceByOpenday(nOpenServerDay);
				if(!info)
				{ 
					OutputMsg( rmTip, " 跨服服务器 原服配置错：%d", nOpenServerDay); 
					return;
				}
				//CSSourceInfo* info = GetLogicServer()->GetLogicServerConfig()->getCSConfigSourceById(ii);
				
				CLocalCrossClient* pCSClient = GetLogicServer()->GetCrossClient();

				std::string strSrvHost = pCSClient->OnGetSrvHost();
				int64_t nPort = pCSClient->OnGetSrvPort(); 
				std::string str = info->cCrossIP; 
				if (strSrvHost != str
				|| nPort != (int64_t)info->nCrossPort)
				{ 
					pCSClient->OnSetCsIndex(info->nkfid);  
					//切换服务器
					pCSClient->OnStopServer(); 
					pCSClient->OnSetSrvHostPort(str, info->nCrossPort); 
					pCSClient->OnSetCsGateAddr(info->cGateAddr);
					pCSClient->OnSetCsGatePort(info->nGatePort); 
					
					std::string str = info->cGateAddr;
					str += ":";
					str += utility::toString(info->nGatePort);
					str += _T("跨服逻辑");
					pCSClient->SetClientName(str.c_str());

					GetLogicServer()->m_nCSGatePort = info->nGatePort;
					GetLogicServer()->m_cGateAddr = info->cGateAddr;
					if(!pCSClient->OnRunServer())
					{
						OutputMsg( rmTip, " 跨服服务器连接出错 CrossIP=[%s], Port=[%d]", str.c_str(), info->nCrossPort);
					}
					
					OutputMsg( rmTip, " 跨服服务器准备切换成功 CrossIP=[%s], Port=[%d]", str.c_str(), info->nCrossPort); 
				}
			}
			
		} 
	}
 
	//处理跨服数据
	for (auto& it: m_ServerActorInfo)
	{
		CCROSSDATA::CServerActorInfo &ServerActorInfo = it.second;
		ServerActorInfo.RunOne(nCurrTick);
	} 
}

void CCrossMgr::LoginSetState(int nSrvId, unsigned int nCrossActorId)
{
	int nRawSrvId = nSrvId / CROSS_SERVER_CAL;
	CCROSSDATA::CServerActorInfo& infos = m_ServerActorInfo[nRawSrvId];
	CCROSSDATA::CActorCrossCache* pCache = infos.AddActorCache(nCrossActorId);
	if(pCache) {
		pCache->InitActorCache();
		// pCache->SetReadyInit(true);
	}
}

void CCrossMgr::InitActorSystem(int nSrvId, unsigned int nCrossActorId)
{
	int nRawSrvId = nSrvId / CROSS_SERVER_CAL;
	CCROSSDATA::CServerActorInfo& infos = m_ServerActorInfo[nRawSrvId];
	CCROSSDATA::CActorCrossCache* pCache = infos.AddActorCache(nCrossActorId);
	if(pCache) {
		pCache->InitActorSystemCache();
		// pCache->SetReadyInit(true);
	}
}

namespace CCROSSDATA
{
	void CServerActorInfo::CloseAllActor()
	{
		if (m_actorDBData.size() > 0)
		{
			CEntityManager *pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
			std::map<int, CActorCrossCache*>::iterator iter = m_actorDBData.begin();
			std::map<int, CActorCrossCache*>::iterator iter_end = m_actorDBData.end();
			for (; iter != iter_end; ++iter)
			{
				unsigned int nActorId = iter->first;
				CActor *pActor = pEntityMgr->GetEntityPtrByActorID(nActorId);
				if (pActor)
					pActor->CloseActor(lwiCloseTransmitAllUser, false);
			}
		}
	}

	void CServerActorInfo::RunOne(TICKCOUNT nCurrTick)
	{
		if(m_actorDBData.size() <= 0) return;

		unsigned int nowTime =  GetGlobalLogicEngine()->getMiniDateTime();
		std::map<int, CActorCrossCache*>::iterator iter = m_actorDBData.begin();
		std::map<int, CActorCrossCache*>::iterator iter_end = m_actorDBData.end();
		for (; iter != iter_end;)
		{
			CActorCrossCache *pCache = iter->second;	
			if(pCache && pCache->m_nInitTimeOut > 0 && nowTime > pCache->m_nInitTimeOut) {
				if(pCache) {
					delete pCache;
					pCache = NULL;
				}
				m_actorDBData.erase(iter++);
				continue;
			}
			if(pCache && pCache->m_bAllDataReady) {
				auto it = m_AllWaitInits.find(pCache->m_nCrossActorId);
				if(it != m_AllWaitInits.end())
				{
					GetLogicServer()->GetGateManager()->PostInternalMessage(CCustomGateManager::SGIM_RECV_LOGIN_DATA,
							jxInterSrvComm::SessionServerProto::sSimulateCheckPasswdResult,
							(Uint64)m_AllWaitInits[pCache->m_nCrossActorId].lLogicKey,
							(Uint64)m_AllWaitInits[pCache->m_nCrossActorId].nGateIndex,
							pCache->GetAccountId()
							);
					OutputMsg(rmTip,_T("[CrossLogin 11] sSimulateCheckPasswdResult nActorId:%d,nAccountid:%d"),
						pCache->m_nCrossActorId,pCache->GetAccountId());
					m_AllWaitInits.erase(it);
				}
			}
			++iter;
		}
	}
}

void CCROSSDATA::CActorCrossCache::InitActorCache()
{
	if(m_bAllDataReady) {

		CEntityManager *pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
		CActor *pActor = pEntityMgr->GetEntityPtrByActorID(m_nCrossActorId);
		if (!pActor) {
			return;
		}

		pActor->CrossInit(&m_pUserData,sizeof(tagActorDbData));
	}
}  
void CCROSSDATA::CActorCrossCache::CacheData(int nType, CDataPacketReader &reader)
{
	int nCount =0;
	if (nType == MSG_CSKILL) {
		{
			m_pUserData.m_Skilldata.clear();
			CSkillSubSystem::SKILLDATA  data;
			reader >> nCount;
			// CSkillProvider *pSkillProvider = GetLogicServer()->GetDataProvider()->GetSkillProvider();
			for(INT_PTR i=0; i < nCount; i++)
			{
				reader.readBuf(&data,sizeof(CSkillSubSystem::SKILLDATA) );
				m_pUserData.m_Skilldata.push_back(data);
			}
		}
	}else if (nType == MSG_CACTOR) {
		{
			tagActorDbData msg;
			if (reader.getAvaliableLength() < sizeof(tagActorDbData))
			{
				return;
			}
			
			reader >> msg;
			int nGuild = m_pUserData.nGuildID;
			int nSocialMask = m_pUserData.nSocialMask;
			memcpy(&m_pUserData,&msg,sizeof(msg));
			m_pUserData.nGuildID = nGuild;
			m_pUserData.nSocialMask = nSocialMask;
		}
	}
	else if (nType == MSG_CGAMESET)
	{
		{
			reader.readBuf(&m_pUserData.m_sGameSet.m_GsData, sizeof(GAMESETSDATA));
			reader.readBuf(&m_pUserData.m_sGameSet.m_GsData2, sizeof(GAMESETSDATA2));
		}
	}else if (nType == MSG_CEQUIP) 
	{
		{
			m_pUserData.m_Equips.clear();
			CUserItem userItem;
			reader >> nCount; //装备的数目
			for (INT_PTR i=0 ; i < nCount; i++ )
			{
				userItem << reader;

				m_pUserData.m_Equips.emplace_back(userItem);
			}
		}
	}else if (nType == MSG_CTITLE) 
	{
		// 加载称号信息
		{
			m_pUserData.m_NewTitleDataList.clear();
			reader >> nCount;
			for(INT_PTR i=0; i < nCount; i++)
			{
				unsigned int nValue = 0,  nTime = 0;
				reader >> nValue >> nTime;
				WORD wId = LOWORD(nValue);
				CNewTitleSystem::NEWTITLEDATA newTitleData;
				newTitleData.wId = wId;
				newTitleData.nRemainTime = nTime;
				m_pUserData.m_NewTitleDataList.push_back(newTitleData);
			}

			m_pUserData.m_CustomTitleDataList.clear();
			reader >> nCount;
			for(INT_PTR i=0; i < nCount; i++)
			{
				unsigned int nValue = 0,  nTime = 0;
				reader >> nValue >> nTime;
				WORD wId = LOWORD(nValue);
				CNewTitleSystem::NEWTITLEDATA newTitleData;
				newTitleData.wId = wId;
				newTitleData.nRemainTime = nTime;
				m_pUserData.m_CustomTitleDataList.push_back(newTitleData);
			}
		}
	}else if (nType == MSG_CRELIVE) {
		// 加载称号信息
		{
			m_pUserData.m_ReliveDataList.clear();
			reader >> nCount;
			for(INT_PTR i=0; i < nCount; i++)
			{
				unsigned int nValue = 0,  nTime = 0;
				reader >> nValue >> nTime;
				WORD wId = LOWORD(nValue);
				CReviveDurationSystem::ReviveDurationDATA newTitleData;
				newTitleData.wId = wId;
				newTitleData.nRemainTime = nTime;
				m_pUserData.m_ReliveDataList.push_back(newTitleData);
			}
		}
	}else if (nType == MSG_CFASH) {
		{
			m_pUserData.m_Almirah.clear();
			reader >> nCount;
			CAlmirahItem oneItem;
			for(INT_PTR i=0; i < nCount; i++)
			{
				reader >> oneItem.nModelId;		
				reader >> oneItem.nLv;
				reader >> oneItem.btTakeOn;
				m_pUserData.m_Almirah.push_back(oneItem);
			}
		}
	}else if (nType == MSG_CSTRENGTH) {
		// 加载强化信息
		{
			m_pUserData.m_nStrengths.clear();
			reader >> nCount;
			for(INT_PTR i=0; i < nCount; i++)
			{
				int nType = 0; 
				StrengthInfo it;
				reader >> nType >>it.nId >> it.nLv;
				if(it.nId > 0)
					m_pUserData.m_nStrengths[nType].push_back(it);
			}
		}
	}else if (nType == MSG_CGHOST) {
		{
			m_pUserData.m_ghost.clear();
			reader >> nCount;
			GhostData oneItem;
			for(INT_PTR i=0; i < nCount; i++)
			{
				reader >> oneItem.nId;		
				reader >> oneItem.nLv;
				reader >> oneItem.nBless;
				m_pUserData.m_ghost.push_back(oneItem);
			}
		}
	}else if (nType == MSG_CHALIDOME) {
		// 加载圣物兵魂
		{
			m_pUserData.vSoulWeapon.clear();
			reader >> nCount;

			for(INT_PTR i = 0; i < nCount; i++)
			{  
				SoulWeaponNetData oneItem;
				oneItem.nId = i + 1;	
				reader >> oneItem.nLorderId >> oneItem.nStarId >> oneItem.nLvId; 
				reader.readString(oneItem.cBestAttr); 
				oneItem.cBestAttr[sizeof(oneItem.cBestAttr)-1] = 0;
				m_pUserData.vSoulWeapon.push_back(oneItem);
			}
		}
	}
	else if (nType == MSG_CUSEBAG)
	{
		{
			m_pUserData.vUseBags.clear();
			CUserItem userItem;
			reader >> nCount; //装备的数目
			for (INT_PTR i=0 ; i < nCount; i++ )
			{
				userItem << reader;

				m_pUserData.vUseBags.emplace_back(userItem);
			}
		}
	}
	else if (nType == MSG_LOOTPET) //宠物系统非 ai宠物
	{	
		{
			m_pUserData.vLootPets.clear();
			reader >> nCount; //装备的数目
			for (INT_PTR i=0 ; i < nCount; i++ )
			{
				CLootPetSystem::LOOTPETDATA oneItem;
				reader >> oneItem.wId >> oneItem.nType >> oneItem.nContinueTime;
				m_pUserData.vLootPets.emplace_back(oneItem);
			}
		}
	} 
	else if (nType == MSG_LOOTPET) //GM数据
	{	 
		m_pUserData.nGmLevel = 0;
		reader >> m_pUserData.nGmLevel;  
	}
	else if (nType == MSG_GLOBAL_DATA) //其他设置
	{
		reader >> nCount; //先判断数量
		if(nCount >= sizeof(m_pUserData.otherData))
		{ 
			reader.readBuf(&m_pUserData.otherData, sizeof(m_pUserData.otherData));
		}
	}
	
	m_nCsLoadStep++;
	if(m_nCsLoadStep == (MSG_CS_END - 1)) //少一个跨服行会数据 ，这个是特殊的
	{
		m_bAllDataReady = true;
	}
}
void CCROSSDATA::CActorCrossCache::InitActorSystemCache()
{
	if(m_bAllDataReady) {
		unsigned int deletTime = GetGlobalLogicEngine()->getMiniDateTime() + 10;
		CEntityManager *pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
		CActor *pActor = pEntityMgr->GetEntityPtrByActorID(m_nCrossActorId);
		if (!pActor)
		{
			return;
		}

		pActor->GetSkillSystem().OnCrossInitData(m_pUserData.m_Skilldata);
		m_pUserData.m_Skilldata.clear();
		pActor->GetGameSetsSystem().OnCrossInitData(m_pUserData.m_sGameSet.m_GsData,
													m_pUserData.m_sGameSet.m_GsData2);

		pActor->GetEquipmentSystem().OnCrossInitData(m_pUserData.m_Equips);
		m_pUserData.m_Equips.clear();
		pActor->GetNewTitleSystem().OnCrossInitData(m_pUserData.m_NewTitleDataList, m_pUserData.m_CustomTitleDataList);
		m_pUserData.m_NewTitleDataList.clear();
		m_pUserData.m_CustomTitleDataList.clear();
		pActor->GetReviveDurationSystem().OnCrossInitData(m_pUserData.m_ReliveDataList);
		m_pUserData.m_ReliveDataList.clear();
		pActor->GetAlmirahSystem().OnCrossInitData(m_pUserData.m_Almirah);
		m_pUserData.m_Almirah.clear();
		pActor->GetStrengthenSystem().OnCrossInitData(m_pUserData.m_nStrengths);
		m_pUserData.m_nStrengths.clear();
		pActor->GetGhostSystem().OnCrossInitData(m_pUserData.m_ghost);
		m_pUserData.m_ghost.clear();
		pActor->GetHallowsSystem().OnCrossInitData(m_pUserData.vSoulWeapon);
		m_pUserData.vSoulWeapon.clear();
		pActor->GetBagSystem().OnCrossInitData(m_pUserData.vUseBags);
		m_pUserData.vUseBags.clear();

		//宠物系统非 ai宠物
		pActor->GetLootPetSystem().OnCrossInitData(m_pUserData.vLootPets);
		pActor->GetMiscSystem().OnCrossInitData(m_pUserData.otherData);
		m_pUserData.vLootPets.clear(); 
		
		memset(&m_pUserData.otherData, 0, sizeof(m_pUserData.otherData));

		pActor->SetScriptCrossServerData(GetActorId());
		SetTimeOut(deletTime);
			// SafeDelete(m_cache[i].m_packet);
			// m_cache.remove(i);
	}
} 



void CCrossMgr::OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size)
{
	//查询角色列表出错
	CDataPacketReader reader(data,size);
	unsigned int nActorID=0;
	BYTE nErrorCode = 0;
	if(size < sizeof(nActorID) + sizeof(nErrorCode)) return ; //数据格式错误
	int nRawServerId, nLoginServerId;
	reader >> nLoginServerId >> nRawServerId;
	reader >> nActorID;
	reader >> nErrorCode;

	/*
	if(nActorID < 0)
	{
		OutputMsg(rmError,"Query Quest Struct Error!!ActorID = %d",nActorID);
		return ; //数据格式错误
	}
	*/
	using namespace jxInterSrvComm::DbServerProto;

	// 查询数据不成功
	if (nErrorCode != jxInterSrvComm::DbServerProto::reSucc)
	{
		OutputMsg(rmError," Data Error!!, cmd = %d, ActorID = %u,errorID=%d",nCmd,nActorID,nErrorCode);
	}
	
	switch(nCmd)
	{
	case jxInterSrvComm::DbServerProto::dcGetCSActorId:
	{
		int nCSActorId = 0;
		reader >> nCSActorId;
		GetLogicServer()->GetCrossServerManager()->PostCrossServerMessage(SSM_CROSS_2_LOGIC_DATA,
		jxInterSrvComm::CrossServerProto::sGetCrossActorId,
		nRawServerId,nActorID,nCSActorId);
		break;
	}
	case jxInterSrvComm::DbServerProto::dcCreateCSActorId: 
	{
		int nCSActorId = 0;
		reader >> nCSActorId;
		GetLogicServer()->GetCrossServerManager()->PostCrossServerMessage(SSM_CROSS_2_LOGIC_DATA,
		jxInterSrvComm::CrossServerProto::sGetCrossActorId,
		nRawServerId,nActorID,nCSActorId);
		break;
	}
	default:break;
	}
}


//聊天信息
bool CCrossMgr::OnGetChatData(CDataPacketReader& packet)
{ 
	unsigned int nSrvId = 0;//跨服id 
	int nPage = 0;
	UINT nNow = 0;
	packet >> nSrvId;
	packet >> nPage;
	packet >> nNow;
	//OutputMsg(rmTip,_T("[CCrossMgr 9] OnGetChatData------------------------  nSrvId:%d nPage:%d nNow:%d"), 
	//					nSrvId, nPage, (int)nNow);

	int nRawServerId = nSrvId ;
	 
	char buff[MAX_SEND_BUFF_SIZE];
	memset(buff, 0, sizeof(buff));
	CDataPacket outPack(buff, sizeof(buff));
	//CDataPacket &outPack = allocSendPacket(); 
	outPack << (unsigned int)nRawServerId;
	  
	CChatManager *pChatMgr = &GetGlobalLogicEngine()->GetChatMgr();  
	pChatMgr->OnMakeChatRecordPacket(outPack, nPage, nNow);
	
	
	GetLogicServer()->GetCrossClient()->SendRawServerData(jxInterSrvComm::CrossServerProto::sSendReqChat, outPack.getMemoryPtr(), outPack.getPosition());

	return true;
}
 
void CCrossMgr::KickAllCrossServerActor()
{
	CVector<void *> actorList;
	GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineAcotrPtrList(actorList);
	for (INT_PTR i = 0; i < actorList.count(); i++)
	{
		CActor *tmpActor = (CActor *)actorList[i];
		if (tmpActor && tmpActor->IsInited() && !tmpActor->IsDestory())
		{
			tmpActor->CloseActor(lwiGateUserLogout, false);
		}		
	}
	
}