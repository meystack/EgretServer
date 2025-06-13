#include "StdAfx.h"
#include "ChatSystem.h"
#include "ctype.h"
#include "../base/Container.hpp"
// 方便组批量修改的宏定义
#define GM_LEVEL0  0
#define GM_LEVEL1  1
#define GM_LEVEL2  2
#define GM_LEVEL3  3
#define GM_LEVEL4  4
#define GM_LEVEL5  5
#define GM_LEVEL6  6
#define GM_LEVEL7  7
#define GM_LEVEL8  8
#ifndef _DEBUG
#define GM_LEVEL9  9
#else
#define GM_LEVEL9  0
#endif
#define GM_LEVEL10  10

//GM指令是以@开头的命令,使用空格分隔，比如@additem 102 1 用[]括起的参数不输入则使用默认
const CChatSystem::GMHANDLE CChatSystem::GmCommandHandlers[]=
{
	{"Go",					GM_LEVEL9,		&CChatSystem::GmGo}, 			// 移动到指定地图坐标    @Go 地图名(或场景id) X(default) Y(default)  
	{"enter",				GM_LEVEL9,		&CChatSystem::GMEnterFuben}, 			// 移动到指定地图坐标    @Go 地图名
	{"Monster",				GM_LEVEL9,		&CChatSystem::GmMonster},  		// 生成怪物 			@Monster 怪物名(或id) 数量(1) 存活时间(0) 等级(0) 血量比(100)
	{"Skill",				GM_LEVEL9,		&CChatSystem::GmLearnSkill},  	// 学习技能             @Skill 技能ID 技能等级
	{"DelSkill",			GM_LEVEL9,		&CChatSystem::GmDeleteSkill},  	// 删除技能             @DelSkill 技能ID
	{"PR",					GM_LEVEL9,		&CChatSystem::GmPR},			// 测试坐标旋转			@PR 相对位置x 相对位置y
	{"AR",					GM_LEVEL9,		&CChatSystem::GmAR},			// 测试坐标旋转			@AR 鼠标位置x 鼠标位置y 相对位置x 相对位置y
	{"AddBuff",				GM_LEVEL9,		&CChatSystem::GmAddBuff},  		// 添加BUFF 			@AddBuff buffId
	{"DelBuff",				GM_LEVEL9,		&CChatSystem::GmDelBuff},  		// 删除BUFF 			@DelBuff buffId
	{"CatchMonster",		GM_LEVEL9,		&CChatSystem::GmCatchMonster},  // 抓取怪物				@CatchMonster 怪物名
	{"Item",				GM_LEVEL9,		&CChatSystem::GmAddItem},  		// 添加物品				@AddItem 物品id 物品num(默认为1)
	{"DelItem",		        GM_LEVEL9,		&CChatSystem::GmDelItem},  		// 删除物品				@DelItem 物品id 物品num(默认为1)
	{"WorldMessage",		GM_LEVEL9,		&CChatSystem::WorldMessage},  	// 系统公告消息 		@WorldMessage 消息内容 公告提示类型
	{"Level",				GM_LEVEL9,		&CChatSystem::GmSetSelfLevel},  // 设置自己的等级 		@Level 等级
	{"AddValue",			GM_LEVEL9,		&CChatSystem::AddValue},  		// 增加数值 		    @AddValue  type num
	{"SetValue",			GM_LEVEL9,		&CChatSystem::SetValue},  		// 设定数值 		    @SetValue  type num
	{"DropItem",			GM_LEVEL9,		&CChatSystem::GmDropItem},  	// 掉落物品 		    @DropItem  type num
	{"Info",				GM_LEVEL9,		&CChatSystem::Info},  			// 查询信息 			@info xxx arg
	{"drop",				GM_LEVEL9,		&CChatSystem::Drop},  			// 掉落表          		@drop id
	{"AddMoney",			GM_LEVEL9,		&CChatSystem::GmAddMoney},		// 添加金钱 			@AddMoney 金钱类型 金钱数量 （类型:0绑定金币 1金币 2绑定元宝 3元宝）
	{"addCircleNum",		GM_LEVEL9,	    &CChatSystem::GMAddCircleNum},	// 添加没转人数 		@addCircleNum 转数 人数 
	{"deathdrop",		    GM_LEVEL9,	    &CChatSystem::GMDeathDropItem},	// 添加没转人数 		@deathdrop
	{"addservermail",		GM_LEVEL9,	    &CChatSystem::GmAddServerMail},	// 添加全服邮件 		@addservermail  [] [] [] [] [] [] [] [] [] [] [] [] [] [] [] [] []
	{"setJm",				GM_LEVEL9,	    &CChatSystem::GMSetJMLv},	// 设置经脉等级 			@setJm  lv
	{"setJyEd",				GM_LEVEL9,	    &CChatSystem::GMSetTradingQuota},	// 设置交易额度 	@setJyEd value
	{"setBless",			GM_LEVEL9,	    &CChatSystem::GMSetBlessValue},	// 设置祝福值 	@setBless value
	{"setQuestState",		GM_LEVEL9,	    &CChatSystem::GMSetQuestState},	// 设置任务状态 	@setQuestState id state
	{"clearQuestlog",		GM_LEVEL9,	    &CChatSystem::GMClearQuestlog},	// 清空任务记录 	@clearQuestlog
	{"Attr",				GM_LEVEL9,		&CChatSystem::GmAttr},			// 设置 @Attr attrId value 
	{"MonthCard",			GM_LEVEL9,		&CChatSystem::GmMonthCard},		// 设置 @Attr attrId value 
	{"DamageInfo",			GM_LEVEL0,		&CChatSystem::GmDamageInfo},	// 设置输出伤害 @damageinfo 1/0
	{"make",				GM_LEVEL9,		&CChatSystem::GmMakeRealItem},	// 造物品 @Make [物品id] [数量] 场景id 怪物id
	{"ChgJob",				GM_LEVEL9,		&CChatSystem::GMChgJob},		// 改变职业 @ChgJob 职业id 性别id
	{"addPet",				GM_LEVEL9,		&CChatSystem::GMADDLootPet},		// 改变职业 @ChgJob 职业id 性别id

	//5级权限以上才能使用	
	{"Move",				GM_LEVEL9,		&CChatSystem::GmMove}, 			// 移动到指定地图       @Move 地图名称
	{"Monster2",			GM_LEVEL9,		&CChatSystem::GmMonster2 },		// 生成怪物 		    @monster2 怪物名字 外形id
	{"AddItem",				GM_LEVEL9,		&CChatSystem::GmAddItem}, //添加物品 @AddItem 物品ID [数量] [品质] [强化等级]
	{"AddPet2",			    GM_LEVEL9,		&CChatSystem::GmAddPet }, //添加宠物 @addpet 宠物的名字 [强化] [品质]
	{"AddHero",				GM_LEVEL9,		&CChatSystem::GmAddHero }, //添加英雄 @AddHero 英雄ID，英雄阶，英雄等级
	{"DoMapActor",			GM_LEVEL9,		&CChatSystem::GmDoMapActor }, //

	{"Shutup",				GM_LEVEL9,		&CChatSystem::GmShutup},  //禁言 @Shutup 角色名称 时间（分钟）
	{"AddExp",				GM_LEVEL9,		&CChatSystem::GmAddSelfExp}, //添加自己的经验 @AddExp 经验值
	{"superman",			GM_LEVEL9,		&CChatSystem::GmSetSuperMan},  //添加任务dQuest 任务id
	{"ReGoto",				GM_LEVEL9,		&CChatSystem::GmReGoto}, //传送至指定角色身边 @ReGoto 角色名称
	{"Who",					GM_LEVEL9,		&CChatSystem::GmWho},  //获取服务器在线人数 @Who 
	{"AdjustExp",			GM_LEVEL9,		&CChatSystem::GmAdjustExp}, //设置别人的经验 @AdjustExp 玩家的名字 数量   
	{"ReleaseShutup",		GM_LEVEL9,		&CChatSystem::ReleaseShutup},  //将指定角色禁言状态解除 @ReleaseShutup 角色名称
	{"ShutupList",			GM_LEVEL9,		&CChatSystem::ShutupList},		//查看禁言列表 @ShutupList
	{"NotifyMsg",			GM_LEVEL9,		&CChatSystem::NotifyMsg},  //给指定玩家发消息 @NotifyMsg 角色名称 消息内容
	{"AdjustLevel",			GM_LEVEL9,		&CChatSystem::GmAdjustLevel},  //调整目标玩家的等级 @adjustlevel 玩家的名字 玩家的等级
	{"MoveToNPC",			GM_LEVEL9,		&CChatSystem::GmMoveToNPC}, //移动到指定地图指定NPC位置 @MoveToNPC 地图名称 NPC名称 
	{"kick",				GM_LEVEL9,		&CChatSystem::GmKick},  //踢人 @kick 玩家名字
	{"sethide",				GM_LEVEL9,		&CChatSystem::GmSetActorHide},			//设置玩家隐身 @sethide 玩家名称 是否隐身（0 显示 1 隐身）
	{"setgm",				GM_LEVEL9,		&CChatSystem::GmSetGm},							//设置Gm的头衔	@setgm 添加头衔（0 取消 1 添加）

	{"Recall",				GM_LEVEL9,		&CChatSystem::GmRecall}, //将指定角色传送到身边 @Recall 角色名称 
	{"targetIntPro",		GM_LEVEL9,		&CChatSystem::GmSetTargetIntPro},  //设置目标属性 @targetIntPro 属性ID 属性的值
	{"TraceSystem",			GM_LEVEL9,		&CChatSystem::GmTraceSystem},  //开启或者关闭系统的网络消息输出 @TraceSystem
	{"ReloadNPC",			GM_LEVEL9,		&CChatSystem::GmRefresh},  //刷新挂在NPC身上的脚本([SYS FUNCTION:全局功能NPC, SYS MONSTER:全局怪物脚本]) @ReloadNPC NPC的名字  
	{"RSF",					GM_LEVEL9,		&CChatSystem::GmReloadGlobalNpc},//重新载入全局npc的脚本 @RSF
	{"RS",					GM_LEVEL9,		&CChatSystem::GmReloadScriptNpc},//重新载入角色所在的场景的所有npc的脚本 @RS
	{"ReloadLang",			GM_LEVEL9,		&CChatSystem::ReloadLang}, //刷新语言包 @ReloadLang	
	{"ReloadItem",			GM_LEVEL9,		&CChatSystem::ReloadItem},  //刷新道具配置 @ReloadItem 
	{"ReloadMonster",		GM_LEVEL9,		&CChatSystem::ReloadMonster},  //刷新怪物配置 @ReloadMonster
	{"call",				GM_LEVEL9,		&CChatSystem::CallScriptFunc},  //调用脚本函数 @call 新手老人 getNpc,1,2 :注意是执行本人所在的场景的npc的脚本
	{"callf",				GM_LEVEL9,		&CChatSystem::CallScriptFuncNoEntity},  //调用脚本函数 @callf 新手老人 getNpc,1,2 :注意是执行本人所在的场景的npc的脚本（不传入实体参数）
	{"AddTargetHero",		GM_LEVEL9,		&CChatSystem::GmAddTargetHero},  //给目标玩家的添加英雄 @AddTargetHero 玩家的名字 英雄的职业 性别 等级 转数
	{"SkillCD",				GM_LEVEL9,		&CChatSystem::GmSkillCDSwitch},  //设置技能CD启用与否 @SkillCD 0/1（关闭/开启）
	{"Acceptrole",			GM_LEVEL9,		&CChatSystem::Acceptrole},  //增加一个任务 @AddQuest 任务id
	{"SetZBPMtime",			GM_LEVEL9,		&CChatSystem::GMSetZBPMtime},		//@setZBPMtime 10 2014-3-10-12:00:00 
	{"SetZBPMdata",			GM_LEVEL9,		&CChatSystem::GMSetZBPMdata},		//@setZBPMdata 30,31,32 100,200,300 10,20,30 
	{"FinishQuest",			GM_LEVEL9,		&CChatSystem::FinishQuest},  //设置任务完成状态 @FinishQuest 任务id	设置完成还是未完成（1是完成，0是未完成）
	{"IntPro",				GM_LEVEL9,		&CChatSystem::GmSetIntProperty},  //设置玩家属性 @IntPro 属性ID 属性的值(整数型的)
	{"showintpro",			GM_LEVEL9,		&CChatSystem::GmShowIntProperty},	//查看玩家属性 #showIntPro 属性ID
	{"AddKeyword",			GM_LEVEL9,		&CChatSystem::GmAddKeyword},  //添加关键字到词库 @AddKeyword 关键词 分类(1:敏感词库, 2禁言词库)
	{"WorldMessage",		GM_LEVEL9,		&CChatSystem::WorldMessage},  //系统公告消息 @WorldMessage 消息内容 公告提示类型
	{"AddGold",				GM_LEVEL9,		&CChatSystem::GmAddGold},	//GM增加指定角色金钱 @AddGold [角色名称] [金钱类型] [金钱数量],,支持对不在线的用户发放金钱，除了元宝
	{"Present",				GM_LEVEL9,		&CChatSystem::GmAddPresent},	//发派物品	@Present 人物名称 物品名称 [数量=1] [品质=0] [强化=0] 是否绑定[yes=1,no=0] 文本信息(参考addgold的说明)
	{"AddFbCount",			GM_LEVEL9,		&CChatSystem::AddFubenCount},  //增加（或减少）副本的次数	@AddFubenCount [人物名称] [副本名称] [数量，负数是减少]
	{"TraceActor",			GM_LEVEL9,		&CChatSystem::GmTraceActor},  //追踪一个玩家的网络数据 @TraceActor
	{"DumpFb",				GM_LEVEL9,		&CChatSystem::GmDumpFuben},  //
	{"checkset",			GM_LEVEL9,		&CChatSystem::GmCheckGameSet},

	///// 暂时没有明确归类的	
	{"Icon",				GM_LEVEL9,		&CChatSystem::GmChangeIcon},  //更改头像 @Icon  头像ID
	{"Property",			GM_LEVEL9,		&CChatSystem::GmPrintProperty},  //打印隐藏的属性 @Property  
	{"Invite",				GM_LEVEL9,		&CChatSystem::GmInviteTeam},  //要求玩家组队 @Invite 玩家名字
	{"LeaveTeam",			GM_LEVEL9,		&CChatSystem::GmLeaveTeam},  //离开队伍 @LeaveTeam 	
	{"SetMonsterMove",		GM_LEVEL9,		&CChatSystem::GmSetMonsterMove},  //GM设置怪物的运动状态 @SetMonsterMove
	{"targetIntPro",		GM_LEVEL9,		&CChatSystem::GmSetTargetIntPro},  //@targetIntPro  属性ID 属性的值
	{"viewPro",				GM_LEVEL9,		&CChatSystem::GmViewTargetProperty},  //@viewIntPro 
    {"ClearBag",			GM_LEVEL9,		&CChatSystem::GmClearBag},	//清空背包 @ClearBag
	{"reloadfw",			GM_LEVEL9,		&CChatSystem::GmRefreshfw}, //刷新过滤字 @reloadfw
	{"ShowFriendList",		GM_LEVEL9,		&CChatSystem::GmShowFriendList},
	{"skill",				GM_LEVEL9,		&CChatSystem::GmLearnSkillByName},  //@LearnSkill 技能ID 技能等级
	{"useskill",			GM_LEVEL9,		&CChatSystem::GmUseSkill},			//@useskill 技能ID 技能等级
	{"forgetskill",			GM_LEVEL9,		&CChatSystem::GmForgetSkillByName},  //@forgetskill 技能名字
	{"SetEquipItemProp",	GM_LEVEL9,		&CChatSystem::SetEquipItemProp},  //@SetEquipItemProp 装备槽位号 propid proval
	{"SetExpGetToday",		GM_LEVEL9,		&CChatSystem::SetExpGetToday}, // @SetExpGetToday 数量
	{"SetTraceOpt",			GM_LEVEL9,		&CChatSystem::SetTraceOpt}, // @SetTraceOpt 类型 开启标志
	{"RefreshMonster",		GM_LEVEL9,		&CChatSystem::RefreshMonster}, // @RefreshMonster
//	{"AddBtTime",			GM_LEVEL9,		&CChatSystem::AddBanneretTime }, // @AddBanneretTime type time(秒为单位)		
	{"SetHeadTitle",		GM_LEVEL9,		&CChatSystem::GmSetHeadTitle }, //@SetTopTitle 头衔ID   0 清空头衔
	{"RefreshRank",			GM_LEVEL9,		&CChatSystem::GmRefreshRank }, //@RefreshTopTitle 刷新排行榜
	{"RefreshHeadTitle",	GM_LEVEL9,		&CChatSystem::GmRefreshHeadTitle}, //@RefreshTopTitle 刷新在线玩家头衔
	{"vtp",					GM_LEVEL9,		&CChatSystem::ViewTargetProp }, //@vtp propid
	{"fcmopen",				GM_LEVEL9,		&CChatSystem::GmSetFcmOpen }, //@fcmopen 
	{"fcmclose",			GM_LEVEL9,		&CChatSystem::GmSetFcmClose }, //@fcmopen 
	{"nreload",				GM_LEVEL9,		&CChatSystem::GmReloadConfigByName }, //@nreload 配置的名字  
	{"ireload",				GM_LEVEL9,		&CChatSystem::GmReloadConfigById }, //@ireload  通过ID装载配置
	{"dmopen",				GM_LEVEL9,		&CChatSystem::GmSetGambleOpen }, //@dmopen  打开赌博系统	
	{"dmclose",				GM_LEVEL9,		&CChatSystem::GmSetGambleClose }, //@dmclose  关闭赌博系统	
	{"setchatlevel",		GM_LEVEL9,		&CChatSystem::GmSetChatLevel }, //@setchatlevel   near 或 World 或 Secret 或 Guild 或 Team 1
	{"setchatrecharge",		GM_LEVEL9,		&CChatSystem::GmSetChatRecharge }, //@setchatrecharge   near 或 World 或 Secret 或 Guild 或 Team 1 
	{"setchatforbitlevel",	GM_LEVEL9,		&CChatSystem::GmSetChatForbitLevel }, //@setchatforbitlevel   最大等级
	{"dbspc",				GM_LEVEL9,		&CChatSystem::GmGetDBSendPacketCount}, //@dbspc   
	{"asi",					GM_LEVEL9,		&CChatSystem::GmSetActorSaveInterval}, //@asi time
	{"kill",				GM_LEVEL9,		&CChatSystem::GMKill },	//@kill 玩家名字 
	{"tracePacket",			GM_LEVEL9,		&CChatSystem::TracePacket }, //@tracepacket 玩家名称
	{"addguildys",			GM_LEVEL9,		&CChatSystem::GuildAddYs }, //@tracepacket 帮派玥石
	{"ReqTran",				GM_LEVEL9,		&CChatSystem::ReqTransmit }, // @ReqTransmit destServerId
	{"si",					GM_LEVEL9,		&CChatSystem::TraceServerInfo }, // @si 打印服务器信息
	{"smf",					GM_LEVEL9,		&CChatSystem::SetActorMsgFilter }, // @sf 设置角色消息过滤器
	{"testssb",				GM_LEVEL9,		&CChatSystem::TestSessionBroad }, // @testssb
	{"startCs",				GM_LEVEL9,		&CChatSystem::GmStartCommonServer }, // @stopCs 关闭连接到跨服
	{"setGuildLeader",		GM_LEVEL9,		&CChatSystem::GmSetGguildLeader },	//@setGuildLeader 帮派名称 帮主名称	设置帮派帮主
	{"Circle",				GM_LEVEL9,		&CChatSystem::GmSetActorCircle}, //@circle [人名]转数
	{"CirclePoint",			GM_LEVEL9,		&CChatSystem::GmResetActorCirclePoint}, //@CirclePoint [人名] 重置潜力点
	{"ClearCsRank",			GM_LEVEL9,		&CChatSystem::GmClearCsRank }, //@ClearCsRank destServerId 删除跨服榜单 带destServerId参数指定删除某个服务器的榜单
	{"SaveCsRank",			GM_LEVEL9,		&CChatSystem::GmSaveCsRank }, //@SaveCsRank 广播所有逻辑服将雕像榜单数据发到session
	{"LoadCsRank",			GM_LEVEL9,		&CChatSystem::GmLoadCsRank }, //@LoadCsRank 广播所有逻辑服重新取下榜单

	{"ClearAllMsg",			GM_LEVEL9,		&CChatSystem::GmClearAllMsg }, //@清屏 
	{"ForbidUser",			GM_LEVEL9,		&CChatSystem::GmForbidUserById }, //@禁止玩家
	{"UnForbidUser",		GM_LEVEL9,		&CChatSystem::GmUnForbidUserById }, //@取消禁止玩家


	
	{"SetWulinMaster",		GM_LEVEL9,		&CChatSystem::GMSetWulinMaster },	//设置武林盟主 @SetWulinMaster 玩家名称
	{"addactivity",			GM_LEVEL9,		&CChatSystem::GmAddActivity },		//Gm增加活跃度
	{"createnpc",			GM_LEVEL9,		&CChatSystem::GmCreateNpc },		//Gm创建npc 场景名 npc名
	{"addnpc",				GM_LEVEL9,		&CChatSystem::GmCreateNpcById},					//GM添加npc @addnpc npcID
	{"ontracepacket",		GM_LEVEL9,		&CChatSystem::GmTracePacket },		//统计数据包 @ontracepacket 0 开启统计 1 关闭统计
	{"setopenservertime",	GM_LEVEL9,		&CChatSystem::GmSetOpenServerTime },	//gm设置开服时间 格式如:@setopenservertime 2012-10-03-01:00:00
	{"setmergeservertime",	GM_LEVEL9,		&CChatSystem::GmSetMergeServerTime },	//gm设置开服时间 格式如:@setmergeservertime 2012-10-03-01:00:00
	{"setChatLog",			GM_LEVEL9,		&CChatSystem::GmSetChatLog},		//@setChatLog 1 默认关闭(0),1表示开启, 设置聊天log是否存盘
	{"setquicktime",		GM_LEVEL9,		&CChatSystem::GmSetQuickTimes },		//设置次数
	{"setEquipDropRate",	GM_LEVEL9,		&CChatSystem::GmSetActorDropRate},		//@setEquipDropRate 暴率值
	{"setquickrate",		GM_LEVEL9,		&CChatSystem::GMSetQuickRate },		//设置加速的倍率
	{"setspid",				GM_LEVEL9,		&CChatSystem::GmSetSpGuidId},		//@setspid 1 设置spID
	{"setServerId",			GM_LEVEL9,		&CChatSystem::GmSetServerId},		//@setServerId 24 设置服务器id
	{"loadguilddata",		GM_LEVEL9,		&CChatSystem::GmLoadGuildDataFromDb},		//从数据服加载行会信息 @loadguilddata	
	{"setactorguild",		GM_LEVEL9,		&CChatSystem::GmSetActorGuild},				//设置玩家的行会 @setactorguild 行会名称 玩家名称 
	{"delactorguild",		GM_LEVEL9,		&CChatSystem::GmDeleteActorGuild},			//设置玩家的行会id为0 @delactorguil 玩家名称	
	{"setluck",				GM_LEVEL9,		&CChatSystem::GmSetEquipLuck},				//设置玩家武器幸运值 @setluck 玩家名 10
	{"SetSbkGuild",			GM_LEVEL9,		&CChatSystem::GMSetSbkGuild },	//设置sbk行会 @SetSbkGuild 行会名称 
	{"bmsbk",				GM_LEVEL9,		&CChatSystem::GmGuildSignUpSbk},			//设置某行会报名今天的沙巴克战 @bmsbk 行会名称 是否报名(0,取消报名 1 报名)
	{"setsbkpos",			GM_LEVEL9,		&CChatSystem::GmSetSbkPos},				//设置沙巴克职位 @setsbkpos  行会名称 玩家名称 职位id(2-5 护法) 1认命 0 取消
	{"createpos",			GM_LEVEL9,		&CChatSystem::GmSetActorCreatePos},			//设置出生点 @createpos 出生点个数
	{"addpos",				GM_LEVEL9,		&CChatSystem::GmAddActorCreatePos},				//gm添加出生点 @addpos 出生点x 出生点y
	{"setidlerole",			GM_LEVEL9,		&CChatSystem::GmSetIdlePlayer},					//设置空闲玩家 @setidlerole 1 1
	{"openquickkick",		GM_LEVEL9,		&CChatSystem::GmOpenQuickKick},				//设置检测使用外挂的人就踢下
	{"openquickseal",		GM_LEVEL9,		&CChatSystem::GmOpenQuickSeal},			//开启使用外挂被踢时封号
	{"setcreateindex",		GM_LEVEL9,		&CChatSystem::GmSetEnterId},					//设置创建角色进入的新手村的索引 @setcreateindex 入口id
	{"setlhzsgm",			GM_LEVEL9,		&CChatSystem::GmSetLhzsTitle},				//设置头衔  @setlhzsgm 玩家名称 头衔id 是否添加（0 取消 1 添加）
	{"SetTempCombineTime",	GM_LEVEL9,		&CChatSystem::GmSetServerTempCombineTime},		//设置服务器的合区时间(用于后台设置开启某些系统用) @SetTempCombineTime 倒数分钟
	{"killmonster",			GM_LEVEL9,		&CChatSystem::GmKillMonster},		//@killmonster 场景名字 怪名字 是否爆装备(1爆，0不爆)
	{"killallmonster",		GM_LEVEL9,		&CChatSystem::GmKillAllMonster},		//@killallmonster 
	{"rkf",					GM_LEVEL9,		&CChatSystem::GmSetCrossConfig},		//@reloadcross 加载跨服配置
	{"kfid",				GM_LEVEL9,		&CChatSystem::GmSetCommonServerId},		//@crossserverid 10000 设置公共服的ID
	{"setrank",				GM_LEVEL9,		&CChatSystem::GmSetRanking},				//设置排行 @setrank 排行名字 Id Value
	{"saveallrank",			GM_LEVEL9,		&CChatSystem::GmSaveAllRank},				//强制所有的存盘
	{"changename",			GM_LEVEL9,		&CChatSystem::GmChangeNameFlag},					//开启修改名字   @changename 状态（0 关闭 1 开启）
	{"opentrace",			GM_LEVEL9,		&CChatSystem::GmOpenTrace},					//   @opentrace 状态（0 关闭 1 开启）
	{"setsavelog",			GM_LEVEL9,		&CChatSystem::GmSetSaveQuestLogFlag},					//设置是否保存任务到日志服务器 @setsavelog 状态(0 关闭 1 开启)
	{"setcoinlog",			GM_LEVEL9,		&CChatSystem::GmSetSaveCoinLogFlag},					//设置是否保存任务到日志服务器 @setcoinlog 状态(0 关闭 1 开启)
	{"setzjlevel",			GM_LEVEL9,		&CChatSystem::GmSetBuildLevel},						//gm改变行会建筑的等级 @setzjlevel 行会名称 建筑编号(1 主殿 2 行会商店 3 月光宝盒 4 封印兽碑 9震天龙弹)  改变建筑等级（正 为加 负 为减)
	{"clearbuilddata",		GM_LEVEL9,		&CChatSystem::GmclearActorBuildData},				//gm清除玩家行会建筑操作数据 @clearbuilddata 玩家名字
	{"clearguildbuild",		GM_LEVEL9,		&CChatSystem::GmClearGuildBuildData},				//清除行会的建筑的数据 @clearguildbuild 行会名称
	{"clearguildtime",		GM_LEVEL9,		&CChatSystem::GmClearGuildBuildTime},				//清除行会的建筑的冷却时间 @clearguildtime 行会名称
	{"changegx",			GM_LEVEL9,		&CChatSystem::GmChangeActorGx},							//改变玩家的贡献 @changegx 玩家名字 改变的值
	{"testfilter",			GM_LEVEL9,		&CChatSystem::GmTestFilter},                            // 测试屏蔽字性能
	{"clearBossHard",		GM_LEVEL9,		&CChatSystem::GmClearBossHard	},					//清除boss难度
	{"setguildcoin",		GM_LEVEL9,		&CChatSystem::GmSetGuildCoin	},						//设置行会资金 @setguildcoin 资金
	{"addmail",				GM_LEVEL9,		&CChatSystem::GmAddMail},									//添加邮件
	{"addonemail",			GM_LEVEL9,		&CChatSystem::GmAddOneMail},								//添加邮件
	{"openckquick",			GM_LEVEL9,		&CChatSystem::GmSetOpenCkSpeedFalg},	//开启检测加速外挂 @openckquick 状态(0 关闭 1 开启)
	{"setwctime",			GM_LEVEL9,		&CChatSystem::GmSetDeviationTimes},		//设置检测加速外挂的误差时间（单位毫秒） @setwctime 误差时间
	{"setchecktime",		GM_LEVEL9,		&CChatSystem::GmSetCkIntevalTime},		//设置校验的间隔时间（单位秒）	@setchecktime  间隔时间
	{"setspeedcheck",		GM_LEVEL9,		&CChatSystem::GmSetSpeedCheck},			//设置检测加速相关值@setspeedcheck 加速检查值(整型,一般为58) 时间比率检测值(整型,一般100) 开始检测发包的次数（整型一般为4）, s_nNormalTime  
	{"setspeedvalue",		GM_LEVEL9,		&CChatSystem::GmSetSpeedValue},		//设置新检测加速外挂的参数 @setspeedvalue 参数类型(1:是否为肯定在加速百分比上限参数 2:是否为肯定在加速百分比下限参数 3:可能在加速百分比上限参数 4:误判百分比下限) 参数值(1到100的数值)
	{"dofile",				GM_LEVEL9,		&CChatSystem::GmLoadNpcFile},		//加载NPC文件
	{"doquest",				GM_LEVEL9,		&CChatSystem::GmLoadQuestFile},		//加载任务npc文件
	{"totalMonsterCount",	GM_LEVEL9,		&CChatSystem::GmTotalAllSceneMonsterCount}, //统计所有地图刷怪总量
	{"mapInfo",				GM_LEVEL9,		&CChatSystem::GmGetCurrSceneInfo}, //获取当前场景信息
	{"setTaxing",			GM_LEVEL9,		&CChatSystem::GmSetTaxing},			//设置税收信息
	{"setrundomkey",		GM_LEVEL9,		&CChatSystem::GmSetRundomKey},		//设置玩家的随机数 @setrundomkey 玩家名称 值
	{"delGuild",			GM_LEVEL9,		&CChatSystem::GmDeleteGuild},		//无条件从数据库中删除
	{"addNewTitle",			GM_LEVEL9,		&CChatSystem::GmAddNewTitle},		//添加新头衔 @addnewtitle 人物名 头衔id
	{"delnewtitle",			GM_LEVEL9,		&CChatSystem::GmDelNewTitle},		//删除新头衔 @delnewtitle 人物名 头衔id
	{"addCustomTitle",		GM_LEVEL9,		&CChatSystem::GmAddCustomTitle},		//添加自定义新头衔 @addcustomtitle 人物名 头衔id
	{"delCustomTitle",		GM_LEVEL9,		&CChatSystem::GmDelCustomTitle},		//删除自定义新头衔 @delcustomtitle 人物名 头衔id
	{"delvar",				GM_LEVEL9,		&CChatSystem::GmDelVar},		//删除静态变量
	{"resetWorldLevel",		GM_LEVEL9,		&CChatSystem::GmResetWorldLevel},//重置世界等级(变0级)
	{"callScript",			GM_LEVEL9,		&CChatSystem::GmSetActorCallScript},//执行脚本函数 @callscript 人物名字 脚本函数名
	{"reLoadWorldLevel",	GM_LEVEL9,		&CChatSystem::GmReLoadWorldLevel},//从数据库读世界等级
	{"setFootPrintPay",		GM_LEVEL9,		&CChatSystem::GmSetFootPrintPay},//设置足迹派送 @setFootPrintPay 开启时间(%d-%d-%d-%d:%d:%d)(开启时间为0表示清数据) 持续分钟数 方案id
	{"setAuction",			GM_LEVEL9,		&CChatSystem::GmSetAuction},		//设置竞拍 物品id/数量/金钱类型/底价/加价
	{"TestBackStage",		GM_LEVEL9,		&CChatSystem::GmTestBackStage},			//测试后台指令
	{"KickSetStatus",		GM_LEVEL9,		&CChatSystem::GmKickAndSetActorStatus},	// 踢人与设置人物状态 @KickSetStatus 禁用标志(非0则为禁用) 转数 等级
	{"OpenBFS",				GM_LEVEL9,		&CChatSystem::GmOpenBallFanStore},		// 开启球迷积分兑换  @OpenBFS 开启或关闭 开启时间 持续天数
	{"OpenBlackHouse",		GM_LEVEL9,		&CChatSystem::GmOpenBlackHouse},		//开启小黑屋 0关闭 1 开启
	{"QuitBlackHouse",		GM_LEVEL9,		&CChatSystem::GmQuitBlackHouse},			//退出小黑屋 参数：人名（不填表示自己)
	{"CleanBlackHouse",		GM_LEVEL9,		&CChatSystem::GmCleanBlackHouse},		//清理小黑屋数据 参数：人名（不填表示自己)
	{"SetSoulLevel",		GM_LEVEL9,		&CChatSystem::GMSetSoulLevel},		//设置武魂等级
	{"AddRankPoint",		GM_LEVEL9,		&CChatSystem::GMAddRankPoint},		//设置排行榜积分 @AddRankPoint 排行榜名字 增加多少积分
	{"SetAchieve",			GM_LEVEL9,		&CChatSystem::SetAchieve},			//设置成就状态
	{"FuBen",				GM_LEVEL9,		&CChatSystem::GmFuBen},				//副本测试GM指令
	{"RingLevel",			GM_LEVEL9,		&CChatSystem::GMSetRingLevel},		//设置特戒培养的等级 @RingLevel 人名 物品id 等级(-1变非激活，0激活了是0级）
	{"DebugLua",			GM_LEVEL9,		&CChatSystem::GmDebugLua},			//调试脚本用
	{"Shop",				GM_LEVEL9,		&CChatSystem::GmShop},				//商店GM指令
	{"QuestMove",			GM_LEVEL9,		&CChatSystem::QuestMove},			//测试速传, 参数@QuestMove 场景id，x,y
	{"PkValue",				GM_LEVEL9,		&CChatSystem::GmPkValue},			//增加PK值
	{"Exploit",				GM_LEVEL9,		&CChatSystem::GmExploit},			//增加功勋值
	{"EquipPosLevel",		GM_LEVEL9,		&CChatSystem::GmSetEquipPosStrongLevel},//设置部位强化的等级 @EquipPosLevel 玩家名 位置 等级(位置=-1即设全部位置)
	{"SetGuildTree",		GM_LEVEL9,		&CChatSystem::GmSetGuildTree},			//设置行会神树	@SetGuildTree   行会名称  神树等级 神树进度
	{"SetGuildFruit",		GM_LEVEL9,		&CChatSystem::GmSetGuildFruit},			//设置行会神树果实	@SetGuildFruit  行会名称  果实等级 果实进度 果实数量
	{"SetMyGuildTree",		GM_LEVEL9,		&CChatSystem::GmSetMyGuildTree},		//设置本人行会神树
	{"SetMyGuildFruit",		GM_LEVEL9,		&CChatSystem::GmSetMyGuildFruit},		//设置本人行会神树果实
	{"SetGuildTask",		GM_LEVEL9,		&CChatSystem::GmSetGuildTask},			//设置行会任务，@SetGuildTask   行会名称  任务ID，任务当前进度
	{"RefreshGuildTask",	GM_LEVEL9,		&CChatSystem::GmRefreshGuildTask},		//@RefreshGuildTask 行会名称  任务进度  --强行刷新行会任务和任务进度
	{"AddConsignItem",		GM_LEVEL9,		&CChatSystem::GmAddConsignItem},		//随机增加寄卖物品 @AddConsignItem [数量] *主要是测试用*
	{"ClearConsignItem",	GM_LEVEL9,		&CChatSystem::GmClearConsignItem},		//删除寄卖物品 @ClearConsignItem [1:清全部，0只清Add出来的]
	{"DoWorship",			GM_LEVEL9,		&CChatSystem::GmDoWorship},				//设置行会神树果实	@DoWorship flag param TO DELETE
	{"DoProtecteTuCity",	GM_LEVEL9,		&CChatSystem::GmDoProtecteTuCity},		//土城保卫战	@DoProtecteTuCity flag param
	{"DoRewardQuest",		GM_LEVEL9,		&CChatSystem::GmDoRewardQuest},			//赏金任务	@DoRewardQuest flag param
	{"DoCom",				GM_LEVEL9,		&CChatSystem::GmDoCompensate},			//游戏补偿	1|2|60|2016-3-2-12:00:00|300|补偿 TO DELETE
	{"DoSupplyBattle",		GM_LEVEL9,		&CChatSystem::GmDoSupplyBattle},		//补给争夺
	{"DoSevenGoodGift",		GM_LEVEL9,		&CChatSystem::GmDoSevenGoodGift},		//七日豪礼 TO DELETE
	{"DoChallengeFB",		GM_LEVEL9,		&CChatSystem::GmDoChallengeFB},			//挑战副本
	{"DoAllCharge",			GM_LEVEL9,		&CChatSystem::GmDoAllCharge},			//累积充值（不可重置）
	{"DoGuildSiege",		GM_LEVEL9,		&CChatSystem::GmDoGuildSiege},			//攻城战
	{"DoGameStore",			GM_LEVEL9,		&CChatSystem::GmDoGameStore},			//游戏商城
	{"GuildDepotItemReject",GM_LEVEL9,		&CChatSystem::GmGuildDepotItemReject},	//行会功能
	{"MaterialFB",			GM_LEVEL9,		&CChatSystem::GmDoMaterialFB},			//材料副本
	{"GuildAuth",			GM_LEVEL9,		&CChatSystem::GmSetGuildAuth},			//官方认证
	{"OpenSevTheme",		GM_LEVEL9,		&CChatSystem::GmDoOpenSevTheme},		//开服主题活动
	{"DailyCharge",			GM_LEVEL9,		&CChatSystem::GmDoDailyCharge},			//开服主题活动
	{"GiveAward",			GM_LEVEL9,		&CChatSystem::GmDoGiveAward},			//角色类型奖励 @GiveAward type id value
	{"GenAttrInSmith",		GM_LEVEL9,		&CChatSystem::GmGenAttrInSmith},		//从极品库中随机抽取N调属性
	{"ReloadVSPDef",		GM_LEVEL9,		&CChatSystem::GmReloadVSPDef},		
	{"setfcmstatus",		GM_LEVEL9,		&CChatSystem::GmSetFcmStatus},	         //设置防沉迷玩家状态 @setfcmstatus statusFlag(0未注册1成年2未成年)	 
};


bool CChatSystem::GmMonthCard(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,2) ==false) return false;
	int nOpera= atoi(args[0]);
	int nValue= atoi(args[1]);
	if (nOpera == 0)
	{
		((CActor*)m_pEntity)->GetMiscSystem().OnBuyMonthCard(nValue);
	}
	else if(nOpera == 1)
	{
		((CActor*)m_pEntity)->GetMiscSystem().OnGetMonthCardAward(nValue);
	}
	return true;
}

bool CChatSystem::GmDamageInfo(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	int nOpera= atoi(args[0]);
	if (nOpera == 0)
	{
		GetGlobalLogicEngine()->SetPrintDamageInfo(false);
	}
	else
	{
		GetGlobalLogicEngine()->SetPrintDamageInfo(true);
	}
	return true;
}

bool CChatSystem::GmAttr(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,2) ==false) return false;
	int nAttrId= atoi(args[0]);
	int nValue= atoi(args[1]);
	GAMEATTR attr;
	attr.type = nAttrId;
	switch(AttrDataTypes[nAttrId])
	{
	case adInt://有符号4字节类型
		attr.value.nValue = nValue;
		break;
	case adUInt://无符号4字节类型
		attr.value.uValue = nValue;
		break;
	case adFloat://单精度浮点类型值
		attr.value.fValue = nValue;
		break; 
	}
	
	#ifdef _DEBUG
	((CAnimal*)m_pEntity)->GetPropertySystem().m_Debug << attr;
	#endif
	((CAnimal*)m_pEntity)->GetPropertySystem().ResertProperty();
	return true;
}

//@Go 地图名(或场景id) X(default) Y(default) 
bool CChatSystem::GmGo(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	CFuBenManager *pFBMgr = GetGlobalLogicEngine()->GetFuBenMgr();
	CFuBen* pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);

	INT_PTR nSceneId = atoi(args[0]);
	INT_PTR nPosX = 0;
	INT_PTR nPosY = 0;

	CScene* pScene = NULL;
	if ( nSceneId <= 0) 
	{
		pScene = pFb->GetSceneByName(args[0]);
	} 
	else 
	{
		pScene = pFb->GetScene(nSceneId);
	}
	if (!pScene)
	{
		return false;
	}
	
	nSceneId = pScene->GetSceneId();

	if (nArgsCount >= 3)
	{
		if(IsArgsValid(args,nArgsCount,sRetMsg,3) ==false) return false;
		nPosX = atoi(args[1]); //x
		nPosY = atoi(args[2]);//y
	}
	else
	{
		if(pScene) {
			if(pScene->GetSceneAreaCenter(nPosX,nPosY,0) == false)
			{
				SCENECONFIG* sceneConf = pScene->GetSceneData();
				nPosX = sceneConf->nDefaultX;
				nPosY = sceneConf->nDefaultY;
			}
		}else return false;
	}
	int nEffId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nBlinkEnterEffect;
	return pFBMgr->EnterFuBen(m_pEntity, pFb, nSceneId, nPosX, nPosY,enDefaultTelePort,nEffId);
	/*
	if (nSceneId <= 0)
	{
		return m_pEntity->TelportSceneByName(args[0],nPosX,nPosY);
	}
	else
	{
		return m_pEntity->Teleport(pFb,nSceneId,nPosX,nPosY);
	}
	*/
}



//@Go 地图名(或场景id) X(default) Y(default) 
bool CChatSystem::GMEnterFuben(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	INT_PTR nFubenId = atoi(args[0]);
	return ((CActor*)m_pEntity)->GetFubenSystem().ReqEnterFuben(nFubenId);
}

//@Monster 怪物名(或id) 数量(1) 存活时间(0) 等级(0) 血量比(100)
bool CChatSystem::GmMonster( char **args, INT_PTR nArgsCount,char * sRetMsg )
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	CScene* pScene = m_pEntity->GetScene();
	if (!pScene)
	{
		return false;
	}
	INT_PTR nID = atoi(args[0]);
	INT_PTR nCount = 0;
	if ( !args[1] ) 
	{
		nCount = 1;
	}
	else
	{
		nCount = atoi( args[1] );
	}
	unsigned int nLive = 0;
	if ( args[2] )
	{
		nLive = atoi(args[2]);
	}

	int nBornLevel = 0;
	if ( args[3] )
	{
		nBornLevel = atoi(args[3]);
	}
	int nPercent = 100;
	if (args[4])
	{
		nPercent = atoi(args[4]);
	}


	PMONSTERCONFIG pConfig = NULL;
	if ( nID <= 0)
	{
		pConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterDataByName(args[0]);
	}
	else
	{
		pConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nID);
	}
	if (!pConfig)
	{
		return false;
	}

	int posX,posY;
	m_pEntity->GetPosition(posX,posY);

	//在人物周围找一个可以刷怪的地方
	INT_PTR j, nDir, nDX, nDY;
	nDir = m_pEntity->GetProperty<int>(PROP_ENTITY_DIR);
	for (j=0; j<8; ++j)
	{
		CSkillSubSystem::GetPosition(posX, posY, nDir, 1, nDX, nDY);
		if (pScene->CanMove(nDX, nDY))
		{
			goto lb_mob;
		}
		nDir = (nDir + 1) & 7; 
	}
	//在周围没有可刷怪的地方，则在人物当前位置刷新
	nDX = posX;
	nDY = posY;
lb_mob:
	for (int i = 0; i < nCount; i++)
	{
		CEntity* pEntity = pScene->CreateEntityAndEnterScene(pConfig->nEntityId, pConfig->btEntityType, nDX, nDY,-1,NULL,nLive, 
			nBornLevel,NULL, 0, nPercent);
		if (pEntity != NULL && pEntity->GetType() == enGatherMonster)
		{
			//pScene->SetMonsterConfig(pEntity,-1,pConfig->nEntityId);
			//((CMonster*)pEntity)->SetLiveTIme(nLive);
		}
	}
	return true;
}

//@PR 相对位置x 相对位置y
bool CChatSystem::GmPR(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,2) ==false) return false;
	INT_PTR nx = atoi(args[0]);
	INT_PTR ny = atoi(args[1]);

	INT_PTR nNewPosX = 0;
	INT_PTR nNewPosY = 0;
	INT_PTR nCerterPosX = m_pEntity->GetProperty<int>(PROP_ENTITY_POSX);
	INT_PTR nCerterPosY = m_pEntity->GetProperty<int>(PROP_ENTITY_POSY);
	INT_PTR nDir = m_pEntity->GetProperty<int>(PROP_ENTITY_DIR);
	CSkillSubSystem::PositionRotation(0,0,nx,ny,nDir,nNewPosX,nNewPosY);
	nNewPosX += nCerterPosX;
	nNewPosY += nCerterPosY;
	
	CActorPacket pack;
	CDataPacket &data =m_pEntity->AllocPacket(pack);
	data <<(BYTE)enDefaultEntitySystemID <<(BYTE)sPrintDebugText;
	char sString[128];
	sprintf_s(sString,sizeof(sString),"目标点为：%d,%d",nNewPosX,nNewPosY);
	data.writeString(sString);
	pack.flush();
}

//@PR 鼠标位置x 鼠标位置y 相对位置x 相对位置y
bool CChatSystem::GmAR(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,2) ==false) return false;
	INT_PTR nMousesPosX = atoi(args[0]);
	INT_PTR nMousesPosY = atoi(args[1]);
	INT_PTR nRelPosX = atoi(args[2]);
	INT_PTR nRelPosY = atoi(args[3]);

	INT_PTR nCerterPosX = m_pEntity->GetProperty<int>(PROP_ENTITY_POSX);
	INT_PTR nCerterPosY = m_pEntity->GetProperty<int>(PROP_ENTITY_POSY);
	INT_PTR nDir = m_pEntity->GetProperty<int>(PROP_ENTITY_DIR);
	INT_PTR nMPosRelX = nMousesPosX- nCerterPosX;
	INT_PTR nMPosRelY = nMousesPosY- nCerterPosY;
	INT_PTR nNewPosX = 0;
	INT_PTR nNewPosY = 0;
	CSkillSubSystem::AccurateRotation(nMPosRelX,nMPosRelY,nRelPosX,nRelPosY,nNewPosX,nNewPosY);
	nNewPosX += nCerterPosX;
	nNewPosY += nCerterPosY;
	
	CActorPacket pack;
	CDataPacket &data =m_pEntity->AllocPacket(pack);
	data <<(BYTE)enDefaultEntitySystemID <<(BYTE)sPrintDebugText;
	char sString[128];
	sprintf_s(sString,sizeof(sString),"目标点为：%d,%d",nNewPosX,nNewPosY);
	data.writeString(sString);
	pack.flush();
}

//@LearnSkill 技能ID 技能等级
bool CChatSystem::GmLearnSkill(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	int nSkillID= atoi(args[0]);
	if (args[1] == NULL)
	{
		int nLevel = nSkillID;
		m_pEntity->GetSkillSystem().AutoLearnVocSkill(nLevel);
	}
	else
	{
		int nSkillLevel = atoi(args[1]);
		return m_pEntity->GetSkillSystem().LearnSkill(nSkillID,nSkillLevel,false,true) == tpNoError;
	}
}


//@AddValue type num
bool CChatSystem::AddValue(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	int nCount = 100;
	int nId= atoi(args[0]);
	if (args[1] != NULL)
	{
		nCount = atoi(args[1]);
	}

	return m_pEntity->GiveAward(nId, 0, nCount,0,0,0,0,GameLog::Log_BackStage);
}
//@SetValue type num
bool CChatSystem::SetValue(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	int nCount = 100;
	int nId= atoi(args[0]);
	if (args[1] != NULL)
	{
		nCount = atoi(args[1]);
	}

	return m_pEntity->SetValueAward(nId, nCount);
}

//@DelSkill 技能ID
bool  CChatSystem::GmDeleteSkill(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;

	int nSkillId = atoi( args[0] );
	if (nSkillId <= 0)
	{
		nSkillId = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillIdByName(args[0]);
		if(nSkillId <= 0)
		{
			strcpy(sRetMsg,"invalid skill");
			return false;
		}
	}

	return ((CActor*)m_pEntity)->GetSkillSystem().ForgetSkill(nSkillId);
}

bool CChatSystem::Drop(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	int dropid = atoi( args[0] );
	std::vector<DROPGOODS> info;
	GetLogicServer()->GetDataProvider()->GetDropCfg().GetGiftDropInfoByDropGroupId(dropid, info);
	int nNum = info.size();
	for(int i = 0; i < nNum; i++)
	{
		DROPGOODS& award = info[i];
		((CActor*)m_pEntity)->GiveAward(award.info.nType, award.info.nId, award.info.nCount);
	}
}

bool CChatSystem::GmDropItem(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,2) ==false) return false;
	int nId = atoi( args[0] );
	int nCount = atoi( args[1] );

	CScene* pScene = m_pEntity->GetScene();

	int nLoopCount = 64;
	INT_PTR nPosX,nPosY;
	m_pEntity->GetPosition(nPosX,nPosY);
	int nNowtime = GetGlobalLogicEngine()->getMiniDateTime();
	for (size_t i = 0; i < nCount && nLoopCount-- > 0;)
	{
		CDropItemEntity *pDropItem = CDropItemEntity::CreateDropItem(pScene,nPosX,nPosY,GameLog::clLootDropBox,"gm",60);
		if (pDropItem)
		{
			CUserItem *pUserItem = GetGlobalLogicEngine()->AllocUserItem(true); //申请一个道具
			if(pUserItem ==NULL) return false;

			if(const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(nId))
			{
				pUserItem->wItemId =pStdItem->m_nIndex ;
				pUserItem->wCount = 1;
				pUserItem->btQuality = pStdItem->b_showQuality;
				pUserItem->wPackageType = pStdItem->m_nPackageType;
				pUserItem->setSource(CStdItem::iqOther,nNowtime);
				pDropItem->SetItem(pUserItem);
			}
			else
			{
				pDropItem->SetMoneyCount(10, nId == 65535? mtYuanbao :mtCoin);
			}
			pDropItem->SetCanPickUpTime(0);
			pDropItem->SetMaster(((CActor*)m_pEntity)); //这个是他的主人了
			
			if (--nCount <= 0) return true;
		}
	}
	return false;
}

bool CChatSystem::Info(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	// if (IsArgsValid(args,nArgsCount,sRetMsg,1) == false) return false;
	// CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	// CScene* pScene = m_pEntity->GetScene();

	// // 辅助容器
	// std::map< int,std::pair<std::string,int> > mapMonster;
	// typedef typename std::map< int,std::pair<std::string,int> >::iterator iterator;
	// typedef std::pair<std::string,int> map_value_type;

	// // 用以输出
	// CActorPacket pack;
	// CDataPacket &data =m_pEntity->AllocPacket(pack);
	// data << (BYTE)enChatSystemID << (BYTE)sSendChat << (BYTE)1;
	// char sString[128];

	// // 查询本地图所有怪物信息
	// if(_stricmp( args[0], "AllMonster") == 0)
	// {
	// 	data.writeString("AllMonster");

	// 	CEntityList& monsterList = pScene->GetMonsterList();
	// 	CLinkedNode<EntityHandle> *pNode;
	// 	CLinkedListIterator<EntityHandle> it(monsterList);
	// 	for (pNode = it.first(); pNode; pNode = it.next())
	// 	{
	// 		CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
	// 		if (pEntity)
	// 		{
	// 			unsigned int nId = pEntity->GetId();
	// 			if (mapMonster.find(nId) == mapMonster.end())
	// 			{
	// 				map_value_type& info = mapMonster[nId];
	// 				info.first = pEntity->GetEntityName();
	// 				info.second = 1;
	// 			}
	// 			else
	// 			{
	// 				map_value_type& info = mapMonster[nId];
	// 				info.second++;
	// 			}
	// 		}
	// 	}

	// 	std::string buff("\n");
	// 	iterator mapIter = mapMonster.begin();
	// 	for (; mapIter != mapMonster.end(); mapIter++)
	// 	{
	// 		int id = (*mapIter).first;
	// 		map_value_type& info = (*mapIter).second;
	// 		sprintf_s(sString,sizeof(sString),"[%s]id:%d,数量:%d; \n",info.first.c_str(),id,info.second);
	// 		buff += sString;
	// 	}
	// 	data.writeString(buff.c_str());
	// }
	// else if (_stricmp( args[0], "AllNpc") == 0)
	// {

	// }
	// else if (_stricmp( args[0], "AllActors") == 0)
	// {

	// }

	// //输出
	// data << m_pEntity->GetLevel();
	// data << Uint64(m_pEntity->GetHandle());
	// data << (BYTE)0;
	// pack.flush();
	return true;
}

//add buff
bool  CChatSystem::GmAddBuff(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	int nId = atoi( args[0] );
	m_pEntity->GetBuffSystem()->Append(nId);
	return true;
}

//del buff 
bool  CChatSystem::GmDelBuff(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	// if(IsArgsValid(args,nArgsCount,sRetMsg,2) ==false) return false;
	// int nType = atoi( args[0] );
	// int nGroup = atoi( args[1] );
	// m_pEntity->GetBuffSystem()->Remove((GAMEATTRTYPE) nType,nGroup);
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	int nId = atoi( args[0] );
	m_pEntity->GetBuffSystem()->RemoveById(nId);
	return true;
}

///抓捕怪物
///@CatchMonster 怪物名
bool CChatSystem::GmCatchMonster(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;

	CObserverSystem *pObserver = ((CAnimal *)m_pEntity)->GetObserverSystem();
	EntityHandle targetHdl = pObserver->FindEntityByName(args[0]);
	if (targetHdl.IsNull())
	{
		strcpy(sRetMsg, "no target");
		return false;
	}

	INT_PTR nEntityId = 0, nLevel = 0, nAiId = 0;
	int nPosX = 0, nPosY = 0;
	if(CEntity *pEntity = GetEntityFromHandle(targetHdl))
	{
		nEntityId = pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID);
		nLevel = pEntity->GetProperty<unsigned int>(PROP_CREATURE_LEVEL);
		nPosX = pEntity->GetProperty<int>(PROP_ENTITY_POSX);
		nPosY = pEntity->GetProperty<int>(PROP_ENTITY_POSY);

		PMONSTERCONFIG pConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nEntityId);
	 	if(pConfig == NULL) return false;
		nAiId = pConfig->wAiConfigId;
		
		CEntityManager* em = GetGlobalLogicEngine()->GetEntityMgr();
		em->DestroyEntity(pEntity->GetHandle());
	}

	CActor *pActor = ((CActor *)m_pEntity);
	return pActor->GetPetSystem().AddPet(nEntityId, nLevel, 4,0,0,0,nPosX,nPosY);

	//CMovementSystem * pMoveSystem = ((CAnimal*)(pEntity))->GetMoveSystem();
	//pMoveSystem->ClearMovement();
	//((CMonster *)pEntity)->SetOwner(m_pEntity->GetHandle());
	//pMoveSystem->MoveFollow(m_pEntity);
	
	//return true;
}

///抛弃怪物
///@ThrowMonster 怪物名
bool CChatSystem::GmThrowMonster(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	return true;
}


//@Move 地图名称
bool  CChatSystem::GmMove(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	CFuBen *pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
	if (pFb)
	{
		CScene* pScene = pFb->GetSceneByName(args[0]);
		if (pScene)
			sprintf(sRetMsg,"sceneid = %d",pScene->GetSceneId());
	}
	int nSenceID = atoi( args[0] );
	if (nSenceID > 0)
	{
		CFuBen * pRetFb =NULL;
		CScene * pRetScene =NULL;
		GetGlobalLogicEngine()->GetFuBenMgr()->GetFbPtrBySceneId(nSenceID,pRetFb,pRetScene);
		if ( pRetScene )
		{
			return m_pEntity->TelportSceneDefaultPoint(pRetScene->GetSceneName(), 0);
		}
		else
		{
			return false;
		}
	}
	return m_pEntity->TelportSceneDefaultPoint(args[0],0);
}

bool CChatSystem::GmMonster2( char **args, INT_PTR nArgsCount,char * sRetMsg )
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,2) ==false) return false;
	CScene* pScene = m_pEntity->GetScene();
	if (!pScene)
	{
		return false;
	}
	int nModelId = atoi(args[1]);
	
	PMONSTERCONFIG pConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterDataByName(args[0]);
	if (!pConfig)
	{
		return false;
	}

	int posX,posY;
	m_pEntity->GetPosition(posX,posY);

	//在人物周围找一个可以刷怪的地方
	INT_PTR j, nDir, nDX, nDY;
	nDir = m_pEntity->GetProperty<int>(PROP_ENTITY_DIR);
	for (j=0; j<8; ++j)
	{
		CSkillSubSystem::GetPosition(posX, posY, nDir, 1, nDX, nDY);
		if (pScene->CanMove(nDX, nDY))
		{
			goto lb_mob;
		}
		nDir = (nDir + 1) & 7; 
	}
	//在周围没有可刷怪的地方，则在人物当前位置刷新
	nDX = posX;
	nDY = posY;
lb_mob:

	CEntity* pEntity = pScene->CreateEntityAndEnterScene(pConfig->nEntityId, pConfig->btEntityType, nDX, nDY);
	if (pEntity != NULL)
	{
		
		//pScene->SetMonsterConfig(pEntity,-1,pConfig->nEntityId);
		//((CMonster*)pEntity)->SetLiveTIme(0);
		((CMonster*)pEntity)->SetProperty<int>(PROP_ENTITY_MODELID,nModelId); //设置模型id
		
	}
	
	return true;
}



bool CChatSystem::GmTestFilter(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	// if(IsArgsValid(args, nArgsCount, sRetMsg,1) == false) return false;
	// int nRunCount = atoi(args[0]);
	
	// if(nRunCount <0) return false;

	// CVector<char *>& data =  GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().data;
	// TICKCOUNT nCurrentTick = _getTickCount();
	
	// CChatManager &mgr= GetGlobalLogicEngine()->GetChatMgr();
	// INT_PTR nMsgCount = data.count();
	// int nTocalCount = nRunCount * (int)nMsgCount;

	// for(int i=0; i< nRunCount; i++)
	// {
	// 	for(int j=0;j < nMsgCount; j++)
	// 	{
	// 		mgr.IsStrInvalid(data[j]);
	// 	}
	// }
	// TICKCOUNT nEnd = _getTickCount();
	
	// TICKCOUNT nTick = nEnd - nCurrentTick;
	// _stprintf(sRetMsg, _T("msgcont: %d,time=%d"), nTocalCount, (int)nTick);
	return true;	
}

bool CChatSystem::GmGetDBSendPacketCount(char **args, INT_PTR nArgsCount, char *sRetMsg)
{	
	size_t count = GetLogicServer()->GetDbClient()->getPacketCount();
	_stprintf(sRetMsg, _T("wait sending packet count:%d"), count);
	return true;
}

bool CChatSystem::GmSetActorSaveInterval(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if(IsArgsValid(args, nArgsCount, sRetMsg,1) == false) return false;
	int nInterval = atoi(args[0]);
	nInterval = __max(nInterval, 20000);
	CActor::m_sSaveDBInterval = nInterval;
	_stprintf(sRetMsg, _T("current actor save interval is: %d ms"), nInterval);
	return true;
}

bool CChatSystem::GMKill(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	const char* szBeKillerName = args[0];
	CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName((char*)szBeKillerName);
	if (!pActor)
	{
		sprintf(sRetMsg, "beKiller invalid ");
		return false;
	}
	pActor->ChangeHP(-(pActor->GetProperty<int>(PROP_CREATURE_MAXHP)), (CActor*)m_pEntity);
	return true;
}

//设置聊天的等级
bool CChatSystem::GmSetChatLevel(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,2) ==false) return false;
	
	if( IsDigit(args[1]) == false  ) return false;
	int nLevel = atoi(args[1]); //设置的等级
	int nChannelId = -1;
	if(_stricmp( args[0], "near") ==0)
	{
		nChannelId= ciChannelNear;
	}
	// else if(_stricmp( args[0], "map")==0)
	// {
	// 	nChannelId= ciChannelMap;
	// }
	// else if(_stricmp( args[0], "camp") ==0)
	// {
	// 	nChannelId= ciChannelZhenying;
	// }
	// else if(_stricmp( args[0], "Horn") ==0)
	// {
	// 	nChannelId= ciChannelHorn;
	// }
	else if (_stricmp( args[0],"World") == 0) // 新加世界频道
	{
		nChannelId = ciChannelWorld;   
	}
	else if(_stricmp( args[0], "Secret") == 0)
	{
		nChannelId = ciChannelSecret;
	}
	// else if(_stricmp( args[0], "Friend") == 0)
	// {
	// 	GetLogicServer()->GetDataProvider()->GetGlobalConfig().nFriendChatLimit = nLevel;
	// 	sprintf(sRetMsg, "setchatlevel channel=%s ,level=%d succeed",args[0],nLevel);
	// 	return true;
	// }
	else if(_stricmp(args[0], "Guild") == 0)
	{
		nChannelId = ciChannelGuild;
	}
	else if(_stricmp(args[0], "Team") == 0)
	{
		nChannelId = ciChannelTeam;
	}
	if(nChannelId == -1)
	{
		sprintf(sRetMsg, "setchatlevel near|map|camp|Horn|World|Secret|Friend|Guild|Team level");
		return false;
	}
	//GLOBALCONFIG &gc = GetLogicServer()->GetDataProvider()->GetGlobalConfig();
	CHATSYSTEMCONFIG &gc = GetLogicServer()->GetDataProvider()->GetChatSystemConfig();
	gc.ChatLimit[nChannelId].wLevel = nLevel;
	sprintf(sRetMsg, "setchatlevel channel=%s ,level=%d succeed",args[0],nLevel);
	return true;
}

//设置聊天的等级
bool CChatSystem::GmSetChatRecharge(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,2) ==false) return false;
	
	if( IsDigit(args[1]) == false  ) return false;
	int nRecharge = atoi(args[1]); //设置的等级
	int nChannelId = -1;
	if(_stricmp( args[0], "near") ==0)
	{
		nChannelId= ciChannelNear;
	}
	// else if(_stricmp( args[0], "map")==0)
	// {
	// 	nChannelId= ciChannelMap;
	// }
	// else if(_stricmp( args[0], "camp") ==0)
	// {
	// 	nChannelId= ciChannelZhenying;
	// }
	// else if(_stricmp( args[0], "Horn") ==0)
	// {
	// 	nChannelId= ciChannelHorn;
	// }
	else if (_stricmp( args[0],"World") == 0) // 新加世界频道
	{
		nChannelId = ciChannelWorld;   
	}
	else if(_stricmp( args[0], "Secret") == 0)
	{
		nChannelId = ciChannelSecret;
	}
	// else if(_stricmp( args[0], "Friend") == 0)
	// {
	// 	GetLogicServer()->GetDataProvider()->GetGlobalConfig().nFriendChatLimit = nLevel;
	// 	sprintf(sRetMsg, "setchatlevel channel=%s ,level=%d succeed",args[0],nLevel);
	// 	return true;
	// }
	else if(_stricmp(args[0], "Guild") == 0)
	{
		nChannelId = ciChannelGuild;
	}
	else if(_stricmp(args[0], "Team") == 0)
	{
		nChannelId = ciChannelTeam;
	}
	if(nChannelId == -1)
	{
		sprintf(sRetMsg, "setchatlevel near|map|camp|Horn|World|Secret|Friend|Guild|Team level");
		return false;
	}
	//GLOBALCONFIG &gc = GetLogicServer()->GetDataProvider()->GetGlobalConfig();
	CHATSYSTEMCONFIG &gc = GetLogicServer()->GetDataProvider()->GetChatSystemConfig();
	gc.ChatLimit[nChannelId].nRechargeAmount = nRecharge;
	sprintf(sRetMsg, "setchatlevel channel=%s ,recharge=%d succeed",args[0],nRecharge);
	return true;
}

//设置聊天禁言的最大等级
bool CChatSystem::GmSetChatForbitLevel(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	
	if( IsDigit(args[0]) == false  ) return false;
	int nLevel = atoi(args[0]); //设置的等级
	
	CHATSYSTEMCONFIG &gc = GetLogicServer()->GetDataProvider()->GetChatSystemConfig();
	gc.nAutoSilenceLevel = nLevel;
	sprintf(sRetMsg, "GmSetChatForbitLevel ,level=%d succeed",nLevel);
	return true;
}
//打开赌博系统
bool CChatSystem::GmSetGambleOpen(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	
	SetGambleFlag(false);
	return true;
}

//关闭赌博系统
bool CChatSystem::GmSetGambleClose(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	
	SetGambleFlag(true);
	return true;
}

//通过名字重载数据
bool CChatSystem::GmReloadConfigByName(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	
	return GetLogicServer()->GetDataProvider()->ReloadConfig(args[0]);
}




//通过ID重载数据
bool CChatSystem::GmReloadConfigById(char **args, INT_PTR nArgsCount, char *sRetMsg)
{

	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	if( IsDigit(args[0]) == false  ) return false;
	int nPropId = atoi(args[0]);
	return   GetLogicServer()->GetDataProvider()->ReloadConfig(nPropId);
}

bool CChatSystem::GmRefreshRank(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (!GetLogicServer()->IsCrossServer())
	{ 
		//不是跨服
		GetGlobalLogicEngine()->GetRankingMgr().UpdateBaseRankData();
	}
	else
	{
		GetGlobalLogicEngine()->GetRankingMgr().UpdateBaseRankCSData(); 
	}
		
	GetGlobalLogicEngine()->GetRankingMgr().LoadBaseRankData();
	return true;
}

bool CChatSystem::GmRefreshHeadTitle(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	return false;
	//GetGlobalLogicEngine()->GetTopTitleMgr().Load();
	return true;
}

bool CChatSystem::GmSetHeadTitle(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	/*
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	CAchieveProvider &provider = GetLogicServer()->GetDataProvider()->GetAchieveConfig();
	CVector<TOPTITLECONFIG> &topTitleList = provider.GetTopTitleList();
	INT_PTR nCount = topTitleList.count();
	if (nCount <= 0)
	{
		sprintf(sRetMsg, "no HeadTitle can set");
		return false;
	}
	INT_PTR nVal = atoi(args[0]);
	if (nVal == -1)
	{
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_HEAD_TITLE, 0);
		sprintf(sRetMsg, "clear HeadTitle success");
		return true;
	}
	INT_PTR nTitleCount = 0;
	for (INT_PTR i = 0; i < nCount; i ++)
	{
		TOPTITLECONFIG &TopTitle = topTitleList[i];
		if (TopTitle.nTitleId == nVal) nTitleCount ++;
	}
	if (nTitleCount <= 0)
	{
		sprintf(sRetMsg, "no such HeadTitle id");
		return false;
	}
	return ((CActor*)m_pEntity)->SetHeadTitle(nVal);
	*/
	return false;
}

bool CChatSystem::GmAddPet(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 1) 
	{
		sprintf(sRetMsg, "need Param of pet");
		return false;
	}

	int max_op_param_count = 3;
	wylib::container::CBaseList<wylib::string::CAnsiString*> op_param_list;
	InitParamList(args, args_count, op_param_list, 3);
	
	int nId = __max(  atoi(op_param_list.get(0)->rawStr()),1);

	int nLevel= __max(  atoi(op_param_list.get(1)->rawStr()), 1) ; 
	int nAiId = atoi(op_param_list.get(2)->rawStr());

	if(! m_pEntity->GetPetSystem().AddPet(nId,  nLevel, nAiId, 1000) )
	{
		sprintf(sRetMsg, "add pet fail");
	}
	else
	{
		sprintf(sRetMsg, "add pet succeed");
	}
	

	SafeReleaseParamList(op_param_list);
	return true;
}


bool CChatSystem::GmAddHero(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 1) 
	{
		sprintf(sRetMsg, "need Param of pet");
		return false;
	}

	int max_op_param_count = 4;
	wylib::container::CBaseList<wylib::string::CAnsiString*> op_param_list;
	InitParamList(args, args_count, op_param_list, 4);

	int  nHeroId  = __max(  atoi(op_param_list.get(0)->rawStr()),1);
	int  nStage    = __max(  atoi(op_param_list.get(1)->rawStr()),1);
	int  nLevel    = __max(  atoi(op_param_list.get(2)->rawStr()), 1) ; 

	int nId = m_pEntity->GetHeroSystem().AddHero(nHeroId, nStage, nLevel);
	if(! nId )
	{
		sprintf(sRetMsg, "add hero fail");
	}
	else
	{
		m_pEntity->GetHeroSystem().SetHeroBattle(nId,true);
		sprintf(sRetMsg, "add hero succeed");
	}


	SafeReleaseParamList(op_param_list);
	return true;
}

bool CChatSystem::GmAddTargetHero(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 1) 
	{
		sprintf(sRetMsg, "need Param of pet");
		return false;
	}

	int max_op_param_count = 5;
	wylib::container::CBaseList<wylib::string::CAnsiString*> op_param_list;
	InitParamList(args, args_count, op_param_list, 5);

	CActor * pActor= GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]) ;

	if(pActor ==NULL)
	{
		strcpy(sRetMsg,"No such user online ");
		return false;
	}


	int nHeroId  = __max(  atoi(op_param_list.get(1)->rawStr()),1);
	int nStage    = __max(  atoi(op_param_list.get(2)->rawStr()), 0) ; 
	int nLevel    = __max(  atoi(op_param_list.get(3)->rawStr()), 0) ; 

	CHeroSystem &sys = pActor->GetHeroSystem();

	
	if (!sys.CanAddHero(true))
	{
		strcpy(sRetMsg,"target hero too more, can't add hero!");
		return false;
	}

	int nId = sys.AddHero(nHeroId, nStage, nLevel);
	if(! nId )
	{
		sprintf(sRetMsg, "add hero fail");
	}
	else
	{
		sys.SetHeroBattle(nId,true);
		sprintf(sRetMsg, "add hero succeed");
	}


	SafeReleaseParamList(op_param_list);
	return true;
}

bool CChatSystem::SetEquipItemProp(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 3) == false)  return false;
	INT_PTR nSlotId = atoi(args[0]);
	INT_PTR nPropId = atoi(args[1]);
	INT_PTR nVal	= atoi(args[2]);
	((CActor *)m_pEntity)->GetEquipmentSystem().SetEquipItemProp(nSlotId, nPropId, nVal);
	return true;
}

bool CChatSystem::SetExpGetToday(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false)  return false;
	int nVal = atoi(args[0]);
	((CActor *)m_pEntity)->SetExpGetTodayForDebug(nVal);
	return true;
}


bool CChatSystem::SetTraceOpt(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 2) == false)  return false;
	static CEntity::EntityTraceType flags[] = {
		CEntity::ettActorSkillDamage,
		CEntity::ettActorBeSkillDamage,
	};
	int nVal = atoi(args[0]);
	if (nVal < 0 || nVal >= ArrayCount(flags))
	{
		sprintf(sRetMsg, "trace type is invalid");
		return false;
	}
	bool bEnable = false;
	if (atoi(args[1]) != 0)
		bEnable = true;
	m_pEntity->SeEntityTraceFlag(flags[nVal], bEnable);
	return true;
}


//重刷语言包的时候，需要刷新tipmsg里的配置
bool CChatSystem::ReloadLang(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	bool result= GetLogicServer()->GetTextProvider().LoadFromFile(_T("data/language/LangCode.txt"));
	
	//result = GetLogicServer()->GetDataProvider()->LoadTipmsgConfig("");
	//if(result ==false) return false;
	//result = GetLogicServer()->GetDataProvider()->LoadMonsterShoutConfig("");
	return result;
}

bool  CChatSystem::GmLearnSkillByName(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	
	INT_PTR nSkillId = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillIdByName(args[0]);
	if(nSkillId <= 0)
	{
		strcpy(sRetMsg,"invalid skill");
		return false;
	}
	INT_PTR nLevel = ((CActor*)m_pEntity)->GetSkillSystem().GetSkillLevel(nSkillId);
	((CActor*)m_pEntity)->GetSkillSystem().StartRealLearnSkill(nSkillId,nLevel+1);
	return true;
}

bool  CChatSystem::GmUseSkill(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,2) ==false) return false;

	INT_PTR nSkillId = atoi(args[0]);
	INT_PTR nLevel = atoi(args[1]);
	if (nSkillId <= 0)
	{
		strcpy(sRetMsg,"invalid skill");
		return false;
	}
	if (nLevel <= 0)
	{
		strcpy(sRetMsg,"invalid level");
		return false;
	}

	EntityHandle handle = m_pEntity->GetTarget();
	CEntity *pEntity = GetEntityFromHandle(handle);
	if(pEntity ==NULL || pEntity->IsDeath())
	{
		pEntity = m_pEntity;
	}

	int x=0,y=0;
	pEntity->GetPosition(x,y);

	INT_PTR nErrorCode = m_pEntity->GetSkillSystem().LaunchSkill(nSkillId,x,y,true);

	if(nErrorCode)
	{
		((CActor *)m_pEntity)->SendOperateResult(false,0,0,false);
	}

	return true;
}

bool  CChatSystem::GmForgetSkillByName(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;

	INT_PTR nSkillId = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillIdByName(args[0]);
	if(nSkillId <= 0)
	{
		strcpy(sRetMsg,"invalid skill");
		return false;
	}
	INT_PTR nLevel = ((CActor*)m_pEntity)->GetSkillSystem().GetSkillLevel(nSkillId);
	((CActor*)m_pEntity)->GetSkillSystem().ForgetSkill(nSkillId);
	return true;
}



//重刷屏蔽字符库
bool  CChatSystem::GmRefreshfw(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	int nHashFun =4, nMemoryBytes= 2500000;
	if( nArgsCount >= 1)
	{
		nHashFun =  atoi(args[0]);
		
	}
	if( nArgsCount >= 2)
	{
		nMemoryBytes =  atoi(args[1]);

	}
	return GetGlobalLogicEngine()->GetChatMgr().ReloadFilterDb(nHashFun,nMemoryBytes);
}

bool CChatSystem::GmViewProperty(char * sRetMsg,int nPropID, CEntity * pEntity)
{
	int nPropValue = 0;
	if(pEntity->GetType() == enActor)
	{
		if(nPropID >= PROP_MAX_ACTOR )
		{
			strcpy(sRetMsg,"prop id too large");
			return false;
		}
	}
	else if(pEntity->GetType() == enMonster )
	{
		if(nPropID > PROP_MONSTER_BORNPOINT )
		{
			strcpy(sRetMsg,"prop id too large");
			return false;
		}

	}
	else 
	{
		strcpy(sRetMsg," entity can not view");
		return false;
	}
	
	sprintf(sRetMsg,"prop,id=%d,int value=%d, float value=%f",nPropID,
		pEntity->GetProperty<int> (nPropID),
		pEntity->GetProperty<float> (nPropID));
	return true;
}
bool CChatSystem::GmViewTargetProperty(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	CEntity * pEntity = GetEntityFromHandle(m_pEntity->GetTarget());
	if(pEntity ==NULL)
	{
		strcpy(sRetMsg,"no target");
		return false ;
	}

	int nPropID= atoi(args[0]);
	return GmViewProperty(sRetMsg,nPropID,pEntity);	
}

bool CChatSystem::GmSkillCDSwitch(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	if (args[0][0] == '1')
	{
		m_pEntity->GetSkillSystem().SetEnableCD(true);
		strcpy(sRetMsg,"<(c0xFF00FF00)Skill CD ON>");

	}
	else
	{
		m_pEntity->GetSkillSystem().SetEnableCD(false);
		strcpy(sRetMsg,"<(c0xFF00FF00)Skill CD OFF>");
	}
	return true;
}


//超人
bool CChatSystem::GmKick(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;


	CActor * pActor= GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]) ;
	if(pActor ==m_pEntity)
	{
		strcpy(sRetMsg,"can not kick self ");
		return false;
	}

	if(pActor ==NULL)
	{
		strcpy(sRetMsg,"No such user online ");
		return false;
	}
	pActor->CloseActor(lwiGmTickActor,false); //关闭玩家的连接
	
	return true;
}

//超人
bool CChatSystem::GmSetSuperMan(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	m_pEntity->m_isSuperman =!m_pEntity->m_isSuperman;
	return true;
}

bool CChatSystem::GmSetTargetIntPro(char **args, INT_PTR nArgsCount,char * sRetMsg)
	{
	if(IsArgsValid(args,nArgsCount,sRetMsg,3) ==false) return false;

	CActor * pActor= GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]) ;
	if(pActor ==m_pEntity)
	{
		strcpy(sRetMsg,"can not modify self ");
		return false;
	}
	if(pActor ==NULL)
	{
		strcpy(sRetMsg,"no such user online");
		return false;
	}
	if( IsDigit(args[1]) == false || IsDigit(args[2]) ==false ) return false;

	int nPropID= atoi(args[1]);
	int nPropValue =atoi(args[2]);
	
	if(nPropID >= PROP_MAX_ACTOR || nPropID<=0 )
	{
		strcpy(sRetMsg,"prop id too large");
		return false;
	}
	if(CanChangeProp(nPropID) ==false) return false;
	pActor->SetProperty<int>(nPropID,(int)nPropValue);
	return true;

}

bool CChatSystem::GmForbidUserById(char **args, INT_PTR nArgsCount, char *sRetMsg)
{ 
	int UserId = atoi(args[0]);
	int sTime = atoi(args[1]); 
	// std::string strId = strCom; 
	// int acterId = atoi(strId.c_str());
	if (GetGlobalLogicEngine()->GetMiscMgr().OnBackForbidUserMis(UserId, sTime * 60))
	{ 
	    return false;
    }  
 			 
	return true;
} 
 bool CChatSystem::GmUnForbidUserById(char **args, INT_PTR nArgsCount, char *sRetMsg)
{ 
	int UserId = atoi(args[0]);

	if (GetGlobalLogicEngine()->GetMiscMgr().OnBackUnForbidUserMis(UserId))
	{
	    return false;
	} 
	return true;
}
//测试用GM设置怪物的运动状态@setmonstermove monsterName 1/2/3/ 
bool CChatSystem::GmSetMonsterMove(char **args, INT_PTR nArgsCount,char * sRetMsg)
{	
	//if(IsArgsValid(args,nArgsCount,sRetMsg,2) ==false) return false;
	// 随便获取一个视野范围内匹配指定名称的怪做目标
	CObserverSystem *pOB = ((CAnimal *)m_pEntity)->GetObserverSystem();
	EntityHandle targetHdl = pOB->FindEntityByName(args[0]);
	if (targetHdl.IsNull())
	{
		strcpy(sRetMsg, "no target");
		return false;
	}
	CEntity *pEntity = GetEntityFromHandle(targetHdl);
	/*CEntity * pEntity = GetEntityFromHandle(m_pEntity->GetTarget());
	if(pEntity ==NULL)
	{
	strcpy(sRetMsg,"no target");
	return false ;
	}*/
	if(pEntity->IsMonster() ==false)
	{
		strcpy(sRetMsg,"not monster or npc");
		return false;
	}
	CMovementSystem * pMoveSystem = ((CAnimal*)(pEntity))->GetMoveSystem();
	int nPosX,nPosY;
	int nType =atoi(args[1]);
	switch(nType)
	{
	case 0:
		pMoveSystem->MoveIdle();
		strcpy(sRetMsg,"stand");
		break;

	case 1:
		pMoveSystem->MoveTargetedHome();
		strcpy(sRetMsg,"move home");
		break;
	case 2:
		pMoveSystem->ClearMovement();
		((CMonster *)pEntity)->SetOwner(m_pEntity->GetHandle());
		pMoveSystem->MoveFollow(m_pEntity);		
		strcpy(sRetMsg,"follow me");
		break;
	case 3:
		pMoveSystem->MoveChase(m_pEntity);
		strcpy(sRetMsg,"chase me");
		break;
	case 4:
		pMoveSystem->MoveConfused();
		strcpy(sRetMsg,"move confused");
		break;
	case 5: 
		pMoveSystem->MoveFleeing(m_pEntity);
		strcpy(sRetMsg,"move flee");
		break;
	case 6:
		nPosX = m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_POSX);
		nPosY = m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_POSY);
		pMoveSystem->MovePoint(nPosX,nPosY);
		break;
	case 7:		
		pMoveSystem->MovePatrol(10);
		strcpy(sRetMsg,"move patrol");
		break;
	case 9:	// random flee
		{
			CMonster *pTarget = (CMonster *)pEntity;
			if (pTarget)
			{
				int nMaxDist = args[2] ? atoi(args[2]) : 10;
				CMovementSystem *ms = pTarget->GetMoveSystem();
				if (ms)
					ms->MoveRandomFlee(nMaxDist, 3, 5);
			}
		}
	default:
		strcpy(sRetMsg,"param 1-7 is valid");

		break;
	}
	return true;
}
//@ReloadNPC NPC的名字  刷新挂在NPC身上的脚本
bool CChatSystem::GmRefresh(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,2) ==false) return false;
	//刷新系统NPC
	if (!_stricmp(args[0], "SYS"))
	{
		CNpc *pNpc;
		//刷新全局功能NPC
		if (!_stricmp(args[1], "FUNCTION"))
		{
			pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
			pNpc->LoadScript(CLogicEngine::szGlobalFuncScriptFile,true);
		}
		else if (!_stricmp(args[1], "MONSTER"))
		{
			pNpc = GetGlobalLogicEngine()->GetMonFuncNpc();
			pNpc->LoadScript(CLogicEngine::szMonsterFuncScriptFile,true);
		}
		else if(!_stricmp(args[1], "ITEM"))
		{
			pNpc = GetGlobalLogicEngine()->GetItemNpc();
			pNpc->LoadScript(CLogicEngine::szItemNpcFile, true);
		}
		else
		{
			strcpy(sRetMsg, "invalid sys npc name, try [FUNCITON,ANYWHERE]");
			return false;
		}
	}
	char* sceneName = NULL;
	char* npcName = NULL;
	if (strcmp(args[0],_T("all")) != 0) sceneName = args[0];
	if (strcmp(args[1],_T("all")) != 0) npcName = args[1];
	GetGlobalLogicEngine()->GetFuBenMgr()->ReloadNpcScript(sceneName,npcName);
	return true;
}


bool CChatSystem::GmReloadGlobalNpc( char **args, INT_PTR nArgsCount,char * sRetMsg )
{
	CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
	return pNpc->LoadScript(CLogicEngine::szGlobalFuncScriptFile,true);
}


bool CChatSystem::GmReloadScriptNpc( char **args, INT_PTR nArgsCount,char * sRetMsg )
{
	return GetGlobalLogicEngine()->GetGlobalNpc()->LoadScript(CLogicEngine::szQuestNpcFile, true);
}

//@call 新手老人 getNpc,1,2
bool CChatSystem::CallScriptFunc( char **args, INT_PTR nArgsCount, char *sRetMsg )
{
	// if(IsArgsValid(args,nArgsCount,sRetMsg,2) ==false) return false;
	// if (!m_pEntity) return false;
	// int nNpcId = atoi(args[0]);
	// if (nNpcId > 0)
	// {
	// 	CScene * pScene = m_pEntity->GetScene();
	// 	if (pScene != NULL)
	// 	{
	// 		CNpc * pNpc = pScene->GetNpc(nNpcId);
	// 		if (pNpc != NULL)
	// 		{
	// 			pNpc->CallScript(m_pEntity, args[1]);
	// 			return true;
	// 		}
	// 	}
	// 	return false;
	// }
	// else
	// {
	// 	GetGlobalLogicEngine()->GetGlobalNpc()->Talk(m_pEntity, args[1]);
	// }
	return true;
}

//@call 新手老人 getNpc,1,2
bool CChatSystem::CallScriptFuncNoEntity( char **args, INT_PTR nArgsCount, char *sRetMsg )
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,2) ==false) return false;
	if (!m_pEntity) return false;
	
	GetGlobalLogicEngine()->GetGlobalNpc()->Call(args[1]);
	return true;
}

//追踪系统的数据
bool CChatSystem::GmTraceSystem(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	if( IsDigit(args[0]) ==false) return false;
	int nFlag = atoi(args[0]);
	CActor::s_nTraceAccountId = nFlag;
	
	return true;

}
//只追踪特定的一个玩家
bool CChatSystem::GmTraceActor(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	CActor * pActor= GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]) ;
	if(pActor ==NULL) return false;
	if( CActor::s_nTraceAccountId )
	{
		 CActor::s_nTraceAccountId =0;
	}
	else
	{
		CActor::s_nTraceAccountId = pActor->GetAccountID();
	}
	return true;
	

}

bool CChatSystem::GmDumpFuben(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	GetGlobalLogicEngine()->GetFuBenMgr()->Dump();
	return true;
}

//查看物品设置
bool CChatSystem::GmCheckGameSet(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	int nPos = atoi(args[0]);
	GAMESETSDATA2& gameset = ((CActor*)m_pEntity)->GetGameSetsSystem().GetGameSet();
	
	int nIntOffs = nPos / 32;
	int nBitOffs = nPos % 32;
	int* pStart = gameset.m_nItems + nIntOffs;
	int result = (*pStart) & (1 << nBitOffs);
	if (result)
	{
		return true;
	}
	return false;
}

//调整目标的等级
bool CChatSystem::GmAdjustLevel(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,2) ==false) return false;

	CActor * pActor= GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]) ;
	
	if(pActor ==NULL)
	{
		strcpy(sRetMsg,"No such user online ");
		return false;
	}
	if( IsDigit(args[1]) ==false) return false;
	int nLevel = atoi(args[1]);
	if(nLevel <= 0 ) return false;
	//if(nLevel > MAX_ACTOR_LEVEL) return false;
	pActor->SetProperty<int>(PROP_CREATURE_LEVEL,nLevel); //关闭玩家的连接
	return true;
}

bool CChatSystem::RefreshMonster(char **args, INT_PTR nArgsCount, char* sRetMsg)
{
	GetGlobalLogicEngine()->GetFuBenMgr()->RefreshMonster();
	return true;
}

bool CChatSystem::FinishQuest(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,2) ==false) return false;
	int qid = atoi(args[0]);
	int flag = atoi(args[1]);
	// m_pEntity->GetQuestSystem()->SetFinish(qid,flag != 0);
	return true;
}
bool CChatSystem::Acceptrole(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	int qid = atoi(args[0]);
	int ret = 0;//m_pEntity->GetQuestSystem()->AddQuestToList(qid,NULL);
	return (ret == CQuestData::qecSucc) ;
}

//修改属性
bool CChatSystem::GmSetFloatProperty(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,2) ==false) return false;
	int nPropID = atoi(args[0]);
	
	if(CanChangeProp(nPropID) ==false) return false; //不能修改属性

	if(nPropID > PROP_ENTITY_ID && nPropID <PROP_MAX_ACTOR  )
	{
		m_pEntity->SetProperty<float>(nPropID,(float)(atof(args[1])));
		return true;
		
	}
	else
	{
		return false;
	}

}


bool CChatSystem::CanChangeProp(INT_PTR nPropId)
{
	char * spid=(char*) GetLogicServer()->GetVSPDefine().GetDefinition("SPID"); //获取
	if(spid ==NULL )return false;
	
	//内网的话能够修改属性
	if(_stricmp(spid,"wyi2") !=0  )
	{
		//外网不能修改元宝和银两的属性
		if(PROP_ACTOR_YUANBAO == nPropId || PROP_ACTOR_COIN == nPropId ||  PROP_ACTOR_DEPOT_COIN == nPropId)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	else	//内网元宝可以改
	{
		return true;
	}
	
	

}

bool CChatSystem::GmShowIntProperty(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	if (!IsArgsValid(args, nArgsCount, sRetMsg, 1))
	{
		return false;
	}
	int nPropId = atoi(args[0]);
	char buff[100];
	if (nPropId >= PROP_ENTITY_ID && nPropId < PROP_MAX_ACTOR)
	{		
		int nResult = m_pEntity->GetProperty<int>(nPropId);
		sprintf(buff, "propId[%d] value is:%d", nPropId, nResult);
		strcat(sRetMsg, buff);
		return true;
	}
	sprintf(buff, "propId[%d] can not get prop value", nPropId);
	strcat(sRetMsg, buff);
	return false;
}


//修改属性
bool CChatSystem::GmSetIntProperty(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,2) ==false) return false;
	int nPropID = atoi(args[0]);
	int nValue = atoi(args[1]);	
	if(CanChangeProp(nPropID) ==false) return false; //不能修改属性

	if(nPropID > PROP_ENTITY_ID && nPropID <PROP_MAX_ACTOR  )
	{
		m_pEntity->SetProperty<unsigned int>(nPropID,(unsigned int)atoi(args[1]) );
		return true;
	}
	else
	{
		return false;
	}
}
//@Property  打印玩家隐藏的属性
bool  CChatSystem::GmPrintProperty(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	char buff[256];
	sprintf(buff,"hpRenew:%4f",m_pEntity->GetProperty<float>(PROP_CREATURE_HP_RATE_RENEW) );
	strcat(sRetMsg,buff);

	sprintf(buff,"mpRenew:%4f",m_pEntity->GetProperty<float>(PROP_CREATURE_MP_RATE_RENEW) );
	strcat(sRetMsg,buff);
	
	sprintf(buff,"socialMask:%d",m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_SOCIALMASK) );
	strcat(sRetMsg,buff);

	sprintf(buff,"attackSpeed:%d",m_pEntity->GetProperty<unsigned int>(PROP_CREATURE_ATTACK_SPEED) );
	strcat(sRetMsg,buff);
	return true;
	
}

bool  CChatSystem::GmLeaveTeam(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	CTeam * pTeam = m_pEntity->GetTeam() ;
	if(pTeam==NULL) return false; //已经在队伍里了
	pTeam->DelMember(m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID),true,true);
	return true;
}
//@Invite  要求玩家组队
bool CChatSystem::GmInviteTeam(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	CActor * pMember= GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]);
	if(pMember ==NULL) return false;
	if(pMember->GetTeam()) return false; //已经在队伍里了
	if(pMember ==m_pEntity) return false; //不能要求自己
	unsigned int nTeamID;
	CTeam *pTeam = m_pEntity->GetTeam();
	
	if(NULL ==pTeam) 
	{
		pTeam= GetGlobalLogicEngine()->GetTeamMgr().CreateTeam(nTeamID);
		if(pTeam ==NULL) return false; //创建队伍失败
		pTeam->SetTeamID(nTeamID);
		pTeam->AddMember(m_pEntity);  //邀请自己的人是队长
	
		pTeam->AddMember(pMember);//自己是个队员而已
	}
	else
	{
		if(pTeam->GetMemberCount() >= MAX_TEAM_MEMBER_COUNT)
		{
			return false;
		}
		return pTeam->AddMember(pMember);//自己是个队员而已
	}
	return true;
}


//@Icon 头像ID 修改头像
bool CChatSystem::GmChangeIcon(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	int nIcon= atoi(args[0]); //
	if(nIcon <0) return false;
	m_pEntity->SetProperty<unsigned int>(PROP_ENTITY_ICON,(unsigned int)nIcon);
	return true;
}


//@AddMoney 金钱类型ID 数量
bool  CChatSystem::GmAddMoney(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,2) ==false) return false;
	int nMoneyType= atoi(args[0]); // 金钱类型
	INT_PTR nCount= (INT_PTR)atof(args[1]); //金钱数量

	
	if( !CanChangeProp(PROP_ACTOR_YUANBAO) ) //如果不能加钱的话，判断一下
	{
		if(mtCoin == nMoneyType || mtYuanbao == nMoneyType  ) return false; //外网不能GM改钱的
	}
	
	if(nMoneyType >= mtMoneyTypeStart && nMoneyType < mtMoneyTypeCount )
	{
		INT_PTR nWay = GameLog::Log_BackStage;
		// if ( nMoneyType == mtYuanbao )
		// {
		// 	nWay = GameLog::clWithdrawYuanBao;
		// }
		((CActor*)m_pEntity)->ChangeMoney(nMoneyType,nCount,nWay,0,_T("GM_ADD_MONEY"));
		return true;
	}

	return false;
}

bool  CChatSystem::GMAddCircleNum(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,2) ==false) return false;
	int nCircle= atoi(args[0]); // 金钱类型
	INT_PTR nCount= (INT_PTR)atof(args[1]); //金钱数量

	
	GetGlobalLogicEngine()->GetGlobalVarMgr().AddCountByCircleLevel(nCircle,nCount);

	return true;
}



bool  CChatSystem::GMDeathDropItem(char **args, INT_PTR nArgsCount,char * sRetMsg)
{


	if(!m_pEntity) return false;
	m_pEntity->OnDeathDropItem();

	return true;
}



bool CChatSystem::GMSetZBPMtime(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args, nArgsCount, sRetMsg, 1) ==false) 
		return false;
	int nPMIdx = atoi(args[0]);
	//GetGlobalLogicEngine()->GetNewHundredManager().DoTreasureAttic((CActor*)m_pEntity, nFlag);
	return true;
}

bool CChatSystem::GMSetZBPMdata(char **args, INT_PTR nArgsCount,char * sRetMsg)
{

	return true;
}


bool CChatSystem::GmSetAuction(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	return true;
}
//测试后台指令
bool CChatSystem::GmTestBackStage(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args, nArgsCount, sRetMsg, 1) ==false) 
		return false;

	return true;
}

//GM添加金币
bool  CChatSystem::GmAddGold(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,3) ==false && IsArgsValid(args,nArgsCount,sRetMsg,4) ==false) return false;

	int nMoneyType = atoi(args[1]);
	
	if( CanChangeProp(PROP_ACTOR_YUANBAO) ==false ) //如果不能加钱的话，判断一下
	{
		if(mtCoin == nMoneyType || mtYuanbao == nMoneyType  ) return false; //外网不能GM改钱的
	}

	int nCount= atoi(args[2]); //数量
	LPCSTR sMsg = "System Msg!";
	if (IsArgsValid(args,nArgsCount,sRetMsg,4))//有文本信息
	{
		sMsg = args[3];
	}
	return CActor::AddChangeMoneyMsgByName(
		args[0],m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID),sMsg,"",nMoneyType,nCount,GameLog::Log_BackStage,m_pEntity->GetEntityName(),true);//记下GM的名字
}
//设置自己的等级
bool  CChatSystem::GmSetSelfLevel(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;

	int nLevel= atoi(args[0]); 
	if( nLevel >=0)// && nLevel <= MAX_ACTOR_LEVEL )
	{
		m_pEntity->SetLevel(nLevel);
		//m_pEntity->GetFriendSystem().SetSocialNeedFresh(m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));
		GetGlobalLogicEngine()->GetWorldLevelMgr().CheckWorldLevel((CActor*)m_pEntity, true);
		return true;
	}
	else
	{
		return false;
	}
}
//设置别人的经验
bool  CChatSystem::GmAdjustExp(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,2) ==false) return false;
	char * name = args[0]; //玩家的名字
	unsigned int nCount= atoi(args[1]); //数量
	if( nCount >=0  )
	{
		CActor * pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(name);
		if(pActor ==NULL)
		{
			strcpy(sRetMsg,"User Not Online");
			return false;
		}
		pActor->SetProperty<Uint64>(PROP_ACTOR_EXP,nCount);
		return true;
	}
	else
	{
		return false;
	}
}
//添加自己的经验
bool  CChatSystem::GmAddSelfExp(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	INT64 nCount= (INT64)atof(args[0]); //数量
	if( nCount >=0  )
	{	
		m_pEntity->AddExp(nCount, GameLog::clGmAddExp);
		return true;
	}
	else
	{
		return false;
	}
}
//GM测试添加物品
bool CChatSystem::GmAddItem(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
    return GmMakeItem(args,nArgsCount,sRetMsg);
}


//GM删除物品
bool CChatSystem::GmDelItem(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
    INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count == 0)
	{
		sprintf(sRetMsg,"Need 3 param at least");
		return false;
	}
	if( _stricmp(args[0],"all") ==0)
	{
		CUserBag & bagSys = m_pEntity->GetBagSystem();
		bagSys.DeleteAllItem();

	}else
	{
		if (args_count < 2)
		{
			sprintf(sRetMsg,"Need 2 param at least");
			return false;
		}
		int itemId		= atoi(args[1]);
		//int itemNum	    = atoi(args[2]);
		
		CUserBag & bagSys = m_pEntity->GetBagSystem();
		CUserItem* itemInfo = bagSys.FindItem(itemId);
		if (!itemInfo)
		{
			return false;
		}
		bagSys.DeleteItem(itemInfo,itemInfo->wCount,"self_GM_del_item",GameLog::Log_BackStage,true);
	}
	return true;
}

bool CChatSystem::GmAddPresent( char **args, INT_PTR nArgsCount,char * sRetMsg )
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 2) 
	{
		sprintf(sRetMsg, "need one more Param");
		return false;
	}

	int max_op_param_count = 7;
	wylib::container::CBaseList<wylib::string::CAnsiString*> op_param_list;
	InitParamList(args, args_count, op_param_list, max_op_param_count);
	//bool result = AddItem(atoi(op_param_list.get(0)->rawStr()), atoi(op_param_list.get(1)->rawStr()), 
	//	atoi(op_param_list.get(2)->rawStr()), atoi(op_param_list.get(3)->rawStr()), sRetMsg);	
	//生成个物品
	char *sItemName = op_param_list.get(1)->rawStr(); 
	const CStdItem* pItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItemByName(sItemName);
	if (!pItem)
	{
		SafeReleaseParamList(op_param_list);
		sprintf(sRetMsg, "No such Item");
		return false ;
	}

	CUserItemContainer::ItemOPParam param;
	param.wItemId = (WORD)pItem->m_nIndex;
	BYTE bCount = (BYTE)(atoi(op_param_list.get(2)->rawStr()));
	param.wCount = bCount == 0?1:bCount;//默认送1个
	param.btQuality = (BYTE)(atoi(op_param_list.get(3)->rawStr()));//默认0
	param.btStrong = (BYTE)(atoi(op_param_list.get(4)->rawStr()));//默认0
	param.btBindFlag = (BYTE)(atoi(op_param_list.get(5)->rawStr()));//默认0

	bool result = CActor::AddItemMsgByName(op_param_list.get(0)->rawStr(),m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID),
		param,op_param_list.get(6)->rawStr(),"",GameLog::clGMAddItem,m_pEntity->GetEntityName(),true);

	SafeReleaseParamList(op_param_list);
	
	return true;
}


bool CChatSystem::GmMakeItem(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	//if(IsArgsValid(args,nArgsCount,sRetMsg,4) ==false) return false;
	
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 1) 
	{
		sprintf(sRetMsg, "need one more Param");
		return false;
	}
	
	int max_op_param_count = 11;
	wylib::container::CBaseList<wylib::string::CAnsiString*> op_param_list;
	InitParamList(args, args_count, op_param_list, max_op_param_count);	
	LPCTSTR sInPut = op_param_list.get(0)->rawStr();
	bool boChar = false;
	for (int i=0;i<strlen(sInPut);i++ )
	{
		if( sInPut[i] < 0 || sInPut[i] > 127 )
		{
			boChar = true;
			break;
		}
	}
	const CStdItem* pItem = NULL;
	if ( boChar )
	{
		pItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItemByName(sInPut);
	}
	else
	{
		int nInput = atoi(sInPut);
		pItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(nInput);
	}
	/*
	char* sStrong = op_param_list.get(4)->rawStr();
	int nValue = 0;
	for (int i =0;i<strlen(sStrong);i++)
	{
		char s = sStrong[i];
		if (s == '1')
		{
			nValue |= (0x1 << (i));
		}
	}
	*/
	if (!pItem)
	{
		SafeReleaseParamList(op_param_list);
		sprintf(sRetMsg, "No such Item");
		return false ;
	}
	int nID = pItem->m_nIndex;

	bool result = AddItem(nID, __max(1, atoi(op_param_list.get(1)->rawStr())), atoi(op_param_list.get(2)->rawStr()), atoi(op_param_list.get(3)->rawStr()), atoi(op_param_list.get(4)->rawStr()),atoi(op_param_list.get(5)->rawStr()),atoi(op_param_list.get(6)->rawStr()),0,sRetMsg);	

	SafeReleaseParamList(op_param_list);
	return result;

}

bool CChatSystem::AddItem(INT_PTR nItemID, INT_PTR nCount, INT_PTR nStar, INT_PTR nLostStar,INT_PTR nBind, INT_PTR bInSourceType, INT_PTR nAreaId,WORD wMonsterId, char * sRetMsg)
{
	const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(nItemID);
	if (pStdItem == NULL)
	{
		strcpy(sRetMsg,"No such Item");
		return false ;//参数错误
	}
	if (nCount <=0 )
	{
		strcpy(sRetMsg,"Param error");
		return false;
	}
	
	CUserItemContainer::ItemOPParam itemData;
	itemData.wItemId		= (WORD)nItemID;
	itemData.btQuality		= (BYTE)0;
	itemData.wCount			= (WORD)nCount;
	itemData.wStar			= (WORD)nStar;
	itemData.bLostStar		= (BYTE)nLostStar;
	itemData.bInSourceType	= (BYTE)bInSourceType;
	itemData.nDropMonsterId = (WORD)wMonsterId;
	itemData.nAreaId	= (int)nAreaId;
	itemData.btBindFlag		= (BYTE)nBind;
	itemData.nCreatetime = GetGlobalLogicEngine()->getMiniDateTime();
	INT_PTR nAddItemCount	= 0;
	memcpy(itemData.cSourceName, m_pEntity->GetEntityName(),sizeof(itemData.cSourceName));
	CUserBag & bagSys = m_pEntity->GetBagSystem();
	nAddItemCount = bagSys.AddItem(itemData,m_pEntity->GetEntityName(),GameLog::Log_BackStage);

	if( nAddItemCount >0)
	{
		sprintf(sRetMsg,"bag add %d items OK, \n itemid = %d",nAddItemCount,nItemID);
		//OutputMsg(rmTip,_T("GM add iTem id=%d,count=%d,Strong=%d,Quality=%d"),nItemID,nAddItemCount,nStrong,nQuality);
		return true;
	}
	else
	{
		strcpy(sRetMsg,"bag add items fail");
		return false;
	}
	return true;
}

//@Recall 角色名称 将指定角色传送到身边
bool CChatSystem::GmRecall(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	CActor * pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]);
	if(pActor == NULL)
	{
		strcpy(sRetMsg,"No such online actorName");	
		return false;
	}
	pActor->MoveToEntity(m_pEntity);
	return true;
}

//@ReGoto 角色名称 传送至指定角色身边
bool CChatSystem::GmReGoto(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) ==false) return false;
	CActor * pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]);
	if(pActor == NULL)
	{
		strcpy(sRetMsg,"No such online actorName");	
		return false;
	}
	return m_pEntity->MoveToEntity(pActor);
	
}

//@MoveToNPC 地图名称 NPC名称
bool CChatSystem::GmMoveToNPC(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 2) == false) return false;
	char *pMapName = args[0];
	char *pNpcName = args[1];
	SIZE_T nMapNameLen = strlen(pMapName);
	if (nMapNameLen <= 0 || strlen(pNpcName) <= 0)
	{
		sprintf(sRetMsg, "invalid scene name or npc name");
		return false;
	}
	
	// 遍历所有的普通副本
	FUBENVECTOR *pFuBenList = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbList();
	if (pFuBenList)
	{
		for (INT_PTR i = 0; i < pFuBenList->count(); i++)
		{
			CFuBen *pFB = (*pFuBenList)[i];
			if (pFB && pFB->GetFbId() == 0)	// 普通场景
			{
				CFuBen::SceneList &sceneLst = pFB->GetSceneList();
				for (INT_PTR scene_idx = 0; scene_idx < sceneLst.count(); scene_idx++)
				{
					CScene *pScene = sceneLst[scene_idx];
					if (pScene)
					{
						LPCTSTR pSceneName = pScene->GetSceneName();
						if (strlen(pSceneName) == nMapNameLen && !strcmp(pSceneName, pMapName))
						{
							// 找到匹配名称的场景，继续找Monster
							CNpc *pNpc = pScene->GetNpc(pNpcName);
							if (pNpc)
							{
								m_pEntity->MoveToEntity(pNpc);
								return true;
							}
						}
					}
				}
			}
		}
	}

	sprintf(sRetMsg, "can't find npc!");
	return false;
}

//@MapMove 源地图名称 目标地图名称
bool CChatSystem::GmMapMove(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	return false;
}

bool  CChatSystem::GmWho(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	INT_PTR nTotalCount = 0;
	//INT_PTR nCount = GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineActorCount(nTotalCount);

	INT_PTR nSimulatorPlayerCount = GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineSimulatorActorCount(nTotalCount);
	INT_PTR nNonGMPlayerCount = GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineActorCount(nTotalCount); 

	int nCount1 = GetGlobalLogicEngine()->GetEntityMgr()->GetOnLineActorCountNoSame();
	sprintf(sRetMsg, "Total Actor Count=%d, NonGM Count=%d,count = %d", (int)nTotalCount, (int)(nNonGMPlayerCount - nSimulatorPlayerCount), nCount1);	
	
	return true;
}

bool CChatSystem::GmClearBag(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	m_pEntity->GetBagSystem().Clear(m_pEntity->GetEntityName(), GameLog::clUserDestroyItem);
	return true;
}

bool CChatSystem::GmShutup(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 2) == false) return false;
	CActor * pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]);
	if (!pActor)
	{
		strcpy(sRetMsg, "No such online actorName");	
		return false;
	}
	INT_PTR nDuration = INT_PTR(atof(args[1]) * 60);
	pActor->GetChatSystem()->SetShutup(true, nDuration);

	return true;
}

bool CChatSystem::GmAddKeyword(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 2) == false) return false;
	char* pKeyword(args[0]);
	int mode(atoi(args[1]));
	return GetGlobalLogicEngine()->GetChatMgr().AddKeyword(pKeyword, mode);
}

bool CChatSystem::ReleaseShutup(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	CActor * pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]);
	if (!pActor)
	{
		strcpy(sRetMsg, "No such online actorName");	
		return false;
	}	
	pActor->GetChatSystem()->SetShutup(false, 0);

	return true;
}


bool CChatSystem::ShutupList(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	// 获取所有在线被禁言玩家列表
	CVector<EntityHandle> actorList;
	GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineAcotrHandleList(actorList);
	INT_PTR nCount = 0; 
	char szForbidMsg[100] = {0};
	for (INT_PTR i = 0; i < actorList.count(); i++)
	{
		CActor *pActor = (CActor *)GetEntityFromHandle(actorList.get(i));
		if (pActor && pActor->GetChatSystem()->IsShutup())
		{
			nCount++;
			sprintf(szForbidMsg, "Forbid Actor_%2d:%s", nCount, pActor->GetEntityName());
			m_pEntity->SendTipmsg(szForbidMsg, ttTipmsgWindow);			
		}
	}
	sprintf(sRetMsg,"Online shutup actor count=%d",nCount);	
	return true;
}


bool CChatSystem::GmShowFriendList(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	char szMsg[40] ={0};
	CVector<int> &friendList = m_pEntity->GetFriendList();
	for (int i = 0; i < friendList.count(); i++)
	{
		sprintf(szMsg, "friend_%d: %d", (i+1), friendList[i]);
		m_pEntity->SendTipmsg(szMsg);
	}

	return true;
}

bool CChatSystem::WorldMessage(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 2) == false) return false;
	char *pMsg = args[0];					// 公告内容
	int nMsgType = atoi(args[1]);			// 公告提示类型（弹对话框、屏幕中央等）
	///nMsgType值都修改了，改成按位操作
	//if (nMsgType >= ttMaxTmType || nMsgType <= ttInvalidTmType)
	//{
	//	sprintf(sRetMsg, "Invalid tipmsg type");
	//	return false;
	//}

	GetGlobalLogicEngine()->GetEntityMgr()->BroadcastTipmsg(pMsg, nMsgType);
	return true;
}

bool CChatSystem::NotifyMsg(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 2) == false) return false;
	char *pActorName = args[0];
	CActor *pTarget = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(pActorName);
	if (!pTarget)
	{
		// 发离线消息		
		CActor::AddGmTxtMsgByActorName(pActorName, args[1],m_pEntity->GetProperty<int>(PROP_ENTITY_ID));
		return false;
	}
	else
	{
		// 发TipMsg消息，增加一个GmTip类型即可
		pTarget->SendTipmsg(args[1], ttGmTip);
	}


	return true;
}

//处理GM命令
void  CChatSystem::ProcessGmCommand(char * msg)
{
	if(msg ==NULL )return;
	char sLogCmd[64];
	sprintf(sLogCmd,"%s",msg);
	char * args[10];			//命令列表
	char *sCmd =  msg + 1;     //命令的字符指针
	char *msgptr = msg + 1; //去掉@，指向下一个
	int  nArgCount = 0;   //参数的个数
	int  nBlankCount =0;  //空格的数量
	memset(args, 0, sizeof(args));
	while(*msgptr && nArgCount < ArrayCount(args))
	{
		//取第一个空格的位置
		char *ptr = strchr(msgptr, ' '); //
		//如果没有空格的话
		if ( !ptr ) 
		{
			//如果前面出现过空格，那么说明后面是一个参数
			if( nBlankCount >0 )
			{
				args[nArgCount] = msgptr;
				nArgCount ++;
			}
			break;
		}
		//命令的话就不要处理
		if( nBlankCount >0 )
		{
			args[nArgCount] = msgptr;
			nArgCount++;
		}
		nBlankCount ++; //空格的数量++
		*ptr = 0;
		ptr++;
		//跳过连续的空格
		while (*ptr && (unsigned char)*ptr <= 0x20 )
			ptr++;
		msgptr = ptr; 
	}
	if(sCmd ==NULL) return; 
	
	INT_PTR nCount =0;
	char retMsg[2048]; //返回参数
	retMsg[0]=0;
	bool flag= false; //是否有命令

	for(INT_PTR i=0;i<ArrayCount( CChatSystem::GmCommandHandlers); i++)
	{
		if( _stricmp(sCmd,CChatSystem::GmCommandHandlers[i].sCommand) ==0)
		{
			flag =true;

			// Check GM Right
			if (GmCommandHandlers[i].nReqMinGmLvl > m_pEntity->GetGmLevel())
			{
			strcat(retMsg, "Permission denied");
			}
			else
			{
				if( (this->*GmCommandHandlers[i].func)(args,nArgCount,retMsg) )//执行
				{
					LPCSTR sName = m_pEntity->GetEntityName();
					strcat(retMsg,":GM Succeed");
					OutputMsg(rmNormal, _T("%s:gm[%s] exec gm order[%s] succeed"), __FUNCTION__, sName, sCmd);
				}
				else
				{
					strcat(retMsg,":GM Fail");
				}
			}

			break;
		}
	}
	if(flag ==false)
	{
		strcat(retMsg,"No such command");
	}

	((CActor *)m_pEntity)->SendTipmsg(retMsg,ttTipmsgWindow);
}



bool CChatSystem::AddFubenCount( char **args, INT_PTR nArgsCount, char *sRetMsg )
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 3) == false) return false;
	CActor * pActor= GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]) ;
	if (!pActor)
	{
		return true;
	}
	//获取副本的groupid
// 	COMMONFUBENCONFIG* pConfig = NULL;//GetGlobalLogicEngine()->GetFuBenMgr()->GetConfigByName(args[1]);
// 	if (pConfig)
// 	{
// 		int nCount = atoi(args[2]);
// 		CCLVariant &var =pActor->GetActorVar();
// 		CCLVariant* pVar = var.get("fb");
// 		if (pVar)
// 		{
// 			char buf[10];
// #ifdef WIN32
// 			_itoa_s(pConfig->nGroupId,buf,sizeof(buf)-1,10);
// #else
// 			snprintf(buf, sizeof(buf)-1, "%d", pConfig->nGroupId );
// #endif
// 			CCLVariant* pVarCount = pVar->get(buf);
// 			if (pVarCount)
// 			{
// 				int nOld = (int)((double)(*pVarCount));
// 				*pVarCount = nOld + nCount;
// 			}
// 		}
// 	}

	return true;
}
bool CChatSystem::ReloadMonster(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	GetGlobalLogicEngine()->GetBackResLoadModule().LoadMonsterConfig();
	return true;
}
bool CChatSystem::ReloadItem(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	GetGlobalLogicEngine()->reloadItemConfig();
	return true;
}

bool CChatSystem::TracePacket(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	const char* szActorName = args[0];
	CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(szActorName);
	if (pActor)
	{
		pActor->SetTracePacket(true);
		return true;
	}

	return false;
}


bool CChatSystem::ViewTargetProp(char **args, INT_PTR nArgsCount, char* sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 2) == false) return false;
	const char* szActorName = args[0];
	CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(szActorName);
	if (!pActor) return false;

	int nPropId = atoi(args[1]);
	int nVal = pActor->GetProperty<int>(nPropId);
	_stprintf(sRetMsg, _T("prop id=%d value=%d"), nPropId, nVal);
	return true;
}

//设置防沉迷开启
bool CChatSystem::GmSetFcmOpen(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	GetLogicServer()->GetDataProvider()->GetGlobalConfig().bIsFcmOpen = true;
	return true;
}

//设置防沉迷关闭
bool CChatSystem::GmSetFcmClose(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	GetLogicServer()->GetDataProvider()->GetGlobalConfig().bIsFcmOpen = false;
	return true;
}

bool CChatSystem::GuildAddYs(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (!m_pEntity) return false;
	int nValue	= atoi(args[0]);
	CActor* pActor = (CActor *)m_pEntity;

	if(pActor->GetGuildSystem()->IsFree()) return false;
	pActor->GetGuildSystem()->GetGuildPtr()->SetGuildYs(nValue);
		
	pActor->GetGuildSystem()->GetGuildPtr()->m_boUpdateTime = true;
	return true;
}

bool CChatSystem::GmSetGguildLeader(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 2) == false) return false;
	char* sGuildName = args[0];
	char* sAcotrName = args[1];

	CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(sAcotrName);
	if (!pActor) return false;

	GetGlobalLogicEngine()->GetGuildMgr().SetGuildLeaderByname(sGuildName,sAcotrName);

	return true;
}
bool CChatSystem::GmSetGuildAuth(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 2) == false) return false;
	char* sGuildName = args[0];
	int nAuthFlag = atoi(args[1]);

	CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuildByName(sGuildName);

	if (!pGuild) return false;

	if (nAuthFlag == 0 || nAuthFlag ==1)
	{
		pGuild->SetCertification(nAuthFlag);
	}
	return true;
}

bool CChatSystem::ReqTransmit(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	CLogicServer *lpLogicServer = GetLogicServer();
	int nId = lpLogicServer->GetCommonServerId();
	if(((CActor *)m_pEntity)->CanTransMit(nId))
	{
		((CActor *)m_pEntity)->RequestTransmitTo(nId);
		return true;
	}
	else
	{
		return false;
	}	
}

bool CChatSystem::TraceServerInfo(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	int nCurrServerId = GetLogicServer()->GetServerIndex();
	CActor *pActor = (CActor *)m_pEntity;
	int nRawServerId = pActor->GetRawServerIndex();
	sprintf(sRetMsg, _T("login_server_id=%d, raw_server_id=%d"), nCurrServerId, nRawServerId);
	return true;
}

bool CChatSystem::SetActorMsgFilter(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (nArgsCount < 1)
	{
		strcpy(sRetMsg, "invalid param");
		return false;
	}

	int nSystemId = atoi(args[0]);
	int nMsgId = -1, nFlag = 1;	
	if (nArgsCount > 2)
	{
		nFlag = atoi(args[2]);
	}
	
	if (nArgsCount > 1)
	{
		nMsgId = atoi(args[1]);
	}

	GetGlobalLogicEngine()->GetActorMsgFilter().SetFilter(nSystemId, nMsgId, nFlag != 0 ? true : false);
	return true;
}

bool CChatSystem::TestSessionBroad(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	char szData[1024] = {0};
	CDataPacket packet(szData, sizeof(szData));
	packet << (WORD)jxInterSrvComm::SessionServerProto::fcDefault;
	const char *data = "abcdefg";
	size_t nLen = strlen(data);
	packet.writeString(data, nLen);
	CLogicSSClient *pSSClient = GetLogicServer()->GetSessionClient();
	pSSClient->BroadMsgToCommonLogicClient(packet.getMemoryPtr(), packet.getLength());
	return true;
}

bool CChatSystem::GmStartCommonServer(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	bool bStart = true;
	bStart = atoi(args[0]) != 0 ? true : false;
	GetLogicServer()->SetStartCommonServer(bStart);

	if (GetLogicServer()->IsStartCommonServer())
		strcpy(sRetMsg, "CommonServer Start!");
	else
		strcpy(sRetMsg, "CommonServer Stop!");

	return true;
}
bool CChatSystem::GmResetActorCirclePoint(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	int nCircle= 0;
	CActor* pActor = (CActor*)m_pEntity;
	if (nArgsCount ==1)
	{
		pActor =  GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]);
		if (!pActor)
		{
			sprintf(sRetMsg, "player %s offline", args[0]);
			return false;
		}
	}
	else if (nArgsCount != 0)
	{
		return false;
	}
	GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction(pActor,"OnCircleReset",1);
	return true;
}
bool CChatSystem::GmSetActorCircle(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	int nCircle= 0;
	CActor* pActor = (CActor*)m_pEntity;
	if (nArgsCount ==2)
	{
		pActor =  GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]);
		if (!pActor)
		{
			sprintf(sRetMsg, "player %s offline", args[0]);
			return false;
		}
		nCircle = atoi(args[1]);
	}
	else if(nArgsCount == 1)
	{
		nCircle = atoi(args[0]);
	}
	else
	{
		return false;
	}
	if(nCircle >= 0)
	{
		int oldCircle = pActor->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE);
		pActor->SetActorCircle(nCircle);
		if (nCircle >= oldCircle)
		{
			//GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction(pActor,"CircleSetPotentialPoint",nCircle);
		}
		else
		{
			//转数变低了，重置下潜力点
			//GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction(pActor,"OnCircleReset",1);
		}
		return true;
	}
	else
	{
		return false;
	}
}


bool CChatSystem::GmClearCsRank(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	INT_PTR nServerId = 0;
	if (nArgsCount >= 1)
	{
		nServerId = atoi(args[0]);
	}
	GetGlobalLogicEngine()->GetTopTitleMgr().ClearCsRank(nServerId);
	return true;
}

bool CChatSystem::GmSaveCsRank(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	GetGlobalLogicEngine()->GetTopTitleMgr().BroadLogicToSaveCsRank();
	return true;
}

bool CChatSystem::GmLoadCsRank(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	GetGlobalLogicEngine()->GetTopTitleMgr().BroadLogicToLoadCsRank();
	return true;
}
bool CChatSystem::GmClearAllMsg(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	GetGlobalLogicEngine()->GetMiscMgr().OnBackClearAllMsg();
	return true;
} 
bool CChatSystem::GMSetSbkGuild(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	char* sGuildName = args[0];

	CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuildByName(sGuildName);
	
	if(pGuild)
	{
		GetGlobalLogicEngine()->GetGuildMgr().SetCityOwnGuildId(pGuild->m_nGid);
	}
	else
	{
		return false;
	}
	return true;
}

bool CChatSystem::GMSetWulinMaster(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	char* sName = args[0];

	CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(sName);
	if (!pActor) return false;

	CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
	if (pNpc == NULL) return false;

	CScriptValueList paramList, retList;

	unsigned int nActorId=pActor->GetProperty<unsigned int>(PROP_ENTITY_ID);

	paramList << (unsigned int)nActorId;
	if (!pNpc->GetScript().Call("OnGmSetWulinMaster", paramList, retList))
	{
		OutputMsg(rmError, _T("call script fun: OnGmSetWulinMaster failed"));
	}

	return true;

}

bool CChatSystem::GmAddActivity(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	int nValue = atoi(args[0]);

	((CActor *)m_pEntity)->GetAcitivityAward(nValue);

	return true;
}


bool CChatSystem::GmCreateNpcById(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;	
	CScene* pScene = m_pEntity->GetScene();
	if (pScene)
	{
		int nX = 0, nY = 0;
		m_pEntity->GetPosition(nX, nY);
		int nNpcId = atoi(args[0]);
		return pScene->GmCreateNpcById(nNpcId, nX, nY);
	}
	return false;
}

bool CChatSystem::GmCreateNpc(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 2) == false) return false;

	CFuBen *pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
	if (pFb)
	{
		CScene* pScene = pFb->GetSceneByName(args[0]);
		if (pScene)
		{
			CNpc* npcEntity = pScene->GetNpc(args[1]);
			if(npcEntity)
			{
				int posX,posY;
				npcEntity->GetPosition(posX,posY);
				if( ((CNpc*)npcEntity)->GetAttriFlag().DenySee == true)
				{
					((CNpc*)npcEntity)->GetAttriFlag().DenySee = false;
					sprintf(sRetMsg, "scene[%s] Npc[%s] is DenySee,x=%d,y=%d", pScene->GetSceneName(), npcEntity->GetEntityName(),posX,posY );
					OutputMsg(rmNormal, sRetMsg);
					return true;
				}
				else
				{
					sprintf(sRetMsg, "scene[%s] Npc[%s] is exist,x=%d,y=%d", pScene->GetSceneName(),npcEntity->GetEntityName(),posX,posY );		
					OutputMsg(rmNormal, sRetMsg);
				}
			}
			return pScene->GmCreateNpcByName(args[1]);
		}
	}

	return false;
}

bool CChatSystem::GmTracePacket(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	int nValue = atoi(args[0]);
	if(nValue == 0)
	{
		GetGlobalLogicEngine()->SetPacketRecordState(true);
	}
	else
	{
		GetGlobalLogicEngine()->SetPacketRecordState(false);
	}
	return true;
}

bool CChatSystem::GmSetOpenServerTime(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;

	CMiniDateTime nStart;
	
	//nStart.encode(args[0]);//服务器的开启时间
	CBackStageSender::StdTimeStr2MiniTime(args[0], nStart);
	if(GetLogicServer())
	{
		GetLogicServer()->SetServerOpenTime(nStart);
	}
	else
	{
		return false;
	}

	return true;
}

bool CChatSystem::GmSetMergeServerTime(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;

	CMiniDateTime nStart;

	//nStart.encode(args[0]);
	CBackStageSender::StdTimeStr2MiniTime(args[0], nStart);
	if(GetLogicServer())
	{
		GetLogicServer()->SetServerCombineTime(nStart);
	}
	else
	{
		return false;
	}

	return true;
}

bool CChatSystem::GmSetQuickTimes(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	return true;
}

bool CChatSystem::GmSetChatLog( char **args, INT_PTR nArgsCount, char * sRetMsg )
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	int nFlag = atoi(args[0]);
	if (nFlag < 0)
	{
		return false;
	}
	
	//GetLogicServer()->GetDataProvider()->GetGlobalConfig().nChatLogFlag = nFlag;
	GetLogicServer()->GetDataProvider()->GetChatSystemConfig().nChatLogFlag = nFlag;
	return true;
}


bool CChatSystem::GmSetActorDropRate( char ** args, INT_PTR nArgsCount, char * sRetMsg )
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	GetLogicServer()->GetDataProvider()->GetGlobalConfig().fWriteNameEquipDropRate = (float)atof(args[0]);
	return true;
}


//
bool CChatSystem::GMSetQuickRate( char **args, INT_PTR nArgsCount, char * sRetMsg )
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	
	float fRate =0.1f;
	if (nArgsCount >= 1)
	{
		fRate = (float)atof(args[0]);
	}
	if(fRate <0.0 || fRate >0.5) 
	{
		sprintf(sRetMsg, " invalide param" );
		return false;
	}
	CActor::s_fKickQuickRate = fRate;
	sprintf(sRetMsg, "quick rate=%f",fRate );
	return true;
}

bool CChatSystem::GmSetSpGuidId(char **args, INT_PTR nArgsCount, char * sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false)
	{
		return false;
	}
	int nId = atoi(args[0]);
	GetLogicServer()->SetSpid(nId);
	//GetLogicServer()->GetMgrClient()->SetSpId(nId);
	GetGlobalLogicEngine()->SetUserItemSpId(nId);
	return true;
}

bool CChatSystem::GmSetServerId(char **args, INT_PTR nArgsCount, char * sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false)
	{
		return false;
	}
	int nServerId = atoi(args[0]);
	GetLogicServer()->SetServerIndex(nServerId);
	GetLogicServer()->GetLogClient()->SetServerIndex(nServerId);
	GetLogicServer()->GetLocalClient()->SetServerIndex(nServerId);
	GetGlobalLogicEngine()->SetUserItemServerId(nServerId);

	return true;
}

bool CChatSystem::GmLoadGuildDataFromDb(char **args, INT_PTR nArgsCount, char * sRetMsg)
{
	GetGlobalLogicEngine()->GetGuildMgr().Load();
	return true;
}

bool CChatSystem::GmSetActorGuild(char **args, INT_PTR nArgsCount, char * sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 2) == false) return false;
	char* sGuildName = args[0];

	CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuildByName(sGuildName);
	CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[1]);
	if(pGuild && pActor)
	{
		if(pActor->GetGuildSystem() && pActor->GetGuildSystem()->GetGuildPtr())
		{
			m_pEntity->SendTipmsg("actor in guild");
			return false;
		}
		if (pGuild->GetMemberCount() >= pGuild->GetMaxMemberCount())
		{
			//达到了人数上限
			m_pEntity->SendOldTipmsgFormatWithId(tpMaxMemberCount,ttTipmsgWindow,pGuild->m_sGuildname);
			return false;
		}
		//加入gid帮派
		pGuild->AddMemberToList((CActor*)pActor);
	}
	else
	{
		return false;
	}
	return true;
}

bool CChatSystem::GmDeleteActorGuild(char **args, INT_PTR nArgsCount, char * sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]);
	if(pActor)
	{
		if(pActor->GetGuildSystem() && pActor->GetGuildSystem()->GetGuildPtr())
		{
			pActor->GetGuildSystem()->GetGuildPtr()->DeleteMember(pActor->GetProperty<unsigned int>(PROP_ENTITY_ID),(CActor*)pActor,NULL,FALSE);
			pActor->GetGuildSystem()->SetGuild(NULL);
		}
		pActor->SetProperty<unsigned int>(PROP_ACTOR_GUILD_ID,0);
		pActor->SetProperty<unsigned int>(PROP_ACTOR_GUILD_LEVEL,0); 
	}
	else
	{
		return false;
	}

	return true;
}

bool CChatSystem::GmSetEquipLuck(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 2) == false) return false;
	CActor * pPlayer = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]);
	if (pPlayer == NULL)
	{
		sprintf(sRetMsg, "player %s offline", args[0]);
		return false;
	}
	CUserItem * pEquipItem = pPlayer->GetEquipmentSystem().GetItemByType(Item::itWeapon);
	if (pEquipItem == NULL)
	{
		sprintf(sRetMsg, "%s not weapon in body", args[0]);
		return false;
	}
	char nLuck = atoi(args[1]);
	if (nLuck < 0 || nLuck > 20)
	{
		sprintf(sRetMsg, "the valid value is [0,20]");
		return false;
	}
	return pPlayer->GetBagSystem().SetItemProperty(pEquipItem, Item::ipItemLuck, nLuck);
}

bool CChatSystem::GmGuildSignUpSbk(char **args, INT_PTR nArgsCount, char * sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 2) == false) return false;
	CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuildByName(args[0]);
	if(pGuild)
	{
		int nId = atoi(args[1]);
		pGuild->m_SignUpFlag = nId;
	}
	return true;
}


bool CChatSystem::GmSetActorHide(char **args, INT_PTR nArgsCount, char * sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 2) == false) return false;

	CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]);

	if(pActor)
	{
		int nType = atoi(args[1]);
		if(nType == 0)
		{
			//pActor->GetAttriFlag().DenySee = false;
			pActor->RemoveState(esStateHide);
		}
		else
		{
			pActor->AddState(esStateHide);
			//pActor->GetAttriFlag().DenySee = true;
		}
	}

	return true;
}

bool CChatSystem::GmSetSbkPos(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 4) == false) return false;
	int nType = atoi(args[2]);
	int nState = atoi(args[3]);
	m_pEntity->GetGuildSystem()->GMSetGuildCityPostion(args[0],nType, nState,args[1]);
	
	return false;
}

bool CChatSystem::GmSetActorCreatePos(char **args, INT_PTR nArgsCount, char * sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	int nCount = atoi(args[0]);

	//GetLogicServer()->GetDataProvider()->GetVocationConfig()->nDefaultCreatePosCount = nCount;

	return true;
}

bool CChatSystem::GmAddActorCreatePos(char **args, INT_PTR nArgsCount, char * sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 2) == false) return false;

	int nPosX = atoi(args[0]);
	int nPosY = atoi(args[1]);

	// for(int i=1;i<=3;i++)
	// {
	// 	PVOCATIONINITCONFIG pConfig =&( GetLogicServer()->GetDataProvider()->GetVocationConfig()->vocationTable[i]);
	// 	if(pConfig->nPosCount < MAX_CREATE_POSNUM-1)
	// 	{
	// 		pConfig->nCreatePos[pConfig->nPosCount].nScenceID = 1;
	// 		pConfig->nCreatePos[pConfig->nPosCount].nPosX = nPosX;
	// 		pConfig->nCreatePos[pConfig->nPosCount].nPosY = nPosY;
	// 		pConfig->nPosCount++;
	// 	}
	// }
	return true;
}

bool CChatSystem::GmSetIdlePlayer(char **args, INT_PTR nArgsCount, char * sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 2) == false) return false;
	byte nType = atoi(args[0]);
	byte nVal = atoi(args[1]);
	if (nType == 1)
	{
		bool boResult = nVal > 0 ? true:false;
		CMovementSystem::s_boCheckIdleLowPly = boResult;
	}
	else if(nType == 2)
	{
		CMovementSystem::s_btMinIdlePlyLevel = nVal;
	}
	return true;
}

bool CChatSystem::GmOpenQuickKick(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	int nType = atoi(args[0]);
	if (nType >= 0 && nType <= 100)
	{
		CActor::s_nCheckQuickCountKick = nType;
		return true;
	}
	return false;
}

bool CChatSystem::GmOpenQuickSeal(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	if(IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	int nType = atoi(args[0]);
	CActor::s_boCheckQuickSealActor = nType > 0 ? true : false;
	return true;
}

bool CChatSystem::GmSetEnterId(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	if(IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	int nType = atoi(args[0]);

	GetLogicServer()->SetGmCreateIndex(nType);
	return true;
}





bool CChatSystem::GmSetLhzsTitle(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	return false;
	if(IsArgsValid(args, nArgsCount, sRetMsg, 3) == false) return false;

	CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]);
	if(pActor)
	{
		int nType = atoi(args[2]);
		int nTitleid = atoi(args[1]);
		bool bFlag = true;
		if(nType == 0)	//取消
			bFlag = false;

		pActor->SetHeadTitle(nTitleid,bFlag);
	}
	else
	{
		return false;
	}

	return true;
}

bool CChatSystem::GmSetGm(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	// if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;

	// int nType = atoi(args[0]);
	// int nTitleid = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nGmTitleId;
	// if(nType == 0)	//取消
	// {
	// 	//((CActor*)m_pEntity)->SetHeadTitle(nTitleid,false);
	// 	((CActor*)m_pEntity)->GetNewTitleSystem().SetCurNewTitle(0);
	// }
	// else
	// {
	// 	//((CActor*)m_pEntity)->SetHeadTitle(nTitleid,true);
	// 	((CActor*)m_pEntity)->GetNewTitleSystem().SetCurNewTitle(nTitleid);
	// }

	return true;
}





//设置服务器的合区时间(用于后台设置开启某些系统用)
bool CChatSystem::GmSetServerTempCombineTime(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	int nMinutes = atoi(args[0]);
	unsigned int nCurrTime = GetGlobalLogicEngine()->getMiniDateTime();
	unsigned int nCombineTime = (nCurrTime + (nMinutes*60));
	GetGlobalLogicEngine()->GetMiscMgr().SetServerTempCombineTime(nCombineTime, nCurrTime);

	return true;
}
bool CChatSystem::GmKillAllMonster(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	CScene* pScene = m_pEntity->GetScene();
	if (pScene)
	{
		pScene->KillAllMonster(m_pEntity);
	}
	return true;
}
bool CChatSystem::GmKillMonster(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 3) == false) return false;
	int nRetCount = 0;
	const char * sSceneName = args[0];
	const char * sName = args[1];
	byte nDropItem = atoi(args[2]);
	CFuBen * pFb = m_pEntity->GetFuBen();
	if (pFb == NULL)
	{
		pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
	}
	CScene * pScene = pFb->GetSceneByName(sSceneName);
	if (pScene != NULL)
	{
		nRetCount = pScene->KillMonsterByName(sName, m_pEntity, nDropItem);
	}
	char buff[100];
	sprintf_s(buff, ArrayCount(buff), " kill monster count:%d", nRetCount);
	strcat(sRetMsg, buff);
	return true;
}

bool CChatSystem::GmSetCrossConfig(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	GetLogicServer()->ReloadCrossConfig(); //重新加载跨服的配置

	return true;
}

bool CChatSystem::GmSetCrossCommonId(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	int nServerId = atoi(args[0]);
	GetLogicServer()->SetCommonServerId(nServerId);
	return true;
}
bool CChatSystem::GmSetCommonServerId(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	if(GetLogicServer()->IsCommonServer() ) return false;
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	int nId = atoi(args[0]);
	GetLogicServer()->SetCommonServerId(nId); //设置跨服的ID

	GetLogicServer()->ReloadCrossConfig(false); //重新加载跨服的配置

	return true;
}

bool CChatSystem::GmSetRanking(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	// if (IsArgsValid(args, nArgsCount, sRetMsg, 3) == false)
	// {
	// 	return false;
	// }
	// int nId = atoi(args[1]);
	// int nVal = atoi(args[2]);
	// if (nVal < 0)
	// {
	// 	return false;
	// }
	// CRanking * pRanking = GetGlobalLogicEngine()->GetRankingMgr().GetRanking(args[0]);
	// if (pRanking != NULL)
	// {
		
	// 	if (nVal == 0)
	// 	{
	// 		pRanking->RemoveId(nId);
	// 	}
	// 	else
	// 	{
			
	// 		CRankingItem * pItem = pRanking->GetPtrFromId(nId);
	// 		if (pItem != NULL)
	// 		{
	// 			int nOldPoint = pItem->GetPoint();
	// 			pRanking->Update(nId, nVal - nOldPoint);
	// 		}
	// 		else
	// 		{
	// 			pRanking->AddItem(nId, nVal);
	// 		}
	// 	}
	// 	return true;
	// }
	return false;
}

bool CChatSystem::GmSaveAllRank(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	GetGlobalLogicEngine()->GetRankingMgr().Save();
	return true;
}

bool CChatSystem::GmChangeNameFlag(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;

	int bState = atoi(args[0]);

	if(bState == 0)
	{
		GetLogicServer()->SetChangeNameFlag(false);
	}
	else
	{
		GetLogicServer()->SetChangeNameFlag(true);
	}
	return true;
}

bool CChatSystem::GmOpenTrace(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	int bState = atoi(args[0]);

	if(bState == 0)
	{
		GetGlobalLogicEngine()->SetOpenPerfLog(false);
	}
	else
	{
		GetGlobalLogicEngine()->SetOpenPerfLog(true);
	}
	return true;
}

bool CChatSystem::GmSetSaveQuestLogFlag(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;

	int nState = atoi(args[0]);
	if(nState == 0)
	{
		GetLogicServer()->SetQuestSaveLog(false);
	}
	else
	{
		GetLogicServer()->SetQuestSaveLog(true);
	}
	return true;
}

bool CChatSystem::GmSetSaveCoinLogFlag(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;

	int nState = atoi(args[0]);
	if(nState == 0)
	{
		GetLogicServer()->SetCoinSaveLog(false);
	}
	else
	{
		GetLogicServer()->SetCoinSaveLog(true);
	}
	return true;
}

bool CChatSystem::GmSetBuildLevel(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 3) == false) return false;

	CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuildByName(args[0]);
	if(pGuild)
	{
		int nIndex = atoi(args[1]);
		int nValue = atoi(args[2]);

		pGuild->ChangeGuildBuildingLev(nIndex,nValue);
	}
	else
	{
		return false;
	}

	return true;
}

bool CChatSystem::GmClearGuildBuildData(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuildByName(args[0]);
	if(pGuild)
	{
		pGuild->SetGuildChallengeTimes(0);
		pGuild->SetBossChanllengeHandle(0);
		pGuild->m_nThunderPower;
		pGuild->m_nFillThunderTimes;
	}
	else
	{
		return false;
	}

	return true;
}

bool CChatSystem::GmClearGuildBuildTime(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuildByName(args[0]);
	if(pGuild)
	{
		pGuild->SetGuildBuildingCdTime(0);
	}
	else
	{
		return false;
	}

	return true;
}

bool CChatSystem::GmclearActorBuildData(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;

	CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]);
	if(pActor)
	{
		pActor->GetGuildSystem()->OnNewDayArriveOP();
	}
	else
	{
		return false;
	}

	return true;
}

bool CChatSystem::GmChangeActorGx(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 2) == false) return false;

	CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]);
	if(pActor)
	{
		pActor->GetGuildSystem()->ChangeGuildGx(atoi(args[1]), GameLog::Log_BackStage);
		pActor->GetGuildSystem()->ChangeTodayGx(atoi(args[1]));
	}
	else
	{
		return false;
	}

	return true;
}

bool CChatSystem::GmClearBossHard(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	GetLogicServer()->GetDataProvider()->GetMonsterConfig().ClearBossGrow();
	return true;
}

bool CChatSystem::GMSetTradingQuota( char **args, INT_PTR nArgsCount, char * sRetMsg )
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	int nValue = atoi(args[0]);
	if(!m_pEntity || nValue < 0) return false;
	m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_TRADINGQUOTA,nValue );
	return true;
}

bool CChatSystem::GMSetBlessValue( char **args, INT_PTR nArgsCount, char * sRetMsg )
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	int nValue = atoi(args[0]);
	if(!m_pEntity || nValue < 0) return false;
	m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_Bless,nValue );
	return true;
}



bool CChatSystem::GMSetQuestState( char **args, INT_PTR nArgsCount, char * sRetMsg )
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	int nId = atoi(args[0]);
	int nState = atoi(args[1]);
	if(!m_pEntity) return false;
	((CActor*)m_pEntity)->GetQuestSystem()->GMSetQuestState(nId, nState);
	return true;
}


bool CChatSystem::GMClearQuestlog( char **args, INT_PTR nArgsCount, char * sRetMsg )
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	if(!m_pEntity) return false;
	((CActor*)m_pEntity)->GetQuestSystem()->ClearAllQuest();
	return true;
}


bool CChatSystem::GMSetJMLv( char **args, INT_PTR nArgsCount, char * sRetMsg )
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	int nValue = atoi(args[0]);
	if(!m_pEntity || nValue < 0) return false;
	m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_MERIDIALv,nValue );
	return true;
}


bool CChatSystem::GmSetGuildCoin( char **args, INT_PTR nArgsCount, char * sRetMsg )
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	int nCoin = atoi(args[0]);
	if (((CActor *)m_pEntity)->GetGuildSystem()->GetGuildPtr())
	{
		((CActor *)m_pEntity)->GetGuildSystem()->GetGuildPtr()->ChangeGuildCoin(nCoin,0,"gm");
		return true;
	}
	return false;
}

bool CChatSystem::GmAddMail(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if ( args_count < 3)
	{
		m_pEntity->GetMailSystem().SendAllMail();
		return true;
	}
	unsigned int nActorId = atoi(args[0]);
	if (nActorId == 0)
	{
		nActorId = m_pEntity->GetId();
	}
	int max_op_param_count = 9;
	wylib::container::CBaseList<wylib::string::CAnsiString*> op_param_list;
	InitParamList(args, args_count, op_param_list, max_op_param_count);
	
	CMailSystem::MAILATTACH mailAttr[MAILATTACHCOUNT];
	
	mailAttr[0].nType = atoi(op_param_list.get(3)->rawStr());
	mailAttr[0].item.wItemId = atoi(op_param_list.get(4)->rawStr());
	mailAttr[0].item.wCount = atoi(op_param_list.get(5)->rawStr());
	mailAttr[1].nType = atoi(op_param_list.get(6)->rawStr());
	mailAttr[1].item.wItemId = atoi(op_param_list.get(7)->rawStr());
	mailAttr[1].item.wCount = atoi(op_param_list.get(8)->rawStr());

	CMailSystem::SendMail(nActorId,args[1],args[2],mailAttr
	);
	return true;
}



bool CChatSystem::GmAddServerMail(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if ( args_count < 3)
	{
		m_pEntity->GetMailSystem().SendAllMail();
		return true;
	}
	unsigned int nActorId = atoi(args[0]);
	// if (nActorId == 0)
	// {
	// 	nActorId = m_pEntity->GetId();
	// }
	unsigned int nCurrTime = GetGlobalLogicEngine()->getMiniDateTime();
	ServerMail serverMail;
	ServerMailDef& mail = serverMail.serverData;
	char* szTitle		= args[1];
	char* szContent		= args[2];
	memcpy(mail.sTitle, szTitle, sizeof(mail.sTitle));
	memcpy(mail.sContent, szContent, sizeof(mail.sContent));
	printf("szTitle:%s, count:%s\n", szTitle, szContent);
	mail.nMinVipLevel			= 0;
	mail.nMaxVipLevel				= 0;
	mail.nMinLevel			= 0;//最低人物等级
	mail.nMaxLevel			= 999;		//最高人物等级
	mail.nMinRegisterTime			= 0;		//最早注册时间
	mail.nMaxRegisterTime		= 0;		//最晚注册时间	
	mail.nMinLoginTime	= 0;		//最早登录时间
	mail.nMaxLoginTime	= 0;		//最晚登录时间
	mail.btOnline	= 0;		//在线	0全部，1在线，2不在线
	mail.btSex	= -1;		//性别 -1全部，0男， 1女
	mail.nWay	= 0;		//用途标识,0系统正常邮件，1后台福利

	int nType	= atoi(args[3]);		//物品type
	int nId	= atoi(args[4]);		// 物品id
	int nCount	= atoi(args[5]);		// 物品数量

	serverMail.mailAttach[0].actorAward.wId		= (WORD)nId;
	serverMail.mailAttach[0].actorAward.btType				= nType;
	serverMail.mailAttach[0].actorAward.wCount			= (WORD)nCount;

	GetGlobalLogicEngine()->GetMailMgr().TestAddServerMail(serverMail);
	return true;
}

bool CChatSystem::GmAddOneMail(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if ( args_count < 11)
	{
		m_pEntity->GetMailSystem().SendAllMail();
		return true;
	}
	unsigned int nActorId = atoi(args[0]);
	if (nActorId == 0)
	{
		nActorId = m_pEntity->GetId();
	}
	char* szTitle		= args[1];
	char* szContent		= args[2];
	int nType			= atoi(args[3]);
	int nId				= atoi(args[4]);
	int nCount			= atoi(args[5]);
	int nFlag			= atoi(args[6]);		//0-正常，1-绑定，2-不可交易
	int nStar			= atoi(args[7]);		//强化星级
	int nLostStar		= atoi(args[8]);		//强化损失星级
	int bInSourceType	= atoi(args[9]);		//铭刻等级
	int nAreaId	= atoi(args[10]);		//铭刻经验

	CMailSystem::MAILATTACH mailAttr[MAILATTACHCOUNT];
	mailAttr[0].nType				= nType;
	mailAttr[0].item.wItemId		= (WORD)nId;
	mailAttr[0].item.wCount			= (WORD)nCount;
	mailAttr[0].item.btFlag			= (BYTE)nFlag;
	mailAttr[0].item.wStar			= (WORD)nStar;
	mailAttr[0].item.bLostStar		= (BYTE)nLostStar;
	mailAttr[0].item.bInSourceType = (BYTE)bInSourceType;
	mailAttr[0].item.nAreaId	= nAreaId;

	CMailSystem::SendMail(nActorId,szTitle, szContent, mailAttr);
	return true;
}

//------------------------反加速外挂相关-----begin--------------------------
bool CChatSystem::GmSetOpenCkSpeedFalg(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;

	int nFlag = atoi(args[0]);
	if(nFlag == 0)
	{
		CCheckSpeedSystem::m_bOpenVerifyFlag = false;
	}
	else
	{
		CCheckSpeedSystem::m_bOpenVerifyFlag = true;
	}

	return true;
}

bool CChatSystem::GmSetDeviationTimes(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;

	CCheckSpeedSystem::m_DeviationTimes = atoi(args[0]);

	return true;
}

bool CChatSystem::GmSetCkIntevalTime(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;

	CCheckSpeedSystem::m_IntevalTime = atoi(args[0]);

	return true;
}

bool CChatSystem::GmSetSpeedCheck(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 4) == false) return false;

	CCheckSpeedSystem::m_nSpeedCheck = atoi(args[0]);
	CCheckSpeedSystem::m_nTimeRate = atoi(args[1]);
	CCheckSpeedSystem::m_nStartCheck = atoi(args[2]);

	_stprintf(sRetMsg, _T("GmSetSpeedCheck, m_nSpeedCheck:%d, m_nTimeRate:%d, m_nStartCheck:%d, s_nNormalTime:%d"), CCheckSpeedSystem::m_nSpeedCheck,
		CCheckSpeedSystem::m_nTimeRate, CCheckSpeedSystem::m_nStartCheck);

	return true;
}

bool CChatSystem::GmSetSpeedValue(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 2) == false) return false;

	int nType = atoi(args[0]);
	switch(nType)
	{
	case 1:
		{
			CCheckSpeedSystem::m_nUpLimit = atoi(args[1]);
			break;
		}

	case 2:
		{
			CCheckSpeedSystem::m_nLowLimit = atoi(args[1]);
			break;
		}

	case 3:
		{
			CCheckSpeedSystem::m_nUpLimit2 = atoi(args[1]);
			break;
		}

	case 4:
		{
			CCheckSpeedSystem::m_nLowLimit2 = atoi(args[1]);
			break;
		}

	default:
		break;
	}
	return true;
}

bool CChatSystem::GmLoadNpcFile(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 2) == false) return false;
	if (!_stricmp(args[0], "SYS"))
	{
		return GetGlobalLogicEngine()->GetMiscMgr().HotUpdateScript(args[1]);	
	}
	return false;
}


bool CChatSystem::GmLoadQuestFile( char ** args, INT_PTR nArgsCount, char * sRetMsg )
{
	// if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) return false;
	// CNpc * pNpc = GetGlobalLogicEngine()->GetScriptNpc();
	// if (pNpc != NULL)
	// {
	// 	char sBuff[256];
	// 	sprintf_s(sBuff, ArrayCount(sBuff), "data/functions/NpcMiscFun/NpcDialog/%s.txt", args[0]);
	// 	return pNpc->LoadScriptBuff(sBuff);
	// }
	return false;
}

bool CChatSystem::GmTotalAllSceneMonsterCount(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	unsigned int nTotalCount = GetGlobalLogicEngine()->GetFuBenMgr()->TotalAllSceneMonsterCount();
	_stprintf(sRetMsg, _T("All Scene Monster Count is %d."), nTotalCount);
	return true;
}

bool CChatSystem::GmGetCurrSceneInfo(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	CScene* pScene = ((CActor*)m_pEntity)->GetScene();
	if ( !pScene )
	{
		return false;
	}
	_stprintf(sRetMsg, _T("name:%s,id:%d W:%d H:%d"), pScene->GetSceneName(),
		pScene->GetSceneId(), pScene->GetSceneWidth(), pScene->GetSceneHeight() );
	return true;
}

bool CChatSystem::GmSetTaxing(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	if (IsArgsValid(args,nArgsCount,sRetMsg,2) == false)
	{
		return false;
	}
	BYTE btType = (BYTE)atoi(args[0]);
	int nValue  = (int)atoi(args[1]);
	if (btType == 1)
	{
		//GetGlobalLogicEngine()->GetGuildMgr().m_CurAllTaxing = nValue;
	}
	else if (btType == 2)
	{
		//GetGlobalLogicEngine()->GetGuildMgr().m_ReciveTaxing = nValue;
	}
	else
	{
		return false;
	}
	return true;
}

bool CChatSystem::GmSetRundomKey(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 2) == false) return false;

	CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]);
	if(pActor)
	{
		pActor->SetRundomKey(atoi(args[1]));
	}

	return true;
}

bool CChatSystem::GmDeleteGuild(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if(IsArgsValid(args,nArgsCount,sRetMsg,1) == false) return false;
	CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]);
	if(pActor)
	{
		if(pActor->GetGuildSystem())
		{
			CGuild* pGuild = pActor->GetGuildSystem()->GetGuildPtr();
			if ( pGuild )
			{
				GetGlobalLogicEngine()->GetGuildMgr().SendDbMsgDeleteGuild(pActor->GetHandle(),pGuild->m_nGid);
			}

		}
	}
	else
	{
		return false;
	}
	return true;
}

bool CChatSystem::GmAddNewTitle(char ** args, INT_PTR nArgsCount, char * sRegMsg)
{
	if (IsArgsValid(args, nArgsCount, sRegMsg, 2) == false) return false;
	CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]);
	if (!pActor)
	{
		return false;
	}
	int nId = atoi(args[1]);
	return pActor->GetNewTitleSystem().addNewTitle(nId);
}

bool CChatSystem::GmDelNewTitle(char ** args, INT_PTR nArgsCount, char * sRegMsg)
{
	if (IsArgsValid(args, nArgsCount, sRegMsg, 2) == false) return false;
	CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]);
	if (!pActor)
	{
		return false;
	}
	int nId = atoi(args[1]);
	return pActor->GetNewTitleSystem().delNewTitle(nId);
}

bool CChatSystem::GmAddCustomTitle(char ** args, INT_PTR nArgsCount, char * sRegMsg)
{
	if (IsArgsValid(args, nArgsCount, sRegMsg, 2) == false) return false;
	CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]);
	if (!pActor)
	{
		return false;
	}
	int nId = atoi(args[1]);
	return pActor->GetNewTitleSystem().addCustomTitle(nId);
}

bool CChatSystem::GmDelCustomTitle(char ** args, INT_PTR nArgsCount, char * sRegMsg)
{
	if (IsArgsValid(args, nArgsCount, sRegMsg, 2) == false) return false;
	CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]);
	if (!pActor)
	{
		return false;
	}
	int nId = atoi(args[1]);
	return pActor->GetNewTitleSystem().delCustomTitle(nId);
}



bool CChatSystem::GmDelVar(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false ) return false;
	CCLVariant &var = m_pEntity->GetActorVar();
	CCLVariant* p = var.get(args[0]);
	if (p)
	{
		p->~CCLVariant();
	}
	return true;
}
bool CChatSystem::GmResetWorldLevel(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	GetGlobalLogicEngine()->GetWorldLevelMgr().ResetWorldLevel();
	return true;
}
bool CChatSystem::GmReLoadWorldLevel(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	GetGlobalLogicEngine()->GetWorldLevelMgr().ResetWorldLevel();
	GetGlobalLogicEngine()->GetWorldLevelMgr().ReloadWorldLevel();
	return true;
}
bool CChatSystem::GmSetFootPrintPay(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (nArgsCount == 1) 
	{
		if (args[0] == NULL)
		{
			return false;
		}
		int nTime =  atoi(args[0]);
		if (nTime == 0)
		{
			//GetGlobalLogicEngine()->GetMiscMgr().OnEveryDayPayOver(CMiscMgr::edaFootPrintPay);
		}
		else
		{
			return false;
		}
	}
	if (IsArgsValid(args, nArgsCount, sRetMsg, 3) == false) 
		return false;
	char* sTime = args[0];
	int nLastTime = atoi(args[1]);
	int nActId = atoi(args[2]);
	CMiniDateTime nStart;
	//nStart.encode(sParam);
	CBackStageSender::StdTimeStr2MiniTime(sTime, nStart);
	//GetGlobalLogicEngine()->GetMiscMgr().OnEveryDayPayStart(CMiscMgr::edaFootPrintPay, nStart.tv, nLastTime, nActId);
	return true;	
}
bool CChatSystem::GmSetActorCallScript(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 2) == false) return false;
	char* sName = args[0];

	CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(sName);
	if (!pActor) return false;

	CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
	if (pNpc == NULL) return false;

	CScriptValueList paramList, retList;

	unsigned int nActorId=pActor->GetProperty<unsigned int>(PROP_ENTITY_ID);

	paramList << (CActor*)pActor;
	if (!pNpc->GetScript().Call(args[1], paramList, retList))
	{
		OutputMsg(rmError, _T("GmSetActorCallScript failed"));
		return false;
	}

	return true;
}

bool CChatSystem::GmOpenBlackHouse(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if (IsArgsValid(args, nArgsCount, sRetMsg, 1) == false) 
	{
		return false;
	}
	int flag = atoi(args[0]);
	if (flag ==  CCheckSpeedSystem::enBlackHouse || flag == CCheckSpeedSystem::enCloseActor)
	{
		CCheckSpeedSystem::m_useSpeedHandle = flag;
	}
	
	return true;
}
bool CChatSystem::GmQuitBlackHouse(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count != 1 && args_count != 0)
	{
		sprintf(sRetMsg,"Need 0 or 1 param");
		return false;
	}
	CActor* pActor = NULL;
	if (args_count == 1)
	{
		char* sName = args[0];
		pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(sName);
	}
	else if (args_count == 0)
	{
		pActor = (CActor*)m_pEntity;
		
	}
	if (pActor)
	{
		pActor->GetCheckSpeedSystem().TriggerEvent(CCheckSpeedSystem::enQuitBlackHouse);
		((CActor*)m_pEntity)->SendTipmsgFormat(_T("%s QuitBlackHouse Success "),ttFlyTip + ttChatWindow, pActor->GetEntityName());
	}
	return true;
}
bool CChatSystem::GmCleanBlackHouse(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count != 1 && args_count != 0)
	{
		sprintf(sRetMsg,"Need 0 or 1 param");
		return false;
	}
	CActor* pActor = NULL;
	if (args_count == 1)
	{
		char* sName = args[0];
		pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(sName);
	}
	else if (args_count == 0)
	{
		pActor = (CActor*)m_pEntity;

	}
	if (pActor)
	{
		pActor->GetCheckSpeedSystem().TriggerEvent(CCheckSpeedSystem::enCleanBlackHousedata);
		pActor->GetCheckSpeedSystem().TriggerEvent(CCheckSpeedSystem::enQuitBlackHouse);
		((CActor*)m_pEntity)->SendTipmsgFormat(_T("%s CleanBlackHouse Success "),ttFlyTip + ttChatWindow, pActor->GetEntityName());
	}
	return true;
}
bool CChatSystem::GmKickAndSetActorStatus(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count != 3 )
	{
		sprintf(sRetMsg,"Need 2 or 3 param");
		return false;
	}
	int nFlag = atoi(args[0]);
	bool boDisApear = false;
	if ( nFlag != 0 )
	{
		boDisApear = true;
	}
	return GetGlobalLogicEngine()->GetMiscMgr().KickAllActorByLevel(atoi(args[1]),atoi(args[2]),boDisApear);

}
bool CChatSystem::GMSetSoulLevel(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if(IsArgsValid(args, nArgsCount, sRetMsg, 2) ==false) 
		return false;
	int nLevel		= atoi(args[1]);
	CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]);
	if (pActor)
	{
		GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction(pActor,"resetSoulLevel",nLevel);
		((CActor*)m_pEntity)->SendTipmsgFormat(_T("%s SetSoulLevel Success "),ttFlyTip + ttChatWindow, pActor->GetEntityName());
	}
	else
	{
		((CActor*)m_pEntity)->SendTipmsgFormat(_T("Error:Actor Offline"),ttFlyTip + ttChatWindow);
	}
	
	return true;
}

bool CChatSystem::GMAddRankPoint(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	// if(IsArgsValid(args, nArgsCount, sRetMsg, 2) ==false) 
	// 	return false;
	// CRanking* rank = GetGlobalLogicEngine()->GetRankingMgr().GetRanking(args[0]);
	// if(!rank){
	// 	sprintf(sRetMsg, "ranking not found: %s", args[0]);
	// 	return false;
	// }
	// return rank->Update(m_pEntity->GetId(), atoi(args[1])) != NULL;
	return true;
}	 
bool CChatSystem::SetAchieve(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if(IsArgsValid(args, nArgsCount, sRetMsg, 3) ==false) 
		return false;
	CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]);
	if (pActor)
	{
		int nAchieveId = atoi(args[1]);
		int nState = atoi(args[2]);
		if (nState == 1)
		{
			pActor->GetAchieveSystem().SetAchieveInstantFinish(nAchieveId,true);
		}
		else
		{
			pActor->GetAchieveSystem().SetAchieveUnFinished(nAchieveId);
		}
		
		((CActor*)m_pEntity)->SendTipmsgFormat(_T("%s SetAchieve Success "),ttFlyTip, pActor->GetEntityName());
	}
	else
	{
		return false;
	}
	return true;
}

bool CChatSystem::GmFuBen(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count != 2 )
	{
		sprintf(sRetMsg,"Need 3 param");
		return false;
	}
	CScriptValueList arg; 
	int nFuBenId = atoi(args[0]);
	int nIndex = atoi(args[1]);
	arg << m_pEntity;
	arg << nFuBenId;			
	arg << nIndex;
	CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
	if (pNpc == NULL)
		return false;
	pNpc->GetScript().Call("DoGmFuBen", arg, arg);
	return true;
}
bool CChatSystem::GMSetRingLevel(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count != 3 )
	{
		sprintf(sRetMsg,"Need 3");
		return false;
	}
	int wRingId		= atoi(args[1]);
	int nLevel = atoi((args[2]));
	CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]);
	if (pActor)
	{
		GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction(pActor,"GMSetRingLevel",wRingId,nLevel);
		((CActor*)m_pEntity)->SendTipmsgFormat(_T("%s SetRingLevel Success "),ttFlyTip + ttChatWindow, pActor->GetEntityName());
	}
	else
	{
		((CActor*)m_pEntity)->SendTipmsgFormat(_T("Error:Actor Offline"),ttFlyTip + ttChatWindow);
	}
	return true;
}
bool CChatSystem::QuestMove(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	if(!IsArgsValid(args, nArgsCount, sRetMsg, 3))return false;
	CScriptValueList paramList, retParamList;
	int nSceneId = 0;
	CFuBen *pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
	if (pFb)
	{
		CScene* pScene = pFb->GetSceneByName(args[0]);
		if (pScene)
			nSceneId = pScene->GetSceneId();
	}
	paramList<<m_pEntity<<nSceneId<<atoi(args[1])<<atoi(args[2]);
	CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
	if (pNpc == NULL) return false;
	if (!pNpc->GetScript().Call("QuestMove", paramList, retParamList))
	{
		const RefString &s = pNpc->GetScript().getLastErrorDesc();
		OutputMsg(rmError, (LPCSTR)s);
	}
	return true;
}

bool CChatSystem::GmDebugLua(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count != 3 )
	{
		sprintf(sRetMsg,"Need 2 param");
		return false;
	}
	CScriptValueList arg, retList; 
	int arg1 = atoi(args[0]);
	int arg2 = atoi(args[1]);
	int arg3 = atoi(args[2]);
	arg << m_pEntity;
	arg << arg1;			
	arg << arg2;
	arg << arg3;
	GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().Call("DOGMDebugLua", arg, retList, 1);
	if(retList.count() >= 0 && (bool)retList[0] == false )
	{
		int a = 1;
	}
	return true;
}

//商店的GM指令
bool CChatSystem::GmShop(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count != 2 )
	{
		sprintf(sRetMsg,"Need 2 param");
		return false;
	}
	CScriptValueList arg; 
	int nShopId = atoi(args[0]);
	int nIndex = atoi(args[1]);
	arg << m_pEntity;
	arg << nShopId;			
	arg << nIndex;
	GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().Call("DoGMShopHandle", arg, arg, 0);
	return true;
}

bool CChatSystem::GmPkValue(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count != 1 )
	{
		sprintf(sRetMsg,"Need 2 param");
		return false;
	}
	CScriptValueList arg; 
	int nPkValue = atoi(args[0]);
	m_pEntity->GetPkSystem().AddPkValue(nPkValue);
	return true;
}	

bool CChatSystem::GmExploit(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	// INT_PTR args_count = GetArgsCount(args, nArgsCount);
	// if (args_count != 1 )
	// {
	// 	m_pEntity->SendOldTipmsgFormatWithId(tpExploitMy, ttTipmsgWindow, (int)m_pEntity->GetProperty<int>(PROP_ACTOR_EXPLOIT));
	// 	return true;
	// }
	// CScriptValueList arg; 
	// int nValue = atoi(args[0]);
	// m_pEntity->ChangeExploit(nValue);
	// m_pEntity->ChangeTodayExploit(nValue);
	return true;
}
bool CChatSystem::GmSetEquipPosStrongLevel(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 3 )
	{
		sprintf(sRetMsg,"Need 3 param at least");
		return true;
	}
	//CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(args[0]);
	//if (!pActor)
	//{
	//	sprintf(sRetMsg,"error:actor offline");
	//	return true;
	//}
	//int nPos = atoi(args[1]);
	//int nLevel = atoi(args[2]);
	//CStdItemProvider& stdItemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	//if (nPos == -1)//全部位置都设
	//{
	//	for (int i = 0; i < CUserEquipment::EquipmentCount; i++)
	//	{
	//		if (stdItemProvider.IsEquipPosStrongValidPos(i))
	//		{
	//			pActor->GetEquipmentSystem().SetEquipPosStrongLevel(i, nLevel);
	//		}
	//	}
	//}
	//else if (stdItemProvider.IsEquipPosStrongValidPos(nPos))
	//{
	//	pActor->GetEquipmentSystem().SetEquipPosStrongLevel(nPos, nLevel);
	//}
	//GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction(pActor,"EquipPosStrongSendData");
	//((CActor*)m_pEntity)->SendTipmsgFormat(_T("%s SetEquipPosStrongLevel Success "),ttFlyTip + ttChatWindow, pActor->GetEntityName());
	return true;
}

bool CChatSystem::GmSetGuildTree(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 3 )
	{
		sprintf(sRetMsg,"Need 3 param at least");
		return false;
	}

	CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuildByName(args[0]);
	if (!pGuild)
	{
		sprintf(sRetMsg,"has no the guild");
		return false;
	}
	int nTreeLevel	= atoi(args[1]);
	int nTreeDegree = atoi(args[2]);
	pGuild->SetGuildTreeData(nTreeLevel, nTreeDegree);
	((CActor*)m_pEntity)->GetGuildSystem()->SendGuildTreeData();
	return true;
}

bool CChatSystem::GmSetGuildFruit(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 4 )
	{
		sprintf(sRetMsg,"Need 4 param at least");
		return false;
	}

	CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuildByName(args[0]);
	if (!pGuild)
	{
		sprintf(sRetMsg,"has no the guild");
		return false;
	}
	int nFruitLevel		= atoi(args[1]);
	int nFruitDegree	= atoi(args[2]);
	int nFruitNum		= atoi(args[3]);
	pGuild->SetGuildFruitData(nFruitLevel, nFruitDegree, nFruitNum);
	((CActor*)m_pEntity)->GetGuildSystem()->SendGuildFruitData();
	return true;
}

bool CChatSystem::GmSetMyGuildTree(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 2 )
	{
		sprintf(sRetMsg,"Need 2 param at least");
		return false;
	}

	CGuild* pGuild = ((CActor*)m_pEntity)->GetGuildSystem()->GetGuildPtr();
	if (!pGuild)
	{
		sprintf(sRetMsg,"has no the guild");
		return false;
	}
	int nTreeLevel	= atoi(args[0]);
	int nTreeDegree = atoi(args[1]);
	pGuild->SetGuildTreeData(nTreeLevel, nTreeDegree);
	((CActor*)m_pEntity)->GetGuildSystem()->SendGuildTreeData();
	return true;
}

bool CChatSystem::GmSetMyGuildFruit(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 3 )
	{
		sprintf(sRetMsg,"Need 3 param at least");
		return false;
	}

	CGuild* pGuild = ((CActor*)m_pEntity)->GetGuildSystem()->GetGuildPtr();
	if (!pGuild)
	{
		sprintf(sRetMsg,"has no the guild");
		return false;
	}
	int nFruitLevel		= atoi(args[0]);
	int nFruitDegree	= atoi(args[1]);
	int nFruitNum		= atoi(args[2]);
	pGuild->SetGuildFruitData(nFruitLevel, nFruitDegree, nFruitNum);
	((CActor*)m_pEntity)->GetGuildSystem()->SendGuildFruitData();
	return true;
}
bool CChatSystem::GmClearConsignItem(char **args, INT_PTR nArgsCount, char *sRetMsg)
{

	return true; 
}
bool CChatSystem::GmAddConsignItem(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 1 )
	{
		sprintf(sRetMsg,"Need 1 param at least");
		return false;
	}
	int nId = atoi(args[0]);
	int nCount = atoi(args[1]);
	int nPrice = atoi(args[2]);
	CStdItemProvider& stdItemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	ConsignmentProvider& consignProvider = GetLogicServer()->GetDataProvider()->GetConsignmentProvider();
	// CVector<const CStdItem*> itemList;
	// for (int i=0; i < stdItemProvider.count(); i++)
	// {
	// 	const CStdItem* pStdItem= &stdItemProvider.operator const CStdItem*()[i];
	// 	if (!pStdItem)
	// 	{
	// 		continue;
	// 	}
	// 	// if (pStdItem->m_nConsignType > 0)
	// 	// {
	// 	// 	itemList.add(pStdItem);
	// 	// }
	// }
	// for (int i =0; i < nCount; i++)
	// {
		// CUserItem* pConsignItem = GetGlobalLogicEngine()->AllocUserItem(true);
		// if (pConsignItem)
		// {
		// 	int idIndex = wrandvalue()%itemList.count();
		// 	const CStdItem* pStdItem = itemList[idIndex];
		// 	pConsignItem->wItemId = pStdItem->m_nIndex ;
		// 	if (pStdItem->isEquipment())
		// 	{
		// 		pConsignItem->btQuality = wrandvalue()%6;
		// 		pConsignItem->wStar = wrandvalue()%10;
		// 		int nQualityDataIndex =  wrandvalue()%stdItemProvider.GetQualityIndexListCount();
		// 		RandAttrSelector::InitSmithByUserItem(pConsignItem, nQualityDataIndex);
		// 	}
		// 	pConsignItem->wCount = wrandvalue()%(pStdItem->m_wDupCount+1)+1;
		// 	CStdItem::AssignInstance(pConsignItem,pStdItem);
		// 	int nPrice = wrandvalue()%consignProvider.nMaxSellPrice+ consignProvider.nMinSellPrice;
		// 	GetGlobalLogicEngine()->GetConsignmentMgr().AddConsignItem(pConsignItem,0, nPrice,pConsignItem->wCount);
		// }
		ConsignmentData data;
		data.nActorId = m_pEntity->GetId();
		data.nPrice = nPrice;
		data.nSellTime = time(NULL) + consignProvider.nSellTime*3600;
		data.nState = eConsignmentOnShelf;
		// data.nItemId = nId;
		// data.nCount = nCount;
		// memcpy(&data.item, );
		GetGlobalLogicEngine()->GetConsignmentMgr().AddConsignItem(data);
	// }
	
	return true;
}

bool CChatSystem::GmSetGuildTask(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 3 )
	{
		sprintf(sRetMsg,"Need 3 param at least");
		return false;
	}

	CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuildByName(args[0]);
	if (!pGuild)
	{
		sprintf(sRetMsg,"has no the guild");
		return false;
	}
	int nTaskId		= atoi(args[1]);
	int nTaskSche	= atoi(args[2]);
	pGuild->SetGuildTaskId(nTaskId);
	pGuild->SetGuildTaskSche(nTaskSche);
	((CActor*)m_pEntity)->GetGuildSystem()->SendGuildTaskInfo();
	return true;
}

bool CChatSystem::GmRefreshGuildTask(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 2 )
	{
		sprintf(sRetMsg,"Need 3 param at least");
		return false;
	}

	CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuildByName(args[0]);
	if (!pGuild)
	{
		sprintf(sRetMsg,"has no the guild");
		return false;
	}
	int nTaskSche	= atoi(args[1]);
	pGuild->RefreshGuildTask(true);				//强制刷新
	pGuild->SetGuildTaskSche(nTaskSche);		//任务进度
	((CActor*)m_pEntity)->GetGuildSystem()->SendGuildTaskInfo();
	return true;
}

bool CChatSystem::GmDoWorship(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 2 )
	{
		sprintf(sRetMsg,"Need 2 param at least");
		return false;
	}
	int nFlag  = atoi(args[0]);
	int nParam = atoi(args[1]);
	GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction((CActor*)m_pEntity, "GmDoWorshipMonarch", nFlag, nParam);
	return true;
}

bool CChatSystem::GmDoProtecteTuCity(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 2 )
	{
		sprintf(sRetMsg,"Need 2 param at least");
		return false;
	}
	int nFlag  = atoi(args[0]);
	int nParam = atoi(args[1]);
	GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction((CActor*)m_pEntity, "GmDoProtecteTuCity", nFlag, nParam);
	return true;
}

bool CChatSystem::GmDoRewardQuest(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 2 )
	{
		sprintf(sRetMsg,"Need 2 param at least");
		return false;
	}
	int nFlag  = atoi(args[0]);
	int nParam = atoi(args[1]);
	GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction((CActor*)m_pEntity, "GmDoRewardQuest", nFlag, nParam);
	return true;
}

//后台补偿
bool CChatSystem::GmDoCompensate(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 1 )
	{
		sprintf(sRetMsg,"Need 1 param at least");
		return false;
	}

	char* strCom = args[0];

	char* sFlag = strchr(strCom,'|');
	if (sFlag != NULL)
	{
		strCom[sFlag-strCom] = 0;	//开关
		sFlag++;
		int nFlag = atoi(strCom);
		if( nFlag == 0 )			//关闭补偿
		{
			CMiscMgr &mgr = GetGlobalLogicEngine()->GetMiscMgr();
			mgr.SetCompensateVar(0);	//设置ID为0关闭
			mgr.DoScriptFunction("TerminateBackCompensate");
		}
		else if( nFlag == 2 )			//清空排行榜（领取数据）
		{
			CMiscMgr &mgr = GetGlobalLogicEngine()->GetMiscMgr();
			mgr.DoScriptFunction("ClearBackCompensateRank");
		}
		else		//打开补偿
		{
			CMiscMgr &mgr = GetGlobalLogicEngine()->GetMiscMgr();
			char* sComId = strchr(sFlag,'|');
			if( sComId )
			{
				sFlag[sComId-sFlag] = 0;
				sComId++;
				int nComId = atoi(sFlag);

				char* sLevel = strchr(sComId,'|');
				if( sLevel )
				{
					sComId[sLevel-sComId] = 0;
					sLevel++;
					int nLevel = atoi(sComId);

					char* sBeginTime = strchr(sLevel,'|');
					if( sBeginTime )
					{
						sLevel[sBeginTime-sLevel] = 0;
						sBeginTime++;
						CMiniDateTime miniBeginTime;
						CBackStageSender::StdTimeStr2MiniTime(sLevel, miniBeginTime);

						char* sLastTime = strchr(sBeginTime,'|');		//持续时间
						if( sLastTime )
						{
							sBeginTime[sLastTime-sBeginTime] = 0;
							sLastTime++;
							int nLastTime = atoi(sBeginTime) * 60;

							char* szTitle = sLastTime;		//最后是补偿title

							mgr.SetCompensateVar( nComId, miniBeginTime, nLastTime, nLevel, szTitle,true );
						}
					}
				}
			}
		}
	}
	else
	{
		int nFlag = atoi(strCom);
		if( nFlag == 0 )			//关闭补偿
		{
			CMiscMgr &mgr = GetGlobalLogicEngine()->GetMiscMgr();
			mgr.SetCompensateVar(0);	//设置ID为0关闭
			mgr.DoScriptFunction("TerminateBackCompensate");
		}
		else if( nFlag == 2 )			//清空排行榜（领取数据）
		{
			CMiscMgr &mgr = GetGlobalLogicEngine()->GetMiscMgr();
			mgr.DoScriptFunction("ClearBackCompensateRank");
		}
	}

	return true;
}

bool CChatSystem::GmDoSupplyBattle(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 2 )
	{
		sprintf(sRetMsg,"Need 2 param at least");
		return false;
	}

	int nFlag  = atoi(args[0]);
	int nParam = atoi(args[1]);
	GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction((CActor*)m_pEntity, "GmDoSupplyBattle", nFlag, nParam);
	return true;
}

bool CChatSystem::GmDoSevenGoodGift(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 2 )
	{
		sprintf(sRetMsg,"Need 2 param at least");
		return false;
	}

	int nFlag  = atoi(args[0]);
	int nParam = atoi(args[1]);
	GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction((CActor*)m_pEntity, "GmDoSevenGoodGift", nFlag, nParam);
	return true;
}

bool CChatSystem::GmDoChallengeFB(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 2 )
	{
		sprintf(sRetMsg,"Need 2 param at least");
		return false;
	}

	CActor *pActor = (CActor*)m_pEntity;
	int nFlag  = atoi(args[0]);
	int nParam = atoi(args[1]);
	GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction((CActor*)m_pEntity, "GmDoChallengeFB", nFlag, nParam);
	return true;
}

bool CChatSystem::GmDoAllCharge(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 2 )
	{
		sprintf(sRetMsg,"Need 2 param at least");
		return false;
	}

	CActor *pActor = (CActor*)m_pEntity;
	int nFlag  = atoi(args[0]);
	int nParam = atoi(args[1]);
	GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction((CActor*)m_pEntity, "GmDoAllCharge", nFlag, nParam);
	return true;
}

bool CChatSystem::GmDoGuildSiege(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 2 )
	{
		sprintf(sRetMsg,"Need 2 param at least");
		return false;
	}

	CActor *pActor = (CActor*)m_pEntity;
	int nFlag  = atoi(args[0]);
	int nParam = atoi(args[1]);
	GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction((CActor*)m_pEntity, "DoGuildSiege", nFlag, nParam);
	return true;
}

bool CChatSystem::GmDoGameStore(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 2 )
	{
		sprintf(sRetMsg,"Need 2 param at least");
		return false;
	}

	CActor *pActor = (CActor*)m_pEntity;
	int nFlag  = atoi(args[0]);
	int nParam = atoi(args[1]);
	GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction((CActor*)m_pEntity, "GmDoGameStore", nFlag, nParam);
	return true;
}

bool CChatSystem::GmGuildDepotItemReject(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 1 )
	{
		sprintf(sRetMsg,"Need 2 param at least");
		return false;
	}

	CActor *pActor = (CActor*)m_pEntity;
	int nGuildId  = atoi( args[0] );
	CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild( nGuildId );
	if (!pGuild)
	{
		return false;
	}
	pGuild->ProcDepotItemInExchangeCheck();

	return true;
}

bool CChatSystem::GmDoMaterialFB(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 2 )
	{
		sprintf(sRetMsg,"Need 2 param at least");
		return false;
	}

	CActor *pActor = (CActor*)m_pEntity;
	int nFlag  = atoi(args[0]);
	int nParam = atoi(args[1]);
	GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction((CActor*)m_pEntity, "GmDoMaterialFuben", nFlag, nParam);
	return true;
}

bool CChatSystem::GmDoOpenSevTheme(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 2 )
	{
		sprintf(sRetMsg,"Need 2 param at least");
		return false;
	}

	CActor *pActor = (CActor*)m_pEntity;
	int nFlag  = atoi(args[0]);
	int nParam = atoi(args[1]);
	GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction((CActor*)m_pEntity, "GmDoOpenSevTheme", nFlag, nParam);
	return true;
}

bool CChatSystem::GmDoDailyCharge(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 2 )
	{
		sprintf(sRetMsg,"Need 2 param at least");
		return false;
	}

	CActor *pActor = (CActor*)m_pEntity;
	int nFlag  = atoi(args[0]);
	int nParam = atoi(args[1]);
	GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction((CActor*)m_pEntity, "GmDoDailyCharge", nFlag, nParam);
	return true;
}

bool CChatSystem::GmGenAttrInSmith(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 4 )
	{
		sprintf(sRetMsg,"Need 3 param at least");
		return false;
	}

	CActor *pActor		= (CActor*)m_pEntity;
	int nSmithId		= atoi(args[0]);
	int nAttrNum		= atoi(args[1]);
	int njob			= atoi(args[2]);
	int nAttrTypeMust	= atoi(args[3]);
	RandAttrSelector::GenerateAttrsInSmith( nSmithId, nAttrNum, njob, nAttrTypeMust );
	return true;
}


bool CChatSystem::GmDoMapActor(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count < 2 )
	{
		sprintf(sRetMsg,"Need 2 param at least");
		return false;
	}
	int nType = atoi(args[0]);
	int nSceneId = atoi(args[1]);
	CFuBen * pRetFb =NULL;
	CScene * pRetScene =NULL;
	GetGlobalLogicEngine()->GetFuBenMgr()->GetFbPtrBySceneId(nSceneId,pRetFb,pRetScene);
	if (!pRetScene)
	{
		return false;
	}
	switch(nType)
	{
		case 0:
			{
				int nLevel = 0;
				if (args[2])
				{
					nLevel = atoi(args[2]);
				}
				int nCount = pRetScene->CalcAllActor(nLevel);
				sprintf(sRetMsg,"S:%d,T:%d",nSceneId,nCount);
				break;
			}
		case 1:
			{
				int nLevel = 0;
				if (args[2])
				{
					nLevel = atoi(args[2]);
				}
				int nCount = pRetScene->CloseSceneAllActor(nLevel);
				sprintf(sRetMsg,"S:%d,T:%d",nSceneId,nCount);
				break;
			}
		default:break;
	}
	return true;
}


bool CChatSystem::GmDoGiveAward(char **args, INT_PTR nArgsCount, char * sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count != 3)
	{
		sprintf(sRetMsg,"Need 3 param");
		return false;
	}

	int nType	= atoi(args[0]);
	int nId		= atoi(args[1]);
	int nValue	= atoi(args[2]);

	CActor *pActor = (CActor*)m_pEntity;
	return pActor->GiveAward(nType, nId, nValue,0,0,0,0,GameLog::Log_BackStage);
}
bool CChatSystem::GmReloadVSPDef(char ** args, INT_PTR nArgsCount, char * sRetMsg)
{
	GetLogicServer()->GetVSPDefine().LoadDefinitions(_T("data/VSPDef.txt"));
	return true;
}

bool CChatSystem::GmSetFcmStatus(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count != 1 )
	{
		sprintf(sRetMsg,"Need 1 param ");
		return false;
	}
	BYTE nFlag  = (BYTE)atoi(args[0]);
	CActor *pActor = (CActor*)m_pEntity;
	return pActor->GetMiscSystem().ReInitialFCM(nFlag);
}

bool CChatSystem::GMChgJob(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count != 2)
	{
		sprintf(sRetMsg,"Need 2 param");
		return false;
	}

	int nVoc	= atoi(args[0]);
	int nSex	= atoi(args[1]);
	CActor *pActor = (CActor*)m_pEntity;
	pActor->UseChangeVocationCard(nVoc, nSex);
	return true;
}

bool CChatSystem::GMADDLootPet(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count != 1)
	{
		sprintf(sRetMsg,"Need 1 param");
		return false;
	}

	int nPetId	= atoi(args[0]);
	CActor *pActor = (CActor*)m_pEntity;
	pActor->GetLootPetSystem().addLootPet(nPetId);
}

bool CChatSystem::GmMakeRealItem(char **args, INT_PTR nArgsCount,char * sRetMsg)
{
	INT_PTR args_count = GetArgsCount(args, nArgsCount);
	if (args_count != 4)
	{
		sprintf(sRetMsg,"Need 3 param");
		return false;
	}

	int nInput	= atoi(args[0]);
	int nNum	= atoi(args[1]);
	int nSceneId		= atoi(args[2]);
	int nMonsterId		= atoi(args[3]);
	const CStdItem* pItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(nInput);
	if (!pItem)
	{
		sprintf(sRetMsg,"Need 1 param ");
		return false;
	}
	int nID = pItem->m_nIndex;

	bool result = AddItem(nID, __max(1, nNum), 0,0,0,CStdItem::iqKillMonster,nSceneId,nMonsterId,sRetMsg);
	return result;

}

//------------------------反加速外挂相关-----end--------------------------


bool CChatSystem::GmOpenBallFanStore(char **args, INT_PTR nArgsCount, char *sRetMsg)
{
	/*
	if(!IsArgsValid(args, nArgsCount, sRetMsg, 1))return false;
	return GetGlobalLogicEngine()->GetMiscMgr().OpenBallFanStore(atoi(args[0]), args[1], args[2] ? atoi(args[2]) : 0);	
	*/
	return true;
}
