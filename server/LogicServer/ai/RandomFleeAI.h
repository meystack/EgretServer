#pragma once

class CRandomFleeAI : public CAnimalAI
{
public:
	typedef CAnimalAI Inherit;
	
	virtual void AttackStart(CAnimal *pAnimal){}

	virtual void AttackStop(CAnimal *pTarget){}

	virtual void AttackedBy(CAnimal *pAttacker, bool boInherit = true);

	virtual void UpdateAI(TICKCOUNT nCurrentTick)
	{
		Inherit::CheckEnemy();
	}

	virtual void EnterInitAI()
	{
		ResetRandomMovement(false);
	}

	virtual bool NeedOntimeCheck(TICKCOUNT nCurrentTick){ return false; }

	virtual bool CheckEnemy(){ return false; }

	virtual bool NeedReSelTarget(){ return false; }

	virtual void ChangeTarget(CAnimal *pTarget) {}

private:
	void ResetRandomMovement(bool bStartImme = false);
	
};