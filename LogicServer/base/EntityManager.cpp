#include "StdAfx.h"
#include "HandleMgr.h"
#include "EntityManager.h"
#include "AMClient.h"
#include "Container.hpp"

/*
CEntityManager::CEntityManager() : m_actorMgr(_T("ActorHdlMgr"),6), m_monsterMgr(_T("MonsterHdlMgr"), 8, 8),
	m_npcMgr(_T("NpcHdlMgr"),6, 1), m_dropItemEntityMgr(_T("DropItemMgr"),1), m_transferMgr(_T("TransferHdlMgr"),1),
	m_entitySCBHdlMgr(_T("ScriptCBHdlMgr")),m_petMgr( _T("PetMgr"), 8, 4), m_fireMgr( _T("fireMgr"),1),
	m_landscapeMgr( _T("landscape"),1),m_heroMgr( _T("hero"),1),m_walkRideMgr(_T("WalkRide"),1)


CEntityManager::CEntityManager() : m_actorMgr(_T("ActorHdlMgr"),40), m_monsterMgr(_T("MonsterHdlMgr"), 20, 8),
	m_npcMgr(_T("NpcHdlMgr"),6, 1), m_dropItemEntityMgr(_T("DropItemMgr"),1), m_transferMgr(_T("TransferHdlMgr"),1),
	m_entitySCBHdlMgr(_T("ScriptCBHdlMgr")),m_petMgr( _T("PetMgr"), 10, 4), m_fireMgr( _T("fireMgr"),1),
	m_landscapeMgr( _T("landscape"),1),m_heroMgr( _T("hero"),1),m_walkRideMgr(_T("WalkRide"),1)
*/

CEntityManager::CEntityManager() : m_actorMgr(_T("ActorHdlMgr"),40), m_monsterMgr(_T("MonsterHdlMgr"), 20, 8),
	m_npcMgr(_T("NpcHdlMgr"),10, 1), m_dropItemEntityMgr(_T("DropItemMgr"),5), m_transferMgr(_T("TransferHdlMgr"),5),
	m_entitySCBHdlMgr(_T("ScriptCBHdlMgr")),m_petMgr( _T("PetMgr"), 10, 4), m_fireMgr( _T("fireMgr"),5),
	m_heroMgr( _T("hero"),1)
{
}

 CEntity * CEntityManager::GetEntity(const EntityHandle &hEntity)
{
	EntityIndex hIndex ( hEntity.GetIndex());
	unsigned nEntityType = hEntity.GetType();
	CEntity * pEntity =NULL;
	switch(nEntityType)
	{
	case enActor:
		pEntity = (CEntity *)m_actorMgr.m_entityMgr.GetDataPtr(hIndex);
		break;
	case enMonster:
	case enGatherMonster:
		pEntity = (CEntity *)m_monsterMgr.m_entityMgr.GetDataPtr(hIndex);
		break;
	case enNpc:
		pEntity = (CEntity *)m_npcMgr.m_entityMgr.GetDataPtr(hIndex);
		break;
	case enDropItem:
		pEntity = (CEntity *)m_dropItemEntityMgr.m_entityMgr.GetDataPtr(hIndex);
		break;
	case enTransfer:
		pEntity = (CEntity *)m_transferMgr.m_entityMgr.GetDataPtr(hIndex);
		break;
	
	case enPet:
		pEntity =  (CEntity *)m_petMgr.m_entityMgr.GetDataPtr(hIndex);
		break;
	case enFire: //火
		pEntity = (CEntity *)m_fireMgr.m_entityMgr.GetDataPtr(hIndex); 
		break;
	case enHero:
		pEntity = (CEntity *)m_heroMgr.m_entityMgr.GetDataPtr(hIndex); 
		break;
	}
	return pEntity;
}


VOID CEntityManager::RunOne()
{
    DECLARE_TIME_PROF("CEntityManager::RunOne");
#ifdef _USE_TRY_CATCH
#ifdef WIN32
	__try
#endif 
#endif 
	{	
		int nPos = 0;
		EntityIndex hIndex;
		SF_TIME_CHECK(); //检测性能
		//TICKCOUNT nCurr = _getTickCount();
	
		TICKCOUNT nCurrentTime =GetLogicCurrTickCount();

		for (INT_PTR i = m_waitRemoveEntityList.count()-1; i >=0 ; i--)
		{
			DestroyEntity(m_waitRemoveEntityList[i]);
		}
		m_waitRemoveEntityList.clear();

		//超时就退出
		m_actorMgr.Run(nCurrentTime);
	
		m_petMgr.Run(nCurrentTime);

#ifndef MONSTER_MANAGER_BY_SCENE
		m_monsterMgr.Run(nCurrentTime);
#endif

		m_heroMgr.Run(nCurrentTime);
 
		//掉落一秒钟执行一次就行了
		if(m_1secTimer.CheckAndSet(nCurrentTime))
		{
			m_dropItemEntityMgr.Run(nCurrentTime);
			m_fireMgr.Run(nCurrentTime); //每秒钟执行一次
			m_npcMgr.Run(nCurrentTime);	
		}
		/*盛趣防沉迷屏蔽
		if(m_1minsTimer.CheckAndSet(nCurrentTime))//一分钟一次
		{ 	
			//盛趣平台走这里
			if(GetLogicServer()->GetDataProvider()->GetGlobalConfig().bIsSqKidsFcmOpen)
			{
				int nReslut = 0;
				std::vector<unsigned int> list;  
				CActor *pActor;
#ifndef ENTITY_MANAGER_USE_STL_MAP
				CList<CActor*>::Iterator it(m_actorMgr.m_list);
				CList<CActor*>::NodeType *pNode;
				for (pNode=it.first(); pNode; pNode = it.next())
				{
					pActor = *pNode; 
					if(!pActor->IsAdult())
					{ 
						list.push_back((unsigned int)(pActor->GetId()));
					}
				}
#else 
				ActorMgr::Iterator it = m_actorMgr.m_EntityMap.begin();
				for (; it != m_actorMgr.m_EntityMap.end(); ++it)
				{ 
					pActor = (CActor*)(it->second);
					if(!pActor->IsAdult())
					{ 
						list.push_back((unsigned int)(pActor->GetId()));
					}
				}
#endif
				sort(list.begin(), list.end());
				list.erase( unique( list.begin(), list.end() ), list.end());
				nReslut = (int)list.size();

				int page = 1 + (nReslut/ 100);

				for(int i = 0; i < page; i++)
				{ 
					std::string strSendBuff = "";
					strSendBuff.resize(500);   
					CDataPacket pack((char *)strSendBuff.data(), 500); 
					int pagesize = 0;
					pack << (byte)1;
					pack << (byte)enMiscSystemID;
					pack << (byte)sFcmAllUseHeatbeat;
					INT_PTR nOffer = pack.getPosition();  
					pack << (byte)pagesize; 
					for(int index = i*100; (index < nReslut)&&(index < (i + 1)*100); index++)
					{ 
						pack << (unsigned int)list[index]; 
						//pActor = (CActor*)list[index]; 
						pagesize++;
					}
					if(pagesize == 0)
					{
						continue;
					}
					
					INT_PTR nOffer2 = pack.getPosition();  
					pack.setPosition(nOffer);
					pack << (byte)pagesize; 
					pack.setPosition(nOffer2);
					pack.setPosition(0);
					CDataPacketReader dp(pack.getOffsetPtr(), pack.getAvaliableLength()); 
					CScriptValueList paramList, retList; 
					paramList << enMiscSystemID;
					paramList << sFcmAllUseHeatbeat; 
					paramList << pActor; 
					paramList << &dp;   
					static CNpc* gp = GetGlobalLogicEngine()->GetGlobalNpc();
					
					bool ret = gp->GetScript().Call("OnNetMsg", paramList, retList);
					if (!ret)
					{
						//脚本错误，将以模态对话框的形式呈现给客户端
						const RefString& s = gp->GetScript().getLastErrorDesc();  
						OutputMsg(rmTip, "load KidsFcm:%s", (const char*)s);
					} 
				}  
			}
		}
		*/
		if(m_5secTimer.CheckAndSet(nCurrentTime))
		{
			while (!m_petExpire.empty() && nCurrentTime > m_petExpire.top().second)
			{
				int nActorId = m_petExpire.top().first;

				if (INT_PTR nCount = m_petCache[nActorId].count())
				{
					CVector<CPetSystem::PETDATA> & petList = m_petCache[nActorId];
					for (INT_PTR i = nCount - 1; i > -1; --i)
					{
						CPetSystem::PPETDATA pPetData = &petList[i];
						if (pPetData)
						{
							EntityHandle petHandle = pPetData->handle;
							int nPetConfigId = pPetData->nConfigId;
							int nSkillId = pPetData->nSkillId;
							petList.remove(i);
							
							if(CEntity *pEntity =GetEntityFromHandle(petHandle))
							{
								CScene* pScene = pEntity->GetScene();
								if (pScene != NULL)
								{
									pScene->ExitScene(pEntity);
								}
								pEntity->Destroy();
								pEntity->SetInitFlag(false);
								pEntity->SetDestroyFlag(true);
								m_petMgr.m_entityMgr.Release(hIndex);
								m_petMgr.DestroyCache(pEntity);
							}
						}
					}
				}
				m_petExpire.pop();
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

CEntity * CEntityManager::CreateEntity(int nEntityType,EntityHandle  &hHandle)
{
	EntityIndex hIndex;
	CEntity * pEntity =NULL;
	SF_TIME_CHECK(); //检测性能，检测创建的实体的销毁情况
	switch(nEntityType)
	{
	case enActor:
		pEntity = (CEntity *)m_actorMgr.m_entityMgr.Acquire(hIndex);
		break;
	case enMonster:
	case enGatherMonster:	//采集怪
		pEntity = (CEntity *)m_monsterMgr.m_entityMgr.Acquire(hIndex);
		break;
	case enNpc:
		pEntity = (CEntity *)m_npcMgr.m_entityMgr.Acquire(hIndex);
		break;
	case enDropItem:
		pEntity = (CEntity *)m_dropItemEntityMgr.m_entityMgr.Acquire(hIndex);
		break;
	case enTransfer: //传送门
		pEntity = (CEntity *)m_transferMgr.m_entityMgr.Acquire(hIndex);
		break;
	
	case enPet:
		pEntity = m_petMgr.m_entityMgr.Acquire(hIndex);
		break;
	case enFire:
		pEntity = m_fireMgr.m_entityMgr.Acquire(hIndex);
		break;
	case enHero:
		pEntity = m_heroMgr.m_entityMgr.Acquire(hIndex);
		break;
	}
	if(pEntity)
	{
		hHandle.init(nEntityType,hIndex);
		pEntity->SetHandle(hHandle);
		switch(nEntityType)
		{
		case enActor:
			m_actorMgr.Add(pEntity);			
			break;
		case enMonster:
		case enGatherMonster:
			m_monsterMgr.Add(pEntity);
			break;
		case enNpc:
			m_npcMgr.Add(pEntity);
			break;
		case enDropItem:
			m_dropItemEntityMgr.Add(pEntity);
			break;
		case enTransfer: //传送门
			m_transferMgr.Add(pEntity);
			break;

		case enPet:
			m_petMgr.Add(pEntity);
			break;
		case enFire:
			m_fireMgr.Add(pEntity);
			break;
		case enHero:
			m_heroMgr.Add(pEntity);
			break;
		}

	}
	return pEntity;
}

bool CEntityManager::DestroyEntity(EntityHandle hEntity, bool bDelayDestroy)
{
	if (bDelayDestroy)
	{
		m_waitRemoveEntityList.add(hEntity);
		return true;
	}

	SF_TIME_CHECK(); //检测性能，检测创建的实体的销毁情况
	EntityIndex hIndex(hEntity.GetIndex());
	unsigned nEntityType = hEntity.GetType();
	CEntity * pEntity = GetEntityFromHandle(hEntity);
	if(pEntity)
	{
		CScene* pScene = pEntity->GetScene();
		if (pScene != NULL)
		{
			pScene->ExitScene(pEntity);
		}
		pEntity->Destroy();
		pEntity->SetInitFlag(false);
		pEntity->SetDestroyFlag(true);
		switch(nEntityType)
		{
		case enActor:
			OnActorClosed((CActor *)pEntity);
			m_actorMgr.m_entityMgr.Release(hIndex);
			m_actorMgr.Destroy(pEntity);
			break;
		case enMonster:
		case enGatherMonster:
			m_monsterMgr.m_entityMgr.Release(hIndex);
			m_monsterMgr.Destroy(pEntity);
			break;
		case enNpc:
			m_npcMgr.m_entityMgr.Release(hIndex);
			m_npcMgr.Destroy(pEntity);
			break;
		case enDropItem:
			m_dropItemEntityMgr.m_entityMgr.Release(hIndex);
			m_dropItemEntityMgr.Destroy(pEntity);
			break;
		case enTransfer:
			m_transferMgr.m_entityMgr.Release(hIndex);
			m_transferMgr.Destroy(pEntity);
			break;

		case enPet:
			m_petMgr.m_entityMgr.Release(hIndex);
			m_petMgr.Destroy(pEntity);
			break;
		case enFire:
			m_fireMgr.m_entityMgr.Release(hIndex);
			m_fireMgr.Destroy(pEntity);
			break;
		case enHero:
			m_heroMgr.m_entityMgr.Release(hIndex);
			m_heroMgr.Destroy(pEntity);
			break;
		}
		return true;
	}
	else
	{
		return false;
	}

}

bool CEntityManager::CachePet(CActor* pMaster, CVector<CPetSystem::PETDATA> *pPetList, long long nExpireTime)
{
	// Check
	if (!pPetList | !pMaster) return false;
	CVector<CPetSystem::PETDATA>& petList = *pPetList;
	INT_PTR nCount = petList.count();
	if (nCount <= 0) return true;

	int nActorId = pMaster->GetId();
	
	// 遍历所有宠物，并处理
	for (INT_PTR i = nCount - 1; i > -1; --i)
	{
		CPetSystem::PPETDATA pPetData = &petList[i];
		if (!pPetData)
		{
			return false;
		}
		EntityHandle petHandle = pPetData->handle;
		if (CEntity * pEntity = GetEntityFromHandle(petHandle))
		{
			// 退出场景
			CScene* pScene = pEntity->GetScene();
			if (pScene != NULL)
			{
				pScene->ExitScene(pEntity);
			}

			// 避免无用的逻辑执行
			pEntity->SetInitFlag(false);

			// 清理状态
			((CPet*)pEntity)->RemoveState(esStateBattle);
			//((CPet*)pEntity)->SetBattleStatus(CPet::PET_BATTLE_STATUE_STAY);
			((CPet*)pEntity)->SetProperty<unsigned int>(PROP_CREATURE_STATE,0);
			((CPet*)pEntity)->SetTarget(NULL);

			// 缓存
			m_petMgr.Cache((CPet*)pEntity);
		}
	}
	
	// 保存 玩家id 与 宠物列表 的映射
	// if(m_petCache.find(nActorId) == m_petCache.end())
	// {
	// 	m_petCache[nActorId].swap(*pPetList);
	// }
	m_petCache[nActorId].swap(*pPetList);
	std::pair<int, long long> expire(nActorId, GetGlobalLogicEngine()->getTickCount() + nExpireTime);
	m_petExpire.push(expire);
	return true;
}

bool CEntityManager::UnCachePet(CActor* pMaster, CVector<CPetSystem::PETDATA> *pPetList, CScene* pScene)
{
	// Check
	if (!pPetList | !pMaster) return false;
	CVector<CPetSystem::PETDATA>& petList = *pPetList;

	int nActorId = pMaster->GetId();
	typedef typename std::map<int, CVector<CPetSystem::PETDATA> >::iterator Iter;

	// 获取宠物
	Iter iter = m_petCache.find(nActorId);
	if(iter != m_petCache.end())
	{
		m_petCache[nActorId].swap(petList);
		m_petCache.erase(iter);
		
		// 遍历所有宠物，并处理
		INT_PTR nCount = petList.count();
		if (nCount > 0)
		{
			for (INT_PTR i = nCount - 1; i > -1; --i)
			{
				CPetSystem::PPETDATA pPetData = &petList[i];
				if (!pPetData)
				{
					return false;
				}
				EntityHandle petHandle = pPetData->handle;
				if (CEntity * pEntity = GetEntityFromHandle(petHandle))
				{
					if (pScene != NULL)
					{
						// 进入场景
						int nX = pEntity->GetProperty<int>(PROP_ENTITY_POSX);
						int nY = pEntity->GetProperty<int>(PROP_ENTITY_POSY);
						pScene->EnterScene(pEntity, nX, nY);

						// 重设主人
						((CPet*)pEntity)->SetMaster(pMaster);
						((CPetAi*)((CPet*)pEntity)->GetAI())->SetFollowStatus(false);
						((CPet*)pEntity)->GetMoveSystem()->PopCurrentMotionState();
						((CPet*)pEntity)->GetMoveSystem()->MoveFollow(pMaster);

						// 逻辑执行
						pEntity->SetInitFlag(true);

						// 缓存
						m_petMgr.unCache((CPet*)pEntity);
					}
				}
			}
		}
	
		return true;
	}
	return false;
}

void CEntityManager::OnActorClosed(CActor* pActor)
{
	// Remove from idlist
	unsigned int nActorId = pActor->GetId();	
	ActorId2Hdl::iterator iter_end = m_actorIdList.end();
	ActorId2Hdl::iterator iter = m_actorIdList.find(nActorId);
	if (iter != iter_end)
	{ 
		m_actorIdList.erase(iter);
	}
	else
	{
		OutputMsg(rmError, _T("%s can't find actor[actorid=%u] in idlist"), __FUNCTION__, nActorId);
	}
}

/*针对某个玩家的数据查询
*/
void CEntityManager::OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size)
{
	//查询角色列表出错
	CDataPacketReader reader(data,size);
	unsigned int nActorID=0;
	BYTE nErrorCode = 0;
	if(size < sizeof(nActorID) + sizeof(nErrorCode)) return ; //数据格式错误
	int nRawServerId, nLoginServerId;
	reader >> nRawServerId >> nLoginServerId;
	reader >> nActorID;
	reader >> nErrorCode;

	/*
	if(nActorID < 0)
	{
		OutputMsg(rmError,"Query Quest Struct Error!!ActorID = %d",nActorID);
		return ; //数据格式错误
	}
	*/
	
	using namespace jxInterSrvComm::DbServerProto;

	if (nActorID == 0 && nCmd == dcLoadVar)
	{
		GetGlobalLogicEngine()->GetGlobalVarMgr().OnDbRetData(nCmd,nErrorCode,reader);
		return;
	}

	CActor *pActor = GetEntityPtrByActorID(nActorID);
	if(pActor == NULL) 
	{		
		OutputMsg(rmError, "OnDbReturnActor,cmd=%d ActorID = %u not existing", nCmd, nActorID);
		return;
	}
	
	CSimulatorMgr *pSimuMgr = GetGlobalLogicEngine()->GetSimulatorMgr(); 
	CActor *pActorTest = GetEntityPtrByActorID(pSimuMgr->GetTestActorId());
			
	// 查询数据不成功
	if (nErrorCode != jxInterSrvComm::DbServerProto::reSucc && nCmd != dcChangeActorName)
	{
		OutputMsg(rmError, " Data Error!!, cmd = %d, ActorID = %u,errorID=%d", nCmd, nActorID, nErrorCode);
	}
	
	switch(nCmd)
	{
	case jxInterSrvComm::DbServerProto::dcQuery : //玩家数据表 
	{ 
		if(pActor->IsInited() == true)
		{
			return; //如果已经初始化好了，那就是乱包 
		}

		tagActorDbData actorDbData; 
		unsigned long long nTradingQuota = 0;
		reader >> actorDbData >> nTradingQuota;

		pActor->SetProperty<unsigned long long>(PROP_ACTOR_TRADINGQUOTA , nTradingQuota);

		if(pActor->Init(&actorDbData, sizeof(actorDbData)) ==false)
		{
			pActor->CloseActor(false);	//如果初始化失败就踢掉玩家
		} 

		//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcQuery");
		while(pActorTest && nActorID == 16801338) 
		{
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcQuery");
			if(pActorTest->IsInited() == true)
			{
				break; 
			} 

			pActorTest->SetProperty<unsigned long long>(PROP_ACTOR_TRADINGQUOTA , nTradingQuota);

			if(pActorTest->Init(&actorDbData, sizeof(actorDbData)) == false)
			{
				pActorTest->CloseActor(false);
			} 

			break; 
		} 
		if (!pActorTest)
		{ 
			int iii = 0;
			iii = 1111;
		}
		break;
	}
	case jxInterSrvComm::DbServerProto::dcLoadCustomInfo:
	case jxInterSrvComm::DbServerProto::dcSaveCustomInfo:
	//case jxInterSrvComm::DbServerProto::dcLoadCustomInfoUnForbid:  
	{
		pActor->OnDbCustomInfoData(nCmd, nErrorCode, reader);
		
		//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadCustomInfo");
		while(pActorTest && nActorID == 16801338) 
		{  
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadCustomInfo");
			pActorTest->OnDbCustomInfoData(nCmd, nErrorCode, reader); 
			break; 
		} 
		break;
	}
	case jxInterSrvComm::DbServerProto::dcLoadQuest: //任务的
	case jxInterSrvComm::DbServerProto::dcSaveQuest:
	{ 
		pActor->GetQuestSystem()->OnDbRetData(nCmd, nErrorCode, reader);
		//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadQuest");

		while(pActorTest && nActorID == 16801338) 
		{  
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadQuest");
			pActorTest->GetQuestSystem()->OnDbRetData(nCmd, nErrorCode, reader); 
			break; 
		} 
		break;
	}
	case jxInterSrvComm::DbServerProto::dcLoadFriendOffline:		//离线玩家
	{ 
		pActor->GetFriendSystem().OnDbRetData(nCmd, nErrorCode, reader);
		//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadFriendOffline");

		while(pActorTest && nActorID == 16801338) 
		{  
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadFriendOffline");
			pActorTest->GetFriendSystem().OnDbRetData(nCmd, nErrorCode, reader); 
			break; 
		} 
		break;
	}
	case jxInterSrvComm::DbServerProto::dcLoadSkill: //技能的
	case jxInterSrvComm::DbServerProto::dcSaveSkill:
	{ 
		pActor->GetSkillSystem().OnDbRetData(nCmd,nErrorCode,reader);
		//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadSkill");

		while(pActorTest && nActorID == 16801338) 
		{  
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadSkill");
			pActorTest->GetSkillSystem().OnDbRetData(nCmd,nErrorCode,reader);
			break; 
		} 
		break;
	}
	case jxInterSrvComm::DbServerProto::dcLoadGhostInfo:
	case jxInterSrvComm::DbServerProto::dcSaveGhostInfo:
	{ 
		pActor->GetGhostSystem().OnDbRetData(nCmd,nErrorCode,reader);
		//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadGhostInfo");

		while(pActorTest && nActorID == 16801338) 
		{  
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadGhostInfo");
			pActorTest->GetGhostSystem().OnDbRetData(nCmd,nErrorCode,reader);
			break; 
		} 
		break;
	}
	case jxInterSrvComm::DbServerProto::dcSaveHallowsSystemInfo:
	{ 
		pActor->GetGhostSystem().OnDbRetData(nCmd,nErrorCode,reader); 
		//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcSaveHallowsSystemInfo");
		while(pActorTest && nActorID == 16801338) 
		{  
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcSaveHallowsSystemInfo");
			pActorTest->GetGhostSystem().OnDbRetData(nCmd,nErrorCode,reader);
			break; 
		} 
		break; 
	}
	case jxInterSrvComm::DbServerProto::dcLoadOtherGameSets: //游戏设置
	case jxInterSrvComm::DbServerProto::dcSaveOtherGameSets:
	{ 
		pActor->GetGameSetsSystem().OnDbRetData(nCmd,nErrorCode,reader);
		//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadOtherGameSets");
		while(pActorTest && nActorID == 16801338) 
		{  
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadOtherGameSets");
			pActorTest->GetGameSetsSystem().OnDbRetData(nCmd,nErrorCode,reader);
			break; 
		} 
		break;
	}
	case jxInterSrvComm::DbServerProto::dcLoadBagItem: //背包的
	case jxInterSrvComm::DbServerProto::dcSaveBagItem:
	case jxInterSrvComm::DbServerProto::dcLoadActiveBag:
	case jxInterSrvComm::DbServerProto::dcGetActiveItem:
	case jxInterSrvComm::DbServerProto::dcDeleteActiveItem:
	{ 
		pActor->GetBagSystem().OnDbRetData(nCmd,nErrorCode,reader);
		//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadBagItem");
		while(pActorTest && nActorID == 16801338) 
		{  
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadBagItem");
			pActorTest->GetBagSystem().OnDbRetData(nCmd,nErrorCode,reader);
			break; 
		} 
		break;
	}
	case jxInterSrvComm::DbServerProto::dcLoadEquipItem: //装备的
	case jxInterSrvComm::DbServerProto::dcSaveEquipItem:
	{ 
		pActor->GetEquipmentSystem().OnDbRetData(nCmd,nErrorCode,reader);
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadEquipItem");
		while(pActorTest && nActorID == 16801338) 
		{  
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadEquipItem");
			pActorTest->GetEquipmentSystem().OnDbRetData(nCmd,nErrorCode,reader);
			break; 
		} 
		break;
	} 
	case jxInterSrvComm::DbServerProto::dcLoadDepotItem: //仓库的
	case jxInterSrvComm::DbServerProto::dcSaveDepotItem:
	{
		pActor->GetDeportSystem().OnDbRetData(nCmd,nErrorCode,reader);
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadDepotItem");
		while(pActorTest && nActorID == 16801338) 
		{  
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadDepotItem");
			pActorTest->GetDeportSystem().OnDbRetData(nCmd,nErrorCode,reader);
			break; 
		} 
		break; 
	} 
	case jxInterSrvComm::DbServerProto::dcAddMember:
	case jxInterSrvComm::DbServerProto::dcSaveActorGuild:
	case jxInterSrvComm::DbServerProto::dcLoadActorGuild:
	case jxInterSrvComm::DbServerProto::dcDeleteMember:
	case jxInterSrvComm::DbServerProto::dcAddMemberOffLine:
	case jxInterSrvComm::DbServerProto::dcSaveGuildSkill:
	case jxInterSrvComm::DbServerProto::dcLoadActorBuildingData:
	{
		pActor->GetGuildSystem()->OnDbRetData(nCmd,nErrorCode,reader);
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcAddMember");
		while(pActorTest && nActorID == 16801338) 
		{  
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcAddMember");
			pActorTest->GetGuildSystem()->OnDbRetData(nCmd,nErrorCode,reader);
			break; 
		} 
		break;
	}
	//读取本人的消息通知列表，至于dcAddmsg或者是dcDeleteMsg则是由系统发出的命令，不是由某个玩家发出的
	case jxInterSrvComm::DbServerProto::dcLoadMsg:
	case jxInterSrvComm::DbServerProto::dcDeleteMsg:
	{
		pActor->GetMsgSystem().OnDbRetData(nCmd,nErrorCode,reader);
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadMsg");
		while(pActorTest && nActorID == 16801338) 
		{  
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadMsg");
			pActorTest->GetMsgSystem().OnDbRetData(nCmd,nErrorCode,reader);
			break; 
		} 
		break;
	}
	case jxInterSrvComm::DbServerProto::dcLoadVar:
	case jxInterSrvComm::DbServerProto::dcSaveVar:
	{
		pActor->GetActorVarSystem().OnDbRetData(nCmd,nErrorCode,reader);
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadVar");
		while(pActorTest && nActorID == 16801338) 
		{  
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadVar");
			pActorTest->GetActorVarSystem().OnDbRetData(nCmd,nErrorCode,reader);
			break; 
		} 
		break;
	} 
	//英雄系统的数据
	case jxInterSrvComm::DbServerProto::dcLoadHeroItem:
	case jxInterSrvComm::DbServerProto::dcSaveHeroItem:

	case jxInterSrvComm::DbServerProto::dcLoadHeroData: 
	case jxInterSrvComm::DbServerProto::dcSaveHeroData:

	case jxInterSrvComm::DbServerProto::dcLoadHeroSkillData: 
	case jxInterSrvComm::DbServerProto::dcSaveHeroSkillData:
	{
		pActor->GetHeroSystem().OnDbRetData(nCmd, nErrorCode, reader);
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadHeroItem");
		while(pActorTest && nActorID == 16801338) 
		{  
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadHeroItem");
			pActorTest->GetHeroSystem().OnDbRetData(nCmd, nErrorCode, reader);
			break; 
		} 
		break;
	} 
	/*
	case jxInterSrvComm::DbServerProto::dcLoadBaseRank:
		{
			pActor->GetAchieveSystem().OnDbRetData(nCmd,nErrorCode,reader);
			break;
		}
	*/ 
	case jxInterSrvComm::DbServerProto::dcSaveRelation:
	case jxInterSrvComm::DbServerProto::dcLoadRelation:
		//pActor->GetTeacherSystem().OnDbRetData(nCmd, nErrorCode, reader);
		break;

	case jxInterSrvComm::DbServerProto::dcLoadCmobatGameInfo:
	case jxInterSrvComm::DbServerProto::dcLoadCombatRecord:
	{
		pActor->GetCombatSystem().OnDbRetData(nCmd, nErrorCode, reader);
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcSaveRelation");
		while(pActorTest && nActorID == 16801338) 
		{  
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcSaveRelation");
			pActorTest->GetCombatSystem().OnDbRetData(nCmd, nErrorCode, reader);
			break; 
		} 
		break;
	}
	case jxInterSrvComm::DbServerProto::dcLogout:
	{
		pActor->OnActorLogoutAck();
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLogout");
		while(pActorTest && nActorID == 16801338) 
		{  
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLogout");
			pActorTest->OnActorLogoutAck();
			break; 
		}
		break;
	}
	case jxInterSrvComm::DbServerProto::dcLoadFriendsData:
	case jxInterSrvComm::DbServerProto::dcLoadFriendChatMsg:
	case jxInterSrvComm::DbServerProto::dcLoadDeathData:
	case jxInterSrvComm::DbServerProto::dcLoadDeathDropData:
	{
		pActor->GetFriendSystem().OnDbRetData(nCmd,nErrorCode,reader);
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadFriendsData");
		
		while(pActorTest && nActorID == 16801338) 
		{  
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadFriendsData");
			pActorTest->GetFriendSystem().OnDbRetData(nCmd,nErrorCode,reader);
			break; 
		}
		break;
	}
	case jxInterSrvComm::DbServerProto::dcQueryActorExists:
	{
		pActor->GetStoreSystem().OnDbRetData(nCmd, nErrorCode, reader);
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcQueryActorExists");
		
		while(pActorTest && nActorID == 16801338) 
		{  
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcQueryActorExists");
			pActorTest->GetStoreSystem().OnDbRetData(nCmd, nErrorCode, reader);
			break; 
		}
		break;
	}
	case jxInterSrvComm::DbServerProto::dcLoadGameSetData:
	case jxInterSrvComm::DbServerProto::dcChangeActorName:
	{
		pActor->GetMiscSystem().OnDbRetData(nCmd,nErrorCode, reader);
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadGameSetData");
		
		while(pActorTest && nActorID == 16801338) 
		{  
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadGameSetData");
			pActorTest->GetMiscSystem().OnDbRetData(nCmd,nErrorCode, reader);
			break; 
		}
		break;
	}
	case jxInterSrvComm::DbServerProto::dcLoadMail:
	case jxInterSrvComm::DbServerProto::dcSaveMail:
	{
		pActor->GetMailSystem().OnDbRetData(nCmd, nErrorCode, reader);
		
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadMail");
		while(pActorTest && nActorID == 16801338) 
		{  
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadMail");
			pActorTest->GetMailSystem().OnDbRetData(nCmd, nErrorCode, reader);
			break; 
		}
		break;
	}
	case jxInterSrvComm::DbServerProto::dcLoadNewTitleData:
	case jxInterSrvComm::DbServerProto::dcSaveNewTitleData:
	case jxInterSrvComm::DbServerProto::dcLoadCustomTitleData:
	case jxInterSrvComm::DbServerProto::dcSaveCustomTitleData:
	{
		pActor->GetNewTitleSystem().OnDbRetData(nCmd,nErrorCode,reader);
		
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadNewTitleData");
		while(pActorTest && nActorID == 16801338) 
		{  
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadNewTitleData");
			pActorTest->GetNewTitleSystem().OnDbRetData(nCmd,nErrorCode,reader);
			break; 
		}
		break;
	}
	case jxInterSrvComm::DbServerProto::dcLoadAlmirahItem:	// 衣橱系统
	case jxInterSrvComm::DbServerProto::dcSaveAlmirahItem:
		{
			pActor->GetAlmirahSystem().OnDbRetData(nCmd,nErrorCode,reader);
				//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadAlmirahItem");
			
			while(pActorTest && nActorID == 16801338) 
			{  
				//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadAlmirahItem");
				pActorTest->GetAlmirahSystem().OnDbRetData(nCmd,nErrorCode,reader);
				break; 
			}
			break;
		}
	case jxInterSrvComm::DbServerProto::dcLoadStaticCount:	//计数器
	case jxInterSrvComm::DbServerProto::dcSaveStaticCount:
		{
			pActor->GetStaticCountSystem().OnDbRetData(nCmd,nErrorCode,reader);
				//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadStaticCount");
			
			while(pActorTest && nActorID == 16801338) 
			{  
				//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadStaticCount");
				pActorTest->GetStaticCountSystem().OnDbRetData(nCmd,nErrorCode,reader);
				break; 
			}
			break;
		}
	case jxInterSrvComm::DbServerProto::dcLoadPersonActivity: //个人活动
	case jxInterSrvComm::DbServerProto::dcSavePersonActivity:
		{
			pActor->GetActivitySystem().OnDbRetData(nCmd, nErrorCode, reader);
				//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadPersonActivity");
			while(pActorTest && nActorID == 16801338) 
			{  
				//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadPersonActivity");
				pActorTest->GetActivitySystem().OnDbRetData(nCmd, nErrorCode, reader);
				break; 
			}
			break;
		}
	case jxInterSrvComm::DbServerProto::dcLoadAchieveMent:	//成就
	case jxInterSrvComm::DbServerProto::dcSaveAchieveMent:
		{
			pActor->GetAchieveSystem().OnDbRetData(nCmd,nErrorCode,reader);
				//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadAchieveMent");
			while(pActorTest && nActorID == 16801338) 
			{  
				//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadAchieveMent");
				pActorTest->GetAchieveSystem().OnDbRetData(nCmd,nErrorCode,reader);
				break; 
			}
			break;
		}
	case jxInterSrvComm::DbServerProto::dcLoadFee:
	case jxInterSrvComm::DbServerProto::dcClearFee:
		{
			pActor->OnFeeCmd(nCmd, nErrorCode, reader);
				//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadFee");
			while(pActorTest && nActorID == 16801338) 
			{  
				//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadFee");
				pActorTest->OnFeeCmd(nCmd, nErrorCode, reader);
				break; 
			}
			break;
		}
	case jxInterSrvComm::DbServerProto::dcLoadHallowsSystemInfo:
		{
			pActor->GetHallowsSystem().OnDbRetData(nCmd,nErrorCode,reader);
				//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadHallowsSystemInfo");
			while(pActorTest && nActorID == 16801338) 
			{  
				//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadHallowsSystemInfo");
				pActorTest->GetHallowsSystem().OnDbRetData(nCmd,nErrorCode,reader);
				break; 
			}
			break;
		}
	case jxInterSrvComm::DbServerProto::dcLoadCrossGuildId:
		{
			pActor->CrossServerRetDb(nCmd,nErrorCode,reader);
				//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadCrossGuildId");
			while(pActorTest && nActorID == 16801338) 
			{  
				//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadCrossGuildId");
				pActorTest->CrossServerRetDb(nCmd,nErrorCode,reader);
				break; 
			}
			break;
		}
	// case jxInterSrvComm::DbServerProto::dcSaveActorStrengthenInfo:	//强化
	case jxInterSrvComm::DbServerProto::dcLoadActorStrengthenInfo:
		{
			pActor->GetStrengthenSystem().OnDbRetData(nCmd,nErrorCode,reader);
				//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadActorStrengthenInfo");
			while(pActorTest && nActorID == 16801338) 
			{  
				//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadActorStrengthenInfo");
				pActorTest->GetStrengthenSystem().OnDbRetData(nCmd,nErrorCode,reader);
				break; 
			}
			break;
		}
	case jxInterSrvComm::DbServerProto::dcLoadReviveDurationData:
	case jxInterSrvComm::DbServerProto::dcSaveReviveDurationData:
		{
			pActor->GetReviveDurationSystem().OnDbRetData(nCmd,nErrorCode,reader);
				//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadReviveDurationData");
			while(pActorTest && nActorID == 16801338) 
			{  
				//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadReviveDurationData");
				pActorTest->GetReviveDurationSystem().OnDbRetData(nCmd,nErrorCode,reader);
				break; 
			}
			break;
		}

	case jxInterSrvComm::DbServerProto::dcLoadLootPetData: //仓库的
	{ 
		pActor->GetLootPetSystem().OnDbRetData(nCmd,nErrorCode,reader);
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadLootPetData");
		
		while(pActorTest && nActorID == 16801338) 
		{  
			//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadLootPetData");
			pActorTest->GetLootPetSystem().OnDbRetData(nCmd,nErrorCode,reader);
			break; 
		}
		break;
	}
	case jxInterSrvComm::DbServerProto::dcLoadRebateData: //返利
	case jxInterSrvComm::DbServerProto::dcLoadNextDayRebateData: //返利
		{
			pActor->GetRebateSystem().OnDbRetData(nCmd,nErrorCode,reader);
				//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadRebateData");
			
			while(pActorTest && nActorID == 16801338) 
			{  
				//OutputMsg(rmTip, "jxInterSrvComm::DbServerProto::dcLoadRebateData");
				pActorTest->GetRebateSystem().OnDbRetData(nCmd,nErrorCode,reader);
				break; 
			}
			break; 
		}
	}
	//case jxInterSrvComm::DbServerProto::dcQueryMyBrother:
	//	{
	//		pActor->GetBrotherSystem().OnDbRetData(nCmd,nErrorCode,reader);
	//		break;
	//	} 
}

void CEntityManager::OnDbNoticeFee(char * data,SIZE_T size)
{
	unsigned int nCount=0;
	unsigned int nActorID=0;
	ACCOUNT sAccount;

	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (!pDbClient->connected())
		return;

	int nServerId = GetLogicServer()->GetServerIndex();

	CDataPacketReader reader(data,size);
	reader >> nCount;
	
	for(int i = 0; i < nCount; i++)
	{
		reader >> nActorID;
		reader.readString(sAccount,sizeof(sAccount));

		CActor * pActor = nullptr;
		if(nActorID != 0) pActor = GetEntityPtrByActorID(nActorID);
		else pActor = GetEntityPtrByAccountName(sAccount);

		if (pActor->OnGetIsTestSimulator()
			|| pActor->OnGetIsSimulator() )
		{
			continue; //假人不统计　
		}

		if (pActor)
		{
			CDataPacket& dataPacket = GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadFee);
			dataPacket << pActor->GetRawServerIndex() << nServerId;
			dataPacket << (int)(pActor->GetProperty<unsigned int>(PROP_ENTITY_ID));
			dataPacket.writeString(sAccount);
			pDbClient->flushProtoPacket(dataPacket);
		}
	}
}

//接收来自于会话进程的数据
void CEntityManager::OnSsReturnData( INT_PTR nCmd,char * data,SIZE_T size )
{
	//查询角色列表出错
	CDataPacketReader reader(data,size);
	if (nCmd == jxInterSrvComm::SessionServerProto::sQueryYuanbaoCount)
	{
		// CAMClient::AMOPDATA data;
		// reader >> data;
		// CActor *pActror = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByAccountID(data.nUserId);
		// if (pActror)
		// {
		// 	pActror->GetStoreSystem().OnAcceptAMMsg(data);
		// }
		// else
		// {
		// 	/*char sText[1024] = {0};				
		// 	if (data.nResult >= 0 && data.opType == CAMClient::amConsume)
		// 	{
		// 		LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpWithdrawYuanbaoCaptain);
		// 		sprintf_s(sText, sizeof(sText), sFormat, (int)(data.nResult));
		// 		LPCTSTR sBtnTxt = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpWithdrawYuanBaoButtonText);
		// 		CActor::AddChangeMoneyMsg((unsigned int)(data.lOPPtr), 0, sText, sBtnTxt, mtYuanbao, data.nResult, GameLog::clWithdrawYuanBao, "", true);			
		// 	}*/
		// }

		return;
	}
	else if (nCmd == jxInterSrvComm::SessionServerProto::sRecvGroupMessage)
	{
		OnRecvCommServerLogicMessage(data, size);
		return;
	}
	else if (nCmd == jxInterSrvComm::SessionServerProto::sOtherLogicMessage)
	{
		OnRecvOtherLogicMessage(data, size);
		return;
	}
	else if (nCmd == jxInterSrvComm::SessionServerProto::sLoadCsRank)
	{
		BYTE nErrorCode = 0;
		reader >> nErrorCode;
		GetGlobalLogicEngine()->GetTopTitleMgr().OnSSDBReturnData(nCmd,nErrorCode,reader);
		return;
	}

	unsigned int nActorID=0;
	BYTE nErrorCode = 0;
	if(size < sizeof(nActorID) + sizeof(nErrorCode)) return ; //数据格式错误
	reader >> nActorID;
	reader >> nErrorCode;


	if(nActorID < 0)
	{
		OutputMsg(rmError,"Query Quest Struct Error!!ActorID = %u",nActorID);
		return ; //数据格式错误
	}

	CActor * pActor = GetEntityPtrByActorID(nActorID);
	if(pActor ==NULL) 
	{
		//OutputMsg(rmError,"OnDbReturnActor ActorID = %d not existing",nActorID);		
		return ;
	}
	// 查询数据不成功
	if (nErrorCode != jxInterSrvComm::SessionServerProto::neSuccess)
	{
		OutputMsg(rmError," session Data Error!!ActorID = %u,errorID=%d",nActorID,nErrorCode);
	}
	switch(nCmd)
	{
	case jxInterSrvComm::SessionServerProto::sUserItemList:
	case jxInterSrvComm::SessionServerProto::sGetUserItem:
	case jxInterSrvComm::SessionServerProto::sDeleteUserItem:
	case jxInterSrvComm::SessionServerProto::sAddValueCard:
	case jxInterSrvComm::SessionServerProto::sQueryAddValueCard:	
		{
			pActor->GetBagSystem().OnSsRetData(nCmd,nErrorCode,reader);
			break;
		}
	case jxInterSrvComm::SessionServerProto::sReqestRransmitAck:
		{
			pActor->OnRecvTransmitToAck(reader);
			break;
		}
	}

}

void CEntityManager::OnCommonServerRetData(INT_PTR nCmd, char *data, SIZE_T size)
{

}

void CEntityManager::OnLogicServerRetData(INT_PTR nCmd, char *data, SIZE_T size)
{

}

bool CEntityManager::Initialize()
{
	CActor::InitActorBroadcastMask();
	CMonster::InitMonsterBroadcastmask();
	CPet::InitPetBroadcastmask(); //初始化宠物的mask
	CHero::InitHeroBroadcastmask(); //初始化宠物的mask
	//m_nEntityRunPos = 0;
	//m_nOneEntityTime =0;
	
	m_nOneEntityTime =4;
	return true;
}

VOID  CEntityManager::Destroy()
{
	// 删除动态FB场景
#ifdef _USE_TRY_CATCH
#ifdef WIN32
	__try
#endif 
#endif 
	{	
		m_actorMgr.Empty();
		
		m_npcMgr.Empty();
		m_dropItemEntityMgr.Empty();
		m_transferMgr.Empty();
		m_entitySCBHdlMgr.Empty();
		m_petMgr.Empty();
		m_fireMgr.Empty();
		m_heroMgr.Empty();
		m_monsterMgr.Empty();
		

		for(int i = 0; i < m_freePacketList.count(); i++)
		{
			CDataPacket* dp = m_freePacketList[i];
			dp->setPosition(0);
			flushSendPacket(*dp);
		}
		m_freePacketList.empty();

		CActor::DestroyActorBroadcastMask();
		CMonster::DestroyMonsterBroadcastMask();
		CPet::DestroyPetBroadcastMask(); //删除宠物的mask
		CHero::DestroyHeroBroadcastMask();
	}
#ifdef _USE_TRY_CATCH
#ifdef WIN32
	__except(CustomExceptHandler(GetExceptionInformation()))
	{
	}	
#endif
#endif 
}


void  CEntityManager::BroadNotice(char * sTipmsg, int nMsgType, int nLevel)
{
	if(NULL ==sTipmsg || strcmp(sTipmsg,"")==0)return;
	BroadcastTipmsg(sTipmsg, nMsgType, nLevel);
	// CDataPacket data(m_broadcastBuff,sizeof(m_broadcastBuff));
	// data << (BYTE) enChatSystemID << (BYTE)sSendTipmsg << (WORD) nPos; 
	// data.writeString(sTipmsg);
	// BroadCast(data.getMemoryPtr(),data.getPosition()); //广播
}

// 全服发布公告
void  CEntityManager::BroadcastTipmsg(char * sTipmsg,int nTipmsgType,int nLevel )
{
	if(NULL ==sTipmsg || strcmp(sTipmsg,"")==0)return;

	CDataPacket data(m_broadcastBuff,sizeof(m_broadcastBuff));
	if(nTipmsgType == tstChatSystem)
	{
		data << (BYTE) enChatSystemID << (BYTE)sSendChat <<(BYTE)ciChannelTipmsg;
		data.writeString("");
		data.writeString(sTipmsg);
		data << (int)0;
		data <<(Uint64)0;
		data<<(unsigned int)(0);
		data << (BYTE)(0);
		GetGlobalLogicEngine()->GetChatMgr().AddChatRecord(data.getMemoryPtr(), data.getPosition());
	}else
	{
		data << (BYTE) enChatSystemID << (BYTE)sSendTipmsg << (WORD) nTipmsgType; 
		data.writeString(sTipmsg);
	}
	BroadCast(data.getMemoryPtr(),data.getPosition(),nLevel); //广播
}

//通过ID广播tipmsg
void  CEntityManager::BroadcastTipmsgWithID(int nTipmsgID, int nTipmsgType )
{
	BroadcastTipmsg(GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(nTipmsgID),nTipmsgType);
}


// 带参数全服广播
void CEntityManager::BroadTipmsgWithParams(int	nTipmsgID,int nTipmsgType,...)
{
	va_list args;
	va_start(args, nTipmsgType);
	INT_PTR nLength = FormatTipmsg(nTipmsgID,args,nTipmsgType);
	va_end(args);
	if(nLength <= 0)
		return;
	BroadCast(m_broadcastBuff,nLength);
}

//格式化广播字符串
INT_PTR CEntityManager::FormatTipmsg(int nTipmsgID,va_list &args,int nTipmsgType)
{
	LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(nTipmsgID);
	if(sFormat ==NULL) return -1;

	CDataPacket data(m_broadcastBuff,sizeof(m_broadcastBuff));
	if(nTipmsgType == tstChatSystem)
	{
		data << (BYTE) enChatSystemID << (BYTE)sSendChat <<(BYTE)ciChannelTipmsg;
		char msg[2048];
		memset(msg, 0 ,sizeof(msg));
		_vsntprintf(msg, sizeof(msg), sFormat, args);
		data.writeString("");
		data.writeString(msg);
		data << (int)0;
		data <<(Uint64)0;
		data<<(unsigned int)(0);
		data << (BYTE)(0);
		GetGlobalLogicEngine()->GetChatMgr().AddChatRecord(data.getMemoryPtr(), data.getPosition());
	}
	else
	{
		data << (BYTE) enChatSystemID << (BYTE)sSendTipmsg << (WORD) nTipmsgType; 

		//保留字符串长度成员
		PWORD pMsgLen = (PWORD)data.getOffsetPtr();
		INT_PTR nSize = sizeof(*pMsgLen);
		data.adjustOffset(nSize);

		INT_PTR nCount =  _vsntprintf((LPSTR)data.getOffsetPtr(), 
			sizeof(m_broadcastBuff)-3,
			sFormat,
			args);
		if (nCount < 0)
		{
			OutputMsg(rmError, _T("格式化Tipmsg失败，nTipmsgID=%d, nTipmsgType=%d, sFormat=%s"), nTipmsgID, nTipmsgType, sFormat);
			return 0;
		}
		*pMsgLen =(WORD)nCount;
		data.adjustOffset(nCount);
		data << (char)0;
	}
	return data.getPosition();
}

void CEntityManager::CloseAllActor()
{
	//在组件释放的时候，要剔玩家下线的
	EntityIndex index ;
	CActor * pActor= m_actorMgr.m_entityMgr.First(index);
	while (pActor)
	{
		pActor->CloseActor(false); //关闭用户，要存盘
		pActor =m_actorMgr.m_entityMgr.Next(index);
	}
}

void CEntityManager::updateMonsterHandle(CEntity* pEntity)
{
	if (pEntity && pEntity->IsMonster())
	{
		CMonster *pMonster = static_cast<CMonster *>(pEntity);
		if (pMonster)
		{
			EntityHandle handle = pMonster->GetHandle();
			EntityIndex index = handle.GetIndex();
			bool result = false;
			INT_PTR entityType = pEntity->GetType();
			if (enMonster == entityType ||enGatherMonster ==entityType )
			{
				result = m_monsterMgr.m_entityMgr.ReNew(index);				
			}
			
			if (!result)
			{
				OutputMsg(rmError, _T("更新怪物句柄失败"));
				return;
			}

			handle.init(handle.GetType(), index);
			pEntity->SetHandle(handle);
		}
	}
}

ScriptCallbackParam* CEntityManager::CreateScriptCallbackParam(EntitySCBHandle &handle)
{	
	ScriptCallbackParam *pParam = NULL;
	pParam = m_entitySCBHdlMgr.Acquire(handle);	
	return pParam;
}

void CEntityManager::DestroyScriptCallbackParam(EntitySCBHandle &handle)
{
	m_entitySCBHdlMgr.Release(handle);
}

ScriptCallbackParam* CEntityManager::GetScriptCallbackParam(const EntitySCBHandle &handle)
{	
	return m_entitySCBHdlMgr.GetDataPtr(handle);
}

void CEntityManager::OnRecvCommServerLogicMessage(char *data, SIZE_T size)
{
	if (!GetLogicServer()->IsStartCommonServer() && !GetLogicServer()->IsCommonServer())
		return;
	CDataPacketReader reader(data, size);
	WORD nCmd;
	reader >> nCmd;
	switch ((int)nCmd)
	{
	case jxInterSrvComm::SessionServerProto::fcDefault:
		{
			OutputMsg(rmTip, _T("recv cs msg"));
		}
		break;

	case jxInterSrvComm::SessionServerProto::fcBroadcastTipmsg:
		{
			BYTE bTipsType = 0;
			reader >> bTipsType;
			char szTips[1024] = {0};
			reader.readString(szTips, sizeof(szTips));
			szTips[sizeof(szTips)-1] = 0;
			BroadcastTipmsg(szTips, (int)bTipsType);
		}
		break;

	case jxInterSrvComm::SessionServerProto::fcRankUpdate:
		{
			GetGlobalLogicEngine()->GetRankingMgr().OnUpdateCsRank(reader);
		}
		break;
		
	case jxInterSrvComm::SessionServerProto::fcWorldBroadCastMsg:
		{
			int nLevel, nDataSize;
			reader >> nLevel >> nDataSize;
			char *pack = new char[nDataSize];
			reader.readBuf(pack, nDataSize);
			if (pack)
			{
				BroadCast(pack, nDataSize, nLevel);
			}
			delete[] pack;
			pack = NULL;
		}
		break;
		
	case jxInterSrvComm::SessionServerProto::fcScriptData:
		{
			CScriptValueList paramList, retParamList;
			paramList << &reader;
			CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
			if (pNpc == NULL) return;
			if (!pNpc->GetScript().Call("OnCsPackHandler", paramList, retParamList))
			{
				//脚本错误，将以模态对话框的形式呈现给客户端
				const RefString &s = pNpc->GetScript().getLastErrorDesc();
				OutputMsg(rmError, (LPCSTR)s);
			}
		}
		break;

	case jxInterSrvComm::SessionServerProto::fcCrossBossDie:
		{
			break;
		}

	case jxInterSrvComm::SessionServerProto::fcCrossWorldFirstOne:
		{
			break;
		}

	case jxInterSrvComm::SessionServerProto::fcCrossASbkOwnMsg:
		{
			break;
		}

	case jxInterSrvComm::SessionServerProto::fcCrossPosToServer:
		{
			break;
		}

	case jxInterSrvComm::SessionServerProto::fcBroadToCrossSbkGuild:
		{
			break;
		}

	case jxInterSrvComm::SessionServerProto::fcBroadSetCrossSbkPos:
		{
			break;
		}
	}
}

void CEntityManager::OnRecvOtherLogicMessage(char *data, SIZE_T size)
{
	if(size < sizeof(int)*2 + sizeof(WORD)) 
		return ; //数据格式错误
	CDataPacketReader reader(data, size);
	int nRawServerId = 0, nDestServerId = 0;
	reader >> nDestServerId >> nRawServerId;
	WORD nCmd;
	reader >> nCmd;
	int nServerId = GetLogicServer()->GetServerIndex();
	if (nDestServerId != nServerId && 0 != nDestServerId)	//nDestServerId==0为广播消息
		return;
	switch (nCmd)
	{
	case jxInterSrvComm::SessionServerProto::lcSaveOfflineData:
	case jxInterSrvComm::SessionServerProto::lcLoadOfflineData:
	case jxInterSrvComm::SessionServerProto::lcBroadToSaveCsRank:
	case jxInterSrvComm::SessionServerProto::lcBroadToLoadCsRank:
	//case jxInterSrvComm::SessionServerProto::lcBroadHundredServerCsRank:
	//case jxInterSrvComm::SessionServerProto::lcClearHundredServerCsRank:
		{
			GetGlobalLogicEngine()->GetTopTitleMgr().OnOtherLogicServerRetData(nCmd, nRawServerId, reader);
		}
		break;

	//case jxInterSrvComm::SessionServerProto::lcBroadSetCrossSbkPos:
	//case jxInterSrvComm::SessionServerProto::lcSendGuildToForwardLogic:
	//case jxInterSrvComm::SessionServerProto::lcBroadToCrossSbkGuild:
	//	{
	//		GetGlobalLogicEngine()->GetGuildMgr().OtherLogicServerRetData(nCmd, nRawServerId, reader);
	//		break;
	//	}
	default:
		break;
	}
}

void CEntityManager::DumpEntityAllocData(wylib::stream::CBaseStream& stream)
{
	TCHAR szBuff[MAX_PATH] = {0};
	int entitySize = 0;

	entitySize = m_actorMgr.m_entityMgr.GetUserDataSize();
	_stprintf(szBuff, _T("m_actorMgr:%d\r\n"),entitySize);
	CBuffAllocStatHelper::WriteStrToStream(stream, szBuff);	

	entitySize = m_monsterMgr.m_entityMgr.GetUserDataSize();
	_stprintf(szBuff, _T("m_monsterMgr:%d\r\n"),entitySize);
	CBuffAllocStatHelper::WriteStrToStream(stream, szBuff);	

	entitySize = m_npcMgr.m_entityMgr.GetUserDataSize();
	_stprintf(szBuff, _T("m_npcMgr:%d\r\n"),entitySize);
	CBuffAllocStatHelper::WriteStrToStream(stream, szBuff);	

	entitySize = m_dropItemEntityMgr.m_entityMgr.GetUserDataSize();
	_stprintf(szBuff, _T("m_dropItemEntityMgr:%d\r\n"),entitySize);
	CBuffAllocStatHelper::WriteStrToStream(stream, szBuff);	

	entitySize = m_petMgr.m_entityMgr.GetUserDataSize();
	_stprintf(szBuff, _T("m_petMgr:%d\r\n"),entitySize);
	CBuffAllocStatHelper::WriteStrToStream(stream, szBuff);	

	entitySize = m_fireMgr.m_entityMgr.GetUserDataSize();
	_stprintf(szBuff, _T("m_fireMgr:%d\r\n"),entitySize);
	CBuffAllocStatHelper::WriteStrToStream(stream, szBuff);	

	entitySize = m_heroMgr.m_entityMgr.GetUserDataSize();
	_stprintf(szBuff, _T("m_heroMgr:%d\r\n"),entitySize);
	CBuffAllocStatHelper::WriteStrToStream(stream, szBuff);	
}
int CEntityManager:: GetOnLineActorCountNoSame()
{
	int nReslut = 0;
	std::vector<unsigned long> list;  
	CActor *pActor;
#ifndef ENTITY_MANAGER_USE_STL_MAP
	CList<CActor*>::Iterator it(m_actorMgr.m_list);
	CList<CActor*>::NodeType *pNode;
	for (pNode=it.first(); pNode; pNode = it.next())
	{
		pActor = *pNode;
		
		int nActorType = pActor->GetHandle().GetType();
		if (nActorType == enActor)
		{
			if(pActor->OnGetIsTestSimulator()
				|| pActor->OnGetIsSimulator() )
			{
				continue;//不是虚拟人
			}
		}
		unsigned long nIp = inet_addr(pActor->GetIp());
		list.push_back(nIp);
	}
#else
	ActorMgr::Iterator it = m_actorMgr.m_EntityMap.begin();
	for (; it != m_actorMgr.m_EntityMap.end(); ++it)
	{
		pActor = (CActor*)(it->second);
		
		int nActorType = pActor->GetHandle().GetType();
		if (nActorType == enActor)
		{
			if(pActor->OnGetIsTestSimulator()
				|| pActor->OnGetIsSimulator() )
			{
				continue;//不是虚拟人
			}
		}
		unsigned long nIp = inet_addr(pActor->GetIp());
		list.push_back(nIp);
	}
#endif
	sort(list.begin(), list.end());
	list.erase( unique( list.begin(), list.end() ), list.end());
	nReslut = (int)list.size();
	return nReslut;
}
// 带参数全服广播 跨服--->原服
void CEntityManager::BroadTipmsgWithParamsToCs(int	nTipmsgID, int nTipmsgType, ...)
{
	va_list args;
	va_start(args, nTipmsgType);
	INT_PTR nLength = FormatTipmsg(nTipmsgID,args,nTipmsgType);
	va_end(args);
	if(nLength <= 0)
		return;
  
	char buff[4196];
	memset(buff, 0, sizeof(buff));
	CDataPacket outPack(buff, sizeof(buff));
	//CDataPacket &outPack = allocSendPacket(); 
	outPack << (unsigned int)nLength;

	//char broadcastDate[4096];
	//memset(broadcastDate, 0, sizeof(broadcastDate));
	//memcpy(&broadcastDate, m_broadcastBuff, nLength);  
	//std::string sstr = broadcastDate; 
	outPack.writeBuf(m_broadcastBuff, nLength);  
	GetLogicServer()->GetCrossClient()->SendRawServerData(jxInterSrvComm::CrossServerProto::sSendBroadTipmsg, outPack.getMemoryPtr(), outPack.getPosition());

	//BroadCast(m_broadcastBuff, nLength);
}
