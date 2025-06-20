﻿// #pragma once

// class CNpc;

// static const int QUESTBTYE = 1000;//这个尽量不要修改，如果修改的话，对应的数据服务器部分也要修改
static const int MAXQUESTBYTE = 8 * QUESTBTYE;

// #pragma pack (push, 1)
// class QuestConfig
// {
// public:
// 	typedef struct tagNpc
// 	{
// 		int nType;	//任务的接受和提交类型，0表示从NPC上接任务；1表示满足接受条件时自动接受；2表示由脚本系统动态发布 
// 		int nSceneId;
// 		int nPosX;
// 		int nPosY;
// 		char sSceneName[128];
// 		char sNpcName[128];
// 	};

// 	//任务目标位置
// 	typedef struct tagQuestTargetLocation
// 	{
// 		int nSceneid;
// 		int nPosX;
// 		int nPosY;
// 	}QUESTTARGETLOCATION,*PQUESTTARGETLOCATION;

// 	//任务目标
// 	typedef struct tagQuestTarget
// 	{
// 		int			nType;	//类型
// 		int			nId;
// 		int			nCount;
// 		int			nRewardIdx;		//关联任务奖励索引
// 		QUESTTARGETLOCATION location;
// 		//char*		sData;//自定义类型
// 	}QUESTTARGET,*PQUESTTARGET;

// 	//任务完成列表
// 	typedef struct tagQuestTargetList
// 	{
// 		int nCount;
// 		QUESTTARGET* pTargetList;
// 	}QUESTTARGETLIST,*PQUESTTARGETLIST;


// 	//奖励配置表
// 	typedef struct	tagAward
// 	{
// 		int nType;
// 		int nId;
// 		int nCount;
		
// 		BYTE boBind;
// 		BYTE job;	//等于0表示不限制
// 		char sex;//小于0表示不限制
		
// 		//char*	sDesc;		//这个是自定义奖励才有
// 		BYTE nStrong;
// 		BYTE nQuality;
// 		float fLevelRate;
// 		float fRingRate;
// 		int nPetExpRate;

// 		/*
// 		* Comments:判断职业是否符合这个奖励的规定
// 		* Param int job:角色的职业
// 		* @Return BOOL:
// 		*/
// 		BOOL isJob(BYTE myjob) const
// 		{
// 			if (job == 0) return TRUE;//不限制职业
// 			else return myjob == job;
// 		}

// 		/*
// 		* Comments:判断性别是否符合这个奖励的规定
// 		* Param int mysex:角色的性别
// 		* @Return BOOL:
// 		*/
// 		BOOL isSex(int mysex) const
// 		{
// 			if (sex < 0) return TRUE;//不限制性别
// 			else return mysex == sex;
// 		}

// 	}AWARD,*PAWARD;

// 	//奖励物品列表
// 	typedef struct tagAwardList
// 	{
// 		int nCount;
// 		AWARD*	pAwardList;
// 	}AWARDLIST,*PAWARDLIST;

// 	//任务所需条件
// 	typedef struct	tagCondition
// 	{
// 		int nType;
// 		int nId;
// 		int	nCount;
// 	}CONDITION,*PCONDITION;

// 	// 任务条件列表
// 	typedef struct tagConditionList
// 	{
// 		int nCount;
// 		CONDITION*	pConditionList;
// 	}CONDITIONLIST,*PCONDITIONLIST;

// 	//typedef struct tagQuestDetail
// 	//{
// 	//	WORD nQid;
// 	//	QuestConfig*	pQuestConfig;
// 	//}QuestDetail;

// 	//任务ID列表
// 	typedef struct tagQuestIDList
// 	{
// 		int nCount;
// 		QuestConfig**	pQuest;	//子任务id
// 	}QUESTIDLIST,*PQUESTIDLIST;


// public:
// 	WORD    nQid;		// 任务Id
// 	char	sName[128];	// 任务名称
// 	int		nParentId;	//父任务ID，如果值为-1则表示没有父任务，否则必须在做完父任务后才能接此任务 
// 	int		nType;		//0是主线任务，1是支线任务，2是环线任务，等等
// 	int		nLevel;		// 达到一定等级才让接任务
// 	int	    nCircle;	//环任务当前环数
// 	WORD	nEntrust;	//委托
// 	WORD	nSpeedYb;	//快速完成
// 	BYTE	bStar;		//刷星最高等级
// 	BYTE	bMultiAward;//多倍奖励
// 	BOOL	boCanGiveUp;	//可否放弃
// 	BOOL    boShowTimerBox; //任务传送
// 	BOOL  boTelportScript;//主线任务传送
// 	bool	boDoubleYB;		//双倍是否只用元宝支付
// 	bool	boNewDayDelete;
// 	char	sContent[500];
// 	char    sPropTalk[500];
// 	char    sCompTalk[500];
// 	char    sCompTip[500];
// 	char    sPassTip[500];
// 	int		nStartNpc;	//接任务的npc
// 	int		nEndNpc;		//结束（完成任务）npc
// 	bool	boRandomTarget;		//随机目标
// 	QUESTTARGETLIST	Target;
// 	DataList<AWARDLIST> Award;
// 	CONDITIONLIST		Condition;

// 	int nTimeLimit;//任务时间限制，单位是秒钟，从接任务后开始计时，必须在时间归零前完成任务，否则任务自动注销，0表示不限制时间； 
// 	int nInterval;		//任务周期，单位是秒。0表示不限制。任务周期表示从当天0点开始，进行任务后的多长时间后才能再次接任务。
// 	int nMaxCount;		//每个任务周期内，可以做多少次，0表示不限制； 
// 	bool boExcludeTree;		//如果接了此任务的子任务，则不能再接此任务； 
// 	QUESTIDLIST	m_vSubQuest;	//保存这个任务的子任务列表，如果有的话
// 	bool boAutoMount;		// 是否接任务后自动上坐骑
// 	CNpc*	pEndNpc;		//完成任务需要交接的NPC，主要用于放弃任务时候，需要执行这个npc的脚本
// public:
// 	inline BOOL IsRootQuest() {return (this->nParentId <= 0);}
// 	inline BOOL IsMainQuest() {return nQid < MAXQUESTBYTE;}
// 	//这个判断 不是主任务（只能做一次），且又nInterval设置了0的，这种也是只能做一次的
// 	inline BOOL IsInterval() { return !IsMainQuest() && nInterval == 0;}
// };
// #pragma pack(pop)