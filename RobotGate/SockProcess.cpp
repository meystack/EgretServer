
#include "StdAfx.h"
#include "DataProcess.h"
#include "SockProcess.h"



CRunSockProcesser::CRunSockProcesser()
{
	m_RunSockType = rsUndefined;
	m_boStoping = TRUE;

	m_ListenSocket			= INVALID_SOCKET;
	m_BindAddr.sin_family	= AF_INET;
	m_BindAddr.sin_addr.s_addr	= 0;
	m_BindAddr.sin_port		= 0;
	m_pDataProcesser		= NULL;
	m_SessionFullTick		= 0;
}

CRunSockProcesser::~CRunSockProcesser()
{
	Stop();
}

VOID CRunSockProcesser::GotError(LPCSTR sErrorFn, LPCSTR sErrAPI, const INT ErrorCode)
{
	OutputMsg( rmError, "CRunSockProcesse::%s Got Error %d on api \"%s\"", sErrorFn, ErrorCode, sErrAPI );
}


BOOL CRunSockProcesser::InitAcceptSocket()
{
	m_ListenSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_IP );
	if ( m_ListenSocket == INVALID_SOCKET )
	{
		GotError( __FUNCTION__, "socket", WSAGetLastError() );
		return FALSE;
	}

#ifdef _MSC_VER
	char optval = 1;
#else
	int optval = 1;
#endif

	if (setsockopt(m_ListenSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == SOCKET_ERROR) {
		GotError( __FUNCTION__, "setsockopt", WSAGetLastError() );
		//assert(false);
		return false;
	}

	if ( SOCKET_ERROR == bind( m_ListenSocket, (struct sockaddr*)&m_BindAddr, sizeof(m_BindAddr) ) )
	{
		GotError( __FUNCTION__, "socket", WSAGetLastError() );
		return FALSE;
	}

	if ( SOCKET_ERROR == listen( m_ListenSocket, 5 ) )
	{
		GotError( __FUNCTION__, "listen", WSAGetLastError() );
		return FALSE;
	}

	return TRUE;
}

VOID CRunSockProcesser::UninitAcceptSocket()
{
	if ( m_ListenSocket != INVALID_SOCKET )
	{
		GotError( __FUNCTION__, "close " , WSAGetLastError());
		closesocket( m_ListenSocket );
		m_ListenSocket = INVALID_SOCKET;
	}
}

VOID CRunSockProcesser::NewSession()
{
	SOCKET nSocket;
	SOCKADDRIN sRemote;
	socklen_t nRemoteSize, nErr;
	u_long uBlock;
	PRUNGATEUSERSESSION pSession;
	
	nRemoteSize = sizeof(sRemote);
	//监听
	nSocket = accept( m_ListenSocket, (sockaddr*)&sRemote, &nRemoteSize );
	if ( nSocket == INVALID_SOCKET )
	{
		GotError( __FUNCTION__, "accept", WSAGetLastError() );
		return;
	}

	//用户队列满后一段时间内不接受新连接
	if ( _getTickCount() < m_SessionFullTick )
	{
		closesocket( nSocket );
		GotError( __FUNCTION__, " _getTickCount() < m_SessionFullTick", WSAGetLastError() );
		return;
	}
	
	//建立一个session,如果session队列满了,记录时间,在10秒内不能接受新的连接
	if ( !(pSession = m_pDataProcesser->NewSession( nSocket, sRemote )) )
	{
		closesocket( nSocket );
		GotError( __FUNCTION__, "用户队列已经满了。", ERROR_SUCCESS );
		m_SessionFullTick = _getTickCount() + 10 * 1000;
		return;
	}

	//设置新的socket连接为允许非阻塞模式
	uBlock = 1;
	nErr = ioctlsocket( nSocket, FIONBIO, &uBlock );
	if ( SOCKET_ERROR == nErr )
	{
		closesocket( nSocket );
		GotError( __FUNCTION__, "ioctlsocket", WSAGetLastError() );
		return;
	}

	int bnodelay = 1;
	nErr = ::setsockopt(nSocket, IPPROTO_TCP, TCP_NODELAY, (const char *)&bnodelay, sizeof(bnodelay));
	if (SOCKET_ERROR == nErr)
	{
		GotError( __FUNCTION__, "setsockopt", WSAGetLastError() );
		return;
	}

	/*AdjustSocketSendBufSize( nSocket, 32 * 1024 );*/
}

BOOL CRunSockProcesser::AdjustSocketSendBufSize(SOCKET s, INT nSize)
{
	int nOldSize, nErr;
	socklen_t nSizeLen;
	nSizeLen = sizeof(nOldSize);
	nErr = getsockopt( s, SOL_SOCKET, SO_SNDBUF, (char*)&nOldSize, &nSizeLen );
	if ( nErr != SOCKET_ERROR )
	{
		if ( nOldSize < nSize ) 
		{
			nOldSize = nSize;
			if ( setsockopt( s, SOL_SOCKET, SO_SNDBUF, (char*)&nOldSize, nSizeLen ) == SOCKET_ERROR )
			{
				GotError( __FUNCTION__, "getsockopt", WSAGetLastError() );
				return FALSE;
			}
		}
	}
	else 
	{
		GotError( __FUNCTION__, "getsockopt", WSAGetLastError() );
		return FALSE;
	}

	return TRUE;
}

VOID CRunSockProcesser::SetBindAddress(LPCSTR sAddress)
{
	m_BindAddr.sin_addr.s_addr = inet_addr(sAddress);
}

VOID CRunSockProcesser::SetBindPort(INT nPort)
{
	m_BindAddr.sin_port = htons(nPort);
}

LPCSTR CRunSockProcesser::GetBindAddress()
{
	return inet_ntoa(m_BindAddr.sin_addr);
}

INT CRunSockProcesser::GetBindPort()
{
	return htons(m_BindAddr.sin_port);
}

RUNSOCKTYPE CRunSockProcesser::GetRunSockType()
{
	return m_RunSockType;
}

CRunDataProcesser* CRunSockProcesser::GetDataProcesser()
{
	return m_pDataProcesser;
}

CRunDataProcesser* CRunSockProcesser::SetDataProcesser(CRunDataProcesser *pDataProcesser)
{
	CRunDataProcesser *pOldProcess = m_pDataProcesser;
	m_pDataProcesser = pDataProcesser;
	return pOldProcess;
}

BOOL CRunSockProcesser::InitBase()
{
#ifdef _MSC_VER
	WSADATA WSAData;
	int nErr;

	nErr = WSAStartup( MAKEWORD(2, 2), &WSAData );
	if ( nErr )
	{
		GotError( __FUNCTION__, "WSAStartup", nErr );
		return FALSE;
	}

	timeBeginPeriod( 1 );
#endif
	return TRUE;
}

VOID CRunSockProcesser::UninitBase()
{
#ifdef _MSC_VER
	timeEndPeriod( 1 );
	WSACleanup();
#endif
}

BOOL CRunSockProcesser::Start()
{
	m_boStoping = FALSE;
	m_SessionFullTick = 0;
	return TRUE;
}

VOID CRunSockProcesser::Stop()
{
	m_boStoping = TRUE;
}
