#include "StdAfx.h"

void CBaseAI::AttackedBy(CAnimal *pAttacker, bool boInherit)
{	
	if (m_pSelf->HasState(esStateBattle))				// 战斗状态通常不切换攻击目标
	{
		if (NeedReSelTarget())
			ChangeTarget(pAttacker);		
	}
	else
	{
		if (!m_pSelf->HasState(esStateReturnHome))		// 非战斗状态下被攻击
		{
			AttackStart(pAttacker);
		}
	}
	
}

void CBaseAI::AttackStart(CAnimal *pTarget)
{
	// 怪物发言（发现目标）、设置怪物进入战斗状态
	if (!pTarget) return;
	bool bFoundNewTarget = false;
	if (!m_pSelf->HasState(esStateBattle) && m_pSelf->IsMonster())		
		((CMonster *)m_pSelf)->MonsterSay(mstFoundTarget);
	m_pSelf->AddState(esStateBattle);

	// 设置怪物的战斗目标
	if (m_pSelf->GetTarget() != pTarget->GetHandle())
	{
		m_pSelf->SetTarget(pTarget->GetHandle());
	}
}

int CBaseAI::GetDistSqrtFromEntity(CEntity *pTarget)
{
	if (!pTarget) return 0;
	int nTargetX, nTargetY;
	pTarget->GetPosition(nTargetX, nTargetY);
	int nSelfX, nSelfY;
	m_pSelf->GetPosition(nSelfX, nSelfY);
	return (int)CMovementSystem::GetDisSqare(nSelfX, nSelfY, nTargetX, nTargetY);
}

void CBaseAI::UpdateAI(TICKCOUNT nCurrentTick)
{	
	if (m_pSelf->HasState(esStateDeath) || m_pSelf->HasState(esStateReturnHome)) return;

	if (m_pSelf->HasState(esStateBattle))
	{
		// 如果目标死亡或者目标隔得太远
		CEntity *pTarget = GetEntityFromHandle(m_pSelf->GetTarget());

		//如果目标不存在、目标死亡、和目标不在一个场景或者和目标距离超过最大追击距离
		if (!pTarget || ((CAnimal *)pTarget)->HasState(esStateDeath) || pTarget->GetPosInfo().nSceneId != m_pSelf->GetPosInfo().nSceneId 
			|| GetDistSqrtFromEntity(pTarget) >= (int)sqrt((float)m_pAiConfig->pursuitDistance)) 
		{			
			EnterEvadeMode();
			return;
		}
	}
}

void CBaseAI::EnterInitAI()
{
	// 清空移动栈
	m_pSelf->GetMoveSystem()->ClearMotionStack();

	if(m_pAiConfig->patrolRadius > 0) // 怪物巡逻
	{
		m_pSelf->GetMoveSystem()->MovePatrol(m_pAiConfig->patrolRadius,m_pAiConfig->nStopMinTime,m_pAiConfig->nStopMaxTime);
	}
}
