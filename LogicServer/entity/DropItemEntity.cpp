#include "StdAfx.h"
#include "DropItemEntity.h"

VOID CDropItemEntity::LogicRun(TICKCOUNT nCurrentTime)
{
	//延迟删除这个实体
	if(nCurrentTime >= m_expireTime)
	{
		GetGlobalLogicEngine()->GetEntityMgr()->DestroyEntity(GetHandle(),true);
	}
}
void CDropItemEntity::Destroy()
{
	if(IsInited() ==false)
	{
		OutputMsg(rmWaning,"CDropItemEntity destroy again");

	}
	Inherited::Destroy();
	if(m_pItem !=NULL)
	{
		GetGlobalLogicEngine()->DestroyUserItem(m_pItem);
		m_pItem =NULL;
	}
}
bool CDropItemEntity::Init(void * data, size_t size)
{
	if( Inherited::Init(data,size) ==false ) return false; 
	m_pItem =NULL; //物品的指针
	m_ownerExpireTime =0; 
	m_btMoneyType = mtCoin;

	//该道具的存活时间
	static int s_nExpiretTime =  GetLogicServer()->GetDataProvider()->GetDropItemConfig().nDropItemExpireTime * 1000;
	m_expireTime = GetGlobalLogicEngine()->getTickCount() + s_nExpiretTime ;
	m_nLogId =0;
	m_CanPickUpTime = 0;
	m_sLogDesc[0] =0;
	m_showLootTips =false;
	
	SetInitFlag(true);
	return true;
} 

//设置这个道具属于他的主人的时间
void CDropItemEntity::SetMasterOwnerTime(int nMinSecond)
{
	m_ownerExpireTime = GetGlobalLogicEngine()->getTickCount() + nMinSecond ;
}

void CDropItemEntity::SetCanPickUpTime(int nTime)
{
	m_CanPickUpTime = GetGlobalLogicEngine()->getTickCount() + nTime* 1000;
}

void CDropItemEntity::SetDropMasterId(CActor * pActor)
{
	if (pActor != NULL)
	{
		SetProperty<unsigned int>(PROP_ENTITY_DIR, pActor->GetProperty<unsigned int>(PROP_ENTITY_ID));
	}	
}


void CDropItemEntity::SetDropNoMasterId()
{

	SetProperty<unsigned int>(PROP_ENTITY_DIR, 1);
}

void CDropItemEntity::SetMaster(CActor *pActor)
{
	if(pActor !=NULL) 
	{
		SetProperty<unsigned int>(PROP_ENTITY_ID,pActor->GetProperty<unsigned int>(PROP_ENTITY_ID));  
		//static int s_nMasterExpiretTime =  GetLogicServer()->GetDataProvider()->GetGlobalConfig().nDropItemMasterProtectTime * 1000;
		static int s_nMasterExpiretTime =  GetLogicServer()->GetDataProvider()->GetDropItemConfig().nDropItemMasterProtectTime * 1000;

		SetMasterOwnerTime(s_nMasterExpiretTime);
	}
}
void CDropItemEntity::SetItem( CUserItem *pUserItem)
{
	if(pUserItem && m_pItem != NULL)
	{
		OutputMsg(rmWaning,"SetItem Old Item Not NULL");
	}
	if(pUserItem)
	{
		const CStdItem *pItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem( pUserItem->wItemId);
		if(pItem)
		{
			// if(pItem->w_candidateIconCount >0) //如果有候选的图标
			// {
			// 	int nIcon = pItem->m_wIcon + wrand(pItem->w_candidateIconCount +1);
			// 	SetProperty<int>(PROP_ENTITY_ICON,nIcon); 
			// }
			// else
			{
				SetProperty<int>(PROP_ENTITY_ICON,pItem->m_wIcon); //设置图标的ID为道具的ID
			}
			if(pItem->m_Flags.showLootTips)
			{
				m_showLootTips =true;
			}
			SetEntityName(pItem->m_sName); //设置名字
		}
		
	}
	m_pItem = pUserItem;
	
}

int CDropItemEntity::GetLootErrorCode(CActor * pActor,bool boObserVer)
{
	if(pActor ==NULL ) return tmLootParamError; //参数非法

	TICKCOUNT nCurTick = GetGlobalLogicEngine()->getTickCount(); //获取当前的时间
	
	//已经过期
	if(nCurTick > m_expireTime)
	{
		return tmLootExpired; 
	}


	if(nCurTick < m_ownerExpireTime)
	{
		unsigned int nMasterId =GetMasterActorId();		//物品所归属的玩家ID
		if(nMasterId && pActor->GetProperty<unsigned int >(PROP_ENTITY_ID) != nMasterId )
		{
			return tmLootNotOwner; 
		}
		unsigned int nTeamId = GetTeamId();

		if(nTeamId &&  nTeamId !=pActor->GetProperty<unsigned int >(PROP_ACTOR_TEAM_ID) )
		{
			return tmLootNotOwner; 
		}	
	}
	else
	{
		if(nCurTick < m_CanPickUpTime)
		{
			return tmLootCanPickUpTime;
		}
	}


	if (!boObserVer)
	{
		int nItemPosX,nItemPosY, nActorPosX,  nActorPosY;
		pActor->GetPosition(nActorPosX,nActorPosY);
		GetPosition(nItemPosX,nItemPosY);
		if(nItemPosX != nActorPosX || nItemPosY != nActorPosY)
		{
			//OutputMsg(rmTip,"client[%d,%d], item[%d,%d]",nActorPosX,nActorPosY,nItemPosX,nItemPosY);
			//return tpNoError;
			return tmLootPosError;
		}
		else
		{
			return tpNoError;
		}
	}
	else
	{
		return tpNoError;
	}

}

//设置这个队伍的归属
void CDropItemEntity::SetTeamId(unsigned int nTeamId)
{

	if(nTeamId )
	{
		SetProperty<unsigned int>(PROP_ENTITY_DIR,nTeamId);
		//static int s_nMasterExpiretTime =  GetLogicServer()->GetDataProvider()->GetGlobalConfig().nDropItemMasterProtectTime * 1000;
		static int s_nMasterExpiretTime =  GetLogicServer()->GetDataProvider()->GetDropItemConfig().nDropItemMasterProtectTime * 1000;

		SetMasterOwnerTime(s_nMasterExpiretTime);
	}
	
}

//设置金钱的数目
void CDropItemEntity::SetMoneyCount( int nMoneyCount, int nMoneyType)
{
	if(nMoneyCount <0) return;
	SetProperty<int>(PROP_ENTITY_MODELID,nMoneyCount); //这个属性没有用，用于存金钱的数目，省点内存
	m_btMoneyType = (BYTE)nMoneyType;	//设置金币类型
	if(nMoneyCount >0 )
	{
		//static int s_MoneyIconIconId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nDropCoinIconId; //金钱使用哪个图标
		//GLOBALCONFIG & prop= GetLogicServer()->GetDataProvider()->GetGlobalConfig();
		DROPITEMCONFIG & prop= GetLogicServer()->GetDataProvider()->GetDropItemConfig();
		
		int nIconId =0, nNameTipsId=0;
		INT_PTR nMaxId = MAX_DROP_COIN_ICON_COUNT -1;

		int *pDropCounts = NULL;
		int *pDropIcons = NULL;
		// if (m_btMoneyType == mtYuanbao)
		// {
		// 	pDropCounts = prop.dropYuanBaoCounts;
		// 	pDropIcons = prop.dropYuanBaoIcons;
		// 	nNameTipsId = tpMoneyTypeYuanbao;
		// }
		if (m_btMoneyType == mtCoin)
		{
			pDropCounts = prop.dropCoinCounts;
			pDropIcons = prop.dropCoinIcons;
			nNameTipsId = tpMoneyTypeCoin;
		}
		else if (m_btMoneyType == mtBindCoin)
		{
			pDropCounts = prop.dropBindCoinCounts;
			pDropIcons = prop.dropBindCoinIcons;
			nNameTipsId = tpMoneyTypeBindCoin;
		}

		if (!pDropCounts || !pDropIcons)
			return;

		for (INT_PTR i = 0; i < nMaxId; i++ )
		{
			int nNextCount = pDropCounts[i+1];
			if (nMoneyCount >= pDropCounts[i] && (nMoneyCount < nNextCount || nNextCount ==0))
			{
				nIconId = pDropIcons[i];
				break;
			}
			/*
			int nNextCount = prop.dropCoinCounts[i+1];
			if(nMoneyCount >= prop.dropCoinCounts[i] &&  ( nMoneyCount < nNextCount || nNextCount ==0) )
			{
				nIconId = prop.dropCoinIcons[i];
				break;
			}
			*/
		}
		//名字是金币
		SetEntityName(GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(nNameTipsId)); //设置名字

		SetProperty<int>(PROP_ENTITY_ICON,nIconId); //设置图标的ID为金钱的图标的ID

	}
}


//获取金钱的数目
unsigned int CDropItemEntity::GetMoneyCount()
{
	return GetProperty<unsigned int>(PROP_ENTITY_MODELID); //这个属性没有用，用于存金钱的数目，省点内存
}

CDropItemEntity * CDropItemEntity::CreateDropItem( CScene *pScene, int nPosX,int nPosY,int nLogId ,LPCTSTR sLogDesc,int nExitTime,bool flag)
{
	
	if (!pScene) {
		return NULL;
	}

	INT_PTR   nDX, nDY;
	bool boResult = false;
	int nMax = 0;
	INT_PTR nTempX = 0;
	INT_PTR nTempY = 0;


	//优先 怪物点掉落
	if(pScene->CanMove(nPosX,nPosY) && !pScene->IsTelePort(nPosX,nPosY)) {
		nMax= pScene->GetTypeEntityCount((int)nPosX, (int)nPosY,enDropItem); //获取掉落的物品的数目
		nTempX = nPosX;
		nTempY = nPosY;
	}

	if(flag) {
		nMax = 666666;//强行进入下面循环
	}		


	if(nMax != 0 ) {

		for(int i = 1; i <= 3; i++) {

			for(int y = -i; y <= i; y++) {

				for(int x = -i; x <= i; x++) {
	
					nDX = nPosX + x;
					nDY = nPosY + y;
					if(pScene->CanMove(nDX,nDY) && !pScene->IsTelePort(nDX,nDY) && !pScene->GetNpcAtXY(nDX,nDY)) {
						if( flag && x==0 && y==0) continue ;

						int nCount= pScene->GetTypeEntityCount((int)nDX, (int)nDY,enDropItem); //获取掉落的物品的数目
						
						if (nCount == 0) {
							boResult = true;
							break;
						}
						else {

							if( nMax > nCount) {
								nMax = nCount;
								nTempX = nDX;
								nTempY = nDY;
							}
						}
					}
				}

				if (boResult) break;
			}

			if (boResult) break;
		}
	}
	if (!boResult ) {

		if (nMax < 5) {
			nDX = nTempX;
			nDY = nTempY;
		}
		else {
			return NULL;
		}
	}

	CDropItemEntity* pEntity =  (CDropItemEntity*)(pScene->CreateEntityAndEnterScene(0, enDropItem, nDX, nDY));
	if(pEntity)
	{
		pEntity->m_nLogId = nLogId;
		if(nExitTime > 0)
			pEntity->m_expireTime = GetGlobalLogicEngine()->getTickCount() + nExitTime * 1000;

		if(sLogDesc) {
			strncpy(pEntity->m_sLogDesc,sLogDesc,sizeof(pEntity->m_sLogDesc));
		}
		return pEntity;
	}

	return NULL;
		
}

CDropItemEntity * CDropItemEntity::CreateOne( CScene *pScene, int nPosX, int nPosY, int nLogId, LPCTSTR sLogDesc, int nExitTime)
{
	static const int MAX_COUNT_OF_ONE_GRID_CAN_DROP = 5;

	if (!pScene) return NULL;

	INT_PTR nDropPosX = nPosX, nDropPosY = nPosY;
	size_t nDist = 0;

	int nLoopCount = 64;
	while (nLoopCount-- > 0)
	{
		int nLeftRight[2] = { nPosX - nDist, nPosX + nDist };
		int nTopDown[2] = { nPosY - nDist, nPosY + nDist };

		//上边
		for (nDropPosX = nLeftRight[0]; nDropPosX <= nLeftRight[1]; nDropPosX++)
		{
			nDropPosY = nTopDown[0];
			if(
				pScene->GetTypeEntityCount(nDropPosX, nDropPosY, enDropItem) < MAX_COUNT_OF_ONE_GRID_CAN_DROP &&
				pScene->CanMove(nDropPosX,nDropPosY) &&
				!pScene->IsTelePort(nDropPosX,nDropPosY)
			)
			{
				goto create_one;
			}
		}

		//右边
		for (nDropPosY = nTopDown[0] + 1; nDropPosY <= nTopDown[1]; nDropPosY++)
		{
			nDropPosX = nLeftRight[1];
			if(
				pScene->GetTypeEntityCount(nDropPosX, nDropPosY, enDropItem) < MAX_COUNT_OF_ONE_GRID_CAN_DROP &&
				pScene->CanMove(nDropPosX,nDropPosY) &&
				!pScene->IsTelePort(nDropPosX,nDropPosY)
			)
			{
				goto create_one;
			}
		}

		//下边
		for (nDropPosX = nLeftRight[1] - 1; nDropPosX >= nLeftRight[0]; nDropPosX--)
		{
			nDropPosY = nTopDown[1];
			if(
				pScene->GetTypeEntityCount(nDropPosX, nDropPosY, enDropItem) < MAX_COUNT_OF_ONE_GRID_CAN_DROP &&
				pScene->CanMove(nDropPosX,nDropPosY) &&
				!pScene->IsTelePort(nDropPosX,nDropPosY)
			)
			{
				goto create_one;
			}
		}

		//左边
		for (nDropPosY = nTopDown[1] - 1; nDropPosY > nTopDown[0]; nDropPosY--)
		{
			nDropPosX = nLeftRight[0];
			if(
				pScene->GetTypeEntityCount(nDropPosX, nDropPosY, enDropItem) < MAX_COUNT_OF_ONE_GRID_CAN_DROP &&
				pScene->CanMove(nDropPosX,nDropPosY) &&
				!pScene->IsTelePort(nDropPosX,nDropPosY)
			)
			{
				goto create_one;
			}
		}

		//向外一圈
		nDist++;
	}

	return NULL;

	// 场景中生成1个掉落品
	create_one:
	{
		CDropItemEntity* pDropItem =  (CDropItemEntity*)(pScene->CreateEntityAndEnterScene(0, enDropItem, nDropPosX, nDropPosY));
		pDropItem->m_nLogId = nLogId;
		if(nExitTime > 0)
			pDropItem->m_expireTime = GetGlobalLogicEngine()->getTickCount() + nExitTime * 1000;

		if(sLogDesc) {
			strncpy(pDropItem->m_sLogDesc,sLogDesc,sizeof(pDropItem->m_sLogDesc));
		}
		return pDropItem;
	}
}

void CDropItemEntity::DealDropItemTimeInfo(CActor * pActor, int type, int pick_time)
{
	static int nNeedpickTime =  GetLogicServer()->GetDataProvider()->GetDropItemConfig().nDropItemNeedTime;
	static int nActorpickTime =  GetLogicServer()->GetDataProvider()->GetDropItemConfig().nDropItemMasterProtectTime;
	CTeam * pTeam = NULL;
	if(pActor)
	{
		pTeam = pActor->GetTeam();
	}
	
	if(type == 0)
	{
		if(pTeam) {
			SetTeamId(pTeam->GetTeamID());
		}
		else {
			if(pActor)
			{
				SetMaster(pActor); //这个是他的主人了
			}
		}
		SetMasterOwnerTime(nActorpickTime*1000);
		SetCanPickUpTime(nActorpickTime);
	}
	else if(type == 1) { //----所有人都能捡取

	} else if(type== 2) { //---仅限击杀者捡取

		if(pTeam) {
			SetTeamId(pTeam->GetTeamID());
		}
		else {
			if(pActor)
			{
				SetMaster(pActor); //这个是他的主人了
			}
		}
		SetCanPickUpTime(100000000);
		SetMasterOwnerTime(100000000);  //设置一个很大的时间,24小时以上

	} else if(type == 666) { //----活动用,手动设置捡取时间
		SetCanPickUpTime(pick_time);
		SetDropNoMasterId();
		
	}else { //----所有人都过60秒才能捡取
		SetCanPickUpTime(nNeedpickTime);
		SetDropNoMasterId();

	}
}


int CDropItemEntity::GetPetLootErrorCode(CActor * pActor,bool boObserVer)
{
	if(pActor ==NULL ) return tmLootParamError; //参数非法

	TICKCOUNT nCurTick = GetGlobalLogicEngine()->getTickCount(); //获取当前的时间
	int nLootPetID = pActor->GetProperty<int>(PROP_ACTOR_LOOT_PET_ID);
	int nDropLootDistance = 0;
	auto cfg  = GetLogicServer()->GetDataProvider()->GetLootPetProvider().GetLootPetConfig(nLootPetID);
	if(cfg) {
		nDropLootDistance = cfg->nLootPetDistance;
	}

	//当捡取宠物都不显示时，默认类型最小Type类型的捡取距离
	if (nLootPetID == 0 && pActor->GetProperty<unsigned long long>(PROP_ACTOR_DEFAULTLOOTPETDIS) != 0)
	{
		nDropLootDistance = pActor->GetProperty<unsigned long long>(PROP_ACTOR_DEFAULTLOOTPETDIS);
	}

	//已经过期
	if(nCurTick > m_expireTime)
	{
		return tmLootExpired; 
	}


	if(nCurTick < m_ownerExpireTime)
	{
		unsigned int nMasterId =GetMasterActorId();		//物品所归属的玩家ID
		if(nMasterId && pActor->GetProperty<unsigned int >(PROP_ENTITY_ID) != nMasterId )
		{
			return tmLootNotOwner; 
		}
		unsigned int nTeamId = GetTeamId();

		if(nTeamId &&  nTeamId !=pActor->GetProperty<unsigned int >(PROP_ACTOR_TEAM_ID) )
		{
			return tmLootNotOwner; 
		}	
	}
	else
	{
		if(nCurTick < m_CanPickUpTime)
		{
			return tmLootCanPickUpTime;
		}
	}


	if (!boObserVer)
	{
		int nItemPosX,nItemPosY, nActorPosX,  nActorPosY;
		pActor->GetPosition(nActorPosX,nActorPosY);
		GetPosition(nItemPosX,nItemPosY);
		if(abs(nItemPosX - nActorPosX) > nDropLootDistance || abs(nItemPosY - nActorPosY) > nDropLootDistance)
		{
			return tmLootPosError;
		}
		else
		{
			return tpNoError;
		}
	}
	else
	{
		return tpNoError;
	}

}