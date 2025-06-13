#pragma once

using namespace wylib::container;

//使用技能的初始化的函数
enum MonsterSkillEvent
{
	mSkillEventCommon=0 ,  //在普通的战斗中使用
	mSkillEventDeath=1,   //在死亡的时候 调用
	mSkillEventBorn =2,   //创建的时候调用
	mSkillEventIdle = 3,	//空闲的时候释放，主要用于一些拉怪的
	mSkilleventBeAttacked = 4,	//被攻击调用
	mSkilleventAlive = 5,		//复活被调用（其实没有死亡）
	mSkilleventkill = 6,  //杀死人掉用
};

// 怪物发言配置
typedef struct tagMonsterTalkConfig
{
	unsigned int nIntervalLower;			// 最小发言间隔
	unsigned int nIntervalUpper;			// 最大发言间隔
	unsigned int nTalkRate;					// 发言概率。百分比
}MonsterTalkConfig;


//AI的配置
typedef struct tagAiConfig
{
	int nId;
	BYTE nAIType;				// AI类型： 主动、被动
	BYTE patrolRadius; 			//巡逻半径
	WORD pursuitDistance; 		//追击半径

	BYTE watchDistance; 			//可视范围
	BYTE returnHomeRenewRate; 		//怪物回归的时候，将按什么比例恢复自己的HP,MP,这里填百分数，100就是100%,0就是0%
	BYTE  bBattleFollowDis; 		//宠物离开主人超过这个距离，将向玩家移动
	BYTE  bTransferDis;				//宠物离开主人的距离，超过这个将自动传送到主人的身边

	int leftHomeMaxDistanceSquare; 		//离开巢位的最大的距离平方，一旦离开出生点超过这个距离就回归
	int nCheckEnemyIntervalInCombat; 	// 战斗中检测敌人间隔时间
	int nStopMinTime;					// 停留最短时间
	int nStopMaxTime;					// 停留最长时间
	int nMoveStopInterval;         		//移动之间的停留，单位ms
	DataList<int> pathPoints;			//怪物寻路径点

	int petFollowMasterTime;       //宠物离开战斗后多少时间以后将往主人方向走，单位ms
	int hitSetTargetRate;      //被攻击的时候，设置攻击自己的实体为目标概率，0-100有效,0表示不切换目标，100表示必定切换目标

	MonsterTalkConfig	m_nonCombatTalkConfig;	// 未进入战斗发言配置
	MonsterTalkConfig	m_combatTalkConfig;		// 进入战斗配置
}AICONFIG,*PAICONFIG;
