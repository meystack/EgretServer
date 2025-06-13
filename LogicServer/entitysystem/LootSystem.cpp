#include "StdAfx.h"
#include "LootSystem.h"

void  CLootSystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet )
{
	if(m_pEntity ==NULL) return; //改组件还没初始化
	unsigned int nHandle = 0;
	switch( nCmd)
	{
	case cLootDropItem:
		{
			BYTE nItemNum = 0;
			packet >> nItemNum;
			std::vector<unsigned int> dropItems;
			for (size_t i = 0; i < nItemNum; i++)
			{
				packet >> nHandle;
				if(LootDropItem(nHandle))
				{
					dropItems.push_back(nHandle);
				}
				else break;
			}
			int nCount = dropItems.size();
			CActorPacket ap;
			CDataPacket& data = ((CActor*)m_pEntity)->AllocPacket(ap);
			data<<(BYTE)GetSystemID()<<(BYTE)sLootDropItem;
			data << (BYTE)nCount;
			if(nCount > 0)
			{
				for (size_t i = 0; i < nCount; i++)
				{
					unsigned int hd = dropItems[i];
					data << hd;
				}
			}
			ap.flush();
			if(nCount > 0)
			{
				CEntityManager* pMgr = GetGlobalLogicEngine()->GetEntityMgr();
				for (size_t i = 0; i < nCount; i++)
				{
					unsigned int id = dropItems[i];
					EntityHandle hd;
					hd.init(enDropItem,id); //这个物品的handle
					pMgr->DestroyEntity(hd); //删除这个实体 
				}
			}
		}
		break;
	case cPetLootDropItem:
		{
			BYTE nItemNum = 0;
			packet >> nItemNum;
			std::vector<unsigned int> dropItems;
			for (size_t i = 0; i < nItemNum; i++)
			{
				packet >> nHandle;
				if(PetLootDropItem(nHandle))
				{
					dropItems.push_back(nHandle);
				}
				else break;
			}
			int nCount = dropItems.size();
			CActorPacket ap;
			CDataPacket& data = ((CActor*)m_pEntity)->AllocPacket(ap);
			data<<(BYTE)GetSystemID()<<(BYTE)sPetLootDropItem;
			data << (BYTE)nCount;
			if(nCount > 0)
			{
				for (size_t i = 0; i < nCount; i++)
				{
					unsigned int hd = dropItems[i];
					data << hd;
				}
			}
			ap.flush();
			if(nCount > 0)
			{
				CEntityManager* pMgr = GetGlobalLogicEngine()->GetEntityMgr();
				for (size_t i = 0; i < nCount; i++)
				{
					unsigned int id = dropItems[i];
					EntityHandle hd;
					hd.init(enDropItem,id); //这个物品的handle
					pMgr->DestroyEntity(hd); //删除这个实体 
				}
			}
		}
		break;
	default:
		break;
		
	}
}

//拾取一个新的箱子
bool CLootSystem::LootDropItem(unsigned int nItemHandle)
{
	EntityHandle hd;
	hd.init(enDropItem,nItemHandle); //这个物品的handle
	int nErrorCode = tpNoError;
	CEntityManager* pMgr = GetGlobalLogicEngine()->GetEntityMgr();
	
	CDropItemEntity *pEntity = (CDropItemEntity *)pMgr->GetEntity(hd);
	if(pEntity ==NULL || pEntity->IsInited() ==false || pEntity->IsDeath() )
	{
		nErrorCode = tmLootPicked;
	}
	else
	{
		nErrorCode = pEntity->GetLootErrorCode(m_pEntity); //获取这个玩家拾取这个物品的错误码
	}
	//这样就无法拾取
	if(nErrorCode)
	{
		m_pEntity->SendTipmsgFormatWithId(nErrorCode,tstUI); //物品不存在
		return false;
	}
	//如果可以拾取的话
	CUserItem *pUserItem = pEntity->GetItem(); //
	if(pUserItem)
	{
		// if (m_pEntity->HasState(esStateStall))
		// {
		// 	// m_pEntity->SendTipmsgFormatWithId(tpInStallLootItem, ttFlyTip);
		// 	return false;
		// }

		CUserBag& bag = m_pEntity->GetBagSystem();
		// if(!bag.bagIsEnough(BagPickUp))
		if(bag.CanAddItem(pUserItem, true)) //如果背包可以添加道具的话
		{
			bag.AddItem(pUserItem,pEntity->m_sLogDesc,GameLog::Log_Loot,true);
			pEntity->SetItem(NULL);//删除这个道具
			// GetLogicServer()->SendItemLocalLog(GameLog::Log_Loot,(CActor*)m_pEntity,pUserItem->wItemId,pUserItem->nType,pUserItem->wCount,1);
		}
		else
		{
			m_pEntity->SendTipmsgFormatWithId(tmDefNoBagNum, tstUI); //背包满了
			return false;
		}
	}

	int nMoneyType = pEntity->GetMoneyType();
	unsigned int nMoneyCount = pEntity->GetMoneyCount();
	if((mtBindCoin == nMoneyType)&&(enActor == m_pEntity->GetType())){
		// if(((CActor*)m_pEntity)->GetBuffSystem()->Exists(aBuffmtBindCoinAddRate)){
			// const float coinRate = ((CActor*)m_pEntity)->m_fBindCoinRate ;
			// nMoneyCount *= coinRate ;
		// }
		if(((CActor*)m_pEntity)->m_fBindCoinRate > 1.0f) {
			const float coinRate = ((CActor*)m_pEntity)->m_fBindCoinRate ;
			nMoneyCount *= coinRate ;
		}
	}
	if (nMoneyType >= mtMoneyTypeStart && nMoneyType < mtMoneyTypeCount && nMoneyCount >0)
	{
		m_pEntity->ChangeMoney(nMoneyType,nMoneyCount,GameLog::Log_Loot,0,pEntity->m_sLogDesc,true, true); //这里爆金钱
		pEntity->SetMoneyCount(0);
	}
	return true;
	// pMgr->DestroyEntity(hd); //删除这个实体 
}


bool CLootSystem::CanLootMoneyOrItem(CTeam * pTeam,CActor * pUser,CDropBag * pBag,unsigned int nBagID)
{
	if(pBag->nMoneyCount >0) return true;
	if(pUser ==NULL || pBag ==NULL) return false;
	unsigned int nActorID = pUser->GetProperty<unsigned int>(PROP_ENTITY_ID);
	INT_PTR nIndex = pTeam->GetActorIndex(nActorID);
	if(nIndex <0 ) return false;

	for(INT_PTR i=0 ;i <MAX_DROP_ITEM_COUNT ; i++)
	{ 
		CDropItem & item = pBag[i].dropItems[i];
		if(item.item.wItemId)
		{
			if(item.btType ==   Item::itQuestItem ) //如果是任务物品
			{
				if( item.GetItemOwnerFlag(nIndex) ) //自己的任务物品
				{
					return true;
				}
			}
			else
			{
				if( item.btLootType == ITEM_OWNER_SPECIAL_ACTOR && item.nActorID != nActorID  )
				{
				}
				else
				{
					return true;
				}
			}
		}
	}
	return false;
}



//拾取一个新的箱子
bool CLootSystem::PetLootDropItem(unsigned int nItemHandle)
{
	//当前场景是否禁止宠物拾取及显示
	CScene *pScene = m_pEntity->GetScene();
	if ( pScene )
	{
		SCENECONFIG* pSceneData = pScene->GetSceneData();
		if ( pSceneData )
		{
			if ( pSceneData->bIsNoPickUp )
			{
				return false;
			}
		}
	}
	

	EntityHandle hd;
	hd.init(enDropItem,nItemHandle); //这个物品的handle
	int nErrorCode = tpNoError;
	CEntityManager* pMgr = GetGlobalLogicEngine()->GetEntityMgr();
	
	CDropItemEntity *pEntity = (CDropItemEntity *)pMgr->GetEntity(hd);
	if(pEntity ==NULL || pEntity->IsInited() ==false || pEntity->IsDeath() )
	{
		nErrorCode = tmLootPicked;
	}
	else
	{
		nErrorCode = pEntity->GetPetLootErrorCode(m_pEntity); //获取这个玩家拾取这个物品的错误码
	}
	//这样就无法拾取
	if(nErrorCode)
	{
		m_pEntity->SendTipmsgFormatWithId(nErrorCode,tstUI); //物品不存在
		return false;
	}
	//如果可以拾取的话
	CUserItem *pUserItem = pEntity->GetItem(); //
	if(pUserItem)
	{
		CUserBag& bag = m_pEntity->GetBagSystem();
		// if(!bag.bagIsEnough(BagPickUp))
		if(bag.CanAddItem(pUserItem, true)) //如果背包可以添加道具的话
		{
			bag.AddItem(pUserItem,pEntity->m_sLogDesc,GameLog::Log_Loot,true);
			pEntity->SetItem(NULL);//删除这个道具
		}
		else
		{
			m_pEntity->SendTipmsgFormatWithId(tmDefNoBagNum, tstUI); //背包满了
			return false;
		}
	}

	int nMoneyType = pEntity->GetMoneyType();
	unsigned int nMoneyCount = pEntity->GetMoneyCount();
	if((mtBindCoin == nMoneyType)&&(enActor == m_pEntity->GetType())){
		if(((CActor*)m_pEntity)->m_fBindCoinRate > 1.0f) {
			const float coinRate = ((CActor*)m_pEntity)->m_fBindCoinRate ;
			nMoneyCount *= coinRate ;
		}
	}
	if (nMoneyType >= mtMoneyTypeStart && nMoneyType < mtMoneyTypeCount && nMoneyCount >0)
	{
		m_pEntity->ChangeMoney(nMoneyType,nMoneyCount,GameLog::Log_Loot,0,pEntity->m_sLogDesc,true, true); //这里爆金钱
		pEntity->SetMoneyCount(0);
	}
	return true;
}