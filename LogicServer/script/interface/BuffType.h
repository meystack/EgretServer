#pragma once
/************************************************************************/
/*
/*                             游戏属性类型定义文件
/*
/*    游戏属性指游戏中物品、技能等能够作用产生的属性。属性由属性的类型和属性的值构成。属
/*  性类型表示属性作用的具体意义，例如物理攻击、体力上限、命中等；而属性值则表示此属性作
/*  用意义的量值，物理攻击+1，物理攻击+2是两个不同的属性，其属性的类型相同，但值不同。
/*
/************************************************************************/

/**
/* 定义游戏中的属性类型
/* ★★注意，此枚举中的成员不能删除和插入成员，增加新属性请添加到末尾，否则会造成灾难性的后果
/*  ▲对于Add类型的属性，支持使用负数作为减少属性值；
/*  ▲对于Power类型的属性，0.1表示增加10%，-0.2表示减少20%；
*********************************/

typedef enum tagGameAttributeType
{

	aUndefined						= 0,	//未定义属性
	aHpAdd							= 1,	//血增加
	aHpPower						= 2,	//血倍率增加
	aMpAdd							= 3,	//蓝增加
	aMpPower						= 4,	//蓝倍率增加

	Level2PropertyStart				= 5,    //***********二级属性开始************
	aMaxHpAdd	  = Level2PropertyStart,	//最大血增加
	aMaxHpPower						= 6,	//最大血倍率增加
	aMaxMpAdd						= 7,	//最大蓝增加
	aMaxMpPower						= 8,	//最大蓝倍率增加
	aPhysicalAttackMinAdd			= 9,	//最小物理攻击增加
	aPhysicalAttackMinPower			= 10,	//最小物理攻击倍率增加
	aPhysicalAttackMaxAdd			= 11,	//最大物理攻击增加
	aPhysicalAttackMaxPower			= 12,	//最大物理攻击倍率增加
	aMagicAttackMinAdd				= 13,	//最小魔法攻击增加
	aMagicAttackMinPower			= 14,	//最小魔法攻击倍率增加
	aMagicAttackMaxAdd				= 15,	//最大魔法攻击增加
	aMagicAttackMaxPower			= 16,	//最大魔法攻击倍率增加
	aWizardAttackMinAdd				= 17,	//最小道术攻击增加
	aWizardAttackMinPower			= 18,	//最小道术攻击倍率增加
	aWizardAttackMaxAdd				= 19,	//最大道术攻击增加
	aWizardAttackMaxPower			= 20,	//最大道术攻击倍率增加
	aPhysicalDefenceMinAdd			= 21,	//最小物理防御增加
	aPhysicalDefenceMinPower		= 22,	//最小物理防御倍率增加
	aPhysicalDefenceMaxAdd			= 23,	//最大物理防御增加
	aPhysicalDefenceMaxPower		= 24,	//最大物理防御倍率增加
	aMagicDefenceMinAdd				= 25,	//最小魔法防御增加
	aMagicDefenceMinPower			= 26,	//最小魔法防御倍率增加
	aMagicDefenceMaxAdd				= 27,	//最大魔法防御增加
	aMagicDefenceMaxPower			= 28,	//最大魔法防御倍率增加
	aHitValueAdd					= 29,	//物理命中增加(准确)
	aHitValuePower					= 30,	//物理命中倍率增加(准确)
	aDogValueAdd					= 31,	//物理闪避增加(敏捷)
	aDogValuePower					= 32,	//物理闪避倍率增加(敏捷)
	aMagicHitRateAdd				= 33,	//魔法命中增加
	aMagicHitRatePower				= 34,	//魔法命中倍率增加
	aMagicDogerateAdd				= 35,	//魔法闪避增加
	aMagicDogeratePower				= 36,	//魔法闪避倍率增加
	aHpRenewRateAdd					= 37,	//生命恢复百分比增加
	aHpRenewRatePower				= 38,	//生命恢复百分比倍率增加
	aMpRenewRateAdd					= 39,	//魔法恢复百分比增加
	aMpRenewRatePower				= 40,	//魔法恢复百分比倍率增加
	aMoveSpeedAdd					= 41,	//移动速度增加
	aMoveSpeedPower					= 42,	//移动速度倍率增加
	aAttackSpeedAdd					= 43,	//攻击速度增加
	aAttackSpeedPower				= 44,	//攻击速度倍率增加
	aLuckAdd						= 45,	//幸运增加
	aLuckPower						= 46,	//幸运倍率增加
	aHpRenewAdd						= 47,	//生命恢复值增加
	aHpRenewPower					= 48,	//生命恢复值倍率增加
	aMpRenewAdd						= 49,	//魔法恢复值增加
	aMpRenewPower					= 50,	//魔法恢复值倍率增加
	a51								= 51,	//
	a52								= 52,	//
	Level2PropertyEnd				= 53,	//*************二级属性结束***************

	GamePropertyCount				= 53,	//实体的属性个数,下面的这些不需要进行属性的计算

	aDamageAbsorb    = GamePropertyCount,   //[BUFF] 伤害抵消的BUFF状态 ( value:池值,  param:吸收万分比 )
	aDamageAbsorbRate				= 54,	//[NORMAL] 伤害减免,万分比
	aDizzy							= 55,	//[BUFF] 麻痹，不可移动，不可释放技能 ( 无参 )
	aExpAdd							= 56,   //[NORMAL] 经验增加一个数值 ( 物品<value:经验> )
	aHpCure							= 57,	//[BUFF] 治愈的BUFF状态  ( value:池值,  param1:每次回复量 )
	aHide							= 58,   //[BUFF] 隐身状态 ( 无参 )
	aExpPower						= 59,   //[NORMAL] 经验的增加倍率，万分比
	aFreeze							= 60,	//[BUFF] 冻结，只能走，不能跑
	aPkProtectState					= 61,   //[BUFF] pk保护状态
	aRealFreeze						= 62,	//[BUFF] 真·冻结，不能走不能跑
	aMpCure							= 63,	//[BUFF] 持续回蓝的BUFF状态  ( value:池值,  param1:每次回复量 )
	aMpReplaceHpRate				= 64,   //[NORMAL] 按比例将MP抵消HP(1:1)，万分比
	aDizzyRateAdd					= 65,   //[BUFF] 麻痹概率，万分比
	aDeductDizzyRate			    = 66,   //[BUFF] 抗麻痹概率，万分比
	aDizzyTimeAdd					= 67,   //[BUFF] 麻痹时长增加
	aDizzyTimeAbsorbAdd				= 68,   //[BUFF] 麻痹时长减免
	BattlePropertyCount				= 69,	//临时设定 属性个数 可更改
	aGuardRate				  		= BattlePropertyCount,	//[BUFF] 守护概率，万分比
	aCuttingRate					= 70,	//[BUFF] 切割概率，万分比
	aaGuardValue					= 71,	//守护成功减免血量
	aNextSkillFlag					= 72,	//[战士技能标记] 战士延次技能的标志（标记战士的下一次附加技能：攻杀，刺杀，半月，烈火，逐日）
	aDieRefreshHpPro				= 73,   //死亡以后立刻回复的HP的比例,1点表示百分之1(复活戒指使用)
	aNewPlayerProtect				= 74,	//[BUFF] 新手保护
	aDoubleAtkRate     			    = 75,   //伤害加成 万分比 ---倍攻
	aAllAttackMinAdd				= 76,	//最小全属性攻击增加
	aAllAttackMaxAdd				= 77,	//最大全属性攻击增加
	aForbidHpRenew					= 78,	//[BUFF] 禁止回血的BUFF状态
	aSuckBloodRate					= 79,	//吸血率 万分比
	aAllAttackAddRate			    = 80,	//攻魔道属性增加 万分比
	aDoubleDefenseRate				= 81,   //双防加成 万分比
	aIgnorDefenceRatio				= 82,  //无视防御比例 万分比
	aNormalAttackAcc				= 83,	//[NORMAL] 普攻加速度，万分比
	aPkDamageAbsorbRate				= 84,	//pk伤害减免
	aSaviorTime						= 85,	//救主灵刃-CD时间
	aSaviorRate						= 86,	//救主灵刃-回复率
	aSaviorValue					= 87,	//救主灵刃-回复值
	aToxicRate						= 88,	//剧毒裁决-淬毒几率
	aToxicDamage					= 89,	//剧毒裁决-淬毒伤害
	aToxicEffect					= 90,	//剧毒裁决-效果
	aFrozenStrength					= 91,	//霜之哀伤-冰冻强度
	aFireDefenseRate				= 92,	//抗火率 使用1点表示1万分之1
	aReduceEquipDropRate			= 93,	//减少玩家死亡爆装备几率 使用1点表示1万分之1
	aDamageDropTime					= 94,	//收到伤害以后，状态时间减少，当buff删除的时候使用技能*************buffSystem使用**********
	aWarriorDamageValueDec			= 95,	//[NORMAL] 固定值降低战士的伤害
	aWarriorDamageRateDec			= 96,	//[NORMAL] 百分比降低战士的伤害
	aMagicianDamageValueDesc		= 97,	//[NORMAL] 固定值降低法师的伤害
	aMagicianDamageRateDesc			= 98,	//[NORMAL] 百分比降低法师的伤害
	aWizardDamageValueDesc			= 99,	//[NORMAL] 固定值降低道士的伤害
	aWizardDamageRateDesc			= 100,	//[NORMAL] 百分比降低法师的伤害
	aMonsterDamageValueDesc			= 101,  //[NORMAL] 固定值降低怪物的伤害
	aMonsterDamageRateDesc			= 102,	//[NORMAL] 百分比降低怪物的伤害
	aDamageReduceRate				= 103,	//[NORMAL] 触发伤害减免的几率
	aDamageReduceValue				= 104,	//[NORMAL] 触发伤害减免的值
	aDamageAddRate					= 105,	//[NORMAL] 触发伤害追加的几率
	aDamageAddValue					= 106,	//[NORMAL] 触发伤害追加的值
	aIgnorDefenceRateSkill			= 107,	//[NORMAL] 技能触发无视防御几率
	aIgnorDefenceRate				= 108,  //[NORMAL] 触发无视防御几率
	a109							= 109,
	aWarriorTargetDamageValue		= 110,  //[NORMAL] 固定值增加对战士的伤害
	aWarriorTargetDamageRate		= 111,  //[NORMAL] 百分比增加对战士的伤害
	aMagicianTargetDamageValue		= 112,  //[NORMAL] 固定值增加对法师的伤害
	aMagicianTargetDamageRate		= 113,	//[NORMAL] 百分比增加对法师的伤害
	aWizardTargetDamageValue		= 114,  //[NORMAL] 固定值增加对道士的伤害
	aWizardTargetDamageRate			= 115,	//[NORMAL] 百分比增加对法师的伤害
	aMonsterTargetDamageValue		= 116,  //[NORMAL] 固定值增加对怪物的伤害
	aMonsterTargetDamageRate		= 117,	//[NORMAL] 百分比增加对怪物的伤害
	aHPMPDeductPower				= 118,	//吃药效果减百分率
	aAddAllDamageRate				= 119,	//攻击时候附加攻击输出的概率,整数配置，1点表示万分之1(暴击几率)
	aAddAllDamagePower				= 120,	//攻击时附加攻击输出的倍率增加（暴击伤害）
	aAddDamageReboundRate			= 121,  //伤害反弹 概率,整数配置，1点表示万分之1
	aAddDamageReboundPower			= 122,	//伤害反弹 倍率
	aAddAllDamageRateEx				= 123,  //会心一击附加攻击输出的概率,整数配置，1点表示万分之1
	aChangeMonsterModle				= 124,  //新的变身属性*************buffSystem使用**********
	aFixed						    = 125,	//定身,不可移动，可释放技能*************buffSystem使用**********
	aSelfAddBuffRate				= 126,  //造成伤害时，自己获得buff的几率（万分比）
	aSelfAddBuffId					= 127,	//造成伤害时，自己获得buff的Id
	aTargetAddBuffRate				= 128,	//造成伤害时, 目标获得buff的几率（万分比）
	aTargetAddBuffId				= 129,	//造成伤害时, 目标获得buff的Id
	aBuffChangeModel				= 130,  //变身状态，bufff改变模型Id ,value 男模型 param 女模型 param2男武器，param3女武器
	a131							= 131,
	aSlient							= 132,  //沉默(不能使用技能)*************buffSystem使用**********
	a133							= 133,	//
	aExpAddRate						= 134,	//根据库加经验
	a135							= 135,
	aCritDamagePower				= 136,  //[NORMAL] 暴击倍率万分比
	aHpMpAdd						= 137,	//回血回蓝(buff,value=总量,param1=回血值，param2=回蓝值)(adUInt)
	aCritDamageRate					= 138,	//[NORMAL] 暴击几率（新）区别于119，(万分比)
	aCritDamageValue				= 139,	//[NORMAL] 暴击力（新）区别于120,原来伤害基础上最后加上aCritDamagePower的伤害
	aDeductCriteDamageRate			= 140,  //[NORMAL] 抗暴（抵抗138的几率）(万分比)
	aFastMedicamentRenew            = 141,  //速回药品回复率
	aBuffExpAdd						= 142,  //增加经验buff
	aBuffHeroExpRate				= 143,	//英雄增加经验倍率buff
	aBuffmtBindCoinAddRate		 	= 144,  //拾取金币加成buff
	aHalfMonthsIncreaseDamage		= 145,	//半月增伤
	aFireIncreaseDamage				= 146,	//烈火增伤
	aDayByDayIncreaseDamage			= 147,	//逐日增伤
	aIceStormIncreaseDamage			= 148,	//冰咆哮增伤
	aFireRainIncreaseDamage			= 149,	//火雨增伤
	aThunderIncreaseDamage			= 150,	//雷电增伤
	aBloodBiteIncreaseDamage		= 151,	//噬血术增伤
	aFireSignIncreaseDamage			= 152,	//火符增伤
	aHalfMonthsReduceDamage			= 153,	//半月减伤
	aFireReduceDamage				= 154,	//烈火减伤
	aDayByDayReduceDamage			= 155,	//逐日减伤
	aIceStormReduceDamage			= 156,	//冰咆哮减伤
	aFireRainReduceDamage			= 157,	//火雨减伤
	aThunderReduceDamage			= 158,	//雷电减伤
	aBloodBiteReduceDamage			= 159,	//噬血术减伤
	aFireSignReduceDamage			= 160,	//火符减伤
	//★添加新的属性类型枚举成员请在GameAttributeCount前添加！★ 
	//并修改AttrDef.cpp中的枚举
	GameAttributeCount,	//此枚举成员的值将会自动取值为已定义属性的数量
	
}GAMEATTRTYPE;
