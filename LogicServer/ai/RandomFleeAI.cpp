#include "StdAfx.h"


void CRandomFleeAI::ResetRandomMovement(bool bStartImme)
{
	m_pSelf->GetMoveSystem()->ClearMotionStack();
	// 对于随机逃跑怪，借用巡逻距离作为单次逃跑的最大距离
	if(m_pAiConfig->patrolRadius > 0)
	{
		m_pSelf->GetMoveSystem()->MoveRandomFlee(m_pAiConfig->patrolRadius, 
												 m_pAiConfig->nStopMinTime, 
												 m_pAiConfig->nStopMaxTime, 
												 bStartImme);
	}
}

void CRandomFleeAI::AttackedBy( CAnimal *pAttacker, bool boInherit )
{
	ResetRandomMovement(true);
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
