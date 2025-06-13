#ifndef	_SELECT_SOCK_PROCESS_H_
#define	_SELECT_SOCK_PROCESS_H_

/*	select 模型的RunSock	*/
class CSelectRunSockProcesser : public CRunSockProcesser
{
	typedef CRunSockProcesser INHERITED;
private:
#ifdef _MSC_VER
	HANDLE				m_hSelectThread;
	HANDLE				m_hAcceptThread;
#else
	pthread_t				m_hSelectThread;
	pthread_t				m_hAcceptThread;
#endif
	
#ifdef _MSC_VER
	static VOID SelectThreadRoutine(void *pRunSock);
	static VOID AcceptThreadRoutine(void *pRunSock);
#else
	static void* SelectThreadRoutine(void *pRunSock);
	static void* AcceptThreadRoutine(void *pRunSock);
#endif

	INT  SelectSessions(PRUNGATEUSERSESSION pSession, INT nMax);

	BOOL StartSelectThreads();
	VOID StopSelectThreads();
public:
	CSelectRunSockProcesser();
	~CSelectRunSockProcesser();

	BOOL Start();
	VOID Stop();
};

#endif

