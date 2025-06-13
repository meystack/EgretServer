#include "StdAfx.h"

#define AI_MONSTER_SKILL_UPDATE

 //怪物的技能的列表
void CAnimalAI::initialize(PAICONFIG pAiconfig)
{
	m_pAiConfig = pAiconfig;
	CSkillSubSystem & skillSystem = m_pSelf->GetSkillSystem();
	unsigned int nID = m_pSelf->GetProperty<unsigned int>(PROP_ENTITY_ID);
	PMONSTERCONFIG pMonsterConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nID);
	m_wMinSkillDis = 1;//这个初始化怪物技能里最大的攻击距离
	if(pMonsterConfig)
	{
		if(!m_pSelf->IsHero())
		{
			for(INT_PTR i=0; i < pMonsterConfig->skills.count ; ++i )
			{
				PMONSTERSKILL  pConfig = & pMonsterConfig->skills[i];
				if( skillSystem.LearnSkill(pConfig->wSkillID,pConfig->bSkillLevel) == tpNoError)
				{

					CSkillSubSystem::PSKILLDATA pSkill= skillSystem.GetSkillInfoPtr(pConfig->wSkillID);
					if(pSkill )
					{
						pSkill->bEvent = pConfig->bEvent ; //将这个保存起来，用于一些特殊场合的事件触发技能
					}

					if(!pSkill->bIsPassive && !pSkill->bIsClose && pConfig->bEvent == mSkillEventCommon)
					{
						INT_PTR nDis =skillSystem.GetSpellTargetDistance(pConfig->wSkillID,pConfig->bSkillLevel);
						if(nDis  >0)
						{
							if(nDis > m_wMinSkillDis && m_pSelf->GetType() != enPet)
							{
								m_wMinSkillDis =(WORD) nDis;
							}
						}
					}

				}
			}
		}
	}

	RefreshSkillDis();
	EnterInitAI();
}

void CAnimalAI::EnterInitAI()
{
	// 清空移动栈
	m_pSelf->GetMoveSystem()->ClearMotionStack();
	if(m_pSelf->IsMonster())
	{
		if(m_pAiConfig->patrolRadius > 0) // 怪物巡逻
		{
			m_pSelf->GetMoveSystem()->MovePatrol(m_pAiConfig->patrolRadius,m_pAiConfig->nStopMinTime,m_pAiConfig->nStopMaxTime);
		}
	}
	
	SetReSelTargetRate(m_pAiConfig->hitSetTargetRate);

	SetNextCheckEnemyTimer();
}

void  CAnimalAI::UpdateAI(TICKCOUNT nCurrentTick)
{
    DECLARE_TIME_PROF("CAnimalAI::UpdateAI");

	//如果在战斗状态
	if(m_pSelf->HasState(esStateDeath)) return;

	// 回归状态也不用更新ai
	if (m_pSelf->HasState(esStateReturnHome)) return;	

	// 战斗状态
	if(m_pSelf->HasState(esStateBattle) )
	{	
		//如果目标不存在的话,这里要进行目标的选择
		CEntity * pEntity = GetEntityFromHandle(m_pSelf->GetTarget()); //获得目标的指针

		//如果目标已经删除或者已经不在一个场景
		if (NULL== pEntity || !pEntity->IsInited() || /*!m_pSelf->CanSee(pEntity) ||*/ pEntity->GetPosInfo().nSceneId != m_pSelf->GetPosInfo().nSceneId) 
		{
			if(m_btAiType == atAiTypeAggressor)
			{
				// 这里主动怪目标死亡之后要切换目标，否则目标死亡之后一直锁定老的死亡目标，不会攻击新玩家
				if (!CheckEnemy())	//找不到目标则回归
					EnterEvadeMode();
			}
			else
			{			
				EnterEvadeMode();	
			}
			return;
		}

		INT_PTR nEntityType = m_pSelf->GetType();	
		CMovementSystem::MotionType nMotionType = m_pSelf->GetMoveSystem()->GetMotionType();
		//如果当前正在追逐就返回
		if (nMotionType == CMovementSystem::mtMotionTypeChase || nMotionType == CMovementSystem::mtMotionTypeIdle)
		{
			if(nEntityType == enMonster)  // 对于普通怪，检测怪物离出生点位置；对于跟随怪，需要检测和跟随者的距离！
			{
				INT_PTR nPosx,nPosY;	
				int nCurrentPosX,  nCurrentPosY;
				m_pSelf->GetPosition(nCurrentPosX,nCurrentPosY);
				// if (m_bInFollowingStats)
				// {
				// 	// 检测玩家和归属者的距离
				// 	CMonster *pMonster = (CMonster *)m_pSelf;
				// 	CActor *pOwner = (CActor *)GetEntityFromHandle(pMonster->GetOwner());
				// 	if (pOwner)
				// 	{
				// 		int nOwnerX, nOwnerY;
				// 		pOwner->GetPosition(nOwnerX, nOwnerY);
				// 		INT_PTR nSuare = (nOwnerX - nCurrentPosX) * (nOwnerX - nCurrentPosX) + (nOwnerY - nCurrentPosY) *(nOwnerY - nCurrentPosY);
				// 		if (nSuare > 400)  // 超过20格
				// 		{							
				// 			EnterEvadeMode();
				// 			return;
				// 		}
				// 	}
				// }
				// else
				{
					if (!m_pSelf->GetAttriFlag().noReturnHome)
					{
						if (nEntityType != enHero && m_pAiConfig->leftHomeMaxDistanceSquare > 0)
						{
							((CMonster*)m_pSelf)->GetBornPoint(nPosx,nPosY);					
							INT_PTR nSuare = (nPosx - nCurrentPosX) * (nPosx - nCurrentPosX) + 
								(nPosY -nCurrentPosY ) *(nPosY -nCurrentPosY );
							if(nSuare >=( m_pAiConfig->leftHomeMaxDistanceSquare  ))
							{
								EnterEvadeMode();
								return;
							}
						}
					}
					
				}				
			}
			//return;
			if (nEntityType == enPet && nMotionType == CMovementSystem::mtMotionTypeChase)
			{
				if(((CPetAi*)this)->IsInAvoidMap(pEntity->GetHandle())) 
				{
					EnterEvadeMode();
				}
			}
		}
		
		int nSelfPosX,nSelfPosY,nTargetPosX,nTargetPosY, nDisX,nDisY;
		m_pSelf->GetPosition(nSelfPosX,nSelfPosY);
		pEntity->GetPosition(nTargetPosX,nTargetPosY);
		nDisX = nSelfPosX - nTargetPosX;
		nDisY = nSelfPosY - nTargetPosY;
		//目标和自己的距离
		INT_PTR nSuareDis = nDisX * nDisX + nDisY * nDisY;
		
		//如果距离比最大的追逐距离要大了，就回归
		if(nSuareDis >= ( m_pAiConfig->pursuitDistance * m_pAiConfig->pursuitDistance))
		{
			EnterEvadeMode();
			return;
		}
		//这里要增加回归的判断
		if( pEntity->IsDeath() || m_pSelf->CanSee(pEntity) ==false)
		{
			if(m_btAiType == atAiTypeAggressor)
			{
				// 这里主动怪目标死亡之后要切换目标，否则目标死亡之后一直锁定老的死亡目标，不会攻击新玩家
				//CheckEnemy();
				//if (GetEntityFromHandle(m_pSelf->GetTarget())) // 如果没有目标，回归
				if (!CheckEnemy())	//找不到目标则回归
					EnterEvadeMode();
			}
			else
			{			
				EnterEvadeMode();	
			}
			
			return;
		}
		
		if (m_1sCheckEnemyInCombat.Check(nCurrentTick))
		{
			SetNextCheckEnemyTimer();
			//OutputMsg(rmNormal, _T("检测: (CurTime=%u)"), nCurrentTick);
			
			if (CheckEnemy())	// 切换目标
			{
				// if (CEntity * pNew = GetEntityFromHandle(m_pSelf->GetTarget()))
				// 	OutputMsg(rmNormal, _T("切换目标: (name:%s)(CurTime=%u)"), pNew->GetEntityName(),nCurrentTick);
				// else OutputMsg(rmNormal, _T("切换目标: (CurTime=%u)"), nCurrentTick);
				return;
			}
		}
		int nDisTance = __max( abs(nDisX),abs(nDisY) ) ;
		if(  nDisTance  <= m_wMinSkillDis ) //如果到达了攻击范围
		{
			/*DECLARE_TIME_PROF("UpdateAI::LaunchSkill");*/
			// if (!(CAnimal*)m_pSelf->CanEnterState(esStateSing)) //当前状态能否释放技能
			// 	return ;
			m_pSelf->GetMoveSystem()->ClearMotionStack();
			if(((CAnimal*)m_pSelf)->CheckCommonOpTick(nCurrentTick) && m_skillTimer.CheckAndSet(nCurrentTick, true) ) //如果能够使用技能了
			{
				m_pSelf->Face2Target(pEntity);
				CSkillSubSystem & skillSystem =  m_pSelf->GetSkillSystem();
				const CVector<CSkillSubSystem::SKILLDATA> & skills= skillSystem.GetSkills();
				bool hasAttack= false; 
				//英雄技能的处理
				/*if(m_pSelf->GetType() == enHero)
				{
				    INT_PTR nCount = skills.count();
					if(m_nSkillIdx  >= nCount)
					{
						m_nSkillIdx = 0;
					}
					if(nCount > 0)
					{
						CSkillSubSystem::SKILLDATA *pSkill = &skills[m_nSkillIdx];
						if(pSkill->bEvent == mSkillEventCommon)
						{
							if(!pSkill->bIsClose && !pSkill->bIsPassive)
							{
								if(skillSystem.LaunchSkill(pSkill->nSkillID,nTargetPosX,nTargetPosY) == tpNoError)
								{
									hasAttack =true; //已经攻击了
									((CAnimal*)m_pSelf)->SetCommonOpNextTime(m_pSelf->GetAttackInterval(),false,false);
									m_nSkillIdx++;
									return;
								}
							}else
							{
								m_nSkillIdx++;
							}
						}else
						{
							m_nSkillIdx++;
						}
						//如果正常技能索引不能释放，则选择能释放的技能释放
						for(INT_PTR i=0; i< skills.count();i++)
						{
							CSkillSubSystem::SKILLDATA *pSkill = &skills[i];
							if(pSkill->bIsClose)
							{
								continue;
							}
							if(pSkill->bIsPassive)
							{
								continue;
							}
							if(skillSystem.LaunchSkill(pSkill->nSkillID,nTargetPosX,nTargetPosY) == tpNoError)
							{
								((CAnimal*)m_pSelf)->SetCommonOpNextTime(m_pSelf->GetAttackInterval(),false,false);
								hasAttack =true; 
								break;
							}
						}
					}
				}
				else*/
				{
					for(INT_PTR i=0; i< skills.count();i++)
					{
						CSkillSubSystem::SKILLDATA *pSkill = &skills[i];
						if(pSkill->bEvent == mSkillEventCommon)
						{
							//如果关闭的了
							if(pSkill->bIsClose)
							{
								continue;
							}

							//被动不使用
							if(pSkill->bIsPassive)
							{
								continue;
							}

							if( !skillSystem.LaunchSkill(pSkill->nSkillID,nTargetPosX,nTargetPosY) )
							{
								//OutputMsg(rmNormal, _T("Monster LaunchSkill: %d, curTime:%u"), skills[i].nSkillID, GetGlobalLogicEngine()->getMiniDateTime());
								//攻击速度取决于攻击速度
								((CAnimal*)m_pSelf)->SetCommonOpNextTime(m_pSelf->GetAttackInterval(),false,false);
								hasAttack =true; //已经攻击了
								continue;
							}
						}
					}
				}

				if( ! hasAttack )  //没有释放技能
				{
					//如果不能进行肉搏的话,就不执行
					if( ! m_pSelf->GetAttriFlag().DenyUseNearAttack)
					{
						if(nDisTance >1)
						{
							m_pSelf->GetMoveSystem()->ClearMotionStack();
							m_pSelf->GetMoveSystem()->MoveChase((CAnimal*)pEntity,1,m_pAiConfig->pursuitDistance); 
						}
						else
						{
							INT_PTR nInterval =m_pSelf->GetAttackInterval() ;
							if( !skillSystem.NearAttack(pEntity->GetHandle(),0,0) && m_pSelf->GetType()!= enHero)
							{
								nInterval += m_pSelf->GetProperty< int>(PROP_CREATURE_ATTACK_SPEED); //添加攻击间隔
							}
							((CAnimal*)m_pSelf)->SetCommonOpNextTime(nInterval,false,false);
							//OutputMsg(rmNormal, _T("攻击: 【%s】 CurTime=%u "), m_pSelf->GetEntityName(), nCurrentTick);
						}
						
					}
					
				}
				
			}	
		}
		else 
		{
			//m_pSelf->GetMoveSystem()->ClearMotionStack();
			m_pSelf->GetMoveSystem()->MoveChase((CAnimal*)pEntity,m_wMinSkillDis,m_pAiConfig->pursuitDistance); 
		}
	}
	// 非战斗状态
	else
	{
		DECLARE_TIME_PROF("UpdateAI::CheckEnemy");
		if( m_1sCheckEnemyInCombat.CheckAndSet(nCurrentTick, true) )
		{
			if(m_pSelf->GetAttriFlag().IdleUseSkill)
			{
				CSkillSubSystem & skillSystem =  m_pSelf->GetSkillSystem();
				const CVector<CSkillSubSystem::SKILLDATA> & skills= skillSystem.GetSkills();
				bool hasAttack= false; 

				for(INT_PTR i=0; i< skills.count();i++)
				{
					CSkillSubSystem::SKILLDATA *pSkill = &skills[i];
					if(pSkill->bEvent == mSkillEventIdle)
					{
						//如果关闭的了
						if(pSkill->bIsClose)
						{
							continue;
						}

						//被动不使用
						if(pSkill->bIsPassive)
						{
							continue;
						}

						if( !skillSystem.LaunchSkill(pSkill->nSkillID,0,0))
						{
							//OutputMsg(rmNormal, _T("Monster LaunchSkill: %d, curTime:%u"), skills[i].nSkillID, GetGlobalLogicEngine()->getMiniDateTime());
							//攻击速度取决于攻击速度
							((CAnimal*)m_pSelf)->SetCommonOpNextTime(m_pSelf->GetAttackInterval(),false,false);
	                        continue;
						}
					}
				}
			}
			CheckEnemy();

		}
		
	}	
}


void CAnimalAI::HandleSkillCDWhenCombatStart()
{
	// 处理进入战斗立即CD技能
	if(!m_pSelf->IsMonster() && !m_pSelf->IsHero() ) return ; 
	if (!m_pSelf->HasState(esStateBattle))
	{
	    m_nSkillIdx  = 0;
		TICKCOUNT nCurrTime = GetGlobalLogicEngine()->getTickCount();
		if (m_pAiConfig)
		{
			CSkillSubSystem &skillSystem =  m_pSelf->GetSkillSystem();
			const CVector<CSkillSubSystem::SKILLDATA> &skills= skillSystem.GetSkills();

			unsigned int nID = m_pSelf->GetProperty<unsigned int>(PROP_ENTITY_ID);
			PMONSTERCONFIG pMonsterConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nID);
			if (!pMonsterConfig) return;
			
			
			for (INT_PTR i = 0; i < pMonsterConfig->skills.count; i++)
			{
				MONSTERSKILL *pAISkillData = pMonsterConfig->skills.GetDataPtr(i);
				
				//只有普通的技能才这样处理，像死亡触发的技能不能这样使用
				if (pAISkillData->bAutoCDWhenEnterCombat && pAISkillData->bEvent == mSkillEventCommon)
				{
					// 技能进入CD					
					for (INT_PTR j = 0; j < skills.count(); j++)
					{
						CSkillSubSystem::SKILLDATA &skillData = skills[j];
						//英雄出战技能进入CD
						/*if(m_pSelf->IsHero())
						{
						    if (skillData.nSkillID == pAISkillData->wSkillID)
							{
								const SKILLONELEVEL *pLevel = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillLevelData(skillData.nSkillID, skillData.nLevel);
								if (pLevel)
								{
									skillData.tick = nCurrTime +  pLevel->nCooldownTimes;
								}
							}
						}else*/ if (skillData.nSkillID == pAISkillData->wSkillID && skillData.nLevel == pAISkillData->bSkillLevel)
						{
							const SKILLONELEVEL *pLevel = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillLevelData(skillData.nSkillID, skillData.nLevel);
							if (pLevel)
							{
								skillData.tick = nCurrTime + pLevel->nCooldownTimes;
							}
						}
					}
				}
			}
		}
	}
}

void CAnimalAI::RefreshSkillDis()
{
	if(m_pSelf ==NULL )
	{
		OutputMsg(rmTip,"RefreshSkillDis m_pSelf has not inited");
		return;
	}

	if (m_pSelf->GetType() == enPet) return;
	

	CSkillSubSystem & skillSystem = m_pSelf->GetSkillSystem();

	//这个初始化怪物技能里最大的攻击距离
	m_wMinSkillDis = 1;

	const CVector<CSkillSubSystem::SKILLDATA> & skills= skillSystem.GetSkills();
	bool hasAttack= false; 

	for(INT_PTR i=0; i< skills.count();i++)
	{
		CSkillSubSystem::SKILLDATA *pSkill = &skills[i];

		if (pSkill->bIsClose) continue; //关闭的技能不栓书

		if (pSkill->bIsPassive) continue;	//被动不处理

		INT_PTR nDis =skillSystem.GetSpellTargetDistance(pSkill->nSkillID,pSkill->nLevel);
		if(nDis > 0)
		{
			if(nDis > m_wMinSkillDis)
			{
				m_wMinSkillDis =(WORD) nDis;
			}
		}
	}
}

void CAnimalAI::AttackStart(CAnimal * pTarget)
{
	if(pTarget ==NULL || pTarget->IsInited() ==false)return; //开始攻击
	bool bFoundNewTarget = true;
	
	//if (!m_pSelf->HasState(esStateBattle))
	//	bFoundNewTarget = true;

#ifdef AI_MONSTER_SKILL_UPDATE
	HandleSkillCDWhenCombatStart();	
#endif
  
	CMovementSystem::MotionType nCurrType = m_pSelf->GetMoveSystem()->GetMotionType();
	if (nCurrType == CMovementSystem::mtMotionTypeChase)
	{			
		m_pSelf->GetMoveSystem()->PopCurrentMotionState();
	}

	const EntityHandle& targetHandle = m_pSelf->GetTarget(); //目标的handle

	CEntity *pCurTarget =GetEntityFromHandle( targetHandle );
	if( pCurTarget && !pCurTarget->IsDeath() )
	{
		if( targetHandle == pTarget->GetHandle() )
		{
			bFoundNewTarget = false;
		}
	}

	m_pSelf->AddState(esStateBattle); //设置战斗状态
	if (m_pSelf->GetType() == enPet && ((CPet*)m_pSelf)->GetLiveTime() == 0)
	{
		if (m_pSelf->GetMoveSystem()->GetMotionType() == CMovementSystem::mtMotionTypeFollow)
		{
			m_pSelf->GetMoveSystem()->PopCurrentMotionState();
		}
	}else if(m_pSelf->GetType() == enHero)
	{
		if (m_pSelf->GetMoveSystem()->GetMotionType() == CMovementSystem::mtMotionTypeFollow)
		{
			m_pSelf->GetMoveSystem()->PopCurrentMotionState();
		}
	}

	if(bFoundNewTarget)
	{		
		// 逃跑 | 回归的时候不切换目标
		
		if (nCurrType == CMovementSystem::mtMotionTypeFleeing || nCurrType == CMovementSystem::mtMotionTypeReturnHome)
		{			
			return;
		}
		m_pSelf->SetTarget( pTarget->GetHandle() ); //选定这个玩家为目标
		
		INT_PTR nDir = m_pSelf->GetTargetDir(pTarget);
		m_pSelf->SetDir(nDir); //转向朝向玩家

		// 如果原来在追逐，停止掉。否则可能会跑到新目标后，又跑向老的目标 （如果当前是乱走，那么此流程会有问题）

		if (m_pSelf->GetType() != enPet)
		{
			m_pSelf->GetMoveSystem()->ClearMotionStack();
			m_pSelf->GetMoveSystem()->MoveChase(pTarget, m_wMinSkillDis, m_pAiConfig->pursuitDistance); 	
		}
	}
	
	//if(!targetHandle.IsNull())
	//{
		INT_PTR nTargetType = pTarget->GetType();
		if( nTargetType== enActor) //如果攻击的目标是宠物，将开始攻击
		{
			((CActor*)pTarget)->GetPetSystem().OnAttacked(m_pSelf);
			((CActor*)pTarget)->GetHeroSystem().OnAttacked(m_pSelf);
		}
		else if(nTargetType == enPet)
		{
			((CPet*)pTarget)->GetAI()->AttackedBy(m_pSelf);
		}
		else if(nTargetType == enHero)
		{
			((CHero*)pTarget)->GetAI()->AttackedBy(m_pSelf);
		}
	//}
	
		
	//发现目标的说话
	// if (bFoundNewTarget && m_pSelf->IsMonster())
	// {
	// 	CMonster *pMonsterPtr = ((CMonster *)m_pSelf);
	// 	pMonsterPtr->MonsterSay(mstFoundTarget);
	// 	const PMONSTERCONFIG pMonster= GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(m_pSelf->GetId());
	// 	if( pMonster !=NULL )
	// 	{
	// 		int nId = pMonster->bTargetMusicId;
	// 		BYTE nRate = pMonster->bTargetMusicRate;
	// 		if(nId >0  && nRate >0)
	// 		{
	// 			if(nRate >= wrand(101))
	// 			{
	// 				char buff[512];
	// 				CDataPacket pack(buff,sizeof(buff));
	// 				pack << (BYTE) enDefaultEntitySystemID;
	// 				pack << (BYTE) sFoundTarget;
	// 				pack << (Uint64) m_pSelf->GetHandle() <<nId ;
	// 				pMonsterPtr->GetObserverSystem()->BroadCast(pack.getMemoryPtr(),pack.getPosition(),false);
	// 			}
	// 		}
	// 	}
	// }
}

 void CAnimalAI::AttackStop(CAnimal * pTarget)
{
	EnterEvadeMode();
}

void CAnimalAI::SetNextCheckEnemyTimer()
{
	if (m_pAiConfig->nCheckEnemyIntervalInCombat <= 0)
		m_1sCheckEnemyInCombat.SetNextHitTimeFromNow(1000000000);
	else
		m_1sCheckEnemyInCombat.SetNextHitTimeFromNow(m_pAiConfig->nCheckEnemyIntervalInCombat);
}

bool  CAnimalAI::NeedOntimeCheck(TICKCOUNT nCurrentTick)
{
	return true;
}

//开始回归
void CAnimalAI::EnterEvadeMode()
{
	if( m_pSelf->HasState(esStateDeath) ) return; //死了不回归
	if( m_pSelf->HasState(esStateReturnHome) ) return; //已经在回归状态了
	
	m_pSelf->RemoveState(esStateBattle); //删除战斗模式		
	CMovementSystem*  pMyMoveSystem(m_pSelf->GetMoveSystem());
	// if (m_bInFollowingStats)	// 跟随状态，不回归
	// {
	// 	if( pMyMoveSystem->GetMotionType() ==  CMovementSystem::mtMotionTypeChase)
	// 	{
	// 		pMyMoveSystem->PopCurrentMotionState(); //删除当前的追逐
	// 	}
	// 	if( pMyMoveSystem->GetMotionType() ==  CMovementSystem::mtMotionTypeIdle)
	// 	{
	// 		pMyMoveSystem->PopCurrentMotionState(); //删除当前的追逐
	// 	}
	// 	return;
	// }

	bool canMove =m_pSelf->CanEnterState(esStateMove);
	bool boNoReturnHome = m_pSelf->GetAttriFlag().noReturnHome;
	if(canMove && !boNoReturnHome)
	{
		m_pSelf->AddState(esStateReturnHome);
	}
	
	if(m_pSelf->GetType() == enMonster )
	{

		if (boNoReturnHome)
		{
			m_pSelf->GetMoveSystem()->ClearMotionStack();
			if(m_pAiConfig->patrolRadius > 0) // 怪物巡逻
			{
				m_pSelf->GetMoveSystem()->MoveNoReturnHome(m_pAiConfig->patrolRadius,m_pAiConfig->nStopMinTime,m_pAiConfig->nStopMaxTime);
			}
		}
		else
		{
			if( pMyMoveSystem->GetMotionType() ==  CMovementSystem::mtMotionTypeChase)
			{
				pMyMoveSystem->PopCurrentMotionState(); //删除当前的追逐

			}
			if(canMove && pMyMoveSystem->GetMotionType() ==  CMovementSystem::mtMotionTypeIdle)
			{
				pMyMoveSystem->PopCurrentMotionState(); //删除当前的追逐
			}
		}


		//怪物在回归的时候需要会一下血
		if(m_pAiConfig->returnHomeRenewRate >0 )
		{
			unsigned int nHpValue = m_pSelf->GetProperty<unsigned int>(PROP_CREATURE_HP);
			unsigned int nMpValue = m_pSelf->GetProperty<unsigned int>(PROP_CREATURE_MP);
			nHpValue = __max(nHpValue, m_pSelf->GetProperty<unsigned int>(PROP_CREATURE_MAXHP)* m_pAiConfig->returnHomeRenewRate /100 );
			nMpValue = __max(nMpValue, m_pSelf->GetProperty<unsigned int>(PROP_CREATURE_MAXMP)* m_pAiConfig->returnHomeRenewRate /100 );
			m_pSelf->SetProperty<unsigned int>(PROP_CREATURE_HP,nHpValue);
			m_pSelf->SetProperty<unsigned int>(PROP_CREATURE_MP,nMpValue);
		}
		if(canMove)
		{
			if (m_pSelf->GetAttriFlag().AttackKiller && m_pSelf->GetAttriFlag().DenyMove)
			{
				PMONSTERCONFIG pConfig  = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(m_pSelf->GetProperty<unsigned int>(PROP_ENTITY_ID));
				if (pConfig)
				{
					INT_PTR nDir = (INT_PTR)(pConfig->btDir != 255? pConfig->btDir : -1);
					INT_PTR nBornPosX, nBornPosY;
					int nPosX,nPosY;
					m_pSelf->GetPosition(nPosX,nPosY);
					((CMonster*)m_pSelf)->GetBornPoint(nBornPosX,nBornPosY);

					if (nPosX != nBornPosX && nBornPosY != nPosY)
					{
						CFuBen* hFb = m_pSelf->GetFuBen();
						GetGlobalLogicEngine()->GetFuBenMgr()->EnterFuBen(m_pSelf,hFb,m_pSelf->GetSceneID(),nBornPosX,nBornPosY);
					}
					m_pSelf->SetDir(nDir);
					m_pSelf->RemoveState(esStateReturnHome);
				}

			}
			else
			{
				if (!boNoReturnHome)
				{
					pMyMoveSystem->MoveTargetedHome(); //是怪物的话要归位
				}
				
			}
		}
		
		m_pSelf->SetTarget(NULL);
		m_pSelf->SetAttackerHandler(0);
		((CMonster *)m_pSelf)->SetVestEntity(EntityHandle());	// 怪物回归清归属
		if (((CMonster *)m_pSelf)->GetAttriFlag().boShowVestEntityName)
		{
			((CMonster *)m_pSelf)->SetVestEntityName("");

		}
	}
}

void CAnimalAI::AttackedBy(CAnimal * pAttacker, bool boInherit)
{
	if (m_pSelf->IsMonster())
		((CMonster *)m_pSelf)->MonsterSay(mstAttacked);
	if (m_pSelf->GetType() == enPet )// ||  m_pSelf->GetType() == enHero)
	{
		if (m_pSelf->GetAttriFlag().PetNoAttact)
		{
			return;
		}
		if ( !pAttacker || pAttacker->IsDeath() )
		{
			return;
		}
		INT_PTR  nDisSquare = m_pSelf->GetEntityDistanceSquare( pAttacker ); //获取距离的平方
		if ( nDisSquare >= (m_pAiConfig->pursuitDistance * m_pAiConfig->pursuitDistance) )
		{
			if (m_pSelf->GetMoveSystem()->GetMotionType() == CMovementSystem::mtMotionTypeChase)
			{
				m_pSelf->GetMoveSystem()->PopCurrentMotionState();
			}
			CActor* pActor = NULL;
			//if(m_pSelf->GetType() == enPet)
			//{
			    pActor =  ((CPet*)m_pSelf)->GetMaster();
			//}else
			//{
			//    pActor =  ((CHero*)m_pSelf)->GetMaster();
			//}
			if ( pActor )
			{
				if (m_pSelf->GetMoveSystem()->GetMotionType() != CMovementSystem::mtMotionTypeFollow)
				{
					m_pSelf->GetMoveSystem()->MoveFollow(pActor);
				}
			}
			return;
		}
	}

	//怪物在回归状态攻击不还手
	if(m_pSelf->HasState(esStateBattle)) //已经在战斗中就不处理
	{
		if (NeedReSelTarget())
			ChangeTarget(pAttacker);
		return;
	}
	
	if(! m_pSelf->HasState(esStateReturnHome))
	{
		AttackStart(pAttacker); //被攻击需要
	}

	if(m_pSelf->GetAttriFlag().BeAttackNeedUseSkill)		//被攻击需要释放技能
	{
		CSkillSubSystem & skillSystem =  m_pSelf->GetSkillSystem();
		const CVector<CSkillSubSystem::SKILLDATA> & skills= skillSystem.GetSkills();

		//这里加这个是为了一个技能
		for(INT_PTR i=0; i< skills.count();i++)
		{
			if(skills[i].bEvent == mSkilleventBeAttacked)
			{
				skillSystem.LaunchSkill(skills[i].nSkillID);
			}
		}
	}
}

bool CAnimalAI::NeedReSelTarget()
{
	if (m_wReSelTargetRate == 0)
		return false;

	unsigned long rn = wrand(100);
	if (rn <= m_wReSelTargetRate)
		return true;

	return false;
}

void CAnimalAI::ChangeTarget(CAnimal *pTarget)
{
	if (!pTarget)  return;
	AttackStart(pTarget);
}

bool CAnimalAI::CheckEnemy()
{	
	if (m_pSelf->IsMonster())
		((CMonster *)m_pSelf)->MonsterSay(mstIdle);

	return false;
}

void CAnimalAI::KillBy(CAnimal  *pAttacker) 
{
	CSkillSubSystem & skillSystem =  m_pSelf->GetSkillSystem();
	const CVector<CSkillSubSystem::SKILLDATA> & skills= skillSystem.GetSkills();

	//这里加这个是为了一个技能
	if(pAttacker)
	{
		EntityHandle handle =  pAttacker->GetHandle() ;
		CEntity *pTarget = GetEntityFromHandle (handle);
		if ( pTarget !=  pAttacker)
		{
			m_pSelf->SetTarget(handle );
		}
	}
	// for(INT_PTR i=0; i< skills.count();i++)
	// {
	// 	if(skills[i].bEvent == mSkillEventDeath)
	// 	{
	// 		skillSystem.LaunchSkill(skills[i].nSkillID,0,0,true,skills[i].nLevel,true);
	// 	}
	// }
}

void CAnimalAI::EntityDestroy() 
{
}
