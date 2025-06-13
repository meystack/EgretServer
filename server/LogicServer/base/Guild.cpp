 #include "StdAfx.h"
#include "GuildManger.h"
#include "../base/Container.hpp"
using namespace jxInterSrvComm::DbServerProto;

//CGuild::GuildInterMsgList::LinkNodeMgr* CGuild::GuildInterMsgList::g_LinkNodeMgr;

/********************以下是CGuild相关*****************************
仅第一次初始化，以后会在 CGuildComponent::LoadGuildListResult() 中取数据库数据进行设置
*/
CGuild::CGuild():m_WarList(),m_WarListHistory()
{
	CGuildProvider& pGuildConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();

	m_nGid = 0;
	m_nRank = 0;
	m_nLevel = 1;		//初始行会等级为1（不要使用）
	m_nFanrong = 0;
	m_nZytype = 0;
	m_sGuildname[0] = 0;
	// m_sFoundname[0] = 0;
	m_sLeaderName[0] = 0;
	m_sQqGroupId[0] = 0;
	m_sYyGroupId[0] = 0;
	m_sInMemo[0] = 0;
	m_sOutMemo[0] = 0;
	m_sYyType = 0;
	m_sYyGrilCompere[0] = 0;
	m_sYyMemo[0] = 0;
	m_sNextUpdateInMemo[0] = 0;
	m_nNewInMemoUpdateTime = 0;
	m_boStatus = true;
	m_boUpdateTime = false;
	m_nJoinMsgCount = 0;
	m_nSecLeaderCount = 0;
	m_GuildCoin = 0;
	m_nDailyGuildCoinDonated = 0; 
	m_GuildYs = 0;
	m_EventCount = 0;
	m_GuildTkCount = 0;
	m_UseGuildBoss = false;
	memset(m_EventRecord,0,sizeof(m_EventRecord));
	m_UiounMsgList.clear();
	m_GuildDepotBag.clear();
	m_GuildDepotRecord.clear();
	m_bSaveDepotRecord = false;
	m_nBidCoin = 0;
	m_CreateTime = 0;
	m_AddMemberFlag = 0;		//默认不自动加入
	ZeroMemory(m_GuildTitles,sizeof(m_GuildTitles));
	m_TileUpdateTime = 0;
	m_ToxicFreshTimes = 0;
	m_KillToxicNum = 0;
	m_GuildDartHandle = 0;
	m_GuildBiaoCheExpriedTime = 0;
	m_ActorOffLine.clear();
	m_ActorOnLine.clear();
	m_GuildMemGxList.empty();
	m_GuildBuildList.clear();

	m_MainHallLev = 1;			//初始行会主殿等级为1，即为行会等级，m_nLevel不要使用
	m_GuildShopLev = 1;
	m_ExerciseRoomlv = 1;
	m_AssemblyHallLv = 1;

	m_GuildChangleTimes = 0;
	m_ChanlgeLeftTimes = 0;
	m_GuildBossLevel = 0;

	m_MoonBoxLevel = 1;
	m_UpLeftTime = 0;

	//m_nAutoSignUp = 0;
	m_SignUpFlag = 0;
	m_BossChanllengeHandle = 0;
	m_btTechLevel = 1;
	m_btDragonThunderLevel = 1;
	m_nThunderPower = 0;
	m_nFillThunderTimes = 0;
	m_nCallTime = 0;

	m_nAutoJoinLevel   = 1;		//允许自动加入行会的最低等级，初始为50
	m_nUpgradeItemNum1 = 0;
	m_nUpgradeItemNum2 = 0;
	m_nUpgradeItemNum3 = 0;

	//行会神树和行会任务(不使用)
	m_nGuildTreeLevel	= 1;			//初始行会神树等级
	m_nGuildTreeDegree	= 0;			//初始行会神树进度，即神树精华
	m_nGuildFruitLevel	= 1;			//初始行会神树果实等级（品质）
	m_nGuildFruitDegree	= 0;			//初始行会神树果实进度（亮灯）
	m_nGuildFruitNum	= GetGuildFruitNumByFruitlevel(m_nGuildTreeLevel);			//初始行会神树果实进度数量
	m_nGuildTreeBackCoin = 0;			//初始行会神树回馈的资金
	m_nGuildTreeFruitTime	= 0;		//行会神树进入结果期期的时间

	m_nGuildTaskId		= 0;			//行会任务ID
	m_nGuildTaskSche	= 0;			//行会任务进度

	//攻城战指令(不使用)
	m_nGuildSiegeCmdIdx			= 0;
	m_nGuildSiegeCmdMsgIdx		= 0;
	m_nGuildSiegeCmdSceneId		= 0;
	m_nGuildSiegeCmdPosX		= 0;
	m_nGuildSiegeCmdPosY		= 0;
	m_nGuildSiegeCmdTime		= 0;
	m_nCertification =  0;
	
	m_CallInfo=std::make_tuple(0,0,0,0,0);
	//m_nExchangeCheckCircleMin   = pGuildConfig.m_GuildDepot.nExchangeNeedCheckCircleMin;		//直接领取装备的最小转数
	//OutputMsg(rmWaning,_T("guildId = %d, m_nExchangeCheckCircleMin = %d"), m_nGid,m_nExchangeCheckCircleMin);
}

void CGuild::Destroy()
{
	for(INT_PTR i= m_GuildDepotBag.count()-1;i >=0; i--)
	{
		GetGlobalLogicEngine()->DestroyUserItem(m_GuildDepotBag[i].nUserItem);
	}
	m_GuildDepotBag.empty();
	m_GuildDepotRecord.empty();
	m_UiounMsgList.empty();
	m_WarList.empty();

	m_ActorOffLine.clear();
	m_ActorOnLine.clear();
}

CGuild::~CGuild()
{
	Destroy();
}

//新行会初始化
VOID CGuild::InitGuild(CActor* pActor)
{
	if (pActor == NULL) return;

	AddMemberToList(pActor,smGuildLeader);//默认是帮主

	// _asncpytA(m_sFoundname,pActor->GetEntityName());

	//初始化行会资金
	m_GuildCoin =  GetLogicServer()->GetDataProvider()->GetGuildConfig().nAwardGuildMoney;
	m_WarList.clear();
	// m_AddMemberFlag = 0;		//默认不自动加入
	//SecLeaderlist.clear();

	RefreshGuildTask(true);		//刷新行会任务
	int nNum =  GetLogicServer()->GetDataProvider()->GetGuildConfig().nBuildNum;
	for(int i = 1; i <= nNum; i++)
	{
		TAGGUILDBUILD build;
		build.nType = i;
		build.nLevel = 0;
		m_GuildBuildList.add(build);
	}
}

int CGuild::AddJoinApply(CActor *pActor)
{
	if(!pActor)
	{
		return enApplyJoin_UnKnown;
	}
	
	if (pActor->OnGetIsTestSimulator()
		|| pActor->OnGetIsSimulator() )
	{
		return enApplyJoin_UnKnown;
	}
	unsigned int nActorGuildId = pActor->GetProperty<unsigned int>(PROP_ACTOR_GUILD_ID);
	if( nActorGuildId > 0 )
	{
		return enApplyJoin_HasGuild;
	}

	unsigned int nActorId = pActor->GetId();
	
	std::map<unsigned int, ApplyList>::iterator it = m_ApplyList.find(nActorId);
	if(it != m_ApplyList.end())
	{
		return enApplyJoin_HasSame;
	}

	ApplyList apply;
	apply.nActorId	= nActorId;
	apply.nSex		= (int)pActor->GetProperty<int>(PROP_ACTOR_SEX);
	apply.nLevel		= (int)pActor->GetProperty<int>(PROP_CREATURE_LEVEL);
	apply.nCircle		=  (int)pActor->GetProperty<int>(PROP_ACTOR_CIRCLE);
	apply.nJob		= (int)pActor->GetProperty<int>(PROP_ACTOR_VOCATION);
	apply.nVip		= (int)pActor->GetProperty<int>(PROP_ACTOR_SUPPER_PLAY_LVL);
	_asncpytA(apply.szName, pActor->GetEntityName());
	m_ApplyList.insert( std::make_pair(nActorId, apply) );
	SendActorApplyToGuild();
	//发送消息到数据服务器--添加
	DealGuildApply(m_nGid, nActorId, 1);

	return enApplyJoin_NoErr;
}


int CGuild::DealApplyByActorId(unsigned int nActorId, int nResult)
{	
	std::map<unsigned int, ApplyList>::iterator it = m_ApplyList.find(nActorId);
	if(it == m_ApplyList.end())
	{
		return enCheckJoin_NoApply;
	}

	int nMemberCount	= (int)GetMemberCount();
	int nMemberCountMax = (int)GetMaxMemberCount();
	if( nResult > 0 && nMemberCount >= nMemberCountMax )
	{
		return enCheckJoin_MemberLimit;
	}

	bool nGid = GetGlobalLogicEngine()->GetGuildMgr().CheckIsJoinGuild( nActorId );
	if(nGid != 0 && nResult)
	{
		return enCheckJoin_HasGuild;
	}
	if( nResult > 0 )		//同意加入行会
	{
		CActor *pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID( nActorId );
		if( pActor )		//申请人在线
		{
			unsigned int nActorGuildId = pActor->GetProperty<unsigned int>(PROP_ACTOR_GUILD_ID);
			if( nActorGuildId > 0 )			//已经加入某行会了
			{
				return enCheckJoin_HasGuild;
			}
			if( GetGlobalLogicEngine()->GetGuildMgr().GetCityOwnGuildId() == m_nGid)
				pActor->GetAchieveSystem().ActorAchievementComplete(nAchieveJionSBKGuild, 1);
			
			AddMemberToList( pActor );						//玩家正式加入行会
			pActor->SendTipmsgFormatWithId(tmAgreeJoin, tstUI);
			CActorPacket ap;
			CDataPacket & dataPacket = pActor->AllocPacket(ap);

			dataPacket << (BYTE)enGuildSystemID << (BYTE)sApplyReslut;
			dataPacket << (unsigned int)m_nGid;
			ap.flush();
		}
		else
		{
			AddMemberToList( &(it->second));
		}

		LPCTSTR sTitle = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmMailGuild);
		char sContent[1024]={0};
		LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmAddGuidMail);
		sprintf_s(sContent, sizeof(sContent), sFormat, ( char *)(GetGuildName()));
		CMailSystem::SendMail(nActorId, sTitle, sContent, NULL, 0);
		
	}
	// UpdateActorShowName();
	m_ApplyList.erase(it);
	//发送消息到数据服务器
	DealGuildApply(m_nGid, nActorId, 2);

	return enApplyJoin_NoErr;
}

 VOID CGuild::DealGuildApply(unsigned int nGid, unsigned int nactorId, int type)
{
	CDataPacket& DataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcDealGuildApply);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId << nServerId;
	DataPacket << (unsigned int)nGid;
	DataPacket << (unsigned int)nactorId;
	DataPacket << (int)type;
	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
}

VOID CGuild::AddMemberToList(CActor* pActor,INT_PTR nGuildPos)
{
	if (!pActor) 
	{ 
		return;
	}
		
	if (pActor->OnGetIsTestSimulator()
		|| pActor->OnGetIsSimulator() )
	{ 
		return;
	}
	if (GetMaxMemberCount() < (GetMemberCount()+1))
	{
		pActor->SendTipmsgFormatWithId(tmMaxMemberCount, ttTipmsgWindow);
		return;
	}
	unsigned int nActorid = 0;
	ActorCommonNode Node;
	nActorid = pActor->GetId();
	pActor->GetGuildSystem()->SetGuild(this);
	pActor->GetGuildSystem()->SendGuildTitleList();
	//设置帮派相关的属性
	pActor->SetProperty<unsigned int>(PROP_ACTOR_GUILD_ID,m_nGid);
	pActor->SetProperty<unsigned int>(PROP_ACTOR_GUILD_LEVEL,GetGuildbuildingLev(gbtMainHall));
	pActor->GetGuildSystem()->SetGuildPos(nGuildPos);

	CGuildSystem* gs = pActor->GetGuildSystem();
	gs->nTk = 0;
	gs->nZzNext = CGuildSystem::GetZZNext();
	gs->SetGuildPos((BYTE)nGuildPos);

	
	UpdateActorData(pActor,&Node);			//将玩家数据存储在行会中
	Node.BasicData.nJoinTime = GetGlobalLogicEngine()->getMiniDateTime();;		//加入行会的时间
	gs->SetJoinTime( Node.BasicData.nJoinTime );
		
	Node.hActorHandle = pActor->GetHandle();
	m_ActorOffLine.push_back(Node);
	m_ActorOnLine.linkAtLast(pActor->GetHandle());
	if (nGuildPos == smGuildLeader)
	{
		m_nLeaderId = Node.BasicData.nActorId;
		_asncpytA(m_sLeaderName,Node.sActorName);
	}

	//发送消息到数据服务器
	AddMember(Node);
	GetGlobalLogicEngine()->GetGuildMgr().AddJoinGuildid(Node.BasicData.nActorId, m_nGid);

	if(gs->m_TodayGx > 0)
	{
		pActor->GetGuildSystem()->SendTodayGetGuildgx();
	}

	if(nGuildPos != smGuildLeader)
		SendAddOrLeftMember(pActor,(BYTE)1,nActorid,Node.sActorName);

	char sText[1024];
	LPCTSTR sFormat;
	sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpNewMemBerTalk);
	sprintf_s(sText, sizeof(sText), sFormat);

	if(nGuildPos != smGuildLeader)
		pActor->GetGuildSystem()->SendWelcomeToNewMember(sText);

	if(GetGlobalLogicEngine()->GetGuildMgr().IsGuildSiegeStart())	//开始沙巴克战
	{
		GetGlobalLogicEngine()->GetGuildMgr().UpdateGuildMemberName((CActor*)pActor);
		UpdateOtherNameClr_GiveSelf(((CActor*)pActor)->GetHandle());
	}

	pActor->GetGuildSystem()->m_GuildGx = 0;

	//记录日志
	if(GetLogicServer()->GetLocalClient())
	{
	}

	AddEventRecord( "", enGuildEvent_MemberJoin, 0, 0, 0, (char *)(Node.sActorName),NULL );
}
 
VOID CGuild::SetMemberOnline(CActor* pActor)
{
	if (!pActor)
	{ 
		return;
	}
	
	if (pActor->OnGetIsTestSimulator()
		|| pActor->OnGetIsSimulator() )
	{ 
		return;
	}
	if (GetMaxMemberCount() < (GetMemberCount()+1))
	{
		pActor->SendTipmsgFormatWithId(tmMaxMemberCount,ttTipmsgWindow);
		return;
	}

	unsigned int nActorid = 0;
	ActorCommonNode* Node = NULL;

	nActorid = pActor->GetId();
	Node = FindMember(nActorid);
	if(Node == NULL)
		return;
	
	pActor->GetGuildSystem()->SetGuild(this);
	pActor->GetGuildSystem()->SendGuildTitleList();
	//设置帮派相关的属性
	pActor->SetProperty<unsigned int>(PROP_ACTOR_GUILD_ID,m_nGid); 
	pActor->GetGuildSystem()->SetGuildPos(Node->BasicData.nType);
	pActor->SetProperty<unsigned int>(PROP_ACTOR_GUILD_LEVEL,GetGuildbuildingLev(gbtMainHall));
		
	Node->hActorHandle = pActor->GetHandle();
	m_ActorOnLine.linkAtLast(pActor->GetHandle());

	// if(GetGlobalLogicEngine()->GetGuildMgr().IsGuildSiegeStart())	//开始沙巴克战
	// {
	// 	GetGlobalLogicEngine()->GetGuildMgr().UpdateGuildMemberName((CActor*)pActor);
	// 	UpdateOtherNameClr_GiveSelf(((CActor*)pActor)->GetHandle());
	// }

	pActor->GetGuildSystem()->m_GuildGx = 0;

	// if(m_nGid == GetGlobalLogicEngine()->GetGuildMgr().m_GuildId)
	// {
	// 	const CGuildProvider& pConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	// 	GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction(pActor,"OnGuildSiegeMasterStatueCheckBuff");
	// }
}

VOID CGuild::AddMemberToList(ApplyList* pApply,INT_PTR nGuildPos)
{
	if(!pApply) 
	{ 
		return;
	}
 	
	if (GetMaxMemberCount() < (GetMemberCount()+1))
	{ 
		return;
	}
	unsigned int nActorid = 0;
	ActorCommonNode Node;
	nActorid = pApply->nActorId;
	UpdateActorData(pApply,&Node);			//将玩家数据存储在行会中
	Node.BasicData.nJoinTime = GetGlobalLogicEngine()->getMiniDateTime();;		//加入行会的时间
		
	m_ActorOffLine.push_back(Node);
	if (nGuildPos == smGuildLeader)
	{
		m_nLeaderId = Node.BasicData.nActorId;
		_asncpytA(m_sLeaderName,Node.sActorName);
	}
	// SaveGuildMember(Node);
	AddMember(Node);
	GetGlobalLogicEngine()->GetGuildMgr().AddJoinGuildid(Node.BasicData.nActorId, m_nGid);
	//记录日志
	if(GetLogicServer()->GetLocalClient())
	{
	}
	AddEventRecord( "", enGuildEvent_MemberJoin, 0, 0, 0, (char *)(Node.sActorName),NULL );
}

/*定期轮询，每隔一段时间检查是否宣战关系到期
*/
VOID CGuild::CheckWarList()
{
	//检查有没有帮派战相关的状态改变	
	bool boWarChange = false;
	INT_PTR nCount = m_WarList.count();
	UINT nNow = GetGlobalLogicEngine()->getMiniDateTime();
	int nDec = GetLogicServer()->GetDataProvider()->GetGuildConfig().m_GuildRelation.nDeclareLastTime;

	for (INT_PTR i = 0; i < nCount;i++)
	{
		WarRelationData* war = &(m_WarList[i]);
		if(war == NULL) continue;

		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(m_WarList[i].nGuildId);
		if(pGuild == NULL) break;

		//宣战结束
		if(m_WarList[i].m_WarData.nNext > 0 && (unsigned int)m_WarList[i].m_WarData.nNext <= nNow)
		{
			SendGuildMsg(0,tpGuildWarEndTips,ttChatWindow,m_sGuildname,pGuild->m_sGuildname,m_sGuildname,pGuild->m_sGuildname,m_WarList[i].m_WarData.nPkCount,pGuild->m_sGuildname,m_sGuildname,m_WarList[i].m_WarData.nDieCount);
			m_WarList[i].m_WarData.nNext = 0;
			m_WarList[i].m_WarData.nState = 0;
			m_WarList[i].m_WarData.nPkCount = 0;
			m_WarList[i].m_WarData.nDieCount = 0;
		}
	}
	if (boWarChange)
	{
		//下发新的敌对帮派列表，同时设置保存
		m_boUpdateTime = true;
		SendWarGuildListToAllMember();
		NotifyUpdateGuildInfo();
	}
}
VOID CGuild::SetOutMemo(LPCSTR sOut,bool boSave)
{
	_asncpytA(m_sOutMemo,sOut);
	if (boSave)
	m_boUpdateTime = boSave;
}
VOID CGuild::SetInMemo(LPCSTR sIn,bool boSave) 
{
	_asncpytA(m_sInMemo,sIn);
	if (boSave)
	m_boUpdateTime = boSave;
}

VOID CGuild::SetGuildGroupMemo(LPCSTR sIn,bool boSave)
{
	_asncpytA(m_sGroupMemo,sIn);
	if (boSave)
		m_boUpdateTime = boSave;
}

/*设置行会等级
引擎启动时 CGuildComponent::AddGuildToList() 和
行会升级时 CGuild::ChangeGuildBuildingLev() 调用
boUpgrade:true-升级，false-强行指定等级（服务器启动或者降级）
*/
VOID CGuild::SetGuildLevel(int value,bool boSave,bool boRank, bool boUpgrade) 
{
	m_nLevel = value;
	if (boSave)
	{
		m_boUpdateTime = boSave;
	}
	if (boRank)
	{
		GetGlobalLogicEngine()->GetGuildMgr().AdjustRank(this);		
	}

	if( boUpgrade )		//行会等级提升
	{
		OnGuildLevelUp();
	}
}

VOID CGuild::SetFr(int value,bool boSave,bool boRank)
{
	if(value < 0)
	{
		value = 0;
		m_boUpdateTime = true;
	}

	m_nFanrong = value;
	if (boSave)
	{
		m_boUpdateTime = boSave;
	}
	if (boRank)
	{
		//GetGlobalLogicEngine()->GetGuildMgr().AdjustRank(this);	
	}
}

VOID CGuild::Save()
{
	m_boUpdateTime = false;
	CDataPacket& DataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveGuild);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId << nServerId;
	DataPacket << (unsigned int)m_nGid;
	DataPacket.writeString(m_sLeaderName); //直接处理空
	DataPacket << (WORD)m_nLevel;
	DataPacket << (int)m_GuildCoin;				//行会资金
	DataPacket.writeString(m_sOutMemo);
	DataPacket.writeString(m_sInMemo);
	DataPacket << (int)m_nBidCoin;
	DataPacket << (int)m_AddMemberFlag;
	// DataPacket.writeString(m_sGroupMemo);
	// DataPacket << (int)m_SignUpFlag;
	DataPacket << (int)m_MainHallLev;
	DataPacket << (int)m_ExerciseRoomlv;
	DataPacket << (int)m_AssemblyHallLv;

	DataPacket << (int)m_nAutoJoinLevel;		//自动加入行会的等级要求
	DataPacket << (int)m_nCertification;

	size_t pos = DataPacket.getPosition();
	DataPacket << (int)0;//先填充0，后面会填入正确的数量
	int nWarCount = 0;

	UINT nNow = GetGlobalLogicEngine()->getMiniDateTime();

	for (int i = 0; i < m_WarList.count();i++)
	{
		WarRelationData& war = m_WarList[i];
		DataPacket << (unsigned int)war.nGuildId;
		DataPacket << (int)war.m_WarData.nRelationShip;
		DataPacket << (int)war.m_WarData.nState;
		DataPacket << (int)war.m_WarData.nPkCount;
		DataPacket << (int)war.m_WarData.nDieCount;
		DataPacket << (int)war.m_WarData.nNext;
		nWarCount++;
	}
	int* pCount = (int*)DataPacket.getPositionPtr(pos);
	*pCount = nWarCount;

	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
}

BOOL CGuild::AddInterMsg(InterMsg Msg)
{
	//如果是邀请玩家，同一个玩家不可以邀请多次
	CLinkedNode<InterMsg> *pNode;
	CLinkedListIterator<InterMsg> it(m_InterMsgList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		InterMsg& Tempmsg = pNode->m_Data;
		if (Tempmsg.nMsgId == Msg.nMsgId && Tempmsg.Node.BasicData.nActorId == Msg.Node.BasicData.nActorId && Tempmsg.hSrcHandle == Msg.hSrcHandle)
		{
			return FALSE;
		}
	}
	m_InterMsgList.linkAtLast(Msg);
	if (Msg.nMsgId == imJoin)
		m_nJoinMsgCount++;
	return TRUE;
}

BOOL CGuild::FindAndDeleteInterMsg(InterMsg& Msg)
{
	BOOL result = FALSE;
	CLinkedNode<InterMsg> *pNode;
	CLinkedListIterator<InterMsg> it(m_InterMsgList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		InterMsg& Tempmsg = pNode->m_Data;		// || strcmp(Tempmsg.szName,Msg.szName)==0
		if (Tempmsg.nMsgId == Msg.nMsgId && 
			(Tempmsg.Node.BasicData.nActorId == Msg.Node.BasicData.nActorId) &&	
			Tempmsg.hSrcHandle == Msg.hSrcHandle)
		{
			if (Tempmsg.nMsgId == imJoin)
				m_nJoinMsgCount--;
			it.remove(pNode);
			result = TRUE;
		}
	}
	return result;
}

BOOL CGuild::FindAndInterMsg(InterMsg& Msg)
{
	BOOL result = FALSE;
	CLinkedNode<InterMsg> *pNode;
	CLinkedListIterator<InterMsg> it(m_InterMsgList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		InterMsg& Tempmsg = pNode->m_Data;		// || strcmp(Tempmsg.szName,Msg.szName)==0
		if (Tempmsg.nMsgId == Msg.nMsgId && 
			strcmp(Tempmsg.szName,Msg.szName)==0 &&	
			Tempmsg.hSrcHandle == Msg.hSrcHandle)
		{
			if (Tempmsg.nMsgId == imJoin)
				m_nJoinMsgCount--;
			it.remove(pNode);
			result = TRUE;
		}
	}
	return result;
}

/*
玩家加入行会，下线时调用
*/
VOID CGuild::UpdateActorData(CActor* pActor,ActorCommonNode* pNode)
{
	if (pActor == NULL || pNode == NULL) return;
	CGuildSystem* gs = pActor->GetGuildSystem();
	pNode->BasicData.nActorId = pActor->GetProperty<unsigned int>(PROP_ENTITY_ID);	
	pNode->BasicData.nGuildGX = gs->GetTotalGuildGx();	//贡献值
	pNode->BasicData.nSex = pActor->GetProperty<int>(PROP_ACTOR_SEX);
	pNode->BasicData.nLevel = pActor->GetProperty<int>(PROP_CREATURE_LEVEL);
	pNode->BasicData.nCircle = pActor->GetProperty<int>(PROP_ACTOR_CIRCLE);
	pNode->BasicData.nJob = pActor->GetProperty<int>(PROP_ACTOR_VOCATION);
	pNode->BasicData.nType = (BYTE)(gs->GetGuildPos());
	pNode->BasicData.nTK = gs->nTk;	//堂口
	pNode->BasicData.nBattleValue = pActor->GetActorAttackValueMax(); //pActor->GetProperty<int>(PROP_ACTOR_BATTLE_POWER);
	//pNode->BasicData.nZyGX = 0;	//阵营贡献,这个值应该从阵营系统中读出
	pNode->BasicData.nZzNext = gs->nZzNext;
	pNode->BasicData.nZjNext = gs->nWealNext;
	pNode->BasicData.nModelId = pActor->GetProperty<int>(PROP_ENTITY_MODELID);
	pNode->BasicData.nWeaponId = pActor->GetProperty<int>(PROP_ACTOR_WEAPONAPPEARANCE);
	pNode->BasicData.nSwingId = pActor->GetProperty<int>(PROP_ACTOR_SWINGAPPEARANCE);
	pNode->BasicData.nTime = pActor->GetLoginTime();
	pNode->BasicData.nVipGrade = pActor->GetProperty<unsigned int>(PROP_ACTOR_SUPPER_PLAY_LVL); 
	_asncpytA(pNode->sActorName,pActor->GetEntityName());
}



/*
玩家加入行会，下线时调用
*/
VOID CGuild::UpdateActorData(ApplyList* pApply,ActorCommonNode* pNode)
{
	if (pApply == NULL || pNode == NULL) return;
	int nNow = GetGlobalLogicEngine()->getMiniDateTime();
	pNode->BasicData.nActorId = pApply->nActorId;
	pNode->BasicData.nSex = pApply->nSex;
	pNode->BasicData.nLevel = pApply->nLevel;
	pNode->BasicData.nJob = pApply->nJob;
	pNode->BasicData.nCircle = pApply->nCircle;
	pNode->BasicData.nTime = nNow;
	_asncpytA(pNode->sActorName,pApply->szName);
}


VOID CGuild::UpdateActorData(CJoinGuildApply* pApply,ActorCommonNode* pNode)
{
	if (pApply == NULL || pNode == NULL) return;
	pNode->BasicData.nActorId = pApply->nActorId;
	pNode->BasicData.nSex = pApply->nSex;
	pNode->BasicData.nLevel = pApply->nLevel;
	pNode->BasicData.nJob = pApply->nJob;
	pNode->BasicData.nCircle = pApply->nCircle;
	_asncpytA(pNode->sActorName,pApply->szActorName);
	pNode->BasicData.nTime = pApply->nLogintime;
	pNode->BasicData.nType = 0;
}

void CGuild::BroadCast(LPCVOID pData,SIZE_T size, int nLevel, int nCircle)
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			CActor* toActor = (CActor*)pEntity;
			if (toActor->CheckLevel(nLevel, nCircle))
			{
				CActorPacket pack;
				CDataPacket &data = toActor->AllocPacket(pack);
				data.writeBuf(pData,size);
				pack.flush();
			}
		}
	}
}

void CGuild::BroadCastExceptSelf(LPCVOID pData,SIZE_T size,unsigned int nMyId)
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			CActor* pActor = (CActor*)pEntity;
			if (pActor->GetId() != nMyId )
			{
				CActor* toActor = (CActor*)pEntity;
				CActorPacket pack;
				CDataPacket &data = toActor->AllocPacket(pack);
				data.writeBuf(pData,size);
				pack.flush();
			}
		}
	}
}


void CGuild::BroadCastByMsgId(int nMsgId)
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			CActor* toActor = (CActor*)pEntity;
			CActorPacket pack;
			CDataPacket &data=toActor->AllocPacket(pack);
			data << (BYTE)enGuildSystemID << (BYTE)nMsgId;
			pack.flush();
		}
	}
}

/*在行会频道公告消息
目前客户端要求必须发送者为一个在线的玩家
*/
void CGuild::BroadCastMsgInGuildChannel(char* szMsg, int nLevel, int nCircle)
{
	// char buff[1024];
	// LPCTSTR szName = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildChannelName);

	// CDataPacket outPack(buff, sizeof(buff)); //下发的数据包
	// GetGlobalLogicEngine()->GetChatMgr().Filter(szMsg) ;
	// outPack << (BYTE) enChatSystemID << (BYTE)sSendChat;
	// outPack	<<(BYTE)ciChannelGuild;
	// outPack.writeString("");	//名字为空，表示系统发的
	// outPack.writeString(szMsg);
	// outPack << (BYTE)9;		//性别（非男非女）
	// outPack << (BYTE)0;		//标志
	// outPack << (BYTE)0;		//yy紫钻
	// outPack << (BYTE)0;		//转数
	// outPack << (BYTE)0;		//sbk职位
	// outPack << (BYTE)0;		//参数
	// BroadCast(outPack.getMemoryPtr(), outPack.getPosition(), nLevel, nCircle);
}

bool CGuild::SendWarUnionMsgToActor(unsigned int nGuildId, unsigned int nActorId,LPCSTR nGuildName)
{
	CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
	if(pGuild == NULL) return false;

	bool bResult = false;
	// CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	// CLinkedNode<EntityHandle> *pNode;
	// CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	// for (pNode = it.first(); pNode; pNode = it.next())
	// {
	// 	EntityHandle& hHandle = pNode->m_Data;
	// 	CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
	// 	if (pEntity && pEntity->GetType() == enActor)
	// 	{
	// 		CActor* toActor = (CActor*)pEntity;
	// 		if(toActor->GetGuildSystem()->GetGuildPos() >= smGuildAssistLeader)
	// 		{
	// 			CActorPacket pack;
	// 			CDataPacket &DataPacket = toActor->AllocPacket(pack);
	// 			DataPacket << (BYTE)enGuildSystemID << (BYTE)sSetGuildRelation;
	// 			DataPacket << nGuildId;
	// 			DataPacket << nActorId;
	// 			DataPacket.writeString(nGuildName?nGuildName:"");
	// 			if(pGuild->m_pLeader)
	// 			{
	// 				DataPacket.writeString(pGuild->m_pLeader->sActorName?pGuild->m_pLeader->sActorName:"");
	// 			}
	// 			else
	// 			{
	// 				DataPacket.writeString("");
	// 			}

	// 			DataPacket << (int)pGuild->GetMemberCount();
	// 			DataPacket << (int)pGuild->GetMaxMemberCount();
	// 			pack.flush();

	// 			bResult = true;
	// 		}
	// 	}
	// }

	return bResult;
}

void CGuild::SendActorApplyToGuild()
{
	// CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
	// if(pGuild == NULL) return;

	//if( m_nJoinMsgCount <= 0 )		//没有申请入帮的请求
	// if( !GetGlobalLogicEngine()->GetGuildMgr().HasJoinGuildApply( nGuildId ) )
	// {
	// 	return;
	// }
	int nState = 2;
	if(ApplySize() >  0)
	{
		nState = 1;
	}

	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			CActor* toActor = (CActor*)pEntity;
			if (toActor->OnGetIsTestSimulator()
				|| toActor->OnGetIsSimulator())
			{
				continue;
			}

			if(toActor->GetGuildSystem()->GetGuildPos() >= smGuildTangzhu)		//官员和以上职位都可以审核
			{
				CActorPacket pack;
				CDataPacket &DataPacket = toActor->AllocPacket(pack);
				DataPacket << (BYTE)enGuildSystemID << (BYTE)sGuildRedPoint;
				DataPacket <<(BYTE)2; // 1 红点 2：黄点
				DataPacket <<(BYTE)4; //1:行会信息 2:行会管理, 3:成员列表, 4:入会审批,5:行会列表,
				DataPacket <<(BYTE)nState;
				pack.flush();
			}
		}
	}
}

bool CGuild::SendOpenBuildFormToClient(CActor* pActor)
{

	return true;
}

//通知客户端重新请求行会列表
void CGuild::UpdateGuildList()
{
	// CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	// CLinkedNode<EntityHandle> *pNode;
	// CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	// for (pNode = it.first(); pNode; pNode = it.next())
	// {
	// 	EntityHandle& hHandle = pNode->m_Data;
	// 	CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
	// 	if (pEntity && pEntity->GetType() == enActor)
	// 	{
	// 		CActor* toActor = (CActor*)pEntity;
	// 		CActorPacket pack;
	// 		CDataPacket &DataPacket = toActor->AllocPacket(pack);
	// 		DataPacket << (BYTE)enGuildSystemID << (BYTE)sReturnWarRelation;
	// 		pack.flush();
	// 	}
	// }
}

void CGuild::AddGuildBuff(int nBuffType,int nGroupID,double dValue,int nTimes, int nInterval,char * buffName,bool timeOverlay, void *pGiver, int param)
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			CActor* toActor = (CActor*)pEntity;
			EntityHandle giverHdl = EntityHandle();

			if (pGiver) 
				giverHdl = ((CEntity *)pGiver)->GetHandle();
			 /*
               此函数已不用，如以后需要用buff，请使用下面方法
               Buff配置文件:data/config/global/StdBuff.txt
               调用BuffId: ((CActor *)pEntity)->GetBuffSystem()->Append(nBuffId) 
             */
			//toActor->GetBuffSystem()->Append(nBuffType,nGroupID,dValue,nTimes,nInterval,buffName,param,timeOverlay, giverHdl);
		}
	}
}

void CGuild::SbkMasterOpenBuff(int nBuffType,int nGroupID,double dValue,int nTimes, int nInterval,char * buffName,bool timeOverlay, void *pGiver, int param)
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			CActor* toActor = (CActor*)pEntity;
			EntityHandle giverHdl = EntityHandle();

			if (pGiver) 
				giverHdl = ((CEntity *)pGiver)->GetHandle();
			 /*
               此函数已不用，如以后需要用buff，请使用下面方法
               Buff配置文件:data/config/global/StdBuff.txt
               调用BuffId: ((CActor *)pEntity)->GetBuffSystem()->Append(nBuffId) 
             */
			//toActor->GetBuffSystem()->Append(nBuffType,nGroupID,dValue,nTimes,nInterval,buffName,param,timeOverlay, giverHdl);
		}
	}
}

void CGuild::AddBuffToUnion(int nBuffType,int nGroupID,double dValue,int nTimes, int nInterval,char * buffName,bool timeOverlay, void *pGiver, int param)
{
	for(int i=0;i<m_WarList.count();i++)
	{
		WarRelationData& war= m_WarList[i];

		if(war.m_WarData.nRelationShip == 1)
		{
			CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(war.nGuildId);
			if(pGuild)
			{
				pGuild->SbkMasterOpenBuff(nBuffType,nGroupID,dValue,nTimes, nInterval,buffName,timeOverlay, pGiver,param);
			}
		}
	}
}

void CGuild::GuildGroupBroadCast(LPCVOID pData,SIZE_T size)
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			CActor* toActor = (CActor*)pEntity;
			CActorPacket pack;
			CDataPacket &data = toActor->AllocPacket(pack);
			data.writeBuf(pData,size);
			pack.flush();
		}
	}
}

void CGuild::SendAddOrLeftMember(CActor* pActor,BYTE nType,unsigned int nActorId,char* nName)
{
	if(nName == NULL)
	{ 
		return;
	}
	if(pActor == NULL)
	{ 
		return;
	}

	if (pActor->OnGetIsTestSimulator()
		|| pActor->OnGetIsSimulator() )
	{ 
		return;
	}
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			CActor* toActor = (CActor*)pEntity;
			unsigned int curActorId = toActor->GetProperty<unsigned int>(PROP_ENTITY_ID);
			if(toActor->GetGuildSystem()->GetGuildPtr() && curActorId != nActorId)
				toActor->GetGuildSystem()->SendGuildMemberAddOrLieve(pActor,nType,nActorId,nName);
		}
	}
}
VOID CGuild::GiveAchieve( INT_PTR nEventId,INT_PTR nParam1 )
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			CActor* toActor = (CActor*)pEntity;
			toActor->GetAchieveSystem().OnEvent(nEventId,nParam1);
		}
	}
}


VOID CGuild::SendGuildMsg(INT_PTR nPos,INT_PTR nTipmsgID,INT_PTR nType, ...)
{
	
	// LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(nTipmsgID);
	// CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	// CLinkedNode<EntityHandle> *pNode;
	// CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	// for (pNode = it.first(); pNode; pNode = it.next())
	// {
	// 	EntityHandle& hHandle = pNode->m_Data;
	// 	CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
	// 	if (pEntity && pEntity->GetType() == enActor)
	// 	{
	// 		CActor* toActor = (CActor*)pEntity;
	// 		if (toActor->GetGuildSystem()->GetGuildPos() >= nPos)
	// 		{
	// 			va_list args;
	// 			va_start(args, nType);
	// 			toActor->SendTipmsgWithArgs(sFormat,args,nType);
	// 			va_end(args);
	// 		}
	// 	}
	// }
	
}

VOID CGuild::SendGuildMsg( INT_PTR nPos,LPCSTR sMsg,INT_PTR nType /*= ttTipmsgWindow*/ )
{
	// CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	// CLinkedNode<EntityHandle> *pNode;
	// CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	// for (pNode = it.first(); pNode; pNode = it.next())
	// {
	// 	EntityHandle& hHandle = pNode->m_Data;
	// 	CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
	// 	if (pEntity && pEntity->GetType() == enActor)
	// 	{
	// 		CActor* toActor = (CActor*)pEntity;
	// 		if (toActor->GetGuildSystem()->GetGuildPos() >= nPos)
	// 		{
	// 			toActor->SendTipmsg(sMsg,nType);
	// 		}
	// 	}
	// }
}




VOID CGuild::SendGuildChatMsg(CActor* pActor, char* msg)
{
	// if(!pActor) return;
	// char buff[1024];
	// CDataPacket outPack(buff, sizeof(buff)); //下发的数据包

	// outPack << (BYTE) enChatSystemID << (BYTE)sSendChat <<(BYTE)ciChannelGuild;

	// outPack.writeString(""); //把自己的名字写上去
	// outPack.writeString(msg);

	// outPack << pActor->GetLevel();
	// Uint64 actorid = Uint64(pActor->GetHandle());
	// outPack <<actorid;
	// outPack << (BYTE)0;
	
	// GuildGroupBroadCast(outPack.getMemoryPtr(), outPack.getPosition());
}


VOID CGuild::FindAndSetHandle(unsigned int nActorid,EntityHandle hHandle)
{
	ActorCommonNode* pNode = FindMember(nActorid);
	if (pNode)
		pNode->hActorHandle = hHandle;
}

char* CGuild::FindGuildMemberName(unsigned int nActorid)
{
	ActorCommonNode* pNode = FindMember(nActorid);
	if (pNode)
		return pNode->sActorName;

	return NULL;
}

VOID CGuild::DeleteOnLineList(EntityHandle& hHandle)
{
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		if (pNode && pNode->m_Data == hHandle)
		{
			it.remove(pNode);
			break;
		}
	}
}

VOID CGuild::DeleteMember(unsigned int nActorid,CActor* pActor,ActorCommonNode* pNode,BOOL boType)
{
	if (!pActor && !pNode)
	{ 
		return;
	}
	 
	unsigned int nDeleteid = 0;//被开除的角色id
	int pos = 0;
	char sMember[32];
	memset(&sMember, 0, sizeof(sMember));
	if (pActor)
	{
		// if(m_nGid == GetGlobalLogicEngine()->GetGuildMgr().m_GuildId)
		// {
		// 	const CGuildProvider& pConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();
		// 	CGuildComponent& guildMgr =  GetGlobalLogicEngine()->GetGuildMgr();
		// 	int nCityPos = guildMgr.getCityPostionById(pActor->GetId());
		// 	if (nCityPos != stNoCityPos)
		// 	{
		// 		guildMgr.SetCityPos(nCityPos,0);
		// 	}
		// 	SendGuildSiegeCmdPanel( pActor, 0);			//关闭攻城战指令面板
		// }

		nDeleteid = pActor->GetProperty<unsigned int>(PROP_ENTITY_ID);
		//在线,则从帮派的在线列表中删除
		EntityHandle hHandle = pActor->GetHandle();
		DeleteOnLineList(hHandle);
		//发送通知消息
		// pActor->SendTipmsgFormatWithId(tmLeftGuild, tstUI,m_sGuildname);
		//设置其属性
		pActor->GetGuildSystem()->OnLeftGuild(boType);

		//pActor->GetGuildSystem()->SetGx(0);
		pActor->GetGuildSystem()->m_GuildGx = 0;

		pos = pActor->GetGuildSystem()->GetGuildPos();

		pActor->ResetShowName();
		//要移除特定的BUFF的Giver
		pActor->GetBuffSystem()->RemoveBuffGiver();
	}
	// else
	// {
	// 	if(boType)
	// 	{
	// 		char MsgBuff[MAX_MSG_COUNT];
	// 		CDataPacket packet(MsgBuff,sizeof(MsgBuff));
	// 		LPCTSTR sNoticeFormat;
	// 		char sText[1024] = {0};
	// 		sNoticeFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpLeftGuild);
	// 		sprintf_s(sText, sizeof(sText), sNoticeFormat, m_sGuildname);
	// 		packet.writeString(sText);
	// 		CMsgSystem::AddOfflineMsg(pNode->BasicData.nActorId, CMsgSystem::mtDeleteGuild, MsgBuff, packet.getPosition(), 0);
	// 	}
	// }
	if (pNode)
	{
		nDeleteid = pNode->BasicData.nActorId;
	}
	//从成员列表中删除
	std::vector<ActorCommonNode>::iterator it = m_ActorOffLine.begin();
	for(; it != m_ActorOffLine.end();it++)
	{
		ActorCommonNode& Node = *it;
		if( Node.BasicData.nActorId == nDeleteid )
		{
			_asncpytA(sMember,Node.sActorName);
			pos = Node.BasicData.nType;
			Node.BasicData.nTotalGx = 0;
			if( !boType )				//玩家主动离开
			{
				AddEventRecord( "", enGuildEvent_MemberLeft, 0, 0, 0, (char *)(Node.sActorName), NULL );
			}
			m_ActorOffLine.erase(it);
			break;
		}
	}

	//向数据服务器发送信息
	CDataPacket& DataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcDeleteMember);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId << nServerId;
	DataPacket << nDeleteid;
	DataPacket << nActorid;//操作人的id
	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);

	if (pos == smGuildAssistLeader)
	{
		m_nSecLeaderCount--;
		if (m_nSecLeaderCount < 0)
		{
			m_nSecLeaderCount = 0;
		}
	}else if (pos == smGuildTangzhu)
	{
		m_GuildTkCount--;
		if (m_GuildTkCount < 0)
		{
			m_GuildTkCount = 0;
		}
	}
	else if( pos == smGuildElite)
	{
		m_GuildEliteCount--;
		if( m_GuildEliteCount < 0)
			m_GuildEliteCount = 0;
	}

	if(GetLogicServer()->GetLocalClient())
	{
	}

	//通知帮派在线成员离开的成员信息
	SendAddOrLeftMember(pActor,(BYTE)2,nDeleteid,sMember);
}

ActorCommonNode* CGuild::FindMember(unsigned int nActorid)
{
	int nSize = m_ActorOffLine.size();
	for(int i = 0; i < nSize; i++)
	{
		ActorCommonNode& Node = m_ActorOffLine[i];
		if (Node.BasicData.nActorId == nActorid)
		{
			return & m_ActorOffLine[i];
		}
	}
	return NULL;
}

ActorCommonNode* CGuild::FindMemberByName(const char* nName)
{
	int nSize = m_ActorOffLine.size();
	for(int i = 0; i < nSize; i++)
	{
		ActorCommonNode& Node = m_ActorOffLine[i];
		if(0== strcmp(nName,Node.sActorName) )
		{
			return & m_ActorOffLine[i];
		}
	}
	return NULL;
}

//获取最大成员数量
int CGuild::GetMaxMemberCount()
{
	int nLevel = GetLevel();		//行会等级
	const GuildLevelConfig* pConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig().GetLevelConfig(nLevel);
	if (pConfig)
	{
		return pConfig->nMaxMember;
	}
	return 0;
}

//获取最大官员数量
int CGuild::GetMaxTkCount()
{
	int nLevel = GetLevel();		//行会等级
	const GuildLevelConfig* pConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig().GetLevelConfig(nLevel);
	if (pConfig)
	{
		return 0;//pConfig->nMaxTZ;
	}
	return 0;
}

/*
获取帮派职位数量，调整时不能超过
注意：成员是总和，只在外人加入时才判断
*/
INT_PTR CGuild::MaxGuildPosCount(int nGuildPos)
{
	int nLevel = GetLevel();		//行会等级
	const GuildLevelConfig* pConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig().GetLevelConfig(nLevel);
	if (pConfig)
	{
		switch (nGuildPos)		//成员title不可修改，则采用此段代码
		{
		case smGuildLeader:
			return 1;
			break;
		case  smGuildAssistLeader:
			return pConfig->nMaxSecLeader;
			break;
		case  smGuildElite:
			return pConfig->nMaxElite;
			break;
		case  smGuildCommon:
			return pConfig->nMaxMember;
			break;
		case  smGuildTangzhu:
			return pConfig->nMaxelders;
			break;
		default:
			break;
		}
	}
	return 0;
}

int CGuild::GetCanWarNextTime(unsigned int nGuildId)
{
	int nNow = GetGlobalLogicEngine()->getMiniDateTime();
	INT_PTR nCount = m_WarList.count();
	for (INT_PTR i = 0; i < nCount; i++)
	{
		WarRelationData& war = m_WarList[i];

		if(war.nGuildId == nGuildId)
		{
			return war.m_WarData.nNext;
		}
	}
	return 0;
}

/*
int	nState;	   //1 宣战状态 0 正常
判断与目标行会是否处于宣战状态，1：宣战中，0：非宣战中（联盟、普通、宣战时间到）
这里的宣战状态，比GetRelationShipForId（）更加实时
有时间限制，必须宣战关系，且在时限之内，才算宣战关系

如果判断是否敌对关系，主要用这个方法
*/
int CGuild::GetRelationShipInWar(unsigned int nGuildId)
{
	int nNow = GetGlobalLogicEngine()->getMiniDateTime();
	INT_PTR nCount = m_WarList.count();
	for (INT_PTR i = 0; i < nCount; i++)
	{
		WarRelationData& war = m_WarList[i];
		
		if(war.nGuildId == nGuildId)
		{
			if(war.m_WarData.nState == 1 && nNow < war.m_WarData.nNext)
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
	}
	return 0;
}


bool CGuild::OnGuildWar()
{
	int nNow = GetGlobalLogicEngine()->getMiniDateTime();
	INT_PTR nCount = m_WarList.count();
	for (INT_PTR i = 0; i < nCount; i++)
	{
		WarRelationData* war = &(m_WarList[i]);
		if(war == NULL) continue;

		if(war->m_WarData.nState == 1 && nNow < war->m_WarData.nNext)
		{
			return true;
		}
	}

	return false;
}

VOID CGuild::SendWarGuildListToAllMember()
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			CActor* toActor = (CActor*)pEntity;
			SendWarGuildList(toActor);
		}
	}
}

VOID CGuild::UpdateGuildMemberGuildLevelProperty()
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			pEntity->SetProperty<unsigned int>(PROP_ACTOR_GUILD_LEVEL,GetGuildbuildingLev(gbtMainHall));;
		}
	}
}

void CGuild::UpdateActorShowName()
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			CActor* toActor = (CActor*)pEntity;
			toActor->ResetShowName();
		}
	}
}

void CGuild::UpdateGuildActorNameInSbk()
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			CActor* toActor = (CActor*)pEntity;
			GetGlobalLogicEngine()->GetGuildMgr().UpdateGuildMemberName((CActor*)toActor);
		}
	}
}

VOID CGuild::SendWarGuildList(CActor* pActor)
{ 
	if (pActor == NULL)
	{ 
		return;
	}
	
	if (pActor->OnGetIsTestSimulator()
		|| pActor->OnGetIsSimulator() )
	{
		return;
	}
	CActorPacket AP;
	CDataPacket& DataPacket = pActor->AllocPacket(AP);

	DataPacket << (BYTE)enGuildSystemID << (BYTE)sGuildWarList;
	int pos = DataPacket.getPosition();
	INT_PTR nCount = m_WarList.count();
	DataPacket << (BYTE)nCount;
	UINT nNow = GetGlobalLogicEngine()->getMiniDateTime();
	int nReult = 0;
	for (INT_PTR i = 0; i < nCount; i++)
	{
		WarRelationData* war = &(m_WarList[i]);
		if(war == NULL) continue;

		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(m_WarList[i].nGuildId);
		if(pGuild == NULL) continue;

		//宣战
		if(m_WarList[i].m_WarData.nNext > 0 && (unsigned int)m_WarList[i].m_WarData.nNext > nNow && m_WarList[i].m_WarData.nState == WarGuild::wsOnWar )
		{
			DataPacket<<(unsigned int)(m_WarList[i].nGuildId);
			nReult++;
		}
		
	}
	BYTE * pSendCnt = (BYTE *)DataPacket.getPositionPtr(pos);
	*pSendCnt = nReult;
	AP.flush();
}

VOID CGuild::SendWarList(CActor* pActor)
{
	// UINT nNow = GetGlobalLogicEngine()->getMiniDateTime();
	// UINT nDec = 0;//GetLogicServer()->GetDataProvider()->GetGuildConfig().nDeclWarTime;
	// UINT nWar = 0;//GetLogicServer()->GetDataProvider()->GetGuildConfig().nWarTime;

	// CActorPacket AP;
	// CDataPacket& DataPacket = pActor->AllocPacket(AP);
	// DataPacket << (BYTE)enGuildSystemID << (BYTE)cWarList;
	// size_t pos = DataPacket.getPosition();
	// DataPacket << (int)0;
	// int nResult = 0;

	// INT_PTR nCount = m_WarList.count();
	// for (INT_PTR i = 0; i < nCount; i++)
	// {
	// 	bool boChange = false;
	// 	//WarGuild& war = m_WarList[i];
	// 	WarGuild war;
	// 	war.checkStatus(nNow,nWar,nDec,boChange);
	// 	//宣战和敌对的状态都发下去
	// 	if ((war.m_WarData.nRelationShip == WarGuild::wsOnWar || war.m_WarData.nRelationShip == WarGuild::wsDeclareWar ) &&
	// 		war.m_pGuild)
	// 	{
	// 		DataPacket.writeString(war.m_pGuild->m_sGuildname);
	// 		DataPacket << (BYTE)war.m_WarData.nRelationShip;
	// 		DataPacket << (UINT)(war.m_WarData.nNext - nNow);//剩下的秒数
	// 		nResult++;
	// 	}
	// }
	// int* pCount = (int*)DataPacket.getPositionPtr(pos);
	// *pCount = nResult;
	// AP.flush();
}

VOID CGuild::SendWarHistory(CActor* pActor)
{
}

VOID CGuild::AddUnionWarGuild(unsigned int nGuildId,CGuild* pGuild)
{
	if (!pGuild)
	{
		pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if (!pGuild)
		{
			return;
		}
	}

	//将对方的盟友加入宣战
	for(int i=0;i<pGuild->m_WarList.count();i++)
	{
		WarRelationData& war= pGuild->m_WarList[i];

		if(war.m_WarData.nRelationShip == 1)
		{
			if(GetRelationShipForId(war.nGuildId) != 1)
			{
				CGuild* nGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(war.nGuildId);
				if(nGuild)
				{
					AddWarGuild(war.nGuildId,NULL);

					//通知对方成员
					nGuild->SendGuildMsg(smGuildCommon,tpDeclareWarNotice,ttTipmsgWindow,m_sGuildname);

					//通知自己帮派成员
					SendGuildMsg(smGuildCommon,tpOnWarNotice,ttTipmsgWindow,nGuild->m_sGuildname);
				}
			}
		}
	}

	for(int i=0;i<m_WarList.count();i++)
	{
		WarRelationData& war= m_WarList[i];

		if(war.m_WarData.nRelationShip == 1)
		{
			if(pGuild->GetRelationShipForId(war.nGuildId) != 1)
			{
				CGuild* nGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(war.nGuildId);
				if(nGuild)
				{
					pGuild->AddWarGuild(war.nGuildId,NULL);
					//通知对方成员
					nGuild->SendGuildMsg(smGuildCommon,tpDeclareWarNotice,ttTipmsgWindow,pGuild->m_sGuildname);
					//通知自己帮派成员
					pGuild->SendGuildMsg(smGuildCommon,tpOnWarNotice,ttTipmsgWindow,nGuild->m_sGuildname);
				}
			}
		}
	}
}

VOID CGuild::AddWarGuild(unsigned int nGuildId,CGuild* pGuild)
{
	if (!pGuild)
	{
		pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if (!pGuild)
		{
			return;
		}
	}

	bool bAdded = false;
	INT_PTR nCount = m_WarList.count();
	for (int i = 0; i < nCount; i++)
	{
		WarRelationData& war= m_WarList[i];
		if(war.nGuildId == nGuildId)
		{
			war.m_WarData.nPkCount = 0;
			war.m_WarData.nDieCount = 0;
			war.m_WarData.nState = 1;
			war.m_WarData.nNext = GetGlobalLogicEngine()->getMiniDateTime() + 
				GetLogicServer()->GetDataProvider()->GetGuildConfig().m_GuildRelation.nDeclareLastTime;
			bAdded = true;
			break;
		}
	}
	if (!bAdded)
	{
		WarRelationData war;
		war.nGuildId = nGuildId;
		war.m_WarData.nState = 1;
		war.m_WarData.nNext = GetGlobalLogicEngine()->getMiniDateTime() + 
			GetLogicServer()->GetDataProvider()->GetGuildConfig().m_GuildRelation.nDeclareLastTime;
		m_WarList.add(war);
	}

	// SetRelationShipWithId(nGuildId,2);

	bAdded = false;
	//往对方的敌对列表中加上自己
	nCount = pGuild->m_WarList.count();
	for (int i = 0; i < nCount; i++)
	{
		WarRelationData& war= pGuild->m_WarList[i];
		if(war.nGuildId == m_nGid)
		{
			war.m_WarData.nPkCount = 0;
			war.m_WarData.nDieCount = 0;
			war.m_WarData.nState = 1;
			war.m_WarData.nNext = GetGlobalLogicEngine()->getMiniDateTime() + 
				GetLogicServer()->GetDataProvider()->GetGuildConfig().m_GuildRelation.nDeclareLastTime;
			bAdded = true;
			break;
		}
	}
	if (!bAdded)
	{
		WarRelationData war;
		war.nGuildId = m_nGid;
		war.m_WarData.nState = 1;
		war.m_WarData.nNext = GetGlobalLogicEngine()->getMiniDateTime() + 
			GetLogicServer()->GetDataProvider()->GetGuildConfig().m_GuildRelation.nDeclareLastTime;
		pGuild->m_WarList.add(war);
	}

	// pGuild->SetRelationShipWithId(m_nGid,2);
	pGuild->m_boUpdateTime = true;
	this->m_boUpdateTime = true;
}

VOID CGuild::AddPkCount( INT_PTR nGid,INT_PTR nCount /*= 1*/ )
{
	INT_PTR nSize = m_WarList.count();
	for (INT_PTR i = 0; i < nSize; i++)
	{
		WarRelationData& war = m_WarList[i];	
		if (war.nGuildId == nGid && war.m_WarData.nState == 1)
		{
			war.m_WarData.nPkCount += (int)nCount;
		}
	}
}

VOID CGuild::AddDieCount( INT_PTR nGid, INT_PTR nCount /*= 1*/ )
{
	INT_PTR nSize = m_WarList.count();
	for (INT_PTR i = 0; i < nSize; i++)
	{
		WarRelationData& war = m_WarList[i];	
		if (war.nGuildId == nGid && war.m_WarData.nState == 1)
		{
			war.m_WarData.nDieCount += (int)nCount;
		}
	}
}

void CGuild::NotifyUpdateGuildInfo()
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			CActor* toActor = (CActor*)pEntity;
			toActor->GetGuildSystem()->NotifyUpdateGuildInfo();
		}
	}
}

void CGuild::NotifyGuildSimpleInfo()
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			CActor* toActor = (CActor*)pEntity;
			toActor->GetGuildSystem()->SendGuildInfoSimple();
		}
	}
}


void CGuild::NotifyGuildDonateData()
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			CActor* toActor = (CActor*)pEntity;
			toActor->GetGuildSystem()->SendGuildDonateData();
		}
	}
}

void CGuild::NotifyGuildTreeBackCoin()
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			CActor* toActor = (CActor*)pEntity;
			toActor->GetGuildSystem()->SendGuildTreeBackCoin();
		}
	}
}

void CGuild::NotifyUpdateDepot()
{
	
}

void CGuild::NotifyImpeachMember(LPCTSTR strPosName, ACTORNAME strOldActorName, ACTORNAME strNewActorName)
{

}

void CGuild::SendChangeTitleToAll(unsigned int nActorId,BYTE nIndex)
{
	
}

void CGuild::SendTitleToAll()
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			CActor* toActor = (CActor*)pEntity;
			toActor->GetGuildSystem()->SendGuildTitleList();
			toActor->ResetShowName();
		}
	}
}

void CGuild::SendActorHelp(LPCSTR nName,int nSceneId,int x,int y)
{
	
}

bool CGuild::BcanUpGuildSkillLevel(CActor* pActor,int sType,int sCount,bool sConsume)
{
	if (!pActor) 
	{
		 return false;
	}
	if (pActor->OnGetIsTestSimulator()
		|| pActor->OnGetIsSimulator() )
	{
		 return false;
	}
	switch(sType)
	{
	case uGuildLevel:		//帮派等级
		{
			if(sCount <= GetLevel())
				return true;
			else
			{
				pActor->SendOldTipmsgFormatWithId(tpNoGuildLevel,ttDialog);
				return false;
			}

			break;
		}

	case uGuildCont:		//帮派资金
		{
			if(sCount <= GetCoin())
				return true;
			else
			{
				pActor->SendOldTipmsgFormatWithId(tpNoGuildCoin,ttDialog);
				return false;
			}

			break;
		}
	case uGuildYs:			//帮派玥石
		{
			if(sCount <= GetGuildYs())
				return true;
			else
			{
				pActor->SendOldTipmsgFormatWithId(tpNoGuildYs,ttDialog);
				return false;
			}

			break;
		}
	}

	return false;
}

bool CGuild::DecountUpGuildSkill(int sType,int sCount,bool sConsume)
{
	if(sConsume == false)
	{ 
		return false;
	}

	switch(sType)
	{
	case uGuildLevel:		//帮派等级
		{
			return true;
			break;
		}

	case uGuildCont:		//帮派资金
		{
			if(sCount <= GetCoin())
			{
				int newCoin = GetCoin() - sCount;
				SetCoin(newCoin);
				m_boUpdateTime = true;
				return true;
			}
			else
				return false;

			break;
		}
	case uGuildYs:			//帮派玥石
		{
			if(sCount <= GetGuildYs())
			{
				int newYs = (-1) * sCount;
				SetGuildYs(newYs);
				m_boUpdateTime = true;
				return true;
			}
			else
				return false;

			break;
		}
	}

	return false;
}

void CGuild::SaveDbGuildSkill(CActor* pActor,int skillId,int skillLevel)
{
	if (!pActor)
	{
		return;
	}
	
	if (pActor->OnGetIsTestSimulator()
		|| pActor->OnGetIsSimulator() )
	{
		return;
	}
	unsigned int nActorid = 0;

	nActorid = pActor->GetProperty<unsigned int>(PROP_ENTITY_ID);

	//发送消息到数据服务器
	CDataPacket& DataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveGuildSkill);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId << nServerId;
	DataPacket << nActorid;
	DataPacket << m_nGid;
	DataPacket << (int)skillId;
	DataPacket << (int)skillLevel;
	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
}

VOID CGuild::SendGuildSkillToAllMember(int skillId,int skillLevel)
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			CActor* toActor = (CActor*)pEntity;
			toActor->GetGuildSystem()->SetGuildSkillLevel(skillId,skillLevel);
			//SendGuildSkill(toActor,skillId,skillLevel);

		}
	}
}

void CGuild::AddEventRecord(char* nRecord,int nEventId,int nParam1,int nParam2,int nParam3,char* nParam4,char* nParam5)
{
	//if(nParam4 == NULL || nRecord == NULL)
	//	return;

	int nTime = 0;
	//OutputMsg(rmError,_T("Guild in AddEventRecord(),nParam5=%s"),nParam5);

	nTime =  GetGlobalLogicEngine()->getMiniDateTime();
	m_EventCount++;

	if(m_EventCount > MAX_EVENTCOUNT)
	{
		m_EventCount = MAX_EVENTCOUNT;
	}

	for(int i=m_EventCount-1;i>0;i--)
	{
		m_EventRecord[i] = m_EventRecord[i-1];
	}

	GUILDEVENTRECOUD tempEvent;
	tempEvent.aTime = time(NULL);

	if( nEventId <= enGuildEvent_NULL || nEventId >= enGuildEvent_Max )   //非法的行会事件ID
	{
		return;
	}

	tempEvent.mEventId = nEventId;

	tempEvent.mParam1 = nParam1;
	tempEvent.mParam2 = nParam2;
	tempEvent.mParam3 = nParam3;

	if( nParam4 )
	{
		_asncpytA( tempEvent.mParam4, nParam4 );
	}
	else
	{
		_asncpytA( tempEvent.mParam4, "" );
	}

	if( nParam5 )
	{
		_asncpytA( tempEvent.mParam5, nParam5 );
	}
	else
	{
		_asncpytA(tempEvent.mParam5, "");
	}

	//GetGuildEventLog(tempEvent, nEventId, nParam1, nParam2, nParam3, nParam4, nParam5);
	//_asncpytA(tempEvent.nEventMsg,nRecord);

	m_EventRecord[0] = tempEvent;
}

void CGuild::BuildUpEventRecord(int nIndex,int nEventId,int nParam1, int nParam2,int nParam3,char* szParam4,char* szParam5)
{
	if( nIndex < 0 || nIndex >= MAX_EVENTCOUNT)
		return;
	/*
	if(nParam4 == NULL && nParam5==NULL)
		return;

	char sText[1024];
	LPCTSTR sFormat;

	LPCTSTR gOldName;//GetGuildPosName(pNode->BasicData.nType);
	LPCTSTR gNewName;//GetGuildPosName(nTargetPos);

	if(nEventId<=tpGuildEventStart)
		nEventId += tpGuildEventStart;

	switch(nEventId)
	{
	case tpGuildEventCoin:
		{
			if(nParam5 == NULL) return;
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildEventCoin);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)(nParam4),(int)nParam1);
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}

	case tpContributeEvent:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpContributeEvent);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)(nParam4),(int)nParam1);
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}
	case tpAddToGuildEvent:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpAddToGuildEvent);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)(nParam4));
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}
	case tpLeftGuildEvent:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpLeftGuildEvent);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)(nParam4));
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}
	case tpUpGuildPosEvent:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpUpGuildPosEvent);
			gOldName = GetGuiPosName(nParam1);
			gNewName = GetGuiPosName(nParam2);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)(nParam4),gOldName,gNewName);
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}

	case tpDownGuildPosEvent:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpDownGuildPosEvent);
			gOldName = GetGuiPosName(nParam1);
			gNewName = GetGuiPosName(nParam2);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)(nParam4),gOldName,gNewName);
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}
	case tpChangeGuildLeaderEvent:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpChangeGuildLeaderEvent);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)(nParam4),( char *)(nParam5));
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}
	case tpDeleteMemEvent:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpDeleteMemEvent);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)(nParam4),( char *)(nParam5));
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}
	case tpCreateGuildEvent:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpCreateGuildEvent);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)(nParam4));
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}
	case tpAddBossScoreEvent:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpAddBossScoreEvent);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)(nParam4));
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}
	case tpAddBossScoreByItem:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpAddBossScoreByItem);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)(nParam4));
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}
	case tpAddBossScoreByZh:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpAddBossScoreByZh);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)(nParam4));
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}
	case tpkillGuildBossEvent:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpkillGuildBossEvent);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)(nParam4));
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}
	case tpGuildDartEvent:
		{	
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildDartEvent);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)(nParam4));
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}
	case tpGuildUpSkill1Evnet:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildUpSkill1Evnet);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)(nParam4),nParam1,nParam2,( char *)(nParam5),nParam3);
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}
	case tpGuildUpSkill2Event:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildUpSkill2Event);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)(nParam4),nParam1,( char *)(nParam5),nParam2);
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}
	case tpUpGuildBossEvent:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpUpGuildBossEvent);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)(nParam4),( char *)(nParam5));
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}
	case tpFinishQuestEvent:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpFinishQuestEvent);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)(nParam4));
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}
	case tpUpGuildLevelEvent:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpUpGuildLevelEvent);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)(nParam4),nParam1,nParam2);
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}

	case tpKillLuoYangMonster:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpKillLuoYangMonster);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)(nParam4),( char *)(nParam5),nParam1,nParam2);
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}
	case tpGetBattleGuildGX:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGetBattleGuildGX);
			sprintf_s(sText, sizeof(sText), sFormat, nParam1,nParam2);
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}
	case tpGetBattleFr:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGetBattleFr);
			sprintf_s(sText, sizeof(sText), sFormat,( char *)(nParam4),( char *)(nParam5), nParam1);
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}

	case tpEditGuildTitle:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpEditGuildTitle);
			sprintf_s(sText, sizeof(sText), sFormat,( char *)(nParam4));
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}

	case tpSetActorTitle:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpSetActorTitle);
			sprintf_s(sText, sizeof(sText), sFormat,( char *)(nParam4),( char *)(nParam5), m_GuildTitles[nParam1-1].nTitleName);
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}

	case tpUpguildlevelEvent:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpUpguildlevelEvent);
			sprintf_s(sText, sizeof(sText), sFormat,( char *)(nParam4), nParam1,nParam2);
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}
	case tpGuildEventCall:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildEventCall);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)(nParam4));
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}
	case tpGuildEventDeclare:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildEventDeclare);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)(nParam4));
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}
	case tpGuildEventBeDeclare:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildEventBeDeclare);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)(nParam4));
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}
	case tpGuildEventUnion:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildEventUnion);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)(nParam4));
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}
	case tpGuildEventBeUnion:
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildEventBeUnion);
			sprintf_s(sText, sizeof(sText), sFormat, ( char *)(nParam4));
			_asncpytA(m_EventRecord[nIndex].nEventMsg,sText);
			break;
		}
	default:
		break;
	}

	//m_EventRecord[0] = tempEvent;
	*/
}

char* CGuild::GetGuiPosName(int posType)
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

void CGuild::SaveAllGuildEvent()
{
	if(m_nGid <= 0) return;

	//OutputMsg(rmTip,_T("save guild event to db"));
	if(m_EventCount > MAX_EVENTCOUNT)
	{
		m_EventCount = MAX_EVENTCOUNT;
	}

	for(int i=0;i<m_EventCount;i++)
	{
		if(i==0)
			SaveGuildEventtoDb(i,0);
		else
			SaveGuildEventtoDb(i,1);
	}

	if(m_bSaveDepotRecord)
	{
		SaveGuildDepoItem();
		SaveGuildDepotRecord();
		m_bSaveDepotRecord = false;
	}

	if (m_boUpdateTime)		
		Save();

}

void CGuild::SaveDepotData()
{
	if(m_bSaveDepotRecord)
	{
		SaveGuildDepoItem();
		SaveGuildDepotRecord();
		m_bSaveDepotRecord = false;
	}
}

void CGuild::SaveGuildDepoItem()
{
	CDataPacket& DataPacket = GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveGuildDepotItem);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId << nServerId;
	DataPacket << (unsigned int)m_nGid;
	int nCount = (int)m_GuildDepotBag.count();

	size_t pos = DataPacket.getPosition();
	DataPacket << (int)0;

	int nResult = 0;

	for(int i=0;i<nCount;i++)
	{
		GUILDDEPOTITEM& nItem = m_GuildDepotBag[i];
		if(nItem.nExitFlag == 0)
		{
			DataPacket << (int)nItem.nActorId;
			DataPacket << (int)nItem.nCoinType;
			DataPacket << (int)nItem.nCoinNum;
			
			DataPacket << (unsigned int)nItem.nExchangeActorId;
			DataPacket.writeString(nItem.szExchangeActorName);
			DataPacket << (unsigned int)nItem.nExchangeTime;
			
			DataPacket << (CUserItem)(*nItem.nUserItem);
			nResult++;
		}
	}

	int* pCount = (int*)DataPacket.getPositionPtr(pos);
	*pCount = nResult;

	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
}

void CGuild::SaveGuildDepotRecord()
{
	CDataPacket& DataPacket = GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveGuildDepotRecord);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId << nServerId;
	DataPacket << (unsigned int)m_nGid;
	int nCount = (int)m_GuildDepotRecord.count();
	if(nCount > 100)
		nCount = 100;

	DataPacket << (int)nCount;

	for(int i=0;i<nCount;i++)
	{
		GUILDDEPOTMSG& nRecord = m_GuildDepotRecord[i];

		DataPacket << nRecord.nCoinNum;
		DataPacket << nRecord.nCoinType;
		DataPacket << nRecord.nNum;
		DataPacket << nRecord.nTime;
		DataPacket << nRecord.nType;
		DataPacket.writeString(nRecord.nActorName);
		DataPacket.writeString(nRecord.nItemName);
	}

	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
}

void CGuild::SaveGuildEventtoDb(int Index,int nType)
{
	if ((Index < 0) || (Index >= MAX_EVENTCOUNT)) return;
	
	CDataPacket& DataPacket = GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveGuildEvent);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId << nServerId;
	DataPacket << (int)nType;
	DataPacket << m_nGid;
	DataPacket << (int)m_EventRecord[Index].aTime;
	DataPacket << (int)m_EventRecord[Index].mEventId;

	DataPacket << (int)m_EventRecord[Index].mParam1;
	DataPacket << (int)m_EventRecord[Index].mParam2;
	DataPacket << (int)m_EventRecord[Index].mParam3;
	//DataPacket << (int)m_EventRecord[Index].mParam4;
	//DataPacket << (int)m_EventRecord[Index].mParam5;

	DataPacket.writeString(m_EventRecord[Index].mParam4);
	DataPacket.writeString(m_EventRecord[Index].mParam5);
	//DataPacket.writeString(m_EventRecord[Index].nEventMsg);

	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
}

void CGuild::GmChangeLeader(const char* nName)
{
	ActorCommonNode* pNode = FindMemberByName(nName);
	if (pNode == NULL) return;
	if (m_nLeaderId == 0) return;
	ACTORNAME sLeaderName;
	_asncpytA(sLeaderName,m_sLeaderName);
	GetGlobalLogicEngine()->GetGuildMgr().OnGuildLeaderChange(this, m_nLeaderId,pNode->BasicData.nActorId,glcGmBackStageSet);
	
	AddEventRecord( "", enGuildEvent_LeaderChange, 0, 0, 0, (char *)(sLeaderName),( char *)(m_sLeaderName));
	//GetLogicServer()->GetLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tpGmSetGuildLeader,ttChatWindow+ttScreenCenter,nName,m_sGuildname);
}

/*
获取与目标行会的关系
这里的行会关系，每10分钟，在CheckWarList（）刷新一次
如果判断是否联盟关系，主要用这个方法
*/
int CGuild::GetRelationShipForId(unsigned int nGuildId)
{
	int nCount = (int)m_WarList.count();

	for(int i=0;i<nCount;i++)
	{
		WarRelationData& nWar = m_WarList[i];

		if(nWar.nGuildId == nGuildId)
		{
			return nWar.m_WarData.nRelationShip;
		}
	}
	return 0;
}

void CGuild::SetRelationShipWithId(unsigned int nGuildId,int nType)
{
	int nCount = (int)m_WarList.count();

	for(int i=0;i<nCount;i++)
	{
		WarRelationData& nWar = m_WarList[i];

		if(nWar.nGuildId == nGuildId)
		{
			nWar.m_WarData.nRelationShip = nType;
			return;
		}
	}

	//如果原来列表没有的，加进去
	WarRelationData nWar;
	nWar.nGuildId = nGuildId;
	nWar.m_WarData.nRelationShip = nType;
	m_WarList.add(nWar);
	m_boUpdateTime = true;
}

/*
在行会仓库中查找某个guid的物品
*/
int CGuild::FindItemIndex(const CUserItem::ItemSeries series)
{
	for(int i=0;i<m_GuildDepotBag.count();i++)
	{
		GUILDDEPOTITEM& nItem = m_GuildDepotBag[i];

		if(nItem.nUserItem && nItem.nUserItem->series.llId == series.llId && nItem.nExitFlag == 0)
		{
			return i;
		}
	}

	return -1;
}

char* CGuild::GetCoinname(int nType)
{
	if(nType == 1)
		return GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpMoneyTypeCoin);
	else if(nType == 2)
		return GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpMoneyTypeYuanbao );

	return NULL;
}
void CGuild::ChangeGuildCoin(int value, INT_PTR nLogId,LPCTSTR pStr)
{
	int cCoin = GetCoin();
	SetCoin(cCoin+value);
	// char sVal[128];
	// sprintf(sVal,"%d",value);
}

char* CGuild::GetDepotRecordMsg(int nIndex,char* sText)
{
	int nCount = (int)m_GuildDepotRecord.count();

	if((nIndex < 1) || (nIndex>=nCount)) return NULL;

	GUILDDEPOTMSG& nMsg = m_GuildDepotRecord[nIndex];

	char* cName = GetCoinname(nMsg.nCoinType);
	if(cName == NULL) return NULL;

	LPCTSTR sFormat;
	switch(nMsg.nType)
	{
	case 1:			//存入
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpPutItemToDepot);
			sprintf_s(sText, sizeof(sText), sFormat, nMsg.nActorName,nMsg.nItemName,nMsg.nNum,nMsg.nCoinNum,cName);
			return sText;
		}

	case 2:			//取出
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGetItemFromDepot);
			sprintf_s(sText, sizeof(sText), sFormat, nMsg.nActorName,nMsg.nItemName,nMsg.nNum,nMsg.nCoinNum,cName);
			return sText;
		}

	case 3:			//收回
		{
			sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGetBackItem);
			sprintf_s(sText, sizeof(sText), sFormat, nMsg.nActorName,nMsg.nItemName,nMsg.nNum);
			return sText;
		}
	}

	return NULL;
}

/*获取行会仓库中已经存放物品数量
nExitFlag 应该没有用了
*/
int CGuild::GetGuildDepotCount()
{
	int nCount = (int)m_GuildDepotBag.count();
	int nResult = 0;

	for(int i=0;i<nCount;i++)
	{
		if(m_GuildDepotBag[i].nExitFlag == 0)
			nResult++;
	}

	return nResult;
}

int CGuild::GetGuildDepotPageMax()
{
	// int nGuildLevel = GetLevel();		//行会等级
	// const GuildLevelConfig* pLevelConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig().GetLevelConfig(nGuildLevel);
	// const CGuildProvider& pConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	// if( pLevelConfig )
	// {
	// 	int nPage = pLevelConfig->nDepotPage;
	// 	if( nPage > pConfig.m_GuildDepot.nDepotMaxPage )
	// 	{
	// 		nPage = pConfig.m_GuildDepot.nDepotMaxPage;
	// 	}
	// 	return nPage;
	// }
	return 0;
}

//处理等待审核的兑换请求
void CGuild::ProcDepotItemInExchangeCheck()
{
	// CGuildProvider& pGuildConfig			= GetLogicServer()->GetDataProvider()->GetGuildConfig();
	// unsigned int nNow						= GetGlobalLogicEngine()->getMiniDateTime();
	// unsigned int nWaitTime					= pGuildConfig.m_GuildDepot.nExchangeWaitCheckHour * 3600;
	// const CStdItemProvider& itemProvider	= GetLogicServer()->GetDataProvider()->GetStdItemProvider();

	// INT_PTR nCount = m_GuildDepotBag.count();
	// for( INT_PTR i=0; i<nCount; i++ )
	// {
	// 	GUILDDEPOTITEM& stDepotItem	= m_GuildDepotBag[i];
	// 	CUserItem *pUserItem = stDepotItem.nUserItem;
	// 	if( pUserItem && stDepotItem.nExchangeActorId > 0 && nNow - stDepotItem.nExchangeTime > nWaitTime  )
	// 	{
	// 		unsigned int nExchangeActorId = stDepotItem.nExchangeActorId;
	// 		ACTORNAME szExchangeActorName;
	// 		_asncpytA(szExchangeActorName, stDepotItem.szExchangeActorName);

	// 		//该物品设置为未被申请兑换
	// 		stDepotItem.nExchangeActorId		= 0;
	// 		stDepotItem.szExchangeActorName[0]	= 0;
	// 		stDepotItem.nExchangeTime			= 0;
	// 		m_bSaveDepotRecord					= true;		//需要存储

	// 		int nNeedGx = 0; 
	// 		const CStdItem *pStdItem  = itemProvider.GetStdItem(pUserItem->wItemId);
	// 		if( pStdItem )
	// 		{
	// 			int nItemCircle = (int)pStdItem->GetUseConditionValue(CStdItem::ItemUseCondition::ucMinCircle);	//转数
	// 			int nItemLevel  = (int)pStdItem->GetUseConditionValue(CStdItem::ItemUseCondition::ucLevel);		//等级
	// 			nNeedGx			= pGuildConfig.GetGuildDepotGetOutNeedGx(nItemCircle, nItemLevel, pStdItem->m_btType);
	// 		}

	// 		if( nNeedGx < 0 )
	// 		{
	// 			nNeedGx = 0;
	// 		}

	// 		LPCTSTR sTitle		= GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildDepotExchangeTitlt3);
	// 		LPCTSTR sContent	= GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildDepotExchangeCont3);

	// 		CMailSystem::MAILATTACH mailAttach[MAILATTACHCOUNT];
	// 		mailAttach[0].nType		= qatGuildContribution;
	// 		mailAttach[0].nResourceCount	= nNeedGx;					//返回玩家申请兑换时扣除的行会贡献
	// 		CMailSystem::SendMail(nExchangeActorId, sTitle, sContent,mailAttach);
	// 	}
	// }
}


void CGuild::UpdateSbkGuidAchieve()
{

	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			CActor* toActor = (CActor*)pEntity;
			toActor->GetAchieveSystem().ActorAchievementComplete(nAchieveJionSBKGuild, 1);
		}
	}
}


void CGuild::UpdateSbkGuildTitle(bool bFlag, bool boChangeNewTitle)
{

	// const CGuildProvider& pConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	// int nLeaderTitleId = pConfig.nSbkOwnerTitleId;
	// int nMemTitleId = pConfig.nSbkMemberTitleId;
	// int nCityOwnGuildId = GetGlobalLogicEngine()->GetGuildMgr().GetCityOwnGuildId();
	// CGuildComponent& guildMgr =  GetGlobalLogicEngine()->GetGuildMgr();

	// CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	// CLinkedNode<EntityHandle> *pNode;
	// CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	// for (pNode = it.first(); pNode; pNode = it.next())
	// {
	// 	EntityHandle& hHandle = pNode->m_Data;
	// 	CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
	// 	if(pEntity == NULL || pEntity->GetType() != enActor)
	// 	{
	// 		continue;
	// 	}
	// 	CActor* pActor = (CActor*)pEntity;
	// 	int curTitleId = pActor->GetProperty<unsigned int>(PROP_ACTOR_CURNEWTITLE);
	// 	int nCityPos = guildMgr.getCityPostionById(pActor->GetId());
	// 	if (nCityPos == stRealMaster)			//是城主
	// 	{
	// 		pActor->GetNewTitleSystem().delNewTitle(nMemTitleId);
	// 		pActor->GetNewTitleSystem().addNewTitle(nLeaderTitleId);
	// 	}
	// 	else
	// 	{
	// 		pActor->GetNewTitleSystem().delNewTitle(nLeaderTitleId);
	// 		if(curTitleId == nLeaderTitleId)
	// 		{
	// 			pActor->SetProperty<unsigned int>(PROP_ACTOR_CURNEWTITLE,0);
	// 		}
	// 	}
	// 	if(pActor->GetGuildSystem()->GetGuildId() == nCityOwnGuildId)//沙城成员
	// 	{
	// 		if(nCityPos != stRealMaster)
	// 		{
	// 			pActor->GetNewTitleSystem().addNewTitle(nMemTitleId);
	// 		}
	// 	}
	// 	else
	// 	{
	// 		pActor->GetNewTitleSystem().delNewTitle(nMemTitleId);
	// 		if(curTitleId == nMemTitleId)
	// 		{
	// 			pActor->SetProperty<unsigned int>(PROP_ACTOR_CURNEWTITLE,0);
	// 		}
	// 	}
	// 	pActor->ResetShowName();
	// }
}

void CGuild::SendCombineSbk3Award()
{
	// int nIndex = 0;
	// CLinkedNode<ActorCommonNode> *pLinkNode;
	// CLinkedListIterator<ActorCommonNode> it(m_ActorOffLine);
	// for (pLinkNode = it.first(); pLinkNode; pLinkNode = it.next())
	// {
	// 	ActorCommonNode& Node = pLinkNode->m_Data;
	// 	CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
	// 	CScriptValueList paramList,retParamList;
	// 	paramList<<m_nGid<<Node.BasicData.nActorId<<Node.sActorName<<Node.BasicData.nType;
	// 	if (pNpc && !pNpc->GetScript().Call("GiveCombineSbk3Award", paramList, retParamList))
	// 	{
	// 		//脚本错误，将以模态对话框的形式呈现给客户端
	// 		const RefString &s = pNpc->GetScript().getLastErrorDesc();
	// 		OutputMsg(rmError, (LPCSTR)s);
	// 	}	
	// }


}

/*
获取行会的职位人数（实时计算）
nGuildPos:1-官员，2-副帮主
*/
int CGuild::GetGuildPosCount(int nGuildPos)
{
	int nCount = 0;

	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	// CLinkedNode<ActorCommonNode> *pNode;
	// CLinkedListIterator<ActorCommonNode> it(m_ActorOffLine);
	int nSize = m_ActorOffLine.size();
	// for (pNode = it.first(); pNode; pNode = it.next())
	// {
	for(int i = 0; i < nSize; i++)
	{
		ActorCommonNode& actor = m_ActorOffLine[i];
		//		if (!actor.boStatus) continue;
		//如果已经上线，发送在线的数据
		CEntity* pOnlineEntity = NULL;
		if (actor.hActorHandle != 0)
		{
			pOnlineEntity = pEntityMgr->GetEntity(actor.hActorHandle);
			if (pOnlineEntity)
			{
				CActor* pOnlineActor = (CActor*)pOnlineEntity;
				if(pOnlineActor->GetGuildSystem()->GetGuildPos() == nGuildPos)
				{
					nCount++;
				}
			}
		}
		else
		{
			if(actor.BasicData.nType == nGuildPos)
			{
				nCount++;
			}
		}
	}

	return nCount;
}

void CGuild::AddToGuildGxList(unsigned int nActorId,int nGx)
{
	/*int nCount = m_GuildMemGxList.count();

	int nIndex = GetGuildGxIndex(nActorId);
	int nNewIndex = nCount;
	for(int i = 0;i<nCount;i++)
	{
	if(nGx > m_GuildMemGxList[i].nGx)
	{
	nNewIndex = i;
	break;
	}
	}

	GUILDMEMGXLIST tmpGxMem;
	tmpGxMem.nActorId = nActorId;
	tmpGxMem.nGx = nGx;

	if( nIndex != nNewIndex )
	{
	if(nIndex >= 0)
	{
	m_GuildMemGxList.remove(nIndex);

	for(int i = 0;i<m_GuildMemGxList.count();i++)
	{
	if(nGx > m_GuildMemGxList[i].nGx)
	{
	nNewIndex = i;
	break;
	}
	}
	if (nNewIndex > m_GuildMemGxList.count())
	{
	nNewIndex = m_GuildMemGxList.count();
	}
	m_GuildMemGxList.insert(nNewIndex,tmpGxMem);
	}
	else
	{
	m_GuildMemGxList.insert(nNewIndex,tmpGxMem);
	}
	}
	else
	{
	m_GuildMemGxList.insert(nNewIndex,tmpGxMem);
	}*/
}

int CGuild::GetGuildGxIndex(unsigned int nActorId)
{
	int nCount = (int)m_GuildMemGxList.count();

	for(int i = 0;i<nCount;i++)
	{
		if(m_GuildMemGxList[i].nActorId == nActorId)
		{
			return i;
		}
	}

	return -1;
}

int	CGuild::GetGuildbuildingLev(int nType)
{
	switch(nType)
	{
	case gbtMainHall:
		return m_MainHallLev;
	case gbExerciserooem:
		return m_ExerciseRoomlv;
	case gbAssemblyhall:
		return m_AssemblyHallLv;
	case gbtGuildShop:
		return m_GuildShopLev;
	case gbtMoonBox:
		return m_MoonBoxLevel;
	case gbtGuildBoss:
		return m_GuildBossLevel;

	case gbtTech:
		return m_btTechLevel;

	case gbtDragonThunder:
		return m_btDragonThunderLevel;
	}

	return 0;
}

void CGuild::ChangeGuildBuildingLev(int nType,int nValue)
{
	m_boUpdateTime = true;

	switch(nType)
	{
	case gbtMainHall:
		m_MainHallLev += nValue;
		SetGuildLevel( m_MainHallLev,true,true,true);			//主殿等级就是行会等级
		OnSendGuildBuildingChange(gbtMainHall, m_MainHallLev);
		UpdateGuildMemberGuildLevelProperty();
		break;
	case gbExerciserooem:
		m_ExerciseRoomlv += nValue;
		OnSendGuildBuildingChange(gbExerciserooem, m_ExerciseRoomlv);
		break;
	case gbAssemblyhall:
		m_AssemblyHallLv += nValue;
		OnSendGuildBuildingChange(gbAssemblyhall, m_AssemblyHallLv);
		break;
	case gbtGuildShop:
		m_GuildShopLev += nValue;
		OnSendGuildBuildingChange(gbtGuildShop, m_GuildShopLev);
		break;
	case gbtMoonBox:
		m_MoonBoxLevel += nValue;
		OnSendGuildBuildingChange(gbtMoonBox, m_MoonBoxLevel);
		break;
	case gbtGuildBoss:
		m_GuildBossLevel += nValue;
		OnSendGuildBuildingChange(gbtGuildBoss, m_GuildBossLevel);
		break;

	case gbtTech:
		m_btTechLevel += nValue;
		OnSendGuildBuildingChange(gbtTech, m_btTechLevel);
		break;

	case gbtDragonThunder:	// 震天龙弹 升级 
		m_btDragonThunderLevel += nValue;
		OnSendGuildBuildingChange(gbtDragonThunder, m_btDragonThunderLevel);
		break;
	default:
		return;
	}
}

void CGuild::OnSendGuildBuildingChange(int nIndex,int nLevel)
{

}

void CGuild::SendGuildChanllengeResult(unsigned int nActorId,int nType,int nResult)
{
	
}

void CGuild::SetGuildBuildingCdTime(int nTime)
{
	m_boUpdateTime = true; 
	m_UpLeftTime = nTime;
	SendGuildBuildCdTime();
}

void CGuild::SendGuildBuildCdTime()
{
}

void CGuild::SendGuildUpgradeItem()
{
}


//计算公式为：   行会建筑评分=100*主殿等级+40*商店等级+50*月光宝盒等级+60*技能塔等级  
int CGuild::CalcGuildBuildingSorce()
{
	int nResult = 100 * m_MainHallLev + 40 * m_GuildShopLev + 50 * m_MoonBoxLevel + 60 * m_btTechLevel;
	return nResult;
}


void CGuild::SetMemberCamp(int nCampId, int nScendId)
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			//((CActor*)pEntity)->SetCamp(nCampId);
			if( pEntity->GetSceneID() == nScendId )
			{
				((CAnimal*)pEntity)->SetCamp(nCampId);
			}
		}
	}
}

void CGuild::AddMemberBuffById(int nBuffId,int nScenceId)
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			//((CActor*)pEntity)->SetCamp(nCampId);
			if( nScenceId > 0 && ((CActor*)pEntity)->GetSceneID() == nScenceId )
			{
				((CActor*)pEntity)->GetBuffSystem()->Append(nBuffId);
			}
		}
	}
}

void CGuild::DelMemberBuffById(int nBuffId)
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			//((CActor*)pEntity)->SetCamp(nCampId);
			((CActor*)pEntity)->GetBuffSystem()->RemoveById(nBuffId);
		}
	}
}

unsigned int CGuild::GetTopBattleFromOnLineList()
{
	unsigned int nResult = 0;
	int nTopBattle = 0;
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			CActor* pActor = (CActor*)pEntity;
			int nBattle = pActor->GetProperty<int>(PROP_ACTOR_BATTLE_POWER);
			if ( nBattle > nTopBattle )
			{
				nResult = pActor->GetId();
			}
		}
	}
	return nResult;
}

void CGuild::AutoChangeLeader( unsigned int nLeaderId )
{
	ActorCommonNode* pNode = FindMember( nLeaderId );
	if (pNode == NULL) return;
	CEntity* pOnLineEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(pNode->BasicData.nActorId);
	if(pOnLineEntity == NULL) return;

	CActor* pActor = (CActor*)pOnLineEntity;
	CGuildSystem* pGS = pActor->GetGuildSystem();
	if (pGS->GetGuildPtr() == NULL || pGS->GetGuildPtr() != this)
	{
		return;
	}
	if ( pGS->GetGuildPos() == smGuildLeader )
	{
		return;
	}
	pGS->SetGuildPos(smGuildLeader);
	ACTORNAME sOldLeaderName = "";
	unsigned int nOldLeaderId = 0;
	if(m_nLeaderId)
	{
		CEntity* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(m_nLeaderId);
		nOldLeaderId = m_nLeaderId;
		if (pEntity && pEntity->GetType() == enActor )
		{
			((CActor*)pEntity)->GetGuildSystem()->SetGuildPos(smGuildCommon);
			_asncpytA(sOldLeaderName,pEntity->GetEntityName());
		}
		ActorCommonNode* mNode = FindMember(m_nLeaderId);
		if(mNode)
		{
			mNode->BasicData.nType = smGuildCommon;
			_asncpytA(sOldLeaderName,mNode->sActorName);
		}
	}
	pNode->BasicData.nType = smGuildLeader;
	m_nLeaderId = pNode->BasicData.nActorId;
	GetLogicServer()->GetLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tpGmSetGuildLeader,ttChatWindow+ttScreenCenter,pActor->GetEntityName(),m_sGuildname);
	AddEventRecord( "", enGuildEvent_LeaderChange, 0, 0, 0, (char *)(sOldLeaderName), (char *)(pActor->GetEntityName()) );

	if(GetLogicServer()->GetLocalClient())
	{
	}
}


/*
获取行会职位的Name，如帮主、副帮主、官员、普通成员
nGuildPos：0、1、2、3
如果行会职位的Name是可以被编辑的，则使用下面被屏蔽的代码；否则可以使用固定配置
*/
char* CGuild::GetGuildTitleName(int nGuildPos)
{
	/*
	if( nGuildPos < 0 || nGuildPos > 3 ) return NULL;		--如果成员title是可以编辑的，则使用此段代码
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


void CGuild::GetUpgradeItemCount(int& nItemNum1, int& nItemNum2, int& nItemNum3)
{
	nItemNum1 = m_nUpgradeItemNum1;
	nItemNum2 = m_nUpgradeItemNum2;
	nItemNum3 = m_nUpgradeItemNum3;
}

void CGuild::SetUpgradeItemCount(int nItemNum1, int nItemNum2, int nItemNum3)
{
	m_nUpgradeItemNum1 = nItemNum1;
	m_nUpgradeItemNum2 = nItemNum2;
	m_nUpgradeItemNum3 = nItemNum3;
	m_boUpdateTime = true;

	SendGuildUpgradeItem();		//同步到行会成员
}

int CGuild::GetAutoJoinLevel()
{
	// if( m_nAutoJoinLevel < 50 )
	// {
	// 	m_nAutoJoinLevel = 50;
	// }
	return m_nAutoJoinLevel;
}

void CGuild::SetAutoJoinLevel(int nNewLevel)
{
	m_nAutoJoinLevel = nNewLevel;
	m_boUpdateTime = true;
}

//每日数据处理
void CGuild::ProcDailyData()
{
	SetDailyGuildCoinDonated(0);
	RefreshGuildTask(true);											//强行刷新行会任务
	//SetGuildActiveSts(GuildActive_Tree, GuildActiveSts_Undo);		//行会神树活动设置为今日未开始
}

void CGuild::ResetGuildTreeAndFruitByGuildLevel()
{
	SetGuildTreeData(GUILDTREE_PERIOD_1, 0);		//设置行会神树数据
	int nNewFuitLevel = GetGuildFruitLevelByGuildlevel();
	if( nNewFuitLevel < GUILDFRUIT_PERIOD_1 )
	{
		nNewFuitLevel = GUILDFRUIT_PERIOD_1;	//至少1级
	}
	int nNewFruitNum  = GetGuildFruitNumByFruitlevel(nNewFuitLevel);
	SetGuildFruitData(nNewFuitLevel,0, nNewFruitNum);
}

int CGuild::GetGuildFruitLevelByGuildlevel()
{
	CGuildProvider& pConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	int nLevel = GetLevel();		//行会等级
	const GuildLevelConfig* pGuildLevelConfig = pConfig.GetLevelConfig(nLevel);
	if( pGuildLevelConfig )
	{
		return pGuildLevelConfig->nFruitLevel;
	}
	return GUILDFRUIT_PERIOD_1;		//果实最低为一级
}

//nFruitLevel，从1-N
int CGuild::GetGuildFruitNumByFruitlevel(int nFruitLevel)
{
	// CGuildProvider& pConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	// if( 0 < nFruitLevel && nFruitLevel <= pConfig.m_GuildTree.m_FruitLevelList.count()  )
	// {
	// 	GuildFruitLevel& stFuitLevel = pConfig.m_GuildTree.m_FruitLevelList[nFruitLevel-1];
	// 	return stFuitLevel.m_nFruitNum;
	// }
	return 0;
}

//设置行会神树数据
void CGuild::SetGuildTreeData(int nTreeLevel, int nTreeDegree)
{
	if( GUILDTREE_PERIOD_1 <= nTreeLevel && nTreeLevel <= GUILDTREE_PERIOD_4 )
	{
		//如果行会神树等级从非结果期 设置为 结果期
		if( m_nGuildTreeLevel != GUILDTREE_PERIOD_4 && nTreeLevel == GUILDTREE_PERIOD_4 )
		{
			m_nGuildTreeFruitTime = GetGlobalLogicEngine()->getMiniDateTime();		//设置结果时间
		}
		m_nGuildTreeLevel	= nTreeLevel;
	}
	if( nTreeDegree >= 0 )
	{
		m_nGuildTreeDegree	= nTreeDegree;
	}
	m_boUpdateTime = true;
	NotifyTreeAndFruitData(1);		//广播神树数据
}

//设置行会神树果实数据
void CGuild::SetGuildFruitData(int nFruitLevel, int nFruitDegree, int nFruitNum)
{
	// CGuildProvider& pConfig		= GetLogicServer()->GetDataProvider()->GetGuildConfig();
	// INT_PTR nFruitLevelCount	= pConfig.m_GuildTree.m_FruitLevelList.count();
	// if( GUILDFRUIT_PERIOD_1 <= nFruitLevel && nFruitLevel <= nFruitLevelCount )
	// {
	// 	m_nGuildFruitLevel	= nFruitLevel;
	// }
	// if( nFruitDegree >= 0 )
	// {
	// 	m_nGuildFruitDegree	= nFruitDegree;
	// }
	// if( nFruitNum >= 0 )
	// {
	// 	m_nGuildFruitNum	= nFruitNum;
	// }
	// m_boUpdateTime = true;
	// NotifyTreeAndFruitData(2);		//广播神树果实数据
}

//处理神树从成熟期到幼苗期
void CGuild::ProcGuildTreeToSapling(unsigned int nNow)
{
	// CGuildProvider& pConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	// int nDiff = nNow - m_nGuildTreeFruitTime;
	// //神树成熟且 过了保护期
	// if( m_nGuildTreeLevel >= GUILDTREE_PERIOD_4 && nDiff > pConfig.m_GuildTree.m_nFruitProtectTime * 3600 )
	// {
	// 	ResetGuildTreeAndFruitByGuildLevel();	//根据行会等级重置
	// 	SetGuildTreeFruitTime(0);				//成熟时间归0
	// 	//NotifyTreeAndFruitData(0);			//向在线成员广播
	// }
}

//行会神树返还资金（暂不需要）
void CGuild::ProcGuildTreeBackCoin()
{
	// if( m_nGuildTreeLevel < GUILDTREE_PERIOD_2 || m_nGuildTreeLevel > GUILDTREE_PERIOD_3 )
	// {
	// 	return;
	// }

	// CGuildProvider& pConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	// if( 1 <= m_nGuildFruitLevel && m_nGuildFruitLevel <= pConfig.m_GuildTree.m_FruitLevelList.count() )
	// {
	// 	GuildFruitLevel& stFruit = pConfig.m_GuildTree.m_FruitLevelList[m_nGuildFruitLevel-1];
	// 	int nBackCoin = stFruit.m_nBackGuildCoin[m_nGuildTreeLevel-2];
	// 	if( nBackCoin == 0 )
	// 	{
	// 		return;
	// 	}
	// 	int nNewCoin = GetCoin() + nBackCoin;
	// 	SetCoin(nNewCoin);
	// 	m_nGuildTreeBackCoin += nBackCoin;
	// 	NotifyGuildTreeBackCoin();				//提示成员
	// }
}

//nFruitLevel，从1-N
const GuildFruitLevel* CGuild::GetGuildFruitLevel(int nFruitLevel)
{
	// CGuildProvider &pGuildCfg = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	// if( 0 < nFruitLevel && nFruitLevel <= pGuildCfg.m_GuildTree.m_FruitLevelList.count() )
	// {
	// 	return &pGuildCfg.m_GuildTree.m_FruitLevelList[nFruitLevel-1];
	// }
	return NULL;
}

/*响应行会升级
行会升级，导致
1、如果神树果实等级没有达到行会等级对应的果实等级，则提升到这个等级，如果已经达到，则不变
结果期也是这样
*/
void CGuild::OnGuildLevelUp()
{
	int nLevel = GetLevel();		//行会等级
	AddEventRecord( "", enGuildEvent_levelUp, nLevel, 0, 0, NULL,NULL);
}

/*
nType:0：发送神树和果实数据，1-仅发送神树数据，2-仅发送果实数据
*/
void CGuild::NotifyTreeAndFruitData(int nType)
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			CActor* toActor = (CActor*)pEntity;
			if(nType == 0 || nType == 1)
			{
				toActor->GetGuildSystem()->SendGuildTreeData();
			}
			if(nType == 0 || nType == 2)
			{
				toActor->GetGuildSystem()->SendGuildFruitData();
			}
		}
	}
}

void CGuild::OnGuildTask(int nTaskType, int nTaskObjId, int nAddTaskSche, int nParam)
{
	if( m_nGuildTaskId <= 0 )
	{
		return;
	}

	int nGuildTaskType = GetGuildTaskType(m_nGuildTaskId);
	if( nTaskType != nGuildTaskType )		//非任务类型
	{
		return;
	}

	int nGuildTaskObjId = GetGuildTaskObjId(m_nGuildTaskId);
	if( nTaskObjId != nGuildTaskObjId )		//非任务目标
	{
		return;
	}

	int nGuildTaskObjNum = GetGuildTaskObjNum(m_nGuildTaskId);
	if( nAddTaskSche >= nGuildTaskObjNum  )		//已经完成
	{
		return;
	}

	m_nGuildTaskSche += nAddTaskSche;
	if( m_nGuildTaskSche > nGuildTaskObjNum )	//任务完成了
	{
		m_nGuildTaskSche = nGuildTaskObjNum;
	}
	m_boUpdateTime	= true;
}

short CGuild::GetGuildTaskType(int nTaskId)
{
	const GuildTask* pTask = GetGuildTaskById(nTaskId);
	if(pTask)
	{
		return pTask->m_nTaskType;
	}
	return 0;
}

int CGuild::GetGuildTaskObjId(int nTaskId)
{
	const GuildTask* pTask = GetGuildTaskById(nTaskId);
	if(pTask)
	{
		return pTask->m_nTaskObjId;
	}
	return 0;
}

int CGuild::GetGuildTaskObjNum(int nTaskId)
{
	const GuildTask* pTask = GetGuildTaskById(nTaskId);
	if(pTask)
	{
		return pTask->m_nTaskObjNum;
	}
	return 0;
}

//刷新行会任务
void CGuild::RefreshGuildTask(bool bForce)
{
	// int nLevel = GetLevel();		//行会等级
	// if( 0 <= nLevel && nLevel < GUILD_LEVEL_MAX )
	// {
	// 	if( !bForce && m_nGuildTaskId > 0 )			//非强行刷新,且 已经有了行会任务
	// 	{
	// 		return;
	// 	}
	// 	CGuildProvider& pConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	// 	GuildLevelTask &stLevelTask = pConfig.m_GuildLevelTaskList[nLevel-1];
	// 	int nCount = (int)stLevelTask.m_GuildTaskList.count();
	// 	if( nCount <= 0 )
	// 	{
	// 		return;
	// 	}
	// 	int nTaskIdx = (int)GetGlobalLogicEngine()->GetMiscMgr().getRandomNum(nCount);
	// 	if( 0 <= nTaskIdx &&  nTaskIdx < stLevelTask.m_GuildTaskList.count() )
	// 	{
	// 		GuildTask &stTask	= stLevelTask.m_GuildTaskList[nTaskIdx];
	// 		m_nGuildTaskId		= stTask.m_nTaskId;
	// 		m_nGuildTaskSche	= 0;					//进度清0
	// 		m_boUpdateTime		= true;
	// 	}
	// }
}

void CGuild::GetGuildTaskData(int &nTaskId, int &nTaskSche)
{
	nTaskId		= m_nGuildTaskId;
	nTaskSche	= m_nGuildTaskSche;
}
void CGuild::GetGuildMemberBasicData(unsigned int nActordId, int& nSex, int& nJob)
{
	ActorCommonNode* pNode = FindMember(nActordId);
	if (pNode)
	{
		nSex = pNode->BasicData.nSex;
		nJob = pNode->BasicData.nJob;
	}
}
const GuildTask* CGuild::GetGuildTaskById(int nTaskId)
{
	// int nGuildLevel = GetLevel();		//行会等级
	// if( 0 <= nGuildLevel && nGuildLevel < GUILD_LEVEL_MAX )
	// {
	// 	CGuildProvider& pConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	// 	//先查找本等级的行会任务列表，有则返回
	// 	GuildLevelTask &stLevelTask = pConfig.m_GuildLevelTaskList[nGuildLevel-1];
	// 	for(INT_PTR i=0; i<stLevelTask.m_GuildTaskList.count(); i++)
	// 	{
	// 		const GuildTask *pTask = &stLevelTask.m_GuildTaskList[i];
	// 		if( pTask->m_nTaskId == nTaskId )
	// 		{
	// 			return pTask;
	// 		}
	// 	}

	// 	//再查找全部列表
	// 	for(int nLevel=0; nLevel<nGuildLevel; nLevel++)
	// 	{
	// 		GuildLevelTask &stLevelTask = pConfig.m_GuildLevelTaskList[nLevel];
	// 		for(INT_PTR i=0; i<stLevelTask.m_GuildTaskList.count(); i++)
	// 		{
	// 			const GuildTask *pTask = &stLevelTask.m_GuildTaskList[i];
	// 			if( pTask->m_nTaskId == nTaskId )
	// 			{
	// 				return pTask;
	// 			}
	// 		}
	// 	}
	// }
	return NULL;
}

/*
将指定玩家的行会审核信息清空，不返还行会贡献
*/
void CGuild::ClearGuildDepotItemExchange( unsigned int nActorId )
{
	for( INT_PTR i=0; i < m_GuildDepotBag.count(); i++ )
	{
		GUILDDEPOTITEM& stDepotItem = m_GuildDepotBag[i];
		if( stDepotItem.nExchangeActorId > 0 && stDepotItem.nExchangeActorId == nActorId )
		{
			stDepotItem.nExchangeActorId		= 0;
			stDepotItem.szExchangeActorName[0]	= 0;
			stDepotItem.nExchangeTime			= 0;
			m_bSaveDepotRecord		= true;		//需要存储
		}
	}
	NotifyUpdateDepot();									//通知客户端重新请求一次
}

void CGuild::GetGuildEventLog(GUILDEVENTRECOUD &stEventRecord, int nGuildEventId, int nParam1, int nParam2, int nParam3, 
	char* szParam4, char* szParam5)
{
	switch( nGuildEventId )
	{
	case enGuildEvent_Create :
		{
			if( !szParam4 )
			{
				return;
			}
			LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmGuildEventCreate);
			sprintf_s(stEventRecord.nEventMsg, sizeof(stEventRecord.nEventMsg), sFormat, szParam4 );
			return;
		}
	case enGuildEvent_MemberJoin :
		{
			if( !szParam4 )
			{
				return;
			}
			LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmGuildEventMemberJoin);
			sprintf_s(stEventRecord.nEventMsg, sizeof(stEventRecord.nEventMsg), sFormat, szParam4 );
			return;
		}
	case enGuildEvent_MemberLeft :
		{
			if( !szParam4 || !szParam5  )
			{
				return;
			}
			LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmGuildEventMemberLeft);
			sprintf_s(stEventRecord.nEventMsg, sizeof(stEventRecord.nEventMsg), sFormat, szParam4 );
			return;
		}
	case enGuildEvent_TickMember :
		{
			if( !szParam4 || !szParam5 )
			{
				return;
			}
			LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmGuildEventTickMember);
			sprintf_s(stEventRecord.nEventMsg, sizeof(stEventRecord.nEventMsg), sFormat, szParam4, szParam5);
			return;
		}
	case enGuildEvent_LeaderChange :
		{
			if( !szParam4 || !szParam5 )
			{
				return;
			}
			LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmGuildEventLeaderChange);
			sprintf_s(stEventRecord.nEventMsg, sizeof(stEventRecord.nEventMsg), sFormat, szParam4, szParam5);
			return;
		}
	case enGuildEvent_AddOfficer :
		{
			if( !szParam4 || !szParam5 )
			{
				return;
			}
			LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmGuildEventAddOfficer);
			sprintf_s(stEventRecord.nEventMsg, sizeof(stEventRecord.nEventMsg), sFormat, szParam4, szParam5);
			return;
		}
	case enGuildEvent_DelOfficer :
		{
			if( !szParam4 || !szParam5 )
			{
				return;
			}
			LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmGuildEventDelOfficer);
			sprintf_s(stEventRecord.nEventMsg, sizeof(stEventRecord.nEventMsg), sFormat, szParam4, szParam5);
			return;
		}
	case enGuildEvent_levelUp :
		{
			LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmGuildEventlevelUp);
			sprintf_s(stEventRecord.nEventMsg, sizeof(stEventRecord.nEventMsg), sFormat, nParam1);
			return;
		}
	case enGuildEvent_Impeach :
		{
			if( !szParam4 || !szParam5 )
			{
				return;
			}
			LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmGuildEventImpeach);
			sprintf_s(stEventRecord.nEventMsg, sizeof(stEventRecord.nEventMsg), sFormat, szParam4, szParam5);
			return;
		}
	// case enGuildEvent_PutInDepot :
	// 	{
	// 		if( !szParam4 || !szParam5 )
	// 		{
	// 			return;
	// 		}
	// 		LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmGuildEventPutInDepot);
	// 		sprintf_s(stEventRecord.nEventMsg, sizeof(stEventRecord.nEventMsg), sFormat, szParam4, szParam5);
	// 		return;
	// 	}
	// case enGuildEvent_GetOutDepot :
	// 	{
	// 		if( !szParam4 || !szParam5 )
	// 		{
	// 			return;
	// 		}
	// 		LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmGuildEventGetOutDepot);
	// 		sprintf_s(stEventRecord.nEventMsg, sizeof(stEventRecord.nEventMsg), sFormat, szParam4, szParam5);
	// 		return;
	// 	}
	}
}

//---------------------攻城战----begin------------------------------------
void CGuild::NotifyGuildSiegeCmdPanel(unsigned char nFlag)
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			CActor* toActor = (CActor*)pEntity;
			SendGuildSiegeCmdPanel(toActor, nFlag);
			if( nFlag == 1 )
			{
				SendGuildSiegeCommand( toActor );
			}
		}
	}
}

void CGuild::SendGuildSiegeCmdPanel(CActor* pActor, unsigned char nFlag)
{
	

	//OutputMsg(rmWaning,_T("SendGuildSiegeCmdPanel, name=%s, nFlag=%d"), pActor->GetEntityName(), nFlag);
}

/*设置指令
nCmdIdx：0，清除指令
*/
void CGuild::SetGuildSiegeCommand(CActor* pActor, int nCmdIdx, int nCmdMsgIdx)
{
	m_nGuildSiegeCmdIdx		= nCmdIdx;
	m_nGuildSiegeCmdMsgIdx	= nCmdMsgIdx;
	if( !pActor || nCmdIdx <=0 )			//清除
	{
		m_nGuildSiegeCmdSceneId = 0;
		m_nGuildSiegeCmdPosX    = 0;
		m_nGuildSiegeCmdPosY	= 0;
		m_nGuildSiegeCmdTime	= 0;
		return;
	}

	m_nGuildSiegeCmdSceneId = pActor->GetSceneID();
	pActor->GetPosition(m_nGuildSiegeCmdPosX, m_nGuildSiegeCmdPosY);
	m_nGuildSiegeCmdTime = GetGlobalLogicEngine()->getMiniDateTime();			//下达命令的时间
	NotifyGuildSiegeCommand();
}

void CGuild::NotifyGuildSiegeCommand()
{

	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_ActorOnLine);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		EntityHandle& hHandle = pNode->m_Data;
		CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		if (pEntity && pEntity->GetType() == enActor)
		{
			CActor* toActor = (CActor*)pEntity;
			SendGuildSiegeCommand(toActor);
		}
	}
}

void CGuild::SendGuildSiegeCommand(CActor* pActor)
{
	
}

//---------------------攻城战----end------------------------------------

void CGuild::SendGuildEventLog(CActor* pActor)
{
	CActorPacket pack;
	CDataPacket &data=pActor->AllocPacket(pack); 
	data << (BYTE)enGuildSystemID << (BYTE)sGuildLog;
	data << (BYTE)m_EventCount;
	for(int i = 0; i < m_EventCount; i++)
	{
		GUILDEVENTRECOUD& event = m_EventRecord[i];

		data << (unsigned int)event.aTime;
		data << (BYTE)event.mEventId;
		data.writeString(event.mParam4);
		data.writeString(event.mParam5);
		data <<(int)event.mParam1;
		data << (int)event.mParam2;
	}

	pack.flush();
}

bool CGuild::decrease(const ActorCommonNode &FirstActor, const ActorCommonNode &SecondActor)
{
	if (FirstActor.BasicData.nTotalGx > SecondActor.BasicData.nTotalGx)
	{
		return true;
	}
	else if (FirstActor.BasicData.nTotalGx == SecondActor.BasicData.nTotalGx)
	{
		return FirstActor.BasicData.nActorId > SecondActor.BasicData.nActorId;
	}
	return false;
}

void CGuild::AfterDonateDealGuild()
{
	sort(m_ActorOffLine.begin(), m_ActorOffLine.end(), CGuild::decrease);
	int nSize = m_ActorOffLine.size();
	m_GuildTkCount = 0;
	m_GuildEliteCount = 0;
	for(int i = 0; i < nSize; i++)
	{
		ActorCommonNode& actor = m_ActorOffLine[i];
		if(actor.BasicData.nTotalGx <= 0)
			continue;
		if(actor.BasicData.nType == smGuildAssistLeader || actor.BasicData.nType == smGuildLeader)
			continue;
		
		CEntity* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(actor.hActorHandle);
		if( m_GuildTkCount < MaxGuildPosCount(smGuildTangzhu) )
		{		
			if(actor.BasicData.nType != smGuildTangzhu)
			{
				AddEventRecord( "", enGuildEvent_UpOfferice,  smGuildTangzhu, 0, 0, (char *)(actor.sActorName), NULL);
			}
			actor.BasicData.nType = smGuildTangzhu;
			SaveGuildMember(actor);
			m_GuildTkCount++;
			if(pEntity)
			{
				((CActor*)pEntity)->GetGuildSystem()->SetGuildPos(smGuildTangzhu);
			}
			continue;
		}
		
		if(m_GuildEliteCount < MaxGuildPosCount(smGuildElite) )
		{

			if(actor.BasicData.nType != smGuildElite)
			{
				AddEventRecord( "", enGuildEvent_UpOfferice,  smGuildElite,0, 0, (char *)(actor.sActorName), NULL);	
			}
			actor.BasicData.nType = smGuildElite;
			SaveGuildMember(actor);
			m_GuildEliteCount++;
			
			if(pEntity)
			{
				((CActor*)pEntity)->GetGuildSystem()->SetGuildPos(smGuildElite);
			}
			continue;
		}

		if(actor.BasicData.nType != smGuildCommon)
		{
			actor.BasicData.nType = smGuildCommon;
			AddEventRecord( "", enGuildEvent_UpOfferice,  smGuildCommon,0, 0, (char *)(actor.sActorName), NULL);
			SaveGuildMember(actor);
			if(pEntity)
			{
				((CActor*)pEntity)->GetGuildSystem()->SetGuildPos(smGuildCommon);
			}
		}

	}
}


VOID CGuild::SaveGuildMember(ActorCommonNode& actor)
{
	CDataPacket& DataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveActorGuild);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId << nServerId;
	DataPacket << (unsigned int)m_nGid;//没加入门派，或者刚给开除了，gid设置0
	DataPacket << (unsigned int)actor.BasicData.nActorId;
	DataPacket << (int)actor.BasicData.nTotalGx;
	DataPacket << (int)(actor.BasicData.nType << 16);
	DataPacket << (UINT)actor.BasicData.nZjNext;
	DataPacket << (UINT)actor.BasicData.nZzNext;
	DataPacket << (int)actor.BasicData.nGuildTiTle;
	DataPacket << (int)actor.BasicData.nTime;		//(int)m_pEntity->GetLoginTime();
	DataPacket << (int)actor.BasicData.nTotalGx;
	DataPacket << (int)actor.BasicData.nModelId;		//(int)m_pEntity->GetProperty<int>(PROP_ENTITY_MODELID);

	DataPacket << (int)actor.BasicData.nModelId;
	DataPacket << (int)0;
	DataPacket << (int)0;

	DataPacket << (int)0;
	DataPacket << (int)0;
	DataPacket << (int)0;

	DataPacket << (int)0;

	DataPacket << (unsigned int)actor.BasicData.nJoinTime;		//加入行会的时间

	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
}


void CGuild::AddMember(ActorCommonNode& actor)
{
	CDataPacket& DataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcAddMember);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId << nServerId;
	DataPacket << m_nGid;
	DataPacket << (unsigned int)actor.BasicData.nActorId;
	DataPacket << (int)(actor.BasicData.nType << 16);//堂口不定
	DataPacket << (int)actor.BasicData.nTotalGx;
	DataPacket << (int)0;
	DataPacket << (int)0;
	DataPacket << (int)0;
	DataPacket << (int)0;
	DataPacket << (int)0;
	DataPacket << (int)0;
	DataPacket << (int)actor.BasicData.nJoinTime;					//加入行会的时间
	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);

	CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID((unsigned int)actor.BasicData.nActorId);
	if (pActor)
	{
		pActor->ResetShowName();
	}
}



void CGuild::LoadSbkOffLinedata()
{
	int nSize = m_ActorOffLine.size();
	for(int i = 0; i < nSize; i++)
	{
		ActorCommonNode& actor = m_ActorOffLine[i];
		if(actor.BasicData.nType == smGuildAssistLeader || actor.BasicData.nType == smGuildLeader)
		{
			// 最后只能从数据库找
			int nActorId = actor.BasicData.nActorId;
			if (!GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId) && 
			!GetGlobalLogicEngine()->GetOfflineUserMgr().GetUserByActorId(nActorId))
			{
				GetGlobalLogicEngine()->GetOfflineUserMgr().LoadUser(nActorId);
			}
		}
	}
}


void CGuild::getsbkOfflineData(CDataPacket& data, int& nCount)
{
	int nSize = m_ActorOffLine.size();
	for(int i = 0; i < nSize; i++)
	{
		ActorCommonNode& actor = m_ActorOffLine[i];
		if(actor.BasicData.nType == smGuildAssistLeader || actor.BasicData.nType == smGuildLeader)
		{
			data <<(BYTE)actor.BasicData.nType;
			GetGlobalLogicEngine()->GetOfflineUserMgr().ViewOffLineActorToPacket(actor.BasicData.nActorId, data);
			nCount++;
		}
	}
}


//跨服相关
void CGuild::SendSbkGuildRankAward(int nRankId)
{
	auto& Cfg = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	std::vector<ACTORAWARD> awards;
	Cfg.GetSbkGuildAward(nRankId, awards);
	int nSize = m_ActorOffLine.size();
	for(int i = 0; i < nSize; i++)
	{
		ActorCommonNode& actor = m_ActorOffLine[i];
		if(actor.BasicData.nType == smGuildAssistLeader || actor.BasicData.nType == smGuildLeader)
		{
			CMailSystem::SendCrossServerMail(actor.BasicData.nActorId,
			Cfg.rankAwardMailTT, Cfg.rankAwardMailCT,awards);
		}
	}
}

void CGuild::SendSbkGuildAward()
{
	auto& Cfg = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	CMailSystem::SendCrossServerMail(m_nLeaderId,Cfg.czrewardsMailTT, Cfg.czrewardsMailCT,Cfg.czrewards);
}