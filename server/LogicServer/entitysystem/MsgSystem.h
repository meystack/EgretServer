#pragma once
/**************************************
 消息子系统，常用于用户离线后保存消息，等用户上线后再发消息通知
**************************************/

class CMsgSystem:
	public CEntitySubSystem<enMsgSystemID,CMsgSystem,CActor>
{
public:
	//离线消息类型
	//由于保存消息的内容是使用二进制，所以每一种消息类型，是有单独的数据格式，系统需要对每一种消息进行组装和解包
	enum tagMsgType
	{
		mtNoType = 0,	//无意义的消息
		mtTxtMsg = 1,	//普通的文字通知,通常是比较重要的文字通知才需要用到
		mtUnMarry = 2,		//离婚
		mtChangeMoney = 3,	//改变玩家的金钱，包括银两和元宝，绑定和非绑定
		mtJoinGuild = 4,	//加入帮派
		mtLeaveGuild = 5,	//离开帮派
		mtAddIndexItem = 6, //获得物品(present等)				
		mtGmTxtMsg = 7,	// GM 系统消息，只有文字内容，因为显示界面和mtTxtMsg不一样，所以单独一种类型
		mtSeverMaster = 8,		//与师傅绝关系
		mtExpelPupil  = 9,		//逐出师门
		mtDeleteGuild   = 13,	//帮派解散
		mtReturnSuggestMsg = 14,	//Gm给玩家直接发送离线消息
		mtGuildDepotMsg = 15,		//从仓库取出物品的离线消息
		mtAuctionItem = 16,		//领取竞拍物品
		mtGiveStoreItem = 17,	//有人赠送了商城物品
		mtPaTaMasterAward = 18,	//爬塔奖励消息
		mtCorpsBattleAward = 19,		//战队竞技给奖励
		mtCombatRankAward = 20,			//[废弃]
		mtWholeActivityAward = 21,		//全民活动奖励[废弃]
		mtGiveFirstSbkGuildAward = 22,	//给予首个占领sbk的行会首领奖励[废弃]
		mtDefendSbkThreeTimes = 23,		//给予首次守沙成功三次行会首领成功[废弃]
		mtDefendSbkSecAwards = 24,		//给予首次守沙成功三次行会的副首领奖励[废弃]
		mtGiveFirstCombineSbkAward = 25,		//给予首个合服占领沙巴克的行会首领奖励[废弃]
		mtGiveFirstCombineSecSbkAward = 26,		//给予首个合服占领沙巴克的服首领奖励[废弃]
		mtGiveCombineDefendSbkAward = 27,		//给予首个合服守沙三次的行会首领奖励[废弃]
		mtGiveCombineDefendSbkSecAward = 28,		//给予首个合服守沙三次的行会副首领奖励[废弃]
		mtGiveCombineActivityAward = 29,			//合服活动奖励
		mtHundredYestodayConsumeRank = 30,			//百服活动昨天消费排名奖励
		mtSurpriseRet = 31,							//惊喜回馈
		mtSendRedPacket = 32,						//赠送红包
		mtSendFire = 33,							//赠送鞭炮
		mtLuckAward = 34,							//春节幸运大抽奖
		mtFireTop1 = 35,							//魅力第一
		mtFestivalWithDraw = 36,					//提取反馈
		mtOldPlayerBack = 37,						//老友回归
		mtSpecialConsumeYb=39,						//特殊的消费记录
		mtCombatLog = 40,							//职业宗师被挑战战报
		//如果还有其他消息，在这里添加
		mtMessageCount //消息通知的数量

	};
public:

	virtual bool Initialize(void *data,SIZE_T size);

	//角色各子系统初始化完毕，向客户端发送创建主角的数据包后，调用该接口
	virtual void OnEnterGame();  

	//返回本玩家的消息列表
	VOID OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode, CDataPacketReader &reader);

	//继承父类接口，处理网络数据包
	virtual void ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet );

	/*
	* Comments:通知消息系统从数据库读入新加入的消息
	* @Return void:
	*/
	void ReloadFromDb(INT64 msgId);

	/*
	* Comments:增加一条新信息到列表中，要检查列表中已存在同样id的信息
	* Param MsgData & msg:
	* @Return void:
	*/
	bool AddToList(MsgData& msg);

	//寻径消息 
	void SendMoveToMsg(LPCSTR sSceneName, INT_PTR nX, INT_PTR nY, LPCSTR sTitle, LPCSTR sTips);

private:
	/*
	* Comments:处理一条消息，如果处理成功，返回true
	* Param INT64 msgId:
	* @Return bool:
	*/
	bool ProcessMsg(INT64 msgId);

	/*
	* Comments:处理一条消息，如果处理成功，返回true
	* Param MsgData & msg:
	* @Return bool:
	*/
	bool ProcessMsg(MsgData& msg);

	/*
	* Comments:发送一条删除信息的命令给数据库服务器
	* Param INT64 nMsgId:要删除的消息id，这个id是表主键，自增长
	* @Return void:
	*/
	void SendDbDeleteMsg(INT64 nMsgId);

	/*
	* Comments:向服务器发送获取自己的消息列表内容
	* Param INT64 nMsgId:如果nMsgid==0，读入所有消息，否则读入固定id的消息
	* @Return void:
	*/
	void SendDbLoadMsg(INT64 nMsgId);

	/*
	* Comments:发送消息到客户端
	* @Return void:
	*/
	void SendMsgToClient(MsgData& msg);

public:
	/*
	* Comments:给已经离线的用户发送一条信息，当用户下次登录的时候，会收到这条消息.
	* Param INT_PTR nActorId:目标角色id
	* Param INT_PTR nMsgType:消息类型
	* Param char * pMsgBuf:发送的消息内容，二进制，大小不超过MAX_MSG_COUNT
	* Param SIZE_T size: pMsgBuf的大小
	* Param INT_PTR nSrcActorId:发出这条消息的角色id，比如GM，当添加消息不成功，比如角色名称错误，需要通知GM,如果是0，表示系统发送
	* @Return bool: 返回true仅表示数据长度没问题，不代表insert到db成功
	*/
	static bool AddOfflineMsg(unsigned int nActorId,INT_PTR nMsgType,char* pMsgBuf,SIZE_T size,unsigned int nSrcActorId);

	/*
	* Comments:给已经离线的用户发送一条信息，当用户下次登录的时候，会收到这条消息.
	* Param INT_PTR nActorId:目标角色名
	* Param INT_PTR nMsgType:消息类型
	* Param char * pMsgBuf:发送的消息内容，二进制，大小不超过MAX_MSG_COUNT
	* Param SIZE_T size: pMsgBuf的大小
	* Param INT_PTR nSrcActorId:发出这条消息的角色id，比如GM，当添加消息不成功，比如角色名称错误，需要通知GM,如果是0，表示系统发送
	* @Return bool: 返回true仅表示数据长度没问题，不代表insert到db成功
	*/
	static bool AddOfflineMsgByName(LPCSTR pActorName, INT_PTR nMsgType, char *pMsgBuf, SIZE_T size,unsigned int nSrcActorId);

	/*
	* Comments:增加一条消息后数据库的返回结果
	* Param INT_PTR nCmd:
	* Param INT_PTR nErrorCode:错误代码
	* Param CDataPacketReader & reader:
	* @Return void:
	*/
	static void OnAddOfflineMsgDbReturn(INT_PTR nCmd,char * data,SIZE_T size);


	/*
	* Comments:增加一条消息后数据库的返回结果
	* Param INT_PTR nCmd:
	* Param INT_PTR nErrorCode:错误代码
	* Param CDataPacketReader & reader:
	* @Return void:
	*/
	static void OnAddOfflineMsgByNameDbReturn(INT_PTR nCmd,char * data,SIZE_T size);
	/*
	//处理赠送鞭炮和红包
	void OnGetRedPacket(INT64 nMsgId);

	//处理赠送鞭炮和红包
	bool OnGetRedPacket(MsgData& msg);

	//发送春节鞭炮、红包消息到客户端
	bool SendFireMsgInfo(INT64 nMsgId, byte nType, WORD nCount, char * sName, char * sBuff);
	*/
public:
	CMsgSystem();
	~CMsgSystem();
private:
	CVector<MsgData>				m_msgList;//所有的消息列表
};
