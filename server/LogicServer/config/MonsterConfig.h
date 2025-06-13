#pragma once
class CAttrCalc;
using namespace wylib::container;

#define MAX_DROP_GROUP_COUNT 8  //最大的掉落组的ID
static const char *szBossDigServerCount	= "szBossDigServerCount";		

//怪物的爆出的配置,1个配置项
typedef struct tagOneDropData
{
	char bAwardType;	//奖励类型
	char bStrong; // 强化等级
	
	WORD wItemID; //物品的ID


	char bBind;		// 绑定属性
	char bMinStrong;  //最小的强化
	char bMinQuality;    //最小的品质
	char bBroadCastType; //怪物广播的类型  0:不广播 1：附近  2：场景 3：副本  4：世界 5：同阵营广播, 10:发给自己
	char bBroadCast; //自定义广播，1挖boss 广播
	int nMinCount;	//最小数量(用于随机奖励数量)
	int nMaxCount;	//最大数量(用于随机奖励数量)

	int nCount; //数量
	
	int nPropability; //概率，万分之1

	bool bTriggerEvent; //是否触发脚本
	char bReserver;
	char bQuality; // 品质
	WORD nQualityDataIndex; //生成极品属性
	BYTE bGroupId; //组的ID


	int nSeed;        //掉落的种子，第几个怪物死亡的时候掉落
	int nDeadTimes;   //该怪物已经死亡的次数
	int	 nTime;		// 物品剩余的存在时间（秒为单位）。0表示不限时间，默认为0


	BYTE bAuxParam; //辅助参数

	char bSex;  //需要的性别
	char bJob; //需要的职业

	int nLootTime;		//掉落在地上可以拾取的时间 单位秒		默认为0
	int nExpireTime;	//掉落在地上的存活时间 默认为全局配置里的时间，这里是针对需要特殊处理的 单位秒
	byte nGrowLv;		//
	BYTE nGrowLvMax;	//
	unsigned int nStartDate;	//开始时间
	unsigned int nEndDate;	//结束时间

} ONEDROPDATA,*PONEDROPDATA;

//掉落组
typedef struct tagDropGroup
{
	DataList<ONEDROPDATA> data;
	BYTE groupNextHitIndex[MAX_DROP_GROUP_COUNT];    //掉落下次掉落的那个index
	BYTE groupMemberCount[MAX_DROP_GROUP_COUNT]; //掉落组一个组里的物品的种类
}DROPGROUP, *PDROPGROUP;

//怪物的技能
/*
typedef struct tagMonsterSkill
{
WORD nSkillID;
WORD nSkillLevel;
}MONSTERSKILL,*PMONSTERSKILL;
*/

//怪物被挖记录
typedef struct tagMonsterActorDig
{
	unsigned int nActorId;
	BYTE nHadDigCount;
}MONSTERACTORDIG;
//仇恨(选择攻击目标用)
typedef struct tagMonsterHatred
{
	unsigned int nActorId;		    //玩家id
	BYTE nEntityType;			//攻击实体类型(actor,hero,pet单独计算)
	INT_PTR nDamageValue;//累计伤害值
	unsigned int nLastAttackTime;//上次攻击时间
	tagMonsterHatred()
	{
		memset(this,0, sizeof(*this));
	}
	
}MONSTERHATRED;
typedef enum BossDropDigFlag
{
	bdfCantDrop = 0,	//不是挖出物品
	bdfDropNormol = 1,	//可挖出的普通物品
	bdfDropRare = 2,	//可挖出的珍稀物品
};
typedef enum MonsterSayType
{
	mstInvalid = -1,
	mstIdle,
	mstFoundTarget,
	mstAttack,
	mstAttacked,
	mstDead,
	mstHpLow,
	mstCreate,
	mstDisappear,
	mstMax,
};

// 怪物发言(每条发言的配置数据)
typedef struct tagMonsterSayInfo
{	
	int						nType;				// 发言类型（和MonsterSayType对应）
	int						nRate;				// 发言概率。 nRate=0表明不发言
	int						nParam;				// 发言参数。如果是血量低于多少的发言，这里配置血量百分比
	int						nCount;				// 发言数量
	MonsterSayBCType		nBCType;			// 广播类型
	int						nTipType;			// 发言提示类型
	char**					pMsgList;			// 发言内容
	int						nLimitLev;			// 发言显示等级限制
	bool					bTransmit;			// 是否添加速传
	int						nLimitTime;			// 速传有效时间
	int						nPosX;				// 速传坐标X
	int						nPosY;				// 速传坐标Y
}MonsterSayInfo;

// 每一类怪物发言配置
typedef struct tagMonsterSayTypeConfig
{
	int						nCount;				// 此类发言配置数量
	MonsterSayInfo*			pMonsterSayCfg;		// 怪物每类发言配置数据
}MonsterSayTypeConfig;

// 怪物发言列表配置
typedef struct tagMonsterSayList
{
	int						nLowerInterval;			// 随机发言时间下限
	int						nUpperInterval;			// 随机发言时间上限
	MonsterSayTypeConfig	vecSayTypeInfo[mstMax];	// 发言类型列表
}MonsterSayList;

// 怪物优先攻击目标怪物列表
typedef struct tagPriorAttackTarget
{
	int						nCount;				// 列表数量
	int						*pTargetIDList;		// 怪物ID列表
}PriorAttackTarget;

typedef struct tagMonsterSkill
{
	WORD wSkillID; //技能的ID
	BYTE bSkillLevel; //技能的等级
	BYTE bEvent;// 在什么事件下触发,0表示普通的战斗，1表示死亡
	bool bAutoCDWhenEnterCombat; // 技能是否在进入战斗立即CD
	//BYTE bCount; //使用的次数
	//unsigned int nInterval; //技能的使用间隔,单位ms
}MONSTERSKILL,*PMONSTERSKILL;

typedef struct tagMonsterConfig
	//public CREATUREDATA
{
public:
	enum tagMonsterType
	{
		mtCommon = 1,		//普通怪
		mtElite = 2,		//精英怪
		mtToumu = 3,		//头目
		mtBoss = 4,			//boss
		mtKungfu = 6,		//练功师怪
		mtHumanoid = 7,		//人形怪
		mtWorldBoss = 8,		//世界boss
		
		mtGirl = 9,			//仙女采集怪
		mtFubenCollect = 10,	//副本中采集怪，先扣物品
		mtCollectOnce = 11,	//只能采集一次
		mtMax,			
	};
	tagMonsterConfig()
	{
		memset(this,0,sizeof(*this)); 
	}
	CAttrCalc cal; //使用计算器存储，初始化的时候将初始化完毕实体的计算器
	
	int nOriginalLevel ;//原始的等级
	int nLevel;  //等级
	int nCircle;  //转生

	
	int nDeadTimes;			//死亡次数
	int nMaxDeadTimes;		//升到下一级级最大死亡次数
	
	int nEntityId; //怪物的di
	
	ACTORNAME szName;
	int nModelId;             //模型的ID
	//unsigned int nAttackSpeed; //攻击速度
	unsigned int nExp ;		//怪物掉落的基础经验
	unsigned int nInnerExp;	//DELETE
	BYTE btMonsterType; //怪物的类型

	//用于人形怪外观
	WORD nWeaponId;		//武器外观ID
	WORD nSwingId;		//翅膀外观ID


	int  nMaxDropHp ;  //最大的掉血量，如果为0就是没有限制按正常流程，如果配置了每次掉的血最多这个数

	//公式 An*n + Bn + c
	int		nMaxHardLv;		//最大boss难度等级
	float	fLevelA;		//计算boss升到下一等级需死亡次数系数A
	float	fLevelB;		//计算boss升到下一等级需死亡次数系数B
	float	fLevelC;		//计算boss升到下一等级需死亡次数系数C

	float	fPropA;			//计算boss在升级后的属性加成系数A
	float	fPropB;			//计算boss在升级后的属性加成系数B
	float	fPropC;			//计算boss在升级后的属性加成系数C

	WORD wAiConfigId; //怪物的ai配置的ID
	
	BYTE bCamp;      //怪物的阵营

	int nAttackInterval ;  // 怪物的攻击间隔，比如使用技能或者肉搏后需要间隔多长时间才能使用，默认是500
	int nattackLevel;  //攻击等级，用于决定怪物被攻击的优先等级,玩家是100，等级越高的越容易被攻击，数值没有上限

	int  maxDropHp; // 
	DROPGROUP drops; //怪物的掉落
	
	int  bAttackMusicId;      //攻击声音的id
	int  bDieMusicId;         //死亡声音的id
	int  bTargetMusicId;      //发现目标声音的id
	int  bDamageMusicId;      //伤害声音的id


	BYTE  bAttackMusicRate;   //攻击声音的概率
	BYTE  bDieMusicRate;	  //死亡声音的概率
	BYTE  bTargetMusicRate;   //发现目标声音的概率
	BYTE  bDamageMusicRate;   //伤害声音的概率

	bool bCreateRank;	//创建怪物是否创建排行榜
	WORD wReSelTargetRate;		// 怪物重新选择敌人的概率。（百分比，默认为0）
	BYTE btAttackType;  // 怪物攻击类型
	BYTE btEntityType;	// 实体类型

	unsigned int nMonsterColor;	// 怪物颜色

	MonsterSayList monsterSayList; // 怪物发言配置
	PriorAttackTarget priorAttackTarget; // 怪物优先攻击列表

	EntityFlags* pflags;		//怪物的属性标志位

	BYTE btDir;				//怪物初始朝向

	WORD wDamageSkill;		//只能被特定技能伤害

	int nGatherLevel;		//采集等级限制

	bool bRecordKillTime;		//需要记录最近一次被击杀的时间（自从服务器启动）
	int nDropid; //掉落组
	int nFlagId; //标志组id
	int nMonLog;//是否统计
	DataList<MONSTERSKILL> skills;
	int nAscriptionOpen;//非活动副本Boss归属描述开关
} MONSTERCONFIG,*PMONSTERCONFIG;

