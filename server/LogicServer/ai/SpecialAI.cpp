
#include "StdAfx.h"

bool CSpecialAI::CheckEnemy()
{
	if(!m_pSelf) return false;
	INT_PTR nType = m_pSelf->GetType();
	CScene * pScene = m_pSelf->GetScene();
	if(pScene ==NULL) return false;

	int nSelfX, nSelfY;
	m_pSelf->GetPosition(nSelfX, nSelfY);

	EntityHandle enemyHandle;
	CEntity *pAttackTarget =NULL; 

	UINT64 nPriority = 0;	
	if(pScene->GetPlayCount() <=0 ) return false; //场景里没有玩家的话就不执行了

	CVector<EntityHandleTag> &entityList = m_pSelf->GetObserverSystem()->GetVisibleList();

	int nDis = __max(m_pAiConfig->watchDistance, 0);

	//主动怪可攻击怪物 
	if(m_pSelf->IsMonster())
	{
		EntityHandle selfHd = m_pSelf->GetHandle();
		//需要检测是否攻击怪物的，也只有部分
		if( m_checkMonsterTimer.CheckAndSet( GetLogicCurrTickCount())  && nDis >0)
		{
			DECLARE_TIME_PROF("CSpecialAI::CheckEnemy");
			CObserverSystem::s_seqVisibleList->clear();
			CFuBenManager* fm = GetGlobalLogicEngine()->GetFuBenMgr();	

			fm->GetVisibleList(m_pSelf, *CObserverSystem::s_seqVisibleList, -nDis, nDis, -nDis, nDis);
			INT_PTR nCount = CObserverSystem::s_seqVisibleList->count();
			if(nCount >1)
			{
				int nMinAnimalDist = 999;
				CAnimal* pTargetAnimal = NULL;
				for(INT_PTR i=0; i< nCount ; i++)
				{
					EntityHandle newHandle = (*CObserverSystem::s_seqVisibleList)[i];
					if(newHandle.GetType() == enMonster && newHandle != selfHd)
					{
						CEntity *pEntity = GetEntityFromHandle(newHandle);
						if (pEntity)
						{
							if (!m_pSelf->CanSee(pEntity))
								continue;
							if (pEntity->IsDeath())
							{
								continue;
							}
							CAnimal *pAnimal = (CAnimal*) pEntity;
							//不能攻击的不算
							if(! m_pSelf->CanAttack( pAnimal) ) continue; 

							INT_PTR nCurrType = m_pSelf->GetMoveSystem()->GetMotionType();
							if (nCurrType == CMovementSystem::mtMotionTypeChase)
							{			
								m_pSelf->GetMoveSystem()->PopCurrentMotionState();
							}
							int nSelfPosX,nSelfPosY,nTargetPosX,nTargetPosY, nDisX,nDisY;
							m_pSelf->GetPosition(nSelfPosX,nSelfPosY);
							pEntity->GetPosition(nTargetPosX,nTargetPosY);
							if (m_pSelf->GetAttriFlag().DenyMove || m_pSelf->GetAttriFlag().DenyBeMove )
							{
								nDisX = nSelfPosX - nTargetPosX;
								nDisY = nSelfPosY - nTargetPosY;
								int nDisTance = __max( abs(nDisX),abs(nDisY) ) ;
								if (nDisTance > m_wMinSkillDis)
									continue;
							}
							int nDist = abs(nSelfPosX - nTargetPosX) + abs(nSelfPosY - nTargetPosY);
							if ( nDist < nMinAnimalDist )
							{
								pTargetAnimal = pAnimal;
								nMinAnimalDist = nDist;
							}
						}
					}
				}
				if (pTargetAnimal)
				{
					AttackStart(pTargetAnimal);
					return true;
				}

			}

		}
	}

	return false;
}

void CSpecialAI::UpdateAI(TICKCOUNT nCurrentTick)
{
	if(m_pSelf->HasState(esStateDeath)) return;
	if (!m_pSelf->GetAttriFlag().DenyMove && !m_pSelf->GetAttriFlag().DenyBeMove)
	{
		Inherid::UpdateAI(nCurrentTick);
		return;
	}
	if (CheckEnemy())
	{
		CEntity * pEntity = GetEntityFromHandle(m_pSelf->GetTarget());
		if (!pEntity)
		{
			return;
		}
		DECLARE_TIME_PROF("CSpecialAI::UpdateAI");
		int nTargetPosX = 0;
		int nTargetPosY = 0;
		pEntity->GetPosition(nTargetPosX,nTargetPosY);
		m_pSelf->Face2Target(pEntity);
		CSkillSubSystem & skillSystem =  m_pSelf->GetSkillSystem();
		const CVector<CSkillSubSystem::SKILLDATA> & skills= skillSystem.GetSkills();
		bool hasAttack= false; 

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

				if( !skillSystem.LaunchSkill(pSkill->nSkillID,nTargetPosX,nTargetPosY))
				{
					//OutputMsg(rmNormal, _T("Monster LaunchSkill: %d, curTime:%u"), skills[i].nSkillID, GetGlobalLogicEngine()->getMiniDateTime());
					//攻击速度取决于攻击速度
					((CAnimal*)m_pSelf)->SetCommonOpNextTime(m_pSelf->GetAttackInterval(),false,false);
					hasAttack =true; //已经攻击了
					break;
				}
			}

		}
	}
}

