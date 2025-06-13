#pragma once

//////////////////////////////////////////////////////////////////////////
// 基本AI：针对无法攻击其他目标的怪物或者NPC
// AI行为：怪物不会攻击别人；被别人攻击会设置目标，进入战斗，不会还手；
//		   目标不存在、目标死亡、目标和自己不在同一场景、距离过远都会导致回归

class CBaseAI : public CAnimalAI
{
public:
	typedef CAnimalAI Inherit;

	virtual void AttackStart(CAnimal *pTarget);

	virtual void AttackStop(CAnimal *pTarget){}

	virtual void AttackedBy(CAnimal *pAttacker, bool boInherit = true);

	virtual void UpdateAI(TICKCOUNT nCurrentTick);

	virtual void EnterInitAI();

	virtual bool NeedOntimeCheck(TICKCOUNT nCurrentTick){ return false; }

	virtual bool CheckEnemy(){ return false; }

	virtual bool NeedReSelTarget(){ return false; }

	virtual void ChangeTarget(CAnimal *pTarget) {}
		
private:
	/*
	* Comments: 计算自己和目标实体的距离的平方
	* Param CEntity * pTarget: 目标实体
	* @Return int:
	*/
	int GetDistSqrtFromEntity(CEntity *pTarget);
};