
#pragma once

class CSpecialAI:
	public CAnimalAI
{
public:
	typedef CAnimalAI Inherid;

public:
	virtual bool CheckEnemy();
	virtual void  UpdateAI(TICKCOUNT nCurrentTick);
private:
	CTimer<1500>        m_checkMonsterTimer;    
};
