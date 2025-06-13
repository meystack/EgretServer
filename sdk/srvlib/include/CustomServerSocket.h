#ifndef _CUSTOM_SERVER_SOCKET_H_
#define _CUSTOM_SERVER_SOCKET_H_

/************************************************************************
                 服务程序通用服务套接字模型类                        
                                                                     
		当服务启动后，将创建两个线程，一个用来接受客户端连接的线程，一个
用来处理已经建立连接客户端数据。										
		此类与CCustomServerClient类是结合使用的，对于子类的客户端连接类 
 也必须继承于CCustomServerClient。客户端的套接字在连接后将被设置为非阻
塞模式并被添加到m_ClientList列表中。                                 
		子类可以通过覆盖ProcessClients、DoStartup、DoStop、             
CreateClientSocket、DestroyClientSocket函数来完成对客户端连接的处理，
 启动、停止服务、创建以及销毁客户端连接对象                           
                                                                      
************************************************************************/

#include "_osdef.h"
#include "Thread.h"
using namespace wylib::thread;

class CCustomServerClientSocket;

class CCustomServerSocket
	: public CCustomWorkSocket
{
public:
	typedef CCustomWorkSocket	Inherited;

private:
	volatile LONG				m_boStoped;				//是否停止工作线程
	//BOOL						m_boAcceptThreadStoped;	//接受新连接的工作线程是否已经停止的标记*必须再接受连接线程停止后才能停止数据工作线程
	TCHAR						m_sServiceName[256];	//服务名称
	TCHAR						m_sBindHost[256];		//服务绑定地址
	INT_PTR						m_nBindPort;			//服务绑定端口

	//THREAD_HANDLE			m_hAcceptThread;		//接受客户端连接线程
	//THREAD_HANDLE			m_hDataThread;			//客户端通信数据处理线程
	
	INT_PTR						m_nLoopCountLimit;		//连接数据处理线程的单次循环次数限制，默认值为2
	TICKCOUNT					m_uLoopTimeLimit;		//连接数据处理线程的单次循环时间限制（单位是毫秒），默认值是4
	LOOPPERFORMANCE				m_LoopPerformance;		//数据处理主循环性能统计
	PERFORMANCEINFO				m_MainProcPerformance;	//主循环性能统计
	PERFORMANCEINFO				m_MainSleepPerformance;	//主循休眠状态统计


	CBaseThread                 *m_pAcceptThread;   //用于处理连接的线程
	CBaseThread                 *m_pDataThread;     //用于处理数据的线程


protected:
	CQueueList<CCustomServerClientSocket*>	m_ClientList;//客户端列表
	CCSLock						m_ClientListLock;		//客户端列表锁

private:
	//接受客户端连接的线程

	static void ServerSocketAcceptThreadRoutine(void *lpServer);

	

	//处理已经连接的客户端的线程
	static void ServerSocketDataThreadRoutine(void *lpServer);

	
protected:
	/*** 子类可能必要覆盖的函数集 ***/
	//当客户端连接后会调用此函数以便创建一个连接对象，子类应当覆盖此函数
	virtual CCustomServerClientSocket* CreateClientSocket(SOCKET nSocket, PSOCKADDR_IN pAddrIn) = 0;
	//销毁一个已经建立的客户端连接对象，默认的操作是进行delete销毁
	virtual VOID DestroyClientSocket(CCustomServerClientSocket *pClientSocket);

protected:
	/*** 事件通知函数集 ***/
	//当客户端连接处理线程启动后立即调用
	virtual VOID OnSocketDataThreadStart();
	//当客户端连接处理线程停止前调用
	virtual VOID OnSocketDataThreadStop();
	//当客户端连接成功后调用
	virtual VOID OnClientConnect(CCustomServerClientSocket *pClientSocke);

protected:
	//提供用于子类进行覆盖的开启服务的函数，如果函数返回FALSE，则启动将会失败
	//函数将在进入监听后，启动接受连接和连接处理线程前调用
	virtual BOOL DoStartup();
	//提供用于族类进行覆盖的停止服务的函数
	//函数将在停止接受连接以及处理数据线程并关闭监听套接字后调用
	virtual VOID DoStop();
	//处理所有已经建立的连接
	virtual VOID ProcessClients();
	//关闭所有客户端连接
	virtual VOID CloseAllClients();

	//覆盖父类例行执行的函数
	VOID SingleRun();

public:
	CCustomServerSocket();
	~CCustomServerSocket();

	//设置服务名称，例如会话、数据，应当在服务启动前调用此函数设置服务名称
	VOID SetServiceName(LPCTSTR sName);
	//设置连接数据处理线程的单次循环次数限制默认值为2，调整此值可针对服务器服务的类型优化性能
	//函数可以再任何时候调用并将立即生效
	VOID SetSingleLoopCount(INT nSingleLoopCount);
	//设置连接数据处理线程的单次循环时间限制（单位是毫秒）默认值是4，调整此值可针对服务器服务的类型优化性能
	//函数可以再任何时候调用并将立即生效
	VOID SetSingleLoopTime(UINT uSingleLoopTimeInMilSec);
	//设置套接字服务绑定的地址（支持域名）
	VOID SetServiceHost(LPCTSTR sHost);
	//设置套接字服务器定的端口
	VOID SetServicePort(const INT_PTR nPort);


	//启动服务器套接字
	BOOL Startup();
	//停止服务器套接字
	VOID Stop();

	
	inline LPCTSTR GetServiceName(){ return m_sServiceName; }
	inline LPCTSTR GetServiceHost(){ return m_sBindHost; }
	inline INT_PTR GetServicePort(){ return m_nBindPort; }
	inline BOOL Started(){ return !m_boStoped; }
};

#endif

