#include "StdAfx.h"
#include "Container.hpp"
using namespace jxInterSrvComm::DbServerProto;


CConsignmentMgr::CConsignmentMgr()
{
	m_boInited = false;
	m_boNeedSort = false;
	m_nSellIndex = 0;
	m_OpenLists.clear();
}

CConsignmentMgr::~CConsignmentMgr()
{
	m_consignmentDataList.clear();
	//m_consignmentIncomeList.clear();
	m_ActorDataModifyList.clear();
}
void CConsignmentMgr::Destroy()
{
	SaveConsignItemToDb();
}
VOID CConsignmentMgr::Load()
{
	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (!m_boInited && pDbClient->connected())
	{
		CDataPacket& DataPacket =pDbClient->allocProtoPacket(dcLoadConsignmentItem);
		int nServerId = GetLogicServer()->GetServerIndex();
		DataPacket << nServerId << nServerId;
		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	}
}

VOID CConsignmentMgr::OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size)
{
	int nRawServerId, nLoginServerId;
	CDataPacketReader inPacket(data,size);
	int nErrorCode;
	inPacket >> nRawServerId >> nLoginServerId >>  nErrorCode;

	if (nCmd == dcLoadConsignmentItem && nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
	{
		WORD nCount = 0;bool isEnd = false;
		inPacket >>nCount >>isEnd;
		for (int i =0; i < nCount; i++)
		{
			
			ConsignmentData data ;
			memset(&data,0,sizeof(data));
			data.item << inPacket;
			inPacket >> data.nActorId; 
			inPacket >> data.nSellTime;
			inPacket >> data.nPrice;
			inPacket >> data.nState;
			inPacket >> data.nAutoId;
			if(data.nAutoId && data.nActorId == 0 && data.nState == eConsignmentOffShelf)
				continue;
			AddConsignItem(data, true);
			// addNewConsignItem(data);
		}
		if (isEnd)
		{
			m_boInited = true;
			CheckConsignTimeOut();
			m_boNeedSort = true;//读完数据排序一次
			ConsignmentProvider& consignmentProvider = GetLogicServer()->GetDataProvider()->GetConsignmentProvider();
			auto& nAutoMaps = consignmentProvider.getAutoInConsignment();
			if(nAutoMaps.size())
			{
				for(auto& cfg : nAutoMaps)
				{
					if(m_OpenLists.find(cfg.second.class1Id) != m_OpenLists.end())
						continue;
					m_nWaitAutoIns.insert(cfg.second);
				}
			}
		}
	}
}


VOID CConsignmentMgr::RunOne(CMiniDateTime& minidate,TICKCOUNT tick)
{
	SF_TIME_CHECK(); //检测性能	
	if (m_1minuteTimer.CheckAndSet(tick))
	{
		CheckConsignTimeOut();
		CheckCanAutoConsign();
	}
	if (m_3minuteTimer.CheckAndSet(tick))
	{
		DelOffShelf();
		SaveConsignItemToDb();
	}
}
void CConsignmentMgr::CheckConsignTimeOut()
{
	ConsignmentProvider& consignmentProvider = GetLogicServer()->GetDataProvider()->GetConsignmentProvider();
	uint32_t minidate = time(NULL);
	INT_PTR nCount = m_consignmentDataList.size();
	for (int i=0; i < nCount; i++)
	{
		ConsignmentData& pData = m_consignmentDataList[i];
		if (pData.nState == eConsignmentOnShelf)
		{
			if (minidate >= pData.nSellTime)
			{
				pData.nState = eConsignmentOffShelf;
				SetActorDataModify(pData.nActorId);
				CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(pData.nActorId);
				if (pActor)
				{
					pActor->GetConsignmentSystem().SenDClientRedPoint(true);
				}
			}
		}
	}
}


bool  CConsignmentMgr::DelConsignItem(ConsignmentData* pDelData)
{
	if (!pDelData)
	{
		return false;
	}
	if(m_consignmentDataList.size() == 0)
		return false;
	bool nFlag = false;
	unsigned int nActorId = pDelData->nActorId;
	std::vector<ConsignmentData>::iterator it = m_consignmentDataList.begin();
	for (; it != m_consignmentDataList.end(); it++)
	{
		ConsignmentData& pData = *it;
		if (pData.nSellIndex == pDelData->nSellIndex)
		{
			SetActorDataModify(pData.nActorId);
			m_boNeedSort = true;
			m_consignmentDataList.erase(it);	
			nFlag =  true;
			break;
		}
	}
	if(nFlag)
		SaveConsignItemToDb();
	return nFlag;
}



ConsignmentData* CConsignmentMgr::GetConsignItem(uint64_t  nSellIndex)
{
	for (INT_PTR i =0; i < m_consignmentDataList.size(); i++)
	{
		ConsignmentData* pData = &m_consignmentDataList[i];
		if (pData && pData->nSellIndex && pData->nSellIndex== nSellIndex)
		{
			return pData;
		}
	}
	return NULL;
}


bool CConsignmentMgr::AddConsignItem(ConsignmentData& nData, bool dbReturn)
{
	bool nFlag = false;
	if(nData.item.wItemId != 0 && nData.item.wCount != 0)
	{
		m_nSellIndex++;
		nData.nSellIndex = m_nSellIndex;
		m_consignmentDataList.push_back(nData);
		SetActorDataModify(nData.nActorId);
		m_boNeedSort = true;
		nFlag = true;
	}
	if(!dbReturn)
		SaveConsignItemToDb();
	return nFlag;
}

void CConsignmentMgr::DealBuyConsignItem(ConsignmentData* nData, unsigned int nActorId)
{
	if (!nData)
	{
		return;
	}

	ConsignmentData data;
	data.nActorId = nActorId;
	data.nPrice = nData->nPrice;
	data.nSellTime = nData->nSellTime;
	data.nState = eConsigmentActorBuy;
	memcpy(&data.item, &(nData->item), sizeof(data.item));
	AddConsignItem(data);
	SetActorDataModify(data.nActorId);
	m_boNeedSort = true;
}
void CConsignmentMgr::AddIncome(ConsignmentData* pData, int nCount)
{
}
int qsortCmp(const void *a ,const void *b)
{
	ConsignmentData* Sa = (ConsignmentData*)a;
	ConsignmentData* Sb = (ConsignmentData*)b;
	if (Sa->nPrice == Sb->nPrice)
	{
		return Sa->nSellTime - Sb->nSellTime;
	}
	return Sa->nPrice - Sb->nPrice;
}

void CConsignmentMgr::SortSearchItem()
{
	if (!m_boNeedSort)
	{
		return;
	}
	INT_PTR nItemCount = m_consignmentDataList.size();
	qsort(&m_consignmentDataList[0], nItemCount, sizeof(m_consignmentDataList[0]),qsortCmp);
	m_boNeedSort = false;
	//sort(List[0], List[nItemCount-1],sortCmp);
}
void CConsignmentMgr::FilterSearchItem(CVector<ConsignmentData*>& List, BYTE nClass1Id, BYTE nClass2Id, BYTE nMinLevel,BYTE nMaxLevel, char nCircle, BYTE nJob, char nQuality, const char *sName)
{
}
unsigned int CConsignmentMgr::GetIncomeValue(unsigned int nActorId)
{
	return 0;
}
ConsignmentIncome* CConsignmentMgr::GetIncome(unsigned int nActorId)
{
	return NULL;
}
bool CConsignmentMgr::DelIncome(unsigned int nActorId)
{
	return false;
}


void CConsignmentMgr::GetMyConsignItem(unsigned int nMyActorId,std::vector<ConsignmentData*>& myItemList)
{
	CConsignmentMgr& mgr = GetGlobalLogicEngine()->GetConsignmentMgr();
	for (INT_PTR i =0; i <mgr.m_consignmentDataList.size();i ++)
	{
		ConsignmentData* pData = &mgr.m_consignmentDataList[i];
		if (pData && pData->nActorId == nMyActorId)
		{
			myItemList.push_back(pData);
		}
	}	
}



void  CConsignmentMgr::GetMyConsignOnSellItem(unsigned int nMyActorId, std::vector<ConsignmentData*>& myItemList)
{
	CConsignmentMgr& mgr = GetGlobalLogicEngine()->GetConsignmentMgr();
	for (INT_PTR i =0; i <mgr.m_consignmentDataList.size();i ++)
	{
		ConsignmentData* pData = &mgr.m_consignmentDataList[i];
		if (pData && pData->nActorId == nMyActorId && pData->nState == eConsignmentOnShelf)
		{
			myItemList.push_back(pData);
		}
	}
}

bool  CConsignmentMgr::GetMyCanGetConsignItem(unsigned int nMyActorId)
{
	CConsignmentMgr& mgr = GetGlobalLogicEngine()->GetConsignmentMgr();
	for (INT_PTR i =0; i <mgr.m_consignmentDataList.size();i ++)
	{
		ConsignmentData* pData = &mgr.m_consignmentDataList[i];
		if (pData && pData->nActorId == nMyActorId && pData->nState > eConsignmentOnShelf)
		{
			return true;
		}
	}
	return false;
}

bool CConsignmentMgr::IsActorDataModify(unsigned int nActorId)
{
	for (INT_PTR i =0; i <m_ActorDataModifyList.size(); i++)
	{
		if (m_ActorDataModifyList[i] == nActorId)
		{
			return true;
		}
	}
	return false;
}
void CConsignmentMgr::SetActorDataModify(unsigned int nActorId)
{
	if (IsActorDataModify(nActorId))
	{
		return;
	}
	else
	{
		m_ActorDataModifyList.push_back(nActorId);
	}
}

void CConsignmentMgr::SaveActorConsignToDb()
{	
}
void CConsignmentMgr::SaveConsignItemToDb()
{
	int nServerId = GetLogicServer()->GetServerIndex();
	INT_PTR nDataCount = m_consignmentDataList.size();
	INT_PTR packetCount = (nDataCount-1)/CONSIGN_SAVE_COUNT_EACH_TIME+1;
	INT_PTR nNeedSendItemCount =0;
	for (INT_PTR curPacketCount =1; curPacketCount <= packetCount; curPacketCount++)
	{
		if (curPacketCount* CONSIGN_SAVE_COUNT_EACH_TIME > nDataCount)
		{
			nNeedSendItemCount =  nDataCount - (curPacketCount-1)*CONSIGN_SAVE_COUNT_EACH_TIME;
		}
		else
		{
			nNeedSendItemCount = CONSIGN_SAVE_COUNT_EACH_TIME;
		}
		CDataPacket & dbNetPack = GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveConsignmentItem);
		dbNetPack << nServerId << nServerId;
		dbNetPack << (int)curPacketCount; 
		dbNetPack << (WORD)nNeedSendItemCount;
		for (INT_PTR i = (curPacketCount-1)* CONSIGN_SAVE_COUNT_EACH_TIME; (i < curPacketCount* CONSIGN_SAVE_COUNT_EACH_TIME) && (i < nDataCount); i++)
		{
			ConsignmentData* data = &m_consignmentDataList[i];
			if (!data)
			{
				continue;
			}
			(CUserItem)data->item >> dbNetPack;
			dbNetPack << data->nSellTime << data->nPrice << data->nActorId << data->nState << data->nAutoId;
		}
		GetLogicServer()->GetDbClient()->flushProtoPacket(dbNetPack);
	}	
}



void CConsignmentMgr::CheckCanAutoConsign()
{
	ConsignmentProvider& consignmentProvider = GetLogicServer()->GetDataProvider()->GetConsignmentProvider();
	int openday = GetLogicServer()->GetDaysSinceOpenServer() ;
	int nowtime = GetGlobalLogicEngine()->getMiniDateTime();
	if(m_OpenLists.size() > 0) {
		auto it = m_OpenLists.begin();
		for(; it != m_OpenLists.end();)
		{
			if(it->second.nEndDay != 0 && it->second.nEndDay < openday)
			{
				it = m_OpenLists.erase(it);
			}else
			{
				AutoConsignmentData& data =it->second;
				if((nowtime > data.nCd) && !CheckInConsignItemByAutoId(data.nId))
				{
					//新加m_consignmentDataList
					AddConsignItemByAuto(data);
				}
				it++;
			}
		}
	}
	if(m_nWaitAutoIns.size())
	{
		auto waitAutos = m_nWaitAutoIns.begin();
		for(; waitAutos != m_nWaitAutoIns.end();)
		{
			auto& cfg = *waitAutos;
			if(cfg.nOpenday > openday)
				break;
			//新加m_OpenLists
			addNewConsignItem(cfg);
			m_nWaitAutoIns.erase(waitAutos++);
		}
	}
}


void CConsignmentMgr::addNewConsignItem(const TaxAutomaticConfig& cfg)
{
	AutoConsignmentData data;
	data.nId = cfg.class1Id;
	data.nEndDay =cfg.nEndday; 
	data.nItemId = cfg.nId;
	data.nNumber = cfg.nNumber;
	data.nPrice = cfg.nPrice;
	m_OpenLists[data.nId] = data;
}



// void CConsignmentMgr::addNewConsignItem(ConsignmentData& dbdata)
// {
// 	ConsignmentProvider& consignmentProvider = GetLogicServer()->GetDataProvider()->GetConsignmentProvider();
// 	auto CfgPtr = consignmentProvider.getPtrAutoInConsignment(dbdata.nAutoId);
// 	int openday = GetLogicServer()->GetDaysSinceOpenServer() ;
// 	if(CfgPtr && dbdata.pData->nState != eConsignmentOnShelf)
// 	{
// 		AutoConsignmentData data;
// 		data.nId = CfgPtr->class1Id;
// 		data.nEndDay =CfgPtr->nEndday; 
// 		data.nItemId = CfgPtr->nId;
// 		data.nNumber = CfgPtr->nNumber;
// 		data.nPrice = CfgPtr->nPrice;
// 		data.nCd = GetGlobalLogicEngine()->getMiniDateTime() + CfgPtr->nSpace;
// 		m_OpenLists[data.nId] = data;
// 	}
// }


bool CConsignmentMgr::CheckInConsignItemByAutoId(int nId)
{
	for (auto& data : m_consignmentDataList)
	{
		if (data.nAutoId == nId && data.nState == eConsignmentOnShelf)
		{
			return true;
		}
	}	
	return false;
}


void CConsignmentMgr::AddConsignItemByAuto(AutoConsignmentData& nData)
{
	if(nData.nItemId != 0 && nData.nNumber != 0)
	{
		
		ConsignmentProvider& consignmentProvider = GetLogicServer()->GetDataProvider()->GetConsignmentProvider();
		auto CfgPtr = consignmentProvider.getPtrAutoInConsignment(nData.nId);
		if(CfgPtr)
		{
			const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(nData.nItemId);
			if(!pStdItem)
				return ;

			ConsignmentData data;
			data.nPrice = nData.nPrice;
			data.nSellTime = time(NULL) + consignmentProvider.nSellTime*3600;
			data.nState = eConsignmentOnShelf;
			CUserItem *pUserItem = GetGlobalLogicEngine()->AllocUserItem(true); //申请一个道具
			if(pUserItem)
			{
				pUserItem->wItemId =(WORD)nData.nItemId ;
				pUserItem->wCount = (WORD)nData.nNumber;
				pUserItem->wPackageType = pStdItem->m_nPackageType;
				memcpy(&data.item, pUserItem, sizeof(data.item));
				GetGlobalLogicEngine()->DestroyUserItem(pUserItem);
			}
			data.nAutoId = nData.nId;
			nData.nCd = GetGlobalLogicEngine()->getMiniDateTime() + CfgPtr->nSpace;
			OutputMsg(rmTip,_T("AddConsignItemByAuto id : %d, cd: %d"),data.nAutoId ,nData.nCd );
			AddConsignItem(data);
		}
		
	}
	return;
}

void CConsignmentMgr::DelOffShelf()
{
	if(m_consignmentDataList.size())
	{
		std::vector<ConsignmentData>::iterator it = m_consignmentDataList.begin();
		for (; it != m_consignmentDataList.end();)
		{
			ConsignmentData& pData = *it;
			if (pData.nState == eConsignmentOffShelf && pData.nAutoId > 0 && pData.nActorId == 0)
			{
				it = m_consignmentDataList.erase(it);	
			}else
			{
				it++;
			}
			
		}
	}
}