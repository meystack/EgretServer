#include "StdAfx.h"
#include "FriendSystem.h"
#include "../base/Container.hpp"
static const char *szDailyTrackNum	= "dailyTrackNum";		//每日追踪玩家的次数

using namespace jxInterSrvComm::DbServerProto;


bool CFriendSystem::Initialize(void *data,SIZE_T size)
{
	PACTORDBDATA  pActorData = (ACTORDBDATA *)data;
	if(pActorData == NULL || size != sizeof(ACTORDBDATA))
	{
		return false;
	}

	//InitActorMood();			//初始化玩家心情（签名）

	//LoadFriendsDataFromDb();
	//LoadFriendChatMsg();

	//LoadActorDeathFromDb();
	//LoadActorDeathDropFromDb();
	return true;
}

//玩家进入游戏
void CFriendSystem::OnEnterGame()
{
	//SendActorMood();			//下发心情
	//GraduateMaster();			//等级到了自动出师
	// SendSocialList(SOCIAL_FRIEND);
	// SendSocialList(SOCIAL_FOLLOW);
}


CFriendSystem::CFriendSystem()
{
	m_FriendList.clear();
	m_bSaveDbFlag = false;

	m_FirstSendList = 0;
	m_applyList.clear();
}

VOID CFriendSystem::OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader )
{
	switch(nCmd)
	{
	case dcLoadFriendsData:
		{
			if (nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
			{
				LoadFriendsFromDbResult(reader);
			}
			break;
		}

	// case dcLoadFriendChatMsg:
	// 	{
	// 		LoadFriendChatMsgResult(reader);
	// 		break;
	// 	}
	// case dcLoadFriendOffline:
	// 	{
	// 		LoadFriendsOfflineFromDbResult(reader);
	// 		break;
	// 	}
	case dcLoadDeathData:
		{
			if (nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
			{
				LoadActorDeathFromDbResult(reader);
			}
			break;
		}
	// case dcLoadDeathDropData:
	// 	{
	// 		if (nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
	// 		{
	// 			LoadActorDeathDropFromDbResult(reader);
	// 		}
	// 		break;
	// 	}
	default:
		{
			break;
		}
	}
}

void CFriendSystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet )
{
	switch (nCmd)
	{
	case cAddFriend:		//申请好友
	case cAddBackList:
	case cAddfollow:
		{
			AddSocialRelation(packet, nCmd);
		}break;

	case cAddResult:        //是否同意添加好友等
		{
			AddSocialResult(packet);	
		}break;
	case cDeleteFriend: //删除好友
		{
			ApplyDelete(packet);
		}break;
	case cGetSocialList: //获取列表
		{
			DealSocialList(packet);
		}break;
	case cReportList:
		{
		SendToClientActorDeathInfo();
	}break;
	case cSetFollowColor:
		{
		SetFollowColor(packet);
	}break;
	case cAllAcceptApply:
		{
			dealAllAcceptApply();
	}break;
	default:
		{
			break;
		}
	}
}



void CFriendSystem::AddSocialRelation(CDataPacketReader &packet, int type)
{
	if(!m_pEntity) return;

	unsigned int nActorId = 0;
	packet >> nActorId;
	char sName[32];
	packet.readString(sName,ArrayCount(sName));
	sName[sizeof(sName)-1] = 0;

	switch (type)
	{
		case cAddFriend:
		{
			AskAddFriend(nActorId, sName);
		}
			break;

		case cAddBackList:
		{
			AddToBlacklist(nActorId, sName);
		}
			break;

		case cAddfollow:
		{
			AddToFollowList(nActorId, sName);
		}
			break;
		default:
			break;
	}

	//DoAddSocialRelation(nIndex, bType, nActorId, sName,bNotice) ;
}


/*申请好友
nActorId：对方玩家id
sName：对方玩家name
注意：两者可能传入其一
*/
void CFriendSystem::AskAddFriend(unsigned int nActorId, ACTORNAME sName)
{
	if (m_pEntity == NULL) return;		//本人

	//通知玩家
	CActor* pEntity = NULL;		//对方
	if(nActorId > 0)
	{
		pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);
	}
	if(!pEntity)
	{
		pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(sName);
		
	}

	static int nAddMinLv = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nFriendChatLimit;
	if (!m_pEntity->CheckLevel(nAddMinLv, 0))
	{
		m_pEntity->SendTipmsgFormatWithId(tmAddFriendLevel, tstFigthing, nAddMinLv);
		return;
	}

	if (pEntity == NULL || pEntity->GetType() != enActor)
	{
		m_pEntity->SendTipmsgFormatWithId(tmTargetOffLine, tstUI); 
		return;
	}

	nActorId = pEntity->GetId();
	if(nActorId == ((CActor*)m_pEntity)->GetProperty<unsigned int>(PROP_ENTITY_ID))
	{
		m_pEntity->SendTipmsgFormatWithId(tmFriendnotaddSelf, tstUI, pEntity->GetEntityName()); 
		return;
	}

	int curNum = GetSocialCurnum(SOCIAL_FRIEND);

	if(curNum >= NUM_LINKMANLASTLY_MAX)
	{
		m_pEntity->SendTipmsgFormatWithId(tmFriendMaxCount,tstFigthing);
		return;
	}

	//int tagetMaxNum = pEntity->GetFriendSystem().GetSocialMaxNum(1);
	int tagetCurNum = pEntity->GetFriendSystem().GetSocialCurnum(SOCIAL_FRIEND);
	if(tagetCurNum >= NUM_LINKMANLASTLY_MAX)
	{
		m_pEntity->SendTipmsgFormatWithId(tmTagetFriendToMax,tstFigthing);
		return;
	}

	int nType = pEntity->GetFriendSystem().GetSocialResultById(m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));
	if(nType == SOCIAL_BLACK)
	{
		return;
	}

	if(GetSocialFlag(pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID),SOCIAL_FRIEND) == true) 
	{
		m_pEntity->SendTipmsgFormatWithId(tmHaveBeYourFriend,tstUI,pEntity->GetEntityName());
		return;
	}

	// if(pEntity->GetFriendSystem().CheckActorIdIsApply(((CActor*)m_pEntity)->GetProperty<unsigned int>(PROP_ENTITY_ID)))
	// {
	// 	return;
	// }

    pEntity->GetFriendSystem().dealApplyList(((CActor*)m_pEntity)->GetProperty<unsigned int>(PROP_ENTITY_ID));
	SendCMdSocialData(SOCIAL_FRIEND, 8,  pEntity); //发送申请

	m_pEntity->SendTipmsgFormatWithId(tmApplyFriendSuccess,tstFigthing);
}




void CFriendSystem::AddToBlacklist(unsigned int nActorId, ACTORNAME sName)
{
	if(m_pEntity == NULL) return;

	CActor* pEntity = NULL;

	if(nActorId > 0)
	{
		pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);
	}

	if(!pEntity)
	{
		pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(sName);
	}

	// unsigned int nTargetId = pEntity->GetId();
	// int nState = GetSocialResultById(nTargetId);
	if(pEntity)
	{
		unsigned int nTargetId = pEntity->GetId();
		if(nTargetId == ((CActor*)m_pEntity)->GetProperty<unsigned int>(PROP_ENTITY_ID))
		{
			m_pEntity->SendTipmsgFormatWithId(tmFriendnotaddSelf, tstUI); 
			return;
		}
		if(GetSocialFlag(nTargetId,SOCIAL_BLACK)) return;

		int curNum = GetSocialCurnum(SOCIAL_BLACK);

		if(curNum >= NUM_LINKMANLASTLY_MAX)
		{
			m_pEntity->SendTipmsgFormatWithId(tmBlackMaxCount,tstFigthing);
			return;
		}

		SendAddFriendInfo(nTargetId,SOCIAL_BLACK); //添加黑名单
		SetSocialFlag(nTargetId, SOCIAL_FRIEND, false); //我方解除好友关系

		((CActor*)pEntity)->GetFriendSystem().SetSocialFlag(m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID), SOCIAL_FRIEND, false); //对方解除好友关系
	}
	else
	{
		m_pEntity->SendTipmsgFormatWithId(tmTargetOffLine,tstUI);
		return;
		// if(nActorId > 0)
		// 	SendDeleteMemToDb(nActorId,m_pEntity->GetId());
	}
	m_pEntity->SendTipmsgFormatWithId(tmAddEnamySuccess, tstUI);
}



void CFriendSystem::AddToFollowList(unsigned int nActorId, ACTORNAME sName)
{
	if(m_pEntity == NULL) return;

	CActor* pEntity = NULL;

	if(nActorId > 0)
	{
		pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);
	}

	if(!pEntity)
	{
		pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(sName);
	}

	if(pEntity)
	{
		unsigned int nTargetId = pEntity->GetId();
		if(nTargetId == ((CActor*)m_pEntity)->GetProperty<unsigned int>(PROP_ENTITY_ID))
		{
			m_pEntity->SendTipmsgFormatWithId(tmFriendnotaddSelf, tstUI); 
			return;
		}
		int nState = GetSocialResultById(nTargetId);

		if(GetSocialFlag(nTargetId, SOCIAL_FOLLOW)) return;

		int curNum = GetSocialCurnum(SOCIAL_FOLLOW);

		if(curNum >= NUM_LINKMANLASTLY_MAX)
		{
			m_pEntity->SendTipmsgFormatWithId(tmFollwerMaxCount,tstFigthing);
			return;
		}

		SendAddFriendInfo(nTargetId, SOCIAL_FOLLOW);
		m_pEntity->SendTipmsgFormatWithId(tmAddEnamySuccess, tstUI);
	}
	else
	{
		m_pEntity->SendTipmsgFormatWithId(tmTargetOffLine,tstUI);
	}
	
}





// void CFriendSystem::DeleteStranger(unsigned int nActorId,char* sName)
// {
// 	SendDeleteSocial(nActorId,sName,2);
// }

/*
删除好友
*/
void CFriendSystem::DeleteFriends(unsigned int nActorId,char* sName, BYTE type)
{
	if(m_pEntity == NULL) return;

	CActor* pEntity = NULL;

	if(nActorId > 0)
	{
		pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);
	}

	if(!pEntity)
	{
		pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(sName);
	}
	
	unsigned int actorId = SendDeleteSocial(nActorId, sName, type);	//删除本人关系列表中与对方的好友关系
	//删除对方关系列表中与自己的好友关系
	if(actorId)
	{
		if(type == SOCIAL_FRIEND)
		{	
			if(pEntity)
			{
				pEntity->GetFriendSystem().SendDeleteSocial(m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID),NULL,SOCIAL_FRIEND);
			}
			else
			{
				SendDeleteMemToDb(actorId,m_pEntity->GetId());
			}
		}
	}
	
	// UpdateTemBuf();

}


void CFriendSystem::SetFollowColor(CDataPacketReader &packet)
{
	BYTE color = 0;
	packet >> color;
	unsigned int nActorid = 0;
	packet >> nActorid;
	if(GetSocialFlag(nActorid, SOCIAL_FOLLOW)) {
		FRIENDRELATION* pFriend = GetFriendLationByActorId(nActorid);
		if(pFriend) {

			pFriend->ncolor = color;
			m_bSaveDbFlag = true;
		}
	}
	SendSocialList(SOCIAL_FOLLOW);

}

FRIENDRELATION* CFriendSystem::GetFriendLationByActorId(unsigned int nActorid)
{
	int count = m_FriendList.count();
	for(int i =0; i < count; i++)
	{
		if(m_FriendList[i].nActorId == nActorid)
		{
			return &m_FriendList[i];
		}
	}
	return NULL;
}


/*解除好友关系
nActorId:是对方id，
FriendId：本人ID
*/
void CFriendSystem::SendDeleteMemToDb(unsigned int nActorId,unsigned int FriendId)
{
	//OutputMsg(rmNormal, "CFriendSystem::SendDeleteMemToDb, nActorId=%u, FriendId=%u, nState=%d", nActorId, FriendId, nState);
	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		// int nIndex = GetSocialIndexById(nActorId, NULL);		//本人与nActorId的关系，此时nActorId是不在线的，-1表示没有任何关系
		// FRIENDRELATION newFriend;			//

		// //if(nIndex < 0) return;  这里会导致另一方不在线的情况
		// int  nIntimacy = 0;				//亲密度
		// long long  nWarTeamId = 0;			//战队id
		// if(nIndex >= 0)		//关系还存在
		// {
		// 	FRIENDRELATION &nFriend = m_FriendList[nIndex];
		// 	nIntimacy = nFriend.nIntimacy; 
		// 	nWarTeamId = nFriend.nWarTeamId;
		// }
		

		CDataPacket& DataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcDeleteFriendData);

		DataPacket<< m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
		DataPacket << (unsigned int)nActorId;
		DataPacket << (unsigned int)FriendId;
		// DataPacket << (int)nState;
		// DataPacket << nIntimacy;
		// DataPacket << nWarTeamId;
		// DatePacket <<(int)nType;

		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	}
}



/*实际操作
nResult：1-同意，0-不同意
nType:  1 好友 5 师傅 6 徒弟
*/
void CFriendSystem::DoAddFriend(BYTE nResult, unsigned int nActorId)
{
	if(m_pEntity == NULL) return;
	if(nResult == 1)
	{
		int index = GetSocialIndexById(nActorId);
		
		CActor* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);
		if(!pEntity )		//对方已下线
		{
			if(nResult == 1)
				m_pEntity->SendTipmsgFormatWithId(tpActorOffline, tstFigthing);
			return;
		}
		int curNum = GetSocialCurnum(SOCIAL_FRIEND);

		if(curNum >= NUM_LINKMANLASTLY_MAX)
		{
			m_pEntity->SendTipmsgFormatWithId(tmFriendMaxCount, tstFigthing);
			return;
		}
		pEntity->GetFriendSystem().SendAddFriendInfo(m_pEntity->GetProperty<int>(PROP_ENTITY_ID), SOCIAL_FRIEND);

		SendAddFriendInfo(nActorId, SOCIAL_FRIEND);

		pEntity->GetFriendSystem().SetSocialFlag(m_pEntity->GetProperty<int>(PROP_ENTITY_ID), SOCIAL_BLACK, false); //需要解除对方黑名单
		dealApplyList(nActorId, 2);
		((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmAddFriendNotice,tstUI, pEntity->GetEntityName());
		((CActor*)pEntity)->SendTipmsgFormatWithId(tmAddFriendNotice,tstUI, m_pEntity->GetEntityName());
		//AddFriendResult(pEntity,nResult);
	}
	
}

/*处理请求
*/
void CFriendSystem::AddSocialResult(CDataPacketReader &packet)
{
	if(m_pEntity == NULL) return;

	int curNum = GetSocialCurnum(SOCIAL_FRIEND);

	// if(curNum >= NUM_LINKMANLASTLY_MAX)
	// {
	// 	m_pEntity->SendTipmsgFormatWithId(tmFriendMaxCount, tstFigthing);
	// 	return;
	// }

	BYTE nResult = 0;
	packet >> nResult;

	unsigned int nActorId;
	packet >> nActorId;

	DoAddFriend(nResult, nActorId);
}

void CFriendSystem::dealAllAcceptApply()
{
	int count = m_applyList.count();
	if(count <= 0)
		return;
	for(int i = count-1; i >-1; i--)
	{
		unsigned int nActorId = m_applyList[i];
		DoAddFriend(1, nActorId);
	}
	m_applyList.clear();
}


void CFriendSystem::ApplyDelete(CDataPacketReader &packet)
{
	if(m_pEntity == NULL) return;

	BYTE type = 0;
	packet >> type;

	unsigned int nActorId;
	packet >> nActorId;
	DeleteFriends(nActorId, NULL, type);
}

/*pEntity:对方玩家
*/
void CFriendSystem::AddFriendResult(CActor* pEntity,int nResult)
{
	if(pEntity == NULL) return;
	if(m_pEntity == NULL) return;

	/*不存在这个玩家的申请消息
	if(IsExitSocialMsg(pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID),SOCIAL_FRIEND) == false)
		return;
	*/
	if(nResult == 1)	//添加好友
	{
		SendAddFriendInfo(pEntity->GetId(),SOCIAL_FRIEND);
		((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmAddFriendNotice,tstUI, pEntity->GetEntityName());

		// UpdateTemBuf();
	}

}


unsigned int CFriendSystem::SendDeleteSocial(unsigned int nActorId,char* sName,int nType)
{
	INT_PTR nCount = m_FriendList.count();

	if(GetSocialFlag(nActorId,nType) == false) 
		return 0;

	m_bSaveDbFlag = true;

	for(INT_PTR i=0;i<nCount;i++)
	{
		FRIENDRELATION &nFriend = m_FriendList[i];
		unsigned int actorId = nFriend.nActorId;

		if(nActorId == nFriend.nActorId || (sName && strcmp(nFriend.sName,sName)==0))
		{
			nFriend.nIntimacy = 0;
			int nState = SetSocialFlag(nActorId,nType, false);
	
			CActorPacket AP;
			CDataPacket& DataPacket = ((CActor*)m_pEntity)->AllocPacket(AP);
			DataPacket << (BYTE)enFriendLogicSystemID << (BYTE)sDeleteFriend;
			// DataPacket << (BYTE)2;
			// DataPacket << (BYTE)nState;
			DataPacket << (unsigned int)actorId;

			AP.flush();
			return actorId;
		}
	}

	return 0;
}


void CFriendSystem::SendAddFriendInfo(unsigned int nActorId,int nType, int nCmd)
{
	if(m_pEntity == NULL || m_pEntity->IsInited() ==false) return;
	CActor* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);

	if(pEntity && pEntity->IsInited() )
	{
		m_bSaveDbFlag = true;
		int nIndex = GetSocialIndexById(nActorId,NULL);
		//FRIENDRELATION newFriend;

		if(nIndex >= 0)
		{
			FRIENDRELATION &nFriend = m_FriendList[nIndex];
			nFriend.nState |= (1 << nType);
			//SetSocialFlag(nFriend.nState, nType, nCmd)
			//nFriend.nState = nType;
			//newFriend = nFriend;
		}
		else
		{
			FRIENDRELATION newFriend;
			newFriend.nActorId = nActorId;
			newFriend.nIcon = pEntity->GetProperty<int>(PROP_ENTITY_ICON);
			newFriend.nJop = pEntity->GetProperty<int>(PROP_ACTOR_VOCATION);
			newFriend.nLevel = pEntity->GetProperty<int>(PROP_CREATURE_LEVEL);
			newFriend.nSex = pEntity->GetProperty<int>(PROP_ACTOR_SEX);
			newFriend.nIntimacy = 0;
			_asncpytA(newFriend.sName,pEntity->GetEntityName());
			newFriend.nState |= (1 << nType);
			//newFriend.nState = nType;
			newFriend.nTime = time(NULL);//GetGlobalLogicEngine()->getMiniDateTime();		//当前时间
			//_asncpytA(newFriend.sGuildname,pEntity->GetEntityName());
			CGuild* pMyGuild =((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
			if(pMyGuild)
			{
				_asncpytA(newFriend.sGuildname,pMyGuild->m_sGuildname);
			}
			m_FriendList.add(newFriend);
		}
		if(nCmd != 0)
			SendCMdSocialData(nType, nCmd, pEntity);
		
		SendSocialList(nType);
	}
}




void CFriendSystem::dealApplyList(unsigned int nActorId, int type)
{
	if(type == 1)
	{
		if(m_pEntity == NULL || m_pEntity->IsInited() ==false) return;
		CActor* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);

		if(pEntity && pEntity->IsInited() )
		{
			if(!CheckActorIdIsApply(nActorId))
			{
				m_applyList.add(nActorId);
			}
		}
	}else
	{
		int count = m_applyList.count();
		for(int i = 0; i < count; i++)
		{
			if(m_applyList[i] == nActorId)
			{
				m_applyList.remove(i);
				return;
			}
		}
	}
	
}



int CFriendSystem::GetSocialResultById(unsigned int nActorId)
{
	INT_PTR nCount = m_FriendList.count();

	for(INT_PTR i=0;i<nCount;i++)
	{
		FRIENDRELATION &nFriend = m_FriendList[i];
		if(nFriend.nActorId == nActorId)
		{
			return (int)(nFriend.nState);
		}
	}

	return -1;
}

int CFriendSystem::GetSocialIndexById(unsigned int nActorId,char* sName)
{
	INT_PTR nCount = m_FriendList.count();

	for(INT_PTR i=0;i<nCount;i++)
	{
		FRIENDRELATION &nFriend = m_FriendList[i];
		if(nFriend.nActorId == nActorId || (sName && strcmp(sName,nFriend.sName)==0))
		{
			return (int)i;
		}
	}

	return -1;
}


void CFriendSystem::SendSocialList(int type)
{ 
    if (((CActor*)m_pEntity)->OnGetIsTestSimulator()
        || ((CActor*)m_pEntity)->OnGetIsSimulator())
    {
        return; 
    }

	CActorPacket AP;
	CDataPacket& DataPacket = ((CActor*)m_pEntity)->AllocPacket(AP);
	DataPacket << (BYTE)enFriendLogicSystemID << (BYTE)sSendSocialList;
	size_t pos = DataPacket.getPosition();
	DataPacket << (int)0;//未必是真实的数量
	int nResult = 0;
	DataPacket << (BYTE)type;

	INT_PTR nCount = m_FriendList.count();

	for(INT_PTR i = 0;i < nCount;i++)
	{
		FRIENDRELATION &nFriend = m_FriendList[i];
		//if(nFriend.nState == cType)
		if(nFriend.nState & (1 << type))
		{
			//DataPacket << (BYTE)nFriend.nState;
			DataPacket << nFriend.nActorId;
			CActor* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nFriend.nActorId);
			int nVip = 0;
			if(pEntity)
			{
				DataPacket.writeString(pEntity->GetEntityName());
				DataPacket << (BYTE)(pEntity->GetProperty<int>(PROP_ACTOR_VOCATION));
				DataPacket << (int)(pEntity->GetProperty<int>(PROP_CREATURE_LEVEL));
				DataPacket << (BYTE)(pEntity->GetProperty<int>(PROP_ENTITY_ICON));
				DataPacket << (BYTE)(pEntity->GetProperty<int>(PROP_ACTOR_SEX));
				DataPacket <<(BYTE)nFriend.ncolor;
				DataPacket << (BYTE)(pEntity->GetProperty<int>(PROP_ACTOR_CIRCLE));
				DataPacket << (BYTE)1;
				CGuild* pMyGuild =((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
				if(NULL == pMyGuild)
				{
					DataPacket.writeString("");
				}
				else
				{
					DataPacket.writeString(pMyGuild->m_sGuildname);	//自己的公会名字
				}
				nVip =  (unsigned int)(pEntity->GetProperty<unsigned int>(PROP_ACTOR_SUPPER_PLAY_LVL));//超玩俱乐部等级
			}
			else
			{
				DataPacket.writeString(nFriend.sName);
				DataPacket << (BYTE)nFriend.nJop;
				DataPacket << (int)nFriend.nLevel;
				DataPacket << (BYTE)nFriend.nIcon;
				// DataPacket.writeString(nFriend.nSignInfo);
				DataPacket << (BYTE)nFriend.nSex;
				DataPacket <<(BYTE)nFriend.ncolor;
				DataPacket <<(BYTE)nFriend.ncircle;
				DataPacket << (BYTE)0;
				DataPacket.writeString(nFriend.sGuildname);	//自己的公会名字
				nVip = nFriend.nVip;//超玩俱乐部等级
			}
			// DataPacket << (int)nFriend.nIntimacy;
			DataPacket << (unsigned int)nFriend.nTime;
			DataPacket << (unsigned int)nVip;
			nResult++;
		}
	}

	int* pCount = (int*)DataPacket.getPositionPtr(pos);
	*pCount = nResult;

	AP.flush();
 
	m_FirstSendList = 1;
}

void CFriendSystem::DealSocialList(CDataPacketReader &packet)
{
	BYTE  cType = 1;
	packet >> cType;
	SendSocialList(cType);
}



void CFriendSystem::LoadFriendsDataFromDb()
{
	if(!m_pEntity)
	{ 
		return;
	}

	if (((CActor*)m_pEntity)->OnGetIsTestSimulator())
	{
	 	return;
	}
	unsigned int nActorId = m_pEntity->GetId();
	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		CDataPacket& DataPacket =pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadFriendsData);
		DataPacket<< m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
		DataPacket << nActorId;
		
		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	} 
}

void CFriendSystem::LoadActorDeathFromDb()
{
	if(!m_pEntity) return;

	unsigned int nActorId = m_pEntity->GetId();

	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		CDataPacket& DataPacket =pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadDeathData);
		DataPacket<< m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
		DataPacket << nActorId;
		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	}
}


/*读取db时，有order by，所以m_FriendList 按nTime从小到大排序
*/
void CFriendSystem::LoadFriendsFromDbResult(CDataPacketReader &reader)
{
	if(m_pEntity == NULL) return;
	int nCount = 0;
	reader >> nCount;

	if(nCount)
	{
		m_FriendList.reserve(nCount);		//reserve()函数提前设定容量大小，避免多次容量扩充操作导致效率低下。
		m_FriendList.trunc(nCount);	

		for (int i = 0; i < nCount; i++)
		{
			FRIENDRELATION &tmpRelation = m_FriendList[i];

			reader >> tmpRelation.nActorId;
			int nType = 0;
			reader >> nType;
			tmpRelation.nState = (BYTE)nType;
			reader.readString(tmpRelation.sName,ArrayCount(tmpRelation.sName));
			reader >> tmpRelation.nSex;
			reader >> tmpRelation.nLevel;
			reader >> tmpRelation.nJop;
			reader >> tmpRelation.nIcon;
			///reader.readString(tmpRelation.nSignInfo,ArrayCount(tmpRelation.nSignInfo));
			reader.readString(tmpRelation.sGuildname,ArrayCount(tmpRelation.sGuildname));
			reader >> tmpRelation.nIntimacy;
			reader >> tmpRelation.nWarTeamId;
			reader >> tmpRelation.nTime;
			reader >> tmpRelation.ncolor;
			reader >> tmpRelation.ncircle;
			reader >> tmpRelation.nVip;
			//OutputMsg(rmError,"CFriendSystem::LoadFriendsFromDbResult, nTime=%d", tmpRelation.nTime);
		}
	}
	((CActor*)m_pEntity)->OnFinishOneInitStep(eMSG_USR_LOGIN_DATA_FRIEND_SYSTEM); //完成一个步骤
	FriendLoginNotice();
}

void CFriendSystem::LoadActorDeathFromDbResult(CDataPacketReader &reader)
{
	int nCount = 0;
	reader >> nCount;

	unsigned int nActorId = 0;
	if(nCount)
	{
		m_ActorDeathList.reserve(nCount);		//reserve()函数提前设定容量大小，避免多次容量扩充操作导致效率低下。
		m_ActorDeathList.trunc(nCount);	

		for (int i = 0; i < nCount; i++)
		{
			ACTORDEATH &stDeath = m_ActorDeathList[i];
			reader >> nActorId;
			reader >> stDeath.nKillerId;
			reader >> stDeath.nKillTime;
			reader.readString(stDeath.sKillerName, ArrayCount(stDeath.sKillerName));
			reader >> stDeath.nSceneId;
			reader >> stDeath.nType;
			// reader >> stDeath.nSceneId;
			// reader >> stDeath.nKillPosX;
			// reader >> stDeath.nKillPosY;
			//OutputMsg(rmNormal,"LoadDeathFromDbResult, nSceneId=%d, nKillPosX=%d, nKillPosY=%d", 
			//	stDeath.nSceneId, stDeath.nKillPosX, stDeath.nKillPosY);
		}
	}
}


void CFriendSystem::SaveToDb()
{
	//OutputMsg(rmNormal,"CFriendSystem::SaveToDb");
	if(!m_pEntity) return;

	//FriendLoginOut();

	if(!m_bSaveDbFlag) return;

	unsigned int nActorId = m_pEntity->GetId();

	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		CDataPacket& DataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveFriendsData);

		DataPacket<< m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
		DataPacket << nActorId;

		size_t pos = DataPacket.getPosition();
		DataPacket << (int)0;//未必是真实的数量

		INT_PTR nCount = m_FriendList.count();
		//DataPacket << (int)nCount;
		int nResult = 0;
		for(INT_PTR i=0;i<nCount;i++)
		{
			FRIENDRELATION &tmpRelation = m_FriendList[i];

			if(tmpRelation.nState > 0)
			{
				nResult++;
				DataPacket << tmpRelation.nActorId;
				DataPacket << (int)tmpRelation.nState;
				DataPacket << tmpRelation.nIntimacy;
				DataPacket << tmpRelation.nWarTeamId;
				DataPacket << tmpRelation.nTime;
				DataPacket << tmpRelation.ncolor;
				//OutputMsg(rmNormal,"CFriendSystem::SaveToDb, nTime=%d",tmpRelation.nTime);
			}
		}

		int* pCount = (int*)DataPacket.getPositionPtr(pos);
		*pCount = nResult;

		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	}
	m_bSaveDbFlag = false;

	// SetSocialNeedFresh(nActorId);
}

//策划案-战报功能
void CFriendSystem::SaveActorDeathToDb()
{
	if(!m_pEntity) return;
	unsigned int nActorId = m_pEntity->GetId();
	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		CDataPacket& DataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveDeathData);

		DataPacket<< m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
		DataPacket << nActorId;

		DeleteDeathData();		//如果死亡数据超过最大数，则删掉最先的数据

		size_t pos = DataPacket.getPosition();
		DataPacket << (int)0;//未必是真实的数量
		INT_PTR nCount = m_ActorDeathList.count();
		//DataPacket << (int)nCount;
		int nResult = 0;
		// unsigned int nDieListCount = GetLogicServer()->GetDataProvider()->GetFriendConfig().m_DieListCount;
		if( nCount > NUM_DEATHLIST_MAX )
		{
			nCount = NUM_DEATHLIST_MAX;		//如果死亡数据超过最大数，等于最大数
		}

		for(INT_PTR i=0;i<nCount;i++)
		{
			ACTORDEATH &tmpDeath = m_ActorDeathList[i];
			nResult++;
			DataPacket << (unsigned int )tmpDeath.nKillerId;
			DataPacket.writeString(tmpDeath.sKillerName);		//可能是玩家或怪
			
			DataPacket << (unsigned int)tmpDeath.nKillTime;
			DataPacket << (int)tmpDeath.nSceneId;
			DataPacket << (BYTE)tmpDeath.nType;
			// DataPacket << tmpDeath.nKillPosX;
			// DataPacket << tmpDeath.nKillPosY;
		}

		int* pCount = (int*)DataPacket.getPositionPtr(pos);
		*pCount = nResult;
		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	}
}


void CFriendSystem::DeleteEnemy(unsigned int nActorId,char* sName)
{
	SendDeleteSocial(nActorId,sName,3);
}


int CFriendSystem::GetSocialMaxNum(int nType)
{
	switch(nType)
	{
	case 1:			//好友
		{
			return GetLogicServer()->GetDataProvider()->GetFriendConfig().m_MaxFriendNum;
		}

	case 2:
		{
			return GetLogicServer()->GetDataProvider()->GetFriendConfig().m_MaxStrangerNum;
		}

	case 3:
		{
			return GetLogicServer()->GetDataProvider()->GetFriendConfig().m_MaxEnemyNum;
		}

	case 4:			//仇敌
		{
			return GetLogicServer()->GetDataProvider()->GetFriendConfig().m_MaxEnemyNum;
		}

	case 5:		//师父
		{
			return GetLogicServer()->GetDataProvider()->GetFriendConfig().m_MaxMaster;
		}

	case 6:		//徒弟
		{
			return GetLogicServer()->GetDataProvider()->GetFriendConfig().m_maxDisciple;
		}
	}

	return 0;
}

int CFriendSystem::GetSocialCurnum(int nType)
{
	INT_PTR nCount = m_FriendList.count();

	int nResult = 0;

	for(INT_PTR i=0;i<nCount;i++)
	{
		if(GetSocialFlag(m_FriendList[i].nActorId,nType))
		{
			nResult++;
		}
	}

	return nResult;
}

void CFriendSystem::FriendLoginOut()
{
	if(m_pEntity == NULL) return;

	INT_PTR nCount = m_FriendList.count();

	for(INT_PTR i=0;i<nCount;i++)
	{
		CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(m_FriendList[i].nActorId);

		if(pActor)
		{
			pActor->GetFriendSystem().UpdateFriendData((CActor*)m_pEntity);
		}
	}
}

void CFriendSystem::UpdateFriendData(CActor* pEntity)
{
	if(m_pEntity == NULL) return;

	if(pEntity == NULL) return;

	INT_PTR nCount = m_FriendList.count();
	for(int i=0;i<nCount;i++)
	{
		if(m_FriendList[i].nActorId == pEntity->GetId())
		{
			_asncpytA(m_FriendList[i].sName,pEntity->GetEntityName());
			CGuild* pMyGuild =((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
			if(pMyGuild)
			{
				_asncpytA(m_FriendList[i].sGuildname,pMyGuild->m_sGuildname);
			}
			m_FriendList[i].nLevel = pEntity->GetProperty<int>(PROP_CREATURE_LEVEL);
			m_FriendList[i].nJop = pEntity->GetProperty<int>(PROP_ACTOR_VOCATION);
			m_FriendList[i].nIcon = pEntity->GetProperty<int>(PROP_ENTITY_ICON);
			m_FriendList[i].nSex = pEntity->GetProperty<int>(PROP_ACTOR_SEX);
			m_FriendList[i].ncircle = pEntity->GetProperty<int>(PROP_ACTOR_CIRCLE);
			_asncpytA(m_FriendList[i].sName,pEntity->GetEntityName());
			break;
		}
	}
}



void CFriendSystem::FriendLoginNotice()
{
	if(m_pEntity == NULL) return;

	INT_PTR nCount = m_FriendList.count();

	for(INT_PTR i=0;i<nCount;i++)
	{
		CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(m_FriendList[i].nActorId);

		if(pActor)
		{
			

			//对方在本人的好友列表中，并且本人在对方的好友列表中
			if( GetSocialFlag(m_FriendList[i].nActorId,SOCIAL_FRIEND) && 
				pActor->GetFriendSystem().GetSocialFlag( m_pEntity->GetId() ,SOCIAL_FRIEND) )
			{
				pActor->SendOldTipmsgFormatWithId(tpFriendLoginTips,ttChatWindow,m_pEntity->GetEntityName());
				continue;
			}
		}
	}
}


unsigned int CFriendSystem::GetSocialRelationId(int nType)
{
	if(m_pEntity == NULL) return 0;
	INT_PTR nCount = m_FriendList.count();
	for(INT_PTR i=0;i<nCount;i++)
	{
		if(m_FriendList[i].nState & (1<<nType))
		{
			return m_FriendList[i].nActorId;
		}
	}

	return 0;
}

char* CFriendSystem::GetSocialActorName(unsigned int nActorId)
{
	if(m_pEntity == NULL) return NULL;
	INT_PTR nCount = m_FriendList.count();
	for(INT_PTR i=0;i<nCount;i++)
	{
		if(m_FriendList[i].nActorId == nActorId)
		{
			return m_FriendList[i].sName;
		}
	}

	return NULL;
}

bool CFriendSystem::GetSocialFlag(unsigned int nActorId,int nType)
{
	if(m_pEntity == NULL) return false;

	INT_PTR nCount = m_FriendList.count();
	for(INT_PTR i=0;i<nCount;i++)
	{
		if(m_FriendList[i].nActorId == nActorId)
		{
			return (m_FriendList[i].nState & (1<<nType))?true:false;
		}
	}

	return false;
}

int CFriendSystem::SetSocialFlag(unsigned int nActorId,int nType,bool nState)
{
	if(m_pEntity == NULL) return 0;

	int nReuslt = 0;

	INT_PTR nCount = m_FriendList.count();
	for(INT_PTR i=0;i<nCount;i++)
	{
		if(m_FriendList[i].nActorId == nActorId)
		{
			if(nState)
			{
				m_FriendList[i].nState |= (1 << nType);
			}
			else
			{
				m_FriendList[i].nState &= ~(1 << nType);
			}
			//m_FriendList[i].nUpdateClient = 1;
			m_bSaveDbFlag = true;

			nReuslt = m_FriendList[i].nState;

			if(m_FriendList[i].nState == 0)
			{
				m_FriendList.remove(i);
			}

		}
	}

	return nReuslt;
}


void CFriendSystem::SendCMdSocialData(int nType, int ncmd, CActor* pEntity)
{
	if(pEntity == NULL) return;
	if(m_pEntity == NULL) return;

	CActorPacket AP;
	CDataPacket& DataPacket = pEntity->AllocPacket(AP);
	DataPacket << (BYTE)enFriendLogicSystemID << (BYTE)ncmd;
	//DataPacket << (BYTE)nType;
	unsigned int myActorId = m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID);
	DataPacket << (unsigned int)myActorId;
	DataPacket.writeString(m_pEntity->GetEntityName());
	//DataPacket << (int)(m_pEntity->GetProperty<int>(PROP_ACTOR_BATTLE_POWER));
	DataPacket << (int)(m_pEntity->GetProperty<int>(PROP_CREATURE_LEVEL));
	DataPacket << (BYTE)(m_pEntity->GetProperty<int>(PROP_ACTOR_VOCATION));
	DataPacket << (BYTE)(m_pEntity->GetProperty<int>(PROP_ACTOR_SEX));
	DataPacket << (unsigned int)(m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_SUPPER_PLAY_LVL));
	// DataPacket << (BYTE)0;
	// CGuild* pMyGuild =((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
	// if(NULL == pMyGuild)
	// {
	// 	DataPacket.writeString("");
	// }
	// else
	// {
	// 	DataPacket.writeString(pMyGuild->m_sGuildname);	//自己的公会名字
	// }
	
	// uint time =  GetGlobalLogicEngine()->getMiniDateTime();
	// DataPacket <<time;
	AP.flush();

}

//0被杀 1杀死
void CFriendSystem::AddDeath(CEntity* pEntity, int nType)
{
	ACTORDEATH					m_ActorDeath;
	m_ActorDeath.nType = nType;
	m_ActorDeath.nActorId = ((CActor*)m_pEntity)->GetId();
	INT_PTR nEntityType = pEntity->GetType();
	m_ActorDeath.nKillerId = 0;
	if( nEntityType == enActor )
	{
		m_ActorDeath.nKillerId = ((CActor*)pEntity)->GetId();
	}
	_asncpytA(m_ActorDeath.sKillerName , pEntity->GetEntityName());
	m_ActorDeath.nSceneId  = m_pEntity->GetSceneID();
	m_ActorDeath.nKillTime = time(NULL);

	// int nPosX=0;
	// int nPosY=0;
	// ((CEntity*)m_pEntity)->GetPosition(nPosX,nPosY);
	// m_ActorDeath.nKillPosX = nPosX;
	// m_ActorDeath.nKillPosY = nPosY;

	// CScene* pScene = ((CEntity*)pEntity)->GetScene();
	// if (pScene)
	// {
	// 	_asncpytA(m_ActorDeath.sSceneName , pScene->GetSceneName());
	// }
	m_ActorDeathList.add(m_ActorDeath);
	m_bSaveDbFlag = true;
	DeleteDeathData();
}



//删掉多余的死亡数据（确保数据不超过最大数)
void CFriendSystem::DeleteDeathData()
{
	INT_PTR nCount = m_ActorDeathList.count();
	//unsigned int nDieListCount = //GetLogicServer()->GetDataProvider()->GetFriendConfig().m_DieListCount;
	if( nCount > NUM_DEATHLIST_MAX)
	{
		for(int i=0; i<nCount; i++)
		{
			ACTORDEATH stDeath = m_ActorDeathList[0];	//
			m_ActorDeathList.remove(0);		//删除第一个元素，即最早的元素
			if( m_ActorDeathList.count() <= NUM_DEATHLIST_MAX )		//数据在4条之内
			{
				break;
			}
		}
	}
}



void CFriendSystem::SendToClientActorDeathInfo()
{
	INT_PTR nCount = m_ActorDeathList.count();
	CActorPacket AP;
	CDataPacket& DataPacket = ((CActor*)m_pEntity)->AllocPacket(AP);
	DataPacket << (BYTE)enFriendLogicSystemID << (BYTE)sReportList;
	DataPacket << (BYTE)nCount;
	for( INT_PTR i=0; i<nCount; i++ )
	{
		ACTORDEATH stDeath = m_ActorDeathList[i];
		DataPacket<<(unsigned int)stDeath.nKillerId;
		DataPacket.writeString(stDeath.sKillerName);
		DataPacket<<(unsigned int)stDeath.nKillTime;
		DataPacket<<(unsigned int)stDeath.nSceneId;
		DataPacket<<(BYTE)stDeath.nType;
	}
	AP.flush();
}


//
bool CFriendSystem::CheckActorIdIsApply(unsigned int nActorId)
{
	if (m_pEntity == NULL)
	{
		return false;
	}
	int count = m_applyList.count();
	for(int i =0; i< count; i++)
	{
		unsigned int id = m_applyList[i];
		if(id == nActorId)
		{
			return true;
		}
	}
	return false;
}
