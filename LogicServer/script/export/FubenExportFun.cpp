
#include "StdAfx.h"
#include "../interface/FubenExportFun.h"

namespace Fuben
{
	unsigned int createFuBen( int nFbID)
	{
		CFuBen* pFb = GetGlobalLogicEngine()->GetFuBenMgr()->CreateFuBen(nFbID);
		if (pFb)
		{
			unsigned int hHandle = pFb->GetHandle();
			return hHandle;
		}
		else
		{
			return 0;
		}
	}

	int getTeamOnlineMemberCount(unsigned int nTeamId)
	{
		CTeam* pTeam = GetGlobalLogicEngine()->GetTeamMgr().GetTeam(nTeamId);
		if (!pTeam) return 0;
		return (int)(((CTeam*)pTeam)->GetOnlineUserCount());
	}


	int getTeamMemberCount( unsigned int nTeamId )
	{
		CTeam* pTeam = GetGlobalLogicEngine()->GetTeamMgr().GetTeam(nTeamId);
		if (!pTeam) return 0;
		return (int)(((CTeam*)pTeam)->GetMemberCount());
	}

	int getTeamMaxMemberCount( unsigned int nTeamId )
	{
		CTeam* pTeam = GetGlobalLogicEngine()->GetTeamMgr().GetTeam(nTeamId);
		if (pTeam)
		{
			return (int)(pTeam->GetTeamMaxMemberCount());
		}
		return 0;
	}

	int getTeamFubenId(unsigned int nTeamId)
	{
		CTeam* pTeam = GetGlobalLogicEngine()->GetTeamMgr().GetTeam(nTeamId);
		if (!pTeam) return 0;
		return ((CTeam*)pTeam)->GetFbId();
	}

	bool isRefuseActor( unsigned int hFbHandle,unsigned int nAcotid )
	{
		CFuBen* pFb =  CFuBenManager::m_FuBenMgr->GetDataPtr(hFbHandle);
		if (pFb) 
		{
			return pFb->IsRefuseActor(nAcotid);
		}
		return false;
	}

	void addRefuseActor( unsigned int hFbHandle,unsigned int nAcotid )
	{
		CFuBen* pFb =  CFuBenManager::m_FuBenMgr->GetDataPtr(hFbHandle);
		if (pFb) 
		{
			pFb->AddRefuseActor(nAcotid);
		}
	}

	int getFubenIdByPtr(void* pFb)
	{
		if (pFb) 
		{
			return ((CFuBen*)pFb)->GetFbId();
		}
		return 0;
	}


	unsigned int getGuildIdByByPtr(void* pFb)
	{
		if (pFb) 
		{
			return ((CFuBen*)pFb)->GetGuildId();
		}
		return 0;
	}

	void* getFubenPtrByHandle(unsigned int hFuben)
	{
		CFuBen* pFb = CFuBenManager::m_FuBenMgr->GetDataPtr(hFuben);
		if (pFb) 
		{
			return pFb;
		}
		return NULL;
	}

	void closeFuben( unsigned int hFuben )
	{
		CFuBen* pFb =  CFuBenManager::m_FuBenMgr->GetDataPtr(hFuben);
		if (pFb) 
		{
			GetGlobalLogicEngine()->GetFuBenMgr()->ReleaseFuBen(pFb);
		}
	}

	unsigned int getFubenHandle( void* pFuben )
	{
		if (!pFuben) return 0;
		return ((CFuBen*)pFuben)->GetHandle();
	}

	unsigned int getStaticFubenHandle()
	{
		CFuBen* pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
		if (pFb)
		{
			return (unsigned int)(pFb->GetHandle());
		}
		return 0;
	}

	void* createMonster( unsigned int hScene,unsigned int nMonsterid,int posX,int posY,unsigned int nLiveTime, unsigned int nBornLevel )
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene)
		{
			PMONSTERCONFIG pConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nMonsterid);
			if(pConfig ==NULL) return NULL;
			int nEntityType = pConfig->btEntityType;
			CEntity* pMonster = 
				pScene->CreateEntityAndEnterScene(nMonsterid, nEntityType, posX, posY,-1,NULL,nLiveTime, nBornLevel);
			if (pMonster != NULL)
			{
				//pScene->SetMonsterConfig(pMonster,-1,nMonsterid);
				//((CMonster*)pMonster)->SetLiveTIme(nLiveTime);
				return pMonster;
			}
		}
		return NULL;
	}


	
	int getreateMonsterPosXY(lua_State* L)
	{
		int hScene = ((int)  lua_tonumber(L,1));
		int posX = ((int)  lua_tonumber(L,2));
		int posY = ((int)  lua_tonumber(L,3));
		int nRange = ((int)  lua_tonumber(L,4));
		lua_newtable(L);
		INT_PTR nMonsterPosX = 0;
    	INT_PTR nMonsterPosY = 0;
		LPCTSTR nSceneName = NULL;
		do
		{
			CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
			if (pScene)
			{
				REFRESHCONFIG referCfg;
				referCfg.nMobX =posX;
				referCfg.nMobY =posY;
				referCfg.nMobRange = nRange;
				if (!pScene->GetMobMonsterXY(&referCfg,nMonsterPosX,nMonsterPosY))
					break;
				nSceneName = pScene->GetSceneName();
			}
		}while (false);
		
		lua_pushstring(L, "name");
		lua_pushstring(L, nSceneName);
		lua_settable(L,-3);
		lua_pushstring(L, "x");
		lua_pushinteger(L, nMonsterPosX);
		lua_settable(L,-3);
		lua_pushstring(L, "y");
		lua_pushinteger(L, nMonsterPosY);
		lua_settable(L,-3);
		return 1;
	}

	void setMonsterIndex(void* pEntity, int nIdx)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enMonster) return;
		((CMonster*)pEntity)->SetIndex(nIdx);
	}

	void ResetMonsterShowName(void* pEntity, const char*  sName)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enMonster) return;
		((CMonster*)pEntity)->ResetShowName(sName);
	}

	bool AddMonsterBuffById(void* pEntity, int nBuffId)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enMonster) return false;
		return ((CMonster*)pEntity)->GetBuffSystem()->Append(nBuffId) ? true:false;
	}

	void DelMonsterBuffById(void* pEntity, int nBuffId)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enMonster) return;
		((CMonster *)pEntity)->GetBuffSystem()->RemoveById(nBuffId);
	}

	void createMonsters( unsigned int hScene,unsigned int nMonsterid,int posX1,int posX2,int posY,int nCount,unsigned int nLiveTime, unsigned int nBornLevel )
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene)
		{
			PMONSTERCONFIG pConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nMonsterid);
			if(pConfig ==NULL) return;
			int nEntityType = pConfig->btEntityType;
			for (INT_PTR i = 0; i < nCount; i++)
			{
				unsigned long seed = posX2 - posX1;
				seed = wrand(seed+1);
				CEntity* pMonster = 
					pScene->CreateEntityAndEnterScene(nMonsterid, nEntityType,	posX1+seed,posY,-1,NULL,nLiveTime, nBornLevel);
				/*
				if (pMonster != NULL)
				{
					pScene->SetMonsterConfig(pMonster,-1,nMonsterid);
					((CMonster*)pMonster)->SetLiveTIme(nLiveTime);
				}
				*/

			}
		}
	}

	int createMonstersInRange(unsigned int hScene,unsigned int nMonsterid,int posX1,int posY1,int posX2,int posY2,int nCount,
		unsigned int nLiveTime, unsigned int nBornLevel, char* szMonsterName, unsigned int nForceVesterId,
		bool bForcePos)
	{
		int nRetCount = 0;
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene)
		{
			if (nCount <= 0){
				OutputMsg(rmError, _T("createMonstersInRange error: nCount <= 0"));
				return 0;
			}
			PMONSTERCONFIG pConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nMonsterid);
			if(pConfig ==NULL) return 0;

			bool boAlwaysEnter = pConfig->pflags->CanAlwaysEnter;

			int nX = 0, nY = 0;
			pScene->GetAreaCenter(nX, nY);
			int nCenterX = nX;
			int nCenterY = nY;
			int nEntityType = pConfig->btEntityType;
			if (enNpc == nEntityType)
			{
				return 0;
			}
			for (INT_PTR i = 0; i < nCount; ++i)
			{
				if (pScene->GetRandomPointByEntityType(nEntityType, posX1, posY1, posX2, posY2, nX, nY, boAlwaysEnter, bForcePos))
				{
					CEntity* pMonster = 
						pScene->CreateEntityAndEnterScene(nMonsterid, nEntityType,	nX, nY, -1, NULL, nLiveTime, nBornLevel,
							szMonsterName, nForceVesterId);

					if (pMonster) 
						nRetCount++;
					if (nCenterX == nX && nCenterY ==nY && (!(nX >= posX1 && nX <=posX2) || !(nY >= posY1 && nY <=posY2)))
					{
						OutputMsg(rmTip,"createMonstersInRange To AreaCenter MonsterId =%d,posX1=%d,posX2=%d,posY1=%d,posY2=%d,boAlwaysEnter=%d",nMonsterid,posX1,posX2,posY1,posY2,boAlwaysEnter);
					}
				}
			}
		}

		return nRetCount;
	}

	void* createOneMonsters(unsigned int hScene,unsigned int nMonsterid,int posX,int posY,int nCount,unsigned int nLiveTime, unsigned int nBornLevel, char* szMonsterName,
						int nForceVesterId,
						int nInitMonsterPercent)
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene)
		{ 
			int id = pScene->GetSceneId();
			PMONSTERCONFIG pConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nMonsterid);
			if(pConfig ==NULL) return NULL;

			int nEntityType = pConfig->btEntityType;
			if(nEntityType ==enActor) //配置错误
			{
				return NULL;
			}
			CEntity* pMonster = pScene->CreateEntityAndEnterScene(nMonsterid, nEntityType,posX,posY,-1,NULL,nLiveTime, nBornLevel,szMonsterName,nForceVesterId,nInitMonsterPercent);
			return pMonster;
		}

		return NULL;
	}
		
	int getMonsterCount(void* pFuben, int nMonsterId)
	{
		return ((CFuBen*)pFuben)->GetMonsterCount(nMonsterId,0);
	}

	int getSceneMonsterCount(void* pFuben, int nScenId, int nMonsterId)
	{
		return ((CFuBen*)pFuben)->GetMonsterCount(nMonsterId,nScenId);
	}

	int createMonstersAndSetPathpoint(lua_State *L)
	{
		int n = lua_gettop(L);
		if (n < 7)
		{
			OutputMsg(rmError, _T("createMonstersAndSetPathpoint error, not enough param"));
			return 0;
		}

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 5)
			|| !lua_isnumber(L, 6) || !lua_isnumber(L, 7) )
		{
			OutputMsg(rmError, _T("createMonstersAndSetPathpoint error, param type invalid"));
			return 0;
		}

		unsigned int hScene = ((unsigned int)  lua_tonumber(L,1));
		unsigned int nMonsterid = ((unsigned int)  lua_tonumber(L,2));
		int posX1 = ((int)  lua_tonumber(L,3));
		int posX2 = ((int)  lua_tonumber(L,4));
		int posY = ((int)  lua_tonumber(L,5));
		int nCount = ((int)  lua_tonumber(L,6));
		unsigned int nLiveTime = ((unsigned int)  lua_tonumber(L,7));
		CVector<int> pathpoint;
		for (int i = 8; i <= n; i++)
		{
			if (lua_isnumber(L, i))
				pathpoint.add((int)lua_tonumber(L, i));
		}

		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene)
		{
			for (INT_PTR i = 0; i < nCount; i++)
			{
				unsigned long seed = posX2 - posX1;
				seed = wrand(seed+1);
				CEntity* pMonster = 
					pScene->CreateEntityAndEnterScene(nMonsterid,enMonster,	posX1+seed,posY,-1,NULL,nLiveTime);
				if (pMonster != NULL)
				{
					//pScene->SetMonsterConfig(pMonster,-1,nMonsterid);
					//((CMonster*)pMonster)->SetLiveTIme(nLiveTime);
					//CAnimalAI *pAI = ((CMonster*)pMonster)->GetAI();
					
				}
			}
		}
		return 0;
	}

	CScene * getScenePtrById( int nSceneId)
	{
		CFuBen * pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
		if (pFb != NULL)
		{
			return pFb->GetScene(nSceneId);
		}
		return NULL;
	}

	CScene * getScenePtrByName(const char * pSceneName)
	{
		CFuBen * pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
		if (pFb != NULL)
		{
			return pFb->GetSceneByName(pSceneName);
		}
		return NULL;
	}

	unsigned int getSceneHandleByName( const char* sSceneName,unsigned int hFuben )
	{
		CFuBen* pFb = NULL;
		if (hFuben == 0)
		{
			pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
		}else
		{
			pFb = CFuBenManager::m_FuBenMgr->GetDataPtr(hFuben);
		}
		if (pFb)
		{
			CScene* pScene = pFb->GetSceneByName(sSceneName);
			return pScene?(unsigned int)pScene->GetHandle():0;
		}
		return 0;
	}

	unsigned int getSceneHandleById( int nSceneId,unsigned int hFuben )
	{
		CFuBen* pFb = NULL;
		if (hFuben == 0)
		{
			pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
		}else
		{
			pFb = CFuBenManager::m_FuBenMgr->GetDataPtr(hFuben);
		}
		if (pFb)
		{
			CScene* pScene = pFb->GetScene(nSceneId);
			return pScene?(unsigned int)pScene->GetHandle():0;
		}
		return 0;
	}	

	int getMyMonsterCount( unsigned int hScene,int nMonsterId )
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene)
		{
			return pScene->GetMonsterCount(nMonsterId,0);
		}
		return 0;
	}

	int getFbValue( void * pFb,int nIndex )
	{
		return ((CFuBen*)pFb)->GetFbValue(nIndex);
	}

	void setFbValue( void * pFb, int nIndex, int nValue )
	{
		((CFuBen*)pFb)->SetFbValue(nIndex,nValue);
	}

	void clearMonster( unsigned int hScene,int nMonsterId )
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene)
		{
			CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
#ifndef MONSTER_MANAGER_BY_SCENE
			CEntityList& list = pScene->GetMonsterList();
			CLinkedNode<EntityHandle> *pNode;
			CLinkedListIterator<EntityHandle> it(list);
			for (pNode = it.first(); pNode; pNode = it.next())
			{
				CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
#else
			std::map<EntityHandle,CMonster*>::iterator iter = pScene->GetMonsterList().begin();
			for (; iter != pScene->GetMonsterList().end() ; ++iter)
			{
				CEntity* pEntity = pEntityMgr->GetEntity(iter->first);
#endif
				if(!pEntity)continue;
				unsigned int id(pEntity->GetId());
				if (!nMonsterId || id == nMonsterId )
				{
					EntityHandle handle = pEntity->GetHandle();
					if (pEntity->IsDeath())
					{
						pScene->ClearDeathEntity(handle);
					}
					pEntityMgr->DestroyEntity(handle);
				}
			}
		}
	}

	void clearMonster(double handle, bool bDelayDestroy)
	{
		INT64 h;
		memcpy(&h, &handle, sizeof(handle));
		EntityHandle eh = h;
		CEntity *pEntity = GetEntityFromHandle(eh);
		if (pEntity)
		{			
			INT64 nType = pEntity->GetType();
			
			if(nType == enMonster || nType == enGatherMonster )
			{
				GetGlobalLogicEngine()->GetEntityMgr()->DestroyEntity(eh, bDelayDestroy);
			}
			else
			{
				OutputMsg(rmWaning,"%s clear monster type error, handle=%lld,name=%s",__FUNCTION__,h,pEntity->GetEntityName());
			}
		}
		else
		{
			//OutputMsg(rmWaning,"%s clear monster fail, handle=%lld",__FUNCTION__,h);
		}
	}
	void clearSceneEntity(unsigned int hScene, int nType)
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene)
		{
			pScene->ClearEntity(nType);
		}
	}
	void killMonsterByActor(void* pMonster, void* pEntity)
	{
		if (!pMonster || (((CEntity*)pMonster)->GetType() != enMonster&& ((CEntity*)pMonster)->GetType() != enGatherMonster))
			return;
		if (!pEntity || ((CEntity*)pEntity)->GetType() != enActor)
			return;
		CMonster* peMonster = (CMonster*)pMonster;
		CEntity* peActor = (CEntity*)pEntity;
		peMonster->SetVestEntity(peActor->GetHandle());
		if (peMonster->GetAttriFlag().boShowVestEntityName)
		{
			peMonster->SetVestEntityName(peActor->GetEntityName());
		}
		peMonster->ChangeHP(-1 *peMonster->GetProperty<int>(PROP_CREATURE_HP));
		peMonster->OnKilledByEntity(peActor);
	}
	void sendMsgToScene( unsigned int hScene, char* sMsg ,int nType)
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene != NULL)
		{
			pScene->SendMsg(sMsg, nType);
		}
	}
	void sendCountDown( unsigned int hScene,int nTime,int nFlag)
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene)
		{
			CEntityList& list = pScene->GetPlayList();
			CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
			CLinkedNode<EntityHandle> *pNode;
			CLinkedListIterator<EntityHandle> it(list);
			for (pNode = it.first(); pNode; pNode = it.next())
			{
				CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
				if (pEntity && pEntity->GetType() == enActor)
				{
					((CActor*)pEntity)->GetMiscSystem().SendCountdown(nTime,nFlag);
				}		
			}
		}
	}

	void sendMsgToFb( unsigned hFb,char* sMsg,int nType )
	{
		CFuBen* pFb =  CFuBenManager::m_FuBenMgr->GetDataPtr(hFb);
		if (pFb) 
		{
			CFuBen::SceneList& list = pFb->GetSceneList();
			for (int i = 0; i < list.count(); i++)
			{
				CScene* scene = list[i];
				if (scene != NULL)
				{
					scene->SendMsg(sMsg, nType);
				}
			}
		}
	}

	void * getNpcPtrById( unsigned int nSceneId, unsigned int nNpcId)
	{
		CFuBen * pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
		if (pFb != NULL)
		{
			CScene * pScene = pFb->GetScene(nSceneId);
			if (pScene != NULL)
			{
				return pScene->GetNpc(nNpcId);
			}
		}
		return NULL;
	}

	void* getNpcPtrByName( unsigned int hScene,char* szName )
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene)
		{
			return pScene->GetNpc(szName);
		}
		return NULL;
	}

	void* getFbActorPtr( unsigned int hFb )
	{
		CFuBen* pFb =  CFuBenManager::m_FuBenMgr->GetDataPtr(hFb);
		if (pFb) 
		{
			CFuBen::SceneList& list = pFb->GetSceneList();
			for (int i = 0; i < list.count(); i++)
			{
				CScene* pScene = list[i];
				if (pScene)
				{
					CEntityList& list = pScene->GetPlayList();
					CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
					CLinkedNode<EntityHandle> *pNode;
					CLinkedListIterator<EntityHandle> it(list);
					for (pNode = it.first(); pNode; pNode = it.next())
					{
						CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
						if (pEntity && pEntity->GetType() == enActor)
						{
							return pEntity;
						}		
					}
				}
			}
		}
		return NULL;
	}

	void* getMonsterPtrById( unsigned int hScene,int nMonsterId )
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene)
		{
			CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
#ifndef MONSTER_MANAGER_BY_SCENE
			CEntityList& list = pScene->GetMonsterList();
			CLinkedNode<EntityHandle> *pNode;
			CLinkedListIterator<EntityHandle> it(list);
			for (pNode = it.first(); pNode; pNode = it.next())
			{
				CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
#else
			std::map<EntityHandle,CMonster*>::iterator iter = pScene->GetMonsterList().begin();
			for (; iter != pScene->GetMonsterList().end() ; ++iter)
			{
				CEntity* pEntity = pEntityMgr->GetEntity(iter->first);
#endif
				if (pEntity && (pEntity->GetProperty<int>(PROP_ENTITY_ID) == nMonsterId))
				{
					return pEntity;
				}
			}
		}
		return NULL;
	}

	void* getMonsterPtrByIdx( unsigned int hScene,int nMonsterId, int nMonsterIdx)
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene)
		{
			CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
#ifndef MONSTER_MANAGER_BY_SCENE
			CEntityList& list = pScene->GetMonsterList();
			CLinkedNode<EntityHandle> *pNode;
			CLinkedListIterator<EntityHandle> it(list);
			for (pNode = it.first(); pNode; pNode = it.next())
			{
				CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
#else
			std::map<EntityHandle,CMonster*>::iterator iter = pScene->GetMonsterList().begin();
			for (; iter != pScene->GetMonsterList().end() ; ++iter)
			{
				CEntity* pEntity = pEntityMgr->GetEntity(iter->first);
#endif
				if (pEntity && (pEntity->GetProperty<int>(PROP_ENTITY_ID) == nMonsterId) && pEntity->GetIndex() == nMonsterIdx )
				{
					return pEntity;
				}		
			}
		}
		return NULL;
	}

	int getMonsterBeAttackActorList(lua_State *L)
	{
		void* pPtr =  tolua_touserdata(L,1,0);
		CEntity* pEntity = (CEntity*)pPtr;
		if (!pEntity || pEntity->GetType() != enMonster)
			return 0;
		CVector<unsigned int>& pAttackList = ((CMonster*)pEntity)->GetBeAccactedActorList();
		LuaHelp::PushNumberVector(L, &(pAttackList[0]), pAttackList.count());
		return 1;
	}

	bool isKillAllMonster( unsigned int hScene,int nMonsterId /*= 0*/ )
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene)
		{
			return pScene->IsKillAllMonster(nMonsterId);
		}
		return false;
	}

	int getLiveMonsterCount( unsigned int hScene,int nMonsterId )
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene)
		{
			return pScene->GetMonsterCount(nMonsterId,0);
		}
		return 0;
	}

	void setSceneTime( unsigned int hScene, int nTime )
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene)
		{
			pScene->SetSceneTime(nTime);
		}
	}

	int getSceneTime(unsigned int hScene)
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene)
		{
			return pScene->GetSceneTime();
		}
		return 0;
	}

	const char* getSceneNameById( int nSceneId )
	{
		FUBENVECTOR* pList = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbList();
		for(INT_PTR i =0 ; i< pList->count(); i++)
		{
			CFuBen *pFb = (*pList)[i];
			if(pFb )
			{
				CScene* pScene = pFb->GetScene(nSceneId);
				if(pScene)
				{
					return pScene->GetSceneName();
				}
			}
		}
		return "";
	}

	int GetAreaListIndex(int nSceneId,int x, int y, unsigned int fbHandle)
	{
		CFuBen *pFb =NULL;
		if (fbHandle == 0)
		{
			pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
		}
		else
		{
			pFb = CFuBenManager::m_FuBenMgr->GetDataPtr(fbHandle);
		}
		if (pFb)
		{
			CScene * pScene = pFb->GetScene(nSceneId);
			if(pScene)
			{
				return pScene->GetAreaListIndex(x,y);
			}
		}
		return -1;
	}

	bool setSceneAreaAttri( unsigned int hScene,int nIndex,int nType,const char* sValues,const char* sFunc/*=NULL*/, double hNpc/*=0*/, int notips/*=0*/)
	{
		int values[256];
		char buf[32];

		INT_PTR nCount = 0;
		int pos = 0;
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene)
		{
			if (sValues)
			{
				int len = (int)strlen(sValues);
				for ( int i = 0; (i < len) && (pos <= sizeof(buf)/sizeof(char)); i++)
				{
					if (*sValues != ',')
					{
						buf[pos++] = *sValues;
					}else
					{
						buf[pos] = 0;
						if (nCount < 256)
						{
							values[nCount++] = _StrToInt(buf);
						}
						
						pos = 0;
					}
					sValues++;
				}
			}
			if (pos != 0 && nCount < 256)
			{
				values[nCount++] = _StrToInt(buf);
				pos = 0;
			}
			unsigned long long d;
			memcpy(&d,&hNpc ,sizeof(hNpc));
			return pScene->SetAreaAttri(nIndex,nType,values,nCount,sFunc,d);
		}
		return false;
	}

	int getKillBossCount( void* pfb )
	{
		if (pfb)
		{
			return (int)(((CFuBen*)pfb)->GetKillBossCount());
		}
		return 0;
	}

	const char* getKillBossActorName( void *pfb,int index )
	{
		if (pfb)
		{
			return ((CFuBen*)pfb)->GetKillBossActorName(index);
		}
		return "";
	}

	int getFbTime( void* pfb )
	{
		if (pfb)
		{
			return ((CFuBen*)pfb)->GetFubenTime();
		}
		return 0;
	}

	void setFbTime(void * pFb, unsigned int nTime)
	{
		if (pFb != NULL)
		{
			((CFuBen *)pFb)->SetFubenTime(nTime);
		}
	}

	int getActorDieCount( void* pfb )
	{
		if (pfb)
		{
			return (int)(((CFuBen*)pfb)->GetActorDieCount());
		}
		return 0;
	}

	int getFbMonsterTotal( void* pfb )
	{
		if (pfb)
		{
			return (int)(((CFuBen*)pfb)->GetMonsterTotal());
		}
		return 0;
	}

	int getKillMonsterCount( void* pfb )
	{
		if (pfb)
		{
			return (int)(((CFuBen*)pfb)->GetKillMonsterCount());
		}
		return 0;
	}

	int getDyanmicVar( lua_State *L )
	{
		CFuBen * pfb = (CFuBen*)lua_touserdata(L, 1);
		if (!pfb)
			return 0;
		return LuaCLVariant::returnValue(L, pfb->GetVar());
	}

	void clearDynamicVar(lua_State *L)
	{
		CFuBen * pfb = (CFuBen*)lua_touserdata(L, 1);
		if (!pfb) return;
		pfb->GetVar().clear();
	}

	void setSceneNpcCanNotSee(int nSceneId,bool bSet)
	{
		FUBENVECTOR* pList = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbList();
		for(INT_PTR i =0 ; i< pList->count(); i++)
		{
			CFuBen *pFb = (*pList)[i];
			if(pFb )
			{
				CScene* pScene = pFb->GetScene(nSceneId);
				if(pScene)
				{
					pScene->SetNpcCanSeeFlag(bSet);
					return;
				}
			}
		}
	}
	
	int getScenePlayerCount(unsigned int nScene)
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(nScene);	
		if (pScene)
		{
			return (int)(pScene->GetPlayCount());
		}
		return 0;
	}

	int getSceenWidth(unsigned int nScene)
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(nScene);	
		if (pScene)
		{
			return (int)(pScene->GetSceneWidth());
		}

		return 0;
	}

	int getScenHeight(unsigned int nScene)
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(nScene);	
		if (pScene)
		{
			return (int)(pScene->GetSceneHeight());
		}

		return 0;
	}

	bool onCanMove(unsigned int nScene,int x,int y)
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(nScene);	
		if (pScene)
		{
			return pScene->m_pMapData->canMove((DWORD)x,(DWORD)y);
		}

		return false;
	}

	void * getNpcPtrFromAllScenes( const char * sNpcName )
	{
		CFuBen * pComFuben = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
		if (pComFuben == NULL)
		{
			return NULL;
		}
		CNpc * pRetNpc = NULL;
		CFuBen::SceneList & comSceneList = pComFuben->GetSceneList();
		INT_PTR nSceneCount = comSceneList.count();
		for (INT_PTR i = 0; i < nSceneCount; i++)
		{
			CScene * pOneScene = comSceneList[i];
			if(pOneScene) {
				pRetNpc = pOneScene->GetNpc(sNpcName);
				if (pRetNpc != NULL)
				{
					return pRetNpc;
				}
			}
		}
		return pRetNpc;
	}

	bool hasMapAreaAttri(void* pScene, int nPosX, int nPosY, int nAttriValue)
	{
		if (pScene == NULL) return false;
		return ((CScene *)pScene)->HasMapAttribute(nPosX,nPosY,nAttriValue);
	}

	void * getFbNpcPtrById( unsigned int nSceneHandle, unsigned int nNpcId )
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(nSceneHandle);	
		if (pScene != NULL)
		{
			return pScene->GetNpc(nNpcId);
		}
		return NULL;
	}

	void clearAllMonster(unsigned int hScene)
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene)
		{
			CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
#ifndef MONSTER_MANAGER_BY_SCENE
			CEntityList& list = pScene->GetMonsterList();
			CLinkedNode<EntityHandle> *pNode;
			CLinkedListIterator<EntityHandle> it(list);
			for (pNode = it.first(); pNode; pNode = it.next())
			{
				CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
#else
			std::map<EntityHandle,CMonster*>::iterator iter = pScene->GetMonsterList().begin();
			for (; iter != pScene->GetMonsterList().end() ; ++iter)
			{
				CEntity* pEntity = pEntityMgr->GetEntity(iter->first);
#endif
				if(!pEntity)continue;
				unsigned int id(pEntity->GetId());
				if (!pEntity->IsDeath())
				{
					pEntityMgr->DestroyEntity(pEntity->GetHandle());
				}		
			}
		}
	}

	int getSceneId(unsigned int hScene)
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene)
		{
			return pScene->GetSceneId();
		}
		return 0;
	}

	int getEntityCountAtXY(unsigned int hScene, int nX, int nY, int nEntityType)
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene)
		{
			return pScene->GetEntityCountAtXY(nX,nY,nEntityType);
		}
		return 0;
	}

	void SendSecneShortCutMsg(unsigned int hScene, const char* szMsg)
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene)
		{
			return pScene->SendSecneShortCutMsg(szMsg);
		}
	}

	void getSceneNeedLevel( void* ptrScene, int x, int y, int& nLevel, int& nCircle)
	{
		nLevel = 0;
		nCircle = 0;
		if ( !ptrScene )
		{
			return;
		}
		CScene* pScene = (CScene*)ptrScene;
		SCENEAREA* pArea = pScene->GetAreaAttri(x,y);
		if ( !pArea )
		{
			return;
		}
		if (pScene->HasMapAttribute(x,y,aaSceneLevel,pArea))
		{
			AreaAttri& attri = pArea->attri[aaSceneLevel];
			if(attri.pValues)
			{
				nLevel = attri.pValues[0];
				if ( attri.nCount > 1 && attri.pValues[1] )
				{
					nCircle = attri.pValues[1];
				}
			}
		}
	}

	//设置某个坐标点不可移动( true 不可移动， false 可移动)
	void setUnableMovePoint( unsigned int hScene, int x, int y, bool boValue)
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if ( !pScene )
		{
			return;
		}
		if (boValue)
		{
			pScene->SetUnablePoint(x,y,0); //不可移动
		}
		else
		{
			pScene->SetUnablePoint(x,y,1); //可移动
		}

	}

	bool useDefaultCreateMonster(unsigned int fbHandle,bool flag ){
		CFuBen* pFb =  CFuBenManager::m_FuBenMgr->GetDataPtr(fbHandle);
		if (pFb) 
		{
			return pFb->change_create_monster_flag(flag);
		}
		return false;
	}

	bool isFuben(void* pFb)
	{
		return ((CFuBen*) pFb)->IsFb();
	}

	void ExitAllFbActor(void* pFb)
	{
		if (pFb)
		{
			((CFuBen*) pFb)->ExitAllActor();
		}
	}

	int GetNowSceneGuildList( unsigned int nSceneId)
	{
		CFuBen * pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
		if (pFb != NULL)
		{
			CScene * pScene = pFb->GetScene(nSceneId);
			if(pScene)
			{
				return pScene->GetNowSceneGuildList();
			}
		}
		return 0;
	}
	void ResetFubenSceneConfig( int nSceneId)
	{
		CFuBen * pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
		if (pFb != NULL)
		{
			CScene * pScene = pFb->GetScene(nSceneId);
			if(pScene)
			{
				return pScene->ResetSceneConfig();
			}
		}
	}

	int getMonsterMaxHp( unsigned int hScene,int nMonsterId )
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene)
		{
			CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
#ifndef MONSTER_MANAGER_BY_SCENE
			CEntityList& list = pScene->GetMonsterList();
			CLinkedNode<EntityHandle> *pNode;
			CLinkedListIterator<EntityHandle> it(list);
			for (pNode = it.first(); pNode; pNode = it.next())
			{
				CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
#else
			std::map<EntityHandle,CMonster*>::iterator iter = pScene->GetMonsterList().begin();
			for (; iter != pScene->GetMonsterList().end() ; ++iter)
			{
				CEntity* pEntity = pEntityMgr->GetEntity(iter->first);
#endif
				if (pEntity && (pEntity->GetProperty<int>(PROP_ENTITY_ID) == nMonsterId))
				{
					return ((CMonster*)pEntity)->GetMaxHp();
				}		
			}
		}
		return 0;
	}

	int getMonsterHp( unsigned int hScene,int nMonsterId )
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene)
		{
			CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
#ifndef MONSTER_MANAGER_BY_SCENE
			CEntityList& list = pScene->GetMonsterList();
			CLinkedNode<EntityHandle> *pNode;
			CLinkedListIterator<EntityHandle> it(list);
			for (pNode = it.first(); pNode; pNode = it.next())
			{
				CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
#else
			std::map<EntityHandle,CMonster*>::iterator iter = pScene->GetMonsterList().begin();
			for (; iter != pScene->GetMonsterList().end() ; ++iter)
			{
				CEntity* pEntity = pEntityMgr->GetEntity(iter->first);
#endif
				if (pEntity && (pEntity->GetProperty<int>(PROP_ENTITY_ID) == nMonsterId))
				{
					return ((CMonster*)pEntity)->GetHp();
				}		
			}
		}
		return 0;
	}
};
