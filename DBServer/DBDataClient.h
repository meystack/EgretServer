#ifndef _DB_DATA_CLIENT_H_
#define _DB_DATA_CLIENT_H_

/*****
★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★
                             ●重要说明 ●

	在2007年开发的c版的dbserver中，如果保存数据失败则会将SQL语句转储到
内存中并在退出的时候保存到文件，启动再加载进内存。同时当客户端选择角色
的时候判断角色ID是否在转储列表中，如果存在则提示保护数据安全并禁止进入
游戏。
	目前游戏引擎具有保存角色数据失败转储文件到内存以及硬盘的功能并可在
启动时恢复数据，因此本版本的数据服务程序将不再提供保存角色失败的转储功
能。角色数据保存失败将完全依赖于游戏引擎的内存以及文件的转储机制。

★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★
*****/


class CDBDataServer;
class CDataPacket;

class CDBDataClient	: public CCustomJXServerClientSocket,
					  public ILogicDBRequestHost
{	
	friend class CDBDataServer;
	friend class CDBDataCache;
	friend class CDBDataClientHandler;
public:
	typedef CCustomJXServerClientSocket Inherited;
	typedef VOID (CDBDataClient::*OnHandleSockPacket)(CDataPacketReader &packet);
public:
	CDBDataClient(CDBDataServer *lpDataServer, CSQLConenction *lpSQLConnection, SOCKET nSocket, PSOCKADDR_IN pAddrIn);
	~CDBDataClient();

	//判断是否具有尚未处理完成对的数据
	BOOL HasRemainData();

	//////////////////////////////////////////////////////////////////////////
	// ILogicDBRequestHost	
	CDataPacket& AllocDataPacket(const jxSrvDef::INTERSRVCMD nCmd);
	void FlushDataPacket(CDataPacket &packet);	

	void Trace()
	{
		OutputMsg(rmNormal,"CDBDataClient Trace %s",getClientName());
		
		m_sDataHandler.Trace();
		OutputMsg(rmNormal,"---BufferAllocator start---");
		CBufferAllocator::ALLOCATOR_MEMORY_INFO memoryInfo;
		m_Allocator.GetMemoryInfo(&memoryInfo);
		
		OutputMsg(rmNormal,"super alloc=%d,free=%d",(int)memoryInfo.SuperBuffer.dwAllocSize,(int)memoryInfo.SuperBuffer.dwFreeSize);
		OutputMsg(rmNormal,"large alloc=%d,free=%d",(int)memoryInfo.LargeBuffer.dwAllocSize,(int)memoryInfo.LargeBuffer.dwFreeSize);
		OutputMsg(rmNormal,"middle alloc=%d,free=%d",(int)memoryInfo.MiddleBuffer.dwAllocSize,(int)memoryInfo.MiddleBuffer.dwFreeSize);
		OutputMsg(rmNormal,"small alloc=%d,free=%d",(int)memoryInfo.SmallBuffer.dwAllocSize,(int)memoryInfo.SmallBuffer.dwFreeSize);
		OutputMsg(rmNormal,"---BufferAllocator end---");
	
		
	}
protected:
	//覆盖连接断开的函数
	VOID OnDisconnected();
	/* 覆盖父类连接断开的函数以便情况注册数据，从而使得对象可以重用 */
	VOID Disconnected();

	//覆盖套接字错误的函数
	VOID OnError(INT errorCode);

	//覆盖处理内部消息的函数
	virtual VOID DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4);
	
		/*** 子类需覆盖的函数集 ***/
	/* 处理单个通信数据包 
	 * nCmd		通信消息命令
	 * inPacket	已经读取出通信命令的数据包，数据包的读取位置指向命令数据后的内容
	 */
	virtual VOID OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket);

	/* 验证客户端注册数据是否有效
	 *@return 返回true表示注册数据有效并，返回false则关闭连接
	 */
	virtual bool OnValidateRegData(const jxSrvDef::PSERVER_REGDATA pRegData);

	virtual void OnRegDataValidated();

	//处理创建帮会
	int CatchCreateGuild(int nActor,LPCSTR sGuildName,	LPCSTR sFoundName,int nIndex,Uint64	hActor,BYTE nZY,INT_PTR nIcon);
	
	int CatchRemoveGuild(int);
private:
	//大型SQL查询语句缓冲长度
	static const SIZE_T dwHugeSQLBufferSize = 1024 * 1024 * 32;

	//定义异步名称操作（角色改名和申请创建帮会时同步名称并申请ID的操作）的数据结构
	typedef union tagASyncNameOPData
	{
		//角色重命名数据
		struct
		{
			unsigned int  nCharId;		//角色ID
			char sCharName[128];//角色名称
		}	RenameChar;
		//创建帮会数据
		jxSrvDef::CREATEGUILD_DATA CreateGuild;
	}*PASYNCNAMEOPDATA;

protected:
	//清除异步名称操作数据列表
	VOID ClearASyncNameOPList();
	//从异步名称操作数据列表中提取操作数据，如果lpOPData指针存在于异步名称操作数据列表中，则删除并返回lpOPData，否则返回NULL
	PASYNCNAMEOPDATA PeekASyncNameOPData(PASYNCNAMEOPDATA lpOPData);
	//在本地数据库中重命名角色，如果操作成功则返回0，数据库查询失败则返回1，数据库重命名操作函数返回失败值时则返回2
	INT_PTR RenameCharOnDatabase(const INT_PTR nCharId, LPCSTR sNewCharName);
	//处理名称服务器对角色重命名操作的返回
	VOID ProcessRenameCharResult(PASYNCNAMEOPDATA lpOPData, const INT_PTR nRenameError);
	//处理名称服务器对申请帮会ID操作的返回
	VOID ProcessAllocGuildResult(PASYNCNAMEOPDATA lpOPData, const INT_PTR nError, INT_PTR nGuildId);

	//处理创建角色
	//VOID ProcessCreateActor( INT_PTR nError,unsigned int nActorId,PCREATEACTORDATA pData);

	//默认的
	VOID DefaultPacket(CDataPacketReader &inPacket);	


	virtual VOID OnRun();

	//VOID LoadActorCombatRank(CDataPacketReader &inPacket);
	//VOID LoadActorCombatGameInfo(CDataPacketReader &inPacket);
	//VOID UpdateActorCombatAward(CDataPacketReader &inPacket);

	//VOID SaveActorCombatGameInfo(CDataPacketReader &inPacket);

	//VOID LoadCombatRecord(CDataPacketReader &inPacket);
	//VOID SaveCombatRecordToDb(CDataPacketReader &inPacket);

	////任务的
	//VOID QueryQuestFromDB(CDataPacketReader &inPacket);
	//VOID QueryQuestFromDB(int nActorId);
	//VOID SaveQuestToDB(CDataPacketReader &inPacket, bool bResp = true);
	////int GetMainQuest(int nActorId,CDataPacket& out);
	//int SaveMainQuest(int nActorId,char* pBuf,SIZE_T nSize);
	//int GetGoingQuest(int nActorid,CDataPacket& out);
	//int SaveGoingQuest(int nActorid,int nTaskId,int id,int value);
	//int GetRepeatQuest(int nActorid,CDataPacket& out);
	//int SaveRepeatQuest(int nActorid,int nTaskId,int qtime);
	//int CleanQuestData(int nActorId);


	
private:
	//分派客户端收到的数据包的处理函数
	VOID DispatchRecvMsg(LPCSTR lpData, SIZE_T dwSize);

	//处理角色重命名
	int CatchRenameChar(LPCSTR lpData, SIZE_T dwSize);
	
	/*
	* Comments:创建一个帮派成功后，调用本函数向逻辑服务器发送结果
	* Param int nErr:错误码
	* Param int index:消息在队列中的索引
	* Param Uint64 hHandle:创建这个帮派的角色的handle，以方便向他发送结果提示语
	* Param int nGid:创建成功的话，这个是帮派的id号
	* Param LPCSTR sGuildName:帮派名称
	* Param LPCSTR sFoundName:创始人名称
	* Param BYTE nZY:阵营编号
	* @Return VOID:
	*/
	void SendAddGuildResult(int nRawServerId, 
							int nLoginServerId,
							int nErr,
						    int index,
							Uint64 hHandle,
							unsigned int nGid,
							LPCSTR sGuildName,
							LPCSTR sFoundName,
							BYTE nZY,
							INT_PTR nIcon);
	// 新的派发数据包处理函数。只是派发，不处理
	void OnDispatchRecvPacketEx(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket);
	void SetNewSQLConnection(CSQLConenction *lpSQLConnection);
private:	
	CDBDataServer*				m_pDataServer;			// 所属数据服务器
	CSQLConenction*				m_pSQLConnection;		// 数据库连接对象
	CSQLConenction*				m_pNewSQLConnection;		// 数据库连接对象
	TICKCOUNT					m_dwClosedTick;			// 连接断开的时间
	PCHAR						m_pHugeSQLBuffer;		// 大型SQL查询语句缓冲，长度为dwHugeSQLBufferSize
	CLockList<PASYNCNAMEOPDATA> m_ASyncNameOPList;		// 异步名称操作队列
	CCSLock						m_ASyncNameOPListLock;		//异步名称操作队列锁	
	CDBDataClientHandler		m_sDataHandler;			// DB请求处理类	
	CGateDBReqestHandler        m_loginHandler;         //登陆的消息
};


#endif

