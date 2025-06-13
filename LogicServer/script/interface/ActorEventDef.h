#pragma once

/*********************************
*
* 本文件主要用于定于角色相关的事件，如升级、学习了某个技能，获得物品等
*
**********************************/


//tolua_begin
enum tagActorEventID
{
	aeNoEvent			= 0,	//未定义事件
	aeLevel				= 1,	//角色升级	参数1：当前等级数
	aeUserLogin			= 2,    //玩家登陆  参数1：是否第一次登录
	aeUserLogout		= 3,    //玩家登出  参数1：人物id
	aeOnActorDeath		= 4,	//人物死亡
	aeReliveTimeOut		= 5,	//判断复活时间是否超时（用户超过5分钟没有选择回城复活还是原地复活），如果到达，脚本处理，送回主城复活. 参数1：人物指针
	aeNewDayArrive		= 6,	//新的一天更新事件，玩家在线期间的0点整点更新事件
	aeOnActorBeKilled	= 7,	//被玩家杀死 参数1：杀人者指针
	aeWithDrawYuanBao	= 8,	//提取元宝 参数1：提取的元宝数量
	aeConsumeYb			= 9,	//消费元宝 参数1：消耗元宝数量
	aeOnEnterFuben		= 10,	//玩家进入副本[副本id]
	aeOnExitFuben		= 11,	//玩家退出副本[副本id]
	aeGuild				= 12,	//帮派相关
	aeEquipComposite    = 13,   //装备合成 参数1:目标装备ItemID
	aeBuffRemoved		= 14,	//buff被删除事件
	aeLeaveTeam			= 15,	//离开队伍
	aeCircle			= 16,	//角色转生	参数1：当前等级数
	aeChangeName		= 17,	//改名
	aeAsyncOpResult		= 18,	//异步操作结果 参数1：类型 参数2：结果 参数3：操作方式 参数4：错误码 参数5：卡号 参数6：增值类型 参数7：Sub类型
	aeHero              = 19,   //英雄系统
	aeHurtMonster		= 20,	//对怪物造成伤害

	
	aeMaxEventCount,
};



enum tagFreePkMode
{
	//和平模式
	fpPeaceful = 1,
	//团队模式
	fpTeam = 2,
	//帮派模式
	fpGuild = 3,
	//敌对模式
	fpEvil = 4,
	//全体模式
	fpPk = 5,
	//阵营模式
	fpZY = 6,

	//联盟模式
	fpUnion = 7,
	
	fpCount = 7,
};

//批量使用的类型
enum tagBathItemType
{
	batExp				= 1, //经验
	batBindCoin			= 2, //绑金（魔神货币）
	batCoin				= 3, //银两
	batActivityExp		= 4, //经验库加经验
	batCirclePower		= 5, //修为
	batGuildCoin		= 6, //行会银票
	batSkillExpItem		= 7, //技能熟练度物品
	batNormalItem		= 8, //普通道具批量使用(单个循环)
	bat9				= 9,
	batBloodSoulItem	= 11, //批量增加血魄碎片
	batMeritorious		= 12, //批量增加功勋抽奖
	batDragonSoulItem   = 13, //批量增加龙魂碎片
	bat15		= 15, //
	batBindGold			= 16, //批量增加绑定元宝
	batBossJifen		= 28, //批量增加boss积分
	bat_29				= 29,
	batMAx,
};

//奖励类型
enum tagAwardType 
{
	qatEquipment			= 0,	//物品或者装备 	id:物品ID count:物品数量 quality:物品品质 strong:强化等级 bind:绑定状态 param:物品指针  --- 物品提示
	qatMoney			    = 1,    //金币 					count:金币 -- UI提示
	qatBindMoney			= 2,	//绑金					count:绑金 -- UI提示
	qatBindYb				= 3,	//绑元 					count:银两 -- 元宝点券数值提示
	qatYuanbao				= 4,	//元宝 					count:元宝 -- 元宝点券数值提示
	qatExp					= 5,	//经验 					count:经验值 param:如果是任务，这个就填写任务的ID，其他的话填关键的有意义的参数，如果没有就填写0
	qatCircleSoul			= 6,	//转生修为			 	count:转生修为 -- 元宝点券数值提示
	qatFlyShoes 			= 7,	//飞鞋点数			 	count:飞鞋点数 -- 元宝点券数值提示
	qatBroat				= 8,	//喇叭点数			 	count:喇叭点数 -- 元宝点券数值提示
	qaIntegral				= 9,	//积分 					count:积分     -- 元宝点券数值提示
	qaGuildDonate			= 10,	//行会贡献 				count：行会贡献 -- 元宝点券数值提示
	qatPrestigeNum			= 11,   //声望					count: 声望数值 
	qatActivity				= 12,	//活跃度				count: 
	qatMultiExpUnused		= 13,  //剩余未使用多倍经验		 count:
	qatBasicExpNum			= 14,  //当前基础经验数值
	qatStatic			    = 15,  //计数器类型
	qatTitle			    = 16,  //称号
	qatWarPoint			    = 17,  //战令积分
	qatGhost			    = 18,  //神魔积分

	qatJyQuota			    = 19,	//交易额度 		count：交易额度 -- 元宝点券数值提示
	qatZLMoney			    = 20,	//战令币
	qatReviveDuration		= 22,	//复活特权
	qatLootPet	        	= 23,	//宠物
	qatCustomTitle	        = 24,	//自定义称号
	qatSecretBoxScore	    = 25,	//秘境宝箱数量(积分)
	qatWordsBoxScore	    = 26,	//字诀宝箱数量(积分)
	qatMaterialsBoxScore	= 27,	//材料宝箱数量(积分)

	qatSkill				= 110,	//技能 count:技能ID param:技能等级
	qatAchieveEvent			= 111,	//完成一个成就

	qatGuildFr				= 123,	//繁荣度 count:奖励繁荣度值
	qatXPVal				= 114,	// XP值	 count:XP值
	qatAddExp				= 115,	//按经验配置表加经验 id:奖励库ID count:普通加成率 quality:加成率 加成率使用以10000为基数的整形 即n/10000
	qatAnger				= 116,	//怒气
	qatBoss					= 117,	//boss
	qatUpgrade				= 118,	//玩家升级
	qatPetUpgrade			= 119,	//宝宝升级
	qatGuildCoin			= 120,	//行会资金
	qatPersonBossJifen      = 121,   //个人BOSS积分
	qat22               	 = 122,   //
	qat23		            = 123,   //
	qat24					= 124,   //
	qatMeritoriousCount		= 125,	//开宝箱次数
	qatBossJifen			= 126,	//boss积分
	qatDragonSoulValue	    = 128,	//龙魂碎片
	qatIntellectBallValue	= 129,   //智珠碎片
	qatMeritoriousValue		= 130,	//功勋点
	qatCombatScore			= 31,	//职业宗师竞技积分
	qatNightFighting		= 132,	//北奇夜战积分
	qatGuildContribution	= 133,	//行会贡献值 count:行会贡献值
	qatAwardTypeCount,				//MAX值
	qatCustomize			= 127,	//自定义奖励

};

//装备操作
enum tagEquipOP
{
	gEquipNoOP				= 0,	//占位
	gEquipStar				= 1,	//装备升星强化
	gEquipRecoverStar		= 2,	//装备损失星级恢复
	gEquipTransferStar		= 3,	//装备强化转移
	gEquipInscript			= 4,	//装备铭刻
	gEquipTransferInscript	= 5,	//装备铭刻转移
	gEquipComposite			= 6,	//装备合成
	gEquipIdentifySlotUnlock= 7,	//装备鉴定属性槽解锁
	gEquipIdentify			= 8,	//装备鉴定
	gEquipTransferIdentify	= 9,	//装备鉴定转移
};

//游戏商城相关操作
enum tagGameStoreOP
{
	gGameStoreNoOP				= 0,	//占位
	gGameStoreFreshMystical		= 1,	//刷新神秘商店
	gGameStoreGetMysticalData	= 2,	//请求神秘商店数据
};

//帮派的相关操作
enum tagGuildOP
{
	gGuildNoOP				= 0,	//占位
	gUpgradeGuild			= 1,	//行会升级
	gPutInUpgradeGuildItem	= 2,	//投入行会升级材料
	gGuildDonate			= 3,	//行会捐献
	gChargeGuildTree		= 4,	//行会神树充能
	gPickGuildFruit			= 5,	//行会神树果实摘取
	gGetGuildTaskAward		= 6,	//领取行会任务奖励
	gGetGuildBenefit		= 7,	//领取行会福利
	gSendGuildTaskBenefitData	= 8,	//下发行会任务福利数据
	gOpenGuildActivity		= 9,	//开启行会活动
	gGetCityPosWelfare		= 10,	//皇城职位福利领取
	gEnterGuildActivity		= 11,	//开启行会活动
	gGuildSiegeSetCmd		= 12,	//[攻城战]下发攻城战指令
	gGuildSiegeBackCity		= 13,	//[攻城战]返回圣灵城
	gGetCityPosWelfareInfo  = 14,  //沙城福利领取情况
};

//技能的相关操作
enum tagSkillOP
{
	gSkillNoOP				= 0,	//占位
	gAddSkillExp			= 1,	//技能升级
};

//好友相关
enum tagFriendOp
{
	gFriendNoOP				= 0,	//占位
	cSendCanAddFriendList	= 1,	//推荐好友列表
	cInitActorMood			= 2,	//初始化玩家心情（签名）
};
//兵魂
enum tagSoldierSoulOp
{
	OpSoldierSoulUnlock			= 1,	//解锁兵魂
	OpSoldierSoulLevelUpSkill	= 2,	//升级兵魂技能
	OpSoldierSoulActiveStar		= 3,	//激活兵魂星星
	OpSoldierSoulAttach			= 4,	//兵魂附身
	OpSoldierSoulEnterGame		= 5,	//进入游戏
};
//装备部位强化-操作
enum tagEquipPosStrongOp
{
	OpEquipPosStrongGetData	    = 1,//请求部位强化数据	
	OpEquipPosStrongUpgrade	    = 2,//提升
	OpEquipPosStrongUpgradeAll  = 3,//全部提升	
};

//通用活动定义（以区别不同的活动）（ZGame不使用）
enum tagCommonActiveIDDef
{
	CommActiveID_No				= 0,	//占位
	CommActiveID_GuildTree		= 10,	//活动-守护神树
	CommActiveID_Supplybattle	= 11,	//活动-补给争夺
};

//行会活动定义
enum tagGuildActiveDef
{
	GuildActive_No			= 0,	//占位
	GuildActive_Tree		= 1,	//行会活动-守护神树
	GuildActive_Max,
};

//行会活动状态
enum tagGuildActiveStsDef
{
	GuildActiveSts_Undo		= 0,	//未开始
	GuildActiveSts_Doing	= 1,	//进行中
	GuildActiveSts_Done		= 2,	//已结束
	GuildActiveSts_Max,
};

//赏金任务状态
enum tagRewardQuestStsDef
{
	RewardQuestSts_Undo		= 0,	//未领取
	RewardQuestSts_Doing	= 1,	//已领取
	RewardQuestSts_Finish	= 2,	//已完成
	RewardQuestSts_Commit	= 3,	//已提交
	RewardQuestSts_Max,
};

//任务的相关操作
enum tagQuesOP
{
	gQuestNoOP				= 0,		//占位
	gGetRewardQuestSts		= 1,		//[赏金任务]请求赏金任务状态
	gResetRewardQuestSts	= 2,		//[赏金任务]重置赏金任务状态
	gAcceptRewardQuest		= 3,		//[赏金任务]接取赏金任务
	gGiveUpRewardQuest		= 4,		//[赏金任务]放弃赏金任务
	gFinishRewardQuest		= 5,		//[赏金任务]立即完成赏金任务
	gCommitRewardQuest		= 6,		//[赏金任务]提交赏金任务
};

//英雄相关操作
enum tagHeroOp 
{
	OpHeroUpStage = 1,  //英雄升阶
    OpHeroUpSkill = 2,  //英雄技能升级
	OpHeroBattle = 3,   //英雄出战
	OpHeroLevelUp = 4,  //英雄升级	
};

//英雄属性
enum tagHeroProperty
{
    enHeroStage = 1, //英雄阶
	enHeroLevel = 2, //英雄等级
	enHeroExp    = 3, //英雄经验
	enHeroBless  = 4, //英雄祝福
	enHeroForce  = 5,//英雄体力
};


//职业竞技相关操作
enum tagCombatOp
{
	OpGetCombatInfo			= 1,		//获取竞技基本信息
	OpSelectCombatPayer		= 2,		//选择竞技对手
	OpCombatStart			= 3,		//发起竞技
	OpClearCooldown			= 4,		//清除冷却时间
	OpRechargeAwrad			= 5,		//兑换奖励
	OpGetCombatLog			= 6,		//请求战报
	OpGetCombatRank			= 7,		//请求竞技排行榜
	OpGetAwardList			= 8,		//请求已兑换奖励
	OpUpdateActorRank   = 9,  //更新角色排名
};
	
//tolua_end
