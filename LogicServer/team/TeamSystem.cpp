#include "StdAfx.h"
#include "../entity/Actor.h"
#include "TeamSystem.h"

/// 附近玩家最大人数
#define MAX_NEAR_ACTOR_COUNT 30

/// 一键邀请最大人数
#define MAX_NOTEAM_ACTOR_COUNT 10

template<>
const CTeamSystem::Inherid::OnHandleSockPacket CTeamSystem::Inherid::Handlers[]=
{
	&CTeamSystem::HandleError,					// 0 
	&CTeamSystem::GetMyTeamInfo,                //1    cGetMyTeamInfo 16-11
	&CTeamSystem::HandleInvite,					// 2	cInviteJoinTeam 
	&CTeamSystem::HandleLeave,					// 3	cLeaveTeam
	&CTeamSystem::HandleApplyJoinTeam,			// 4	cApplyJoinTeam
	&CTeamSystem::HandleSetCaptin,				// 5	cSetCaptin
	&CTeamSystem::HandleKickMember,				// 6	cKickMember
	&CTeamSystem::HandleDestroyTeam,			// 7	cDestroyTeam
	&CTeamSystem::HandleApplyJoinTeamReply,		// 8	cApplyJoinTeamReply
	&CTeamSystem::HandleGetAllowTeamState,      //9     cGetAllowTeamState
	&CTeamSystem::HandleSetIsAllow,	           // 10	cIsAutoTeam
	&CTeamSystem::GetNearTeamInfo,				// 11	cAskNearTeamInfo附近队伍
};

void CTeamSystem::OnEnterGame()
{
	nUpdataDt = GetGlobalLogicEngine()->getMiniDateTime();
	CTeam* pTeam = m_pEntity->GetTeam(); //这个队伍在存在的话
	if(pTeam)
	{
		if(pTeam->IsTeamValid())
		{
			if(!pTeam->AddMember(m_pEntity, false))
			{
				m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_TEAM_ID, 0);
			}
			pTeam->SendMemberData(m_pEntity);
		}
		else
		{
			m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_TEAM_ID, 0);
		}
	}
	m_nInit = true;
}

bool CTeamSystem::Initialize(void *data,SIZE_T size)
{
	if(data ==NULL) return false;
	PACTORDBDATA  pActorData = (ACTORDBDATA *)data;
	if(pActorData ==NULL || size != sizeof(ACTORDBDATA)) 
	{
		OutputMsg(rmError,_T("data len error len=%d ,correct len=%d"),size, sizeof(ACTORDBDATA));
		return false;
	}
	
	m_nUpdateActorListDt = 0;
	m_nAllowTeam = 0;
	m_nInit = false;
	m_NearTeam.clear();
	m_NoTeamActor.clear();

	//玩家上线的时候，如果在队伍里话，需要找到他的队伍
	unsigned int nTeamID = pActorData->nTeamID;
	if(nTeamID == 0) return true;

	bool flag = false; // 是否加入了队伍
	CTeam * pTeam = GetGlobalLogicEngine()->GetTeamMgr().GetTeam(nTeamID); //这个队伍在存在的话
	if(pTeam)
	{
		if(pTeam->GetActorIndex( m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID) ) >=0 )
		{
			flag = true;		
		}
	}
	//如果玩家加入队伍失败，设置它的装备为没有加入队伍
	if(!flag)
	{
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_TEAM_ID,0); 
		m_pEntity->SetSocialMask(smTeamMember,false);
	}
	m_pEntity->SetSocialMask(smTeamCaptin,false); //上来肯定不是队长了
	
	return true;
}

void CTeamSystem::BroadCast(void * data,SIZE_T size)
{
	CTeam * pTeam = m_pEntity->GetTeam();
	if(pTeam)
	{
		pTeam->BroadCast(data,size);
	}
	else
	{
		//CActor* pActor = (CActor*)m_pEntity;
		//int nTeamId = pActor->GetProperty<int>(PROP_ACTOR_TEAMFUBEN_TEAMID);
		//FubenTeam *pTeam = GetGlobalLogicEngine()->GetFuBenMgr()->GetTeam(nTeamId);
		//if(pTeam)
		//{
		//    GetGlobalLogicEngine()->GetFuBenMgr()->TeamFubenBroadCast(pTeam,data,size);
		//} else
		//{
		    m_pEntity->SendTipmsgFormatWithId(tmTeamChatFail,tstFigthing);  //提示对方无法使用队伍频道
		//}  
	}
}

void CTeamSystem::Destroy()
{
	//玩家在下线的时候要通知队伍自己下线了
	if(m_pEntity ==NULL) return;
	CTeam * pTeam = m_pEntity->GetTeam();
	if(pTeam && m_nInit) 
	{
		pTeam->SetUserOffline(m_pEntity);
	}
}

void  CTeamSystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet )
{
	if(m_pEntity == NULL) return; //系统没有初始化完毕
	if(nCmd >= 0 && nCmd < ArrayCount(CTeamSystem::Inherid::Handlers))
	{
		(this->*Handlers[nCmd] )(packet);
	}
}


void CTeamSystem::HandleGetAllowTeamState(CDataPacketReader& packet)
{
	CActorPacket pack;
	CDataPacket& data = ((CActor*)m_pEntity)->AllocPacket(pack);
	data << (BYTE)enTeamSystemID <<(BYTE)enTeamSystemsGetAllowTeam <<(BYTE)m_nAllowTeam;
	pack.flush();
}


//邀请加入队伍
void CTeamSystem::HandleInvite(CDataPacketReader &packet)
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	int nOpenLevel = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nTeamOpenLevel;
	if (!m_pEntity->CheckLevel(nOpenLevel,0))
	{
		m_pEntity->SendTipmsgFormatWithId(tmTeamLevelLimited,tstUI,nOpenLevel);
		return;
	}

	ACTORNAME name;
	unsigned int nActorId = 0;
	packet.readString(name,sizeof(name)); //读取名字
	packet >> nActorId;
	CActor* pMember = NULL;
	if(nActorId > 0 )
	{
		pMember = pEntityMgr->GetEntityPtrByActorID(nActorId);
	}
	
	if(!pMember)
	{
		pMember = pEntityMgr->GetActorPtrByName(name);
	}

	//pMember= pEntityMgr->GetActorPtrByName(name);
	if(pMember ==NULL)
	{
		m_pEntity->SendTipmsgFormatWithId(tmTeamOpOfflineUser,tstUI,name);
		return;
	}
	if(pMember == m_pEntity)
	{
		m_pEntity->SendTipmsgFormatWithId(tmCanNotOPOnSelf,tstUI); //不能对自己操作
		return ;
	}

	if(((CActor *)pMember)->HasState(esStateTeamFuben) )
	{
		m_pEntity->SendTipmsgFormatWithId(tpInTeamFuben,tstUI); 
		return ;
	}

	InviteJoinTeam(pMember);
	// BYTE type; 
	// packet >> type;
	// if (type)
	// {
	// 	ACTORNAME name;
	// 	packet.readString(name,sizeof(name)); //读取名字
	// 	CActor *pMember= pEntityMgr->GetActorPtrByName(name);
	// 	if(pMember ==NULL)
	// 	{
	// 		m_pEntity->SendOldTipmsgFormatWithId(tpTeamOpOfflineUser,ttFlyTip,name);
	// 		return;
	// 	}
	// 	if(pMember == m_pEntity)
	// 	{
	// 		m_pEntity->SendOldTipmsgWithId(tpCanNotOPOnSelf,ttFlyTip); //不能对自己操作
	// 		return ;
	// 	}

	// 	if(((CActor *)pMember)->HasState(esStateTeamFuben) )
	// 	{
	// 		m_pEntity->SendOldTipmsgWithId(tpInTeamFuben,ttFlyTip); 
	// 		return ;
	// 	}

	// 	InviteJoinTeam(pMember);
	// }
	// else
	// {
	// 	for (INT_PTR i = m_NoTeamActor.count() - 1; i >= 0; --i)
	// 	{
	// 		CActor *pMember = pEntityMgr->GetEntityPtrByActorID(m_NoTeamActor[i]);
	// 		if(pMember == NULL || pMember == m_pEntity)
	// 		   continue;
	// 		if(pMember->HasState(esStateTeamFuben) )
	//            continue;
	// 		InviteJoinTeam(pMember);
	// 	}
	// }
}

//退出队伍
void CTeamSystem::HandleLeave(CDataPacketReader &packet)
{
	//退出队伍
	CTeam * pTeam = m_pEntity->GetTeam();
	if(pTeam ) 
	{
		if (pTeam->IsChallenge())
		{
			m_pEntity->SendTipmsgFormatWithId(tmInChallengeCannotLeft,ttFlyTip);
			return;
		}
		pTeam->DelMember(m_pEntity->GetId());
	}
	// SetAllowTeam(0);
}

//申请加入队伍
void CTeamSystem::HandleApplyJoinTeam(CDataPacketReader &packet)
{
	int nOpenLevel = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nTeamOpenLevel;
	if (!m_pEntity->CheckLevel(nOpenLevel,0))
	{
        m_pEntity->SendTipmsgFormatWithId(tmTeamLevelLimited,tstUI,nOpenLevel);
		return;
	}
	BYTE type = 1;

	CActor *pMember = nullptr;
	unsigned int nActorId = 0;
	if (type)
	{
		//自己申请加入别人的队伍
		//packet.readString(name,sizeof(name)); //读取名字
		packet >> nActorId;
		pMember = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);
		if(pMember ==NULL)
		{
			m_pEntity->SendTipmsgFormatWithId(tmTeamApplyOfflineUser,tstUI);
			return ;
		}
		/*CFuBen* pFb = pMember->GetFuBen();
		if (pFb && pFb->IsFb())
		{
		pMember->SendOldTipmsgWithId(tpInFubenCanNotJoinTeam); 
		return;
		}*/
		//在擂台不能加入

		if(pMember == m_pEntity)
		{
			m_pEntity->SendTipmsgFormatWithId(tmCanNotOPOnSelf,tstUI); //不能对自己操作
			return;
		}
	}
	//自己已经在队伍中了
	if(m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_TEAM_ID) )
	{
		m_pEntity->SendTipmsgFormatWithId(tmTeamSelfIsInTeam,tstUI); //不能对自己操作
		return;
	}

	if (m_pEntity->HasState(esStateTeamFuben))
	{
		m_pEntity->SendTipmsgFormatWithId(tmHasFubenTeam, tstUI);
		return ;
	}

	if (type)
	{
		CTeam * pTeam = pMember->GetTeam();
		if(pTeam ==NULL)
		{
			m_pEntity->SendTipmsgFormatWithId(tmTeamApplyNotTeamUser,tstUI, pMember->GetEntityName());
			return ;
		}
		ApplyJoinOneTeam(pTeam);
	}
	// else	// 批量申请附近队伍
	// {
	// 	CTeamManager& teamMgr = GetGlobalLogicEngine()->GetTeamMgr();
	// 	for (std::map<unsigned int,int>::const_iterator it = m_NearTeam.begin(); it != m_NearTeam.end(); ++it)
	// 	{
	// 		CTeam* pTeam = teamMgr.GetTeam(it->first);
	// 		if (pTeam && pTeam->GetMemberCount() < pTeam->GetTeamMaxMemberCount())
	// 		{
	// 			ApplyJoinOneTeam(pTeam);
	// 		}
	// 	}
	// }
}

void CTeamSystem::ApplyJoinOneTeam(CTeam* pTeam)
{
	if (!pTeam)
		return;
	if (pTeam->IsChallenge())
	{
		m_pEntity->SendTipmsgFormatWithId(tmInChallengeCannotJoin,tstUI);
		return;
	}
	//队伍在副本中
	/*if (pTeam->GetFubenStatus() == tsWar)
	{
		m_pEntity->SendOldTipmsgWithId(tpTeamInFubenCanNotAddMember);
		return;
	}*/
	if(pTeam->GetMemberCount() >= pTeam->GetTeamMaxMemberCount())
	{
		m_pEntity->SendTipmsgFormatWithId(tmTeamIsFull,tstUI);
		return;
	}
	CActor * pCaptin = pTeam->GetCaptin(); //队长
	if(pCaptin ==NULL) return; //没有队长，出现了问题
	// if (pCaptin->GetMiscSystem().GetGameSetting(GAME_SETTING_TYPE_BASE, GAME_SETTING_TYPE_BASE_AUTO_INVITETEAM) > 0)
	// {
	// 	pTeam->AddMember(m_pEntity);
	// }
	// else
	{
		//向队长发消息，申请入队
		CActorPacket pack;
		CDataPacket &data =  pCaptin->AllocPacket(pack);
		data << (BYTE)enTeamSystemID << (BYTE)enTeamSystemsApplyJoinTeam << (unsigned int) m_pEntity->GetId();
		// data << (BYTE)m_pEntity->GetProperty<unsigned int>(PROP_CREATURE_LEVEL); //等级
		// data << (BYTE)m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_VOCATION); //职业
		data.writeString(m_pEntity->GetEntityName());
		pack.flush();
		m_pEntity->SendTipmsgFormatWithId(tmTeamApplySuccess, tstUI);
	}
	SetAllowTeam(1);
	
}

//设置队长
void CTeamSystem::HandleSetCaptin(CDataPacketReader &packet)
{
	unsigned int nActorID = 0;
	packet >> nActorID; 
	CTeam * pTeam;
	bool isCaptin = CheckIsCaptin(pTeam);
	if(isCaptin)
	{
		//如果正在擂台，不能换队长
		if (pTeam->IsChallenge())
		{
			m_pEntity->SendTipmsgFormatWithId(tmInChallengeCannotChangeCaptin); 
			return;
		}
		CActor* pActor = pTeam->GetActorByID(nActorID);
		if(pActor == NULL)
		{
			m_pEntity->SendTipmsgFormatWithId(tmTeamOfflineUserCannotCaptin,tstUI);
			return ;
		}
		if(pActor == m_pEntity)
		{
			m_pEntity->SendTipmsgFormatWithId(tmCanNotOPOnSelf,tstUI); //不能对自己操作
			return ;
		}

		pTeam->SetCaptin(pActor);//设置这个玩家为队长		
	}	
}


//踢人
void CTeamSystem::HandleKickMember(CDataPacketReader &packet)
{
	unsigned int nActorID =0;
	packet >> nActorID; 
	CTeam * pTeam;
	bool isCaptin = CheckIsCaptin(pTeam);
	if(isCaptin)
	{
		if (pTeam->IsChallenge())
		{
			m_pEntity->SendTipmsgFormatWithId(tmInChallengeCannotDelMember,tstUI);
			return;
		}

		if(m_pEntity != pTeam->GetCaptin())
		{
			m_pEntity->SendTipmsgFormatWithId(tmNotTeamFubenCaptain, tstUI);
			return;
		}

		if(nActorID == m_pEntity->GetId())
		{
			m_pEntity->SendTipmsgFormatWithId(tmCanNotOPOnSelf, tstUI);
			return;
		}
		pTeam->DelMember(nActorID); //提出一个人出队伍
	}	
}


bool  CTeamSystem::CheckIsCaptin(CTeam* & pTeam,bool bWithTipmsg)
{
	pTeam = m_pEntity->GetTeam();
	if(pTeam == NULL)
	{
		if(bWithTipmsg)
		{
			m_pEntity->SendTipmsgFormatWithId(tmTeamNotInTeam,tstUI);
		}
		return false;
	}
	//需要队长才能操作
	else if(m_pEntity != pTeam->GetCaptin())
	{
		if(bWithTipmsg)
		{
			m_pEntity->SendTipmsgFormatWithId(tmTeamNeedCaptin,tstUI);
		}
		return false;
	}
	else
	{
		return true;
	}	
}


//解散队伍
void CTeamSystem::HandleDestroyTeam(CDataPacketReader &packet)
{
	CTeam * pTeam;
	bool isCaptin = CheckIsCaptin(pTeam);
	if(isCaptin)
	{
		pTeam->DestroyTeam();
	}
}


//回复申请入队
void CTeamSystem::HandleApplyJoinTeamReply(CDataPacketReader &packet)
{	
	CTeam * pTeam;
	bool isCaptin = CheckIsCaptin(pTeam);
	if(isCaptin)
	{
		unsigned int nActorID = 0;
		BYTE bReplyResult=0; //处理的结果
		packet >> nActorID >> bReplyResult;
		CActor * pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorID);
		if(pActor ==NULL)
		{
			m_pEntity->SendTipmsgFormatWithId(tmTeamOpOfflineUser,tstUI, ""); //玩家不在线
			return;
		}

		if(pTeam->GetMemberCount() >= pTeam->GetTeamMaxMemberCount())
		{
			m_pEntity->SendTipmsgFormatWithId(tmTeamIsFull,tstUI);
			pActor->SendTipmsgFormatWithId(tmTeamIsFull,tstUI);
			return;
		}
		if(bReplyResult)
		{
			//这个玩家已经有队伍了,如果队伍是当前队伍的话，那么情况出现在玩家下线再上线的时候
			CTeam *pCurrentTime = pActor->GetTeam();
			if(pCurrentTime)//如果已经加入了别人的队伍
			{
				return;		
			}

			{
				pTeam->AddMember(pActor);
			}
		}
		else	// 拒绝加入
		{
			pActor->SendTipmsgFormatWithId(tmTeamRefuseApply,tstUI,m_pEntity->GetEntityName());
		}
	}
}

void CTeamSystem::HandleSetIsAllow(CDataPacketReader& packet)
{
	BYTE result = 0;
	packet >> result;

	m_nAllowTeam = (int)result;
}

void CTeamSystem::SetAllowTeam(int type)
{
	m_nAllowTeam = type;
}

//回复邀请入队
void CTeamSystem::HandleInviteJoinTeamReply(CDataPacketReader &packet)
{	
	//自己已经在队伍中了，不能再加入别的队伍
	CTeam* pMyTeam = m_pEntity->GetTeam();
	if(pMyTeam && pMyTeam->GetMemberCount() > 0)
	{
		m_pEntity->SendTipmsgFormatWithId(tmTeamSelfIsInTeam,ttFlyTip);
		return;
	}

	ACTORNAME name;
	packet.readString(name,sizeof(name)); //读取名字
	BYTE bResylt=0,bAuto = 0; //是否同意,0:非自动，1：自动同意
	packet >> bResylt;
	// packet >> bAuto;

	CActor *pMember= GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(name);
	if(pMember == NULL)
	{
		//邀请自己的人已经下线
		m_pEntity->SendTipmsgFormatWithId(tmTeamOpOfflineUser,tstUI,name);
		return ;
	}
	if( !bResylt ) //如果是拒绝
	{
		//xx拒绝绝加入队伍
		pMember->SendTipmsgFormatWithId(tmOtherRefuseJoinTeam,tstUI,m_pEntity->GetEntityName());
		return;
	}
	if (pMyTeam && pMyTeam->GetMemberCount() <= 1)
	{
		pMyTeam->DelMember(m_pEntity->GetId());//先退出自己的队伍
	}
	CTeam * pTeam=	pMember->GetTeam();	

	if( pTeam ) //队伍
	{
		if(pTeam->GetMemberCount() >= pTeam->GetTeamMaxMemberCount())
		{
			m_pEntity->SendTipmsgFormatWithId(tmTeamIsFull,tstUI);
			return;
		}
		////如果队伍已经在副本中，不能新增队员
		//int status = pTeam->GetFubenStatus();
		//if ( status == tsReady)
		//{
		//	//判断能否加入队伍
		//	CScriptValueList paramList;
		//	m_pEntity->InitEventScriptVaueList(paramList,aeAddToFubenTeam);
		//	paramList << pTeam->GetTeamID();
		//	m_pEntity->OnEvent(paramList,paramList);
		//}
		//else
		{
			if (pTeam->AddMember(m_pEntity) )
			{
				// if(bAuto)
				// {
				// 	CActor* pCaptin = pTeam->GetCaptin();
				// 	LPCSTR sName = pCaptin?pCaptin->GetEntityName():"";
				// 	m_pEntity->SendTipmsgFormatWithId(tmTeamJoinTeam,stsUI,sName);
				// }
			}
		}		
	}
	else //否则创建一个队伍
	{
		pTeam = CreateTeam(pMember);
		//自己是个队员而已
		if(pTeam!=NULL && pTeam->AddMember(m_pEntity) )
		{
			// if (bAuto)
			// {
			// 	m_pEntity->SendTipmsgFormatWithId(tmTeamJoinTeam,tstUI,pMember->GetEntityName());
			// }
		}		
	}
}

void CTeamSystem::AskCreateTeam(CDataPacketReader & packet)
{
	CreateTeam();
}

//创建队伍
CTeam *CTeamSystem::CreateTeam(CActor* pMember)
{
	int nOpenLevel = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nTeamOpenLevel;
	if (!m_pEntity->CheckLevel(nOpenLevel,0))
	{
		m_pEntity->SendTipmsgFormatWithId(tmTeamLevelLimited,tstUI,nOpenLevel);
		return nullptr;
	}

	if (!pMember)
	{
		pMember = m_pEntity;
	}

	if(!pMember)
		return nullptr;
	CTeam* pTeam = pMember->GetTeam();
	if (pTeam)	// 已经有队伍
	{
		return nullptr;
	}
    if (pMember->HasState(esStateTeamFuben))
	{
	    pMember->SendTipmsgFormatWithId(tmHasFubenTeam, tstUI);
		return nullptr;
	}

	CScene* pScene = pMember->GetScene();
	int x,y;
	pMember->GetPosition(x,y);
	if (pScene && pScene->HasMapAttribute(x,y,aaNotCreateTeam))
	{
		return nullptr;
	}

	unsigned int nTeamID;
	pTeam = GetGlobalLogicEngine()->GetTeamMgr().CreateTeam(nTeamID);
	if(!pTeam)
		return nullptr;			//创建队伍失败
	pTeam->AddMember(pMember);   //自己是队长
	pMember->SendTipmsgFormatWithId(tmTeamCreated, tstUI);
	return pTeam;
}

void CTeamSystem::TeamMoveBroadcast(  )
{
	
	//如果没有初始化就不发
	if (!m_pEntity || m_pEntity->IsInited() ==false) return;
	CTeam* pTeam = m_pEntity->GetTeam();
	if (pTeam)
	{
		int nDis = GetLogicServer()->GetDataProvider()->GetGlobalConfig().teamLootMaxDistanceSquare;
		char data[32];
		char Selfdata[32];	//发送给自己的信息，告诉自己和别人的关系
		int nActorId = m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID);

		CDataPacket pack(data,sizeof(data));
		CDataPacket selfPack(Selfdata,sizeof(Selfdata));
		int x, y;
		m_pEntity->GetPosition(x,y);
		pack << (BYTE)enTeamSystemID << (BYTE)enTeamSystemsMemberMove << (unsigned int)nActorId << (int)m_pEntity->GetSceneID() << (WORD)x << (WORD)y;
		BYTE* pDis = (BYTE*)pack.getOffsetPtr();
		pack << (BYTE)0;//是否超出了经验共享范围,0:超出，1未超出


		selfPack << (BYTE)enTeamSystemID << (BYTE)enTeamSystemsMemberMove;
		unsigned int* pOtherId = (unsigned int*)selfPack.getOffsetPtr();
		selfPack << (int)0;
		int* pSceneId = (int*)selfPack.getOffsetPtr();
		selfPack << (int)0;
		WORD* pX = (WORD*)selfPack.getOffsetPtr();
		selfPack << (WORD)0;
		WORD* pY = (WORD*)selfPack.getOffsetPtr();
		selfPack << (WORD)0;
		BYTE* pSelfDis = (BYTE*)selfPack.getOffsetPtr();
		selfPack << (BYTE)0;//是否超出了经验共享范围,0:超出，1未超出

		const TEAMMEMBER* pMembers = pTeam->GetMemberList();
		for (INT_PTR i=0; i< MAX_TEAM_MEMBER_COUNT ; i++)
		{
			CActor* pOtherActor = pMembers[i].pActor;
			if( pOtherActor && pOtherActor != m_pEntity && pOtherActor->IsInited() && pOtherActor->GetTeam() == pTeam)
			{
				*pDis = *pSelfDis = m_pEntity->GetEntityDistanceSquare(pOtherActor) <= nDis?1:0;
				pOtherActor->SendData(data,pack.getPosition());

				*pOtherId = pOtherActor->GetProperty<unsigned int>(PROP_ENTITY_ID);
				*pSceneId = (int)pOtherActor->GetSceneID();

				pOtherActor->GetPosition(x,y);
				*pX = (WORD)x;
				*pY = (WORD)y;
				m_pEntity->SendData(Selfdata,selfPack.getPosition());
			}
		}
		//pTeam->UpdateBufMember();
	}
	
}

void CTeamSystem::InviteJoinTeam(CActor* pMember )
{
	if (!pMember)
	{
		return;
	}
	int nOpenLevel = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nTeamOpenLevel;
	if (!pMember->CheckLevel(nOpenLevel,0))
	{
		m_pEntity->SendTipmsgFormatWithId(tmOtherTeamLevelLimited,tstUI,nOpenLevel);
		return;
	}
	CTeam* pMyTeam = m_pEntity->GetTeam();
	if(pMyTeam) 
	{
		if( pMyTeam->GetMemberCount() >= pMyTeam->GetTeamMaxMemberCount())
		{
			m_pEntity->SendTipmsgFormatWithId(tmTeamIsFull,tstUI); 
			return;
		}
	}

	CTeam* pTeam = pMember->GetTeam();
	if (pTeam && pTeam->GetMemberCount() > 0)	//队伍只有自己，可以直接退出原有的队伍，加入新队伍
	{
		m_pEntity->SendTipmsgFormatWithId(tmTeamIsInTeam,ttFlyTip,pMember->GetEntityName()); 
		return ;
	}
	if(pMember->GetTeamSystem().IsAllowTeam())
	{
		if( pMyTeam ) //队伍
		{
			////如果队伍已经在副本中，不能新增队员
			//int status = pTeam->GetFubenStatus();
			//if ( status == tsReady)
			//{
			//	//判断能否加入队伍
			//	CScriptValueList paramList;
			//	m_pEntity->InitEventScriptVaueList(paramList,aeAddToFubenTeam);
			//	paramList << pTeam->GetTeamID();
			//	m_pEntity->OnEvent(paramList,paramList);
			//}
			//else

			CActor* pCaptin = pMyTeam->GetCaptin();
			if(!pCaptin) return;
			
			if(pCaptin->GetProperty<unsigned int>(PROP_ENTITY_ID) != m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID))
			{
				m_pEntity->SendTipmsgFormatWithId(tmTeamNeedCaptin, tstUI);
				return;
			}
			if (pMyTeam->AddMember(pMember) )
			{
				CActor* pCaptin = pMyTeam->GetCaptin();
				LPCSTR sName = pCaptin?pCaptin->GetEntityName():"";
				pMember->SendTipmsgFormatWithId(tmTeamJoinTeam,tstUI,sName);
				pMyTeam->SendMemberData(pMember);
			}		
		}
		else //否则创建一个队伍
		{
			pTeam = CreateTeam(m_pEntity);
			if(pTeam!=NULL && pTeam->AddMember(pMember) )
			{
				CActor* pCaptin = pTeam->GetCaptin();
				if(!pCaptin) return;
				m_pEntity->SendTipmsgFormatWithId(tmTeamJoinTeam,tstUI,pCaptin->GetEntityName());
				pTeam->SendMemberData(m_pEntity,pMember);
			}
		}
	}else
	{
		m_pEntity->SendTipmsgFormatWithId(tmTeamRefuseApply, tstUI);
		// CActorPacket pack;
		// CDataPacket &data =  pMember->AllocPacket(pack);
		// data << (BYTE)enTeamSystemID << (BYTE)enTeamSystemsInviteJoinTeam ;
		// data.writeString(m_pEntity->GetEntityName());
		// // 邀请者等级
		// // data << (WORD)m_pEntity->GetProperty<int>(PROP_CREATURE_LEVEL);
		// data << m_pEntity->GetId();
		// // data <<  (BYTE)m_pEntity->GetProperty<int>(PROP_ACTOR_VOCATION);
		// pack.flush();
	}
}

void CTeamSystem::GetMyTeamInfo(CDataPacketReader& packet)
{
	if(m_pEntity == NULL) return;

	CTeam* myTeam = m_pEntity->GetTeam();
	if(myTeam)
	{
		if(myTeam->IsTeamValid())
		{
			myTeam->SendMemberData(m_pEntity);
			return;
		}
	}
	CActorPacket ap;
	CDataPacket& data = ((CActor*)m_pEntity)->AllocPacket(ap);
	data << (BYTE)enTeamSystemID << (BYTE) enTeamSystemsInitTeam;
	data<< (BYTE)0;
	ap.flush();
}

void CTeamSystem::GetNearTeamInfo(CDataPacketReader & packet)
{
	if (m_pEntity == NULL)
	{
		return;
	}
	unsigned int nNowDt = GetGlobalLogicEngine()->getMiniDateTime();
	if(nNowDt < nUpdataDt + 5)
	{
		//return;
	}
	nUpdataDt = nNowDt;

	int nTeamCount = 0;
	const CVector<EntityHandleTag>& entityList = m_pEntity->GetObserverSystem()->GetVisibleList();
	
	m_NearTeam.clear();
	CTeam* pMyTeam = ((CActor*)m_pEntity)->GetTeam();
	if(pMyTeam != NULL)				//不能有自己的队伍
	{
		int nTeamId = pMyTeam->GetTeamID();
		m_NearTeam[nTeamId] = 1;
	}
	CActorPacket pack;
	CDataPacket &data =  m_pEntity->AllocPacket(pack);
	data << (BYTE)enTeamSystemID << (BYTE)enTeamSystemsSendNearTeamInfo;
	size_t pos = data.getPosition();
	data << (BYTE)0;				//先随便写个数字，后面会写个正确的进来
	for (INT_PTR i = 0; i < entityList.count(); i++)
	{
		CEntity* pEntity = GetEntityFromHandle(entityList[i].m_handle);
		if (pEntity && pEntity->GetType() == enActor && pEntity != m_pEntity)
		{
			CTeam* pTeam = ((CActor*)pEntity)->GetTeam();
			if(pTeam != NULL)
			{
				CActor* pActor = pTeam->GetCaptin();
				unsigned int nTeamId = pTeam->GetTeamID();
				if(pActor != NULL && m_NearTeam.find(nTeamId) == m_NearTeam.end())
				{
					data << (unsigned int)pActor->GetProperty<unsigned int>(PROP_ENTITY_ID);
					data.writeString(pActor->GetEntityName());
					data << (int)pActor->GetProperty<int>(PROP_CREATURE_LEVEL);
					data << (BYTE)pActor->GetProperty<int>(PROP_ACTOR_CIRCLE);
					data << (BYTE)pTeam->GetMemberCount();
					CGuild* pMyGuild = pActor->GetGuildSystem()->GetGuildPtr();
					data.writeString(pMyGuild ? pMyGuild->m_sGuildname : "");	//队长的公会名字

					nTeamCount++;
					m_NearTeam[nTeamId] = 1; // 标记已经读取过的队伍
				}
			}
		}
	}
	BYTE* pCount = (BYTE*)data.getPositionPtr(pos);
	*pCount = nTeamCount;
	pack.flush();
}

void CTeamSystem::GetNearActorsInfo(CDataPacketReader & packet){
	if (m_pEntity == NULL)
	{
		return;
	}
	unsigned int nNowDt = GetGlobalLogicEngine()->getMiniDateTime();
	if(m_nUpdateActorListDt && nNowDt < m_nUpdateActorListDt + 30)
	{
		//return;
	}
	m_nUpdateActorListDt = nNowDt;

	const CVector<EntityHandleTag>& entityList = m_pEntity->GetObserverSystem()->GetVisibleList();
	unsigned int nActorCount = 0;
	CActorPacket pack;
	CDataPacket &data =  m_pEntity->AllocPacket(pack);
	data << (BYTE)enTeamSystemID << (BYTE)enTeamSystemsSendNearActorInfo;
	size_t pos = data.getPosition();
	data << nActorCount;				//先随便写个数字，后面会写个正确的进来
	m_NoTeamActor.clear();
	int nNoTeamCount = 0;
	for (INT_PTR i = 0; nActorCount < MAX_NEAR_ACTOR_COUNT && i < entityList.count(); i++)
	{
		CEntity* pEntity = GetEntityFromHandle(entityList[i].m_handle);
		if (pEntity && pEntity->GetType() == enActor && pEntity != m_pEntity)
		{
			CActor* pActor = (CActor*)pEntity;
			data.writeString(pActor->GetEntityName());
			data << (WORD)pActor->GetProperty<int>(PROP_CREATURE_LEVEL);
			// 队长职业
			data << (BYTE)pActor->GetProperty<unsigned int>(PROP_ACTOR_VOCATION); //职业
			CGuild* pMyGuild = pActor->GetGuildSystem()->GetGuildPtr();
			data.writeString(pMyGuild ? pMyGuild->m_sGuildname : "");	//队长的公会名字
			CTeam* pTeam = pActor->GetTeam();
			if (!pTeam && nNoTeamCount < MAX_NOTEAM_ACTOR_COUNT)
			{
				m_NoTeamActor.add(pActor->GetId());
				++nNoTeamCount;
			}
			data << (BYTE)!!pTeam;
			++nActorCount;
		}
	}
	unsigned int* pCount = (unsigned int*)data.getPositionPtr(pos);
	*pCount = nActorCount;
	pack.flush();
}
