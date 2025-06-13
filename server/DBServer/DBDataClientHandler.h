#ifndef DB_DATA_CLIENT_HANDLER_H__
#define DB_DATA_CLIENT_HANDLER_H__

/*
	DBClient逻辑处理类。处理来自客户端(主要是逻辑服)的DB请求，这里之关系逻辑处理，不关心数据接收相关的处理。
		原来的DBClient将通信线程和DB请求逻辑处理和在一起，在逻辑服DB操作比较频繁、DB查询压力大，查询和更新DB
	比较缓慢的情况下，会导致堆积的未处理请求越来越多，延迟很慢，并且由于DBClient数据处理不过来导致逻辑服那边堆
	积的	DB请求数据包也越来越多，造成逻辑服内存消耗巨大。
		为解决上述问题，将DBClient的数据接收和处理逻辑分开。接收数据线程只关心数据接收，不处理逻辑，这样不会造
	成堆积数据包；同时逻辑处理区分优先级，不是一次性处理完所有大量玩家的存盘数据，分时处理，并且设定优先级策略，
	查询数据的优先级高于更新数据的优先级。
*/

class CDBDataClientHandler 	: 
	public wylib::thread::CBaseThread,
	public CSendPacketPool
{
public:		
	friend class CDBDataCache;

	// ctor and dtor
	CDBDataClientHandler();
	~CDBDataClientHandler();

	/*
	* Comments: 开始运行。
	* @Param ILogicDBRequestHost* pHost:
	* @Param CSQLConenction *lpSQLConnection:
	* @Return bool: 成功返回true；失败返回false
	* @Remark: 开启后台线程并且等待后台线程开始运行。
	*/
	bool Start(ILogicDBRequestHost* pHost, CSQLConenction *lpSQLConnection);

	/*
	* Comments:
	* @Return bool:
	* @Remark:
	*/
	bool Stop();

	/*
	* Comments: 添加一个DB请求的消息
	* Param jxSrvDef::INTERSRVCMD nCmd: 消息Id
	* Param CDataPacketReader & packet: 消息数据
	* @Return void:
	* @Remark: 将DB请求数据添加到队列中。这里不处理数据。供通信线程调用
	*/
	void AddMessage(jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader& packet);
	
	CLogicDBReqestHandler& GetDBRequestHandler() { return m_LogicDBReqHandler; }

	//追踪内存使用情况
	void Trace();
	
protected:
	/*
	* Comments: 设置读操作标记
	* Param BOOL flag:
	* @Return void:
	* @Remark: 如果当前有读操作未完成，flag设置为true；否则设置为false
	*/
	inline void SetReadOpFlag(BOOL flag)
	{
		InterlockedExchange(&m_bHasReadOp, flag);
	}

	/*
	* Comments: 获取读操作标记
	* @Return bool: 如果当前存在读操作，返回true，否则返回false
	* @Remark:
	*/
	BOOL HasReadOPFlag() const { return m_bHasReadOp; }

	/*
	* Comments: 判断当前是否是读操作
	* Param jxSrvDef::INTERSRVCMD nCmd:
	* @Return bool:
	* @Remark:
	*/
	bool IsReadCmd(jxSrvDef::INTERSRVCMD nCmd);

	/*
	* Comments: 线程处理函数
	* @Return void:
	* @Remark:
	*/
	virtual void OnRountine();

	/*
	* Comments: 线程逻辑处理函数
	* @Return void:
	* @Remark:
	*/
	void ThreadLogicProc();

	/*
	* Comments: DB请求逻辑处理
	* @Return void:
	* @Remark: 遍历DB请求列表并且处理。对于玩家存盘请求，直接加入到ActorCache数据里头；对于查询数据直接执行SQL查询			   
	*/
	void RunOne();

	/*
	* Comments: 分配数据包	
	* @Return void:
	* @Remark:
	*/
	CDataPacket* AllocPacket();

	/*
	* Comments: 释放用完的数据包
	* Param CDataPacket * packet:
	* @Return void:
	* @Remark:
	*/
	void FreePacket(CDataPacket* packet);

	/*
	* Comments: 销毁数据包列表
	* Param CQueueList<CDataPacket * > & pkgList:
	* @Return void:
	* @Remark: 删除对象
	*/
	void DestroyPacketList(CQueueList<CDataPacket*>& pkgList);

	/*
	* Comments: 处理DB请求消息
	* Param jxSrvDef::INTERSRVCMD nCmd: 消息码
	* Param CDataPacket & packet: 消息参数
	* @Return void:
	* @Remark:
	*/
	void HandleDBMessage(jxSrvDef::INTERSRVCMD nCmd, CDataPacket &packet);

private:	
	CQueueList<CDataPacket*>			m_sMsgList;					// DB请求消息列表（格式：消息ID+消息参数）。
	CCSLock								m_sMsgListLock;
	CQueueList<CDataPacket*>			m_sFreeList;				// 自由消息数据包
	CCSLock								m_sFreeListLock;
	//CBufferAllocator					m_allocator;				// BuffAllocator，用于DataPacket的内存分配
	CDBDataCache						m_sActorCacheData;			// 角色DB存盘Cache数据
	volatile long						m_bHasReadOp;				// 当前是否有读DB操作
	volatile long						m_lStop;					// 线程停止标记	
	volatile long						m_lHasStart;				//是否已经启动
	volatile long						m_lLogicIsRunning;			//逻辑线程是否已经启动

	ILogicDBRequestHost*				m_pHost;					//
	CCSLock								m_sStartLock;				// 线程开始启动锁
#ifdef WIN32
	HANDLE								m_hStartEvent;			//启动信号事件
	HANDLE								m_hStopEvent;			//停止信号事件
#else
	sem_t								m_hStartEvent;           //启动信号事件
	sem_t								m_hStopEvent;           //停止信号事件
#endif
	CLogicDBReqestHandler				m_LogicDBReqHandler;		// DB请求处理	

	static int							s_nDBHandleSleep;			// DB逻辑处理睡眠时间
	TICKCOUNT							 m_memoryTimer;              //定时器
};

#endif	// _DB_DATA_CLIENT_HANDLER_H__