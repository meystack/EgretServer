
#include "StdAfx.h"

#include "DataProcess.h"
#include "SockProcess.h"
#include "IOCPSockProcess.h"

#ifdef WIN32

VOID CIOCPRunSockProcesser::ComplationPortWorkThreadRoutine(CIOCPRunSockProcesser *pRunSock)
{
	HANDLE hIOPort;
	DWORD dwBytesTransfered;
	ULONG_PTR uComplationKey;
	PRUNSOCKOVERLAPPED lpOverlapped;
	PRUNGATEUSERSESSION pSession;
	int nErr;

	hIOPort = pRunSock->m_hIOPort;
	while ( !pRunSock->m_boStoping )
	{
		dwBytesTransfered = 0;
		uComplationKey = 0;
		lpOverlapped = NULL;
		if ( GetQueuedCompletionStatus( hIOPort, &dwBytesTransfered, &uComplationKey, (LPOVERLAPPED*)&lpOverlapped, 10000 ) )
		{
			if ( !lpOverlapped )
				continue;

			//AcceptEx
			if ( lpOverlapped->btAction == aaAccept )
			{
				pRunSock->NewSession();
				continue;
			}

			pSession = lpOverlapped->pUser;
			//套接字已关闭
			if ( !dwBytesTransfered )
			{
				pSession->boRemoteClosed = true;
				pRunSock->m_pDataProcesser->PostCloseSession( pSession );
				continue;
			}

			//WSARecv
			if ( lpOverlapped->btAction == aaRecv )
			{
				pSession->Overlapped.WSABuf.buf[dwBytesTransfered] = 0;
				pRunSock->m_pDataProcesser->AddRecvBuf( pSession, uComplationKey, lpOverlapped->WSABuf.buf, dwBytesTransfered );
				if ( pSession->nSocket != INVALID_SOCKET && !pSession->boMarkToClose && !pSession->boRemoteClosed )
				{
					pRunSock->PostRecv( pSession );
				}
				continue;
			}

			//WSASend
			if ( lpOverlapped->btAction == aaSend )
			{
				continue;
			}
		}
		else
		{
			nErr = GetLastError();
			if ( nErr == WAIT_TIMEOUT )
				continue;

			//socket closed
			if ( lpOverlapped )
			{
				pSession = lpOverlapped->pUser;
				pSession->boRemoteClosed = true;
				pRunSock->m_pDataProcesser->PostCloseSession( pSession );
				continue;
			}

			pRunSock->GotError( __FUNCTION__, "GetQueuedCompletionStatus", nErr );
		}
	}

	ExitThread( 0 );
}


CIOCPRunSockProcesser::CIOCPRunSockProcesser():CRunSockProcesser()
{
	m_RunSockType			= rsIOCP;
	m_hIOPort				= NULL;
	m_nWorkThreadCount		= 1;
	ZeroMemory( m_hIOWorkThreads, sizeof(m_hIOWorkThreads) );
	m_NewSocket				= INVALID_SOCKET;
}

CIOCPRunSockProcesser::~CIOCPRunSockProcesser()
{
	Stop();
}

BOOL CIOCPRunSockProcesser::InitComplationPort()
{
	if ( !m_hIOPort )
	{
		m_hIOPort = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 0 );
		if ( !m_hIOPort )
		{
			GotError( __FUNCTION__, "CreateIoCompletionPort", GetLastError() );
			return FALSE;
		}
	}

	return TRUE;
}

VOID CIOCPRunSockProcesser::UninitComplationPort()
{
	if ( m_hIOPort )
	{
		CancelIo( m_hIOPort );
		CloseHandle( m_hIOPort );
		m_hIOPort = NULL;
	}
}

BOOL CIOCPRunSockProcesser::InitAcceptSocket()
{
	if ( !INHERITED::InitAcceptSocket() )
		return FALSE;

	HANDLE hPort = CreateIoCompletionPort( (HANDLE)m_ListenSocket, m_hIOPort, (ULONG_PTR)m_ListenSocket, 0 );
	if ( !hPort || hPort != m_hIOPort )
	{
		GotError( __FUNCTION__, "CreateIoCompletionPort", WSAGetLastError() );
		return FALSE;
	}

	return TRUE;
}

BOOL CIOCPRunSockProcesser::StartIOCPThreads()
{
	int i, nThreadCount;

	nThreadCount = __max( 1, m_nWorkThreadCount );
	for ( i=0; i<nThreadCount; ++i )
	{
		if ( !m_hIOWorkThreads[i] )
		{
			m_hIOWorkThreads[i] = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)ComplationPortWorkThreadRoutine, this, 0, NULL );
			if ( !m_hIOWorkThreads[i] )
			{
				GotError( __FUNCTION__, "CreateThread", GetLastError() );
				return FALSE;
			}
		}
	}

	return TRUE;
}

VOID CIOCPRunSockProcesser::StopIOCPThreads()
{
	int i;

	for ( i=0; i<MAX_ASYNCIO_THREAD; ++i )
	{
		if ( m_hIOWorkThreads[i] )
		{
			do
			{
				PostQueuedCompletionStatus( m_hIOPort, 0, NULL, NULL );
			}
			while ( WaitForSingleObject( m_hIOWorkThreads[i], 100 ) == WAIT_TIMEOUT );
			CloseHandle( m_hIOWorkThreads[i] );
			m_hIOWorkThreads[i] = NULL;
		}
	}
}

VOID CIOCPRunSockProcesser::NewSession()
{
	SOCKET nSocket;
	PSOCKADDRIN pLocalAddr, pRemoteAddr;
	int localSize, remoteSize, nErr;
	PRUNGATEUSERSESSION pSession;
	u_long uBlock;
	

	nSocket = m_NewSocket;
	m_NewSocket = INVALID_SOCKET;
	//继续接受新的连接
	PostAccept();

	//更新接受的套接字的信息
	nErr = setsockopt( nSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&m_ListenSocket, sizeof(m_ListenSocket) );
	if ( nErr == SOCKET_ERROR )
	{
		closesocket( nSocket );
		GotError( __FUNCTION__, "setsockopt", WSAGetLastError() );
		return;
	}

	//获取接受的套接字对端地址
	GetAcceptExSockaddrs( &m_ASyncAcceptAddr, 0, sizeof(m_ASyncAcceptAddr[0]) * 2, sizeof(m_ASyncAcceptAddr[0]) * 2,
		(sockaddr**)&pLocalAddr, &localSize, (sockaddr**)&pRemoteAddr, &remoteSize );

	//更新地址信息
	nErr = getpeername( nSocket, (sockaddr*)pRemoteAddr, &remoteSize );
	if ( nErr == SOCKET_ERROR )
	{
		closesocket( nSocket );
		GotError( __FUNCTION__, "getpeername", WSAGetLastError() );
		return;
	}

	//将套接字设置为非阻塞
	uBlock = 1;
	nErr = ioctlsocket( nSocket, FIONBIO, &uBlock );
	if ( SOCKET_ERROR == nErr )
	{
		closesocket( nSocket );
		GotError( __FUNCTION__, "ioctlsocket", WSAGetLastError() );
		return;
	}
	
	//申请用户会话指针
	if ( !(pSession = m_pDataProcesser->NewSession( nSocket, *pRemoteAddr )) )
	{
		closesocket( nSocket );
		GotError( __FUNCTION__, "用户队列已经满了。", ERROR_SUCCESS );
		return;
	}

	if ( m_hIOPort != CreateIoCompletionPort( (HANDLE)nSocket, m_hIOPort, (ULONG_PTR)pSession->nVerifyIdx, 0 ) )
	{
		GotError( __FUNCTION__, "CreateIoCompletionPort", GetLastError() );
		closesocket( nSocket );
		return;
	}

	//AdjustSocketSendBufSize( nSocket, 32 * 1024 );

	if ( !PostRecv( pSession ) )
	{
		closesocket( nSocket );
		return;
	}
}

BOOL CIOCPRunSockProcesser::PostAccept()
{
	int nErr;

	//建立套接字作为新接受的连接使用
	m_NewSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_IP );
	if ( m_NewSocket == INVALID_SOCKET )
	{
		GotError( __FUNCTION__, "socket", WSAGetLastError() );
		return FALSE;
	}

	ZeroMemory( &m_ASyncAcceptOverlapped, sizeof(m_ASyncAcceptOverlapped) );
	m_ASyncAcceptOverlapped.btAction = aaAccept;

	//异步的Accept
	nErr = AcceptEx( m_ListenSocket, m_NewSocket, &m_ASyncAcceptAddr, 0, sizeof(m_ASyncAcceptAddr[0]) * 2, 
		sizeof(m_ASyncAcceptAddr[0]) * 2, NULL, &m_ASyncAcceptOverlapped.Overlapped );
	if ( nErr == SOCKET_ERROR )
	{
		GotError( __FUNCTION__, "AcceptEx", WSAGetLastError() );
		return FALSE;
	}

	return TRUE;
}

BOOL CIOCPRunSockProcesser::PostRecv(PRUNGATEUSERSESSION pSession)
{
	int nErr;
	DWORD dwBytesRecved, dwFlags;

	while ( TRUE )
	{
		dwBytesRecved = dwFlags = 0;
		pSession->Overlapped.btAction = aaRecv;
		ZeroMemory( &pSession->Overlapped.Overlapped, sizeof(pSession->Overlapped.Overlapped) );
		pSession->Overlapped.WSABuf.buf = pSession->sRecvBuf;
		pSession->Overlapped.WSABuf.len = sizeof(pSession->sRecvBuf) - 1;
		nErr = WSARecv( pSession->nSocket, &pSession->Overlapped.WSABuf, 1, &dwBytesRecved, &dwFlags, &pSession->Overlapped.Overlapped, NULL );
		if ( !nErr )
		{
			//连接已断开
			if ( !dwBytesRecved )
			{
				pSession->boRemoteClosed = true;
				m_pDataProcesser->PostCloseSession( pSession );
				return FALSE;
			}
			else 
			{
				pSession->Overlapped.WSABuf.buf[dwBytesRecved] = 0;
				m_pDataProcesser->AddRecvBuf( pSession, pSession->nVerifyIdx, pSession->Overlapped.WSABuf.buf, dwBytesRecved );
				if ( pSession->nSocket == INVALID_SOCKET || pSession->boMarkToClose || pSession->boRemoteClosed )
				{
					return FALSE;
				}
			}
		}
		else
		{
			nErr = WSAGetLastError();
			if ( nErr != WSA_IO_PENDING )
			{
				m_pDataProcesser->PostCloseSession( pSession );
				GotError( __FUNCTION__, "WSARecv", nErr );
				return FALSE;
			}
			break;
		}
	}

	return TRUE;
}

INT CIOCPRunSockProcesser::GetWorkThreadCount()
{
	return __max( 1, m_nWorkThreadCount );
}

VOID CIOCPRunSockProcesser::SetWorkThreadCount(INT ThreadCount)
{
	if ( m_boStoping && ThreadCount > 0 && ThreadCount < MAX_ASYNCIO_THREAD )
	{
		m_nWorkThreadCount = ThreadCount;
	}
}

BOOL CIOCPRunSockProcesser::Start()
{
	if ( !INHERITED::Start() )
		return FALSE;
	return InitComplationPort() && StartIOCPThreads() && InitAcceptSocket() && PostAccept();
}

VOID CIOCPRunSockProcesser::Stop()
{
	INHERITED::Stop();

	StopIOCPThreads();
	UninitComplationPort();
	UninitAcceptSocket();
}


#endif
