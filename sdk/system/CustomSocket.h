#ifndef _CUSTOM_SOCKET_H_
#define _CUSTOM_SOCKET_H_

/******************************************************************
 *
 *							wyLib库	2008 - 2010
 *
 *	$ 通用套接字处理类 $
 *  
 *	- 主要功能 - 
 *
 *	基本的套接字封装类，支持阻塞和非阻塞模式。
 *
 *****************************************************************/
#include "_osdef.h"
#include "SocketConfig.h"

namespace wylib
{
	namespace inet
	{
		namespace socket
		{
			/*	基本TCP套接字类

			*/
			class CCustomSocket
			{
			protected:
				SOCKET		m_nSocket;		//套接字
				SOCKADDR_IN	m_LocalAddr;	//绑定的本地地址
				SOCKADDR_IN	m_RemoteAddr;	//远程地址
				bool		m_boBlock;		//是否非阻塞模式
				bool		m_boConnected;	//是否已建立连接
				bool		m_boConnecting;	//是否正在连接
			protected:
				//提供向最外层逻辑通知连接建立的事件
				virtual VOID OnConnected();
				//提供向最外层逻辑通知连接断开的事件
				virtual VOID OnDisconnected();
				//提供向最外层逻辑通知连接发生错误的事件
				virtual VOID OnError(int errorCode);
			protected:
				//继承类通过重载此函数处理建立连接需要进行的操作
				virtual VOID Connected();
				//继承类通过重载此函数处理断开连接需要进行的操作
				virtual VOID Disconnected();
				//继承类通过重载此函数处理连接发生了错误的所需要进行的操作
				virtual VOID SocketError(int nErrorCode);
			public:
				CCustomSocket();
				virtual ~CCustomSocket();

				//获得SOCKET
				inline SOCKET getSocket(){ return m_nSocket; }
				//设置SOCKET
				VOID setSocket(const SOCKET socket);
				//获取是阻塞模式。true表示阻塞，false表示非阻塞
				inline bool getBlockMode(){ return m_boBlock; }
				//设置阻塞模式。true表示阻塞，false表示非阻塞，返回值为socket错误号，0表示成功
				INT setBlockMode(const bool block);
				//判断SOCKET是否处于连接状态
				inline bool connected(){ return m_boConnected; }
				//判断SOCKET是否处于正在连接状态
				inline bool connecting(){ return m_boConnecting; }
				//断开连接并关闭套接字
				VOID close();
				//断开连接,sd表示断开的方式,分别有SD_RECEIVE、SD_SEND和SD_BOTH
				INT shutDown(const INT sd);
				//绑定指定32位的IP地址和字符串绑定到制定的地址和端口，返回值为socket错误号，0表示成功
				INT bind(const ULONG addr, const INT port);
				//通过指定IP地址字符串和端口绑定到指定的地址和端口，返回值为socket错误号，0表示成功
				INT bind(const char * addr, const INT port);
				//获取绑定的本地地址和端口的sockaddr_in结构
				inline VOID getLoaclAddr(PSOCKADDR_IN addr_in){ *addr_in = m_LocalAddr; }
				//获取远程的地址和端口的sockaddr_in结构
				inline VOID getRemoteAddr(PSOCKADDR_IN addr_in){ *addr_in = m_RemoteAddr; }
				//设置远程地址信息
				inline VOID setRemoteAddr(const PSOCKADDR_IN addr_in){ m_RemoteAddr = *addr_in; }
				//获取接收缓冲区大小，返回值为socket错误号，0表示成功。
				INT getRecvBufSize(ULONG *size);
				//设置接收缓冲区大小，返回值为socket错误号，0表示成功。
				INT setRecvBufSize(ULONG size);
				//获取发送缓冲区大小，返回值为socket错误号，0表示成功。
				INT getSendBufSize(ULONG *size);
				//设置发送缓冲区大小，返回值为socket错误号，0表示成功。
				INT setSendBufSize(ULONG size);
				//开始监听，backlog表示每次可接受的连接数量，返回值为socket错误号，0表示成功
				INT listen(const INT backlog = 5);
				//通过32位IP地址连接到制定的端口，返回值为socket错误号，0表示成功。对于非阻塞模式返回0并不表示已经建立连接
				INT connect(const ULONG addr, const INT port);
				//通过IP地址字符串连接到指定的端口，返回值为socket错误号，0表示成功。对于非阻塞模式返回0并不表示已经建立连接
				INT connect(const char * addr, const INT port);

				INT connect(const char * addr, const INT port, int timeout);
				INT connect(const ULONG addr, const INT port, int timeout);

				//接收新的连接，wait_msec表示最大等待毫秒数，函数返回0表示成功且socket被接收的套接字填充，返回SOCKET_ERROR-1表示等待超时，否则函数返回socket错误号
				INT accept(SOCKET * socket, unsigned long wait_msec, PSOCKADDR_IN addr);
				//创建套接字，返回值为socket错误号，0表示成功且socket的内容被填充为新建立的socket
				INT createSocket(OUT SOCKET *socket, const INT af = AF_INET, const int type = SOCK_STREAM, const int protocol = IPPROTO_TCP);
				//读取套接字内容。返回0表示连接已断开，返回SOCKET_ERROR表示发生错误，返回SOCKET_ERROR-1表示没有数据可读(仅对于非阻塞模式套接字)，否则表示实际接收的字节数
				virtual INT recv(LPVOID buf, INT len, const INT flags = 0);
				//向套接字写入内容，返回0表示连接已断开，返回SOCKET_ERROR表示发生错误，返回SOCKET_ERROR-1表示send操作会发生阻塞且没有数据被发送(仅对于非阻塞模式套接字)，否则返回发送的字节数。
				virtual INT send(LPVOID buf, INT len, const INT flags = 0);
			};
		};
	};
};

#endif

