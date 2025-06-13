#pragma once
#include<string>
/***************************************************************/
/*
/*  聊天子系统
/*  处理玩家的聊天的数据,GM指令
/*  
/***************************************************************/
#include"../base/ChatManager.h"

class CActor;

class CChatSystem:
	public CEntitySubSystem<enChatSystemID,CChatSystem,CActor>
{
public:

	/*
	* Comments: GM指令的参数说明
	* Param char * * args: 参数字符串列表
	* Param INT_PTR nArgsCount:参数的个数
	* Param char * sRetMsg:返回给客户端的结果字符串，一般不要写，需要返回
	      提示给客户端的就可以往sRetMsg写点东西，比如在线人数等特殊情况,大小不好超过50字节
	* @Return bool:执行成功返回true，否则返回false
	*/
	typedef bool (CChatSystem::*HandleGmCommandFunc)(char **args, INT_PTR nArgsCount,char *sRetMsg); 
	
	//这里是玩家的命令的处理函数
	typedef struct tagGmHandle
	{
		char* sCommand;				// 命令的名字
		int nReqMinGmLvl;			// 此GM命令所需的最低GM等级。 GM等级越高，权限越大
		HandleGmCommandFunc func;	// 处理函数
	} GMHANDLE ,*PGMHANDLE;

	typedef CEntitySubSystem<enChatSystemID,CChatSystem,CActor> Inherid;

	/*处理网络数据
	 * nCmd 消息号
	 * packet 网络包
	 */
	void  ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet );
	
	//重载初始化函数，暂时的话所有的都是GM
	bool Initialize(void *data,SIZE_T size)
	{ 
		m_bShutup = false; 
		m_bIsTalkFree =false;
		m_nFreePostTime = 0;
		m_NextSubmitTime = 0;
		for (INT_PTR i = 0; i < ciChannelMax; i++)
			m_nNextHitTime[i] = 0;

		m_nChatHitCount = 0;
		m_nSameChatMsgCount =0;
		m_lastChatMsg[0] = '\0';
		memset(m_ForbitMap, 0, sizeof(m_ForbitMap));

		PACTORDBDATA  pActorData = (ACTORDBDATA *)data;
		if(pActorData ==NULL || size != sizeof(ACTORDBDATA)) 
		{
			OutputMsg(rmError,_T("data len error len=%d ,correct len=%d"),size, sizeof(ACTORDBDATA));
			return false;
		}
		//将db的数据拷贝过来
		m_nFreePostTime = pActorData->nShutUpTime;

		return true;
	}

	void OnEnterGame();

	void SetForbit(int nChannelId, bool value)
	{
		if (nChannelId>= 0 && nChannelId < ciChannelMax)
		{
			m_ForbitMap[nChannelId] = value;
		}
	}

	bool IsForbit(int nChannelId)
	{
		if (nChannelId>= 0 && nChannelId < ciChannelMax)
		{
			return m_ForbitMap[nChannelId] == true;
		}
		return false;
	}

	/*
	* Comments: 设置玩家禁言信息
	* Param bool bForbid: 禁言或者解禁。
	* Param INT_PTR nDuration:如果bForbid为true，这里指禁言时常，秒为单位
	* @Return void:
	*/
	void SetShutup(bool bForbid, INT_PTR nDuration);

	/*
	* Comments: 发送消息
	* Param int channel: 频道
	* Param char* outPack:消息
	* Param SIZE_T nSize:消息大小
	* @Return void:
	*/
	bool SendChatMessageByChannel(int channel, char * outPack,SIZE_T nSize);


	/* 
	* Comments: 设置玩家禁言信息
	* Param bool bForbid: 禁言或者解禁。
	* Param INT_PTR nDuration: 如果bForbid为true，这里指禁言时常，秒为单位
	* @Return void:  
	*/
	static void SetOffLineShutUp(unsigned int nActorId, bool bForbid, INT_PTR nDuration);
	
	/* 
	* Comments: 
	* Param unsigned int nActorId: 
	* Param unsigned int nFreePostTime: 
	* @Return void:  
	*/
	static void SaveShutUpData(unsigned int nActorId, unsigned int nFreePostTime);
	/*
	* Comments: 判断玩家是否被禁言
	* @Return bool: 禁言返回true；否则返回false
	*/
	bool IsShutup();

	/*
	* Comments: 获取自由发言时间
	* @Return unsigned int: 返回MiniTime
	*/
	unsigned int GetFreePostTime();

	/*
	* Comments: 保存聊天系统数据(存到脚本数据里头)
	* @Return void:
	*/
	void SaveToScriptData(PACTORDBDATA pData);
	

	/*
	* Comments: 读取聊天系统数据(从脚本数据中读取)
	* @Return void:
	*/
	void LoadFromScriptData();


	/*
	* Comments:设置发言是否免费
	* Param bool isFree: 如果为true那么将免费否则发言扩费
	* @Return void:
	*/
	inline void SetChatFree(bool isFree)
	{
		m_bIsTalkFree = isFree;
	}

	inline bool IsFreeTalk() { return m_bIsTalkFree; }

	/*
	* Comments:设置赌博系统是否开启
	* Param bool isClose:是否是开启的
	* @Return :
	*/
	static void SetGambleFlag(bool isClose);

	/*
	* Comments: 发言
	* Param int nChannleID: 聊天频道的ID定义 enum tagChannelID
	* Param const char * pMsg: 发言内容
	* Param bool boSave:是否存到日志
	* @Return void:
	*/
	bool SendChat(int nChannleID, char *msg, bool boSave = true, byte nParam = 0, std::string sendName = "", LONG64 itemId = 0);

	//保存聊天信息
	void SaveChatMsg(int ChannelId, char * userName,  char * strMsg);

	bool HaveCutChar(char* InputStr);
	bool CheckChatLen(char* msg);
private:
	

	/*
	* Comments: 处理GM命令
	* Param char * sStr:
	* @Return void:
	*/
	void ProcessGmCommand(char * sStr);

	//下面放一些GM命令

	bool GmAttr(char **args, INT_PTR nArgsCount,char * sRetMsg);

	///在身边创建一个怪物
	///@Monster 怪物名(或id) 数量(1) 存活时间(0) 等级(0) 血量比(100)
	bool GmMonster(char **args, INT_PTR nArgsCount,char * sRetMsg);

	///移动
	///@Go 地图名(或场景id) X Y 
	bool GmGo(char **args, INT_PTR nArgsCount,char * sRetMsg);

	bool GMEnterFuben(char **args, INT_PTR nArgsCount,char * sRetMsg);

	//@PR 相对位置x 相对位置y
	bool GmPR(char **args, INT_PTR nArgsCount,char * sRetMsg);

	//@PR 鼠标位置x 鼠标位置y 相对位置x 相对位置y
	bool GmAR(char **args, INT_PTR nArgsCount,char * sRetMsg);

	///学习技能
	///@Skill 技能ID 技能等级
	bool GmLearnSkill(char **args, INT_PTR nArgsCount,char * sRetMsg);

	///删除技能
	///@DelSkill 技能ID/名字
	bool GmDeleteSkill(char **args, INT_PTR nArgsCount, char *sRetMsg);

	///添加BUFF
	///@AddBuff buffId
	bool GmAddBuff(char **args, INT_PTR nArgsCount,char * sRetMsg);

	///删除BUFF
	///@DelBuff buffId
	bool GmDelBuff(char **args, INT_PTR nArgsCount,char * sRetMsg);

	///抓捕怪物
	///@CatchMonster 怪物实体id
	bool GmCatchMonster(char **args, INT_PTR nArgsCount,char * sRetMsg);

	///抛弃怪物
	///@ThrowMonster 怪物实体id
	bool GmThrowMonster(char **args, INT_PTR nArgsCount,char * sRetMsg);
	
	///掉落物品
	///@DropItem itemid 数量
	bool GmDropItem(char **args, INT_PTR nArgsCount,char * sRetMsg);

	///查询信息
	///@info xxx arg
	bool Info(char **args, INT_PTR nArgsCount,char * sRetMsg);

	bool GmSetWd(char **args, INT_PTR nArgsCount,char * sRetMsg);
	bool Drop(char **args, INT_PTR nArgsCount,char * sRetMsg);
	//GM添加金钱
	bool GmAddGold(char **args, INT_PTR nArgsCount,char * sRetMsg);
	//GM送物品
	bool GmAddPresent(char **args, INT_PTR nArgsCount,char * sRetMsg);

	//设置自己的等级
	bool GmSetSelfLevel(char **args, INT_PTR nArgsCount,char * sRetMsg);
	
	//GM删除物品
	//bool GmDelItem(char **args, INT_PTR nArgsCount,char * sRetMsg){return false;}
	
	//调整他人经验AjustExp name 1000
	bool GmAdjustExp(char **args, INT_PTR nArgsCount,char * sRetMsg);

	//自己的经验增加 Addexp 100
	bool GmAddSelfExp(char **args, INT_PTR nArgsCount,char * sRetMsg);
	
	//GM测试添加物品 @AddItem 物品名称  数量 品质 强化等级
	bool GmAddItem(char **args, INT_PTR nArgsCount,char * sRetMsg); 

	//添加数值
	///@AddValue type num
	bool AddValue(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//设定数值
	///@SetValue type num
	bool SetValue(char **args, INT_PTR nArgsCount, char *sRetMsg);
	
	//GM测试删除物品 @DelItem 物品名称  数量
	bool GmDelItem(char **args, INT_PTR nArgsCount,char * sRetMsg); 

	//制造物品 @Make 物品名称 数量   强化 强化最大值 绑定 铭刻等级 铭刻经验 物品时间
	bool GmMakeItem(char **args, INT_PTR nArgsCount,char * sRetMsg);

	//@Move 地图名称，移动到指定的地图
	bool GmMove(char **args, INT_PTR nArgsCount,char * sRetMsg);

	//@Recall 角色名称 将指定角色传送到身边
	bool GmRecall(char **args, INT_PTR nArgsCount,char * sRetMsg);
	
	//@ReGoto 角色名称 传送至指定角色身边
	bool GmReGoto(char **args, INT_PTR nArgsCount,char * sRetMsg);

	//@MoveToNPC 地图名称 NPC名称
	bool GmMoveToNPC(char **args, INT_PTR nArgsCount,char * sRetMsg);

	//@MapMove 源地图名称 目标地图名称
	bool GmMapMove(char **args, INT_PTR nArgsCount,char * sRetMsg);
	
	//@Who 在线人数
	bool GmWho(char **args, INT_PTR nArgsCount,char * sRetMsg);


	/// @AddMoney 金钱类型ID 数量
	bool GmAddMoney(char **args, INT_PTR nArgsCount,char * sRetMsg);
	//添加转生人数
	bool GMAddCircleNum(char **args, INT_PTR nArgsCount,char * sRetMsg);
	//死亡掉落
	bool GMDeathDropItem(char **args, INT_PTR nArgsCount,char * sRetMsg);
	bool GmMonthCard(char **args, INT_PTR nArgsCount,char * sRetMsg);
	bool GmDamageInfo(char **args, INT_PTR nArgsCount,char * sRetMsg);
	bool  GMSetZBPMtime(char **args, INT_PTR nArgsCount,char * sRetMsg);
	bool  GMSetZBPMdata(char **args, INT_PTR nArgsCount,char * sRetMsg);

	//竞拍
	bool GmSetAuction(char **args, INT_PTR nArgsCount,char * sRetMsg);
	bool  GmTestBackStage(char **args, INT_PTR nArgsCount,char * sRetMsg);

	//@Icon 头像ID 修改头像
	bool GmChangeIcon(char **args, INT_PTR nArgsCount,char * sRetMsg);

	//@Property  打印玩家隐藏的属性
	bool GmPrintProperty(char **args, INT_PTR nArgsCount,char * sRetMsg);

	//@Invite  要求玩家组队
	bool GmInviteTeam(char **args, INT_PTR nArgsCount,char * sRetMsg);

	//答应组队
	bool GmReplyTeam(char **args, INT_PTR nArgsCount,char * sRetMsg );

	//离开队伍
	bool GmLeaveTeam(char **args, INT_PTR nArgsCount,char * sRetMsg);

	//修改属性
	bool GmSetIntProperty(char **args, INT_PTR nArgsCount,char * sRetMsg);

	//修改属性
	bool GmSetFloatProperty(char **args, INT_PTR nArgsCount,char * sRetMsg);

	//打印Int属性值
	bool GmShowIntProperty(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//@ReloadNPC NPC的名字  刷新挂在NPC身上的脚本
	bool GmRefresh(char **args, INT_PTR nArgsCount,char * sRetMsg);
	//@RSF 重新读入全局npc的脚本
	bool GmReloadGlobalNpc(char **args, INT_PTR nArgsCount,char * sRetMsg);
	//重新载入我所在的场景的所有npc的脚本
	bool GmReloadScriptNpc(char **args, INT_PTR nArgsCount,char * sRetMsg);
	
	//测试用GM设置怪物的运动状态
	bool GmSetMonsterMove(char **args, INT_PTR nArgsCount,char * sRetMsg);

	//@targetIntPro  属性ID 属性的值
	bool GmSetTargetIntPro(char **args, INT_PTR nArgsCount,char * sRetMsg);
	//增加宠物
	bool GMADDLootPet(char **args, INT_PTR nArgsCount,char * sRetMsg);

	//设置某个任务是否完成
	bool FinishQuest(char **args, INT_PTR nArgsCount,char * sRetMsg);

	//增加一个任务
	bool Acceptrole(char **args, INT_PTR nArgsCount,char * sRetMsg);

	//超人
	bool GmSetSuperMan(char **args, INT_PTR nArgsCount,char * sRetMsg);

	//踢人
	bool GmKick(char **args, INT_PTR nArgsCount,char * sRetMsg);

	//看目标的属性
	bool GmViewTargetProperty(char **args, INT_PTR nArgsCount,char * sRetMsg);
	
	//看目标的属性
	//bool GmViewTargetFloatProperty(char **args, INT_PTR nArgsCount,char * sRetMsg);

	bool GmSkillCDSwitch(char **args, INT_PTR nArgsCount,char * sRetMsg);

	/*
	* Comments: 清空玩家背包
	* Param char * * args: 字符串参数表
	* Param INT_PTR nArgsCount:参数数量
	* Param char * sRetMsg:操作结果描述信息
	* @Return bool:成功返回true；失败返回false
	*/
	bool GmClearBag(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//重刷屏蔽字符库
	bool GmRefreshfw(char **args, INT_PTR nArgsCount, char *sRetMsg);

	/*
	* Comments: 将指定角色禁言
	* Param char * * args: 字符串参数表
	* Param INT_PTR nArgsCount:参数数量
	* Param char * sRetMsg:操作结果描述信息
	* @Return bool:成功返回true；失败返回false
	*/
	bool GmShutup(char **args, INT_PTR nArgsCount, char *sRetMsg);
	/**
	添加关键字到词库
	*/
	bool GmAddKeyword(char **args, INT_PTR nArgsCount, char *sRetMsg);
	/*
	* Comments: 将指定角色禁言状态解除
	* Param char * * args: 字符串参数表
	* Param INT_PTR nArgsCount:参数数量
	* Param char * sRetMsg:操作结果描述信息
	* @Return bool:成功返回true；失败返回false
	*/
	bool ReleaseShutup(char **args, INT_PTR nArgsCount, char *sRetMsg);


	/*
	* Comments: 查看禁言列表
	* Param char * * args: 字符串参数表
	* Param INT_PTR nArgsCount:参数数量
	* Param char * sRetMsg:操作结果描述信息
	* @Return bool:成功返回true；失败返回false
	*/
	bool ShutupList(char **args, INT_PTR nArgsCount, char *sRetMsg);
	

	/*
	* Comments: 显示好友列表
	* Param char * * args: 字符串参数表
	* Param INT_PTR nArgsCount:参数数量
	* Param char * sRetMsg:操作结果描述信息
	* @Return bool:成功返回true；失败返回false
	*/
	bool GmShowFriendList(char **args, INT_PTR nArgsCount, char *sRetMsg);


	/*
	* Comments: 系统公告消息
	* Param char * * args: 字符串参数表
	* Param INT_PTR nArgsCount:参数数量
	* Param char * sRetMsg:操作结果描述信息
	* @Return bool:成功返回true；失败返回false
	*/
	bool WorldMessage(char **args, INT_PTR nArgsCount, char *sRetMsg);


	/*
	* Comments: 给指定玩家发消息
	* Param char * * args: 字符串参数表
	* Param INT_PTR nArgsCount:参数数量
	* Param char * sRetMsg:操作结果描述信息
	* @Return bool:成功返回true；失败返回false
	*/
	bool NotifyMsg(char **args, INT_PTR nArgsCount, char *sRetMsg);
	
	
	/*
	* Comments: 设置加速的阈值
	* Param char * * args: 字符串参数表
	* Param INT_PTR nArgsCount:参数数量
	* Param char * sRetMsg:操作结果描述信息
	* @Return bool:成功返回true；失败返回false
	*/
	bool GMSetQuickRate(char **args, INT_PTR nArgsCount, char *sRetMsg);


	/*
	* Comments: 根据名字学习技能
	* Param char * * args: 字符串参数表
	* Param INT_PTR nArgsCount:参数数量
	* Param char * sRetMsg:操作结果描述信息
	* @Return bool:成功返回true；失败返回false
	*/
	bool GmLearnSkillByName(char **args, INT_PTR nArgsCount, char *sRetMsg);
	
	/*
	* Comments: 使用技能
	* Param char * * args: 字符串参数表
	* Param INT_PTR nArgsCount:参数数量
	* Param char * sRetMsg:操作结果描述信息
	* @Return bool:成功返回true；失败返回false
	*/
	bool GmUseSkill(char **args, INT_PTR nArgsCount, char *sRetMsg);
	/*
	* Comments: 根据名字遗忘技能
	* Param char * * args: 字符串参数表
	* Param INT_PTR nArgsCount:参数数量
	* Param char * sRetMsg:操作结果描述信息
	* @Return bool:成功返回true；失败返回false
	*/
	bool GmForgetSkillByName(char **args, INT_PTR nArgsCount, char *sRetMsg);

	/*
	* Comments:设置百服活动开关
	* Param char * * args:
	* Param INT_PTR nArgsCount:
	* Param char * sRetMsg:
	* @Return bool:
	*/
	bool GmSetHundredServer(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	/*
	* Comments: 修改装备属性
	* Param char * * args:
	* Param INT_PTR nArgsCount:
	* Param char * sRetMsg:
	* @Return bool:
	*/
	bool SetEquipItemProp(char **args, INT_PTR nArgsCount, char *sRetMsg);

	/*
	* Comments:  设置当天获取杀怪经验总量
	* Param char * * args:
	* Param INT_PTR nArgsCount:
	* Param char * sRetMsg:
	* @Return bool:
	*/
	bool SetExpGetToday(char **args, INT_PTR nArgsCount, char *sRetMsg);

	/*
	* Comments:增加每日进入副本的次数
	* Param char * * args:
	* Param INT_PTR nArgsCount:
	* Param char * sRetMsg:
	* @Return bool:
	*/
	bool AddFubenCount(char **args, INT_PTR nArgsCount, char *sRetMsg);

	/*
	* Comments:执行npc的脚本函数
	* Param char * * args:
	* Param INT_PTR nArgsCount:
	* Param char * sRetMsg:
	* @Return bool:
	*/
	bool CallScriptFunc(char **args, INT_PTR nArgsCount, char *sRetMsg);

	/*
	* Comments:执行npc的脚本函数(不传入实体参数)
	* Param char * * args:
	* Param INT_PTR nArgsCount:
	* Param char * sRetMsg:
	* @Return bool:
	*/
	bool CallScriptFuncNoEntity(char **args, INT_PTR nArgsCount, char *sRetMsg);

	/*
	* Comments: 设置trace级别
	* Param char * * args:
	* Param INT_PTR nArgsCount:
	* Param char * sRetMsg:
	* @Return bool:
	*/
	bool SetTraceOpt(char **args, INT_PTR nArgsCount, char *sRetMsg);

	/*
	* Comments:是否能否修改属性
	* Param INT_PTR nPropId:属性的ID
	* @Return bool: 能返回true，否则返回false
	*/
	bool CanChangeProp(INT_PTR nPropId);


	/*
	* Comments: 刷新语言包
	* Param char * * args:
	* Param INT_PTR nArgsCount:
	* Param char * sRetMsg:
	* @Return bool:
	*/
	bool ReloadLang(char **args, INT_PTR nArgsCount, char *sRetMsg);	

	bool ReloadMonster(char **args, INT_PTR nArgsCount, char *sRetMsg);
	// 重新加载物品配置数据
	bool ReloadItem(char **args, INT_PTR nArgsCount, char *sRetMsg);
	
	//调整目标的等级
	bool GmAdjustLevel(char **args, INT_PTR nArgsCount, char *sRetMsg);

	// 刷新所有场景中的怪物
	bool RefreshMonster(char **args, INT_PTR nArgsCount, char *sRetMsg);	
	// 设置头衔
	bool GmSetHeadTitle(char **args, INT_PTR nArgsCount, char *sRetMsg);
	// 刷新在线玩家头衔
	bool GmRefreshHeadTitle(char **args, INT_PTR nArgsCount, char *sRetMsg);
	//刷新排行榜
	bool GmRefreshRank(char **args, INT_PTR nArgsCount, char *sRetMsg);
	//获取聊天频道名称

	//求救
	void BroadcastSendHelp(CDataPacketReader &inPack);

	char* GetChannelName(int ChannelId);
	/*
	* Comments: 查看其他玩家的属性
	* Param INT_PTR nVehicleId:
	* Param INT_PTR nPathId:
	* @Return bool:
	* @Remark:
	*/
	bool ViewTargetProp(char **args, INT_PTR nArgsCount, char* sRetMsg);

	/*
	* Comments:判断一个字符串是否是数字
	* Param char * str: 字符串比如12211
	* @Return bool:是返回true，否则返回false
	*/
	bool IsDigit(char * str);

	//追踪玩家的数据
	bool GmTraceActor(char **args, INT_PTR nArgsCount, char *sRetMsg);
	bool GmDumpFuben(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//查看物品设置
	bool GmCheckGameSet(char **args, INT_PTR nArgsCount, char *sRetMsg);
	
	//追踪玩家的数据
	bool GmTraceSystem(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//制造一个宠物
	bool GmAddPet(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//制造一个英雄
	bool GmAddHero(char **args, INT_PTR nArgsCount, char *sRetMsg);
	//给目标的添加一个英雄
	bool GmAddTargetHero(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//设置防沉迷开启
	bool GmSetFcmOpen(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//设置防沉迷关闭
	bool GmSetFcmClose(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//通过名字重载数据
	bool GmReloadConfigByName(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//通过ID重载数据
	bool GmReloadConfigById(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//打开赌博系统
	bool GmSetGambleOpen(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//关闭赌博系统
	bool GmSetGambleClose(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//设置聊天的等级
	bool GmSetChatLevel(char **args, INT_PTR nArgsCount, char *sRetMsg);
	//设置聊天的充值金额
	bool GmSetChatRecharge(char **args, INT_PTR nArgsCount, char *sRetMsg);
	//设置聊天禁言的最大等级
	bool GmSetChatForbitLevel(char **args, INT_PTR nArgsCount, char *sRetMsg);

	// 设置db发送数据包数量
	bool GmGetDBSendPacketCount(char **args, INT_PTR nArgsCount, char *sRetMsg);
	// 设置角色保存数据间隔
	bool GmSetActorSaveInterval(char **args, INT_PTR nArgsCount, char *sRetMsg);
	// 杀死某个玩家
	bool GMKill(char **args, INT_PTR nArgsCount, char *sRetMsg);
	// 追踪玩家数据包
	bool TracePacket(char **args, INT_PTR nArgsCount, char *sRetMsg);
	//添加帮派玥石
	bool GuildAddYs(char **args, INT_PTR nArgsCount, char *sRetMsg);
	//设置帮派帮主
	bool GmSetGguildLeader(char **args, INT_PTR nArgsCount, char *sRetMsg);
	//设置认证工会
	bool GmSetGuildAuth(char **args, INT_PTR nArgsCount, char *sRetMsg);
	// 请求传送
	bool ReqTransmit(char **args, INT_PTR nArgsCount, char *sRetMsg);
	// 输出服务器信息
	bool TraceServerInfo(char **args, INT_PTR nArgsCount, char *sRetMsg);
	// 设置角色消息过滤器
	bool SetActorMsgFilter(char **args, INT_PTR nArgsCount, char *sRetMsg);
	// 测试session消息广播
	bool TestSessionBroad(char **args, INT_PTR nArgsCount, char *sRetMsg);
	// 设置是否连接跨服
	bool GmStartCommonServer(char **args, INT_PTR nArgsCount, char *sRetMsg);	
	
	//按模型刷怪
	bool GmMonster2(char **args, INT_PTR nArgsCount, char *sRetMsg);	

	//设置自己的转数
	bool GmSetActorCircle(char **args, INT_PTR nArgsCount,char * sRetMsg);
	//重置潜力点
	bool GmResetActorCirclePoint(char **args, INT_PTR nArgsCount,char * sRetMsg);

	// 删除跨服榜单 带destServerId参数指定删除某个服务器的榜单
	bool GmClearCsRank(char **args, INT_PTR nArgsCount, char *sRetMsg);
	// 广播所有逻辑服将雕像榜单数据发到session
	bool GmSaveCsRank(char **args, INT_PTR nArgsCount, char *sRetMsg);
	// 广播所有逻辑服重新取下榜单
	bool GmLoadCsRank(char **args, INT_PTR nArgsCount, char *sRetMsg);

    //清屏
    bool GmClearAllMsg(char **args, INT_PTR nArgsCount, char *sRetMsg);

    //@禁止和取消禁止
    bool GmForbidUserById(char **args, INT_PTR nArgsCount, char *sRetMsg);
    bool GmUnForbidUserById(char **args, INT_PTR nArgsCount, char *sRetMsg); 


	//设置沙巴克占领行会id
	bool GMSetSbkGuild(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//设置武林盟主
	bool GMSetWulinMaster(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//增加活跃度
	bool GmAddActivity(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//Gm创建npc
	bool GmCreateNpc(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//统计数据包
	bool GmTracePacket(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//gm设置开服时间
	bool GmSetOpenServerTime(char **args, INT_PTR nArgsCount, char *sRetMsg);


	//gm设置合服时间
	bool GmSetMergeServerTime(char **args, INT_PTR nArgsCount, char *sRetMsg);


	//gm发送守沙三天的奖励
	bool GmSendSbkDefendAward(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//gm设置加速几次掉线
	bool GmSetQuickTimes(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//设置聊天信息是否存盘
	bool GmSetChatLog(char **args, INT_PTR nArgsCount, char * sRetMsg);

	//设置人物部位掉落概率
	bool GmSetActorDropRate(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//设置本服的SP
	bool GmSetSpGuidId(char **args, INT_PTR nArgsCount, char * sRetMsg);

	//设置本服的服务器ID
	bool GmSetServerId(char **args, INT_PTR nArgsCount, char * sRetMsg);

	//从数据服加载行会信息
	bool GmLoadGuildDataFromDb(char **args, INT_PTR nArgsCount, char * sRetMsg);

	//gm设置玩家的行会
	bool GmSetActorGuild(char **args, INT_PTR nArgsCount, char * sRetMsg);

	//gm清空玩家的行会id
	bool GmDeleteActorGuild(char **args, INT_PTR nArgsCount, char * sRetMsg);

	/*
	* Comments:设置玩家武器的幸运值
	* Param char * * args:
	* Param INT_PTR nArgsCount:
	* Param char * sRetMsg:
	* @Return bool:
	*/
	bool GmSetEquipLuck(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//设置行会报名沙巴克战
	bool GmGuildSignUpSbk(char **args, INT_PTR nArgsCount, char * sRetMsg);


	//设置隐身
	bool GmSetActorHide(char **args, INT_PTR nArgsCount, char * sRetMsg);

	//设置出生点的个数
	bool GmSetActorCreatePos(char **args, INT_PTR nArgsCount, char * sRetMsg);

	//添加出生点
	bool GmAddActorCreatePos(char **args, INT_PTR nArgsCount, char * sRetMsg);

	//设置空闲玩家是否要踢下线
	bool GmSetIdlePlayer(char **args, INT_PTR nArgsCount, char * sRetMsg);

	//设置是否踢使用挂外的人下线
	bool GmOpenQuickKick(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//设置是否开户使用外挂被踢后就封帐号
	bool GmOpenQuickSeal(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//设置创号进入的场景索引
	bool GmSetEnterId(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//设置沙巴克职位
	bool GmSetSbkPos(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//节日活动设置
	bool GmSetHoliday(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//gm设置头衔
	bool GmSetLhzsTitle(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//设置Gm头衔
	bool GmSetGm(char ** args, INT_PTR nArgsCount, char * sRetMsg);


	//设置服务器的合区时间(用于后台设置开启某些系统用)
	bool GmSetServerTempCombineTime(char ** args, INT_PTR nArgsCount, char * sRetMsg);


	bool GmKillAllMonster(char ** args, INT_PTR nArgsCount, char * sRetMsg);
	bool GmKillMonster(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//刷新跨服的配置
	bool GmSetCrossConfig(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	bool GmSetCrossCommonId(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//设置跨服的id
	bool GmSetCommonServerId(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//随机增加寄售物品(测试用)
	bool GmAddConsignItem(char **args, INT_PTR nArgsCount, char *sRetMsg);
	//删除寄卖物品
	bool GmClearConsignItem(char **args, INT_PTR nArgsCount, char *sRetMsg);
	//设置排行选项
	bool GmSetRanking(char ** args, INT_PTR nArgsCount, char * sRetMsg);


	//强制的存盘操作
	bool GmSaveAllRank(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//开启修改名字
	bool GmChangeNameFlag(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//是否打开还是开启追踪
	bool GmOpenTrace(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//设置是否保存任务到日志服务器
	bool GmSetSaveQuestLogFlag(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//设置是否保存任务到日志服务器
	bool GmSetSaveCoinLogFlag(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//gm设置行会建筑的等级
	bool GmSetBuildLevel(char **args, INT_PTR nArgsCount, char *sRetMsg);

	bool GmclearActorBuildData(char **args, INT_PTR nArgsCount, char *sRetMsg);

	bool GmClearGuildBuildData(char **args, INT_PTR nArgsCount, char *sRetMsg);

	bool GmClearGuildBuildTime(char **args, INT_PTR nArgsCount, char *sRetMsg);

	bool GmChangeActorGx(char **args, INT_PTR nArgsCount, char *sRetMsg);

	bool GmTestFilter(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//设置行会资金
	bool GmSetGuildCoin(char **args, INT_PTR nArgsCount, char * sRetMsg);


	//开启检测加速外挂
	bool GmSetOpenCkSpeedFalg(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//设置检测加速外挂的误差时间
	bool GmSetDeviationTimes(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//设置校验的间隔时间
	bool GmSetCkIntevalTime(char **args, INT_PTR nArgsCount, char *sRetMsg);

	bool GmSetSpeedCheck(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//设置新检测加速外挂的参数
	bool GmSetSpeedValue(char **args, INT_PTR nArgsCount, char *sRetMsg);

	bool GmAddNewTitle(char ** args, INT_PTR nArgsCount, char * sRegMsg);
	bool GmDelNewTitle(char ** args, INT_PTR nArgsCount, char * sRegMsg);

	bool GmAddCustomTitle(char ** args, INT_PTR nArgsCount, char * sRegMsg);
	bool GmDelCustomTitle(char ** args, INT_PTR nArgsCount, char * sRegMsg);

	//踢人与禁用
	bool GmKickAndSetActorStatus(char **args, INT_PTR nArgsCount, char *sRetMsg);
	//重置武魂等级
	bool GMSetSoulLevel(char **args, INT_PTR nArgsCount, char *sRetMsg);
	/// 增加排行榜积分
	bool GMAddRankPoint(char **args, INT_PTR nArgsCount, char *sRetMsg);
	//设置成就状态
	bool SetAchieve(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//--调试脚本用,支持二个参数
	bool GmDebugLua(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//商店的GM指令
	bool GmShop(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//增加PK值的GM指令
	bool GmPkValue(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//功勋的GM指令
	bool GmExploit(char **args, INT_PTR nArgsCount, char *sRetMsg);
	
	//部位强化等级
	bool GmSetEquipPosStrongLevel(char **args, INT_PTR nArgsCount, char *sRetMsg);
	//设置行会神树
	bool GmSetGuildTree(char **args, INT_PTR nArgsCount, char *sRetMsg);
	//设置行会神树果实
	bool GmSetGuildFruit(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//设置本人行会神树
	bool GmSetMyGuildTree(char **args, INT_PTR nArgsCount, char *sRetMsg);
	//设置本人行会神树果实
	bool GmSetMyGuildFruit(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//设置行会任务
	bool GmSetGuildTask(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//强行刷新行会任务
	bool GmRefreshGuildTask(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//膜拜君主
	bool GmDoWorship(char ** args, INT_PTR nArgsCount, char * sRegMsg);

	//土城保卫战
	bool GmDoProtecteTuCity(char ** args, INT_PTR nArgsCount, char * sRegMsg);

	//赏金任务
	bool GmDoRewardQuest(char ** args, INT_PTR nArgsCount, char * sRegMsg);

	//游戏补偿
	bool GmDoCompensate(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//补给争夺
	bool GmDoSupplyBattle(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//七日豪礼
	bool GmDoSevenGoodGift(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	bool GmDoChallengeFB(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	bool GmDoAllCharge(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	bool GmDoGuildSiege(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	bool GmDoGameStore(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	bool GmGuildDepotItemReject(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	bool GmDoMaterialFB(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	bool GmDoOpenSevTheme(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	bool GmDoDailyCharge(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	bool GmDoMapActor(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	bool GmDoGiveAward(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	bool GmGenAttrInSmith(char ** args, INT_PTR nArgsCount, char * sRetMsg);
	
	bool GmReloadVSPDef(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	bool GmSetFcmStatus(char **args, INT_PTR nArgsCount, char *sRetMsg);
	
	//跨服消息请求
	void OnCustomReqCsChat(CDataPacketReader& inPack); 
	//void OnSendCsChat(CChatManager::CHATRECORD& record);
private:
	bool m_bShutup;						// 是否处于禁言状态
	bool m_bIsTalkFree;                 //发言是否是免费的

	unsigned int m_nFreePostTime;		// 解禁时间（s）
	CTimer<1000>		m_ChatTimer;	// 发言定时器
	CTimer<5000>         m_smileTimer;   //发表情的时间间隔

	int					m_NextSubmitTime;	// 下次提交建议的时间

	TICKCOUNT			m_nNextHitTime[ciChannelMax];	// 每个频道下次发言时间
	
	static const GMHANDLE		GmCommandHandlers[];	//GM的处理函数列表

	/*
	* Comments: GM添加物品都调用这个功能
	* Param INT_PTR nItemID:		物品的ID
	* Param INT_PTR nCount:			物品的数量
	* Param INT_PTR nStar:			物品的强化星级
	* Param INT_PTR nLostStar:		物品强化损失星级
	* Param INT_PTR nBind			绑定
	* Param INT_PTR nInscriptLv		铭刻等级
	* Param INT_PTR nAreaId	来源场景
	* Param INT_PTR wMonsterId	怪物id
	* Param char * sRetMsg:			返回信息
	* @Return bool: 成功返回true，否则返回false
	*/
	bool AddItem(INT_PTR nItemID, INT_PTR nCount, INT_PTR nStar=0, INT_PTR nLostStar=0,INT_PTR nBind=0, INT_PTR nInscriptLv =0, INT_PTR nAreaId=0, WORD wMonsterId = 0, char * sRetMsg =""); 
	
	
	/*
	* Comments: 检测参数列表是否正确
	* Param char * * args: 参数的指针
	* Param INT_PTR nArgsCount:参数的格式
	* Param char * sRetMsg:返回的信息指针
	* Param INT_PTR nValidArgsCount: 正确的参数的个数
	* @Return bool:
	*/
	inline bool IsArgsValid(char **args, INT_PTR nArgsCount,char * sRetMsg,INT_PTR nValidArgsCount)
	{
		if(nValidArgsCount  > nArgsCount)
		{
			sprintf(sRetMsg,"Need %d Params,%d Params input",nValidArgsCount,nArgsCount);
			return false ;//参数错误
		}
		for(INT_PTR i =0; i< nValidArgsCount; i++)
		{
			if(args[i] ==NULL)
			{
				sprintf(sRetMsg,"args[%d] =NULL error",i);
				return false;
			}
		}
		return true;
	}

	inline INT_PTR GetArgsCount(char **args, INT_PTR nArgsCount)
	{		
		for (INT_PTR i = 0; i < nArgsCount; i++)
		{
			if (args[i] == NULL)
			{
				return i;
			}
		}

		return nArgsCount;
	}

	inline void InitParamList(char **args, INT_PTR argc, wylib::container::CBaseList<wylib::string::CAnsiString*> &result_list, INT_PTR reqParamCount)
	{
		result_list.reserve(reqParamCount);	
		argc = __min(argc, reqParamCount);
		for (INT_PTR i = 0; i < argc; i++)
		{
			result_list.push(new wylib::string::CAnsiString(args[i]));
		}
		for (INT_PTR i = argc; i < reqParamCount; i++)
		{
			result_list.push(new wylib::string::CAnsiString(""));
		}
	}

	inline void SafeReleaseParamList(wylib::container::CBaseList<wylib::string::CAnsiString*> &result_list)
	{		
		for (int i = 0; i < result_list.count(); i++)
			delete result_list.get(i);
	}

	//查看属性
	bool GmViewProperty(char * sRetMsg,int nPropID, CEntity * pEntity);

	bool GMChgJob(char **args, INT_PTR nArgsCount,char * sRetMsg);
		//
	bool GmMakeRealItem(char **args, INT_PTR nArgsCount,char * sRetMsg);
	/*
	* Comments: 检测聊天频道CD
	* Param tagChannelID cid: 聊天频道id
	* @Return bool: 如果该聊天频道还在CD中返回true；否则返回false
	* @Remark:
	*/
	bool CheckChannelCD(tagChannelID cid);
	
	/*
	* Comments: 检测聊天限制
	* Param tagChannelID cid: 聊天频道Id
	* @Return bool: 如果符合聊天等级限制，返回true；否则返回false
	* @Remark:
	*/
	bool CheckLimit(tagChannelID cid);

	//清空boss成长等级数据
	bool GmClearBossHard(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//设置交易额度
	bool GMSetTradingQuota(char **args, INT_PTR nArgsCount, char *sRetMsg);

	bool GMSetBlessValue( char **args, INT_PTR nArgsCount, char * sRetMsg );

	bool GMSetQuestState( char **args, INT_PTR nArgsCount, char * sRetMsg );

	bool GMClearQuestlog( char **args, INT_PTR nArgsCount, char * sRetMsg );

	//设置经脉等级
	bool GMSetJMLv(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//通过npcID创建npc
	bool GmCreateNpcById(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//发送邮件
	bool GmAddMail(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//发送全服邮件
	bool GmAddServerMail(char ** args, INT_PTR nArgsCount, char * sRetMsg);
	//发送邮件
	bool GmAddOneMail(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//加载npc配置
	bool GmLoadNpcFile(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//加载任务的npc文件
	bool GmLoadQuestFile(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//统计所有地图刷出怪物数量
	bool GmTotalAllSceneMonsterCount(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//获取当前所在地图信息
	bool GmGetCurrSceneInfo(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//清除当前世界税收或者已领税收
	bool GmSetTaxing(char ** args, INT_PTR nArgsCount, char * sRetMsg);

	//设置玩家脚本key
	bool GmSetRundomKey(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//无条件删除行会
	bool GmDeleteGuild(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//重置世界等级为0 
	bool GmResetWorldLevel(char **args, INT_PTR nArgsCount, char *sRetMsg);
	//重新从数据库读世界等级
	bool GmReLoadWorldLevel(char **args, INT_PTR nArgsCount, char *sRetMsg);
	//
	bool GmSetFootPrintPay(char **args, INT_PTR nArgsCount, char *sRetMsg);
	bool GmSetActorCallScript(char **args, INT_PTR nArgsCount, char *sRetMsg);

	// 检测禁言
	void CheckAutoSilencing(const char* msg);
	void GetSortChatMsg(const char* msg, char* all, char* word);
	bool GmDelVar(char **args, INT_PTR nArgsCount, char *sRetMsg);

	/// 开启球迷积分兑换
	bool GmOpenBallFanStore(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//开启小黑屋
	bool GmOpenBlackHouse(char **args, INT_PTR nArgsCount, char *sRetMsg);
	//退出小黑屋
	bool GmQuitBlackHouse(char **args, INT_PTR nArgsCount, char *sRetMsg);
	//清理小黑屋数据
	bool GmCleanBlackHouse(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//副本的GM指令
	bool GmFuBen(char **args, INT_PTR nArgsCount, char *sRetMsg);
	//设置特戒培养等级
	bool GMSetRingLevel(char **args, INT_PTR nArgsCount, char *sRetMsg);
	//测试速传
	bool QuestMove(char **args, INT_PTR nArgsCount, char *sRetMsg);

	//贪玩聊天上报
	void TanWanChatReport(LPCTSTR szAccount, int nActorId, LPCTSTR szActorName, int nChannelId, LPCTSTR szMsg, LPCTSTR szSendToActorName);

	/// 聊天内容
	size_t m_nChatHitCount; // 聊天触发敏感词个数
	char  m_lastChatMsg[200];//上次聊天内容
	BYTE  m_nSameChatMsgCount;//相同聊天内容次数

	bool m_ForbitMap[ciChannelMax];//禁止聊天限制
};


