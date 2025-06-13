#pragma once

//玩家的职业配置
#define  LEVEL_PROPERTY_COUNT 16    //对应生命值，魔法值, 最小物理攻击,最大物理攻击，最小魔法攻击，最大魔法攻击，最小道术攻击，最大道术攻击，最小物理防御，最大物理防御，最小魔法防御，最大魔法防御，准确，敏捷
//#define  LEVEL_UP_PROPERTY_COUNT 9    //初始化属性随着等级的成长 外功攻击、内功攻击、外功防御、内功防御、生命值、内力值、命中值、闪避值、暴击值

#define MAX_CREATE_POSNUM 100		//最多的出生点

//2次方程参数配置，都使用 value= a* level ^2  + b * level +c 的2次方程配置
//其中a,b,c是浮点型的数字
typedef struct tagQuadraticParams
{
	float quadraticParam[LEVEL_PROPERTY_COUNT]; //二次方参数a,对应生命值，魔法值, 最小物理攻击,最大物理攻击，最小魔法攻击，最大魔法攻击，最小道术攻击，最大道术攻击，最小物理防御，最大物理防御，最小魔法防御，最大魔法防御，准确，敏捷
	float linearParam[LEVEL_PROPERTY_COUNT];	//1次方参数b，对应生命值，魔法值, 最小物理攻击,最大物理攻击，最小魔法攻击，最大魔法攻击，最小道术攻击，最大道术攻击，最小物理防御，最大物理防御，最小魔法防御，最大魔法防御，准确，敏捷
	float constantParam [LEVEL_PROPERTY_COUNT]; //常数参数c，对应生命值，魔法值, 最小物理攻击,最大物理攻击，最小魔法攻击，最大魔法攻击，最小道术攻击，最大道术攻击，最小物理防御，最大物理防御，最小魔法防御，最大魔法防御，准确，敏捷
}QUADRATICPARAMS, *PQUADRATICPARAMS;

////各职业每个等级的属性
//typedef struct tagEntityLevelProperty
//{
//	int levelMin;	//等级范围下限
//	int levelMax;	//等级范围上限
//	QUADRATICPARAMS params;	//2次方程参数
//}ENTITYLEVELPROPERTY,*PENTITYLEVELPROPERTY;

typedef struct tagEntityLevelProp
{
	int levelProp[LEVEL_PROPERTY_COUNT];
}ENTITYLEVELPROPERTY, *PENTITYLEVELPROPERTY;

//创建玩家的出生点
typedef struct tagActorCreatePos
{
	int      nScenceID; //初始化的场景ID
	int      nPosX; //出生点的x
	int      nPosY; // 出生点的y
}ACTORCREATEPOS;

//每个职业的初始化属性
typedef struct tagVocationInitConfig
	//public ENTITYLEVELPROPERTY
{
	//unsigned nFbID;  //初始化的副本ID
	//unsigned nScenceID; //初始化的场景ID
	//int      nPosX; //出生点的x
	//int      nPosY; // 出生点的y
	int nDefaultMaleModelID;//默认的男性模型ID
	int nDefaultFemaleModelID;//-默认的女性职业的模型ID
	int nNearAttackType;  //普通攻击的攻击类型
	int nNearAttackDis;   //普通攻击的攻击距离，默认是1格
	int nManIcon;     //男头像
	int nFemaleIcon;     //女头像
	int nMaxAnger;			//怒气的上限
	
	int heroAiId;			//英雄的ai ID
	int heroDefaultSkill;	//本职业英雄默认的技能，在创建的时候给英雄学会

	int nFireDefenseRate;	//抗火率 使用1点表示1万分之1

	DataList<CStdItem::AttributeGroup> circleAddAttrs;	//玩家转生属性加成表(同物品类似)

	DataList<ENTITYLEVELPROPERTY>  actorProps; //玩家的属性
	//DataList<ENTITYLEVELPROPERTY>  heroProps; //英雄的属性
	
	//QUADRATICPARAMS  heroMergeProp;	//英雄附体的属性

	//char   maleHeroName[32];    //男英雄的默认名字
	//char   femaleHeroName[32];  //女英雄的默认名字

	//int    nPosCount;			//出生点的的个数
	//ACTORCREATEPOS nCreatePos[MAX_CREATE_POSNUM];		//出生点

} VOCATIONINITCONFIG,*PVOCATIONINITCONFIG;

//出生的副本配置
typedef struct tagCreateDefaultFb
{
	int  nToken;
	int  nFbId;

	int    nPosCount;			//出生点的的个数
	ACTORCREATEPOS nCreatePos[MAX_CREATE_POSNUM];		//出生点
}CRESTEDEFAULTFb;

/* 暴击几率（%） = 攻击方暴击值  / （ 攻击方角色等级 * 100 ） ，这里的100的配置的是参数
	防守方闪避几率（%） = 防守方闪避值 / (防守方角色等级*100) ，这里的100的配置的是参数
	暴击几率（%） = 攻击方暴击值  / （ 攻击方角色等级 * 100 ），这里的100的配置的是参数
*/
typedef struct tagVocationInitTable
{
	unsigned int nDefaultMoveSpeed;  //玩家的默认速度
	unsigned int nDefaultAttackSpeed; //默认的攻击速度
	AttriGroup	  initAttrs;//属性
	
	VOCATIONINITCONFIG vocationTable[enMaxVocCount];

	CVector<CRESTEDEFAULTFb> nDefualtFb;

	/*
	* Comments: 根据职业和性别，获取默认的模型ID
	* Param INT_PTR nVocation:职业id
	* Param INT_PTR nIsFemale:是女的1，男的0
	* @Return int:返回默认的模型ID
	*/
	inline int GetVocationDefaultModel(INT_PTR nVocation,INT_PTR nIsFemale)
	{
		if(nVocation >=0 && nVocation < enMaxVocCount )
		{
			if(nIsFemale)
			{
				return vocationTable[nVocation].nDefaultFemaleModelID;
			}
			else
			{
				return vocationTable[nVocation].nDefaultMaleModelID;
			}
		}
		return 0;
	}

	//更据职业获取
	inline int GetVocationMaxAnger(INT_PTR nVocation)
	{
		if(nVocation >=0 && nVocation < enMaxVocCount )
		{
			return vocationTable[nVocation].nMaxAnger;
		}

		return 0;
	}

	//获取等级属性参数
	inline PENTITYLEVELPROPERTY GetVocationLevelPropParam(INT_PTR nEnityType, INT_PTR nVocation, INT_PTR nLevel)
	{
		if (nVocation < 0 || nVocation >= enMaxVocCount)
		{
			OutputMsg(rmError, _T("%s 不是有效的职业:%d"), __FUNCTION__, nVocation);
			return NULL;
		}
		DataList<ENTITYLEVELPROPERTY> *pProps = NULL;
		switch (nEnityType)
		{
		case enActor:
			pProps = &(vocationTable[nVocation].actorProps);
			break;
		case enHero:
			//pProps = &(vocationTable[nVocation].heroProps);
			break;
		}

		if (pProps == NULL) return NULL;

		if (nLevel > 0 && nLevel < pProps->count )
		{
			return &pProps->pData[nLevel];
		}

		return NULL;
	}

} VOCATIONINITTABLE ,*PVOCATIONINITTABLE;

