#include "SocketConfig.h"

#ifdef CONFIG_USE_WIN_SOCKET

using namespace wylib::inet::socket;

int CCustomSocket::setBlockMode(const bool block)
{
	u_long ulock;

	if ( block == m_boBlock )
		return 0;

	if ( m_nSocket != INVALID_SOCKET )
	{
		ulock = block ? 0 : 1;
		if ( ioctlsocket( m_nSocket, FIONBIO, &ulock ) )
		{
			return WSAGetLastError();
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
		closesocket( m_nSocket );
		m_nSocket = INVALID_SOCKET;
	}
	m_boConnected = false;
	m_boConnecting = false;
	m_boBlock = true;
}

INT CCustomSocket::shutDown(const INT sd)
{
	INT nErr = 0;

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

	ZeroMemory(&addrin, sizeof(addrin));
	addrin.sin_family = AF_INET;
	addrin.sin_addr.s_addr = addr;
	addrin.sin_port = htons(port);

	nErr = ::bind( m_nSocket, (sockaddr*)&addrin, sizeof(addrin) );
	if ( nErr == 0 )
	{
		m_LocalAddr = addrin;
	}

	return nErr ? WSAGetLastError() : 0;
}

INT CCustomSocket::bind(const char * addr, const INT port)
{
	hostent *phost;
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
	return WSAGetLastError();
}

INT CCustomSocket::getRecvBufSize(ULONG *size)
{
	int oplen = sizeof(*size);
	int nErr = getsockopt( m_nSocket, SOL_SOCKET, SO_RCVBUF, (char*)size, &oplen );
	return nErr ? WSAGetLastError() : 0;
}

INT CCustomSocket::setRecvBufSize(ULONG size)
{
	int nErr = setsockopt( m_nSocket, SOL_SOCKET, SO_RCVBUF, (char*)&size, sizeof(size) );
	return nErr ? WSAGetLastError() : 0;
}

INT CCustomSocket::getSendBufSize(ULONG *size)
{
	int oplen = sizeof(*size);
	int nErr = getsockopt( m_nSocket, SOL_SOCKET, SO_SNDBUF, (char*)size, &oplen );
	return nErr ? WSAGetLastError() : 0;
}

INT CCustomSocket::setSendBufSize(ULONG size)
{
	int nErr = setsockopt( m_nSocket, SOL_SOCKET, SO_SNDBUF, (char*)&size, sizeof(size) );
	return nErr ? WSAGetLastError() : 0;
}

INT CCustomSocket::listen(const INT backlog)
{
	int nErr;
	nErr = ::listen( m_nSocket, backlog );

	return nErr ? WSAGetLastError() : 0;
}

int CCustomSocket::connect(const char * addr, const INT port)
{
	hostent *phost;
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
		return connect( addr.s_addr, port );
	}
	return WSAGetLastError();
}

INT CCustomSocket::accept(SOCKET *socket, unsigned long wait_msec, PSOCKADDR_IN addr)
{
	int nErr;
	fd_set set;
	timeval tv;
	int addrsize;

	FD_ZERO( &set );
	FD_SET( m_nSocket, &set );

	tv.tv_sec = long(wait_msec / 1000);
	tv.tv_usec = long(wait_msec * 1000);
	
	nErr = select( int(m_nSocket + 1), &set, NULL, NULL, &tv );
	if ( nErr < 0 )
		return WSAGetLastError();
	if ( nErr > 0 )
	{
		addrsize = sizeof(*addr);
		*socket = ::accept( m_nSocket, (sockaddr*)addr, &addrsize );
		if ( *socket == INVALID_SOCKET )
		{
			return WSAGetLastError();
		}
		return 0;
	}

	return SOCKET_ERROR - 1;
}

INT CCustomSocket::connect(const ULONG addr, const INT port)
{
	int nErr;
	SOCKADDR_IN addrin;

	ZeroMemory(&addrin, sizeof(addrin));
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
		nErr = WSAGetLastError();
		if ( nErr == WSAEWOULDBLOCK )
		{
			nErr = 0;
			m_boConnected = false;
			m_boConnecting = true;
		}
	}

	return nErr;
}

INT CCustomSocket::createSocket(SOCKET *socket, const INT af, const int type, const int protocol)
{
	*socket = ::socket( af, type, protocol );
	if ( *socket == INVALID_SOCKET )
	{
		return WSAGetLastError();
	}

	return 0;
}
	
INT CCustomSocket::recv(LPVOID buf, INT len, const INT flags)
{
	int nErr = ::recv( m_nSocket, (char*)buf, len, flags );

	if ( nErr == 0 )
	{
		close();
	}
	else if ( nErr < 0 )
	{
		if ( !m_boBlock )
		{
			nErr = WSAGetLastError();
			if ( nErr != WSAEWOULDBLOCK )
			{
				SocketError( nErr );
				nErr = -1;
			}
			else nErr = SOCKET_ERROR - 1;
		}
	}

	return nErr;
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
				nErr = WSAGetLastError();
				if ( nErr != WSAEWOULDBLOCK )
				{
					nRet = SOCKET_ERROR;
					SocketError( WSAGetLastError() );
				}
				else if ( nRet == 0 )
				{
					nRet = SOCKET_ERROR - 1;
				}
			}
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
