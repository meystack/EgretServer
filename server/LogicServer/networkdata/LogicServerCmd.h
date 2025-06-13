#pragma once

/****************************************************************************************/
/* 逻辑服务器与客户端之间通信协议定义                                                   
/****************************************************************************************/

//下面是一些子系统ID的定义
//tolua_begin
enum tagSystemID
{
	enDefaultEntitySystemID	= 0,  //默认的系统，如果没有子系统分派，就分派到这里
	enMoveSystemID			= 1,  //移动子系统的ID
	enPropertySystemID		= 2,  //属性子系的ID
	enObserverSystemID		= 3,  //观察者子系统的ID
	enBuffSystemID			= 4,  //Buff子系统的ID
	enSkillSystemID			= 5,  //技能子系统的ID
	enQuestSystemID			= 6,  //任务子系统的ID
	enEuipSystemID			= 7,  //装备子系统的ID
	enBagSystemID			= 8,  //背包子系统的ID   
	enChatSystemID			= 9,  //聊天子系统的ID
	enGuildSystemID			= 10, //行会子系统的ID
	enStaticCountSystemID	= 11, //计数器系统的ID
	enStoreSystemID			= 12, //商城系统的ID
	enDealSystemID			= 13, //交易子系统的ID
	enActorVarSystemID		= 14, //玩家变量系统
	enLootSystemID			= 15, //拾取子系统的ID
	enTeamSystemID			= 16, //队伍子系统的ID
	enGameSetsSystemID		= 17, //游戏设置子系统的ID
	enTreasureSystem		= 18, //宝物功能
	enBasicFunctionsSystemID= 19, //基础功能子系统
	enFubenSystemID			= 20, //副本子系统的ID
	enInteractionID			= 21, //交互子系统的ID
	enMsgSystemID			= 22, //消息子系统
	enDepotSystemID			= 23, //仓库子系统
	enPkSystemID			= 24, //PK子系统
	enActivityID	        = 25, //活动系统
	enMiscSystemID			= 26, //杂七杂八子系统，各种杂项的处理
	enConsignmentSystemID	= 27, //交易行
	enAchieveSystemID		= 28, //成就子系统
	enStrengthenSystemID	= 29, //强化系统
	enEffectSystemID		= 30, //特效子系统
	enGhostSystemID			= 31, //神魔
	enPlatforMwelfareID		= 32,//平台福利
	enCrossServerSystemID   = 33, //跨服系统
	enPetSystemID		    = 34, //宠物子系统
	enLootPetSystemID		= 35, //宠物捡取系统
	enRebateSystemID		= 36, //返利系统
	en_ID37			        = 37,
	en_ID38			        = 38,
	enStallSystemID			= 39, //摆摊子系统
	en_ID40			        = 40,
	enFriendLogicSystemID	= 41, //好友子系统
	en_ID42					= 42,
	en_ID43					= 43,
	enHeroSystemId			= 44, //英雄子系统
	en_ID15					= 45,
	en_ID46			        = 46,
	en_ID47			        = 47,
	en_ID48			        = 48,
	enBossSystemID			= 49, //BOSS子系统
	enMailSystemID			= 50, //邮件子系统
	enAlmirahSystemID		= 51, //衣橱子系统(翅膀)
	enCheckSpeedSystemID	= 52, //加速外挂子系统
	en_ID53			        = 53,
	enNewTitleSystemID		= 54, //新头衔子系统
	en_ID55			        = 55,
	en_ID56 		        = 56,
	enCombatSystemID		= 57, //职业竞技子系统
	enHallowsSystemID		= 58, //圣物系统
	enReviveDurationSystemID= 59, //复活特权系统
	enRedPoint              = 200, //红点协议
	enSystemCount,				  //逻辑部分用到的最大的

	enLoginSystemId			= 255, //登陆子系统，用于登陆的判断	
};

//杂七杂八系统的通信码
enum eMiscSystemCode
{
	cGetActorInfo =1, //获取玩家的信息
	cFcmExitGame =2,  //防沉迷点击确定需要退出游戏
	cRankingData = 4,		//请求排行榜数据
	cGetServerTime =5,     //请求服务器的时间
	CSetQQBlueDiamond = 6,//设置蓝钻
	CGetQQBlueDiamond = 7,//蓝钻特权
	CGetQQBlueDiamondGift = 8,//设置蓝钻
	cEveryDayCheck = 29,		//每日签到(没用，用的福利系统)
	cSaveGameSetData	= 30,	///< 游戏设置保存
	cLoginBYLoginTools = 31,	//是否使用登陆器登陆
	cGetActorBuffData = 33,		//请求buff数据
	cChangeNameOp = 40,					//改名字 
	cGetNearActor  = 44,			//获取附近的玩家
	cCircleCount       = 45,            //每转转生人数
	cGetActorIdInRank       = 46,            //获取在排行榜的玩家id
	cGetActorInfoByNameFuzzy	= 60,	//模糊查询在线玩家
	cBuyMonthCard = 61,	//购买月卡（byte:类型）
	cGetMonthCardAward = 62,//获取月卡奖励（byte:类型）
	cUseCdkey = 63,//使用激活码 (string:激活码)
	cBuyFrenzy = 64,//购买狂暴
	cGetMoBaiInfo = 65,//膜拜排行信息
	cGetsbkOfflineInfo = 66,//沙巴克信息
	cUpgreadOffice = 67,//升级官阶

	cLuckyTree = 68,//摇钱树
	cGetLuckyMoney = 69,//摇一摇
	cGetOnlineTime = 70,//获取累计登录时间
	cGetOnlineAward = 71,//获取累计登录奖励
	cGetSbkRank = 72,//沙巴克排行
	cGetCenterMoBaiInfo = 73,//跨服膜拜排行信息
	
	cUseNewCdkey = 74,//新的激活码 //主要是游戏猫项目


	sSendActorInfo =1, //下发玩家的信息
	sWriteFcmInfo =2, //开始填写防沉迷资料 
	sFcm3Hour =3,  //防沉迷的时间过了3小时
	sFcmLogin =4, //防沉迷的时间过了5小时
	sRankingUpdate = 5,		//新增了或减少了排行榜，客户端要重新获取
	sRankList = 6,		//发排行榜列表
	sRankData = 7,		//发排行榜数据
	sSendServerTime= 8,  //下发服务器的时间
	sSendAreaBuffData = 10,	//玩家经过一个给buff的区域时，下发这个区域的属性
	sCountDown = 11,		//给客户端发送一个倒计时
	sCreateScoreRecorder = 12,//创建一个计分器(INT64：计分器ID、STIRNG:计分器名称）
	sSetScoreRecorderItems = 13,//设置计分项的分值（INT64：计分器ID、WORD:变更的计分项的数量、STRING：计分项名称、INT：分值）
	sDeleteScoreRecorder = 14,//删除一个计分器(INT64：计分器ID）
	sAddSlave = 16, // 添加下属
	sRemoveSlave = 17, // 删除下属
	sScrEffect = 18,//播放全屏特效
	sScrEffectCode = 19,	//播放全屏特效，与消息18的区别是，这个是代码实现的特效
	sNoticeExpChange = 20,	//有玩家经验改变

	sChangeEntityModel = 26,		//改变实体模型
	sSendOpenServerDays = 28,			//下发开服第几天
	sSendGetacivityAwardTips = 29,		//下发领取奖励的提示
	sSendGameSetData			= 31,	///< 下发游戏设置的数据
	sDeleteSceneEffect = 32,			//下发删除全屏场景特效
	sSendActorBuffData = 33,			//返回buff数据

	//sSendCombinSbkData = 34,			//返回合区沙城争霸的数据
	//sSendDrayYbRankData = 35,			//返回合区充值排行榜 
	//sSendCombineAwadrdTips = 36,		//下发合区奖励的提示
	sSendCombineActGift = 34,			//[影魅合服]下发活跃度好礼情况
	sSendCombineFirstCharge=35,			//[影魅合服]下发首冲奖励领取情况
	sSendCombineConsumeRank=36,			//[影魅合服]下发土豪排行榜数据

	sSendMysticShopItem = 37,			//下发神秘商店商品
	sSendBuyMysticResult = 38,			//返回购买神秘商店的商品是否成功
	sSendBuyMysticRecord = 40,			//下发神秘商店购买记录
	//sSendTempCombineTime= 41,			//下发服务器的合区时间(用于后台设置开启某些系统用)影魅合服屏蔽
	sSendCombineSbkInfo	=41,			//[影魅合服]下发英雄霸主占领情况
	sSendCombineHuntCount = 42,			//[影魅合服]下发狩猎剩余名额
	//sOpenRedGift = 42,				//打开赠送界面
	sOpenOldPlayerBack = 43,			//打开/关闭老玩家回归图标
	sGetNearActor = 44,					//获取附近的玩家
	sCircleCount     = 45,                 //升级转生
	sGetActorIdInRank  = 46,               //返回在排行榜的玩家id
	sBeforeAuctionList = 48,			//下发温泉竞拍前数据
	sAfterAuctionList = 49,				//下发竞拍后的数据
	sSendAskJoinPool = 50,				//邀请人进入池中
	sSendHotAuctionTime = 51,			//下发竞拍还剩下的秒数

	sSendCustomEffect = 56,				//控制客户端个性特效
	sSendTurnOnOffIconFlag = 57,		//设置图标打开或关闭

	sSendActorInfoByNameFuzzy	= 60,	//下发模糊查询的在线玩家
	sGetMoBaiInfo = 65,//膜拜排行信息
	sGetsbkOfflineInfo = 66,//沙巴克信息
	sWorldLevel			=68,			//世界等级信息
	sWorldLevelClose			=69,	//世界等级信息图标关闭

	//sBanneretLeftTimes			= 71,	///< 爵位剩余名额
	sSendLackItem				= 72,	///< 缺少爵位相关物品
	//sBanneretLeftFreeHornTimes	= 73,	///< 免费传音次数
	sVestName = 77,						//下发怪物归属
	sSendPkValueWashCount = 78,			//发送洗红名次数
	sSendChangeNameResult = 79,			//发送改名结果
	sUseCdkey = 80,//使用激活码结果 (byte:结果，0成功，>1为错误码)
	sCrossAtvArea = 81,//通过活动区域 (byte:结果，0进入，1为离开)
	sGetQQBlueDiamond = 82,//蓝钻特权
	sLuckyTree = 83,//摇钱树
	sGetLuckyMoney = 84,//摇一摇
	sGetOnlineTime = 85,//获取累计登录时间
	sGetSbkRank = 86,//沙巴克排行
	sGetCenterMoBaiInfo = 87,//跨服膜拜排行信息
	
	sFcmUseOnline 		= 88,//防沉迷，登陆
	sFcmUseOffline 		= 89,//防沉迷，离线
	sFcmUseHeatbeat 	= 90,//防沉迷，心跳
	sFcmAllUseHeatbeat 	= 91,//防沉迷，心跳
	sFcmClientTip 		= 92,//防沉迷，通知客户端
	
};

enum tagBossSystemCode
{
	// cBossFubenOpt = 2,
	// cGetOneBossInfo = 3,		//请求一个boss信息
	// cGetWildBossSts = 7,		//请求野外BOSS状态
	// cGetWorldBossSts = 8,		//请求世界BOSS状态
	// cGetBossLastKillTime = 9,	//[通用方法]请求BOSS被击杀的最后时间
	// cTeleToBoss = 11,			//[通用方法]传送到某个BOSS处
	// cAnheiBossAsk = 12,			//请求boss之家boss刷新情况
	cRequestBossInfo = 1,      //请求boss信息
	cTeleToBoss = 2,			//[通用方法]传送到某个BOSS处

	sSendBossInfo = 1,
	sTeleToBoss = 2,			//[通用方法]传送到某个BOSS处
	sBossBelong = 3,			//[通用方法]boss归属
	sBossTimes = 4,				//[通用方法]boss 次数
	sSendCurPlayerNum = 5,		//当前副本中玩家人数
	sSendBossResult   = 6,		//发送打Boss结果
	// sSendBossFubenInfo = 2,
	// sSendOneBossInfo = 3,		//下发一个boss信息
	// sSendKungfuAttack = 4,		//攻击练功师怪的数据
	// sBossHome = 6,				//返回boss之家信息
	// sSendWildBossSts = 7,		//下发野外BOSS状态
	// sSendWorldBossSts = 8,		//下发世界BOSS状态
	// sSendBossLastKillTime = 9,	//[通用方法]下发BOSS被击杀的最后时间
	// sNoticeBossIcon = 10,		//[通用方法]通知客户端显示某个BOSS的头像
	// sAnheiBossAsk = 12,			//发送boss之家boss刷新情况
	//sBossSystemSendDigBossAward=13,//发送挖boss奖励
};

//装备子系统的网络命令码
enum tagEquipSystemCode
{
	//通过GUID装备 一个物品,Uint64:ItemGuid
	cTakeOnEquip=1,

	//脱下一件装备，参数是装备的GUID,Uint64:ItemGuid
	cTakeOffEquipWithGuid =2,

	//获取自身的装备数据
	cGetEquip=3,

	//获取其他玩家的装备
	cGetOtherEquip=5,

	//设置是否隐藏时装
	cSetHideFationCloth =6, 

	//查看玩家的信息
	cGetRankOtherEquip =7,
	//查看跨服玩家的信息
	cGetCenterRankOtherEquip =8,

	//查看非活动副本Boss归属玩家信息
	cGetBelongActorInfo = 15,
	//最大的ID，用于判断
	//cMaxEquipCodeID , 



	//通知客户端装备一件物品,BYTE:nPos,Uint64:ItemGuid
	sTakeOnEquip =1,

	//通知客户端脱下一件装备，BYTE:nPos
	sTakeOffEquip =2,

	//初始化玩家的装备BYTE:nCount, (BYTE:nPos, 装备的数据) nCount 个
	sGetUserEquip =3,

	//装备的耐久发生变化BYTE:nPos,unsigned int:nDua
	sEquipDataChage =4,

	//查看其他玩家的装备
	sGetOtherEqip =5, 

	// 删除一件装备(BYTE:nPos, guid)
	sDelEquip = 6,

	//查看玩家的信息
	sGetRankOtherEquip =7,
	//查看跨服玩家的信息
	sGetCenterRankOtherEquip =8,

	//返回不能回收的装备id
	sCannotRecoverEquip=9, //delete 

	//查看离线玩家英雄新消息
	sViewOfflineUserHero=10, //delete 

	sSendRingCd = 11,

	sSendReliveRingCd = 13,

	sSendBelongActorInfo = 15,//非活动副本Boss归属玩家信息
};
//帮派子系统的命令码
enum tagGuildSystemCode
{
	cGuildInfo=1,			//请求本帮派的详细信息
	cGuildMember = 2,		//帮派成员列表
	cGuildList=3,			//本服内的所有帮派
	cAddGuild = 4,			//创建帮派
	cDelGuild=5,			//删除帮派
	cJoinApplyMsgList=6,	//显示用户申请加入的帮派的申请
	cGuildBuilds = 7,      	//行会建筑
	cUpdateMemo=8,			//更新帮派公告
	cGuildDonate = 9,		//帮派捐献
	cJoinApply=10,			//用户提交加入帮派的申请
	cDeleteMember=11,		//剔除成员
	cChangeGuildPos=12,		//设置职位
	cLeaderChange=13,		//帮主让位
	cLeftGuild=14,			//离开行会
	cSetAddMemberFlag = 15,	//设置直接加入
	cGuildUpdateBuild = 16,  //升级建筑
	cDealGuildApply = 17,  //处理申请信息
	cImpeachGuildMember = 18,		//弹劾上级成员
	cDeclareWar = 19,  //宣战
	cGuildDonateInfo = 21,  //贡献信息
	cGuildWarList = 22,  //宣战列表
	cGuildLog = 23,  //行会日志
	cGuildGetSbkAward= 24,  //沙巴克领奖
	cGuildGetSbkInfo= 25,  //沙巴克信息

	sGuildInfo=1,			//本帮派的详细信息，包括帮派名称，阵营，帮主名称等等
	sGuildMember=2,			//帮派成员列表
	sGuildList=3,			//本服内的所有帮派
	sAddGuild=4,			//返回创建帮派结果
	sDelGuild=5,			//删除帮派
	sJoinApplyMsgList = 6,        //申请列表
	sGuildBuilds    = 7, 	//行会建筑
	sUpdateMemo	= 8, //更新公告
	sGuildDonate = 9,		//帮派捐献
	sJoinApply=10,			//用户提交加入帮派的申请
	sDeleteMember=11,		//剔除成员
	sChangeGuildPos=12,		//设置职位
	sLeaderChange=13,		//帮主让位
	sLeftGuild=14,			//离开行会
	sSetAddMemberFlag = 15,	//设置直接加入
	sGuildUpdateBuild = 16,  //升级建筑
	sDealGuildApply = 17,  //处理申请信息
	sImpeachGuildMember = 18,  //弹劾
	sDeclareWar = 19,  //宣战
	sApplyReslut = 20,  //申请结果
	sGuildDonateInfo = 21,  //贡献信息
	sGuildWarList = 22,  //宣战列表
	sGuildLog = 23,  //行会日志
	sGuildRedPoint = 24,  //行会红黄点   1, 红点 2： 黄点  1行会申请列表
	sGuildGetSbkAward = 25,// 领奖结果
	sGuildGetSbkInfo = 26,//沙巴克信息

};

//副本系统的消息号
enum eFubenSystemCode
{
	cEnterFuben = 1,		//申请进入副本
	cExitFuben = 2,		    //退出副本
	cGetFubenAward = 3,		//领取副本奖励

	sFubenResult = 3,		//副本结果
	sFubenRestTime = 4,		//发送副本剩余时间（当前场景的）
	sFubenSchedule = 5,		//副本进度信息
};

//商城系统通信协议号
enum tagStoreSystemCode
{

	cStoreSystemShopInfo = 1, //商城信息
	cStoreSystemBuyShop  = 2,//购买商品

	sStoreSystemShopInfo = 1, //商城信息
	sStoreSystemBuyShop  = 2,//购买商品
	sOpenStore  = 3,//打开商城
	// //从商城购买东西
	// //int:id(卖的物品的标示),int:count
	// cBuyStoreItem =1,

	// //获取商城的销售排名
	// cGetStoreRank =2,

	// // 查询可提取元宝
	// cQueryYuanBaoCount = 3,

	// /// 请求提取元宝
	// cWithdrawYuanBao = 4,

	// //设置是否购买全服播报
	// cChangeBroadcastFlag =5,

	// //获取最新的动态商品数据
	// cGetDynamicStoreData =6,

	// //获取消耗排行
	// cGetConsumeRank = 7,

	// //领取奖励
	// cGetYbConsumeAwards = 8,

	// //获取竞拍信息
	// cGetAuctionData = 9,

	// //确认出价竞拍
	// cConfirmAuction = 10,

	// //客户端点击加注按钮
	// cClickAuctionBtn = 11,

	// //赠送商城 物品
	// cGiveStoreItem = 12,

	// //购买随机npc商城物品
	// cBuyNpcShopItem = 14,

	// //一键购买
	// cOneKeyBuyItems = 15,

	// //[通用商店]购买物品
	// cShopBuyItems = 16,

	// //[通用商店]请求商店信息
	// cShopHistory = 17,

	// cFreshMisticalShop		= 19,				//[商城-神秘商店]玩家刷新神秘商店
	// cGetMisticalShopData	= 20,				//[商城-神秘商店]请求神秘商店数据
	// cGetActorStoreBuy		= 21,				//客户端请求玩家购买限购商品的数据

	// //回应购买商城物品的结果
	// sSendBuyStoreItem = 1,


	// //下发商城的销售排名 WORD:count (int:itemID,int:count)...count个
	// sSendStoreRank=2,

	// // 下发玩家可提取元宝数量
	// sSendYuanBaoCount = 3,

	// //下发动态商城的数据
	// sSendDynamicStoreData =4,

	// //广播分类刷新
	// sBroadLebelRefresh =5,

	// //广播商品数目变化
	// sBroadMerchanCountChange =6,

	// //返回消耗排行
	// sSendConsumeRank = 7,

	// //下发领取奖励的提示
	// sSendGetAwardsNotice = 8,

	// //下发竞拍物品信息
	// sSendAuctionItemData = 9,

	// //下发竞拍信息
	// sSendAuctionMsg = 10,

	// //下发显示竞拍加注提示按钮
	// sSendAuctionTips = 11,

	// //下发显示竞拍开始提示按钮
	// sSendAuctionStartTips = 12,

	// //下发随机npc商城物品
	// sBuyNpcShopItem = 13,

	// sSendMisticalShopData	= 20,				//[商城-神秘商店]下发神秘商店数据
	// sSendActorStoreBuy		= 21,				//下发玩家购买限购商品的数量
};

//移动系统的命令码
enum tagMoveSystemCode
{

	//玩家走路，移动包((WORD Dir,WORD: nPosx,WORD:nPosy) )
	cMove = 1,
	
	//玩家跑((WORD Dir,WORD: nPosx,WORD:nPosy) )
	cRun =2,

	//跳跃
	cJump =3,

	//转向
	cTurnAround=4,

	//使用小飞鞋
	cFlyShoes = 5 ,

	//NPC传送
	cNpcTransform=6,

	sAutoPath = 1,		//通知前端自动寻路到npc
	sFlyShoes = 5, //通知客户端飞鞋传送打开npcid
	//NPC传送
	sNpcTransform=6, //npc传送
};


//默认系统的，没有具体系统的
enum tagDefaultEntitySystemCode
{

	//login to the game (int accountid,int: actorid)
	cLogin =1,

	//心跳包 unsigned int: tickCount
	cHeartbeat =2,  

	//选择目标实体 unsigned int 64: 目标实体的handle
	cSetTargetEntity =3, 

	//设置鼠标的点中场景的位置 WORD:x,WORD :y
	cSetMousePos =4, 

	// 与npc对话
	cNpcTalk = 5,

	cMessageBox =6 ,		//用户点击弹出对话框的按钮h后的返回消息
	cTestHeartBeat = 7,		// 测试心跳包

	cClientGetLoginDays = 8, //客户端请求登录天数

	cCommonTelePort  = 9,	//通用传送
	
	cAppKeepAlive = 20, // 网关和逻辑服务器的应用层心跳包
	cCheckTime = 21,

	cChooseRelive = 22, // 请求复活

	cReqRankData = 23,	// 请求排行榜数据

	cClientSendfcmInfo = 24, //客户端发送平台防沉迷信息

	cYYLogin = 25, // BYTE:用户类型（2为YY大厅用户）

	cCallMember = 26,	// 召唤成员 Byte（1队伍2工会）
	
	cAgreeCall = 27,	// 同意召唤 Byte（1队伍2工会）

	cChangeVoc = 28,//改变职业（Byte:职业类型(1战士2法师3道士)，Byte:性别(0男1女)）

	cGetPcClientDownLoadGift = 29,//

	c360Login = 30,//360登录

	c7GameLogin = 31,//7游戏登录
	
	c37GameLogin = 32,//37游戏登录

 
	//login if fail,return the errorcode (BYTE: nErrorCode) 
	sUserLogin =1,


	//实体出现,这里不会是玩家,是NPC,逻辑物件等等
	sEntityAppear = 2,

	//创建主角
	sCreateMainActor = 3,


	//别的玩家出现在视野 
	sOtherActorAppear =4,

	//实体消失
	sEntityDisappear =5,


	//实体的属性改变(非主角)
	sEntityPropertyChange =6, //

	//主角的属性发生了改变
	sMainactorPropertyChange =7,


	//重新设置主角的坐标
	sResetMainActorPos =8,



	//其他实体的移动,Uint64 :handle,WORD: nPosx,WORD:nPosy) 
	sOtherEntityMove = 9,

	//心跳包
	sHeartbeat =10,  

	// 下发npc的对话内容
	sNpcTalk = 11,

	////通知客户端打开一个窗口
	sOpenDlg = 12,

	//主角传送或者切换场景,string:场景名字(sceneName), string:地图名字(mapName),WORD: nSceneID,WORD: nPosX,WORD: nPosY
	sEnterScene= 13,

	//通知客户端，玩家传送过来了，需要播放特效 Uint64 :handle,WORD: nPosx,WORD:nPosy 
	sActorTransport=14,

	//其他实体的跑,Uint64 :handle,WORD: nPosx,WORD:nPosy) 
	sOtherEntityRun =15,

	//开始技能的吟唱 
	sStartSingSpell =16, 

	//结束吟唱
	sEndSingSpell =17,

	//开始释放技能(Uint64: 施法者的handle,word:技能id, byte: 技能等级)
	sStartSpellSkill =18,

	//给目标添加一个特效 Uint64:施法者的handle,Uint64:目标的handle,BYTE: 特效的类型, WORD: 特效的ID, int: 特效剩余的时间，单位ms
	sAddEffect =19,

	//实体收到攻击，播放受击动作 Uint64: 受击者的handle
	sSkillHit =20,

	//下发敌对帮派的列表
	sWarGuildList = 21,

	sMessageBox = 22,	//要求客户端显示一个弹出对话框，对话框可包含多个按钮，用户点击某个按钮后将执行脚本函数

	//角色跳跃,Uint64 :handle,WORD: nPosx,WORD:nPosy,BYTE:nDir) 
	sEntityJump = 23,

	//公共操作的结果
	sOperatorResult =24, 

	//转向
	sTurnAround =25,

	//近身攻击
	sNearAttack =26 ,//近身攻击

	//瞬间移动
	sInstatnceMove =27 ,

	//转职结果（0成功）
	sChangeVocResult =28,

	//闪避
	sDoDge =29, 

	//冲锋
	sRapidMove =30,

	//怪物重用
	sReuse = 31,

	//添加场景特效
	sAddSceneEffect=32,

	sNPCQuestState = 33,//提示npc身上是否有可接任务或者可完成任务

	sLostTarget=34, //失去目标

	sEntityDeath =35, //实体死亡

	sGather = 36,  // 玩家采集怪

	sUpdateActorNameClr = 37, // 更新角色名称颜色（可以是自己，也可以是周围其他玩家）

	sConsecutiveKillCDChanged = 38, // 玩家连斩CDTime改变

	sDelEffect =39,     //删除特效

	sChangeShowName =40,    //改变显示的名字

	sSceneAreaAttri = 41,	//发送区域属性

	sScreenShake=42, //屏幕振动

	sBabyPropChange=43, // 下属属性改变

	sBabyPosChange=44, // 下属位置改变

	sChangeAttackType =48,  //宠物改变攻击类型
	
	sTestHeartBeatAck = 49, // 测试心跳包应答消息
	sBeatFly =50,         //被击飞
	sStartTransmit = 51, // 开始传送
	sOpenCommonServer= 52, // 下发跨服开启状态

	sSpecialEntityAppear =53,       //一些特殊的生物出现

	
	sMoveAhead=54,   //按一点的速度往前面移动若干格

	sMoveBack = 55, //按一点的速度后退
	
	sResetOtherEntityPos = 56, //重设一个实体的坐标

	sSendCultureInfo = 57,		//文明度的信息

	sFoundTarget =58,           //发现目标，用于怪物的声音播放

	sDogDeath = 59,				//道士狗死亡通知客户端挂机

	sHeadCountDown = 60,				//头顶倒计时

	sTargetRapidMove = 61,			//实体冲锋到一个坐标点位置

	sNoticeBuyItems = 62,			//确认购买一个物品

	sSendShortCutMsg = 63,			//快捷栏上消息的（不消失）
	sNotEnoughAward	= 64,			//奖励，货币，物品等不足的提示，对应客户端获取途径
	sSendLoginDays = 65,			//下发登录天数

	sEnterSceneEffect = 66,	// 实体的传送特效

	sMonsterAppear = 67, //怪物出现
	sPetAppear = 68, 	//宠物出现
	sNPCAppear = 69, 	//NPC出现

	sMoneyChange = 70,	//金钱变化

	sSendRankData = 71, //发送排行榜数据

	sActorReliveInfo = 72, //玩家复活信息 id(byte),limit(byte),num(byte),n1(byte),n2{byte}...
	sActorRelive = 73, //玩家复活 aid
	sMemberCall = 74, //成员召唤

	sPcClientDownLoadState = 75, //微端领奖

	sActorForbid = 251,//封角色
	sKictout = 252,//踢玩家
	sCheckTimeBack = 253,
	sPrintDebugText = 254,			//下发测试文本
	sFcmKictout = 255,			//防沉迷踢玩家
};



//buff系统的消息
enum tagBuffSystemCode
{
	//增加buff,Uint64:EntityHandle, BYTE:type,BYTE:Group,DWORD:dwRestTime,BYTE:boIsNewBuff
	sAddBuff =1,

	//删除一个buff Uint64:EntityHandle, BYTE:type,BYTE:Group
	sDelBuff =2,
	
	//Del buff by type ID. Uint64:EntityHandle, BYTE:type,
	sDelBuffType =3,

	//初始化主角的buff数据
	sInitMainActorBuffData =4, 

	//库存有改变
	sChangeBuffVallue = 5,
	//库存有改变
	sChangeBuffCdTime = 6,
};

enum tagChatSystemCode
{
	//客户端上发聊天信息(BYTE nChannelID,String: msg )
	cSendChat = 1,
	
	cShowChat =3, //展示

	cGmBroadCast =5, //GM发送世界公告
	cGetChatShowItem =6, //获取展示结果

	cSendHelp	= 8,		//求救
	
	//请求跨服消息
	cSendReqCsChat = 11,

	cSendClearMsgRefresh = 12,//清屏刷新


	//服务器下发聊天消息(BYTE nChannelID,string :Name, string: msg)

	sSendChat =1,
	//下发的系统系统提示信息(BYTE:showPos,string:msg)
	sSendTipmsg =2,

    scShowChat =3, //展示

	sChatResult = 4, //返回聊天结果
	sGetChatShowItem =6, //获取展示结果

	sNonPlayerChatNear = 7, // 非玩家附近聊天

	sGmBroadCast =8,  //GM世界公告

	sSendSubmitResult = 10,	//返回游戏建议提交的结果
 
	//请求跨服消息
	sSendReqCsChat = 11,
	sSendClearMsg = 12,//清屏
};

//任务子系统的命令码
enum tagQuestSystemCode
{
	cQueryQuest = 1,///查询所有的正在进行的任务的数据
	cChangetQuestState = 2,///修改任务状态
	cQuestTel = 5,///传送

	sQueryQuest = 1,	//查询所有的正在进行的任务的数据
	sChangetQuestState = 2, // 接取任务
	sDeleteQuest = 3, // 删除任务
	sAddQuest = 4, // 新增一条任务
	sQuestTel = 5,///传送
	sChangeQuestState = 6, // 状态变化
};

enum tagBagSystemCode
{
	enBagSystemcAddItem =1,			    //添加物品
	enBagSystemcQueryItem =2,			//查找玩家的物品列表
	enBagSystemcDelItem =3,			    //删除一个物品
	enBagSystemcBagSplit = 5,			//拆分背包Uint64: itemGuild, WORD:物品的数量
	enBagSystemcBagMerge =6,			//背包物品叠加 Uint64: srcGuid, Uint64: tgtGuid
	enBagSystemscArrangeBagOver=7,		//整理背包
	enBagSystemcUseItem =8,			    //使用物品  Uint64: itemGuid
	// enBagSystemcArrangeBag = 9,		//整理背包(自动合并数量)
	encExChangeMoney         = 9,        //兑换金币
	enRecoverItem             = 10,       //回收道具
	enBagSystemcBatchUseItem =11,		//批量使用
	
	cBagSystemQueryItemsExtraInfo = 12, 	//道具特殊属性
	cBagSystemQueryItemsExtraInfoOne = 13,//道具特殊属性一个

 	//enBagSystemcBagMergeBindInfect= 10,	//背包物品叠加（绑定感染）
	enBagSystemcMaxBagCodeID,			//背包系统消息号最大值


	enBagSystemsAddItem=1,					// 服务器下发，添加物品
	enBagSystemsInitBagItem =2,				///初始化玩家的背包物品 WORD:count 后面接信息
	enBagSystemsDelItem =3,		           //服务器下发删除一个物品,Uint64: itemGuild
	enBagSystemsItemCountChange =4 ,		//物品的数量发生改变 Uint64 itemGuild,WORD:nNewCount 
	enBagSystemsSendBagEnlargeFee=5,		//获取背包扩展需要的费用 int:fee 扩容需要的金钱的数量	
	enBagSystemsItemInfoChange =6,			//装备的信息发生变化
	enBagSystemsUseItemResult=7, 			//使用物品的结果word:itemID, byte:result
	enBagSystemsItemTimeUp = 8,			//一个物品的时间到期，被系统收回了(Uint64 物品系列号，BYTE：0表示背包中，1表示身上装备，2表示仓库中）
	enBagSystemsBagGetNewItem =9 ,			//背包添加新的道具
	enBagSystemsArrangeBagOver=10,			//整理背包(自动合并数量)完成
	enBagSystemsRecoverResult=11,			//回收结果
	sBagSystemQueryItemsExtraInfo = 12, 	//道具特殊属性
	sBagSystemQueryItemsExtraInfoOne = 13,//道具特殊属性一个
};

//技能系统的命令码
enum eSkillSystemCode
{
	//初始化玩家的技能 BYTE:count,( WORD:nSkillID,unsigned char:nLevel,unsigned char:nLearnedSecretSauce,unsigned int nExpOrCd) count个
	sInitUserSkill =1,
	
	//服务器下发升级技能的结果 WORD:nSkillID, BYTE:nNewLevel
	sTrainSkillResult=2,

	
	//技能的秘籍学习nSecretSauceID表示玩家学习了哪个秘籍1,2,3,4 WORD:nSkillID, BYTE:nSecretSauceID
	sLearnMiji =3,

	//技能的经验发生改变
	sSkillExpChange =4,

	//下发一个技能的cd时间
	sSendSkillCd =5,

	sAttackByActor =6, //受到其他玩家的PK掉血

	sSelfDamageOther =7, //自身给目标造成了伤害

	sTargetAbsort =8, //目标吸收了伤害

	sSetSkillOpen =9, //停止或者启用一个技能

	sDeleteMiji =10, //删除技能的秘籍

	sForgetSkill =11,  //遗忘一个技能，

	sSetSkillCdTime =12, //设置技能的冷却时间

	sSetSkillSingTime =13, //设置技能的吟唱时间
	
	sGatherMonsterProgress =14, //设置采集怪物进度条进度

	sPlayGuide = 15,	//客户端播放引导性特效

	sSoldierSoulSkillStateChange = 16,//兵魂技能激活状态改变
	
	sSkillSystemPetCountChange=17,//召唤宠物技能的数量发生变化



//////////////////////////////////////////////////

	//客户端请求下发技能的数据
	cGetSkill =1,

	//使用技能,WORD:nSkillID
	cUseSkill =2,   

	//升级技能,WORD:nSkillID
	cTrainSkill =3,

	//客户端同步一个技能的CD
	cGetCd =4, 

	cSetDefaultSkill =5, //设置默认技能

	cNearAttack =6, //近身攻击

	cStartSing =7, //客户端开始吟唱技能

	cStartGather = 8, // 客户端开始采集怪

	cDeleteMiji =9, //删除技能的秘籍

	cSetSkillOpen =10, //设置技能开启

	cLearnMiji =11, //学习秘籍

	cAddSkillExp = 17,		//增加技能熟练度

	cOpenSkill = 18,	// 开启技能

	cCloseSkill = 19,	// 关闭技能

	cMaxSkillCodeCount, //最大的上发的数目,这个没意义
};

//交易系统通信协议号
enum eDealSystemCode
{
	cDealSystemRequestDeal = 1,	//发起交易(HANDLE：实体ID)
	cDealSystemReplyDeal = 2,		//回应交易请求(HANDLE：交易申请人实体ID，bool：是否接受交易请求)
	cDealSystemAddDealItem = 3,	//添加交易物品(ItemSeries：交易物品系列号)
	cDealSystemAddNumber = 4,//改变交易金钱数量(INT:金钱数量)
	cDealSystemLockDeal = 5,		//锁定交易
	cDealSystemCancelDeal = 6,	//取消交易
	cDealSystemConfimDeal = 7,	//确认交易


	sDealSystemDealRequest = 1,	//发送交易请求(HANDLE：交易申请人实体ID，String：交易申请人名称)
	sDealSystemDealRefused = 2,	//交易被拒绝([String：对方名称])
	sDealSystemStartDeal = 3,		//开始交易(HANDLE：交易对方实体ID，String：交易对方名称，WORD：交易对方等级)
	sDealSystemAddDealItemRet = 4,//返回添加交易物品结果(bool：添加成功否)
	sDealSystemTargetAddDealItem=5,//交易对方添加物品(CUserItem：物品数据)
	sDealSystemChangeDealCoinRet=6,//返回改变交易金钱数量结果(bool：改变成功否，INT：当前我交易的金钱数量)
	sDealSystemTargetChgDealCoin=7,//交易对方改变交易金钱数量(INT：金钱数量)
	sDealSystemLockDeal = 8,		//交易锁定状态变更(bool：我是否锁定，bool：对方是否锁定)
	sDealSystemDealCanceled = 9,	//交易已被取消
	// sDealSystemDealNotLocked = 9,//交易尚未锁定
	sDealSystemDealComplete = 10,	//交易完成
};

//拾取系统通信协议号
enum eLootSystemCode
{
	cLootItem = 1,	//拾取背包里的一个物品, unsigned int: nDropBagID,WORD:itemID
	
	cLootMoney =2, //拾取背包里的金钱, unsigned int: nDropBagID
	
	cLootAll =3,    //拾取背包里的全部的金钱和物品  unsigned int: nDropBagID

	cQueryDropBag =4, //获取掉落包裹的信息  unsigned int: nDropBagID

	//cLootQuestItem =5, //拾取任务物品， WORD: 任务物品的ID
	
	cRollItem =5, //Roll一件物品 Uint64 :itemGUID, BYTE: Roll的方式

	cStartRollItem =6, //发起Roll一件物品,物品的ID,包裹的ID

	cCaptinAssignedItem =7, //队长分配一件物品

	cCloseDropBag =8,  //玩家关闭包裹
	
	cLootDropItem =9,  //拾取掉落物品，(传奇版本)
	cPetLootDropItem =10,  //拾取掉落物品，(传奇版本)
	

	//增加一个掉落包裹,unsigned int: nDropBagID, BYTE:是否是属于队伍的(队伍的1，个人的是0)
    //WORD:nSceneID, int: posX, int:posY 
	
	sAddDropBag = 1,	

	sDeleteDropBag=2,  // 删除一个掉落包裹,unsigned int :nDropBagID

	sDeleteDropBagMoney =3, //删除掉落包裹里的金钱 unsigned int :nDropBagID

	sDeleteDropBagItem =4,  // 删除背包的里物品 unsigned int :nDropBagID, Uint64: nItemGuid 

	sSendDropBagInfo =5, // 发送掉落包裹的信息 //unsigned int nDropBagID,int:金钱数量,BYTE:nItemCount  ( CUserItem:物品的数据) nItemCount个

	sDeleteQuestItem =6,  //删除一件任务物品,WORD: ItemID

	sDropBagStateChange =7, //包裹的状态发生改变 unsigned int:nDropBagID BYTE:state

	sDropItemStateChange =8, //包裹物品的状态发生改变 unsigned int:nDropBagID, WORD:物品的ID, BYTE:物品的状态 unsigned int: nActorID
	//打开了筛子的窗口 unsigned int:nbagid,word: 物品的ID,Byte:quality,bytestrong
	sOpenLootDialog =9,   

	sDropItemAppear =10,  //掉落物品出现(传奇版本)

	sDropItemDisAppear =11,  //掉落物品消失(传奇版本)
	sLootDropItem  =12,  //拾取掉落物品(传奇版本)
	sPetLootDropItem =13,  //拾取掉落物品，(传奇版本)
}; 

/// 队伍系统
enum eTeamSystemCode
{
	enTeamSyetemcGetMemberData      = 1,    ///请求数据
	enTeamSystemcInviteJoinTeam		= 2,	///< 邀请加入队伍
	enTeamSystemcLeaveTeam			= 3,    ///< 退出队伍
	enTeamSystemcApplyJoinTeam		= 4,    ///< 申请加入队伍
	enTeamSystemcSetCaptin			= 5,    ///< 设置一个人为队长
	enTeamSystemcKickMember			= 6,	///< 踢出一个玩家
	enTeamSystemcDestroyTeam		= 7,    ///< 解散队伍
	enTeamSystemcApplyJoinTeamReply = 8,	///< 队长回复申请入队
	//enTeamSystemcInviteJoinTeamReply= 9,	///< 回复邀请入队	
	enTeamSystemcGetAllowTeam       = 9,      //获取字段状态
	enTeamsystemcIsAutoTeam         = 10,   //是否自动组队
	enTeamSystemcAskNearTeamInfo	= 11,	///< 获得附近的队伍
	// enTeamSystemcCreateTeam			= 10,	///< 创建队伍
	// enTeamSystemcGetNearActors		= 11,	///< 获得附近的角色

	enTeamSystemsInitTeam			= 1,	///< 初始化队伍成员列表
	enTeamSystemsAddMember			= 2,    ///< 添加一个成员
	enTeamSystemsDelMember			= 3,    ///< 删除一个成员
	enTeamSystemsApplyJoinTeam		= 4,    ///< 玩家申请加入队伍
	enTeamSystemsSetCaptin			= 5,    ///< 设置一个人为队长
	enTeamSystemsMemberLogout		= 6,    ///< 一个玩家退出队伍
	enTeamSystemsDestroyTeam		= 7,	///解散队伍
	enTeamSystemsGetAllowTeam       = 9,    //返回字段状态
	enTeamSystemsSendNearTeamInfo	= 11,	///< 发送附近的队伍信息
	enTeamSystemsMemOnlineState     = 12,   //玩家上下线
	enTeamSystemsUpdateMember		= 13,	//更新队员信息(等级，hp)
	
	enTeamSystemsSendNearActorInfo	= 100,	///< 发送附近的角色信息
	enTeamSystemsSendBuffIds		= 101,	///< 发送buff id
	enTeamSystemsMemberMove			= 102,	//广播自己的坐标或场景的变化
};



//pk系统的消息号
enum ePkSystemCode
{
	cInviteMatch = 1,		//邀请切磋
	cInviteResult = 2,		//玩家回复邀请
	cSetFreePkMode = 3,		//设置自由PK模式
	cOnFriendIntercede = 4, //帮人求情
	cIntercedeMineral = 5,  //上缴水晶

	sSendInvite = 1,		//服务器下发邀请的消息
	sStartMatch = 2,		//给双方下发开始切磋的消息
	sSetFreePkMode = 3,		//发送玩家的pK模式
	sKillHe = 4,			//B 原处于 非战斗状态， A 攻击 B 令其进入 战斗状态时， 才会弹出 打他丫滴！ 窗口。
};

enum eActivityCode
{
	cActivityCommonOperator = 1, 	//通用操作请求
	cReqActivityData = 2,			//请求获取某个活动的数据
	cReqYYHallFreshManGift = 3,		//请求领取YY大厅新手礼包
	cReqYYHallLoginGift = 4,		//请求领取YY大厅登录礼包（byte:第n个）
	cReqYYHallLevelGift = 5,		//请求领取YY大厅等级礼包（byte:第n个）
	cReqYYHallNobleGift = 6,		//请求领取YY大厅贵族礼包（byte:第n个）
	cReqYYVIPNewSrvGift = 7,		//请求领取YY VIP新服豪礼（byte:第n个）
	cReqYYVIPDailyGift = 8,			//请求领取YY VIP每日礼包（byte:第n个）
	cReqYYVIPWeeklyGift = 9,		//请求领取YY VIP每周礼包（byte:第n个）
	cReqSuperVipV1_V3   = 10,		//请求超玩v1-v3礼包      (byte:第n个)
	cReqSuperVipDaily   = 11,		//请求超玩每日礼包        (byte:第n个)

	cReqActorTypesScores= 15,		//获取 秘境打宝 玩家的秘境宝箱数量(积分)、字诀宝箱数量(积分)、材料宝箱数量(积分)

	sSendPersonActivity = 1, 		//发送个人活动开启数据
	sSendGlobalActivity = 2, 		//发送全局活动开启数据
	sSendPersonActivityUpdate = 3,	//更新个人活动开启数据
	sSendGlobalActivityUpdate = 4,	//更新全局活动开启数据
	sSendActivityEnd = 5,			//活动结束
	sActivityOperator = 6, 			//通用操作返回
	SActivityResult = 7,			//活动结果或旗帜
	SActivityPopup = 8,				//活动弹框提示
	sYYHallData = 9,				//推送YY大厅数据
	sYYVipData = 10,				//推送YY VIP数据
	sSupperVipData = 11,			//推送超玩VIP数据
	sSendCompletPlayerNum = 12,		//逃脱试炼活动，已经逃脱的人数

	sSendActorTypesScores = 15,		//发送 秘境打宝 玩家的秘境宝箱数量(积分)、字诀宝箱数量(积分)、材料宝箱数量(积分)
};

//仓库子系统
enum eDeportSystemCode
{
	cDeportGetItems=1, //获取仓库的物品列表
	cDeportItemBag2Deport=2,   //把一个物品从背包拖放到仓库
	cDeportItemDepot2Bag=3,   // 把一个物品从仓库拖放到背包
	cDepotRomove =4,      //删除仓库物品
	cDepoBuy = 5,			//买仓库页
	cDeportArrangeBagOver = 6,//整理仓库
	
	cMaxDepotCodeID, //最大的背包的数量]

	sDeportSendItems=1, // 下发仓库的物品的列表
	sDeportAddItem =2, //仓库获得物品
	sDeportDelItem =3, //仓库删除物品
	sDeportItemCountChange= 4, //仓库的物品数量发生改变
	sDepotSendGridNum = 5,	//下发仓库格子数量
	sDeportArrangeBagOver = 6,//
	
};

enum eMsgSystemCode
{
	cReadMsg = 1,			//玩家看了一条消息

	sAddMsg = 1,		//发送给客户端的消息
	sDeleteMsg = 2,			//删除一条消息
	sResultMsg = 3,		//返回处理结果
	sMoveToMsg = 4,		//下发移动寻径消息
};

enum enAchieveSystemCode
{
	// cGetAchieveData=1,    //获取成就的数据
	// cGetAchieveAwards=2,  //获取成就的奖励
	
	// cGetTitleData =3,     //获取当前的称号数据
	// cSetTitle =4,        //设置当

	// cGetBabgeData = 5,		//获取徽章的数据
	// cExChangeBadge = 6,		//兑换成就

	// sSendAchieveData=1,   //下发成就的数据
	// sFinishAchieve=2,     //完成一个成就
	// sAchieveEventTrigger =3, //成就一个事件触发了
	// sAchieveGetAwardsResult=4, //成就领取奖励的结果
	// sSendTitleData =5, //下发称号的数据
	// sSendGetTitle =6, //获得一个称号
	// sSendLostTitle =7, //失去一个称号
	// sSendBabgeData = 8,		//下发徽章列表
	// /////////////////////////////////////////////
	// cGetNewAchieveAwards=10,		//获取成就的奖励
	// sSendNewAchieveData = 10,		//下发成就的数据
	// sSendNewAchieveResult = 11,		//成就领取奖励的结果

	cGetAchieveData=1,    //获取成就的数据
	cGetAchieveAwards=2,  //获取成就的奖励
	cPushAchieveRedPoint = 3,//请求红点
	sSendAchieveData=1,    //获取成就的数据
	sGetAchieveAwards  =2,    //领取一个成就
	sPushAchieveRedPoint  =3,    //完成一个成就

	cGetMedalData = 11,	  //获取勋章的等级信息
	cUpMedalLevel = 12,   //升级勋章等级
	
	sSendMedalData = 11,	  //下发勋章的等级信息
	sUpMedalLevelResult = 12,   //升级结果
};

enum enPetSystemCode
{
	cPetSetBattleState =1,         //设置战斗状态,1:呆在原地，2，跟随玩家
	cRecallPet = 2,				   //召回宠物
	cRemoveBattlePet = 3,          //宠物消失

	sPetCount = 1,				//更新宠物数量
};


enum enHeroSystemCode
{
	cHeroGetList =1,              //获取英雄列表
	cHeroStageUp = 2,             //英雄升阶
	cHeroSkillLevelUp = 3,        //英雄技能升级
	cHeroChangeName = 4,          //英雄改名
	cSetHeroState = 5,            //英雄出战或休息
	cViewHero  = 6,               //查看英雄

	sHeroList =1,                 //下发英雄列表
	sHeroSkillList =2,            //下发英雄技能列表
	sHeroUpdate =3,               //英雄数据创建或更新
	sHeroSkillUpdate  = 4,        //技能数据创建或更新
	sHeroStateChange =5,          //英雄状态变更
	sHeroExpChange = 6,           //英雄的经验更新
	sHeroView =7,                 //查看英雄
	sHeroOpResult =8,             //英雄操作的结果
};

enum enTeacherSystemCode
{
	cSearchMastersOrPupils = 1,		//寻求师傅/徒弟
	cTeacherSysOpt		   = 2,		//师徒相关操作
	cLoadTeacherInfo		= 3,	//加载师徒信息
	cExchangeBonus			= 4,	//兑换红利
	cExtendPupilSlot		= 5,	//扩展徒弟栏位

	sSendSearchResult		= 1,	//下发师傅/徒弟查询列表
	sSendTeacherOptResult	= 2,	//师徒相关操作结果
	sLoadTeacherList		= 3,	//加载师傅/徒弟列表
	sUpdateBonus			= 4,	//更新徒弟的红利
	sExtendPupilSlot		= 5,	//扩展徒弟栏位
};

//传奇协议

enum enStallSystemCode
{
	cStartStall = 1,				//开始摆摊
	cEndStall = 2,					//结束摆摊
	cLeaveMsg = 3,					//留言
	cBuyStallItem = 4,				//购买物品
	cViewStall = 5,					//查看摊位
	cSendAd = 6,					//发送广告
	cStallItemOpt = 7,				//摊位物品操作
	cCanStall = 8,					//查看是否可以摆摊


	sStallInfo = 1,					//下发摊位信息
	sAddLeaveMsg = 2,					//添加留言
	sAddBuyLog = 3,					//添加购买记录
	sExtendGrid = 4,				//扩展摊位
	sStallResult = 5,					//下发结束摆摊
	sStallBroadCast = 6,				//下发广播
	sStallItemOpt = 7,					//下发摊位物品操作结果
	sNoticeSaleItem = 8,				//下发已出售的物品
	sStallAreaResult = 9,				//下发是否是摆摊区域
};

//好友协议
enum enFriendSystemCode
{
	cAddFriend = 1,					//添加好友等
	cAddResult = 2,					//是否同意添加好友等
	cGetSocialList = 3,				//获取关系列表
	cAddBackList = 4,               //添加黑名单
	cAddfollow = 5,                 //添加关注
	cSetFollowColor = 6,            //设置关注颜色
	cDeleteFriend = 7,              //删除好友
	cReportList = 8,              //删除好友
	cAllAcceptApply = 9,              //全部接受
	// cChangeFriendState = 9,         //修改状态

	//cTrackActor = 5,				//追踪玩家

	

	//cFriendChat = 9,				//好友聊天
	// cSendConsumeCoin = 10,			//发送坐标扣除金币
	// cGetCanApplyMasterList = 11,	//请求可以拜师或者可以收徒的列表
	// cSendMarryResuly = 12,			//是否同意结婚
	// cGetIdentTimes = 13,			//huoqu鉴定次数
	// cGetTrackInfo = 14,				//获取追踪的信息
	// cSetMood = 15,					//设置心情
	// cIWillRevenge = 16,				//我要复仇（点击仇人面板的‘我要复仇’）
	// cGetOnlineFrdTeamSt = 17,		//获取在线好友是否有队伍
	// cFastRemoveFriends = 20,		//一键删除好友
	// cGetSocialStsList = 21,			//请求好友状态
	// cDivorceConfirm  = 23,			//协议离婚确认

	// cGetLinkManLately = 29,			//请求最近联系人信息
	// cGetTrackActorFee = 30,			//请求追踪玩家的消耗

	// cGetActorDeathInfo = 33,		//请求玩家死亡数据

	// cFriendCallRequest = 34,		//召唤好友
	// cFriendCallResult = 35,			//回应召唤	

	// sReturnAddToOther = 1,			//返回添加好友等需要对方同意
	// sSendAddFriend = 2,				//返回添加好友等的信息
	sSendSocialList = 3,			//返回关系列表
	// sSendCanAddFriendList = 4,		//下发推荐的好友列表
	// sSendTrackInfo = 5,				//返回追踪的信息
	// sSendAlarmInfo = 6,				//发送报警信息
	// sSendmood = 7,					//返回心情
	sApplyFriend = 8,                 //申请好友
	sDeleteFriend = 7,                //删除好友
	sReportList = 9,              //战表列表

	// sSendUpdateList = 8,			//下发需要更新的关系列表
	// sSendFriendChat = 9,			//返回好友聊天消息
	// sSendApplyMasterList = 11,		//下发可以拜师或者可以收徒的列表
	// sRevengeBeginTime   = 17,		//‘我要复仇’的开始时间
	// sSendOnlineFrdTeamSt = 18,		//下发在线好友组队状态
	// sSendSocialStsList = 21,		//下发好友状态
	// sSendAskMarryResult = 22,		//发送求婚请求结果给求婚者
	// sSendDivorceApply  = 23,		//协议离婚请求
	// sDivorceOk  = 24,				//离婚成功，参数1-协议离婚，2-强制离婚
	// sSendLinkManLately = 29,		//发送最近联系人信息
	// sSendTrackActorFee = 30,		//下发追踪玩家的消耗

	// cSendActorDeathInfo = 33,		//下发玩家死亡数据
	// sFriendCallRequest = 34,		//被召唤
	// sRecommendFriend = 36,		//推荐好友
};

//登陆子系统上发的消息
enum tagLoginSystemCode
{
	
	cCheckUserPasswd =1, //发包检测用户名 密码
	cCreateAccount =2,   //创建账户，客户端不用发,用于测试
	cGetUserList=3,      //获取玩家列表
	cCreateActor =4,     //创建角色
	cDeleteActor =5,      //删除用户
	cRandName = 6,        //获取随机的名字
	cCheckSecondPsw = 7,
	cCreateSecondPsw = 8,
	cCrossServerInit = 9, //跨服数据初始化
	
	
	cHeatBeat=128,        //心跳包

	sLoginErrorCode=1,   //向客户端返回登陆的错误
	sGetUserList =2,     //向客户端下发用户列表
	sCreateActor =3,     //创建角色
	sDeleteActor =4,     //删除角色
	sRandNameActor =5,   //随机名字
	sCheckSecondPswResult = 6,
	sCreateSecondPswRessult = 7,
	sOtherLoginGame = 8,//顶号了
	sCrossServerInit = 9, //跨服数据初始化
};

enum tagMailSystemCode
{
	cMailSystemMailOpt = 1,			//邮件操作
	cMailSystemDeleteMails = 2,		//删除邮件
	cMailSystemGetMailItems = 3,    //收取附件
	cMailSystemDeleteMailAll = 4,	//删除所有邮件
	cMailSystemGetMailItemsAll =5,	//提取所有邮件附件
	cMailSystemSomeMails = 6,		//删除数量邮件

	sMailSystemSendAllMail = 1,		//发送所有邮件信息(登录下发)
	sMailSystemNewMail = 2,			//新邮件到达
	sMailSystemDeleteMails = 3,		//确认删除一封邮件
	sMailSystemSendMailItems = 4,	//确认收取附件的邮件
	sMailSystemSomeMails = 6,		//删除数量邮件
};

//检测外挂加速
enum tagCheckSpeedCode
{
	cSendSpeedTimeData = 1,			//客户单发送时间校验包

	sSendSpeedTimeData = 1,			//服务器下发时间校验包
};

//新头衔系统
enum tagNewTitleCode
{
	cClientGetNewTitleData = 1, //客户端请求
	cClientCutOverNewTitle = 2,//客户端请求切换称号
	cClientGetCustomTitleData = 3, //客户端请求自定义称号数据
	cClientCutOverCustomTitle = 4,//客户端请求切换自定义称号

	sSendNewTitleData = 1, //返回所有头衔数据
	sSendAddNewTitleData = 2,//添加一个头衔
	sSendDelNewTitleData = 3,//删除一个头衔
	sSendAddTimeTitleData = 4,//续期一个头衔
	sSendCustomTitleData = 5, //返回所有自定义称号数据
	sSendAddCustomTitleData = 6,//添加一个自定义称号
	sSendDelCustomTitleData = 7,//删除一个自定义称号
	sSendAddTimeCustomTitleData = 8,//续期一个自定义称号
};

enum tagAlmirahSystemCode
{
	cGetInfo = 1,			//请求列表
	cTakeOnAndOff = 2,	//时装操作 1穿上，0脱下
	cActivation = 3,		//激活翅膀
	cUpdateFashion = 4,		//升级

	sSendInfo = 1,		//下发翅膀列表	
	sTakeOnAndOff = 2,	//穿戴返回
	sActivation = 3,	//翅膀激活返回
	sUpdateFashion = 4,		//升级
};	

//寄卖系统的消息号
enum eConsignmentSystemCode
{
	cConsignmentSystemGetAllConsign			= 1,	//获取交易行的寄卖物品
	cConsignmentSystemGetMyConsign			= 2,	//获取本人的正在寄卖物品
	cConsignmentSystemBuyConsign			= 3,	//购买物品
	cConsignmentSystemAddConsign			= 4,	//上架
	cConsignmentSystemGetSellItem			= 6,	//下架
	cConsignmentSystemGetMoney				= 7,	//领取收益
	cConsignmentSystemAward			        = 8,	//奖励

	sConsignmentSystemGetAllConsign			= 1,	//获取交易行的寄卖物品
	sConsignmentSystemGetMyConsign			= 2,	//获取本人的正在寄卖物品
	sConsignmentSystemBuyConsign			= 3,	//购买物品
	sConsignmentSystemAddConsign			= 4,	//上架
	sConsignmentSystemGetSellItem			= 6,	//下架
	sConsignmentSystemGetMoney				= 7,	//领取收益
	sConsignmentSystemAward			        = 8,	//奖励
	sConsignmentSystemRedPoint			    = 9,	//红点提示
};

//职业竞技子系统协议
enum tagCombatSystem
{
	cGetCombatInfo				= 1,		//获取竞技基本信息
	cSelectCombatPayer			= 2,		//选择竞技对手
	cCombatStart				= 3,		//发起竞技
	cClearCooldown				= 4,		//清除冷却时间
	cRechargeAwrad				= 5,		//兑换奖励
	cGetCombatLog				= 6,		//请求战报
	cGetCombatRank				= 7,		//请求竞技排行榜
	cGetShopInfo				= 8,		//请求兑换商店信息

	sSendCombatInfo				= 1,		//返回竞技基本信息
	sSendSelectResult			= 2,		//返回竞技对手
	sCombatResult				= 3,		//返回竞技结果
	sClearResult				= 4,		//返回清除结果
	sRechargeResult				= 5,		//返回兑换结构
	sSendCombatLog				= 6,		//返回战报
	sSendCombatRank				= 7,		//返回排行榜
	sSendShopInfo				= 8,		//返回兑换商店信息
};

/** 名称服务器操作返回值定义 **/
typedef enum tagLogicServerOPError
{
	neSuccess = 0,			//操作成功
}LOGICSERVER_OPERROR;

//通用传送
enum   tagTelePort
{
	enDefaultTelePort = 0,		//默认
	enQuestTelePort = 1,		//任务
	enRecommendedUpgrade = 3,		//推荐升级传送
};

//游戏设置子系统协议
enum tagGameSetsSystem
{
	cGetGameSetsInfo		        = 1,		//获取游戏设置信息
	cUpdateKeyBoards     			= 2,		//设置快捷键
	cDiscardKeyBoards     			= 3,		//丢弃
	cSetGameCheck					= 4,		//设置 游戏设置的选项（打钩）
	cSetGameValue  					= 5,		//设置 游戏设置得值
	cSetGameReset  					= 6,		//重置
	cGameSetOneKeyItem				= 7,		//物品一键设置 

	cGetAppGameSetsInfo		        = 11,		//app
	cUpdateAppKeyBoards     		= 12,		//App设置快捷键 手机端5个格子
	cDiscardAppKeyBoards    		= 13,		//丢弃 App设置
	
	sSendGameKeyBoardnfo			= 1,		//返回游戏按键信息
	sSendKeyBoardsInfo				= 2,		//返回快捷键信息
	sSendGameSetInfo				= 3,		//发送 游戏设置信息 //没有格子信息
	sSendOneGameSetInfo				= 4,		//发送 一个页签的游戏设置信息

	sSendAppGameKeyBoardsInfo		= 11,		//App设置快捷键 返回快捷键信息 手机端5个格子 
	sSendAppKeyBoardsInfo			= 12,		//返回快捷键信息
};


enum tagStaticCountSystem
{
	enStaticCountSystemcExchangeCount   = 1,   //兑换次数
	enStaticCountSystemcUpCircle        = 2,   //转生升级
	enStaticCountSystemcExchangeSoul    = 3,   //兑换修为
	enStaticCountSystemcMeridiansUPLv   = 4,   //经脉升级


	enStaticCountSystemExchangeCount    = 1,   //兑换次数--返回
	enStaticCountSystemsUpCircle        = 2,   //转生升级--返回
	enStaticCountSystemsExchangeSoul    = 3,   //兑换修为--返回
	enStaticCountSystemsMeridiansUPLv   = 4,   //经脉升级
};


enum tagBasicFunctionsSystem
{
	encompostItem             = 1,       //合成道具
	enForgeItem               =	2,       //锻造
	enForgeItemId             = 3,       //本服锻造id
	enRefining            	  = 4,       //洗炼
	enRefiningReplace      	  = 5,       //洗炼替换
	enYBRecover      	 	  = 6,       //回收
	enYBRecoverInfo      	  = 7,       //回收数据
	enItemUpStar      	      = 8,       //升星
	enCompostResult             =	1,	     //合成结果
	enForgeResult             =	2,	     //锻造结果
	enForgeItemIdResult       = 3,	     //本服锻造id
	enRefiningResult          = 4,       //洗炼结果
	enRefiningReplaceResult   = 5,       //洗炼替换结果
	enYBRecoverResult         = 6,       //回收结果
	enYBRecoverInfoResult     = 7,       //回收数据结果
	enItemUpStarResult        = 8,       //升星
};

enum TagenTreasureSystem
{
	encBless                    = 1,    //祝福
	
	ensBless                    = 1,    //祝福返回
};

enum StrengthenSystem
{
	cGetStrengthenSystemInfo      = 1, //请求数据
	cUpStrengthenSystemInfo      = 2, //升级
	
	sGetStrengthenSystemInfo      = 1, //请求数据
	sUpStrengthenSystemInfo      = 2, //升级
};

enum HallowsSystem
{
	cGetHallowshenSystemInfo      	= 1, //请求数据
	cUpHallowshenSystemInfo      	= 2, //升级
	cRefiningHallowsSystemInfo      = 3, //洗炼
	cRefiningReplaceHallowsSystemInfo = 4, //洗炼替换
	  
	sGetHallowshenSystemInfo      = 1, //请求数据
	sUpHallowshenSystemInfo      = 2, //升级
	sRefiningHallowsSystemInfo      = 3, //洗炼
	sRefiningReplaceHallowsSystemInfo = 4, //洗炼替换结果 
};
enum GhostSystemID
{
	cGetGHostInfos =1, //数据
	cUpGhostLv = 2, //升级

	sGetGHostInfos = 1,//
	sUpGhostLv = 2,//
};

enum PlatforMwelfareID
{
	cGet4366Infos = 1,//请求4366平台福利数据
	cGet4366LoginAward = 2,//
	cGet4366PhoneAward = 3,//请求4366平台福利数据
	cGet4366IdCardAward = 4,//请求4366平台福利数据
	c4366LoginType = 5,//请求4366平台登录
	c4366LoginTypeGift = 6,//请求微端登录礼包


	cGetQQHallInfos = 7,//请求qq平台福利数据
	cGetQQHallRegisteAward = 8,//请求qq平台注册
	cGetQQqHallLevelAward = 9,//请求qq平台成长
	cGetQQqHallActiveAward = 10,//请求qq平台活跃

	cReq360UserPrivilegeGift = 11,	//请求 360大玩家特权 礼包

	cMasterLuLogin = 12,	//请求鲁大师登录
	cReqMasterLuAward = 13,	//请求鲁大师奖励
	cReq360Gift = 14,	    //请求 360 礼包

	
	c37Logon = 15,					//37平台登陆 
	cUse37DownloadBox = 16,			//37盒子下载礼包
	cUse37GiftFCM = 17,				//37防沉迷
	cUse37GiftPhone = 18,			//37手机礼包
	cUse37GiftDaily = 19,			//37每日礼包
	cUse37LevelDaily = 20,			//37等级礼包
 
	cSogouLogon = 24,				//搜狗登陆 
	cSogouGiftSkin = 25,			//搜狗皮肤登陆 
	cSogouGiftRookie = 26,			//搜狗新手礼包 
	cSogouGiftLogin = 27,			//搜狗登陆礼包 
	cSogouGiftLevel = 28,			//搜狗等级礼包 
	cSogouGiftPhone = 29,			//搜狗手机绑定礼包 

	cReqku25Login = 35,
	cReqku25Gift = 36,				//请求 ku25 奖励
	cReqQiDianLogin = 37,
	cReqQiDianGift = 38,			//请求 QiDian 奖励
	cReqAiQiYiLogin = 39,
	cReqAiQiYiGift = 40,			//请求 AiQiYi 奖励
	cReqYaoDouLogin = 41,
	cReqYaoDouGift = 42,			//请求 YaoDou 奖励
	cReqTanWanLogin = 43,
	cReqTanWanGift = 44,			//请求 贪玩 奖励
	cReqGame2Login = 45,
	cReqGame2Gift = 46,				//请求 哥们 奖励
	cReq2144Login = 47,
	cReq2144Gift = 48,				//请求 2144 奖励

	cReqKuaiWanLogin = 50,
	cReqKuaiWanGift = 51,				//请求 快玩 奖励

	cReqSWLogin = 55,
	cReqSWGift = 56,				//请求 顺网 奖励

	cReqXunWanLogin = 60,
	cReqXunWanGift = 61,			//请求 迅玩 奖励

	sGet4366Infos = 1,				//请求4366平台福利数据
	sGetQQhallInfos = 2,			//请求qq平台福利数据

	sSend360UserPrivilegeData = 3, 	//推送 360大玩家特权 数据

	sSend7GameGitfData = 4, 		// 7游戏 微信礼包 
	
	sSendMasterLuGitfData = 5, 		// 鲁大师 数据  


	sSend37GameGitfData = 15, 		// 37游戏数据反馈全部
	sUse37DownloadBox = 16,			//37盒子下载礼包
	sUse37GiftFCM = 17,				//37防沉迷
	sUse37GiftPhone = 18,			//37手机礼包
	sUse37GiftDaily = 19,			//37每日礼包
	sUse37LevelDaily = 20,			//37等级礼包 
 
	sSogouLogonData = 24,			//搜狗登陆数据 
	sSogouGiftSkin = 25,			//搜狗皮肤礼包获取
	sSogouGiftRookie = 26,			//搜狗新手礼包 
	sSogouGiftLogin = 27,			//搜狗登陆礼包 
	sSogouGiftLevel = 28,			//搜狗等级礼包 
	sSogouGiftPhone = 29,			//搜狗手机绑定礼包 

	sSendku25UserData = 35,     // 发送ku25 数据
	sSendQiDianUserData = 38,     // 发送QiDian 数据
	sSendAiQiYiUserData = 40,     // 发送AiQiYi 数据
	sSendYaoDouUserData = 42,     // 发送YaoDou 数据
	cSendTanWanGift = 44,		  // 发送贪玩 数据
	cSendTaGame2Gift = 46,		  // 发送哥们 数据
	cSend2144UserData = 48,		  // 发送2144 数据
	cSendKuaiWanUserData = 51,	  // 发送快玩 数据
	cSendSWUserData = 56,		  // 发送顺网 数据
	cSendXunWanUserData = 61,	  // 发送迅玩 数据
};

enum CrossServerSystemID
{
	cReqInit = 1,//获取跨服标识唯一id
	cReqLogin = 2, //请求登录跨服

	sReqInit = 1,//
	sReqLogin = 2, //请求登录跨服
};

//新头衔系统
enum LootPetSystemID
{
	cClientGetAllLootPets = 1, //客户端请求
	cClientEquiptLootPet = 2,//客户端请求切换

	sClientGetAllLootPets = 1, //返回所有头衔数据
	sClientAddLootPet = 2,//添加一个
	sSendDelLootPetData = 3,//删除一个
	sSendAddLootPetTimeData = 4,//续期一个
};


//tolua_end

#define MAX_TEAM_MEMBER_COUNT		11			//队伍里的最大人数，队伍最多5人
#define MAX_OFFLINE_MEMBER_COUNT	10			//最大的不在线玩家的数量	


