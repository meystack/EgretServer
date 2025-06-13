#include "StdAfx.h"
#include "CrossSystem.h"

CCrossSystem::CCrossSystem()
{
	m_bCrossWorldOne = false;
}

CCrossSystem::~CCrossSystem()
{

}

bool CCrossSystem::Initialize(void *data,SIZE_T size)
{
	PACTORDBDATA  pActorData = (ACTORDBDATA *)data;
	if(pActorData ==NULL || size != sizeof(ACTORDBDATA))
	{
		return false;
	}

	return true;
}

void CCrossSystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet )
{
	switch(nCmd)
	{
	case cReqInit:
	{
		GetCrossServerActorId();
	}
	break;
	case cReqLogin:
	{
		LoginCrossServer();
	}break;
	default:
		break;
	}
}

void CCrossSystem::TriggerEvent(INT_PTR nSubEvent,INT_PTR nParam1,INT_PTR nParam2,INT_PTR nParam3,INT_PTR nParam4)
{
	// if(!m_pEntity) return;

	// CScriptValueList paramList;
	// ((CActor*)m_pEntity)->InitEventScriptVaueList(paramList,aeCrossOp);
	// paramList << (int)nSubEvent;
	// if(nParam1 >=0)
	// {
	// 	paramList << (int)nParam1;
	// 	if(nParam2 >=0)
	// 	{
	// 		paramList << (int)nParam2;
	// 		if(nParam3 >= 0)
	// 		{
	// 			paramList << (int)nParam3;
	// 		}
	// 	}	
	// }

	// ((CActor*)m_pEntity)->OnEvent(paramList,paramList);
}

void CCrossSystem::GetCrossServerActorId()
{
	if(!m_pEntity) return;

	unsigned int nActorId = m_pEntity->GetId();
	unsigned int nAccountId = m_pEntity->GetAccountID();
	int nCSSrvid = GetLogicServer()->GetCrossServerId();//跨服本服Srvid
	CLocalCrossClient* pCSClient = GetLogicServer()->GetCrossClient();
	if (pCSClient->connected())
	{
		int nCrossActorId = ((CActor*)m_pEntity)->GetProperty<unsigned int>(PROP_CROSS_SERVER_ACTORID);
		// if(!nCrossActorId) 
		{
			CDataPacket& DataPacket = pCSClient->allocProtoPacket(jxInterSrvComm::CrossServerProto::cGetCrossActorId);
			DataPacket<< GetLogicServer()->GetCrossServerId();
			DataPacket << (unsigned int)nActorId;
			DataPacket << nCrossActorId;
			DataPacket << (unsigned int)nAccountId; 
			 
			std::string strName = ""; 
			// int nCsIndex = pCSClient->OnGetCsIndex();  
			// CSSourceInfo* info = GetLogicServer()->GetLogicServerConfig()->getCSConfigSourceById(nCsIndex);
			// if(info) 
			// {
			// 	strName += "CSName";
			// 	strName += "."; 
			// }
			strName += m_pEntity->GetEntityName(); 
			DataPacket.writeString(strName.c_str()); 
			GetLogicServer()->GetCrossClient()->flushProtoPacket(DataPacket);
			OutputMsg(rmTip,_T("[CrossLogin 1] GetCrossServerActorId :%d"),nActorId);
		}
		// else {
		// 	SendCrossServerInfo(nCrossActorId);
		// 	 ((CActor*)m_pEntity)->SendLoginData2CrossServer();
		// }
		return;
	}
	
	CActorPacket ap;
	CDataPacket& data = m_pEntity->AllocPacket(ap);
	data <<(BYTE)enCrossServerSystemID <<(BYTE)sReqInit;
	data <<(BYTE)1;
	ap.flush();
	// ((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmNoMoreCoin, tstUI);
}


void CCrossSystem::LoginCrossServer()
{
	if(!m_pEntity) return;
	((CActor*)m_pEntity)->SendLoginData2CrossServer();
	
	// unsigned int nActorId = m_pEntity->GetId();
	// CLocalCrossClient* pCSClient = GetLogicServer()->GetCrossClient();
	// if (pCSClient->connected())
	// {
	// 	CDataPacket& DataPacket =pCSClient->allocProtoPacket(jxInterSrvComm::CrossServerProto::cReqCrossLogin);
	// 	DataPacket<< GetLogicServer()->GetCrossServerId();
	// 	DataPacket << (unsigned int)nActorId;
	// 	GetLogicServer()->GetCrossClient()->flushProtoPacket(DataPacket);
	// }
}


void CCrossSystem::SendCrossServerInfo(int nCrossActorId, int nSouthActorId)
{
	if(!m_pEntity) return;
	
	CSimulatorMgr *pSimuMgr = GetGlobalLogicEngine()->GetSimulatorMgr(); 
	int nCrossServerSrvId = GetLogicServer()->GetCrossServerId(); //跨服服务器id
	((CActor*)m_pEntity)->SetProperty<unsigned int>(PROP_CROSS_SERVER_ACTORID,nCrossActorId);
	if(!nCrossActorId ) {
		CActorPacket ap;
		CDataPacket& data = m_pEntity->AllocPacket(ap);
		data <<(BYTE)enCrossServerSystemID <<(BYTE)sReqInit;
		data <<(BYTE)1; //失败
		ap.flush();
		return;
	}
	//成功
	CLocalCrossClient* pCSClient = GetLogicServer()->GetCrossClient();
	int nCsIndex = pCSClient->OnGetCsIndex();  
	//CSSourceInfo* info = GetLogicServer()->GetLogicServerConfig()->getCSConfigSourceById(nCsIndex);
	//if(info) 
	// CROSSSERVERCFG* cfg = GetLogicServer()->GetLogicServerConfig()->getCSConfigCrossById(nCrossServerSrvId);
	// if(cfg)
	{ 
		CActorPacket ap;
		CDataPacket& data = m_pEntity->AllocPacket(ap);
		data <<(BYTE)enCrossServerSystemID <<(BYTE)sReqInit;
		data <<(BYTE)0; 
		data.writeString(pCSClient->OnGetCsGateAddr().c_str()); 
		//data.writeString(GetLogicServer()->m_cGateAddr.c_str()); 
		//data.writeString(GetLogicServer()->m_cGateAddr);
		data << (unsigned int)nCrossServerSrvId;
		data << (unsigned int)nCrossActorId;
		printf("SendCrossServerInfo:%d,ip:%s, port:%d\n",nCrossActorId, pCSClient->OnGetCsGateAddr().c_str(), pCSClient->OnGetCsGatePort());
		data << (unsigned int)(pCSClient->OnGetCsGatePort());
		ap.flush();
	}
	
	//跨服成功添加列表，防止原服创建虚拟玩家 
	pSimuMgr->OnAddRuningCsActor(nSouthActorId);

	OutputMsg(rmTip, _T("[simulator], 玩家跨服 add %d"), nSouthActorId );
	GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmActorLoginKuafu, tstKillDrop, (char*)(m_pEntity->GetEntityName()));
	
}