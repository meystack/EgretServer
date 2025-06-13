#pragma once
/****************************************************************************************/
/* 技能DB相关的一些结构体的定义                                               
/****************************************************************************************/
#pragma  pack(4)
/// 玩家的一个技能需要存盘的数据
//#define MAX_LEARNED_SKILL_COUNT 64 //一个玩家最多学习的技能个数
typedef struct tagOneSkillDbData
{
	WORD			nSkillID;			///< 技能的ID
	unsigned char	nLevel;				///< 技能的等级,如果没有学习就是0
	unsigned char	nIsClosed;			///< 技能是否被关闭着，如果1表示被关闭着，0表示正常
	unsigned int	nCd ;				///< 冷却时间
	unsigned int	nExp;				///< 技能的经验
	unsigned int	nMijiExpiredTime;	///< 秘籍的过期时间
	int				nMijiId;			///< 镶嵌的秘籍
}ONESKILLDBDATA, *PONESKILLDBDATA;


#pragma  pack()