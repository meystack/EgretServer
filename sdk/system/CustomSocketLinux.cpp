#include <cstring>
#include <errno.h>
#include "SocketConfig.h"

#ifdef CONFIG_USE_LINUX_SOCKET

using namespace wylib::inet::socket;

INT CCustomSocket::setBlockMode(const bool block)
{
	u_long ulock;

	if ( block == m_boBlock )
		return 0;

	if ( m_nSocket != INVALID_SOCKET )
	{
		ulock = block ? 0 : 1;
		if ( ioctl( m_nSocket, FIONBIO, &ulock ) )
		{
			return errno;
		}
	}

	m_boBlock = block;
	return 0;
}

VOID CCustomSocket::close()
{
	if ( m_nSocket != INVALID_SOCKET )
	{
		if ( m_boConnected )
		{
			Disconnected();
		}
		shutdown(m_nSocket, SHUT_RDWR);
		::close(m_nSocket);
		m_nSocket = INVALID_SOCKET;
	}
	m_boConnected = false;
	m_boConnecting = false;
	m_boBlock = true;
}

INT CCustomSocket::shutDown(const INT sd)
{
	int nErr = 0;

	if ( m_boConnected && m_nSocket != INVALID_SOCKET )
	{
		nErr = shutdown( m_nSocket, sd );
		if ( nErr == 0 )
		{
			m_boConnected = false;
			m_boConnecting = false;
			Disconnected();
		}
	}

	return 0;
}

INT CCustomSocket::bind(const ULONG addr, const INT port)
{
	INT nErr;
	SOCKADDR_IN addrin;

	memset(&addrin, 0, sizeof(addrin));
	addrin.sin_family = AF_INET;
	//addrin.sin_addr.s_addr = addr;
	addrin.sin_addr.s_addr = htonl(INADDR_ANY);

	addrin.sin_port = htons((u_short)port);

	int tmp =1;
	setsockopt(m_nSocket,SOL_SOCKET,SO_REUSEADDR,&tmp, sizeof(tmp));
	
	nErr = ::bind( m_nSocket, (const sockaddr*)&addrin, sizeof(addrin) );
	//if ( nErr == 0 )
	{
		m_LocalAddr = addrin;
	}

	//return nErr;
	return 0;
}

INT CCustomSocket::bind(const char * addr, const INT port)
{
	INT nErr;
        SOCKADDR_IN addrin;

        memset(&addrin, 0, sizeof(addrin));
        addrin.sin_family = AF_INET;
        addrin.sin_addr.s_addr = inet_addr(addr);
        addrin.sin_port = htons((u_short)port);
		int tmp=1;
		setsockopt(m_nSocket,SOL_SOCKET,SO_REUSEADDR,&tmp, sizeof(tmp));
        nErr = ::bind( m_nSocket, (const sockaddr*)&addrin, sizeof(addrin) );
        if ( nErr == 0 )
        {
                m_LocalAddr = addrin;
        }

        return nErr;
/*
	struct hostent *phost;

#ifdef UNICODE
	wylib::string::CWideString ws(addr);
	wylib::string::CAnsiString *as = ws.toAStr();
	phost = gethostbyname(*as);	
	delete as;
#else
	phost = gethostbyname(addr);
#endif

	if ( phost )
	{
		in_addr addr;
		addr.s_addr = *(u_long*)phost->h_addr_list[0];
		return bind( addr.s_addr, port );
	}
	return errno; 
*/
}

INT CCustomSocket::getRecvBufSize(ULONG *size)
{
	unsigned int oplen = sizeof(*size);
	return getsockopt( m_nSocket, SOL_SOCKET, SO_RCVBUF, (char*)size, &oplen );
}

INT CCustomSocket::setRecvBufSize(ULONG size)
{
	return setsockopt(m_nSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&size, sizeof(size));
}

INT CCustomSocket::getSendBufSize(ULONG *size)
{
	unsigned int oplen = sizeof(*size);
	return getsockopt( m_nSocket, SOL_SOCKET, SO_SNDBUF, (char*)size, &oplen );
}

INT CCustomSocket::setSendBufSize(ULONG size)
{
	return setsockopt( m_nSocket, SOL_SOCKET, SO_SNDBUF, (const char*)&size, sizeof(size) );
}

INT CCustomSocket::listen(const INT backlog)
{
	return ::listen( m_nSocket, backlog );
}

int CCustomSocket::connect(const char * addr, const INT port)
{
	struct hostent *phost = NULL;

#ifdef UNICODE
	wylib::string::CWideString ws(addr);
	wylib::string::CAnsiString *as = ws.toAStr();
	phost = gethostbyname(*as);	
	delete as;
#else
	phost = gethostbyname(addr);
#endif

	if ( phost )
	{
		in_addr addr_;
		addr_.s_addr = inet_addr(addr);//*(u_long*)(phost->h_addr_list[0]);
		return connect( addr_.s_addr, port );
	}
	return -1;
}

int CCustomSocket::accept(SOCKET *socket, unsigned long wait_msec, PSOCKADDR_IN addr)
{
	int nErr;
	fd_set set;
	timeval tv;
	unsigned int addrsize;

	FD_ZERO( &set );
	FD_SET( m_nSocket, &set );

	tv.tv_sec = long(wait_msec / 1000);
	tv.tv_usec = long( (wait_msec %1000)* 1000);

	nErr = select( int(m_nSocket + 1), &set, NULL, NULL, &tv );
	if ( nErr < 0 )
		return nErr;
	if ( nErr > 0 )
	{
		addrsize = sizeof(*addr);
		*socket = ::accept( m_nSocket, (sockaddr*)addr, &addrsize );
		if ( *socket == INVALID_SOCKET )
		{
			return nErr;
		}
		return 0;
	}

	return SOCKET_ERROR - 1;
}

INT CCustomSocket::connect(const ULONG addr, const INT port)
{
	int nErr;
	SOCKADDR_IN addrin;

	memset(&addrin, 0, sizeof(addrin));
	addrin.sin_family = AF_INET;
	addrin.sin_addr.s_addr = addr;
	addrin.sin_port = htons(port);

	nErr = ::connect( m_nSocket, (sockaddr*)&addrin, sizeof(addrin) );
	if ( nErr == 0 )
	{
		m_boConnected = true;
		m_boConnecting = false;
		Connected();
	}
	else 
	{
 		nErr = errno;
 		if ( nErr == ENOBUFS)
 		{
			nErr = 0;
			m_boConnected = false;
			m_boConnecting = true;
		}
	}

	return nErr;
}

INT CCustomSocket::connect(const char * addr, const INT port, int timeout)
{
	struct hostent *phost;

#ifdef UNICODE
	wylib::string::CWideString ws(addr);
	wylib::string::CAnsiString *as = ws.toAStr();
	phost = gethostbyname(*as);	
	delete as;
#else
	phost = gethostbyname(addr);
#endif

	if ( phost )
	{
		in_addr addr;
		addr.s_addr = *(u_long*)phost->h_addr_list[0];
		return connect( addr.s_addr, port, timeout);
	}
	return -1;
}

INT CCustomSocket::connect(const ULONG addr, const INT port, int timeout)
{
	int nErr;
	SOCKADDR_IN addrin;
	memset(&addrin, 0, sizeof(addrin));
	addrin.sin_family = AF_INET;
	addrin.sin_addr.s_addr = addr;
	addrin.sin_port = htons(port);

	// 超时需要在非阻塞下
	unsigned long mode = 1;
	ioctl(m_nSocket, FIONBIO, &mode);
	nErr = 0;
	if ( ::connect( m_nSocket, (sockaddr*)&addrin, sizeof(addrin) ) == 0 )
	{
		m_boConnected = true;
		m_boConnecting = false;
		Connected();
	}
	else
	{
		// 设置回阻塞
		mode = 0;
		ioctl(m_nSocket, FIONBIO, &mode);

		// 使用select超时connet
		struct timeval tm={timeout,0};
		fd_set wset;
		FD_ZERO(&wset);
		FD_SET(m_nSocket, &wset);
		if( ::select(m_nSocket+1, NULL, &wset, NULL, &tm) > 0)
		{
			int error=-1, len=sizeof(int);
			getsockopt(m_nSocket, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
			if(error == 0)
			{
				m_boConnected = true;
				m_boConnecting = false;
				Connected();
				return 0;
			}
		}
		
		// 执行到这里说明连接不成功
		nErr = errno;
		if ( nErr == ENOBUFS)
		{
			nErr = 0;
			m_boConnected = false;
			m_boConnecting = true;
		}
	}
	return nErr;
}

int CCustomSocket::createSocket(SOCKET *socket, const INT af, const int type, const int protocol)
{
	*socket = ::socket( af, type, protocol );
	if ( *socket == INVALID_SOCKET )
	{
		return *socket;
	}
	m_nSocket = *socket ;
	return 0;
}

/* 原来的recv的实现
int nErr = ::recv( m_nSocket, (char*)buf, len, flags );

if ( nErr == 0 )
{
	close();
}
else if ( nErr < 0 )
{
	if ( !m_boBlock )
	{
		nErr = errno; 
		if ( nErr != ENOBUFS )
		{
			SocketError( nErr );
			nErr = -1;
		}
		else 
		{
			nErr = SOCKET_ERROR - 1;
		}
	}
}

return nErr;
*/

int CCustomSocket::recv(LPVOID buf, INT size, const INT flags)
{
	if ( !buf || size < 1) return 0;
	
	int r = 0 ;
	r = ::recv(m_nSocket, buf, size, flags);
	if( r == 0 )
	{
		// 关闭连接
		close();
		return 0 ;
	}
	else if( r < 0 )
	{
		return -1 ;
	}
	
	return r;  
}

INT CCustomSocket::send(LPVOID buf, INT len, const INT flags)
{
	int nRet, nErr;
	char *ptr = (char*)buf;

	nRet = 0;
	while ( len > 0 )
	{
		nErr = ::send( m_nSocket, (char*)ptr, len, flags );
		if ( nErr == 0 )
		{
			nRet = 0;
			close();
			break;
		}
		else if ( nErr < 0 )
		{
			if ( !m_boBlock )
			{
				if ( errno == EINTR )   // 因为发生中断，中断处理后可以继续发送，所以此处应选择暂时退出函数， 然后再继续调用send进行接收数据
				{
					continue ;
				}
				else if(errno == EAGAIN)
				{
					Sleep(10);
					continue ;
				}
				
			}
			nRet = nErr;
			break;
		}
		else 
		{
			nRet += nErr;
			ptr += nErr;
			len -= nErr;
		}
	}
	return nRet;
}

#endif
