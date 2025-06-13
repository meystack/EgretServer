#include "StdAfx.h"
#include "Creature.h"


VOID CCreature::SetDestoryTime(UINT nTime)
{
	m_nDestoryTime = GetGlobalLogicEngine()->getMiniDateTime() + nTime;//5秒后清除
}

VOID CCreature::ChangeHP(int nValue,CEntity * pKiller,bool bIgnoreDamageRedure, bool bIgnoreMaxDropHp, bool boSkillResult, int btHitType)
{
	if(nValue == 0) 
	{
		return;
	}
	int nCurrent =  GetProperty<unsigned int >(PROP_CREATURE_HP);

	int nResult =nValue +  nCurrent;
	int nMaxValue =(int) GetProperty<unsigned int >(PROP_CREATURE_MAXHP);

	if(0 <= nValue && nMaxValue <= nCurrent)
	{
		return;
	}
	
	if(nResult <0)
	{
		nResult =0;
	}
	else
	{
		if( nResult >nMaxValue )
		{
			nResult= nMaxValue;
		}
	}
	SetProperty<unsigned int>(PROP_CREATURE_HP,(unsigned int)nResult);
	if(nResult == 0 && GetType() == enActor)	//反弹死么有名字的问题(会调用二次被杀)
	{
		//((CActor*)this)->TriggerBeKilledEvent(pKiller,0);		//通知前端复活面板显示扣了多少
	}
}