#include "StdAfx.h"
#include "../base/Container.hpp"
using namespace jxInterSrvComm::DbServerProto;	//数据服务器和逻辑服务器的命令码

CTopTitleMgr::CTopTitleMgr()
{
	m_nDelay = 0;
	m_nRefCount = 0;
	m_nUpdateTime = 0;
}

CTopTitleMgr::~CTopTitleMgr()
{
	for (int i=0; i < MAX_RANK_COUNT;i++ )
	{
		RANKUSERLIST& list = m_rankActorList[i];
		list.empty();
	}
	m_RawRankList.empty();
	m_CsRankList.empty();
}


bool CTopTitleMgr::Initialize()
{
	//LoadCsRank();
	return true;
}

void CTopTitleMgr::RunOne(TICKCOUNT nCurrTickCount)
{
	// int nNowTime = time(NULL);
	// if (nNowTime >= m_nUpdateTime)
	// {
	// 	//UpdateRawRankList();
	// 	//UpdateVocNpcStatue(stRsVocation);
	// 	UpdateBaseRankData();
	// 	Load();
	// 	m_nUpdateTime = nNowTime + 60*10;
	// }
}

// void CTopTitleMgr::Load()
// {
//  	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
// 	if (pDbClient->connected())
// 	{
// 		CDataPacket& DataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadBaseRank);
// 		int nServerId = GetLogicServer()->GetServerIndex(); 
// 		DataPacket << nServerId << nServerId;
// 		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
// 	}
// }


// void CTopTitleMgr::UpdateBaseRankData()
// {
// 	// 增加头衔排行榜引用计数
// 	AddTopRankRef();

// 	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
// 	if (pDbClient->connected())
// 	{
// 		CDataPacket &DataPacket =pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveBaseRank);
// 		int nServerId = GetLogicServer()->GetServerIndex();
// 		DataPacket << nServerId << nServerId;
// 		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
// 	}
// }

void CTopTitleMgr::maintainTopTitle()
{
	// const int FIRST_TOP = 1;	//排行榜第一名
	// int nRankIndex = 0;
	// int nTitle =0;
	// int nJobTipIndex = 0;
	// for (int i =0; i< MAX_TOP_TITLE_COUNT; i++)
	// {
	// 	if (i ==0)
	// 	{
	// 		nRankIndex = rtMagicianLevel;//第一法师
	// 		nTitle = MAX_TOP_TITLE_MASTER;
	// 		nJobTipIndex = tpJobNameMagician;
	// 	}
	// 	else if (i == 1)
	// 	{
	// 		nRankIndex = rtWarriorLevel;//第一战士
	// 		nTitle = MAX_TOP_TITLE_WARRIOR;
	// 		nJobTipIndex = tpJobNameWarrior;
	// 	}
	// 	else if (i == 2)
	// 	{
	// 		nRankIndex = rtWizardLevel;//第一道士
	// 		nTitle = MAX_TOP_TITLE_WIZARD;
	// 		nJobTipIndex = tpJobNameWizard;
	// 	}

	// 	unsigned int nOldActorId = (unsigned int)GetGlobalLogicEngine()->GetMiscMgr().GetSystemStaticVarValue(szTopTitleRand[i],0);
	// 	PTOPTITLERANKINFO pRankInfo = GetActorInfoByCsRank(nRankIndex,FIRST_TOP);
	// 	unsigned int nActorId = pRankInfo != NULL ? pRankInfo->m_nActorId:0;
		
	// 	GetGlobalLogicEngine()->GetMiscMgr().SetSystemStaticVarValue(szTopTitleRand[i],nActorId);
		
	// 	if (nOldActorId > 0 && nActorId != nOldActorId)
	// 	{
	// 		CActor *oldActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nOldActorId);
	// 		if(oldActor != NULL)
	// 		{
	// 			oldActor->GetNewTitleSystem().delNewTitle(nTitle);
	// 		}
	// 	}
	// 	if (nActorId > 0 && nActorId != nOldActorId)
	// 	{
	// 		CActor *pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);
	// 		if(pActor != NULL)
	// 		{
	// 			// pActor->GetNewTitleSystem().addNewTitle(nTitle);
	// 			pActor->GetNewTitleSystem().SetCurNewTitle(nTitle);
	// 		}
	// 		char msg[256];
	// 		LPCTSTR pFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpMaintainTopTitle);
	// 		LPCTSTR pJogName = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(nJobTipIndex);
	// 		sprintf(msg, pFormat, pRankInfo->m_sName, pJogName);
	// 		GetGlobalLogicEngine()->GetEntityMgr()->BroadcastTipmsg(msg,ttScreenCenter + ttChatWindow);
	// 	}
		
		
	// }
	
}

//获取排行榜的第几名的角色id
unsigned int CTopTitleMgr::GetActorIdByCsRank(INT_PTR nRankIndex,int nRank)
{
	PTOPTITLERANKINFO pRankInfo = GetActorInfoByCsRank(nRankIndex,nRank);
	if (pRankInfo)
	{
		return pRankInfo->m_nActorId;
	}
	return 0;
}
PTOPTITLERANKINFO CTopTitleMgr::GetActorInfoByCsRank(INT_PTR nRankIndex,int nRank)
{
	if(MAX_RANK_COUNT <= nRankIndex)
		return NULL;
	RANKUSERLIST& list = m_rankActorList[nRankIndex];
	if(list.count() < nRank)
	{
		return NULL;
	}
	for(INT_PTR j=0; j< list.count();j ++ )
	{
		TOPTITLERANKINFO &data = list[j];
		if(data.m_nRank == nRank)
		{
			return &data;
		}
	}
	return NULL;
}
void CTopTitleMgr::OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size)
{
	// if (nCmd == jxInterSrvComm::DbServerProto::dcSaveBaseRank)
	// {
	// 	CDataPacketReader inPacket(data, size);
	// 	int nRawServerId = 0, nLoginServerId = 0;
	// 	BYTE nErrorCode;
	// 	inPacket >> nRawServerId >> nLoginServerId >> nErrorCode;
	// 	if (nErrorCode != jxInterSrvComm::DbServerProto::reSucc)
	// 	{
	// 		OutputMsg(rmError,_T("Save BaseRank Error!code=%d"),nErrorCode);
	// 		return;
	// 	}
	// 	// else
	// 	// {
	// 	// 	// 减少更新排行榜引用计数
	// 	// 	ReleaseTopRankRef();
	// 	// }
	// }

	// // 读取排行榜入内存
	// if (nCmd == dcLoadBaseRank)
	// {
	// 	CDataPacketReader inPacket(data, size);
	// 	int nRawServerId = 0, nLoginServerId = 0;
	// 	BYTE nErrorCode;
	// 	inPacket >> nRawServerId >> nLoginServerId >> nErrorCode;
	// 	if (nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
	// 	{
	// 		int nCount = 0;
	// 		inPacket >> nCount;
	// 		//初始化
	// 		for (INT_PTR i = 0; i < MAX_RANK_COUNT; i++)
	// 		{
	// 			RANKUSERLIST& list = m_rankActorList[i];
	// 			list.clear();
	// 		}
	// 		for (INT_PTR i = 0; i < nCount; i ++)
	// 		{
	// 			TOPTITLERANKINFO info;
	// 			inPacket >> info.m_nActorId >> info.m_nRankId >> info.m_nRank >> info.m_nValue>> info.m_nParam;
	// 			inPacket.readString(info.m_sName,ArrayCount(info.m_sName));
	// 			INT_PTR nRankIndex = (INT_PTR)(info.m_nRankId);
	// 			if ((nRankIndex < MAX_RANK_COUNT) && (nRankIndex >= 0) )
	// 			{
	// 				RANKUSERLIST& list = m_rankActorList[nRankIndex];
	// 				list.add(info);
	// 			}

	// 		}
	// 		m_bInit = true;

	// 		//更新在线玩家头衔
	// 		//SetOnlineActorHeadTitle();

	// 		//ResetOfflineUser();

	// 		//标记更新雕像嘀嗒
	// 		TICKCOUNT nCurrTime = GetGlobalLogicEngine()->getTickCount();
	// 		//m_nDelay = nCurrTime + 30000;
	// 		m_nDelay = nCurrTime + 60*10;

	// 		int nMaxTopLevel = GetGlobalLogicEngine()->GetMiscMgr().GetSystemStaticVarValue(szTopMaxLevel);
	// 		int nToplevel = GetTopLevel();
	// 		if(nToplevel > nMaxTopLevel)
	// 			SetTopMaxLevel(nToplevel);
	// 	}
	// 	else
	// 	{
	// 		OutputMsg(rmError,_T("Load TopTitleRank Error!code=%d"),nErrorCode);
	// 		return;
	// 	}
	// }

}



//依据type 发送rank
// void CTopTitleMgr::SendTopRankData(CActor * m_pEntity)
// {
	
// 	// if(type > MAX_RANK_COUNT)
// 	// 	return;
// 	if(!m_bInit)
// 	{
// 		UpdateBaseRankData();
// 		Load();
// 		return;
// 	}
// 	int nDays = GetLogicServer()->GetDaysSinceOpenServer();
// 	if(nDays > 1)
// 	{
// 		if(m_pEntity == NULL) return;
// 		CActorPacket AP;
// 		CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);
// 		//CVector<CRanking*>& List = GetGlobalLogicEngine()->GetRankingMgr().GetList();
// 		DataPacket << (BYTE)enMiscSystemID << (BYTE)sRankData;
// 		DataPacket << (BYTE)MAX_RANK_COUNT;
// 		// int rankNum = m_rankActorList.size();
// 		for(int i = 0; i < MAX_RANK_COUNT; i++)
// 		{
// 			DataPacket << (BYTE)i;
// 			RANKUSERLIST& rank = m_rankActorList[i];
// 			int count = rank.count();
// 			DataPacket << (BYTE)count;
// 			for(int j = 0; j < count; j++)
// 			{
// 				int k = j+1;
// 				DataPacket << (BYTE)k;
// 				unsigned int actorid = rank[j].m_nActorId;
// 				DataPacket << (unsigned int)actorid;
// 				DataPacket << (int )rank[j].m_nValue;
// 				DataPacket << (int )rank[j].m_nParam;
// 				DataPacket.writeString(rank[j].m_sName);
// 			}
// 		}
// 		AP.flush();	
// 	}
// }

//重置玩家的离线消息，将请求
void CTopTitleMgr::ResetOfflineUser()
{
// 	COfflineUserMgr & mgr = GetGlobalLogicEngine()->GetOfflineUserMgr();
// 	mgr.RemoveAllUser(); //先全部清除
// 	mgr.RemoveAllUserHero();	//清除全部英雄信息
// 	for (INT_PTR i = 0; i < MAX_RANK_COUNT; i++)
// 	{
// 		RANKUSERLIST& list = m_rankActorList[i];
// 		for(INT_PTR j=0; j< list.count();j ++ )
// 		{
// 			TOPTITLERANKINFO &data = list[j];
// 			if (data.m_nActorId <= 0)
// 				continue;
// 			if (data.m_nRankId >= rtHeroLevel && data.m_nRankId <= rtWizardHeroBp)
// 			{
// 				if (data.m_nParam <= 0)
// 					continue;
// 				mgr.LoadUserHero(data.m_nActorId, data.m_nParam);
// 			}
// 			else if (data.m_nRankId == rtMagicItemBp)
// 			{
// 				//mgr.LoadMagicData(data.m_nActorId);
// 			}
// 			else
// 			{
// 				mgr.LoadUser(data.m_nActorId);
// 			}
// 		}
// 	}
}

bool CTopTitleMgr::SetOnlineActorHeadTitle()
{
	bool bSet = false;
	CVector<void*> pEntityList;
	pEntityList.clear();
	GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineAcotrPtrList(pEntityList);
	INT_PTR nCount = pEntityList.count();
	for (INT_PTR i = 0; i < nCount; i++)
	{
		CActor *pActor = (CActor *)pEntityList[i];
		if (pActor && pActor->GetType() == enActor)
		{
			// if (pActor->OnGetIsTestSimulator()
			// 	|| pActor->OnGetIsSimulator() )
			// {
			// 	continue; 　
			// }　　
			bSet = SetRankHeadTitle(pActor);
		}
	}
	return bSet;
}


// void CTopTitleMgr::UpdateRankData()
// {
// 	// 清空引用计数
// 	m_nRefCount = 0;
// 	this->UpdateBaseRankData();
// 	//GetGlobalLogicEngine()->GetCampComponent().UpdateWorldCampContrRankEveryDay();
// }

void CTopTitleMgr::ResetNpcStatue()
{
	// CMiscProvider &provider = GetLogicServer()->GetDataProvider()->GetMiscConfig();
	// CVector<NPCSTATUE> *pNpcStatueList = provider.GetNpcStatueList();
	// for (int i = 0; i < pNpcStatueList->count(); i++)
	// {
	// 	NPCSTATUE npc = pNpcStatueList->get(i);
	// 	CNpc *pNpc = CNpc::GetNpcPtr(npc.sSceneName, npc.sNpcName);
	// 	if (pNpc)
	// 	{
	// 		//pNpc->ChangeShowName(g_nRadius, npc.sNpcName);
	// 		//pNpc->SetProperty<UINT>(PROP_ENTITY_ID, 0);
	// 		// 设置属性
	// 		//pNpc->SetModelId(0);
	// 		//pNpc->SetWeaponAppearId(0);
	// 		//pNpc->SetTitle(npc.sTitle);
	// 	}
	// }
}

bool CTopTitleMgr::SetNpcStatue(INT_PTR nType, INT_PTR nActorId, LPCTSTR sName, INT_PTR nSex, INT_PTR nParam)
{
	// CMiscProvider &provider = GetLogicServer()->GetDataProvider()->GetMiscConfig();
	// PNPCSTATUE pStatue = provider.GetNpcStatue(nType, nParam);
	// if (pStatue == NULL) return false;
	// CNpc *pNpc = CNpc::GetNpcPtr(pStatue->sSceneName, pStatue->sNpcName);
	// if (pNpc == NULL) 
	// {
	// 	OutputMsg(rmError, "set npc statue error, npc can not be found!npcName=%s", pStatue->sNpcName);
	// 	return false;
	// }
	// if (nActorId == 0)
	// {
	// 	pNpc->ChangeShowName(g_nRadius, (LPCTSTR)pStatue->sNpcName);
	// }
	// else
	// {
	// 	pNpc->ChangeShowName(g_nRadius, sName);
	// }

	// pNpc->SetProperty<unsigned int>(PROP_ENTITY_ID, (unsigned int)nActorId);
	// INT_PTR nModelId = pStatue->nMaleModelId;
	// if (nSex == 1)
	// 	nModelId = pStatue->nFemaleModelId;
	// pNpc->ChangeModel(g_nRadius, (int)nModelId);

	return true;
}

void CTopTitleMgr::UpdateRawRankList()
{
	// INT_PTR nServerId = GetLogicServer()->GetServerIndex();
	// COfflineUserMgr &mgr = GetGlobalLogicEngine()->GetOfflineUserMgr();
	// m_RawRankList.clear();
	// for (INT_PTR i = rtWarriorBp; i <= rtWizardBp; ++ i)
	// {
	// 	if (m_rankActorList[i].count() == 0) 
	// 		continue;
	// 	TOPTITLERANKINFO &info = m_rankActorList[i][0];
	// 	COfflineUser *pUser = mgr.GetUserByActorId(info.m_nActorId, true);
	// 	INT_PTR nVocation = pUser->nVocation;
	// 	if (nVocation <= enVocNone || nVocation >= enMaxVocCount)
	// 		continue;
	// 	RANKDATA tmpData;
	// 	tmpData.nActorId = info.m_nActorId;
	// 	strcpy(tmpData.sName, pUser->name);
	// 	tmpData.sName[sizeof(tmpData.sName)-1] = 0;
	// 	tmpData.nSex = pUser->nSex;
	// 	tmpData.nVocation = pUser->nVocation;
	// 	tmpData.nServerId = (int)nServerId;
	// 	tmpData.nValue = pUser->nBattlePower;

	// 	m_RawRankList.add(tmpData);
	// }
	
	// m_nDelay = 0;
}

void CTopTitleMgr::UpdateVocNpcStatue(INT_PTR nType)
{
	if (nType != stRsVocation && nType != stCsVocation) return;
	COfflineUserMgr &offMgr = GetGlobalLogicEngine()->GetOfflineUserMgr();
	CVector<NPCSTATUE> *pNpcList = GetLogicServer()->GetDataProvider()->GetMiscConfig().GetNpcStatueList();
	for (INT_PTR i = 0; i < pNpcList->count(); ++ i)
	{
		NPCSTATUE npc = pNpcList->get(i);
		if ((INT_PTR)npc.btType != nType)
			continue;
		bool bSet = false;
		if (nType == stRsVocation)
		{
			for (INT_PTR j = 0; j < m_RawRankList.count(); ++ j)
			{
				PRANKDATA pData = &m_RawRankList[j];
				if (pData->nVocation == (unsigned int)npc.btParam)
				{
					SetNpcStatue(nType, pData->nActorId, pData->sName, pData->nSex, pData->nVocation);
					bSet = true;
					break;
				}
			}
		}
		else
		{
			for (INT_PTR j = 0; j < m_CsRankList.count(); ++ j)
			{
				PRANKDATA pData = &m_CsRankList[j];
				if (pData->nVocation == (unsigned int)npc.btParam)
				{
					SetNpcStatue(nType, pData->nActorId, pData->sName, pData->nSex, pData->nVocation);
					bSet = true;
					break;
				}
			}
		}
		
		if (!bSet)
		{
			SetNpcStatue(nType, 0, npc.sNpcName, 0, npc.btParam);
		}

	}
}

void CTopTitleMgr::SaveCsRank()
{
	CLogicSSClient *pSSClient = GetLogicServer()->GetSessionClient();
	CDataPacket &packet = pSSClient->allocProtoPacket(jxInterSrvComm::SessionServerProto::cSaveCsRank);
	INT_PTR nCount = m_RawRankList.count();
	packet << (WORD)nCount;
	for (INT_PTR i = 0; i < nCount; i++)
	{
		RANKDATA data = m_RawRankList[i];
		packet.writeBuf(&data, sizeof(RANKDATA));
	}
	pSSClient->flushProtoPacket(packet);
	OutputMsg(rmTip, _T("发消息给Session保存跨服排行榜！"));
}

void CTopTitleMgr::LoadCsRank()
{
	CLogicSSClient *pSSClient = GetLogicServer()->GetSessionClient();
	CDataPacket &packet = pSSClient->allocProtoPacket(jxInterSrvComm::SessionServerProto::cLoadCsRank);
	pSSClient->flushProtoPacket(packet);
	OutputMsg(rmTip, _T("发消息给Session获取跨服排行榜！"));
}

void CTopTitleMgr::ClearCsRank(INT_PTR nServerId)
{
	CLogicSSClient *pSSClient = GetLogicServer()->GetSessionClient();
	CDataPacket &packet = pSSClient->allocProtoPacket(jxInterSrvComm::SessionServerProto::cClearCsRank);
	packet << (int)nServerId;
	pSSClient->flushProtoPacket(packet);
	OutputMsg(rmTip, _T("发消息给Session清除跨服排行榜！ServerId=%d"), nServerId);
}

void CTopTitleMgr::BroadLogicToSaveCsRank()
{
	char buff[256] = {0};
	CDataPacket packet(buff, sizeof(buff));
	WORD nCmd = (WORD)jxInterSrvComm::SessionServerProto::lcBroadToSaveCsRank;
	CLogicSSClient *pSSClient = GetLogicServer()->GetSessionClient();
	pSSClient->BroadMsgToLogicClient(nCmd, packet.getMemoryPtr(), packet.getLength());
}

void CTopTitleMgr::BroadLogicToLoadCsRank()
{
	char buff[256] = {0};
	CDataPacket packet(buff, sizeof(buff));
	WORD nCmd = (WORD)jxInterSrvComm::SessionServerProto::lcBroadToLoadCsRank;
	CLogicSSClient *pSSClient = GetLogicServer()->GetSessionClient();
	pSSClient->BroadMsgToLogicClient(nCmd, packet.getMemoryPtr(), packet.getLength());
}

void CTopTitleMgr::OnSSDBReturnData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader)
{
	if (jxInterSrvComm::SessionServerProto::sLoadCsRank == nCmd)
	{
		if (nErrorCode != jxInterSrvComm::SessionServerProto::neSuccess)
		{
			OutputMsg(rmError, "LoadCsRank Error! Code=%d", nErrorCode);
			return;
		}

		int nCount = 0;
		m_CsRankList.clear();
		reader >> nCount;
		for (INT_PTR i = 0; i < nCount; i++)
		{
			RANKDATA data;
			reader.readBuf(&data, sizeof(RANKDATA));
			m_CsRankList.add(data);
		}

		//更新NPC雕像
		UpdateVocNpcStatue(stCsVocation);
	}
}

void CTopTitleMgr::OnOtherLogicServerRetData(int nCmd, int nOtherServerId, CDataPacketReader &reader)
{
	switch (nCmd)
	{
	case jxInterSrvComm::SessionServerProto::lcSaveOfflineData:
		{
			SetUserOfflineData(reader);
		}
		break;
	case jxInterSrvComm::SessionServerProto::lcLoadOfflineData:
		{
			unsigned int nActorId = 0;
			reader >> nActorId;
			ReturnUserOfflineData(nOtherServerId, nActorId);
		}
		break;
	case jxInterSrvComm::SessionServerProto::lcBroadToSaveCsRank:
		{
			SaveCsRank();
		}
		break;
	case jxInterSrvComm::SessionServerProto::lcBroadToLoadCsRank:
		{
			LoadCsRank();
		}
		break;
		/*
	case jxInterSrvComm::SessionServerProto::lcClearHundredServerCsRank:
		{
			GetGlobalLogicEngine()->GetMiscMgr().ClearHundredCrossRank();
			break;
		}
	case jxInterSrvComm::SessionServerProto::lcBroadTreasureAuctionCsRank:
		{
			//GetGlobalLogicEngine()->GetMiscMgr().OtherWuYiAuctionRankToLocal(reader);
			break;
		}
		*/
	}
}

void CTopTitleMgr::SetUserOfflineData(CDataPacketReader &reader)
{
	// unsigned int	nActorId = 0;
	// reader >> nActorId;
	// if (nActorId <= 0)
	// 	return;
	// bool isExit;
	// COfflineUserMgr &mgr = GetGlobalLogicEngine()->GetOfflineUserMgr();
	// COfflineUser *pUser = mgr.AddOffLineUser(nActorId, isExit);
	// if (!isExit && pUser)
	// {
	// 	mgr.OnLoadActorBasic(nActorId, reader);
	// 	mgr.OnLoadActorOther(nActorId, reader);
	// }	
}

void CTopTitleMgr::LoadUserOfflineData(int nDestServerId, unsigned int nActorId)
{
	// if (nActorId <= 0 || nDestServerId <= 0) 
	// 	return;

	// //本服的不需要跨服请求
	// if (GetLogicServer()->GetServerIndex() == nDestServerId)
	// {
	// 	COfflineUser *pUser = GetGlobalLogicEngine()->GetOfflineUserMgr().GetUserByActorId(nActorId, true);
	// 	return;
	// }
	// //有数据不需要请求
	// if (GetGlobalLogicEngine()->GetOfflineUserMgr().GetUserByActorId(nActorId))
	// {
	// 	return;
	// }
	// char buff[256] = {0};
	// CDataPacket packet(buff, sizeof(buff));
	// WORD nCmd = (WORD)jxInterSrvComm::SessionServerProto::lcLoadOfflineData;
	// packet << (unsigned int)nActorId;
	// CLogicSSClient *pSSClient = GetLogicServer()->GetSessionClient();
	// pSSClient->SendMsgToDestLogicClient(nCmd, nDestServerId, packet.getMemoryPtr(), packet.getLength());
}

void CTopTitleMgr::ReturnUserOfflineData(int nDestServerId, unsigned int nActorId)
{
	// if (nActorId <= 0)
	// 	return;
	// int nRawServerId = GetLogicServer()->GetServerIndex();
	// if (nDestServerId == nRawServerId)
	// 	return;
	// COfflineUser *pUser = GetGlobalLogicEngine()->GetOfflineUserMgr().GetUserByActorId(nActorId, true);
	// if (pUser == NULL || pUser->name[0] == '\0')
	// 	return;
	// CLogicSSClient *pSSClient = GetLogicServer()->GetSessionClient();
	// CDataPacket &packet = pSSClient->allocProtoPacket(jxInterSrvComm::SessionServerProto::cBroadcastOr1SrvMessage);
	// packet << (int)nDestServerId << (int)nRawServerId;
	// packet << (WORD)jxInterSrvComm::SessionServerProto::lcSaveOfflineData;
	// packet << nActorId;
	// packet.writeBuf(pUser, sizeof(ActorOfflineMsg));
	// //packet << pUser->gemData;
	// INT_PTR nPos = packet.getPosition();
	// packet << (int)0;
	// int nCount = 0;
	// for(INT_PTR i = 0; i < CEquipVessel::EquipmentCount; i++)
	// {
	// 	CUserItem *pUserItem = pUser->equips.m_Items[i];
	// 	if (pUserItem)
	// 	{
	// 		((CUserItem)*pUserItem) >> packet;
	// 		// packet.writeBuf(pUserItem, sizeof(CUserItem));
	// 		nCount++;
	// 	}
	// }
	// int *pCount = (int *)packet.getPositionPtr(nPos);
	// *pCount = (int)nCount;
	// pSSClient->flushProtoPacket(packet);
}

int CTopTitleMgr::GetServerIdByCsRank(unsigned int nActorId)
{
	if (nActorId <= 0)
		return 0;
	for (INT_PTR i = 0; i < m_CsRankList.count(); i++)
	{
		RANKDATA &data = m_CsRankList[i];
		if (data.nActorId == nActorId)
		{
			return data.nServerId;
		}
	}
	return 0;
}


bool CTopTitleMgr::SetRankHeadTitle(CActor *pActor)
{
	if (pActor == NULL) return false;
	unsigned int nActorId = pActor->GetId();
	CVector<TOPTITLECONFIG> &tTitleList = GetLogicServer()->GetDataProvider()->GetAchieveConfig().GetTopTitleList();
	INT_PTR nConfigCount = tTitleList.count();
	if (nConfigCount <= 0) return false;
	for (INT_PTR i = 0; i < nConfigCount; ++i)
	{
		TOPTITLECONFIG &tTitleConfig = tTitleList[i];
		INT_PTR nRankIndex = (INT_PTR)(tTitleConfig.nRankId);
		if (nRankIndex < 0 || nRankIndex >= MAX_RANK_COUNT)
			continue;
		RANKUSERLIST &data = m_rankActorList[nRankIndex];
		INT_PTR nRankCount = data.count();
		INT_PTR nMinRank = __min(tTitleConfig.nMinRank, tTitleConfig.nMaxRank);
		INT_PTR nMaxRank = __max(tTitleConfig.nMinRank, tTitleConfig.nMaxRank);

		bool bSetFlag = false;
		for (INT_PTR j = 0; j < nRankCount; ++j)
		{
			TOPTITLERANKINFO& info = data[j];
			if (info.m_nActorId == nActorId && info.m_nRank >= nMinRank && info.m_nRank <= nMaxRank)
			{
				bSetFlag = true;
				pActor->SetHeadTitle(tTitleConfig.nTitleId, true);
				break;
			}	
		}

		if (!bSetFlag)
			pActor->SetHeadTitle(tTitleConfig.nTitleId, false);
		
	}

	return true;
}


void CTopTitleMgr::SetTopMaxLevel(int nLevel)
{
	if(nLevel > nMaxTopLevel)
	{
		nMaxTopLevel = nLevel;
		GetGlobalLogicEngine()->GetMiscMgr().SetSystemStaticVarValue(szTopMaxLevel,nMaxTopLevel);
	}
		
}



int CTopTitleMgr::GetTopLevel()
{
	RANKUSERLIST& list = m_rankActorList[rtAllLevel];
	if(list.count() <= 0 )
	{
		return 1;
	}
	return list[0].m_nValue;
}
