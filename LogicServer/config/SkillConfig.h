#pragma once




//技能配置表
#define MAX_SECRET_SAUCE_COUNT_ONE_SKILL 4  //一个技能最大的秘笈数量
#define MAX_SKILL_LEVEL 20        //技能的最大的等级
#define SKILLCON_PARAM 5		  //技能筛选69，70参数

//技能的中心点的类型
enum enSkillCerterPosType
{
	stSkillTargetCenter		= 0, //以目标为中心
	stSkillActorCenter		= 1, //以主角为中心
	stSkillSpellPointCenter = 2, //以施法点为中心
	stSkillActorCenterDir   = 3, //以主角瞬时坐标方向为中心
};

//范围的分类
enum enSkillRangeType
{
	stSkillRangeSingle,					 //无范围，仅针对目标的单体技能(单体)
	stSkillRangeRotationSingle,			 //线性旋转(单体)
	stSkillRangeRotationGroup,			 //线性范围(群体)
	stSkillRangeGroup,					 //范围(群体)
	stSkillRangeAccurateRotation,		 //矢量精确旋转，以中心点与鼠标的位置的连线作为旋转轴旋转，比线性旋转更精确，用于法师的火球术
    stSkillRangePointCenter,             //施法点中心范围(群体),  与范围(群体)不同的是传递给PointSkillResult的是施法点中心坐标
	stSkillRangeNoTarget,				 //无目标的
};

enum enSkillType
{
	stPassiveSkill				= 0,	//被动技能
	stPhysicAttackSkill			= 1,	//物理攻击技能
	stMagicAttackSkill			= 2,	//魔法攻击技能
	stTransferSkill			    = 3,	//传送技能
	stLifeSkill					= 4,    //生活技能
	stOtherSpecialSkill			= 5,    //其他特殊的技能，比如队伍buff，通过技能实现的
};

enum enSkillClass
{
	scSkillClassVocBasic		= 1,	//职业基本技能
	scSkillClassMonster			= 2,	//怪物技能
	scSkillCriticalBlow			= 3,	//必杀
	scSkillDig					= 4,	//挖矿技能 TO DELETE
	scSkillHideAttach			= 5,	//隐藏的附加技能(烈火剑法,逐日剑法，不检查)
	scSkillRide					= 6,	//座驾技能
	
	scSkillClassGuild			= 11,	//行会技能，展示在行会面板上的技能(保存于人物的技能列表中)
};

//技能的释放结果的ID
enum enSkillResultID
{
	srSkillResultError					= 0,	//占位
	srSkillResultPhysicAttack			= 1,	//物理攻击
	srSkillResultMagicAttack			= 2,	//法术攻击
	srSkillResulWizardAttack			= 3,	//道术攻击
	srSkillResultAddBuff				= 4,	//增加buff
	srSkillResultDelBuff				= 5,	//删除buff
	srSkillCatchMonster					= 6,	//抓怪
	srSkillResultBeatBack				= 7,	//击退
	srSkillResultMobFire				= 8,    //召唤火墙
	srSkillResultMobPet					= 9,    //召唤宠物
	srSkillResultGoAhead				= 10,   //往前跑若干格
	srSkillResultGoBack					= 11,   //向后退若干格
	srSkillResultLastSafeAreaRTransfer	= 12,   //上一个安全区随机传送
	srSkillResultChangeProperty			= 13,	//被动技能
	srSkillResultCure					= 14,	//瞬间治疗
	srSkillResultHolyAttack				= 15,	//神圣攻击
	srSkillResultMoveToTarget			= 16,   //瞬移到目标旁
	srSkillResultMoveBackHome			= 17,   //瞬移回巢
	srSkillResultTransfer				= 18,   //指定场景地点传送
	srSkillResultSceneTransfer			= 19,   //本场景随机传送
	srSkillResultLastSafeAreaTransfer	= 20,	//上一个安全区传送

	srSkillResultDeleteEffect			= 21,	//删除特效
	srSkillResulIgnoreDefence			= 22,   //忽视目标的防御
	srSkillResulCatchTarget				= 23,	//把目标抓到自己身边
	srSkillResultSetAttackTarget		= 24,	//设置攻击目标
	srSkillResultMobEntity				= 25,   //刷怪
	srSkillResultChargeAttack			= 26,	//冲锋
	srSkillResultSelfHpCureTarget		= 27,	//自身的HP的百分比治疗目标
	srSkillResultLostTarget				= 28,	//丢弃目标
	srSkillResultChangEquipDua			= 29,	//改变装备的耐久
	srSkillResultDropMonsterItem		= 30,	//按怪物的掉率掉落东西
	srSkillResultScreenShake			= 15,	//振屏
	srSkillResultChangeFubenProgress	= 32,	//改变副本进度
	srSkillResultChangeModel			= 33,	//改变外形
	srSkillResultRateChangeProperty		= 34,	//百分比改属性
	srSkillResultDeleteSkillCd			= 35,	//删除技能的CD
	srSkillResultGiveAward				= 36,	//给奖励	
	srSkillResultBeatFly				= 37,	//击飞
	srSkillResultPlayGuide				= 38,	//客户端播放引导性特
	srSkillResultIgnoreMaxDropHp		= 39,   //忽略每次最大伤害限制
	srSkillResultPointMove				= 40,	//把目标移动到地图一个点
	srSkillResultSelfPointMove			= 41,	//自己移动到范围内的鼠标指定的点
	srSkillResultTargetRapidMove		= 42,	//向某目标冲撞
	srSkillResultRemovePet				= 43,	//删除宠物
	srSkillResultChangeMonsterAttacked  = 44,	//改变怪物是否能被攻击
	srSkillResultFastSelfCurHp			= 45,	//瞬回血
	srSkillResultTargetPointMove  		= 46,   //把目标移动到范围内的鼠标指定的点

};

//技能的训练或者释放条件ID
enum enSkillTrainCondition
{
	/// 释放条件
	scSkillConditonHP =1, 			//消耗命
	scSkillConditonMP =2, 			//消耗蓝
	scSkillConditionWithoutBuff =3, //不存在buff
	scSkillConditionTargetDistance =4,//和目标的距离

	/// 旧的

	scSkillConditonLevel =1, //等级必须大于等于一定的级别
	scSkillConditonMoney =2, //金钱
	scSkillConditonItem =3, //物品
	scSkillConditonExp =4, //经验
	scSkillConditonXiuwei =5, //修为
	scSkillConditonVocation =6, //职业需求
	//scSkillConditonHP =7, //消耗命
	//scSkillConditonMP =8, //消耗蓝
	scSkillConditionTargetExist =9, //目标存在
	scSkillConditionPetCount =10, //存在宠物的数量
	scSkillConditionWithBuff =11, //存在buff
	//scSkillConditionWithoutBuff =12,// 身上必须没有指定类型的buff
	//scSkillConditionTargetDistance =13,//和目标的距离
	scSkillConditionTargetOverLap =14,//和目标重叠

	scSkillConditionFaceTarget =15, ////需要面向目标
	scSkillConditionOneDirTarget =16,//需要和目标的方向
	scSkillConditionMountExpLimited=17,//坐骑经验不够
	scSkillConditionHpRateBelow=18,//自身的血的百分比必须低于一定的值

	scSkillConditionSpecialTarget =19,  //需要特定的目标
	scSkillConditionTargetCanAttack =20,  //目标可攻击
	scSkillConditionEfficiency = 21,    //需要生活技能的熟练度
	scSkillConditionJumpPoweer =22,  //轻功不够
	scSkillConditinBindCoin =23,  //绑定银两的消耗
	scSkillConditinCampTitle =24,  //阵营的职位的条件
	scSkillConditionBrotherCount =25, //结拜的数目

	scSkillConditionTargetDis =26, //需要目标，并且和目标的y轴的绝对距离少于等于value
	scSkillConditioOnlineBrotherCount =27, //队伍里的在线的结拜好友的数量 >= value，不包括自己

	scSkillConditionBaseMpRate =28,		//消耗基础蓝的万分比  value表示基础蓝的万分比，比如8500表示85%，玩家的基础蓝表示等级带来的MaxMp,不包括buff，装备等附加的
	scSkillConditionBaseHpRate=29,		//消耗基础血的万分比  value表示基础血的万分比，比如8500表示85%，玩家的基础血表示等级带来的MaxHp,不包括buff，装备等附加的
	scSkillConditionBattleState =30,	//在与玩家的战斗状态,value 1表示是 0表示否

	scSkillConditionGuildCont=31,
	scSkillConditionSkillLevel =32,
	scSkillConditionXpValue=33,
	scSkillConditionMetiralDura =34, //装备材料的耐久消耗
	scSkillConditionTargetLine =35, //和目标一条直线上

	scSkillConditionDigTool =36, //挖矿工具
	scSkillConditionAnger =37, //怒气值
	scSkillConditionArea = 38, //需要特定的区域类型,value表示区域的属性类型
	scSkillConditionCircle =39, //转数的限制

};


//技能的效果类型
enum enSkillEffectType
{
	stSkillEffectTypeNone			= 0, //无效果
	stSkillEffectTypeSwing			= 1, //挥洒
	stSkillEffectTypeCast			= 2, //投掷与爆炸一样（暂时未用）
	stSkillEffectTypeFly			= 3, //飞行
	stSkillEffectTypeExplode		= 4, //爆炸 与投掷一样 上层
	stSkillEffectTypeFootAlways		= 5, //脚下持续
	stSkillEffectTypeAlways			= 6, //持续 只能人物使用
	stSkillEffectDirByActor			= 7, //只能人物使用,按方向
	stSkillEffectTypeExplodeFloor	= 8, //爆炸 下层
};

enum enSkillPropertyValueType
{
	ptValueTypeInt=0, //属性的数值类型是整数
	ptValueTypeFloat =1,//属性的数值类型是浮点数
	ptValueTypeBuffRate = 2,//被动技能触发添加buff的概率
	ptValueTypeBuffId  =3,//被动技能触发添加buff的Id
	ptValueTypeSkillAddBuff =4,//学习/遗忘技能时添加/删除buff
};


//技能在特殊的buff下是否能用，包括封、断、晕眩
enum enSkillSpecialBuffCondition
{
	stSkillSpecialBuffNone =0,      //都不能用
	stSkillSpecialBuffFengDuan,  //在封断的时候能够使用
	stSkillSpecialBuffDizzy,     //在晕眩的时候可以使用
	stSkillSpecialBuffAll,       //在封断的时候都能够使用
};

//伤害类型，用于技能命中和一些攻击、防御时判定
enum enSkillHitType
{
	htDefault = 0,	//默认
	htFire,			//火墙
	htBuff,         //buf伤害
};

//技能升级或者释放的条件，分表表示条件的ID,value，是否消耗，用在条件的判断上
typedef struct tagSkillTrainSpellCondition
{
	BYTE nConditionID;  //条件ID
	BYTE bConsumed;     //是否要消耗
	BYTE bReserve;      //保留
	int  nCount;   //数量
	INT64  nValue;        //值
} SKILLTRAINSPELLCONDITION,*PSKILLTRAINSPELLCONDITION;

//技能的秘籍数据
typedef struct tagSkillMiji
{
	WORD nMijiId;    //秘籍的ID
	WORD nMinLevel;  //可以学习该秘籍的最低的技能等级
}SKILLMIJI,*PSKILLMIJI;


//Buff条件需要单独判断
/*
typedef struct tagBuffCondition
{
	BYTE nType; // buff的类型 
	BYTE nCount; //数量 
	WORD nID;   // buff的id
} BUFFCONDITION, *PBUFFCONDITION;
*/


//技能目标选择的筛选条件
typedef struct tagTargetSelCondition
{
	int nKey; //条件ID
	GAMEATTRVALUE value; //条件的值
}TARGETSELCONDITION,*PTARGETSELCONDITION;


//特效的配置
typedef struct tagSpecialEffect
{
	BYTE nType;  //特效的类型
	BYTE nAlways; //是否总是生效
	int nID;   //特效的ID
	int nKeepTime;//持续时间
	int nDelayTime; //延迟生效时间
	BYTE nSceneEffect;
	int nTotalCount;//特效总帧数
	int nTotalDirCount;//总方向数
} SPECIALEFFECT,*PSPECIALEFFECT;

//技能的释放结果
typedef struct tagSkillResult
{
	int		nId;
	int		nValue;
	int		nDelay;
	int		nParam1;
	int		nParam2;
	int		nParam3;
	int		nParam4;
	int		nParam5;
	int		nParam6;
	int		nResultType;
	BYTE	btHitType;
	BYTE	ignoreTargetDis; //忽视和目标的距离(例如：道士技能：虚空黑洞)
	BYTE	bBuffType;			//bufftype
	/*
	BYTE nResultType;   //效果的类型ID
	BYTE bVt;          //值的类型 (浮点等)
	WORD wMjID;			//秘笈的ID，如果是0表示是一个普通的技能效果
	BYTE bLevel;      //使用技能的等级（用于特殊技能）

	BYTE alwaysTakeEffect;   //就算施法者下线了也要生效
	BYTE bRateType;    //属性比例是相对于哪个数值,0表示物理攻击，1表示法术攻击，2表示道术攻击，。。。后面可以扩展
	BYTE timeAddType;  //时间是基于哪个属性的比例提高,0表示物理攻击，1表示魔法攻击，2表示道术攻击，3表示等级，其他可以增加
	BYTE intervalAddType; //buff的时间间隔是基于哪个属性的比例提高,0表示物理攻击，1表示魔法攻击，2表示道术攻击，3表示等级，其他可以增加
	int  nId;
	int  nInterval; 
	int  nDelay;        //延迟，单位是毫秒
	int  nTimes;        //作用的次数
	int  nValue;        //值
	int   valueRate;   //值的万分比
	int   timeRate;  //时间的万分比
	int   intervalRate;  // buff的时间的万分比
	WORD   maxTime;     //buff最大的作用次数
	WORD   maxDur;   //buff最大的作用间隔
	int   nEffectId; //特效的ID
	int   nEffectType; //特效的类型
	int	  nAiId;		//召唤宝宝的aiId
	int   nColorPriority;  //添加buff的时候的优先级，从1开始，优先级高的将顶替优先级低的
	int   nColor;          //添加buff的时候给添加的颜色，主要是道士的毒，麻痹的毒
	BYTE	ignoreTargetDis; //忽视和目标的距离，用于野蛮冲撞这样的技能，撞开的时候依然有效
	BYTE	btHitType;		//伤害类型，用于技能命中伤害时判定 1:火墙 
	byte	nParam;
	*/
	tagSkillResult()
	{
		memset(this,0,sizeof(*this) );
	}

}SKILLRESULT,*PSKILLRESULT;


//技能的等级信息
typedef struct tagSkillAction
{
	int nTargetType ;     //目标的类型
	DataList<SKILLRESULT> skillResults; //技能释放结果
	DataList<SPECIALEFFECT> specialEffect; //升级的特效
	DataList<TARGETSELCONDITION> targetConditions; //目标选择的条件
}SKILLACTION ,*PSKILLACTION;


//在一个特定范围里的技能的效果
typedef struct tagSkillOneRange
{
	int nStartX;  //x左边的点
	int nEndX;    //x右边的点
	int nStartY;  //y上面的点
	int nEndY;    //y下面的点
	int rangeType; // 范围的类型
	int rangeCenter; //范围的中心
	int nDelay;		//延迟目标选择
	int nForceHitTargetDis;//非指向的群体技能可以强制打中已选目标的距离（主要是处理跑动的目标打不中的问题)
	DataList<SKILLRESULT> skillResults; //技能释放结果
	DataList<TARGETSELCONDITION> targetConditions; //目标选择的条件
	//DataList<SKILLACTION> actions; //技能在这个范围内的效果
}SKILLONERANGE,*PSKILLONERANGE;


typedef struct tagSkillOneLevel
{
	unsigned int nLevel;
	//DataList<SKILLONERANGE> ranges; //技能在每一个范围里的效果
	DataList<PSKILLONERANGE> pranges; //技能在每一个范围里的效果
	DataList<SKILLTRAINSPELLCONDITION> trainConditions; //训练条件
	DataList<SKILLTRAINSPELLCONDITION> spellConditions;//技能的使用需求
	unsigned int nSingTime;			//吟唱时间
	unsigned int nCooldownTimes;	//CD时间
	int nAttrIdx;
	int nHitId;
	int nMaxPet; 
	bool isSceneEffect = false;
}SKILLONELEVEL,*PSKILLONELEVEL;



/*一个技能的数据*/
struct OneSkillData
{
	char  sSkillName[60]; //技能名字
	WORD  nSkillID; //技能ID
	BYTE  nSkillType; //技能的类型，近身攻击之类
	BYTE  bSpecialBuffCond; //技能在封断、晕眩的时候是否能够使用
	BYTE  bSkillClass; //技能的分类0职业被动技能,1职业主动技能，2阵营技能,3结拜技能,4侠侣技能，5职业绝学，6江湖绝学，7通用技能,8其他技能
	BYTE  bIsSwitch; //是否手动开关
	BYTE  bPriority;  //技能的优先级
	bool  bIsAutoLearn;//是否自动学
	BYTE  bVocation;  //职业 
	bool  bBeedCalPassiveProperty; //是否需要计算被动属性，某些主动技能也需要按被动技能计算属性
	bool  bSingleEmptyEffect;
	int   nCommonCdTime; //技能使用的时候公共cd时间
	int nAutoLearnSkillLvl; //自动学习技能的等级
	DataList<SKILLONELEVEL> levels ;  // 技能的等级
	BYTE btClientDir;	//使用客户端方向
	WORD  wAttrId;	//对应的属性id
	bool boIsDelete; //是否删除
};

