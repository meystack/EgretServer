#include "StdAfx.h"
#include "Container.hpp"
CBufferAllocator* CRankingMgr::m_pAllocator;

RankAllocator::pointer
RankAllocator::allocate(size_type _n, const void*)
{
    if (CRankingMgr::m_pAllocator)
    {
        return (pointer)CRankingMgr::m_pAllocator->AllocBuffer(_n);
    }
    return NULL;
}
void RankAllocator::deallocate(pointer _p, size_type)
{
    if (CRankingMgr::m_pAllocator)
    {
        CRankingMgr::m_pAllocator->FreeBuffer(_p);
    }
}

void RankAllocator::destroy(pointer pRank)
{
	pRank->~CRanking();
}
CRankingMgr::CRankingMgr():m_RankingList()
{ 
}
BOOL CRankingMgr::Initialize()
{
	return TRUE;
}
CRankingMgr::~CRankingMgr()
{
	Destroy();
}
VOID CRankingMgr::Destroy()
{
	//释放资源
#ifdef _USE_TRY_CATCH
#ifdef WIN32
	__try
#endif
#endif 
	{	
		RankIterator iter = m_RankingList.begin();
		OutputMsg(rmTip,"bigin remove rank ,conut =%d", m_RankingList.size());
		for (; iter != m_RankingList.end();)
		{
			CRanking& rank = (*iter).second;
			OutputMsg(rmTip,"remove rank ,%s,rankid=%d",rank.GetName(),(*iter).first);

			LPCSTR sName = rank.GetName();

			//如果销毁的时候已经被销毁了
			if(rank.IsModify())
			{
				rank.Save(NULL,true);
			}
			m_RankingList.erase(iter++);
		}
		OutputMsg(rmTip, "remove rank all sucess!"  );
		m_RankingList.clear();
	}
#ifdef _USE_TRY_CATCH
#ifdef WIN32
	__except(CustomExceptHandler(GetExceptionInformation()))
	{
	}	
#endif 
#endif 
}

CRanking* CRankingMgr::GetRanking( INT_PTR rankId )
{
	if (m_RankingList.find(rankId) != m_RankingList.end())
		return &m_RankingList[rankId];
	return NULL;
}

void CRankingMgr::Remove( INT_PTR rankId  )
{
	if (m_RankingList.find(rankId) != m_RankingList.end())
	{
		CRanking& rank = m_RankingList[rankId];

		// 防止活动排行榜重新加载文件，因此删除文件
		char sPath[256];
		sprintf(sPath,"./data/runtime/rank/%s.Rank", rank.GetName());
		if (FDOP::FileExists(sPath) && (-1 == remove(sPath)))
		{
			OutputMsg(rmTip,"CRankingMgr::Remove %s,rankfilename=%s failed",rank.GetName(),sPath);
		}
		OutputMsg(rmTip,"CRankingMgr::Remove %s,rank=%d",rank.GetName(),rankId);

		// 销毁
		m_RankingList.erase(m_RankingList.find(rankId));
		return;
	}
	NotifyUpdate();
}

CRanking* CRankingMgr::Add( INT_PTR rankId,LPCSTR sFileName,INT_PTR nMax ,int boDisplay,int nBroadCount)
{
	if (!sFileName || strcmp(sFileName,"") == 0) return NULL;

	CRanking* pRank = GetRanking(rankId);
	if (pRank)
	{
		OutputMsg(rmTip,"add rank exit,id=%d,%s",rankId,sFileName);
		return NULL;
	}
	
	CRanking& rank = m_RankingList[rankId];
	OutputMsg(rmTip,"add rank ,%s,ptr=%d",sFileName,rankId);
	new(&rank)CRanking(sFileName,nMax,boDisplay != 0 ? true : false, nBroadCount);
	
	if (boDisplay)
	{
		NotifyUpdate();
	}
	return &rank;
}

void CRankingMgr::NotifyUpdate()
{
	char data[2];
	data[0] = enMiscSystemID;
	data[1] = sRankingUpdate;
	GetGlobalLogicEngine()->GetEntityMgr()->BroadCast(data,sizeof(data));
}

void CRankingMgr::NotifyRankUpdateCs(INT_PTR rankId)
{
	CRanking *rank = GetRanking(rankId);
	if (!rank) return;
	CLogicSSClient *pSSClient = GetLogicServer()->GetSessionClient();
	CDataPacket &packet = pSSClient->allocProtoPacket(jxInterSrvComm::SessionServerProto::cSendGroupMessage);
	packet << (WORD)jxInterSrvComm::SessionServerProto::fcRankUpdate;
	packet.writeString(rank->GetName(), strlen(rank->GetName()));	// 排行榜名称
	int nColCount = rank->GetColCount();
	packet << (int)nColCount;	// 排行榜sub列数
	CVector<CRankingItem *> &itemList = rank->GetList();
	int nCount = __min(rank->GetMax(), (int)(itemList.count()));
	packet << (int)nCount;	// 排行榜项数
	// 发排行榜项
	for (INT_PTR i = 0; i < nCount; i++)
	{
		CRankingItem *item = itemList[i];
		packet << (int)item->GetId() << (int)item->GetPoint();	// ID,分数
		for (INT_PTR j = 0; j < nColCount; j++)
		{
			packet.writeString(item->GetSubData(j));
		}
	}
	pSSClient->flushProtoPacket(packet);
}

void CRankingMgr::OnUpdateCsRank(CDataPacketReader &packet)
{
	// char szName[32] = {0};
	// packet.readString(szName, sizeof(szName));
	// CRanking *rank = GetRanking(szName);
	// if (!rank) return;
	// rank->clear();
	// int nColCount, nCount;
	// packet >> nColCount >> nCount;
	// for (INT_PTR i = 0; i < nCount; i++)
	// {
	// 	int nId, nPoint;
	// 	packet >> nId >> nPoint;
	// 	CRankingItem *item = rank->AddItem((INT_PTR)nId, (INT_PTR)nPoint);
	// 	for (INT_PTR j = 0; j < nColCount; j++)
	// 	{
	// 		char szSubData[100];
	// 		packet.readString(szSubData);
	// 		//szSubData[sizeof(szSubData)-1] = 0;	// 防止溢出 // readString 已经做了这个工作，多此一举
	// 		item->SetSub(j, szSubData);
	// 	}
	// }
}

VOID CRankingMgr::RunOne(TICKCOUNT currTick)
{
	//OutputMsg( rmTip, "[CRankingMgr] --------------------------------------------- Tick: %lld, SaveTime:%lld", (long long)currTick, (long long)m_SaveTime.GetNextTime());
	// 即时排行榜，定时存文件
	if (m_SaveTime.CheckAndSet(currTick))
	{
		SYSTEMTIME curTime;  
		CMiniDateTime ncurTime(GetGlobalLogicEngine()->getMiniDateTime());
		ncurTime.decode(curTime); 

		OutputMsg( rmTip, "[CRankingMgr] curTime Tick: %04d-%02d-%02d %02d:%02d:%02d", curTime.wYear, curTime.wMonth, curTime.wDay, curTime.wHour, curTime.wMinute, curTime.wSecond); 
		 
		SystemTime sysTime;
		GetSystemTime(sysTime);
		OutputMsg( rmTip, "[CRankingMgr] curTime2 Tick: %04d-%02d-%02d %02d:%02d:%02d", sysTime.year_, sysTime.mon_, sysTime.mday_, sysTime.hour_, sysTime.min_, sysTime.sec_); 
 
		// //更新日期和时间 
		// SYSTEMTIME	tCurSysTime;
		// GetLocalTime(&tCurSysTime);
		// CMiniDateTime CurMiniDateTime;
		// UINT nowtime = CurMiniDateTime.encode(tCurSysTime); 
		// OutputMsg( rmTip, "[CRankingMgr] curTime3 Tick: %d-%d-%d %d:%d:%d", tCurSysTime.wYear, tCurSysTime.wMonth, tCurSysTime.wDay, tCurSysTime.wHour, tCurSysTime.wMinute, tCurSysTime.wSecond); 
		 
		time_t tt;
		time( &tt );
		tt = tt + 8*3600;  // transform the time zone
		tm* t= gmtime( &tt ); 
			OutputMsg( rmTip, "[CRankingMgr] curTime3 Tick: %04d-%02d-%02d %02d:%02d:%02d", 
				t->tm_year + 1900,
				t->tm_mon + 1,
				t->tm_mday,
				t->tm_hour,
				t->tm_min,
				t->tm_sec);
		
		OutputMsg( rmTip, "[CRankingMgr] CRankingMgr::RunOne Tick: %lld, SaveTime:%lld", (long long)currTick, (long long)m_SaveTime.GetNextTime());
		Save();
	}

	// 基础排行榜，定时从数据服获取
	if (m_LoadBaseRankTime.CheckAndSet(currTick, true))
	{
		if (!GetLogicServer()->IsCrossServer())
		{ 
			//不是跨服
			UpdateBaseRankData();
		}
		else
		{ 
			UpdateBaseRankCSData();
		}
		
		LoadBaseRankData();
	}
	if (m_LoadCenterTankTime.Check(currTick))
	{
		LoadCenterRankData(enCSALLLevel);
	}
	if (m_LoalCenterMoBaiRankTime.Check(currTick))
	{
		LoadCenterRankData(enCSMoBaiRank);
	} 
}

/*服务器关闭，保存排行榜，不在这里
在VOID CRankingMgr::Destroy()中
*/
VOID CRankingMgr::Save()
{
	RankIterator iter = m_RankingList.begin();
	for (; iter != m_RankingList.end();iter++)
	{
		CRanking& rank = (*iter).second;
		if (rank.IsValid() && rank.IsModify())
		{
			OutputMsg(rmTip,"Save rank:%d %s", (*iter).first, rank.GetName());
			rank.Save(NULL,true);
		}
	}
}
void CRankingMgr::UpdateBaseRankCSData()
{
	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		CDataPacket &DataPacket =pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveBaseRankCS);
		int nServerId = GetLogicServer()->GetServerIndex();
		DataPacket << nServerId << nServerId;
		 
		OutputMsg(rmTip,"[UpdateBaseRankCSData]1 跨服的排行榜保存数据 !" );
    	CMiniDateTime now_time = CMiniDateTime::now();  
		DataPacket << (unsigned int)now_time;
		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	}
}
void CRankingMgr::UpdateBaseRankData()
{
	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		CDataPacket &DataPacket =pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveBaseRank);
		int nServerId = GetLogicServer()->GetServerIndex();
		DataPacket << nServerId << nServerId;
		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	}
}
void CRankingMgr::LoadBaseRankData()
{
	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		CDataPacket& DataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadBaseRank);
		int nServerId = GetLogicServer()->GetServerIndex(); 
		DataPacket << nServerId << nServerId;
		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	}
}

void CRankingMgr::LoadCenterRankData(int nRankId)
{
	CLocalCrossClient* pCSClient = GetLogicServer()->GetCrossClient();
	if (pCSClient->connected())
	{
		CDataPacket& DataPacket = pCSClient->allocProtoPacket(jxInterSrvComm::CrossServerProto::cCSGetRankList);
		int nServerId = GetLogicServer()->GetCrossServerId(); 
		DataPacket << nServerId << nRankId;
		GetLogicServer()->GetCrossClient()->flushProtoPacket(DataPacket);
	}
	if(enCSALLLevel == nRankId)
	{ 
		m_LoadCenterTankTime.SetNextHitTimeFromNow(43200000);//43200000
	}
	else if(enCSMoBaiRank == nRankId)
	{
		m_LoalCenterMoBaiRankTime.SetNextHitTimeFromNow(43200000);//86400000 
	}
}

void CRankingMgr::OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size)
{
	// 已存储基础排行榜
	if (nCmd == jxInterSrvComm::DbServerProto::dcSaveBaseRank)
	{
		CDataPacketReader inPacket(data, size);
		int nRawServerId = 0, nLoginServerId = 0;
		BYTE nErrorCode;
		inPacket >> nRawServerId >> nLoginServerId >> nErrorCode;
		if (nErrorCode != jxInterSrvComm::DbServerProto::reSucc)
		{
			OutputMsg(rmError, _T("Save BaseRank Error! code = %d"), nErrorCode);
			return;
		}
	}
	else if (nCmd == jxInterSrvComm::DbServerProto::dcSaveBaseRankCS)
	{
		CDataPacketReader inPacket(data, size);
		int nRawServerId = 0, nLoginServerId = 0;
		BYTE nErrorCode;
		inPacket >> nRawServerId >> nLoginServerId >> nErrorCode;
		if (nErrorCode != jxInterSrvComm::DbServerProto::reSucc)
		{
			OutputMsg(rmError, _T("Save BaseRankCS Error! code = %d"), nErrorCode);
			return;
		}
	}
	// 读取排行榜入内存
	else if (nCmd == jxInterSrvComm::DbServerProto::dcLoadBaseRank)
	{
		CDataPacketReader inPacket(data, size);
		int nRawServerId = 0, nLoginServerId = 0;
		BYTE nErrorCode;
		inPacket >> nRawServerId >> nLoginServerId >> nErrorCode;
		m_oldrankLists = m_newrankLists;
		m_newrankLists.clear();
		static int nMinRankValue = GetLogicServer()->GetDataProvider()->GetRankConfig().nMinRankId;
		if (nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
		{
			int nCount = 0;
			inPacket >> nCount;

			//建立排行榜
			for (INT_PTR rankId = enRankBaseBegin; rankId < enRankBaseEnd; rankId++)
			{
				if(!GetLogicServer()->IsCrossServer()
				 	&& (rankId == enCSALLLevel || rankId == enCSMoBaiRank))
					 
					continue;
				CRanking* pRank = GetRanking(rankId);
				if (pRank == NULL)
				{
					pRank = Add(rankId, "None", 1000, false);
				}
				pRank->SetSaveFlag(false);
				pRank->clear();
			}
			int nMaxTopLevel = GetGlobalLogicEngine()->GetMiscMgr().GetSystemStaticVarValue(szTopMaxLevel);
			
			// 设置数据
			for (INT_PTR i = 0; i < nCount; i ++)
			{
				int nVip = 0;
				TOPTITLERANKINFO info;
				inPacket >> info.m_nActorId >> info.m_nRankId >> info.m_nRank >> info.m_nValue>> info.m_nParam;
				inPacket.readString(info.m_sName,ArrayCount(info.m_sName));
				inPacket >> info.m_nJob;
				inPacket >> info.m_nSex;
				inPacket >> info.n_yyVip;
				inPacket.readString(info.m_sGuildName,ArrayCount(info.m_sGuildName));
				inPacket >> nVip;
				INT_PTR nRankIndex = (INT_PTR)(info.m_nRankId);
				if((!GetLogicServer()->IsCrossServer()) && (nRankIndex == enCSALLLevel || nRankIndex == enCSMoBaiRank))
					continue;
				CRanking* pRank = GetRanking(nRankIndex);
				if (pRank)
				{
					CRankingItem* pItem = pRank->AddItem(info.m_nActorId, info.m_nValue,false);
					if (pItem)
					{
						pItem->AddColumn("name",-1);
						pItem->SetSub(0, info.m_sName);
						int nValue = info.m_nSex*1000 + info.m_nJob;
						pItem->SetParam(info.m_nParam);
						pItem->SetParam1(info.n_yyVip);
						pItem->AddColumn("sexJob",-1);
						pItem->SetSub(1, nValue);
						pItem->AddColumn("guildname",-1);
						pItem->SetSub(2, info.m_sGuildName);
						pItem->AddColumn("nVip",-1);
						pItem->SetSub(3, nVip);
					}
				}
				if(info.m_nRank <= nMinRankValue)
				{
					int nId = info.m_nRankId *100 + info.m_nRank ;
					m_newrankLists[info.m_nActorId].insert(nId);
				}
				if(enMoBaiRankList == info.m_nRankId)
				{
					if (!GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(info.m_nActorId) && 
						!GetGlobalLogicEngine()->GetOfflineUserMgr().GetUserByActorId(info.m_nActorId))
					{
						// 最后只能从数据库找
						GetGlobalLogicEngine()->GetOfflineUserMgr().LoadUser(info.m_nActorId);	
					}
				}
				if(enCSMoBaiRank == info.m_nRankId)
				{
					if (!GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(info.m_nActorId) && 
						!GetGlobalLogicEngine()->GetOfflineUserMgr().GetUserByActorId(info.m_nActorId))
					{
						// 最后只能从数据库找
						GetGlobalLogicEngine()->GetOfflineUserMgr().LoadUser(info.m_nActorId);	
					}
				}
				if(enCSALLLevel == info.m_nRankId && info.m_nRank < 20)
				{
					if (!GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(info.m_nActorId) && 
						!GetGlobalLogicEngine()->GetOfflineUserMgr().GetUserByActorId(info.m_nActorId))
					{
						// 最后只能从数据库找
						GetGlobalLogicEngine()->GetOfflineUserMgr().LoadUser(info.m_nActorId);	
					}
				}
				if(info.m_nRankId == 0 && info.m_nValue > nMaxTopLevel) {
					nMaxTopLevel = info.m_nValue;
				}
			}
			GetGlobalLogicEngine()->GetTopTitleMgr().SetTopMaxLevel(nMaxTopLevel);
		}
		else
		{
			OutputMsg(rmError,_T("Load TopTitleRank Error!code=%d"),nErrorCode);
			return;
		}
		SendRankTitle();
	}
}


void CRankingMgr::SendRankTitle(CActor* pActor)
{
	auto& mapRanks = GetLogicServer()->GetDataProvider()->GetRankConfig().m_RankConfigs;
	if(pActor)
	{
		unsigned int nActorId = pActor->GetId();
		auto it = m_newrankLists.find(nActorId);
		if(it != m_newrankLists.end()) {
			for(auto id : it->second)
			{
				auto config = mapRanks.find(id);
				if(mapRanks.end() != config)
				{
					pActor->GetNewTitleSystem().addNewTitle(config->second);
				}
			}
		}
	}
	else
	{

		for(auto key : m_oldrankLists)
		{
			CActor* actor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(key.first);
			if(actor)
			{
				for(auto id : key.second)
				{
					if(!m_newrankLists.count(key.first) || !m_newrankLists[key.first].count(id))
					{
						auto config = mapRanks.find(id);
						if(mapRanks.end() != config)
						{
							actor->GetNewTitleSystem().delNewTitle(config->second);
						}
					}
				}
			}
		}
		for(auto newtitles : m_newrankLists)
		{
			CActor* actor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(newtitles.first);
			if(actor)
			{
				for(auto id : newtitles.second)
				{
					auto config = mapRanks.find(id);
					if(mapRanks.end() != config && actor->GetNewTitleSystem().IsHaveNewTitle(config->second) < 0)
					{
						actor->GetNewTitleSystem().addNewTitle(config->second);
					}
				}
			}
		}
	}
}
//获取
void CRankingMgr::GetRankListByRankId(int nRankId, CDataPacket& inPacket)
{
	CRanking* pRank = GetRanking(nRankId);
	if (pRank == NULL)
	{
		pRank = Add(nRankId, "None", 1000, false);
	}
	pRank->SendCrossServerToPack(100,inPacket);
}

//设置
void CRankingMgr::SetRankList(int nRankId, CDataPacketReader& inPacket)
{
	// 已存储基础排行榜
	BYTE nErrorCode;
	BYTE nCount = 0;
	inPacket >> nCount;
	if(nRankId == enCSALLLevel)
		GetGlobalLogicEngine()->GetOfflineCenterUserMgr().Destroy(); //离线玩家管理器
	//建立排行榜
	CRanking* pRank = GetRanking(nRankId);
	if (pRank == NULL)
	{
		pRank = Add(nRankId, "None", 1000, false);
	}
	if (pRank)
	{
		pRank->SetSaveFlag(false);
		pRank->clear();
		for (INT_PTR i = 0; i < nCount; i ++)
		{
			// int nVip = 0;
			TOPTITLERANKINFO info;
			inPacket >> info.m_nActorId;
			inPacket >> info.m_nValue;
			inPacket.readString(info.m_sName,ArrayCount(info.m_sName));
			inPacket >> info.m_nParam;
			inPacket >> info.n_yyVip;
			char sValue[10] = {0};
			inPacket.readString(sValue,ArrayCount(sValue));
			inPacket.readString(info.m_sGuildName,ArrayCount(info.m_sGuildName));
			// char sPdId[10] = {0};
			// inPacket.readString(sPdId,ArrayCount(sPdId));
			// INT_PTR nRankIndex = (INT_PTR)(info.m_nRankId);
			char nVip[10] = {0};
			inPacket.readString(nVip,ArrayCount(nVip));
			CRankingItem* pItem = pRank->AddItem(info.m_nActorId, info.m_nValue,false);
			if (pItem)
			{
				pItem->AddColumn("name",-1);
				pItem->SetSub(0, info.m_sName);
				pItem->SetParam(info.m_nParam);
				pItem->SetParam1(info.n_yyVip);
				pItem->AddColumn("sexJob",-1);
				pItem->SetSub(1, sValue);
				pItem->AddColumn("guildname",-1);
				pItem->SetSub(2, info.m_sGuildName);
				pItem->AddColumn("nVip",-1);
				pItem->SetSub(3, nVip);
			}
		}
		// if(enMoBaiRankList == info.m_nRankId)
		// {
		// 	if (!GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(info.m_nActorId) && 
		// 		!GetGlobalLogicEngine()->GetOfflineUserMgr().GetUserByActorId(info.m_nActorId))
		// 	{
		// 		// 最后只能从数据库找
		// 		GetGlobalLogicEngine()->GetOfflineUserMgr().LoadUser(info.m_nActorId);	
		// 	}
		// }
	}
}
