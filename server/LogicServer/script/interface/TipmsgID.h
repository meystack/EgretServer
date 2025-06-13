#pragma once
//tolua_begin
enum tagTigMsgID
{
	tmNoError=0,					//没有错误
	tmNoMoreCoin=1,					//金币不足
	tmNomoreYubao=2,				//元宝不足
	tmMpLimited=3,					//魔法不足
	tmLevelLimited=4,				//等级不足
	tmExpLimited=5,					//经验不足
	tmCommonCDLimited=6,			//操作过快
	tmDistanceLimited=7,			//距离过远
	tmGetGold,						//获得金币%u
	tmGetYB,						//获得元宝%u
	tmGetBindYB,					//获得绑元%u
	tmLostGold=11,					//失去金币%u
	tmLostYB,						//失去元宝%u
	tmLostBindYB,					//失去绑元%u
	tmGetExp,						//获得经验%lld点
	tmTarCantAttack,				//该目标不能够攻击
	tmInSkillCD=16,					//技能正在冷却中
	tmLvlCantLearnSkill,			//等级不足,无法学习技能
	tmVocCantLearnSkill,			//职业不符,无法学习技能
	tmCirCantLearnSkill,			//转数不足,无法学习技能
	tmItemCantLearnSkill,			//道具不足,无法学习技能
	tmSuccLearnSkill=21,			//您成功学会技能:%s
	tmSuccUpgrSkill,				//您成功将技能%s提升至%d级
	tmOpenSkill_3,					//刺杀剑术已开启
	tmCloseSkill_3,					//刺杀剑术已关闭
	tmOpenSkill_4,					//半月弯刀已开启
	tmCloseSkill_4=26,				//半月弯刀已关闭
	tmPrepSkill_6,					//烈火剑法已准备就绪
	tmPrepSkill_8,					//逐日剑法已准备就绪
	tmToxicosis,					//你中毒了
	tmDefenceUPAtTime,				//物防和魔御提升%s秒
	tmDefenceRevert=31,				//物防和魔御恢复正常
	tmchatLevelLimit,       		//聊天等级不足
	tmchatCDLimit,          		//聊天Cd
	tmchaToLength,          		//聊天内容过长
	tmchatillegal,          		//聊天内容存在非法字符
	tmBoratNotEnough = 36,  		//喇叭不足
	tmBoratNotEnough_2,
	tmBoratNotEnough_3,
	tmBoratNotEnough_4,
	tmBoratNotEnough_5,
	tmchatIsBan =41 ,       		//聊天禁言
	tmflyshoesNotEnough,    		//飞鞋不足
	tmChatDisabledInArea,			// 此区域无法聊天
	tmpChatTargetOffLine,   		//聊天无法查找
	tmAddFriendLevel = 45,			//请%d级后再添加好友
	tmFriendMaxCount,				//人数已达上限
	tmTagetFriendToMax,				//对方好友人数已达上限
	tmHaveBeYourFriend,				//%s已是你的好友
	tmAddEnamySuccess,				//添加成功
	tmAddFriendNotice ,				//你跟%s 已经是好友了
	tmItemCanNotDelete,     		//该物品无法被销毁
	tmFailForDealing,       		//在交易的时候不能这么做
	tmDropMonsterBroad = 53,		//【玩家】在【地图名字】击败了【怪物名字】中掉落【物品名字】
	tmTargetOffLine = 54,  		    //目标已经离线
	tmTeamLevelLimited = 55,		//等级不足%d级,不能组队
	tmOtherTeamLevelLimited,		//对方等级不足%d级,不能组队
	tmTeamOpOfflineUser,    		//玩家%s离线，不能进行操作
	tmTeamIsFull,           		//队伍已经满员，无法加入更多的成员
	tmTeamIsInTeam,         		//玩家已经在队伍中了
	tmTeamCreated,					// 您创建了队伍
	tmTeamJoinTeam,					// 你加入了%s的队伍
	tmTeamApplyOfflineUser, 		//对方已经不在线，申请入队失败
	tmTeamApplyNotTeamUser, 		//对方没有队伍，申请失败
	tmTeamNeedCaptin,             	//需要队长才能够操作
	tmTeamNotInTeam,              	//不在队伍里，无法操作
	tmTeamOfflineUserCannotCaptin, 	//离线玩家不能成为队长
	tmTeamRefuseApply,            	//拒绝了入队请求
	tmTeamSelfIsInTeam,            	//自己已经在队伍中了，不能再加入其它的队伍
	tmTeamChatFail,                 //没有加入队伍，无法队伍发言
	tmSetNewCaptin,               	//队长已经移交给了%s
	tmOtherRefuseJoinTeam , 		//%s拒绝加入队伍
	tmCanNotOPOnSelf,       		//不能对自己操作
	tmNoTeam,						//队伍不存在或已解散 --
	tmInviteOtherInFuben,			//要邀请的人已经在副本中，不能和你组队
	tmTeamInFubenCanNotAddMember,	//队伍在副本中，不能新增队员
	tmInFubenCanNotJoinTeam,		//在副本中不能加入其他队伍
	tmInChallengeCannotChangeCaptin,//在擂台不能换队长
	tmInChallengeCannotDelMember,	//擂台战不能踢队员
	tmInChallengeCannotLeft,		//擂台不能离开队伍
	tmInChallengeCannotJoin,		//队伍在擂台不能加入
	tmInFuben,						//你已在一个副本中，如果要创建新的副本，请先退出原有的副本
	tmNotTeamFubenCaptain,			//只有队长才可进行本操作
	tmHasCreateFubenTeam,			//已经创建了副本队伍
	tmInputIsForbid,           		//您的输入有敏感词语，请重新输入
	tmSelfAddTeam,					// 您已成功加入队伍
	tmAddTeamMember,				// 欢迎[%s]加入我们队伍
	tmSelfLeavTeam,					// 您已离队
	tmDelTeamMember,				// %s已离队
	tmHasFubenTeam,     			//你已经创建了副本队伍，不能再创建组队
	tmCoinNotEnough = 90,       	//金币不足
	tmItemNotExist ,             	//物品没找到
	tmCircleMaxLevelLimit,          //转生等级已达最大值
	tmCircleOpenServerDayLimit,     //开服%s天开启
	tmCircleLevelLimit,             //等级达到%d级开启
	tmCircleLastNumLimit,           //%d转达到%d人
	tmCircleSoulNotEnough,          //修为不足
	tmDataError,         			//数据异常
	tmCircleLimtiTimes,             //达到每日最大限制次数
	tmItemNotEnough,             	//物品不足
	tmItemCanNotUse = 100,          //道具不能使用
	tmItemInCd,             		//道具在cd
	tmItemUseSuccess,             	//道具使用成功
	tmItemCanNotUseNow, 			//当前状态无法使用道具
	tmAddInegral,  				    //您获得了%s积分
	tmPointNotEnough,  				//积分不足
	tmOperSucc,						//操作成功，通用的提示语
	tmApplyLimiltLevel,				//您的等级不足%s级不能加入行会
	tmGetGuildGx,					//获得行会贡献%d
	tmDelGuildGx,					//消耗行会贡献%d
	tmInGuildDart,					//该玩家在行会运镖过程中，为行会做贡献，无法开除。
	tmGuildInfuben,					//该玩家在行会斗魂副本中，为行会做贡献，无法开除。
	tmMeInFuben,					//正在行会斗魂副本中，不能退出行会
	tmNoDelGuildInfuben,			//正在行会斗魂副本中，不能解散行会
	tmCreateGuildErrorLevel = 114,	//不够等级创建行会
	tmCreateGuildHasInGuild,		//已经在一个行会里了，不能再另外创建行会
	tmLeftGuildCanNotJoin,          //脱离行会后%d小时内不能加入行会,不能创建新行会
	tmNoRight,				        //不够权限
	tmCreateGuildSucc,				//创建行会成功
	tmCreateGuildNameErr,			//行会名含有非法字符
	tmCreateGuildNameInUse,			//行会名称已被使用
	tmCreateGuildInterErr,			//数据服务器的内部错误
	tmJoinApplyHasInGuild,			//申请加入行会的的时候，之前已经加入过行会
	tmNoThisGuild,					//不存在这个行会，或者已被解散
	tmNotInGuild,					//还没有加入行会
	tmChangeLeaderSucc,				//更换会长成功
	tmGuildPosLimit,				//%s人数已达上限,无法再任命!
	tmMaxMemberCount,				//达到人数上限，不能新增人
	tmUpdateMemoSucc,				//公告更新成功
	tmApplySubmit,					//申请加入行会已提交
	tmLeaderCanNotLeft,				//会长不能脱离行会
	tmInSbkWar,						//在攻城战期间（20:00-21:00），行会禁止职务调动
	tmGuildObjHasBeenPos,			//对方已经是%s
	tmGuildCoinLimit,				//资金不足
	tmGuildBuildLvLimit,			//已达最大值
	tmGuildMainBuildLvLimit,		//行会需%d级 才可以升级
	tmActorInGuild,					//对方已加入其它行会
	tmTeamApplySuccess,				//申请成功，请耐心等待
	tmLeftGuild,					//%s被开除或者脱离了帮派
	tmGuildFlftAndClearGx,			//您离开了行会，行会贡献度清0
	tmGuildEventCreate,				//%s创建了行会
	tmGuildEventMemberJoin,			//%s加入了行会！
	tmGuildEventMemberLeft,			//%s离开了行会！
	tmGuildEventTickMember,			//%s将%s踢出了行会！
	tmGuildEventLeaderChange,		//%s把首领禅让给%s
	tmGuildEventAddOfficer,			//%s被任命为%s！
	tmGuildEventDelOfficer,			//%s被撤销了%s！
	tmGuildEventlevelUp,			//本行会已成功提升至%d级！
	tmGuildEventImpeach,			//%s弹劾成功，成为了%s！
	tmGuildJoinApplyAlready,		//"您已经提出申请，请耐心等待审核",
	tmNearAttackSkillName,  		//普通攻击
	tmSkillKillEntity,  			//你杀死了%s
	tmBeSkilled,					//"你在战斗中被[%s]击败",要显示掉落功勋
	tmBeSkilledByOther,      		//"你在战斗中被[%s]击败",
	tmAgreeJoin,       				//同意审批
	tmNotPeaceful,					//已是敌对状态
	tmDeclareWarNotice,				//宣战后通知对方成员
	tmDeclareSucc,					//向%s行会宣战成功
	tmOnWarNotice,					//%s向我们宣战了！
	tmNoMoreGuildCoin,				//行会资金不足,无法宣战
	tmIsUnionDecare,				//同盟关系不能宣战
	tmBeLeader,                     //%s成为会长
	tmGuildImpeachObjOnline,		//对方在线，不能弹劾
	tmGuildImpeachSelf,				//不能弹劾自己
	tmGuildImpeachMemberPosErr, 	//你的官职高于或等于弹劾职位，弹劾失败
	tmGuildImpeachEvent,			//%s弹劾了%s,成为%s
	tmGuildInProtectDay,			//该官员在%d保护期内
	tmGuildEventStart,				//帮派事件的开始
	tmAddToGuildEvent,				//加入本帮派
	tmLeftGuildEvent,				//离开了本帮派
	tmUpGuildPosEvent,				//从 XX 升为 XX
	tmDownGuildPosEvent,			//从 XX 降为 XX
	tmGuildHead,					//会长
	tmGuildAssistLeader,			//副会长
	tmGuildTangZhu,					//长老
	tmGuildElite,					//精英
	tmGuildMember,					//成员
	tmChangeGuildLeaderEvent,		//将帮主之位禅让给
	tmDeleteMemEvent,				//开除帮派
	tmCreateGuildEvent,				//创建了帮派
	tmGuildEventCoin,				//<(%s)> 捐献了%d资金
	tmDisableOperator,				// 此区域禁止此操作
	tmKilledByActor, 				//<(n%s)>在<(c0xFFFFFF)%s>被<(n%s)>击败!	
	tmGuildBeKillTips,				//<(c0xFFFFFF)%s>的%s<(n%s)>在<(c0xFFFFFF)%s>被<(n%s)>击败,甚是狼狈!
	tmGuildSecLeaderBeKilledTips,	//<(c0xFFFFFF)%s>的副首领<(n%s)>在<(c0xFFFFFF)%s>被<(n%s)>击败,甚是狼狈!（不使用）
	tmSbkGuildLeaderBeKilledtips,	//沙城城主<(n%s)>在<(c0xFFFFFF)%s>被<(n%s)>击败，城主威严荡然无存!
	tmBagIsFull,       				//背包已经满
	tmLeftBagNumNotEnough,       	//背包不足
	tmCircleNotEnough,       		//转生不足
	tmApplyFriendSuccess,       	//申请好友发送成功
	tmComPoseLimit,       			//条件不足
	tmGuildDonateLimitTimes,      	//今日捐赠次数已达最大上限
	tmActorForgeItem,       		//%s用金刚石锻造了%s
	tmPKMod_1=193,      			//你已切换为和平模式
	tmPKMod_2,       				//你已切换为团队模式
	tmPKMod_3,      				//你已切换为帮派模式
	tmPKMod_4,       				//你已切换为敌对模式
	tmPKMod_5,       				//你已切换为全体模式
	tmPKMod_6,       				//你已切换为阵营模式
	tmConsiFull,					//上架商品数量已满，请下架商品后再操作
	tmConsignLevelLimit,			//等级不足%d,不能寄卖
	tmConsignHaveNotItem,			//背包没有该物品,不能寄卖
	tmConsignItemNotCount,			//该物品数量不足
	tmConsiItemBinded,				//绑定物品不能寄卖
	tmConsiItemCantSell,			//该物品不能寄卖
	tmConsiItemPriveInvalid,		//寄卖价格超出范围
	tmConsignItemNotItem,			//寄卖的物品已不存在
	tmConsignBuyPriceError,         //寄卖价格不对
	tmConsignCantGetNotYou,			//该物品不是你的，不能取回
	tmConsiBagFullCannotOp,			//您的背包空格不足，请整理后再操作 --不用了
	tmConsignItemOffShelf,			//该物品已经过期下架了
	tmConsignItemActorBuy,			//该物品已经被他人购买了
	tmConsiNoYuanbaoToBuy,			//您的元宝不足，无法购买物品
	tmConsignBuyItemCountexceed,	//超出了可购买数量
	tmConsiBuySuccAddToBag,			//购买成功
	tmConsignAddIncomeMailTitle,	//出售商品成功
	tmConsignAddIncomeMailContent,	//您的商品%s已有买家购买%d个，获得收益：%d，请及时在寄售面板领取您的收益。
	tmConsignSearchNothing,			//没有搜索到相应商品
	tmConsignCantBuyMySelf,			//这是你寄卖的商品，无法购买
	tmConsignItemOnSell,			//该物品正在售卖
	tmNoSafeAreaNoGet,				//不在安全区内无法领取
	tmMailForceIsFull, 				//体力已满，邮件暂不能提取
	tmMailError,                	//邮件错误
	tmDelMailSuccess,				//清楚成功,
	tmNoSafeAreaGetMail,			//不在安全区内无法提取附件
	tmMailHasItem,					//附件未领取
	tmMailGetItemSuccess,			//领取成功
	tmNoMoreBindCoin,				//绑定金币不足
	tmNomoreBindYubao,				//银两不足
	tmYouAreDealingNow,				//你已经在交易了，请先完成本次交易
	tmDealTargetNotExists,			//交易对方不在线
	tmTargetIsDealing,				//对方正在交易，请稍后再试
	tmAlreadyInvaiteDeal,			//交易请求已经发出，请等待对方回应
	tmTargetDoNotDealWithYou,		//%s拒绝了你的交易申请
	tmDealRequesterHasLeaved,		//交易对方已经离开，交易被取消
	tmDealBlankWasFull,				//交易物品栏已满，无法继续添加物品
	tmYouMustLockDealFirst,			//只能在双发都锁定交易后才能完成交易
	tmDealCanceledOfMyBagWasFull,	//由于你的背包无法完全容纳交易对方放入的物品，交易已被取消
	tmDealcanceldOfTargetBagWasFull,//由于对方背包无法完全容纳你放入的物品，交易已被取消
	tmDealDataValidateFailure,		//交易数据验证失败，交易已被取消
	tmDealTargetConfimed,			//对方已经确认交易，您现在可以按下交易键完成交易
	tmTargetHasLockedDeal,			//对方已经锁定交易，您现在可以按下锁定按钮后点击交易按钮
	tmYouMayClickDealBtnNow,		//对方已经锁定交易，您现在可以按下锁交易按钮等待对方完成交易
	tmWaitTargetLockDeal,			//您已经锁定交易，请等待对方按下锁定按钮
	tmItemNoDeal,					// 物品无法交易
	tmDealTargetTooFar,				// 距离太远，交易取消
	tmDealHasLocked,				// 已锁定
	tmTargetMoneyIsFull,        	//对方金钱已经超过最大数量
	tmSelfMoneryIsFull,         	//自己的金钱超过最大限度 
	tmAreaCannotTrade,				// 区域禁止交易
	tmDealTargetLevelLimit,   		//对方的等级不够%s级，无法进行交易
	tmDealApplyCdLimit,   			//每次邀请需间隔30秒
	tmDealSelfLevelLimit,     		//您需要在%s级以后才可以与他人交易
	tmGuildDonateNotEnough,     	//行会贡献不足
	tmMailFullCantAdd,		  		//邮箱已满，请清空邮箱以便接收新邮件
	tmMailItemGetReady,		  		//邮件附件已领取
	tmFlyShoesError,				//该传送功能尚未开放
	tmLevelExceednoenter,			//等级过高，无法进入
	tmLevelLowernoenter,			//等级不足，无法进入
	tmMailNOItem,                   //没有附件
	tmTagetDistanceTooFar,          //距离太远
	tmDealNotAtSeafArea,            //不在安全区，私人交易失败
	tmItemIsQuestItem,				// 任务物品无法交易
	tmDealCancel,					// 交易被取消
	tmDealApplyTooMuch,				// 对方已经收到太多交易邀请，请稍后再试
	tmDealComplete,					//交易完成
	tmLogDealWith,					//与%s交易
	tmDealNumTooLarge,             	//一次交易金额太大
	tmMeridiansMaxLv,             	//当前经脉已达最大等级
	tmItemRegionCanNotUseItem, 		//当前区域内不能使用物品
	tmDealItemExist,             	//该物品已添加
	tmQuotaNoEnough,             	//交易额度不足
	tmYbTaxRateGet,             	//元宝交易税扣除成功
	tmCoinTaxRateGet,             	//金币交易税扣除成功
	tmGuildRequireLvLimit,           //该行会要求%s等级
	tmYbLimitNoImpeach,          	 //元宝不足, 无法弹劾
	tmImpeachOfficeError,          	 //不能弹劾非会长
	tmItemIsSellOrTimeOut,          //该商品已被购买或到期下架
	tmBlessAddValue,          		//祝福值增加%s
	tmServerConsiFull,				//全服上架商品数量已满，请下架商品后再操作
	tmTriggerExpLmt_1,				//触发本日基础经验一级上限
	tmTriggerExpLmt_2,				//触发本日基础经验二级上限
	tmTriggerExpLmt_3,				//触发本日基础经验三级上限
	tmGetExpMulti ,					//获得多倍经验%lld
	tmBlessNoAddValue,				//无效
	______palceholder,				//占位，pk值
	tmCircleSoul,                   //转生修为%s
	tmBroatNum,                     //喇叭数量%s
	tmFlyShoeNum,                   //飞鞋数量%s
	tmGuildDonate,                  //行会贡献%s
	tmNotInSafeArea,                 //非安全区禁止操作
	tmMultiExpExhaust,				//多倍经验已用完，打怪只能获得基础经验
	tmAddExpGeneral,				//经验+%lld
	tmAddExpBasic,					//获得基础经验%lld
	tmAchieveLimitCondition,		//限制条件未达成
	tmUseItemAddExpGeneral,		    //多倍经验+x（物品）
	tmAchieveAwardIsGet,		    //成就奖励已经领取
	tmMedalTouchTopLevel,			//勋章达到满级
	tmMedalLvUpFail,				//没有达到升级条件
	tmMedalLvUpSuccess,				//勋章升级成功
	tmdoubleExpFull =300,			//多倍经验已满
	tmCircleToMaxLv,				//已达转生允许的级别上限
	tmBlessSubValue,          		//祝福值减少%s
	tmAppraisalNoItem,          	//鉴定材料不足
	tmAppraisalNoTimes,          	//鉴定次数不足
	tmAppraisalMaxLevel,            //达到转生允许的级别上限，无法鉴定
	tmNoPrestigeNum,               	//声望不足
	tmNoActivityValue,              //活跃度不足
	tmMoreCoin,              		//大量金币
	tmNoTimes,              		//次数不足
	tmUnreachOpenServer,			//未达到开服天数，传送门无法使用
	tmMapLimitTeleport,				//该地图禁止传送
	tmActorOffLine,  		    	//玩家不在线
	tmBindYuanbaoFull,              // 已经无法获取更多的绑定元宝
	tmYuanbaoFull,              	// 已经无法获取更多的元宝
	tmBindCoinFull,               	// 已经无法获取更多的绑定金币
	tmCoinFull,               		// 已经无法获取更多的金币
	tmNoOpen,               		// 未开放
	tmThisChanalForbit,        		// 你在该频道尚不能聊天
	tmGetSpeedAwardSucess,			//领取神速礼包成功
	tmSbkCityName,                  //沙巴克显示
	tmAddExpbyPaoDian,				//泡点经验提示
	tmKillMonsterNotEnouth,			//杀怪不够提示
	tmGetChuangGuanGiftSucc,		//领取闯关礼包成功
	tmGetChuangGuanGiftYet,			//已领取闯关礼包
	tmGetSpeedAwardYet,				//已领取神速礼包
	tmGetDuoBaoGiftSucc,			//领取夺宝战礼包成功
	tmGetDuoBaoGiftYet,				//已领取夺宝战礼包
	tmActivityTimeUnReach,			//未到活动时间
	tmActivityUnReachsongbaoTime,	//未到送宝时间
	tmSongBaoSuccess,				//送宝成功
	tmSongBaoYet,					//已送宝
	tmkillMonsterEnoughYet,			//杀怪够了继续杀提醒提交任务
	tmNeedItemNotEnough,			//所需物品不足
	tmNoInOpenTime,					//不是开启时间
	tmChangeNameNotOpen,			//改名功能暂未开启，尽请期待
	tmNoChangeNameItem,				//没有改名字的道具
	tmChangeNameSuc,				//修改名字成功
	tmNeInvalidName,				//名称无效，名称中包含非法字符或长度不合法
	tmNeNameinuse,					//名称已被使用
	tmTelePortLimitActivity,		//达到活动传送限制
	tmTelePortTimeUnopen,			//未到开放时间无法通过NPC传送进入
	tmBossDeath,			        //boss 已死亡 活动已结束
	tmBossGwSuccess,			     //鼓舞成功
	tmMailGuild,              		//行会
	tmAddGuidMail,                  //%s 同意你加入该行会
	tmDeathDropMail,                  //在%s 被击败
	tmDeathDropEqMail,                  //%s:%s
	tmPoint,                  //“, ”
	tmEquipName1,                  //武器
	tmEquipName2,                  //衣服
	tmEquipName3,                  //头盔
	tmEquipName4,                  //项链
	tmEquipName5,                  //勋章
	tmEquipName6,                  //手镯
	tmEquipName7,                  //戒指
	tmEquipName8,                  //腰带
	tmEquipName9,                  //鞋子
	tmEquipName10,                  //魂玉
	tmEquipName11,                  //斗笠
	tmEquipName12,                  //面甲
	tmEquipName13,                 //披风
	tmEquipName14,                  //盾牌
	tmDeathDropMailTitle,             //死亡掉落提示
	tmDefNoBagNum,             //剩余背包格数不足
	tmEquipNoBagNum,             //无法调整装备
	tmRecoverNoBagNum,             //装备，材料包裹
	tmForgeNoBagNum,             //背包不足，装备必须剩余1格
	tmTenForgeNoBagNum,             //背包不足，装备必须剩余10格
	tmGiftNoBagNum,             //背包不足，装备、材料包裹必须剩余6格
	tmActivateSuit,				//%s 激活
	tmInvalidSuit,               //%s 失效
	______palceholder1,  		 //占位
	______palceholder2,  		 //占位
	tmGuaJiJuanZhouOnUse,		 //挂机卷轴加速结束才可再次使用本道具
	tmGuaJiJuanZhouUseSucc,		 //挂机卷轴使用成功	
	tmSZBossDeath,		 //BOss死亡	
	tmItemLimitSZBossTime,		 //挑战次数大于100次，无法使用
	tmGetNewTitleNotice = 378,	//恭喜%s获得%s称号
	tmSelfGetNewTitleNotice,	//恭喜获得%s称号
	tmFCMOpened,				//防沉迷系统开启
	tmFcmHalfExp,		//您已累计在线超过3小时，游戏收益变为正常的50%，请下线休息。
	tmFcmZeroExp,       //您已累计在线超过5小时，游戏收益变为0，请下线休息。
	tmFcmPerHour,		//防沉迷整点提示，您已被纳入防沉迷，请合理安排游戏时间
	tmWarPointNoEnough,	 //战令积分不足
	tmSelfItemNotEnough,	 //%s 不足
	tmBanChat,	 //禁言中
	tmItemSplitBagFail , //背包物品拆分失败
	tmCannotCatchPetMore, //无法携带更多宠物
	tmDaYaoYueKaGongGao,  //大药月卡购买公告
	tmYongHengGongGao,	  //永恒卡购买公告
	tmFollwerMaxCount,	  //关注人数已达上限
	tmBlackMaxCount,	  //黑名单人数已达上限
	tmRefiningReplace, //替换成功
	tmComposeBagLimit, //合成背包失败提示
	tmChallengBossSucess, //挑战boss 成功
	tmFXTransportYuanBaoNotEnough, //飞鞋传送的元宝充值不足限制提示
	tmDB1,  		 //夺宝1
	tmDB2,  		 //夺宝2
	tmBaiKaGongGao,
	tmLvKaGongGao,
	tmLanKaGongGao,
	tmZiKaGongGao,
	tmChengKaGongGao,
	tmEquipName15,                  //宝珠
	tmDeathMailTitle,                  //
	tmDeathMailContent,                  //在%s被%s杀死了 
	tmEquipName16,                  //魔器
	tmInActivityLimitOperate,                  //活动期间禁止改操作
	tmUpStarFailDownStar,                  //升级失败 星级降级
	tmItemUpMaxStar1,                  //%s 把 %s 升到最大星
	tmItemUpMaxStar2,                  //%s 把 %s 升到最大星
	tmUpCircleTips,                  //
	tmGhost,                  //
	tmNoBindCoin2City,                  //
	______palceholder3,
	tmExclusiveTips,             //嘟嘟会员-[%s] [%s] 登录开启专属高爆特权，顶级神装极限爆率，我也想要！
	tmRankTips,                  //全服第[%s] [%s] [%s]
	tmRankPowerTips,             //全服第[%s] [%s] [%s]
	tmRankNum1,                  // 一
	tmRankNum2,                  //二
	tmRankNum3,                  //三
	tmActorJob1,                 //战士
	tmActorJob2,                 //法师
	tmActorJob3,                 //道士
	tmActorPower1,               //物攻
	tmActorPower2,               //法功
	tmActorPower3,               //道攻
	tmSbkTips,               	//"【%s】-【%s】成为沙城之主，吾等前方，绝无敌手！"
	tmGhostTips,             	//"【%s】-【%s】成功炼化神魔之体，主宰大陆，舍我其谁！"
	tmFrenzyReOpen,            	//"您已开启狂暴状态！"
	tmFrenzyOpenSucc,           //"开启成功！"
	tmEquipQuality0,                  //0白色
	tmEquipQuality1,                  //1绿色
	tmEquipQuality2,                  //2蓝色
	tmEquipQuality3,                  //3紫色
	tmEquipQuality4,                  //4橙色
	tmEquipQuality5,                  //5红色
	tmInDonateRank,                   //进入%d 名
	tmDonateRankNoDonate,            //不满足最低金额
	tmDonateRanksuccess,            //捐献%d元宝，捐献成功
	tmkuangbaolimir,            //狂暴未开启
	tmghostlimit,            //神魔点数不足
	tmTeamCallNotEnough,     //队伍召集令不足
	tmGuildCallNotEnough,    //行会召集令不足
	tmCallSuccWait,            //召集成功，请耐心等待
	tmGuildCallLimit,       //只有会长/副会长可以使用
	tmSceneCallLimit,        //该场景无法进行召唤
	tmCallNoTeam,        //当前没有队伍
	tmLootNotOwner,                 //目前还归属其他人，稍后才能拾取
	tmLootParamError,                 //参数错误
	tmLootExpired,                 //要拾取的物品已经过期
	tmLootPosError,                //不在同一个坐标，无法拾取
	tmLootCanPickUpTime,		//未达到能拾取的时间
	tmLootPicked,					//物品已被拾取
	tmFriendnotaddSelf,					//无法对自己操作
	tmChatnotaddSelf,					//无法对自己发炎
	tmUpgreadOffice,					//官职升级成功
	tmMaxOffice,					//已经达到最高官职
	tmOfficeBroad,					//
	tmLimitConsignment,					//寄售行限制
	tmLimitDeal,					//私人交易限制
	tmComPoseOfficeLimit,       			//条件不足
	tmComPoseVipLimit,       			//特权不足
	______palceholder4,       			//
	______palceholder5,
	______palceholder6,
	______palceholder7,
	tmDescribeStr1, //《》
	tmDescribeStr2, //《》
	tmDropMonsterBroadItem,		//E:${%d},${10001},${%I64d},&U:&C:16718530&T:${%s}|
	tmItemCanNotPutDepot,//禁止放仓库
	tmdepotBagIsFull,//仓库已满
	tmChgVocInCD,	//转职CD未到，请稍后尝试
	tmdepotLvLimit,	//仓库等级限制
	tmdepotCardLvLimit,	//仓库特权等级限制
	tmCircleLvLimit,	//转生等级不足
	tmBag2Deport,	//%s背包到仓库
	tmDeport2Bag,	//%s仓库到背包
	tmConsigmentCanGet,//寄售行可领取
	tmActivity10017Limit,//
	tmActivity10017LimitTimes,//
	tmActivity10017LimitBags,//
	tmActivity10017OperateSuccess,//10017操作成功
	tmItemQuality0,                  //0白色
	tmItemQuality1,                  //1绿色
	tmItemQuality2,                  //2蓝色
	tmItemQuality3,                  //3紫色
	tmItemQuality4,                  //4橙色
	tmItemQuality5,                  //5红色
	______palceholder8,       			//
	______palceholder9,
	______palceholder10,
	______palceholder11,
	______palceholder12,
	tmOpenDayLimitCircle,		 //开服天数不足%s天，无法进行转生
	tmBaiKaLiaoTianXiTongGongGao,
	tmLvKaLiaoTianXiTongGongGao,
	tmLanKaLiaoTianXiTongGongGao,
	tmZiKaLiaoTianXiTongGongGao,
	tmChengKaLiaoTianXiTongGongGao,
	tmRankNum4,                  //二
	tmRankNum5,                  //三
	tmRankNum6,                  //三
	tmRankNum7,                  //三
	tmRankNum8,                  //三
	tmRankNum9,                  //三
	tmRankNum10,                  //三
	tmChatVipLimit,                  //三
	tmChatVipLeftTimes,                  //三
	tmUseItemOpenDaylimit,  
	______palceholder13,  
	______palceholder14,
	______palceholder15,
	tmZLMoneyNoEnough,//战令币不足
	tmUseRecoverItem,//
	______palceholder16,
	tmNeedLv,//
	tmAtv14Tips,// 
	tmNull, //占位
	tmChengXinGongGao,//橙星会员 
	tmChengXinLiaoTianXiTongGongGao,	//橙星会员聊天
	
	tmKuaFuBossPkWin,					//跨服首领提示 玩家PK赢
	tmKuaFuBossPkLost,					//跨服首领提示 玩家PK输
	tmKuaFuDimensionalKeyNotEnough,		//跨服次元首领提示 次元钥匙不足 
	tmKilledByActorCs, 					//跨服信息<(n%s)>在<(c0xFFFFFF)%s>被<(n%s)>击败!	
	tmKuaFuBossTipsCs, 					//跨服信息 跨服首领提示
	tmActorLoginKuafu, 					//登录跨服提示
	tmInternalWorkLevelLimit,			//"|C:0xf56f00&T:内功等级不足,无法学习技能|"
	tmInternalWorkLevelNotEnough,		//"|C:0xf56f00&T:内功等级不足|"
	tmLootPetMaturity,					//"您的圣宠“%s”已到期。"
	tmGuildFunctionNotOpen,				//"|C:0xf56f00&T:行会功能未开放|"
	tmUseItemUselimit,  				//物品使用限制//新增
	tmChengYueGongGao,//橙月会员 
	tmChengYueLiaoTianXiTongGongGao,	//橙月会员聊天
	tmtmFrenzyCardLvLimit,	//狂暴月卡等级限制
	tmGuildLevelLimit,				 	//物品使用，行会等级限制
};
//tolua_end

// TO DELETE
// System.txt
enum tagTipmsgID
{
	tpNoError=0,			//没有错误
	//技能系统
	tpSkillConfigError = 1,		//技能配置错误
	tpSkillTrainLevelLimited,   //等级不够无法升级
	tpSkillTrainCoinLimited,   //银两不够，无法升级
	tpSkillTrainBindcoinLimited,   //绑定银两不够，无法升级

	tpSkillTrainExpLimited,    //经验不够，无法升级
	tpSkillTrainVocationLimited, //职业不符合，无法升级
	tpSkillTrainHasLearned,  //已经升级了这个技能无需要升级
	tpSkillSpellHpLimited,    //生命值不够，无法释放
	tpSkillSpellMpLimited,   //蓝不够，无法释放技能
	
	tpSkillIsClosed,         //该技能已经关闭，无法使用
	tpSkillSpellNotLeared,   //没有学习这个技能，无法使用技能
	tpSkillSellCDLimited,		///< 当前技能正在冷却中
	tpSkillSpellNoTarget,  //请先选择施法对象或者施法点
	tpSkillTrainItemLimited ,//物品不够，无法升级
	tpSkillTrainWithBuff ,//存在buff
	tpSkillTrainWithoutBuff ,//不存在buff
	tpSkillTrainDistance ,//和目标的距离
	tpSkillTrainOverLap ,//和目标的重合
	tpSkillTrainFaceTarget, //需要面向目标
	
	tpSkillTrainOneDirTarget, //需要和目标的方向
	tpSkillTrainMountExpLimited, //坐骑经验不够
	tpSkillTrainHpRateBelow,   //自身的血的百分比必须低于一定的值
	tpSkillTrainFriendTarget , //必须需要友好目标
	tpSkillTrainEnemyTarget ,  //必须选择敌对目标
	tpSkillTrainCanAttackTarget ,  //该目标不能够攻击
	tpSkillTrainEfficicyNotEnough ,  //生活技能的熟练度不够

	tpSkillTrainYDistance,         //与目标的纵向距离过远
	tpSkillTrainNotInBattle,//当前不在战斗中，无法使用
	tpSkillTrainInBattle,//当前在战斗中，无法使用
	tpSkillTrainGuildContri,//帮派贡献不够
	tpSkillTrainGuildSkillLevel,//帮派技能等级不够

	tpSkillLearnLevelLimited,	//等级不足，无法学习技能
	tpSkillLearnVocationLimited,	//职业不符，无法学习技能
	tpSkillLearnCircleLimited,		//转数不足，无法学习技能
	tpSkillLearnItemLimited,		//道具不足，无法学习技能

	tpSkillLearnSucceed,   //"您成功地学会了技能%s",
	tpSkillLevelUp,		//"您成功地将技能%s提升到%d级",
	tpSkillInnerDamage,  //技能%s对%s造成了%d点内功攻击
	
	tpNearAttackSkillName,  //普通攻击
	tpSkillKillEntity,  //你杀死了%s
	tpBeSkilled,			//"你在战斗中被[%s]击败",	要显示掉落功勋
	tpBeSkilledByOther,       //"你在战斗中被[%s]击败",

	tpSkillMijiNotMatch, //%s技能无法配置该秘籍
	tpSkillMijiLevelLimit, //需要将技能提升到%d级才能使用这个秘籍
	tpSkillMijiHasLearn,  //已经学习了另外一个秘籍，请先移除该秘籍
	tpSkillMijiLearnSucceed, //成功学习了一个秘籍
	tpSkillForgetSkill,    //技能[%s]已经不能使用

	tpSkillGetExp,         //获得经验%d点
	tpSkillGetMultiplyExp, //获得经验%d点，其中%d点为多倍经验
	
	tpSkillIsUpMostLevel , //已经是最高级了
	tpSkillCannotSpell, //技能无法使用
	tpSkillSingParamError = 52, //吟唱技能的参数不一致


	//任务系统
	tpQuestNotGiveUp,	//任务不能放弃
	tpGiveUpSucc,		//任务放弃成功
	tpQuestFullBag,		//你无法领取奖励，请检查背包
	tpQuestRemain = 56,      //剩下：
	
	//聊天系统
	tpChatDisabledInArea,	// 此区域无法聊天
	tpChatDisableOperator,	// 此区域禁止此操作
	tpChatTargetOffLine, //目标已经离线

	//物品系统
	tpItemConfigError = 60,     //物品配置错误
	tpItemNotExist,   //要穿戴的物品不存在
	tpItemCannotTakeOn,   //物品不能穿戴
	tpItemCannotTakeoff,   //因无法取下当前位置的装备，无法装备
	tpItemTakeoffFailForBagFull,   //背包已满，无法取下物品的消息
	tpItemCannotTakeOnForLevel, //等级不够，无法装备
	tpItemCannotTakeOnForSex, //性别不符，无法装备
	tpItemCannotTakeOnForVocation, //职业不符，无法装备
	tpItemCannotTakeOnForCircle,			//转数不够，无法装备//没用到
	tpItemCannotTakeOnForBattlePower,		//战力不够，无法装备
	tpItemCannotTakeOnSameTypeSpeRing,		//同种特戒只能佩戴一个

	tpItemMoneyNotEnoughEnlargeBag, //扩展背包失败,扩展%d格需要花费%d%s
	tpItemEnlargeSucceed,      //成功将背包扩大%d格并获得%d经验
	tpItemBagTooBig, //背包已经足够大，不能扩展
	tpItemBagOverMax,	//超过背包最大格子啦，不能扩展%d格
	tpItemSplitBagFail , //背包物品拆分失败
	tpItemMergeFail , //背包物品合并失败
	tpPkValueIsEmpty,  //杀戮值无需要减

	tpItemCanNotPour,  //此物品不能用了灌注所选的源泉
	tpItemNoNeedPour,  //装备的是满的，不需要灌注
	tpItemBagGetItem,  //背包里获得物品获得%d件[%s]
	tpItemBagLostItem, //失去[%d]个[%s]
	tpItemCanNotDelete, //该物品不能销毁
	tpBagIsFull,       //背包已经满
	tpItemTakenOnBinded, //[%s]穿戴后被绑定
	tpItemCanNotUseNow, //当前状态无法使用道具
	tpItemNeedMoreGrid,  //需要清理出%d个格子才能领取
	tpBagFullNotGetSysItem,//需要清理背包以接收系统转发的物品
    tpItemRegionCanNotUseItem, //本区域内不能使用物品
	tpItemInCD,					// 该物品正在冷却中
	tpItemNoBattlePet,        //没有出战的宠物
	tpItemNoDuraCanNotUseItem,	//此物品使用次数不足，无法使用
	tpItemDenyHeroUse,			//此物品英雄不能使用

	tpReliveRingActive,		//你的复活戒使你免除了一次死亡！在一分钟内无法再次生效，请注意安全
	tpDizzyActive,			//你触发了麻痹效果，你的目标被麻痹5秒

	//仓库系统
	tpDepotBagIsFull = 95, //仓库背包已经满了，不能再添加
	tpVipGrideLow,		//需要是VIP且等级达到%d级才能使用
	tpItemCanNotPutDepot,//该物品不能放仓库
	tpFailForDealing, //在交易的时候不能这么做

	tpBookQuestMaxStar,   //你已刷到最高星，请尽快完接受任务

	//工会系统
	tpCreateGuildErrorLevel = 100,		//不够等级创建帮派
	tpCreateGuildHasInGuild,		//已经在一个帮派里了，不能再另外创建帮派
	tpNoRightToDelete,				//不够权限解散帮派
	tpCreateGuildSucc,				//创建帮派成功
	tpCreateGuildNameErr,			//帮派名含有非法字符
	tpCreateGuildNameInUse,			//帮派名称已被使用
	tpCreateGuildInterErr,			//数据服务器的内部错误
	tpNoRight,						//邀请玩家加入的权限不够
	tpJoinApplyHasInGuild,			//申请加入帮派的的时候，之前已经加入过帮派
	tpNoThisGuild,					//不存在这个帮派，或者已被解散
	tpJoniApplyReject,				//申请加入被拒绝
	tpJoinGuildSucc,				//成功加入帮派,这个一般是广播给帮派成员的消息
	tpLeftGuild,					//被开除或者脱离了帮派
	tpLeftGuildCanNotJoin,			//脱离行会后%d小时内不能加入行会,不能创建新行会
	tpLeftGuildCanNotJoin1,			//该玩家已加入其他行会,无法邀请
	tpNotInGuild,					//还没有加入帮派
	
	tpChangeLeaderSucc,				//更换帮主成功
	tpBeLeader,						//成为帮主
	tpGuildPosLimit,				//%s人数已达上限,无法再任命!
	tpUpPos,						//被升职了
	tpDownPos,						//被降职了
	tpMaxMemberCount,				//达到人数上限，不能新增人
	tpUpdateMemoSucc,				//公告更新成功
	tpDeleteGuildSucc,				//解散帮派成功
	tpDeleteGuildErr,				//解散帮派失败
	tpApplySubmit,					//申请加入帮派已提交
	tpOperSucc,						//操作成功，通用的提示语
	tpLeaderCanNotLeft,				//帮主不能脱离帮派
	tpZjOneDayOneTime,				//帮派福利每日只能领取一次！
	tpGetZjSucc,					//获取帮派福利成功
	tpNotZj,						//还没有帮派福利可以领取
	
	
	tpDelGuildTimeErr,				//你必须任职一定的时间后才能解散帮派
	tpGuildHasApply,				//有新玩家申请加入帮派，请尽快审核吧。
	tpDeleteGuildMemo,				//解散后发给帮众的公告
	tpNoZj,							//身上没有召集令，不能召唤
	tpHasSendCall,					//已发召唤消息给对方，请等待对方回应
	tpCallOffLine,					//对方不在线，无法召唤
	tpRefuseCall,					//拒绝召唤
	tpNotPeaceful,					//已是敌对状态
	tpDeclareWarNotice,				//宣战后通知对方成员
	tpDeclareSucc,					//宣战成功
	tpOnWarNotice,					//我们已经向帮派%s宣战了！
	tpOtherLeftGuild,				//某人离开了帮派
	tpCallOtherInFuben,				//被召唤的人在副本里，不能被召唤
	tpCallInFuben,					//本地图不允许召唤他人
	tpRefuseInviteGuild,			//拒绝帮派邀请
	tpGuildNoOne,					//玩家已经下线
	tpGuildNoItemFr,				//没有增加繁荣度的兴帮令
	tpGuildFullBag,					//背包已满，无法领取福利
	
	tptherIsForbidTransfer,        //对方处于禁止召唤区域，无法接收你的召唤！

	//商城系统
	tpStoreItemNotExists = 150,	//商品不存在
	tpStoreNotEnoughBagSpace,//背包已满，无法购买商品
	tpStoreNotEnoughMoney,	//余额不足购买商品
	tpStoreNoYuanBaoCanWithdraw,	// 没有可提取的元宝
	tpStoreWithdrawYuanBaoFailed,	// 查询或者提取元宝错误
	tpStoreSingleCountLimit,        //限量商品一次只能购买%d个
	tpStoreTotalCountLimit,        //限量商品分类里只能购买%d个
	tpStoreHasSellEmpty,           //该商品已经全部售完


	//交易系统
	tpYouAreDealingNow = 158,		//你已经在交易了，请先完成本次交易
	tpDealTargetNotExists,	//交易对方不在线
	tpTargetIsDealing,		//对方正在交易，请稍后再试
	tpYouInStall,			//您正在摆摊，不能交易哦
	tpTargetInStall,		//对方正在摆摊，不能交易哦
	tpAlreadyInvaiteDeal,	//交易请求已经发出，请等待对方回应
	tpTargetDoNotDealWithYou,//对方拒绝与你交易
	tpDealRequesterHasLeaved,//交易对方已经离开，交易被取消
	tpDealBlankWasFull,		//交易物品栏已满，无法继续添加物品
	
	tpYouMustLockDealFirst,	//只能在双发都锁定交易后才能完成交易
	tpDealCanceledOfMyBagWasFull,//由于你的背包无法完全容纳交易对方放入的物品，交易已被取消
	tpDealcanceldOfTargetBagWasFull,//由于对方背包无法完全容纳你放入的物品，交易已被取消
	tpDealDataValidateFailure,//交易数据验证失败，交易已被取消
	tpDealTargetConfimed,	//对方已经确认交易，您现在可以按下交易键完成交易
	tpTargetHasLockedDeal,	//对方已经锁定交易，您现在可以按下锁定按钮后点击交易按钮
	tpYouMayClickDealBtnNow,//对方已经锁定交易，您现在可以按下锁交易按钮等待对方完成交易
	tpWaitTargetLockDeal,	//您已经锁定交易，请等待对方按下锁定按钮
	tpItemIsBinded,         //绑定物品无法交易
	tpItemIsQuestItem,		// 任务物品无法交易
	tpDealTargetTooFar,			// 距离太远，交易取消
	tpDealHasLocked,			// 已锁定
	tpTargetMoneyIsFull,        //对方金钱已经超过最大数量
	tpSelfMoneryIsFull,         //自己的金钱超过最大限度 
	tpAreaCannotTrade,			// 区域禁止交易

	//拾取系统
	tpLootFailForBagFull = 182, //背包满了，拾取失败
	tpLootItem, //%s拾取了[%s]
	tpLootTaskItemBagFull,	// 添加任务物品失败，背包已经满
	
	tpGetBindCoin,   //获得绑定金钱%d
	tpLostBindCoin,   //失去绑定金钱%d
	tpGetCoin,      //获得金钱%d
	tpLostCoin,      //失去金钱%d
	

	tpGetBindYuanbao,   //"获得礼金%d",
	tpLostBindYuanbao,   //"失去礼金%d",
	tpGetYuanbao,   //"获得元宝%d",
	tpLostYuanbao,   //"失去元宝%d",
	tpBindYuanbaoFull, // 已经无法获取更多的绑定元宝


	//队伍系统
	tpTeamOpOfflineUser,         //玩家%s离线，不能进行操作
	tpTeamIsFull,                //队伍已经满员，无法加入更多的成员
	tpTeamIsInTeam,              //玩家已经在队伍中了
	tpTeamCreated,				 // 您创建了队伍
	tpTeamJoinTeam,				 // 你加入了%s的队伍
	
	tpTeamApplyOfflineUser,      //玩家%s已经不在线，申请入队失败
	tpTeamApplyNotTeamUser,      //玩家%s没有队伍，申请失败
	tpTeamNeedCaptin,             //需要队长才能够操作
	tpTeamNotInTeam,              //不在队伍里，无法操作
	tpTeamOfflineUserCannotCaptin, //离线玩家不能成为队长
	tpTeamRefuseApply,            //拒绝了入队请求
	tpTeamSelfIsInTeam,            //自己已经在队伍中了，不能再加入其它的队伍
	tpTeamChatFail,               //没有加入队伍，无法队伍发言
	tpSetNewCaptin,               //队长已经移交给了%s
	tpLootItemLevelWrite,         //[废弃]白装以上ROLL
	tpLootItemLevelGreen,         //[废弃]绿装以上Roll
	tpLootItemLevelBlue,          //[废弃]蓝装以上Roll
	tpLootItemLevelPuple,         //[废弃]紫装以上ROLL

	tpLootItemStyleTeam,		//[废弃]物品的拾取方式已经改为队伍拾取
	tpLootIteamStyleRotate,		//[废弃]物品的拾取方式已经改为轮流拾取
	tpLootIteamStyleFree,		//[废弃]物品的拾取方式已经改为自由拾取
	tpLootItemStyleCaptin,		//[废弃]物品的拾取方式已经改为队长分配
	tpOtherRefuseJoinTeam , //%s拒绝加入队伍
	tpCanNotOPOnSelf,       //不能对自己操作
	tpNoTeam,			//队伍不存在或已解散
	tpInviteOtherInFuben,	//要邀请的人已经在副本中，不能和你组队
	tpTeamInFubenCanNotAddMember,		//队伍在副本中，不能新增队员
	tpInFubenCanNotJoinTeam,		//在副本中不能加入其他队伍

	tpInChallengeCannotChangeCaptin,	//在擂台不能换队长
	tpInChallengeCannotDelMember,		//擂台战不能踢队员
	tpInChallengeCannotLeft,		//擂台不能离开队伍
	tpInChallengeCannotJoin,		//队伍在擂台不能加入
	tpInFuben,		//你已在一个副本中，如果要创建新的副本，请先退出原有的副本
	tpNotTeamFubenCaptain,		//只有队长才可进行本操作
	tpHasCreateFubenTeam,		//已经创建了副本队伍
	tpInputIsForbid,           //您的输入有敏感词语，请重新输入

	//PK子系统
	tpNPCTradeBagFull = 230,    //背包已满。
	tpConsiItemCannotDeal,		//此物品不能交易(未使用)
	//Misc子系统
	tpFcmNotifyBeFcm, //您已经被纳入了防沉迷，抵制不良游戏,拒绝盗版游戏,...
	tpFcmNotify,        //您已在线游戏时间%d小时，请您合理安排您的游戏时间。
	tpFcmHalfLogin,   //您当前已经进入疲劳游戏时间，游戏收益变成正常的50%，请您合理安排游戏时间。
	tpFcmHalfInt,		//您已累计在线%d小时，您已进入疲劳游戏时间，游戏收益变为正常的50%，请您下线休息。
	tpFcmHalfFloat,   //您已累计在线%2f小时，您已进入疲劳游戏时间，游戏收益变为正常的50%，请您下线休息。
	tpFcmZero,       //您已累计在线5小时，您已进入不健康游戏时间，游戏收益变为0，请下线休息。


	//其他杂项
	tpJumpTargetCanNotJump, //不可跳跃到目标点
	tpWithdrawYuanbaoCaptain,		// 提取元宝离线消息提示标题： 您提取的%d元宝，请点击领取。
	tpWithdrawYuanBaoButtonText,	// 提取元宝离线消息提示框按钮文字
	tpChatTooFrequent,				// 发言太频繁
	tpChatLevelTooLow,				// 等级高于%级才能在此频道聊天
	tpSystemClosing,                //服务器将在%d秒后关闭！请及时下线，避免不必要的损失！
	tpConfirm,                     //确定


	//成就系统的
	tpAchieveGetAchieve,    //您获得了成就 xxx
	tpAchieveLostTitle, //你失去了称号%s
	tpAchieveUseTitle,	//成功使用称号%s

	//活动包裹
	tpActivityBagGetItemFail, // 活动包裹领取物品失败
	
	//金钱类型
	tpMoneyTypeYuanbao, //元宝
	tpMoneyTyeBindYuanbao, //绑定元宝
	tpMoneyTypeCoin,    //银两
	tpMoneyTypeBindCoin, //绑定银两

	tpLeftParentheses, //全角的方括号，左边部分（
	tpRightParentheses, //全角的方括号，右边部分）
	tpAddRenown, //获取声望%d
	tpDelRenown,  //失去声望%d
	tpAddRenowFail,  //"您当前的声望已经达到了上限，本次添加的%d声望失败",
	tpActivityItemNotity,           //您有未领取的活动物品，请打开背包点击活动背包领取相关物品
	tpCannotMountInTheArea,			// 此区域禁止骑乘
	//英雄相关
	tpHeroBattleFailSForCount,        //已经有一个宠物出战了
	tpHeroBattleForceLacking,        //英雄出战体力不足
	tpHeroSkillLevelFull,                 //英雄技能已经满级
	tpHeroGetExp,                    //战神获得经验%d点
	tpStateCallHeroFail,             //在某些状态下不能召唤宠物
	tpCannotCallHeroInTheArea,		// 此区域禁止召唤英雄
	tpOtherHasNoHero,               //对方没有英雄,无法查看 
	tpHeroSkillLevelUp,              //英雄<(c0xFF8000FF)【%s】>的技能<(c0xFF00FF00)【%s】>提升到%d级
	tpHeroSkillNotActivate,            //该英雄技能尚未激活，不能升级
	tpHeroEquipTimeup,				//英雄装备%s过时已消失
	tpSmileCd,                    //表情正在冷却中

	tpStoreItemNotForSale,			//该商品没有上架销售，不能购买
	tpStoreItemNumErr,				//购买商城商品数量错误,
	tpStoreItemMustPay,				//商城商品必须付费购买
	tpStoreItemDailyLimit,			//购买商城商品超过今日限额

	tpLootCatherMonsterBroadCast,  //【%s】%s 从 %s，获得物品%s！
	tpAddExp,					//+%d经验
	
	tpStoreBroadCast , //xx从城花费xx购买了
	tpChatMoneyLimit,  //在世界频道聊天需要%d银两

	tpBigSmileFee,	//非vip发送大表情需要%d银两，可惜你不够
	tpLimitStoreBroadCast , //xx从城花费xx购买了

	tpGuildSbkDate,		//%d-%d 20:00-21:00
	tpGuildSbkBack,		//周四回收沙城,不可报名攻城
	tpAddAcitivity,    //您获得了xxx点活跃度
	tpLoseAcitivity,	//"你失去了%d点活跃度",

	tpQuestDoubleAwardFail,  //任务刷星 
	tpQuestTrust,   //任务多倍奖励
	tpQuestFinishImmediately,  //任务立刻完成
	tpCampShout,		//%s在[%s]的[%d,%d]处呼救，请速去帮忙
	tpNoViewbleHero,    //对方没有可以查看的宠物
	tpHeroGet,  //恭喜%s获得珍贵的灵兽%s，从此闯荡江湖有了有力的助手！
	tpNoGuildLevel,		//帮派等级不够不能升级
	tpNoGuildCoin,		//帮派资金不够
	tpNoGuildYs,		//帮派玥石不够
	tpCoinFlush,		//委托公告

	tpCreateGuildSuccess,	//XX阵营的XX成功创建帮派：%s。
	tpInvitLimiltLevel,			//邀请的玩家的等级不足xx级
	tpApplyLimiltLevel,			//您的等级不足%s级不能加入帮派
	tpGetGuildGx,				//获得帮派贡献%d
	tpDelGuildGx,				//消耗帮派贡献%d
	tpInGuildDart,				//该玩家在帮派运镖过程中，为帮派做贡献，无法开除。
	tpGuildInfuben,				//该玩家在帮派斗魂副本中，为帮派做贡献，无法开除。
	tpMeInFuben,				//正在帮派斗魂副本中，不能退出帮派
	tpNoDelGuildInfuben,		//正在帮派斗魂副本中，不能解散帮派
	tpNoticGuildFrMsg,			//请注意：贵帮派繁荣度已低于2000，可捐献兴帮令或积极参与帮派任务、帮派活动提高繁荣度。当帮派繁荣度低于1000，帮派自动解散。  row: 352
	
	//帮派事件
	tpGuildEventStart,				//帮派事件的开始
	tpGuildEventCoin,				//<(%s)> 捐献了%d行会钱票
	tpContributeEvent,				//((c0xFFC7A232)%s)>  捐赠兴帮令×1
	tpAddToGuildEvent,				//加入本帮派
	tpLeftGuildEvent,				//离开了本帮派
	tpUpGuildPosEvent,				//从 XX 升为 XX
	tpDownGuildPosEvent,			//从 XX 降为 XX
	tpGuildHead,					//帮主
	tpGuildSecHead,					//副帮主
	tpGuildTangZhu,					//堂主
	tpGuildMember,					//帮众
	tpChangeGuildLeaderEvent,		//将帮主之位禅让给
	tpDeleteMemEvent,				//开除帮派
	tpCreateGuildEvent,				//创建了帮派
	tpAddBossScoreEvent,			//花费10元宝膜拜斗魂 13
	tpAddBossScoreByItem,			//使用斗魂石×1膜拜斗魂
	tpAddBossScoreByZh,				//消耗战魂膜拜斗魂
	tpkillGuildBossEvent,			//全体成员合力将 <(c0xFFC7A232)%s(%d级)> 击败,获得帮派贡献%d,帮派繁荣%d,帮派玥石%d
	tpGuildDartEvent,				//顺利完成帮派运镖，增加帮派资金%d，获得帮贡%d、经验%d
	tpGuildUpSkill1Evnet,			// 花费帮派资金%d，玥石×%d将虎威破虏升级至%d级
	tpGuildUpSkill2Event,			//花费帮派资金%d将虎威破虏升级至%d级
	tpUpGuildBossEvent,				//在众人膜拜下境界提升至<(c0xFFC7A232)%s>（%d级）
	tpFinishQuestEvent,				//顺利完成帮派任务，增加帮派繁荣10000，获得帮贡500、经验152410
	tpUpGuildLevelEvent,			//"<(c0xFFC7A232)%s>花费帮派资金%d将本帮派升级至%d级",
	tpKillLuoYangMonster,			//【%s】在洛阳战场击败【%s】为本帮派增加【%d】帮派资金和【%d】帮派繁荣
	tpGetBattleGuildGX,				//在全体帮众的努力下，本帮派在此次洛阳战中获得【%d】帮派资金和【%d】帮派繁荣
	tpGetBattleFr,					//【%s】在洛阳战场击败【%s】获得【%d】帮贡
	tpEditGuildTitle,				//<(c0xFFC7A232)%s>编辑了行会封号
	tpSetActorTitle,				//<(c0xFFC7A232)%s>把<(c0xFFC7A232)%s>分配至<(c0xFFC7A232)%s>
	tpUpguildlevelEvent,			//<(c0xFFC7A232)%s>花费%d元宝将成员上限扩充至%d人
	tpDeleteGuildMsg,				//您所在的帮派由帮主解散。您的帮派贡献减半，背包或仓库中的战旗与斗魂石等帮派专属道具已删除。对您造成的不便，敬请谅解。
	tpDeleteGuildOffMsg,			//您所在的行会资金在1000点或以下，根据规则删除贵行会。对您造成的不便，敬请谅解。
	tpGuildAddCoin,					//<(n%s)>捐献%d绑定金币,增加行会资金%d，增加行会贡献%d
	tpAddToGuildmsg,				//欢迎%s加入本帮派！从此同舟共济！共闯江湖！
	tpGetAwardFaild,				//领取奖励失败
	tpPlayerViewYou,				//xx正在偷偷打量你哦
	tpAddGuildFr,					//获得帮派繁荣%d
	tpTransmitPattern,				//"<速传/F%s:%d:%d:%s:%d>"
	tpServerIdPattern,				//"[%d区]"
	tpChannelSecret,				//私聊
	tpChannelNear,					//同屏
	tpChannelHorn,					//传音
	tpChannelGuild,					//帮派
	tpChannelTeam,					//队伍
	tpChannelBigTeam,				//团队
	tpChannelMap,					//地图
	tpChannelTipmsg,				//系统提示
	tpChannelZhenying,				//好友
	tpChannelSystem,				//系统
	tpChannelWorld,					//世界
	tpChannelHelp,					//呼救
	tpSubmitFrequent,				//您提交的过于频繁，请稍事休息。
	tpUnderMinLevel,				//您的等级不够，需要达到%d才能进入
	tpUpMaxLevel,					//您的等级超过%d不能进入
	tpAwardGiftBag,					//<n%s>成功兑换了充值给力礼包<(c0xFF00FF00)%s>！
	tpGmSetGuildLeader,				//恭喜<n%s>成为<(c0xFF00FF00)%s>的帮主
	tpGuildLeaderName,				//首领
	tpGuildAssistLeaderName,		//"副首领",
	tpGuildTangzhuName,				//"官员",
	tpGuildCommonName,				//"成员",

	tpExpandMan,					//行会扩充人口
	tpSetGuildQSuc,					//设置成功
	tpNoMoreGuildCoin,				//行会资金不足,无法宣战
	tpIsUnionRelation,				//已经是同盟关系
	tpEnemyRelation,				//你们是敌对关系不能添加为同盟
	tpCreateGuildNeedItem,			//你没有行会号角不能创建行会
	tpCreateGuildNeedCoin,			//你没有足够金币，不能创建行会",
	tpBecomeUnionGuild,				//X行会与X行会正式结盟，同创伟业！
	tpRefuseUnion,					//X行会拒绝与贵行会联盟。
	tpRemoveUnionRelation,			//A行会与B行会正式解除联盟关系！
	tpIsUnionCanNotBeEnmery,		//你们已是联盟，不对成为敌对
	tpIsEnemryRelation,				//你们已是敌对关系
	tpIsEnemryTips,					//A行会将贵行会视为敌对行会！
	tpRefuseEnemryRelation,			//贵行会与B行会正式解除敌对关系！
	tpIsUnionDecare,				//同盟关系不能宣战

	
	tpPutItemToDepot,				//<(c0xFF00FF00)%s>存入<(c0xFF00FF00)%s> ×<(c0xFF00FF00)%d>，设定取出价格<(c0xFF00FF00)%d>%s
	tpGetBackItem,					//<(c0xFF00FF00)%s>收回<(c0xFF00FF00)%s> ×<(c0xFF00FF00)%d>
	tpGetItemFromDepot,				//<(c0xFF00FF00)%s>取出<(c0xFF00FF00)%s> ×<(c0xFF00FF00)%d>，花费<(c0xFF00FF00)%d>%s
	tpNoMoreCoin,					//金币不够
	tpNomoreYubao,					//元宝不够
	tpGetItemFromGuildDepot,		//行会仓库取物品
	tpRankGuild,					//行会竞技排名
	tpUpGuildRankTip,				//恭喜贵行会排名提升到%d
	tpGuildRankSame,				//竞价不够，行会排名不变
	tpSetTitleOneTimes,				//一天只能编辑封号一次

	tpLootNotOwner,                 //目前还归属其他人，稍后才能拾取
	tpLootParamError,                 //参数错误
	tpLootExpired,                 //要拾取的物品已经过期
	tpLootPosError,                //不在同一个坐标，无法拾取

	tpHaveStall,					//你已在摆摊啦
	tpNeaerHaveStall,				//附近有人在摆摊
	tpStallerOnffline,				//摊主不在线
	tpStallBindItem,				//摆摊失败，%s物品已绑定，不能摆摊
	tpNotStallState,				//%s不是摆摊状态，不能查看
	tpBuyStallNeedMoney,			//购买此物品需要%d，你的只有%d
	tpSendStallAdMoney,				//发送摊位广告需要%d金钱，你只有%d
	tpNeedBagGrid,					//此次购买需要预留%d个背包空位
	tpLeaveWord,					//留言后给予提示信息，您已成功向摊主留言
	tpDealNoStall,					//您正在交易，不能摆摊哦
	tpStallNoUseItem,				//"摆摊状态下不能使用物品",
	tpStallNoCall,					//摆摊状态下不能召唤

	tpNoAcceptSignUp,				//周三不接受报名
	tpIsFriendGuild,				//贵行会占领方不用报名
	tpSignUpAgain,					//贵行会已报名 
	tpNoMoreSignGuildCoin,			//行会资金不够%d
	tpSignUpSecces,					//成功报名明天的<沙城战(20:00-21:00)/@@showWin,82,1>

	tpGetMoneyFromGuildDepot,		//您存入行会仓库的%s被%s取出，获得%d金币
	tpGetYbFromGuildDepot,			//您存入行会仓库的%s被%s取出，获得%d元宝
	tpSkillTrainPetCountLimit,      //无法召唤更多的下属

	tpPetSleep,  //下属休息
	tpPetAttack, //下属攻击

	tpSbkGuildWarEnd,				//本轮沙巴克攻城战结束！<(c0xFFC7A232)%s>成功占领沙巴克！所向无敌！
	tpNoOccupiedGuild,				//本轮沙巴克攻城战结束！很遗憾！居然没有行会有能力占领沙巴克！
	tpWeaponCurseAdd2,					//武器被诅咒了, 幸运减少，当前诅咒值为：%d
	tpStallBuyLog,					//购买了Ｘ个【物品】

	tpNoMetiral ,  //没有装备材料
	tpMetiralDuraLimit, //装备的耐久度不够
	tpSetSbkPostionSuc,				//设置成功
	tpHoldSbkCity,					//XXX（行会名）暂时占领沙巴克皇宫！他们会不会笑到最后呢？勇士们都快来抢城了！
	tpIsMaxGrid,					//
	tpWeaponCurseAdd,				//武器被诅咒了,幸运减少，当前幸运值为：%d
	tpNewMemBerTalk,				//我是新加入行会的成员，请各位大哥大姐多多指教！
	tpActorInGuild,					//对方已加入其它行会
	tpOnPkStateNow,					//进入pk状态
	tpOutPkState,					//退出pk状态
	tpGetHonorTips,					//获得荣誉
	tpAddToGuildTimeUp,				//该玩家脱离行会后%d小时内不能加入其他行会
	tpBidGuildRankNotice,			//<(c0xFFC7A232)%s>正在通过<#c0xFF00FF00#竞价/@@showSysWin,7，3>抢夺行会排名
	tpBeTheUnionNotice,				//联盟申请已发出，请等候回复
	tpOnDeclarState,				//贵行会与<(c0xFFC7A232)%s>宣战中。为了行会荣誉，奋勇杀敌去！
	tpSkillExpChange,               //技能%s的熟练度变为%d

	tpAwardTypeExp,             //经验
	tpAwardTypeGuildCont,       //帮派贡献
	tpAwardTypeZyCont,             //阵营贡献
	tpAwardTypeAchievePoint,             //成就点
	tpAwardTypeActivity,             //活跃度
	tpAwardTypeGuildFr,             //帮派繁荣度
	tpAwardTypeHonour,             //荣誉
	tpAwardTypeCircleSoul,             //转生灵魄

	tpKillMonsterGetAward,         //获得道具

	tpGetCircleSoul,     //"获得转生灵魄%d",
	tpLostCircleSoul,    //失去转生灵魄%d

	tpActorOffline,			//玩家不在线
	tpAddFriendNotice,		//您添加了好友%s
	tpAddEnemyTips,			//<(c0xFFC7A232)%s>添加至您的仇人名单中
	tpTrackActorOffline,		//您要追踪的玩家当前不在线！
	tpFriendMaxCount,				//您的好友人数已达上限
	tpBeActorEnemy,					//您是%s的仇人，无法进行本操作
	tpFriendLoginTips,				//您的好友%s上线了！

	tpNotVipMoney,					//抱歉，购买此服务的钱不够
	tpHaveGetAward,					//今天的奖励你已领取过了哦
	tpDealDistanceTooLong,         //距离太远，无法交易
	tpDealNumTooLarge,             //一次交易金额太大
	tpJobNameWarrior,				//战士
	tpJobNameMagician,				//法师
	tpJobNameWizard,				//道士

	tpSignUpOpenSvr4Day,			//成功报名开服第三天的<沙城战(20:00-21:00)/@@showWin,82,1>
	tpSignUpTodayAuto,				//成功报名今天的<沙城战(20:00-21:00)/@@showWin,82,1>
	tpTopickBattleNotice,			//本日主题活动领先玩家：<(c0xFFC7A232)%s>。建议玩家保持绝对领先优势！
	tpTopicBattleLastTips,			//23:00本日主题活动截至，当前领先玩家：<(c0xFFC7A232)%s>。建议玩家保持绝对领先优势！
	tpAccountBattleNotice,			//恭喜<(c0xFFC7A232)%s>成为本日“职业战力大比拼”主题活动的获奖者，获得十大珍宝之一：<(c0xFFC7A232)%s>

	tpDealTargetLevelLimit,   //对方的等级不够%d级，无法进行交易
	tpDealSelfLevelLimit,     //您需要在%d级以后才可以与他人交易

	tpSafeAreaDestroyForbid,		//安全区不能这么做
	tpSbkMasterLoginOnTips,			//全体行注目礼！称霸整个大陆的<(c0xFFC7A232)%s>行会的掌门，伟大的沙巴克城主<(n%s)>上线了
	tpUseSilverVip,					//购买至尊爵位

	tpGetGuildCoinTips,				//获得行会资金%d
	tpSignUpGuildWarLoginTips,		//贵行会已报名参与明天的沙巴克攻城战(20:00-22:00)，请做好充分准备，攻下沙巴克！
	tpGetAngerTips,     //"获得怒气%d",
	tpLostAngerTips,    //失去怒气%d
	tpNoMoreAnger,		//怒气不够%d

	tpSkillConditionDigTool,       //需要装备鹤嘴锄

	tpSkillConditionDigToolNoDua,  //鹤嘴锄的没耐久了 

	tpSkillConditionArea,  //特殊的场景才能使用

	tpLootCanPickUpTime,		//未达到能拾取的时间

	tpBlessGift,				//[废弃]

	tpSetSbkMasterWife,			//无上光荣！<(n%s)>任职屠龙城的城主夫人！
	tpSetSbkPosSecMonster,		//"无上光荣！<(n%s)>任职屠龙城的副城主！",
	tpSetSbkPosTips2,			//无上光荣！<(n%s)>任职屠龙城的大将军！
	tpSetSbkPosTips3,			//无上光荣！<(n%s)>任职屠龙城的大政令！
	tpSetSbkPosTips4,			//无上光荣！<(n%s)>任职屠龙城的大总管！
	tpTheMaxLevel,				//已达最大等级，不能再升级了
	tpRideMaxLevel,				//坐骑星等已最高，无法再提升

	tpIGuildCallCanNotUse,		 //本区域内不能使用拉传
	tpGuildCallResultNotUse,	//本区域内不能被拉传

	tpDropPlayerBroad,			//<(n%s)>在【%s】中掉落【%s】，坐标%d,%d
	tpDropMonsterBroad,			//【怪物名字】在【地图名字】中掉落【物品名字】，坐标XXX,XXX
	tpPetGetLevel,				//您的召唤兽已经达到%d级
	tpPetUpgradeFail,			//使用失败，您没召唤兽或都超过%d级了
	tpFcmHalfIncome,		 //防沉迷收入减半
	tpFcmZeroIncome,		 //防沉迷收入为0

	tpSetCityPosNotGuildMem,		//只能设置本行会的成员为官员
	tpRoleNotExists,		//抱歉，赠送失败，没有<(n%s)>这个角色名
	tpGiveFrdBagFull,		//<(n%s)>的背包无法放下赠送的物品，请通知他整理再赠送",
	tpGetGiveStoreItem,		//恭喜你成功领取<(n%s)>赠送给你的商城物品
	tpGiveOfflineStoreItem, //[%s]赠送给你商城物品:%d个[%s]，请接收
	tpGiveItemOKNotice,		//恭喜你送给<(n%s)>的%d个%s商城物品的操作成功
	tpGiveItemBagFull,		//背包已满不能接收赠送物品，请留出足够空间下次登陆再接收
	tpGiveBroad,			//"【<(n%s)>】非常慷慨的赠送了【<(n%s)>】%d件【%s】，真是羡煞旁人！",
	tpNoStorePoint,			//你的商城积分不够，购买该物品需要%d商城积分
	tpBuyedStoreItem,		//恭喜你用%d积分购买了%d个%s商城物品

	tpNotSbkCityOwner,		//不是沙城城主，无法装备
	tpNotOpenRideSystem,		//开服到达第四天并且等级到达50级才开启坐骑
	tpTagetFriendToMax,			//对方好友人数已达上限

	tpNotRightToSignUp,				//只能由行会首领、副首领报名沙城战
	tpTagetGuildLeaderOffline,		//对方行会的首领、副首领不在线，无法收到联盟邀请

	tpGuildWarEndTips,				//	"<(c0xFF00FF00)%s>行会与<(c0xFF00FF00)%s>行会的宣战结束,<(c0xFF00FF00)%s>行会击杀<(c0xFF00FF00)%s>行会<(c0xFF7CFC00)%d>人次,<(c0xFF00FF00)%s>行会击杀<(c0xFF00FF00)%s>行会<(c0xFF7CFC00)%d>人次",
	tpRequestFriendTips,			//已向对方发出添加好友申请，请等候回复

	tpInDartCanNotEnterFuben,		    //正在护送美女中，无法进入副本（可回到村长处放弃再操作）
	tpInDartCannotEnterActivity,		//正在护送美女中，无法参与活动（可回到村长处放弃再操作）
	tpInDartCannotBeCall,				//该玩家正在护送美女中，无法被召唤（可回到村长处放弃再操作）
	tpInDartCannotBeBuild,				//正在护送美女中,无法传送(可回到村长处放弃再操作)

	tpHaveMasterTips,					//你已有师父,不能再拜师
	tpSendApplyMaster,					//你的拜师请求已发出
	tpRefuseApplyMaster,				//%s拒绝了你的拜师请求

	tpCanNotApplyMaster,				//你的等级不符合拜师的条件
	tpCanNotBeYourMaster,				//您的拜师目标与您的等级在5级之内
	tpCanNotBeYourMaster2,				//不能拜低于自己等级的玩家为师傅
	tpCanNotBeMaster,					//你的等级不符合做师父
	tpStudentLevelError,				//目标等级太低，不能收其为徒
	tpCanNotBeYourStudent,				//您的收徒目标与您的等级在5级之内
	tpCanNotBeYourStudent2,				//不能收高于自己等级的玩家为徒弟

	tpStudentBeToMax,					//徒弟人数已达上限
	tpSendAskStudent,					//你的收徒请求已发出
	tpRefuseBeYourStudent,				//%s拒绝了你的收徒请求

	tpApplyStudentSucess,				//%s已成功成为你的徒弟
	tpBeYourMasterSucess,				//%s已成功成为的师父
	tpCanNotGatherNow,				//暂时不能采集
	tpGruasuateSuccess,				//你的门下弟子<(c0xFF00FF00)%s>在你的指导下成已出师!
	tpUpLevelGiveMasterAward,		//名师出高徒！门下弟子<(c0xFF00FF00)%s>在你的指导下升级至<(c0xFF00FF00)%d>
	tpUpLevelGiveStudentAwawd,			//伯乐赏识！你在<(c0xFF00FF00)%s>指导下升级至<(c0xFF00FF00)%d>

	tpTagetHaveMasterTips,				//对方已有师父
	tpTagetHaveMaxStudentTips,			//对方的徒弟人数已满

	tpGatherNotOwner,					//不是你的主人
	tpDeleteMasterNoCoin,				//你的金币不足,无法叛离师门
	tpDeleteStudentNoCoin,				//你的金币不足,无法逐出弟子
	tpDeleteMasterSucess,				//成功叛离师门
	tpYourStudentDelteTips,				//你的徒弟<(c0xFF00FF00)%s>叛离师门
	tpDeleteStudentSucess,				//成功逐出弟子
	tpYourBeDeleteToStudent,			//你被师父<(c0xFF00FF00)%s>逐出师门了

	tpYourMateOnlogin,					//你的伴侣%s上线了
	tpMasterOnLoginTips,				//你的师父%s上线了
	tpStudentOnLoginTips,				//你的徒弟%s上线了

	tpYourMateloginOut,					//你的伴侣%s下线了
	tpMasterLoginOutTips,				//你的师父%s下线了
	tpStudentLoginOutTips,				//你的徒弟%s下线了
	tpFriendLoginOutTips,				//你的好友%s下线了

	tpSendMsgToMateGradute,				//你的同门XXX 在成功升至X 级出师!诚心祝贺他! 送花祝贺

	tpInDartCanNotRide,					//正在护送美女,不能上坐骑
	tpSkillTrainCircleLimited,         //转数限制
	tpAddEnamySuccess,					//你将%s加成仇敌
	tpSkillExpIsFull,					//技能的熟练度已经加到最大
	tpHeroSkillExpItemLimited ,			 //需要%s或者%d元宝增加熟练度
	
	tpHeroSkillClose ,					//英雄的技能关闭
	tpHeroSkillOpen,                    //英雄的技能开启
	tpHeroSkillPassiveCanNotClose,                    //英雄的技能开启
	tpHeroCanNotGetExp,                    //英雄已经无法获得经验了
	tpHeroCanNotGetMore,                    //英雄数目已满，无法再增加
	tpHeroNotBattle,					//需要出战的英雄才能使用
	tpHeroLevelMax,						//您的英雄已经达到70级,建议转生后再使用.我要转生
	tpHeroLevelAboveOwner,				//您的英雄已超过你的等级%d级,无法使用
	tpHeroChangeNameSuccess,			//恭喜您更改英雄名字成功

	tpSbkCityName,						//沙城
	tpNoGuildCanNotUsed,				//没有行会，不能使用
	tpHaveBeYourFriend,					//%s已是你的好友
	tpPetRemove,						//%s跟随时间到，自动消失

	tpNewPlayerProtectSelf,					//您正处于新手保护状态，不可攻击他人，30级后此状态将自动解除。
	tpNewPlayerProtectOther,				//对方处于新手保护状态，攻击无效，30级后此状态将自动解除。
	tpNewPlayerProtectDel,					//失去新手保护状态

	tpLogMonsterDropItem,					//怪物%s掉落
	tpLogSendBigSmile,						//发送大表情
	tpLogWorldChat,							//世界频道聊天
	tpLogSellChannal,						//出售频道
	tpLogDealWith,							//与%s交易
	tpLogSendFriPos,						//好友发送位置
	tpLogDeleteMaster,						//叛离师门
	tpLogDriveOutMaster,					//逐出师门
	tpLogBuyStallItem,						//购买摆摊物品
	tpLogSellStallItem,						//出售摆摊物品
	tpLogGuildHelp,							//行会求救
	tpLogDestroyCoin,						//丢弃金币
	tpLogEnlargeBag,						//扩充背包
	tpLogResultPoint,						//增加玩家的返点数
	tpLogMeetPetRemCoin,					//偶遇宠物扣除主人金币
	tpGetTodayActivityAward,				//恭喜获得今日的全民活动奖励
	tpPetDropMasterItems,					//苦逼！衰神让XXX 丢失XXX ×14
	tpPetNotDropMstItems,					//百无禁忌！XXX 躲过衰神一击！无损失！
	tpPetDropMasterMoney,					//悲催！穷神让XXX 丢失金币×6666
	tpPetNotDropMstMoney,					//XXX 居然比穷神还穷啊？各位兄弟姐妹资助一下这位可怜人吧！
	tpHeroDropEquip,                         //玩家爆出装备

	tpInPataBattle,							//你正在努力的扫荡，不能移动的哦
	tpInStallState,							//你正在摆摊，等收摊后再去参加吧
	tpInDartGril,							//你正在护送美女，等护送完成再去参加婚礼吧
	tpGuildSbkCmd1Tips,						//全军集合！

	tpGuildSbkCmdNoticy,					//当前占领行会:\\<(c0xFFFFFF)%s>\\\\<(c0xFFDAA520)提示:>沙皇宫只留下自己行会的人就可以临时占领行会！坚持到最后吧！

	tpFriendChatLmt,						//好友消息发送失败，您达到%d级后才可以给好友发送消息


	tpNoMoreGuildCoinSendCmd,				//贵行会的行会资金不足，无法发送指令

	tpGuildBeKillTips,						//<(c0xFFFFFF)%s>的%s<(n%s)>在<(c0xFFFFFF)%s>被<(n%s)>击败,甚是狼狈!
	tpGuildSecLeaderBeKilledTips,			//<(c0xFFFFFF)%s>的副首领<(n%s)>在<(c0xFFFFFF)%s>被<(n%s)>击败,甚是狼狈!（不使用）
	tpSbkGuildLeaderBeKilledtips,			//沙城城主<(n%s)>在<(c0xFFFFFF)%s>被<(n%s)>击败，城主威严荡然无存!

	tpSpeedSingleAward,						//很抱歉，任务加速至少需要%d元宝
	tpSpeedDoubleAward,						//双倍任务加速需要%d元宝

	tpFinishAuthQuest,						//恭喜你完成了自动完成任务%s并得到奖励
	tpNotHeroMsg,							//暂时没有该英雄信息

	tpGMQuickBuffName,                      //GM的buff的名字

	tpNotFreeBagCannotGetAwards,			//背包的格子不足%d,不能领取奖励

	
	tpBookQuestHaveFinished,				//你的任务目标已经完成，不需要再次完成
	tpBookQuestNoMoney,						//本次操作需要%d%s
	tpBookQuestSupItem,						//天书任务得到惊喜物品
	tpBookQuestGetSupItem,					//背包空格不足，不能完成任务。
	tpBookQuestArriveMaxLv,					//你已刷新到本等级段最高级数
	tpBookQuestNoCoinFlush,					//你刷星的%s不够哦
	tpBookQuestFinTimes,					//今日次数已用完，您还可以使用下面的“+”按钮额外增加任务次数
	
	tpBookQuestMaxAccpetTimes,				//该任务的次数已做满，请明天再来领取
	tpHaveQuestNotFlush,					//你已经领了任务，不能再刷星
	tpExtraAcceptConsume,					//额外接任务消耗元宝
	tpYbFinBookTarget,						//用元宝只完成目标
	tpYouInPrison,							//你在监狱中，不能转送

	tpAddRideUseTimeSuc,					//恭喜你激活坐骑成功
	tpNoTranScene,							//抱歉，该场景不能传送的哟
	tpInStallLootItem,						//你正在摆摊，不能捡地上的物品哦

	//仓库密码锁
	tpDepotLockNotLocked,					//您仍未设置仓库密锁
	tpDepotLockUnLocked,					//您的仓库仍未被锁定
	tpDepotLockPwdError,					//您所输入的密锁有误
	tpDepotLockOnUnLock,					//您已临时解除仓库密锁
	tpDepotLockOnLock,						//您已经恢复仓库保护功能
	tpDepotLockLocked,						//您仓库保护功能正在运行中
	tpDepotLockSetLockSuc,					//设置仓库密锁成功
	tpDepotLockChgLockSuc,					//修改仓库密锁成功
	tpDepotLockDelLockSuc,					//您取消了仓库保护功能
	tpDepotLockPwdErr,						//密码不能为空或含有非法字符

	tpNoMoreCircleNotUse,					//转生次数不够不能使用
	tpUpCircleCanNotUse,					//转次数过大不能使用

	tpBackRemoveItem,						//后台删除玩家物品
	tpBackRemoveMoney,						//后台扣除玩家金钱
	
	tpNoChangeNameItem,						//没有改名字的道具
	tpChangeNameSuc,						//修改名字成功
	tpNeInvalidName,						//名称无效，名称中包含非法字符或长度不合法
	tpNeNameinuse,						//名称已被使用
	tpFirstDayLevelAwardTips,			//恭喜<(n%s)>成为开服首日“冲级狂人”主题活动的获奖者，获得十大珍宝之一：<(c0xFF00FF00)战神宝甲>
	tpGMRemoveItemNotice,					//亲家的玩家，GM帮您删除了一个物品
	tpGmRemoveMoneyNotice,					//亲家的玩家，GM帮您扣除了一部分金钱
	tpDepotDepositNoMoney,					//存入失败，您的背包没有足够%d可存入
	tpDepotWithdrawNoMoney,					//取出失败，您的仓库没有足够%d可取出
	tpDepotDepositSucc,						//恭喜你，存入成功%d%s
	tpDepotWithdrawSucc,					//恭喜你，取出成功%d%s
	tpSceneNotTeamCall,						//此场景不能使用队伍召唤
	tpVipClearRedName,						//VIP清洗红名
	tpNoDropItemArea,						//此场不能丢弃物品

	tpCrossSbkEndMsg,						//恭喜%d区<(c0xFF00FF00)%s>在<(n%s)>带领下成功占领沙城！本战区所向披靡！
	tpIsCrossGuildCanNotChange,				//参加跨服沙城战的行会不能禅让会长
	tpSceneCanNotCrossTransfer,             //当前场景无法跨服传送
	
	tpChangeNameNotOpen,					//改名功能暂为开启，敬请期待！
	tpCrossServerIsClose,                    //跨服没有开启
	tpIsCrossServering,                    //正在进入跨服务器


	tpNoGuildCanNotSignSbk,					//没有行会不能报名跨服沙城战
	tpNotGuildPosCanNotSignUp,				//只有副掌门以上的职位才可以报名
	tpCanNotSignTimeNotUp,					//只有星期三并且开服10天后才可以报名
	tpNoSingCrossItem,						//没有跨服攻城头像
	tpSignCrossToGuildMem,					//贵行会已报名周四跨服沙城战，请做好充分准备攻下跨服沙城战!
	tpIsSignCrossGuildSbk,					//贵行会已报名跨服沙城战

	tpGetGuildGxTips,					//获得行会贡献
	tpDelteGuildGxTips,					//消耗行会贡献

	tpInSbkWar,							//在攻城战期间（20:00-21:00），行会禁止职务调动
	tpSkillTrainNoHasJobPetCount,        //当前召唤出的宠物不足
	tpGuildCoinAward,					//行会捐献完后领取奖励
	tpHaveGetCoinAward,					//你今天已领取过捐献奖励，请明天继续捐献
	tpNoGetCoinAward,					//当天捐献还没完成，请完成捐献资金后再来领取
	tpNoticeGetAward,					//主殿等级对应的捐献已满，请通知行会成员及时领取奖励

	tpTopHeroCannotUseForLevel,			//英雄等级不够，无法使用
	tpTopHeroCannotUseForCircle,		//英雄转数不够，无法使用
	tpNoHero,							//你没有英雄，无法使用
	tpNoAttackNotice,					//在XX模式下，不能对其造成伤害
	tpPkModePeace,						//和平
	tpPkModeTeam,						//队伍
	tpPkModeGuild,						//行会
	tpPkModeEvil,						//善恶
	tpPkModePk,							//PK
	tpPkModeCamp,						//阵营
	tpNoFireArea,						//您不在大篝火的范围内，无法获得经验
	tpDepotDepositMoneyMax,				 //存入失败，仓库可存%s金额已到上限
	tpBookFinTimesOver,					//今日已完成的比骑任务次数大于最大次数，不能继续购买
	tpMaxBuyBookTimes,					//你今天购买的天书任务次数已达最大值
	tpNoMoneyToBuyBook,					//购买一次天书任务需要%d元宝
	tpBuyBookTimesLog,					//天书任务购买次数

	tpAnimyTips,						// "上次击杀你的仇人：%s\\可在仇人面板查看并追踪其踪迹"
	tpSendRingFail,						//抱歉，传送戒指传送失败
	tpSendRingSucc,						//恭喜你，通过传送戒指达到目的地
	tpSenderRingCding,					//传送戒指传送功能冷却中

	tpAddFriendLevel,					//请%d级后再添加好友
	tpRideCanEquip,						//你没骑上特殊坐骑，不能装备

	tpNoTeamCallArea,					//此区域不能使用召唤
	tpNoItemDigBoss,					//购买物品
	tpGuildBuyVipErr,					//您给%d个成员购买Vip，需要%d元宝
	tpGuildBuyVipLog,					//帮主给成员购买VIP
	tpSbkWifeIsFemale,					//城主夫人只能设置成女性玩家
	tpWhoWalkRide,						//%s\\%s的坐骑
	tpAddGuildVip,						//国泰兴帮，你的帮主为了你充了VIP
	tpNoDigBossPersonCount,						//没有可挖次数了
	tpDigBossSucc,						//恭喜你从BOSS中挖出宝贝
	tpInMonsterNoRide,					//现在是变身状态，不可乘骑
	tpYouGraduate,						//您成功出师了
	tpYourPupilGraduate,				//您的徒弟%s成功出师了
	tpPupilToTeachetExp,				//您离线期间，您的徒弟们给你贡献%d点经验

	tpCancel,							//取消
	tpSameTypeBuffExist,				//已经有了相同类型的buff，是否覆盖"
	tpStoreMailTitle,					//赠送物品
	tpStoreMailContent,					//恭喜获得XXXX赠送X个【XXX】
	tpSDLevelMailGiftTest,				//"《武易传奇》删档封测赠送元宝",
	tpSDLevelMailGiftContent,			//"恭喜您升级达到%d级，获得%d元宝。祝游戏愉快！\n继续升级会有元宝奖励，请多多支持武易传奇！\n若在游戏中发现Bug，或者对游戏有任何建议的，还请通过小地图旁边的“Bug提交”按钮，把内容提交给我们。我们一定会认真阅读您提交的建议及问题，将武易传奇完善至您最喜欢的游戏"
	tpStallLevelLimited,					//"摆摊系统需要45级"
	tpActivityMailTitle,					//给您发的物品
	tpActivityMailContent,					//物品在附件请，请注意查收
	tpNoCirclePotentialPoint,					//转生之力不足
	tpInDartNotStall,					//正在护送美女中，不能摆摊
	tpAchievePointLimited,				//"成就点不足"
	tpExchangeAchieveBage,				//兑换成功

	tpAchieveAwardItem,					//成就兑换获得勋章
	tpRideEquipNoGrid,					//请在背包留出一个空格子
	tpRideEquipToBag,					//坐骑装备到背包
	tpGetMailItemGrids,					//领取邮件附近需要%d个空格
	tpNoGuildHelp,						//该区域不允许使用求救功能
	tpCombineDrawYbGift,				//合服充值大惊喜，恭喜<(n%s)>成功领取[%s]！


	tpNoGuildMainHall,					//行会主殿等级不足
	tpNoGuildTech,						//行会科技等级不足
	tpBattleNoRide,						//战斗状态不能上坐骑
	tpCircleSoulStr,					//转生灵魂更新
	tpRideUnActivity,					//请先激活本阶坐骑再使用		

	tpDepotDeposit,						//存入仓库货币
	tpDepotWithdraw,					//取出仓库货币

	tpSbkPosErrorLeaderChange,			//对方已有沙巴克职位，不能禅让。
	tpSbkPosErrorChange,				//对方已有沙巴克职位，不能设置。
	
	tpCityPosErrorInSiege,				//攻城战期间不能设置职位
	tpNewHundredSBKMailContent,			//百服首个沙巴克邮件内容

	tpFiveAttrNameGold,					//金
	tpFiveAttrNameWood,					//木
	tpFiveAttrNameWater,				//水
	tpFiveAttrNameFire,					//火
	tpFiveAttrNameEarth,				//土

	tpSkillTrainHpRateAbove,			//自身的血的百分比必须高于一定的值
	tpFubenCollectNotItem,				//锤子数量不足，请找副本NPC购买

	tpWarChariotNoTeleport,				//战车状态下不能传送。
	tpWarChariotNoRide,					//战车状态下不能上坐骑。
	tpWarChariotNoStall,				//战车状态下不能摆摊。
	tpWarChariotNoGuildCall,			//战车状态下不能传送。
	tpWarChariotNoSkill,				//此技能在战车状态下不能使用。
	tpCanNotGatherBySBKGuild,			//只有沙巴克攻城双方才能采集

	tpEquipPropMoveLog,					//装备转移属性
	tpEquipMovePropNoYb,				//转移该装备需要%d元宝

	tpWarChariotNoClickNpc,				//战车状态不能使用NPC
	tpOnlyCollectOnce,					//抱歉，不能重复采集

	tpEquipMoveError,					//目标装备的强化等级比材料装备的强化等级高，不需要转移！
	tpEquipMoveInitSmith,				//极品属性转移
	tpEquipMoveStrong,					//强化属性转移
	tpEquipMoveAuth,					//鉴定属性转移
	tpEquipMoveLuck,					//幸运属性转移
	tpBuffAddMoney,						//buff增减Money

	tpMovePropInitSmith,				//转移极品属性
	tpMovePropStrong,					//转移强化属性
	tpMovePropAuth,						//转移鉴定属性
	tpMovePropLuck,						//转移幸运属性
	tpTakeOnRideInterval,				//%d秒后才能再次上马
	tpNotDamageBySkill,					//不能使用本技能攻击
	tpSendMailFormat,					//%s %d-%d发邮件日志格式	
	tpExpDoubleWorldLevel,				//获得经验%d点，其中%d点为多倍经验,世界等级奖励%d点额外经验
	tpExpWorldLevel,					//获得经验%d点，世界等级奖励%d点额外经验
	
	tpCannotDepotBagByStall,			//摆摊状态下不能使用仓库

	tpAddNewTitle,						//"增加头衔",
	tpDelNewTitle,						//"撤销头衔",
	tpOfferTaskWaitAcceptErr,			//"此悬赏任务不处于未接取状态",
	tpNoLevelGetActAward,				//你与世界等级差距超过%d级，不能领取
	tpCanNotUseByVipLevel,				//vip等级不足不能使用

	tpNotDropItemCommon,				//跨服场景不能丢弃

	tpCoinChanged,						//"金币数量变化",
	tpGuildSbkAwardError,				//"需要有英雄城官职才能领取奖励",
	tpOnNpcEventMsg,					//"%s\\NPC Function Error:%s"
	tpCannotSendFriendByStore,			//跨服中不能使用赠送功能。
	tpCanNotSendFriendByNotOnLine,		//人物不在线，不能使用赠送功能

	tpMoneyDescYuanbao,				  //元宝
	tpMoneyDescBindYuanbao,			  //礼券
	tpMoneyDescCoin,				  //金币
	tpMoneyDescBindCoin,			  //绑定金币
	tpMoneyNotEnough,				  //%s数量不足

	tpLevelNotEnoughGather,			 //等级不足%d级，不能采集
	tpNoMoreMoney,					//已经无法获取更多的%s,请及时消费
	tpPkChange,						//"此怪物只能被战车攻击"

	tpGuildMaxLevel,				//"帮派已经升级到最大等级"
	tpOnlineMinAwardOpen,			//"在线分钟元宝奖励已开启, 等级限制(%d,%d)",
	tpOnlineMinAwardClose,			//"在线分钟元宝奖励已关闭, 等级限制(%d,%d)",
	tpClearOnlineMiniute,			//"清除玩家[%s]的今日在线时长（分钟）",
	tpClearOnlineMinAward,			//"清除玩家[%s]的今日在线元宝奖励数据",
	tpOnlineMiniute,				//"玩家[%s]今日在线%d分钟"
	tpOnlineMiniuteAwardFlag,		//"玩家[%s]今日第%d个在线元宝奖励领取标记为:%d",

	tpNotSbkSignInOpen2Day,			//"开服前两2天不能手工报名",
	tpNoGuildToBuy,					//"请加入行会再购买"
	tpSbkGuildWarEndMaster,			//恭喜城主%s获得任期内杀人免加PK值特权，同时所有英雄行会以外的玩家击杀英雄城所有成员，不会增加PK值！
	tpTargetRejectDeal,				//"对方拒绝接收交易"
	tpGuildSiegeAutoSignMsg,		//今晚有资格进入沙皇宫占领皇宫赢得最终胜利的行会名单已出，请前往查看			
	tpGuildSBKMsg1,  // "贵行会已报名今日英雄城战（20:00—21:00），请做好充分准备攻下英雄城；"
	
	//行会攻城
	
	tpGuildSiegeNoSignTeleport,		//需要有占领资格的行会成员才可进入皇宫
	tpGuildSetPosNotLeader,			//不是君主，不能设置职位
	tpSetCityPosGuildLeaderCantSet, //无法设置君主作为护法
	//寄卖
	tpConsiFull,					//上架商品数量已满，请下架商品后再操作
	tpConsignLevelLimit,			//等级不足%d,不能寄卖
	tpConsignHaveNotItem,			//背包没有该物品,不能寄卖
	tpConsignItemNotCount,			//该物品数量不足
	tpConsiItemBinded,				//绑定物品不能寄卖
	tpConsiItemCantSell,			//该物品不能寄卖
	tpConsiItemPriveInvalid,		//寄卖价格超出范围
	tpConsignItemNotItem,			//寄卖的物品已不存在
	tpConsignCantGetNotYou,			//该物品不是你的，不能取回
	tpConsiBagFullCannotOp,			//您的背包空格不足，请整理后再操作
	tpConsignItemOffShelf,			//该物品已经过期下架了
	tpConsiNoYuanbaoToBuy,			//您的元宝不足，无法购买物品
	tpConsignBuyItemCountexceed,	//超出了可购买数量
	tpConsiBuySuccAddToBag,			//购买成功

	tpTargetNotGuildTran,		//"目标区域不能使用拉传",
	tpRideTopicRideBattleTopName,	//坐骑战力 %s

	tpBuyBanneret1,					//优惠购买至尊爵位
	tpSOCIALANIMAL,					//你是A的仇人，无法添加为好友
	tpReCallOtherInFuben,			//对方所在地图无法被召唤
	tpReSOCIALANIMAL,				//无法添加仇人为好友，请先删除仇人

	tpGuildEventCall,				//%s使用了一键召唤功能
	tpGuildEventDeclare,			//贵行会与%s行会宣战
	tpGuildEventBeDeclare,			//%s行会与贵行会宣战
	tpGuildEventUnion,				//贵行会与%s行会联盟
	tpGuildEventBeUnion,			//%s行会与贵行会联盟
	tpBroadcastGetItem,				//玩家名字在神秘回廊获得XXXXXX！极品属性谁与争锋！
	tpDigBossStoreNotEnough,		//空格子不够40,请清理寻宝仓库
	tpDigBossNoTime,				//当前BOSS 已无剩余可挖次数
	tpDigBossGetItem,				//"超好人品！<n%s>挖掘%s时获得了%s）！"
	tpLootPicked,					//物品已被拾取
	tpNotHaveSkill,           //该技能未学习，无法增加熟练度。
	tpIsHighestSkill,          //该技能已满级。
	tpCanNotUseSkillExpItem, //该技能暂无法升级，请提升角色等级
	tpUseSkillExpItemSucc, //增加熟练度成功
	tpSelfAddTeam,	// 您已成功加入队伍
	tpAddTeamMember,	// 欢迎[%s]加入我们队伍
	tpSelfLeavTeam,	// 您已离队
	tpDelTeamMember,	// %s已离队
	tpDeathDropExp,		//死亡掉落经验 -%d
	tpLogDepotBugLog,		//购买仓库页面

	tpTrackActorFeeUnenough,	//追踪玩家费用不足
	tpLogDepotMiss,			//仓库页面过期
	tpLogDepotMailTitle,	//仓库过期主题
	tpLogDepotMailContent,	//仓库过期内容
	tpExploitGet,			//"获得功勋%d点",
	tpExploitTooWeak,		//你击败的玩家太过弱小，没有获得战绩！
	tpExploitLimit,		   //你的战绩已达%d上限，无法再获得
	tpExploitLv,		   //请在40级后再修改ＰＫ模式
	tpExploitMy,		   //当前功勋值%d！
	tpBeSkilled02,			//"你在战斗中被[%s]击败",	不显示掉落功勋
	tpSafeAreaKill,			//安全区不能这么做
	tpModeNoKill,			//"此ＰＫ模式下无法对他人造成伤害",
	tpfpEvil,				//此ＰＫ模式下只能对褐名、红名玩家造成伤害
	tpExploitLimitToday,	//你的当日战绩已达%d上限，无法再获得
	tpEnterF,				//进入战斗状态
	tpEnterPK,				//进入PK状态
	TpPK00009,				//"您为%s求情，减少其-%dPK值",
	TpPK00010,				//"的好友%s为您求情%dPK值",
	TpPK00011,				//"你的PK值-%d",
	TpPK00012,				//"您的求情次数已满，明天请早",
	tpGuildNullSearch,		//"输入搜索信息错误",
	tpDelMail,				//成功删除邮件%d封,
	tpGuildPosChange,		//%s被变更为行会%s!
	tpGuildObjHasBeenPos,	//对方已经是%s

	tpMailFullCantAdd,		  //邮箱已满，请清空邮箱以便接收新邮件
	tpPersonBossJifen,        //个人BOSS积分
    tpPersonBossJifenAdd,     //"获得个人BOSS积分%d",

	tpGuildJoinApplyAlready,	//"您已经提出申请，请耐心等待审核",
	tpGuildImpeachObjOnline,	//对方在线，不能弹劾
	tpGuildImpeachSelf,			//不能弹劾自己
	tpGuildImpeachMemberPosErr, //你的官职高于或等于弹劾职位，弹劾失败
	tpGuildImpeachEvent,		//%s弹劾了%s,成为%s
	tpGuildInProtectDay,		//该官员在%d保护期内
	tpNoGuild,					//没有加入行会
	tpGuildFlftAndClearGx,		//您离开了行会，行会贡献度清0
	tpGuildChannelName,			//行会通告
	tpGuildLevelup2Fruit,		//行会主殿提升到%d级，神树果实品质飞跃为（%s, %s）
	tpBagFullCantGather,		//背包已满，不能采集
	tpGetForce,                 //获得体力%d点
	tpLostForce,                //失去体力%d点
	tpAddInnerExp,				//获得内劲经验%d点
	tpFrindNotOnline,           //好友不在线，无法邀请
	tpTeamFubenIsFull,          //队伍已满员,无法邀请加入
	tpTeamFubenIsNotExist,      //队伍不存在，请刷新列表
	tpTeamFubenIsBegin,         //队伍已在副本中，无法邀请加入
	tpFriendInOtherTeamFuben,   //好友已经在别的副本队伍，无法邀请加入
	tpConsignAddIncomeMailTitle,//出售商品成功
	tpConsignAddIncomeMailContent,//您的商品%s已有买家购买%d个，获得收益：%d，请及时在寄售面板领取您的收益。
	tpConsignSearchNothing,		//没有搜索到相应商品
	tpConsignCantBuyMySelf,		//这是你寄卖的商品，无法购买
	tpExploitKillNormalGet,		//你击败其他玩家，获得战绩xxx ，战绩可以兑换大量经验。
	tpExploitKillGuildMemberGet,//你击败的玩家为非联盟行会成员，获得双倍战绩xxx ，战绩可以兑换大量经验。
	tpExploitToolessToReduce,	//您被%s击败，因为你的战绩少得可怜，所有没有丢失
	tpExploitBeKilledReduceExploit,	//您被%s击败，失去战绩%d ，赶紧先把战绩换经验吧。
	tpExploitTitleChange,			//刀刀直达厉害之处，XXX战绩再上一层，获得称号XXXXX
	tpQuitFubenTeam,            //你离开了队伍
	tpHasFubenTeam,             //你已经创建了副本队伍，不能再创建组队
	tpConsignBuyPriceError,			//购买金额有误，请重新操作

	tpBookQuestAcceptNpcNotFound,			//"找不到接取渡魔任务的NPC",
	tpBookQuestAcceptNpcTooFar,				//"距离接取渡魔任务的NPC太远",
	tpBookQuestCommitNpcNotFound,			//"找不到提交渡魔任务的NPC",
	tpBookQuestCommitNpcTooFar,				//"距离提交渡魔任务的NPC太远",
	tpBookQuestCommitLevelLimit,			//进行渡魔任务需要达到%d级
	tpBookQuestCondFail,					//不满足渡魔任务接取条件//
	tpInTeamFuben,                          //对方在团队副本中，不能邀请
	tpKickedTeam,                           //您已被踢出副本队伍，1分钟内无法再次加入该队伍

	tpBookQuestRefreshOk,					//"渡魔任务刷星成功",			--渡魔任务
	tpBookQuestRefreshFail,					//"渡魔任务刷星失败",			--渡魔任务
	tpForce,                                //体力值
	tpMergeBindInfect,						//道具合并(绑定感染)
	tpCaptainRedName,                       //队长红名状态，无法自动开始团队副本
	tpDieBroadKiller,						//"%s最后一击玩家：<(n%s)>",
	tpDieBroadVester,						//"%s掉落归属玩家：<(n%s)>",
	tpFriendSearchNull,						//查询名称不能为空
	tpTeamFubenReturnMailTitle,             //团队副本返还体力
    tpTeamFubenReturnMailContent  ,         //您在团队副本获得返还体力，请查收！
	tpMailForceIsFull, 				        //体力已满，邮件暂不能提取

	tpSupplyBattleCamp1,					//"云闻",	--补给争夺阵营1
	tpSupplyBattleCamp2,					//"雍勒",	--补给争夺阵营2
	tpInEscort1,  //您在押运中，不能组队
	tpInEscort2,  //对方在押运中，不能组队。
	tpAddNewTitleBroadcast,//玩家XXX获得了头衔XXX，名扬四海！
	tpAddNewTitleTime,		   //头衔 XX 续期成功，时效增加 XX秒
	tpClientCutOverNewTitleId,	//头衔 XX 已回收,无法佩戴
	tpAddDoubleInnerExp,				//获得内劲经验%d点，其中%d点为多倍内劲经验.
	tpFlyingShoes,					//道具不够不能传送
	tpConsignShoutTimeLimit,		//点击太快，请稍后再吆喝
	tpConsignShoutMsg,				//一位勇士以%d元宝出售%s ×%d！欲购从速，先到先得！我要抢购
	tpInEscortCanNotRide,			//急速押运中,不能上坐骑
	tbBookAwardRateErr,				//错误的奖励倍数
	tpAddGodShieldValue,			//获得神盾碎片%d
	tpReduceGodShieldValue,			//减少神盾碎片%d

	tpGuildDepotPutInNotCircleItem,		//非转生装备，不能放入行会仓库
	tpGuildDepotGetOutGxUnEnough,		//行会贡献不足%s，不能兑换
	tpGuildDepotPutInItem,				//%s在行会仓库中投入了%s，获得%d行会贡献
	tpGuildDepotGetOutItemDelGx,		//兑换成功消耗行会贡献%d
	tpGuildDepotGetOutItem,				//%s在行会仓库中兑换了%s，消耗%d行会贡献
	tpGuildDepotDelItem,				//%s%s在行会仓库中丢弃了%s

	tpAddBloodSoulValue,			//获得血魄碎片%d
	tpReduceBloodSoulValue,			//减少血魄碎片%d	
	tpAwardTypeExploit,				   //战绩
	tpAwardTypeInnerStrengthExp,	  //内劲经验
	tpAwardTypeGodShieldValue,		 //兵魂值
	tpAwardTypeBloodSoulValue,		 //启兵值
	tpSkillTrainSoldierSoulActAllStar,//需要激活对应兵魂的所有珠子
	tpGetWingPoint,						//获得%d羽魂
	tpLostWingPoint,					//失去%d羽魂
	tpGuildDepotPutInBind,				//绑定装备或不可交易装备，不能放入行会仓库
	tpGetBossJiFen,						//"获得Boss积分%d",
	tpLostBossJiFen,					//"失去Boss积分%d",
	tpHasLearnSkill,					//已经学习了该技能

	tpGuildDepotPutInNotEquip,			//不是装备，不能投入行会仓库
	tpGuildDepotPutInMaxNum,			//今日投入行会仓库的次数已经达到%d次
	tpJoinTeamFubenMsg,                 //我已加入队伍,可在队伍频道与队友沟通

	tpGuildDepotPutInMsg,                //"投入装备，获得行会贡献<(c0xFFFF00)%d>"
	tpDigBossGetItemMailTitle,			//挖Boss奖励
	tpDigBossGetItemMailContent,		//您刚刚在BOSS挖宝中获得以下奖励，请查收

	tpGuildDepotPutInUnvalidType,		//该类型装备不能投入行会仓库

	tpEscortCannotAttack,            //镖车血量已降至最低，攻击无效!
	tpAddDragonSoulValue,			//获得龙魂碎片%d
	tpReduceDragonSoulValue,		//减少龙魂碎片%d
	tpAddIntellectBallValue,		//获得智珠碎片%d
	tpReduceIntellectBallValue,		//减少智珠碎片%d
	tpAwardTypeDragonSoulValue,		//龙魂碎片
	tpAwardTypeIntellectBallValue,	//智珠碎片
	tpHeroGetForce,				 //英雄获得体力%d点
	tpIsAchieveGiveAwards,				//你已经领过了
	tpIsAchieveFinished,				//未达成不可领取

	tpGuildDepotExchangeNo,			//该行会仓库物品没有人申请兑换
	tpGuildDepotExchangeTitlt1,		//"行会仓库装备兑换通过",
	tpGuildDepotExchangeCont1,		//"申请兑换行会仓库装备的请求审批通过",
	tpGuildDepotExchangeTitlt2,		//"行会仓库装备兑换拒绝",
	tpGuildDepotExchangeCont2,		//"你申请兑换行会仓库装备的请求审批通过",

	tpDepotGridNumLimit,					//格子数量不足
	tp_depotBagIsFull,						//仓库已满
	tpCannotm_depotBagByStall,				//摆摊状态不能使用仓库
	tpGuildDepotItemCheckingNoExchange,		//该行会仓库物品正等待审核，不能兑换
	tpGuildDepotItemCheckingNoClear,		//"该行会仓库物品正等待审核，不能清理
	tpGuildDepotItemExchangeAgree,			//"您同意了%s 兑换 %s的申请",					--
	tpGuildDepotItemCheckingReject,			//"您拒绝了%s 兑换 %s的申请",
	tpGuildDepotItemDoExchange,				//"您的兑换申请已经提交，请耐心等待"
	tpGuildDepotItemExchangeCircleLimit,	//"可以直接兑换的转数不能低于%d转",
	tpGuildDepotItemExchangeCircleSet,		//可直接兑换的转数被设置为%d转",

	tpGuildDepotExchangeTitlt3,			//"行会仓库装备申请超时",
	tpGuildDepotExchangeCont3,			//"你申请兑换行会仓库装备的请求超时，请查收返还的行会贡献",

	tpCombatScore,						//竞技宗师积分
	tpCombatChallengeCountLimit,		//魔神令不足
	tpCombatPickCountLimit,				//竞技宗师筛选次数限制
	tpCombatCDLimit,					//竞技宗师挑战cd中
	tpCombatLevelLimit,					//需要%d级才能开始挑战

	tpHeroNameIsEmpty,					//英雄名字不能为空
	tpQuestIsDoing,						//任务正在进行中，不能重复接取
	tpMainQuestHadDone,					//该主线任务已做了一次，不能再接
	tpNewAchieve,						//


	tpStoreCanNotUse,					//商品不能立即使用

	tpGuildDeleteMailTitle,				//"行会被解散",								--
	tpGuildDeleteMailCont,				//"您所在的行会由于行会资金不足，已解散"
	tpRemoveGuildDepotItem,				//"您成功删除了%d件%d转装备"

	tpGuildEventCreate,					//%s创建了行会
	tpGuildEventMemberJoin,				//%s加入了行会！
	tpGuildEventMemberLeft,				//%s离开了行会！
	tpGuildEventTickMember,				//%s将%s踢出了行会！
	tpGuildEventLeaderChange,			//%s把首领禅让给%s
	tpGuildEventAddOfficer,				//%s被任命为%s！
	tpGuildEventDelOfficer,				//%s被撤销了%s！
	tpGuildEventlevelUp,				//本行会已成功提升至%d级！
	tpGuildEventImpeach,				//%s弹劾成功，成为了%s！
	tpGuildEventPutInDepot,				//%s在行会仓库中投入了%s！
	tpGuildEventGetOutDepot,			//%s在行会仓库中兑换了%s！

	tpGuildCallCoinNotEnough,			//行会资金不足,不能集结
	tpGuildCallSceneNotRight,			//该区域不能行会集结或响应集结
	tpGuildCallNotOwner,				//只有首领才能集结
	tpGuildCallTargetIsNotOnLine,			//首领已经下线,无法集结
	tpGuildCallTargetSceneNotRight,				//首领不在可集结区域
	tpGuildCallChatInfo,			//首领<(n%s)>消耗了%d资金,召唤所有在线成员前往他所在的地图
	tpEscorting,					//护送物资中，无法进行该操作
	tpKillScoreDouble,             //<(c0xFFFFFF00)战绩+%d点>,其中%d为双倍战绩
	tpKillScore,  //<(c0xFFFFFF00)获得功勋%d点>
	tpInviteMsgSend,  //邀请信息已发送
	tpHeroGetExpRate,                    //战神获得经验%d点,其中%d为多倍经验
	tpNoMoreGuildCoinUnion,				//行会资金不足,无法同盟
	tpTeamLevelLimited,					//等级不足%d级,不能组队
	tpOtherTeamLevelLimited,			//对方等级不足%d级,不能组队
	tpKilledByActor, //<(n%s)>在<(c0xFFFFFF)%s>被<(n%s)>击败!		
	tpMaintainTopTitle,					//恭喜%s成为了第一%s，万人之上
};
