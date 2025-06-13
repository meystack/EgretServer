#pragma once

//为了防止名字污染，放在一个GameLog里
//tolua_begin
namespace GameLog
{
	enum tagLogType
	{
		Log_Recharge = 1,		//充值
		Log_RechargeCard = 2,	//充值卡
		Log_Deal = 3,			//私人交易
		Log_BackStage = 4,		//后台
		Log_Consigment = 5,     //寄售行
		Log_CreateGuild = 6,     //工会创建
		Log_GuildDonate = 7,     //行会贡献
		Log_GuildImpeach = 8,     //行会弹劾
		Log_Loot = 9,     			//捡取
		Log_MoneyExChange = 10,     //兑换  //10
		Log_LearnSkill = 11, 		//学习技能
		Log_Activity10002 = 12, //购买礼包
		Log_Activity4 = 13, //膜拜活动
		Log_Activity10009= 14, //达标类活动
		Log_PickUp= 15, 		//捡取
		Log_Activity8= 16, //世界boss活动
		Log_Relive= 17, //复活
		Log_Compose= 18, //合成装备
		Log_BossSystem = 19, //boss系统消耗
		Log_WorldChat = 20, //世界聊天  //20
		Log_MeridiansUP = 21, //经脉升级
		Log_Strengthen = 22, //强化
		Log_JoinFuben = 23, //进入副本
		Log_Bless     = 24,//祝福
		Log_Quest    = 25,//任务传送
		Log_ShopBuy    = 26,//商城购买
		Log_Activity2    = 27,//鉴定类活动
		Log_Achieve    = 28,//成就奖励
		Log_UseGift    = 29,//使用礼包
		Log_UseBox    = 30,//使用宝箱  //30
		Log_KillMonster    = 31,//杀怪
		Log_ForgeItem    = 32,//锻造
		Log_Mail    = 33,//邮件
		Log_Recover    = 34,//回收
		Log_FirstLogin    = 35,//首次登陆
		Log_TakeOn    = 36,//穿装备
		Log_TakeOff    = 37,//脱装备
		Log_ItemMerge    = 38, //合并道具
		Log_DeathDropItem    = 39, //死亡掉落
		Log_CircleEx    = 40, //转生兑换  //40
		Log_ChangeuseName    = 41, //改名
		Log_UserItem = 42, //自己使用道具
		Log_SelfDelItem = 43, //自己删除道具
		Log_SkillCostItem = 44, //技能消耗
		Log_Activity10001 = 45, //累计充值活动
		Log_Activity10010 = 46, //开服寻宝活动
		Log_Activity10003 = 47,  //首冲
		Log_Activity10004 = 48,  //七天登录
		Log_Activity10005 = 49, // 每日签到
		Log_Activity10006 = 50,  //投资基金类  //50
		Log_Activity9     = 51,  //独闯天涯
		Log_Activity5     = 52,  //npc夺宝类
		Log_Activity13    = 53,  //驻守任务
		Log_Activity12    = 54,  //材料副本
		Log_Activity11    = 55,  //夜战沃玛三
		Log_Activity7     = 56,  //藏经峡谷
		Log_Activity3     = 57,  //大乱斗
		Log_Activity1     = 58,  //烧猪刷怪副本类
		Log_FlyShoestran  = 59, //飞鞋传送
		Log_Activity10012 = 60, //YY会员
		Log_Activity14    = 61,  //战令活动
		Log_Activity15    = 62,  //任务活动
		Log_Fashion       = 63,  //时装	
		Log_NPCTelep       = 64,  //npc传送	
		Log_CleanRedName   = 65,  //清红名
		Log_MonthCard	 = 66, //月卡
		Log_Medal =	 67,//勋章
		Log_MEDICINECard	 = 68, //大药月卡
		Log_FreeCard	 = 69, //初心契约
		Log_ForeverCard	 = 70, //永恒契约&&色卡&&首充会员
		Log_Refining = 71,//洗炼
		Log_Activity10015 = 72, //每周礼包
		Log_YBRecover = 73, //元宝回收
		Log_UpCircle    = 74, //转生升级
		Log_PaoDian     = 75, //泡点
		Log_UpStar     = 76, //升星
		Log_SBKaward    = 77, //沙巴克
		Log_Area    = 78, //地图属性
		Log_TeamKillMonster    = 79, //组队杀怪
		Log_BuyFrezy = 80,//购买狂暴
		Log_Activity16    = 81,  //捐献活动
		Log_ghost    = 82,  //神魔之体
		Log_Call = 83,//召唤
		Log_Office = 84,//购买官阶
		Log_FourStarStrength = 85,//四象
		Log_CirtRing = 86,//暴击特戒
		Log_DizzyRing = 87,//麻痹戒指
		Log_ChgVocation = 88,//转职
		Log_Bag2Deport	= 89,		//物品从背包转移到仓库
		Log_Deport2Bag	= 90,		//物品从仓库转移到背包
		Log_DeportRemoveItem			= 91,		//仓库删除物品
		Log_DepotMerge    = 92, //仓库合并道具
		Log_4366Login    = 93, //4366登录礼包
		Log_4366Phone    = 94, //4366手机礼包
		Log_4366IdCard    = 95, //4366认证礼包
		Log_4366ExeLogin    = 96, //4366微端礼包
		Log_Activity10017 = 97,//兑换活动
		Log_QQhallLevel    = 98, //qq大厅等级
		Log_QQhallRegiste    = 99, //qq大厅注册
		Log_QQhallActive    = 100, //qq大厅活跃
		Log_PcGift    = 101, //微端下载奖励
		Log_QQBlueNewPlayer    = 102, //蓝钻新手礼包
		Log_QQBlueDaily    = 103, //蓝钻每日礼包
		Log_QQBlueGrowUp    = 104, //蓝钻成长礼包
		Log_QQBlueLevel    = 105, //蓝钻等级礼包
		Log_Activity10019 = 106, //个人达标类活动 
		Log_MoneyTree = 107, //摇钱树 
		Log_UseVipCard= 108, //回收卡
		Log_SoulWeapon = 109,		//兵魂 
		Log_TotalOnlineTime= 110, //累计在线
		Log_ActivityWMSAcross = 111,  //跨服夜战沃玛三
		Log_Activity23       = 112,//跨服膜拜
		Log_Activity10020 = 113, //开服寻宝活动
		Log_Activity26    = 114, //逃脱试炼活动
		Log_Activity10022 = 115, //360大玩家特权
		Log_Platform37 = 116, //37平台特权
		Log_Platform7Game = 117, //7游戏平台特权
		Log_PlatformQiDian = 118, //起点平台特权
		Log_Activity10024 = 119, //鲁大师平台
		Log_Activity10027 = 120, //ku25平台
		Log_PlatformSoGou = 121, //搜狗平台特权
		Log_Activity10028 = 122, //QiDian平台
		Log_Activity10029 = 123, //aiqiyi平台
		Log_Activity10030 = 124, //yaodou平台
		Log_Activity10031 = 125, //贪玩平台
		Log_Activity10032 = 126, //哥们平台
		Log_Activity10034 = 127, //2144平台
		Log_Activity10035 = 128, //快玩平台
		Log_Activity10036 = 129, //顺网平台
		Log_Words = 130, //字诀
		Log_Activity10037 = 131, //迅玩平台
		Log_Activity28    = 132, //秘境打宝
		Log_Max
	};

	//日志类型
	enum  taggGameLogType
	{
		//物品id,数量(可以是负数),备注
		clTakeOnItem				= 1,		//穿上装备
		clTakeOffItem				= 2,		//取下装备
		clUserItem					= 3,		//用户使用物品
		clUserDestroyItem			= 4,		//用户销毁物品
		clBuyItem					= 5,		//用户从NPC商店购买物品
		clSellItem					= 6,		//用户将物品卖出到NPC商店
		clTimeupItem				= 7,		//物品到期消失
		clGMAddItem					= 8,		//GM添加制造
		clWorldTalkItem				= 9,		//世界广播消费道具
		clAchieveRewardItem			= 10,		//成就奖励物品
		clQuestGiveItem				= 11,		//任务奖励物品
		clBuyStoreItem				= 12,		//购买商城物品获得
		clGuildFr					= 13,		//帮派繁荣度
		clSkillTrainItem			= 14,		//技能升级消耗物品 
		clEquipSplit				= 15,		//物品拆分
		clItemMerge					= 16,		//物品叠加
		clDealLostIteam				= 17,		//交易失去物品
		clDealGetItem				= 18,		//交易获得物品
		clBag2Deport				= 19,		//物品从背包转移到仓库
		clDeport2Bag				= 20,		//物品从仓库转移到背包
		clDeportRemoveItem			= 21,		//仓库删除物品
		clQuestLostItem				= 22,		//任务删除物品
		clServerGiveItem			= 23,		//服务后台送物品
		clLeftSceneRemoveItem		= 24,		//离开场景删除物品
		cl_25						= 25,
		clSkillLostItem				= 26,       //技能删除物品
		clDropItemGroup				= 27,		//丢物品到地上
		clChangeuseName				= 28,		//玩家改名字
		clGmAddMoney				= 29,		//GM增加了金钱
		clLearnSkillMoney			= 30,		//学习技能消费金钱
		clChatMoney					= 31,		//聊天消耗金钱
		clAchieveRewardMoney		= 32,		//成就奖励金钱
		clQuestGiveMoney			= 33,		//任务奖励金钱
		clStoreBuyMoney				= 34,		//商城购买物品的消耗
		clEnlargeBag				= 35,		//扩大背包
		clCreateGuildLostMoney		= 36,		//创建帮派时缴纳的钱币
		clWithdrawYuanBao			= 37,		//提取元宝	
		clServerGiveMoney			= 38,		//后台奖励金钱
		clUseEntrust				= 39,       //多倍任务消耗金钱
		clSkillDeMoney				= 40,  		//宠物强化系列消耗金钱
		clBagDestroyMoney			= 41,		//背包丢弃金钱
		cl_42						= 42,
		cl_43						= 43,
		cl_44						= 44,
		clConsumeFirstTimes			= 45,		//首次消耗元宝
		clBatchUseItem				= 46,		//批量使用道具
		clQuestGiveExp				= 47,		//任务奖励经验
		clGmAddExp					= 48,		//GM添加经验 
		clKillMonsterExp			= 49,		//杀死怪物获得经验
		clItemAddExp				= 50,		//经验流通物品获得
		clEQuipPropertyStrong		= 51,		//装备得到了强化
		clEQuipPropertyRandForge	= 52,		//随机锻造
		clEQuipPropertyInject		= 53,	    //装备鉴定的次数
		clEQuipPropertyQuality		= 54,		//装备得到了品质提升
		clEquipPropertyDurChange	= 55,       //装备的耐久发生了改变
		clEquipPropertyMaxDurChange = 56,       //装备的最大耐久就发生了改变
		clEquipPropertyCountChange	= 57,       //物品的数量发生了改变
		clEquipPropertyBindChange	= 58,       //装备的绑定属性发生改变
		clEquipPropertyForge		= 59,       //装备进行了鉴定
		clEquipPropertyLuck			= 60,		//装备幸运值或诅咒值
		clAddQuest					= 61,		//接受一个任务
		clFinishQuest				= 62,		//完成任务
		clGiveupQuest				= 63,		//放弃任务
		clCreateGuild				= 64,		//创建帮派
		clDeleteGuild				= 65,		//解散帮派
		clJoinGuild					= 66,		//加入帮派
		clExitGuild					= 67,		//退出帮派
		clChangeGuildPos			= 68,		//升降职
		clGetGuildCoinAward			= 69,		//领取捐献后的奖励
		clLevelUp					= 70,		//玩家等级升
		clLoginExp					= 71,		//上线记录经验
		clLogOutExp					= 72,		//下线记录经验
		clPkActor					= 73,		//pk玩家
		clNewDayArrive				= 74,		//玩家进入新一天
		clMap						= 75,		//记录玩家的地图位置
		clEnterFb					= 76,		//进入副本
		clExitFb					= 77,		//退出副本
		clOpenFb					= 78,		//开启副本
		clHeroAdd					= 79,  		//添加英雄
		clHeroRemove				= 80,		//删除英雄
		clHeroUpgrade				= 81,		//英雄升级
		clHeroTakeOnEquip			= 82,		//英雄穿上装备
		clHeroTakeOffEquip			= 83,		//英雄脱下装备
		clHeroLearnSkill			= 84,		//英雄学习技能
		clHeroForgetSkill			= 85,		//英雄遗忘技能
		clHeroChangeName			= 87,		//英雄改名
		cl_88						= 88,
		cl_89						= 89,
		cl_90						= 90,
		clHeroSkillLevelUp			= 91,		//英雄技能升级
		clHeroStageUp    			= 92,		//英雄进阶
		cl_93						= 93,
		clStallBuyItem				= 94,		//摆摊中购买物品
		clStallSellItem				= 95,		//摆摊中出售物品
		clStallBuyItemMoney			= 96,		//摆摊中买物品的钱
		clItemSpecSmith				= 97,		//保存装备的极品属性
		clCircleChange				= 98,		//转数变化
		clMonsterBeKilled			= 99,		//怪物被杀死
		clStallStatistics			= 100,		//摆摊事件追踪
		clVocationChange			= 101,		//职业转换
		clSexChange					= 102,		//性别转换
		clSpeedFinishQuest			= 103,		//元宝立即完成任务
		clDefendSbkGuild			= 104,		//沙巴克占领的行会日志
		clOutPutItemCount			= 105,		//物品产出统计
		clFlushQuest				= 106,		//刷星任务消耗金币或元宝
		clBookQuest					= 107,		//天书任务惊喜物品
		clAgainBookQuest			= 108,		//再次接天书任务
		clYbSpeedBook				= 109,		//渡魔任务立即完成
		clBookQuest2Award			= 110,		//渡魔任务双倍奖励
		clBookQuest3Award			= 111,		//渡魔任务三倍奖励
		clLootDropBox				= 112,		//拾取死亡掉落的物品
		clGMBackRemoveItem			= 113,		//后台删除物品
		clGMBackRemoveMoney			= 114,		//后台扣除玩家钱
		clGiveSalary				= 115,		//领取工资
		clDepotMoneyOpt				= 116,		//仓库金钱操作
		clEquipSharp				= 117,		//装备锋利值改变
		clSaveUseGmCmdToLog			= 118,		//使用GM命令
		clActorTotalConsume			= 119,		//玩家一天总消费
		clKungfuAttactExp			= 120,		//练功师怪攻击所得经验
		clMailGetItem				= 121,		//邮件系统收取附件
		clBoss						= 122,		//挖boss
		clPupilToTeacherExp			= 123,      //徒弟杀怪孝敬给师傅的经验
		clExchangeAcheive			= 124,		//成就点兑换获得物品
		clUseSpeedToolLog			= 125,		//使用加速外挂
		clUseSpeedToolSuspected		= 128,		//疑似使用加速
		clAchieveAwardOther			= 130,		//成就领取奖励默认类型
		clSkillResultGiveAward		= 131,		//技能奖励道具
		clBuffAddExp				= 132,		//buff增加经验
		clCircleSoul				= 133,		//转生灵魂更改
		clBufGetActivityExp			= 134,		//BUFF加经验，按经验库配置加
		clGuildGx					= 135,		//帮会贡献
		clActorDepotMoney			= 136,		//人物对仓库金钱操作
		clSignCommonSbk				= 137,	    //报名正常服沙巴克攻城战
		clFubenCollectRemItem		= 138,		//副本中采集怪
		clEquipPropMove				= 139,		//装备属性转移
		clMoveEquipPropDel			= 140,		//装备被转移掉属性
		clMoveEquipPropAdd			= 141,		//装备转移增加属性
		clBuffAddMoney				= 142,		//buff增加金钱
		clScriptCreateDropItem		= 143,		//脚本创建物品
		clSendMail					= 144,		//发邮件
		clDelMail					= 145,		//删除邮件
		clWorldLevelChange			= 146,		//世界等级变更
		clAddNewTitle				= 147,		//玩家增加头衔
		clDelNewTitle				= 148,		//玩家撤销头衔
		clAutoChangeLeader			= 149,		//弹劾行会掌门
		clCoinChanged				= 150,		//玩家金币变化
		clNpcTradeRebuyItem			= 153,      //npc商店回购物品
		clEqipRepair				= 154,		//装备修理
		clDeathDropExp				= 155,		//死亡掉经验
		clReliveConsume				= 156,		//复活消耗
		clAcceptRoleByMoney			= 157,		//消耗金钱接任务
		clEnlargeDepot				= 158,		//扩展仓库
		clSoldierSoulUnlock			= 159,		//解锁兵魂
		clSoldierSoulLevelupSkill   = 160,		//升级兵魂技能
		clSoldierSoulActStart		= 161,		//激活兵魂星星
		clSoldierSoulTakeOn			= 162,		//附身兵魂
		clSoldierSoulTakeOff		= 163,		//卸下兵魂
		clTrackActorFee				= 164,		//追踪玩家的费用
		clEquipStar                 = 165,      //装备升星
		clUseBackToCityItem         = 166,      //使用回城卷
		clUseSkillBook              = 167,      //使用技能书
		clRedName					= 168,		//清洗红名消费
		clCircleLevel               = 169,      //转生
		clPersonBoss                = 170,      //个人BOSS
		clEquipRecover              = 171,      //装备分解
        clEquipComposite            = 172,      //装备合成
		clOnNpcTeleport				= 173,		//传送石传送
		clGuildDonateDelCoin        = 174,      //行会捐献扣除金币
		clGuildDonateDelItem        = 175,      //行会捐献扣除道具
		clEquipPosStrongUpgrade		= 176,		//部位强化等级提升
		clCircleFuben        		= 177,		//转生副本
		clGuildTreeChargeDelItem 	= 178,		//行会神树充能扣除道具
		clGuildFuitGet				= 179,		//摘取行会神树果实
		cWingActivation				= 180,		//翅膀吞噬材料
		clGuildGetTaskAward			= 181,		//领取行会任务奖励
		clGuildGetBenefit			= 182,		//领取行会福利
		clUseExpItem                = 183,      //使用经验丹
		cGuildSiegeGatherGetJadeSeal= 184,		//获得玉玺
		cGuildSiegeDeleteJadeSeal	= 185,		//删除玉玺	
		cGuildGetCityPosWelfare		= 186,		//领皇城职位福利
		cGuildSiegeCallGuard		= 187,		//攻城召唤弓箭守卫
		cGuildSiegeStatueWorship	= 188,		//皇城雕像膜拜
		cGuildSiegeStatueGoldBlack  = 189,		//皇城雕像贴金抹黑
		crepairAllEquipItemFunc		= 190,		//使用战神油恢复耐久
		cGuildSiegeDropJadeSeal		= 191,		//清除玉玺
        clForce                     = 192,      //体力值更改
		cUseInnerItem				= 193,		//内劲道具消耗
		clTeamFuben                 = 194,      //团队副本
		clGuildActSucc              = 195,      //成功通过行会活动
		clConsignmentSellItem		= 196,		//寄卖物品
		clConsignmentGetItemBack	= 197,		//取回寄卖物品
		clConsignmentBuyItem		= 198,		//购买寄卖物品
		clConsignmentGetIncome		= 199,		//领取寄卖收益
		clGetOnlineReward			= 200,		//在线奖励
		clOnCheckSignIn				= 201,		//签到奖励
		clWorshMonarchAddExp		= 202,		//膜拜君主增加经验
		clLogOutExpSave				= 203,		//离线经验
		clWorkDayAward				= 204,		//[每日福利]资源找回
		clOnAsyncOpResultFunc		= 205,		//[每日福利]6.激活码礼包
		clExploitExchangeExp		= 206,		//战绩兑换经验
		clExploitChangeLog			= 207,		//战绩变化日志
		clThings					= 208,		//沙城秘宝
		clProtectTuCityBegin		= 209,		//保卫土城活动开始
		clProtectTuCityEnd			= 210,		//保卫土城活动结束
		clRewardQuestReset			= 211,		//[赏金任务]重置赏金任务
		clOpenServerEarYb			= 212,		//开服活动赚Yb
		clResetRewardQuestDelItem	= 213,		//[赏金任务]重置扣除道具
		clResetRewardQuestDelMoney	= 214,		//[赏金任务]重置扣除元宝
		cl_215						= 215,
		clFinishRewardQuestDelMoney	= 216,		//[赏金任务]立即完成扣除元宝
		clItemMergeBindInfect		= 217,		//物品叠加（绑定感染）
		clAddExpByExpBox			= 218,		//通过经验魔盒获得经验
		clUseExpBox					= 219,		//使用经验魔盒
		clGetDailyChargeAward		= 220,		//领取每日充值奖励 (原clOpenServerDailyCharge)
		clUseExpJade                = 221,      //使用经验勾玉
		clMonarchInspireCoin		= 222,      //膜拜鼓舞扣除金币
		clMonarchInspireYuanBao		= 223,      //膜拜鼓舞扣除元宝
		clBloodyFlames				= 224,      //浴血火海
        clSupplyBegin               = 225,      //[补给争夺]活动开始
		clSupplyEnd                 = 226,      //[补给争夺]活动结束
		clEscort                    = 227,      //极速运镖
		clZhuMo                     = 228,      //全民诛魔
		clRedPacketSend				= 229,		//发送红包
		clGrabRedPacketSuc			= 230,		//抢到红包
		clLauncher                  = 236,      //登陆器奖励
		clGetSevenGoodGift			= 237,		//[七日豪礼]领取七日豪礼
		clGetTotalChargeGift		= 238,		//累计充值奖励
		clFirstChargeAward			= 239,		//首充奖励
		clGetGameBackCompensate		= 240,		//领取游戏后台补偿
		clFlyingShoes				= 241,		//小飞翼飞boss
		clPersonBossSweep           = 242,      //个人BOSS扫荡
		cl_245						= 245,
		clOpenServerGift            = 246,      //开服大礼包
		clOpenSellBox               = 247,      //开启售卖礼包
		clExpRefine                 = 249,      //经验炼制
		clConsignmentShouting		= 250,		//寄卖吆喝
		clExpCell                   = 251,      //经验密室
		clGetActivityAward          = 252,      //领取活跃度奖励
		clMicronesia				= 253,      //创世密境
		clTeamPaTaPassAward			= 254,		//团队爬塔通关奖励
		clBookQuest4Award			= 255,		//渡魔任务四倍奖励
		clPersonPaTaPassAward		= 256,		//个人爬塔通关奖励
		clOpenSellBoxItem1          = 257,      //80级装备礼包
		clOpenSellBoxItem2          = 258,      //高手礼包【限时】
		clOpenSellBoxItem3          = 259,      //至尊礼包【限时】
		clOpenSellBoxItem4          = 260,      //转生特惠礼包【限时】
		clOpenServerGiftItem1       = 261,      //翅膀特惠礼包
        clOpenServerGiftItem3       = 263,      //龙珠特惠礼包
		clOpenServerGiftItem4       = 264,      //强化特惠礼包 
		clOpenServerGiftItem5       = 265,      //启兵特惠礼包
		clBagToGuildDepot			= 266,		//物品从背包转移到行会仓库
		clGuildDepotToBag			= 267,		//物品从行会仓库转移到背包
		clGuildDepotDel				= 268,		//物品从行会仓库丢弃
		clExchangeCirclSoulExp		= 269,		//英魂兑换经验
		clExchangeWingPointExp		= 270,		//使用道具获得羽魂
		clChallengeFBGetAward		= 271,		//领取挑战副本的通关奖励
		clChallengeFBGetAwardDelYB	= 272,		//领取挑战副本的双倍通关奖励消耗的元宝
		clUseSkillBookByMoney       = 273,      //消耗货币技能书
		clOpenServerGetAward        = 274,		//开服活动领取奖励
		clBossHomeEnter				= 275,		//进入boss之家
		clRecommendedUpgrade		= 276,		//练级传送
		clMoneyTree                 = 277,      //摇钱树
		clOpenServerGiftItem6       = 278,      //神炉特惠礼包【限时】
		clOpenServerGiftItem7       = 279,      //战神技能礼包【限时】
		clOpenServerGiftItem8       = 280,      //技能飞升礼包【限时】
		clOpenServerGiftItem9       = 281,      //神翼特惠礼包【限时】
		clAllChargeGiftAdd			= 282,		//获得累计充值礼包（不可重置）
		clAllChargeGiftGet			= 283,		//领取累计充值礼包（不可重置）
		cl_284						= 284,
		cl_285						= 285,
		clUseJieBiaoBox             = 286,      //使用劫镖盒子
		clEQuipPropertyInscriptLevel= 287,		//铭刻等级
		clEQuipPropertyLostStar		= 288,		//强化损失星级
		clGodStoveGetItem			= 289,		//神炉装备获得
		clGodStoveUpgradeItem		= 290,		//神炉装备升级
		clAddSkillExpDelItem		= 291,		//增加技能熟练度扣除道具
		clAllDay					= 292,		//闯天关
		clEquipStarAddRateFee       = 293,      //装备强化增加成功率费用
		clEquipStarRecoverFee       = 294,      //恢复损失强化星级费用
		clEquipStarTransferFee      = 295,      //装备强化星级转移费用
		clEquipInscriptDelItem      = 296,      //装备铭刻扣除经验道具
		clEquipInscriptFee			= 297,      //装备铭刻扣除费用
		clEquipInscriptTransFee		= 298,      //装备铭刻转移扣除费用
		clEquipInscriptTransDelItem	= 299,      //装备铭刻转移扣除铭刻保护物品
		clEquipCompositeDelItem		= 300,      //装备合成扣除装备和材料
		clEquipCompositeAddItem		= 301,      //装备合成增加装备
		clBossjfExchange            = 302 ,     //Boss积分兑换寻宝积分
		clHeroExp                   = 303,      //英雄经验丹
		clFreshGameMystiaclShopDelItem = 304,   //刷新商城的神秘商店扣道具
		clFreshGameMystiaclShopDelFee  = 305,   //刷新商城的神秘商店扣元宝
		clBuyGameMystiaclShopItemNum= 306,		//购买神秘商店物品的数量
		clBuyGameMystiaclShopItemFee= 307,		//购买神秘商店商品的费用
		clMeritoriousAward			= 308,		//功勋系统奖励
		clRestoreHpMp               = 310,      //Npc满血回复
		clCombatChallengeAward		= 311,		//职业宗师挑战奖励
		clCombatRechargeAward		= 312,		//职业宗师兑换奖励

		clMaterialFBEnterFee		= 313,		//进入材料副本扣除消耗
		clMaterialFBGetAwardFee		= 314,		//领取材料副本奖励扣除元宝
		clMaterialFBGetAward		= 315,		//领取材料副本奖励
		clMaterialFBSweepFee		= 316,		//扫荡材料副本扣除元宝
		clMaterialFBSweepAward		= 317,		//扫荡材料副本获取奖励
		clGoMaYa                    = 318,      //进入玛雅神殿
		clYYPurpleDiamondAward      = 319,      //yy紫钻礼包获取
		clStoreBuyOp				= 320,		//商城购买操作
		clGRBossAward               = 321,      //个人BOSS单倍领取
		clGRBossDoubleAward         = 322,      //个人BOSS双倍领取

		clBossMob					= 323,		//BOSS被创建
		clBossKilled				= 324,		//BOSS被击杀
		clBossTimeout				= 325,		//BOSS生命周期到
		clActivityRecord			= 326,		//活动参与记录
		clHeroForce                 = 327,      //英雄体力丹

		clItemRedNameMedicalUse		= 328,		//红名药水使用
		clOpenServerThemeSendAward	= 329,		//发送某个开服主题活动的奖励
		clDailyChargeResetRank		= 330,		//每日充值奖励补发邮件并且清空排行榜
		clGuildDepotPutIn			= 331,		//"放入行会仓库",
		clGuildDepotGetOut			= 332,		//"兑换行会仓库物品",
		clGuildDepotCheck			= 333,		//"审核行会仓库兑换",
		clLockBossEnterConsume		= 334,		//进入锁妖塔场景扣除的费用
		clGuildNest					= 335,		//行会魔窟
		clNightFighting				= 336,		//北奇夜战
		clRollItem					= 337,		//开箱子/礼包
		clHeroSystemPack            = 338,      //战神速升礼包

		clEquipIdentifySlotUnlock	= 339,      //装备鉴定属性槽解锁
		clEquipIdentify				= 340,      //装备鉴定

		clEquipIdentifyUnlockSlotFee= 341,      //装备鉴定属性槽解锁的消费
		clEquipIdentifyFee			= 342,		//装备鉴定的消费
		clEquipIdentifySlotLockFee	= 343,		//装备鉴定时的锁定属性的消费
		clEquipIdentifyTransferFee	= 344,		//装备鉴定属性转移的消费
		clChatAutoShutUp			= 345,			//聊天拉人自动禁言
		clGuildBidRank				= 346,		//行会竞价排名
		clGuildChangeCoin			= 347,		//行会资金改变
		clPackItem                  = 348,      //装备打捆
		clDayMatch					= 353,		//每日竞技
		clgetYYGiftAward			= 354,		//YY大厅特权领取礼包
		clComboServer               = 356,      //合服活动
		clOpenServerBoss            = 357,      //开服全民BOSS
		clOpenServerGiftItem10      = 358,		//紫金礼盒【限时】
		clOpenServerGiftItem11      = 359,		//朱红礼盒【限时】
		clOpenServerGiftItem12		= 360,		//黄金礼盒【限时】
		clGroupBuy 					= 361,  //开服团购
		clRoleFuben                 = 362, //主线任务副本
		clFirstLogin                = 363, //首次登陆奖励
		cNpcPersongift              = 364,      //npc活动领取个人宝箱
		cNpcBigTreasure             = 365,      //npc活动领取大宝箱
		clAddExpbyPaoDian			= 366,		//通过泡点增加经验
		clChargeYuanbao				= 367,		//通过道具充值元宝
		clAllchargeBonus			= 368,		//累计充值奖励活动
		clTehuilibaoAtv				= 369,     	//特惠礼包活动购买礼包
		clShouchongAtv				= 370,		//首冲活动领取礼包
		clDaKaActivity				= 371,		//打卡签到相关活动
		clDengLuJiJingAtv			= 372, 		//登录基金活动发放奖励
		clBuyDengLuJiJingAtv		= 373,		//购买登录基金
		//*****************php后台所用到的日志id*********************
		clKillMonsterItem			= 500,		//杀怪获得物品
		cl_501			            = 501,
		clDealTransferMoney			= 502,		//交易获得金钱或扣除金钱
		clDealTransYuanbao			= 503,		//交易转移元宝
		clGetYbFirstTimes			= 504,		//首次提取元宝
		clKilledByMonster			= 505,		//给怪物杀死
		clStallSellItemMoney		= 506,		//摊中出售物品的钱
		clDeathDropItem				= 507,		//死亡后掉落物品
		clOnlineTime				= 508,		//在线时长(单位秒)
		//******************日志id预留到599*****************************
		clAddItem				    = 510,		//添加物品
		clEXchangeCoin				= 511,		//兑换
		clComPoseItem				= 512,		//合成
		clForgetem					= 513,		//锻造
		clConsumeSource				= 514,		//消耗
	};
};
//tolua_end

namespace GameLog
{
	//添加字符串格式
	//-- Log_([a-zA-Z0-9]*)[ ]*=[ 0-9]*
	//-- "$1"
	static LPCSTR LogItemReason[] = {
		"NO",
		"Recharge",	//充值
		"RechargeCard",	//充值卡
		"Deal",		//私人交易
		"BackStage",	//后台
		"Consigment",	//寄售行
		"CreateGuild",	//工会创建
		"GuildDonate",	//行会贡献
		"GuildImpeac",	//行会弹劾
		"Loot",     			//捡取
		"MoneyExChange",     //兑换  //10
		"LearnSkill", 		//学习技能
		"Activity10002", //购买礼包
		"Activity4", 	//膜拜活动
		"Activity10009", //达标类活动
		"PickUp", 		//捡取
		"Activity8", //世界boss活动
		"Relive", //复活
		"Compose", //合成装备
		"BossSystem", //boss系统消耗
		"WorldChat", //世界聊天    //20
		"MeridiansUP", //经脉升级
		"Strengthen", //强化
		"JoinFuben", //进入副本   
		"Bless",//祝福
		"Quest",//任务传送
		"ShopBuy",//商城购买
		"Activity2",//鉴定类活动
		"Achieve",//成就奖励
		"UseGift",//使用礼包
		"UseBox",//使用宝箱       //30
		"KillMonster",//杀怪
		"ForgeItem",//锻造
		"Mail",//邮件
		"Recover",//回收
		"FirstLogin",//首次登陆
		"TakeOn",//穿装备
		"TakeOff",//脱装备
		"ItemMerge", //合并道具
		"DeathDropItem", //死亡掉落
		"CircleEx", //转生兑换      //40
		"ChangeuseName", //改名
		"UserItem", //自己使用道具
		"SelfDelItem", //自己删除道具
		"SkillCostItem", //技能消耗 //44
		"Activity10001", //累计充值活动
		"Activity10010", //开服寻宝活动
		"Activity10003",  //首冲
		"Activity10004",  //七天登录
		"Activity10005", // 每日签到
		"Activity10006",  //投资基金类  //50
		"Activity9",  //独闯天涯
		"Activity5",  //npc夺宝类
		"Activity13",  //驻守任务
		"Activity12",  //材料副本
		"Activity11",  //夜战沃玛三
		"Activity7",  //藏经峡谷
		"Activity3",  //大乱斗
		"Activity1",  //烧猪刷怪副本类
		"FlyShoestran", //飞鞋传送
		"Activity10012",//YY会员
		"Activity14",  //战令活动
		"Activity15",  //任务活动
		"Fashion",  //时装	
		"NPCTelep",  //npc传送	
		"CleanRedName",  //清红名
		"MonthCard", //月卡 
		"Medal",//勋章
		"MEDICINECard" , //大药月卡
		"FreeCard", //初心契约
		"ForeverCard", //永恒契约&&首充会员&&色卡
		"Refining",//洗炼
		"Activity10015",//每周礼包
		"YBRecover", //元宝回收
		"UpCircle", //转生升级
		"PaoDian",//泡点
		"UpStar",//升星
		"SBKaward",//沙巴克
		"Log_Area", //地图属性
		"Log_TeamKillMonster", //组队杀怪
		"BuyFrezy",//购买狂暴
		"Activity16",  //捐献活动
		"Ghost",  //神魔之体
		"Call",//召唤
		"Office",//购买官阶
		"FourStarStrength", //四象
		"CirtRing" ,//暴击特戒
		"DizzyRing" ,//麻痹戒指
		"ChgVocation",//转职
		"Bag2Deport",		//物品从背包转移到仓库
		"Deport2Bag",		//物品从仓库转移到背包
		"DeportRemoveItem",//删除仓库物品
		"DepotMerge", //仓库合并道具
		"4366Login"  , //4366登录礼包
		"4366Phone" , //4366手机礼包
		"4366IdCard", //4366认证礼包
		"4366ExeLogin", //4366微端礼包
		"Activity10017",//兑换活动
		"QQhallLevel" , //qq大厅等级
		"QQhallRegiste"  , //qq大厅注册
		"QQhallActive"  , //qq大厅活跃
		"PcGift" , //微端下载奖励
		"QQBlueNewPlayer"  , //蓝钻新手礼包
		"QQBlueDaily"  , //蓝钻每日礼包
		"QQBlueGrowUp" , //蓝钻成长礼包
		"QQBlueLevel", //蓝钻等级礼包
		"Activity10019", //个人达标类活动
		"MoneyTree", //摇钱树 
		"UseVipCard", //回收卡 
		"SoulWeapon", //兵魂  
		"TotalOnlineTime", //累计在线
		"ActivityWMSAcross", //跨服沃玛三
		"Activity23" ,//跨服膜拜
		"Log_Activity10020", //开服寻宝活动
		"Log_Activity26",    //逃脱试炼活动
		"Log_Activity10022", //360大玩家特权
		"Log_Platform37", //37平台特权
		"Log_Platform7Game", //7游戏平台特权 
		"Log_PlatformQiDian", //起点平台特权
		"Log_Activity10024", //鲁大师平台
		"Log_Activity10027", //ku25平台 
		"Log_PlatformSoGou", //搜狗
		"Log_Activity10028", //QiDian平台
		"Log_Activity10029", //aiqiyi平台
		"Log_Activity10030", //yaodou平台
		"Log_Activity10031", //贪玩平台
		"Log_Activity10032", //哥们平台
		"Log_Activity10034", //2144平台
		"Log_Activity10035", //快玩平台
		"Log_Activity10036", //顺网平台
		"Log_Words",		 //字诀
		"Log_Activity10037", //迅玩平台
		"Log_Activity28", 	 //秘境打宝
	};
}

static_assert(sizeof(GameLog::LogItemReason)/sizeof(LPCSTR) == GameLog::Log_Max, "tagLogType 长度应跟 GameLog::LogItemReason 保持一致");
