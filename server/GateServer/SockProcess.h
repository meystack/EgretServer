#ifndef	_SOCKPROCESS_H_
#define	_SOCKPROCESS_H_

typedef enum tagRunSockType
{
	rsUndefined = 0,
	rsSelect,		//select 模型
	rsIOCP,			//IOCP 模型
}RUNSOCKTYPE;

class CRunDataProcesser;

class CRunSockProcesser
{
protected:
	RUNSOCKTYPE			m_RunSockType;
	BOOL				m_boStoping;
	SOCKET				m_ListenSocket;		//监听连接的套接字句柄
	SOCKADDRIN			m_BindAddr;			//服务监听绑定地址
	CRunDataProcesser	*m_pDataProcesser;
	TICKCOUNT			m_SessionFullTick;	
	VOID GotError(LPCSTR sErrorFn, LPCSTR sErrAPI, const INT ErrorCode);

	virtual BOOL InitAcceptSocket();
	virtual VOID UninitAcceptSocket();
	virtual VOID NewSession();
	BOOL AdjustSocketSendBufSize(SOCKET s, INT nSize);
public:
	CRunSockProcesser();
	virtual ~CRunSockProcesser();

	VOID	SetBindAddress(LPCSTR sAddress);
	VOID	SetBindPort(INT nPort);
	LPCSTR	GetBindAddress();
	INT		GetBindPort();
	RUNSOCKTYPE	GetRunSockType();
	CRunDataProcesser*	GetDataProcesser();
	CRunDataProcesser*	SetDataProcesser(CRunDataProcesser *pDataProcesser);

	BOOL InitBase();
	VOID UninitBase();

	virtual BOOL Start();
	virtual VOID Stop();
};


#endif
