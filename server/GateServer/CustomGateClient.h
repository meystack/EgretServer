#pragma once



// 网关客户端类，负责处理和服务器数据包的接收和发送
// 采取阻塞方式实现
class CCustomGateClient : protected CSendPacketPool
{
public:
	typedef struct tagGateMsg
	{
		GATEMSGHDR		header;
		int				nBuffLen;			// 数据buff的长度
		char*			data;
	}GateMsg;


	// ctor and dtor
	CCustomGateClient();
	virtual ~CCustomGateClient();

	// 启动客户端
	bool Startup();
	// 停止客户端
	void Stop();
	// 判读是否连接上服务器
	bool connected() const { return m_bConnected; }
	// 增加发送数据
	void PutMessage(int nIdent, 
					int nSessionIdx, 
					SOCKET nSocket, 
					int nServerIdx, 
					unsigned long long param, 
					char *pBuffer, 
					int nBufSize);

	// 获取客户端名称
	inline LPCTSTR GetClientName() const { return m_szClientName; };
	// 设置客户端名称
	void SetClientName(LPCTSTR sClientName);
	// 获取远程服务器地址
	inline LPCTSTR GetServerHost() const { return m_szSrvHost; }
	// 设置远程服务器地址（支持域名）
	void SetServerHost(LPCTSTR sHost);
	// 获取远程服务器端口
	inline INT_PTR GetServerPort() const { return m_nSrvPort; }
	// 设置远程服务器端口
	void SetServerPort(const INT_PTR nPort);
	// Dump数据包情况
	void Dump();
	
	//覆盖例行执行函数
	void SingleRun();	

protected:	
	// 连接到服务器
	virtual void OnConnected();
	virtual void OnClosed();
	// 连接到服务器成功后发送注册客户端的消息
	virtual void SendRegisteClient();
	// 当长时间没有通信后发送保持连接的消息
	virtual void SendKeepAlive();
	// 接收到网关数据
	//virtual void ProcessRecvBuffers(const GateMsg* msgData) = 0;
	virtual void ProcessRecvBuffers(const GATEMSGHDR* pHeader, LPCSTR pData, size_t nLen) = 0;
	// 关闭套接字
	void Close();
	// 创建套接字
	bool CreateSocket();
	// 连接到服务器
	bool ConnectToServerImpl();	
	// 发送指定长度的数据
	bool SendPointedData(const char* pData, size_t nLen);
	bool ReadPointedData(char* pOutBuff, size_t nLen);
	// 分配发送给服务器的数据包
	CDataPacket* AllocPacket();
	// 分配接收到的数据包
	CDataPacket* AllocRecvPacket();

private:
	// 初始化套接字库
	bool InitSockLib();
	// 卸载套接字库
	void UnintSocketLib();
	// 工作线程函数
#ifdef _MSC_VER
	static DWORD WINAPI staticConnectThreadRountine(LPVOID pParam);
#else
	static void* staticConnectThreadRountine(void* pParam);
#endif
	
	void ConnectToServerRoutine();
	// 发送数据线程
#ifdef _MSC_VER
	static DWORD WINAPI staticSendDataThreadRountine(LPVOID pParam);
#else
	static void* staticSendDataThreadRountine(void* pParam);
#endif
	void SendDataRoutine();
	// 接收数据线程
#ifdef _MSC_VER
	static DWORD WINAPI staticRecvDataThreadRountine(LPVOID pParam);
#else
	static void* staticRecvDataThreadRountine(void* pParam);
#endif
	void RecvDataRoutine();
	void ProcessRecvDataImpl();
	void DestroyPacketList(CQueueList<CDataPacket*>& pkgList);
private:
	SOCKET					m_socket;
	TCHAR					m_szSrvHost[256];		// 服务器地址
	INT_PTR					m_nSrvPort;				// 服务器端口
	TCHAR					m_szClientName[256];	// 客户端名称
	bool					m_bConnected;			// 是否已经连接上
	volatile LONG			m_bStoped;				// 是否标记为停止
#ifdef _MSC_VER
	HANDLE					m_hConnectThread;		// 连接服务器线程
	HANDLE					m_hSendDataThread;		// 发送数据线程
	HANDLE					m_hRecvDataThread;		// 接收数据线程
#else
	pthread_t				m_hConnectThread;
	pthread_t				m_hSendDataThread;
	pthread_t				m_hRecvDataThread;
#endif
	CQueueList<CDataPacket*>m_SendMsgList;			// 发送消息队列
	CCSLock					m_SendMsgLock;			// 发送消息锁	
	CQueueList<CDataPacket*>m_FreeMsgList;			// 空闲消息队列
	CCSLock					m_FreeMsgLock;			// 空闲消息锁	
	GateMsg					m_RecvMsg;				// 当前接受的消息数据
	CBufferAllocator		m_Allocator;			// 内存块管理器
	// 接收数据不直接派发上去，而是经过这个缓冲，保证接收数据的及时性
	CQueueList<CDataPacket*>m_RecvMsgList;			// 接受消息队列
	CCSLock					m_RecvMsgLock;			// 接受消息锁
	CQueueList<CDataPacket*>m_RecvFreeMsgList;		// 空闲消息队列
	CCSLock					m_RecvFreeMsgLock;		// 空闲消息锁	
	CDataPacket				m_SendBigPacket;		// 发送给服务器的大数据包缓存

	int nPacketCount;
};
