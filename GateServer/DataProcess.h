#ifndef	_DATAPROCESS_H_
#define	_DATAPROCESS_H_


#define	RUNDATA_MAX_SENDTHREAD	8
#define MAX_SEND_BUFF_SIZE 8192 //最大的发送缓从的大小

class CRunDataProcesser;
using namespace wylib::container;


//数据发送线程
typedef struct tagRunDataSendThread
{
#ifdef _MSC_VER
	HANDLE				hThread;
#else
	pthread_t			hThread;
#endif
	int					nThreadIdx;
	DWORD				dwThreadId;
	TICKCOUNT			dwProcTick;
	TICKCOUNT			dwSleepTick;
	bool				boSendEWouldBlock;
	bool				boSendFewBuffer;
	BYTE				btRcvs[2];
	CRunDataProcesser	*pRunData;
	CRITICAL_SECTION	SendQueueLock;
	CBaseList<LPVOID>	*pSendAppendList;
	CBaseList<LPVOID>	*pSendProcList;
}RUNDATASENDTHREAD, *PRUNDATASENDTHREAD;

//网关数据处理类型
typedef enum tagDataProcesserType
{
	dp_Default	  = 0,
	dp_GameEngine = 1
}DATAPROCESSERTYPE;

class CRunSockProcesser;
class CRunDataProcesser : public CCustomGateClient
	//: public CCustomClientSocket
{
public:
	//typedef CCustomClientSocket	Inherited;
	typedef CCustomGateClient Inherited;
	static int s_nIgnoreDataPacket;
private:
	typedef struct tagClientRecvBuf
	{
		PRUNGATEUSERSESSION	pSession;
		int					nBufferSize;
		ULONG_PTR			nVerify;
	}CLIENTRECVBUF, *PCLIENTRECVBUF,
	 SENDCLIENTBUF, *PSENDCLIENTBUF;
	static const int SIZE_OPENUSER	= -1;
	static const int SIZE_CLOSEUSER = -2;
	
private:
	int						m_nActiveUser;			//活动连接数
	BOOL					m_boStoping;			//是否正在停止服务
	BOOL					m_boStarted;			//是否已经启动服务
	
	//HANDLE				m_hPipe;				//与服务器通信管道
	//CHAR					m_sPipeName[MAX_PATH];	//管道名称
	CHAR					m_sGateName[64];		//网关名称


	int						m_nSendThreadCount;		//发送线程数量
	DATAPROCESSERTYPE		m_DataProcessType;

#ifdef _MSC_VER
	HANDLE					m_hProcRecvThread;
#else
	pthread_t				m_hProcRecvThread;
#endif
	
	RUNDATASENDTHREAD		m_SendThreads[RUNDATA_MAX_SENDTHREAD];

	RTL_CRITICAL_SECTION	m_SessionLock;
	int						m_nMaxSessionCount;
	PRUNGATEUSERSESSION		m_Sessions;				//用户会话队列

	RTL_CRITICAL_SECTION	m_RecvQueueLock;
	CBaseList<LPVOID>		m_RecvQueue[2];
	CBaseList<LPVOID>	   *m_pRecvAppendList;
	CBaseList<LPVOID>	   *m_pRecvProcList;

	RUNGATEDATABUF			m_ServerBuf;

	long long				m_nUserVerify;
	BOOL					m_boPrintC2SMsg;//是否打印客户端向服务器发送的数据包
	BOOL					m_boPrintS2CMsg;//是否打印服务器向客户端发送的数据包
	//CCustomClientSocket		m_pServerRcvSocket; //接受服务器的数据的socket
	//CNetworkDataHandler              m_DataPacker; //数据接包器
	CRunSockProcesser*		m_pSockProcesser;	// Sock用户处理
	int				m_nCheckTimeCount;
	long long		m_llOccuTimeDiff;
	long long		m_llSvrTimeDiff; // 服务器与本客户端的时间差
	long long		m_llSendKeepAliveTime; // 网关发送心跳包给服务器的时间

#ifdef _MSC_VER
	static VOID STDCALL RecvDataProcessRoutine(void *pRunData);
	static VOID STDCALL SendDataProcessRoutine(void *pRunThread);
	//static VOID STDCALL RecvServerProcessRoutine(void *pRunData);
#else
	static VOID* RecvDataProcessRoutine(void *pRunData);
	static VOID* SendDataProcessRoutine(void *pRunThread);
	//static VOID* RecvServerProcessRoutine(void *pRunData);
#endif

	VOID InitSessions();
	VOID UninitSessions();
	VOID FreeRecvBuffers();
	VOID FreeSendBuffers(PRUNDATASENDTHREAD pSendThread);

	BOOL CopyWaitSendBuffers(PRUNDATASENDTHREAD pSendThread, BOOL boForceCopy);
	

protected:
	VOID GetSessionLoginInfo(PRUNGATEUSERSESSION pSession, char *sBuffer, int nBufferSize);

protected:
	VOID GotError(LPCSTR sErrFn, LPCSTR sErrApi, int nErr);
	VOID InitSendThreadData();
	VOID UninitSendThreadData();
	VOID SendCheck(int nIdent);
	VOID SendGateUserInfo();
public:
	VOID SendOpenSession(PRUNGATEUSERSESSION pSession);
	VOID SendCloseSession(PRUNGATEUSERSESSION pSession, BOOL boCloseOnServer = TRUE, INT nReason = 0);
protected:
	VOID SendKeepAliveAck(PRUNGATEUSERSESSION pSession, const char *pBuffer, SIZE_T nBufSize);
	VOID CloseAllSessions(BOOL boForceClose);
	VOID CancelRemainSendSessionBuffers();
	BOOL CheckCloseSessions(BOOL boForceCheck);
	VOID CheckSendSessionBuffers(PRUNDATASENDTHREAD pSendThread);
	
	//websocket握手
	VOID ShakeHandsHandle(PRUNGATEUSERSESSION pSession, char* pBuffer, int BufferSize);

	//发送消息到服务器
	VOID SendServerMessage(int nIdent, int nSessionIdx, SOCKET nSocket, int nServerIdx, char *pBuffer, int nBufSize);
	
	//发送数据到客户端
	VOID PostUserServerData(PRUNGATEUSERSESSION pSession, const char *pBuffer, int nBufferSize, BOOL boWriteWsHdr=TRUE, BOOL boWriteProtoHdr=FALSE,  TICKCOUNT nSendTime = 0);

	//处理客户端发过来的数据
	VOID ProcessUserRecvPacket(PRUNGATEUSERSESSION pSession, char *pBuffer, int nBufferSize);
	
	//重载CustomWorkSocket的函数,处理服务器的数据数据,将调用ProcessServerPacket
	virtual void ProcessRecvBuffers(const GATEMSGHDR* pHeader, LPCSTR pData, size_t nLen); 
	VOID DispathRecvMessage(const PGATEMSGHDR pMsgHdr, const char *pBuffer, SIZE_T nBufSize);
	
	VOID OnConnected();
	virtual VOID OnClosed();

	VOID OnError(INT errorCode);
	VOID OnDisconnected();

	
public:
	CRunDataProcesser(const int nMaxSessionCount);
	~CRunDataProcesser();


	
	SIZE_T	m_dwProcessRecvSize;		//处理客户端接收数据大小
	SIZE_T	m_dwWaitSendUserSize;		//待发送给用户的数据包大小
	SIZE_T	m_dwWaitSendQueueSize;		//待发给用户的数据包队列大小
	SIZE_T	m_dwRecvSeverSize;			//接收服务器数据大小
	SIZE_T	m_dwSendUserSize;			//发送用户数据大小
	TICKCOUNT	m_dwLastProcUsrMsgTime;		//上次处理用户数据时间
	TICKCOUNT	m_dwLastProcSrvMsgTime;		//上次处理服务器数据时间
	TICKCOUNT	m_dwLastRecvSrvMsgTime;		//上次接收服务器数据时间
	TICKCOUNT	m_dwProcSrvThreadSleep;		//处理服务器数据线程休眠时间
	SIZE_T	m_dwSendQueueSize;			//待发数据队列数据包大小

	INT		GetActiveUserCount();
	inline LPCSTR GetName(){ return m_sGateName; }
	VOID	SetName(LPCSTR sName);
	//VOID	SetPipeName(LPCSTR sPipeName);
	INT		GetSendThreadCount();
	VOID	SetSendThreadCount(INT ThreadCount);
	DATAPROCESSERTYPE GetDataProcessType();
	VOID	SetDataProcessType(DATAPROCESSERTYPE Type);
	INT		GetMaxSessionCount();
	VOID	SetRunSockProcesser(CRunSockProcesser* pSockProcesser);
	CRunSockProcesser* GetRunSockProcesser() const { return m_pSockProcesser; }
	PRUNDATASENDTHREAD	GetSendThreadInfo(INT ThreadIdx);
	PRUNGATEUSERSESSION	GetFirstSession();
	VOID	SetPrintC2SMessage(const BOOL boPrint);
	VOID	SetPrintS2CMessage(const BOOL boPrint);

	BOOL Startup();
	VOID Stop();

	PRUNGATEUSERSESSION NewSession(SOCKET nSocket, SOCKADDRIN RemoteAddr);
	VOID AddRecvBuf(PRUNGATEUSERSESSION pSession, unsigned long long nVerify, char *pBuffer, int BufferSize);

	static VOID  ShowGateStatus(CRunDataProcesser *pRunData);
	static int ChangeConsoleLine(int nLine, int nPos, LPCSTR sFormat, ...);	
	static unsigned int s_nSndThreadSleepTime; // 发送线程Sleep时间（毫秒为单位）

};

#endif
