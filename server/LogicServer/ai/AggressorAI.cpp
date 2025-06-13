#include "StdAfx.h"

EntityVector* CAggressorAI::s_pVisibleEntityList = NULL;

bool CAggressorAI::CheckEnemy()
{
	bool result = false;
	Inherid::CheckEnemy();	
	//static CVector<int> sEmptyPriorAttackList;
	INT_PTR nType = m_pSelf->GetType();
	CScene * pScene = m_pSelf->GetScene();
	if(pScene ==NULL) return false;

	int nSelfX, nSelfY;
	int nTargetX, nTargetY;
	m_pSelf->GetPosition(nSelfX, nSelfY);
	
	EntityHandle enemyHandle;
	CEntity *pAttackTarget =NULL; 

	UINT64 nPriority = 0;
	int nTargetDisSquare = 0;
	if(pScene->GetPlayCount() <=0 ) return false; //场景里没有玩家的话就不执行了

	CVector<EntityHandleTag> &entityList = m_pSelf->GetObserverSystem()->GetVisibleList();
	
	INT_PTR nActorCount = entityList.count(); //玩家的数目
	//获取这个场景里所有的宠物

	CEntityList &petList = pScene->GetPetList();

	CEntityList &heroList = pScene->GetHeroList();

	INT_PTR nPetCount  = petList.count(); //宠物的数目
	INT_PTR nHeroCount = heroList.count();

	INT_PTR nPetAndActorCount = nPetCount  + nActorCount ; //宠物+玩家的数目
	INT_PTR nPetActorHeroCount = nPetAndActorCount + nHeroCount;       //英雄 + 宠物 + 玩家的数目

	if(!m_pSelf->GetAttriFlag().CanAttackMonster && nPetActorHeroCount <=0) return false; //视野里没有玩家和宠物，则返回

	int nVisibleList = __max(m_pAiConfig->watchDistance, 0);
	int nDis = nVisibleList;
	nVisibleList *= nVisibleList;
	
	CLinkedNode<EntityHandle> *pNode =NULL;
	CLinkedListIterator<EntityHandle> it(petList);
	CLinkedListIterator<EntityHandle> itHero(heroList);

	bool bDenyInitiativeAttackActor = m_pSelf->GetAttriFlag().DenyInitiativeAttackActor;
	bool bOnlyAttackRedActorFlag = m_pSelf->GetAttriFlag().AttackKiller;
	bool bAttackPetFlag = m_pSelf->GetAttriFlag().AttackPet;

	PMONSTERCONFIG pMonConf = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(m_pSelf->GetId());

	//若可攻击怪物
	if(m_pSelf->GetAttriFlag().CanAttackMonster && m_pSelf->IsMonster())
	{
		EntityHandle selfHd = m_pSelf->GetHandle();

		//需要检测是否攻击怪物的，也只有部分
		if( m_checkMonsterTimer.CheckAndSet( GetLogicCurrTickCount(), true)  && nDis>0 )
		{
			CObserverSystem::s_seqVisibleList->clear();
			CFuBenManager* fm = GetGlobalLogicEngine()->GetFuBenMgr();	
			
			fm->GetVisibleList(m_pSelf, *CObserverSystem::s_seqVisibleList, -nDis, nDis, -nDis, nDis);
			INT_PTR nCount = CObserverSystem::s_seqVisibleList->count();
			if(nCount > 1)
			{
				for(INT_PTR i=0; i< nCount ; i++)
				{
					EntityHandle newHandle = (*CObserverSystem::s_seqVisibleList)[i];
					if(newHandle.GetType() == enMonster && newHandle != selfHd)
					{
						CEntity *pEntity = GetEntityFromHandle(newHandle);
						if (pEntity)
						{
							// 不主动攻击被动怪
							if (((CAnimal*)pEntity)->GetAI()->m_btAiType == atAiTypePassive) continue;
							
							if (!m_pSelf->CanSee(pEntity))
								continue;
							CAnimal *pAnimal = (CAnimal*) pEntity;
							//不能攻击的不算
							if(! m_pSelf->CanAttack( pAnimal) ) continue; 

							AttackStart(pAnimal);
							return true;

						}
					}
				}
			}
		}
	}
	
	//遍历玩家,宠物、英雄
	for (INT_PTR i = 0; i < nPetActorHeroCount ; ++i)
	{
		//先遍历玩家再遍历宠物
		EntityHandle handler ;
		if(i < nActorCount )
		{
			handler = entityList[i].m_handle;
		}
		else if( i >= nActorCount && i < nPetAndActorCount  ) 
		{
			if( i== nActorCount)
			{
				 pNode = it.first();
			}
			if(pNode)
			{
				 handler = pNode->m_Data;
				 pNode = it.next();
			}
		}
		else if( i >= nPetAndActorCount && i < nPetActorHeroCount)
		{
			if( i== nPetAndActorCount )
			{
				pNode = itHero.first();
			}
			if(pNode)
			{
				handler = pNode->m_Data;
				pNode = itHero.next();
			}
		}
		
		//得到一个攻击对象
		if (CEntity *pEntity = GetEntityFromHandle(handler))
		{
			CAnimal *pAnimal = (CAnimal*) pEntity;

			if(pEntity->isAnimal() ==false) 	continue;	//为动物
			if(!m_pSelf->CanSee(pEntity)) 		continue;	//可视this->GetTarget() == pEntity->GetHandle()
			if (pAnimal->IsDeath()) 			continue;	//非死亡

			pEntity->GetPosition(nTargetX, nTargetY);
			int nDisSquare = (nTargetX - nSelfX ) * (nTargetX - nSelfX ) +  (nTargetY - nSelfY ) * (nTargetY - nSelfY );

			if(nDisSquare > nVisibleList) 		continue;   //距离过远的不选择

			//主动怪配置DenyInitiativeAttackActor
			if (bDenyInitiativeAttackActor && (enActor == handler.GetType() || enPet == handler.GetType() || enHero == handler.GetType()))
			{
				//主动怪只主动攻击红名玩家
				if (pEntity->GetType() == enActor)
				{
					if(!m_pSelf->CanAttack( pAnimal)) 
						continue;
						
					if((bOnlyAttackRedActorFlag && ((CActor*)pEntity)->GetNameColorData()<3))
						continue;
				}

				//主动怪不主动攻击宠物
				if (pEntity->GetType() == enPet)
				{
					if (!bAttackPetFlag)
						continue;
					if(!m_pSelf->CanAttack( pAnimal)) 
						continue;
				}

				//主动怪不主动攻击英雄
				if (pEntity->GetType() == enHero)
				{
				    if(!bAttackPetFlag)
					{
						if(!m_pSelf->CanAttack( pAnimal)) 
							continue;	
					}
				}
			}
			else
			{
				if (pEntity->GetType() == enActor)
				{
					if(!m_pSelf->CanAttack( pAnimal)) 
						continue;
				} else
				{
				    if(!bAttackPetFlag && !m_pSelf->CanAttack( pAnimal))
					{
						continue;	
					}
				}
			}
			
			
			UINT64 curPriority = 0;

			//boss 用累计伤害计算仇恨值
			if(m_pSelf->IsMonster() && pMonConf &&
			 (pMonConf->btMonsterType == MONSTERCONFIG::mtBoss || 
			 pMonConf->btMonsterType ==  MONSTERCONFIG::mtToumu || 
			 pMonConf->btMonsterType ==  MONSTERCONFIG::mtWorldBoss))
			{
				curPriority = ((CMonster*)m_pSelf)->GetMonsterHetred(pEntity);
				if ((curPriority > nPriority) || (curPriority == nPriority && nDisSquare < nTargetDisSquare) || nPriority == 0)
				{
					nPriority = curPriority;
					nTargetDisSquare = nDisSquare;
					pAttackTarget = pEntity;
				}
			}
			//其他的，同等仇恨条件下，距离越远的，攻击优先级低
			else
			{
				UINT64 nAttackLevel = pAnimal->GetAttackLevel();	
				++nAttackLevel;
				curPriority = (UINT64) (nAttackLevel << 32 );			
				curPriority -= nDisSquare;
				if (curPriority >= nPriority)
				{
					nPriority = curPriority;
					pAttackTarget = pEntity;
				}
			}

		}		
	} // for

	//怪物能够攻击怪物的话，再取下旁边的列表
	if (pAttackTarget && pAttackTarget->isAnimal())
	{
		AttackStart((CAnimal *)pAttackTarget);
		result = true;
	}

	return result;
}

void CAggressorAI::MoveInSight(CAnimal * pTarget,INT_PTR nPosX,INT_PTR nPosY)
{
	unsigned int nSelfPosX= m_pSelf->GetProperty<unsigned int>(PROP_ENTITY_POSX);
	unsigned int nSelfPosY= m_pSelf->GetProperty<unsigned int>(PROP_ENTITY_POSY);
	INT_PTR nDis= CMovementSystem::GetDisSqare(nSelfPosX,nPosY,nPosX,nPosY);
	if( nDis <= Inherid::m_pAiConfig->watchDistance * Inherid::m_pAiConfig->watchDistance) //进入了可视范围
	{
		if (m_pSelf->HasState( esStateBattle ) ==false)
		{
			AttackStart(pTarget); //开始攻击
		}
	}	
}

bool CAggressorAI::CalcTargetPrior(CEntity *pTarget, const CVector<int> *priorTargetList, UINT64 &priority, int nSelfX, int nSelfY)
{	
	/*DECLARE_FUN_TIME_PROF()*/
	priority = 0;
	if (!m_pSelf->IsMonster()) return false;
	if (!pTarget || ((CAnimal *)pTarget)->HasState(esStateDeath)) return false;
	CMonster *pThisMonster	= (CMonster *)m_pSelf;
	if (!pThisMonster->CanAttack((CAnimal *)pTarget))
		return false;

	bool bIsOwnerTarget = false;
	bool bIsValidTarget = false;
	bool bIsPriorTarget = false;
	INT_PTR targetType = pTarget->GetType();
	int nAttackLevel = 0, nDistSqrt = 0;
	
	bool bIsBabyMonster = false;			// 自己是否是宝宝怪	
	CEntity *pMaster		= GetEntityFromHandle(pThisMonster->GetOwner());
	if (pMaster && pMaster->GetType() == enActor) 
		bIsBabyMonster = true;

	if (bIsBabyMonster)
	{
		CEntity *pMasterTarget = GetEntityFromHandle(pMaster->GetTarget());
		if (pMasterTarget && ((CAnimal *)pMaster)->CanAttack((CAnimal *)pMasterTarget) && pThisMonster->CanAttack((CAnimal *)pMasterTarget))
		{
			bIsOwnerTarget = true;
			priority = ComposePriority(true, false, 0, 0);			// 最高优先级
			return true;
		}
	}
		 
	if (targetType == enMonster)
	{
		CMonster *pTargetMonster = (CMonster *)pTarget;
		CEntity *pTargetOwner = GetEntityFromHandle(pTargetMonster->GetOwner());
		bool bTargetIsBabay = false;
		if (pTargetOwner && pTargetOwner->GetType() == enActor)
			bTargetIsBabay = true;
		
		int monsterId = pTargetMonster->GetId();	
		if (priorTargetList)
		{
			for (INT_PTR i = 0; i < priorTargetList->count(); ++i)
			{
				if (monsterId == (*priorTargetList)[i])
				{
					//bIsValidTarget = true;
					bIsPriorTarget = true;
					break;
				}
			}
		}
	

		if (bIsBabyMonster)		// 自己是宝宝怪
		{
			if (bTargetIsBabay)		// 宝宝怪不主动攻击宝宝怪
			{
				bIsValidTarget = false;
			}
			else					// 目标是非宝宝怪，会主动攻击
			{
				// 是合法目标
				bIsValidTarget = true;
			}
		}
		else		// 自己是非宝宝怪
		{
			if (bTargetIsBabay)
			{
				// 是合法目标
				bIsValidTarget = true;
			}
			else		// 目标不是宝宝怪，只有是优选攻击目标列表中的怪才会被攻击
			{
				if (bIsPriorTarget)
					bIsValidTarget = true;
			}
		}
	}
	else if (targetType == enActor || targetType ==enPet || targetType == enHero)
	{
		if (bIsBabyMonster)
		{
			// 宝宝怪不主动攻击玩家
		}
		else
		{
			// 非宝宝怪， 主动攻击玩家	
			bIsValidTarget = true;
		}		
	}

	bool bIsDectDist = false;
	if (bIsValidTarget)	
	{
		int nX, nY;
		pTarget->GetPosition(nX, nY);
		nAttackLevel = ((CAnimal *)pTarget)->GetAttackLevel();			
		nDistSqrt = (int)CMovementSystem::GetDisSqare(nX, nY, nSelfX, nSelfY);		
		if (nDistSqrt <= Inherid::m_pAiConfig->watchDistance * Inherid::m_pAiConfig->watchDistance)
		{
			bIsDectDist = true;		
			priority = ComposePriority(bIsOwnerTarget, bIsPriorTarget, nAttackLevel, nDistSqrt);
		}
	}
	
	return bIsValidTarget && bIsDectDist;
}
