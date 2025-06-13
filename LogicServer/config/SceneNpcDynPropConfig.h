/************************************************************************/
/* 文件名: SceneNpcDynPropConfig.h										*/	    
/* 描  述：定义副本NPC动态属性变化倍率配置数据结构						*/
/* 备  注：目前针对HP、外攻、内攻、外防、内防五个属性进行动态变化		*/
/* 按照等级（base 1）进行配置，数量不限。								*/
/************************************************************************/
#pragma once

#define NPC_DYN_PROP_COUNT		1
typedef enum tagNpcDynPropType	// NPC动态属性类型
{
	DPT_HPRate,					// HP
	DPT_OuterAttack,			// 外攻
	DPT_InnerAttack,			// 内攻
	DPT_OuterDefence,			// 外防
	DPT_InnerDefence			// 内防
}NpcDynPropType;

typedef enum tagNpcDynPropCalcTypeID	// NPC动态属性计算器使用的属性类型
{
	DPCT_HPRate = aMaxHpPower,						// HP
	//DPCT_OuterAttack = aOutAttackPower,				// 外攻
	//DPCT_InnerAttack = aInnerAttackPower,			// 内攻
	//DPCT_OuterDefence = aOutDefencePower,			// 外防
	//DPCT_InnerDefence = aInnerDefencePower			// 内防
};

// 等级差对怪物属性的影响系数
typedef struct tagNpcDynPropRateList
{
	float fRate[NPC_DYN_PROP_COUNT];
}NpcDynPropListRate;

// 队伍人数对怪物属性的影响系数
typedef struct tagTeamNumAdjustRate
{
	NpcDynPropListRate teamAdjustRateInfo[MAX_TEAM_MEMBER_COUNT];
}TeamNumAdjustRate;

typedef struct tagSceneNpcDynPropConfig
{
	CVector<NpcDynPropListRate> vecDynPropRate;			// 等级差影响系数
	NpcDynPropListRate teamNumAdjustRateInfo[MAX_TEAM_MEMBER_COUNT];// 队伍人数影响系数	
}SceneNpcDynPropConfig, *pSceneNpcDynPropConfig;

