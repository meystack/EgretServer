#ifndef _CUSTOM_SERVERCLIENT_SOCKET_EX_H_
#define _CUSTOM_SERVERCLIENT_SOCKET_EX_H_

/************************************************************
*
*	服务器接受的客户端连接类
*
*		连接建立后，套接字将被调整为异步非阻塞模式。
*
*		数据的发送使用队列堆积的形式，而不是使用缓冲区堆积
*	的方式，所有提交的待发送的数据将被保存到一个列表中排队
*	发送，在添加发送数据的时候，可以指定是否优先发送以便将
*	数据插入到待发队列的头部而优先发送。
*
***********************************************************/

#include "Lock.h"
#include "Thread.h"

using namespace wylib::time::tick64;
using namespace wylib::container;
using namespace wylib::sync::lock;
using namespace wylib::thread;


class CCustomServerClientSocketEx : public CCustomServerClientSocket, CBaseThread
{
public:
	typedef CCustomServerClientSocket Inherited;
	typedef CBaseThread InheritedThread;
public:
	CCustomServerClientSocketEx();
	CCustomServerClientSocketEx(SOCKET nSocket, SOCKADDR_IN *pClientAddr);
	virtual ~CCustomServerClientSocketEx();

	// 开始工作，创建工作线程发送数据
	void StartWork();
	// 停止工作线程发送数据
	void StopWork();
protected:	
	// 重写基类函数，这里不发送数据，单独开个线程去发送数据
	//virtual VOID SendSocketBuffers();	
protected:	
	//static DWORD WINAPI staticSendDataRountine(LPVOID pParam);
	//void SendDataRoutine();
	virtual void OnRountine();

	
	size_t sendToSocketEx(CCustomSocket& socket);
protected:	
	//HANDLE					m_hSendDataThread;
	volatile bool			m_bStop;	
	//HANDLE					m_hStopEvent;			//停止信号事件
};


#endif

