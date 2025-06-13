#pragma once

/****************************************************************************************/
/* 服务端之间通信协议定义                                                   
/* 命名规范：
/* namespace 服务端名称Proto
/* {
/*    enum e目标服务端名称Code
/*    {
/*       //通信消息号注释，数据段格式（数据名称：数据类型【byte,short,int,shortString,string】）
/*       c消息号别名 = 消息号值，其中c表示通信双方中的客户端方发送，从1开始
/*       //通信消息号注释，数据段格式（数据名称：数据类型【byte,short,int,shortString,string】）
/*       s消息号别名 = 消息号值，其中s表示通信双方中的服务端方发送，从1开始
/*    };
/* };
/****************************************************************************************/

namespace jxInterSrvComm
{
	namespace NameServerProto
	{
		enum eNameServerCode
		{
			//发送通信心跳包
			cKeepAlive = 1,
			//申请角色ID（角色名称：string）
			cAllocCharId,
			//申请帮会ID（帮会名称：string）
			cAllocGuildId,
			//申请重命名角色（角色ID：int，角色新名称：string）
			cRenameChar,
			//申请重命名帮会（帮会ID：int，帮会新名称：string）
			cRenameGuild,
			//申请跨服角色ID（角色名称：string）
			cCrossAllocCharId,

			//发送通信心跳包
			sKeepAlive = 1,
			//返回申请角色ID结果（角色名称：string，操作错误号：int(NAMESERVER_OPERROR)，角色分配的ID值：int（仅当操作成功时有意义））
			sAllocCharId,
			//返回申请帮会ID结果（角色名称：string，操作错误号：int(NAMESERVER_OPERROR)，帮会分配的ID值：int（仅当操作成功时有意义））
			sAllocGuildId,
			//返回申请重命名角色结果（角色新名称：string，操作错误号：int(NAMESERVER_OPERROR)）
			sRenameChar,
			//返回申请重命名帮会结果（帮会新名称：string，操作错误号：int(NAMESERVER_OPERROR)）
			sRenameGuild,
			//跨服角色ID结果
			sCrossAllocCharId,
		};

		/** 名称服务器操作返回值定义 **/
		typedef enum tagNameServerOPError
		{
			neSuccess = 0,			//操作成功
			neInvalidName = 1,		//名称无效，名称中包含非法字符或长度不合法
			neDataBaseError = 2,	//名称服务器数据库操作错误
			neDataBaseCallError = 3,//名称服务器数据库调用错误
			neNameinuse = 4,		//名称已被使用
			neDBlimitCreate = 5,		//禁止创建角色
			neTimedOut = 0xFFFF,	//操作超时（本地定义的错误，非名称服务器返回的错误）
		}NAMESERVER_OPERROR;
	}
		
	namespace SessionServerProto
	{
		enum eSessionServerCode
		{
			//发送通信心跳包
			cKeepAlive = 0,

			//返回是否在线的数据包(nSessionID:int, nIsOnline:BYTE)
			cQueryResult =1,
			
			//关闭会话连接(nSessionID: int)
			cCloseSession =2,

			//向session服务器更新状态(nSessionId:int, state:int )
			cUpdateSession =3,

			//逻辑服务器向会话服务器发送在线人数
			cOnlineCount = 4,
			//获取用户的活动背包数据
			cUserItemList = 5,
			//用户领取活动背包
			cGetUserItem = 6,
			//删除活动物品
			cDeleteUserItem = 7,

			//增值序列号
			cAddValueCard = 8,

			// 查询增值卡信息
			cQueryAddValueCard = 9,
			// 查询元宝数量
			cQueryYuanbaoCount = 10,
			// 提取元宝
			cWithdrawYuanbao = 11,
			//确认已经收到了
			cConformOpenSession =12,
			// 请求传送
			cRequestTransmit = 13,

			//战区里派发
			cSendGroupMessage = 14,

			//向所有的服广播或者发给特定服
			cBroadcastOr1SrvMessage = 15,

			// 保存战力排行榜
			cSaveCsRank = 16,
			// 获取跨服战力排行榜数据
			cLoadCsRank = 17,
			// 清除跨服排行榜
			cClearCsRank = 18,

			//逻辑服发来的，准备检测用户名和密码
			cLogicCheckPasswd =19,
			
			//逻辑服发来的，创建一个账户
			cLogicCreateAccount =20,
			
			//设置公告服的ID
			cSetCommonServerId =21, 

			//转发后台消息给会话
			cSendMgrMsgToSession = 22,

			//用于腾讯平台的消息
			cQQMsg=23,

			//设置中心会话服的连接
			CSendCommonPlatformInfo = 24,

			//请求传送到跨平台服务器
			CTransToCommonPlatform = 25,

			
			//发送通信心跳包
			sKeepAlive = 0,
			//打开一个连接,（session数据: GLOBALSESSIONOPENDATA )
			sOpenSession =1, 
			//关闭一个连接(nSessionId:int )
			sCloseSession =2,
			//更新状态(nSessionId:int, state:int )
			sUpdateSession=3,
			//查询是否session在线(nSessionID: int)
			sQuerySessionExist =4,

			//返回用户的活动背包数据
			sUserItemList = 5,
			//用户领取活动背包的结果
			sGetUserItem = 6,
			//删除活动物品
			sDeleteUserItem = 7,

			//增值序列号
			sAddValueCard= 8,
			// 查询增值卡
			sQueryAddValueCard = 9,
			// 查询元宝数量
			sQueryYuanbaoCount = 10,
			// 提取元宝
			sWithdrawYuanbao = 11,
			// 请求传送应答消息
			sReqestRransmitAck = 12,

			// 获取跨服战力排行榜数据
			sLoadCsRank = 13,
			
			//检测账户的结果
			sCheckPasswdResult = 14,
			
			//创建账户的结果
			sCreateAccountResult = 15,

			//转发从其他逻辑服来的数据
			sOtherLogicMessage =16,

			// 转发战区的消息
			sRecvGroupMessage = 17,

			//踢掉在线的跨服用户
			sKickCrossServerUser=18,

			//腾讯相关消息的返回 （集市任务等）
			sQQMsg=19,

			//请求跨平台传送回答
			sReqestPlatformAck = 20,

			//集市任务相关
			sTaskAck = 21,
			//模拟检测账户的结果
			sSimulateCheckPasswdResult = 25,

		};

		/** 跨服消息定义 **/
		enum// eForwardLogicMsgCmd
		{
			fcDefault = 0,	// 默认
			fcBroadcastTipmsg,	// 跨服发布全服系统提示
			fcRankUpdate,	// 跨服Ranking更新
			fcWorldBroadCastMsg,	// 跨服世界广播消息包
			fcScriptData,	// 脚本数据包
			fcCrossBossDie,		//跨服boss被杀广播
			fcCrossWorldFirstOne,		//跨服天下第一战神
			fcCrossASbkOwnMsg,			//跨服沙巴克占领消息
			fcCrossPosToServer,			//发送跨服沙巴克职位给各服务器
			fcBroadToCrossSbkGuild,		//广播可以攻沙的跨服的行会
			fcBroadSetCrossSbkPos,		//广播设置跨服沙巴克职位
			fcSendCrossBattlefieldSignupInfoToCommonServer, ///< 向跨服发送降魔战场的报名信息
			fcBroadCrossBattlefieldSignupInfo,	///< 降魔战场广播报名信息
		};

		/** 其他逻辑服消息发送定义 **/
		enum// eOtherLogicMsgCmd
		{
			dmDefault = 0,

			lcLoadOfflineData,	//请求装载离线信息
			lcSaveOfflineData,	//返回离线信息

			lcBroadToSaveCsRank,	//广播所有逻辑服将雕像榜单数据发到session
			lcBroadToLoadCsRank,	//广播所有逻辑服重新取下榜单

			//lcBroadHundredServerCsRank,  //广播百服排行
			//lcClearHundredServerCsRank, //广播清除所有百服排行

			//lcBroadDoule12LotteryPointCsRank,  //广播百服排行
			//lcBroadNHLotteryPointCsRank,  //广播百服排行

			//lcSendGuildToForwardLogic,		//发送行会列表到跨服服务器

			//lcBroadTreasureAuctionCsRank,	//发送珍宝拍卖排行榜
		};

		/** 名称服务器操作返回值定义 **/
		typedef enum tagSeServerOPError
		{
			neSuccess = 0,			//操作成功
			neSSErr,				//数据库执行错误
		}SESSION_SERVER_OPERROR;
	}

	namespace DbServerProto
	{
		/**数据服务器和逻辑服务器的命令码**/
		enum// tagDBRecvCmd
		{
			dcDefault,			//默认处理，0
			dcQuery=1,			//查询数据
			dcSave = 2,				//保存数据
			dcLoadActor = 3,		//加载用户认证信息，3
			dcLoadQuest = 4,         //装置任务数据
			dcSaveQuest = 5,		//保存用户的任务数据
			dcLoadSkill = 6,        //装置技能数据
			dcSaveSkill = 7,        //保持技能数据
			dcLoadOtherGameSets = 8,                 //load游戏快捷键设置
			dcSaveOtherGameSets = 9,                 //save游戏快捷键设置
			dcLoadBagItem = 10,      //装载背包物品
			dcSaveBagItem = 11,      //保存背包物品数据
			dcLoadEquipItem = 12,    //装载装备物品
			dcSaveEquipItem = 13,    //保存装备物品
			dcLoadDepotItem = 14,    //装载仓库物品
			dcSaveDepotItem = 15,    //保存仓库物品，13
			dcLoadVar = 16,			//加载角色变量
			dcSaveVar = 17,			//保存角色变量
			dcQuerySubSystemData = 18,  //装载其他子系统的数据，在各子系统全部都创建完毕以后，向数据服务器发送该数据包
			dcLoadCircleCount = 19, //装载转生次数总总人数
			dcLoadStaticCount = 20, //装载玩家计数器
			dcSaveStaticCount = 21, //保存玩家计数器
			dcLoadActiveBag,	//加载角色的活动背包
			dcGetActiveItem,	//获取活动物品
			dcDeleteActiveItem,	//删除活动物品			
			dcLoadHeroItem,     //装载英雄的物品
			dcSaveHeroItem,    //保存英雄的物品
			dcLogout,			// 玩家下线
			dcLoadGuildList,	//帮派组件读入所有帮派的数据
			dcLoadGuildDetail,	//帮派的详细数据
			dcLoadGuildApplyLists = 30, //帮派申请列表
			dcDealGuildApply, //处理申请
			dcLoadActorApplyResult, //加载玩家申请行会结果
			dcSaveActorApplyResult, //保存玩家申请行会结果
			dcAddGuild,			//增加帮派
			dcDeleteGuild,		//解散帮派
			dcSaveGuild = 36,		//保存帮派的信息
			dcAddMember,		//增加帮派成员
			dcSaveActorGuild,	//保存个人帮派信息
			dcLoadActorGuild,	//读入个人帮派信息
			dcDeleteMember = 40,		//删除某个成员
			dcAddMemberOffLine,	//帮派增加一个不在线的成员,数据服务器要返回这个成员的详细数据
			dcInitGuildSkill,	//初始化帮派技能
			dcLoadGuildSkill,	//增加帮派技能的详细数据
			dcSaveGuildEvent,	//保存帮派事件
			dcLoadGuildEvent,	//初始化帮派事件
			dcSaveGuildDepotRecord,		//保存行会仓库的操作记录
			dcLoadGuildDepotRecord,		//加载行会仓库操作记录
			dcSaveGuildDepotItem,	//存储行会仓库物品到数据库
			dcLoadGuildDepotItem,	//加载行会仓库物品
			dcSaveGuildSkill = 50,	//保存帮派技能信息
			dcLoadAchieveMent, //加载玩家成就数据
			dcSaveAchieveMent, //保存玩家成就数据
			dcLoadMsg,				//读取用户的消息数据
			dcDeleteMsg,			//删除某条消息
			dcAddMsg,				//增加用户一条信息		
			dcAddMsgByActorName,	// 增加用户一条信息（通过玩家名称）		
			dcLoadHeroData,        //装载宠物的数据
			dcLoadHeroSkillData,   //装载宠物的技能的数据
			dcSaveHeroData,       //保存宠物的数据
			dcSaveHeroSkillData,     //保存宠物的技能数据		
			dcLoadBaseRank,	//读取有关头衔的排行榜数据
			dcSaveBaseRank, //刷新头衔排行榜数据
			dcSaveBaseRankCS, //刷新跨服排行榜数据
			dcLoadPetDetailData,  //装载宠物的装备/技能等具体数据
			dcSaveRelation,			//保存师徒信息
			dcLoadRelation,			//加载师徒信息

			dcLoadOfflineUserInfo,    //装载离线玩家的信息
			dcLoadOfflineUserBasic,   //装载离线玩家的基本信息
			dcLoadOfflineUserOther,  //装载离线玩家的装备信息
			dcSaveOfflineInfo,	 // 保持玩家离线信息
			dcLoadOfflineUserHeroInfo ,    //装载离线玩家英雄的信息
			dcLoadOfflineUserHeroBasic,   //装载离线玩家英雄的基本信息
			dcLoadOfflineUserHeroEquips,  //装载离线玩家英雄的装备信息 TO DELETE

			dcUpdateChallengeData,	// 更新擂台信息
			dcLoadCombatRankInfo,	//从db获取战力排行的信息
			dcLoadCmobatGameInfo,	//加载职业宗师的基本信息
			dcSaveCombatGameInfo,	//保存职业宗师的基本信息
			dcLoadCombatRecord,		//加载职业宗师战报
			dcSaveCombatRecord,		//保存职业宗师战报
			dcUpdateCombatBaseInfo,	//刷新个人排名的奖励排名
			
			
			dcLoadFriendsData,		//加载好友等关系的数据
			dcSaveFriendsData,		//保存好友数据
			dcDeleteFriendData,		//删除好友数据
			dcAddFriendChatMsg,			//保存离线聊天消息
			dcLoadFriendChatMsg,		//加载离线聊天消息

			dcAddFilterWordsMsg,		//后台添加屏蔽字到表里
			dcBackRemoveItem,			//后台删除物品
			dcBackRemoveMoney,			//台后删除金钱

			dcLoadGameSetData,			//加载游戏设置的数据
			dcSaveGameSetData,			//保存游戏设置的数据

			dcLoadTopLevelActor,		//取各职业排名第一名的玩家
			dcLoadCombinLevelActor,		//合区等级大于60级的玩家
			dcLoadCombinBattleActor,		//合区战力排行前10名的玩家
			dcLoadActorConsumeRank,		//取消耗元宝排名前三的玩家
			dcLoadTopicBattleData,		//取各职业排名第一的玩家
			dcLoadTopicHeroData,		//取英雄前三名的玩家
			dcLoadAllActivityData,		//获取全民奖励的数据
			dcLoadSwingLevelData,		//取翅膀排名前三的玩家

			dcReloadCrossConfig,		//重新加载跨服配置

			dcQueryActorExists,			//查询角色名是否存在

			dcSendDbServerStopCmd,		//发送关闭数据服的消息
			dcUpdateActorState,			//设置玩家状态

			dcLoadActorBuildingData,	//加载个人行会建筑的数据

			dcLoadMail,						//加载邮件
			dcSaveMail,						//保存邮件
			dcSaveOfflineMail,				//保存离线邮件

			dcClearActor,					//清理角色

			dcLoadFriendOffline,			//加载N天内没上线的好友信息
			dcLoadAlmirahItem,    //装载衣橱物品
			dcSaveAlmirahItem,    //保存衣橱物品
			dcLoadNewTitleData,				//加载新头衔数据
			dcSaveNewTitleData,				//保存新头衔数据
			dcLoadDeathData,			//加载角色死亡的数据
			dcSaveDeathData,			//保存角色死亡的数据
			dcLoadDeathDropData,			//加载角色死亡掉落的数据
			dcSaveDeathDropData,			//保存角色死亡掉落的数据
			dcLoadTopLevelActorData,	//读取等级最高玩家信息(不读转数了)

			dcSaveConsignmentItem,		//保存交易行(actorid= 0的数据),
			dcLoadConsignmentItem,		//读取交易行(寄卖数据)
			dcSaveActorConsignment,		//保存交易行数据(按玩家id cache)
			dcLoadConsignmentIncome,		//读取交易行收益
			dcLoadNewServerMail,			//读取新的全服邮件
			dcSaveNewServerMail,			//保存全服邮件
			dcAddNewServerMail,			//保存全服邮件

			dcLoadActivityList,		// 加载活动数据
			dcSaveActivityList,		// 保存活动数据

			dcLoadPersonActivity,	// 加载个人活动数据
			dcSavePersonActivity,	// 保存个人活动数据

			dcLoadBossInfo, //保存boss信息 --策划要求 需要记录 所以存db ---不用了
			dcSaveBossInfo, //加载boss信息

			dcLoadActorStrengthenInfo, //保存强化信息
			dcSaveActorStrengthenInfo, //加载强化信息
			
			dcLoadofflineAchieve,		// 加载离线玩家的成就信息
			dcSaveofflineAchieve,		// 保存离线玩家的成就信息
			dcLoadGhostInfo,		// 加载神魔
			dcSaveGhostInfo,		// 保存神魔

			dcLoadCrossGuildId,		//跨服行会id
			dcLoadLootPetData,		// 加载宠物
			dcSaveLootPetData,		// 保存宠物

			dcLoadRebateData,		//加载返利活动
			dcLoadNextDayRebateData,// 加载 次日发放 返利活动
			dcUpdateRebateData,		// 更新返利活动	
			dcSaveRebateData,		// 保存返利活动
			dcClearRebateData,		// 清理返利活动

			dcSaveCustomInfo,	 	// 保持玩家自定义信息
			dcLoadCustomInfo,		// 查询玩家自定义信息
			dcLoadCustomInfoUnForbid,		// 查询玩家自定义信息
			dcSaveCustomInfoUnForbid,		// 
			//-------------------------------------------------------------
			dcInitDB=150,				//初始化数据库，特殊指令
			dcResActorDbSessionData,  //回复数据服务器，玩家逻辑服务器的验证数据已经收到
			dcSetGambleFlag,          //设置赌博系统是否关闭
			dcUpdateGateUserInfo,	  // 更新网关用户信息
			
			dcLoadFee,			//加载充值信息
			dcClearFee,			//清除充值信息
			dcNoticeFee,		//加载所有充值信息
			dcBackChangeTradingQuota, //后台修改交易额度

			dcLoadCustomTitleData,				//加载自定义称号数据
			dcSaveCustomTitleData,				//保存自定义称号数据

			dcLoadCustomInfoCustomTitleId,			// 查询玩家 自定义称号 自定义信息
			dcSaveCustomInfoCustomTitleId,			// 保存玩家 自定义称号 自定义信息
			//////////////////////////////////////////////////////////////////////////
			// 选择角色和进入游戏用到的一些DB请求定义，DBCenter用到
			// DataCenter --> DataBase 
			DC2DB_QueryActorList = 200,			// 查询账号角色列表
			DC2DB_EnterGame,					// 请求进入用户			

			//////////////////////////////////////////////////////////////////////////
			// DataBase  --> DataCenter
			DB2DC_QueryActorListAck = 200,		// 查询账号角色列表Ack
			DB2DC_EnterGameAck,					// 请求进入用户Ack						
			DB2DC_UpdateLogicClientList,		// 更新DBServer连接的逻辑客户端列表

			//下面是用于登陆的，一个比较大的修改
			dcGetActorList =500,               //获取玩家角色列表
			dcCreateActor,                     //创建玩家
			dcDeleteActor,                     //删除角色
			dcGetRandName,                     //获取随机的名字
			dcChangeActorName,					//玩家改名字
			dcCheckSecondPsw,
			dcCreateSecondPsw,
			dcGetCSActorId, 					//获取玩家跨服id
			dcCreateCSActorId, 					//获取玩家跨服id
			dcLoginMsgMax,                     //最大的装载消息包

			dcLoadHallowsSystemInfo, 			//圣物保存信息
			dcSaveHallowsSystemInfo, 			//
			dcLoadReviveDurationData, 	//加载复活特权信息
			dcSaveReviveDurationData, 	//保存复活特权信息
		};
	
		// 通信的错误码定义
		enum// tagResultError
		{
			reSucc = 0,
			reDbErr,		//数据库执行错误
			reNoActor,		//没有该角色的属性信息
			reSaveErr,		//保存属性失败
			reNoLoad,		//还没有读入数据
			reDbNotConnected,  //DB连接不上
			reParamError,      //参数错误
			reNameError,	//名字不合法
			reNameInUse,	//名字被使用
			reTimeOut,		//操作超时
			reInterErr,		//内部错误
			reNoHero,		//没有英雄信息
		};
	}

	namespace AsyncOperatorCode
	{
		enum// tagAsyncOpError
		{
			soeSucc = 0,			// 成功
			soeDBErr,				// db执行错误（或者db无法连接）
			soeNoCardInfo,			// 不存在此卡信息
			soeTimeExpired,			// 查询超时
		};
	}
	namespace CrossServerProto
	{
		enum// eCrossServerCode
		{
			//发送通信心跳包
			cKeepAlive = 0,

			//返回是否在线的数据包
			cQueryResult =1,
			
			//关闭会话连接(nSessionID: int)
			cCloseSession =2,
			
			//逻辑服发来的，登录数据
			cSendCrossData =4,
			//逻辑服发来的，登录
			cReqCrossLogin =5,
			//删除玩家
			cSendCloseActor =6,
			//获取玩家在跨服的id
			cGetCrossActorId = 7,
			cSendCSMail = 8,//
 
			cCSGetRankList = 9,//跨服排行
			cGetActorOfflineData = 10,//获取玩家数据
			//逻辑服，请求系统聊天消息
			cSendReqChat = 12,  
			
			//发送通信心跳包
			sKeepAlive = 0,
			//逻辑服发来的，登录
			sReqLogin =5,
			sGetCrossActorId = 7,//
			sSendCSMail = 8,//发送邮件 

			sCSGetRankList = 9,//
			sGetActorOfflineData = 10,//获取玩家数据
			
			//跨服发送消息，广播到各服
			sSendBroadTipmsg = 11, 
			//逻辑服，请求系统聊天消息
			sSendReqChat = 12, 
			//跨服发送系统消息，广播到各服
			sSendBroadSysTipmsg = 13, 
		};
	}
}
