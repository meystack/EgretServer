
#include "StdAfx.h"
#include "DataProcess.h"
#include "SockProcess.h"
#include "SelectSockProcess.h"

#ifdef _MSC_VER
VOID CSelectRunSockProcesser::SelectThreadRoutine(void * arg)
#else
void * CSelectRunSockProcesser::SelectThreadRoutine(void * arg)
#endif
{
	CSelectRunSockProcesser *pRunSock = (CSelectRunSockProcesser*)arg;

	PRUNGATEUSERSESSION pFirstSession, pLastSession, pSession;
	int nLoop = 0;

	pSession = pFirstSession = pRunSock->m_pDataProcesser->GetFirstSession();
	pLastSession = pFirstSession + pRunSock->m_pDataProcesser->GetMaxSessionCount();

	while ( !pRunSock->m_boStoping )
	{
		pSession += pRunSock->SelectSessions( pSession, (int)(pLastSession - pSession) );
		if ( pSession >= pLastSession )
			pSession = pFirstSession;

		nLoop++;
		if ( nLoop > 32 )
		{
			Sleep( 1 );
			nLoop = 0;
		}
	}
	ExitThread( 0 );
}

#ifdef _MSC_VER
VOID CSelectRunSockProcesser::AcceptThreadRoutine(void *arg)
#else
void * CSelectRunSockProcesser::AcceptThreadRoutine(void * arg)
#endif
{
	CSelectRunSockProcesser *pRunSock = (CSelectRunSockProcesser*)arg;

	while ( !pRunSock->m_boStoping )
	{
		pRunSock->NewSession();
	}
	ExitThread( 0 );
}

CSelectRunSockProcesser::CSelectRunSockProcesser():CRunSockProcesser()
{
	m_RunSockType	= rsSelect;
	m_hSelectThread = NULL;
	m_hAcceptThread = NULL;
}

CSelectRunSockProcesser::~CSelectRunSockProcesser()
{
	Stop();
}

INT CSelectRunSockProcesser::SelectSessions(PRUNGATEUSERSESSION pSession, INT nMax)
{
	PRUNGATEUSERSESSION pSessions[FD_SETSIZE];
	fd_set readfds, errfds;
	timeval tv;
	int nErr, nFdSSCount, nResult;
	SOCKET	nSocketMax = 0;

	FD_ZERO( &readfds );
	FD_ZERO( &errfds );

	tv.tv_sec = 0;
	tv.tv_usec = 200000;
	nFdSSCount = 0;
	nResult = 0;


	if ( nMax > FD_SETSIZE )
		nMax = FD_SETSIZE;

	while ( nResult < nMax )
	{
		if ( pSession->nSocket != INVALID_SOCKET && !pSession->boMarkToClose && !pSession->boRemoteClosed )
		{
			FD_SET( pSession->nSocket, &readfds );
			pSessions[nFdSSCount] = pSession;
			nFdSSCount++;
			if(pSession->nSocket > nSocketMax) nSocketMax = pSession->nSocket;
		}
		pSession++;
		nResult++;
	}

	++nSocketMax;

	if ( nFdSSCount > 0 )
	{
		#ifdef WIN32
			nErr = select(0, &readfds, NULL, &errfds, &tv );
		#else
			nErr = select(nSocketMax, &readfds, NULL, &errfds, &tv );
		#endif

		if ( nErr > 0 )
		{
			while ( nFdSSCount > 0 )
			{
				nFdSSCount--;
				pSession = pSessions[nFdSSCount];
				if ( FD_ISSET( pSession->nSocket, &errfds ) )
				{
					pSession->boRemoteClosed = true;
					m_pDataProcesser->SendCloseSession( pSession, TRUE, 2001 );
				}
				else if ( FD_ISSET( pSession->nSocket, &readfds ) )
				{
					nErr = recv( pSession->nSocket, pSession->sRecvBuf, sizeof(pSession->sRecvBuf)-1, 0 );
					if ( nErr > 0 )
					{
						pSession->sRecvBuf[nErr] = 0;
						m_pDataProcesser->AddRecvBuf( pSession, pSession->nVerifyIdx, pSession->sRecvBuf, nErr );
					}
					else if (nErr == SOCKET_ERROR)
					{
						pSession->boRemoteClosed = true;
						m_pDataProcesser->SendCloseSession( pSession, TRUE, 2002 );
					}
				}
			}
		}
		else if ( nErr == SOCKET_ERROR )
		{
			GotError( __FUNCTION__, "select", WSAGetLastError() );
		}
	}

	return nResult;
}

BOOL CSelectRunSockProcesser::StartSelectThreads()
{
#ifdef _MSC_VER
	m_hSelectThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)SelectThreadRoutine, this, 0, NULL );
#else
	pthread_create(&m_hSelectThread, NULL, SelectThreadRoutine, this);
#endif

	if ( !m_hSelectThread )
	{
		GotError( __FUNCTION__, "CreateThread", GetLastError() );
		return FALSE;
	}

#ifdef _MSC_VER
	m_hAcceptThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)AcceptThreadRoutine, this, 0, NULL );
#else
	pthread_create(&m_hAcceptThread, NULL, AcceptThreadRoutine, this);
#endif
	if ( !m_hAcceptThread )
	{
		GotError( __FUNCTION__, "CreateThread", GetLastError() );
		return FALSE;
	}

	return TRUE;
}

VOID CSelectRunSockProcesser::StopSelectThreads()
{
	CloseThread( m_hAcceptThread );
	CloseThread( m_hSelectThread );
}

BOOL CSelectRunSockProcesser::Start()
{
	if ( !INHERITED::Start() )
		return FALSE;
	return InitAcceptSocket() && StartSelectThreads();
}

VOID CSelectRunSockProcesser::Stop()
{
	INHERITED::Stop();
	UninitAcceptSocket();
	StopSelectThreads();
}
