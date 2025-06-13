#pragma once
//成就系统中用到的各种事件

//tolua_begin
enum  tagAchieveAtomEventID
{
	eAchieveEventIdNone				= 0,        //占位的
	// eAchieveEventLogin				= 1,        //登陆游戏			参数1：是否使用登陆器,参数2：登陆的天数，参数3：流失的天数，参数4：提取元宝的数目，参数5：合服登陆的天数
	// eAchieveEventLevelUp			= 2,        //等级提升			参数1：等级数
	// eAchieveEventFinishQuest		= 3,        //完成任务			参数1：任务id
	// eAchieveEventJoinGuild			= 4,       //加入帮派 
	// eAchieveEventEnterFb			= 5,       //进入副本			参数1：副本id
	// eAchieveEventEquipStrong		= 6,		//装备强化 
	// eAchieveEventEquipEnlay			= 7,		//嵌入宝石			参数1：魂石的等级 参数2：此等级魂石的数量
	// eAchieveEventShopBuy			= 8,		//商城购买物品		参数1：物品id 参数2：物品数量
	// eAchieveEventCoinChange			= 9,		//银两				参数1：银两数量
	// eAchieveBindCoinChange			= 10,		//绑定银两			参数1：绑定银两数量
	// aAchieveEventEnterScene			= 11,		//进入场景			参数1：场景id
	// aAchieveEventConsumerYuanbao	= 12,		//消费元宝			参数1：数量
	// aAchieveDrawYuBao				= 13,		//提取元宝			参数1：元宝数量
	// aAchiveCircleTimes				= 14,		//转生				参数1：转生的次数
	// aAchiveCheckTimes				= 15,		//签到				参数1：签到的次数
	// aAchivePassFuben				= 16,		//通过副本			参数1：副本id
	// aAchiveBattle					= 18,		//战斗力			参数1：战斗力值
	// aAchiveSignInTimes				= 19,		//签到				参数1：签到的次数

	nAchieveLevel          = 1, // 等级   
    nAchieveCircle         = 2, //转生
    nAchieveJmLevl         = 3, //经脉
    nAchieveBless          = 4, //祝福
    nAchieveActorNum       = 5, //数值资源
    nAchieveRecharge       = 6, //充值
    nAchieveKillMonster    = 7, //杀怪
    nAchieveFuBen          = 8, //通关副本 
    nAchieveGuildDonate    = 9, //行会捐献
    nAchieveRecover        = 10, //回收
    nAchieveForge          = 11, //金刚石锻造
    nAchieveLogin          = 12, //登录
    nAchieveContinueLogin  = 13, //连续登陆
    nAchieveEquipment      = 14, //装备
	nAchieveCompleteAchieve= 15, //完成其他成就
	nAchieveGetDonateValue = 16, //累计获得行会贡献
	nAchieveActivity       = 17, //累计参加活动
	nAchieveCost           = 18, //累计消耗
	nAchieveStrengthenLv   = 19, //强化
	nAchieveSoulJadeLv     = 20, //魂玉等级
	nAchieveCostItem       = 21, //消耗道具	21
	nAchievekillOneMonster = 22, //击杀某种怪物	22
	nAchieveKillSZBoss     = 23, //挑战神装boss	23
	nAchieveGetSZBossAward = 24, //获得神装boss奖励
	nAchieveJionSBKGuild   = 25, //成为沙巴克成员
	nAchieveJoinJDFuben    = 26, //参与禁地副本次数
	nAchieveCompleteActivity  = 27, //累计完成活动
	nAchieveRechargeDay  = 28, //累计充值天数
	nAchieveDropItem   = 29, //掉落成就
	nAchieveZL   = 30, //此项仅做标记,实际战令任务的捐献不走成就类型,注意不同任务捐献相同道具的情况下全部单独计算
	nAchieveRealRecharge   = 31, //充值(返利卡充值不计算)   
	eMaxAtomEventID,      //最大的原子事件的ID   
};
//tolua_end