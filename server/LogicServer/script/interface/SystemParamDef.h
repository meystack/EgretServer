#pragma once
//下面是一些系统的参数的定义，比如职业的ID,门派的id,实体类型等等




////////////////////////////////////////////////////////////////下面这些要导出脚本的，需要导出的才放下面!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11
//tolua_begin

//排行榜ID
enum enRankID
{
	// 这个是长期存在的非即时排行榜，合服不影响（定时从数据库中排序获取，0-100）
	enRankBaseBegin = 0,
	enRankAllLevel = 0,					// 等级排行榜
	enRankVoc1Level = 1,				// 战士等级排行榜（职业1）
	enRankVoc2Level = 2,				// 法师等级排行榜（职业2）
	enRankVoc3Level = 3,				// 道士等级排行榜（职业3）
	enMoBaiRankList = 4,				// 膜拜排行
	enPhyAtkMaxRankList = 5,				//物攻
	enMagicAtkMaxRankList = 6,				//法功
	enWizardAtkMaxRankList = 7,				//道攻
	enCSALLLevel = 8,				//跨服排行
	enCSMoBaiRank = 9,				//跨服膜拜排行
	enRankBaseEnd = 10,

	// 这个是即时排行榜，合服会清空（101-200）
	enRankGuildSbkSiege = 101,			// 帮会战排行榜
	enRankTodayConsumeYuBao = 102,		// 今日消费排行
	enRankYesterdayConsumeYuBao = 103,	// 昨日消费排行
};

//成就特权
enum enAchievePrivilege
{
	enAddMeritorious = 1,		//百分比增加获得功勋值，配置值1代表1%
	enAddWarehouse,				//固定值增加仓库格子，配置1代表1个格子
	enAddEquip,					//固定值增加装备回收系统上限，配置1代表增加1万上限
	enAddGod,					//固定值增加金砖使用次数上限，配置1代表增加1次使用次数
	enSubHit,					//百分比减少受到玩家伤害的值，配置100代表减少1%受到玩家伤害
	enAddToken,					//固定值增加神魔令上限值，配置1代表增加上限1点 
	enAchievePrivilegeMax,		//最大值 
};

/// 金钱的类型的定义
enum eMoneyType
{
	mtCoin				=1,	//可交易的金钱
	mtBindCoin			=2,	//绑定金钱
	mtBindYuanbao		=3,	//绑定元宝
	mtYuanbao			=4,	//可交易的元宝
	mtFlyShoes			=7,	//小飞鞋
	mtJyQuota			=19,//交易额度

	mtCircleSoul=16,			//英魂 TO DELETE
	mtWingSoul = 17,			//羽魂 TO DELETE
	
	mtPosStrongValue = 18,	//启兵值 TO DELETE
	mtSoldierSoul = 25,		//兵魂 TO DELETE
	mtBossJiFen = 26,		//Boss积分 TO DELETE 

	mtMoneyTypeCount,
	mtMoneyTypeStart = mtCoin,
};

//系统提示的类型
enum tagTipmsgType
{
	tstGetItem = 1, 	//获取物品提示
	tstProperty = 2, 	//经验属性提示
	tstEcomeny = 3,		//点券与元宝提示
	tstFigthing = 4,	//战斗信息提示
	tstUI = 5, 			//UI提示
	tstChatSystem = 6,	//聊天框系统信息
	tstChatNotice = 7,	//聊天框公告
	tstRevolving = 8,	//走马灯公告
	tstBigRevolving = 9,//大走马灯
	tstPop = 10,		//公告弹框
	tstExclusive = 11,		//专属
	tstKillDrop = 12,		//闪屏
	tstFirstKillDrop = 13,  //首杀
	tstBossRelive = 14,  //boss刷新
	tstDropWindows = 15,  //掉落弹窗
	tstUIWindows = 16,  //弹窗提示
	tstBindCoinNotEnough  = 17,	//绑定金币不足提示
	tstCoinNotEnough      = 18, //金币不足提示
	tstBindYbNotEnough    = 19, //绑定元宝不足提示
	tstYbNotEnough        = 20, //元宝不足提示
	tstUpdateClient        = 21, //客户端更新提示

	// TO DELETE

	ttInvalidTmType = -1, // 无效的Tipmsg类型
	ttTipmsgWindow =1, //只在右侧提示栏显示
	ttScreenCenter=2, //屏幕中央，用于xxx把xxx强化到6级之类的全服公告
	ttDialog =4,      //弹出框
	ttBroadcast =8,  //公告栏，用于GM发通知
	ttMessage = 16,	//短消息，在屏幕中央悬挂，点击后查看内容，
	ttWarmTip  =32,   //温馨提示
	ttGmTip = 64,	// Gm提示信息
	ttChatWindow =128, //左侧聊天栏里，用于一些系统公告在聊天栏显示
	ttPropertyTip = 512,	//属性tip
	ttFlyTip	= 256,	//人物飘出
	ttShortcutTip = 1024,	//快捷栏上面的tip
	ttGetResources = 2048,	//获得资源飘字
	//ttMaxTmType,		// 最大Tipmsg类型
};


//玩家身上的社会关系的bit位定义
enum tagSocialMaskDef
{
	smGuildCommon			= 0,	//帮会普通成员
	smGuildElite            = 1,    //精英
	smGuildTangzhu			= 2,	//长老
	smGuildAssistLeader		= 3,	//副会长
	smGuildLeader			= 4,	//会长
	smTeamMember			= 6,	//是队伍的普通成员
	smTeamCaptin			= 7,	//是队长	
	smStoreBroadcastClose	= 8,	//是否关闭，0表示开启的，1表示关闭的
	smHideFationCloth		= 9,	//是否隐藏时装，1表示隐藏时装（暂时保留）
	smDarwActivityAward		= 10,	//活跃度是否领取奖励
	//smIsWulinMaster			= 9,	//是武林盟主，0表示不是，1表示是
	smHideHeadMsg			= 11,	//是否隐藏头顶的信息，1表示隐藏头顶信息（包括名字，头衔，称号，帮派等），0表示不限制
	smShowRedWeaponEffect	= 12,	//是否显示神器的特效，1表示显示，0表示不显示
	smIsInCommonServer		= 13,	//是否在跨服的场景，1表示是，0表示否
	//smSaveGameAddress		= 13,	//是否保存了游戏网页
	//smFlushStarToLv			= 14,	//刷星到指定星级
	//smStarUseGold			= 15,	//刷星余额用元宝补充
	//smIsSbkCityMaster		= 16,	//是否是攻城-君主1是0否//不用了
	smGmFlag				= 14,	//GM标记
	smHideWeaponExtend		= 15,   //是否隐藏幻武外观，1表示隐藏（暂时保留）
	smIsFamous				= 16,   //是否是名人堂会员
	smHideExtraWeapon		= 17,   //[幻武玄甲]是否隐藏幻武装备外观（1-隐藏，0-显示）
	smHideExtraDress		= 18,   //[幻武玄甲]是否隐藏玄甲装备外观（1-隐藏，0-显示）
};

namespace Item
{
	/** 定义物品类型的枚举类型 **/   
	enum ItemType
	{
		itUndefinedType			= 0,	//未定义类型的物品
		itWeapon				= 1,	//武器
		itDress					= 2,	//衣服
		itHelmet				= 3,	//头盔
		itNecklace				= 4,	//项链
		itDecoration			= 5,    //勋章
		itBracelet				= 6,	//手镯
		itRing					= 7,	//戒指
		itGirdle				= 8,	//腰带
		itShoes					= 9,	//鞋子
		itEquipDiamond			= 10,   //魂玉
		itSzBambooHat		    = 11,   //神装斗笠
		itSztFaceNail			= 12,	//神装面甲
		itSzitCape			    = 13,	//神装披风
		itSzitShiel			    = 14,	//神装盾牌
		itPearl			        = 15,	//宝珠
		itMagicpWeapon		    = 16,	//魔器
		itSilverNeedle 			= 17,   //银针
		itDetermination         = 18,   //心决
		itGoldSeal              = 19,   //金印
		itSandersPearl          = 20,   //檀珠
		itSilverHairClasp 		= 21,   //银花簪
		itHeartNourishingJade   = 22,   //养心玉
		itGoldObsidianBead      = 23,   //金曜珠
		itBlackSandalwoodStone  = 24,   //黑檀石
		itEquipMax,						//最大的装备ID

		itQuestItem				= 101,	//任务物品
		itFunctionItem			= 102,	//功能物品，可以双击执行功能脚本的
		// itMedicaments			= 103,	//普通药品(是否拾取由玩家指定)
		// itFastMedicaments		= 104,	//速回药品
		// itItemDiamond			= 105,  //宝石
		// itMedicaments2			= 106,	//普通药品(必定拾取)
		// itExpBox				= 113,  //经验魔盒，吸收杀怪经验
		// itMine					= 114,  //矿物，和普通物品比它的耐久表示纯度和最大纯度
		// itMoSheng			    = 115,	//魔神道具
		// itCircleSoul            = 116,  //英魂道具
		itDailyUse              = 117,  //每日使用限制(如:英魂道具) 同类道具您今天已使用%d / %d次
		// itSellBox               = 118,  //售卖类箱子道具
		// itSkillBook             = 119,  //需要货币的技能书
		itAutoUse               = 120,  //自动使用物品
		// itJieBiaoBox            = 121,	//劫镖盒子 
		// itInscriptProtectItem	= 122,  //装备铭刻保护石头
		// itMeritorious			= 123,  //功勋道具
		// itHeroForce		        = 124,  //战神体力丹
		itGold                  = 125,  //金砖
		// itExp                   = 126,  //人物经验丹
		itItems                 = 127,  //普通物品
		itSlowHpdMed            = 128,  //慢回金创药
		itSlowBluedMed          = 129,  //慢回魔法药
		itFastMedicament        = 130,  //瞬回药
		itGift                  = 131,  //礼包
		itRandomMove            = 132,  //随机传送卷
		itDoubleExp             = 133,  //多倍经验
		itHair                  = 134,  //发型
		itYuanBaoCharge			= 135,	//充值卡充值元宝
		itBox                   = 136, //宝箱
		itGuaJiJuanZhou			= 137, //挂机卷轴加多倍经验倍率buff
		itAddBuff				= 138, //获得buff
		itRecover				= 139, //
		itSelectOneBox			= 140, //多个宝箱开启其中一个
		itRebateYuanBaoCharge	= 141, //返利卡充值元宝


		itItemTypeCount,			//物品类型的数量，最大值，类型值不一定是连续的
	};


	enum tagItemProperty
	{
		//下面的是动态的属性，只有通过指针获取
		ipItemID , //物品的ID
		ipItemCount, //物品的数量
		ipItemStrong, //强化等级
		ipItemQuality,  //装备的品质
		ipItemBind,    //物品的绑定标记
		ipItemType, //物品的类型
		ipItemHole1IsOpen,//宝石槽位1是否开启
		ipItemHole2IsOpen, //宝石槽位2是否开启
		ipItemHole3IsOpen, //宝石槽位3是否开启
		ipItemHole4IsOpen, //宝石槽位4是否开启
		ipItemHole1Item,//宝石槽位1的物品
		ipItemHole2Item,//宝石槽位2的物品
		ipItemHole3Item,//宝石槽位3的物品
		ipItemHole4Item,//宝石槽位4的物品
		ipItemRandForge,  //随机精锻一件装备，读取用于随机计算一个属性，返回属性和id
		//设置用于随机精锻一个属性
		ipItemForgeTimes, //精锻的次数
		ipItemForgeProperty1Value, // 读取/设置 精锻的1个属性的值,
		ipItemForgeProperty2Value, //读取/设置精锻的1个属性的值,
		ipItemForgeProperty3Value, //读取/设置精锻的1个属性的值,
		ipItemDua,                 //装备的耐久
		ipItemDuaMax,              //装备的最大耐久
		ipItemExpiredTime,         //过期时间
		ipItemLuck,					//幸运值设置
		ipItemInscriptExp,			//铭刻经验
		ipItemSharp,				//装备的锋利值
		ipItemLostStar,				//当前强化损失的星级（强化）
		ipItemInscriptLevel,		//铭刻等级
		ipItemStar,                 //装备星级（强化等级）
		ipIdentifySlotNum,			//物品鉴定属性槽被解锁的数量
		ipIdentifyNum,				//物品已经被鉴定的次数
		ipItemStaticPropStart ,   

		//////下面的这些属性通过指针或者ID都能够获取
		ipItemDealMoneyType =ipItemStaticPropStart,           //物品卖商店的金钱的类型
		ipItemDealMoneyCount,           //物品卖商店的金钱的数量
		ipItemActorLevel,       //需要玩家的等级
		ipItemSuitId,             //套装ID
		ipItemForgeMaxTimes,      //精锻的最大次数
		ipItemDenyStorage,        //是否禁止放角色仓库
		ipItemDenySell,          //是否禁止卖到商店里
		ipItemInlayable,          //是否能够镶嵌宝石 ---不用了
		ipItemNeedVocation,           //需要的职业，0表示无职业要求
		ipItemNeedSex,          //物品需要的性别
		ipItemSmithId,           //精锻的配置id
		ipItemBreakId,          //分解的配置的id
		ipItemNotConsumeForCircleForge,	//是否转生锻造时不需要副装备
		ipItemStaticDuaMax,       //静态物品的耐久最大(物品表配置的)
		ipItemForgeProtect,			//保护鉴定的属性
		ipItemShape,				//物品的shape
		ipItemMatchAllSuite,		//是否是符合全部的套装
		ipItemSpecialRingType,		//特戒类型
		ipItemStrongCount,			//可强化次数
		ipItemActorCircle,       //需要玩家的转数
		ipItemDenyGuildDepot,        //是否禁止放行会仓库
		ipItemCanIdentify,			//是否可以被鉴定
		ipItemMaxProperty,        
	};

}
/**
** 定义用户物品的标志属性
**/
enum UserItemFlag
{
	ufUnBind = 0,
	ufBinded = 1,	//绑定
	ufDenyDeal = 2,//不可交易
};


//数据类型定义，用于namespace DataPack
enum tagDataType{
	dtByte = 0,
	dtChar = 1,
	dtWord = 2,
	dtShort = 3,
	dtInt = 4,
	dtUint = 5,
	dtInt64 = 6,
	dtUint64 = 7,
	dtString = 8,
};


/*
*实体的状态
*状态是持续性的，比如打坐，死亡，吟唱，摆摊
*也就是有一个过程的
*/
enum tagEntityState
{
	esStateStand				= 0,	//静止状态
	esStateMove					= 1,    //行走状态
	esStateBattle				= 2,	//战斗状态
	esStateDeath				= 3,	//死亡状态
	esStateReturnHome			= 4,	//回归状态(用于怪物)

	esStateStall				= 5,	//摆摊状态
	esStateMoveForbid			= 6,	//禁止移动状态,buff设置的
	esStateSlient				= 7,	//沉默状态
	esStateDizzy				= 8,    //晕眩状态
	esStateAutoBattle			= 9,	//挂机状态
	esStateHide					= 10,	//隐身状态
	esDisableSkillCD			= 11,	//禁用技能CD标志（开发和测试用）
	esPaTaFubenBattle			= 12,	//爬塔副本中3
	esStateOwnPet				= 13,	//拥有宠物状态
	esStateHeroMerge			= 14,	//英雄附体状态
	esStateSing					= 15,   //吟唱状态
	esStateDenyAttackedByActor  = 16,	//不能被攻击状态
	esStateTeamFuben			= 17,	//团队副本状态
	esSupplyBattleSack			= 18,	//补给争夺得到粮食包的状态

	esState_19					= 19,
	esState_20					= 20,
	esState_21					= 21,
	esState_22					= 22,
	esState_23					= 23,
	esState_24					= 24,
	esState_25					= 25,
	esState_26					= 26,
	esState_27					= 27,
	esState_28					= 28,
	esState_29					= 29,
	esState_30					= 30,
	esState_31					= 31,

	esMaxStateCount, //状态的数量最大只能31位
};


//装备的存储位置每一个表示什么
enum tagEquipSlot
{
	itWeaponPos,					//武器位置
	itDressPos,						//衣服
	itHelmetPos,					//头盔

	itNecklacePos ,					 //项链
	itDecorationPos ,				 //勋章(未开放)

	itBraceletPos,                  //手镯
	itRingPos,					  //戒指位置
//	itLeftBraceletPos  ,			//左边的手镯
//	itRightBraceletPos  ,			//右边的手镯

//	itLeftRingPos,					//左边的戒指位置
//	itRightRingPos,					//右边的戒指位置

	itGirdlePos,					//腰带
	itShoesPos,						//鞋子
	itEquipDiamondPos,				//魂玉

	itBambooHatPos,				//神装斗笠
	itFaceNailPos,				//神装面甲
	itCapePos,				//神装披风
	itShieldPos,				//神装盾牌

	itPearlPos,              //宝珠
	itMagicpPos,              //魔器
	itSilverNeedlePos,       //银针
	itDeterminationPos,      //心决
	itGoldSealPos,           //金印
	itSandersPearlPos,       //檀珠
	itSilverHairClaspPos, 	  	//银花簪
	itHeartNourishingJadePos,   //养心玉
	itGoldObsidianBeadPos,      //金曜珠
	itBlackSandalwoodStonePos,  //黑檀石
	// itSpecialRingPos,				//特戒(未开放)
	// itExtraWeaponPos,				//幻武的位置(未开放)
	// itExtraDressPos,				//玄甲的位置(未开放)
	// itBloodSoulPos,					//血魄
	// itGodShieldPos,					//神盾
	// itDragonSoulPos,				//龙魂
	// itIntellectBallPos,				//智珠
	itMaxEquipPos
};

enum tagSceneAreaFlag 
{
	scfGuildSiege = 1,//攻城区域
};
enum tagGuildCityPos //皇城职位(攻城)
{
	stNoCityPos,
	stRealMaster,			//君主
	stEastProtector,		//副城主
	stSouthProtector,		//南护法
	stWestProtector,		//西护法
	stNorthProtector,		//北护法
	stMaxGuildCityPos,
};
enum tagGuildPlayerPosType  //广播行会玩家信息类型
{
	gptEnter,				//第一次进入
	gptMove,				//里面移动
	gptDisappear,			//消失
};
enum tagGuildLeaderChangeType //帮主更换
{
	glcLeaderChange,//帮主禅让
	glcImpeachLeader,//弹劾
	glcGmBackStageSet,//gm或者后台设置
};

//行会任务类型定义
enum tagGuildTaskType
{
	gttKillMonster=10,			//杀怪
};
//levelConfig对应id
enum tagOpenLevel 
{
	lvGodStove =1,	//神炉
	lvExploit	=6	//战绩系统
};

//行会事件ID
enum enGuildEventId
{
	enGuildEvent_NULL,				//占位
	enGuildEvent_Create,			//%s(玩家)创建了行会
	enGuildEvent_MemberJoin,		//%s(玩家)加入了行会！
	enGuildEvent_MemberLeft,		//%s(玩家)离开了行会！
	enGuildEvent_TickMember,		//%s(操作人)将%s(玩家)踢出了行会！
	enGuildEvent_LeaderChange,		//%s(原首领)把首领禅让给%s(新首领)
	enGuildEvent_AddOfficer,		//%s(玩家)被任命为%s(官名)！
	enGuildEvent_DelOfficer,		//%s(玩家)被撤销了%s(官名)！
	enGuildEvent_levelUp,			//本行会已成功提升至%d级！
	enGuildEvent_Impeach,			//%s(玩家)弹劾成功，成为了%s(官名)！
	enGuildEvent_War,				//%s向我方行会宣战！
	enGuildEvent_ToWar,				//向%s宣战！
	enGuildEvent_DonateCoin,		//%s(玩家)捐献了%s金币！
	enGuildEvent_DonateYB,			//%s(玩家)捐献了%s元宝！
	enGuildEvent_UpOfferice,		//恭喜%s(玩家)成为%s(官名)！
	// enGuildEvent_PutInDepot,		//%s(玩家)在行会仓库中投入了%s(装备名字）！
	// enGuildEvent_GetOutDepot,		//%s(玩家)在行会仓库中兑换了%s(装备名字）！
	enGuildEvent_Max,
};

//申请加入行会的错误代码
enum enErrApplyJoinGuild
{
	enApplyJoin_NoErr,			// 成功
	enApplyJoin_HasAgreed,		// 已经被某个行会同意了 
	enApplyJoin_HasSame,		// 已经申请了此行会
	enApplyJoin_NoGuild,		// 行会不存在
	enApplyJoin_MemberLimit,	// 行会人数超过
	enApplyJoin_ActorErr,		// 玩家错误
	enApplyJoin_HasGuild,		// 已经加入了行会
	enApplyJoin_UnKnown,		// 未知错误
	enApplyJoin_Max,			// 
};

//审核加入行会的错误代码
enum enErrCheckJoinGuild
{
	enCheckJoin_NoErr,			// 成功
	enCheckJoin_NoApply,		// 没有申请
	enCheckJoin_NoGuild,		// 行会不存在
	enCheckJoin_MemberLimit,	// 行会成员达到上限
	enCheckJoin_HasGuild,		// 已经加入其它行会
	enCheckJoin_UnKnown,		// 未知错误
	enCheckJoin_MAx,			// 
};

//邮件事件
enum enMailEvent
{
	enMailEvent_None,					//占位
	enMailEvent_LevelUp,				//角色升级
	enMailEvent_RideLevelUp,			//坐骑升级
	enMailEven_Max,
};

//属性库抽取属性的锁定类型
enum enSmithLockType
{
	enSmithLockType_No,							//不锁定
	enSmithLockType_Type,						//锁定属性类型
	enSmithLockType_All,						//锁定全部（属性类型和属性值）
};

//生成装备的某类型属性
enum enGenerateItemAttr
{
	enGenerateItemAttr_No,						//占位
	enGenerateItemAttr_SmithAttr,				//生成装备属性->pUserItem->smithAttrs
};

//实体类型定义
enum tagEntityType
{
	enActor=0,     			//玩家
	enMonster =1,  			//怪物，具有战斗功能
	enNpc=2,				//NPC,继承于Monster，有会话功能,战斗功能
	enDropItem =3, 			//掉落的物品
	enPet =4,				//宠物
	enFire =5, 				//法师的火
	enTransfer = 6, 		//传送门
	enHero = 7, 			//英雄
	enGatherMonster = 8, 	//采集怪
};

//聊天频道的ID定义
enum tagChannelID
{
	ciChannelSecret =0, //私聊,密语 
	ciChannelNear =1,  //附近频道，同屏
	//ciChannelHorn =2, //喇叭传音频道 --放在世界频道了
	ciChannelGuild =2, //工会，帮派
	ciChannelTeam =3, //队伍频道,5人
	//ciChannelBigTeam =5, //团队,20人
	//ciChannelMap =6,  //地图，本地图可见
	//ciChannelZhenying = 8, // 阵营频道
	ciChannelWorld = 4,		//世界频道
	//ciChannelHelp= 11,		//呼救频道
	//ciChannelSell = 12,		//出售频道
	//ciChannelFriend = 13,	//好友聊天 
	//ciChannelActivity = 14,	//活动频道，如新年
	ciChannelTipmsg =5,       //系统提示
	ciChannelSystem=6,		   // 系统频道
	ciChannelIntegrated = 7, //综合频道
	ciChannelMax,		// 最大频道号 
};

enum tagActorVocation
{
	enVocNone =0,    //无职业或者任何职业

	enVocWarrior=1, //战士
	enVocMagician =2, //法师
	enVocWizard =3, //道士
	enMaxVocCount ,  //最大的职业数量
};

//tolua_end
////////////////////////////////////////////////////////////////上面这些要导出脚本的，不需要导出的往下放!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11

#define MALEACTOR  0 //男性角色
#define FEMALEACTOR 1 //女性角色

//#define MAX_ACTOR_LEVEL 255  //玩家的最大等级

#define MAX_HEADTITLE_COUNT 32 //最大头衔个数

#define MAX_WORKS 64	//快乐上班 最大副本活动数

#define MAX_RIDE_LEVEL 100	//坐骑最大等级

enum tagZhenying
{
	zyNone		= 0,	// 没有阵营
	zyWuJi		= 1,	// 无忌
	zyXiaoYao	= 2,	// 逍遥
	zyRiYue		= 3,    // 日月
	zyMax		= 4,	
};


// 一些杂项数据
enum enMiscDataType
{
	enMiscData_AddCampDailyTaskCount,	// 增加阵营日常任务次数
	enMiscData_AddDailyTaskCount,		// 增加日常任务次数(非阵营)
	enMiscDataMax
};


// 怪物发言广播类型
enum MonsterSayBCType
{
	mssInvalid,		// 无效广播类型
	mssNear,		// 附近广播
	mssScene,		// 场景广播
	mssFuben,		// 副本
	mssWorld,		// 世界
	mssCamp,		// 阵营广播

	mssSelf =10,       //发给自己,策划的需求..
	mssMax, 
};

//Messagebox是否不再显示系统ID
enum tagMsgBoxShowId
{
	mbsDefault,		//默认 没有那个选项
	mbsMagicSky,	//魔幻星宫
};

//宝石槽位
enum tagInlayHoleSlot
{
	dsInlayHole1,	//宝石槽位1
	dsInlayHole2,	//宝石槽位2
	dsInlayHole3,	//宝石槽位3
	dsInlayHole4,	//宝石槽位4
	dsInlayHole5,	//宝石槽位5
	//dsInlayHole6,	//宝石槽位6

	dsMaxInlayHole
};

enum RecordType
{
	rRecordOnLineTime	= 0,
	rRecordBeKilled		= 1,
	rRecordKillMonster	= 2,
	rRecord,
};
enum tagTeleportPassId //传送点通过条件判断id
{
	tpGuildSiege = 1,//行会攻城，占领红幡才能通过传送点
};

/** 定义物品使用限制类型的枚举类型 **/   
enum
{
	eITEM_USE_LIMIT_NULL			= 0,	//未定义类型
	eITEM_USE_LIMIT_DAY				= 1,	//武器 
	eITEM_USE_LIMIT_WEEK		    = 2,	//魔器 
	eITEM_USE_LIMIT_MONTH			= 3,   	//黑檀石 
	eITEM_USE_LIMIT_MAX,					//上限
};
