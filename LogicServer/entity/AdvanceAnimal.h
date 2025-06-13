
#pragma once

/***************************************************************/
/*
/*                     高级动物,继承于动物,包括宠物和玩家,有装备子系统
/*
/***************************************************************/


//怪物和玩家拥有的属性
struct AdvanceAnimalProp
{
	int		m_nReduceEquipDropRate;	     //减少玩家死亡爆装备几率 10000为单位
	
	WORD     m_nWarriorDamageRateDec;    //百分比降低战士的伤害
	WORD     m_nMagicianDamageRateDec;  //百分比降低法师的伤害
	WORD     m_nWizardDamageRateDec;     //百分比降低道士的伤害
	WORD     m_nMonsterDamageRateDec;    //百分比降低怪物的伤害

	WORD     m_nDamageReduceRate;       //触发伤害减免的几率
	WORD     m_nDamageAddRate;			//触发伤害追加的几率
	WORD     m_nIgnorDefenceRate;       //触发无视防御几率

	
	int     m_nWarriorDamageValueDec;   //固定值降低战士的伤害
	int     m_nMagicianDamageValueDec; //固定值降低法师的伤害
	int     m_nWizardDamageValueDec;    //固定值降低道士的伤害
	int     m_nMonsterDamageValueDec;   //固定值降低怪物的伤害
	
	int     m_nDamageReduceValue;      //触发伤害减免的值
	int     m_nDamageAddValue;		   //触发伤害追加的值
	int     m_nIgnorDefenceRateSkill;  //技能出发无视防御几率

	AdvanceAnimalProp()
	{
		memset(this,0,sizeof(*this));
	}
};


class CAdvanceAnimal:
	public CAnimal,
	public AdvanceAnimalProp
{
public:
	typedef CAnimal Inherited;

	/*** 覆盖父类的函数集 ***/
	//初始化
	virtual bool Init(void * data, size_t size); 
	//析构
	virtual void Destroy();

	////例行逻辑检查函数
	//virtual void LogicRun(TICKCOUNT nCurrentTime);

protected:
	//CUserEquipment		m_Equiptment;		//装备子系统
	//CTimer<1000>		m_EquipChkTimer;	//装备子系统时间检查Timer	
};
