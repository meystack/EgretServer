#pragma once
/*
   被动怪的实现类
*/
class CPassiveAi:
	public CAnimalAI
{
public:
	typedef CAnimalAI Inherid;
	/*
	* Comments: 开始攻击其他实体
	* Param CCreature * pTarget:目标实体的指针
	* @Return void:
	*/
	//virtual void AttackStart(CAnimal * pTarget);

	/*
	* Comments:结束攻击其他实体
	* Param CAnimal * pTarget:目标实体的指针
	* @Return void:
	*/
	//virtual void AttackStop(CAnimal * pTarget);
	

	/*
	* Comments:受到其他实体的攻击
	* Param CAnimal * pAttacker:攻击者的指针
	* @Return void:
	*/
	//virtual void AttackedBy(CAnimal * pAttacker) ;


	/*
	* Comments: 更新怪物的AI
	* Param TICKCOUNT nCurrentTik:当前的tick
	* @Return void:
	*/
	//virtual void  UpdateAI(TICKCOUNT nCurrentTick);

	/*
	* Comments: 自身刚刚被杀死
	* Param CAnimal *pAttacker: 杀死自己的实体指针
	* @Return void:
	*/
	//virtual void KillBy(CAnimal  *pAttacker) ;

	/*
	* Comments: 杀死另外一个实体
	* Param CAnimal *: 被杀死者的指针
	* @Return void:
	*/
	//virtual void KillOther(CAnimal *);
	


	/*
	* Comments: 受到伤害
	* Param CAnimal * pAttacker: 攻击者的指针
	* Param nDamageValue: 伤害的值
	* @Return void:
	*/
	//virtual void DamageBy(CAnimal * pAttacker,INT_PTR nDamageValue );

	/*
	* Comments: 伤害输出给一个实体
	* Param CAnimal * pTarget: 目标实体
	* Param INT_PTR nDamageValue:伤害的值
	* @Return void:
	*/
	//virtual void DamageTo(CAnimal * pTarget,INT_PTR nDamageValue);

	/*
	* Comments: 进入逃避模式，开始逃避
	* @Return void:
	*/
	//virtual void EnterEvadeMode();
};