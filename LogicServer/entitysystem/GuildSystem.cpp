#include "StdAfx.h"

#include "GuildSystem.h"
#include <string>

static const char *szGuildAutoJoinLevel	= "GuildAutoJoinLevel";		//不使用
static const char *szDailyDonateCoin	= "DailyDonateCoin";		//每日捐献金币数量
static const char *szDailyDonateItem	= "DailyDonateItem";		//每日捐献道具数量

static const char *szTodayChargeTreeNum	= "TodayChargeTreeNum";		//玩家当日已经充能的次数
static const char *szTodayPickFruitNum	= "TodayPickFruitNum";		//玩家当日已经采摘果实的次数


static const char *szTodayGetTaskAwardNum = "dailyGuildTaskAwardNum";	//当日领取奖励次数
static const char *szDailyDepotPutInNum	  = "dailyDepotPutInNum";		//每日可投入行会仓库次数
static const char *szVarCreateGuildIsYbReplace = "CreateGuildIsYbReplace"; //动态变量，创建行会时是否元宝代替

//ActorNodeList::LinkNodeMgr* ActorNodeList::g_LinkNodeMgr;

#define SEARCH_GUILD_BY_GUILDNAME   1
#define SEARCH_GUILD_BY_LEADERNAME  2

//行会操作结果符号（协议（10,16）使用）
#define GUILD_OP_JOIN_OK		3
#define GUILD_OP_LEFT_OK		4
#define GUILD_CAN_PICKFRUIT		5		//可以摘取神树果实
#define GUILD_CAN_GETTASKAWRD	6		//可以领取行会任务奖励

CGuildSystem::CGuildSystem()
{
	m_pGuild = NULL;
	nTk = 0;
	nWealNext = 0;
	nZzNext = 0;
	boSaveed = FALSE;
	nTomorrow = CMiniDateTime::tomorrow();
	boWealClear = true;
	m_Score = 0;
	m_InGuildFuben = false;
	m_InDartNow = false;
	m_GuildGx = 0;
	m_CallGuildBossFlag = false;
	m_nTitle = 0;
	m_TodayGx = 0;
	m_HasShxTimes = 0;
	m_ExploreTimes = 0;
	m_ExploreItemId = 0;
	m_ChallengeTimes = 0;
	m_AddChallengeTimes = 0;
	m_nJoinTime	= 0;					//加入行会的时间

	m_AddShxTimes = 0;
	m_nGetCoinAwardFlag = 0;
	m_nTodayDonateCoin = 0;
	m_nTodayDonateItem = 0;

	m_nTodayChargeTreeNum = 0;
	m_nTodayPickFruitNum   = 0;
	m_nFastApply = 0;
}

VOID CGuildSystem::OnTimeCheck(TICKCOUNT nTickCount)
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return;
	}
	if (TimeOK(nZzNext))
	{
		nZzNext = 0;
	}
	
	if (nTomorrow != 0 && TimeOK(nTomorrow))
	{
		ClearZJ();
		nTomorrow = CMiniDateTime::tomorrow();
	}else if (boWealClear && m_pEntity && m_pEntity->GetBagSystem().count() > 0)//今日之前的召集令还没删除
	{
		ClearZJ();
		boWealClear = false;
	}
}


VOID CGuildSystem::OnDeleteGuild(int nErrorCode)
{
	if (m_pEntity == NULL) return;
	if (m_pGuild == NULL) return;
	CGuild* pGuild = m_pGuild;//要删除的帮派
	if(nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
	{
		//成功的话后期处理
		//帮派解散后，帮主帮派贡献值/2（往下取整）。其他成员帮派贡献值不变。
		//所有成员本身任务列表的帮派任务删除。
		//帮派仓库中的物品全部清空。故该帮主在解散时需二次确认：您确定解散帮派吗？解散后，帮派仓库的物品将全部清空。

		//记录日志
		if(GetLogicServer()->GetLocalClient())
		{
		}
		GetGlobalLogicEngine()->GetGuildMgr().DeleteGuild(pGuild);

		//SetGx(0);
		boSaveed = TRUE;
		//设置本帮派无效
		m_pEntity->SendTipmsgFormatWithId(tpDeleteGuildSucc);	
		m_pGuild = NULL;

		nZzNext = GetGlobalLogicEngine()->getMiniDateTime() +
			GetLogicServer()->GetDataProvider()->GetGuildConfig().nLeftTimeLimit;//设置他的在职时间，在规定时间内不能再加入其他帮派
		// CActorPacket ap;
		// CDataPacket& dataPack = m_pEntity->AllocPacket(ap);
		// dataPack << (BYTE)enGuildSystemID << (BYTE)sDelGuild;
	}
	else
	{
		m_pEntity->SendTipmsgFormatWithId(tpDeleteGuildErr);
	}
	
	CActor *pActor = (CActor*)m_pEntity;
	pActor->ResetShowName();
}


VOID CGuildSystem::OnCreateGuild(unsigned int nGid,int nErrorCode,CGuild* pGuild)
{
	if (m_pEntity == NULL) return;
	CActor *pActor = (CActor*)m_pEntity;

	if (nErrorCode == jxInterSrvComm::DbServerProto::reSucc && pGuild)
	{
		if (m_pGuild != NULL)
		{
			//不应该出现这个异常
			OutputMsg(rmError,_T("Actor Create Guild More than One time!gid=%u,actorname=%s"),nGid,m_pEntity->GetEntityName());
			return;
		}
		// bool bIsYbReplace = 0;
		// CCLVariant& dyVal =  m_pEntity->GetDynamicVar();
		// CCLVariant* pIsYbReplace =  dyVal.get(szVarCreateGuildIsYbReplace);
		// if (pIsYbReplace)
		// {
		// 	bIsYbReplace = true;
		// 	pIsYbReplace->clear();
		// }
		// int nCreateNeedItem			= GetLogicServer()->GetDataProvider()->GetGuildConfig().nCreateNeedItem;
		INT_PTR nCreateNeedYb		= GetLogicServer()->GetDataProvider()->GetGuildConfig().nCreateNeedYb;
		// INT_PTR nMyItemCount = m_pEntity->GetBagSystem().GetItemCount(nCreateNeedItem);
		unsigned int nMyYb = pActor->GetMoneyCount(mtYuanbao);		//元宝
		if(nMyYb < nCreateNeedYb)
		{
			GetGlobalLogicEngine()->GetGuildMgr().SendDbMsgDeleteGuild(m_pEntity->GetHandle(),nGid);
			pActor->SendTipmsgFormatWithId(tmNomoreYubao, tstUI);
			return;
		}
		pActor->ChangeMoney( mtYuanbao, -nCreateNeedYb, GameLog::Log_CreateGuild, 0, "create guild yb", true);
		// if( nMyItemCount < 1)
		// {
		// 	if (nMyYb >= nCreateNeedYb && bIsYbReplace)
		// 	{
		// 		pActor->ChangeMoney( mtYuanbao, -nCreateNeedYb, GameLog::clGuildFr, 0, "create guild yb", true);
		// 	}
		// 	else
		// 	{
		// 		//不正常，不够钱缴纳创建帮派费用
		// 		OutputMsg(rmError,_T("Actor Create Guild no item and no yb!gid=%u,actorid=%s"),nGid,m_pEntity->GetEntityName());
		// 		//删除掉刚创建的帮派
		// 		GetGlobalLogicEngine()->GetGuildMgr().SendDbMsgDeleteGuild(m_pEntity->GetHandle(),nGid);
		// 		return;
		// 	}
		// }
		// else
		// {
		// 	//扣除行会道具
		// 	CUserItemContainer::ItemOPParam itemPara;
		// 	itemPara.wItemId = nCreateNeedItem;
		// 	itemPara.wCount = 1;//设置一个大的数字，不用先看背包有多少这个物品
		// 	itemPara.btStrong = -1;
		// 	itemPara.btQuality = -1;//
		// 	m_pEntity->GetBagSystem().DeleteItem(itemPara,pGuild->m_sGuildname,GameLog::clGuildFr);
		// }


		//做新帮派初始化的工作
		pGuild->InitGuild((CActor*)m_pEntity);

		// SendGuildTitleList();

		//m_pEntity->SendTipmsgFormatWithId(tmCreateGuildSucc, tstUI);
		// ((CActor*)m_pEntity)->GetQuestSystem()->OnQuestEvent(CQuestData::qcGuildLevel, 1, 1);
		// CEntityManager* em		= GetGlobalLogicEngine()->GetEntityMgr();
		// em->BroadTipmsgWithParams(tmCreateGuildSucc, tstUI, ( char *)(m_pEntity->GetEntityName()),pGuild->m_sGuildname);

		// ((CActor*)m_pEntity)->ResetShowName();

		

		//SendGuildbuildingInfo();

		//添加帮派事件
		char sText[1024];
		LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpCreateGuildEvent);
		sprintf_s(sText, sizeof(sText), sFormat, ( char *)((CActor*)m_pEntity->GetEntityName()));

		pGuild->AddEventRecord( sText,enGuildEvent_Create, 0, 0, 0, (char *)((CActor*)m_pEntity->GetEntityName()),NULL );

		//记录日志
		if(GetLogicServer()->GetLocalClient())
		{
		}
	}
	else
	{
		OutputMsg(rmTip,_T("Create Guild Error!code =%d"),nErrorCode);
		//创建帮派出错的提示
		if (nErrorCode == jxInterSrvComm::DbServerProto::reNameError)
		{
			m_pEntity->SendTipmsgFormatWithId(tmCreateGuildNameErr, tstUI);	
		}
		else if (nErrorCode == jxInterSrvComm::DbServerProto::reNameInUse)
		{
			m_pEntity->SendTipmsgFormatWithId(tmCreateGuildNameInUse,tstUI);	
		}
		else
		{
			m_pEntity->SendTipmsgFormatWithId(tmCreateGuildInterErr,tstUI);	
		}
	}

	CActorPacket AP;
	CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);

	DataPacket << (BYTE)enGuildSystemID;
	//知发送创建帮派和解散帮派的通知

	DataPacket << (BYTE)sAddGuild;
	DataPacket << (BYTE)nErrorCode;
	DataPacket << (int)nGid;
	AP.flush();

	pActor->ResetShowName();
	pActor->SaveDb();
}

/*
玩家应邀加入帮派
*/
VOID CGuildSystem::InviteResult(CDataPacketReader &packet)
{
	if (m_pEntity == NULL) return;
	BYTE ret = 0;
	int gid = 0;
	//处理消息
	CGuild::InterMsg Msg;
	Msg.hActorHandle = m_pEntity->GetHandle();
	Msg.nMsgId = CGuild::imInvite;

	packet >> ret;
	packet >> gid;
	packet >> Msg.hSrcHandle;
	//packet.readString(Msg.szName,ArrayCount(Msg.szName));
	_asncpytA(Msg.szName,m_pEntity->GetEntityName());

	CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(gid);

	if (!pGuild)
	{
		return;
	}

	
	if (pGuild->FindAndInterMsg(Msg))//之前的确有人邀请他
	{
		if (!ret)//拒绝，通知对方
		{
			CEntity* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(Msg.hSrcHandle);
			if (pEntity && pEntity->GetType() == enActor)
			{
				((CActor*)pEntity)->SendOldTipmsgFormatWithId(tpRefuseInviteGuild,ttFlyTip);
			}
			return;
		}
		if (m_pGuild == NULL)//如果还没加入帮派
		{
			if (pGuild->GetMemberCount() >= pGuild->GetMaxMemberCount())
			{
				//达到了人数上限
				m_pEntity->SendOldTipmsgFormatWithId(tpMaxMemberCount,ttFlyTip,pGuild->m_sGuildname);
				return;
			}
			//加入gid帮派
			pGuild->AddMemberToList((CActor*)m_pEntity);

			
			//添加帮派事件
			char sText[1024];
			//LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpAddToGuildEvent);
			//sprintf_s( "", sizeof(sText), sFormat, ( char *)((CActor*)m_pEntity->GetEntityName()) );
			//pGuild->AddEventRecord( "", enGuildEvent_MemberJoin, 0, 0, 0, (char *)((CActor*)m_pEntity->GetEntityName()),NULL );

			LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpAddToGuildmsg);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)((CActor*)m_pEntity->GetEntityName()));
			//pGuild->SendGuildMsg(smGuildCommon,sText,ttChatWindow);
			((CActor *)m_pEntity)->GetChatSystem()->SendChat(ciChannelGuild, sText);

			CGuild::InterMsg nMsg;
			nMsg.nMsgId = CGuild::imJoin;
			nMsg.hActorHandle = m_pEntity->GetHandle();

			pGuild->FindAndInterMsg(nMsg);
		}
		else
		{
			m_pEntity->SendOldTipmsgFormatWithId(tpJoinApplyHasInGuild,ttFlyTip);
		}
	}
}

VOID CGuildSystem::OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader )
{
	switch(nCmd)
	{
	case jxInterSrvComm::DbServerProto::dcAddMember:
		{
			int Gid = 0;
			reader >> Gid;
			if (nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
			{
				//OutputMsg(rmTip,_T("DB Add Member Succ!gid=%d"),Gid);
			}else
			{
				OutputMsg(rmTip,_T("DB Add Member Error!gid=%d"),Gid);
			}
			break;
		}
		
	case jxInterSrvComm::DbServerProto::dcSaveActorGuild:
		if (nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
		{
			//OutputMsg(rmTip,_T("Save Actor Guild data Succ!"));
		}else
		{
			OutputMsg(rmTip,_T("Save Actor Guild data Error!"));
		}
		break;
	case jxInterSrvComm::DbServerProto::dcLoadActorGuild:
		{
			break;
		}
	case jxInterSrvComm::DbServerProto::dcDeleteMember:
		if (nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
		{
			//OutputMsg(rmTip,_T("delete member Succ!"));
		}else
		{
			OutputMsg(rmTip,_T("delete member Error!"));
		}
		break;
	case jxInterSrvComm::DbServerProto::dcAddMemberOffLine:
		{
			if (nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
			{
				//OutputMsg(rmTip,_T("dcAddMemberOffLine member Succ!"));
				unsigned int nGid = 0;
				reader >> nGid;
				ActorCommonNode Node;
				reader >> Node;
				Node.hActorHandle = 0;
				if (m_pGuild && m_pGuild->m_nGid == nGid)
					m_pGuild->m_ActorOffLine.push_back(Node);				
			}else
			{
				OutputMsg(rmTip,_T("dcAddMemberOffLine member Error!"));
			}
			break;
		}
	case jxInterSrvComm::DbServerProto::dcSaveGuildSkill:
		{
			if (nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
			{
				unsigned int nGid = 0;
				int nSkillId = 0;
				int nLevel = 0;
				reader >> nGid;
				reader >> nSkillId;
				reader >> nLevel;
				if (m_pGuild && m_pGuild->m_nGid == nGid)
				{
					SetGuildSkillLevel(nSkillId,nLevel);
					m_pGuild->SendGuildSkillToAllMember(nSkillId,nLevel);
				}
			}
			break;
		}

	case jxInterSrvComm::DbServerProto::dcLoadActorBuildingData:
		{
			OnRevActorBuildData(reader);
			break;
		}

	}
}
// VOID CGuildComponent::DealGuildApplyResult( CDataPacketReader & inPacket ) 
// {
// 	if(!m_pEntity || !m_pGuild) return;

// 	unsigned int nGuildId = 0;
// 	inPacket >> nGuildId;//帮派id
// 	unsigned int nActorId = 0;
// 	inPacket >> nActorId;
// 	int nResult =0;
// 	int nType = 0;
// 	inPacket >> nType;
// 	inPacket >> nResult;
// 	if(nGuildId != m_pGuild->GetGuildId())
// 	{
// 		return;
// 	}

// 	//同意
// 	if(nType == 1)
// 	{
// 		if(nResult == 1)
// 		{
// 			m_pEntity->SendTipmsgFormatWithId(tmActorInGuild, tstUI);
// 			return;
// 		}else
// 		{
// 			m_pGuild->DeleteApply(nActorId);
// 			//处理添加玩家信息

// 		}
		
// 	}
// 	else if(nTyp == 2) //添加申请
// 	{
		
// 	}
// 	else //拒绝
// 	{
// 		m_pGuild->DeleteApply(nActorId);
// 	}
// }

/*
下发成员申请加入行会的列表
*/
///不用了
VOID CGuildSystem::JoinApplyMsgList()
{
	if (m_pEntity == NULL) return;
	if (m_pGuild == NULL) return;
	
	if (IsFree() || GetGuildPos() <= smGuildCommon)//帮主、副帮主、官员才可查看
	{
		return;
	}

	CActorPacket AP;
	CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);
	DataPacket << (BYTE)enGuildSystemID << (BYTE)sJoinApplyMsgList;
	
	size_t pos = DataPacket.getPosition();

	DataPacket << (int)0;//未必是真实的数量

	int nResult = 0;
	CLinkedNode<CGuild::InterMsg> *pNode;
	CLinkedListIterator<CGuild::InterMsg> it(m_pGuild->m_InterMsgList);
	for (pNode = it.first(); pNode && nResult < m_pGuild->m_nJoinMsgCount; pNode = it.next())
	{
		
		CGuild::InterMsg& Msg = pNode->m_Data;
		if (Msg.nMsgId == CGuild::imJoin)
		{
			//下发玩家信息
			CEntity* pEnity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(Msg.Node.BasicData.nActorId);
			if (pEnity && pEnity->GetType() == enActor)
			{
				CActor* pActor = (CActor*)pEnity;
				DataPacket << pActor->GetProperty<unsigned int>(PROP_ENTITY_ID);
				DataPacket << pActor->GetHandle();
				DataPacket << (BYTE)(pActor->GetProperty<int>(PROP_ACTOR_SEX));
				DataPacket << pActor->GetProperty<int>(PROP_CREATURE_LEVEL);
				DataPacket << (BYTE)pActor->GetMenPai();
				DataPacket << (BYTE)pActor->GetProperty<int>(PROP_ACTOR_VOCATION);
				//DataPacket << (unsigned int)pActor->GetProperty<unsigned int>(PROP_ACTOR_BATTLE_POWER);
				DataPacket << (unsigned int)pActor->GetActorAttackValueMax();	//战斗力改为攻击力
				DataPacket << (BYTE)0;
				DataPacket.writeString(pActor->GetEntityName());
			}
			else
			{
				//如果他已经下线
				DataPacket << (unsigned int)Msg.Node.BasicData.nActorId;
				DataPacket << Msg.hActorHandle;
				DataPacket << (BYTE)(Msg.Node.BasicData.nSex);
				DataPacket << (int)(Msg.Node.BasicData.nLevel);
				DataPacket << (BYTE)(Msg.Node.BasicData.nMenPai);
				DataPacket << (BYTE)(Msg.Node.BasicData.nJob);
				DataPacket << (unsigned int)(Msg.Node.BasicData.nBattleValue);
				DataPacket << (BYTE)(Msg.Node.BasicData.nVipGrade);
				DataPacket.writeString(Msg.Node.sActorName);
			}
			nResult++;
		}
	}
	int* pCount = (int*)DataPacket.getPositionPtr(pos);
	*pCount = nResult;
	AP.flush();
}

VOID CGuildSystem::JoinApplyResult(CDataPacketReader &packet)
{
	if (m_pGuild == NULL) return;
	//看看你是否够权限审核
	if (GetGuildPos() < smGuildTangzhu)		//官员、副帮主、帮主都可以审核
	{
		if (m_pEntity == NULL) return;
		m_pEntity->SendTipmsgFormatWithId(tmNoRight, tstUI);
		return;
	}
	unsigned int nActorid = 0;
	BYTE ApplyResult;

	packet >> ApplyResult;
	packet >> nActorid;
	int nRetCode = m_pGuild->DealApplyByActorId(nActorid, ApplyResult);
	
	if( nRetCode == enCheckJoin_NoErr )
	{
		
		SendJoinGuildApply();		//刷新审核者的面板
		GetGlobalLogicEngine()->GetGuildMgr().AdjustRank(m_pGuild);
		m_pGuild->SendActorApplyToGuild();
	}
	else if( nRetCode == enCheckJoin_MemberLimit )
	{
		//达到了人数上限
		m_pEntity->SendTipmsgFormatWithId(tmMaxMemberCount, tstUI);
		return;
	}
	else if( nRetCode == enCheckJoin_HasGuild )
	{
		((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmActorInGuild, tstUI);
		return;
	}
}

VOID CGuildSystem::JoinApply(CDataPacketReader &packet)
{
	if (m_pEntity == NULL) return;

	unsigned int nJoinGuildId = 0;
	packet >> nJoinGuildId;
	packet >> m_nFastApply;
	
	JoinToGuild(nJoinGuildId);
}

void CGuildSystem::JoinToGuildByRank(int nRank)
{
	CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuildByRank(nRank);
	if (pGuild == NULL)
	{
		m_pEntity->SendOldTipmsgFormatWithId(tpNoThisGuild,ttFlyTip);
		return;
	}

	JoinToGuild(pGuild->m_nGid);
}

void CGuildSystem::JoinToGuild(unsigned int nJoinGuildId)
{
	if(!m_pEntity) return;
	int nLeftTime = 0, nMinLevel = 0,nAotuLevel = 0, errorCode = 0;
	BYTE result = 0;
	do
	{
		if (m_pGuild)
		{
			errorCode = 100;
			break;
		}
		unsigned int nMyGuild = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_GUILD_ID);
		if(nMyGuild > 0)
		{
			return;
		}
		nLeftTime = m_pEntity->GetStaticCountSystem().GetStaticCount(NLEFT_GUILDTIME);
		if (nLeftTime != 0 && time(NULL) < nLeftTime)
		{
			//脱离帮派后，要在规定的时间内，不能加入其他帮派，也不能接受邀请
			errorCode = 101;
			break;
		}

		//判断等级	
		nMinLevel = GetLogicServer()->GetDataProvider()->GetGuildConfig().nLevelLimit;	
		if (m_pEntity->GetProperty<int>(PROP_CREATURE_LEVEL) < nMinLevel)
		{
			errorCode = 102;
			break;
		}
		//加入到内部消息中
		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nJoinGuildId);
		if (pGuild == NULL)
		{
			errorCode = 103;
			break;
		}
		if (pGuild->m_boStatus == false)
		{
			errorCode = 107;
			break;
		} 
		if (pGuild->GetMemberCount() >= pGuild->GetMaxMemberCount())
		{
			//达到了人数上限
			errorCode = 105;
			break;
		}

		nAotuLevel = pGuild->GetAutoJoinLevel();
		//直接加入
		if (m_pEntity->GetProperty<int>(PROP_CREATURE_LEVEL) < nAotuLevel)
		{
			//达到了人数上限
			errorCode = 106;
			break;
		}
		
		if( pGuild->m_AddMemberFlag == 1 &&  m_pEntity->GetProperty<int>(PROP_CREATURE_LEVEL) >= nAotuLevel )
		{
			pGuild->AddMemberToList(m_pEntity);
			LPCTSTR sTitle = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmMailGuild);
			char sContent[1024]={0};
			LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmAddGuidMail);
			sprintf_s(sContent, sizeof(sContent), sFormat, ( char *)(pGuild->GetGuildName()));
			CMailSystem::SendMail(m_pEntity->GetProperty<int>(PROP_ENTITY_ID), sTitle, sContent, NULL, 0);
			// GetGlobalLogicEngine()->GetGuildMgr().AddJoinGuildid(m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID), pGuild->GetGuildId());
			result = 1;
		}
		else
		{
			errorCode = pGuild->AddJoinApply((CActor*)m_pEntity);
			if(errorCode == enApplyJoin_NoErr)
				result = 2;
		}
	}
	while(false);

	if(errorCode && !m_nFastApply)
	{
		SendErrorTipMsg(errorCode, nLeftTime, nAotuLevel, nMinLevel);
	}
	// else
	// {
	// 	m_pEntity->SendTipmsgFormatWithId(tmApplySubmit, tstUI);
	// }
	if(result)
	{
		CActorPacket ap;
		CDataPacket& dataPack = m_pEntity->AllocPacket(ap);
		dataPack << (BYTE)enGuildSystemID <<(BYTE)sJoinApply;
		dataPack << (unsigned int)nJoinGuildId;
		dataPack<<(BYTE)result;
		ap.flush();
	}	
}

void CGuildSystem::SendErrorTipMsg(int errorCode, int nLeftTime, int nAotuLevel, int nLevelLimit)
{
	switch (errorCode)
	{
	case enApplyJoin_NoErr:
		m_pEntity->SendTipmsgFormatWithId(tmApplySubmit, tstUI);
		break;
	case enApplyJoin_HasGuild:
		m_pEntity->SendTipmsgFormatWithId(tmJoinApplyHasInGuild, tstUI);
		break;
	case enApplyJoin_NoGuild:
		m_pEntity->SendTipmsgFormatWithId(tmNoThisGuild, tstUI);
		break;
	case enApplyJoin_MemberLimit:
		m_pEntity->SendTipmsgFormatWithId(tmMaxMemberCount, tstUI);
		break;
	case enApplyJoin_HasSame:
		m_pEntity->SendTipmsgFormatWithId(tmGuildJoinApplyAlready, tstUI);
		break;
	case 100:
		m_pEntity->SendTipmsgFormatWithId(tmJoinApplyHasInGuild, tstUI);
		break;
	case 101:
		m_pEntity->SendTipmsgFormatWithId(tmLeftGuildCanNotJoin, tstUI, nLeftTime/3600);
		break;
	case 102:
	{
		std::string minLevel = intToString(nLevelLimit);
		m_pEntity->SendTipmsgFormatWithId(tmApplyLimiltLevel,tstUI ,minLevel.c_str());
	}break;
	case 103:
		m_pEntity->SendTipmsgFormatWithId(tmNoThisGuild, tstUI);
		break;
	case 105:
		m_pEntity->SendTipmsgFormatWithId(tmMaxMemberCount, tstUI);
		break;
	case 106:
	{
		std::string limitLv = intToString(nAotuLevel);
		m_pEntity->SendTipmsgFormatWithId(tmGuildRequireLvLimit, tstUI, limitLv.c_str());
	}
		break;
	default:
		break;
	}
}


VOID CGuildSystem::InviteActor(CDataPacketReader &packet)
{
	// if (!m_pEntity) return;
	// if (!m_pGuild || GetGuildPos() < smGuildAssistLeader)
	// {
	// 	//副帮主以上才可以邀请加入
	// 	m_pEntity->SendOldTipmsgFormatWithId(tpNoRight,ttFlyTip);
	// 	return;
	// }
	// if (m_pGuild->GetMemberCount() >= m_pGuild->GetMaxMemberCount())
	// {
	// 	//达到了人数上限
	// 	m_pEntity->SendOldTipmsgFormatWithId(tpMaxMemberCount,ttFlyTip,m_pGuild->m_sGuildname);
	// 	return;
	// }
	

	// CGuild::InterMsg Msg;
	// packet >> Msg.hActorHandle;//被邀请的玩家
	// packet.readString(Msg.szName,ArrayCount(Msg.szName));
	// Msg.nMsgId = CGuild::imInvite;
	// Msg.hSrcHandle = m_pEntity->GetHandle();

	// //通知玩家
	// CActor* pEntity = NULL;
	// if(Msg.hActorHandle != 0)
	// {
	// 	CEntity* temp = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(Msg.hActorHandle);
	// 	if (temp && temp->GetType() == enActor)
	// 	{
	// 		pEntity = (CActor*)temp;
	// 	}
	// }
	// if (!pEntity)
	// {
	// 	pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(Msg.szName);
	// }
	// if (pEntity && pEntity->GetType() == enActor)
	// {	
	// 	//判断等级
	// 	int nMinLevel = GetLogicServer()->GetDataProvider()->GetGuildConfig().nLevelLimit;

	// 	if (pEntity->GetProperty<int>(PROP_CREATURE_LEVEL) < nMinLevel)
	// 	{
	// 		m_pEntity->SendOldTipmsgFormatWithId(tpInvitLimiltLevel,ttFlyTip,nMinLevel);
	// 		return;
	// 	}

	// 	if (pEntity->GetGuildSystem()->m_pGuild)
	// 	{
	// 		m_pEntity->SendOldTipmsgFormatWithId(tpLeftGuildCanNotJoin1,ttFlyTip);
	// 		return;
	// 	}
	// 	if (!TimeOK(((CActor*)pEntity)->GetGuildSystem()->nZzNext))
	// 	{
	// 		//脱离帮派后，在规定时间内不能接收邀请
	// 		int nLeftTimeLimit = GetLogicServer()->GetDataProvider()->GetGuildConfig().nLeftTimeLimit/3600;
	// 		m_pEntity->SendOldTipmsgFormatWithId(tpAddToGuildTimeUp,ttFlyTip,nLeftTimeLimit);
	// 		return;
	// 	}

	// 	CActorPacket AP;
	// 	CDataPacket& DataPacket = ((CActor*)pEntity)->AllocPacket(AP);
	// 	DataPacket << (BYTE)enGuildSystemID << (BYTE)sNoticeInvite;
	// 	DataPacket << (unsigned int)m_pGuild->m_nGid;
	// 	DataPacket << (Uint64)(m_pEntity->GetHandle());
	// 	DataPacket.writeString(m_pGuild->m_sGuildname);
	// 	if(m_pGuild->m_pLeader)
	// 	{
	// 		DataPacket.writeString(m_pGuild->m_pLeader->sActorName);
	// 	}
	// 	else
	// 	{
	// 		DataPacket.writeString(m_pGuild->m_sFoundname);
	// 	}
	// 	DataPacket << (int)m_pGuild->GetMemberCount();
	// 	DataPacket << (int)m_pGuild->GetMaxMemberCount();
	// 	AP.flush();

	// 	m_pGuild->AddInterMsg(Msg);				//加到消息队列
	// 	m_pEntity->SendOldTipmsgFormatWithId(tpInviteMsgSend,ttFlyTip);
	// }
	// else
	// {
	// 	m_pEntity->SendOldTipmsgFormatWithId(tpGuildNoOne,ttFlyTip);
	// }
}
bool CGuildSystem::Initialize(void *data,SIZE_T size)
{
	PACTORDBDATA  pActorData = (ACTORDBDATA *)data;
	if(pActorData ==NULL || size != sizeof(ACTORDBDATA))
	{
		OutputMsg(rmWaning,_T("创建角色的数据长度错误，帮派系统初始化失败"));
		return false;
	}
	//CGuild* pGuild = NULL;
	m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_GUILD_ID,pActorData->nGuildID);
	
	CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(pActorData->nGuildID);
	if ( pGuild )
	{
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_GUILD_LEVEL,pGuild->GetGuildbuildingLev(gbtMainHall));
	}
	return true;	
}	

/*
玩家登录时，从行会节点中获取一些信息
*/
 void CGuildSystem::OnEnterGame()
 {
	 if( m_pEntity == NULL )
	 {
		return;
	 }
	
	 //下发语音频道聊天信息
	 CGuild* pGuild = NULL;
	 unsigned int nGid = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_GUILD_ID);
	 if( nGid >= 0 )		//
	 {
		 pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGid);
		 if( pGuild && pGuild->m_boStatus )		//行会存在，并且状态没有被删除
		 {
			 ActorCommonNode* pNode = pGuild->FindMember(m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));
			 if (pNode)
			 {
				 SetGuildPos(pNode->BasicData.nType);
				 nWealNext	= pNode->BasicData.nZjNext;
				 nZzNext	= pNode->BasicData.nZzNext;		//这个属性不一样，玩家离开行会也要维护的
				 nTk		= pNode->BasicData.nTK;
				 m_nTitle	= pNode->BasicData.nGuildTiTle;

				 pNode->BasicData.nTime = m_pEntity->GetLoginTime();	//玩家登陆时间

				 m_GuildGx	= pNode->BasicData.nTotalGx;		//玩家今日行会贡献度
				 m_nJoinTime= pNode->BasicData.nJoinTime;		//更新加入行会的时间
				 //m_TodayGx = pNode->BasicData.nTodayGx;


				/*
				 INT_PTR guildPos = GetGuildPos();
				 if (guildPos == smGuildAssistLeader || guildPos == smGuildLeader)
				 {
					 guildPos = (guildPos == smGuildLeader?1:2);
					 m_pEntity->GetAchieveSystem().OnEvent(eAchieveEventGuildMaster,guildPos);
				 }
				 */
			 }
			 else//帮派成员没找到他，说明他没加入
			 {
				 pGuild = NULL;
				 if( nGid > 0 )
				 {
					 //防止出现actor的guildid大于0，但是行会实际不存在的情况
					 ((CActor*)m_pEntity)->SetProperty<unsigned int>(PROP_ACTOR_GUILD_ID,0);
					 ((CActor*)m_pEntity)->SetProperty<unsigned int>(PROP_ACTOR_GUILD_LEVEL,0);
				 }

				 //判断能否审核通过加入行会（相当于中途加入）
				 pGuild = GetGlobalLogicEngine()->GetGuildMgr().ActorJoinGuildByApply( (CActor*)m_pEntity );
			 }
		 }
		 else				//行会不存在，或者已被删除
		 {
			 //防止出现actor的guildid大于0，但是行会实际不存在的情况
			 ((CActor*)m_pEntity)->SetProperty<unsigned int>(PROP_ACTOR_GUILD_ID,0);
			 ((CActor*)m_pEntity)->SetProperty<unsigned int>(PROP_ACTOR_GUILD_LEVEL,0);

			 //判断能否审核通过加入行会（相当于中途加入）
			 pGuild = GetGlobalLogicEngine()->GetGuildMgr().ActorJoinGuildByApply( (CActor*)m_pEntity );
		 }
	 }

	 LoadActorBuildingData();

	 m_pGuild = pGuild;
	if (m_pGuild)
	{
		if(GetGlobalLogicEngine()->GetGuildMgr().GetCityOwnGuildId() == m_pGuild->GetGuildId())
		{
			((CActor*)m_pEntity)->GetAchieveSystem().ActorAchievementComplete(nAchieveJionSBKGuild, 1);
		}
		OnLogin();
		OnLoginDeclarNotice();
		if(GetGlobalLogicEngine()->GetGuildMgr().IsGuildSiegeStart() && ((CActor*)m_pEntity)->GetSceneAreaFlag() == scfGuildSiege)
			UpdateNameClr_OnPkModeChanged(m_pEntity->GetHandle());
	}
	//  else
	//  {
	// 	 //m_pEntity->SetSocialMask(smIsSbkCityMaster,false);
	//  }
	 //else//没加入任何帮派
	 //{
	 //	SetGuildPos(smGuildCommon);
	 //}
	//  if (m_pEntity->GetLastLogoutTime() >= CMiniDateTime::today())//如果上次下线与今天是同一天，不用清除召集令
	//  {
	// 	 boWealClear = false;
	//  }

	//  m_nTodayDonateCoin = ((CActor*)m_pEntity)->GetStaticVarValue(szDailyDonateCoin);
	//  m_nTodayDonateItem = ((CActor*)m_pEntity)->GetStaticVarValue(szDailyDonateItem);

	//  m_nTodayChargeTreeNum = ((CActor*)m_pEntity)->GetStaticVarValue(szTodayChargeTreeNum);
	//  m_nTodayPickFruitNum = ((CActor*)m_pEntity)->GetStaticVarValue(szTodayPickFruitNum);

	//  if( m_pGuild )
	//  {
	// 	 CGuildProvider& pConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	// 	 int nPickNumLast = pConfig.m_GuildTree.m_nDailyPickLimit - m_nTodayPickFruitNum;	//剩余采摘次数
	// 	 if( m_pGuild->m_nGuildTreeLevel >= GUILDTREE_PERIOD_4 &&  nPickNumLast > 0 )
	// 	 {
	// 		 SendGuildOpOk(GUILD_CAN_PICKFRUIT);		//可以摘取果实
	// 	 }

	// 	 int nGuildTaskObjNum	= m_pGuild->GetGuildTaskObjNum(m_pGuild->m_nGuildTaskId);
	// 	 int nGetTaskAwardNum	=((CActor*)m_pEntity)->GetStaticVarValue(szTodayGetTaskAwardNum);
	// 	 if( m_pGuild->m_nGuildTaskSche >= nGuildTaskObjNum && nGetTaskAwardNum <= 0 )	//任务已完成，尚未领奖
	// 	 {
	// 		 SendGuildOpOk(GUILD_CAN_GETTASKAWRD);
	// 	 }

	// 	 //如果某个活动已经开启，则通知客户端
	// 	 for( int nGuildActiveId=GuildActive_Tree; nGuildActiveId< GuildActive_Max; nGuildActiveId++ )
	// 	 {
	// 		 if( m_pGuild->GetGuildActiveSts( nGuildActiveId ) == 1 )
	// 		 {
	// 			 SendGuildActiveSts(nGuildActiveId);
	// 		 }
	// 	 }

	// 	 SendGuildSiegeCmdData( );			//下发攻城战指令
	//  }
 }


void CGuildSystem::OnLoginDeclarNotice()
{
	if(m_pGuild == NULL) return;

	INT_PTR nCount = m_pGuild->m_WarList.count();
	for (INT_PTR i = 0; i < nCount; i++)
	{
		WarRelationData& war= m_pGuild->m_WarList[i];

		if( m_pGuild->GetRelationShipInWar(war.nGuildId) == GUILD_WARSHIP_YES )
		{
			CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(war.nGuildId);

			if(pGuild)
			{
				m_pEntity->SendOldTipmsgFormatWithId(tpOnDeclarState,ttChatWindow + ttScreenCenter,pGuild->m_sGuildname);
			}
		}
	}

	if(m_pGuild->m_SignUpFlag == 1)	//已报名明天的沙巴克战  
	{
		//m_pEntity->SendOldTipmsgFormatWithId(tpSignUpGuildWarLoginTips,ttChatWindow + ttTipmsgWindow);
	}

	/*if(m_pGuild->m_SignUpFlagLast == 1)
	{
	SYSTEMTIME &sysTime = GetLogicServer()->GetLogicEngine()->getSysTime();
	if(sysTime.wHour < 21)
	{
	m_pEntity->SendOldTipmsgFormatWithId(tpGuildSBKMsg1,ttChatWindow + ttTipmsgWindow);
	}
	}*/
	//TODOSBK
}


//玩家登陆时，处理行会相关数据
//GetGlobalLogicEngine()->GetGuildMgr().m_GuildId : SBK占领行会
//m_pGuild->m_nGid：本人所在行会
VOID CGuildSystem::OnLogin()
{
	if (m_pEntity == NULL) return;
	if (m_pGuild)
	{
		const CGuildProvider& pConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();

		//加到在线列表中
		m_pGuild->m_ActorOnLine.linkAtLast(m_pEntity->GetHandle());
		//非在线列表中的node要设置成pActor的handle
		m_pGuild->FindAndSetHandle(m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID),m_pEntity->GetHandle());

		//下发这个帮派的敌对帮派列表
		m_pGuild->SendWarGuildList(m_pEntity);

		SendGuildTitleList();
		m_pGuild->SendActorApplyToGuild();			//通知有入帮消息，必须在m_pGuild->m_ActorOnLine.linkAtLast()之后调用

		CheckGuildPlayerPos(gptEnter);
	}
	else
	{		
		ClearZJ();//不再任何帮派，清理帮派物品（召集令）
		// m_pEntity->GetQuestSystem()->ClearGuildQuest();//清理帮派任务
	}
}

void CGuildSystem::OnActorLoginOut()
{
	
}

VOID CGuildSystem::Destroy()
{
	if (m_pGuild)
	{
		if (m_pEntity == NULL) return;
		EntityHandle hHandle = m_pEntity->GetHandle();
		//先从在线列表中删除
		m_pGuild->DeleteOnLineList(hHandle);
		//设置它不在线
		ActorCommonNode * pOffLineNode = m_pGuild->FindMember(m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));
		if (pOffLineNode)
		{
			pOffLineNode->hActorHandle = 0;
			//更新数据
			m_pGuild->UpdateActorData((CActor*)m_pEntity,pOffLineNode);
		}
	}
}

int CGuildSystem::CreateGuild(CDataPacketReader &packet)
{
	if (m_pEntity == NULL) return qeSucc;
	
	//是否已在某个帮派中
	if (!IsFree())
	{
		m_pEntity->SendTipmsgFormatWithId(tmCreateGuildHasInGuild,tstUI);
		return qeNotFree;
	}

	// if (nZzNext != 0 && !TimeOK(nZzNext))
	// {
	// 	//脱离帮派后，要在规定的时间内，不能加入其他帮派，也不能接受邀请，不能创建帮派
	// 	int nLeftTimeLimit = GetLogicServer()->GetDataProvider()->GetGuildConfig().nLeftTimeLimit/3600;
	// 	m_pEntity->SendTipmsgFormatWithId(tmLeftGuildCanNotJoin,tstUI,nLeftTimeLimit);
	// 	return qeTimeErr;
	// }


	int nLeftTime = m_pEntity->GetStaticCountSystem().GetStaticCount(NLEFT_GUILDTIME);
	if (nLeftTime != 0 && time(NULL) < nLeftTime)
	{
		//脱离帮派后，要在规定的时间内，不能加入其他帮派，也不能接受邀请
		int nLeftTimeLimit = GetLogicServer()->GetDataProvider()->GetGuildConfig().nLeftTimeLimit/3600;
		m_pEntity->SendTipmsgFormatWithId(tmLeftGuildCanNotJoin, tstUI, nLeftTimeLimit);
		return qeTimeErr;
	}

	//判断等级
	int nMinLevel = GetLogicServer()->GetDataProvider()->GetGuildConfig().nLevelLimit;

	if (m_pEntity->GetProperty<int>(PROP_CREATURE_LEVEL) < nMinLevel)
	{
		m_pEntity->SendTipmsgFormatWithId(tmCreateGuildErrorLevel, tstUI);
		return qeLevel;
	}

	char sGuildName[32];
	packet.readString(sGuildName,ArrayCount(sGuildName));
	sGuildName[sizeof(sGuildName)-1] = 0;
	GetGlobalLogicEngine()->GetChatMgr().Filter(sGuildName);

	//这里不能扣除钱币，因为有可能创建失败的
	INT_PTR nCreateNeedYb		= GetLogicServer()->GetDataProvider()->GetGuildConfig().nCreateNeedYb;
	unsigned int nMyYb = ((CActor*)m_pEntity)->GetMoneyCount(mtYuanbao);
	if( nMyYb < nCreateNeedYb )		//没有足够的游戏币
	{
		m_pEntity->SendTipmsgFormatWithId(tmNomoreYubao, tstUI);
		return 0;
	}
	CGuildComponent& mgr = GetGlobalLogicEngine()->GetGuildMgr();

	//首先通知数据库服务器创建这个行会的存储数据
	mgr.SendDbAddGuild(m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID),m_pEntity->GetHandle(),sGuildName,m_pEntity->GetEntityName());
	return qeSucc;
}

int CGuildSystem::DeleteGuild()
{
	if (m_pEntity == NULL) return qeSucc;
	if(m_pGuild == NULL) return qeSucc;
	return qeSucc;
}

int CGuildSystem::GetGuildLevel()
{
	return m_pGuild?m_pGuild->GetLevel():0;
}

void CGuildSystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet )
{
	switch (nCmd)
	{
	case cGuildInfo:		//请求本行会详细信息
		SendGuildInfoResult();		//下发行会详细信息
		break;
	case cGuildMember: //成员列表
		SendMemberList();
		break;
	case cGuildList: //行会列表
		SendAllGuildInfo(packet);
		break;
	case cAddGuild:   //创建行会
		CreateGuild(packet);
		break;
	// case cDelGuild: //不需要
	// 	DeleteGuild();
	// 	break;
	// case cInvite:
	// 	InviteActor(packet);
	// 	break;
	// case sInviteResult:
	// 	InviteResult(packet);
	// 	break;
	case cJoinApply:
		JoinApply(packet);
		break;
	case cJoinApplyMsgList:				//请求申请行会的玩家的列表
		SendJoinGuildApply();
		break;

	case cGuildBuilds:
		sendGuilBuilds();
		break;
	case cDeleteMember:
		{
			unsigned int nActorid = 0;
			packet >> nActorid;
			DeleteMemberRequest(nActorid);
			break;
		}
	case cLeftGuild:
		{
			BYTE bType = 0;
			//packet >> bType;
		
			LeftGuild(bType);
			break;
		}
	case cChangeGuildPos:
		{
			ChangeGuildPos(packet);
			//SendGuildCityPosInfo();
			break;
		}
	case cLeaderChange:
		{
			LeaderChange(packet);
			break;
		}
	case cUpdateMemo:
		{
			UpdateMemo(packet);
			break;
		}
	case cDeclareWar:
		{
			unsigned int nObjGuildid = 0;
			packet >> nObjGuildid;
			DeclareWar(nObjGuildid);
			break;
		}
	

	case cGuildDonate:
		{
			byte nType = 0;
			packet >> nType;

			GuildDonate(nType);
			break;
		}
	case cGuildDonateInfo:
		{
			SendGuildDonateInfo();
			break;
		}

	
	case cSetAddMemberFlag:
	{
		OnSetAddMemberFlag(packet);
		break;
	}
	case cGuildUpdateBuild:
	{
		UpdateGuildBuild(packet);
		break;
	}
	case cDealGuildApply:
	{
		JoinApplyResult(packet);
		break;
	}
	case cGuildLog:
	{
		SendGuildEventLog();
		break;
	}
	case cImpeachGuildMember:
		{
			unsigned int nObjActorId = 0;
			packet >> nObjActorId;
			ImpeachGuildMember(nObjActorId);
			break;
		}

	case cGuildWarList:
		{
			SendGuildWarList();
			break;
		}
	case cGuildGetSbkInfo:
		{
			SendSbkInfo();
			break;
		}
	default:
		break;
	}
}


VOID CGuildSystem::HandItemToGuildDepot(CDataPacketReader &packet)
{
	if (m_pEntity == NULL) return;
	if (m_pGuild == NULL) return;

	/*
	if (GetGuildPos() < smGuildAssistLeader)
	{
		//副帮主以上才有权限操作
		m_pEntity->SendOldTipmsgFormatWithId(tpNoRight,ttFlyTip);
		return;
	}
	*/

	BYTE nType = 0;
	packet >> nType;
	int nMoney = 0;
	packet >> nMoney;

	CUserItem::ItemSeries series ;
	packet >> series;

	if(nMoney < 0)
	{
		nMoney = 0;
	}

	const CGuildProvider& guildConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();

	CUserBag& bag= m_pEntity->GetBagSystem();
	//从角色的包裹中查找要穿戴的物品
	INT_PTR nBagIdx = bag.FindIndex(series);
	if ( nBagIdx < 0 )
	{
		m_pEntity->SendOldTipmsgWithId(tpItemNotExist,ttFlyTip);
		return ;
	}

	CUserItem *pUserItem = bag[nBagIdx];
	if(pUserItem ==NULL) return;
	if( pUserItem->binded() || pUserItem->IsDenyDeal() )		//绑定装备或不可交易装备不能放入仓库
	{
		m_pEntity->SendOldTipmsgWithId(tpGuildDepotPutInBind,ttFlyTip ); //
		return;
	}

	if( m_pEntity->GetDealSystem().IsDealing() ) //在交易的时候不能放仓库
	{
		m_pEntity->SendOldTipmsgWithId(tpFailForDealing,ttFlyTip ); //
		return;
	}

	//这里要处理仓库是否已经满的情况
	//if(m_pGuild->GetGuildDepotCount() >= MAX_GUILDDEPOTBAG)
	int nDepotGridCount = m_pGuild->GetGuildDepotGridsCount();
	if( m_pGuild->GetGuildDepotCount() >= nDepotGridCount )		//行会仓库满了
	{
		m_pEntity->SendOldTipmsgWithId(tpDepotBagIsFull,ttFlyTip); 
		return ;
	}
	const CStdItem * pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
	if(pStdItem ==NULL) return;

	//--废弃
	//特戒、幻武、玄甲装备不能被投入行会仓库

	// if( pStdItem->m_btType == Item::itSpecialRing ||pStdItem->m_btType == Item::itExtraWeapon 
	// 	|| pStdItem->m_btType == Item::itExtraDress )
	// {
	// 	m_pEntity->SendOldTipmsgWithId(tpGuildDepotPutInUnvalidType,ttFlyTip );
	// 	return;
	// }


	if( pStdItem->m_Flags.denyGuildDepot )				//注意，denyStorage是禁止放入玩家仓库
	{
		m_pEntity->SendOldTipmsgWithId(tpItemCanNotPutDepot,ttFlyTip); //禁止存放行会仓库
		return ;
	}

	if( !pStdItem->isEquipment() )
	{
		m_pEntity->SendOldTipmsgWithId(tpGuildDepotPutInNotEquip,ttFlyTip);	//不是装备
		return ;
	}

	int nItemCircle = (int)pStdItem->GetUseConditionValue(CStdItem::ItemUseCondition::ucMinCircle);	//转数
	int nItemLevel  = (int)pStdItem->GetUseConditionValue(CStdItem::ItemUseCondition::ucLevel);		//等级
	/*
	if( nItemCircle <= 0  )
	{
		m_pEntity->SendOldTipmsgWithId(tpGuildDepotPutInNotCircleItem, ttFlyTip); //非转生装备禁止放仓库
		return;
	}
	*/

	//((CActor*)m_pEntity)->SetStaticVarValue(szDailyDepotPutInNum, nDailyDepotPutInNum+1 );		//每日投入次数加1

	// bag.RemoveItem(nBagIdx,"guild_deport",GameLog::clBagToGuildDepot,false);

	pUserItem->btSharp = 0;
	GUILDDEPOTITEM nItem;
	nItem.nActorId = m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID);
	nItem.nCoinType = (int)nType;
	nItem.nCoinNum = nMoney;

	nItem.nUserItem = pUserItem;
	m_pGuild->m_GuildDepotBag.add(nItem);

	/*不记录操作
	GUILDDEPOTMSG nDepotMsg;
	nDepotMsg.nType = 1;
	_asncpytA(nDepotMsg.nActorName,m_pEntity->GetEntityName());
	nDepotMsg.nTime = GetGlobalLogicEngine()->getMiniDateTime();
	nDepotMsg.nCoinType = nType;
	nDepotMsg.nCoinNum = nMoney;
	_asncpytA(nDepotMsg.nItemName,pStdItem->m_sName);
	nDepotMsg.nNum = pUserItem->wCount;
	m_pGuild->m_GuildDepotRecord.insert(0,nDepotMsg);
	*/
	m_pGuild->NotifyUpdateDepot();		//通知客户端重新请求数据
	m_pGuild->m_bSaveDepotRecord = true;

	CGuildProvider& pGuildConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	int nAddGx = pGuildConfig.GetGuildDepotPutInAddGx(nItemCircle, nItemLevel, pStdItem->m_btType);
	if( nAddGx > 0 )
	{
		ChangeGuildGx(nAddGx, GameLog::Log_GuildDonate);		//增加本人的行会贡献

		//int nLastTimes = nDailyDepotPutInLimit-nDailyDepotPutInNum-1;
		m_pEntity->SendOldTipmsgFormatWithId(tpGuildDepotPutInMsg, ttFlyTip, nAddGx); 
	}

	char chatMsg[1024]={0};
	LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildDepotPutInItem);
	sprintf_s(chatMsg, sizeof(chatMsg), sFormat, ( char *)(m_pEntity->GetEntityName()), 
		CUserBag::GetItemLink((int)pUserItem->wItemId, pUserItem), nAddGx);
	m_pGuild->BroadCastMsgInGuildChannel(chatMsg);

	//记录日志
	if(GetLogicServer()->GetLocalClient())
	{
	}
	// m_pGuild->AddEventRecord( "", enGuildEvent_PutInDepot, 0, 0, 0, (char *)(m_pEntity->GetEntityName()), (char *)pStdItem->m_sName );
}


/*
审核行会仓库物品兑换请求
*/
void CGuildSystem::CheckGuildDepotItemExchange( CDataPacketReader &packet )
{
	if( m_pEntity == NULL )
	{
		return;
	}
	if( m_pGuild == NULL )
	{
		return;
	}

	if( GetGuildPos() < smGuildAssistLeader )		//只有会长、副会长才有权限审核
	{
		//官员以上才有权限操作
		m_pEntity->SendOldTipmsgFormatWithId(tpNoRight,ttFlyTip);
		return;
	}

	CUserItem::ItemSeries series ;
	BYTE nIsPass=0;
	packet >> series >> nIsPass;

	INT_PTR nIdx = m_pGuild->FindItemIndex(series);				//在行会仓库里查找此装备
	if( nIdx < 0 )
	{
		m_pEntity->SendOldTipmsgWithId(tpItemNotExist);
		return ;
	}

	GUILDDEPOTITEM &stDepotItem = m_pGuild->m_GuildDepotBag[nIdx];
	if( stDepotItem.nExchangeActorId <= 0 )		//没有人兑换
	{
		m_pEntity->SendOldTipmsgWithId(tpGuildDepotExchangeNo);
		return;
	}
	CUserItem *pUserItem = stDepotItem.nUserItem;
	if( !pUserItem )
	{
		return;
	}

	const CStdItem * pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
	if( pStdItem ==NULL )
	{
		return;
	}
	int nItemCircle = (int)pStdItem->GetUseConditionValue(CStdItem::ItemUseCondition::ucMinCircle);	//转数
	int nItemLevel  = (int)pStdItem->GetUseConditionValue(CStdItem::ItemUseCondition::ucLevel);		//等级

	if( nIsPass )			//审核通过
	{
		LPCTSTR sTitle		= GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildDepotExchangeTitlt1);
		LPCTSTR sContent	= GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildDepotExchangeCont1);
		CMailSystem::SendMailFromUserItem(stDepotItem.nExchangeActorId, sTitle, sContent, pUserItem);

		char sText[1024];
		LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildDepotItemExchangeAgree);
		sprintf_s(sText, sizeof(sText), sFormat, ( char *)( stDepotItem.szExchangeActorName ),
			( char *) CUserBag::GetItemLink( pUserItem->wItemId, pUserItem ) );
		m_pEntity->SendTipmsg( sText, ttFlyTip+ttTipmsgWindow );

		// m_pGuild->AddEventRecord( "", enGuildEvent_GetOutDepot, 0, 0, 0, (char *)( stDepotItem.szExchangeActorName ), 
		// 	(char *)pStdItem->m_sName );
		m_pGuild->m_GuildDepotBag.remove(nIdx);			//仓库删除此物品(仅仅将该元素踢出列表，但是存储空间还在)
		m_pGuild->m_bSaveDepotRecord	= true;			//需要存储
		if (pUserItem)//发邮件后内存要释放
		{
			GetGlobalLogicEngine()->DestroyUserItem(pUserItem);
		}
	}
	else
	{
		unsigned int nExchangeActorId = stDepotItem.nExchangeActorId;
		ACTORNAME szExchangeActorName;
		_asncpytA(szExchangeActorName, stDepotItem.szExchangeActorName);

		//该物品设置为未被申请兑换
		stDepotItem.nExchangeActorId		= 0;
		stDepotItem.szExchangeActorName[0]	= 0;
		stDepotItem.nExchangeTime			= 0;
		m_pGuild->m_bSaveDepotRecord		= true;		//需要存储

		LPCTSTR sTitle		= GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildDepotExchangeTitlt2);
		LPCTSTR sContent	= GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildDepotExchangeCont2);

		CGuildProvider& pGuildConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();
		int nNeedGx = pGuildConfig.GetGuildDepotGetOutNeedGx(nItemCircle, nItemLevel, pStdItem->m_btType);
		if( nNeedGx < 0 )
		{
			nNeedGx = 0;
		}
		CMailSystem::MAILATTACH mailAttach[MAILATTACHCOUNT];
		mailAttach[0].nType		= qatGuildContribution;
		mailAttach[0].item.wCount	= nNeedGx;					//返回玩家申请兑换时扣除的行会贡献
		CMailSystem::SendMail(nExchangeActorId, sTitle, sContent,mailAttach);

		char sText[1024];
		LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildDepotItemCheckingReject);
				sprintf_s( sText, sizeof(sText), sFormat, ( char *)( szExchangeActorName ),
				( char *) CUserBag::GetItemLink( pUserItem->wItemId, pUserItem ) );
		m_pEntity->SendTipmsg( sText, ttFlyTip+ttTipmsgWindow );
	}
	m_pGuild->NotifyUpdateDepot();				//通知客户端重新请求一次

	//记录日志
	if(GetLogicServer()->GetLocalClient())
	{
	}
}

VOID CGuildSystem::SendWarList()
{
	if (m_pEntity == NULL) return;
	if (m_pGuild == NULL)
	{
		m_pEntity->SendOldTipmsgFormatWithId(tpNotInGuild,ttFlyTip);
		return;
	}
	m_pGuild->SendWarList(m_pEntity);
}

VOID CGuildSystem::SendWarHistory()
{
	if (m_pEntity == NULL) return;
	if (m_pGuild == NULL)
	{
		m_pEntity->SendOldTipmsgFormatWithId(tpNotInGuild,ttFlyTip);
		return;
	}
	m_pGuild->SendWarHistory(m_pEntity);
}

//设置与目标行会的关系
VOID CGuildSystem::SetGuildRelation( BYTE nType, unsigned int nObjGuildId )
{
	if (m_pEntity == NULL) return;
	if (m_pGuild == NULL)
	{
		m_pEntity->SendOldTipmsgFormatWithId(tpNotInGuild,ttFlyTip);
		return;
	}

	if (nObjGuildId == m_pGuild->m_nGid ) return;

	int nGuildPos = GetGuildPos();
	//帮派帮主、副帮主
	if (nGuildPos != smGuildLeader && nGuildPos != smGuildAssistLeader)
	{
		//只能帮主做这个操作
		m_pEntity->SendOldTipmsgFormatWithId(tpNoRight,ttFlyTip);
		return;
	}

	switch(nType)
	{
		//联盟
	case 1:
		{
			SetUnionGuild(nObjGuildId,(int)nType);
			break;
		}
		//敌对
	case 2:
		{
			//SetEnemryGuild(nGuildId);
			break;
		}
		//解除联盟
	case 3:
		{
			RefuseGuildUnion(nObjGuildId);
			break;
		}
		//解除敌对
	case 4:
		{
			//RefuseEnemryGuild(nGuildId);
			break;
		}

	default:
		break;
	}
}

void CGuildSystem::SetEnemryGuild(unsigned int nGuildId)
{
	if (m_pEntity == NULL) return;
	if (m_pGuild == NULL) return;

	CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
	if(pGuild)
	{
		int curType = m_pGuild->GetRelationShipForId(nGuildId);

		//已经是联盟关系
		if(curType == 1)
		{
			m_pEntity->SendOldTipmsgFormatWithId(tpIsUnionCanNotBeEnmery,ttFlyTip);
			return;
		}

		//是敌对关系
		if(curType == 2)
		{
			m_pEntity->SendOldTipmsgFormatWithId(tpIsEnemryRelation,ttFlyTip);
			return;
		}

		m_pGuild->SetRelationShipWithId(nGuildId,2);
		pGuild->SetRelationShipWithId(m_pGuild->m_nGid,2);

		m_pGuild->UpdateGuildList();
		pGuild->UpdateGuildList();

		pGuild->SendGuildMsg(smGuildCommon,tpIsEnemryTips,ttTipmsgWindow,m_pGuild->m_sGuildname);
		m_pGuild->SendGuildMsg(smGuildCommon,tpIsEnemryTips,ttTipmsgWindow,pGuild->m_sGuildname);
	}
}

void CGuildSystem::RefuseEnemryGuild(unsigned int nGuildId)
{
	if (m_pEntity == NULL) return;
	if (m_pGuild == NULL) return;

	CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
	if(pGuild)
	{
		int curType = m_pGuild->GetRelationShipForId(nGuildId);

		if(curType == 2)
		{
			m_pGuild->SetRelationShipWithId(nGuildId,0);
			pGuild->SetRelationShipWithId(m_pGuild->m_nGid,0);

			m_pGuild->UpdateGuildList();
			pGuild->UpdateGuildList();

			pGuild->SendGuildMsg(smGuildCommon,tpRefuseEnemryRelation,ttTipmsgWindow,m_pGuild->m_sGuildname);
			m_pGuild->SendGuildMsg(smGuildCommon,tpRefuseEnemryRelation,ttTipmsgWindow,pGuild->m_sGuildname);
		}
	}
}

void CGuildSystem::RefuseGuildUnion(unsigned int nGuildId)
{
	if (m_pEntity == NULL) return;
	if (m_pGuild == NULL) return;

	CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
	if(pGuild)
	{
		int curType = m_pGuild->GetRelationShipForId(nGuildId);

		if(curType == 1)
		{
			m_pGuild->SetRelationShipWithId(nGuildId,0);
			pGuild->SetRelationShipWithId(m_pGuild->m_nGid,0);

			m_pGuild->UpdateGuildList();
			pGuild->UpdateGuildList();

			if(GetGlobalLogicEngine()->GetGuildMgr().IsGuildSiegeStart())	//开始沙巴克战
			{
				m_pGuild->UpdateGuildActorNameInSbk();
				pGuild->UpdateGuildActorNameInSbk();
			}

			pGuild->SendGuildMsg(smGuildCommon,tpRemoveUnionRelation,ttScreenCenter+ttTipmsgWindow,m_pGuild->m_sGuildname);
			m_pGuild->SendGuildMsg(smGuildCommon,tpRemoveUnionRelation,ttScreenCenter+ttTipmsgWindow,pGuild->m_sGuildname);
			//行会模式地图下广播下坐标的敌对关系
			for(int i =0; i <=1;i++)
			{
				CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
				CLinkedNode<EntityHandle> *pNode;
				CLinkedListIterator<EntityHandle> it(i==0?m_pGuild->m_ActorOnLine:pGuild->m_ActorOnLine);
				for (pNode = it.first(); pNode; pNode = it.next())
				{
					EntityHandle& hHandle = pNode->m_Data;
					CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
					if (pEntity && pEntity->GetType() == enActor)
					{
						CActor* pActor = (CActor*)pEntity;
						pActor->GetGuildSystem()->CheckGuildPlayerPos(gptMove);
					}
				}
			}
		}
	}
}

void CGuildSystem::SetUnionGuild(unsigned int nGuildId,int nType)
{
	if (m_pEntity == NULL) return;
	if (m_pGuild == NULL) return;

	CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);

	if(pGuild)
	{
		int curType = m_pGuild->GetRelationShipForId(nGuildId);

		//已经是联盟关系
		if(curType == 1)
		{
			m_pEntity->SendOldTipmsgFormatWithId(tpIsUnionRelation,ttFlyTip);
			return;
		}

		//是敌对关系
		if(curType == 2)
		{
			m_pEntity->SendOldTipmsgFormatWithId(tpEnemyRelation,ttFlyTip);
			return;
		}

		//需要的行会资金
		int needCoin = GetLogicServer()->GetDataProvider()->GetGuildConfig().m_GuildRelation.nUnionNeedCoin;
		if(m_pGuild->GetCoin() < needCoin)
		{
			m_pEntity->SendOldTipmsgFormatWithId(tpNoMoreGuildCoinUnion,ttFlyTip,needCoin);
			return;
		}

		

		if(pGuild->SendWarUnionMsgToActor(m_pGuild->m_nGid,m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID),m_pGuild->m_sGuildname) == false)
		{
			m_pEntity->SendOldTipmsgFormatWithId(tpTagetGuildLeaderOffline,ttFlyTip);
			return;
		}
		m_pEntity->SendOldTipmsgFormatWithId(tpBeTheUnionNotice,ttChatWindow + ttTipmsgWindow);
		
		m_pGuild->ChangeGuildCoin(-needCoin, 0,"SetUnion");

		INTERRELATIONMSG tmpMsg;

		tmpMsg.nGuildId = nGuildId;

		int nCount = (int)m_pGuild->m_UiounMsgList.count();

		for(int i=0;i<nCount;i++)
		{
			if(tmpMsg.nGuildId == m_pGuild->m_UiounMsgList[i].nGuildId)
				return;
		}

		m_pGuild->m_UiounMsgList.add(tmpMsg);

		//添加帮派事件
		//char sText[1024];
		//LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildEventUnion);
		//sprintf_s(sText, sizeof(sText), sFormat, ( char *)(pGuild->GetGuildName()));
		//m_pGuild->AddEventRecord(sText,tpGuildEventUnion,0,0,0,( char *)(pGuild->GetGuildName()), "");

		//添加帮派事件
		//char sText1[1024];
		//LPCTSTR sFormat1 = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildEventBeUnion);
		//sprintf_s(sText1, sizeof(sText1), sFormat1, ( char *)(m_pGuild->GetGuildName()));
		//pGuild->AddEventRecord(sText1,tpGuildEventBeUnion,0,0,0,( char *)(m_pGuild->GetGuildName()), "");
		
	}
}

VOID CGuildSystem::WarRelationResult(CDataPacketReader &packet)
{
	if (m_pEntity == NULL) return;
	if (m_pGuild == NULL) return;

	int nGuildPos = GetGuildPos();
	//帮派帮主、副帮主
	if (nGuildPos != smGuildLeader && nGuildPos != smGuildAssistLeader)
	{
		//只能帮主做这个操作
		m_pEntity->SendOldTipmsgFormatWithId(tpNoRight,ttFlyTip);
		return;
	}
	
	BYTE nType = 0;
	packet >> nType;
	unsigned int nGuildId = 0;
	packet >> nGuildId;
	unsigned int nActorId = 0;
	packet >> nActorId;

	CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
	if(pGuild)
	{
		int curType = m_pGuild->GetRelationShipForId(nGuildId);

		//已经是联盟关系
		if(curType == 1)
		{
			m_pEntity->SendOldTipmsgFormatWithId(tpIsUnionRelation,ttFlyTip);
			return;
		}

		//是敌对关系
		if(curType == 2)
		{
			m_pEntity->SendOldTipmsgFormatWithId(tpEnemyRelation,ttFlyTip);
			return;
		}

		int nIndex = -1;
		for(int i=0;i<pGuild->m_UiounMsgList.count();i++)
		{
			INTERRELATIONMSG& tmpMsg = pGuild->m_UiounMsgList[i];

			if(tmpMsg.nGuildId == m_pGuild->m_nGid)
			{
				nIndex = i;
				break;
			}
		}

		if(nIndex < 0) return;

		CEntity* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);		
		//同意
		if(nType == 1)
		{
			m_pGuild->SetRelationShipWithId(nGuildId,nType);
			pGuild->SetRelationShipWithId(m_pGuild->m_nGid,nType);
			pGuild->m_UiounMsgList.remove(nIndex);

			m_pGuild->UpdateGuildList();
			pGuild->UpdateGuildList();

			if(GetGlobalLogicEngine()->GetGuildMgr().IsGuildSiegeStart())	//开始沙巴克战
			{
				m_pGuild->UpdateGuildActorNameInSbk();
				pGuild->UpdateGuildActorNameInSbk();
			}

			/* 取消联盟广播 改为对行会内部广播
			
			*/
			//m_pGuild->SendGuildMsg( smGuildCommon, tpBecomeUnionGuild, ttChatWindow, m_pGuild->m_sGuildname,pGuild->m_sGuildname);
			//pGuild->SendGuildMsg( smGuildCommon, tpBecomeUnionGuild, ttChatWindow, m_pGuild->m_sGuildname,pGuild->m_sGuildname);
			
			CEntityManager* em	= GetGlobalLogicEngine()->GetEntityMgr();
			em->BroadTipmsgWithParams(tpBecomeUnionGuild, ttScreenCenter + ttChatWindow, m_pGuild->m_sGuildname,pGuild->m_sGuildname);


			char sText[100] = {0};
			LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpBecomeUnionGuild);
			sprintf_s(sText, sizeof(sText), sFormat, m_pGuild->m_sGuildname, pGuild->m_sGuildname);
			// m_pGuild->BroadCastMsgInGuildChannel(sText);
			// pGuild->BroadCastMsgInGuildChannel(sText);
			//OutputMsg(rmNormal,"WarRelationResult, sFormat=%s, sText=%s",sFormat, sText);
			
			//行会模式地图下广播下坐标的敌对关系
			for(int i =0; i <=1;i++)
			{
				CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
				CLinkedNode<EntityHandle> *pNode;
				CLinkedListIterator<EntityHandle> it(i==0?m_pGuild->m_ActorOnLine:pGuild->m_ActorOnLine);
				for (pNode = it.first(); pNode; pNode = it.next())
				{
					EntityHandle& hHandle = pNode->m_Data;
					CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
					if (pEntity && pEntity->GetType() == enActor)
					{
						CActor* pActor = (CActor*)pEntity;
						pActor->GetGuildSystem()->CheckGuildPlayerPos(gptMove);
					}
				}
			}
		}
		//拒绝
		else if(nType == 2)
		{
			pGuild->m_UiounMsgList.remove(nIndex);

			if (pEntity && pEntity->GetType() == enActor)
			{
				CActor* pActor = (CActor*)pEntity;
				pActor->SendOldTipmsgFormatWithId(tpRefuseUnion,ttFlyTip,m_pGuild->m_sGuildname);
			}
		}
	}
}

VOID CGuildSystem::DeclareWar(unsigned int nObjGuildid)
{
	if (m_pEntity == NULL) return;
	if (m_pGuild == NULL)
	{
		m_pEntity->SendTipmsgFormatWithId(tmNotInGuild, tstUI);
		return;
	}
	int nGuildPos = GetGuildPos();
	//帮派帮主、副帮主
	if (nGuildPos < smGuildElite)
	{
		m_pEntity->SendTipmsgFormatWithId(tmNoRight, tstUI);
		return;
	}

	if (nObjGuildid == m_pGuild->m_nGid ) return;//不能向自己帮派宣战

	CGuild* pObjGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nObjGuildid);
	if (pObjGuild)
	{
		int rs = m_pGuild->GetRelationShipInWar(nObjGuildid);	//获取本人行会与对方行会是否是宣战关系

		//已经宣战
		if (rs == 1)
		{
			m_pEntity->SendTipmsgFormatWithId(tmNotPeaceful, tstUI);
			return;
		}

		int curType = m_pGuild->GetRelationShipForId(nObjGuildid);
		//同盟关系不能宣战
		if(curType == 1)
		{
			m_pEntity->SendOldTipmsgFormatWithId(tmIsUnionDecare, tstUI);
			return;
		}

		//需要的行会资金
		int needCoin = GetLogicServer()->GetDataProvider()->GetGuildConfig().m_GuildRelation.nDeclareNeedCoin;
		if(m_pGuild->GetCoin() < needCoin)
		{
			m_pEntity->SendTipmsgFormatWithId(tmNoMoreGuildCoin, tstUI, needCoin);
			return;
		}

		m_pGuild->ChangeGuildCoin(-needCoin,0, "DeclareWar");

		//加入到列表中 宣战是相互的
		m_pGuild->AddWarGuild(nObjGuildid,pObjGuild);
		pObjGuild->AddWarGuild(m_pGuild->m_nGid, m_pGuild);
		//m_pGuild->AddUnionWarGuild(nObjGuildid,pObjGuild);

		//通知对方成员
		//pObjGuild->SendGuildMsg(smGuildCommon,tpDeclareWarNotice,ttScreenCenter+ttTipmsgWindow,m_pGuild->m_sGuildname);
		//pGuild->NotifyUpdateGuildInfo();

		//m_pEntity->SendTipmsgFormatWithId(tmDeclareSucc, tstUI);
		//通知自己帮派成员
		//m_pGuild->SendGuildMsg(smGuildCommon,tpOnWarNotice,ttScreenCenter+ttTipmsgWindow,pObjGuild->m_sGuildname);

		m_pGuild->UpdateGuildList();
		pObjGuild->UpdateGuildList();

		// UpdateNameClr_OnPkModeChanged( ((CActor*)m_pEntity)->GetHandle() );		//将自己名称颜色发给周围玩家（包括本人）
		// UpdateOtherNameClr_GiveSelf( ((CActor*)m_pEntity)->GetHandle() );		//将周围玩家的名称颜色发给本人
		//如果通知周围玩家，更新他的周围玩家名称颜色，就会导致N*N的数据量
		//NotifyOtherNameClr_GiveThemSelf( ((CActor*)m_pEntity)->GetHandle() );	//通知周围玩家更新他本人的名称颜色

		//添加帮派事件
		char sText[1024];
		LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmDeclareSucc);
		sprintf_s(sText, sizeof(sText), sFormat, ( char *)(pObjGuild->GetGuildName()));
		m_pGuild->AddEventRecord("",enGuildEvent_ToWar,0,0,0,( char *)(pObjGuild->GetGuildName()), "");
		m_pGuild->SendGuildChatMsg(m_pEntity, sText);
		//添加帮派事件
		char sText1[1024];
		LPCTSTR sFormat1 = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmOnWarNotice);
		sprintf_s(sText1, sizeof(sText1), sFormat1, ( char *)(m_pGuild->GetGuildName()));
		pObjGuild->SendGuildChatMsg(m_pEntity, sText1);
		pObjGuild->AddEventRecord("",enGuildEvent_War,0,0,0,( char *)(m_pGuild->GetGuildName()), "");

		m_pGuild->SendWarGuildListToAllMember();
		pObjGuild->SendWarGuildListToAllMember();
		
		//发送敌对帮派的列表
		//m_pGuild->SendWarList(m_pEntity);
		//m_pGuild->NotifyUpdateGuildInfo();
		CActorPacket ap;
		CDataPacket & dataPacket = m_pEntity->AllocPacket(ap);
		dataPacket << (BYTE)enGuildSystemID << (BYTE)sDeclareWar;
		dataPacket <<(unsigned int)nObjGuildid;
		ap.flush();
	}
}

VOID CGuildSystem::GetUpdateGuildInfo()
{
	
}

//行会升级
VOID CGuildSystem::UpgradeGuild()
{
	if (m_pEntity == NULL) return;
	if (m_pGuild == NULL)
	{
		m_pEntity->SendOldTipmsgFormatWithId(tpNotInGuild,ttFlyTip);
		return;
	}

	INT_PTR nMyLevel = m_pGuild->GetLevel();
	if (GetLogicServer()->GetDataProvider()->GetGuildConfig().nMaxLevel <= nMyLevel)
	{
		return;
	}

	int myPos = GetGuildPos();
	if (myPos < smGuildAssistLeader )
	{
		//只能帮主或副帮主才能做这个操作
		m_pEntity->SendOldTipmsgFormatWithId(tpNoRight,ttFlyTip);
		return;
	}
	TriggerEvent(gUpgradeGuild);

	/*
	const GuildLevelConfig* pConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig().GetLevelConfig(nMyLevel);
	if(pConfig)
	{
		int needYb = pConfig->nNeedYb;
		//元宝不够
		if(m_pEntity->GetProperty<int>(PROP_ACTOR_YUANBAO) < needYb)
		{
			m_pEntity->SendOldTipmsgFormatWithId(tpNomoreYubao,ttFlyTip);
			return;
		}

		LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpExpandMan);
		((CActor*)m_pEntity)->ChangeMoney(mtYuanbao,-needYb,118,0,sFormat);

		nMyLevel++;
		m_pGuild->SetLevel((int)nMyLevel,true,true);

		//添加帮派事件
		char sText[1024];
		LPCTSTR strFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpUpguildlevelEvent);

		const GuildLevelConfig* pNewConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig().GetLevelConfig(nMyLevel);
		if(pNewConfig)
		{
			//发送消息给对方
			CActorPacket AP;
			CDataPacket& DataPacket = ((CActor*)m_pEntity)->AllocPacket(AP);
			DataPacket << (BYTE)enGuildSystemID << (BYTE)cUpgrade;
			DataPacket << (int)(pNewConfig->nMaxMember);
			AP.flush();

			sprintf_s(sText, sizeof(sText), strFormat, ( char *)((CActor*)m_pEntity->GetEntityName()),needYb,pNewConfig->nMaxMember);
			m_pGuild->AddEventRecord(sText,tpCreateGuildEvent,needYb,pNewConfig->nMaxMember,0,( char *)((CActor*)m_pEntity->GetEntityName()),NULL);

		}
	}
	*/
}

VOID CGuildSystem::GetZJ()
{

}

VOID CGuildSystem::UpdateMemo(CDataPacketReader &packet)
{
	if (m_pGuild == NULL) return;
	if (m_pEntity == NULL) return;

	// BYTE nMemoType = 0;
	// packet >> nMemoType;

	int nMyPos = GetGuildPos();
	if (nMyPos < smGuildAssistLeader)
	{
		m_pEntity->SendTipmsgFormatWithId(tmNoRight, tstUI);
		return;
	}

	char nMemo[1024];
	memset(nMemo, 0, sizeof(nMemo));
	packet.readString(nMemo, ArrayCount(nMemo));	
	//nMemo[strlen(nMemo) - 1] = 0;
	// if ( nMemoType < 0 || nMemoType >3)
	// 	return;
	GetGlobalLogicEngine()->GetChatMgr().Filter(nMemo);//过滤字符
	// if (nMemoType == 1)
	// {
	// 	m_pGuild->SetInMemo(nMemo,TRUE);
	// }else if(nMemoType == 2)
	// {
	// 	m_pGuild->SetOutMemo(nMemo,TRUE);
	// }
	// else if(nMemoType == 3)
	// {
	// 	m_pGuild->SetGuildGroupMemo(nMemo,TRUE);
	// }
	m_pGuild->SetOutMemo(nMemo,TRUE);
	m_pEntity->SendTipmsgFormatWithId(tmUpdateMemoSucc, tstUI);
	// if(nMemoType < 3)
	// 	m_pEntity->SendTipmsgFormatWithId(tmUpdateMemoSucc, tstUI);

	m_pGuild->NotifyUpdateGuildInfo();
}

VOID CGuildSystem::SetGuildQGroupId(CDataPacketReader &packet)
{
	if (m_pGuild == NULL) return;
	if (m_pEntity == NULL) return;

	int nMyPos = GetGuildPos();
	if (nMyPos < smGuildAssistLeader)
	{
		m_pEntity->SendOldTipmsgFormatWithId(tpNoRight,ttFlyTip);
		return;
	}

	char strQq[32];
	packet.readString(strQq,ArrayCount(strQq));
	strQq[sizeof(strQq)-1] = 0;

	GetGlobalLogicEngine()->GetChatMgr().Filter(strQq);
	m_pGuild->SetQqGroupId(strQq,true);

	m_pEntity->SendOldTipmsgFormatWithId(tpSetGuildQSuc,ttFlyTip);
	m_pGuild->NotifyUpdateGuildInfo();
}

VOID CGuildSystem::SetGuildYGroupId(CDataPacketReader &packet)
{
	if (m_pGuild == NULL) return;
	if (m_pEntity == NULL) return;

	int nMyPos = GetGuildPos();
	if (nMyPos != smGuildAssistLeader && nMyPos != smGuildLeader)
	{
		m_pEntity->SendOldTipmsgFormatWithId(tpNoRight,ttFlyTip);
		return;
	}

	BYTE nType = 0;

	packet >> nType;

	char strYy[32], grilCompere[32];
	packet.readString(strYy,ArrayCount(strYy));
	strYy[sizeof(strYy)-1]=0;
	
	packet.readString(grilCompere, ArrayCount(grilCompere));

	char gMsg[1024];
	packet.readString(gMsg,ArrayCount(gMsg));
	gMsg[sizeof(gMsg)-1]=0;

	GetGlobalLogicEngine()->GetChatMgr().Filter(gMsg);

	m_pGuild->SetYyType(nType);
	m_pGuild->SetYyGroupId(strYy,true);
	m_pGuild->SetYyGridCompere(grilCompere);
	m_pGuild->SetYyMemo(gMsg,true);

	m_pEntity->SendOldTipmsgFormatWithId(tpSetGuildQSuc,ttFlyTip);
	m_pGuild->NotifyUpdateGuildInfo();
}

/*
nTargetPos:0-成员，1-官员，2-副帮主，3-帮主
帮主一旦禅让，就直接变成成员
*/
VOID CGuildSystem::ChangeGuildPos(CDataPacketReader &packet)
{

	if (m_pEntity == NULL) return;
	if (m_pGuild == NULL)
	{
		m_pEntity->SendTipmsgFormatWithId(tmNotInGuild, tstUI);
		return;
	}

	//BYTE nChangeType;//升职还是降职
	//1:升级
	//	2：降级
	unsigned int nActorid;			//目标角色id
	BYTE		nObjNewPos;			//目标的新职位
	//packet >> nChangeType;
	packet >> nActorid;
	packet >> nObjNewPos;			//这里参数就是 0-成员，2-副帮主
	if (nActorid == m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID)) return;
	//目标职位不能是帮主
	if( nObjNewPos < smGuildCommon || nObjNewPos >= smGuildLeader ) return;

	int nMyPos = GetGuildPos();		//本人职位
	if( nMyPos != smGuildLeader)   //首领或者副首领允许
	{
		m_pEntity->SendTipmsgFormatWithId(tmNoRight,tstUI);
		return;
	}
	if(nObjNewPos != smGuildAssistLeader && nObjNewPos != smGuildCommon)
	{
		m_pEntity->SendTipmsgFormatWithId(tmNoRight,tstUI);
		return;
	}
	if(nObjNewPos>=nMyPos)
	{
		m_pEntity->SendTipmsgFormatWithId(tmNoRight,tstUI);
		return;
	}

	if (GetGlobalLogicEngine()->GetGuildMgr().IsGuildSiegeStart())
	{
		m_pEntity->SendTipmsgFormatWithId(tmInSbkWar, tstUI);
		return;
	}
	//查找这个人是否加入帮派
	ActorCommonNode* pNode = m_pGuild->FindMember(nActorid);
	if (!pNode)
	{
		OutputMsg(rmTip,_T("can not find guild member,actorid=%d"),nActorid);
		return;
	}
	int nObjHisPos = 0;		//对方的原帮派地位
	CEntity* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(pNode->hActorHandle);
	CGuildSystem* pGS = NULL;
	if (pEntity && pEntity->GetType() == enActor)
	{
		//在线
		pGS = ((CActor*)pEntity)->GetGuildSystem();
		if (pGS->m_pGuild == NULL || pGS->m_pGuild != m_pGuild)
		{
			//不同帮派，其实也不一定要做这个判断，保险一点
			return;
		}
		nObjHisPos = pGS->GetGuildPos();
	}
	else
	{
		//不在线
		nObjHisPos = pNode->BasicData.nType;
	}

	if (nObjHisPos == nObjNewPos) 
	{
		char* szNewGuildTitle = m_pGuild->GetGuildTitleName(nObjNewPos);
		if( szNewGuildTitle )
		{
			m_pEntity->SendTipmsgFormatWithId(tmGuildObjHasBeenPos, tstUI,szNewGuildTitle);
		}
		return;	//职位跟原来的一样
	}

	if( nObjHisPos>=nMyPos )		//对方现在的职位高于本人职位
	{
		m_pEntity->SendTipmsgFormatWithId(tmNoRight, tstUI);
		return;
	}

	//判断人数，不同的帮派级别，副帮主的人数是不一样的
	if( nObjNewPos == smGuildAssistLeader )		//把某人升/降职到副帮主，要判断人数是否超出了
	{
		if( m_pGuild->GetGuildPosCount(smGuildAssistLeader) >= m_pGuild->MaxGuildPosCount(smGuildAssistLeader) )
		{
			m_pEntity->SendTipmsgFormatWithId(tmGuildPosLimit,tstUI, m_pGuild->GetGuildTitleName(smGuildAssistLeader));
			return;
		}
		m_pGuild->m_nSecLeaderCount++;
	}
	else if( nObjNewPos == smGuildTangzhu )		//把某人升/降职到官员（堂主），要判断人数是否超出了
	{
		if( m_pGuild->GetGuildPosCount(smGuildTangzhu) >= m_pGuild->MaxGuildPosCount(smGuildTangzhu) )
		{
			m_pEntity->SendTipmsgFormatWithId(tmGuildPosLimit, tstUI, m_pGuild->GetGuildTitleName(smGuildTangzhu));
			return;
		}
		m_pGuild->m_GuildTkCount++;
	}

	//原职位数量减一
	if (nObjHisPos == smGuildAssistLeader)
	{
		m_pGuild->m_nSecLeaderCount--;
	}
	else if (nObjHisPos == smGuildTangzhu)
	{
		m_pGuild->m_GuildTkCount--;
	}

	//添加帮派事件
	char sText[1024];
	LPCTSTR sFormat;
	if( nObjHisPos > nObjNewPos )
	{
		// sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tmDownGuildPosEvent);
		pNode->BasicData.nGuildTiTle = 0;
		m_pGuild->SendChangeTitleToAll(nActorid,0);
	}
	// else
		// sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tmUpGuildPosEvent);

	// LPCTSTR gOldName = m_pGuild->GetGuildTitleName(nObjHisPos);		//GetGuildPosName(nObjHisPos);
	// LPCTSTR gNewName = m_pGuild->GetGuildTitleName(nObjNewPos);		//GetGuildPosName(nObjNewPos);
	// sprintf_s(sText, sizeof(sText), sFormat, ( char *)(pNode->sActorName),gOldName,gNewName);

	if( nObjNewPos > smGuildCommon )			//任命官员
	{
		m_pGuild->AddEventRecord( "", enGuildEvent_AddOfficer,  nObjNewPos, 0, 0, (char *)(pNode->sActorName), NULL);
	}
	else	//撤销官员
	{
		m_pGuild->AddEventRecord( "", enGuildEvent_DelOfficer,  nObjHisPos, 0, 0, (char *)(pNode->sActorName), NULL);
	}

	//设置对方的职位
	pNode->BasicData.nType		= nObjNewPos;
	pNode->BasicData.nZzNext	= GetZZNext();

	if(pGS)
	{
		pGS->SetGuildPos(nObjNewPos);
		pGS->m_nTitle = pNode->BasicData.nGuildTiTle;
	}
	else
	{
		//保存到数据库
		Save(m_pGuild->m_nGid,pNode->BasicData.nActorId,pNode->BasicData.nTotalGx,pNode->BasicData.nType << 16,
			pNode->BasicData.nZjNext,pNode->BasicData.nZzNext,pNode->BasicData.nGuildTiTle,pNode->BasicData.nGuildCityPos,
			pNode->BasicData.nTime, pNode->BasicData.nModelId, pNode->BasicData.nJoinTime);
	}
	//给操作的人一个提示
	m_pEntity->SendTipmsgFormatWithId(tmOperSucc, tstUI);
	((CActor*)m_pEntity)->ResetShowName();

	if(pEntity)
		((CActor*)pEntity)->ResetShowName();

	//记录日志
	if(GetLogicServer()->GetLocalClient())
	{
	}
	m_pGuild->AfterDonateDealGuild();
	SendMemberList();		//刷新列表
}

/*
帮主禅让
*/
VOID CGuildSystem::LeaderChange(CDataPacketReader &packet)
{
	if (m_pEntity == NULL) return;
	if (m_pGuild == NULL)
	{
		m_pEntity->SendTipmsgFormatWithId(tmNotInGuild, tstUI);
		return;
	}
	if (GetGuildPos() != smGuildLeader)
	{
		//只能帮主做这个操作
		m_pEntity->SendTipmsgFormatWithId(tmNoRight,tstUI);
		return;
	}
	CGuildComponent& guildMgr = GetGlobalLogicEngine()->GetGuildMgr();

	SYSTEMTIME &sysTime = GetLogicServer()->GetLogicEngine()->getSysTime();
	WORD nWeek = sysTime.wDayOfWeek;
	/*
	if( (GetGlobalLogicEngine()->GetGuildMgr().IsCanCrossSbkGuild(m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_GUILD_ID)) == true) && (nWeek ==3 || nWeek == 4) )
	{
		m_pEntity->SendOldTipmsgFormatWithId(tpIsCrossGuildCanNotChange,ttFlyTip);
		return;
	}

	if( ((CActor*)m_pEntity)->GetCrossSystem().IsCrossSbkCityOwn(m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_GUILD_ID)))
	{
		m_pEntity->SendOldTipmsgFormatWithId(tpIsCrossGuildCanNotChange,ttFlyTip);
		return;
	}
	*/

	unsigned int nActorid = 0;//新帮主
	packet >> nActorid;
	//看看这个新帮主是否已加入这个帮派，是否够20级，是否在职超过72小时
	CEntity* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorid);
	// if (!pEntity || pEntity->GetType() != enActor)
	// {
	// 	m_pEntity->SendOldTipmsgFormatWithId(tpGuildNoOne,ttFlyTip);
	// 	return;
	// }
	if (guildMgr.IsGuildSiegeStart())
	{
		m_pEntity->SendTipmsgFormatWithId(tmInSbkWar, tstUI);
		return;
	}

	int nLevel = 0;
	UINT nzz = 0;
	int nObjHisPos = 0;//对方的帮派地位

	int nMinLevel = GetLogicServer()->GetDataProvider()->GetGuildConfig().nLevelLimit;

	unsigned int nMyId = m_pEntity->GetId();
	ActorCommonNode* pActorNode = m_pGuild->FindMember(nActorid);		//查找是否存在此帮派成员
	if (!pActorNode)
	{
		OutputMsg(rmTip,_T("can not find guild member,actorid=%d"),nActorid);
		return;
	}
	ActorCommonNode* pMyNode = m_pGuild->FindMember(nMyId);		//查找是否存在此帮派成员
	if (!pMyNode)
	{
		OutputMsg(rmTip,_T("can not find guild member,actorid=%d"),nMyId);
		return;
	}
	if(nObjHisPos == 0)
		nObjHisPos = pActorNode->BasicData.nType;
	if(nLevel == 0)
		nLevel = pActorNode->BasicData.nLevel;
	if(nLevel < nMinLevel)
	{
		return;
	}

	guildMgr.OnGuildLeaderChange(m_pGuild, m_pEntity->GetId(),nActorid, glcLeaderChange);


	if(!pEntity)
	{
		Save(m_pGuild->m_nGid,pActorNode->BasicData.nActorId, pActorNode->BasicData.nTotalGx, pActorNode->BasicData.nType << 16,
		pActorNode->BasicData.nZjNext, pActorNode->BasicData.nZzNext, pActorNode->BasicData.nGuildTiTle, pActorNode->BasicData.nGuildCityPos,
		pActorNode->BasicData.nTime, pActorNode->BasicData.nModelId, pActorNode->BasicData.nJoinTime);
	}
	
	//添加帮派事件
	char sText[1024];
	LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmChangeGuildLeaderEvent);
	sprintf_s(sText, sizeof(sText), sFormat, ( char *)(m_pEntity->GetEntityName()),( char *)(pActorNode->sActorName));

	m_pGuild->AddEventRecord( "", enGuildEvent_LeaderChange, 0, 0, 0, (char *)(m_pEntity->GetEntityName()),( char *)(pActorNode->sActorName));
	m_pEntity->SendTipmsgFormatWithId(tmChangeLeaderSucc, tstUI);

	//记录日志
	if(GetLogicServer()->GetLocalClient())
	{
	}
	m_pGuild->AfterDonateDealGuild();
	CActorPacket ap;
	CDataPacket & dataPacket = m_pEntity->AllocPacket(ap);
	dataPacket << (BYTE)enGuildSystemID << (BYTE)sLeaderChange;
	dataPacket <<(BYTE)0;
	ap.flush();
}

//玩家主动离开行会
VOID CGuildSystem::LeftGuild(BYTE bType)
{
	if (m_pEntity == NULL) return;
	if (m_pGuild == NULL)
	{
		m_pEntity->SendTipmsgFormatWithId(tmNotInGuild,tstUI);
		return;
	}
	if (GetGlobalLogicEngine()->GetGuildMgr().IsGuildSiegeStart())
	{
		m_pEntity->SendTipmsgFormatWithId(tmInSbkWar, tstUI);
		return;
	}
	if(m_InGuildFuben == true)
	{
		m_pEntity->SendTipmsgFormatWithId(tmMeInFuben,tstUI);
		return ;
	}

	//帮主不能退出帮派
	if (GetGuildPos() == smGuildLeader)
	{
		m_pEntity->SendTipmsgFormatWithId(tmLeaderCanNotLeft,tstUI);
		return;
	}

	if(bType == 0)
	if(m_InDartNow == true)
	{
		// CActorPacket pack;
		// CDataPacket &data=m_pEntity->AllocPacket(pack); 
		// data << (BYTE)enGuildSystemID << (BYTE)cSendInDart;

		// pack.flush();
		return;
	}

	//if(m_InDartNow == true)
	//	TriggerEvent(gLeftGuild);

	//删除成员
	m_pGuild->DeleteMember(m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID),(CActor*)m_pEntity,NULL,FALSE);
	m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_GUILD_ID , 0);
	m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_GUILD_LEVEL, 0);

	CActorPacket AP;
	CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);

	DataPacket << (BYTE)enGuildSystemID << (BYTE)sLeftGuild;
	DataPacket <<(BYTE)1;
	AP.flush();
}

/*把玩家踢出行会
成员可能不在线
*/
VOID CGuildSystem::DeleteMemberRequest(unsigned int nActorid)
{
	if (m_pEntity == NULL) return;
	int nMyGuildPos = GetGuildPos();
	if (m_pGuild == NULL || nMyGuildPos < smGuildTangzhu )
	{
		//只有帮主才可以进行此操作
		m_pEntity->SendTipmsgFormatWithId(tmNoRight, tstUI);
		return;
	}	
	if (GetGlobalLogicEngine()->GetGuildMgr().IsGuildSiegeStart())
	{
		m_pEntity->SendTipmsgFormatWithId(tmInSbkWar, tstUI);
		return;
	}

	ActorCommonNode* pNode = m_pGuild->FindMember(nActorid);
	if (pNode)
	{
		int nHisGuildPos = pNode->BasicData.nType;
		CEntity* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(pNode->hActorHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			if(((CActor*)pEntity)->GetGuildSystem()->m_InGuildFuben == true)
			{
				((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmGuildInfuben,tstUI);
				return ;
			}

			if(((CActor*)pEntity)->GetGuildSystem()->m_InDartNow == true)
			{
				((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmInGuildDart,tstUI);
				return ;
			}

			//在线
			CGuildSystem* pGS = ((CActor*)pEntity)->GetGuildSystem();
			if (pGS->m_pGuild == NULL || pGS->m_pGuild != m_pGuild)
			{
				//不同帮派，其实也不一定要做这个判断，保险一点
				return;
			}
			nHisGuildPos = pGS->GetGuildPos();
		}
		if ((nMyGuildPos - nHisGuildPos) <= 0)
		{
			m_pEntity->SendTipmsgFormatWithId(tmNoRight,tstUI);
			return;
		}
		//添加帮派事件
		m_pGuild->AddEventRecord( "", enGuildEvent_TickMember, 0, 0, 0, (char *)(m_pEntity->GetEntityName()), (char *)(pNode->sActorName) );

		//可以开除成员了:)
		m_pGuild->DeleteMember(m_pEntity->GetId(),(CActor*)pEntity,pNode,TRUE);

		int nTime = GetGlobalLogicEngine()->getMiniDateTime() +
			GetLogicServer()->GetDataProvider()->GetGuildConfig().nLeftTimeLimit;//设置他的在职时间，在规定时间内不能再加入其他帮派
		//被开除的成员不在线，保存数据
		if(pEntity == NULL)
			Save(0, (unsigned int)nActorid,0,0,0,nTime,0,0,0,0,0);
		else if(pEntity && pEntity->GetType() == enActor)
		{
			((CActor*)pEntity)->ResetShowName();
		}
		
		//刷新职位
		m_pGuild->AfterDonateDealGuild();
		SendMemberList();		//刷新列表
		GetGlobalLogicEngine()->GetGuildMgr().DeleteJoinGuildRelust(nActorid); //删除全局 玩家申请的结果
	}
	else
	{
		OutputMsg(rmTip,_T("can not find guild member,actorid=%u"),nActorid);
	}
}

/*
玩家主动离开行会或者被踢出行会触发
boType：true-被踢出行会，false-主动离开
*/
VOID CGuildSystem::OnLeftGuild(BOOL boType)
{
	if( m_pEntity == NULL ) return;
	if( m_pGuild == NULL ) return;

	boSaveed = TRUE;	//最后的数据保存,包括贡献值和nZZNext都要保存

	m_pGuild = NULL;

	//真正离开行会之后
	if (m_pEntity)
	{
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_GUILD_ID,0);
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_GUILD_LEVEL,0);
		SetGuildPos(smGuildCommon);
		if (!boType)
		{
			//自己脱离了帮派
			int nLeftTime = time(NULL) +
				GetLogicServer()->GetDataProvider()->GetGuildConfig().nLeftTimeLimit;//设置他的在职时间，在规定时间内不能再加入其他帮派
			m_pEntity->GetStaticCountSystem().SetStaticCount(NLEFT_GUILDTIME, nLeftTime);
		}
		ClearZJ();//清理召集令，如果他不在线，那等下次上线的时候清理
		//清除所有的帮派任务
		//m_pEntity->GetQuestSystem()->ClearGuildQuest();

		//要求离开行会（主动离开或者被T），行会贡献度都清0
		//m_pEntity->SetProperty<int>(PROP_ACTOR_GUILDEXP,0);
		m_pEntity->SendTipmsgFormatWithId(tmGuildFlftAndClearGx, tstUI);

		//GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction((CActor*)m_pEntity,"OnGuildSiegeMasterStatueCheckBuff");
		GetGlobalLogicEngine()->GetGuildMgr().DeleteJoinGuildRelust(m_pEntity->GetId()); //删除全局 玩家申请的结果
	}
}

void CGuildSystem::TakeOnOffDragonRobe(BYTE nType)
{
	// if (m_pEntity == NULL) return;
	// CGuildProvider& guildProvider = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	// if(nType)//穿上龙袍
	// {
	// 	if (m_pEntity->GetBuffSystem()->Exists(guildProvider.nDragonRoleBuffId))
	// 	{
	// 		return;
	// 	}
	// 	if (m_pEntity->GetId() == GetGlobalLogicEngine()->GetGuildMgr().GetCityOwnMasterId())
	// 	{
	// 		m_pEntity->GetBuffSystem()->Append(guildProvider.nDragonRoleBuffId);
	// 	}
	// }
	// else
	// {
	// 	if (m_pEntity->GetBuffSystem()->Exists(guildProvider.nDragonRoleBuffId))
	// 	{
	// 		m_pEntity->GetBuffSystem()->RemoveById(guildProvider.nDragonRoleBuffId);
	// 	}
	// }
}
void CGuildSystem::SendGuildCityPosInfo()
{
	
}

/*
客户端处理（10，2）消息时，
如果数量为1，表示是本人信息更新，则不需要清空成员列表，只需要更新本人那一列的信息，其他成员信息照旧显示
如果数量不为1，则先清空旧的列表
*/
VOID CGuildSystem::SendMemberList(unsigned int nActorId)
{
	if (m_pEntity == NULL) return;
	if (m_pGuild == NULL) return;
	CMiniDateTime nCurrentTime;
	nCurrentTime.tv = GetGlobalLogicEngine()->getMiniDateTime();

	CActorPacket AP;
	CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);
	DataPacket << (BYTE)enGuildSystemID << (BYTE)sGuildMember;
	
	size_t pos = DataPacket.getPosition();
	DataPacket << (int)0;//未必是真实的数量
	int nResult = 0;
	
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	int nSize = m_pGuild->m_ActorOffLine.size();
	for(int i = 0; i < nSize; i++)
	{
		
		ActorCommonNode& actor = m_pGuild->m_ActorOffLine[i];
		if( nActorId > 0 && nActorId != actor.BasicData.nActorId )		//指定某个成员的数据
		{
			continue;
		}

		CEntity* pOnlineEntity = NULL;
		if (actor.hActorHandle != 0)
		{
			pOnlineEntity = pEntityMgr->GetEntity(actor.hActorHandle);
		}
		if (pOnlineEntity)
		{
			CActor* pOnlineActor = (CActor*)pOnlineEntity;
			DataPacket << pOnlineEntity->GetProperty<unsigned int>(PROP_ENTITY_ID);
			DataPacket.writeString(pOnlineActor->GetEntityName());
			DataPacket << (int)(pOnlineEntity->GetProperty<int>(PROP_CREATURE_LEVEL));
			DataPacket << (BYTE)(pOnlineEntity->GetProperty<int>(PROP_ACTOR_CIRCLE));
			DataPacket << (BYTE)(pOnlineEntity->GetProperty<int>(PROP_ACTOR_VOCATION));
			DataPacket << (BYTE)(pOnlineEntity->GetProperty<int>(PROP_ACTOR_SEX));
			int nPos = pOnlineActor->GetGuildSystem()->GetGuildPos();
			DataPacket << (BYTE)nPos;
			DataPacket << (int)(pOnlineActor->GetGuildSystem()->m_GuildGx);
			DataPacket << (BYTE)1;
			DataPacket << (unsigned int)(pOnlineActor->GetLoginTime());
			DataPacket << (unsigned int)(pOnlineEntity->GetProperty<int>(PROP_ACTOR_SUPPER_PLAY_LVL));
		}
		else
		{
			DataPacket << (unsigned int)actor.BasicData.nActorId;
			DataPacket.writeString(actor.sActorName);
			std::string st = actor.sActorName;
			DataPacket << (int)actor.BasicData.nLevel;
			DataPacket << (BYTE)actor.BasicData.nCircle;
			DataPacket << (BYTE)actor.BasicData.nJob;
			DataPacket << (BYTE)actor.BasicData.nSex;

			DataPacket << (BYTE)actor.BasicData.nType;
			DataPacket << (int)(actor.BasicData.nGuildGX);
			DataPacket << (BYTE)0;
			int nOnlineTime =nCurrentTime.tv - actor.BasicData.nTime;//在线时间
			if(nOnlineTime <0)
			{
				nOnlineTime =0;
			}
			DataPacket << (unsigned int)nOnlineTime;
			DataPacket << (unsigned int)(actor.BasicData.nVipGrade);
		}
		nResult++;
	}
	int* pCount = (int*)DataPacket.getPositionPtr(pos);
	*pCount = nResult;
	AP.flush();
}

VOID CGuildSystem::SendGuildCard(CDataPacketReader &packet)
{
}

void CGuildSystem::SendGuildCardByRank(int nRank)
{
}


VOID CGuildSystem::SendAllGuildInfo(CDataPacketReader &packet)
{
	if (m_pEntity == NULL) return;
	CActorPacket AP;
	CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);
	unsigned int nActorid = m_pEntity->GetId();
	DataPacket << (BYTE)enGuildSystemID << (BYTE)sGuildList;

	CGuildComponent& mgr = GetGlobalLogicEngine()->GetGuildMgr();
	size_t pos = DataPacket.getPosition();

	int total = 0;
	DataPacket << total;	//总数

	int nNow = GetGlobalLogicEngine()->getMiniDateTime();

	CLinkedNode<CGuild*> *pNode;
	CLinkedListIterator<CGuild*> it(*mgr.m_pRank);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CGuild* pAllGuild = pNode->m_Data;
		if (pAllGuild && pAllGuild->m_boStatus) //&& ( (m_pGuild && (nType == m_pGuild->GetRelationShipForId(pAllGuild->m_nGid))) || (m_pGuild && (nType == 3 && m_pGuild->GetRelationShipForId(pAllGuild->m_nGid) == 0)) || nType == 0))//已经加入帮派的，要能查看全部，没有加入的只能看本阵营的
		{
		
			DataPacket.writeString(pAllGuild->m_sGuildname);
			DataPacket <<(unsigned int)pAllGuild->m_nGid;
			pAllGuild->m_nRank = total + 1;
			DataPacket << (int)pAllGuild->m_nRank;
			char* sLeaderName = pAllGuild->FindGuildMemberName(pAllGuild->m_nLeaderId);
			printf("SendAllGuildInfo %d, name :%s \n",pAllGuild->m_nGid, sLeaderName);
			if(sLeaderName)
				DataPacket.writeString(sLeaderName);
			else
				DataPacket.writeString("");
			
			DataPacket << (int)pAllGuild->m_MainHallLev;
			DataPacket << (int)pAllGuild->GetMemberCount();
			DataPacket << (int)pAllGuild->GetMaxMemberCount();
			DataPacket << (BYTE)pAllGuild->m_AddMemberFlag;
			DataPacket << (int)pAllGuild->m_nAutoJoinLevel;
			int nTime = 0;
			if(m_pGuild)
			{
				if(m_pGuild->GetRelationShipInWar(pAllGuild->m_nGid) == 0 && m_pGuild->GetRelationShipForId(pAllGuild->m_nGid) == 2)
				{
					m_pGuild->SetRelationShipWithId(pAllGuild->m_nGid,0);
					pAllGuild->SetRelationShipWithId(m_pGuild->m_nGid,0);
				}
				else if(m_pGuild->GetRelationShipInWar(pAllGuild->m_nGid) == 1)
				{
					m_pGuild->SetRelationShipWithId(pAllGuild->m_nGid,2);
					pAllGuild->SetRelationShipWithId(m_pGuild->m_nGid,2);
				}
				DataPacket << (BYTE)m_pGuild->GetRelationShipInWar(pAllGuild->m_nGid);		//行会宣战关系
			}
			else
			{
				DataPacket << (BYTE)0;
			}
			DataPacket << (bool)(pAllGuild->CheckIsApply(nActorid));
			total++;
		}
	}
	int* pCount = (int*)DataPacket.getPositionPtr(pos);
	*pCount = total;


	AP.flush();
}

void CGuildSystem::BroadCast(char * pData, SIZE_T size)
{
	if (m_pEntity == NULL) return;

	unsigned int nGuildId = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_GUILD_ID);
	if(GetLogicServer()->IsCommonServer())
	{
		CVector<void *> actorList;
		GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineAcotrPtrList(actorList);
		for (INT_PTR i = 0; i < actorList.count(); i++)
		{
			CActor *tmpActor = (CActor *)actorList[i];
			if(tmpActor)
			{ 
				if (tmpActor->OnGetIsTestSimulator()
					|| tmpActor->OnGetIsSimulator() )
				{ 
					continue;
				}

				if(nGuildId == tmpActor->GetProperty<unsigned int>(PROP_ACTOR_GUILD_ID))
				{
					CActorPacket pack;
					CDataPacket &data = tmpActor->AllocPacket(pack);
					data.writeBuf(pData,size);
					pack.flush();
				}
			}
		}
		return;
	}

	if (m_pGuild == NULL)
	{
		m_pEntity->SendTipmsgFormatWithId(tmNotInGuild, tstUI);
		return;
	}
	m_pGuild->BroadCast(pData,size);
}

int CGuildSystem::GetGx()
{
	return m_pEntity?m_pEntity->GetProperty<int>(PROP_ACTOR_GUILDEXP):0;
}

VOID CGuildSystem::SetGx(int nValue)
{
	if (nValue < 0)
	{
		nValue = 0;
	}

	int gGx = (int)GetGx();

	//增加帮派贡献
	if(gGx<nValue)
		AddTotalGuildGx(nValue-gGx);

	if (m_pEntity)
	{
		m_pEntity->SetProperty<int>(PROP_ACTOR_GUILDEXP,nValue);
	}
}

/*
下发行会详细信息
*/
VOID CGuildSystem::SendGuildInfoResult()
{
	if (m_pEntity == NULL) return;
	CActorPacket AP;
	CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);

	DataPacket << (BYTE)enGuildSystemID << (BYTE)sGuildInfo;

	
	if ( GetGlobalLogicEngine()->GetGuildMgr().IsGuildClose() )
	{
		DataPacket << (CHAR)2;
	} else {
		if (!m_pGuild)
		{
			DataPacket << (CHAR)0;
		}else
		{
			DataPacket << (CHAR)1;
			DataPacket<<(unsigned int)(m_pGuild->m_nGid);
			DataPacket.writeString(m_pGuild->m_sGuildname);
			std::string sLeaderName = "";
			char* pName = m_pGuild->FindGuildMemberName(m_pGuild->m_nLeaderId);
			if(pName)
				sLeaderName = pName;
			
			DataPacket.writeString(sLeaderName.c_str());
			DataPacket.writeString(m_pGuild->GetOutMemo());
			DataPacket << (WORD) m_pGuild->GetLevel();		//行会主殿的等级就是行会等级
			DataPacket << (int)(m_pGuild->GetMemberCount());
			DataPacket <<(int)(m_pGuild->GetMaxMemberCount());
			DataPacket << (int)(m_pGuild->GetCoin());		//行会资金
			DataPacket << (int)GetTotalGuildGx();
			DataPacket << (BYTE)GetGuildPos();
			DataPacket << (BYTE)m_pGuild->m_AddMemberFlag;
			int nAutoJoinLevel = m_pGuild->GetAutoJoinLevel();
			DataPacket << (WORD)nAutoJoinLevel;
			
		}
	}
	AP.flush();

	if ( GetGlobalLogicEngine()->GetGuildMgr().IsGuildClose() )
	{
		m_pEntity->SendTipmsgWithId(tmGuildFunctionNotOpen,tstUI);
	}
}

/*
下发行会简单信息
*/
VOID CGuildSystem::SendGuildInfoSimple()
{

}

int CGuildSystem::GetGuildPos()
{
	if (m_pEntity == NULL) return smGuildCommon;

	if (m_pEntity->GetSocialMask(smGuildLeader))
	{
		return smGuildLeader;
	}
	else if (m_pEntity->GetSocialMask(smGuildAssistLeader))
	{
		return smGuildAssistLeader;
	}else if (m_pEntity->GetSocialMask(smGuildTangzhu))
	{
		return smGuildTangzhu;

	}else if (m_pEntity->GetSocialMask(smGuildElite))
	{
		return smGuildElite;
	}
	return smGuildCommon;
}

VOID CGuildSystem::SetGuildPos(INT_PTR nPos)
{
	if (m_pEntity == NULL) return;
	//nPos设置为true，其他设置为0
	for (int i = smGuildCommon; i <= smGuildLeader; i++)
	{
		if (i == nPos)
		{
			m_pEntity->SetSocialMask(nPos,TRUE);
			nZzNext = GetZZNext();
		}
		else m_pEntity->SetSocialMask(i,FALSE);
	}

}

void CGuildSystem::Save(PACTORDBDATA pData)
{
	if (m_pEntity == NULL ) return;
	if (m_pGuild)
	{
		ActorCommonNode* pNode = m_pGuild->FindMember(m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));
		if (pNode)
		{
			pNode->BasicData.nTime = m_pEntity->GetLoginTime();		//最后的登陆时间
			pNode->BasicData.nTotalGx = m_GuildGx;
			pNode->BasicData.nIcon = m_pEntity->GetProperty<int>(PROP_ENTITY_ICON);
			pNode->BasicData.nGuildTiTle = m_nTitle;
			pNode->BasicData.nGuildCityPos = 0;
			pNode->BasicData.nGuildGX = m_pEntity->GetProperty<int>(PROP_ACTOR_GUILDEXP);
			//pNode->BasicData.nTodayGx = m_TodayGx;
		}
	}

	unsigned int nLoginTime = m_pEntity->GetLoginTime();
	int nModleId = m_pEntity->GetProperty<int>(PROP_ENTITY_MODELID);
	Save((unsigned int)(m_pGuild?m_pGuild->m_nGid:0),m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID),m_GuildGx,GetGuildPos() << 16,
		nWealNext,nZzNext, m_nTitle, GetGuildCityPos(), nLoginTime, nModleId,m_nJoinTime);
	pData->nGuildID = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_GUILD_ID);
	pData->nSocialMask = m_pEntity->GetProperty<int>(PROP_ACTOR_SOCIALMASK);
	boSaveed = FALSE;

	((CActor*)m_pEntity)->SetStaticVarValue(szDailyDonateCoin, m_nTodayDonateCoin);
	((CActor*)m_pEntity)->SetStaticVarValue(szDailyDonateItem, m_nTodayDonateItem);
	((CActor*)m_pEntity)->SetStaticVarValue(szTodayChargeTreeNum, m_nTodayChargeTreeNum);
	((CActor*)m_pEntity)->SetStaticVarValue(szTodayPickFruitNum, m_nTodayPickFruitNum);
}

VOID CGuildSystem::Save(unsigned int SaveGid,unsigned int SaveActorid,int SaveGx,int typetk,UINT SaveZJ,UINT SaveZZ,
	int nTitle,int nPosType, unsigned int nLoginTine, int nModleId, unsigned int nJoinTime)
{
	CDataPacket& DataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveActorGuild);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << m_pEntity->GetRawServerIndex() << nServerId;
	DataPacket << (unsigned int)SaveGid;//没加入门派，或者刚给开除了，gid设置0
	DataPacket << (unsigned int)SaveActorid ;
	DataPacket << (int)SaveGx;
	DataPacket << (int)typetk;
	DataPacket << (UINT)SaveZJ;
	DataPacket << (UINT)SaveZZ;
	DataPacket << (int)nTitle;
	DataPacket << (int)nLoginTine;		//(int)m_pEntity->GetLoginTime();
	DataPacket << (int)nPosType;
	DataPacket << (int)nModleId;		//(int)m_pEntity->GetProperty<int>(PROP_ENTITY_MODELID);

	DataPacket << (int)m_TodayGx;
	DataPacket << (int)m_HasShxTimes;
	DataPacket << (int)m_AddShxTimes;

	DataPacket << (int)m_ExploreTimes;
	DataPacket << (int)m_ExploreItemId;
	DataPacket << (int)m_ChallengeTimes;

	DataPacket << (int)m_AddChallengeTimes;

	DataPacket << (unsigned int)nJoinTime;		//加入行会的时间

	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
}

void CGuildSystem::LoadActorBuildingData()
{
	CDataPacket& DataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadActorBuildingData);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << m_pEntity->GetRawServerIndex() << nServerId;
	DataPacket << (int)(m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));

	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
}

VOID CGuildSystem::ClearZJ()
{
	if (!m_pEntity || !m_pGuild) return;
	//CUserItemContainer::ItemOPParam itemPara;
	//itemPara.wItemId = GetLogicServer()->GetDataProvider()->GetGuildConfig().nZJItemId;
	//itemPara.btCount = 255;//设置一个大的数字，不用先看背包有多少这个物品
	//itemPara.btStrong = -1;
	//itemPara.btQuality = -1;
	//m_pEntity->GetBagSystem().DeleteItem(itemPara,m_pGuild->m_sGuildname,GameLog::clGuildClear);
	nWealNext = 0;//表示已清除，随时可以领取新的


}

BOOL CGuildSystem::TimeOK(UINT nTime)
{
	if (nTime < GetGlobalLogicEngine()->getMiniDateTime()) return TRUE;
	return FALSE;
}

 UINT CGuildSystem::GetZZNext()
 {
	return GetGlobalLogicEngine()->getMiniDateTime(); //+  GetLogicServer()->GetDataProvider()->GetGuildConfig().nLeaderTimeLimit;
 }

/*
提示客户端重新请求一遍
*/
 void CGuildSystem::NotifyUpdateGuildInfo()
 {

 }

 void CGuildSystem::SendUpLevelInfo()
 {
	 
 }

 int CGuildSystem::GetGuildSkillLevel(int skillId)
 {
	 if ((!m_pEntity) || (!m_pGuild)) return 0;

	 for (INT_PTR i =0 ; i< m_pGuild->m_GuildSkillList.count();i ++)
	 {
		 if(skillId == m_pGuild->m_GuildSkillList[i].nSkillId)
			 return m_pGuild->m_GuildSkillList[i].nLevel;
	 }

	 return 0;
 }

 void CGuildSystem::SetGuildSkillLevel(int skillId,int skillLevel)
 {
	 if ((!m_pEntity) || (!m_pGuild)) return;

	 for (INT_PTR i =0 ; i< m_pGuild->m_GuildSkillList.count();i ++)
	 {
		 if(skillId == m_pGuild->m_GuildSkillList[i].nSkillId)
		 {
			 m_pGuild->m_GuildSkillList[i].nLevel=skillLevel;
			 break;
		 }
	 }

 }

 bool CGuildSystem::CompareGuildSkillLevel(int skillId,int skillLevel)
 {
	 if ((!m_pEntity) || (!m_pGuild)) return false;

	 for (INT_PTR i =0 ; i< m_pGuild->m_GuildSkillList.count();i ++)
	 {
		 if(skillId == m_pGuild->m_GuildSkillList[i].nSkillId)
		 {
			 if(m_pGuild->m_GuildSkillList[i].nLevel >= skillLevel)
				 return true;
			 else
				 return false;
		 }
	 }

	 return false;
 }

 void CGuildSystem::GetDonateReward()
 {

 }


 void CGuildSystem::GuildDonate(BYTE nType)
 {
	if ((!m_pEntity) || (!m_pGuild)) return;
	
	CGuildProvider& guildCfg = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	DonateCfg* pDonateCfg = guildCfg.GetGuildDonateCfg(nType);
	if(pDonateCfg == NULL)
	{
		m_pEntity->SendTipmsgFormatWithId(tmDataError, tstUI);
		return;
	}
	CActor* pActor = (CActor*)m_pEntity;
	int nValue = m_pEntity->GetStaticCountSystem().GetStaticCount(pDonateCfg->nStaticCountType);
	if(nValue >= pDonateCfg->nLimitTimes)
	{
		m_pEntity->SendTipmsgFormatWithId(tmGuildDonateLimitTimes, tstUI);
		return;
	}
	nType =m_pEntity->GetMoneyTypeByAward(nType);
	int nCoin= m_pEntity->GetMoneyCount((eMoneyType)nType);
	
	if( nCoin < pDonateCfg->nCost)
	{
		if(nType == mtBindCoin)
			m_pEntity->SendTipmsgFormatWithId(tmNoMoreBindCoin, tstUI);
		else if(nType == mtBindYuanbao)
			m_pEntity->SendTipmsgFormatWithId(tmNomoreBindYubao, tstUI);
		return;
	}
	char sText[1024];
	LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmGuildEventCoin);
	sprintf_s(sText, sizeof(sText), sFormat, ( char *)(m_pEntity->GetEntityName()),(int)pDonateCfg->nCost);
	pActor->ChangeMoney(nType, -pDonateCfg->nCost, GameLog::Log_GuildDonate, 0,sText, true);
	if(nType == mtBindCoin)
	{
		m_pGuild->AddEventRecord( "", enGuildEvent_DonateCoin, pDonateCfg->nCost, 0, 0, ( char *)(m_pEntity->GetEntityName()),NULL );
	}else
	{
		m_pGuild->AddEventRecord( "", enGuildEvent_DonateYB, pDonateCfg->nCost, 0, 0, ( char *)(m_pEntity->GetEntityName()),NULL );
	}
	pActor->GetAchieveSystem().ActorAchievementComplete(nAchieveGuildDonate, 1, nType);
	ChangeGuildGx(pDonateCfg->nAddDonate,GameLog::Log_GuildDonate);
	m_pGuild->ChangeGuildCoin(pDonateCfg->nAddCoin);
	m_pEntity->GetStaticCountSystem().SetStaticCount(pDonateCfg->nStaticCountType, nValue+1);
	m_pGuild->m_boUpdateTime = true;
	
	// m_pEntity->GetAchieveSystem().ActorAchievementComplete(nAchieveGetDonateValue, pDonateCfg->nAddDonate);
	// m_pGuild->AddEventRecord(sText,tmGuildEventCoin,(int)pDonateCfg->nAddCoin,0,0,( char *)(m_pEntity->GetEntityName()), "");
	m_pGuild->AfterDonateDealGuild();
	CActorPacket ap;
	CDataPacket& dataPack = m_pEntity->AllocPacket(ap);
	dataPack <<(BYTE)enGuildSystemID << (BYTE)sGuildDonate;

	ap.flush();
 }

 /*
 VOID CGuildSystem::AddGuildGx(int nValue) 
 { 
	 int nGX = (int)GetGx();
	 SetGx(nGX+ nValue);

	 if(nValue>0)
		 ((CActor*)m_pEntity)->SendOldTipmsgFormatWithId(tpGetGuildGx,ttTipmsgWindow, nValue);
	 else if(nValue<0)
		 ((CActor*)m_pEntity)->SendOldTipmsgFormatWithId(tpDelGuildGx,ttTipmsgWindow, -1*nValue);
 }
 */


 void CGuildSystem::ChangeGuildFr(int value)
 {
	//  if(m_pGuild==NULL) return;
	//  int gFr = m_pGuild->GetFr();
	//  m_pGuild->SetFr(gFr+value,TRUE,true);

	//  if(value > 0)
	// 	 ((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmAddGuildFr, tstUI, value);
 }

 void CGuildSystem::SendGuildEventRecord(int nPage,int nNum)
 {

 }

 //不支持不同的帮派不同的职务Name
 char* CGuildSystem::GetGuildPosName(int posType)
 {
	 switch(posType)
	 {
	 case 0:
		 return GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildMember);
	 case 1:
		 return GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildTangZhu);
	 case 2:
		 return GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildSecHead);
	 case 3:
		 return GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildHead);
	 default:
		 break;
	 }

	 return NULL;
 }

 bool CGuildSystem::NaiveStringMatching(char* srcStr,char *destStr) 
 { 
	 int n = (int)strlen(srcStr); 
	 int m = (int)strlen(destStr); 

	 if(n <m || m==0) 
	 { 
		 return false; 
	 } 

	 bool find=true; 
	 int i = 0;

	 for(int j=0;j<=n-m;j++) 
	 { 
		 find=true; 
		 for(i=0;i<m;i++) 
		 { 
			 if(srcStr[j+i]!=destStr[i]) 
			 { 
				 find=false; 
				 break; 
			 } 
		 } 

		 if(i==m)
		 {
			 find = true;
			 break;
		 }
	 } 

	 return find;
 } 

 void CGuildSystem::GuildGroupChat(CDataPacketReader &packet)
 {
	
 }

 void CGuildSystem::SendWelcomeToNewMember(char* nMsg)
 {
	 
 }

 void CGuildSystem::SendSearchGuildResult(CDataPacketReader &packet)
 {
 }

 void CGuildSystem::SendDeleteGuildResult(int bType)
 {
	CActorPacket pack;
	CDataPacket &data=m_pEntity->AllocPacket(pack);

	data << (BYTE)enGuildSystemID << (BYTE)cDelGuild;

	// int nIndex = GetScoreRank(actorId);

	// if(bType<0 || bType>1)
	// bType = 0;

	data << (BYTE)0;

	pack.flush();
 }

 void CGuildSystem::AddTotalGuildGx(int value)
 {
	 if(value > 0)
	 {
		 m_GuildGx += value;
	 }
 }

 void CGuildSystem::SendGuildMemberAddOrLieve(CActor* pActor,BYTE bType,unsigned int nActorId,char* nName)
 {
	
 }

 /*
 申请兑换行会仓库
 低于指定转数+等级的装备直接兑换，高于的需要审核
 */
VOID CGuildSystem::GetItemFromGuildDepot(CDataPacketReader &packet)
{
	// if (m_pEntity == NULL) return;
	// if (m_pGuild == NULL) return;

	// CUserItem::ItemSeries series ;
	// BYTE nOpType=0;
	// packet >> series >> nOpType;

	// if( nOpType == 2 )		//丢弃操作
	// {
	// 	if( GetGuildPos() < smGuildTangzhu )
	// 	{
	// 		//官员以上才有权限操作
	// 		m_pEntity->SendOldTipmsgFormatWithId(tpNoRight,ttFlyTip);
	// 		return;
	// 	}
	// }

	// INT_PTR nIdx = m_pGuild->FindItemIndex(series);
	// if( nIdx < 0 )
	// {
	// 	m_pEntity->SendOldTipmsgWithId(tpItemNotExist);
	// 	return ;
	// }

	// int nType = 2;
	// if(m_pGuild->m_GuildDepotBag[nIdx].nActorId == m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID))
	// 	nType = 3;


	// CUserBag& bag= m_pEntity->GetBagSystem();
	// GUILDDEPOTITEM &stDepotItem = m_pGuild->m_GuildDepotBag[nIdx];			//仓库物品结构
	// CUserItem* pUserItem = stDepotItem.nUserItem;
	// if( pUserItem == NULL )
	// {
	// 	return;
	// }
	// const CStdItem * pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
	// if( pStdItem ==NULL )
	// {
	// 	return;
	// }
	// //if(pUserItem ==NULL) return;
	// if( nOpType == 2 )		//丢弃操作
	// {
	// 	if( stDepotItem.nExchangeActorId > 0 )
	// 	{
	// 		m_pEntity->SendOldTipmsgFormatWithId(tpGuildDepotItemCheckingNoClear, ttFlyTip);
	// 		return;
	// 	}
	// 	m_pGuild->m_GuildDepotBag.remove(nIdx);		//仓库删除此物品
	// 	m_pGuild->NotifyUpdateDepot();				//通知客户端重新请求一次
	// 	m_pGuild->m_bSaveDepotRecord = true;

	// 	char chatMsg[1024]={0};
	// 	LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildDepotDelItem);
	// 	sprintf_s(chatMsg, sizeof(chatMsg), sFormat, 
	// 		(char *)m_pGuild->GetGuildTitleName( GetGuildPos() ), 
	// 		(char *)( m_pEntity->GetEntityName() ), 
	// 		CUserBag::GetItemLink( (int)pUserItem->wItemId, pUserItem) );
	// 	m_pGuild->BroadCastMsgInGuildChannel(chatMsg);
	// 	return;
	// }
	// else
	// {
	// 	if( stDepotItem.nExchangeActorId > 0 )
	// 	{
	// 		m_pEntity->SendOldTipmsgFormatWithId(tpGuildDepotItemCheckingNoExchange, ttFlyTip);
	// 		return;
	// 	}

	// 	int nItemCircle = (int)pStdItem->GetUseConditionValue(CStdItem::ItemUseCondition::ucMinCircle);	//转数
	// 	int nItemLevel  = (int)pStdItem->GetUseConditionValue(CStdItem::ItemUseCondition::ucLevel);		//等级
	// 	CGuildProvider& pGuildConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	// 	int nNeedGx = pGuildConfig.GetGuildDepotGetOutNeedGx(nItemCircle, nItemLevel, pStdItem->m_btType);
	// 	if( nNeedGx < 0 )   //配置错了
	// 	{
	// 		return;
	// 	}
	// 	if( GetGx() < nNeedGx )
	// 	{
	// 		m_pEntity->SendOldTipmsgFormatWithId(tpGuildDepotGetOutGxUnEnough , ttFlyTip, nNeedGx);    //行会贡献不足
	// 		return;
	// 	}

	// 	if( GetGuildPos() <= smGuildTangzhu && nItemCircle >= m_pGuild->m_nExchangeCheckCircleMin )	//成员或官员，并且需要审核
	// 	{
	// 		ChangeGuildGx( -1*nNeedGx );		//扣除本人的行会贡献

	// 		//设置兑换玩家的信息
	// 		stDepotItem.nExchangeActorId = m_pEntity->GetId();
	// 		_asncpytA(stDepotItem.szExchangeActorName, m_pEntity->GetEntityName());	
	// 		stDepotItem.nExchangeTime	= GetGlobalLogicEngine()->getMiniDateTime();

	// 		m_pEntity->SendOldTipmsgWithId(tpGuildDepotItemDoExchange);
	// 		m_pGuild->m_bSaveDepotRecord = true;

    //         //向会长，副会长发送审核图标
	// 		CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	// 		CLinkedNode<ActorCommonNode> *pNode;
	// 		CLinkedListIterator<ActorCommonNode> it(m_pGuild->m_ActorOffLine);
	// 		for( pNode = it.first(); pNode; pNode = it.next() )
	// 		{
	// 			ActorCommonNode& actor = pNode->m_Data;
	// 			CEntity* pEntity = NULL;
	// 			if( actor.hActorHandle != 0 )
	// 			{
	// 				pEntity = pEntityMgr->GetEntity(actor.hActorHandle);
	// 			}
	// 			if( pEntity && pEntity->GetType() == enActor) //玩家在线
	// 			{
	// 				 int  nPos = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPos();
	// 				 if(nPos ==  smGuildLeader ||  nPos == smGuildAssistLeader)
	// 				 {
	// 				       CActorPacket ap;
	// 					   CDataPacket& pack = ((CActor*)pEntity)->AllocPacket(ap);
	// 					   pack << (BYTE) enGuildSystemID<< (BYTE)sGuildCheckEquipIcon ;
	// 					   ap.flush();
	// 				 }
	// 			}
	// 		}

	// 	}
	// 	else		//直接兑换
	// 	{
	// 		if(! bag.CanAddItem(pUserItem) )
	// 		{
	// 			m_pEntity->SendOldTipmsgWithId(tpBagIsFull);			//背包已经满
	// 			return;
	// 		}

	// 		ChangeGuildGx( -1*nNeedGx );					//扣除本人的行会贡献
	// 		m_pGuild->m_GuildDepotBag.remove(nIdx);			//必须在bag.AddItem()之前
	// 		if( bag.AddItem(pUserItem,"guilddeport_2_bag",GameLog::clGuildDepotToBag,true) )
	// 		{
	// 			//m_pGuild-> m_GuildDepotBag[nIdx].nExitFlag = 1;		//被取出了
	// 			m_pGuild->m_bSaveDepotRecord = true;
	// 			m_pEntity->SendOldTipmsgFormatWithId(tpGuildDepotGetOutItemDelGx , ttFlyTip, nNeedGx);    //行会贡献不足
	// 			char chatMsg[1024]={0};
	// 			LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildDepotGetOutItem);
	// 			sprintf_s(chatMsg, sizeof(chatMsg), sFormat, ( char *)(m_pEntity->GetEntityName()), 
	// 				CUserBag::GetItemLink((int)pUserItem->wItemId, pUserItem), nNeedGx);
	// 			m_pGuild->BroadCastMsgInGuildChannel(chatMsg);
	// 		}
	// 	}
	// 	m_pGuild->NotifyUpdateDepot();				//通知客户端重新请求一次
	// }
}

//获取行会仓库物品和转数的数量
VOID CGuildSystem::GetDepotItemCircleAndNum()
{
	
}


 //获取行会仓库物品列表
 VOID CGuildSystem::GetDepotItemList(CDataPacketReader &packet)
 {
	 if (m_pEntity == NULL) return;
	 if (m_pGuild == NULL) return;

	 BYTE nPage		= 0;	//仓库第N页，从1-N
	 BYTE nJobCond  = 0;	//按职业搜索，0-无要求，1-战士、2-法师、3-巫师
	 BYTE nGxCond   = 0;	//按行会贡献搜索，0-否，1是
	 BYTE nCircle	= 0;	//转数
	 BYTE nNeedCheck= 0;	//是否需要审核，0-全部，1-需要审核，2-不需要审核
	 packet >> nPage >> nJobCond >> nGxCond >> nCircle >> nNeedCheck;
	 if( nJobCond == 0 && nGxCond == 0 && nCircle == 0 && nNeedCheck == 0 )		//无条件查询
	 {
		 GetDepotItemListNoCond( nPage );
	 }
	 else
	 {
		 GetDepotItemListCond( nPage, nJobCond, nGxCond, nCircle, nNeedCheck );
	 }
 }

/*
没有任何条件，一定是普通查看
*/
 void CGuildSystem::GetDepotItemListNoCond(int nPage)
 {
	 
 }

/*
nNeedCheck:0-全部，1-需要审核，2-不需要审核
*/
 void CGuildSystem::GetDepotItemListCond( int nPage, int nJobCond, int nGxCond, int nCircle, int nNeedCheck )
 {
 }

 VOID CGuildSystem::SendGuildDepotRecord(CDataPacketReader &packet)
 {
	
 }

 //行会竞价排名
 VOID CGuildSystem::OnStartBidGuildRank()
 {
 }

 //向玩家发送推荐行会的消息
 //注意：需要@addexp触发，不能直接@level
 VOID CGuildSystem::SendSuggestGuildToActor(int nLevel)
 {
 }

 void CGuildSystem::OnRequestHelp(CDataPacketReader &packet)
 {
 }

 //设置自动加入行会的等级要求
 void CGuildSystem::OnSetAddMemberFlag(CDataPacketReader &packet)
 {
	 if (m_pEntity == NULL) return;
	 if (m_pGuild == NULL) return;

	 if( GetGuildPos() < smGuildTangzhu )		//官员、副帮主、帮主都可以设置
	 {
		 m_pEntity->SendOldTipmsgFormatWithId(tpNoRight,ttFlyTip);
		 return;
	 }

	 BYTE bFlag = 0;
	 WORD nLevel = 0;
	 packet >> bFlag >> nLevel;

	 //if(m_pGuild->m_AddMemberFlag == bFlag) return;
	 m_pGuild->m_AddMemberFlag = (int)bFlag;
	 m_pGuild->SetAutoJoinLevel(nLevel);
	 //GetGlobalLogicEngine()->GetMiscMgr().SetSystemStaticVarValue(szGuildAutoJoinLevel, nLevel);
	 SendMemberFlag();
 }

 void CGuildSystem::SendMemberFlag()
 {
	 if (m_pEntity == NULL) return;
	 if (m_pGuild == NULL) return;

	 CActorPacket pack;
	 CDataPacket &data=m_pEntity->AllocPacket(pack);
	 //int nAutoJoinLevel = GetGlobalLogicEngine()->GetMiscMgr().GetSystemStaticVarValue(szGuildAutoJoinLevel);
	 int nAutoJoinLevel = m_pGuild->GetAutoJoinLevel();
	 data << (BYTE)enGuildSystemID << (BYTE)sSetAddMemberFlag;
	 data << (BYTE)m_pGuild->m_AddMemberFlag;
	 data << (WORD)nAutoJoinLevel;
	 pack.flush();
 }

 void CGuildSystem::OnEditGuildTitle(CDataPacketReader &packet)
 {
	if (m_pEntity == NULL) return;
	if (m_pGuild == NULL) return;

	if (GetGuildPos() < smGuildAssistLeader)
	{
		//副帮主以上才可以
		m_pEntity->SendOldTipmsgFormatWithId(tpNoRight,ttFlyTip);
		return;
	}

	CMiniDateTime nTime;
	nTime.tv = GetGlobalLogicEngine()->getMiniDateTime();

	CMiniDateTime LastTime;
	LastTime.tv = m_pGuild->m_TileUpdateTime;
	if(nTime.isSameDay(LastTime)) 
	{
		m_pEntity->SendOldTipmsgFormatWithId(tpSetTitleOneTimes,ttFlyTip);
		return;
	}

	BYTE nCount = 0;
	packet >> nCount;

	for(int i=0;i<(int)nCount;i++)
	{
		BYTE nIndex = 0;
		packet >> nIndex;

		char strName[32];
		packet.readString(strName,ArrayCount(strName));
		strName[sizeof(strName)-1] = 0;

		GetGlobalLogicEngine()->GetChatMgr().Filter(strName);

		if(nIndex > 0 && nIndex < 8)
		{
			_asncpytA(m_pGuild->m_GuildTitles[nIndex-1].nTitleName,strName);
		}
	}

	if(nCount > 0)
	{
		m_pGuild->m_TileUpdateTime =  GetGlobalLogicEngine()->getMiniDateTime();
		m_pGuild->m_boUpdateTime = true;
		m_pGuild->SendTitleToAll();
		

		//添加帮派事件
		//char sText[1024];
		//LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpEditGuildTitle);
		//sprintf_s(sText, sizeof(sText), sFormat, ( char *)((CActor*)m_pEntity->GetEntityName()));

		//m_pGuild->AddEventRecord(sText,tpEditGuildTitle,0,0,0,( char *)((CActor*)m_pEntity->GetEntityName()),NULL);
	}
}


/*
获取帮派成员职位Name，3-首领、2-副首领、1-官员、0-普通成员
代码简单修改后，可以支持 不同帮派不同职位Name
m_nTitle：当职位编辑的情况下使用的，目前不使用，采用固定配置
*/
char* CGuildSystem::GetGuildTitleName()
{
	if(m_pGuild == NULL) return NULL;

	//if(m_nTitle <= 0 || m_nTitle > 4 /*7*/) return NULL;
	//return m_pGuild->m_GuildTitles[m_nTitle-1].nTitleName;
	int nGuildPos = GetGuildPos();
	/*
	if( nGuildPos < 0 || nGuildPos > 3 ) return NULL;		--如果成员title可修改，则才是此段代码
	return m_pGuild->m_GuildTitles[3-nGuildPos].nTitleName;
	*/
	switch (nGuildPos)		//成员title不可修改，则采用此段代码
	{
	case smGuildLeader:
		return GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmGuildHead);
		break;
	case  smGuildAssistLeader:
		return GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmGuildAssistLeader);
		break;
	case  smGuildTangzhu:
		return GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmGuildTangZhu);
		break;
	case  smGuildElite:
		return GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmGuildElite);
		break;
	case  smGuildCommon:
		return GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmGuildMember);
		break;
	default:
		break;
	}
	return NULL;
}

void CGuildSystem::SendGuildTitleList()
{

}

/*
1-帮主
2-副帮主
3-
*/
void CGuildSystem::OnChangeActorTitle(CDataPacketReader &packet)
{
	if (m_pEntity == NULL) return;
	if (m_pGuild == NULL) return;

	if (GetGuildPos() < smGuildAssistLeader)
	{
		//副帮主以上才可以
		m_pEntity->SendOldTipmsgFormatWithId(tpNoRight,ttFlyTip);
		return;
	}

	unsigned int nActorId = 0;
	packet >> nActorId;

	BYTE nIndex = 0;
	packet >> nIndex;

	bool bSetFlag = false;

	//查找这个人是否加入帮派
	ActorCommonNode* pNode = m_pGuild->FindMember(nActorId);
	if (!pNode)
	{
		OutputMsg(rmTip,_T("can not find guild member,actorid=%u"),nActorId);
		return;
	}

	CEntity* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);
	if (pEntity && pEntity->GetType() == enActor)
	{
		if(((CActor*)pEntity)->GetGuildSystem()->GetGuildPos() >= smGuildAssistLeader)
		{
			return;
		}

		if(((CActor*)pEntity)->GetGuildSystem()->m_nTitle != nIndex)
		{
			((CActor*)pEntity)->GetGuildSystem()->m_nTitle = (int)nIndex;
			bSetFlag = true;
		}
	}
	else
	{
		if(pNode->BasicData.nType >= smGuildAssistLeader)
		{
			return;
		}

		if(pNode->BasicData.nGuildTiTle != nIndex)
		{
			pNode->BasicData.nGuildTiTle = (int)nIndex;
			bSetFlag = true;
		}
	}

	if(bSetFlag)
	{
		m_pGuild->SendChangeTitleToAll(nActorId,nIndex);
		m_pEntity->SendTipmsgFormatWithId(tmOperSucc,ttFlyTip);

		((CActor*)m_pEntity)->ResetShowName();

		if(nIndex > 0)
		{
			//添加帮派事件
			//char sText[1024];
			//LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpSetActorTitle);
			//sprintf_s(sText, sizeof(sText), sFormat, ( char *)((CActor*)m_pEntity->GetEntityName()),( char *)pNode->sActorName,m_pGuild->m_GuildTitles[nIndex-1].nTitleName);

			//m_pGuild->AddEventRecord(sText,tpSetActorTitle,nIndex,0,0,( char *)((CActor*)m_pEntity->GetEntityName()),( char *)pNode->sActorName);
		}
	}
}





void CGuildSystem::SendSignUpGuildList()
{
	// if (!m_pEntity || m_pEntity->GetType() != enActor)
	// {
	// 	return ;
	// }
	// CGuildComponent& guildMgr = GetGlobalLogicEngine()->GetGuildMgr();
	// CGuildProvider& guildProvider = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	// CActorPacket pack;
	// CDataPacket &DataPacket=m_pEntity->AllocPacket(pack);
	
	// DataPacket << (BYTE)enGuildSystemID << (BYTE)sGetSignUpGuildList;
	// size_t pos = DataPacket.getPosition();
	// WORD nCount = 0;	//报名的数量
	// DataPacket << nCount;
	// CLinkedNode<CGuild*> *pNode;
	// CLinkedListIterator<CGuild*> it(*guildMgr.m_pRank);
	// for (pNode = it.first(); pNode; pNode = it.next())
	// {
	// 	CGuild* pAllGuild = pNode->m_Data;
	// 	if (pAllGuild && pAllGuild->m_boStatus)
	// 	{
	// 		if ((guildMgr.m_boTodaySign && pAllGuild->m_SignUpFlag)||//今天已经报名了，19:00之后-攻城结束前
	// 			!guildMgr.m_boTodaySign && pAllGuild->m_MainHallLev >= guildProvider.nAutoSignMainHallLevel)//没到报名时间或者攻城结束
	// 		{
	// 			nCount++;
	// 			DataPacket.writeString(pAllGuild->m_sGuildname);
	// 		}
	// 	}
	// }
	// WORD* pCount = (WORD*)DataPacket.getPositionPtr(pos);
	// *pCount = nCount;
	// pack.flush();
}



void CGuildSystem::GMSetGuildCityPostion(const char* sGuildName, BYTE nType, BYTE nState, const char* sName)
{

	// CGuildComponent& guildMgr = GetGlobalLogicEngine()->GetGuildMgr();
	// CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuildByName(sGuildName);
	// if (pGuild)
	// {
	// 	if(pGuild->GetGuildId() !=  guildMgr.m_GuildId) 
	// 	{
	// 		OutputMsg(rmTip,_T("Not Sbk Guild,guildid =%u,  "),pGuild->m_nGid);
	// 		return;
	// 	}
	// 	if(guildMgr.IsGuildSiegeStart())		//开始沙巴克战
	// 	{
	// 		m_pEntity->SendOldTipmsgWithId(tpCityPosErrorInSiege, ttFlyTip);
	// 		return;
	// 	}
	// 	if(nType <= stRealMaster || nType >= stMaxGuildCityPos) return ;
	// 	//任命
	// 	if(nState > 0)
	// 	{
	// 		ActorCommonNode* pNode = pGuild->FindMemberByName(sName);
	// 		if (pNode == NULL)		//行会没有这个成员
	// 		{
	// 			m_pEntity->SendOldTipmsgFormatWithId(tpSetCityPosNotGuildMem,ttFlyTip);
	// 			return;
	// 		}

	// 		if(pNode->BasicData.nType == smGuildLeader) return;

	// 		if(guildMgr.getCityPostionById(pNode->BasicData.nActorId) > stNoCityPos) 
	// 		{
	// 			m_pEntity->SendOldTipmsgWithId(tpSbkPosErrorChange, ttFlyTip);
	// 			return;
	// 		}

	// 		guildMgr.SetCityPos(nType,pNode->BasicData.nActorId);
	// 		m_pEntity->SendOldTipmsgFormatWithId(tpSetSbkPostionSuc,ttFlyTip);

	// 		int nTipsId = tpSetSbkPosSecMonster;

	// 		switch(nType)
	// 		{
	// 		case stEastProtector:
	// 			nTipsId = tpSetSbkMasterWife;
	// 			break;
	// 		case stSouthProtector:
	// 			nTipsId = tpSetSbkPosSecMonster;
	// 			break;

	// 		case stWestProtector:
	// 			nTipsId = tpSetSbkPosTips2;
	// 			break;

	// 		case stNorthProtector:
	// 			nTipsId = tpSetSbkPosTips3;
	// 			break;
	// 		}

	// 		CEntityManager* em	= GetGlobalLogicEngine()->GetEntityMgr();
	// 		em->BroadTipmsgWithParams(nTipsId, ttScreenCenter + ttChatWindow, ( char *)sName);
			
	// 	}
	// 	else
	// 	{
	// 		guildMgr.SetCityPos(nType,0);
	// 	}
	// 	SendGuildCityPosInfo();
	// }

}
void CGuildSystem::SetGuildCityPostion(BYTE nType, BYTE nState, const char* sName)
{
	if(m_pEntity == NULL) return;
	if(m_pGuild == NULL) return;
	CGuildComponent& guildMgr = GetGlobalLogicEngine()->GetGuildMgr();
	CEntityManager* em	= GetGlobalLogicEngine()->GetEntityMgr();
	if(m_pGuild->m_nGid != guildMgr.m_GuildId) 
	{
		OutputMsg(rmTip,_T("Not Sbk Guild,guildid =%u,  "),m_pGuild->m_nGid);
		return;
	}

	if(guildMgr.IsGuildSiegeStart())		//开始沙巴克战
	{
		m_pEntity->SendOldTipmsgWithId(tpCityPosErrorInSiege, ttFlyTip);
		return;
	}

	if(GetGuildPos() == smGuildLeader)
	{
		if(nType <= stRealMaster || nType >= stMaxGuildCityPos) return ;
		//任命
		if(nState > 0)
		{
			ActorCommonNode* pNode = NULL;
			pNode = m_pGuild->FindMemberByName(sName);
			if (pNode == NULL)		//行会没有这个成员
			{
				m_pEntity->SendOldTipmsgFormatWithId(tpSetCityPosNotGuildMem,ttFlyTip);
				return;
			}
			if(pNode->BasicData.nType == smGuildLeader)
			{
				//无法设置君主作为护法
				m_pEntity->SendOldTipmsgFormatWithId(tpSetCityPosGuildLeaderCantSet,ttFlyTip);
				return;
			}

			if(guildMgr.getCityPostionById(pNode->BasicData.nActorId) > stNoCityPos) 
			{
				m_pEntity->SendOldTipmsgWithId(tpSbkPosErrorChange, ttFlyTip);
				return;
			}

			guildMgr.SetCityPos(nType,pNode->BasicData.nActorId);
			CActor *pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(pNode->BasicData.nActorId);
			if(pActor != NULL)
			{
				CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
				if (pNpc != NULL) 
				{
					CScriptValueList paramList, retParamList;
					paramList << pActor;
					pNpc->GetScript().Call("GetCityPosWelfareInfo",paramList,retParamList);
				}
			}


			m_pEntity->SendOldTipmsgFormatWithId(tpSetSbkPostionSuc,ttFlyTip);

			int nTipsId = tpSetSbkPosSecMonster;

			switch(nType)
			{
			case stEastProtector:
				nTipsId = tpSetSbkMasterWife;
				break;
			case stSouthProtector:
				nTipsId = tpSetSbkPosSecMonster;
				break;

			case stWestProtector:
				nTipsId = tpSetSbkPosTips2;
				break;

			case stNorthProtector:
				nTipsId = tpSetSbkPosTips3;
				break;
			}
			em->BroadTipmsgWithParams(nTipsId, ttScreenCenter + ttChatWindow, ( char *)sName);
		}
		else
		{
			guildMgr.SetCityPos(nType,0);
		}
		SendGuildCityPosInfo();
	}
	else
	{
		m_pEntity->SendOldTipmsgFormatWithId(tpGuildSetPosNotLeader,ttFlyTip);
	}
}


int CGuildSystem::GetGuildCityPos()
{
	if(m_pEntity == NULL) return 0;
	if(m_pGuild == NULL) return 0;
	CGuildComponent& guildMgr = GetGlobalLogicEngine()->GetGuildMgr();
	if(m_pGuild->m_nGid != guildMgr.GetCityOwnGuildId()) return stNoCityPos;
	return guildMgr.getCityPostionById(m_pEntity->GetId());
}




/*
行会成员被击杀
pKiller：击杀者
*/
void CGuildSystem::OnBeKilledByOtherTips(CActor* pKiller,char* szSceneName)
{
	if(m_pEntity == NULL) return;
	
	if(pKiller == NULL || szSceneName == NULL) return;
	CEntityManager* em	= GetGlobalLogicEngine()->GetEntityMgr();
	if(em == NULL) return;

	if(m_pGuild == NULL) 
	{
		return;
	}



	// static int sMinLv = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nGuilderKilledMinLevel;
	// if( m_pEntity->GetProperty<int>(PROP_CREATURE_LEVEL) <= sMinLv )
	// {
	// 	return;
	// }
	if(m_pGuild->m_nGid > 0 && m_pGuild->m_nGid == GetGlobalLogicEngine()->GetGuildMgr().GetCityOwnGuildId() 
		&& GetGuildCityPos() == stRealMaster)
	{
		int x,y;
		m_pEntity->GetPosition(x,y);
		// em->BroadTipmsgWithParams(tmSbkGuildLeaderBeKilledtips, tstFigthing, m_pEntity->GetEntityName(),
		// 	szSceneName, x,y,pKiller->GetEntityName());
		return;
	}

	// int nGuildPos = GetGuildPos();
	// if( nGuildPos == smGuildLeader || nGuildPos == smGuildAssistLeader || nGuildPos == smGuildTangzhu )
	// {
	// 	LPCTSTR gPosName = m_pGuild->GetGuildTitleName(nGuildPos);	
	// 	em->BroadTipmsgWithParams(tmGuildBeKillTips, tstFigthing, m_pGuild->m_sGuildname, gPosName,
	// 		m_pEntity->GetEntityName(), szSceneName, pKiller->GetEntityName());
	// }
	// else if(GetGuildPos() == smGuildAssistLeader)
	// {
	// 	em->BroadTipmsgWithParams(tmGuildSecLeaderBeKilledTips,tstFigthing,m_pGuild->m_sGuildname,m_pEntity->GetEntityName(),szSceneName,pKiller->GetEntityName());
	// }else
	// {
	// 
	// }
}



void CGuildSystem::OnChangeNameGuildOp()
{
	if(m_pEntity == NULL) return;
	if(m_pGuild == NULL) return;

	unsigned int nActorId = ((CActor*)m_pEntity)->GetProperty<unsigned int>(PROP_ENTITY_ID);

	ActorCommonNode* pNode = m_pGuild->FindMember(nActorId);
	if(pNode)
	{
		_asncpytA(pNode->sActorName,m_pEntity->GetEntityName());
	}
}

void CGuildSystem::ChangeGuildGx(int nGx, int nLogId)
{
	if(!m_pEntity) return;
	if(!m_pGuild) return;
	if (nGx == 0 )
	{
		return;
	}
	int mGx = m_pEntity->GetProperty<int>(PROP_ACTOR_GUILDEXP);
	int nOldValue = mGx;
	mGx += nGx;

	if(mGx < 0)
		mGx = 0;

	if(mGx > MAXGUILDGXEXP)
	{
		mGx = MAXGUILDGXEXP;
	}
	m_pEntity->GiveAward(qaGuildDonate, 0, nGx,0,0,0,0,nLogId);
	// m_pEntity->SetProperty<int>(PROP_ACTOR_GUILDEXP,mGx);

	int nNewValue = mGx;
	if(nGx > 0)
	{
		m_GuildGx += nGx;
		if(m_GuildGx < 0)
			m_GuildGx = 0;
	}

	ActorCommonNode* pNode = m_pGuild->FindMember(m_pEntity->GetId());
	if (pNode)
	{
		pNode->BasicData.nGuildGX = m_pEntity->GetProperty<int>(PROP_ACTOR_GUILDEXP);
		pNode->BasicData.nTotalGx = m_GuildGx;
		m_pGuild->AddToGuildGxList(pNode->BasicData.nActorId,(int)pNode->BasicData.nGuildGX);
	}

	if(nGx > 0)
	{
		m_pEntity->SendOldTipmsgFormatWithId(tpGetGuildGxTips, ttTipmsgWindow,nGx);
	}
	else if(nGx < 0)
	{
		m_pEntity->SendOldTipmsgFormatWithId(tpDelteGuildGxTips,ttTipmsgWindow,-nGx);
	}

	//记录日志
	if(GetLogicServer()->GetLocalClient())
	{
	}
}

void CGuildSystem::ChangeTodayGx(int nValue)
{
	if(!m_pEntity) return;
	if(!m_pGuild) return;

	if(nValue > 0)
	{
		if(nValue > MAXGUILDGXEXP)
		{
			nValue = MAXGUILDGXEXP;
		}
		m_TodayGx += nValue;

		if(m_TodayGx > MAXGUILDGXEXP)
		{
			m_TodayGx = MAXGUILDGXEXP;
		}
		SendTodayGetGuildgx();
	}
}

void CGuildSystem::SendGxRankinglist()
{
	
}

/*
如果只有行会主殿一个建筑，则不使用，行会等级（主殿等级）在行会详细信息（10,1）的协议返回
*/
void CGuildSystem::SendGuildbuildingInfo()
{
	

}

void CGuildSystem::SendTodayGetGuildgx()
{
	// if(!m_pEntity) return;
	// if(!m_pGuild) return;

	// CActorPacket pack;
	// CDataPacket &DataPacket=m_pEntity->AllocPacket(pack);

	// DataPacket << (BYTE)enGuildSystemID << (BYTE)sSendTodayBuildingGx;
	
	// DataPacket << (int)m_TodayGx;
	// pack.flush();
}

//下发行会捐献数据
void CGuildSystem::SendGuildDonateData()
{

}

void CGuildSystem::SendGuildbuildCdTime()
{
	
}

void CGuildSystem::OnRevActorBuildData(CDataPacketReader &reader)
{
	if(!m_pEntity) return;

	reader >> m_TodayGx;
	reader >> m_HasShxTimes;
	reader >> m_AddShxTimes;

	reader >> m_ExploreTimes;
	reader >> m_ExploreItemId;
	reader >> m_ChallengeTimes;

	reader >> m_AddChallengeTimes;

	

	if(m_TodayGx > 0)
	{
		 SendTodayGetGuildgx();
	}
}

void CGuildSystem::OnNewDayArriveOP(bool bState)
{
	if(!m_pEntity) return;
	m_TodayGx = 0;
	m_HasShxTimes = 0;
	m_AddShxTimes = 0;
	m_ExploreTimes = 0;
	m_ChallengeTimes = 0;
	m_AddChallengeTimes = 0;
	m_nTodayDonateCoin = 0;
}

void CGuildSystem::SendCallTimeRemain()
{

}



unsigned int CGuildSystem::GetGuildId()
{
	CGuild* pGuild = ((CActor*)m_pEntity)->GetGuildSystem()->GetGuildPtr();
	if (pGuild)
	{
		return pGuild->m_nGid;
	}
	return 0;
}





/*
通知客户端，某些操作成功
*/
void CGuildSystem::SendGuildOpOk(BYTE nType)
{
	// if ( !m_pEntity ) 
	// 	return;

	// CActorPacket AP;
	// CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);

	// DataPacket << (BYTE)enGuildSystemID << (BYTE)sGuildOpOk;
	// DataPacket << (BYTE)nType;
	// AP.flush();
}

void CGuildSystem::TriggerEvent(INT_PTR nSubEvent,INT_PTR nParam1,INT_PTR nParam2,INT_PTR nParam3,INT_PTR nParam4)
{
	CScriptValueList paramList;
	//((CActor*)m_pEntity)->InitEventScriptVaueList(paramList,aeGuild);
	paramList << (int)nSubEvent;
	if(nParam1 >=0)
	{
		paramList << (int)nParam1;
		if(nParam2 >=0)
		{
			paramList << (int)nParam2;
			if(nParam3 >=0)
			{
				paramList << (int)nParam3;
				if(nParam4 >=0)
				{
					paramList << (int)nParam4;
				}
			}
		}	
	}

	((CActor*)m_pEntity)->OnEvent(aeGuild,paramList,paramList);
}

void CGuildSystem::PutInUpgradeGuildItem(int nCount1, int nCount2, int nCount3)
{
	if ((!m_pEntity) || (!m_pGuild)) return;
	TriggerEvent(gPutInUpgradeGuildItem, nCount1, nCount2, nCount3);
}

void CGuildSystem::SendGuildUpgradeItem()
{
	
}

/*行会成员弹劾上级成员
*/
void CGuildSystem::ImpeachGuildMember(unsigned int nObjActorId)
{
	if (m_pEntity == NULL) return;
	if (m_pGuild == NULL)
	{
		m_pEntity->SendTipmsgFormatWithId(tmNotInGuild, tstUI);
		return;
	}
	if (GetGlobalLogicEngine()->GetGuildMgr().IsGuildSiegeStart())
	{
		m_pEntity->SendTipmsgFormatWithId(tmInSbkWar, tstUI);
		return;
	}
	CGuildComponent& guildMgr = GetGlobalLogicEngine()->GetGuildMgr();
	CActor *pActor = (CActor *)m_pEntity;
	if( nObjActorId == pActor->GetId() )		//不能弹劾自己
	{
		pActor->SendOldTipmsgFormatWithId(tmGuildImpeachSelf,tstUI);
		return;
	}

	int nMyHisPos = GetGuildPos();		//本人原职位
	ActorCommonNode* pMyNode = m_pGuild->FindMember(pActor->GetId());
	if (!pMyNode)
	{
		OutputMsg(rmTip,_T("I am not the guild member"));
		return;
	}
	
	//查找这个人是否加入帮派
	ActorCommonNode* pObjNode = m_pGuild->FindMember(nObjActorId);
	if (!pObjNode)
	{
		OutputMsg(rmTip,_T("can not find guild member,actorid=%d"),nObjActorId);
		return;
	}
	int nObjPos = 0;		//对方的行会职位
	CEntity* pObjEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(pObjNode->hActorHandle);
	if( pObjEntity )		//对方在线，不能弹劾
	{
		pActor->SendTipmsgFormatWithId(tmGuildImpeachObjOnline, tstUI);
		return;
	}
	else
	{
		//不在线
		nObjPos = pObjNode->BasicData.nType;	//对方职位
	}

	CGuildProvider& pConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	int nNow = GetGlobalLogicEngine()->getMiniDateTime();
	if( nNow < pObjNode->BasicData.nTime + pConfig.nProtectDay*86400)
	{
		m_pEntity->SendTipmsgFormatWithId(tmGuildInProtectDay,tstUI, pConfig.nProtectDay);
		return;
	}

	if( nObjPos != smGuildLeader )	//
	{
		m_pEntity->SendTipmsgFormatWithId(tmImpeachOfficeError,tstUI);
		return;
	}

	if(nMyHisPos < smGuildTangzhu)
	{
		m_pEntity->SendTipmsgFormatWithId(tmNoRight,tstUI);
		return;
	}

	int yb = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_YUANBAO);
	if( yb < pConfig.nImpeachcost)
	{
		m_pEntity->SendTipmsgFormatWithId(tmYbLimitNoImpeach, tstUI);
		return;
	}

	int objsbkPos =  guildMgr.getCityPostionById(pObjNode->BasicData.nActorId);//获取对方sbk职位
	int mysbkPos =  guildMgr.getCityPostionById(pMyNode->BasicData.nActorId);//获取对方sbk职位
	guildMgr.SetCityPos(mysbkPos, 0);
	guildMgr.SetCityPos(objsbkPos, pMyNode->BasicData.nActorId);


	pMyNode->BasicData.nType	= pObjNode->BasicData.nType;				//获取对方职位
	pObjNode->BasicData.nType	= smGuildCommon;		//对方变成普通成员
	// if( nObjPos == smGuildLeader )	//本人节点成为帮主节点
	// {
	// 	guildMgr.OnGuildLeaderChange(m_pGuild, nObjActorId, m_pEntity->GetId(),glcImpeachLeader);
	// }
	pMyNode->BasicData.nZzNext	= GetZZNext();

	pActor->GetGuildSystem()->SetGuildPos(nObjPos);
	pActor->GetGuildSystem()->m_nTitle = pObjNode->BasicData.nGuildTiTle;		//封号序号（不使用了）
	pActor->ResetShowName();
	if( nObjPos == smGuildLeader )	//本人节点成为帮主节点
	{
		guildMgr.OnGuildLeaderChange(m_pGuild, nObjActorId, m_pEntity->GetId(),glcImpeachLeader);
	}

	if (nMyHisPos == smGuildAssistLeader)	//副帮主
	{
		m_pGuild->m_nSecLeaderCount--;
	}
	else if (nMyHisPos == smGuildTangzhu)	//官员
	{
		m_pGuild->m_GuildTkCount--;
	}

	char sText[1024];
	LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmGuildEventCoin);
	sprintf_s(sText, sizeof(sText), sFormat, ( char *)(m_pEntity->GetEntityName()),(int)pConfig.nImpeachcost);
	pActor->ChangeMoney(mtYuanbao, -pConfig.nImpeachcost, GameLog::Log_GuildImpeach, 0,sText,true);

	//添加帮派事件
	//char sText[1024]={0};
	//LPCTSTR sFormat;
	//sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildImpeachEvent);
	LPCTSTR strPosName = m_pGuild->GetGuildTitleName(nObjPos);		//GetGuildPosName(nObjHisPos);
	//sprintf_s(sText, sizeof(sText), sFormat, (char *)(pMyNode->sActorName), (char *)(pObjNode->sActorName), strPosName);
	m_pGuild->AddEventRecord( "", enGuildEvent_Impeach, nObjPos, 0, 0, (char *)pObjNode->sActorName, NULL);

	//保存到数据库
	Save(m_pGuild->m_nGid,pObjNode->BasicData.nActorId, pObjNode->BasicData.nTotalGx, pObjNode->BasicData.nType << 16,
		pObjNode->BasicData.nZjNext, pObjNode->BasicData.nZzNext, pObjNode->BasicData.nGuildTiTle, pObjNode->BasicData.nGuildCityPos,
		pObjNode->BasicData.nTime, pObjNode->BasicData.nModelId, pObjNode->BasicData.nJoinTime);
	//保存到数据库
	Save(m_pGuild->m_nGid,pMyNode->BasicData.nActorId, pMyNode->BasicData.nTotalGx, pMyNode->BasicData.nType << 16,
		pMyNode->BasicData.nZjNext, pMyNode->BasicData.nZzNext, pMyNode->BasicData.nGuildTiTle, pMyNode->BasicData.nGuildCityPos,
		pMyNode->BasicData.nTime, pMyNode->BasicData.nModelId, pMyNode->BasicData.nJoinTime);

	// char chatMsg[128]={0};
	// LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildPosChange);
	// sprintf_s(chatMsg, sizeof(chatMsg), sFormat, ( char *)(pMyNode->sActorName), strPosName);
	//((CActor *)m_pEntity)->GetChatSystem()->SendChat(ciChannelGuild, chatMsg);
	//m_pGuild->BroadCastMsgInGuildChannel(chatMsg);
	SendMemberList();		//刷新列表

	//通知其他成员有弹劾发生了
	m_pGuild->NotifyImpeachMember(strPosName, pObjNode->sActorName,  pMyNode->sActorName);
}

int CGuildSystem::GetDailyDonateCoinLimit()
{
	if( m_pEntity == NULL ) return 0;
	if( m_pGuild == NULL )  return 0;

	CGuildProvider& pConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	int nGuildLevel = m_pGuild->GetLevel();
	if( nGuildLevel > 0 && nGuildLevel <= pConfig.m_DailyDonateLimitList.count() ) //1到N
	{
		return pConfig.m_DailyDonateLimitList[nGuildLevel-1].m_nCoinLimit;
	}
	return 0;
}

int CGuildSystem::GetDailyDonateItemLimit()
{
	if( m_pEntity == NULL ) return 0;
	if( m_pGuild == NULL )  return 0;

	CGuildProvider& pConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	int nGuildLevel = m_pGuild->GetLevel();
	if( nGuildLevel > 0 && nGuildLevel <= pConfig.m_DailyDonateLimitList.count() ) //1到N
	{
		return pConfig.m_DailyDonateLimitList[nGuildLevel-1].m_nItemLimit;
	}
	return 0;
}

//行会神树充能
void CGuildSystem::ChargeGuildTree()
{
	if ((!m_pEntity) || (!m_pGuild)) return;
	TriggerEvent(gChargeGuildTree);
}

//摘取行会神树果实
void CGuildSystem::PickGuildFruit()
{
	if ((!m_pEntity) || (!m_pGuild)) return;
	TriggerEvent(gPickGuildFruit);
}


//下发行会神树数据
void CGuildSystem::SendGuildTreeData()
{
	// if(!m_pEntity) return;
	// if(!m_pGuild) return;

	// CGuildProvider& pConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	// int nChargeNumLast = pConfig.m_GuildTree.m_nDailyChargeLimit - m_nTodayChargeTreeNum;
	// if( nChargeNumLast < 0 )
	// {
	// 	nChargeNumLast = 0;
	// }
	// CActorPacket pack;
	// CDataPacket &DataPacket=m_pEntity->AllocPacket(pack);
	// DataPacket << (BYTE)enGuildSystemID << (BYTE)sSendGuildTreeData;
	// DataPacket << (short)m_pGuild->m_nGuildTreeLevel;
	// DataPacket << (int)m_pGuild->m_nGuildTreeDegree;
	// DataPacket << (short)nChargeNumLast;
	// pack.flush();
}

//下发行会神树果实数据
void CGuildSystem::SendGuildFruitData()
{

}

void CGuildSystem::SendGuildTreeBackCoin()
{
	
}

//下发行会神树果实被采摘的数据
void CGuildSystem::SendGuildFruitPickData()
{
	// 
}

void CGuildSystem::OnGuildTask(int nTaskType, int nTaskObjId, int nAddTaskSche, int nParam)
{
	if(!m_pEntity) return;
	if(!m_pGuild) return;
	m_pGuild->OnGuildTask(nTaskType, nTaskObjId, nAddTaskSche, nParam);
	SendGuildTaskInfo();
}

//下发行会任务信息
void CGuildSystem::SendGuildTaskInfo()
{
	
}

//领取行会任务
void CGuildSystem::GetGuildTaskAward()
{
	if ((!m_pEntity) || (!m_pGuild)) return;
	TriggerEvent(gGetGuildTaskAward);
}

//领取行会福利
void CGuildSystem::GetGuildBenefit()
{
	if ((!m_pEntity) || (!m_pGuild)) return;
	TriggerEvent(gGetGuildBenefit);
}

void CGuildSystem::SendGuildTaskBenefitData()
{
	if ((!m_pEntity) || (!m_pGuild)) return;
	TriggerEvent(gSendGuildTaskBenefitData);
}

/*下发攻城战指令
*/
void CGuildSystem::SendGuildSiegeCmdData( )
{
	if( !m_pEntity || (!m_pGuild) ) return;
	if( m_pGuild->m_SignUpFlag != 1 )			//行会没有报名参加攻城战
	{
		return;
	}

	CActor* pActor = (CActor*)m_pEntity;
	//如果行会战正在进行中，则通知
	if( GetGlobalLogicEngine()->GetGuildMgr().IsGuildSiegeStart() )
	{
		m_pGuild->SendGuildSiegeCmdPanel( pActor, 1);			//打开面板
		m_pGuild->SendGuildSiegeCommand( pActor );				//发送命令
	}
}

void CGuildSystem::SetExchangeNeedCheckCirclrMin(int nCircleMin)
{
	// if( !m_pEntity || (!m_pGuild) )
	// {
	// 	return;
	// }

	// if( GetGuildPos() < smGuildAssistLeader )		//只有会长、副会长才有权限设置
	// {
	// 	//官员以上才有权限操作
	// 	m_pEntity->SendOldTipmsgFormatWithId(tpNoRight,ttFlyTip);
	// 	return;
	// }

	// CGuildProvider& pGuildConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	// int nCheckCircleMin = pGuildConfig.m_GuildDepot.nExchangeNeedCheckCircleMin;
	// if( nCircleMin < nCheckCircleMin )
	// {
	// 	m_pEntity->SendOldTipmsgFormatWithId( tpGuildDepotItemExchangeCircleLimit, ttFlyTip, nCheckCircleMin );
	// 	return;
	// }
	// m_pGuild->m_nExchangeCheckCircleMin = nCircleMin;
	// m_pGuild->m_boUpdateTime = true;
	// SendGuildInfoSimple();

	// char chatMsg[128]={0};
	// LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildDepotItemExchangeCircleSet);
	// sprintf_s(chatMsg, sizeof(chatMsg), sFormat, ((CActor*)m_pEntity)->GetEntityName(), nCircleMin);
	// m_pGuild->BroadCastMsgInGuildChannel(chatMsg);

	// m_pGuild->NotifyGuildSimpleInfo();			//推送其它客户端
}

void CGuildSystem::OneKeyClearDepotItem( int nCircle )
{
	if( !m_pEntity || (!m_pGuild) )
	{
		return;
	}

	if( GetGuildPos() < smGuildTangzhu )		//只有官员才有权限设置
	{
		//官员以上才有权限操作
		m_pEntity->SendOldTipmsgFormatWithId(tpNoRight,ttFlyTip);
		return;
	}

	int nDelItemNum = 0;						//被删除的装备
	const CStdItemProvider& itemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	INT_PTR nCount = m_pGuild->m_GuildDepotBag.count();
	for( INT_PTR i=nCount-1; i>=0; i-- )			//删除物品，从链表从尾到头删除
	{
		GUILDDEPOTITEM& nItem	= m_pGuild->m_GuildDepotBag[i];
		CUserItem *pUserItem	= nItem.nUserItem;
		if( nItem.nExchangeActorId == 0 && pUserItem )		//非正在审核中的装备
		{
			const CStdItem *pStdItem  = itemProvider.GetStdItem(pUserItem->wItemId);
			if( pStdItem )
			{
				int nItemCircle = (int)pStdItem->GetUseConditionValue(CStdItem::ItemUseCondition::ucMinCircle);	//转数
				if( nCircle == nItemCircle )
				{
					m_pGuild->m_GuildDepotBag.remove(i);			//仓库删掉物品
					nDelItemNum ++;
				}
			}
		}
	}
	m_pGuild->NotifyUpdateDepot();							//通知客户端重新请求数据
	m_pGuild->m_bSaveDepotRecord = true;

	if( nDelItemNum > 0 )
	{
		m_pEntity->SendOldTipmsgFormatWithId(tpRemoveGuildDepotItem, ttFlyTip, nDelItemNum, nCircle);
	}
}

void CGuildSystem::SendJoinGuildApply()
{
	if (m_pEntity == NULL) return;
	if (m_pGuild == NULL) return;

	CActorPacket AP;
	CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);
	DataPacket << (BYTE)enGuildSystemID << (BYTE)sJoinApplyMsgList;
	size_t pos = DataPacket.getPosition();
	DataPacket << (int)0;//未必是真实的数量
	int nResult = 0;

	int nSize = m_pGuild->m_ApplyList.size();
	if(nSize > 0)
	{
		std::map<unsigned int,ApplyList>::iterator it = m_pGuild->m_ApplyList.begin();
		for(; it != m_pGuild->m_ApplyList.end(); it++)
		{
			ApplyList* pJoinApply = &(it->second);
			DataPacket << (unsigned int)pJoinApply->nActorId;
			DataPacket.writeString(pJoinApply->szName);
			DataPacket << (int)pJoinApply->nLevel;
			DataPacket << (BYTE)pJoinApply->nCircle;//
			DataPacket << (BYTE)pJoinApply->nJob;
			DataPacket << (BYTE)pJoinApply->nSex;
			DataPacket << (unsigned int)pJoinApply->nVip;
			nResult++;
		}
	}

	int* pCount = (int*)DataPacket.getPositionPtr(pos);
	*pCount = nResult;
	AP.flush();
}

//新成员成功加入行会
void CGuildSystem::OnJoinGuildSuccess()
{
	if( m_pEntity == NULL ) return;
	if( m_pGuild == NULL ) return;

	CActor* pActor = (CActor*)m_pEntity;
	pActor->ResetShowName();			//重刷Name
	// pActor->GetAchieveSystem().OnEvent(eAchieveEventJoinGuild); 
	// pActor->GetQuestSystem()->OnQuestEvent(CQuestData::qtEnterGuild, 1, 1);


	//添加帮派事件
	char sText[1024];
	//LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpAddToGuildEvent);
	//sprintf_s(sText, sizeof(sText), sFormat, ( char *)(m_pEntity->GetEntityName()));

	m_pGuild->AddEventRecord( "", enGuildEvent_MemberJoin, 0, 0, 0, (char *)(m_pEntity->GetEntityName()),NULL );

	LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpAddToGuildmsg);
	sprintf_s(sText, sizeof(sText), sFormat, ( char *)(m_pEntity->GetEntityName()));
	//pGuild->SendGuildMsg(smGuildCommon,sText,ttChatWindow);
	//((CActor *)m_pEntity)->GetChatSystem()->SendChat(ciChannelGuild, sText);
	m_pGuild->BroadCastMsgInGuildChannel(sText);

	//m_pEntity->GetQuestSystem()->OnQuestEvent(CQuestData::qcGuildLevel, 1, 1);
	SendGuildSiegeCmdData();			//加入行会，通知攻城战指令
	SendGuildOpOk(GUILD_OP_JOIN_OK);
}

int CGuildSystem::QuickSortGuildDepotCmp(const void *a ,const void *b)
{
	GUILDDEPOTITEM* Sa = (GUILDDEPOTITEM*)a;
	GUILDDEPOTITEM* Sb = (GUILDDEPOTITEM*)b;
	int nSaIsInCheck = Sa->nExchangeActorId > 0 ? 0: 1;
	int nSbIsInCheck = Sb->nExchangeActorId > 0 ? 0: 1;

	if( nSaIsInCheck == nSbIsInCheck )			//未审核的排在前面
	{
		return Sa->nUserItem->wItemId - Sb->nUserItem->wItemId;
	}
	return nSbIsInCheck - nSaIsInCheck;
}

VOID CGuildSystem::CallGuildMember()
{
	// if (m_pGuild == NULL) return;
	// if (m_pEntity == NULL || m_pEntity ->GetType() != enActor) return;

	// if (GetGuildPos() != smGuildLeader &&  GetGuildPos() != smGuildAssistLeader)
	// {
	// 	m_pEntity->SendOldTipmsgFormatWithId(tpGuildCallNotOwner,ttFlyTip);
	// 	return;
	// }
	// CGuildProvider& pConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	// int x,y;
	// ((CActor*)m_pEntity)->GetPosition(x,y);
	// CScene* pScene = ((CActor*)m_pEntity)->GetScene();
	// if (pScene && pScene->HasMapAttribute(x,y,aaSceneNotTransfer,1))
	// {
	// 	m_pEntity->SendOldTipmsgFormatWithId(tpGuildCallSceneNotRight,ttFlyTip);
	// 	return;
	// }

	// if (m_pGuild->GetCoin() < pConfig.nCallMemberNeed )
	// {
	// 	m_pEntity->SendOldTipmsgFormatWithId(tpGuildCallCoinNotEnough,ttFlyTip);
	// 	return;
	// }
	// m_pGuild->ChangeGuildCoin(-pConfig.nCallMemberNeed, 0,"call member");

	// char sText[1024];
	// LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildCallChatInfo);
	// sprintf_s(sText, sizeof(sText), sFormat, ( char *)(m_pEntity->GetEntityName()), pConfig.nCallMemberNeed);
	// m_pGuild->BroadCastMsgInGuildChannel(sText);


	// CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	// CLinkedNode<EntityHandle> *pNode;
	// CLinkedListIterator<EntityHandle> it(m_pGuild->m_ActorOnLine);
	// for (pNode = it.first(); pNode; pNode = it.next())
	// {
	// 	EntityHandle& hHandle = pNode->m_Data;
	// 	CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
	// 	if (pEntity && pEntity->GetType() == enActor && hHandle != m_pEntity->GetHandle())
	// 	{
	// 		CActor* toActor = (CActor*)pEntity;

	// 		CActorPacket pack;
	// 		CDataPacket &DataPacket = toActor->AllocPacket(pack);
	// 		DataPacket << (BYTE)enGuildSystemID << (BYTE)sCallRequest << m_pEntity->GetHandle() << m_pEntity->GetEntityName();
	// 		pack.flush();

	// 	}
	// }
}

VOID CGuildSystem::CallGuildMemberBack(CDataPacketReader &packet)
{
	// if (m_pGuild == NULL) return;
	// if (m_pEntity == NULL || m_pEntity ->GetType() != enActor) return;

	// BYTE bResult;
	// EntityHandle hHandle;

	// packet >> bResult >> hHandle;

	// if (bResult == 0)
	// {
	// 	return;
	// }

	// CGuildProvider& pConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	// CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	// CEntity* pEntity = pEntityMgr->GetEntity(hHandle);

	// if(!pEntity)
	// {
	// 	m_pEntity->SendOldTipmsgFormatWithId(tpGuildCallTargetIsNotOnLine,ttFlyTip);
	// 	return;
	// }
	// int x,y;
	// ((CActor*)m_pEntity)->GetPosition(x,y);
	// CScene* pScene = ((CActor*)m_pEntity)->GetScene();
	// if (pScene && pScene->HasMapAttribute(x,y,aaSceneNotTransfer,1))
	// {
	// 	CActor* toActor = (CActor*)m_pEntity;
	// 	CActorPacket pack;
	// 	CDataPacket &DataPacket = toActor->AllocPacket(pack);
	// 	DataPacket << (BYTE)enGuildSystemID << (BYTE)sCallRequest << pEntity->GetHandle() << pEntity->GetEntityName();
	// 	pack.flush();
	// 	m_pEntity->SendOldTipmsgFormatWithId(tpGuildCallSceneNotRight,ttFlyTip);
	// 	return;
	// }

	// int nX,nY;
	// ((CActor*)pEntity)->GetPosition(nX,nY);
	// CScene* pTargetScene = ((CActor*)pEntity)->GetScene();
	// if (pTargetScene && pTargetScene->HasMapAttribute(nX,nY,aaSceneNotTransfer,1))
	// {
	// 	m_pEntity->SendOldTipmsgFormatWithId(tpGuildCallTargetSceneNotRight,ttFlyTip);
	// 	return;
	// }

	// pTargetScene->EnterScene(m_pEntity,nX, nY);

	// CScriptValueList paramList, retParamList;
	// CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
	// paramList << pEntity << m_pEntity;
	// if (pNpc == NULL) return;
	// if (!pNpc->GetScript().Call("CallGuildMemberBack", paramList, retParamList))
	// {
	// 	const RefString &s = pNpc->GetScript().getLastErrorDesc();
	// 	OutputMsg(rmError, (LPCSTR)s);
	// }


}
void CGuildSystem::SendGuildPlayerPos(int nType,CVector<void*>& playerList)
{
	
}
void CGuildSystem::CheckGuildPlayerPos(int nType)
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor || !m_pEntity->IsInited())
	{
		return;
	}
	int x,y;
	m_pEntity->GetPosition(x,y);
	if (m_pEntity->GetScene() && m_pEntity->GetScene()->HasMapAttribute(x,y,aaForcePkMode,fpUnion))
	{
		((CActor*)m_pEntity)->GetGuildSystem()->BroadCastGuildPlayerPos(nType);
	}
}
void CGuildSystem::BroadCastGuildPlayerPos(int nType, CScene* pScene)
{
	DECLARE_TIME_PROF("CGuildSystem::BroadCastGuildPlayerPos");
	if (m_pEntity == NULL || m_pEntity ->GetType() != enActor ) return;
	if (!pScene)
		pScene = m_pEntity->GetScene();
	if (!pScene) return;
	
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(pScene->GetPlayList());
	CVector<void*> playerList;//区域玩家列表
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
		if (pEntity && pEntity->GetType() == enActor)
		{
			CActor* pActor = (CActor*)pEntity;
			int x,y;
			pActor->GetPosition(x,y);
			if (pScene->HasMapAttribute(x,y,aaForcePkMode,fpUnion) && m_pEntity!= pActor)
			{
				playerList.add(pActor);
			}
		}		
	}
	if (nType == gptEnter || nType == gptDisappear) //自己进入或出去，初始化区域所有坐标或全部消失
	{
		m_pEntity->GetGuildSystem()->SendGuildPlayerPos(nType,playerList);
	}
	
	CVector<void*> updateList;
	updateList.add(m_pEntity);
	for (int i =0; i < playerList.count(); i++)
	{
		CActor* pActor = (CActor*)playerList[i];
		if(pActor) 
		{
			pActor->GetGuildSystem()->SendGuildPlayerPos(nType,updateList);
		}

	}
}


void CGuildSystem::sendGuilBuilds()
{
	if(!m_pEntity) return;

	if(!m_pGuild ) return;

	CActorPacket ap;
	CDataPacket& dataPack = m_pEntity->AllocPacket(ap);
	dataPack <<(BYTE)enGuildSystemID << (BYTE)sGuildBuilds;

	int nNum = GetLogicServer()->GetDataProvider()->GetGuildConfig().nBuildNum;
	dataPack << (BYTE)nNum;
	for(int i = 1; i <= nNum ; i++)
	{
		dataPack << (BYTE)i;
		dataPack <<(BYTE)(m_pGuild->GetGuildbuildingLev(i));
	}

	ap.flush();
}

void CGuildSystem::UpdateGuildBuild(CDataPacketReader& packet)
{
	if(!m_pGuild || !m_pEntity) return;

	int pos = GetGuildPos();
	if(pos < smGuildAssistLeader)
	{
		m_pEntity->SendTipmsgFormatWithId(tmNoRight, tstUI);
		return;
	}
	BYTE nType = 0;
	packet >> nType;
	int nMax = GetLogicServer()->GetDataProvider()->GetGuildConfig().nBuildNum;
	if(nType > nMax)
	{
		m_pEntity->SendTipmsgFormatWithId(tmDataError, tstUI);
		return;
	}

	int level = m_pGuild->GetGuildbuildingLev(nType);
	const GuildBuildCfg* pbuild =  GetLogicServer()->GetDataProvider()->GetGuildConfig().GetBuildCfgPtr(nType, level+1);
	if(pbuild != NULL)
	{
		if(m_pGuild->GetCoin() < pbuild->nCost)
		{
			m_pEntity->SendTipmsgFormatWithId(tmGuildCoinLimit, tstUI);
			return;
		}
		if(m_pGuild->GetGuildbuildingLev(gbtMainHall) < pbuild->nLimit)
		{
			m_pEntity->SendTipmsgFormatWithId(tmGuildMainBuildLvLimit, tstUI, pbuild->nLimit);
			return;
		}
		m_pGuild->ChangeGuildCoin(-(pbuild->nCost));
		m_pGuild->ChangeGuildBuildingLev(nType, 1);
		sendGuilBuilds();
	}
	else
	{
		m_pEntity->SendTipmsgFormatWithId(tmGuildBuildLvLimit, tstUI);
		return;
	}
	

}



 void CGuildSystem::SendGuildDonateInfo()
 {
	if(!m_pEntity) return;

	if(!m_pGuild ) return;

	CActorPacket ap;
	CDataPacket& dataPack = m_pEntity->AllocPacket(ap);
	dataPack <<(BYTE)enGuildSystemID << (BYTE)sGuildDonateInfo;

	CGuildProvider& guildCfg = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	int nSize = guildCfg.m_nDonate.size();
	dataPack << (BYTE)nSize;
	if(nSize > 0)
	{
		std::map<int, DonateCfg>::iterator  it = guildCfg.m_nDonate.begin();
		for(; it != guildCfg.m_nDonate.end(); it++)
		{
			dataPack << (BYTE)(it->first);
			dataPack <<(BYTE)(m_pEntity->GetStaticCountSystem().GetStaticCount(it->second.nStaticCountType));
		}
	}

	ap.flush();


 }

VOID CGuildSystem::SendGuildWarList()
{
	if(!m_pEntity) return;
	if(!m_pGuild) return;

	m_pGuild->SendWarGuildList(m_pEntity);
}

VOID CGuildSystem::SendGuildEventLog()
{
	if(!m_pEntity) return;
	if(!m_pGuild) return;

	m_pGuild->SendGuildEventLog(m_pEntity);
}

void CGuildSystem::SendSbkInfo()
{
	if(!m_pEntity) return;
	CActorPacket ap;
	CDataPacket& data = ((CActor*)m_pEntity)->AllocPacket(ap);
	data << (BYTE)GetSystemID() << (BYTE)sGuildGetSbkInfo;
	const char* guildName = GetGlobalLogicEngine()->GetGuildMgr().getSbkGuildName();
	const char* leaderName = GetGlobalLogicEngine()->GetGuildMgr().getSbkGuildLeaderName();
	data.writeString(leaderName);
	data.writeString(guildName);
	ap.flush();
}


bool CGuildSystem::checkSbkGuildSeniorofficials()
{
	if(!m_pEntity) return false;
	if(!m_pGuild) return false;
	if(m_pGuild->GetGuildId() == GetGlobalLogicEngine()->GetGuildMgr().GetCityOwnGuildId())
	{
		int nMyPos = GetGuildPos();		//本人职位
		if(nMyPos >= smGuildAssistLeader)
		{
			return true;
		}
	}
	return false;
}
