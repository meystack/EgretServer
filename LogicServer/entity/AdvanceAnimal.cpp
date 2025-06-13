#include "StdAfx.h"
#include "AdvanceAnimal.h"

bool CAdvanceAnimal::Init(void * data, size_t size)
{
	if ( !Inherited::Init(data,size) )
		return false;
	/*
	if(!m_Equiptment.Create(this,data,size))
	{
		OutputMsg(rmWaning,_T("Init fail for equip system "));
		return false; // 属性子系统的初始化
	}
	*/

	//创建的时候是死亡的需要记录为死亡状态
	if(GetProperty<unsigned int>(PROP_CREATURE_HP) ==0)
	{
		AddState(esStateDeath);
	}
	//m_EquipChkTimer.Reset();

	m_nReduceEquipDropRate = 0;

	return true;
}

VOID CAdvanceAnimal::Destroy()
{
	//m_Equiptment.Destroy(); 
	Inherited::Destroy();
}

//VOID CAdvanceAnimal::LogicRun(TICKCOUNT nCurrentTime)
//{
//	/*
//	if (m_EquipChkTimer.CheckAndSet(nCurrentTime))
//	{
//		m_Equiptment.OnTimeCheck(nCurrentTime);
//	}
//	*/
//
//	Inherited::LogicRun(nCurrentTime);
//}
