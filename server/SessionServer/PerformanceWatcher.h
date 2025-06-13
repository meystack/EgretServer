#pragma once


/*
class CSSManager;
class CPerformanceWnd;
class CServerConnectionWnd;

class CPerformanceWatcher
	: public wylib::thread::CBaseThread
{
public:
	typedef CBaseThread Inherited;
	struct ServerConnetionInfo
	{
		CHAR		sSrvName[64];
		SOCKADDR_IN s_Addr;
		TICKCOUNT	dwConnectTick;
	};
	typedef CBaseList<ServerConnetionInfo> CServerConnectionList;

private:
	CSSManager		*m_pSSManager;
	CPerformanceWnd	*m_pPerformanceWnd;
	CServerConnectionWnd *m_pServerConnectionWnd;

protected:
	void OnRountine();
	
private:
	void ShowPerformance();
	void ShowServerConnections();
	static UINT STDCALL EnumServerConnectionCallBack(CServerConnectionList *, CSessionClient *);
public:
	CPerformanceWatcher(CSSManager *lpSSManager);
	~CPerformanceWatcher(void);

	VOID ShowPerformanceWindow(const BOOL boShow);
	VOID ShowServerConnectionWindow(const BOOL boShow);
};
*/



