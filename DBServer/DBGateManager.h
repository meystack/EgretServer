#ifndef _GATEMANAGER_H_
#define	_GATEMANAGER_H_


class CDBServer;
class CDBGateManager
	: public CCustomGateManager
{
public:
	friend class CDBGate;
	typedef CCustomGateManager Inherited;
public:
	CDBGateManager(CDBServer *lpDBEngine);
	~CDBGateManager();

	//初始化
	VOID Initialize();
	//卸载
	VOID Uninitialize();
	/* 投递创建通过名称服务器申请角色名称的结果 */
	inline VOID PostAllocCharIdResult(int nSessionId, jxInterSrvComm::NameServerProto::NAMESERVER_OPERROR eError, const int nCharId)
	{
		PostInternalMessage(GTIM_ALLOC_CHARID_RESULT, nSessionId, eError, nCharId);
	}
	/* 
	* Comments: 收到逻辑发来的玩家的登陆认证数据
	* Param const unsigned int nSessionID: 账户的id
	* Param unsigned int nActorID: 角色的ID
	* @Return VOID: 
	*/
	inline VOID PostLogicResponseSessionData(const unsigned int nSessionID,unsigned int nActorID)
	{
		PostInternalMessage(LOGIC_RESPONSE_DB_SESSION_DATA, nSessionID, nActorID,0);
	}
	/* 停止网关服务器 */
	VOID Stop();

	// 获取优选职业（当前最少的职业）
	int GetPriorityJob(int nSrvIdx);
	// 获取可以选择的阵营（阵营人数低于35%的才能选择），返回按位的掩码
	int GetOptionalZy(int nSrvIdx);
	// 获取优选阵营（当前最少的阵营）
	tagZhenying GetPriorityZY(int nSrvIdx);

	inline CSQLConenction* GetSQLConnectionPtr() { return &m_SQLConnection; }

protected:
	/* 返回异步查询网关中是否存在指定会话ID的用户的结果 */
	VOID OnResultGateUserExists(int nSessionId, const BOOL boOnline);
	//进行内部消息的处理分派
	virtual VOID DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4);
	VOID OnRun();

private:
	BOOL ConnectSQL();
	/* 处理名称服务器返回申请角色ID的结果
	 * nError	错误代码，0表示成功
	 * nCharId	当错误代码为0时有效，表示名称服务器分配的角色ID
	 */
	VOID AllocCharIdResult(const int nSessionId, const INT_PTR nError, const int nCharId);

	//逻辑收到玩家的验证的数据
	VOID LogicResponseSessionData(const unsigned int nSessionId,unsigned int nActorId);
	// DB连接成功（初始化阵营和职业数据）
	VOID OnDBConnected();
	// DB服务器启动的时候自动加载玩家阵营分布数据
	VOID LoadZyInitData();
	// 保存玩家阵营分布数据（定时保存）
	VOID SaveZyData();
	
	// 新玩家创号
	void OnNewPlayerCreated(int nSrvIdx, tagZhenying zy, tagActorVocation voc);	
	// DB服务器启动的时候自动加载玩家职业分布数据
	VOID LoadJobInitData();
	// 保存玩家职业分布数据（定时保存）
	VOID SaveJobData();
	
	

private:
	CDBGate				m_Gates[MaxGateCount];	//网关列表
	CDBServer*			m_pDBServer;			//所属DBEngine
	CSQLConenction		m_SQLConnection;		//网关数据库连接对象
	TICKCOUNT			m_dwReconnectSQLTick;	//下次重新连接数据库的时间

	// 用于创建角色选择阵营和职业	
	bool							m_bLoadZyJobData;// 是否已经加载了初始的阵营、职业分布数据（针对DB断开重连的情况）
	wylib::container::CBaseList<PlayerZYDistribute>		m_vecZyDist;	// 玩家阵营分布
	wylib::container::CBaseList<PlayerJobDistribute>	m_vecJobDist;	// 玩家职业分布
	bool							m_bZyDataDirty;
	bool							m_bJobDataDirty;
	CCSLock							m_zyLock;
	CCSLock							m_jobLock;
	TICKCOUNT						m_nNextZyJobSaveTime;
private:
	//申请角色ID返回（此消息由NSClient发送）(Param1=会话ID,Param2=操作结果,Param3=角色ID（仅在操作成功时有效）)
	static const UINT_PTR GTIM_ALLOC_CHARID_RESULT = 10001;
	static const UINT_PTR LOGIC_RESPONSE_DB_SESSION_DATA = 10002; //逻辑服务器返回收到了玩家的登陆的会话数据

};


#endif
