#ifndef _CUSTOM_SERVERCLIENT_SOCKET_H_
#define _CUSTOM_SERVERCLIENT_SOCKET_H_

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

#include "Thread.h"
#include "Lock.h"

using namespace wylib::time::tick64;
using namespace wylib::container;
using namespace wylib::sync::lock;
using namespace wylib::thread;

class CCustomServerClientSocket : public CCustomWorkSocket,  public Counter<CCustomServerClientSocket>
{
public:
	typedef CCustomWorkSocket Inherited;

public:
	CCustomServerClientSocket();
	CCustomServerClientSocket(SOCKET nSocket, SOCKADDR_IN *pClientAddr);
	virtual ~CCustomServerClientSocket();

	//获取客户端连接地址和端口
	inline SOCKADDR_IN GetRemoteAddrIn(){ return m_RemoteAddr; }
	//获取客户端名称
	inline LPCTSTR GetRemoteHost(){ return m_sRemoteHost; }
	//获取客户端端口
	inline INT GetRemotePort(){ return m_nRemotePort; }
	//获取和设置是否主动向客户端发送保持连接数据属性，默认值为TRUE
	inline BOOL GetActiveKeepAlive(){ return m_boActiveKeepAlive; }
	inline VOID SetActiveKeepAlive(const BOOL boActive){ m_boActiveKeepAlive = boActive; }
	//由外部调用例行执行函数
	inline VOID Run(){ SingleRun(); }

protected:
	/*** 子类可能必要覆盖的函数集 ***/
	//当长时间没有通信后发送保持连接的消息
	virtual VOID SendKeepAlive();

protected:
	//设置客户端套接字
	VOID SetClientSocket(SOCKET nSocket, SOCKADDR_IN *pClientAddr);
	//覆盖 例行执行函数
	VOID SingleRun();

protected:
	SOCKADDR_IN				m_RemoteAddr;		//客户端连接地址和端口
	char					m_sRemoteHost[128];	//客户端名称
	int					m_nRemotePort;		//客户端端口
	BOOL					m_boActiveKeepAlive;//是否主动向客户端发送保持连接数据包
};


#endif

