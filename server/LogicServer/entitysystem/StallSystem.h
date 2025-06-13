// #pragma once

// class CActor;

// class CStallSystem:
// 	public CEntitySubSystem<enStallSystemID,CStallSystem,CActor>
// {
// public:
// 	typedef CEntitySubSystem<enStallSystemID,CStallSystem,CActor> Inherid;
// 	CStallSystem(void);
// 	~CStallSystem(void);

// public:
// 	/*
// 	* Comments: 处理网络数据
// 	* Param INT_PTR nCmd:协议id
// 	* Param CDataPacketReader & packet:数据包
// 	* @Return void:
// 	*/
// 	void ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet );

// public:
// 	/*
// 	* Comments:开始摆摊
// 	* Param CDataPacketReader & packet:数据包
// 	* @Return void:
// 	*/
// 	void StartStall(CDataPacketReader& packet);

// 	//获取摆摊数据
// 	bool GetStallInfo(CDataPacketReader& packet);

// 	/*
// 	* Comments:结束摆摊
// 	* @Return void:
// 	*/
// 	void EndStall();

// 	/*
// 	* Comments:留言
// 	* Param CDataPacketReader & packet:数据包
// 	* @Return void:
// 	*/
// 	void LeaveMsg(CDataPacketReader & packet);

// 	/*
// 	* Comments:当有留言时，添加到摊位上
// 	* Param CStallData::STALLMSG & msg:组装好的msg
// 	* @Return bool:成功返回true
// 	*/
// 	bool OnLeaveMsg(CStallData::STALLMSG& msg, BYTE nCoin = 0, int nPrice = 0);

// 	//发送留言内容 sName店主的名字
// 	void SendLeaveMsg(CActor * pActor,ACTORNAME sName);

// 	/*
// 	* Comments:查看摊位
// 	* Param CDataPacketReader & packet:数据包
// 	* @Return void:
// 	*/
// 	void ViewStall(CDataPacketReader& packet);

// 	/*
// 	* Comments:填充摊位信息
// 	* Param CDataPacket & netPack:将要发送的数据包
// 	* @Return bool:成功返回true
// 	*/
// 	bool FillStallData(CDataPacket & netPack);

// 	/*
// 	* Comments:购买摊位上的物品
// 	* Param CDataPacketReader & packet:数据包
// 	* @Return void:
// 	*/
// 	void BuyStallItem(CDataPacketReader & packet);

// 	/*
// 	* Comments:当摊位格子扩展时
// 	* Param byte nGrid:扩展格子数
// 	* @Return bool:成功返回true
// 	*/
// 	bool OnEnlargeStallGrid(byte nGrid);

// 	/*
// 	* Comments:当有物品被买时
// 	* Param CUserItem::series:物品的guid 
// 	* @Return bool:成功返回true
// 	*/
// 	bool OnBuyItem(CUserItem::ItemSeries& series);

// 	const CStallData::STALLITEM* GetStallItemByGuid(CUserItem::ItemSeries series);

// 	/*
// 	* Comments:从摊位中查找指定物品
// 	* Param CUserItem::ItemSeries series:物品guid
// 	* @Return INT_PTR:查找成功返回索引,失败返回-1
// 	*/
// 	INT_PTR GetStallItemIdxByGuid(CUserItem::ItemSeries series);

// 	/*
// 	* Comments:对摊拉上下架操作
// 	* Param CDataPacketReader & pakcet:数据包
// 	* @Return void:
// 	*/
// 	void OptStallItem(CDataPacketReader & pakcet);

// 	/*
// 	* Comments:通知附近买家，摊位发生变化
// 	* @Return void:
// 	*/
// 	void NoticeNearBuyers(byte nMsgType);

// 	/*
// 	* Comments:当主角下线时
// 	* @Return void:
// 	*/
// 	void OnActorLogout();

// 	/*
// 	* Comments:发送广告
// 	* @Return void:
// 	*/
// 	//void SendStallAd(CDataPacketReader & packet);

// 	//玩家进入游戏的时候触发，用于发送某些数据给玩家初始化
// 	virtual void OnEnterGame();
// 	/*
// 	* Comments:封装发送摆摊结果
// 	* Param BYTE nStallResult:摆摊结果
// 	* @Return void:
// 	*/
// 	void SendStallResult(BYTE nStallResult);
// 	/*
// 	* Comments:检查玩家是否在摆摊区域
// 	* @Return void:
// 	*/
// 	void CheckStallArea(CDataPacketReader & pakcet);
// 	/* 
// 	* Comments: 是否够等级，不够弹提示
// 	* @Return bool:  true够，false不够
// 	*/
// 	bool CheckStallLevel();

// 	char * GetStallName() {return m_stallInfo.sName;}
// private:
// 	CStallData::STALLINFO m_stallInfo;					//摊位信息
// };
