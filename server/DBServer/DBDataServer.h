#ifndef _DB_DATA_SERVER_H_
#define _DB_DATA_SERVER_H_
#include <map>
class CJobZyCountMgr;
class CDBServer;
class CDBDataClient;


//内部消息号
typedef enum tagDbServerInterMsg
{
	DSIM_POST_OPEN_CHAR_LOAD = 3001,
	DSIM_POST_RENAMECHAR_RESULT,
	DSIM_POST_ALLOC_GUILID_RESULT,
	DCIM_POST_CREATE_ACTOR_RESULT,   //收到了名字服务器返回的创建角色的结构
	DCIM_POST_CREATE_CS_ACTOR_RESULT,   //收到了名字服务器返回的创建角色的结构

	DCIM_RENAMECHAR_RESULT = 4001,
	DCIM_ALLOC_GUILID_RESULT,
	DCIM_SEND_LOAD_ACTOR,
	DCIM_CREATE_ACTOR_RESULT,
	DCIM_CREATE_CS_ACTOR_RESULT,//跨服角色创建
};



class CDBDataServer
	: public CCustomServerSocket
{
public:
	typedef CCustomServerSocket Inherited;
	static const int sSaveNameInterval	= 300000;  // 保存随机名称间隔
	static const int sDumpInterval		= 300000;  // dump时间间隔
	static const int sUpdateLogicClientListInterval = 10000; // 10s更新一次
	static bool m_bGableIsClose;

public:
	CDBDataServer(CDBServer* lpDBEngine);
	~CDBDataServer();
	

	//获取所属DB引擎
	inline CDBServer* GetDBEngine(){ return m_pDBServer; }

	//获取指定ID的数据客户端数量，如果nServerIndex参数为0则返回总的已连接的数据客户端数量
	INT_PTR GetAvailableDataClientCount(const INT_PTR nServerIndex);
	//判断指定的角色是否保存数据失败了（如果角色的数据通过DataClient保存失败，则处于数据同步和安全考虑，角色将不能登录）
	BOOL IsCharSavedFailure(const INT_PTR nCharId);
	//向特定的数据服务器投递打开角色加载认证数据
	inline VOID PostOpenCharLoad(UINT64 nServerIndex, UINT64 nSessionId, UINT64 nCharId,UINT64 nCurrentTick )
	{
		PostInternalMessage(DSIM_POST_OPEN_CHAR_LOAD, nServerIndex, nSessionId, nCharId,nCurrentTick);
	}

	/*投递通过名称服务器申请帮会名称的结果
	 *pResult 使用一个包含2个INT_PTR的数组，[0]保存操作结果(jxSrvDef::ASYNC_OPERROR)，[1]保存帮会ID
	 */
	inline VOID PostAllocGuildNameResult(INT_PTR nServerIndex, PINT_PTR pResult, UINT_PTR lpCreateGuildIdent)
	{
		PostInternalMessage(DSIM_POST_ALLOC_GUILID_RESULT, nServerIndex, (UINT_PTR)pResult, lpCreateGuildIdent);
	}
	
	/* 投递通过名称服务器修改一个角色的名字的结果  */
	inline VOID PostRenameCharResult(INT_PTR nServerIndex, jxInterSrvComm::NameServerProto::NAMESERVER_OPERROR eError, UINT_PTR lpRenameCharIdent)
	{
		PostInternalMessage(DSIM_POST_RENAMECHAR_RESULT, nServerIndex, eError, lpRenameCharIdent);
	}

	inline VOID PostCreateActorResult(INT_PTR nServerIndex, int eError, unsigned int nActorId,PCREATEACTORDATA pData)
	{
		PostInternalMessage(DCIM_POST_CREATE_ACTOR_RESULT, nServerIndex, eError,nActorId,(UINT64)pData);
	}

	inline VOID PostCreateCrossServerActorResult(INT_PTR nServerIndex, int eError, unsigned int nActorId,PCREATEACTORDATA pData)
	{
		PostInternalMessage(DCIM_POST_CREATE_CS_ACTOR_RESULT, nServerIndex, eError,nActorId,(UINT64)pData);
	}


	// 检查逻辑服务器是否已经连接上来
	BOOL	getGameReady(int nSrvIdx);
	inline CSQLConenction* GetSQLConnection() { return &m_SQLConnection; }
		
	/*
	* Comments: 更新连接的逻辑客户端列表
	* @Return void:
	* @Remark:
	*/
	void UpdateLogicClientList();

	//获取玩家和阵营的人数的管理器
	inline CJobZyCountMgr *GetJobZyMgr()
	{
		return &m_jobzyMgr;
	}

	//查找内存的使用情况
	void Trace();

protected:
	//覆盖创建连接对象的函数
	CCustomServerClientSocket* CreateClientSocket(SOCKET nSocket, PSOCKADDR_IN pAddrIn);
	//覆盖销毁一个已经建立的客户端对象
	VOID DestroyClientSocket(CCustomServerClientSocket *pClientSocket);
	//覆盖父类处理客户端连接的函数
	VOID ProcessClients();
	
	//覆盖分派内部消息处理的函数
	VOID DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4);

	//覆盖当客户端连接处理线程停止前调用函数
	VOID OnSocketDataThreadStop();
	//覆盖父类例行执行函数
	VOID SingleRun();
	//覆盖父类调用例行RUN的函数
	VOID OnRun();

	//排行榜保存时间读取
	BOOL LoadRefreshRankTime();
	//排行榜保存时间
	VOID SaveRefreshRankTime();
	
	//向数据客户端追加发送数据，如果nServerIndex为0，则向所有数据客户端追加发送数据
	//lpData是编码后的数据，如果为空，则dwSize必须为0，如果boPrimary为TRUE，则数据会优先发送
	//函数返回向多少个数据客户端的发送缓冲追加了数据
	INT_PTR SendDataClientMsg(const INT_PTR nServerIndex, LPCVOID lpData, SIZE_T dwSize);
	//向数据客户端投递内部消息，如果nServerIndex为0，则向所有数据客户端投递消息
	//函数返回向多少个数据客户端投递了消息
	INT_PTR PostClientInternalMessages(UINT64 nServerIndex, UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4=0);
	
private:
	//连接到数据库
	BOOL ConnectSQL();
	//处理已经关闭的数据客户端对象
	VOID ProcessClosedClients();
	//判断是否所有数据数据均已处理完毕以便安全的停止服务
	BOOL AllDataProcessed();
	//强制释放所有客户端连接占用的内存（销毁连接对象）
	VOID FreeAllClient();
	
private:
	CDBServer*						m_pDBServer;		//所属数据引擎管理器
	CSQLConenction					m_SQLConnection;	//数据库连接对象--数据线程调用
	TICKCOUNT						m_dwReconnectSQLTick;//下次重新连接数据的时间
	TICKCOUNT						m_dwDeleteOutDataTick;//下次从数据库中删除作废数据的时间
	TICKCOUNT                       m_dwNextDumpTime;     //下次dmp的时间
	TICKCOUNT						m_dwSaveNameFile;
	CBaseList<CDBDataClient*>		m_CloseClientList;	//已经关闭的数据客户端列表
	TICKCOUNT						m_dwNextUpdateClientListTick; // 下次更新客户端列表时间
	CCSLock                         m_CloseClientLock;

	CJobZyCountMgr                  m_jobzyMgr;    //管理职业平衡以及阵营平衡的管理器
	TICKCOUNT						m_nNextZyJobSaveTime;


public://TODO 这里暂时这样，后续需要换成Get/Set API提供访问修改
	CDBDataClient*					m_pCurClient;
	std::map<unsigned int,bool>		m_ActorIdMap;
	CCSLock							m_ActorIdMapLock;
	CSQLConenction					m_FeeSQLConnection;	//充值数据库连接对象
	TICKCOUNT						m_nNextReadFeeTime;//下次读取充值指令的时间
public://TODO 这里暂时这样，后续需要换成Get/Set API提供访问修改
	CSQLConenction					m_NowSQLConnection;	//当前线程数据库对象

private:
	///*** 定义内部逻辑处理消息号  ***/
	////向特定的数据服务器投递打开角色加载认证数据（Param1=服务器ID，Param2=会话ID，Param3=角色ID）
	//static const UINT DSIM_POST_OPEN_CHAR_LOAD		= 3001;
	////名称客户端向指定的服务器返回重命名角色的结果(Param1=服务器ID,Param2=操作结果,Param3=申请更名操作时传递的操作唯一标识)
	//static const UINT DSIM_POST_RENAMECHAR_RESULT	= 3002;	
	///*名称客户端向指定的服务器返回申请帮会ID的结果
	//	Param1=服务器ID,
	//	Param2=（2个INT的数组，[0]保存操作结果，[1]保存帮会ID，处理消息后必须对数组进行free释放）,
	//	Param3=申请帮会ID操作时传递的操作唯一标识
	//*/
	//static const UINT DSIM_POST_ALLOC_GUILID_RESULT	= 3003;	

};

#endif

