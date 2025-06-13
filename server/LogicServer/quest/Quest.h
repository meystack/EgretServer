#pragma once
//tolua_begin
/*****************************************
*	定义每个任务的数据
******************************************/
#pragma pack (push, 1)



class CQuestData
{
public:

	enum enQuestState
	{
		qsAuto = -1,
		qsCustom = 0,
	};


	//定义返回给客户端的错误码
	enum QuestErrorCode
	{
		qecSucc = 0,	//成功
		qecNoQuest = 1,//用户没有指定的任务在身
		qecCanNotFinish=2,//用户还没达到完成任务的条件
		qecGetAward=3,	//领取不到奖励，通常是由于背包不够大
		qecGetAwardPara = 4,	//领取的奖励不合符要求，比如本职业不能领取本物品
		qecNotGiveUp=5,			//本任务不能放弃
		qecNoQuestItem =6,	//没有指定的正在进行的任务的类型
		qecDoing	=7,		//正在做这个任务，不能新增
		qecNewCond = 8,		//没达到接受任务的条件
		qecMainFinish = 9,	//主线任务已做了一次，不能再接
		qecRepeatMax = 10,	//已经超过了每日可做的最大次数
		qecInterval = 11,	//周期任务只能做一次
		qecParent = 12,		//父任务没完成
		qecExcludeTree = 13,	//子任务正在进行中，不能接这个任务
		qecSkill = 14,			//不能学习这个技能
		qecNotAutoCompleteQuest = 15,	//不是一个自动完成的任务
		qecQuestFull = 16,		//任务已满
		qecErr = 127,	//内部错误，不用显示给用户看
	};

	enum QuestTarget
	{
		//1 杀怪类、2 穿戴某种等级的装备 消耗类、3 升级、4 回收
		qtMonster = 1,
		qtEquip = 2,
		qtLevel = 3,
		qtRecover = 4,
		qtcircle = 5, //转生
		qtSkillLv = 6, //技能
		qtFuben = 7, //副本
		qtBless = 8, //祝福星级
		qtCompose = 9, //合成
		qtEquipItem = 10, //装备组
		qtGetItem = 11, //获得装备
	};

	//任务激活的条件

	enum QuestCondition
	{
		qcLevel = 0,//0、角色等级
		qcGuildLevel = 1,//1、帮派等级
		qcJob = 2,	//2、职业分类
		qcMenPai = 3,//3、门派分类
		qcSceneId = 4,	//4、所在场景
		qcKill = 5,	//5、杀戮值
		//qcZhanHun = 6,	//6、战魂值
		qcPreQuest = 7,	//7、前置任务
		qcItem = 8,	//8、身上携带物品
		qcItemCount = 9,//9、物品数量
		qcUserItem = 10,			//10、对应道具ID,输入道具ID，玩家双击该道具后添加任务(zac:这种是通过物品获得任务）
		qcMulMp = 11,			//11、多个门派可以接的任务
		qcMulParent = 12,		//12、支持两个前置任务
		qcSex = 13,				//13、性别
		qcPreDoing = 14,		//有一个正在做，但没完成的任务时，才能接
		qcCircle = 15,			//转生达到条件
		qcLevelSection = 16,	//等级区间
		qcCircleSection = 17,	//转生等级区间
		qcOpenServerDay = 18,	//开服天数限制
		qcMeritorious = 19,		//功勋达到多少	
		qcAchieveComplete = 20, //完成某个成就
		qcOpenDay = 21, //开服第几天
		qcMaxConition,		//配置文件的值不能超过这个

	};
	
};
#pragma pack(pop)
//tolua_end