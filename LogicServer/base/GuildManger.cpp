#include "StdAfx.h"
#include "GuildManger.h"
#include "../base/Container.hpp"
static const LPCTSTR g_szGuildSbkSiege = _T("GuildSiegeRanking.txt");


using namespace jxInterSrvComm::DbServerProto;

CBufferAllocator*		CGuildComponent::m_pAllocator;

CGuildComponent::CGuildComponent()
{
	m_pAllocator = new CBufferAllocator();
	m_pGuildList = (CList<CGuild*>*)m_pAllocator->AllocBuffer(sizeof(*m_pGuildList));
	new(m_pGuildList)CList<CGuild*>(m_pAllocator);
	m_pRank = (CList<CGuild*>*)m_pAllocator->AllocBuffer(sizeof(*m_pRank));
	new(m_pRank) CList<CGuild*>(m_pAllocator);
	m_GuildId = 0;
	m_GuildSiegeStartFlag=0;
	m_DbInitGuild = false;
	m_GuidlDetailLoadCount = 0;
	m_boTodaySign = 0;
	m_bIsModifyData = false;
	m_bGuildClose = false;
	m_bGuildClear = false;
	m_GuildApplyResult.clear();
	Load();//读入数据
}

CGuildComponent::~CGuildComponent()
{
	CLinkedNode<CGuild*> *pNode;
	{
		CLinkedListIterator<CGuild*> it(*m_pGuildList);
		for (pNode = it.first(); pNode; pNode = it.next())
		{
			pNode->m_Data->~CGuild();
			m_pAllocator->FreeBuffer(pNode->m_Data);
		}
	}

	//释放资源
	m_pGuildList->~CList<CGuild*>();
	m_pAllocator->FreeBuffer(m_pGuildList);
	m_pRank->~CList<CGuild*>();
	m_pAllocator->FreeBuffer(m_pRank);

	SafeDelete(m_pAllocator);


}

VOID  CGuildComponent::Destroy()
{
#ifdef _USE_TRY_CATCH
#ifdef WIN32
	__try
#endif 
#endif 
	{	
		CLinkedNode<CGuild*> *pNode;
		CLinkedListIterator<CGuild*> it(*m_pGuildList);
		for (pNode = it.first(); pNode; pNode = it.next())
		{
			CGuild* pGuild = pNode->m_Data;
			if(pGuild)
			{
				pGuild->Destroy();
			}
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

void CGuildComponent::SaveAllGuildEvent()
{
	CLinkedNode<CGuild*> *pNode;
	CLinkedListIterator<CGuild*> it(*m_pGuildList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		pNode->m_Data->SaveAllGuildEvent();
	}
	
	SaveGuildSiegeRanking();
}

CGuild* CGuildComponent::FindGuild(unsigned int nGid)
{
	//这里m_pGuildList是链表，所以用遍历查找，可以考虑用二叉树
	CLinkedNode<CGuild*> *pNode;
	CLinkedListIterator<CGuild*> it(*m_pGuildList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		if (pNode->m_Data->m_nGid == nGid)
			return (pNode->m_Data);
	}
	return NULL;
}

CGuild* CGuildComponent::FindGuildByRank(int nRank)
{
	CLinkedNode<CGuild*> *pNode;
	CLinkedListIterator<CGuild*> it(*m_pGuildList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		if (pNode->m_Data->m_nRank == nRank)
			return (pNode->m_Data);
	}
	return NULL;
}

CGuild* CGuildComponent::FindGuildByName(const char* nName)
{
	if(nName == NULL) return NULL;

	CLinkedNode<CGuild*> *pNode;
	CLinkedListIterator<CGuild*> it(*m_pGuildList);

	for (pNode = it.first(); pNode; pNode = it.next())
	{
		if(0== strcmp(nName,pNode->m_Data->m_sGuildname) )
			return (pNode->m_Data);
	}
	return NULL;
}

char* CGuildComponent::GetGuildNameByRank(int nRank)
{
	CGuild* pGuild = FindGuildByRank(nRank);

	if(pGuild == NULL)
		return NULL;

	return pGuild->m_sGuildname;
}

const char* CGuildComponent::GetGuildLeaderNameByRank(int nRank)
{
	CGuild* pGuild = FindGuildByRank(nRank);

	if(pGuild == NULL)
		return NULL;
	return pGuild->FindGuildMemberName(pGuild->m_nLeaderId);
}

int CGuildComponent::GetGuildMemNumByRank(int nRank)
{
	CGuild* pGuild = FindGuildByRank(nRank);

	if(pGuild == NULL)
		return 0;

	return (int)pGuild->m_ActorOffLine.size();
}

int CGuildComponent::GetGuildCoinByRank(int nRank)
{
	CGuild* pGuild = FindGuildByRank(nRank);

	if(pGuild == NULL)
		return 0;

	return pGuild->GetCoin();
}

bool CGuildComponent::HasGuildByRank(int nRank)
{
	CGuild* pGuild = FindGuildByRank(nRank);

	if(pGuild == NULL)
		return false;
	
	return true;
}

void CGuildComponent::SetGuildLeaderByname(char* nGuildname,char* nName)
{
	CGuild* pGuild = FindGuildByName(nGuildname);

	if(pGuild == NULL)
		return;

	pGuild->GmChangeLeader(nName);
}
void CGuildComponent::OnGuildLeaderChange(CGuild* pGuild, unsigned int nOldLeaderId, unsigned int nNewLeaderId, int nChangeType)
{
	//行会首领替换这里统一处理，包括禅让，弹劾，gm后台设置,方便维护
	//弹劾注意部分逻辑是弹劾副帮主等其他职位使用的。
	if (pGuild == NULL || nOldLeaderId == nNewLeaderId)
	{
		return;
	}
	ActorCommonNode* pOldNode = pGuild->FindMember(nOldLeaderId);
	ActorCommonNode* pNewNode = pGuild->FindMember(nNewLeaderId);
	if (!pOldNode || !pNewNode)
	{
		return;
	}
	//后台支持下不在线帮主替换
	CActor* pOldActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nOldLeaderId);
	CActor* pNewActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nNewLeaderId);

	pNewNode->BasicData.nType		= smGuildLeader;
	pGuild->m_nLeaderId			= nNewLeaderId;
	pOldNode->BasicData.nType = smGuildCommon;

	_asncpytA(pGuild->m_sLeaderName,pNewNode->sActorName);
	pGuild->SaveGuildMember(*pNewNode);
	pGuild->SaveGuildMember(*pOldNode);
	if (pOldActor)
	{
		pOldActor->GetGuildSystem()->SetGuildPos(smGuildCommon);			//原帮主直接变成成员
	}
	if (pNewActor)
	{
		pNewActor->GetGuildSystem()->SetGuildPos(smGuildLeader);	//对方设置为帮主
		//pActor->GetAchieveSystem().OnEvent(eAchieveEventGuildMaster,1);
		//m_nTitle = 0;
		//m_pGuild->SendChangeTitleToAll(m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID),0);
	}

	if(pGuild->m_nGid == m_GuildId)		//沙巴克行会
	{
		//const CGuildProvider& pConfig	= GetLogicServer()->GetDataProvider()->GetGuildConfig();
		//int nLeaderTitleId	= pConfig.nSbkOwnerTitleId;
		//int nMemTitleId		= pConfig.nSbkMemberTitleId;
		if (pOldActor)
		{
			pOldActor->GetGuildSystem()->TakeOnOffDragonRobe(0);
			//pOldActor->GetNewTitleSystem().delNewTitle(nLeaderTitleId);
			//pOldActor->GetNewTitleSystem().addNewTitle(nMemTitleId);
		}
		if (pNewActor)
		{
			//pNewActor->GetNewTitleSystem().delNewTitle(nMemTitleId);
			//pNewActor->GetNewTitleSystem().addNewTitle(nLeaderTitleId);
			pNewActor->GetGuildSystem()->TakeOnOffDragonRobe(1);
			int nPos = pNewActor->GetGuildSystem()->GetGuildCityPos();
			SetCityPos(nPos,0);
			SetCityPos(stRealMaster, nNewLeaderId);
		}
		SetGuildSiegeMasterStatue();
	}
	if (pOldActor)
	{
		pOldActor->CollectOperate(CEntityOPCollector::coRefAbility);
		pOldActor->ResetShowName();
		pOldActor->GetGuildSystem()->SendGuildInfoResult();
		pOldActor->GetGuildSystem()->SendMemberList();
	}
	if (pNewActor)
	{
		pNewActor->CollectOperate(CEntityOPCollector::coRefAbility);
		pNewActor->ResetShowName();
		pNewActor->GetGuildSystem()->SendGuildInfoResult();
		pNewActor->GetGuildSystem()->SendMemberList();
	}
	pGuild->Save();

	// char chatMsg[128]={0};
	// LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildPosChange);
	// sprintf_s(chatMsg, sizeof(chatMsg), sFormat, ( char *)(pNewNode->sActorName), pGuild->GetGuildTitleName(smGuildLeader));
	//((CActor *)m_pEntity)->GetChatSystem()->SendChat(ciChannelGuild, chatMsg);
	// pGuild->BroadCastMsgInGuildChannel(chatMsg);
}

unsigned int CGuildComponent::FindGuildIdByHandle(unsigned int nHandle)
{
	CLinkedNode<CGuild*> *pNode;
	CLinkedListIterator<CGuild*> it(*m_pGuildList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		if (pNode->m_Data->m_BossChanllengeHandle == nHandle)
			return (pNode->m_Data->m_nGid);
	}
	return 0;
}

/*
竞价调整行会排名
*/
VOID CGuildComponent::AdjustRank(CGuild* pGuild)
{
	if (pGuild == NULL || !pGuild->m_boStatus) return;
	int i = 1;
	bool boNewPos = FALSE;
	int nGuildMemSize = pGuild->m_ActorOffLine.size();
	CLinkedNode<CGuild*> *pNode;
	CLinkedListIterator<CGuild*> it(*m_pRank);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CGuild* pCurrGuild = pNode->m_Data;
		//清除无用的帮派
		if (!pCurrGuild->m_boStatus)
		{
			it.remove(pNode);
			continue;
		}
		if (pGuild == pCurrGuild)
		{
			//找到自己
			it.remove(pNode);
			continue;
		}
		//比较等级和繁荣度
		if (!boNewPos)
		{
			// if ((pGuild->m_nBidCoin >=pCurrGuild->m_nBidCoin) || ((pGuild->m_nBidCoin == pCurrGuild->m_nBidCoin) && (pGuild->m_nBidCoin < pCurrGuild->m_nBidCoin)))
			// {
			// 	//插在它前面
			// 	m_pRank->linkBefore(pGuild,pNode);
			// 	pGuild->m_nRank = i++;//排名
			// 	boNewPos = TRUE;
			// 	pCurrGuild->m_nRank = i++;
			// 	continue;
			// }
			int nGuildMemCount = pCurrGuild->m_ActorOffLine.size();
			if ((pGuild->m_MainHallLev > pCurrGuild->m_MainHallLev) || ((pGuild->m_MainHallLev == pCurrGuild->m_MainHallLev) && (nGuildMemSize > nGuildMemCount)))
			{
				//插在它前面
				m_pRank->linkBefore(pGuild,pNode);
				pGuild->m_nRank = i++;//排名
				boNewPos = TRUE;
				pCurrGuild->m_nRank = i++;
				continue;
			}
		}
		pCurrGuild->m_nRank = i++;		
	}
	if (!boNewPos)
	{
		m_pRank->linkAtLast(pGuild);
		pGuild->m_nRank = i;
	}
}

//将一个新行会增加到列表中
CGuild* CGuildComponent::AddGuildToList(unsigned int nGid,WORD nLevel,int nCoin,int nYType,int nBidCoin,int nTime,int nFlag,int sFlag,LPCSTR qGroupId,LPCSTR yGroupId,
	LPCSTR sGuildname,LPCSTR sFoundname,LPCSTR sInMemo,LPCSTR sOutMemo,LPCSTR nyMemo,LPCSTR groupMemo)
{
	CGuild* pGuild = (CGuild*)m_pAllocator->AllocBuffer(sizeof(CGuild));
	new(pGuild)CGuild();
	pGuild->m_nGid = nGid;

	//这里设置值后不用再保存到数据库了，因为本身就是刚刚从数据库读出来的
	pGuild->SetGuildLevel( nLevel,FALSE,false, false);
	_asncpytA(pGuild->m_sGuildname,sGuildname);
	// _asncpytA(pGuild->m_sFoundname,sFoundname);
	_asncpytA(pGuild->m_sLeaderName,sFoundname);
	pGuild->SetInMemo(sInMemo,FALSE);
	pGuild->SetOutMemo(sOutMemo,FALSE);

	pGuild->m_nBidCoin	= nBidCoin;
	pGuild->SetCoin(nCoin);
	pGuild->m_CreateTime = nTime;
	pGuild->m_AddMemberFlag = nFlag;

	if(qGroupId)
		_asncpytA(pGuild->m_sQqGroupId,qGroupId);
	if(yGroupId)
		_asncpytA(pGuild->m_sYyGroupId,yGroupId);

	pGuild->SetYyType(nYType,false);
	pGuild->SetYyMemo(nyMemo,false);

	pGuild->SetGuildGroupMemo(groupMemo,FALSE);

	pGuild->m_SignUpFlag = sFlag;

	m_pGuildList->linkAtLast(pGuild);

	//加入排行榜，调整排名
	AdjustRank(pGuild);

	//pGuild->ResetGuildTreeAndFruitByGuildLevel();		//根据行会等级设置行会神树和果实数据
	return pGuild;
}
void CGuildComponent::SaveGuildSiegeRanking()
{
	CRankingMgr &RankingMgr = GetGlobalLogicEngine()->GetRankingMgr();
	CRanking *pRanking = RankingMgr.GetRanking(enRankGuildSbkSiege);
	if (pRanking == NULL)
	{
		pRanking = RankingMgr.Add(enRankGuildSbkSiege, g_szGuildSbkSiege, 1, 0);
		if (!pRanking)
			return;
	}
	CRankingItem* pItem = pRanking->GetItem(0);
	if (!pItem)
	{
		pItem = pRanking->AddItem(0);
	}
	if(pItem)
	{
		pItem->SetId(m_GuildId);
		for (int i = stRealMaster; i< stMaxGuildCityPos; i++)
		{
			pItem->SetSub(GSRC_RealMasterId-stRealMaster +i,m_PostionId[i]);	
		}
		pItem->SetSub(GCRC_IsAutoSignUp, m_boTodaySign);
	}
	pRanking->Save(g_szGuildSbkSiege,true);	
}
void CGuildComponent::LoadGuildSiegeRanking()
{
	CRankingMgr &RankingMgr = GetGlobalLogicEngine()->GetRankingMgr();

	CRanking *pRanking = RankingMgr.GetRanking(enRankGuildSbkSiege);
	if (pRanking == NULL)
	{
		pRanking = RankingMgr.Add(enRankGuildSbkSiege, g_szGuildSbkSiege, 1, 0);
		if (!pRanking)
			return;
	}
	if (!pRanking->Load(g_szGuildSbkSiege))
	{
		for (int i=0; i< ArrayCount(GuildSiegeRankingColumnName); i++)
		{
			pRanking->AddColumn(GuildSiegeRankingColumnName[i],-1);
		}
	}
	else
	{
		int nCurCount = pRanking->GetColCount();
		if ( nCurCount < GSRC_Count)
		{
			char sBuff[128];
			for (int i = nCurCount; i < GSRC_Count; i++)		//这里是增加新的列
			{
				sprintf_s(sBuff, ArrayCount(sBuff), "%s", GuildSiegeRankingColumnName[i]);
				pRanking->AddColumn(sBuff, -1);
			}
		}
	}
	CRankingItem* pItem = pRanking->GetItem(0);

	if(pItem)
	{
		m_GuildId = pItem->GetId();
		for (int i = stRealMaster; i< stMaxGuildCityPos; i++)
		{
			m_PostionId[i] = atoi(pItem->GetSubData(GSRC_RealMasterId-stRealMaster+i));
		}
		m_boTodaySign = atoi(pItem->GetSubData(GCRC_IsAutoSignUp)) > 0;

		CGuild* pGuild = FindGuild(m_GuildId);
		if (pGuild)
		{
			pGuild->LoadSbkOffLinedata();
		}
	}
	
}






VOID CGuildComponent::Load()
{
	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (!m_DbInitGuild && m_pGuildList->count() <= 0 && pDbClient->connected())
	{
		CDataPacket& DataPacket =pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadGuildList);
		int nServerId = GetLogicServer()->GetServerIndex();
		DataPacket << nServerId << nServerId;
		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);

		CDataPacket& Data =pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadActorApplyResult);
		Data << nServerId << nServerId;
		GetLogicServer()->GetDbClient()->flushProtoPacket(Data);
	}
}

VOID CGuildComponent::LoadGuildDetail(unsigned int nGid)
{
	CDataPacket& DataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadGuildDetail);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId << nServerId;
	DataPacket << (unsigned int)nGid;
	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
}

VOID CGuildComponent::LoadGuildApplyLists(unsigned int nGid)
{
	CDataPacket& DataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadGuildApplyLists);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId << nServerId;
	DataPacket << (unsigned int)nGid;
	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
}


VOID CGuildComponent::SaveGuildApplyReslut()
{
	if(!m_DbInitGuild || !m_bIsModifyData) return;
	CDataPacket& DataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveActorApplyResult);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId << nServerId;
	int nCount = m_GuildApplyResult.size();
	DataPacket <<nCount;
	std::map<unsigned int, unsigned int>::iterator it = m_GuildApplyResult.begin();
	for(; it != m_GuildApplyResult.end(); it++)
	{
		DataPacket<<(unsigned int)(it->first);
		DataPacket<<(unsigned int)(it->second);
	}
	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
}

void CGuildComponent::LoadGuildSkill(unsigned int nGid)
{
	CDataPacket& DataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadGuildSkill);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId << nServerId;
	DataPacket << (unsigned int)nGid;
	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
}

//增加一个帮派
VOID CGuildComponent::SendDbAddGuild(unsigned int nActorid,Uint64 hEntity,LPCSTR sGuildName,LPCSTR sFoundname)
{
	INT_PTR nIndex = 0;
	UserOperData* pData = GetFreeUserOperData(nIndex);
	if (pData == NULL) return;

	pData->hActor = hEntity;
	pData->nTimeOut = GetGlobalLogicEngine()->getMiniDateTime() + GUILDTIMEOUT;//超时时间
	pData->nType = dcAddGuild;

	CDataPacket& DataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcAddGuild);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId << nServerId;
	DataPacket <<  nActorid;
	DataPacket << (int)nIndex;
	DataPacket << (Uint64)(pData->hActor);
	DataPacket.writeString(sGuildName);
	DataPacket.writeString(sFoundname);
	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
}

VOID CGuildComponent::SendDbMsgDeleteGuild(Uint64 hEntity,unsigned int id)
{
	//保存用户内容，有结果返回通知客户
	INT_PTR nIndex = 0;
	UserOperData* pData = GetFreeUserOperData(nIndex);
	if (pData == NULL) return;

	CDataPacket& DataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcDeleteGuild);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId << nServerId;
	DataPacket << (unsigned int)id;
	pData->hActor = hEntity;
	pData->nTimeOut = GetGlobalLogicEngine()->getMiniDateTime() + GUILDTIMEOUT;//超时时间
	pData->nType = dcDeleteGuild;

	DataPacket << (int)nIndex;
	DataPacket << (Uint64)hEntity;
	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
}

bool CGuildComponent::DeleteGuild(CGuild* pGuild,int bType)
{
	//公告一下
	if (pGuild == NULL) return true;

	// LPCTSTR szMailTitle = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildDeleteMailTitle);
	// LPCTSTR szMailCont  = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGuildDeleteMailCont);

	//所有成员退出帮派
	//不在线的成员属性已经在数据库修改，这里只修改在线成员的属性
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	// CLinkedNode<ActorCommonNode> *pNode;
	// CLinkedListIterator<ActorCommonNode> it(pGuild->m_ActorOffLine);
	int nSize = pGuild->m_ActorOffLine.size();
	// for( pNode = it.first(); pNode; pNode = it.next() )
	// {
	for(int i = 0; i < nSize ; i++)
	{
		ActorCommonNode& actor = pGuild->m_ActorOffLine[i];
		//EntityHandle& hHandle = pNode->m_Data;
		//CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
		CEntity* pEntity = NULL;
		if( actor.hActorHandle != 0 )
		{
			pEntity = pEntityMgr->GetEntity(actor.hActorHandle);
		}
		if( pEntity && pEntity->GetType() == enActor )		//玩家在线
		{
			((CActor*)pEntity)->GetGuildSystem()->SetGuild(NULL);
			((CActor*)pEntity)->GetGuildSystem()->SendDeleteGuildResult(bType);
			((CActor*)pEntity)->SetProperty<unsigned int>(PROP_ACTOR_GUILD_ID,0);
			((CActor*)pEntity)->SetProperty<unsigned int>(PROP_ACTOR_GUILD_LEVEL,0);
			((CActor*)pEntity)->ResetShowName();
		}
		if( actor.BasicData.nActorId > 0 ) 
		{
			DeleteJoinGuildRelust(actor.BasicData.nActorId);//清理加入列表
		}
		// else		//玩家离线
		// {
		// 	char MsgBuff[MAX_MSG_COUNT];
		// 	CDataPacket packet(MsgBuff,sizeof(MsgBuff));
		// 	char sText[1024] = {0};
		// 	LPCTSTR sNoticeFormat;
			
		// 	if( bType == 0 )		//帮主解散
		// 		sNoticeFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpDeleteGuildMsg);
		// 	else					//资金不足解散
		// 		sNoticeFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpDeleteGuildOffMsg);

		// 	packet.writeString(sNoticeFormat);
		// 	CMsgSystem::AddOfflineMsg(actor.BasicData.nActorId, CMsgSystem::mtDeleteGuild, MsgBuff, packet.getPosition(), 0);
		// }

		// if( actor.BasicData.nActorId > 0 )			//现在只有资金不足解散了
		// {
		// 	CMailSystem::SendMail(actor.BasicData.nActorId, szMailTitle, szMailCont);
		// }
	}

	pGuild->m_ActorOffLine.clear();
	pGuild->m_ActorOnLine.clear();
	pGuild->m_nLeaderId = 0;
	pGuild->m_nSecLeaderCount = 0;
	pGuild->m_GuildTkCount = 0;
	pGuild->m_WarList.clear();		//对方的warlist也要删除
	pGuild->m_boStatus = false;
	pGuild->m_ApplyList.clear();

	
	//以上是帮派的清理工作
	return true;
}

void CGuildComponent::LoadActorApplyResult(CDataPacketReader& inPacket)
{
	int nCount = 0;
	inPacket >> nCount;
	if(nCount == 0)
	{
		return;
	}
	for(int i = 0; i < nCount; i++)
	{
		unsigned int nActorId = 0;
		inPacket >> nActorId;
		unsigned int nGid = 0;
		inPacket >> nGid;
		std::map<unsigned int, unsigned int>::iterator it = m_GuildApplyResult.find(nActorId);
		if(it == m_GuildApplyResult.end())
		{
			m_GuildApplyResult.insert(std::make_pair(nActorId, nGid));
		}
	}
}
VOID CGuildComponent::LoadGuildListResult( CDataPacketReader &inPacket)
{
	int nCount = 0;
	inPacket >> nCount;
	for (int i = 0; i < nCount; i++)
	{
		WORD nLevel = 0;
		unsigned int nGid;
		inPacket >> nGid;
		inPacket >> nLevel;
		char sGuildname[32];
		inPacket.readString(sGuildname,ArrayCount(sGuildname));
		sGuildname[sizeof(sGuildname)-1] = 0;
		char sLeaderName[32];
		inPacket.readString(sLeaderName,ArrayCount(sLeaderName));
		sLeaderName[sizeof(sLeaderName)-1] = 0;
		char sInMsg[MAXMEMOCOUNT];
		inPacket.readString(sInMsg,ArrayCount(sInMsg));
		sInMsg[sizeof(sInMsg)-1] = 0;
		char sOutMsg[MAXMEMOCOUNT];
		inPacket.readString(sOutMsg,ArrayCount(sOutMsg));
		sOutMsg[sizeof(sOutMsg)-1] = 0;

		int nCoin = 0;
		inPacket >> nCoin;
		int nBidCoin = 0;
		inPacket >> nBidCoin;

		int nCreateTime;
		inPacket >> nCreateTime;

		int nFlag = 0;
		inPacket >> nFlag;

		int nHallLevel = 0;
		inPacket >> nHallLevel;

		int exerciseroomlv = 0;
		inPacket >> exerciseroomlv;

		int assemblyhalllv = 0;
		inPacket >> assemblyhalllv;
		int nAutojoinlevel = 0;
		inPacket >> nAutojoinlevel;
		
		unsigned int  actorId = 0;
		inPacket >> actorId;

		int nCertification = 0;
		inPacket >> nCertification;
		//CGuild* pGuild = AddGuildToList(nGid,nLevel,nCoin,nYType,nBidCoin,nCreateTime,nFlag,sFlag,sQGroupId,sYGroupId,sGuildname,sFoundname,sInMsg,sOutMsg,sYyMsg,sGroupMsg);
		CGuild* pGuild = AddGuildToList(nGid,nLevel,nCoin,0,nBidCoin,nCreateTime,nFlag,0,0,0,sGuildname,sLeaderName,sInMsg,sOutMsg);

		if(pGuild)
		{
			pGuild->m_MainHallLev = nHallLevel;
			if(pGuild->m_MainHallLev < 1)
			{
				pGuild->m_MainHallLev = 1;
			}

			pGuild->m_ExerciseRoomlv = exerciseroomlv;
			if(pGuild->m_ExerciseRoomlv < 1)
			{
				pGuild->m_ExerciseRoomlv = 1;
			}

			pGuild->m_AssemblyHallLv = assemblyhalllv;
			if(pGuild->m_AssemblyHallLv < 1)
			{
				pGuild->m_AssemblyHallLv = 1;
			}

			pGuild->m_nAutoJoinLevel	= nAutojoinlevel;

			pGuild->m_nCertification = nCertification;
			pGuild->m_nLeaderId = actorId;
		}
	
		//一开始就获取成员，以后想办法优化
		LoadGuildDetail(nGid);
		LoadGuildEvent(nGid);
		LoadGuildApplyLists(nGid);
	}
	// if (nCount == 0 && !m_DbInitGuild)
	// {
	// 	m_DbInitGuild = true;
	// 	LoadGuildSiegeRanking();//加载帮派成员读排行榜
	// }
}



void CGuildComponent::LoadGuildEvent(unsigned int nGid)
{
	CDataPacket& DataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadGuildEvent);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId << nServerId;
	DataPacket << (unsigned int)nGid;
	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
}

void CGuildComponent::LoadGuildDepot(unsigned int nGid)
{
	CDataPacket& DataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadGuildDepotItem);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId << nServerId;
	DataPacket << (unsigned int)nGid;
	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
}

void CGuildComponent::LoadGDepotRecord(unsigned int nGid)
{
	CDataPacket& DataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadGuildDepotRecord);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId << nServerId;
	DataPacket << (unsigned int)nGid;
	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
}

/*每周行会维护
每周日凌晨扣除行会资金，资金不足则删除行会
*/
void CGuildComponent::MaintainGuildWeekly()
{
	// CGuildProvider& guildConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	// CLinkedNode<CGuild*> *pNode;
	// CLinkedListIterator<CGuild*> it(*m_pGuildList);
	// int i = 0;
	// int nNeedCoin = guildConfig.nMinGuildCoin;
	// for (pNode = it.first(); pNode; pNode = it.next())
	// {
	// 	CGuild* pGuild = pNode->m_Data;
	// 	if (pGuild)
	// 	{
	// 		pGuild->m_GuildChangleTimes = 0;
	// 		const GuildLevelConfig* pConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig().GetLevelConfig(pGuild->GetLevel());
	// 		int curCoin = (-1) * pConfig->nWeekDecCoin;			//扣除行会维护资金
	// 		//int curCoin = (-1) * GetLogicServer()->GetDataProvider()->GetGuildConfig().nDecGuildCoin;
	// 		//pGuild->ChangeGuildCoin(curCoin * pGuild->m_MainHallLev);
	// 		pGuild->ChangeGuildCoin(curCoin,0,"MaintainWeek");
	// 		if(pGuild->GetCoin() <= nNeedCoin)
	// 		{
	// 			SendDbMsgDeleteGuild(0,pGuild->m_nGid);
	// 		}
	// 	}
	// }
}

int CGuildComponent::LogSenderDelGuild(char* sGuildName)
{
	CGuild* pGuild = FindGuildByName(sGuildName);
	int nResult = 0;
	if(pGuild)
	{
		SendDbMsgDeleteGuild(0,pGuild->m_nGid);
		nResult = 1;
	}
	return nResult;
}



VOID CGuildComponent::LoadGuildSkillResult( CDataPacketReader & inPacket ) 
{
	unsigned int nGuildId = 0;
	inPacket >> nGuildId;//帮派id
	CGuild* pGuild = FindGuild(nGuildId);

	if (pGuild == NULL)
	{
		OutputMsg(rmError,_T("No Guild in LoadGuildDetailResult(),GuildId=%d"),nGuildId);
		return;
	}

	int nCount = 0;

	//加载帮派技能
	inPacket >> nCount;
	if (nCount > 0)
	{
		pGuild->m_GuildSkillList.reserve(nCount);
		pGuild->m_GuildSkillList.trunc(nCount);
		for (int i = 0; i < nCount; i++)
		{
			GUILDSKILLS& mGuildSkill = pGuild->m_GuildSkillList[i];
			inPacket >> mGuildSkill;
		}
	}
	else
	{
		InitGuildSKill(pGuild);
	}
}

VOID CGuildComponent::LoadGuildEventResult( CDataPacketReader & inPacket )
{
	unsigned int nGuildId = 0;
	inPacket >> nGuildId;//帮派id
	CGuild* pGuild = FindGuild(nGuildId);
	int nCount = 0;

	inPacket >> nCount;
	
	if(nCount > MAX_EVENTCOUNT)
	{
		nCount = MAX_EVENTCOUNT;
	}
	
	
	if (nCount > 0 && pGuild)
	{
		pGuild->m_EventCount = nCount;
		for (int i = 0; i < nCount; i++)
		{
			GUILDEVENTRECOUD& mGuildEvent = pGuild->m_EventRecord[i];
			inPacket >> mGuildEvent.aTime;
			inPacket >> mGuildEvent.mEventId;

			inPacket >> mGuildEvent.mParam1;
			inPacket >> mGuildEvent.mParam2;
			inPacket >> mGuildEvent.mParam3;

			inPacket.readString(mGuildEvent.mParam4,ArrayCount(mGuildEvent.mParam4));
			inPacket.readString(mGuildEvent.mParam5,ArrayCount(mGuildEvent.mParam5));

			//pGuild->BuildUpEventRecord(i,mGuildEvent.mEventId,mGuildEvent.mParam1,mGuildEvent.mParam2,mGuildEvent.mParam3,mGuildEvent.mParam4,mGuildEvent.mParam5);
			pGuild->GetGuildEventLog(mGuildEvent, mGuildEvent.mEventId,mGuildEvent.mParam1,mGuildEvent.mParam2,mGuildEvent.mParam3,mGuildEvent.mParam4,mGuildEvent.mParam5);
			//inPacket.readString(mGuildEvent.nEventMsg,ArrayCount(mGuildEvent.nEventMsg));

			//OutputMsg(rmError,_T("Guild in LoadGuildEventResult(),nEventMsg=%s"),mGuildEvent.nEventMsg);
		}
	}
}

void CGuildComponent::LoadGuildDepotResult(CDataPacketReader & inPacket)
{
	unsigned int nGuildId = 0;
	inPacket >> nGuildId;//帮派id
	CGuild* pGuild = FindGuild(nGuildId);
	int nCount = 0;

	inPacket >> nCount;
	CUserItem userItem;
	CLogicEngine *pEngineer = GetLogicServer()->GetLogicEngine();
	if(nCount > 0 && pGuild)
	{
		for(int i=0;i<nCount;i++)
		{
			GUILDDEPOTITEM nItem;

			inPacket >> nItem.nActorId;
			inPacket >> nItem.nCoinType;
			inPacket >> nItem.nCoinNum;

			inPacket >> nItem.nExchangeActorId;
			inPacket.readString(nItem.szExchangeActorName, ArrayCount(nItem.szExchangeActorName));
			inPacket >> nItem.nExchangeTime;
			userItem << inPacket;
			// INT_PTR nReadSize = inPacket.readBuf(&userItem,sizeof(CUserItem));
			// if( sizeof(CUserItem) !=  nReadSize) //读取内存
			// {
			// 	OutputMsg(rmError,"[%s] read net io error,size=%d ",__FUNCTION__,(int) nReadSize);
			// 		continue;
			// }
			CUserItem *pUerItem = pEngineer->CopyAllocUserItem(false,&userItem); //不申请序列号	
			if(pUerItem)
			{
				nItem.nUserItem = pUerItem;
			}
			else
			{
				continue;
			}
			//inPacket >> (*nItem.nUserItem);
			nItem.nExitFlag = 0;

			pGuild->m_GuildDepotBag.add(nItem);
		}
	}
}

void CGuildComponent::LoadDepotRecordResult(CDataPacketReader & inPacket)
{
	unsigned int nGuildId = 0;
	inPacket >> nGuildId;//帮派id
	CGuild* pGuild = FindGuild(nGuildId);
	int nCount = 0;

	inPacket >> nCount;

	if(nCount > 0 && pGuild)
	{
		for(int i=0;i<nCount;i++)
		{
			GUILDDEPOTMSG nRecord;

			inPacket >> nRecord.nType;
			inPacket >> nRecord.nTime;
			inPacket.readString(nRecord.nActorName,ArrayCount(nRecord.nActorName));
			inPacket.readString(nRecord.nItemName,ArrayCount(nRecord.nItemName));
			inPacket >> nRecord.nNum;
			inPacket >> nRecord.nCoinType;
			inPacket >> nRecord.nCoinNum;

			pGuild->m_GuildDepotRecord.add(nRecord);
		}
	}
}


/*
装载行会详细数据

获取从CLogicDBReqestHandler::LoadGuildDetail()发送的节点数据
*/
VOID CGuildComponent::LoadGuildDetailResult( CDataPacketReader & inPacket ) 
{

	unsigned int nGuildId = 0;
	inPacket >> nGuildId;//帮派id
	CGuild* pGuild = FindGuild(nGuildId);
	if (pGuild == NULL)
	{
		OutputMsg(rmError,_T("No Guild in LoadGuildDetailResult(),GuildId=%u"),nGuildId);
		return;
	}
	if (pGuild->GetMemberCount() > 0)
	{
		OutputMsg(rmError,_T(" Guild Members is Loaded one time before!,GuildId=%u"),nGuildId);
		return;
	}
	//副帮主列表，副帮主可有多个
	int nCount = 0;
	inPacket >> nCount;
	CStdItemProvider & stdItemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
 	for (int i = 0; i < nCount; i++)
	{
		ActorCommonNode node;
		inPacket >> node;				//获取从CLogicDBReqestHandler::LoadGuildDetail()发送的节点数据

		if (node.BasicData.nType == smGuildAssistLeader)
		{
			pGuild->m_nSecLeaderCount++;		//副帮主数量
		}
		if (node.BasicData.nType == smGuildTangzhu)
		{
			pGuild->m_GuildTkCount++;			//官员（堂主）数量
		}
		node.hActorHandle = 0;
		//修正翅膀外观
		if (node.BasicData.nSwingId > 0)
		{
			const CStdItem * pStdItem = stdItemProvider.GetStdItem(node.BasicData.nSwingId);
			if (pStdItem != NULL)
			{
				node.BasicData.nSwingId = pStdItem->m_wShape;
			}
		}

		pGuild->m_ActorOffLine.push_back(node);
		if (node.BasicData.nType == smGuildLeader)
		{
			//帮主
			pGuild->m_nLeaderId = node.BasicData.nActorId;
			_asncpytA(pGuild->m_sLeaderName,node.sActorName);
		}
		pGuild->AddToGuildGxList(node.BasicData.nActorId,(int)node.BasicData.nGuildGX);
	}

	inPacket >> nCount;
	if (nCount > 0)
	{
		pGuild->m_WarList.reserve(nCount);
		pGuild->m_WarList.trunc(nCount);
	}
	for (int i = 0; i < nCount; i++)
	{
		WarRelationData& war = pGuild->m_WarList[i];
		//WarRelationData war;
		unsigned int nWarGuildId = 0;
		inPacket >> war.m_WarData.nRelationShip;
		inPacket >> war.m_WarData.nState;
		inPacket >> war.nGuildId;
		inPacket >> war.m_WarData.nPkCount;
		inPacket >> war.m_WarData.nDieCount;
		inPacket >> war.m_WarData.nNext;

		//war.m_pGuild = FindGuild(nWarGuildId);
	}

	inPacket >> nCount;

	pGuild->m_ApplyList.clear();
	for (int i = 0; i < nCount; i++)
	{
		ApplyList apply;

		inPacket >> apply.nActorId;
		inPacket.readString(apply.szName, ArrayCount(apply.szName));
		apply.szName[sizeof(apply.szName)-1] = 0;
		inPacket >> apply.nSex;
		inPacket >> apply.nLevel;
		inPacket >> apply.nCircle;
		inPacket >> apply.nJob;
		pGuild->m_ApplyList.insert( std::make_pair( apply.nActorId, apply));
	}

	
	m_GuidlDetailLoadCount++;
	if (m_GuidlDetailLoadCount == m_pGuildList->count() && !m_DbInitGuild)
	{
		m_DbInitGuild = true;
		LoadGuildSiegeRanking();//加载帮派成员读排行榜
	}

	//OutputMsg(rmTip,_T("Load Guild War Data Success!GuildId=%d,count=%d"),nGuildId,nCount);
}

VOID CGuildComponent::OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size)
{
	int nRawServerId, nLoginServerId;
	unsigned int nGid = 0;
	CDataPacketReader inPacket(data,size);
	BYTE nErrorCode;
	inPacket >> nRawServerId >> nLoginServerId >> nErrorCode;

	switch(nCmd)
	{
	case dcLoadGuildList:
		if (nErrorCode == reSucc)
		{
			LoadGuildListResult(inPacket);
		}else
		{
			OutputMsg(rmError,_T("Guild LoadGuildListResult Error!code=%d"),nErrorCode);
		}
		break;
	case dcLoadActorApplyResult:
	{
		if (nErrorCode == reSucc)
		{
			LoadActorApplyResult(inPacket);
		}else
		{
			OutputMsg(rmError,_T("Guild dcLoadActorApplyResult Error!code=%d"),nErrorCode);
		}
		break;
	}
	case dcLoadGuildDetail:
		if (nErrorCode == reSucc)
		{
			LoadGuildDetailResult(inPacket);
		}
		else
		{
			OutputMsg(rmError,_T("Guild LoadGuildDetailResult Error!code=%d"),nErrorCode);
		}
		break;

	case dcLoadGuildApplyLists:
		if (nErrorCode == reSucc)
		{
			LoadGuildApplyListsResult(inPacket);
		}
		else
		{
			OutputMsg(rmError,_T("Guild LoadGuildApplyLists Error!code=%d"),nErrorCode);
		}
		break;

	case dcAddGuild:
		AddGuildResult(inPacket, nErrorCode);
		break;

	case dcDeleteGuild:		
		int		index;
		Uint64	hHandle;
		inPacket >> nGid;
		inPacket >> index;
		inPacket >> hHandle;

		if (nErrorCode == reSucc)
		{			
			//从列表中删除
			CGuild* pGuild = FindGuild(nGid);

			if(hHandle<=0)
				DeleteGuild(pGuild,1);

			if (pGuild)
			{
				pGuild->m_boStatus = FALSE;			//行会状态设置为：已解散
			}
			//OutputMsg(rmTip,_T("delete guild ok!gid = %d!"),nGid);
		}
		else
		{
			OutputMsg(rmError,_T("delete guild error!gid = %u!"),nGid);
		}

		NotifyActor(index,hHandle,dcDeleteGuild,nErrorCode,nGid);
		break;

	case dcLoadGuildSkill:
		if (nErrorCode == reSucc)
		{
			LoadGuildSkillResult(inPacket);
		}
		else
		{
			OutputMsg(rmError,_T("Guild dcLoadGuildSkill Error!code=%d"),nErrorCode);
		}
		break;
	case dcSaveGuild:
		{
			
			inPacket >> nGid;
			if (nErrorCode == reSucc)
			{						
				//OutputMsg(rmTip,_T("Save guild ok!gid = %d!"),nGid);
			}
			else
			{
				OutputMsg(rmError,_T("Save guild error!gid = %u!"),nGid);
			}
			break;
		}
	case dcLoadGuildEvent:
		{
			if (nErrorCode == reSucc)
			{
				LoadGuildEventResult(inPacket);
			}
			else
			{
				OutputMsg(rmError,_T("Guild dcLoadGuildEvent Error!code=%d"),nErrorCode);
			}
			break;

		}
	case dcLoadGuildDepotItem:
		{
			if (nErrorCode == reSucc)
			{
				LoadGuildDepotResult(inPacket);
			}
			else
			{
				OutputMsg(rmError,_T("Guild dcLoadGuildDepotItem Error!code=%d"),nErrorCode);
			}

			break;
		}
	case dcLoadGuildDepotRecord:
		{
			if (nErrorCode == reSucc)
			{
				LoadDepotRecordResult(inPacket);
			}
			else
			{
				OutputMsg(rmError,_T("Guild dcLoadGuildDepotRecord Error!code=%d"),nErrorCode);
			}
			break;
		}
		
	}	
}


//loadLoadApplyLists
void CGuildComponent::LoadGuildApplyListsResult(CDataPacketReader &inPacket)
{

	unsigned int nGuildId = 0;
	inPacket >> nGuildId;//帮派id
	CGuild* pGuild = FindGuild(nGuildId);

	if (pGuild == NULL)
	{
		OutputMsg(rmError,_T("No Guild in LoadGuildApplyLists(),GuildId=%d"),nGuildId);
		return;
	}

	int nCount = 0;

	inPacket >> nCount;
	if (nCount > 0)
	{
		for (int i = 0; i < nCount; i++)
		{
			ApplyList applyInfo;
			unsigned int nActorId;
			inPacket >> applyInfo.nActorId;
			inPacket.readString(applyInfo.szName,ArrayCount(applyInfo.szName));
			applyInfo.szName[sizeof(applyInfo.szName)-1] = 0;
			int nsex = 0;
			inPacket >> applyInfo.nSex;
			int nLevel = 0;
			inPacket >> applyInfo.nLevel;
			int nCircle = 0;
			inPacket >> applyInfo.nCircle;
			int nJob = 0;
			inPacket >> applyInfo.nJob;
			unsigned int nGuid;
			inPacket >> nGuid;
			unsigned int nVip;
			inPacket >> applyInfo.nVip;
			std::map<unsigned int, ApplyList>::iterator it = pGuild->m_ApplyList.find(nActorId);
			if(it == pGuild->m_ApplyList.end())
			{
				pGuild->m_ApplyList.insert(std::make_pair(applyInfo.nActorId, applyInfo));
			}
			
		}
	}
}

//创建一个新行会成功
void CGuildComponent::AddGuildResult( CDataPacketReader &inPacket, int nErrorCode ) 
{
	CGuild guild;
	int		index;
	Uint64	hHandle;
	inPacket >> index;
	inPacket >> hHandle;
	CGuild* pGuild = NULL;
	if (nErrorCode == reSucc)
	{
		inPacket >> guild.m_nGid;
		inPacket.readString(guild.m_sGuildname,ArrayCount(guild.m_sGuildname));
		guild.m_sGuildname[sizeof(guild.m_sGuildname)-1] = 0;
		// inPacket.readString(guild.m_sFoundname,ArrayCount(guild.m_sFoundname));
		// guild.m_sFoundname[sizeof(guild.m_sFoundname)-1] = 0;
		inPacket.readString(guild.m_sLeaderName,ArrayCount(guild.m_sLeaderName));
		guild.m_sLeaderName[sizeof(guild.m_sLeaderName)-1] = 0;
		guild.m_nZytype = 0;
		guild.m_nIcon = 0;

		int nTime = GetGlobalLogicEngine()->getMiniDateTime();
		int nFlag = GetLogicServer()->GetDataProvider()->GetGuildConfig().nApproval;
		pGuild = AddGuildToList(guild.m_nGid,1,0,0,0,nTime,nFlag,0,NULL,NULL,guild.m_sGuildname,guild.m_sLeaderName);

		//初始化帮派繁荣度
		//pGuild->SetGuildFr(GetLogicServer()->GetDataProvider()->GetGuildConfig().initCreateFr);
		//OutputMsg(rmTip,_T("Add Guild Success!GuildId=%d"),guild.m_nGid);
	}
	else
	{
		OutputMsg(rmError,_T("Add Guild Error!GuildId=%u,code=%d"),guild.m_nGid,nErrorCode);
	}

	NotifyActor(index,hHandle,dcAddGuild,nErrorCode,guild.m_nGid,pGuild);
}

VOID CGuildComponent::NotifyActor(Uint64 nIndex,Uint64	hHandle,int nType,int nErrorCode,unsigned int nGuildId,CGuild* pGuild)
{
	//通知玩家结果
	if ((INT_PTR)nIndex >= 0 && (INT_PTR)nIndex < m_UserOperList.count())
	{
		UserOperData& data = m_UserOperList[nIndex];
		if (data.hActor == hHandle)
		{
			CEntity* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity((EntityHandle&)hHandle);
			if (pEntity != NULL && pEntity->GetType() == enActor)
			{
				if (nType == dcAddGuild)
				{
					((CActor*)pEntity)->GetGuildSystem()->OnCreateGuild(nGuildId,nErrorCode,pGuild);
				}
				else
				{
					((CActor*)pEntity)->GetGuildSystem()->OnDeleteGuild(nErrorCode);
				}				
			}
			data.nTimeOut = 0;
		}
		
	}
}

VOID CGuildComponent::RunOne()
{	
	TICKCOUNT nCurr = GetGlobalLogicEngine()->getTickCount();

	int i = 0;

	if (m_timer.CheckAndSet(nCurr))
	{	
		CLinkedNode<CGuild*> *pNode;
		CLinkedListIterator<CGuild*> it(*m_pGuildList);
		
		unsigned int nCurTime = GetGlobalLogicEngine()->getMiniDateTime();
		for (pNode = it.first(); pNode; pNode = it.next())
		{
			CGuild* pGuild = pNode->m_Data;
			if (pGuild)
			{
				pGuild->CheckWarList();

				if (pGuild->m_boUpdateTime)		//当帮派等级、繁荣度、公告等改变时，nUpdateTime才会改变，然后通知这里保存数据
				{
					//pGuild->Save();//是否有数据需要保存
					pGuild->SaveAllGuildEvent();
				}
				if (pGuild->m_nNewInMemoUpdateTime > 0 && nCurTime >= pGuild->m_nNewInMemoUpdateTime)
				{
					pGuild->SetInMemo(pGuild->GetNextUpdateInmemo(),true);
				}

				//加入行会弹劾 数据保存存在问题，暂时屏蔽
				//pGuild->ImpeachLeader();
			}
		}

		// CGuild* pGuild = FindGuild(m_GuildId);
		// if (pGuild)
		// {
		// 	pGuild->LoadSbkOffLinedata();
		// }
	}
	if(m_SecTimer.CheckAndSet(nCurr))
	{
		// int nNeedCoin = GetLogicServer()->GetDataProvider()->GetGuildConfig().nNoticeGuildCoin;

		CLinkedNode<CGuild*> *pNode;
		CLinkedListIterator<CGuild*> it(*m_pGuildList);

		for (pNode = it.first(); pNode; pNode = it.next())
		{
			CGuild* pGuild = pNode->m_Data;
			if (pGuild)
			{
				// if(pGuild->GetCoin() < nNeedCoin)		//行会资金低于2000进行提示
				// {   
				// 	 //pGuild->SendGuildMsg(smGuildCommon,tpNoticGuildFrMsg,ttScreenCenter + ttChatWindow);
				// }

				pGuild->SaveDepotData();
			}
		}
		SaveGuildSiegeRanking();
	}

	if( m_10MinsTimer.CheckAndSet(nCurr) )			//10分钟定时器
	{
		CLinkedNode<CGuild*> *pNode;
		CLinkedListIterator<CGuild*> it(*m_pGuildList);
		for( pNode = it.first(); pNode; pNode = it.next() )
		{
			CGuild* pGuild = pNode->m_Data;
			if (pGuild)
			{
				pGuild->ProcDepotItemInExchangeCheck();
			}
		}
		SaveGuildApplyReslut();
	}

	if( m_1sTimer.CheckAndSet(nCurr) )			//1秒定时器
	{
		GUILDDATA &stGuildClose = GetLogicServer()->GetDataProvider()->GetGuildConfig().GetGuildCloseConfig();
		if( stGuildClose.nIsOpen )
		{		
			//更新行会关闭时间、重置是否关闭行会
			if ( stGuildClose.nEndTime != 0 && GetGlobalLogicEngine()->getMiniDateTime() > stGuildClose.nEndTime )
			{
				m_bGuildClose = false;

				// 循环活动的nTimeIdx永远不等于nTimeCount，这个nTimeIdx已在UpdateOpenTime中维护了。
				if (stGuildClose.nTimeIdx != stGuildClose.nTimeCount)
				{
					bool result = GetLogicServer()->GetDataProvider()->GetGuildConfig().UpdateTime(GetGlobalLogicEngine()->GetGlobalNpcLua(),stGuildClose,false);
					if (result)
					{
						stGuildClose.nStartTime = stGuildClose.nStartTime;
						stGuildClose.nEndTime = stGuildClose.nEndTime;
					}          
				}
			}

			//关闭行会
			if ( GetGlobalLogicEngine()->getMiniDateTime() >= stGuildClose.nStartTime && GetGlobalLogicEngine()->getMiniDateTime() <= stGuildClose.nEndTime && !m_bGuildClose )
			{
				m_bGuildClose = true;
			}
		}


		GUILDDATA &stGuildClear = GetLogicServer()->GetDataProvider()->GetGuildConfig().GetGuildClearConfig();
		if ( stGuildClear.nIsOpen ) 
		{
			//更新行会清理时间、重置是否清理行会
			if ( stGuildClear.nEndTime != 0 && GetGlobalLogicEngine()->getMiniDateTime() > stGuildClear.nEndTime )
			{
				m_bGuildClear = false;

				// 循环活动的nTimeIdx永远不等于nTimeCount，这个nTimeIdx已在UpdateOpenTime中维护了。
				if (stGuildClear.nTimeIdx != stGuildClear.nTimeCount)
				{
					bool result = GetLogicServer()->GetDataProvider()->GetGuildConfig().UpdateTime(GetGlobalLogicEngine()->GetGlobalNpcLua(),stGuildClear,false);
					if (result)
					{
						stGuildClear.nStartTime = stGuildClear.nStartTime;
						stGuildClear.nEndTime = stGuildClear.nEndTime;
					}          
				}
			}

			//清理行会
			if ( GetGlobalLogicEngine()->getMiniDateTime() >= stGuildClear.nStartTime && GetGlobalLogicEngine()->getMiniDateTime() <= stGuildClear.nEndTime && !m_bGuildClear )
			{
				ClearAllGuild();
				m_bGuildClear = true;
			}
		}
	}
}


CGuildComponent::UserOperData* CGuildComponent::GetFreeUserOperData(INT_PTR& nIndex)
{
	int ntime = GetGlobalLogicEngine()->getMiniDateTime();
	for (int i = 0; i < m_UserOperList.count(); i++)
	{
		UserOperData& data = m_UserOperList[i];

		if (data.nTimeOut == 0)
		{
			nIndex = i;
			return &data;
		}
		if (data.nTimeOut < (unsigned int)ntime)
		{
			NotifyActor(i,data.hActor,data.nType,reTimeOut,0);//提示处理客户超时
			nIndex = i;
			return &data;
		}
	}
	UserOperData newData;
	m_UserOperList.add(newData);
	nIndex = m_UserOperList.count()-1;
	UserOperData& data = m_UserOperList[nIndex];		
	return &data;
}



VOID CGuildComponent::test()
{
	CGuildComponent&	GuildMgr = GetGlobalLogicEngine()->GetGuildMgr();
	//GuildMgr.AddGuild(1111111,"myokbug","zanc",0);

	//GuildMgr.DeleteGuild(111111222,6);
	GuildMgr.LoadGuildDetail(7);
}

VOID CGuildComponent::InitGuildSKill(CGuild* pGuild)
{
	return;
}

void CGuildComponent::StartGuildSiege()
{
	if(GetLogicServer()->IsCommonServer()) return;
	m_GuildSiegeStartFlag = 1;
}

void CGuildComponent::SetCityOwnGuildId(unsigned int nGuildId)
{
	int nOldGuild = m_GuildId;
	m_GuildId = nGuildId;
	CGuild* pGuild = FindGuild(m_GuildId);
	if (pGuild)
	{
		// SetCityPos(stRealMaster,pGuild->m_pLeader->BasicData.nActorId);
		// //设置首领相关
		// if(pGuild->m_pLeader)
		// {
		// 	CEntity* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(pGuild->m_pLeader->BasicData.nActorId);
		// 	if (pEntity && pEntity->GetType() == enActor)
		// 	{
		// 		//((CActor*)pEntity)->SetSocialMask(smIsSbkCityMaster,true);
		// 		CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
		// 		if (pNpc != NULL) 
		// 		{
		// 			CScriptValueList paramList, retParamList;
		// 			paramList << pEntity;
		// 			pNpc->GetScript().Call("GetCityPosWelfareInfo",paramList,retParamList);
		// 		}
		// 	}
		// }
		// SetGuildSiegeMasterStatue();
		pGuild->UpdateActorShowName();
		SaveGuildSiegeRanking();
		pGuild->UpdateSbkGuidAchieve();
		pGuild->UpdateSbkGuildTitle(true);
	}
	//取消原来的沙巴克
	if(nOldGuild)
	{
		CGuild* pGuild = FindGuild(nOldGuild);
		if (pGuild)
		{
			pGuild->UpdateActorShowName();
		}
	}
}

void CGuildComponent::GuildSiegeEnd(unsigned int nGuildId)
{
	// if(GetLogicServer()->IsCommonServer()) return;
	// if (!IsGuildSiegeStart())
	// {
	// 	return;
	// }
	// m_GuildSiegeStartFlag = false;

	
	// if (nGuildId > 0)
	// {
	// 	SetCityOwnGuildId(nGuildId);
	// }
	

	// CGuildProvider& guildConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();

	// //在沙皇宫的踢到外面
	// CFuBen* pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
	// if (pFb)
	// {
	// 	CScene* pScene = pFb->GetScene(guildConfig.nPalaceSceneId);
	// 	if (pScene)
	// 	{
	// 		CEntityList& entityList = pScene->GetPlayList();
	// 		CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	// 		CLinkedNode<EntityHandle> *pNode;
	// 		CLinkedListIterator<EntityHandle> it(entityList);
	// 		for (pNode = it.first(); pNode; pNode = it.next())
	// 		{
	// 			CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
	// 			if (pEntity && pEntity->GetType() == enActor)
	// 			{
	// 				CActor* pActor = (CActor*)pEntity;
	// 				if (pActor->GetGuildSystem()->GetGuildId() != m_GuildId)
	// 				{
	// 					ReturnSiegeScene& toScene = GetLogicServer()->GetDataProvider()->GetGuildConfig().returnSiegeScene;

	// 					CScene *pReturnScene = pFb->GetScene(toScene.nSceneId);
	// 					if (pReturnScene)
	// 					{
	// 						pReturnScene->EnterSceneRandPt(pActor, toScene.nPosX, toScene.nPosY, toScene.nRadius, toScene.nRadius);
	// 					}

	// 				}
	// 			}		
	// 		}
	// 	}
	// }

	// CScriptValueList paramList, retParamList;
	// CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
	// if (pNpc == NULL) return;
	// if (!pNpc->GetScript().Call("OnGuildSiegeEnd", paramList, retParamList))
	// {
	// 	const RefString &s = pNpc->GetScript().getLastErrorDesc();
	// 	OutputMsg(rmError, (LPCSTR)s);
	// }

	// SaveGuildSiegeRanking();//结束保存一次
	
}

bool CGuildComponent::IsGuildSiegeTodayOpen()
{
	// int nDays = GetLogicServer()->GetDaysSinceOpenServer();
	// int nCombineDays = GetLogicServer()->GetDaysSinceCombineServer();
	// CGuildProvider & guildPrv = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	// int nOpenServerLastDay =  GetLogicServer()->GetDataProvider()->GetOpenServerConfig().nOpenServerDays;
	// SYSTEMTIME &sysTime = GetLogicServer()->GetLogicEngine()->getSysTime();
	// if((nDays > 0 && nDays < guildPrv.nOpenSvrSingDays)) //开服前2天不攻城
	// {
	// 	return false;
	// }
	// else if (nCombineDays > 0 && nCombineDays < guildPrv.nOpenSvrSingDays) //合服前2天不攻城
	// {
	// 	return false;
	// }
	// else if ((nDays >= guildPrv.nOpenSvrSingDays && nDays <= nOpenServerLastDay) || (nCombineDays >= guildPrv.nOpenSvrSingDays  && nCombineDays <= nOpenServerLastDay )) //开服或者合服第三天
	// {
	// 	return true;
	// }
	// else
	// {
	// 	for (INT_PTR i= 0; i < guildPrv.vSiegeDayList.count(); i++)
	// 	{
	// 		BYTE nSiegeDay = guildPrv.vSiegeDayList[i];
	// 		if (nSiegeDay == sysTime.wDayOfWeek)
	// 		{
	// 			return true;
	// 		}
	// 	}
	// }
	return false;
}
void CGuildComponent::AutoSignGuildSiege()
{
	// if(!IsGuildSiegeTodayOpen())
	// {
	// 	return;
	// }
	// m_boTodaySign = true;
	// CGuildProvider & guildPrv = GetLogicServer()->GetDataProvider()->GetGuildConfig();	
	// CLinkedNode<CGuild*> *pNode;
	// CLinkedListIterator<CGuild*> it(*m_pGuildList);
	// for (pNode = it.first(); pNode; pNode = it.next())
	// {
	// 	CGuild* pGuild = pNode->m_Data;
	// 	if (pGuild->m_MainHallLev >= guildPrv.nAutoSignMainHallLevel && pGuild->m_boStatus)
	// 	{
	// 		pGuild->m_SignUpFlag = 1;
	// 		pGuild->m_boUpdateTime = true;
	// 	}
	// }
	// GetGlobalLogicEngine()->GetEntityMgr()->BroadcastTipmsgWithID(tpGuildSiegeAutoSignMsg,ttChatWindow + ttScreenCenter);
}
void CGuildComponent::ResetSignGuildsiege()
{
	// if(!IsGuildSiegeTodayOpen())
	// {
	// 	return;
	// }
	// m_boTodaySign = false;
	// CGuildProvider & guildPrv = GetLogicServer()->GetDataProvider()->GetGuildConfig();	
	// CLinkedNode<CGuild*> *pNode;
	// CLinkedListIterator<CGuild*> it(*m_pGuildList);
	// for (pNode = it.first(); pNode; pNode = it.next())
	// {
	// 	CGuild* pGuild = pNode->m_Data;
	// 	if (pGuild->m_MainHallLev >= guildPrv.nAutoSignMainHallLevel && pGuild->m_boStatus)
	// 	{
	// 		pGuild->m_SignUpFlag = 0;
	// 		pGuild->m_boUpdateTime = true;
	// 	}

	// 	pGuild->SetGuildSiegeCommand(NULL, 0, 0);		//清除旧的行会指令
	// }
	// SaveGuildSiegeRanking();
}

void CGuildComponent::ClearGuildSiegeCommand()
{
	CLinkedNode<CGuild*> *pNode;
	CLinkedListIterator<CGuild*> it(*m_pGuildList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CGuild* pGuild = pNode->m_Data;
		if( pGuild )
		{
			pGuild->SetGuildSiegeCommand(NULL, 0, 0);		//清除旧的行会指令
		}
	}
}

void CGuildComponent::ClearGuildSiegeData()
{
	// CGuild* pGuild = FindGuild(m_GuildId);
	// if(pGuild)//m_GuildId!=0;
	// {
	// 	CActor* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(m_PostionId[stRealMaster]);
	// 	if (pEntity && pEntity->GetType() == enActor)
	// 	{
	// 		pEntity->GetGuildSystem()->TakeOnOffDragonRobe(0);
	// 		//pEntity->SetSocialMask(smIsSbkCityMaster, false);
	// 	}
	// }
	// memset(m_PostionId, 0, sizeof(m_PostionId));
	// m_GuildId = 0;

	// if (pGuild)//m_GuildId =0;
	// {
	// 	pGuild->UpdateActorShowName();
	// }

	// SetGuildSiegeMasterStatue();//重置雕像
	// SaveGuildSiegeRanking();
	// if (pGuild)
	// {
	// 	pGuild->UpdateSbkGuildTitle(true);	//回收沙城时回收头衔
	// }
}
void CGuildComponent::SetCityPos(int nPos, unsigned int nActorId)
{
	if (nPos <= stNoCityPos && nPos >= stMaxGuildCityPos)
	{
		return ;
	}
	unsigned int nOldPosActorId = m_PostionId[nPos];
	m_PostionId[nPos] = nActorId;
	CEntityManager* em	= GetGlobalLogicEngine()->GetEntityMgr();
	if (nActorId > 0)
	{
		CActor* pActor = em->GetEntityPtrByActorID(nActorId);
		if (pActor)
		{
			pActor->ResetShowName();//更新皇城职位称号
		}
	}
	if (nOldPosActorId > 0)
	{
		CActor* pActor = em->GetEntityPtrByActorID(nOldPosActorId);
		if (pActor)
		{
			pActor->ResetShowName();//更新皇城职位称号
		}
	}
	SaveGuildSiegeRanking();
}
int CGuildComponent::getCityPostionById(unsigned int nActorId)
{
	if (nActorId == 0)
	{
		return stNoCityPos;
	}
	for (int i= stNoCityPos; i < ArrayCount(m_PostionId); i++)
	{
		if (m_PostionId[i] == nActorId)
		{
			return i;
		}
	}
	return stNoCityPos;
}
const char* CGuildComponent::GetCityPosName(unsigned int nActorId)
{
	int nPos = getCityPostionById(nActorId);
	if (nPos != stNoCityPos)
	{
		return GetLogicServer()->GetDataProvider()->GetGuildConfig().GetGuildCityPosName(nPos);
	}
	return NULL;
}
char* CGuildComponent::GetCityPostionInfo(int nPos, int &nJob,int &nSex)
{
	if(m_GuildId <= 0) return NULL;

	CGuild* pGuild= FindGuild(m_GuildId);

	if(pGuild == NULL) return NULL;
	if(nPos >= stRealMaster && nPos < stMaxGuildCityPos)
	{
		unsigned int nActorId = m_PostionId[nPos];
		if (nActorId > 0)
		{
			ActorCommonNode* pNode = pGuild->FindMember(nActorId);
			if (pNode != NULL)
			{
				nJob = pNode->BasicData.nJob;
				nSex = pNode->BasicData.nSex;	
				return pNode->sActorName;
			}
		}
	}

	return NULL;
}
void CGuildComponent::SetGuildSiegeMasterStatue()
{
	GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction("SetGuildSiegeMasterStatueModel");
}


//获取沙巴克城主的id
//注意：沙巴克城主，一定是占领帮派的帮主吧,如果帮主换了，那城主就是新帮主吧
unsigned int CGuildComponent::GetCityOwnMasterId()
{
	if(m_GuildId <= 0)
		return 0;

	CGuild* pGuild= FindGuild(m_GuildId);

	if(pGuild)
	{
		if(pGuild->m_nLeaderId)
			return pGuild->m_nLeaderId;
		else
			return 0;
	}
	else
	{
		return 0;
	}
}





void CGuildComponent::UpdateGuildMemberName(CActor* pActor)
{
	if(!pActor) return;

	if(IsGuildSiegeStart() && pActor->GetSceneAreaFlag() == scfGuildSiege)
		UpdateNameClr_OnPkModeChanged(pActor->GetHandle());
}



//将行会神树从成熟期回归到幼苗期
void CGuildComponent::ProcGuildTreeToSapling()
{
	CLinkedNode<CGuild*> *pNode;
	CLinkedListIterator<CGuild*> it(*m_pGuildList);

	int nNow = GetGlobalLogicEngine()->getMiniDateTime();
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CGuild* pGuild = pNode->m_Data;
		if (pGuild)
		{
			pGuild->ProcGuildTreeToSapling(nNow);
		}
	}
}

//将行会任务刷新
void CGuildComponent::ProcDailyData()
{
	CLinkedNode<CGuild*> *pNode;
	CLinkedListIterator<CGuild*> it(*m_pGuildList);

	int nNow = GetGlobalLogicEngine()->getMiniDateTime();
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CGuild* pGuild = pNode->m_Data;
		if (pGuild)
		{
			pGuild->ProcDailyData();
		}
	}
}
//下发行会战指挥面板到全体参战的成员
void CGuildComponent::SendGuildSiegeCmdPanel(unsigned char nFlag)
{
	CLinkedNode<CGuild*> *pNode;
	CLinkedListIterator<CGuild*> it(*m_pGuildList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CGuild* pGuild = pNode->m_Data;
		if (pGuild->m_SignUpFlag == 1)
		{
			pGuild->NotifyGuildSiegeCmdPanel(nFlag);
		}
	}
}

bool CGuildComponent::CanPassPalaceTeleport(CActor* pActor)
{
	bool result = false;
	if(!pActor)
	{
		return result;
	}
	CGuild* pGuild = pActor->GetGuildSystem()->GetGuildPtr();
	if (pGuild)
	{
		if(pGuild->GetGuildId() == m_GuildId)
			result = true;
	}
	// if(!pActor)
	// {
	// 	return false;
	// }
	// if (IsGuildSiegeStart())
	// {
	// 	CGuild* pGuild = pActor->GetGuildSystem()->GetGuildPtr();
	// 	if (pGuild)
	// 	{
	// 		result = pGuild->m_SignUpFlag != 0;
	// 	}
	// 	if (!result)
	// 	{
	// 		ReturnSiegeScene& toScene = GetLogicServer()->GetDataProvider()->GetGuildConfig().returnSiegeScene;
	// 		CFuBen * pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
	// 		if (pFb != NULL)
	// 		{
	// 			CScene *pScene = pFb->GetScene(toScene.nSceneId);
	// 			if (pScene)
	// 			{
	// 				pScene->EnterSceneRandPt(pActor, toScene.nPosX, toScene.nPosY, toScene.nRadius, toScene.nRadius);
	// 				pActor->SendOldTipmsgWithId(tpGuildSiegeNoSignTeleport, ttFlyTip);
	// 			}
				
	// 		}
			
	// 	}
	// }
	return result;
}

/*
CJoinGuildApply* CGuildComponent::GetJoinGuildApply( unsigned int nActor )
{
	return NULL;
}
*/

int CGuildComponent::AddJoinGuildApply( CActor *pActor, unsigned int nGuildId )
{
	if( !pActor )
	{
		return enApplyJoin_ActorErr;
	}

	unsigned int nActorGuildId = pActor->GetProperty<unsigned int>(PROP_ACTOR_GUILD_ID);
	if( nActorGuildId > 0 )
	{
		return enApplyJoin_HasGuild;
	}

	unsigned int nActorId = pActor->GetId();
	CJoinGuildApply* pJoinApply = GetJoinGuildApply( nActorId );
	if( pJoinApply )
	{
		CGuild* pGuild = FindGuild( nGuildId );
		if( !pGuild )
		{
			return enApplyJoin_NoGuild;
		}

		if( pJoinApply->nGuildId > 0 )
		{
			return enApplyJoin_HasAgreed;
		}

		for( INT_PTR i=0 ;i< pJoinApply->listObjGuild.size(); i++ )
		{
			unsigned int nGuildIdApplied = pJoinApply->listObjGuild[i];
			if( nGuildIdApplied ==  nGuildId )		//已经申请了
			{
				return	enApplyJoin_HasSame;
			}
		}

		int nMemberCount	= (int)pGuild->GetMemberCount();
		int nMemberCountMax = (int)pGuild->GetMaxMemberCount();
		if( nMemberCount >= nMemberCountMax )
		{
			return enApplyJoin_MemberLimit;
		}
		pJoinApply->listObjGuild.push_back( nGuildId );
		//OutputMsg( rmWaning,_T("AddJoinGuildApply, listObjGuild.count =%d"),pJoinApply->listObjGuild.count() );
	}
	else
	{
		CJoinGuildApply *pNewJoinApply = new CJoinGuildApply();
		if( !pNewJoinApply )
		{
			return enApplyJoin_UnKnown;
		}
		pNewJoinApply->nActorId	= nActorId;
		pNewJoinApply->nGuildId	= 0;
		pNewJoinApply->nSex		= (char)pActor->GetProperty<int>(PROP_ACTOR_SEX);
		pNewJoinApply->nLevel		= (int)pActor->GetProperty<int>(PROP_CREATURE_LEVEL);
		pNewJoinApply->nMenPai		= (char)pActor->GetMenPai();
		pNewJoinApply->nJob		= (char)pActor->GetProperty<int>(PROP_ACTOR_VOCATION);
		pNewJoinApply->nCombatPower= (unsigned int)pActor->GetActorAttackValueMax();
		pNewJoinApply->nVipGrade	= (unsigned int)pActor->GetProperty< unsigned int>(PROP_ACTOR_SUPPER_PLAY_LVL);;
		pNewJoinApply->nLogintime	= (unsigned char)pActor->GetLoginTime();
		_asncpytA(pNewJoinApply->szActorName, pActor->GetEntityName());
		pNewJoinApply->listObjGuild.push_back( nGuildId );
		m_JoinGuildApplyMap.insert( std::make_pair(nActorId, pNewJoinApply) );

		//OutputMsg( rmWaning,_T("AddJoinGuildApply, listObjGuild.count =%d"),pNewJoinApply->listObjGuild.count() );
	}

	OutputMsg( rmWaning,_T("AddJoinGuildApply,m_JoinGuildApplyMap.count =%d"),m_JoinGuildApplyMap.size() );
	return enApplyJoin_NoErr;
}

/*
审核玩家加入某个行会的申请
nResult：0；拒绝，>0:同意
*/
int CGuildComponent::CheckJoinGuildApply( unsigned int nActorId, unsigned int nGuildId, char nResult )
{
	CGuild* pGuild = FindGuild( nGuildId );
	if( !pGuild )
	{
		return enCheckJoin_NoGuild;
	}

	int nMemberCount	= (int)pGuild->GetMemberCount();
	int nMemberCountMax = (int)pGuild->GetMaxMemberCount();
	if( nResult > 0 && nMemberCount >= nMemberCountMax )
	{
		return enCheckJoin_MemberLimit;
	}

	CJoinGuildApply* pJoinApply = GetJoinGuildApply( nActorId );
	if( pJoinApply )
	{
		if( nResult > 0 && pJoinApply->nGuildId > 0 )			//已经有行会抢先批准了
		{
			return enCheckJoin_HasGuild;
		}
		for( INT_PTR i=pJoinApply->listObjGuild.size()-1 ;i > -1; i-- )
		{
			unsigned int nGuildIdApplied = pJoinApply->listObjGuild[i];
			if( nGuildIdApplied ==  nGuildId )		//有申请此行会的记录
			{
				if( nResult > 0 )		//同意加入行会
				{
					pJoinApply->nGuildId = nGuildId;
					CActor *pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID( nActorId );
					if( pActor )		//申请人在线
					{
						unsigned int nActorGuildId = pActor->GetProperty<unsigned int>(PROP_ACTOR_GUILD_ID);
						if( nActorGuildId > 0 )			//已经加入某行会了
						{
							return enCheckJoin_HasGuild;
						}
						pGuild->AddMemberToList( pActor );						//玩家正式加入行会
						pJoinApply->DeleteGuildApply(nGuildId);

					}//如果申请人不在线，则节点保留，其登录时加入行会
					else
					{
						pGuild->AddMemberToList( pActor );		
					}
					
				}
				else		//拒绝
				{
					pJoinApply->DeleteGuildApply(nGuildId);						//拒绝，则删除该条guildId
					//OutputMsg( rmWaning,_T("CheckJoinGuildApply, listObjGuild.count =%d"),pJoinApply->listObjGuild.count() );
				}
				if( pJoinApply->listObjGuild.size() == 0 )
				{
					RemoveActorJoinGuildApply( nActorId );				//申请数量为0，删除该玩家的节点
				}

				//OutputMsg( rmWaning,_T("CheckJoinGuildApply,m_JoinGuildApplyMap.count =%d"),m_JoinGuildApplyMap.size() );
				return enCheckJoin_NoErr;
			}
		}
	}
	return enCheckJoin_NoApply;
}

/*
玩家通过申请加入行会
*/
CGuild*  CGuildComponent::ActorJoinGuildByApply( CActor* pActor )
{
	if(!pActor) return NULL;
	unsigned int nActorGuildId = pActor->GetProperty<unsigned int>(PROP_ACTOR_GUILD_ID);
	if( nActorGuildId > 0 )			//已经加入某行会了
	{
		return NULL;
	}

	unsigned int nActorId = pActor->GetId();
	unsigned int nGid = CheckIsJoinGuild( nActorId );
	if( nGid > 0 )
{
		CGuild* pGuild = FindGuild(nGid);
		if( pGuild && pGuild->m_boStatus )			//行会存在，并且没有被解散（注意被解散的行会的实例还是存在的）
		{
			pGuild->SetMemberOnline( pActor );					//玩家正式加入行会(还有一些判断)
			return pActor->GetGuildSystem()->GetGuildPtr();
		}
	}
	return NULL;
}

//某行会是否有申请入会的消息
bool CGuildComponent::HasJoinGuildApply( unsigned int nGuildId )
{
	if( nGuildId == 0 )
	{
		return false;
	}
	for( JoinGuildApplyMap::iterator iter = m_JoinGuildApplyMap.begin(); iter != m_JoinGuildApplyMap.end(); iter++ )
	{
		CJoinGuildApply* pJoinApply = ( CJoinGuildApply* )iter->second;
		if( pJoinApply && pJoinApply->nGuildId == 0 )		//尚未被某个行会同意
		{
			for( INT_PTR i=0; i < pJoinApply->listObjGuild.size(); i++ )
			{
				unsigned int nGuildIdApplied = pJoinApply->listObjGuild[i];
				if( nGuildId == nGuildIdApplied )
				{
					return true;
				}
			}
		}
	}
	return false;
}

const char* CGuildComponent::getSbkGuildName()
{
	CGuild* pGuild = FindGuild(m_GuildId);
	if(pGuild)
		return pGuild->GetGuildName();
	return NULL;
}

const char* CGuildComponent::getSbkGuildLeaderName()
{
	CGuild* pGuild = FindGuild(m_GuildId);
	if(pGuild)
		return pGuild->m_sLeaderName;
	return NULL;
}

void CGuildComponent::getsbkOfflineData(CDataPacket& data )
{
	int pos = data.getPosition();
	int nCount = 0;
	data << (BYTE)nCount;
	CGuild* pGuild = FindGuild(m_GuildId);
	if(pGuild)
	{
		pGuild->getsbkOfflineData(data, nCount);
	}
	byte* pByte = (BYTE*)data.getPositionPtr(pos);
	*pByte = (BYTE)nCount;

	return;
}


void CGuildComponent::SendSbkGuildRankAward(int nGuildId, int nRankId )
{
	CGuild* pGuild = FindGuild(nGuildId);
	if(pGuild)
	{
		pGuild->SendSbkGuildRankAward(nRankId);
	}
	return;
}


void CGuildComponent::SendSbkGuildAward()
{
	CGuild* pGuild = FindGuild(m_GuildId);
	if(pGuild)
	{
		pGuild->SendSbkGuildAward();
	}
	return;
}

bool CGuildComponent::ClearAllGuild()
{
	CLinkedNode<CGuild*> *pNode;
	CLinkedListIterator<CGuild*> it(*m_pRank);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CGuild* pGuild = pNode->m_Data;
		if (pGuild && pGuild->m_boStatus) 
		{
			DeleteGuild(pGuild, 1);
			
			SendDbMsgDeleteGuild(0, pGuild->m_nGid);
		}
	}
}