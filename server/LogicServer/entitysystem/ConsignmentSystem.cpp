#include "StdAfx.h"
#include "../base/Container.hpp"
using namespace jxInterSrvComm::DbServerProto;

ConsignmentSystem::ConsignmentSystem()
{

}

ConsignmentSystem::~ConsignmentSystem()
{
	
	
}


bool ConsignmentSystem::Initialize( void *data,SIZE_T size )
{
	if (!m_pEntity)
	{
		return false;
	}


	return true;
}
void ConsignmentSystem::OnEnterGame()
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return;
	}
}

VOID ConsignmentSystem::OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader )
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return;
	}
	
}



void ConsignmentSystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet )
{
	
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return;
	}
	switch(nCmd)
	{
	case cConsignmentSystemGetAllConsign:	//获取寄卖物品
		{	
			SendAllConsignItem();
			break;
		}
	case cConsignmentSystemGetMyConsign:	//获取自己寄卖物品
		{
			SendMyConsignItem();
			break;
		}
	case cConsignmentSystemBuyConsign:   //购买物品
		{
			// CUserItem::ItemSeries itemGuid;
			uint64_t nSellIndex = 0;
			packet >> nSellIndex;
			int count = 0;
			unsigned int nBuyPrice = 0;
			packet >> count >> nBuyPrice;
			BuyItem(nSellIndex, count,nBuyPrice);
			break;
			
		}
	case cConsignmentSystemAddConsign: //上架
		{
			AddSellItem(packet);
			break;
		}
	case cConsignmentSystemGetSellItem: //下架
		{
			// CUserItem::ItemSeries itemGuid;
			CancelSellItem(packet);
			break;
		}
	case cConsignmentSystemGetMoney: //领取收益
		{
			GetConsignIncome(packet);//领取寄卖收益
			break;
		}
	case cConsignmentSystemAward: //奖励
		{
			SendConsignIncome();//领取寄卖收益
			break;
		}
	default:
		break;
	}
}


void ConsignmentSystem::SendSearchConsignItem(CDataPacketReader &packet)
{
	// if ( !m_pEntity || m_pEntity->GetType() != enActor )
	// {
	// 	return ;
	// }
	// WORD nPage = 0;
	// BYTE nClass1Id = 0, nClass2Id = 0, nMinLevel =0, nMaxLevel =0, nJob = 0, nIsClickSearch =0;
	// char nCircle = -1,nQuality=-1;
	// char sName[64] = {"\0"};
	// packet >> nPage >> nClass1Id >> nClass2Id >> nMinLevel >> nMaxLevel >> nCircle >> nJob >> nQuality;
	// packet.readString(sName);

	// CConsignmentMgr& mgr = GetGlobalLogicEngine()->GetConsignmentMgr();
	// ConsignmentProvider& provider = GetLogicServer()->GetDataProvider()->GetConsignmentProvider();
	// CVector<ConsignmentData*> List;
	// mgr.SortSearchItem();
	// mgr.FilterSearchItem(List,nClass1Id, nClass2Id, nMinLevel, nMaxLevel, nCircle, nJob, nQuality,sName);//过滤搜索条件
	// INT_PTR nItemCount = List.count();
	// INT_PTR nTotalPage = 0;		//总页数
	// INT_PTR nNeedSendCount =0;  //下发的物品数
	// INT_PTR nStartSendIndex = 0;//开始下发物品的索引
	// if (nItemCount > 0)
	// {
	// 	if (nItemCount % provider.nPageCount == 0)
	// 	{ 
	// 		nTotalPage = nItemCount/provider.nPageCount;
	// 		nNeedSendCount = provider.nPageCount;
	// 	}
	// 	else
	// 	{
	// 		nTotalPage = nItemCount/provider.nPageCount + 1;
	// 		nNeedSendCount = (nPage >= nTotalPage) ? nItemCount % provider.nPageCount :provider.nPageCount;
	// 	}
	// 	if (nPage >= nTotalPage)
	// 	{
	// 		nPage = (WORD)nTotalPage;
	// 	}
	// 	if (nNeedSendCount >= nItemCount)
	// 	{
	// 		nNeedSendCount = nItemCount;
	// 	}
	// 	nStartSendIndex = (nPage-1)*(provider.nPageCount);
	// }
	// CActorPacket AP;
	// CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);
	// DataPacket << (BYTE)enConsignmentSystemID << (BYTE)sConsignmentSystemSearchConsignResult;
	// DataPacket << (WORD)nPage <<(WORD)nTotalPage <<(BYTE)nNeedSendCount;
	
	// for (INT_PTR i = nStartSendIndex; i < nStartSendIndex + nNeedSendCount; i++)
	// {
	// 	ConsignmentData* pData = List[i];
	// 	if (pData && pData->pUserItem)
	// 	{
	// 		DataPacket << *(pData->pUserItem);
	// 		DataPacket << pData->nPrice;
	// 	}
	// }
	// AP.flush();

	// if (nItemCount == 0)//提示没有搜索到相应商品
	// {
	// 	if (!(strcmp(sName,"")== 0 && nClass1Id ==0  && nClass2Id == 0))
	// 	{
	// 		m_pEntity->SendOldTipmsgWithId(tpConsignSearchNothing, ttFlyTip);
	// 	}
	// }	
}

bool ConsignmentSystem::returnConsignProto(CDataPacket & dataPacket, ConsignmentData* pData, uint32_t  nActorId)
{
	if(!pData) return false;
	unsigned int nLeftTime = 0;
	unsigned int nCurr = time(NULL);
	if (pData->nSellTime > nCurr)
	{
		nLeftTime = pData->nSellTime - nCurr;
	}
	if (pData->item.wItemId != 0 &&  pData->item.wCount != 0 && pData->nState == eConsignmentOnShelf && nLeftTime > 0 && pData->nActorId != nActorId)
	{
		(CUserItem)pData->item >> dataPacket;
		dataPacket <<(Uint64)pData->nSellIndex;
		// dataPacket <<(unsigned int )pData->nItemId;
		// dataPacket <<(unsigned int )pData->nCount;
		dataPacket <<(unsigned int )nLeftTime;
		dataPacket <<(unsigned int )pData->nPrice;
		dataPacket <<(BYTE)pData->nState;
		// dataPacket.writeString(pData->nBestAttr);
		// dataPacket <<(WORD)pData->wStar;
		return true;
	}
	return false;
}

void ConsignmentSystem::SendAllConsignItem()
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return ;
	}
	
	ConsignmentProvider& provider = GetLogicServer()->GetDataProvider()->GetConsignmentProvider();
	CConsignmentMgr& mgr = GetGlobalLogicEngine()->GetConsignmentMgr();
	// std::vector<ConsignmentData*> myItemList;
	CActorPacket pack;
	CDataPacket & netPack = m_pEntity->AllocPacket(pack);
	netPack << (BYTE)GetSystemID() << (BYTE)sConsignmentSystemGetAllConsign;
	netPack << (unsigned int )1000;//mgr.GetIncomeValue(m_pEntity->GetId());
	int  nCount = mgr.m_consignmentDataList.size();
	int nPos = netPack.getPosition();
	netPack << nCount;
	
	unsigned int nActorId =  m_pEntity->GetId();
	unsigned int nCurr = time(NULL);//GetGlobalLogicEngine()->getMiniDateTime();
	int nReslut = 0;
	for (INT_PTR i = 0; i < nCount; i++)
	{
		ConsignmentData* pData = &mgr.m_consignmentDataList[i];
		// if(!pData) continue;
		// unsigned int nLeftTime = 0;
		// unsigned int nCurr = time(NULL);
		// if (pData->nSellTime > nCurr)
		// {
		// 	nLeftTime = pData->nSellTime - nCurr;
		// }
		// if (pData->nItemId != 0 &&  pData->nCount != 0 && pData->nState == eConsignmentOnShelf && nLeftTime > 0 && pData->nActorId != nActorId)
		// {
		// 	//netPack <<  *(pData->pUserItem);
		// 	netPack <<(Uint64)pData->nSellIndex;
		// 	netPack <<(unsigned int )pData->nItemId;
		// 	netPack <<(unsigned int )pData->nCount;
		// 	netPack <<(unsigned int )nLeftTime;
		// 	netPack <<(unsigned int )pData->nPrice;
		// 	netPack <<(BYTE)pData->nState;
		// 	nReslut++;
		// }
		if(returnConsignProto(netPack, pData, nActorId))
			nReslut++;
			
	}
	int* pCount = (int*)netPack.getPositionPtr(nPos);
	*pCount = nReslut;
	pack.flush();
}
void ConsignmentSystem::SendMyConsignItem()
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return ;
	}
	
	ConsignmentProvider& provider = GetLogicServer()->GetDataProvider()->GetConsignmentProvider();
	CConsignmentMgr& mgr = GetGlobalLogicEngine()->GetConsignmentMgr();
	std::vector<ConsignmentData*> myItemList;
	mgr.GetMyConsignItem(m_pEntity->GetId(),myItemList);
	CActorPacket pack;
	CDataPacket & netPack = m_pEntity->AllocPacket(pack);
	netPack << (BYTE)GetSystemID() << (BYTE)sConsignmentSystemGetMyConsign;



	int nPos = netPack.getPosition();
	int nCount = (BYTE)myItemList.size();
	netPack << nCount;
	unsigned int nCurr = time(NULL);//GetGlobalLogicEngine()->getMiniDateTime();
	int nReslut = 0;
	for (INT_PTR i = 0; i < nCount; i++)
	{
		ConsignmentData* pData = myItemList[i];
		if(returnConsignProto(netPack, pData, 0))
			nReslut++;
	}
	// netPack << (unsigned int )mgr.GetIncomeValue(m_pEntity->GetId());
	int* pCount = (int*)netPack.getPositionPtr(nPos);
	*pCount = nReslut;
	pack.flush();
}
ConsignmentData* ConsignmentSystem::GetSellItem(uint64_t nIndex)
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return NULL;
	}
	CConsignmentMgr& mgr = GetGlobalLogicEngine()->GetConsignmentMgr();
	std::vector<ConsignmentData*> myItemList;
	mgr.GetMyConsignItem(m_pEntity->GetId(),myItemList);
	for (INT_PTR i =0; i <myItemList.size(); i++)
	{
		ConsignmentData* pData = myItemList[i];
		if (pData && pData->nSellIndex == nIndex)
		{

			return pData;
		}
	}
	return NULL;
}
void ConsignmentSystem::AddSellItem(CDataPacketReader &packet)
{
	
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return ;
	}
	CUserItem::ItemSeries itemGuid;
	unsigned int nPrice = 0;
	packet >> itemGuid.llId;
	packet >> nPrice;

	CActor* pActor = (CActor*)m_pEntity;


	CUserItem* pItem = NULL;
	const CStdItem* pStdItem = NULL;


	ConsignmentProvider& consignmentProvider = GetLogicServer()->GetDataProvider()->GetConsignmentProvider();
	CConsignmentMgr& mgr = GetGlobalLogicEngine()->GetConsignmentMgr();
	int nLevel = pActor->GetProperty<unsigned int>(PROP_CREATURE_LEVEL);
	if (nLevel < consignmentProvider.nOpenLevel)
	{
		pActor->SendTipmsgFormatWithId(tmConsignLevelLimit, tstUI, consignmentProvider.nOpenLevel);
		return ;
	}

	int nCircleLevel = pActor->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE);
	int nCardLv = pActor->GetMiscSystem().GetMaxColorCardLevel() ;
	if (nCircleLevel < consignmentProvider.nConsignLevel[0] && nCardLv < consignmentProvider.nConsignLevel[1])
	{
		pActor->SendTipmsg(consignmentProvider.sDealLimitTips, tstUI);
		return ;
	}

	std::vector<ConsignmentData*> myItemList;
	mgr.GetMyConsignOnSellItem(m_pEntity->GetId(),myItemList);
	int nMMaxSell = consignmentProvider.nMaxShelfCount + ((CActor*)m_pEntity)->GetMiscSystem().GetJiShouHangMaxCount();

	if ((myItemList.size()) >= nMMaxSell)
	{
		pActor->SendTipmsgFormatWithId(tmConsiFull,tstUI);
		return ;
	}

	if (mgr.nCount() >= consignmentProvider.nMaxServerCont)
	{
		pActor->SendTipmsgFormatWithId(tmServerConsiFull,tstUI);
		return ;
	}

	if(mgr.GetMyCanGetConsignItem(m_pEntity->GetId()))
	{
		pActor->SendTipmsgFormatWithId(tmConsigmentCanGet,tstUI);
		return ;
	}

	//背包是否有这个物品，以及数量足够
	CUserBag& bag = m_pEntity->GetBagSystem();
	pItem = bag.FindItemByGuid(itemGuid);
	if (!pItem || pItem->wCount <= 0)
	{
		pActor->SendTipmsgFormatWithId(tmConsignHaveNotItem, tstUI);
		return ;
	}
	//策划要求 可以上架
	// if(strlen(pItem->cBestAttr))
	// {
	// 	pActor->SendTipmsgFormatWithId(tmConsiItemCantSell, tstUI);
	// 	return ;
	// }

	pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pItem->wItemId);
	if (!pStdItem)
	{
		return ;
	}
	if (pStdItem->m_Flags.denySell && strlen(pItem->cBestAttr) == 0)
	{
		pActor->SendTipmsgFormatWithId(tmConsiItemCantSell, tstUI);
		return;
	}
	if (nPrice > consignmentProvider.nMaxSellPrice || nPrice < consignmentProvider.nMinSellPrice)
	{
		pActor->SendTipmsgFormatWithId(tmConsiItemPriveInvalid, tstUI);
		return;
	}
	int  nMoneyType = mtBindCoin;
	unsigned int nMycoin= m_pEntity->GetMoneyCount(mtBindCoin);
	INT_PTR nNeedCion = (INT_PTR)consignmentProvider.nSellCost;
	if (nMycoin < nNeedCion)
	{

		{
			m_pEntity->SendTipmsgFormatWithId(tmNoMoreBindCoin, tstUI);
			return;
		}
	}

	ConsignmentData data;
	
	data.nActorId = pActor->GetId();
	data.nPrice = nPrice;
	data.nSellTime = time(NULL) + consignmentProvider.nSellTime*3600;
	data.nState = eConsignmentOnShelf;
	// data.item = *pStdItem;
	memcpy(&data.item, pItem, sizeof(data.item));
	// data.nItemId = pItem->wItemId;
	// data.nCount = pItem->wCount;
	// data.wStar = pItem->wStar;
	// memcpy(data.nBestAttr, pItem->cBestAttr, sizeof(data.nBestAttr));
	// data.nBestAttr[sizeof(data.nBestAttr)-1] = 0;
	bool bResult = mgr.AddConsignItem(data);
	m_pEntity->ChangeMoney(nMoneyType,-nNeedCion,GameLog::Log_Consigment,
			0,GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetNameById(pItem->wItemId),true);
	if (bResult)
	{
		//删除物品

		bag.DeleteItem(pItem, pItem->wCount, _T("consignment sell"),GameLog::Log_Consigment);
		SendMyConsignItem();
		CActorPacket pack;
		CDataPacket & netPack = m_pEntity->AllocPacket(pack);
		netPack << (BYTE)GetSystemID() << (BYTE)sConsignmentSystemAddConsign;
		netPack<<(BYTE)0;
		pack.flush();
	}

}
void ConsignmentSystem::SendBuyItemResult()
{

	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return ;
	}
	CActorPacket pack;
	CDataPacket & netPack = m_pEntity->AllocPacket(pack);
	netPack << (BYTE)GetSystemID() << (BYTE)sConsignmentSystemBuyConsign;
	netPack << (BYTE) 1;
	pack.flush();
}

void ConsignmentSystem::BuyItem(uint64_t nSellIndex, int nCount, unsigned int nBuyPrice)
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return ;
	}
	int nErrorcode = 0;
	do
	{
		unsigned int nMyActorId = m_pEntity->GetId();
		CConsignmentMgr& mgr = GetGlobalLogicEngine()->GetConsignmentMgr();
		ConsignmentProvider& consignmentProvider = GetLogicServer()->GetDataProvider()->GetConsignmentProvider();
		tagEditionInfo& editionInfo = GetLogicServer()->GetDataProvider()->GetEditionProvider().GetEditionInfo();
		ConsignmentData* pData = mgr.GetConsignItem(nSellIndex);
		int nLevel = m_pEntity->GetProperty<unsigned int>(PROP_CREATURE_LEVEL);
		if (nLevel < consignmentProvider.nOpenLevel)
		{
			m_pEntity->SendTipmsgFormatWithId(tmConsignLevelLimit, tstUI,consignmentProvider.nOpenLevel);
			return ;
		}

		if (!pData || pData->item.wItemId == 0)
		{
			SendBuyItemResult();
			m_pEntity->SendTipmsgFormatWithId(tmConsignItemNotItem, tstUI);
			return ;
		}
		if (nCount != pData->item.wCount)
		{
			SendBuyItemResult();
			m_pEntity->SendTipmsgFormatWithId(tmConsignItemNotItem, tstUI);
			return ;
		}
		if (nBuyPrice != pData->nPrice)
		{
			SendBuyItemResult();
			m_pEntity->SendTipmsgFormatWithId(tmConsignBuyPriceError, tstUI);
			return; 
		}
		if (nMyActorId == pData->nActorId)
		{
			SendBuyItemResult();
			m_pEntity->SendTipmsgFormatWithId(tmConsignCantBuyMySelf, tstUI);
			return; 
		}
		if (pData->nState != eConsignmentOnShelf)
		{
			//已过期的物品
			SendBuyItemResult();
			m_pEntity->SendTipmsgFormatWithId(tmConsignItemOffShelf, tstUI);
			return ;
		}
		// if (pData->nState == eConsigmentActorBuy)
		// {
		// 	//已过期的物品
		// 	SendBuyItemResult();
		// 	m_pEntity->SendTipmsgFormatWithId(tmConsignItemActorBuy, tstUI);
		// 	return ;
		// }
		if (pData->item.wCount < nCount)
		{
			//重发剩余数量
			SendBuyItemResult();
			m_pEntity->SendTipmsgFormatWithId(tmConsignBuyItemCountexceed, tstUI);
			return;
		}
		unsigned int nMyYb = m_pEntity->GetMoneyCount(mtYuanbao);
		
		INT_PTR nNeedYb = (INT_PTR)pData->nPrice;
		if (nMyYb < nNeedYb)
		{
			m_pEntity->SendTipmsgFormatWithId(tmConsiNoYuanbaoToBuy, tstUI);
			return;
		}
		//交易额度
		if(editionInfo.nStart)
		{
			int32_t  nQuota =  m_pEntity->GetMoneyCount(mtJyQuota);
			if(nQuota < (int)(nNeedYb*(editionInfo.nProportion/100.0)) )
			{
				m_pEntity->SendTipmsgFormatWithId(tmQuotaNoEnough, tstUI);
				return;
			}
		}
		
		//扣钱	
		m_pEntity->ChangeMoney(mtYuanbao,-nNeedYb,GameLog::Log_Consigment,
			0,GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetNameById(pData->item.wItemId),true);
		if(editionInfo.nStart && ((CActor*)m_pEntity)->GetMiscSystem().GetMaxColorCardLevel() < editionInfo.nPrivilege)
		{
			m_pEntity->ChangeMoney(mtJyQuota,(int)(nNeedYb*(editionInfo.nProportion/100.0))*-1,GameLog::Log_Consigment,
			0,GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetNameById(pData->item.wItemId),true);
		}
		//  m_pEntity->SetProperty<unsigned int >(PROP_ACTOR_TRADINGQUOTA, nQuota-nNeedYb);

		if (pData->item.wCount == nCount)//全部购买
		{

			pData->nState = eConsigmentIsSell;
			CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(pData->nActorId);
			if (pActor)
			{
				pActor->GetConsignmentSystem().SenDClientRedPoint(false);
			}
			mgr.DealBuyConsignItem(pData,m_pEntity->GetId());
		}

		// 税
		int nTax = pData->nPrice*consignmentProvider.fTaxRate/100;
		if(nTax <= 0)
			nTax = 1;

		// 金额流入
		GetGlobalLogicEngine()->AddDealToLog(GameLog::Log_Consigment,
			pData->nActorId,  m_pEntity->GetId(), 
			0,0, mtYuanbao, nNeedYb, nTax,((CActor*)m_pEntity)->getOldSrvId());

		// 物品流入
		GetGlobalLogicEngine()->AddDealToLog(GameLog::Log_Consigment,
			m_pEntity->GetId(), pData->nActorId, 
			pData->item.wItemId,pData->item.wCount,0,0,0,((CActor*)m_pEntity)->getOldSrvId());
			
		// if (GetLogicServer()->GetLocalClient())//日志
		// {
		// 	char sBuff[256];
		// 	char sTarget[64];
		// 	char sSeries[64]; 
		// 	sprintf(sSeries,"%lld",uLoglld); //序列号
		// 	sprintf(sTarget,"%u",nLogActorId);
		// 	sprintf(sBuff,"price:%d,buyCount:%d",nLogPrice,nCount);
		// }
		// CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(pData->nActorId);
		// if (pActor)
		// {
		// 	m_pEntity->GetConsignmentSystem().SendAllConsignItem();
		// }
		SendAllConsignItem();
		m_pEntity->SendTipmsgFormatWithId(tmConsiBuySuccAddToBag, tstUI);
		SenDClientRedPoint(true);
		SendConsignIncome();
	} while (false);
	
}


void ConsignmentSystem::ShoutingItem(CUserItem::ItemSeries itemGuid)
{
	// if ( !m_pEntity || m_pEntity->GetType() != enActor )
	// {
	// 	return ;
	// }
	// CConsignmentMgr& mgr = GetGlobalLogicEngine()->GetConsignmentMgr();
	// ConsignmentProvider& provider = GetLogicServer()->GetDataProvider()->GetConsignmentProvider();
	// ConsignmentData* pData = GetSellItem(itemGuid);
	// if (!pData)
	// {
	// 	m_pEntity->SendOldTipmsgWithId(tpConsignItemNotItem, ttFlyTip);
	// 	return ;
	// }
	
	// unsigned int nCurrTime = GetGlobalLogicEngine()->getMiniDateTime();
	
	// if (pData->nState == eConsignmentOffShelf)
	// {
	// 	//已过期的物品
	// 	m_pEntity->SendOldTipmsgWithId(tpConsignItemOffShelf, ttFlyTip);
	// 	return ;
	// }
	// unsigned int nMyMoney = m_pEntity->GetMoneyCount(mtBindCoin);
	// if (nMyMoney < (unsigned int)provider.nShoutMoney)
	// {
	// 	m_pEntity->SendMoneyNotEnough(mtBindCoin, provider.nShoutMoney);
	// 	return;
	// }
	// const char* pConsignItemLinkg = CUserBag::GetConsignItemLink(pData->pUserItem, pData->nPrice);
	// if (!pConsignItemLinkg)
	// {
	// 	return;
	// }
	// //扣钱	
	// m_pEntity->ChangeMoney(mtBindCoin,-provider.nShoutMoney,GameLog::clConsignmentShouting,
	// 	0,GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetNameById(pData->pUserItem->wItemId),true);
	
	// char buff[1024];
	// LPCTSTR sTipFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpConsignShoutMsg);
	// sprintf(buff,sTipFormat,pData->nPrice, pConsignItemLinkg, pData->pUserItem->wCount);
	// GetGlobalLogicEngine()->GetEntityMgr()->BroadcastTipmsg(buff,ttChatWindow,provider.nOpenLevel);
	//成功之后处理
}
void ConsignmentSystem::CancelSellItem(CDataPacketReader& packet)
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return ;
	}
	BYTE nType = 0;
	packet >> nType;
	std::vector<ConsignmentData*> mSellList;
	mSellList.clear();
	CConsignmentMgr& mgr = GetGlobalLogicEngine()->GetConsignmentMgr();
	if(nType == 1)
	{
		uint64_t nSellIndex = 0;
		packet >> nSellIndex;
		ConsignmentData* pData = GetSellItem(nSellIndex);
		if (!pData)
		{
			m_pEntity->SendTipmsgFormatWithId(tmConsignItemNotItem, tstUI);
			return ;
		}
		if ( pData->nActorId != m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID))
		{
			m_pEntity->SendTipmsgFormatWithId(tmConsignCantGetNotYou, tstUI);
			return ;
		}

		if (pData->nState == eConsigmentIsSell)
		{
			//已过期的物品
			m_pEntity->SendTipmsgFormatWithId(tmItemIsSellOrTimeOut, ttFlyTip);
			return ;
		}
		mSellList.push_back(pData);
	}
	else
	{
		// CConsignmentMgr& mgr = GetGlobalLogicEngine()->GetConsignmentMgr();
		mgr.GetMyConsignItem(m_pEntity->GetId(),mSellList);
	}
	int nSize = mSellList.size();
	if(nSize > 0)
	{
		for(int i = 0; i < nSize; i++)
		{
			ConsignmentData* pData = mSellList[i];
			if (!pData)
			{
				continue;
			}
			if ( pData->nActorId != m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID))
			{
				continue;
			}

			if (pData->nState != eConsignmentOnShelf)
			{
				continue;
			}

			//判断背包是否满，可以则加到背包中，否则提示
			// CUserBag& bag = m_pEntity->GetBagSystem();
			// if (bag.availableMinCount() <= 0)
			// {
			// 	m_pEntity->SendTipmsgFormatWithId(tmConsiBagFullCannotOp, tstUI);
			// 	return ;
			// }
			// m_pEntity->GiveAward(0, pData->nItemId, pData->nCount);
			//bag.AddItem(pData->pUserItem,_T("consign cancel"),GameLog::clConsignmentGetItemBack);
			// mgr.DelConsignItem(pData);
			pData->nState = eConsignmentOffShelf;
			pData->nSellTime = 0;
			mgr.SetActorDataModify(pData->nActorId);
			// SendMyConsignItem();
		}
		SendMyConsignItem();

		CActorPacket pack;
		CDataPacket & netPack = m_pEntity->AllocPacket(pack);
		netPack << (BYTE)GetSystemID() << (BYTE)sConsignmentSystemGetSellItem;
		netPack<<(BYTE)0;
		pack.flush();
		SenDClientRedPoint(true);
	}
}
//领取寄卖收益
void ConsignmentSystem::GetConsignIncome(CDataPacketReader& packet)
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return ;
	}
	// int x,y;
	// m_pEntity->GetPosition(x,y);
	// CScene * pScene = m_pEntity->GetScene();
	// if (pScene == NULL)
	// {
	// 	return;
	// }
	// if(!pScene->HasMapAttribute(x,y,aaSaft))
	// {
	// 	m_pEntity->SendTipmsgFormatWithId(tmNoSafeAreaNoGet, tstUI);
	// 	return;
	// }

	CConsignmentMgr& mgr = GetGlobalLogicEngine()->GetConsignmentMgr();
	BYTE nType = 0;
	packet >> nType;
	std::vector<ConsignmentData*> actorConsignItemList;
	actorConsignItemList.clear();
	if(nType == 1)
	{
		uint64_t nSellIndex = 0;
		packet >> nSellIndex;
		ConsignmentData* pData = GetSellItem(nSellIndex);
		if (!pData)
		{
			m_pEntity->SendTipmsgFormatWithId(tmConsignItemNotItem, tstUI);
			return;
		}
		if(pData->nState == eConsignmentOnShelf)
		{
			m_pEntity->SendTipmsgFormatWithId(tmConsignItemOnSell, tstUI);
			return;
		}
		actorConsignItemList.push_back(pData);
	}
	else
	{
		// std::vector<ConsignmentData*> actorConsignItemList;
		mgr.GetMyConsignItem(m_pEntity->GetId(), actorConsignItemList);
		
	}
	
	int nSize = actorConsignItemList.size();
	if(nSize <= 0)
		return;

	int nNeedBagNum = 0;
	for(int k = 0; k < nSize; k++)
	{
		ConsignmentData* pData = actorConsignItemList[k];
		if (!pData) continue;

		if(pData->nState == eConsignmentOffShelf || pData->nState == eConsigmentActorBuy)
			nNeedBagNum++;
	}
	//只会存在单次的
	if(!m_pEntity->GetBagSystem().bagIsEnough(BagDeal))
	{
		m_pEntity->SendTipmsgFormatWithId(tmDefNoBagNum, tstUI);
		return;
	}
	ConsignmentProvider& consignmentProvider = GetLogicServer()->GetDataProvider()->GetConsignmentProvider();
	for(int i = nSize-1; i > -1; i--)
	{
		ConsignmentData* pData = actorConsignItemList[i];
		if (!pData) continue;

		if(pData->nState == eConsignmentOnShelf)
		{
			m_pEntity->SendTipmsgFormatWithId(tmConsignItemOnSell, tstUI);
			return;
		}

		if(pData->nState == eConsignmentOffShelf || pData->nState == eConsigmentActorBuy)
		{
			CUserItemContainer::ItemOPParam iParam;
			iParam.wItemId =  pData->item.wItemId;
			iParam.wCount = pData->item.wCount;
			iParam.btQuality = pData->item.btQuality;
			iParam.btStrong = pData->item.btStrong;
			iParam.btBindFlag = pData->item.btFlag;
			iParam.wStar = pData->item.wStar;
			iParam.setSource(pData->item.bInSourceType, pData->item.nCreatetime,pData->item.nAreaId,pData->item.nDropMonsterId,pData->item.cSourceName);
			if(strlen(pData->item.cBestAttr) > 0)
			{
				memcpy(iParam.cBestAttr, pData->item.cBestAttr,sizeof(iParam.cBestAttr));
				iParam.cBestAttr[sizeof(iParam.cBestAttr)-1]= '\0';	
			}
			m_pEntity->GiveAward(0, pData->item.wItemId,  pData->item.wCount,0,0,0,0,GameLog::Log_Consigment,"",0, &iParam);
			// m_pEntity->GiveAward(0, pData->nItemId, pData->nCount, ,iParam);
		}
		else if(pData->nState == eConsigmentIsSell)
		{
			int nTax = pData->nPrice*consignmentProvider.fTaxRate/100;
			if(nTax <= 0)
				nTax = 1;
			if (m_pEntity->ChangeMoney(mtYuanbao, pData->nPrice-nTax,GameLog::Log_Consigment, 0,"consign getIncome"))
			{
				m_pEntity->SendTipmsgFormatWithId(tmYbTaxRateGet, tstUI);
			}
		}
		mgr.DelConsignItem(pData);
	}
	SendConsignIncome();
	SenDClientRedPoint(false);
}


void ConsignmentSystem::SendConsignIncome()
{
	if(!m_pEntity) return;
	CConsignmentMgr& mgr = GetGlobalLogicEngine()->GetConsignmentMgr();
	unsigned int nActorId = m_pEntity->GetId();

	std::vector<ConsignmentData*> actorConsignItemList;
	mgr.GetMyConsignItem(m_pEntity->GetId(), actorConsignItemList);
	int nSize = actorConsignItemList.size();

	CActorPacket pack;
	CDataPacket & netPack = m_pEntity->AllocPacket(pack);
	netPack << (BYTE)GetSystemID() << (BYTE)sConsignmentSystemAward;
	int nPos = netPack.getPosition();
	netPack << nSize;
	int nReslut = 0;
	for(int i = 0; i < nSize; i++)
	{
		ConsignmentData* pData = actorConsignItemList[i];
		if(!pData) continue;
		if (pData->item.wItemId != 0 && pData->item.wCount != 0 && (pData->nState == eConsignmentOffShelf ||
		pData->nState == eConsigmentIsSell || 
		pData->nState == eConsigmentActorBuy) && pData->nActorId == nActorId)
		{
			(CUserItem)pData->item >> netPack;
			netPack <<(Uint64)pData->nSellIndex;
			// netPack <<(unsigned int )pData->nItemId;
			// netPack <<(unsigned int )pData->nCount;
			netPack <<(unsigned int )pData->nSellTime ;
			netPack <<(unsigned int )pData->nPrice;
			netPack <<(BYTE)pData->nState;
			nReslut++;
			// netPack.writeString(pData->nBestAttr);
			// netPack <<(WORD)pData->wStar;
		}
	}
	int *pCount = (int*)netPack.getPositionPtr(nPos);
	*pCount = nReslut;
	pack.flush();
}



//红点提示
void ConsignmentSystem::SenDClientRedPoint(bool bRedPoint)
{
	if(!m_pEntity) return;
	CConsignmentMgr& mgr = GetGlobalLogicEngine()->GetConsignmentMgr();
	unsigned int nActorId = m_pEntity->GetId();
	if(!bRedPoint)
	{
		bRedPoint = mgr.GetMyCanGetConsignItem(m_pEntity->GetId());
	}

	CActorPacket pack;
	CDataPacket & netPack = ((CActor*)m_pEntity)->AllocPacket(pack);
	BYTE redPoint = bRedPoint != false? 1:0;
	netPack << (BYTE)enConsignmentSystemID << (BYTE)sConsignmentSystemRedPoint << redPoint;
	pack.flush();
}