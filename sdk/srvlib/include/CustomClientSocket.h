#ifndef _CUSTOM_CLIENT_SOCKET_H_
#define _CUSTOM_CLIENT_SOCKET_H_

/************************************************************

	服务器与服务器之间通信的客户端套接字连接模型类

		每建立一个连接类，将自动创建一个线程用于自动化处理
	连接的数据和进行逻辑处理。

		套接字在连接建立后，将被调整为异步非阻塞模式。

		数据的发送使用队列堆积的形式，而不是使用缓冲区堆积
	的方式，所有提交的待发送的数据将被保存到一个列表中排队
	发送，在添加发送数据的时候，可以指定是否优先发送以便将
	数据插入到待发队列的头部而优先发送。

***********************************************************/
#include "_osdef.h"
#include "Thread.h"

using namespace wylib::time::tick64;
using namespace wylib::container;
using namespace wylib::sync::lock;
using namespace wylib::thread;

class CCustomClientSocket : 
	public CCustomWorkSocket,
	public wylib::thread::CBaseThread 
{
public:
	typedef CCustomWorkSocket Inherited;
	typedef wylib::thread::CBaseThread InheritedThread;
public:
	CCustomClientSocket();
	virtual ~CCustomClientSocket();

	//获取客户端名称
	inline LPCTSTR GetClientName(){ return m_sClientName; };
	//获取远程服务器地址
	inline LPCTSTR GetServerHost(){ return m_sSrvHost; }
	//设置远程服务器地址（支持域名）
	VOID SetServerHost(LPCTSTR sHost);
	//获取远程服务器端口
	inline INT_PTR GetServerPort(){ return m_nSrvPort; }
	//设置远程服务器端口
	VOID SetServerPort(const INT_PTR nPort);

	//启动客户端
	BOOL Startup();
	//停止客户端
	VOID Stop();

	//是否要检测内存
	VOID SetCheckMemory(bool bCheckMemory)
	{
		m_bNeedCheckMemory =bCheckMemory;
	}
protected:
	const static int CHECK_MEMORY_INTERVAL =  5 * 60 * 1000;
		
	/*** 以下函数集为子类可能需要覆盖的函数 ***/
	//当连接处理线程启动后立即调用
	virtual VOID OnWorkThreadStart();
	//当连接处理线程停止前调用
	virtual VOID OnWorkThreadStop();
	//连接到服务器成功后发送注册客户端的消息
	virtual VOID SendRegisteClient();
	//当长时间没有通信后发送保持连接的消息
	virtual VOID SendKeepAlive();

protected:
	//设置客户端名称
	VOID SetClientName(LPCTSTR sClientName);
	//连接到服务器
	BOOL ConnectToServer();
	//覆盖例行执行函数
	VOID SingleRun();
	//获取工作线程ID
	DWORD GetWorkThreadId();

protected:
	//覆盖连接已经建立的函数
	VOID Connected();
	//覆盖断开连接的处理函数
	VOID Disconnected();
	//覆盖套接字错误的函数
	VOID SocketError(INT errorCode);

	virtual void OnRountine() ; //定时调用

	virtual void OnThreadStarted();

	virtual void OnTerminated();

private:
	//客户端连接处理线程例行函数
	//static VOID STDCALL CustomClientSocketWorkRoutine(CCustomClientSocket *lpClient);

	
private:
	TCHAR					m_sSrvHost[256];		//服务器地址
	INT_PTR					m_nSrvPort;				//服务器端口
	TCHAR					m_sClientName[256];		//客户端名称
	
	//volatile LONG			m_boStoped;				//是否标记为停止
	
	//THREAD_HANDLE 			m_hThread;
	//DWORD				m_dwThreadId;			//工作线程ID
	
	//HANDLE					m_hThread;				//工作线程句柄	
	

	TICKCOUNT				m_dwReconnectTick;		//重新连接服务器的时间
	TICKCOUNT               m_dwCheckMemoryTick;    //检测内存的时间
	
	LOOPPERFORMANCE			m_LoopPerformance;		//数据处理主循环性能统计
	PERFORMANCEINFO			m_MainProcPerformance;	//主循环性能统计
	PERFORMANCEINFO			m_MainSleepPerformance;	//主循休眠状态统计
	bool                    m_bNeedCheckMemory;     //是否需要定期检测内存
};


#endif

