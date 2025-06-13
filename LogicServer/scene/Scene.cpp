#include "StdAfx.h"
#include "EntityGrid.h"
#include "Scene.h"
#include "../LogicServer.h"

using namespace wylib::container;

bool binarySearch(int* vList, INT_PTR nCount,INT_PTR nValue) 
{
	INT_PTR low = 0;
	INT_PTR high = nCount - 1;
	while (low <= high) {
		INT_PTR mid = ((UINT_PTR) (low + high)) >> 1;
		int nMidValue = vList[mid];
		if (nMidValue< nValue)
			low = mid + 1;
		else if (nMidValue > nValue)
			high = mid - 1;
		else
			return true; // key found
	}
	return false;   // key not found.
}

CScene::CScene()
:m_AStar()
{
	m_vRefreshMonster = NULL;
	m_pMapData = NULL;
	m_pSceneData = NULL;
	m_Grid.SetScene(this);
	m_pMapAreaTable = NULL;
	m_pTelePortTable = NULL;
	m_restTime = 0;
	m_nRefreshtime = 0;
#ifndef MONSTER_MANAGER_BY_SCENE
#else
	m_MonInterator = m_MonsterList.end();
#endif
}

CScene::~CScene()
{
	//释放所有的实体
	m_Grid.Release();
	m_Grid.SetScene(this);
	if (m_vRefreshMonster)
	{
		GFreeBuffer(m_vRefreshMonster);
		m_vRefreshMonster = NULL;
	}
	ReleaseConfig();
#ifndef MONSTER_MANAGER_BY_SCENE
#else
	m_MonsterList.clear();
	m_MonInterator = m_MonsterList.end();
#endif
}

BOOL CScene::Init(SCENECONFIG* pData,CAbstractMap* pMapData,CFuBen* pFb,CObjectAllocator<char>* dataAlloc,bool boRun)
{
	SF_TIME_CHECK(); //检测性能
	DECLARE_TIME_PROF("CScene::Init");
	m_pSrcSceneData = pData;

	// 复制一份场景配置文件（主要是区域属性）
	CloneSceneConfig(pData);

	// 刷怪配置
	if (m_vRefreshMonster)
	{
		GFreeBuffer(m_vRefreshMonster);
		m_vRefreshMonster = NULL;
	}
	SetFuBen(pFb);

	// 获取对应的地图数据（场景宽、高	和网格信息）
	m_pMapData = pMapData;
	m_nWidth = pMapData->getWidth();
	m_nHeight = pMapData->getHeight();
	m_Grid.Init((int)m_nHeight,(int)m_nWidth);
	m_Grid.SetScene(this);
	m_AStar.initFromMap(m_pMapData);
#ifndef MONSTER_MANAGER_BY_SCENE
#else
	m_MonsterList.clear();
	m_MonInterator = m_MonsterList.end();
#endif
	return TRUE;
}

void CScene::CreateNpcTransport(bool boRun)
{
	if (m_pSceneData)
	{		
		CreateNpc(m_pSceneData,boRun);		// 创建npc		
		CreateTeleport();					// 创建传送门
	}
}

BOOL CScene::Reset()
{
	//m_Grid要保证角色和怪物清理出去
	//m_Grid.Reset();

	//删除场景的实体
	//NPC不删除
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it;
	CEntityManager* pMgr = GetGlobalLogicEngine()->GetEntityMgr();

	//销毁怪物
#ifndef MONSTER_MANAGER_BY_SCENE
	it.setList(m_MonsterList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		pMgr->DestroyEntity( pNode->m_Data);
	}
	m_MonsterList.clear();
#else
	// std::map<EntityHandle,CMonster*>::iterator iter = m_MonsterList.begin();
	// for (; iter != m_MonsterList.end(); ++iter)
	// {
	// 	pMgr->DestroyEntity( iter->first );
	// }
	std::map<EntityHandle,CMonster*>::iterator iter = m_MonInterator;
	while (iter != m_MonsterList.end())
	{
		pMgr->DestroyEntity( iter->first );//删除时，会触发 CScene::RemoveEntity， m_MonInterator自增；
		if (iter == m_MonInterator) break;
		iter = m_MonInterator;
	}
	m_MonsterList.clear();
	m_MonInterator = m_MonsterList.end();
#endif
	
	//删除掉落物品
	it.setList(m_dropItemList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		pMgr->DestroyEntity( pNode->m_Data);
	}
	m_dropItemList.clear();

	//删除火
	it.setList(m_fireList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		pMgr->DestroyEntity( pNode->m_Data);
	}
	m_fireList.clear();

	//重置npc
	it.setList(m_NpcList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CEntity* pEntity = pMgr->GetEntity(pNode->m_Data);
		if (pEntity != NULL && pEntity->GetType() == enNpc)
		{
			((CNpc*)pEntity)->ResetScript();
			pEntity->SetInitFlag(false);
		}
	}

	//重新copy配置文件
	CloneSceneConfig(m_pSrcSceneData);

	m_restTime = 0;
	//遍历所有NPC，调用reset
	
	//m_PlayList要清空
	m_PlayList.clear();
	m_petList.clear();
	m_heroList.clear();
	m_DeathList.clear();

	return TRUE;
}

void CScene::DestroySceneEntity()
{

}
void CScene::OnReuse()
{
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_NpcList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CEntity* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(pNode->m_Data);
		if (pEntity != NULL && pEntity->GetType() == enNpc)
		{
			pEntity->SetInitFlag(true);//使npc的logicrun能执行
		}
	}
	if (!m_vRefreshMonster)
	{
		m_vRefreshMonster = (REFRESHVALUE*)GAllocBuffer(sizeof(REFRESHVALUE)*(m_pSceneData->vRefreshList.nCount));
	}
	ResetRefreshList(&m_pSceneData->vRefreshList);
}

void CScene::Clone(CScene& DstScene,bool boRun)
{
	m_Grid.SetScene(this);
	//clone刷怪点信息
	if (m_pSceneData && boRun)
	{
		DstScene.SetRefreshList(&m_pSceneData->vRefreshList);
	}
	DstScene.m_pMapAreaTable = this->m_pMapAreaTable;
	DstScene.m_pTelePortTable = this->m_pTelePortTable;
}

bool CScene::CanPassScence(CActor* pActor)
{
	if( !pActor ) return false;
	return true;
}

bool CScene::CanEnter(CActor* pActor,INT_PTR x,INT_PTR y,bool bState)
{
	if(SCENEAREA* pArea = GetAreaAttri(x,y))
	{
		int nLevel = pActor->GetProperty<int>(PROP_CREATURE_LEVEL);
		bool boGm = pActor->GetGmLevel() > 0;
		//进入场景的最低等级
		if (HasMapAttribute(x,y,aaSceneLevel,pArea) && !boGm)
		{
			AreaAttri& attri = pArea->attri[aaSceneLevel];

			if(attri.pValues)
			{
				int nCircle = 0;
				if ( attri.nCount > 1 && attri.pValues[1] )
				{
					nCircle = attri.pValues[1];
				}
				if(!pActor->CheckLevel(attri.pValues[0], nCircle))
				{
					if(bState)
						//pActor->SendOldTipmsgFormatWithId(tpUnderMinLevel,ttDialog,nCircle,attri.pValues[0]);
						pActor->SendTipmsgWithId(tmLevelLowernoenter,tstUI);
					return false;
				}
			}
		}

		//进入场景的最高等级
		if (HasMapAttribute(x,y,aaSceneMaxLevel,pArea) && !boGm)
		{
			AreaAttri& attri = pArea->attri[aaSceneMaxLevel];

			if(attri.pValues)
			{
				if(nLevel > attri.pValues[0])
				{
					if(bState)
						//pActor->SendOldTipmsgFormatWithId(tpUpMaxLevel,ttDialog,attri.pValues[0]);
						pActor->SendTipmsgWithId(tmLevelExceednoenter,tstUI);
					return false;
				}
			}
		}
	}

	// if(IsTelePort(x,y) == true)		//传过去点是传送门
	// {
	// 	bool boTelePort = false;
	// 	int nIndex = m_pTelePortTable[y*m_nWidth+x];
	// 	if (GetSceneData() && nIndex <= GetSceneData()->TeleportList.nCount)
	// 	{			
	// 		TeleportConfig& config = GetSceneData()->TeleportList.pList[nIndex -1];
	// 		CEntity * pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(config.nHandle);
	// 		if (pEntity != NULL)
	// 		{
	// 			boTelePort = pEntity->GetAttriFlag().DenySee;
	// 		}
	// 	}
	// 	if (!boTelePort)
	// 	{
	// 		OutputMsg(rmError, _T("::%s want to MoveTo telepot(posX:%d posY:%d)"), pActor->GetEntityName(), x, y);
	// 		return false;
	// 	}		
	// }

	return true;
}

bool CScene::EnterScene(CEntity * pEntity,INT_PTR nPosX,INT_PTR nPosY, int nType,int nValue, int nParam)
{
	if (pEntity == NULL) return false;

	// 判断是否同屏传送
	int nCurPosX = 0,nCurPosY = 0;
	pEntity->GetPosition(nCurPosX, nCurPosY);

	//pLastScence有可能是NULL
	CFuBen *pLastFuben = pEntity->GetFuBen(); //获取实体上一个的副本
	CScene *pLastScence = pEntity->GetScene(); //获取实体上一个的场景

	bool bState = true;
	if (pLastScence == NULL) bState = false;

	INT_PTR nEntityType = pEntity ->GetType();
	if (enNpc == nEntityType && pEntity->IsInited())
	{
		OutputMsg(rmError, _T("%s:npc[%s] try to enter to scene, X:%d,Y:%d, is Stoped"), __FUNCTION__, pEntity->GetEntityName(),(int)nPosX,(int)nPosY);
		return false;
	}
	bool isActor = nEntityType == enActor;

	// 进入判断
	bool bCanAlwaysEnter = false;
	INT_PTR nDir = -1;
	if (pEntity->IsMonster())
	{
		PMONSTERCONFIG pConfig  = 
			GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));
		if (pConfig != NULL)
		{
			EntityFlags* pFlag = pConfig->pflags;
			if (pFlag->CanAlwaysEnter)
				bCanAlwaysEnter = true;

			nDir = (INT_PTR)(pConfig->btDir != 255? pConfig->btDir : -1);
		}

	}
	else if(nEntityType == enActor || nEntityType == enHero || nEntityType == enPet)
	{
		bCanAlwaysEnter = true; //玩家进入场景是要强制进入的
	}
	if(!isActor && m_Grid.GetMoveablePoint(pEntity,this,nPosX,nPosY,nPosX,nPosY,bCanAlwaysEnter) == false)
	{
		OutputMsg(rmError, _T("%s:Entity[%s] Grid can't get moveable point; X:%d,Y:%d"), __FUNCTION__, pEntity->GetEntityName(),(int)nPosX,(int)nPosY);
		return false;
	}
	else
	{
		if (!m_Grid.CanAddEntity(pEntity,nPosX,nPosY,true,bCanAlwaysEnter))
		{
			OutputMsg(rmError, _T("%s:Entity[%s] 当前点不能站人; X:%d,Y:%d"), __FUNCTION__, pEntity->GetEntityName(),(int)nPosX,(int)nPosY);
			return false;
		}
		else if( isActor && !CanEnter( (CActor*)pEntity,nPosX,nPosY,bState))
		{
			OutputMsg(rmError, _T("%s:Actor[%s] 不满足进入场景条件; X:%d,Y:%d"), __FUNCTION__, pEntity->GetEntityName(),(int)nPosX,(int)nPosY);
			return false;
		}
	}

	// 先发一次属性变化
	if (isActor)
		((CActor *)pEntity)->GetObserverSystem()->UpdateActorEntityProp();
	
	// 跨场景及跨格子处理
	CFuBen* pFb = pEntity->GetFuBen();
	int nOldX=0,nOldY=0;
	pEntity->GetPosition(nOldX, nOldY);
	if(pLastScence != this)
	{
		bool CanPass = true;
		if(  isActor )
		{
			CanPass = CanPassScence((CActor*)pEntity);
		}
		
		if(CanPass == false) return false;

		if (pLastScence) pLastScence->ExitScene(pEntity);//会在原来的网格删除这个
		if (m_Grid.MoveTo(pEntity,nPosX,nPosY,true,bCanAlwaysEnter) != seSucc)
		{
			OutputMsg(rmNormal, _T("------Grid::MoveTo failed %s posX:%d posY:%d"), pEntity->GetEntityName(), (int)nPosX, (int)nPosY);
			return false;
		}

		if (pEntity->IsInited() && nEntityType == enPet)
		{
			if (CActor* pMaster = ((CPet *)pEntity)->GetMaster())
			{
				((CActor *)pMaster)->GetObserverSystem()->EntityAppear(pEntity->GetHandle(), pEntity);
			}
		}
	}
	else
	{
		if (/*isInObserverList || */(pLastFuben == GetFuBen() && pLastScence == this))
		{
			if (!((CAnimal*)pEntity)->GetMoveSystem()->InstantMove(nPosX,nPosY,1,nType,nValue))
				return false;
		}
		else
		{
			if (m_Grid.MoveTo(pEntity,nPosX,nPosY,false,bCanAlwaysEnter) != seSucc) 
				return false;
		}
	}
	
	if (isActor && (pLastFuben != GetFuBen() || pLastScence != this))
	{
		((CActor *)pEntity)->OnEnterScene(nType, nValue, nParam);
	}

	// 记录实例链表
	if(pLastScence != this)
	{
		switch(nEntityType)
		{
		case enActor:
			m_PlayList.linkAtLast(pEntity->GetHandle());
			if(pLastScence)
				((CActor *)pEntity)->NeedBroadSceneEffect(GetSceneId(),pLastScence->GetSceneId());
			break;
		case enMonster:
		case enGatherMonster:
			((CMonster*)pEntity)->SetBornPoint(nPosX,nPosY);
			pEntity->SetDir(nDir); 
#ifndef MONSTER_MANAGER_BY_SCENE
			m_MonsterList.linkAtLast(pEntity->GetHandle());
#else
			m_MonsterList[pEntity->GetHandle()] = (CMonster*)pEntity;
#endif
			if (m_pFb)
			{
				m_pFb->AddMonsterTotal();//统计副本中的怪物总数
			}
			break;
		case enTransfer:
			m_TeleportList.linkAtLast(pEntity->GetHandle());
			break;
		case enNpc:
			m_NpcList.linkAtLast(pEntity->GetHandle());
			break;
		case enPet:
			m_petList.linkAtLast(pEntity->GetHandle());
			break;
		case enHero:
			m_heroList.linkAtLast(pEntity->GetHandle());
			break;
		case enDropItem:
			m_dropItemList.linkAtLast(pEntity->GetHandle()); 
			break;
		case enFire:
			m_fireList.linkAtLast(pEntity->GetHandle());
			break;
		}

		
	}

	 CFuBen *pFB = GetFuBen();
	 if (pLastScence != this && pFB)
	 	pFB->UpdateFbPlayerCountAndAverageLevel(pEntity, true);
		 
	// // 事件处理
	// if (pFB && pFB->IsFb() && nEntityType != enNpc && pLastFuben != GetFuBen())
	// {
	// 	if (pLastScence != this)
	// 		pFB->UpdateFbPlayerCountAndAverageLevel(pEntity, true);

	// 	// 进入副本事件
	// 	CScriptValueList paramList;
	// 	paramList << (int)CFubenSystem::enOnEnter;
	// 	paramList << (int)pFB->GetConfig()->nFbConfType;
	// 	paramList << (int)pFB->GetFbId();
	// 	paramList << (int)pFB->GetConfig()->nFbEnterType;
	// 	paramList << pFB;
	// 	paramList << (int)GetSceneId();
	// 	paramList << pEntity;
	// 	if(!GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().CallModule("FubenDispatcher","OnEvent", paramList, paramList, 1))
	// 	{
	// 		OutputMsg(rmError,"[FubenSystem] OnEvent OnEnter 错误，副本ID=%d，副本类型=%d !",pFB->GetFbId(),pFB->GetConfig()->nFbConfType);
	// 	}
	// }

	return true;	
}

bool CScene::EnterSceneRandPt(CEntity *pEntity, INT_PTR nPosX, INT_PTR nPosY, INT_PTR nWeight, INT_PTR nHeight, int nType ,
	int nValue, int nParam)
{
	if (0 == nWeight && 0 == nHeight)
	{
		return EnterScene(pEntity, nPosX, nPosY, nType,nValue,nParam);
	}

	//选择随机点传送
	int nMinX = (int)__max(nPosX - nWeight, 0);
	int nMaxX = (int)__min(nPosX + nWeight, m_Grid.m_nCol-1);
	int nMinY = (int)__max(nPosY - nHeight, 0);
	int nMaxY = (int)__min(nPosY + nHeight, m_Grid.m_nRow-1);

	int nX, nY = 0;
	if (!GetRandomPoint(pEntity, nMinX, nMinY, nMaxX, nMaxY, nX, nY))
	{
		//OutputMsg(rmError, "%s error! Can not find pos to move!", __FUNCTION__);
		//return false;
		nX = (int)nPosX;
		nY = (int)nPosY;
	}

	return EnterScene(pEntity, nX, nY,nType,nValue,nParam);
}

void CScene::ExitScene(CEntity * pEntity)
{
	INT_PTR nEntityType = pEntity->GetType();
	unsigned int nBornPointID;
	int nEntityId;

	CFuBen *pFuBen = ((CActor*)pEntity)->GetFuBen();
	int nFubenId = 0;
	if( pFuBen )
	{
		nFubenId = pFuBen->GetFbId();
	}

	switch(nEntityType)
	{
	case enActor:		
		((CActor*)pEntity)->OnPreExitScene(); //exit之前，要做
		RemoveEntity(pEntity);
		break;
	
	//新的实体
	case enNpc:
	case enPet:
	case enHero:
	case enFire:
		RemoveEntity(pEntity);
		break;
	
	case enMonster:
	case enGatherMonster:
		//当怪物（实体）死亡时，通知刷怪点
		nBornPointID = pEntity->GetProperty<unsigned int>(PROP_MONSTER_BORNPOINT);
		nEntityId = pEntity->GetProperty<int>(PROP_ENTITY_ID);
		if (m_pSceneData && nBornPointID >=0 && nBornPointID < (unsigned int)m_pSceneData->vRefreshList.nCount)
		{
			REFRESHCONFIG* rc = m_pSceneData->vRefreshList.pList + nBornPointID;
			if (rc->nEntityId == nEntityId)//怪物id要相等
			{
				REFRESHVALUE* refresh = m_vRefreshMonster+ nBornPointID;	
				if(refresh->nCurrCount >0)
				{
					refresh->nCurrCount--;
				}
				
			}
		} 
		RemoveEntity(pEntity);
		break;
	}

	m_Grid.DeleteEntity(pEntity);
	if (pEntity->GetType() == enActor)
	{
		if (CAnimal* pAnimal = dynamic_cast<CAnimal*>(pEntity))
			pAnimal->GetObserverSystem()->ClearEntityList();
	}
}

void CScene::RefreshMonster()
{
	TICKCOUNT nCurrent = GetLogicCurrTickCount(); //当前的tickcount

	int nCount = m_pSceneData ? m_pSceneData->vRefreshList.nCount:0;
	CMonsterProvider& md = GetLogicServer()->GetDataProvider()->GetMonsterConfig();
	INT_PTR nUserCount = GetGlobalLogicEngine()->getOnlinceCount();
	for(int i = 0; i < nCount; i++)
	{
		//检查每个刷怪点
		REFRESHCONFIG* pRefreshConf = m_pSceneData->vRefreshList.pList + i;				// 刷怪点静态配置
		REFRESHVALUE*  pData = m_vRefreshMonster + i;									// 刷怪点动态数据（当前刷怪点下次刷怪时间、当前怪物数量）
		PMONSTERCONFIG pMonsterConfig = md.GetMonsterData(pRefreshConf->nEntityId);		// 怪物配置信息
		if (!pMonsterConfig) continue;		

		if(pData->nNextTime > nCurrent)
		{
			continue;
		}
		
		// 怪物的数目根据在线总人数计算
		int nCreateCount = (int)GetRefreshMonsterCount(pRefreshConf, pMonsterConfig, nUserCount);
		nCreateCount = nCreateCount - pData->nCurrCount;//需要补刷的数量
		//unsigned long pointCount = pRefreshConf->nPointsCount;
		if (/*pointCount > 0 &&*/ nCreateCount > 0 )
		{
			//unsigned long seed = 0;
			for(; nCreateCount > 0; nCreateCount-- )
			{
				//seed = wrand(pointCount);
				INT_PTR nMonsterPosX = 0;//pRefreshConf->Points[seed].x;
				INT_PTR nMonsterPosY = 0;//pRefreshConf->Points[seed].y;
				if (!GetMobMonsterXY(pRefreshConf,nMonsterPosX,nMonsterPosY))
				{
					continue;
				}
				CEntity* pEntity = CreateEntityAndEnterScene(pRefreshConf->nEntityId, 
															pMonsterConfig->btEntityType,
															nMonsterPosX,nMonsterPosY,i,NULL,
															pRefreshConf->nLiveTime);
				if (pEntity != NULL)
				{
					pData->nCurrCount++;
					//SetMonsterConfig(pEntity, i, pRefreshConf->nEntityId);
				}
			}	
		}
	}
}

INT_PTR CScene::GetRefreshMonsterCount( REFRESHCONFIG* pRefreshConf, PMONSTERCONFIG pMC, INT_PTR nOnlinePlayerCnt)
{
	static const INT_PTR nCountBase = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nOnlineCountBase;	//以1000人为基数
	INT_PTR nResult = pRefreshConf->nCount;
	if (m_pFb && m_pFb->IsFb()) 
		return nResult;

	if (MONSTERCONFIG::mtBoss == pMC->btMonsterType || MONSTERCONFIG::mtToumu == pMC->btMonsterType || pMC->pflags->DenyAddNumber)
		return nResult;

	INT_PTR nRestCount = nOnlinePlayerCnt - nCountBase;
	if (nRestCount > 0)
	{
		INT_PTR nAddCount = (INT_PTR)((float)(pRefreshConf->nCount) * ( (float)nRestCount / (float)nCountBase ));
		nAddCount = nAddCount <= 0?1:nAddCount;
		nAddCount = nAddCount > pRefreshConf->nCount?pRefreshConf->nCount:nAddCount;//最多增加1倍
		nResult = pRefreshConf->nCount + nAddCount;
	}
	return nResult;
}

int CScene::GetFlushTime(int nMonsId, TICKCOUNT nCurTick,bool bsec,bool bDie)
{
	int nCount = m_pSceneData ? m_pSceneData->vRefreshList.nCount : 0;
	for (int i = 0; i < nCount; i++)
	{
		REFRESHCONFIG * pRefreshCfg = m_pSceneData->vRefreshList.pList + i;
		REFRESHVALUE * pRefreshVal = m_vRefreshMonster + i;
		if(NULL == pRefreshCfg || NULL == pRefreshVal)
			return 0;
		int nDeadDigSec = 0;
		if (nMonsId == pRefreshCfg->nEntityId)
		{
			if (pRefreshVal->nCurrCount > 0 || pRefreshVal->nNextTime <= nCurTick)
			{
				// if(bsec &&  GetBossIsDieById(nMonsId))		//boss之家怪物死亡尸体还在
				// {
				// 	nDeadDigSec = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nBossDeadRemDig;
				// }
				
			}
			int nFlushTime = (int)((pRefreshVal->nNextTime - nCurTick)/60000);
			if(bsec)
			{
				nCurTick = GetLogicCurrTickCount();
				nFlushTime = (int)((pRefreshVal->nNextTime - nCurTick)/1000);
			}
			if (nFlushTime <= 0)
			{
				return 1;
			}
			nFlushTime += nDeadDigSec;
			return nFlushTime;
		}
	}
	return 0;
}

bool CScene::GetBossIsDieById(int nMonsId)
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	INT_PTR nDeathCount = m_DeathList.count();
	for(INT_PTR i= nDeathCount -1; i >=0; i--)
	{
		EntityHandle hd =m_DeathList[i];
		CEntity* pEntity = pEntityMgr->GetEntity(hd);
		if(pEntity)
		{
			if (enMonster == pEntity->GetType())
			{
				CMonster* pMonster = (CMonster*)pEntity;
				if(pMonster->GetId() == nMonsId)
					return true;
			}
		}
	}
	return false;
}

bool CScene::RunOne()
{
	DECLARE_TIME_PROF("CScene::RunOne");
	TICKCOUNT dwCurr = _getTickCount(); //这里要使用当前的逻辑时间	

	//清除掉死亡的怪物
	if (m_5s.CheckAndSet(dwCurr))
	{
		//检查死亡的实体
		UINT nNow = GetGlobalLogicEngine()->getMiniDateTime();
		CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
		//CLinkedNode<EntityHandle> *pNode;
		//CLinkedListIterator<EntityHandle> it(m_DeathList);
		INT_PTR nDeathCount = m_DeathList.count();
		for(INT_PTR i= nDeathCount -1; i >=0; i--)
		{
			EntityHandle hd =m_DeathList[i];

			CEntity* pEntity = pEntityMgr->GetEntity(hd);
			if (pEntity)
			{
				CCreature* pCreature = ((CCreature*)pEntity);
				//这里的实体居然是活的
				//if (pCreature->GetDeathTime() < nNow)
				//{
				if (enMonster == pEntity->GetType() || enGatherMonster == pEntity->GetType())
				{
					CMonster* pMonster = (CMonster*)pEntity;
					pMonster->OnEntityDestroy();
					pEntityMgr->DestroyEntity(hd);					
				}
				else
				{
					pEntityMgr->DestroyEntity(hd);
				}
				m_DeathList.remove(i);
				//}
			}
			else
			{
				OutputMsg(rmWaning,_T("CScene::RunOne() Destroy death entity entity is NULL,Hd=%lld"),(Uint64)hd);
				m_DeathList.remove(i);
			}

		}
		int x = 0; 
		int y = 0;
		GetAreaCenter(x, y);
		SCENEAREA* pArea = GetAreaAttri(x,y);
		int nNowTime = time(NULL);
		if ((pArea && HasMapAttribute(x, y, aaOverTimeTick))) {
			if(nNowTime >= m_nRefreshtime)
			{
				if(m_nRefreshtime)
					KickOutAllActor();
				int nNowTime = time(NULL);
				int nNextTime = 0;
				AreaAttri& attri = pArea->attri[aaOverTimeTick];
				if ( attri.nCount > 0) {
					for (INT_PTR i = 0; i < attri.nCount; i++)
					{

						int nTime = getNowZeroTime() +attri.pValues[i];
					
						if(nNowTime >= nTime)
						{
							int nextDay = getNowZeroTime() + attri.pValues[i] + 24*3600;
							if(nNextTime == 0)
								nNextTime = nextDay;
							
							if(nextDay < nNextTime)
								nNextTime = nextDay;
						}
						else{
							nNextTime = nTime;
							break;
						}
					}
					m_nRefreshtime = nNextTime;
				}
			}
		}
		CreateWaitNpc();
	}
	
	//本函数主要执行刷怪的逻辑，玩家数量大于0才刷怪 （改为主场景才这样刷怪，具体副本具体实现）
	if ((!m_pFb->IsFb() ||m_pFb->is_use_default_create_monster()) && m_PlayList.count() > 0)
	{
		if (m_500msTimer.CheckAndSet(dwCurr))
		{
			CLogicDataProvider *pProvider = GetLogicServer()->GetDataProvider();
			static int nTotalTime = pProvider->GetGlobalConfig().nSceneRefreshMonsterTime;

			int nCount = m_pSceneData?m_pSceneData->vRefreshList.nCount:0;
			INT_PTR nUserCount = GetGlobalLogicEngine()->getOnlinceCount();
			for(int i = 0; i < nCount; i++)
			{
				if (i != CFuBenManager::m_RefreshPara.nRefreshPos)
					continue;

				//检查每个刷怪点
				REFRESHCONFIG* pRefreshConf = m_pSceneData->vRefreshList.pList + i;
				REFRESHVALUE*  pData = m_vRefreshMonster + i;
				PMONSTERCONFIG pMonsterConfig = pProvider->GetMonsterConfig().GetMonsterData(pRefreshConf->nEntityId);
				if (!pMonsterConfig) continue;

				if (pData->nNextTime < dwCurr)//到刷怪的时候了
				{				
					//怪物的数目根据在线总人数计算
					int nCreateCount = (int)GetRefreshMonsterCount(pRefreshConf, pMonsterConfig, nUserCount);
					nCreateCount = nCreateCount - pData->nCurrCount;//需要补刷的数量
					if (nCreateCount > 0)
					{
						for(; nCreateCount > 0; nCreateCount-- )
						{							
							//seed = wrand(pointCount);
							INT_PTR nMonsterPosX = 0;//pRefreshConf->Points[seed].x;
							INT_PTR nMonsterPosY = 0;//pRefreshConf->Points[seed].y;
							if (!GetMobMonsterXY(pRefreshConf,nMonsterPosX,nMonsterPosY))
							{
								continue;
							}
							//这里属于配置有问题
							switch (pMonsterConfig->btEntityType )
							{
							case enActor:
							case enHero:
							case enPet:
								continue;
							}
							CEntity* pEntity = CreateEntityAndEnterScene(pRefreshConf->nEntityId, 
								pMonsterConfig->btEntityType, nMonsterPosX,nMonsterPosY, i, NULL, pRefreshConf->nLiveTime);
							
							if (pEntity != NULL)
							{
								pData->nCurrCount++;
							}
						}
					}
					
					pData->nNextTime = pRefreshConf->nNextRefreshTime + dwCurr;
					
					if ((_getTickCount() - dwCurr ) > nTotalTime)
					{
						OutputMsg(rmTip,_T("场景%d第%d条刷怪时间%d过长"),GetSceneId(), i,_getTickCount() - dwCurr);
						return true;
					}
				}

				CFuBenManager::m_RefreshPara.nRefreshPos = i + 1;
			}
			
			// GetGlobalLogicEngine()->GetBossMgr().CreateBossEntity(this);
		}

		if (m_1s.CheckAndSet(dwCurr))
		{
			bool isFb = GetFuBen()->IsFb();

			//判断场景时间是否到，如果到的话触发脚本事件，脚本将所有玩家踢出
			if ( isFb && m_restTime != 0 && GetGlobalLogicEngine()->getMiniDateTime() > m_restTime )
			{
				m_restTime = 0;
			}
		}	
	}

	if (m_PlayList.count() > 0)
	{
		#ifndef MONSTER_MANAGER_BY_SCENE
		#else
			CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
			while (m_MonInterator != m_MonsterList.end())
			{
				CEntity* pEntity = pEntityMgr->GetEntity(m_MonInterator->first);
				pEntity->RunOne(dwCurr);
				if( _getTickCount() - dwCurr > 4 ) //每个地图的怪物逻辑遍历不要超过4毫秒
				{
					if (m_MonInterator != m_MonsterList.end()) ++m_MonInterator;
					return true;
				}
				if (m_MonInterator != m_MonsterList.end()) ++m_MonInterator;
			}
			if (m_MonInterator == m_MonsterList.end())
			{
				m_MonInterator = m_MonsterList.begin();
			}
		#endif
	}

	return false;
}

void CScene::Trace()
{
	TICKCOUNT nTick = GetGlobalLogicEngine()->getTickCount();
	TICKCOUNT nLastRun = m_5s.GetNextTime();  //上次刷新的时间
	int tickDis =(int)(nTick - nLastRun) ;
	if(tickDis < 0 )
	{
		tickDis =0;
	}
	
	OutputMsg(rmNormal,"name=%s,death=%d,playcount=%d,rundis=%d ms",
		GetSceneName(),(int)m_DeathList.count(),(int)m_PlayList.count(),
		tickDis);

}

void CScene::_AddDeathEntity(CEntity* pEntity, LPCSTR file, INT_PTR line)
{
	if (!pEntity) return;

	bool boAddToList = true;
	INT_PTR entityType = pEntity->GetType();

	//尝试去销毁npc
	if(entityType == enNpc)
	{
		OutputMsg(rmWaning,"%s Try to destroy npc %s",__FUNCTION__,pEntity->GetEntityName());
		return ;
	}
	else if (enActor == entityType || enHero == entityType || enPet == entityType )
	{
		return;//玩家死亡不在这里处理
	}
	else if(entityType == enFire)
	{
		GetGlobalLogicEngine()->GetEntityMgr()->DestroyEntity(pEntity->GetHandle());
		return;
	}	
	else if (enMonster == entityType || enGatherMonster == entityType)
	{
		// CMonster *pMonster =(CMonster*)pEntity;
		// if(pMonster->GetMonsterType() == MONSTERCONFIG::mtBoss)
		// {
		// 	if(pMonster->GetBossReferId() > 0)
		// 	{
		// 		GetGlobalLogicEngine()->GetBossMgr().BossCallLuaScript(pMonster->GetBossReferId() , CBossSystem::enOnBossDeath);
		// 	}
		// }
		if(pEntity->GetAttriFlag().CanReuse)
		{
			unsigned int nBornPointID = pEntity->GetProperty<unsigned int>(PROP_MONSTER_BORNPOINT);
			boAddToList = false;
			CEntityMsg msg(CEntityMsg::emReUseMonster, pEntity->GetHandle());
			msg.dwDelay = 5000;
			pEntity->PostEntityMsg(msg);
			
			if (m_pSceneData && nBornPointID >=0 && nBornPointID < (unsigned int)m_pSceneData->vRefreshList.nCount)
			{
				REFRESHVALUE* refresh = m_vRefreshMonster+ nBornPointID;
				refresh->nCurrCount++;
			}
			return;
		}
		
	}
	
	if (boAddToList && pEntity->isAnimal())
	{		
		CCreature* pCreature = ((CCreature*)pEntity);
		if(pCreature->IsInDeathList()) return; //已经在场景里了
		pCreature->SetDestoryTime(DEATHTIME);		// 所有实体删除事件固定2秒?
		//m_DeathList.linkAtLast(pEntity->GetHandle());
		
		m_DeathList.add(pEntity->GetHandle());

		if(pEntity->GetType() == enMonster  && GetFuBen()->GetFbId()==0 )
		{
			CMonster *pMonster =(CMonster*)pEntity;
			if(pMonster->GetMonsterType() == MONSTERCONFIG::mtBoss)
			{
				OutputMsg(rmNormal, "%s liveTime is over,_AddDeathEntity bossLiveTime = %d,file=%s,line=%d", (char*)pMonster->GetEntityName(),(int)pMonster->GetLiveTime(),
					(char*)file,(int)line);
			}
		}
	}

}


CEntity* CScene::CreateEntityAndEnterScene(INT_PTR nEntityId, 
										  INT_PTR nType,
										  INT_PTR nX,
										  INT_PTR nY,
										  INT_PTR nParam,
										  CActor* pOnwer,
										  INT_PTR nLiveTime,
										  INT_PTR nBornLevel,
										  char* szMonsterName,
										  INT_PTR nForceVesterId,
										  INT_PTR nInitMonsterPercent
										  )
{

	EntityHandle hHandle;
	CEntity* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->CreateEntity((int)nType, hHandle);	
	if (pEntity)
	{
		pEntity->SetProperty<unsigned int>(PROP_ENTITY_ID, (unsigned int)nEntityId) ;
		if (EnterScene(pEntity, nX, nY))
		{
			if (enPet == nType)
			{
				((CPet*)pEntity)->SetMaster(pOnwer); //宠物的主人
				//设置等级，因为需要这个刷属性的
				((CPet*)pEntity)->SetProperty<unsigned int>(PROP_CREATURE_LEVEL, (unsigned int)nParam) ;
			}
			else if(enHero == nType) 
			{
				((CHero*)pEntity)->SetMaster(pOnwer); //宠物的主人
				((CPet*)pEntity)->SetProperty<unsigned int>(PROP_CREATURE_LEVEL, (unsigned int)nParam) ;
			}
			else if(enMonster == nType)
			{
				pEntity->SetProperty<unsigned int>(PROP_MONSTER_INITHPPERCENT,(unsigned int)nInitMonsterPercent);
			}

			if (!pEntity->Init(NULL, 0))
			{
				GetGlobalLogicEngine()->GetEntityMgr()->DestroyEntity(hHandle);
				pEntity =NULL;
			}
			else
			{
				if (enMonster == nType || enGatherMonster == nType)
				{
					PMONSTERCONFIG pMonConf = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nEntityId);
					// 怪物刷新脚本处理(当前的处理方式是通过对需要执行脚本怪物在刷新的时候执行registerMonsterMob函数设置此变量。并非在怪物中支持配置)
					if (pMonConf && pMonConf->pflags->bMobTriggerScript)
					{
						pEntity->SetEntityName((char*)pMonConf->szName);
						CScriptValueList arg;
						arg <<  pEntity;			
						arg << nEntityId;
						arg << this->GetSceneId();
						arg << nX;
						arg << nY;
						GetGlobalLogicEngine()->GetMonFuncNpc()->GetScript().Call("OnMonsterMob", arg, arg, 0);
					}

					if( pMonConf->btMonsterType == tagMonsterConfig::mtBoss )		//BOSS创建，记录日志
					{
						if( GetLogicServer()->GetLocalClient() )
						{
						}
					}
					
					SetMonsterConfig(pEntity, nParam,nEntityId,nBornLevel);
					CMonster *pMonster = ((CMonster*)pEntity);
					pMonster->SetLiveTime((unsigned int)nLiveTime);		//设置生命周期
					pMonster->SetProperty<int>(PROP_MONSTER_LIVE_TIMEOUT, pMonster->GetLiveTime() );  //怪物生命到期时间
					
					if(pOnwer)
					{
						((CMonster*)pMonster)->SetOwner(pOnwer->GetHandle());
					}
					CSkillSubSystem & skillSystem =  pMonster->GetSkillSystem();
					const CVector<CSkillSubSystem::SKILLDATA> & skills= skillSystem.GetSkills();
					for(INT_PTR i=0; i< skills.count();i++)
					{
						if(skills[i].bEvent == mSkillEventBorn)
						{
							skillSystem.LaunchSkill(skills[i].nSkillID,0,0,true);
						}
					}

					if (enMonster == nType || enGatherMonster == nType)
					{
						((CMonster *)pEntity)->MonsterSay(mstCreate);
					}

					if( enMonster == nType)
					{
						CMonster *pMonster = (CMonster *)pEntity;
						pMonster->SetForceVesterId((unsigned int)nForceVesterId);				//设置怪物的强制归属
						if( szMonsterName )
						{
							//pMonster->ResetShowName(szMonsterName);
							pMonster->ChangeShowName(12, szMonsterName);
						}
						if (pMonster->GetMonsterType() == MONSTERCONFIG::mtBoss && GetFuBen()->GetFbId()==0)
						{

							OutputMsg(rmTip,"Boss Enter scene %s,scene=%s,x=%d,y=%d",pMonster->GetEntityName(),GetSceneName(),(int)nX,(int)nY);

						}
						if (pMonster->GetAttriFlag().DenyAttackedByActor)
						{
							pMonster->AddState(esStateDenyAttackedByActor);
						}
					}

				}
			}
		}
		else
		{	
			GetGlobalLogicEngine()->GetEntityMgr()->DestroyEntity(hHandle);
			pEntity = NULL;
		}
	}
	
	return pEntity;
}

bool CScene::GetSeqVisibleList(int nPosX,int nPosY, SequenceEntityList& visibleList, int nLeft,int nRight,int nUp,int nDown)
{
	int nMinX = __max(nPosX + nLeft,0);
	int nMaxX = __min(nPosX + nRight,m_Grid.m_nCol-1);
	int nMinY = __max(nPosY + nUp,0);
	int nMaxY = __min(nPosY + nDown,m_Grid.m_nRow-1);
	visibleList.clear();

	typedef typename EntityPosVector::iterator Iter;

	for (int j = nMinX; j <= nMaxX; j++)
	{	
		MapEntityGrid* pGrid = m_Grid.GetList(j);
		if (pGrid)
		{
			EntityPosVector& entitys = pGrid->m_entitys;

#ifndef GRID_USE_STL_MAP
			for (INT_PTR i = 0; i < entitys.count(); i++)
			{
				int nPosY = entitys[i].pos;
				if(nPosY >= nMinY && nPosY <= nMaxY )
				{
					if(CEntity *pEntity = GetEntityFromHandle(entitys[i].hd))
					{
						if (pEntity->IsInited())
							visibleList.insert(entitys[i].hd);
					}
				}

			}
#else
			
			Iter it;
			Iter end = entitys.end();
			for (it = entitys.begin(); it != end; it++)
			{
				int nPosY = (*it).second;
				if(nPosY >= nMinY && nPosY <= nMaxY )
				{
					if(CEntity *pEntity = GetEntityFromHandle((*it).first))
					{
						if (pEntity->IsInited())
							visibleList.insert((*it).first);
					}
				}
			}
#endif
		}
	}

	
	for (int j = 0, nMaxXPos = GetSceneWidth(); j <= nMaxXPos; j++)
	{	
		MapEntityGrid* pGrid = m_Grid.GetList(j);
		if (pGrid)
		{
			EntityPosVector& entitys = pGrid->m_entitys;

#ifndef GRID_USE_STL_MAP
			for (INT_PTR i = 0; i < entitys.count(); i++)
			{
				if(CEntity *pEntity = GetEntityFromHandle(entitys[i].hd))
				{
					int nPostionY = entitys[i].pos;
					if (pEntity->IsInited())
					{
						//次元Boss归属玩家,无论是否在视野中都可见
						if ( pEntity->GetType() == enActor && ((CActor *)pEntity)->IsBeLongBossActor() && pEntity->GetFuBen() && pEntity->GetFuBen()->GetFbId() == 23 && pEntity->GetSceneID() == 317 && pEntity->GetProperty<unsigned int>(PROP_CREATURE_HP) > 0 )
						{
							visibleList.insert(entitys[i].hd);
						}

						//次元Boss怪物,无论是否在视野中都可见
						if ( pEntity->GetType() == enMonster && pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID) == 386 && pEntity->GetFuBen()->GetFbId() == 23 && pEntity->GetSceneID() == 317 && pEntity->GetProperty<unsigned int>(PROP_CREATURE_HP) > 0 )
						{
							visibleList.insert(entitys[i].hd);
						}
					}
				}
			}
#else
			
			Iter it;
			Iter end = entitys.end();
			for (it = entitys.begin(); it != end; it++)
			{
				if(CEntity *pEntity = GetEntityFromHandle((*it).first))
				{
					if (pEntity->IsInited())
					{
						//次元Boss归属玩家,无论是否在视野中都可见
						if ( pEntity->GetType() == enActor && ((CActor *)pEntity)->IsBeLongBossActor() && pEntity->GetFuBen() && pEntity->GetFuBen()->GetFbId() == 23 && pEntity->GetSceneID() == 317 && pEntity->GetProperty<unsigned int>(PROP_CREATURE_HP) > 0 )
						{
							visibleList.insert((*it).first);
						}

						//次元Boss怪物,无论是否在视野中都可见
						if ( pEntity->GetType() == enMonster && pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID) == 386 && pEntity->GetFuBen()->GetFbId() == 23 && pEntity->GetSceneID() == 317 && pEntity->GetProperty<unsigned int>(PROP_CREATURE_HP) > 0 )
						{
							visibleList.insert((*it).first);
						}
					}
				}
			}
#endif
		}
	}

	return true;
}


bool CScene::GetVisibleList(int nPosX,int nPosY, 
							CObserverEntityList& vVisibleList, 
							int nLeft, 
							int  nRight, 
							int nUp, 
							int nDown, 
							bool *pbActorInVisibleList)
{
	int nMinX = __max(nPosX + nLeft,0);
	int nMaxX = __min(nPosX + nRight,m_Grid.m_nCol-1);
	int nMinY = __max(nPosY + nUp,0);
	int nMaxY = __min(nPosY + nDown,m_Grid.m_nRow-1);	
	vVisibleList.clear();
	bool bActorInVisibleList = false;
	for (int j = nMinX; j <= nMaxX; j++)
	{
		MapEntityGrid* pGrid = m_Grid.GetList(j);
		if (pGrid)
		{
			EntityPosVector& entitys = pGrid->m_entitys;

#ifndef GRID_USE_STL_MAP
			for (INT_PTR i = 0; i < entitys.count(); i++)
			{
				if (!bActorInVisibleList && entitys[i].hd.GetType() == enActor)
					bActorInVisibleList = true;
				int nPosY = entitys[i].pos;
				if( nPosY >= nMinY && nPosY <= nMaxY)
				{
					EntityHandleTag entityTag;
					entityTag.m_handle	= entitys[i].hd;
					entityTag.m_tag		= eENTITY_APPEAR_UNCHANGE;	
					vVisibleList.linkAtLast(entityTag);
				}

			}
#else
			typedef typename EntityPosVector::iterator Iter;
			Iter it = entitys.begin(), end = entitys.end();
			for (; it != end; it++)
			{
				if (!bActorInVisibleList && (*it).first.GetType() == enActor)
					bActorInVisibleList = true;

				int nPosY = (*it).second;
				if( nPosY >= nMinY && nPosY <= nMaxY)
				{
					EntityHandleTag entityTag;
					entityTag.m_handle	= (*it).first;
					entityTag.m_tag		= eENTITY_APPEAR_UNCHANGE;	
					vVisibleList.linkAtLast(entityTag);
				}
			}
#endif
		}
	}
	if (pbActorInVisibleList)
		*pbActorInVisibleList = bActorInVisibleList;
	return true;
}

bool CScene::GetVisibleList(CEntity * pEntity, EntityVector& vVisibleList, int nLeft, int nRight, int nUp, int nDown)
{
	if(pEntity == NULL) return false;
	int nPosX,nPosY;
	pEntity->GetPosition(nPosX,nPosY);
	return GetVisibleList(nPosX,nPosY,vVisibleList,nLeft,nRight,nUp,nDown);
}

bool CScene::GetVisibleList(int nPosX,int nPosY, EntityVector& vVisibleList, int nLeft, int nRight, int nUp, int nDown)
{	
	int nMinX = __max(nPosX + nLeft,0);
	int nMaxX = __min(nPosX + nRight, m_Grid.m_nCol-1);
	int nMinY = __max(nPosY + nUp,0);
	int nMaxY = __min(nPosY + nDown, m_Grid.m_nRow-1);
	vVisibleList.clear();
	int j = 0;
	for (j = nMinX; j <= nMaxX; j++)
	{			
		MapEntityGrid* pGrid = m_Grid.GetList(j);

		if (pGrid)
		{
#ifndef GRID_USE_STL_MAP
			for(INT_PTR i=pGrid->m_entitys.count()-1; i>-1; i-- )
			{
				int nPos =  pGrid->m_entitys[i].pos;
				if( nPos>= nMinY && nPos <= nMaxY)
				{
					vVisibleList.add( pGrid->m_entitys[i].hd);
				}
			}
#else
		EntityPosVector& entitys = pGrid->m_entitys;
		typedef typename EntityPosVector::iterator Iter;
		Iter it = entitys.begin(), end = entitys.end();
		for (; it != end; it++)
		{
			int nPos = (*it).second;
			if( nPos>= nMinY && nPos <= nMaxY)
			{
				vVisibleList.add((*it).first);
			}
		}
#endif
		}

	}
	
	return true;
}


void CScene::ClearIncorrectHandle( CEntity * pEntity, int nLeft, int nRight, int nUp, int nDown)
{
	int nPosX,nPosY;
	pEntity->GetPosition(nPosX,nPosY);
	int nMinX = __max(nPosX + nLeft,0);
	int nMaxX = __min(nPosX + nRight,m_Grid.m_nCol-1);
	int nMinY = __max(nPosY + nUp,0);
	int nMaxY = __min(nPosY + nDown,m_Grid.m_nRow-1);


	for (int j = nMinX; j <= nMaxX; j++)
	{
		MapEntityGrid* pGrid = m_Grid.GetList(j);
		if (pGrid)
		{
			CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
			EntityPosVector& entitys = pGrid->m_entitys;

#ifndef GRID_USE_STL_MAP
			for (INT_PTR i = entitys.count()-1; i >= 0; i--)
			{
				int nPosY = entitys[i].pos;
				if(nPosY >= nMinY && nPosY <= nMaxY)
				{
					CEntity* pEntity = pEntityMgr->GetEntity(entitys[i].hd);
					if (!pEntity)
						entitys.remove(i);
				}
			}
#else
			typedef typename EntityPosVector::iterator Iter;
			Iter it = entitys.begin(), end = entitys.end();
			for (; it != end; it++)
			{
				int nPosY = (*it).second;
				if(nPosY >= nMinY && nPosY <= nMaxY )
				{
					CEntity* pEntity = pEntityMgr->GetEntity((*it).first);
					if (!pEntity)
						entitys.erase(it--);
				}
			}
#endif
		}
	}
}

void CScene::SetRefreshList(REFRESHLIST* list)
{
	if (m_vRefreshMonster)
	{
		GFreeBuffer(m_vRefreshMonster);
		m_vRefreshMonster = NULL;
	}
	m_vRefreshMonster = (REFRESHVALUE*)GAllocBuffer(sizeof(REFRESHVALUE)*(list->nCount));
	ResetRefreshList(list);
}

CNpc * CScene::GetNpc(int nNpcId)
{
	CLinkedNode<EntityHandle> * pNode;
	CLinkedListIterator<EntityHandle> it(m_NpcList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CEntity * pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(pNode->m_Data);
		if (pEntity != NULL && pEntity->GetId() == nNpcId)
		{
			return (CNpc *)pEntity;
		}
	}
	return NULL;
}

CNpc* CScene::GetNpc(LPCSTR sName)
{
	if (sName == NULL || sName[0] == 0) return NULL;
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_NpcList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CEntity* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(pNode->m_Data);
		if (pEntity != NULL && strcmp(pEntity->GetEntityName(),sName) == 0)
		{
			return (CNpc*)pEntity;
		}
	}
	return NULL;
}

void CScene::SetNpcCanSeeFlag(bool bSet)
{
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_NpcList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CEntity* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(pNode->m_Data);
		if (pEntity != NULL)
		{
			NPCCONFIG* npcConfig = ((CNpc*)pEntity)->GetConfig();

			if(npcConfig && npcConfig->bCanHideFlag == true)
				((CNpc*)pEntity)->GetAttriFlag().DenySee = bSet;
		}
	}
}

CNpc* CScene::GetNpcAtXY(const INT_PTR nX, const INT_PTR nY)
{
	MapEntityGrid* pGrid = m_Grid.GetList(nX);
	if (pGrid)
	{
		EntityPosVector& entitys = pGrid->m_entitys;

#ifndef GRID_USE_STL_MAP
		for (INT_PTR i = entitys.count()-1; i >-1; i--)
		{			 

			if (entitys[i].hd.GetType() == enNpc && entitys[i].pos == nY)
			{
				return (CNpc*)GetEntityFromHandle(entitys[i].hd); 
			}
		}
#else
		typedef typename EntityPosVector::iterator Iter;
		Iter it = entitys.begin(), end = entitys.end();
		for (; it != end; it++)
		{
			if ((*it).first.GetType() == enNpc && (*it).second == nY)
			{
				return (CNpc*)GetEntityFromHandle((*it).first); 
			}
		}
#endif
	}

	return NULL;
}

int CScene::GetEntityCountAtXY(const int nX, const int nY, const int nEntityType)
{
	int nResult = 0;
	MapEntityGrid* pGrid = m_Grid.GetList(nX);
	if (pGrid)
	{
		EntityPosVector& entitys = pGrid->m_entitys;

#ifndef GRID_USE_STL_MAP
		for (INT_PTR i = 0; i < entitys.count(); i++)
		{
			if (entitys[i].hd.GetType() == nEntityType || nEntityType < 0)
			{
				nResult++;
			}
		}
#else
		typedef typename EntityPosVector::iterator Iter;
		Iter it = entitys.begin(), end = entitys.end();
		for (; it != end; it++)
		{
			if ((*it).first.GetType() == nEntityType || nEntityType < 0)
			{
				nResult++;
			}
		}
#endif
	}
	return nResult;
}

INT_PTR CScene::ReloadNpcScript(LPCTSTR szNpcName)
{
	INT_PTR result = 0;
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_NpcList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
		if (pEntity && pEntity->GetType() == enNpc)
		{
			CNpc* pNpc = ((CNpc*)pEntity);
			NPCCONFIG* pConfig = pNpc->GetConfig();
			if (pConfig && (szNpcName == NULL || strcmp(szNpcName, pConfig->sName)==0))
				result += pNpc->LoadScript(pConfig->sScript,true);
		}
	}
	
	return result;
}

 void CScene::ExitAllActor()
 {
	 if (!m_pFb->IsFb()) return;
	 CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	 CLinkedNode<EntityHandle> *pNode;
	 CLinkedListIterator<EntityHandle> it(m_PlayList);
	 for (pNode = it.first(); pNode; pNode = it.next())
	 {
		 CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
		 if (pEntity && pEntity->GetType() == enActor)
		 {
			((CActor*)pEntity)->ReturnToStaticScene();
		 }
		 m_PlayList.remove(pNode); //修改了一个死循环
	 }
 }

void CScene::SetMonsterConfig( CEntity* pEntity, INT_PTR bornPoint, INT_PTR nMonsterId, INT_PTR nBornLevel )
{
	if (!pEntity)
		return;

	pEntity->SetProperty<unsigned int>(PROP_MONSTER_BORNPOINT,(unsigned int)bornPoint);
	PMONSTERCONFIG pConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nMonsterId);
	if (pConfig)
	{
		if(bornPoint >= 0 && bornPoint < m_pSceneData->vRefreshList.nCount)
		{
			REFRESHCONFIG* pSceneRefreshConfig = m_pSceneData->vRefreshList.pList + bornPoint;			
			if (pSceneRefreshConfig && pSceneRefreshConfig->nNextRefreshTime == 0) //能否被重用
			{
				EntityFlags & flags = pEntity->GetAttriFlag();
				flags.CanReuse =true;
			}
		}


		//pEntity->SetProperty<unsigned int>(PROP_CREATURE_ATTACK_SPEED,)
		//放到属性里去刷
		//pEntity->SetProperty<unsigned int>(PROP_CREATURE_HP,pConfig->cal.m_AttrValues[aMaxHpAdd].uValue);
		//pEntity->SetProperty<unsigned int>(PROP_CREATURE_MP,pConfig->cal.m_AttrValues[aMaxMpAdd].uValue);
		if ( nBornLevel > 0 )
		{
			unsigned int nLevel = (unsigned int)(pConfig->nOriginalLevel+nBornLevel);
			pConfig->nLevel = nLevel;
			pEntity->SetProperty<unsigned int>(PROP_CREATURE_LEVEL,nLevel);
			pEntity->OnLevelUp(nLevel);
		}
		else
		{
			pEntity->SetProperty<unsigned int>(PROP_CREATURE_LEVEL,pConfig->nLevel);
		}
		pEntity->SetProperty<int>(PROP_MONSTER_CIRCLE, pConfig->nCircle);
		((CAnimal*)pEntity)->SetCamp(pConfig->bCamp);
		//pEntity->SetProperty<unsigned int>(PROP_ENTITY_ID,pConfig->nEntityId);
		pEntity->SetEntityName((char*)pConfig->szName);
		pEntity->SetProperty<int>(PROP_ENTITY_MODELID,pConfig->nModelId);
		pEntity->SetProperty<int>(PROP_ENTITY_ID,(int)nMonsterId);
		pEntity->SetInitFlag(true);
	}
	else
	{
		OutputMsg(rmTip,_T("set monster config error"));
	}
}

bool CScene::GetPath(CEntity* pEntity, CVector<SMALLCPOINT> &outPaths, int targetX, int targetY)
{
	if (!pEntity) return false;

#if _DEBUG
	struct timespec tp_begin;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tp_begin);
#endif

	INT_PTR nCurrentPosX = pEntity->GetProperty<int>(PROP_ENTITY_POSX);
	INT_PTR nCurrentPosY = pEntity->GetProperty<int>(PROP_ENTITY_POSY);
	bool ret = m_AStar.getPath(pEntity->GetType(), nCurrentPosX, nCurrentPosY, targetX, targetY, outPaths, this);
	
#if _DEBUG
	struct timespec tp_end;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tp_end);
	int secdiff = tp_end.tv_sec - tp_begin.tv_sec;
	long long nsecdiff = tp_end.tv_nsec - tp_begin.tv_nsec;
	if ((secdiff > 0) || nsecdiff > 100 * 1000 * 1000)
	{
		OutputMsg(rmError,_T("%s %lld 寻路时间太长！sec:%d, nano:%lld"), pEntity->GetEntityName(), (Uint64)(pEntity->GetHandle()), secdiff, nsecdiff);
	}
#endif

return ret;
}


void CScene::SetAreaConfig(CObjectAllocator<char>& dataAlloc)
{
	if (!m_pSceneData)
		return;

	if (m_pSceneData->vAreaList.nCount <= 0)
	{
		m_pMapAreaTable = NULL;
		return;
	}
	
	size_t dwAreaDataSize = sizeof(m_pMapAreaTable[0])*m_nWidth*m_nHeight;
	m_pMapAreaTable = (PBYTE)dataAlloc.allocObjects(dwAreaDataSize);
	ZeroMemory(m_pMapAreaTable, dwAreaDataSize);

	//生成区域地图
	CAreaRegion region;
	region.setSize(m_nWidth, m_nHeight);
	for (int i = 0; i < m_pSceneData->vAreaList.nCount; i++)
	{
		SCENEAREA& area = m_pSceneData->vAreaList.pList[i];
		//直接将区域数据生成到地图区域数据内存中
		if (!region.addArea(area.lpPoints,area.nPointCount, i, m_pMapAreaTable))
		{
			OutputMsg(rmError,_T("generate area error!sceneid=%d,areaid=%d"),GetSceneId(),i);
			continue;
		}
	}
}

void CScene::SetTelePortTable(CObjectAllocator<char>& dataAlloc)
{
	if (!m_pSceneData)
		return;

	size_t nSize = sizeof(m_pTelePortTable[0])*m_nWidth*m_nHeight;
	m_pTelePortTable= (PBYTE)dataAlloc.allocObjects(nSize);
	ZeroMemory(m_pTelePortTable,nSize);

	// 在传送门的周围计算每个点。记录的是传送门的索引（based 1），传送门的xy坐标往外nDist距离的点都算传送点	
	for (int i = 0; i < m_pSceneData->TeleportList.nCount; i++)
	{
		TeleportConfig& config	= m_pSceneData->TeleportList.pList[i];
		int nDist = config.nDist;
		INT_PTR nStartX			= __max(0,config.nPosX - nDist);
		INT_PTR nEndX			= __min(m_nWidth,config.nPosX + nDist);
		INT_PTR nStartY			= __max(0,config.nPosY - nDist);
		INT_PTR nEndY			= __min(m_nHeight,config.nPosY + nDist);
		for (INT_PTR x = nStartX; x <= nEndX; x++)
		{
			for (INT_PTR y = nStartY; y <= nEndY; y++)
			{
				// 要能走的地方才能传送，避免跳跃的时候传送。 这里记录每个格子对应的传送门索引（从1开始）
				if (m_pMapData->canMove((DWORD)x,(DWORD)y))
				{	
					m_pTelePortTable[y*m_nWidth+x] = i+1;
				}
			}
		}
	}
}

bool CScene::HasMapAttribute(INT_PTR nPosX, INT_PTR nPosY, BYTE bType, SCENEAREA* pArea)
{
	if (!m_pSceneData)
		 return false;

	if (aaNoAttri == bType || bType >= aaAttriCount) return false;
	if (!pArea)
	{
		pArea = GetAreaAttri(nPosX, nPosY);
		if (!pArea)
			return false;
	}
	// 感觉这里存在数据冗余。attri本身就是类型的数组，怎么具体的属性数据里头又有bType???
	if (pArea->attri[bType].bType == bType)
	{
		return true;
	}
	return false;
}

bool CScene::HasMapAttribute(INT_PTR nPosX, INT_PTR nPosY, BYTE bType, int value , SCENEAREA* pArea)
{
	if (!m_pSceneData)
		return false;
	
	if (aaNoAttri == bType || bType >= aaAttriCount) return false;
	if (!pArea)
	{
		pArea = GetAreaAttri(nPosX,nPosY);
		if (!pArea)
			return false;
	}
	if (pArea->attri[bType].bType == bType)
	{
		AreaAttri& attri = pArea->attri[bType];
		return binarySearch(attri.pValues,attri.nCount,value);
	}
	
	return false;
 }

bool CScene::GetMapAttriValue(INT_PTR nPosX, INT_PTR nPosY, BYTE bType, int& value, SCENEAREA* pArea)
{
	value = 0;
	if (!m_pSceneData)
		return false;
	
	if (aaNoAttri == bType || bType >= aaAttriCount) return false;
	if (!pArea)
	{
		pArea = GetAreaAttri(nPosX,nPosY);
		if (!pArea)
			return false;
	}
	
	if (pArea->attri[bType].bType == bType)
	{
		AreaAttri& attri = pArea->attri[bType];
		if (attri.nCount > 0)
		{
			value = attri.pValues[0];
			return true;
		}
	}

	return false;
}


SCENEAREA* CScene::GetAreaAttri(INT_PTR x, INT_PTR y)
{
	if (!m_pSceneData || !m_pSceneData->vAreaList.pList)
		return NULL;
	
	if (x <0 || x >= m_nWidth || y < 0 || y >= m_nHeight)
	{
		return NULL;
	}
	if(m_pMapAreaTable ==NULL)
	{
		return NULL;
	}
	BYTE index = m_pMapAreaTable[y * m_nWidth + x];
	if (index >= m_pSceneData->vAreaList.nCount)
	{
		OutputMsg(rmError,_T("map attribut data error!sceneid=%d, x,y=%d,%d, index=%d, count=%d"),
				 m_pSceneData->nScenceId,
				 x,
				 y,
				 index,
				 m_pSceneData->vAreaList.nCount);
		return NULL;
	}
	
	return &(m_pSceneData->vAreaList.pList[index]);
}


int CScene::GetAreaListIndex(INT_PTR x, INT_PTR y)
{
	if (!m_pSceneData || !m_pSceneData->vAreaList.pList)
		return -1;
	
	if (x <0 || x >= m_nWidth || y < 0 || y >= m_nHeight)
	{
		return -1;
	}
	if(m_pMapAreaTable ==NULL)
	{
		return -1;
	}
	BYTE index = m_pMapAreaTable[y * m_nWidth + x];
	if (index >= m_pSceneData->vAreaList.nCount)
	{
		OutputMsg(rmError,_T("map attribut data error!sceneid=%d, x,y=%d,%d, index=%d, count=%d"),
				 m_pSceneData->nScenceId,
				 x,
				 y,
				 index,
				 m_pSceneData->vAreaList.nCount);
		return -1;
	}
	
	return index;
}

void CScene::GetAreaCenter(int& nPosX, int& nPosY)
{
	if (!m_pSceneData || !m_pSceneData->vAreaList.pList)
		return;
	
	if (nPosX < 0 || nPosX >= m_nWidth || nPosY < 0 || nPosY >= m_nHeight)
		return;

	if (!m_pMapAreaTable)
		return;
	
	BYTE index = m_pMapAreaTable[nPosY * m_nWidth + nPosX];
	SCENEAREA& area = m_pSceneData->vAreaList.pList[index];
	nPosX = area.Center[0];
	nPosY = area.Center[1];
 }

void CScene::GetFirstAreaCenterPoint(int &nPosX, int &nPosY)
{
	if (!m_pSceneData || !m_pSceneData->vAreaList.pList)
		return;

	SCENEAREA& area = m_pSceneData->vAreaList.pList[0];
	nPosX = area.Center[0];
	nPosY = area.Center[1];
}

void CScene::GetRangeAreaCrossPoint(CEntity* pEntity, int &nPosX, int& nPosY)
{
	if (!m_pSceneData || !m_pSceneData->vAreaList.pList)
		return;
	int nRandomX, nRandomY = 0;
	SCENEAREA& pArea = m_pSceneData->vAreaList.pList[0];
	if ( pArea.nPointCount >= 3 
		&& GetRandomPoint(pEntity, pArea.lpPoints[0].x, pArea.lpPoints[0].y, pArea.lpPoints[2].x, pArea.lpPoints[2].y, nRandomX, nRandomY))
	{
		nPosX = nRandomX;
		nPosY = nRandomY;
	}
}

bool CScene::IsSameArea(INT_PTR nOldX, INT_PTR nOldY, INT_PTR nNewX, INT_PTR nNewY)
{
	if (nOldX < 0 || nOldX >= m_nWidth || nOldY < 0 || nOldY >= m_nHeight)
	{
		return false;
	}
	if (nNewX < 0 || nNewX >= m_nWidth || nNewY < 0 || nNewY >= m_nHeight)
	{
		return false;
	}

	if (!m_pMapAreaTable)
	{
		return true;
	}
	
	return m_pMapAreaTable[nOldY * m_nWidth + nOldX] == m_pMapAreaTable[nNewY * m_nWidth + nNewX];
}

bool CScene::IsTelePort(INT_PTR nX, INT_PTR nY)
{
	if (nX >= m_nWidth || nY >= m_nHeight)
	{
		OutputMsg(rmError, _T("%s error x:%d,y:%d,real width:%d,height:%d"), __FUNCTION__, (int)nX, (int)nY,(int)m_nWidth,(int)m_nHeight);
		return false;
	}
	return m_pTelePortTable[nY*m_nWidth+nX] != 0;
}
 
int CScene::GetMonsterCount(int nMonsterId, EntityHandle hHandle)
{
	int result = 0;
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
#ifndef MONSTER_MANAGER_BY_SCENE
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_MonsterList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
#else
	std::map<EntityHandle,CMonster*>::iterator iter = m_MonsterList.begin();
	for (; iter != m_MonsterList.end(); ++iter)
	{
		CEntity* pEntity = pEntityMgr->GetEntity(iter->first);
#endif
		if (pEntity)
		{
			if ( (nMonsterId == 0 || pEntity->GetProperty<int>(PROP_ENTITY_ID) == nMonsterId) 
				&& (hHandle == 0 || hHandle == ((CMonster*)pEntity)->GetOwner()) 
				&& !((CMonster*)pEntity)->HasState(esStateDeath) )
			{
				result++;
			}
		}
		else
		{
			OutputMsg(rmError,_T("GetMonsterCount 有一个不存在的实体"));
		}
	}
	return result;
}
void CScene::KillAllMonster(CEntity * pGmEntity)
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
#ifndef MONSTER_MANAGER_BY_SCENE
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_MonsterList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
#else
	std::map<EntityHandle,CMonster*>::iterator iter = m_MonsterList.begin();
	for (; iter != m_MonsterList.end(); ++iter)
	{
		CEntity* pEntity = pEntityMgr->GetEntity(iter->first);
#endif
		if (pEntity)
		{
			pEntity->SetProperty<int>(PROP_CREATURE_HP, 0);
			((CMonster *)pEntity)->OnKilledByEntity(pGmEntity);
		}
	}
}
int CScene::KillMonsterByName(const char * sName, CEntity * pGmEntity, byte nDropItem)
{
	int nCount = 0;
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CEntityList killMonsterList;
#ifndef MONSTER_MANAGER_BY_SCENE
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_MonsterList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
#else
	std::map<EntityHandle,CMonster*>::iterator iter = m_MonsterList.begin();
	for (; iter != m_MonsterList.end(); ++iter)
	{
		CEntity* pEntity = pEntityMgr->GetEntity(iter->first);
#endif
		if (pEntity != NULL && !pEntity->IsDeath() && _stricmp(pEntity->GetEntityName(), sName) == 0)
		{
			killMonsterList.linkAtLast(pEntity->GetHandle());
		}
	}
	CLinkedNode<EntityHandle> *pNodeKill;
	CLinkedListIterator<EntityHandle> killIt(killMonsterList);
	for (pNodeKill = killIt.first(); pNodeKill; pNodeKill = killIt.next())
	{

		CEntity* pEntity = pEntityMgr->GetEntity(pNodeKill->m_Data);
		if (pEntity != NULL && !pEntity->IsDeath())
		{
			pEntity->SetProperty<int>(PROP_CREATURE_HP, 0);		
			if (nDropItem > 0)
			{
				((CMonster *)pEntity)->OnKilledByEntity(pGmEntity);
			}
				
			nCount++;
		}
	}
	return nCount;
}
void CScene::ClearEntity(int nType)
{
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it;

	bool flag =true;
	switch (nType)
	{
	case enNpc:
		it.setList(m_NpcList);
		break;

	case enActor:
		it.setList(m_PlayList); 
		break;

	case enMonster:
	case enGatherMonster:
#ifndef MONSTER_MANAGER_BY_SCENE
		it.setList(m_MonsterList); 
#else
	{
		CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
		std::map<EntityHandle,CMonster*>::iterator iter = m_MonsterList.begin();
		for (; iter != m_MonsterList.end(); ++iter)
		{
			CEntity* pEntity = pEntityMgr->GetEntity(iter->first);
			if (pEntity != NULL)
			{
				EntityHandle handle = pEntity->GetHandle();
				if (pEntity->IsDeath())
				{
					ClearDeathEntity(handle);
				}
				pEntityMgr->DestroyEntity(handle);
			}
		}
		return;
	}
#endif
		break;

	case enTransfer:
		it.setList(m_TeleportList); 
		break;

	case enPet:
		it.setList(m_petList); 	
		break;

	case enHero:
		it.setList(m_heroList); 	
		break;

	case enDropItem:
		it.setList(m_dropItemList);
		break;

	case enFire:
		it.setList(m_fireList);
		break;

	default:
		flag =false;
		break;
	}

	if(flag)
	{
		CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
		for (pNode = it.first(); pNode; pNode = it.next())
		{		
			CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
			if(!pEntity)continue;
			EntityHandle handle = pEntity->GetHandle();
			if (pEntity->IsDeath())
			{
				ClearDeathEntity(handle);
			}
			pEntityMgr->DestroyEntity(handle);
		}
	}
}
void CScene::ClearDeathEntity(EntityHandle handle)
{
	for(INT_PTR i= m_DeathList.count() -1; i >=0; i--)
	{
		EntityHandle hd = m_DeathList[i];
		if (hd == handle)
		{
			m_DeathList.remove(i);
			break;
		}
	}
}
INT_PTR CScene::GetLiveMonsterCount()
 {
	 int result = 0;
	 CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
#ifndef MONSTER_MANAGER_BY_SCENE
	 CLinkedNode<EntityHandle> *pNode;
	 CLinkedListIterator<EntityHandle> it(m_MonsterList);
	 for (pNode = it.first(); pNode; pNode = it.next())
	 {
		 CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
#else
	std::map<EntityHandle,CMonster*>::iterator iter = m_MonsterList.begin();
	for (; iter != m_MonsterList.end(); ++iter)
	{
		CEntity* pEntity = pEntityMgr->GetEntity(iter->first);
#endif
		 if (pEntity && !pEntity->IsDeath())
		 {
			result ++;
		 }
	 }
	 return result;
 }
bool CScene::IsKillAllMonster( int nMonsterId, EntityHandle hHandle)
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
#ifndef MONSTER_MANAGER_BY_SCENE
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_MonsterList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
#else
	std::map<EntityHandle,CMonster*>::iterator iter = m_MonsterList.begin();
	for (; iter != m_MonsterList.end(); ++iter)
	{
		CEntity* pEntity = pEntityMgr->GetEntity(iter->first);
#endif
		if (pEntity)
		{
			if ( (nMonsterId == 0 || pEntity->GetProperty<int>(PROP_ENTITY_ID) == nMonsterId) 
				&& (hHandle == 0 || hHandle == ((CMonster*)pEntity)->GetOwner()) 
				&& !((CMonster*)pEntity)->HasState(esStateDeath) )
			{
				return false;
			}
		}
		else
		{
			OutputMsg(rmError,_T("GetMonsterCount 有一个不存在的实体"));
		}
	}

	return true;
}

void CScene::RemoveEntity(CEntity* pEntity)
{
	const EntityHandle& handle = pEntity->GetHandle();
	CLinkedNode<EntityHandle> *pNode;
	INT_PTR nEntityType = pEntity->GetType();

	//CEntityList * pEntityList;
	CLinkedListIterator<EntityHandle> it;
	
	bool flag =true;
	switch (nEntityType)
	{
	case enNpc:
		it.setList(m_NpcList);
		break;

	case enActor:
		it.setList(m_PlayList); 
		break;

	case enMonster:
	case enGatherMonster:
#ifndef MONSTER_MANAGER_BY_SCENE
		it.setList(m_MonsterList); 
#else
	{
		CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
		std::map<EntityHandle,CMonster*>::iterator iter = m_MonsterList.find(handle);
		if (iter != m_MonsterList.end())
		{
			if (m_MonInterator == iter)
			{
				m_MonsterList.erase(m_MonInterator++);
				if (m_MonInterator == m_MonsterList.end())
					m_MonInterator = m_MonsterList.begin();
			}
			else m_MonsterList.erase(iter);
		}
		return;
	}
#endif
		break;

	case enTransfer:
		it.setList(m_TeleportList); 
		break;
	
	case enPet:
		it.setList(m_petList); 	
		break;
		
	case enHero:
		it.setList(m_heroList); 	
		break;
		
	case enDropItem:
		it.setList(m_dropItemList);
		break;

	case enFire:
		it.setList(m_fireList);
		break;
	
	default:
		flag =false;
		break;
	}

	if(flag)
	{
		for (pNode = it.first(); pNode; pNode = it.next())
		{
			if (handle == pNode->m_Data)
			{
				it.remove(pNode);
				break;
			}
		}
	}
	

}

void CScene::Broadcast(char *pData, SIZE_T size)
{
	EntityHandle handler;
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedListIterator<EntityHandle> it(m_PlayList);
	for (CLinkedNode<EntityHandle> *pNode = it.first(); pNode; pNode = it.next())
	{
		CActor* pActor = (CActor *)(pEntityMgr->GetEntity(pNode->m_Data));
		if (pActor && pActor->IsInited())
		{
			CActorPacket pack;
			CDataPacket &data = pActor->AllocPacket(pack);
			data.writeBuf(pData, size);
			pack.flush();
		}
	}	
}
void CScene::BroadcastNewAttr()
{
	EntityHandle handler;
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedListIterator<EntityHandle> it(m_PlayList);
	for (CLinkedNode<EntityHandle> *pNode = it.first(); pNode; pNode = it.next())
	{
		CActor* pActor = (CActor *)(pEntityMgr->GetEntity(pNode->m_Data));
		if (pActor && pActor->IsInited())
		{
			int x,y;
			pActor->GetPosition(x,y);
			
			SCENEAREA* pArea = GetAreaAttri(x,y);	//直接返回区域属性，后面的函数判断直接用这个,避免多次获取
			if(pArea)
			{
				GetGrid().SendAreaConfig(pArea,pActor); 
			}
		}
	}	
}

int CScene::GetSceneTime()
{
	if (m_restTime == 0) return 0;
	int restTime = m_restTime - GetGlobalLogicEngine()->getMiniDateTime();
	return restTime < 0 ? 0 : restTime;
}

void CScene::SetSceneTime(UINT nTime)
{
	m_restTime = GetGlobalLogicEngine()->getMiniDateTime() + nTime;
}

INT_PTR CScene::GetAreaCount()
{
	return m_pSceneData?m_pSceneData->vAreaList.nCount:0;
}

bool CScene::SetAreaAttri(INT_PTR nIndex, INT_PTR nType, int* pValues, INT_PTR nCount, LPCSTR sFunc, Uint64 hNpc, int noTips)
{
	if (m_pSceneData && nIndex >= 0 && nIndex < m_pSceneData->vAreaList.nCount && nType > aaNoAttri && nType < aaAttriCount)
	{
		SCENEAREA* pArea = m_pSceneData->vAreaList.pList + nIndex;
		if (pArea)
		{
			pArea->NoTips = noTips;
			AreaAttri& attri = pArea->attri[nType];
			attri.bType = (BYTE)nType;
			attri.nCount = (WORD)nCount;
			attri.hNpc = hNpc;
			if (attri.sFunc)
			{
				GFreeBuffer(attri.sFunc);
				attri.sFunc = NULL;
			}
			if (sFunc)
			{
				INT_PTR len = strlen(sFunc);
				attri.sFunc = (char*) GAllocBuffer(len+1);
				strcpy(attri.sFunc,sFunc);
				attri.sFunc[len] = 0;
			}
				
			if (attri.pValues)
			{
				GFreeBuffer(attri.pValues);
				attri.pValues = NULL;
				attri.nCount = 0;
			}
			if (nCount > 0 && pValues)
			{
				int size = (int)(sizeof(pValues[0])*nCount);
				attri.pValues = (int*)GAllocBuffer(size);
				memcpy(attri.pValues,pValues,size);
				attri.nCount = (WORD)nCount;
			}
			INT_PTR nIndex1 = nType / 32;
			INT_PTR nBit = nType % 32;
			pArea->attrBits[nIndex1] |= (1 << nBit);

			bool result = CEnvirConfig::checkParam(pArea,nType);
			//这里要广播消息
			if (result)
			{
			}
			BroadcastNewAttr();
			return result;
		}
		return true;
	}
	return false;
}

bool CScene::CreateNpc( SCENECONFIG* pData ,bool bInited)
{
	DECLARE_TIME_PROF("CScene::CreateNpc");
	int nOpenday = GetLogicServer()->GetDaysSinceOpenServer();
	for (int i = 0; i < pData->NpcList.nCount; i++)
	{
		NPCPOS * npcPos = pData->NpcList.pPosList + i;
		NPCCONFIG * npcConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetNpcData(npcPos->nId);
		if (NULL == npcConfig)
		{
			//OutputMsg(rmError, _T("在创建场景[%s]中，没有找到npcId[%d]的配置"), pData->szScenceName, npcPos->nId);
			continue;
		}
		if(npcConfig->nOpenDay && npcConfig->nOpenDay > nOpenday)
		{
			m_waitNpcLists.push_back(*npcPos);
			continue;
		}
		CEntity* pEntity = CreateEntityAndEnterScene(npcPos->nId,enNpc,npcPos->nPosX,npcPos->nPosY); // npc不需要id
		if (pEntity != NULL)
		{
			((CNpc*)pEntity)->SetConfig(npcConfig);
			((CNpc *)pEntity)->InitNPCAi();
			if (pEntity->GetEntityName()[0] == 0)
			{
				pEntity->SetEntityName((char*)npcConfig->sName);
			}
			pEntity->SetTitle(npcConfig->sTitle);
			pEntity->SetProperty<int>(PROP_ENTITY_MODELID,npcConfig->nModelId);
			pEntity->SetProperty<int>(PROP_ENTITY_ICON,npcConfig->nIcon);
			//((CNpc *)pEntity)->SetCamp(npcConfig->bCamp); //设置NPC的阵营

			//pEntity->SetProperty<UINT>(PROP_CREATURE_MAXHP,10000);
			//pEntity->SetProperty<UINT>(PROP_CREATURE_HP,10000);
			//pEntity->SetProperty<UINT>(PROP_CREATURE_MAXMP,10000);
			//pEntity->SetProperty<UINT>(PROP_CREATURE_MP,10000);

			//if (npcConfig->bCanHideFlag)			//设置不可见
			//{
			//	((CNpc*)pEntity)->GetAttriFlag().DenySee = npcConfig->bCanHideFlag;
			//}
			/*if (!((CNpc*)pEntity)->LoadScript())
			{
			OutputMsg(rmError,_T("load npc script error!sceneid=%d,npcName=%s,scenename=%s,scriptFile=%s"),
			GetSceneId(),npcConfig->sName,GetSceneName(),npcConfig->sScript);
			return false;
			}*/
			pEntity->SetInitFlag(bInited);//如果是创建备用的，这个设置成false

		}
		else
		{
			//不应该有失败的情况
			OutputMsg(rmError,_T("Create NPC error!!!Npc Name =%s,SceneId=%d"),npcConfig->sName,GetSceneId());
		}
	}
	return true;
}

bool CScene::CreateWaitNpc()
{
	if(!m_waitNpcLists.size())
	{
		return true;
	}
	DECLARE_TIME_PROF("CScene::CreateWaitNpc");
	int nOpenday = GetLogicServer()->GetDaysSinceOpenServer();
	auto nIt = m_waitNpcLists.begin();
	for(; nIt != m_waitNpcLists.end();)
	{
		NPCPOS npcPos = *nIt;
		NPCCONFIG * npcConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetNpcData(npcPos.nId);
		if (NULL == npcConfig)
		{
			//OutputMsg(rmError, _T("在创建场景[%s]中，没有找到npcId[%d]的配置"), pData->szScenceName, npcPos->nId);
			nIt++;
			continue;
		}
		if(npcConfig->nOpenDay && npcConfig->nOpenDay > nOpenday)
		{
			nIt++;
			continue;
		}
		CEntity* pEntity = CreateEntityAndEnterScene(npcPos.nId,enNpc,npcPos.nPosX,npcPos.nPosY); // npc不需要id
		if (pEntity != NULL)
		{
			((CNpc*)pEntity)->SetConfig(npcConfig);
			((CNpc *)pEntity)->InitNPCAi();
			if (pEntity->GetEntityName()[0] == 0)
			{
				pEntity->SetEntityName((char*)npcConfig->sName);
			}
			pEntity->SetTitle(npcConfig->sTitle);
			pEntity->SetProperty<int>(PROP_ENTITY_MODELID,npcConfig->nModelId);
			pEntity->SetProperty<int>(PROP_ENTITY_ICON,npcConfig->nIcon);
			//((CNpc *)pEntity)->SetCamp(npcConfig->bCamp); //设置NPC的阵营

			//pEntity->SetProperty<UINT>(PROP_CREATURE_MAXHP,10000);
			//pEntity->SetProperty<UINT>(PROP_CREATURE_HP,10000);
			//pEntity->SetProperty<UINT>(PROP_CREATURE_MAXMP,10000);
			//pEntity->SetProperty<UINT>(PROP_CREATURE_MP,10000);

			//if (npcConfig->bCanHideFlag)			//设置不可见
			//{
			//	((CNpc*)pEntity)->GetAttriFlag().DenySee = npcConfig->bCanHideFlag;
			//}
			/*if (!((CNpc*)pEntity)->LoadScript())
			{
			OutputMsg(rmError,_T("load npc script error!sceneid=%d,npcName=%s,scenename=%s,scriptFile=%s"),
			GetSceneId(),npcConfig->sName,GetSceneName(),npcConfig->sScript);
			return false;
			}*/
			pEntity->SetInitFlag(true);//如果是创建备用的，这个设置成false
		}
		nIt = m_waitNpcLists.erase(nIt);
	}
	return true;
}

bool CScene::GmCreateNpcById(int nNpcID, int nX, int nY)
{
	NPCCONFIG * pNpcConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetNpcData(nNpcID);
	if(pNpcConfig != NULL)
	{
		CEntity* pEntity = CreateEntityAndEnterScene(nNpcID,enNpc,nX, nY); // npc不需要id
		if (pEntity != NULL)
		{
			((CNpc*)pEntity)->SetConfig(pNpcConfig);
			((CNpc *)pEntity)->InitNPCAi();
			pEntity->SetEntityName((char*)pNpcConfig->sName);
			pEntity->SetTitle(pNpcConfig->sTitle);
			pEntity->SetProperty<int>(PROP_ENTITY_MODELID,pNpcConfig->nModelId);
			pEntity->SetProperty<int>(PROP_ENTITY_ICON,pNpcConfig->nIcon);
			//((CNpc *)pEntity)->SetCamp(pNpcConfig->bCamp); //设置NPC的阵营

			pEntity->SetProperty<UINT>(PROP_CREATURE_MAXHP,10000);
			pEntity->SetProperty<UINT>(PROP_CREATURE_HP,10000);
			pEntity->SetProperty<UINT>(PROP_CREATURE_MAXMP,10000);
			pEntity->SetProperty<UINT>(PROP_CREATURE_MP,10000);

			pEntity->SetInitFlag(true);//如果是创建备用的，这个设置成false

			return true;
		}
	}
	return false;
}

bool CScene::GmCreateNpcByName(char* npcName)
{
	if(!npcName) return false;
	if(!m_pSceneData) return false;

	for (int i = 0; i < m_pSceneData->NpcList.nCount; i++)
	{
		NPCPOS* npcPos = m_pSceneData->NpcList.pPosList + i;
		NPCCONFIG * npcConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetNpcData(npcPos->nId);
		if (NULL == npcConfig)
		{
			continue;
		}
		if(strcmp(npcName,npcConfig->sName) == 0)
		{
			CEntity* pEntity = CreateEntityAndEnterScene(npcPos->nId,enNpc,npcPos->nPosX,npcPos->nPosY); // npc不需要id
			if (pEntity != NULL)
			{
				((CNpc*)pEntity)->SetConfig(npcConfig);
				((CNpc *)pEntity)->InitNPCAi();
				pEntity->SetEntityName((char*)npcConfig->sName);
				pEntity->SetTitle(npcConfig->sTitle);
				pEntity->SetProperty<int>(PROP_ENTITY_MODELID,npcConfig->nModelId);
				pEntity->SetProperty<int>(PROP_ENTITY_ICON,npcConfig->nIcon);
				//((CNpc *)pEntity)->SetCamp(npcConfig->bCamp); //设置NPC的阵营

				pEntity->SetProperty<UINT>(PROP_CREATURE_MAXHP,10000);
				pEntity->SetProperty<UINT>(PROP_CREATURE_HP,10000);
				pEntity->SetProperty<UINT>(PROP_CREATURE_MAXMP,10000);
				pEntity->SetProperty<UINT>(PROP_CREATURE_MP,10000);

				//if (npcConfig->bCanHideFlag)			//设置不可见
				//{
				//	((CNpc*)pEntity)->GetAttriFlag().DenySee = npcConfig->bCanHideFlag;
				//}
				if (!((CNpc*)pEntity)->LoadScript())
				{
					OutputMsg(rmError,_T("gmCreate npc script error!sceneid=%d,npcName=%s,scenename=%s,scriptFile=%s"),
						GetSceneId(),npcConfig->sName,GetSceneName(),npcConfig->sScript);
					return false;
				}
				pEntity->SetInitFlag(true);//如果是创建备用的，这个设置成false

				return true;
			}
		}
	}
	return false;
}

void CScene::ResetRefreshList(REFRESHLIST* list)
{
	TICKCOUNT dwTime = GetLogicCurrTickCount();
	for (int i = 0; i < list->nCount; i++)
	{
		REFRESHVALUE* data		= m_vRefreshMonster + i;
		REFRESHCONFIG* pConfig	= list->pList + i;
		data->nCurrCount			= 0;
		data->nNextTime			= dwTime + pConfig->nFirstTime;
	}
}

void CScene::CreateTeleport()
{
	for (int i = 0; i < m_pSceneData->TeleportList.nCount; i++)
	{
		TeleportConfig* pConfig = m_pSceneData->TeleportList.pList + i;
		CEntity* pEntity = CreateEntityAndEnterScene(0, 
													enTransfer, 
													pConfig->nPosX,
													pConfig->nPosY);
		if (pEntity != NULL)
		{
			pEntity->SetProperty<int>(PROP_ENTITY_MODELID, pConfig->nModelId);
			pEntity->SetEntityName(pConfig->szName);
			pConfig->nHandle = pEntity->GetHandle();
			// 设置传送门的目标点坐标
			((CTransfer*)pEntity)->SetParams(pConfig->nSceneId,pConfig->nToPosX,pConfig->nToPosY);
			pEntity->SetInitFlag(true);
			if (!pConfig->bUse)
			{
				pEntity->GetAttriFlag().DenySee = true;
			}
		}
		else
		{
			// 不应该有失败的情况
			OutputMsg(rmError,_T("Create Teleport error!!!Scene Name =%s,SceneId=%d"), 
					 GetSceneName(), 
					 GetSceneId());
		}
	}
}

void CScene::CloneSceneConfig( SCENECONFIG* pData )
{
	// 区域属性、刷怪动态属性
	ReleaseConfig();//释放原有的
	m_pSceneData = pData;
	if (pData)
	{
		m_pSceneData = (SCENECONFIG*)GAllocBuffer(sizeof(SCENECONFIG));
		memcpy(m_pSceneData,pData,sizeof(SCENECONFIG));
		//区域属性复制一份
		if (pData->vAreaList.pList)
		{
			int sz = sizeof(SCENEAREA)*pData->vAreaList.nCount;
			m_pSceneData->vAreaList.pList = (SCENEAREA*)GAllocBuffer(sz);
			memcpy(m_pSceneData->vAreaList.pList,pData->vAreaList.pList,sz);
			for ( INT_PTR i = 0; i < m_pSceneData->vAreaList.nCount; i++)
			{
				SCENEAREA* pArea = m_pSceneData->vAreaList.pList + i;
				for (INT_PTR j = 0; j < aaAttriCount; j++)
				{
					AreaAttri& attri = pArea->attri[j];
					if (attri.pValues)
					{
						int size = sizeof(attri.pValues[0])*(attri.nCount);
						int* pTemp = attri.pValues;
						attri.pValues = (int*)GAllocBuffer(size);
						memcpy(attri.pValues,pTemp,size);
					}
					if (attri.sFunc)
					{
						LPCSTR sTemp = attri.sFunc;
						INT_PTR len = ArrayCount(sTemp);
						attri.sFunc = (char*) GAllocBuffer(len+1);
						strcpy(attri.sFunc,sTemp);
						attri.sFunc[len] = 0;
					}
				}
			}

		}
	}
}

void CScene::ReleaseConfig()
{
	if (m_pSceneData)
	{
		if (m_pSceneData->vAreaList.pList)
		{
			for ( INT_PTR i = 0; i < m_pSceneData->vAreaList.nCount; i++)
			{
				SCENEAREA* pArea = m_pSceneData->vAreaList.pList + i;
				for (INT_PTR j = 0; j < aaAttriCount; j++)
				{
					AreaAttri& attri = pArea->attri[j];
					if (attri.pValues)
					{
						GFreeBuffer(attri.pValues);
						attri.pValues = NULL;
					}
					if (attri.sFunc)
					{
						GFreeBuffer(attri.sFunc);
						attri.sFunc = NULL;
					}
				}
			}
			GFreeBuffer(m_pSceneData->vAreaList.pList);
			m_pSceneData->vAreaList.pList = NULL;
		}
		GFreeBuffer(m_pSceneData);
		m_pSceneData = NULL;
	}
}



bool CScene::GetRandomPoint(CEntity *pEntity, int nP1_x, int nP1_y, int nP2_x, int nP2_y, int &nResult_x, int &nResult_y)
{
	//选择随机点传送
	int nMinX = __min(nP1_x, nP2_x);
	int nMaxX = __max(nP1_x, nP2_x);
	int nMinY = __min(nP1_y, nP2_y);
	int nMaxY = __max(nP1_y, nP2_y);

	INT_PTR nGridCount = (nMaxX - nMinX + 1) * (nMaxY - nMinY + 1);

	do 
	{
		INT_PTR nRandomX = nP1_x;
		if (nMaxX > nMinX)
			nRandomX = (rand() % (nMaxX - nMinX)) + nMinX;

		INT_PTR nRandomY = nP1_y;
		if (nMaxY > nMinY)
			nRandomY = (rand() % (nMaxY - nMinY)) + nMinY;

		if (m_Grid.CanAddEntity(pEntity, nRandomX, nRandomY,false)) 
		{
			nResult_x = (int)nRandomX;
			nResult_y = (int)nRandomY;
			break;
		}

		nGridCount--;
	} while (nGridCount > 0);

	if (nResult_x < 0 || nResult_x >= m_Grid.m_nCol || nResult_y < 0 || nResult_y >= m_Grid.m_nRow)
		return false;

	return true;
}

void CScene::SendMsg(LPCSTR sMsg, int nType)
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(GetPlayList());
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
		if (pEntity && pEntity->GetType() == enActor)
		{
			((CActor*)pEntity)->SendTipmsg(sMsg,nType);
		}
	}
}

void CScene::SendSecneShortCutMsg(LPCSTR sMsg)
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(GetPlayList());
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
		if (pEntity && pEntity->GetType() == enActor)
		{
			((CActor*)pEntity)->SendShortCutMsg(sMsg);
		}
	}
}

bool CScene::GetRandomPointByEntityType(int nEntityType, int nP1_x, int nP1_y, int nP2_x, int nP2_y, int &nResult_x, int &nResult_y, 
	bool canAlwaysEnter,bool bForcePos)
{
	//选择随机点传送
	int nMinX = __min(nP1_x, nP2_x);
	int nMaxX = __max(nP1_x, nP2_x);
	int nMinY = __min(nP1_y, nP2_y);
	int nMaxY = __max(nP1_y, nP2_y);
	int nRangeX = (nMaxX - nMinX + 1);
	int nRangeY = (nMaxY - nMinY + 1);
	INT_PTR nGridCount = nRangeX * nRangeY;

	do 
	{
		INT_PTR nRandomX = nP1_x;
		if (nMaxX > nMinX)
			nRandomX = (rand() % nRangeX) + nMinX;

		INT_PTR nRandomY = nP1_y;
		if (nMaxY > nMinY)
			nRandomY = (rand() % nRangeY) + nMinY;

		if( bForcePos )		//改点强制允许创建（玩玩该坐标点就是玩家坐标点，才能保证是合法的坐标）
		{
			nResult_x = (int)nRandomX;
			nResult_y = (int)nRandomY;
			break;
		}

		if (m_Grid.CanCross(nEntityType, nRandomX, nRandomY,canAlwaysEnter) && !IsTelePort(nRandomX,nRandomY))
		{
			nResult_x = (int)nRandomX;
			nResult_y = (int)nRandomY;
			break;
		}

		nGridCount--;
	} while (nGridCount > 0);

	if (nResult_x < 0 || nResult_x >= m_Grid.m_nCol || nResult_y < 0 || nResult_y >= m_Grid.m_nRow)
		return false;

	return true;
}

bool CScene::GetMobMonsterXY(REFRESHCONFIG* pRefreshConf, INT_PTR &nX, INT_PTR &nY)
{
	bool boResult = false;
	if (!pRefreshConf)
	{
		return boResult;
	}
	int nMinX = pRefreshConf->nMobX - pRefreshConf->nMobRange;
	int nMaxX = pRefreshConf->nMobRange * 2 + 0;
	nX = nMinX + wrand(nMaxX);

	int nMinY = pRefreshConf->nMobY - pRefreshConf->nMobRange;
	int nMaxY = pRefreshConf->nMobRange * 2 + 0;
	nY = nMinY + wrand(nMaxY);

	int nWhileCount = 0;
	INT_PTR nCenterX,nCenterY;
	if ( CanMove( nX, nY ))
	{
		boResult = true;
	}
	else
	{
		//nCenterX = nX - 6;
		//nCenterY = nY - 6;
		while( nWhileCount <  100 )
		{
			//nX = nCenterX + wrand(14);
			//nY = nCenterY + wrand(14);
			nX = nMinX + wrand(nMaxX);
			nY = nMinY + wrand(nMaxY);
			if (CanMove(nX,nY))
			{
				boResult = true;
				break;
			}
			nWhileCount++;
		}
	}
	if ( !boResult )
	{
		nX = pRefreshConf->nMobX;
		nY = pRefreshConf->nMobY;
		boResult = true;
	}
	return boResult;
}
CEntity* CScene::getMonsterPtrById(int nMonsterId )
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
#ifndef MONSTER_MANAGER_BY_SCENE
	CEntityList& list = GetMonsterList();
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(list);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
#else
	std::map<EntityHandle,CMonster*>::iterator iter = GetMonsterList().begin();
	for (; iter != GetMonsterList().end() ; ++iter)
	{
		CEntity* pEntity = pEntityMgr->GetEntity(iter->first);
#endif
		if (pEntity && (pEntity->GetProperty<int>(PROP_ENTITY_ID) == nMonsterId))
		{
			return pEntity;
		}		
	}
	return NULL;
}

int CScene::CalcAllActor(int nLevel)
{
	int nResult = 0;
	EntityHandle handler;
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedListIterator<EntityHandle> it(m_PlayList);
	for (CLinkedNode<EntityHandle> *pNode = it.first(); pNode; pNode = it.next())
	{
		CActor* pActor = (CActor *)(pEntityMgr->GetEntity(pNode->m_Data));
		if (pActor && pActor->IsInited() && !pActor->IsDestory() && pActor->GetProperty<int>(PROP_CREATURE_LEVEL) > nLevel)
		{
			nResult++;
		}
	}	
	return nResult;
}

int CScene::CloseSceneAllActor(int nLevel)
{
	int nResult = 0;
	EntityHandle handler;
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedListIterator<EntityHandle> it(m_PlayList);
	for (CLinkedNode<EntityHandle> *pNode = it.first(); pNode; pNode = it.next())
	{
		CActor* pActor = (CActor *)(pEntityMgr->GetEntity(pNode->m_Data));
		if (pActor && pActor->IsInited() && !pActor->IsDestory() && pActor->GetProperty<int>(PROP_CREATURE_LEVEL) > nLevel)
		{
			pActor->CloseActor(lwiGmTickActor,false);
			nResult++;
		}
	}
	return nResult;
}

int CScene::GetNowSceneGuildList()
{
	int nResult = 0;
	std::set<int> guildList;
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CLinkedListIterator<EntityHandle> it(m_PlayList);
	for (CLinkedNode<EntityHandle> *pNode = it.first(); pNode; pNode = it.next())
	{
		CActor* pActor = (CActor *)(pEntityMgr->GetEntity(pNode->m_Data));
		if (pActor && pActor->IsInited() && !pActor->IsDestory() && !pActor->IsDeath())
		{
			int nGuildId=pActor->GetProperty<int>(PROP_ACTOR_GUILD_ID);
			guildList.insert(nGuildId);
		}
	}
	if(guildList.size() == 1)
	{
		std::set<int>::iterator it = guildList.begin();
		for(; it != guildList.end(); it++)
		return *it;
	}
	return nResult;
}


void CScene::ResetSceneConfig()
{
	CloneSceneConfig(m_pSrcSceneData);
	BroadcastNewAttr();
}

 void CScene::KickOutAllActor()
 {
	 CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	 CLinkedNode<EntityHandle> *pNode;
	 CLinkedListIterator<EntityHandle> it(m_PlayList);
	 for (pNode = it.first(); pNode; pNode = it.next())
	 {
		 CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
		 if (pEntity && pEntity->GetType() == enActor)
		 {
			((CActor*)pEntity)->ReturnCity();
		 }
		 m_PlayList.remove(pNode); //修改了一个死循环
	 }
 }
