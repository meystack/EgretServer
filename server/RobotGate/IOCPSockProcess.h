#ifndef	_IOCP_SOCK_PROCESS_H_
#define	_IOCP_SOCK_PROCESS_H_

//最大IO异步处理线程数量
#define	MAX_ASYNCIO_THREAD	32

#ifdef WIN32

/*	iocp 模型的RunSock	*/
class CIOCPRunSockProcesser : public CRunSockProcesser
{
	typedef CRunSockProcesser INHERITED;
private:
	HANDLE				m_hIOPort;			//完成端口句柄
	INT					m_nWorkThreadCount;	//工作线程数量,仅在Socket处理方式为IOCP时有效
	HANDLE				m_hIOWorkThreads[MAX_ASYNCIO_THREAD];
	SOCKADDRIN			m_ASyncAcceptAddr[4];
	RUNSOCKOVERLAPPED	m_ASyncAcceptOverlapped;
	SOCKET				m_NewSocket;		//用于AcceptEx的接受连接套接字

	static VOID ComplationPortWorkThreadRoutine(CIOCPRunSockProcesser *pRunSock);

	BOOL InitComplationPort();
	VOID UninitComplationPort();
	BOOL InitAcceptSocket();
	BOOL StartIOCPThreads();
	VOID StopIOCPThreads();
	VOID NewSession();
	BOOL PostAccept();
	BOOL PostRecv(PRUNGATEUSERSESSION pSession);
public:
	CIOCPRunSockProcesser();
	~CIOCPRunSockProcesser();

	INT  GetWorkThreadCount();
	VOID SetWorkThreadCount(INT ThreadCount);

	BOOL Start();
	VOID Stop();
};

#endif
#endif
