#include "StdAfx.h"
#include "../interface/ItemExportFun.h"
namespace Item
{
	void sendItemProcessResult(void * pEntity,  void* pUserItem,int nProceccType,bool bResult,bool bAutoOperate)
	{
	/*	if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return;

		CUserItem::ItemSeries itemGuid;
		if (pUserItem != NULL)
		{
			itemGuid = ((CUserItem*) pUserItem)->series;
		}
		else
		{
			itemGuid.llId = 0;
		}
		
		CActorPacket pack;
		CDataPacket & data = ((CActor*)pEntity)->AllocPacket(pack);
		data << (BYTE) enBagSystemID << (BYTE) sNotifyItemProcessResult << itemGuid << (BYTE)nProceccType;

		if(bResult)
		{
			data << (BYTE)1;
		}
		else
		{
			data << (BYTE)0;
		}

		if (bAutoOperate)
		{
			data << (BYTE)1;
		} 
		else
		{
			data << (BYTE)0;
		}

		pack.flush();*/
	}

	double getItemGuid(void * pItem)
	{
		CUserItem * pUserItem = (CUserItem *) pItem;
		if(pUserItem ==NULL) return 0;
		double d;
		memcpy(&d,&pUserItem->series.llId,sizeof(d));
		return d;
	}

	char* getItemGuidStr(void *pItem)
	{
		static char strGuid[32];
		CUserItem * pUserItem = (CUserItem *)pItem;
		if(pUserItem == NULL) 
		{
			strGuid[0]= '\0';
		}
		else
		{
			sprintf(strGuid, "%llu", pUserItem->series.llId);
		}

		return strGuid;
	}

	void sendItemProcessConsume(void * pEntity,void * pUserItem,  int nProcessType,int nItemID,int nCount, int nMoneyType,int nMoneyCount, int nProtectItemID, int nProtectCount)
	{
		/*if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return;
		CActorPacket pack;
		CDataPacket & data = ((CActor*)pEntity)->AllocPacket(pack);
		data << (BYTE) enBagSystemID << (BYTE) sNotifyItemProcessConsume ;
		data << ((CUserItem*)pUserItem)->series	;
		data<<	(BYTE )nCount << (BYTE)nMoneyType << (int)nMoneyCount<<(WORD)(nItemID) << (WORD)(nProtectItemID)<< (BYTE)nProtectCount <<(BYTE) nProcessType;
		pack.flush();*/
	}


	bool setItemProperty(void * pEntity,  void* pUserItem,int nPropertyID,int nValue)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		if(pUserItem ==NULL) return false;
		//int nAuxValue;
		//装备的锻造的属性是特殊处理的
		/*
		if(nPropertyID != Item::ipItemForge)
		{
			if(GetItemProperty(pEntity,pUserItem ,nPropertyID,nAuxValue) == nValue) return false;//没有改变
		}
		*/

		return ((CActor *)pEntity)->GetBagSystem().SetItemProperty((CUserItem*)pUserItem,nPropertyID,nValue);
	}

	int getItemProperty(void * pEntity,void * pItem, int propID,int &nAuxRetValue)
	{
		if(propID <0 || propID >=ipItemMaxProperty ) return 0;
		if(pItem ==NULL) return 0;
		nAuxRetValue =0;
		CUserItem * pUserItem = (CUserItem *) pItem;

		if(pUserItem == NULL) return 0;
		const CStdItem * pStdItem;
//		const CStdItem::ItemUseCondition *pCond;
		//PACKEDGAMEATTR attr;
		//INT_PTR nValue;
		//是静态属性
		if(propID >= ipItemStaticPropStart && propID < ipItemMaxProperty)
		{
			return getItemPropertyById(pUserItem->wItemId,propID);
		}
		switch(propID)
		{
		case ipItemID:
			return pUserItem->wItemId;
			break;
		case ipItemCount:
			return pUserItem->wCount;
			break;
		case ipItemStrong:
			return pUserItem->btStrong;
			break;
		case ipItemQuality:
			return pUserItem->btQuality;
			break;
		
		case ipItemBind:
			return pUserItem->btFlag;
			break;
		case ipItemType:
			pStdItem =GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
			if(pStdItem ==NULL) return 0;
			return pStdItem->m_btType;
			break;
		case ipItemHole1IsOpen://宝石槽位1是否开启
			return 0;
			break;

		case ipItemHole2IsOpen: //宝石槽位2是否开启
			return 0;
			break;

		case ipItemHole3IsOpen: //宝石槽位3是否开启
			return 0;
			break;
		case ipItemHole4IsOpen: //宝石槽位3是否开启
			return 0;
			break;

		case ipItemHole1Item://宝石槽位1的物品
			return 0;
			break;
		case ipItemHole2Item://宝石槽位2的物品
			return 0;
			break;
		case ipItemHole3Item://宝石槽位3的物品
			return 0;
			break;
		case ipItemHole4Item://宝石槽位4的物品
			return 0;
			break;
		/*
		case ipItemForgeTimes:
			return pUserItem->btSmithCount;
			break;
		*/
		case ipItemForgeProperty1Value:
			return (int)pUserItem->smithAttrs[0].nValue;
			break;
		case ipItemForgeProperty2Value:
			return (int)pUserItem->smithAttrs[1].nValue;
			break;
		case ipItemForgeProperty3Value:
			return (int)pUserItem->smithAttrs[2].nValue;
			break;
		case ipItemDua:
			return 0;
			break;
		case ipItemDuaMax:
			return 0;
			break;
		//随机精锻
		case ipItemRandForge:			
			return 1;
			break;
		case ipItemForgeProtect:
			return 1;
		case ipItemLuck:
			return (int)pUserItem->btLuck;
		case ipItemSharp:
			return (int)pUserItem->btSharp;
		case ipItemLostStar:
			return pUserItem->bLostStar;
		case ipItemStar:
			return pUserItem->wStar;
		case ipIdentifySlotNum:				//鉴定属性槽被解锁的数量（默认为1）
			if( pUserItem->wIdentifySlotNum <= 0 )
			{
				pUserItem->wIdentifySlotNum = 1;
			}
			return pUserItem->wIdentifySlotNum;
		}
		return 0;
	}

	void * createItem(int nItemID,int nCount,int nStar, int nLostStar, int bInSourceType, int nAreaId, int nBind, int ntime)
	{
		CUserItem* pUserItem = GetLogicServer()->GetLogicEngine()->AllocUserItem(true); //新的物品
		if(pUserItem ==NULL) return NULL;
		

		//通过物品ID查找物品配置对象
		const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(nItemID);
		if ( !pStdItem )
			return NULL;
		pUserItem->wCount = (WORD)nCount; 
		pUserItem->wItemId  = (WORD) nItemID;
		pUserItem->btFlag =(BYTE) nBind;
		pUserItem->btQuality =(BYTE) 0;
		pUserItem->btStrong = (BYTE) 0;
		pUserItem->wStar = nStar;
		pUserItem->bLostStar = nLostStar;
		pUserItem->bInSourceType = bInSourceType;
		pUserItem->nAreaId = nAreaId;
		CStdItem::AssignInstance  (pUserItem,pStdItem) ;
		if (ntime > 0)
		{
			pUserItem->nCreatetime = ntime;
			pUserItem->nCreatetime.startRecord(GetLogicServer()->GetLogicEngine()->getMiniDateTime());
		}
		//产生极品属性
		/*	CUserItemContainer::ItemOPParam ItemPara;
		ItemPara.wItemId = (WORD) nItemID;
		ItemPara.btQuality  =(BYTE) 0;
		ItemPara.wStar = (WORD) nStrong;
		ItemPara.wCount =(WORD)nCount; 
		ItemPara.btBindFlag = (BYTE) nBind;
		ItemPara.nLeftTime = nLeftTime;*/
		//RandAttrSelector::InitSmithById(ItemPara.wItemId, ItemPara.btQuality, nQualityDataIndex, ItemPara.nSmith);

		/*for (int i=0; i < ArrayCount(ItemPara.nSmith); i++)
		{
			pUserItem->smithAttrs[i].nValue = ItemPara.nSmith[i];
		}*/

		return (void *)pUserItem;
	}

	

	int addItem(void *pEntity,void *pItem,char * comment,int nLogWay,int nItemCount,int bNotice)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		CUserItem * pUserItem = (CUserItem * ) pItem;
		if(pUserItem == NULL ) return 0;
		if(nItemCount >0)
		{
			pUserItem->wCount = (WORD) nItemCount;
		}
		return (int)(((CActor *)pEntity)->GetBagSystem().AddItem((CUserItem*)pItem,comment,nLogWay, true, true, bNotice));
	}
	
	int removeItem(void *pEntity,void *pItem,int nCount, char * sComment,int nLogID,bool bNeedFreeMemory)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		if(pItem == NULL || pItem ==NULL) return 0;
		return (int)(((CActor *)pEntity)->GetBagSystem().DeleteItem((CUserItem*)pItem,nCount,sComment,nLogID,bNeedFreeMemory));
	}

	int removeItemDura(void *pEntity,void *pItem,int nCount, char * sComment,int nLogID,bool bNeedFreeMemory)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		if(pItem == NULL || pItem ==NULL) return 0;
		return (int)(((CActor *)pEntity)->GetBagSystem().RemoveItemDura((CUserItem*)pItem,nCount,sComment,nLogID,bNeedFreeMemory));
	}

	int removeOtherItem(void *pEntity,void *pItem,int nCount, char * sComment,int nLogID,bool bNeedFreeMemory, bool bReqSB)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		if(pItem == NULL || pItem ==NULL) return 0;
		return (int)(((CActor *)pEntity)->GetBagSystem().DeleteOtherItem((CUserItem*)pItem,nCount,sComment,nLogID,bNeedFreeMemory,bReqSB));
	}

	int removeItemByType(void* pEntity, int nType, bool bIncEquipBar, char * sComment,int nLogID)
	{
		if (!pEntity || ((CEntity*)pEntity)->GetType() != enActor)
			return 0;

		CActor* pActor = (CActor *)pEntity;
		INT_PTR nRemovedCnt = pActor->GetBagSystem().RemoveItemByType(nType, sComment, nLogID);
		if (bIncEquipBar)
			nRemovedCnt += pActor->GetEquipmentSystem().RemoveItemByType(nType, sComment, nLogID);
		return (int)nRemovedCnt;
	}

	int removeEquip(void* pEntity, void* pItem, const char* sComment, int nLogId, bool bIncHero)
	{
		if (!pEntity || !pItem || ((CEntity*)pEntity)->GetType() != enActor)
			return 0;

		CActor* pActor = (CActor *)pEntity;
		CUserEquipment &equipSys = pActor->GetEquipmentSystem();
		INT_PTR idx = equipSys.FindIndex(((CUserItem *)pItem)->series);	
		int nResult = 0;
		if(idx >=0)	//人物装备
		{
			nResult = equipSys.DeleteEquip((int)idx, sComment, nLogId);
		}
		else
		{
			if (bIncHero)
			{
				//遍历英雄
				CVector<CHeroSystem::HERODATA> &heros = pActor->GetHeroSystem().GetHeroList();
				for (INT_PTR i = 0; i < heros.count(); i++)
				{
					bool boResult = false;
					CHeroEquip *pEquips = &(heros[i].equips);
					CUserItem *pUserItem = pEquips->GetEquipByGuid(((CUserItem *)pItem)->series);
					if (pUserItem)
					{
						if (pEquips->DirectRemoveEquip(pActor,pUserItem,sComment,nLogId,true))
						{
							boResult = true;
							nResult++;
						}
					}
					if (boResult)
					{
						pActor->GetHeroSystem().ResetProperty(i, false,true);
					}
				}
			}
			
		}

		return nResult;

	}

	void* getItemByType(void* pEntity, int nType, bool bIncEquipBar, bool &bInEquipBar)
	{
		if (!pEntity || ((CEntity*)pEntity)->GetType() != enActor)
			return NULL;

		CActor* pActor = (CActor *)pEntity;
		CUserItem* pItem = pActor->GetBagSystem().GetItemByType(nType);
		bInEquipBar = false;
		if (!pItem && bIncEquipBar)
		{
			pItem = pActor->GetEquipmentSystem().GetItemByType(nType);
			if (pItem)
				bInEquipBar = true;
		}

		return pItem;
	}

	const char* getItemLinkMsg( int nItemID,void * pUserItem )
	{
		return CUserBag::GetItemLink(nItemID,(CUserItem*)pUserItem);
	}
	
	const char * getAwardDesc(int nAwardType,int nAwardId, bool useChatLink,void *pUserItem )
	{
		return CActor::GetAwardTypeDesc(nAwardType,nAwardId,useChatLink,(CUserItem*)pUserItem);
	}

	int getItemPropertyById(int nItemID,int nPropID)
	{
		
		const CStdItem *pStdItem =GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(nItemID);
		if(pStdItem ==NULL) return 0;
		 
		int nConditionID =0 ;
		switch(nPropID)
		{
		case ipItemType:
			return pStdItem->m_btType;
			break;
		case ipItemDealMoneyType://物品卖商店的金钱的类型
			return pStdItem->m_btDealType;
			break;
		case ipItemDealMoneyCount://物品卖商店的金钱的数量
			return pStdItem->m_nPrice; 
			break;
		// case ipItemStaticDuaMax:         //物品的最大耐久
		// 	return pStdItem->m_dwDura;
		// 	break;
		case ipItemSuitId:             //套装ID
			return pStdItem->m_wSuitID;
			break;
		case ipItemBreakId:
			return pStdItem->m_nRecoverId;
			break;
		case ipItemForgeMaxTimes:      //精锻的最大次数
			return 1;
			break;
		case ipItemDenyStorage:        //是否禁止放仓库
			return pStdItem->m_Flags.denyStorage?1:0;
			break;
		case ipItemDenySell:          //是否禁止放商店
			return pStdItem->m_Flags.denySell?1:0;
			break;
		// case ipItemInlayable:          //是否能够镶嵌宝石
		// 	return pStdItem->m_Flags.inlayable?1:0;
		// 	break;
		// case ipItemNotConsumeForCircleForge:          //转生锻造时不需要副装备
		// 	return pStdItem->m_Flags.notConsumeForCircleForge?1:0;
		// 	break;
		case ipItemNeedVocation:           //需要的职业，0表示无职业要求
			nConditionID =CStdItem::ItemUseCondition::ucJob;
			break;
			//玩家的精锻的配置id
		// case ipItemSmithId:
		// 	return (int) pStdItem->m_btSmithId;
		// 	break;
		case ipItemShape:
			return (int)pStdItem->m_wShape;
			break;
		case ipItemNeedSex:          //物品需要的性别
			nConditionID =CStdItem::ItemUseCondition::ucGender;
			break;
		case ipItemActorLevel:
			nConditionID =CStdItem::ItemUseCondition::ucLevel;
			break;
		case ipItemMatchAllSuite:
			return pStdItem->m_Flags.matchAllSuit?1:0;
			break;
		// case ipItemSpecialRingType:
		// 	return pStdItem->b_specRing;
		// 	break;
		// case ipItemStrongCount:
		// 	return (int)pStdItem->m_StrongCount;
		// 	break;
		case ipItemActorCircle:
			nConditionID = CStdItem::ItemUseCondition::ucMinCircle;
			break;
		case ipItemDenyGuildDepot:        //是否禁止放行会仓库
			return pStdItem->m_Flags.denyGuildDepot?1:0;
			break;
		// case ipItemCanIdentify:			//是否可以被鉴定
		// 	return pStdItem->m_Flags.bCanIdentify? 1:0;
		// 	break;
		}


		if( nConditionID)
		{
			for (INT_PTR i=pStdItem->m_Conditions.nCount-1; i>-1; --i)
			{
				CStdItem::ItemUseCondition& condit = pStdItem->m_Conditions.pConds[i];
				if(condit.btCond ==nConditionID)
				{
					return condit.nValue;
				}
			}
		}
		return 0;
	}
	
	int getEquipCount(void * pEntity)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		return (int)(((CActor*)pEntity)->GetEquipmentSystem().GetEquipCount());
	}

	
	void* getEquipBySortID(void * pEntity,int nSortID)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return NULL;
		return (void*) (((CActor*)pEntity)->GetEquipmentSystem().GetEquipBySortID(nSortID));
	}

	int getBagItemCount(void * pEntity)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		return (int)(((CActor*)pEntity)->GetBagSystem().count());
	}


	
	void * getBagItemBySortID(void * pEntity,int nSortID)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return NULL;
		if(nSortID <=0 || nSortID > ((CActor*)pEntity)->GetBagSystem().count() )
		{
			return NULL;
		}
		return (void*)((CActor*)pEntity)->GetBagSystem()[nSortID-1];
	}

	void * getBagItemPtrById(void * pEntity,int lItemId, int nQuality , int nStrong)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return NULL;
		return (void*)((CActor*)pEntity)->GetBagSystem().FindItem(lItemId, nQuality, nStrong);
	}

	void * getBagItemPtrByGuid(void * pEntity,double lItemGuid)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return NULL;
		CUserItem::ItemSeries series;
		
		memcpy(&series.llId,&lItemGuid,sizeof(lItemGuid));
		//series.llId = lItemGuid;
		return (void*)((CActor*)pEntity)->GetBagSystem().FindItemByGuid(series);
	
	}

	
	void * getEquipPtrByGuid(void *pEntity, double lItemGuid)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return NULL;
		CUserItem::ItemSeries series;
		//series.llId = lItemGuid;
		memcpy(&series.llId,&lItemGuid,sizeof(lItemGuid));
		return (void*) (((CActor*)pEntity)->GetEquipmentSystem().GetEquipByGuid(series));
	}

	void * getEquipItemPtr(void *pEntity, double lItemGuid, int &nItemPos, int &nHeroId, int nEntityId)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return NULL;
		CUserItem::ItemSeries series;
		//series.llId = lItemGuid;
		memcpy(&series.llId,&lItemGuid,sizeof(lItemGuid));

		return (void*) (((CActor*)pEntity)->GetBagSystem().GetEquipItemPtr(series, nItemPos, nHeroId, nEntityId));
	}

	void releaseItemPtr(void * pItem)
	{
		CUserItem * pUserItem  = (CUserItem*)pItem;
		if(pUserItem ==NULL) return;
		GetGlobalLogicEngine()->DestroyUserItem(pUserItem);
	}

	
	int getBagEmptyGridCount(void *pEntity, int type)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		return (int)(((CActor * )pEntity)->GetBagSystem().availableCount(type));
	}

	int getAllBagMinEmptyGridCount(void *pEntity)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		return (int)(((CActor * )pEntity)->GetBagSystem().availableMinCount());
	}

	bool bagIsEnough(void *pEntity,int nType){
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		return ((CActor * )pEntity)->GetBagSystem().bagIsEnough(nType);
	}


	int getAddItemNeedGridCount(void *pEntity,int nItemID, int nItemCount,int nQuality,
		int nStrong, int nBind, int nParam)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		CUserItemContainer::ItemOPParam param;
		if (nItemID > 0)
		{
			param.wItemId =(WORD) nItemID;
			param.wCount =(WORD) nItemCount;
			param.btQuality =(BYTE) nQuality;
			param.btStrong = (BYTE) nStrong;
			param.btBindFlag =(BYTE) nBind;
		}
		else
		{
			//param = *(CUserItemContainer::ItemOPParam *)nParam;
			return 0;
		}
		return (int)(((CActor * )pEntity)->GetBagSystem().GetAddItemNeedGridCount(param));
	}

	int getItemStaticAttrById(int nItemId, int nAttrType){
		const CStdItem *pStdItem =GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(nItemId);
		if(pStdItem ==NULL) return -1;
		for(INT_PTR i = 0; i < pStdItem->m_StaticAttrs.nCount; ++i){
			GAMEATTR& attr = pStdItem->m_StaticAttrs.pAttrs[i];			
			if (  nAttrType == attr.type){
				return attr.value.nValue;
			}
		}
		return -1;
	}

	int getItemPackageTyp(int nItemId)
	{
		const CStdItem *pStdItem =GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(nItemId);
		if(pStdItem ==NULL) return NULL;
		return pStdItem->m_nPackageType;
	}

	char * getItemName(int nItemId)
	{
		const CStdItem *pStdItem =GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(nItemId);
		if(pStdItem ==NULL) return NULL;
		return (char *)pStdItem->m_sName;
	}
	
	bool addSellItem(void * pEntity, void * itemPtr, int nItemCount)
	{
		if(itemPtr ==NULL || pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		((CUserItem*) itemPtr)->wCount =(WORD) nItemCount;
		return (((CActor * )pEntity)->GetBagSystem().AddSellItem((CUserItem*) itemPtr));
	}

	
	bool delSellItem(void * pEntity, void * pItem)
	{
		if(pItem ==NULL || pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return (((CActor * )pEntity)->GetBagSystem().DelSellItem((CUserItem*) pItem));
	}

	
	
	void *  getSellItem(void * pEntity, double lItemGuid)
	{
		if( pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return NULL;
		unsigned long long guid;
		memcpy(&guid, &lItemGuid,sizeof(guid));
		return (void *)(((CActor * )pEntity)->GetBagSystem().GetSellItem( guid));
	}

	//ZGame不使用
	bool isForgeItem( void* itemPtr )
	{
		return false;
		//return itemPtr && ((CUserItem*)itemPtr)->btSmithCount > 0;
	}

	bool isStrongLevel( void* pEntity,int nLevel )
	{
		if( pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor*)pEntity)->GetBagSystem().IsStrongLevel(nLevel) || ((CActor*)pEntity)->GetEquipmentSystem().IsStrongLevel(nLevel);
	}

	/*ZGame不使用
	bool hasForgeItem( void* pEntity )
	{
		if( pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor*)pEntity)->GetBagSystem().IsForgeItem() || ((CActor*)pEntity)->GetEquipmentSystem().IsForgeItem();
	}
	*/

	void* getDuraFullItemFromBag( void* pEntity,int nItemId )
	{
		if( pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return NULL;
		return ((CActor*)pEntity)->GetBagSystem().GetDuraFullItemById(nItemId);
	}

	int getSuitIdByItemId( int nItemId )
	{
		const CStdItem* pItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(nItemId);
		if (pItem)
		{
			return pItem->m_wSuitID;
		}
		return 0;
	}

	bool isGetAllSuitEquip( void* pEntity,int nSuit,int nCount )
	{
		if( pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		PONESUIT  pSuit = NULL;//GetLogicServer()->GetDataProvider()->GetSuitConfig().GetDataPtr(nSuit);
		if(!pSuit) return false;
		int nResult = 0;
		//检查所有装备是否齐全
		for (INT_PTR i = 0; i < pSuit->itmList.count; i++)
		{
			INT_PTR nItemId = pSuit->itmList.pData[i];
			if (((CActor*)pEntity)->GetEquipmentSystem().FindItemByID(nItemId))
			{
				nResult++;
			}

		}
		return nResult >= nCount;
	}
	bool takeOffItem(void* pEntity, void* pUserItem, int nHeroId, bool checkGrid)
	{
		if (!pEntity || !pUserItem || ((CEntity *)pEntity)->GetType() != enActor) 
			return false;

		CActor* pActor = (CActor *)pEntity;
		CUserItem* pItem = (CUserItem *)pUserItem;
		int location = pItem->btHandPos;
		if(location !=0 && location !=1)
		{
			location =-1;
		}
        return pActor->GetEquipmentSystem().TakeOff(pItem->series,checkGrid);
	}
	bool equipItem(void* pEntity, void* pUserItem, int nHeroId, bool isBagEquip)
	{
		if (!pEntity || !pUserItem || ((CEntity *)pEntity)->GetType() != enActor) 
			return false;

		CActor* pActor = (CActor *)pEntity;
		CUserItem* pItem = (CUserItem *)pUserItem;
		int location = pItem->btHandPos;
		if(location !=0 && location !=1)
		{
			location =-1;
		}
		if (isBagEquip)
		{
			  return pActor->GetEquipmentSystem().TakeOn(pItem->series);
		}
		else
		{
			  return pActor->GetEquipmentSystem().TakeOn(pItem);
		}
	}


	void clearBag(void* pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) 
			return;
		CActor* pActor = (CActor *)pEntity;
		pActor->GetBagSystem().Clear(pActor->GetEntityName(), GameLog::clUserDestroyItem);
	}

	void * getEquipByType( void * pEntity, int nType )
	{
		if (!pEntity || ((CEntity*)pEntity)->GetType() != enActor)
			return NULL;
		return ((CActor *)pEntity)->GetEquipmentSystem().GetItemByType(nType);
	}
	//---不用了
	// bool isDenyRepair(int nItemId)
	// {
	// 	const CStdItem *pStdItem =GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(nItemId);
	// 	if(pStdItem ==NULL) return 0;

	// 	return (bool)(pStdItem->m_Flags.denyRepair);
	// }

	int getBagItemIndex(void* pEntity,void * itemPtr)
	{
		if (!pEntity || !itemPtr || ((CEntity *)pEntity)->GetType() != enActor) 
			return -1;

		CActor* pActor = (CActor *)pEntity;
		CUserItem* pItem = (CUserItem *)itemPtr;
		return pActor->GetBagSystem().GetBagItemIndex(pItem);

	}

	void* getItemPtrByIndex(void* pEntity,int nIndex)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) 
			return NULL;
		CActor* pActor = (CActor *)pEntity;
		return pActor->GetBagSystem().GetItemByIdx(nIndex);
	}

	int getItemId( void * pItem )
	{
		CUserItem * pUserItem = (CUserItem*)pItem;
		if (pUserItem != NULL)
		{
			return pUserItem->wItemId;
		}
		return 0;
	}

	// bool initEquipBaseAttr( void * pEntity, void *pItemPtr, int nQualityDataIndex )
	// {
	// 	if (NULL == pEntity || NULL == pItemPtr || ((CEntity *)pEntity)->GetType() != enActor)
	// 	{
	// 		return false;
	// 	}
	// 	CUserItem * pUserItem = (CUserItem *)pItemPtr;
	// 	if (NULL == pUserItem)
	// 	{
	// 		return false;
	// 	}
	// 	return RandAttrSelector::InitSmithByUserItem(pUserItem, nQualityDataIndex);
	// }

	bool isGuildMeltingItem(void* pItem)
	{
		if (!pItem)
		{
			return false;
		}
		CUserItem* pUserItem = (CUserItem*)pItem;
		if (!pUserItem)
		{
			return false;
		}
		const CStdItem* pStdItem =GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
		if (!pStdItem)
		{
			return false;
		}
		return pStdItem->isMeltingItem();
	}


	

	bool  isEquipment(void *pItemPtr)
	{
		if (!pItemPtr)
			return false;
		CUserItem* pUserItem = (CUserItem*)pItemPtr;
		if (!pUserItem)
		{
			return false;
		}
		return isEquipmentById(pUserItem->wItemId);
	}
	bool isEquipmentById(int wItemId)
	{
		const CStdItem* pStdItem =GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(wItemId);
		if (!pStdItem)
		{
			return false;
		}
		return pStdItem->isEquipment();
	}
	void* getItemPtrHighestStar(void  *pEntity, int  nItemID)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) 
			return NULL;
		CActor* pActor = (CActor *)pEntity;
		return pActor->GetBagSystem().GetItemPtrHighestStar((WORD)nItemID);
	}

	//
	void generateItemAttrsInSmith( void * pEntity,  void *pItemPtr, int nSmithId, int nAttrNum, unsigned char nLockType1, unsigned char nLockType2, 
		unsigned char nLockType3, int nFlag, unsigned char nJob )
	{
		if( !pEntity || ((CEntity *)pEntity)->GetType() != enActor ) 
		{
			return;
		}

		if( !pItemPtr )
		{
			return;
		}
		
		CActor* pActor = (CActor *)pEntity;
		CUserItem* pUserItem = ( CUserItem* )pItemPtr;

		if( nAttrNum <= 0 )
		{
			return;
		}
		if( nAttrNum > CUserItem::MaxSmithAttrCount )
		{
			nAttrNum = CUserItem::MaxSmithAttrCount;
		}

		int nLockType[ CUserItem::MaxSmithAttrCount ];
		nLockType[0] = nLockType1;
		nLockType[1] = nLockType2;
		nLockType[2] = nLockType3;
		nLockType[3] = 0;					//保留
		nLockType[4] = 0;					//保留
		pActor->GenerateItemAttrsInSmith( pUserItem, nSmithId, nAttrNum, nLockType, nFlag, nJob );
	}

	void generateOneAttrInSmith( int nSmithId, unsigned char& nAttrType, int& nAttrValue, unsigned char nLockType )
	{
		RandAttrSelector::GenerateOneAttrInSmith( nSmithId, nAttrType, nAttrValue, nLockType );
	}

	/*
	装备是否存在某种属性
	nFlag：1-鉴定属性
	*/
	bool hasItemAttrs( void * pEntity, void *pItemPtr, int nFlag )
	{
		if( !pEntity || ((CEntity *)pEntity)->GetType() != enActor ) 
		{
			return false;
		}

		if( !pItemPtr )
		{
			return false;
		}

		CActor* pActor = (CActor *)pEntity;
		CUserItem* pUserItem = ( CUserItem* )pItemPtr;
		return pActor->GetBagSystem().HasItemIdentifyAttrs( pUserItem );
	}
	/*
	装备属性转移
	nFlag：1-鉴定属性转移
	*/
	bool transferItemAttrs( void * pEntity, void *pSrcItemPtr, void *pDesItemPtr, int nFlag )
	{
		if( !pEntity || ((CEntity *)pEntity)->GetType() != enActor ) 
		{
			return false;
		}

		if( !pSrcItemPtr )
		{
			return false;
		}

		if( !pDesItemPtr )
		{
			return false;
		}

		CActor* pActor = (CActor *)pEntity;
		CUserItem* pSrcUserItem = ( CUserItem* )pSrcItemPtr;
		CUserItem* pDesUserItem = ( CUserItem* )pDesItemPtr;
		return pActor->TransferItemAttrs( pSrcUserItem, pDesUserItem, nFlag );
	}

	bool drop_item_in_random_area_byGroupID(int sceneId, int nPosX,int nPosY,int dropGroupId,int pick_time, int nDropTips){
		
		CFuBen * pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
		if (pFb == NULL) return false ;
	
		CScene *pScene=	pFb->GetScene(sceneId);
		if (pScene == NULL) return false ;

		if(!CMonster::RealDropItemByDropGroupId(pScene,nPosX,nPosY,dropGroupId,pick_time,nDropTips)) return false ;
		return true ;
	}

}
