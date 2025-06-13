#include "StdAfx.h"
#include "BasicFunctionsSystem.h"


BasicFunctionsSystem::BasicFunctionsSystem()
{
	
}

BasicFunctionsSystem::~BasicFunctionsSystem()
{
}

bool BasicFunctionsSystem::Initialize(void *data, SIZE_T size)
{
	PACTORDBDATA pActorData = (ACTORDBDATA *)data;
	if (pActorData == NULL || size != sizeof(ACTORDBDATA))
	{
		OutputMsg(rmError, _T("data len error len=%d ,correct len=%d"), size, sizeof(ACTORDBDATA));
		return false;
	}
	return true;
}

VOID BasicFunctionsSystem::ProcessNetData(INT_PTR nCmd, CDataPacketReader &packet)
{

	if (m_pEntity == NULL)
		return;
	if (m_pEntity->GetHandle().GetType() != enActor)
		return;
	switch (nCmd)
	{
	case encompostItem: //合成
	{
		HandleComposeItem(packet);
		break;
	}
	case enForgeItem: //锻造
	{
		HandleForgeItem(packet);
		break;
	}
	case enForgeItemId: //id本服锻造id
	{
		HandleForgeInfo(packet);
		break;
	}
	case enRefining://洗炼
	{
		HandleRefining(packet);
		break;
	}
	case enRefiningReplace://
	{
		HandleRefiningReplace(packet);
		break;
	}
	case enItemUpStar:
	{
		UpdateItemStar(packet);
		break;
	}
	case enYBRecoverInfo:
	{
		SendRecoverItemInfo();
		break;
	}

	default:
		break;
	}
}

void BasicFunctionsSystem::HandleForgeItem(CDataPacketReader& inPakcet)
{
	if(!m_pEntity) return;
	CActor* pActor = (CActor*)m_pEntity;
	BYTE nCount = 0;
	inPakcet >>nCount;
	if(nCount != 1 && nCount != 10)
	{
		return;
	}
	CStdItemProvider& stdItemPv = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	static int itemId = stdItemPv.nForgeItemId;
	int nCost = nCount > 1?stdItemPv.nTenForgeCost:stdItemPv.nOnceForgeCost;
    CUserBag& bag = m_pEntity->GetBagSystem();


	int nType = nCount > 1? BagTenForge : BagForge;
	int tipId = nCount > 1? tmTenForgeNoBagNum : tmForgeNoBagNum;
	if(!bag.bagIsEnough(nType) )
	{
		pActor->SendTipmsgFormatWithId(tmTenForgeNoBagNum,tstUI);
		return;
	}
	if(bag.GetItemCount(itemId) < nCost)
	{
		pActor->SendTipmsgFormatWithId(tmItemNotEnough,tstUI);
        return;
	}
	int nTopLevel = GetGlobalLogicEngine()->GetTopTitleMgr().GetTopMaxLevel();
	int nDropId = stdItemPv.GetForgeDropIdByLevel(nTopLevel);

    std::vector<DROPINFO> nResults;
    nResults.clear();
	//扣道具
	do
	{
		CUserItem *pUserItem = bag.FindItem(itemId);
		if(pUserItem)
		{
			int cost = nCost < pUserItem->wCount? nCost:pUserItem->wCount;
			nCost -= cost;
			if(nCost < 0)
			{
				nCost = 0;
			}
			if(cost > 0)
				bag.DeleteItem(pUserItem, cost,"self_compose_item",GameLog::Log_ForgeItem,true);
		}
	}while(nCost > 0);

	for(int i = 0; i < nCount; i++)
	{
		std::vector<DROPGOODS> dropInfos;
		GetLogicServer()->GetDataProvider()->GetDropCfg().GetGiftDropInfoByDropGroupId(nDropId, dropInfos);
		int count  = dropInfos.size();
		if(count)
		{
			for(int j = 0; j < count; j++)
			{
				//printf("error forge item count %d\n", dropInfos[j].info.nCount);
				const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(dropInfos[j].info.nId);
				if(!pStdItem)
					continue;

				CUserItemContainer::ItemOPParam iParam;
				iParam.wItemId = dropInfos[j].info.nId;
				iParam.wCount = (WORD)dropInfos[j].info.nCount;
				iParam.btQuality = pStdItem->b_showQuality;
				iParam.btStrong = 0;
				iParam.btBindFlag = 0;
				int num = dropInfos[j].v_bestAttr.size();

				for(int k = 0; k < num; k++)
				{
					char buf[10];
					if(k != 0)
						strcat(iParam.cBestAttr, "|");

					DropAttribute data = dropInfos[j].v_bestAttr[k];
					sprintf(buf, "%d,%d", data.nType, data.nValue);
					strcat(iParam.cBestAttr, buf);
				}
				iParam.cBestAttr[sizeof(iParam.cBestAttr)-1]= '\0';

				pActor->GiveAward(dropInfos[j].info.nType, dropInfos[j].info.nId, dropInfos[j].info.nCount,0,0,0,0,GameLog::Log_ForgeItem,"",0, &iParam);
				nResults.push_back(dropInfos[j].info);
				//祖玛级以上的广播
				if (pActor && pStdItem->m_nItemlvl > 2) {
					GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmActorForgeItem, tstChatSystem, pActor->GetEntityName(),pStdItem->m_sName);
					GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmActorForgeItem, tstRevolving, pActor->GetEntityName(), pStdItem->m_sName);
				}
			}
		}	
	}
	int nSize = nResults.size();
	pActor->GetAchieveSystem().ActorAchievementComplete(nAchieveForge, nCount);
    CActorPacket ap;
	CDataPacket& dataPacket = pActor->AllocPacket(ap);
    dataPacket << (BYTE)enBasicFunctionsSystemID << (BYTE)enForgeResult;
    dataPacket << (BYTE)nSize;
    for(int k = 0; k < nSize; k++)
    {
        dataPacket<<(BYTE)(nResults[k].nType);
        dataPacket<<(int)(nResults[k].nId);
        dataPacket<<(int)(nResults[k].nCount);
    }
	ap.flush();
}

void BasicFunctionsSystem::HandleComposeItem(CDataPacketReader& inPacket)
{

	if(!m_pEntity) return;
	CActor* pActor = (CActor*)m_pEntity;
	WORD nId = 0;
	BYTE nTimes = 0;
	inPacket >> nId >> nTimes;

	if ( nTimes != 1 && nTimes != 10 )
	{
		return;
	}

	CStdItemProvider& stdItemPv = GetLogicServer()->GetDataProvider()->GetStdItemProvider();

	ItemComposeCfg* pComposeCfg = stdItemPv.GetComposeCfgById(nId);
	if(!pComposeCfg)
	{
		return;
	}

	if(!pActor->CheckGhostLevel(pComposeCfg->nGhostLevelLimit))
	{
		pActor->SendTipmsgFormatWithId(tmComPoseLimit, tstUI);
		return;
	}
	if(!pActor->CheckLevel(pComposeCfg->nLevelLimt, pComposeCfg->nCircle))
	{
		pActor->SendTipmsgFormatWithId(tmComPoseLimit, tstUI);
		return;
	}
	if(!pActor->CheckOffice(pComposeCfg->nOfficeId))
	{
		auto Cfgptr = GetLogicServer()->GetDataProvider()->GetMiscConfig().GetOfficeCfgPtrByLevel(pComposeCfg->nOfficeId);
		if(Cfgptr)
			pActor->SendTipmsgFormatWithId(tmComPoseOfficeLimit, tstUI,Cfgptr->sName);
		return;
	}
	if(pActor->GetMiscSystem().GetMaxColorCardLevel() < pComposeCfg->nVipLimit)
	{
		pActor->SendTipmsgFormatWithId(tmComPoseVipLimit, tstUI);
		return;
	}

	int nDays = GetLogicServer()->GetDaysSinceOpenServer();
	if(nDays < pComposeCfg->nOpenServerDay)
	{
		pActor->SendTipmsgFormatWithId(tmCircleOpenServerDayLimit, tstUI, nDays);
		return;
	}

	if(pComposeCfg->nArea)
	{
		if (!m_pEntity->HasMapAttribute(pComposeCfg->nArea))
		{
			m_pEntity->SendTipmsgFormatWithId(tmNotInSafeArea, tstUI);
			return;
		}
	}
	
    CUserBag& bag = m_pEntity->GetBagSystem();
	int count = pComposeCfg->table.size();
	for(int i = 0; i < count; i++)
	{
		ComposeTableCfg& table = pComposeCfg->table[i];
		// CUserItem *pUserItem = bag.FindItem(table.nId, -1, -1, table.nCount);
		// if(pUserItem ==NULL)
		// {
		// 	pActor->SendTipmsgFormatWithId(tmItemNotExist,tstUI);
		// 	return;
		// }
		bool bResult = pActor->CheckActorSource(table.nType, table.nId, table.nCount * nTimes, tstUI);
		if(!bResult)
		{
			// pActor->SendTipmsgFormatWithId(tmItemNotEnough,tstUI);
			return;
		}
		
	}

	if(!bag.bagIsEnough(nTimes == 1 ? BagCompose:BagComposes) )
	{
		pActor->SendTipmsgFormatWithId(tmComposeBagLimit,tstUI);
		return;
	}
	if(pComposeCfg->composeItem.nType == qatEquipment)
	{
		CUserItemContainer::ItemOPParam ItemOp;
		ItemOp.wItemId = (WORD)pComposeCfg->composeItem.nId;
		ItemOp.wCount = (WORD)pComposeCfg->composeItem.nCount * nTimes;
		if(!pActor->GetBagSystem().CanAddItem(ItemOp))
		{
			pActor->SendTipmsgFormatWithId(tmLeftBagNumNotEnough,tstUI);
			return;
		}
	}
	
	
	for(int i = 0; i < count; i++)
	{
		ComposeTableCfg& table = pComposeCfg->table[i];
		// if(table.nType == 0)
		// {
		// 	CUserItem *pUserItem = bag.FindItem(table.nId, -1, -1, table.nCount);
		// 	if(pUserItem ==NULL)
		// 	{
		// 		pActor->SendTipmsgFormatWithId(tmItemNotExist,tstUI);
		// 		return;
		// 	}
		// 	bag.DeleteItem(pUserItem,table.nCount,"self_compose_item",GameLog::clComPoseItem,true);
		// }
		// else
		// {
		// 	pActor->GiveAward(table.nType,table.nId, -(table.nCount));
		// }
		char logs[64] = {0};
		if(table.nType == 0 ) {
			const CStdItem* pStdItem = stdItemPv.GetStdItem(table.nId);
			if(pStdItem) {
				sprintf_s(logs,64,"%s",pStdItem->m_sName);
				logs[63] = 0;
			}
		}else
		{
			const SourceConfig* pConfig = stdItemPv.GetNumericalConfig(table.nId);
			if(pConfig) {
				sprintf_s(logs,64,"%s",pConfig->name);
				logs[63] = 0;
			}
		}
		pActor->RemoveConsume(table.nType, table.nId, table.nCount * nTimes, -1, -1,-1,0, GameLog::Log_Compose,logs);
	}

	// int nCount = pComposeCfg->composeItem.size();
	// for(int i = 0; i < nCount; i++)
	// {
	// 	ComposeTableCfg& table = pComposeCfg->composeItem[i];
	// 	CUserItem *pUserItem = bag.FindItem( pComposeCfg->composeItem.nId, -1, -1,  pComposeCfg->composeItem.nCount);
	// 	if(pUserItem ==NULL)
	// 	{
	// 		pActor->SendTipmsgFormatWithId(tmItemNotExist,tstUI);
	// 		return;
	// 	}
	// 	pActor->GiveAward( pComposeCfg->composeItem.nType,  pComposeCfg->composeItem.nId,  pComposeCfg->composeItem.nCount);
	// }
	// CUserItem *pUserItem = bag.FindItem( pComposeCfg->composeItem.nId, -1, -1,  pComposeCfg->composeItem.nCount);
	// if(pUserItem ==NULL)
	// {
	// 	pActor->SendTipmsgFormatWithId(tmItemNotExist,tstUI);
	// 	return;
	// }

	((CActor*)m_pEntity)->GetQuestSystem()->OnQuestEvent(CQuestData::qtCompose, 1 * nTimes, nId);
	pActor->GiveAward( pComposeCfg->composeItem.nType,  pComposeCfg->composeItem.nId,  pComposeCfg->composeItem.nCount * nTimes,0,0,0,0,GameLog::Log_Compose);
	CActorPacket ap;
	CDataPacket& data = ((CActor*)m_pEntity)->AllocPacket(ap);
	data<<(BYTE)GetSystemID() << (BYTE)enCompostResult;
	ap.flush();
	return;
}

void BasicFunctionsSystem::HandleForgeInfo(CDataPacketReader& inPacket)
{
	if(!m_pEntity) return;
	CActor* pActor = (CActor*)m_pEntity;
	CActorPacket ap;
	CDataPacket& data = pActor->AllocPacket(ap);
	data <<(BYTE)enBasicFunctionsSystemID <<(BYTE)enForgeItemIdResult ;
	CStdItemProvider& stdItemPv = GetLogicServer()->GetDataProvider()->GetStdItemProvider();

	int nTopLevel = GetGlobalLogicEngine()->GetTopTitleMgr().GetTopMaxLevel();
	int nId = stdItemPv.GetForgeIdByLevel(nTopLevel);
	data<<(BYTE)nId;
	ap.flush();
}

void BasicFunctionsSystem::HandleRefining(CDataPacketReader& inPacket)
{
	if(!m_pEntity) return ;
	CActor* pActor = (CActor*)m_pEntity;
	int nerrorCore = 0;
	byte nType = 0;//type: 1为装备 2:背包
	inPacket >> nType;
	byte nReplace = 0; //1替换 0 ：不替换
	CUserItem::ItemSeries series;
	inPacket >> series.llId;
	inPacket >> nReplace;
	int nEqPos = 0;
	BYTE bCanReplace = 0;
	CUserItem* pUseItem = NULL;
	do
	{
		CUserBag& bag = ((CActor*)m_pEntity)->GetBagSystem();
		if(nType == 1)
		{
			CUserEquipment& equip = ((CActor*)m_pEntity)->GetEquipmentSystem();
			for(int nPos = itWeaponPos; nPos < itMaxEquipPos; nPos++)
			{
				if(equip.m_Items[nPos])
				{
					if(equip.m_Items[nPos]->series.llId == series.llId)
					{
						pUseItem = equip.m_Items[nPos];
						nEqPos = nPos;
						break;
					}
				}
			}
			
		}
		else if(nType == 2) {
			// CUserBag& bag = ((CAvtor*)m_pEntity)->GetBagSystem();
			pUseItem = bag.FindItemByGuid(series);
		}
		else
			nerrorCore =1;
		
		if(nerrorCore)
			break;
		

		if(!pUseItem)
		{
			nerrorCore = 2;
			break;
		}
		if (((CActor *)m_pEntity)->GetDealSystem().FindDealItem(pUseItem))
		{
			((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmFailForDealing,tstUI);
			nerrorCore = 6;
			break;
		}
		RefiningCfg* cfg = GetLogicServer()->GetDataProvider()->GetRefiningConfig().getRefiningCfg(pUseItem->wItemId);
		if(!cfg)
		{
			nerrorCore = 3;
			break;
		}
		int nCostYb = 0;
		if(cfg->cost.size())
		{
			for(int i = 0; i < cfg->cost.size(); i++)
			{
				bool bReplace = false;
				ACTORAWARD& table = cfg->cost[i];
				ReplaceCfg* res = GetLogicServer()->GetDataProvider()->GetRefiningConfig().getReplaceCfg(table.wId);
				if(res)
				{
					if(nReplace & (1<< res->nId))
					{
						bReplace = true;
						nCostYb += table.wCount*(res->nPrice);
					}
				}
				if(!bReplace)
				{
					if(!pActor->CheckActorSource(table.btType, table.wId, table.wCount, tstUI))
					{
						nerrorCore = 4;
						break;
					}
				}
				
			}
		}
		if(nerrorCore)
			break;
		
		if(nCostYb)
		{
			if(!pActor->CheckActorSource(qatYuanbao, qatYuanbao, nCostYb, tstUI))
			{
				nerrorCore = 5;
				break;
			}
		}

		if(cfg->cost.size())
		{
			for(int i = 0; i < cfg->cost.size(); i++)
			{
				bool bReplace = false;
				ACTORAWARD& table = cfg->cost[i];
				ReplaceCfg* res = GetLogicServer()->GetDataProvider()->GetRefiningConfig().getReplaceCfg(table.wId);
				if(res)
				{
					if(nReplace & (1<< res->nId))
						bReplace = true;
				}
				if(!bReplace)
				{
					pActor->RemoveConsume(table.btType, table.wId, table.wCount, -1, -1,-1,0, GameLog::Log_Refining);
				}
				
			}
		}
		if(nCostYb)
		{
			pActor->RemoveConsume(qatYuanbao, qatYuanbao, nCostYb, -1, -1,-1,0, GameLog::Log_Refining);
		}

		DROPGOODS item;
		const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUseItem->wItemId);
		if(pStdItem &&  Item::itUndefinedType < pStdItem->m_btType < Item::itEquipMax)
		{
			GetLogicServer()->GetDataProvider()->GetRefiningConfig().GetRefineAttribute(cfg->nRefineId, item);
		}
		std::map<int, int> new_attr;
		memset(pUseItem->Refining, 0, sizeof(pUseItem->Refining));
		int num = item.v_bestAttr.size();
		for(int j = 0; j < num; j++)
		{
			char buf[10];
			if(j != 0)
				strcat(pUseItem->Refining, "|");

			DropAttribute data = item.v_bestAttr[j];
			sprintf(buf, "%d,%d", data.nType, data.nValue);
			new_attr[data.nType] = data.nValue;
			strcat(pUseItem->Refining, buf);
		}
		pUseItem->Refining[sizeof(pUseItem->Refining)-1]= '\0';	

		//默认木有极品属性的数据直接替换
		if(strlen(pUseItem->cBestAttr) == 0 && strlen(pUseItem->Refining))
		{
			memcpy(pUseItem->cBestAttr, pUseItem->Refining, sizeof(pUseItem->cBestAttr));
			memset(pUseItem->Refining, 0, sizeof(pUseItem->Refining));
			bCanReplace = 1;
		}
		std::map<int, int> old_attr;
		if(!bCanReplace && strlen(pUseItem->cBestAttr))
		{
			std::vector<std::string> results = SplitStr(pUseItem->cBestAttr, "|");
			int nCount = results.size();
			for (int i = 0; i < nCount; i++) {
				std::vector<std::string> temp_res = SplitStr(results[i], ",");
				if (temp_res.size() >= 2) {
					int nType = atoi(temp_res[0].c_str());
					int nValue = atoi(temp_res[1].c_str());
					old_attr[nType] = nValue;
				}
			}
			if(old_attr.size() )
			{
				bool bAutoReplace = true;
				std::map<int, int>::iterator it = old_attr.begin();
				for(; it != old_attr.end(); it++)
				{
					std::map<int, int>::iterator iter = new_attr.find(it->first);
					if(iter == new_attr.end() || (iter->second < it->second))
					{
						bAutoReplace = false;
						break;
					}
				}
				if(bAutoReplace) {
					memcpy(pUseItem->cBestAttr, pUseItem->Refining, sizeof(pUseItem->cBestAttr));
					memset(pUseItem->Refining, 0, sizeof(pUseItem->Refining));
					bCanReplace = 1;
				}
			}
		}

		if(bCanReplace)
		{
			//需要重新计算属性
			m_pEntity->CollectOperate(CEntityOPCollector::coRefAbility);	
			// 更新升级后的血、蓝
		}
		if(nReplace)
		{
			((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmRefiningReplace,tstUI);
		}

		if(nType == 2)
			bag.NotifyItemChange(pUseItem);
		else if(nType == 1)
			((CActor*)m_pEntity)->GetEquipmentSystem().EquipChange(nEqPos);

	} while(false);
	
	CActorPacket ap;
	CDataPacket& data = ((CActor*)m_pEntity)->AllocPacket(ap);
	data <<(byte)GetSystemID()<<(byte)enRefiningResult;
	data <<(byte)nerrorCore;
	data << (int64)series.llId;
	data << (byte)bCanReplace;
	if(pUseItem)
	{
		data.writeString(pUseItem->cBestAttr);
		data.writeString(pUseItem->Refining);
	}
	ap.flush();
	
}

void BasicFunctionsSystem::HandleRefiningReplace(CDataPacketReader& inPacket)
{
	if(!m_pEntity) return ;
	byte errorCode = 0;
	byte nType = 0;//type: 1为装备 2:背包
	inPacket >> nType;
	CUserItem::ItemSeries series;
	inPacket >> series.llId;
	CUserItem* pUseItem = NULL;
	int nEqPos = 0;
	do
	{
		if(nType == 1)
		{
			CUserEquipment& equip = ((CActor*)m_pEntity)->GetEquipmentSystem();
			for(int nPos = itWeaponPos; nPos < itMaxEquipPos; nPos++)
			{
				if(equip.m_Items[nPos])
				{
					if(equip.m_Items[nPos]->series.llId == series.llId)
					{
						pUseItem = equip.m_Items[nPos];
						nEqPos = nPos;
						break;
					}
				}
			}
		}
		else if(nType == 2) {
			CUserBag& bag = ((CActor*)m_pEntity)->GetBagSystem();
			pUseItem = bag.FindItemByGuid(series);
		}
		else
			errorCode = 1;
		
		if(errorCode)
			break;
		
		if(!pUseItem)
		{
			errorCode = 2;
			break;
		}

		if(strlen(pUseItem->Refining) == 0)
		{
			errorCode = 3;
			break;
		}

		memcpy(pUseItem->cBestAttr, pUseItem->Refining, sizeof(pUseItem->cBestAttr));
		memset(pUseItem->Refining, 0, sizeof(pUseItem->Refining));
	
		if(nType == 2)
			((CActor*)m_pEntity)->GetBagSystem().NotifyItemChange(pUseItem);
		else if(nType == 1)
			((CActor*)m_pEntity)->GetEquipmentSystem().EquipChange(nEqPos);
		//需要重新计算属性
		m_pEntity->CollectOperate(CEntityOPCollector::coRefAbility);	
		// 更新升级后的血、蓝

	} while (false);
	
	CActorPacket ap;
	CDataPacket& data = ((CActor*)m_pEntity)->AllocPacket(ap);
	data <<(byte)GetSystemID() <<(byte)enRefiningReplaceResult;
	data <<(byte)errorCode;
	data << (int64)series.llId;
	if(pUseItem)
	{
		data.writeString(pUseItem->cBestAttr);
		data.writeString(pUseItem->Refining);
	}
	ap.flush();

}

void BasicFunctionsSystem::SendRecoverItemInfo()
{
	if(!m_pEntity) return;
	CActorPacket ap;
	CDataPacket & outPack = ((CActor*)m_pEntity)->AllocPacket(ap);
	outPack << (BYTE) GetSystemID()<<(BYTE)enYBRecoverInfoResult;
	int nCount = 0;
	int nPos = outPack.getPosition();
	outPack<<(int)nCount;
	CStdItemProvider& stdItemPv = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	std::map<int, ItemRecoverCfg>& m_Cfgs = stdItemPv.GetRecoverItemCfgLists();
	int nNowOpenDay = GetLogicServer()->GetDaysSinceOpenServer();
	CScriptValueList paramList, retParamList;
	CHAR sFnArgs[1024];
	sFnArgs[0]=0;
	paramList << ((CActor*)m_pEntity);
	paramList << &ap;
	if(GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().Call("ClientGetData", paramList, retParamList, 1))
	{
		if(retParamList.count() > 0)
		{
			int *pInt = (int*)outPack.getPositionPtr(nPos);
			*pInt = (int)retParamList[0];
		}
	}
	ap.flush();
}

void BasicFunctionsSystem::UpdateItemStar(CDataPacketReader& packet)
{
	if(!m_pEntity) return;
	// CUserItem::ItemSeries series;
	// packet >> series.llId;
	byte nReplace = 0; //1替换 0 ：不替换
	CUserItem::ItemSeries series;
	packet >> series.llId;
	packet >> nReplace;
	int errorCode = 0;
	CActor* pActor = (CActor*)m_pEntity;
	if(!pActor) return;
	CUserItem * pUseItem = NULL;
	do
	{
		int nEqPos = 0;
		pUseItem = pActor->GetEquipmentSystem().GetEquipByGuid(series);
		if(!pUseItem) {
			errorCode = 1;//道具不存在
			break;
		}
		nEqPos = pActor->GetEquipmentSystem().FindIndex(series);
		if(nEqPos == -1) {
			errorCode = 1;//道具不存在
			break;
		}
		
		ItemUpStarCfg * cfg = GetLogicServer()->GetDataProvider()->GetUpStar().getItemUpStarCfg(pUseItem->wItemId, pUseItem->wStar+1);
		if(!cfg) {
			errorCode = 3;
			break;
		}

		ItemUpStarCfg* pCostcfg = GetLogicServer()->GetDataProvider()->GetUpStar().getItemUpStarCfg(pUseItem->wItemId, pUseItem->wStar);
		if(!pCostcfg) {
			errorCode = 2;
			break;
		}
		// if(pCostcfg->cost.size() > 0) {
		// 	for(auto co : pCostcfg->cost) {
		// 		if(!pActor->CheckActorSource(co.btType, co.wId, co.wCount, tstUI))
		// 			return;
		// 	}
		// }
		int nCostYb = 0;
		if(pCostcfg->cost.size())
		{
			for(int i = 0; i < pCostcfg->cost.size(); i++)
			{
				bool bReplace = false;
				ACTORAWARD& table = pCostcfg->cost[i];
				StarReplaceCfg* res = GetLogicServer()->GetDataProvider()->GetUpStar().getUpStareplaceCfg(table.wId);
				if(res)
				{
					if(nReplace & (1<< res->nId))
					{
						bReplace = true;
						nCostYb += table.wCount*(res->nPrice);
					}
				}
				if(!bReplace)
				{
					if(!pActor->CheckActorSource(table.btType, table.wId, table.wCount, tstUI))
					{
						errorCode = 4;
						return;
					}
				}
				
			}
		}
		if(errorCode)
			return;
		
		if(nCostYb)
		{
			if(!pActor->CheckActorSource(qatYuanbao, qatYuanbao, nCostYb, tstUI))
			{
				errorCode = 5;
				return;
			}
		}
		if(errorCode)
			return;

		
		int nDownLv = 0;
		if(wrand(10000) > cfg->nUpRate) {
			int nrate = wrand(cfg->nMaxRate);
			if(cfg->m_rates.size()) {
				int nrand = 0;
				for(auto r : cfg->m_rates) {
					nrand += r.nRate;
					if(nrate <= nrand) {
						nDownLv = r.nValue;
						break;
					}
				}
			}

			if(nDownLv > pUseItem->wStar)
				nDownLv = pUseItem->wStar;
			((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmUpStarFailDownStar, tstUI, nDownLv);	
			pUseItem->wStar -= nDownLv;
			errorCode = 4;
		}
		else
		{
			pUseItem->wStar += 1;
			int nTipId = 0;
			if(pUseItem->wStar >= 11) {
				nTipId = tmItemUpMaxStar2;
			}
			else if(pUseItem->wStar >= 8)
			{
				nTipId = tmItemUpMaxStar1;
			}
			if(nTipId > 0)
			{
				LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(nTipId);
				const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUseItem->wItemId);
				if(pStdItem && sFormat) {
					char sContent[1024]={0};
					sprintf_s(sContent, sizeof(sContent), sFormat, ( char *)(pActor->GetEntityName()), ( char *)(pStdItem->m_sName), pUseItem->wStar);
					GetGlobalLogicEngine()->GetEntityMgr()->BroadNotice(sContent,tstRevolving);
					GetGlobalLogicEngine()->GetEntityMgr()->BroadNotice(sContent,tstChatSystem);
				}
			}
		}
		
		if(pCostcfg->cost.size())
		{
			for(int i = 0; i < pCostcfg->cost.size(); i++)
			{
				bool bReplace = false;
				ACTORAWARD& table = pCostcfg->cost[i];
				StarReplaceCfg* res = GetLogicServer()->GetDataProvider()->GetUpStar().getUpStareplaceCfg(table.wId);
				if(res)
				{
					if(nReplace & (1<< res->nId))
						bReplace = true;
				}
				if(!bReplace)
				{
					pActor->RemoveConsume(table.btType, table.wId, table.wCount, -1, -1,-1,0, GameLog::Log_UpStar);
				}
			}
		}
		if(nCostYb)
		{
			pActor->RemoveConsume(qatYuanbao, qatYuanbao, nCostYb, -1, -1,-1,0, GameLog::Log_UpStar);
		}
		// if(pCostcfg->cost.size() > 0) {
		// 	for(auto co : pCostcfg->cost) {
		// 		pActor->RemoveConsume(co.btType, co.wId, co.wCount,-1,-1,-1,0,GameLog::Log_UpStar,"up item star cost");
		// 	}
		// }	

		pActor->GetEquipmentSystem().EquipChange(nEqPos);
		//需要重新计算属性
		m_pEntity->CollectOperate(CEntityOPCollector::coRefAbility);	
		// 更新升级后的血、蓝

		if(nReplace)
		{
			((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmRefiningReplace,tstUI);
		}
		break;

	}while(false);
	CActorPacket ap;
	CDataPacket& data = pActor->AllocPacket(ap);
	data <<(BYTE)GetSystemID() <<(BYTE)enItemUpStarResult;
	data <<(BYTE)errorCode;
	data <<(int64)series.llId;
	if(pUseItem)
		data <<(WORD)(pUseItem->wStar);
	ap.flush();
}
