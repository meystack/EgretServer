#pragma once
/***************************************************************/
/*
/*                    属性子系统
/*  1) 根据玩家的1级属性计算玩家的2级属性
/*  2) 根据玩家的等级,性别,职业,来决定玩家的模型
/*  3) NPC的将初始化系统,进行一些属性的改变运算
/***************************************************************/
#pragma once

class CCreature;

class CPropertySystem:
	public CEntitySubSystem<enPropertySystemID,CPropertySystem,CCreature>
{
public:
	//重载这个函数,初始化玩家的一些2级属性
	bool Initialize(void *data,SIZE_T size);

	/// 玩家升级的时候调用,需要计算一下模型ID是否需要改变
	inline VOID OnLevelUp(){ResertProperty();}
	
	/// 重置属性
	bool ResertProperty();	
	
	/*
	* Comments: 初始化怪物的属性计算器，为了计算的方便，把怪物的属性放到计算器里
	* Param CREATURBATTLEEDATA & data:  怪物的2级属性
	* Param CAttrCalc & cal: 目标计算器
	* @Return VOID:
	*/
	static VOID InitMonsterCalculator( CREATURBATTLEEDATA & data, CAttrCalc& cal );

	
	/*
	* Comments:把1级属性的数值属性的加成到2级属性里去
	* Param CAttrCalc & cal:计算器,把里边的1级属性映射到2级属性
	* Param PLEVEL1PROPERTYTRANSFER pProp: 属性转换表
	* @Return VOID:
	*/
	static VOID Level1ValuePropertyTransfer( CAttrCalc& cal,PLEVEL1PROPERTYTRANSFER pProp); //

	/*
	* Comments:初始化实体的等级信息，包括初始化的属性+等级带来的属性的加成
	* Param CAttrCalc & cal: 结果计算器
	* Param PENTITYLEVELPROPERTY pPropTable:等级的属性的配置
	* Param INT_PTR nLevel:实体的等级
	* @Return void:
	*/
	static void InitEntityLevelProperty(CAttrCalc &cal,PENTITYLEVELPROPERTY pPropTable,INT_PTR nLevel);
	
	/*
	* Comments:计算加成boss难度等级
	* Param PMONSTERCONFIG pConfig:怪的配置
	* @Return bool:成功返回true
	*/
	static bool calcMonsterLevelHard( PMONSTERCONFIG pConfig,CAttrCalc &cal,INT_PTR nBornLevel = 0, const INT_PTR nLevel=0);

	/*
	* Comments: 设置属性
	* @Param INT_PTR nPropertyType: 属性类型
	* @Param int nAddValue: 百分比
	* @Return void:
	* @Remark:
	*/
	void AddPropertyValue(INT_PTR nPropertyType, int nAddValue);//
	/*
	* Comments: 设置属性
	* @Param INT_PTR nPropertyType: 属性类型
	* @Param int nAddValue: 万分比
	* @Return void:
	* @Remark:
	*/
	void AddPropertyRatioValue(INT_PTR nPropertyType, int nAddValue);//

#ifdef _DEBUG
public:

	CAttrCalc m_Debug;

#endif

private:
	
	//设置玩家的属性,把计算器里的属性写到属性集
	VOID SetEntityProperty(CAttrCalc& cal);
	
	/// 初始化怪物的属性,把结果写到计算器
	VOID InitMonsterProperty(CAttrCalc& cal);

	//初始化玩家初始属性
	bool InitActorInitProperty(CAttrCalc &cal);

	//初始化玩家的等级属性信息,把结果写到计算器
	bool InitActorLevelProperty(CAttrCalc& cal);

	//初始化玩家的转生属性信息,把结果写到计算器
	bool InitActorCircleProperty(CAttrCalc &calc);
	//初始化玩家的祝福属性信息,把结果写到计算器
	bool InitActorBlessProperty(CAttrCalc &calc);
	//初始化玩家的经脉属性信息,把结果写到计算器
	bool InitActorMeridianProperty(CAttrCalc &calc);
	//初始化玩家的狂暴属性信息，把结果写道计算器
	bool InitActorFrenzyProperty(CAttrCalc &calc);
	//初始化玩家的官阶属性信息，把结果写道计算器
	bool InitActorOfficeProperty(CAttrCalc &calc);
	/*
	* Comments: 返回这个实体的默认的移动速度，都是从配置表里读取的
	* @Return int: 返回默认的移动速度，也就是1秒钟经过多少格
	*/
	int  GetDefaultMoveSpeed();

	/*
	* Comments: 获取默认的攻击速度
	* @Return int: 返回攻击速度的间隔时间，也就是ms
	*/
	int GetDefaultAttackSpeed();

	/**
	* @brief: 更新怪物动态属性（针对副本怪）	
	* @param[in]calc: 属性计算器
	*/
	void updateMonsterDynProp(CAttrCalc &calc);

	
	//计算角色头衔属性
	bool CalcActorHeadTitleProperty(CAttrCalc &calc);

	//获取防火率
	unsigned int GetFireDefenseRate();

	/*
	* Comments:计算宠物基本属性
	* Param PMONSTERCONFIG pConfig:怪的配置
	* @Return bool:成功返回true
	*/
	bool CalcPetBaseProperty(CAttrCalc &calc, const INT_PTR nMonsterId, const INT_PTR nLevel);

};

