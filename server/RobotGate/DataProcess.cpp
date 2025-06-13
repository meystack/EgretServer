
#include "StdAfx.h"
#include "DataProcess.h"
#include "SockProcess.h"
#include <signal.h>
HANDLE g_hStdOut;

unsigned int CRunDataProcesser::s_nSndThreadSleepTime = 20;
extern bool g_ServerIsRunning;

//接收数据的线程
#ifdef _MSC_VER
VOID STDCALL CRunDataProcesser::RecvDataProcdaessRoutine(void *arg)
#else
VOID* CRunDataProcesser::RecvDataProcessRoutine(void *arg)
#endif
{	
	CRunDataProcesser * pRunData = (CRunDataProcesser *)arg;

	//LPCRITICAL_SECTION pQueueLock;
	CBaseList<LPVOID>  *pRecvList;
	PCLIENTRECVBUF *ppRecvBufs, pRecvBuf;
	PRUNGATEUSERSESSION pSession;
	INT_PTR nBufferCount;
	TICKCOUNT dwProcStartTick, dwLockDataTick, dwLockCheckTick, dwSendCheckTick, dwPerfDumpTick, dwSendGateUserInfo;

	//pQueueLock = &pRunData->m_RecvQueueLock;
	pRecvList  = pRunData->m_pRecvProcList;	
	pRunData->m_llSendKeepAliveTime = 0;
	dwLockDataTick = dwLockCheckTick = dwPerfDumpTick = dwSendGateUserInfo = _getTickCount();
	dwSendCheckTick = dwSendGateUserInfo + 2000;
	while ( !pRunData->m_boStoping )
	{		
		dwProcStartTick = _getTickCount();

		//发送网关自检
		if ( dwProcStartTick - dwSendCheckTick >= 2000 )
		{
			pRunData->SendCheck( GM_CHECKSERVER );
			dwSendCheckTick = dwProcStartTick;
			pRunData->m_llSendKeepAliveTime = dwSendCheckTick;			
		}

		// 发送网关用户数目，一分钟发一次
		if (dwProcStartTick - dwSendGateUserInfo >= 60000 )
		{
			pRunData->SendGateUserInfo();
			dwSendGateUserInfo = dwProcStartTick;
		}

		//检查会话关闭
		if ( dwProcStartTick - dwLockCheckTick >= 5000 )
		{
			if ( pRunData->CheckCloseSessions( dwProcStartTick - dwLockCheckTick >= 10000 ) )
			{
				dwProcStartTick = dwLockCheckTick = _getTickCount();
			}
		}
		// dump statistic
		if (dwProcStartTick - dwPerfDumpTick >= 120000)
		{
			CTimeProfMgr::getSingleton().dump();
			dwPerfDumpTick = dwProcStartTick;
		}

		//如果未能获得数据队列锁则让出本次CPU时间
		if ( !TryEnterCriticalSection( &pRunData->m_RecvQueueLock ) )
		{
			if ( dwProcStartTick - dwLockDataTick  < 60 )
			{
				Sleep( 1 );
				continue;
			}
			else
			{
				EnterCriticalSection( &pRunData->m_RecvQueueLock );
				dwProcStartTick = dwLockDataTick = _getTickCount();
			}
		}
		else dwLockDataTick = dwProcStartTick;

		//交换数据处理队列
		pRecvList = pRunData->m_pRecvAppendList;
		pRunData->m_pRecvAppendList = pRunData->m_pRecvProcList;
		pRunData->m_pRecvProcList = pRecvList;
		LeaveCriticalSection( &pRunData->m_RecvQueueLock );

		//如果没有待处理的数据则让出本次CPU时间
		nBufferCount = pRecvList->count();
		if ( !nBufferCount )
		{
			Sleep( 1 );
			continue;
		}

		ppRecvBufs = (PCLIENTRECVBUF*)(void**)(*pRecvList);
		while ( nBufferCount > 0 )
		{
			pRecvBuf = *ppRecvBufs;
			nBufferCount--;
			ppRecvBufs++;
			pSession = pRecvBuf->pSession;
			if ( pRecvBuf->nBufferSize > 0 )
			{
				//如果会话被关闭或效验过期则不予处理
				if ( pSession->nSocket != INVALID_SOCKET && pSession->nVerifyIdx == pRecvBuf->nVerify && !pSession->boMarkToClose && !pSession->boRemoteClosed )
				{
					pSession->dwClientMsgTick = dwProcStartTick;
					//OutputMsg( rmTip, "dwProcStartTick=%I64d",dwProcStartTick);
					pRunData->ProcessUserRecvPacket( pSession, (char*)(pRecvBuf + 1), pRecvBuf->nBufferSize );
					pSession->nRecvPacketCount++;
				}
			}
			free( pRecvBuf );
		}
		pRecvList->clear();
		pRunData->m_dwLastProcUsrMsgTime = _getTickCount() - dwProcStartTick;
		//循环小于指定时间则休眠一次
		//if ( pRunData->m_dwLastProcUsrMsgTime < 16 )
		if(TRUE)
		{
			Sleep ( 1 );
		}
	}
	ExitThread( 0 );
}

//发送数据的线程
#ifdef _MSC_VER
VOID STDCALL CRunDataProcesser::SendDataProcessRoutine(void* arg)
#else
VOID* CRunDataProcesser::SendDataProcessRoutine(void* arg)
#endif
{	
	PRUNDATASENDTHREAD pRunThread = (PRUNDATASENDTHREAD)arg;

	TICKCOUNT dwProcStartTick;
	CRunDataProcesser *pRunData;
	int nLockSendBufQLockFail = 0;
	pRunData = pRunThread->pRunData;
	(void)signal(SIGPIPE, SIG_IGN);
	
	while ( !pRunData->m_boStoping )
	{
		dwProcStartTick = _getTickCount();			
		if ( pRunData->CopyWaitSendBuffers( pRunThread, TRUE) )
			nLockSendBufQLockFail = 0;
		else nLockSendBufQLockFail ++;

		pRunData->CheckSendSessionBuffers( pRunThread );

		//循环小于指定时间则休眠一次
		pRunThread->dwProcTick = _getTickCount() - dwProcStartTick;
		//if ( pRunThread->dwProcTick < 20 )
		
		{			
			dwProcStartTick = _getTickCount();			
			Sleep(s_nSndThreadSleepTime);
			TICKCOUNT curTick = _getTickCount();			
			pRunThread->dwSleepTick = curTick - dwProcStartTick;
		}
		
		/*else 
			pRunThread->dwSleepTick = 0;*/
	}
	ExitThread( 0 );
}

//收到了服务器的数据,进行处理
//VOID CRunDataProcesser::ProcessRecvBuffers(const GateMsg* msg)
VOID CRunDataProcesser::ProcessRecvBuffers(const GATEMSGHDR* pHeader, LPCSTR pData, size_t nLen)
{
	Assert(pHeader->dwGateCode == RUNGATECODE);	
	/*if (msg->header.wIdent == GM_DATA)
		Assert(msg->header.wTemp == RUNTEMPCODE);*/
	if (s_nIgnoreDataPacket)
		return;
	//如果是关闭服务器的话
	if(pHeader->wIdent ==GM_CLOSE_SERVER)
	{
		OutputMsg(rmTip,"Back server close from back");
		g_ServerIsRunning =false;
		return;
	}
	else if (pHeader->wIdent == GM_APPBIGPACK)
	{
		char* pBuffEnd = (char *)pData + nLen;
		GATEMSGHDR* pCurHeader = (GATEMSGHDR *)pData;
		while ((char*)pCurHeader < pBuffEnd)
		{
			if (pCurHeader->dwGateCode != RUNGATECODE)
			{
				OutputMsg(rmError, _T("%s recv packet invalid..."), __FUNCTION__);
				break;
			}
			DispathRecvMessage(pCurHeader, (const char*)(pCurHeader+1), pCurHeader->nDataSize);
			pCurHeader = (GATEMSGHDR *)((char *)pCurHeader + (sizeof(GATEMSGHDR) + pCurHeader->nDataSize));
		}
	}
	else
		DispathRecvMessage((GATEMSGHDR * const)pHeader, pData, nLen);
	//DECLARE_FUN_TIME_PROF();
	//if (s_nIgnoreDataPacket)
	//{
	//	Inherited::ProcessRecvBuffers(pDataBuffer);
	//	SwapRecvProcessBuffers();
	//	return;
	//}
	//
	//INT_PTR nLen, nCheckMsgLen;
	//char *pBuf, *pMsgBuf;
	//PGATEMSGHDR pMsgHdr;
	//BOOL boTipNotRunGateCode;

	////如果连接已断开则丢弃所有数据
	//if ( !connected() )
	//{
	//	OutputMsg( rmWaning, _T("ProcessRecvBuffers,socket is close ,drop data") );
	//	Inherited::ProcessRecvBuffers(pDataBuffer);
	//	SwapRecvProcessBuffers();
	//	return;
	//}

	//nLen = 0;
	//pBuf = NULL;

	//pBuf = pDataBuffer->pPointer;
	//nLen = pDataBuffer->nOffset - (pBuf - pDataBuffer->pBuffer);
	////处理数据包的条件是接收缓冲的长度不少于一个网关通信消息头
	//if ( nLen >= sizeof(*pMsgHdr) )
	//{
	//	boTipNotRunGateCode = FALSE;
	//	while ( TRUE )
	//	{
	//		pMsgHdr = (PGATEMSGHDR)pBuf;
	//		//检查协议头是否有效（dwGateCode必须是RUNGATECODE）
	//		if ( pMsgHdr->dwGateCode == RUNGATECODE )
	//		{
	//			if (pMsgHdr->nDataSize >= 0 )
	//			{
	//				nCheckMsgLen = pMsgHdr->nDataSize + sizeof(*pMsgHdr);

	//				if ( nCheckMsgLen < 0x80000 )//丢弃长度大于0x8000的数据包
	//				{
	//					boTipNotRunGateCode = FALSE;
	//					if ( nLen < nCheckMsgLen )
	//						break;
	//					pMsgBuf = pBuf + sizeof(*pMsgHdr);
	//					DispathRecvMessage( pMsgHdr, pMsgBuf, pMsgHdr->nDataSize );
	//				}
	//				else Assert(0);
	//			}
	//			else
	//			{
	//				Assert(pMsgHdr->nDataSize >= 0);
	//				nCheckMsgLen = sizeof(*pMsgHdr);
	//			}
	//			pBuf += nCheckMsgLen;
	//			nLen -= nCheckMsgLen;
	//		}
	//		else
	//		{
	//			pBuf++;
	//			nLen--;
	//			if ( !boTipNotRunGateCode )
	//			{
	//				boTipNotRunGateCode = TRUE;
	//				OutputMsg( rmWaning, _T("%s droped a packed(not RUNGATECODE)"), _T(__FUNCTION__) );
	//			}
	//		}
	//		if ( nLen < sizeof(*pMsgHdr) )
	//			break;
	//	}

	//	pDataBuffer->pPointer = pBuf;
	//}
	//SwapRecvProcessBuffers();	
}


// 接收服务器数据的线程
/* 以前使用了管道,现在使用socket
 //xiaoql  2010-11-11日修改
VOID STDCALL CRunDataProcesser::RecvServerProcessRoutine(CRunDataProcesser *pRunData)
{
	
#pragma __CPMSG__("----------------基类修改为ClientSocket，则可以去掉这个函数以及对线程的创建")
	DWORD dwSize, dwBytesRead;
	HANDLE hPipe;
	PRUNGATEDATBUF pDataBuf;
	DWORD dwPipeConnectTick = 0;
	DWORD dwProcStartTick;

	pDataBuf = &pRunData->m_ServerBuf;

	while ( !pRunData->m_boStoping )
	{
		dwProcStartTick = _getTickCount();
		hPipe = pRunData->m_hPipe;
		if ( hPipe == INVALID_HANDLE_VALUE )
		{
			Sleep( 16 );
			if ( dwProcStartTick - dwPipeConnectTick >= 3000 )
			{
				if ( !pRunData->OpenServerPipe() )
					dwPipeConnectTick = _getTickCount();
				else OutputMsg( rmTip, "LogicServer Pipe conencted" );
			}
			continue;
		}

		//如果管道没有数据则让出处理器时间
		dwSize = GetFileSize( hPipe, NULL );
		if ( !dwSize )
		{
			Sleep( 1 );
			pRunData->m_dwProcSrvThreadSleep = _getTickCount() - dwProcStartTick;
			continue;
		}
		else if ( dwSize == INVALID_FILE_SIZE )
		{
			pDataBuf->nOffset = 0;
			pRunData->GotError( __FUNCTION__, "GetFileSize", GetLastError() );
			pRunData->ServerPipeError();
			continue;
		}

		pRunData->m_dwProcSrvThreadSleep = 0;

		//增长接受数据包
		if ( pDataBuf->nSize < pDataBuf->nOffset + (int)dwSize )
		{
			pDataBuf->nSize += (dwSize / 1024 + 1) * 1024;
			pDataBuf->lpBuffer = (char*)realloc( pDataBuf->lpBuffer, pDataBuf->nSize );
		}

		//循环读取数据
		while ( TRUE )
		{
			if ( !ReadFile( hPipe, &pDataBuf->lpBuffer[pDataBuf->nOffset], dwSize, &dwBytesRead, NULL ) )
			{
				pDataBuf->nOffset = 0;
				pRunData->GotError( __FUNCTION__, "ReadFile", GetLastError() );
				pRunData->ServerPipeError();
				break;
			}
			pDataBuf->nOffset += dwBytesRead;
			if ( dwBytesRead >= dwSize )
				break;
		}
		
		InterlockedExchange( (LONG*)&pRunData->m_dwLastRecvSrvMsgTime, _getTickCount() - dwProcStartTick );
		InterlockedExchangeAdd( (LONG*)&pRunData->m_dwRecvSeverSize, dwSize );

		//处理数据
		if ( pDataBuf->nOffset )
		{
			pRunData->ProcessServerPacket();
		}
	}
	ExitThread( 0 );
}
*/


VOID CRunDataProcesser::DispathRecvMessage(const PGATEMSGHDR pMsgHdr, const char *pBuffer, SIZE_T nBufSize)
{
		PRUNGATEUSERSESSION pSession;
		/*
		if(GM_DATA ==pMsgHdr->wIdent)
		{
			OutputMsg(rmTip,  _T("收到后台数据,cmd=%d,wSessionIdx=%d,wServerIdx=%d,socket=%d\n"), 
				pMsgHdr->wIdent, pMsgHdr->wSessionIdx,pMsgHdr->wServerIdx,pMsgHdr->nSocket);
		}
		*/
		switch ( pMsgHdr->wIdent )
		{
		case GM_CLOSE:
			if ( pMsgHdr->wSessionIdx < m_nMaxSessionCount )
			{
				pSession = &m_Sessions[pMsgHdr->wSessionIdx];
	
				if ( pSession->nSocket == pMsgHdr->nSocket && !pSession->boMarkToClose && !pSession->boRemoteClosed )
				{
					SendCloseSession( pSession, FALSE, enReasonGM  );
				}
				else 
				{
					OutputMsg( rmWaning, _T("GM_CLOSE socket不匹配  wSessionIdx=%d,本地Socket=%d,后台的为%d"),
						pMsgHdr->wSessionIdx, pSession->nSocket, pMsgHdr->nSocket );
				}
			
			}
			break;
		case GM_CHECKCLIENT:
			{
				// 保存服务器的时间
				if (m_llSendKeepAliveTime > 0)
				{
					if (m_nCheckTimeCount < 10)
					{				
						TICKCOUNT nTimeDiff = pMsgHdr->tickCount - _getTickCount();
						m_llOccuTimeDiff += nTimeDiff;				
						m_nCheckTimeCount++;
						m_llSvrTimeDiff = (long long)(m_llOccuTimeDiff / (float)m_nCheckTimeCount);
						OutputMsg(rmNormal, _T("timediff=%lld,result dif=%lld,checkTimecount=%d"), 
							nTimeDiff, 
							m_llSvrTimeDiff, 
							m_nCheckTimeCount);
					}
				}
			
			
				/*OutputMsg(rmNormal, _T("------logicSvrTime=%I64d  gateTime=%I64d, timeDif=%I64d"), pMsgHdr->tickCount, nCurrTick, m_llSvrTimeDiff);*/
			}
			break;
		case GM_APPKEEPALIVE:
			{
				// 给服务器会应答消息			
				pSession = &m_Sessions[pMsgHdr->wSessionIdx];
				if ( pSession->nSocket == pMsgHdr->nSocket && !pSession->boMarkToClose && !pSession->boRemoteClosed )
				{
					SendKeepAliveAck( pSession, pBuffer, nBufSize);
				}
				break;
			}
		case GM_SERVERUSERINDEX:
			if ( pMsgHdr->wSessionIdx < m_nMaxSessionCount )
			{
				pSession = &m_Sessions[pMsgHdr->wSessionIdx];
				if ( pSession->nSocket == pMsgHdr->nSocket && !pSession->boMarkToClose && !pSession->boRemoteClosed )
				{
					pSession->nServerIdx = pMsgHdr->wServerIdx;
					OutputMsg(rmNormal, _T("recv GM_ServerIndex msg. socket=%d, sessionId=%d, set serverIdx=%d"), 
						(int)pSession->nSocket, (int)pSession->nIndex, pSession->nServerIdx);
				}
				else
				{
					OutputMsg(rmError, _T("GM_ServerIndex Error, session closed or socket unmatch(session socket=%d, sessionId=%d, msghdr socket=%d"),
						(int)pSession->nSocket, (int)pSession->nIndex, (int)pMsgHdr->nSocket);
				}
			}
			break;
		
		case GM_DATA:
			if ( pMsgHdr->wSessionIdx < m_nMaxSessionCount )
			{
				pSession = &m_Sessions[pMsgHdr->wSessionIdx];
				if ( !pSession->boMarkToClose && !pSession->boRemoteClosed )
				{
					if ( pSession->nSocket == pMsgHdr->nSocket )
					{
						PostUserServerData( pSession, pBuffer, (INT)nBufSize, TRUE, TRUE, (TICKCOUNT)pMsgHdr->tickCount);
					}
					else
					{
						OutputMsg( rmWaning, "GM_DATA 会话(%d:%s)套接字不匹配，data Socket=%d,local socket=%d", pMsgHdr->wSessionIdx, pSession->sLoginCharName, pMsgHdr->nSocket,pSession->nSocket );
						//SendCloseSession((SOCKET)pMsgHdr->nSocket); //如果套接字不匹配
						//SendCloseSession( pSession, TRUE, enReasonAllSessionClose  );
					}
				
				}
				else 
				{
					OutputMsg( rmWaning, "GM_DATA 会话(%d:%s,%d,%d)已关闭", pMsgHdr->wSessionIdx, pSession->sLoginCharName,
						(int)(pSession->boMarkToClose),(int)(pSession->boRemoteClosed));
					//这种情况下再次向逻辑发送关闭连接
					//SendCloseSession((SOCKET)pMsgHdr->nSocket);
				}
			}
			else OutputMsg( rmWaning, "GM_DATA 会话ID无效(%d)", pMsgHdr->wSessionIdx );
			break;
			
		default:
			{
				GotError( __FUNCTION__, _T("无效的消息序列"), pMsgHdr->wIdent );
			}
			break;
		}
	
}

VOID CRunDataProcesser::OnConnected()
{
	Inherited::OnConnected();
	m_llSendKeepAliveTime = 0;
	m_nCheckTimeCount = 0;
	m_llOccuTimeDiff = 0;
	OutputMsg(rmTip, _T("BackServer Connected"));
}

VOID CRunDataProcesser::OnClosed()
{
	Inherited::OnClosed();
	CloseAllSessions(true);
}

VOID CRunDataProcesser::OnError(int errorCode)
{
	OutputError(errorCode, _T("BackServer Socket Error "));
}

VOID CRunDataProcesser::OnDisconnected()
{
	m_llSvrTimeDiff		= 0;
	m_nCheckTimeCount	= 0;
	m_llOccuTimeDiff	= 0;	
	OutputMsg(rmWaning, _T("BackServer Connection Closed"));
}

CRunDataProcesser::CRunDataProcesser(const int nMaxSessionCount)
	:Inherited()
{
	m_nActiveUser = 0;
	m_boStoping	  = FALSE;
	m_boStarted	  = FALSE;

	m_DataProcessType	= dp_Default;

	/*
	m_hPipe = INVALID_HANDLE_VALUE;
	strcpy( m_sPipeName, "\\\\.\\pipe\\default" );

	sprintf( m_sGateName, "游戏网关%d", _getTickCount() % 100 );
	*/

	m_hProcRecvThread	= NULL;
	ZeroMemory( m_SendThreads, sizeof(m_SendThreads) );

	m_nSendThreadCount	= 2;
	m_nMaxSessionCount	= nMaxSessionCount;  // 默认值: 16384
	m_Sessions	= NULL;
	m_pSockProcesser = NULL;

	InitializeCriticalSection( &m_SessionLock );
	if ( m_Sessions ) ZeroMemory( m_Sessions, sizeof(m_Sessions) );


	InitializeCriticalSection( &m_RecvQueueLock );
	m_pRecvAppendList = &m_RecvQueue[0];
	m_pRecvProcList	  = &m_RecvQueue[1];

	m_ServerBuf.lpBuffer= NULL;
	m_ServerBuf.nSize	= 0;
	m_ServerBuf.nOffset	= 0;

	m_nUserVerify		= 1;
	m_dwProcessRecvSize = 0;

	m_boPrintC2SMsg = FALSE;
	m_boPrintS2CMsg = FALSE;

	m_llSvrTimeDiff = 0;
	m_nCheckTimeCount = 0;
	m_llOccuTimeDiff = 0;
	InitSendThreadData();
	//m_DataPacker.SetNeedEncrypt(true); //需要加密
}

CRunDataProcesser::~CRunDataProcesser()
{
	Stop();
	UninitSendThreadData();
	DeleteCriticalSection( &m_RecvQueueLock );
	DeleteCriticalSection( &m_SessionLock );
}

VOID CRunDataProcesser::InitSessions()
{
	int i;
	PRUNGATEUSERSESSION pSession;

	EnterCriticalSection( &m_SessionLock );
	if ( !m_Sessions )
	{
		m_Sessions = (PRUNGATEUSERSESSION)malloc(sizeof(m_Sessions[0]) * m_nMaxSessionCount);
		ZeroMemory( m_Sessions, sizeof(m_Sessions[0]) * m_nMaxSessionCount );
	}
	pSession = m_Sessions;
	for ( i=0; i<m_nMaxSessionCount; ++i )
	{
		pSession->nIndex	= i;
		pSession->nSocket	= INVALID_SOCKET;
#ifdef WIN32
		pSession->Overlapped.pUser = pSession;
#endif
		pSession->nServerIdx		= 0;
		pSession->btPacketIdx		= 0;
		pSession->wPacketError		= 0;
		pSession->nRecvPacketCount	= 0;
		pSession->nSendPacketCount	= 0;
		pSession->boMarkToClose		= false;
		pSession->boRemoteClosed	= false;
		pSession->boSendAvaliable	= true;
		INIT_SESSION_SEND( pSession );
		pSession++;
	}
	LeaveCriticalSection( &m_SessionLock );
}

VOID CRunDataProcesser::UninitSessions()
{
	int i;
	PRUNGATEUSERSESSION pSession;

	if ( !m_Sessions ) return;
	EnterCriticalSection( &m_SessionLock );
	pSession = m_Sessions;
	for ( i=0; i<m_nMaxSessionCount; ++i )
	{
		if ( pSession->RecvBuf.lpBuffer )
		{
			free( pSession->RecvBuf.lpBuffer );
			pSession->RecvBuf.lpBuffer = NULL;
			pSession->RecvBuf.nSize = pSession->RecvBuf.nOffset = 0;
		}
		if ( pSession->SendBuf.lpBuffer )
		{
			free( pSession->SendBuf.lpBuffer );
			pSession->SendBuf.lpBuffer = NULL;
			pSession->SendBuf.nSize = pSession->SendBuf.nOffset = 0;
		}
		UNINIT_SESSION_SEND( pSession );
		pSession++;
	}
	free( m_Sessions );
	m_Sessions = NULL;
	LeaveCriticalSection( &m_SessionLock );
}

VOID CRunDataProcesser::FreeRecvBuffers()
{
	INT_PTR i;
	void **pBuffers;
	EnterCriticalSection( &m_RecvQueueLock );

	pBuffers = *m_pRecvAppendList;
	for ( i=m_pRecvAppendList->count()-1; i>-1; --i )
	{
		free( pBuffers[i] );
	}
	m_pRecvAppendList->clear();

	
	pBuffers = *m_pRecvProcList;
	for ( i=m_pRecvProcList->count()-1; i>-1; --i )
	{
		free( pBuffers[i] );
	}
	m_pRecvProcList->clear();

	LeaveCriticalSection( &m_RecvQueueLock );
}

VOID CRunDataProcesser::FreeSendBuffers(PRUNDATASENDTHREAD pSendThread)
{
	INT_PTR i;
	void **pBuffers;
	EnterCriticalSection( &pSendThread->SendQueueLock );

	pBuffers = *pSendThread->pSendAppendList;
	for ( i=pSendThread->pSendAppendList->count()-1; i>-1; --i )
	{
		free( pBuffers[i] );
	}
	pSendThread->pSendAppendList->clear();

	
	pBuffers = *pSendThread->pSendProcList;
	for ( i=pSendThread->pSendProcList->count()-1; i>-1; --i )
	{
		free( pBuffers[i] );
	}
	pSendThread->pSendProcList->clear();

	LeaveCriticalSection( &pSendThread->SendQueueLock );
}
/*
BOOL CRunDataProcesser::OpenServerPipe()
{
	int nErr;

	while ( TRUE )
	{
		m_hPipe = CreateFile( m_sPipeName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL );
		if ( m_hPipe != INVALID_HANDLE_VALUE )
			return TRUE;

		nErr = GetLastError();
		if ( nErr != ERROR_PIPE_BUSY )
		{
			GotError( __FUNCTION__, "CreateFile", nErr );
			break;
		}
		
		if ( !WaitNamedPipe( m_sPipeName, 10000 ) )
		{
			nErr = GetLastError();
			GotError( __FUNCTION__, "WaitNamedPipe", nErr );
			break;
		}
	}

	return FALSE;
}



VOID CRunDataProcesser::CloseServerPipe()
{
	if ( m_hPipe != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_hPipe );
		m_hPipe = INVALID_HANDLE_VALUE;
	}
}

VOID CRunDataProcesser::ServerPipeError()
{
	if ( m_hPipe != INVALID_HANDLE_VALUE )
	{
		CancelRemainSendSessionBuffers();
		CloseHandle( m_hPipe );
		m_hPipe = INVALID_HANDLE_VALUE;
		//CloseAllSessions 必须在CloseHandle之后调用，否则CloseAllSessions->SendServerMessage->ServerPipeError 造成栈溢出
		CloseAllSessions( TRUE );
	}
}
*/

BOOL CRunDataProcesser::CopyWaitSendBuffers(PRUNDATASENDTHREAD pSendThread, BOOL boForceCopy)
{
	PSENDCLIENTBUF pSendBuf, *ppBufers;
	CBaseList<LPVOID> *pBufferList;
	PRUNGATEUSERSESSION pSession;
	INT_PTR nCount, nSize, nTotalSize, nAppendSize;
	TICKCOUNT dwMsgTick;

	if ( !boForceCopy )
		boForceCopy = TryEnterCriticalSection( &pSendThread->SendQueueLock );
	else EnterCriticalSection( &pSendThread->SendQueueLock );
	
	if ( boForceCopy )
	{
		pBufferList = pSendThread->pSendAppendList;
		pSendThread->pSendAppendList = pSendThread->pSendProcList;
		pSendThread->pSendProcList = pBufferList;
		LeaveCriticalSection( &pSendThread->SendQueueLock );

		nCount = pBufferList->count();
		if ( nCount > 0 )
		{
			dwMsgTick = _getTickCount();
			ppBufers = (PSENDCLIENTBUF*)(void**)(*pBufferList);
			nTotalSize = 0;
			nAppendSize = 0;
			do
			{
				pSendBuf = *ppBufers;
				ppBufers++;
				nCount--;

				pSession = pSendBuf->pSession;
				if ( pSession->nSocket != INVALID_SOCKET && !pSession->boMarkToClose && !pSession->boRemoteClosed )
				{
					nSize = pSession->SendBuf.nOffset + pSendBuf->nBufferSize + 1;
					if ( pSession->SendBuf.nSize < nSize )
					{
						nSize = (nSize / SESSION_DATAGROW_SIZE + 1) * SESSION_DATAGROW_SIZE;
						pSession->SendBuf.nSize = (INT)nSize;
						pSession->SendBuf.lpBuffer = (char*)realloc( pSession->SendBuf.lpBuffer, nSize );
					}
					memcpy( &pSession->SendBuf.lpBuffer[pSession->SendBuf.nOffset], pSendBuf + 1, pSendBuf->nBufferSize );
					pSession->SendBuf.nOffset += pSendBuf->nBufferSize;
					pSession->SendBuf.lpBuffer[pSession->SendBuf.nOffset] = 0;
					pSession->dwServerMsgTick = dwMsgTick;
					nAppendSize += pSendBuf->nBufferSize;
				}
				nTotalSize += pSendBuf->nBufferSize;
				free( pSendBuf );
			}
			while ( nCount > 0 );
			pBufferList->clear();

			// 原子操作
			InterlockedExchangeAdd( (LONG*)&m_dwWaitSendUserSize, (LONG)nAppendSize );
			InterlockedExchangeAdd( (LONG*)&m_dwWaitSendQueueSize, -(INT)nTotalSize );
		}
	}

	return boForceCopy;
}

VOID CRunDataProcesser::GetSessionLoginInfo(PRUNGATEUSERSESSION pSession, char *sBuffer, int nBufferSize)
{
}


VOID CRunDataProcesser::GotError(LPCSTR sErrFn, LPCSTR sErrApi, int nErr)
{
	OutputMsg( rmError, _T("%s Got Error %d on api \"%s\""), sErrFn, nErr, sErrApi );
}

VOID CRunDataProcesser::InitSendThreadData()
{
	int i;

	for ( i=0; i<m_nSendThreadCount; ++i )
	{
		InitializeCriticalSection( &m_SendThreads[i].SendQueueLock );
		m_SendThreads[i].pRunData = this;
		m_SendThreads[i].nThreadIdx = i;
		m_SendThreads[i].boSendEWouldBlock = false;
		m_SendThreads[i].boSendFewBuffer = false;
		m_SendThreads[i].pSendAppendList = new CBaseList<LPVOID>();
		m_SendThreads[i].pSendProcList = new CBaseList<LPVOID>();		
	}
}

VOID CRunDataProcesser::UninitSendThreadData()
{
	int i;

	for ( i=0; i<m_nSendThreadCount; ++i )
	{
		SafeDelete( m_SendThreads[i].pSendAppendList );
		SafeDelete( m_SendThreads[i].pSendProcList );
		DeleteCriticalSection( &m_SendThreads[i].SendQueueLock );
	}
}

VOID CRunDataProcesser::SendCheck(int nIdent)
{
	SendServerMessage( nIdent, 0, 0, 0, m_sGateName, (int)strlen(m_sGateName) + 1 );
}

VOID CRunDataProcesser::SendGateUserInfo()
{
	GATEUSERINFO gi;
	gi.nUserCount = GetActiveUserCount();
	gi.nGatePort = GetRunSockProcesser()->GetBindPort();
	SendServerMessage(GM_GATEUSERINFO, 0, 0, 0, (char *)&gi, sizeof(GATEUSERINFO));
}

VOID CRunDataProcesser::SendOpenSession(PRUNGATEUSERSESSION pSession)
{
	char sRemoteAddr[32];
	strcpy( sRemoteAddr, inet_ntoa( pSession->SockAddr.sin_addr ) );
	SendServerMessage( GM_OPEN, pSession->nIndex, pSession->nSocket, 0, sRemoteAddr, (int)strlen(sRemoteAddr) + 1 );
	OutputMsg(rmTip, _T("SendOpenSession: socket=%d sessionId=%d, remoteAddr=%s"), (int)pSession->nSocket, (int)pSession->nIndex, sRemoteAddr);
}

VOID CRunDataProcesser::SendCloseSession(PRUNGATEUSERSESSION pSession, BOOL boCloseOnServer, INT nReason)
{
	PCLIENTRECVBUF pRecvBuf;
	int nSocket;

	if ( !pSession->boMarkToClose && pSession->nSocket != INVALID_SOCKET )
	{
		OutputMsg( rmWaning, "关闭会话 socket=%d, sessionId=%d, serverId=%d,reasonId=%d,nVerifyIdx=%ld", (int)pSession->nSocket, 
			(int)pSession->nIndex, (int)pSession->nServerIdx, nReason, pSession->nVerifyIdx );
		
		pSession->boMarkToClose = true;

		nSocket = (int)pSession->nSocket;
		pSession->nSocket = INVALID_SOCKET;
		pSession->dwCloseTick = _getTickCount();
		pSession->nVerifyIdx = 0;
		bool isInCommu =( pSession->nSessionSatus == enCommunication); //当前是否在通信状态
		pSession->nSessionSatus = enSessionStatusIdle;  //初始化状态

		if (nSocket!=INVALID_SOCKET) closesocket( nSocket );

		//如果需要向服务器发送关闭用户的消息则将消息添加到队列
		//并且需要通信状态了，否则不需要发包，说明还没有通过验证
		if ( boCloseOnServer  && isInCommu )
		{
			SendServerMessage( GM_CLOSE, 0, nSocket, 0, NULL, 0 );
		}
		m_nActiveUser--;
	}
}

VOID CRunDataProcesser::SendKeepAliveAck(PRUNGATEUSERSESSION pSession, const char *pBuffer, SIZE_T nBufSize)
{	
	SendServerMessage(GM_APPKEEPALIVEACK, pSession->nIndex, pSession->nSocket, pSession->nServerIdx, (char *)pBuffer, (int)nBufSize);
}


VOID CRunDataProcesser::CloseAllSessions(BOOL boForceClose)
{
	int i;
	TICKCOUNT dwCurTick;
	PRUNGATEUSERSESSION pSession;

	if ( !m_Sessions ) return;
	pSession = m_Sessions;
	EnterCriticalSection( &m_SessionLock );
	EnterCriticalSection( &m_RecvQueueLock );
	for ( i=0; i<m_nSendThreadCount; ++i )
	{
		EnterCriticalSection( &m_SendThreads[i].SendQueueLock );
	}
	Sleep( 3000 );//休眠以等待数据接收以及各发送线程被阻塞

	dwCurTick = _getTickCount();
	for ( i=0; i<m_nMaxSessionCount; ++i )
	{
		if ( pSession->nSocket != INVALID_SOCKET )
		{
			if ( boForceClose )
			{
				OutputMsg(rmTip,_T("CRunDataProcesser::CloseAllSessions close socket=%d"),(int)pSession->nSocket);
				SendCloseSession( pSession, TRUE, enReasonAllSessionClose  );
				//closesocket( pSession->nSocket );
				pSession->boMarkToClose = true;
				pSession->boRemoteClosed = true;
				pSession->dwCloseTick = 0;
				pSession->nSocket = INVALID_SOCKET;
				pSession->nVerifyIdx = 0;
				m_nActiveUser--;

				if ( pSession->RecvBuf.lpBuffer )
				{
					free( pSession->RecvBuf.lpBuffer );
					pSession->RecvBuf.lpBuffer = NULL;
				}
				pSession->RecvBuf.nSize = pSession->RecvBuf.nOffset = 0;
				if ( pSession->SendBuf.lpBuffer )
				{
					free( pSession->SendBuf.lpBuffer );
					pSession->SendBuf.lpBuffer = NULL;
				}
				pSession->SendBuf.nSize = pSession->SendBuf.nOffset = 0;
			}
			else SendCloseSession( pSession, TRUE, enReasonAllSessionClose  );
		}
		pSession++;
	}

	for ( i=0; i<m_nSendThreadCount; ++i )
	{
		LeaveCriticalSection( &m_SendThreads[i].SendQueueLock );
	}
	LeaveCriticalSection( &m_RecvQueueLock );
	LeaveCriticalSection( &m_SessionLock );
}

VOID CRunDataProcesser::CancelRemainSendSessionBuffers()
{
	PSENDCLIENTBUF pSendBuf, *ppBufers;
	CBaseList<LPVOID> *pBufferList;
	INT_PTR i, nCount, nTotalSize;

	nTotalSize = 0;
	for ( i=0; i<m_nSendThreadCount; ++i )
	{
		EnterCriticalSection( &m_SendThreads[i].SendQueueLock );
		
		pBufferList = m_SendThreads[i].pSendProcList;
		nCount = pBufferList->count();
		if ( nCount )
		{
			ppBufers = (PSENDCLIENTBUF*)(void**)(*pBufferList);
			do
			{
				pSendBuf = *ppBufers;
				ppBufers++;
				nCount--;
				nTotalSize += pSendBuf->nBufferSize;
				free( pSendBuf );
			}
			while ( nCount > 0 );
			pBufferList->clear();
		}

		pBufferList = m_SendThreads[i].pSendAppendList;
		nCount = pBufferList->count();
		if ( nCount )
		{
			ppBufers = (PSENDCLIENTBUF*)(void**)(*pBufferList);
			do
			{
				pSendBuf = *ppBufers;
				ppBufers++;
				nCount--;
				nTotalSize += pSendBuf->nBufferSize;
				free( pSendBuf );
			}
			while ( nCount > 0 );
			pBufferList->clear();
		}

		LeaveCriticalSection( &m_SendThreads[i].SendQueueLock );
	}

	InterlockedExchangeAdd( (LONG*)&m_dwWaitSendQueueSize, -(INT)nTotalSize );
}

BOOL CRunDataProcesser::CheckCloseSessions(BOOL boForceCheck)
{
	static const DWORD dwCloseIdleSessionLong = 10 * 60 * 1000;
	int i, boResult;
	TICKCOUNT dwCurTick;
	PRUNGATEUSERSESSION pSession;

	if ( boForceCheck )
	{
		EnterCriticalSection( &m_SessionLock );
		boResult = TRUE;
	}
	else  boResult = TryEnterCriticalSection( &m_SessionLock );

	if ( boResult )
	{
		dwCurTick = _getTickCount();
		pSession = m_Sessions - 1;

		for ( i=0; i<m_nMaxSessionCount; ++i )
		{
			pSession++;

			if ( pSession->boMarkToClose || pSession->boRemoteClosed )
			{
				//加上pSession->dwCloseTick非0判断，防止内存错误
				//可能的原因是标记pSession->boMarkToClose或pSession->boRemoteClosed为true的线程尚未来得及
				//给pSession->dwCloseTick赋值，而此线程恰好已经执行到此处对pSession的资源进行释放，从而
				//造成内存错误。
				if ( pSession->dwCloseTick && dwCurTick - pSession->dwCloseTick >= 10 * 1000 )
				{
					if(pSession->nSocket != INVALID_SOCKET)
					{
						OutputMsg( rmWaning, "Socket%d 标记为删除",(int)pSession->nSocket );
					}
					
					LOCK_SESSION_SEND( pSession );
					pSession->nSocket = INVALID_SOCKET;
					pSession->nVerifyIdx = 0;

					if ( pSession->RecvBuf.lpBuffer )
					{
						free( pSession->RecvBuf.lpBuffer );
						pSession->RecvBuf.lpBuffer = NULL;
					}

					pSession->RecvBuf.nSize = pSession->RecvBuf.nOffset = 0;
					if ( pSession->SendBuf.lpBuffer )
					{
						free( pSession->SendBuf.lpBuffer );
						pSession->SendBuf.lpBuffer = NULL;
					}
					pSession->SendBuf.nSize = 0;//SendBuf.nOffset值由发送线程维护

					pSession->boMarkToClose = false;
					pSession->boRemoteClosed = false;
					pSession->dwCloseTick = 0;
					UNLOCK_SESSION_SEND( pSession );
				}
			}
			else if ( pSession->nSocket != INVALID_SOCKET )
			{
				if (dwCurTick - pSession->dwClientMsgTick >= dwCloseIdleSessionLong)
				{
					OutputMsg( rmWaning, "客户端Idle，dwCurTick=%ld,dwClientMsgTick=%ld",(long long)dwCurTick, (long)pSession->dwClientMsgTick);
					SendCloseSession( pSession, TRUE, enReasonIdle   );
				}
				if(dwCurTick - pSession->dwServerMsgTick >= dwCloseIdleSessionLong)
				{
					//OutputMsg( rmWaning, "服务器较长时间处于非活动状态的会话，关闭会话" );
					//OutputMsg( rmWaning, "服务器端idle，dwCurTick=%I64d,dwClientMsgTick=%I64d",(long long)dwCurTick, (long long)pSession->dwServerMsgTick);					
					//SendCloseSession( pSession, TRUE, enReasonIdle   );
				}
			}
		}
		LeaveCriticalSection( &m_SessionLock );
	}

	return boResult;
}

VOID CRunDataProcesser::CheckSendSessionBuffers(PRUNDATASENDTHREAD pSendThread)
{
	DECLARE_FUN_TIME_PROF();
	static char sSendCheckBuf[RUNGATE_ONCESENDUSE_LIMIT];

	int nErr, nRemainSize;
	char *pBuffer;
	PRUNGATEUSERSESSION pSession = &m_Sessions[pSendThread->nThreadIdx];

	pSendThread->boSendEWouldBlock = false;
	pSendThread->boSendFewBuffer = false;

	for ( ;pSession <= &m_Sessions[m_nMaxSessionCount-1]; pSession += m_nSendThreadCount )
	{
		if ( pSession->nSocket != INVALID_SOCKET && !pSession->boMarkToClose && !pSession->boRemoteClosed )
		{
			if ( !pSession->boSendAvaliable )
			{
				if ( _getTickCount() >= pSession->dwSendTimeOut )
				{
					pSession->boSendAvaliable = true;
					pSession->dwSendTimeOut = 0;
				}
				else continue;
			}

			if ( TRYLOCK_SESSION_SEND( pSession ) )
			{
				nRemainSize = pSession->SendBuf.nOffset;
				if ( nRemainSize > 4096 * 1024 )
				{
					SendCloseSession( pSession, TRUE, enReasonSenderBuffLarge   );
					nRemainSize = 0;
					OutputMsg( rmWaning, "关闭了一个发送数据队列大于4MB的连接。" );
				}
				if ( nRemainSize )
				{
					pBuffer = pSession->SendBuf.lpBuffer;					
					if ( nRemainSize >= MAXINT )//不让它执行这段代码
					{
						sSendCheckBuf[0] = '*';
						memcpy( &sSendCheckBuf[1], pBuffer, RUNGATE_ONCESENDUSE_LIMIT -1 );
						nErr = ::send( pSession->nSocket, sSendCheckBuf, RUNGATE_ONCESENDUSE_LIMIT, 0 );
						if ( nErr > 0 )
						{
							if ( nErr != RUNGATE_ONCESENDUSE_LIMIT )
								pSendThread->boSendFewBuffer = true;
							InterlockedExchangeAdd( (LONG*)&m_dwWaitSendUserSize, -(nErr - 1) );
							InterlockedExchangeAdd( (LONG*)&m_dwSendUserSize, nErr );
							nRemainSize -= nErr - 1;
							memmove( pBuffer, &pBuffer[nErr - 1], nRemainSize );
							pSession->SendBuf.nOffset = nRemainSize;
							pBuffer[nRemainSize] = 0;
							pSession->nSendPacketCount++;
						}
						else if ( !nErr || WSAGetLastError() != WSAEWOULDBLOCK )
						{
							InterlockedExchangeAdd( (LONG*)&m_dwWaitSendUserSize, -pSession->SendBuf.nOffset );
							pSession->boRemoteClosed = true;
							pSession->SendBuf.nOffset = 0;
							SendCloseSession( pSession, TRUE, enReasonSocketError  );
						}
						else 
						{
							pSession->boSendAvaliable = false;
							pSession->dwSendTimeOut = _getTickCount() + RUNGATE_SENDCHECK_TIMEOUT;
							pSendThread->boSendEWouldBlock = true;
						}
					}
					else
					{
						TICKCOUNT nCurTick = _getTickCount();
						if (nCurTick < pSession->nLastSendPacketTime)
							pSession->nLastSendPacketTime = 0;
						if (nCurTick - pSession->nLastSendPacketTime >= 15)
						{
							pSession->nLastSendPacketTime = nCurTick;
							nErr = ::send( pSession->nSocket, pBuffer, nRemainSize, 0 );
							if ( nErr > 0 )
							{	
								pSession->nSendPacketCount++;
								InterlockedExchangeAdd( (LONG*)&m_dwWaitSendUserSize, -nErr );
								InterlockedExchangeAdd( (LONG*)&m_dwSendUserSize, nErr );
								if ( nErr < nRemainSize )
								{
									pSendThread->boSendFewBuffer = true;
									memmove( pBuffer, &pBuffer[nErr], nRemainSize - nErr );
									nRemainSize -= nErr;
									pBuffer[nRemainSize] = 0;
									pSession->SendBuf.nOffset = nRemainSize;
									pSession->nTimeStampOffset = nRemainSize;
								}
								else
								{
									pBuffer[0] = 0;
									pSession->SendBuf.nOffset = 0;
									pSession->nTimeStampOffset = 0;
								}

#ifdef _DEBUG
								char * pIp = inet_ntoa(pSession->SockAddr.sin_addr);
								OutputMsg( rmTip, "[%s]Idx=%d: 发到客户端, 已发长度%d, 剩余待发%d;", 
									pIp, pSession->nVerifyIdx, nErr, pSession->SendBuf.nOffset);
#endif
							}
							else
							{
								int nErrorId = WSAGetLastError();
								if ( !nErr || nErrorId != WSAEWOULDBLOCK )
								{
									OutputMsg(rmWaning,"Socket =%d,send fail, errorid=%d ",(int)pSession->nSocket,nErrorId);

									InterlockedExchangeAdd( (LONG*)&m_dwWaitSendUserSize, -nRemainSize );
									InterlockedExchangeAdd( (LONG*)&m_dwSendUserSize, nRemainSize );
									pBuffer[0] = 0;
									pSession->SendBuf.nOffset = 0;
									pSession->boRemoteClosed = true;
									SendCloseSession( pSession, TRUE, enReasonSenderBuffLarge   );
								}
								else
								{
									pSession->boSendAvaliable = false;
									pSession->dwSendTimeOut = _getTickCount() + RUNGATE_SENDCHECK_TIMEOUT;
									pSendThread->boSendEWouldBlock = true;
								}
							}
						}
					}
				}
				UNLOCK_SESSION_SEND( pSession );
			}
		}
		else
		{
			//会话关闭后减少待发送数据统计值
			if ( nRemainSize = pSession->SendBuf.nOffset )
			{
				InterlockedExchangeAdd( (LONG*)&m_dwWaitSendUserSize, -nRemainSize );
				pSession->SendBuf.nOffset = 0;
			}
		}
	}




}
//发送数据到服务器
VOID CRunDataProcesser::SendServerMessage(int nIdent, int nSessionIdx, SOCKET nSocket, int nServerIdx, char *pBuffer, int nBufSize)
{	
	if ( !connected() )
		return;

	PutMessage(nIdent, nSessionIdx, nSocket, nServerIdx, m_llSvrTimeDiff, pBuffer, nBufSize);
}

/*
* 解包数据
* param[] pInBuff 输入的数据包
* param[] nInSize 输入的数据包的长度
* param[] pOutBuff 输出的数据包指针
* param[] nOutSize 输出的数据包的长度
* param[] nInProcessSize  输入的数据包处理了的长度，如果大于0，那么前面的这个字节的数据包可以丢弃了
* param[] nOutProcessSize 处理好的数据长度，处理好的数据放在pOutBuff里
* ret  如果进行了数据处理返回true，否则返回false。数据处理包括发现前面有无效数字，拷贝了一个完整的数据包，丢弃一些字节等
*/
static bool DataUnpack(PRUNGATEUSERSESSION pSession,
	char* pInBuff, 	SIZE_T nInSize, 
	char* pOutBuff, SIZE_T nOutSize, 
	SIZE_T& nInProcessSize,	SIZE_T& nOutProcessSize)
{
	nInProcessSize =0;
	nOutProcessSize =0;
	SIZE_T nHeaderSize = sizeof(DATAHEADER); 
	if (nInSize < nHeaderSize ) return false; //输入的数据不够
	if (NULL == pInBuff || NULL == pOutBuff) return false;

	bool tagFlag =false; //tag的寻找标志
	SIZE_T i =0;
	WORD * pTagPos =NULL;
	
	for ( ;i < nInSize -1; i++)
	{
		pTagPos = (WORD *)(pInBuff + i);
		if(*pTagPos == pSession->nKey) //如果找到了tag
		{
			tagFlag = true; //找到了头
			break;
		} 			 
	}

	if (tagFlag ==false )
	{
		i ++;
	}
	nInProcessSize = i; //前面的几个字节，如果不是头，将说明是垃圾数据，将被干掉
	SIZE_T nLeftCount = nInSize  - i; //还有多少个字节再后面
	//剩下的数据不够头，或者没有找到头
	if (tagFlag ==false || nLeftCount  < nHeaderSize ) 
	{
		if (nInProcessSize >0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	DATAHEADER data; //把数据拷贝一份过来
	memcpy( (void *)&data,(void*) (pInBuff + i),nHeaderSize ); //拷贝一份
	
	PDATAHEADER pHeader = 	&data; //把后面的转换为数据头的指针

	SIZE_T nTotalPackSize =  pHeader->len + nHeaderSize ;  //一个完整的数据包的需要的长度

	if ( nTotalPackSize  >  nLeftCount ) return false; //数据不齐全，没有len个字节发送过来
	else
	{
		//拷贝数据
		memcpy(pOutBuff,(void *)(pInBuff + i ),nTotalPackSize)	;
		nInProcessSize = i + nTotalPackSize;
		nOutProcessSize = nTotalPackSize;
	}
	return true;
}

//收到用户的数据包
VOID CRunDataProcesser::ProcessUserRecvPacket(PRUNGATEUSERSESSION pSession, char *pBuffer, int nBufferSize)
{
	DECLARE_FUN_TIME_PROF();
	InterlockedExchangeAdd( (LONG*)&m_dwProcessRecvSize, nBufferSize );

	//session中已缓存的数据（剩下待处理的数据）
	int nLeftDataSize = pSession->RecvBuf.nOffset + nBufferSize + 1;
	if ( pSession->RecvBuf.nSize < nLeftDataSize )
	{
		pSession->RecvBuf.nSize = (nLeftDataSize / SESSION_DATAGROW_SIZE + 1) * SESSION_DATAGROW_SIZE;
		pSession->RecvBuf.lpBuffer = (char*)realloc( pSession->RecvBuf.lpBuffer, pSession->RecvBuf.nSize );
	}
	nLeftDataSize--;

	//将数据拷贝过来
	char *pDataBuffer;
	pDataBuffer = pSession->RecvBuf.lpBuffer;
	memcpy( &pDataBuffer[pSession->RecvBuf.nOffset], pBuffer, nBufferSize );
	pSession->RecvBuf.nOffset = nLeftDataSize;
	pDataBuffer[nLeftDataSize] = 0;

	//如果剩余数据超过峰值则丢弃
	if ( pSession->RecvBuf.nOffset >= SESSION_MAX_RECVPROC_SIZE )
	{
		pSession->RecvBuf.nOffset = 0;
		SendCloseSession( pSession, TRUE, enReasonLargePackLeft  );
		return;
	}

	//处理Session的buffer
	char pDataProcBuf[MAX_SEND_BUFF_SIZE];
	#ifdef _DEBUG
		memset(pDataProcBuf, 0, MAX_SEND_BUFF_SIZE);
	#endif
	while ( nLeftDataSize > 0 )
	{
		if ( pSession->wPacketError >= SESSION_PACKETERR_MAX )
		{
			nLeftDataSize = 0;
			SendCloseSession( pSession, TRUE, enReasonRecvLargerPack );
			OutputMsg( rmWaning, "wPacketError=%d,超出了",pSession->wPacketError);
			break;
		}
		if ( pSession->boMarkToClose || pSession->boRemoteClosed )
		{
			OutputMsg( rmTip, "远程关闭，不处理数据");
			break;
		}
		SIZE_T nBuffSize  = pSession->RecvBuf.nOffset ;

		//刚连接上来
		if (enConn == pSession->nSessionSatus)
		{
			SendOpenSession(pSession);//发送到LogicServer新增连接
			pSession->nKey = (unsigned short)wrand(INT_MAX);
			PostUserServerData(pSession, (char *)&pSession->nKey, 2);
			pSession->nSessionSatus = enCommunication;
		}
		//正常通讯，将输出的数据要传递到LogicServer
		else if (pSession->nSessionSatus == enCommunication)
		{
			SIZE_T nInprocessSize =0 ,nOutProcessSize =0;
			// 数据解包处理
			if (DataUnpack(pSession, pDataBuffer, nLeftDataSize, pDataProcBuf, MAX_SEND_BUFF_SIZE, nInprocessSize, nOutProcessSize))
			{
				// 调整已处理过的字节
				if  (nInprocessSize > 0)
				{
					nLeftDataSize -=  (INT)nInprocessSize;
					if (nLeftDataSize < 0)
					{
						nLeftDataSize = 0;
					}
					else
					{
						pDataBuffer = (char *)(pDataBuffer + nInprocessSize);
					}
				}

				//检查服务端key
				if (nOutProcessSize >= 2)
				{
					unsigned short tag = *((unsigned short*)pDataProcBuf);
					if(tag != pSession->nKey)
					{
						SendCloseSession(pSession, TRUE, 2);
						return;
					}

					//截去头部
					char* pOutData = (char*)pDataProcBuf + sizeof(DATAHEADER);
					nOutProcessSize -= sizeof(DATAHEADER);

					//把数据发到服务器
					SendServerMessage(GM_DATA, pSession->nIndex, pSession->nSocket,
						pSession->nServerIdx, pOutData, (INT)nOutProcessSize);
				}
			}
			else
			{
				OutputMsg( rmTip, "消息长度不够，等待下一次组包");
				break;
			}
		}
	}

	//修正Session的buffer
	if (pSession->RecvBuf.lpBuffer != NULL)
	{
		if ( nLeftDataSize > 0 )
		{
			if ( pDataBuffer != pSession->RecvBuf.lpBuffer )
			{
				memmove( pSession->RecvBuf.lpBuffer, &pSession->RecvBuf.lpBuffer[pSession->RecvBuf.nOffset - nLeftDataSize], nLeftDataSize );
				pSession->RecvBuf.lpBuffer[nLeftDataSize] = 0;
				pSession->RecvBuf.nOffset = nLeftDataSize;
			}
		}
		else 
		{
			pSession->RecvBuf.lpBuffer[0] = 0;
			pSession->RecvBuf.nOffset = 0;
		}
	}
}

int CRunDataProcesser::s_nIgnoreDataPacket = 0;
VOID CRunDataProcesser::PostUserServerData(PRUNGATEUSERSESSION pSession, const char *pBuffer, int nBufferSize, BOOL boWriteWsHdr, BOOL boWriteProtoHdr, TICKCOUNT nSendTime)
{
	DECLARE_FUN_TIME_PROF();
	char *pDataPtr;
	PRUNDATASENDTHREAD pSendThread;
	PSENDCLIENTBUF pSendBuf;
	PDATAHEADER pProtoHdr;
	static char sPrintS2CBuffer[81920];

	pSendThread = &m_SendThreads[pSession->nIndex % m_nSendThreadCount];
	pSendBuf = (PSENDCLIENTBUF)malloc( nBufferSize + sizeof(*pSendBuf) + 4 + sizeof(*pProtoHdr) + 16);
	pSendBuf->pSession = pSession;

	pDataPtr = (char*)(pSendBuf + 1);
	if ( boWriteProtoHdr )
	{
		pProtoHdr = (PDATAHEADER)pDataPtr;
		pProtoHdr->tag = pSession->nKey;
		pProtoHdr->len = nBufferSize;	
		pDataPtr += sizeof(*pProtoHdr);
	}

	memcpy( pDataPtr, pBuffer, nBufferSize );	
	if ( boWriteProtoHdr ) 
		nBufferSize += sizeof(*pProtoHdr);
	pSendBuf->nBufferSize = nBufferSize;
	InterlockedExchangeAdd( (LONG*)&m_dwWaitSendQueueSize, nBufferSize );

	EnterCriticalSection( &pSendThread->SendQueueLock );
	pSendThread->pSendAppendList->add( pSendBuf );
	LeaveCriticalSection( &pSendThread->SendQueueLock );
}

INT CRunDataProcesser::GetActiveUserCount()
{
	return m_nActiveUser;
}

VOID CRunDataProcesser::SetName(LPCSTR sName)
{
	ZeroMemory( m_sGateName, sizeof(m_sGateName) );
	strncpy( m_sGateName, sName, __min(sizeof(m_sGateName)-1, strlen(sName)) );
	SetClientName(sName);
}
/*
VOID CRunDataProcesser::SetPipeName(LPCSTR sPipeName)
{
	ZeroMemory( m_sPipeName, sizeof(m_sPipeName) );
	strncpy( m_sPipeName, sPipeName, __min(sizeof(m_sPipeName)-1, strlen(sPipeName)) );
}
*/

INT CRunDataProcesser::GetSendThreadCount()
{
	return m_nSendThreadCount;
}

VOID CRunDataProcesser::SetSendThreadCount(INT ThreadCount)
{
	if ( !m_boStarted && ThreadCount > 0 && ThreadCount <= RUNDATA_MAX_SENDTHREAD )
	{
		m_nSendThreadCount = ThreadCount;
	}
}

DATAPROCESSERTYPE	CRunDataProcesser::GetDataProcessType()
{
	return m_DataProcessType;
}

VOID CRunDataProcesser::SetDataProcessType(DATAPROCESSERTYPE Type)
{
	m_DataProcessType = Type;
}

VOID CRunDataProcesser::SetRunSockProcesser(CRunSockProcesser* pSockProcesser)
{
	m_pSockProcesser = pSockProcesser;
}

INT CRunDataProcesser::GetMaxSessionCount()
{
	return m_nMaxSessionCount;
}

PRUNDATASENDTHREAD	CRunDataProcesser::GetSendThreadInfo(INT ThreadIdx)
{
	if ( ThreadIdx >= 0 && ThreadIdx < RUNDATA_MAX_SENDTHREAD )
	{
		if ( m_SendThreads[ThreadIdx].hThread )
			return &m_SendThreads[ThreadIdx];
	}
	return NULL;
}

PRUNGATEUSERSESSION	CRunDataProcesser::GetFirstSession()
{
	return m_Sessions;
}

VOID	CRunDataProcesser::SetPrintC2SMessage(const BOOL boPrint)
{
	m_boPrintC2SMsg = boPrint;
}

VOID	CRunDataProcesser::SetPrintS2CMessage(const BOOL boPrint)
{
	m_boPrintS2CMsg = boPrint;
}



BOOL CRunDataProcesser::Startup()
{
	int i;

	m_boStoping = FALSE;

	m_dwWaitSendUserSize = 0;
	m_dwWaitSendQueueSize = 0;
	m_dwLastProcUsrMsgTime = 0;
	m_dwLastProcSrvMsgTime = 0;
	m_dwLastRecvSrvMsgTime = 0;
	m_dwRecvSeverSize = 0;
	m_dwSendUserSize = 0;
	m_dwProcSrvThreadSleep = 0;
	m_dwSendQueueSize = 0;

	//初始化会话队列
	InitSessions();

	//创建用户数据处理线程
#ifdef _MSC_VER
	m_hProcRecvThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)RecvDataProcessRoutine, this, 0, NULL );
	if ( !m_hProcRecvThread )
	{
		GotError( __FUNCTION__, _T("CreateThread"), GetLastError() );
		return FALSE;
	}
#else
	if (pthread_create(&m_hProcRecvThread, NULL, RecvDataProcessRoutine, this)) {
		return FALSE;
	}
#endif
	
	//创建用户数据发送线程
	for ( i=0; i<m_nSendThreadCount; ++i )
	{
		m_SendThreads[i].pRunData = this;
		m_SendThreads[i].nThreadIdx = i;
		m_SendThreads[i].boSendEWouldBlock = false;
		m_SendThreads[i].boSendFewBuffer = false;
#ifdef _MSC_VER
		m_SendThreads[i].hThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)SendDataProcessRoutine, &m_SendThreads[i], 0, &m_SendThreads[i].dwThreadId );
		if ( !m_SendThreads[i].hThread )
		{
			GotError( __FUNCTION__, _T("CreateThread"), GetLastError() );
			return FALSE;
		}
#else
		if (pthread_create(&(m_SendThreads[i].hThread), NULL, SendDataProcessRoutine, &m_SendThreads[i])) {
			return FALSE;
		}
#endif
	}

	if ( Inherited::Startup() )
	{
		m_boStarted = TRUE;
		return TRUE;
	}
	else return FALSE;
}


int CRunDataProcesser::ChangeConsoleLine(int nLine, int nPos, LPCSTR sFormat, ...)
{
	COORD cPos;
	va_list	args;

	cPos.X = nPos;
	cPos.Y = nLine;
#ifdef _MSC_VER
	SetConsoleCursorPosition( g_hStdOut, cPos );
#endif
	va_start(args, sFormat);
	cPos.X = vfprintf( stdout, sFormat, args );
	va_end(args);

	return cPos.X;
}

VOID  CRunDataProcesser::ShowGateStatus(CRunDataProcesser *pRunData)
{
	int i, nLine = 0;
	PRUNDATASENDTHREAD pSendThread;

	ChangeConsoleLine( nLine, 0, "---------------------网关状态[%s]-----------------\n", pRunData->GetName());
	nLine++;

	ChangeConsoleLine( nLine, 0, "活动连接数:%d      \n",  pRunData->GetActiveUserCount() );
	nLine++;

	ChangeConsoleLine( nLine, 0, "接收用户数据: %dB 发送用户数据: %dKB 待发用户数据: %dKB 待发用户队列: %dKB \n", 
		pRunData->m_dwProcessRecvSize, pRunData->m_dwSendUserSize / 1024, pRunData->m_dwWaitSendUserSize / 1024, 
		pRunData->m_dwSendQueueSize / 1024 );
	nLine++;

	ChangeConsoleLine( nLine, 0, "接收引擎数据: %dKB 接收数据时间: %d毫秒 引擎数据处理: %d毫秒 用户数据处理: %d毫秒 \n", 
		pRunData->m_dwRecvSeverSize / 1024, pRunData->m_dwLastRecvSrvMsgTime, pRunData->m_dwLastProcSrvMsgTime,
		pRunData->m_dwLastProcUsrMsgTime );
	nLine++;

	for ( i=0; i<RUNDATA_MAX_SENDTHREAD; ++i )
	{
		pSendThread = pRunData->GetSendThreadInfo( i );
		if ( !pSendThread )
			continue;
		ChangeConsoleLine( nLine, 0, "发送线程%d[数据处理: %d毫秒 休眠时间: %d毫秒 标志:%d/%d] \n", 
			i, pSendThread->dwProcTick, pSendThread->dwSleepTick, pSendThread->boSendEWouldBlock, pSendThread->boSendFewBuffer );
		nLine++;
		InterlockedExchange( (LONG*)&pSendThread->dwProcTick, 0 );
		InterlockedExchange( (LONG*)&pSendThread->dwSleepTick, 0 );
	}

	ChangeConsoleLine( nLine, 0, "服务器数据处理线程休眠: %d毫秒\n", 
		pRunData->m_dwProcSrvThreadSleep );
	nLine++;

	ChangeConsoleLine( nLine, 0, "----------------------------------------------------------------\n" );
	nLine++;
	pRunData->Dump();
	InterlockedExchange( (LONG*)&pRunData->m_dwProcessRecvSize, 0 );
	InterlockedExchange( (LONG*)&pRunData->m_dwSendUserSize, 0 );
	InterlockedExchange( (LONG*)&pRunData->m_dwRecvSeverSize, 0 );
	
}

VOID CRunDataProcesser::Stop()
{
	int i;
	m_boStoping = TRUE;

	Inherited::Stop();

	CloseThread( m_hProcRecvThread );
	for ( i=0; i<m_nSendThreadCount; ++i )
	{
		if ( m_SendThreads[i].hThread )
		{
			CloseThread( m_SendThreads[i].hThread );
			FreeSendBuffers( &m_SendThreads[i] );
		}
	}

	//CloseServerPipe();


	if ( m_ServerBuf.lpBuffer )
	{
		free( m_ServerBuf.lpBuffer );
		m_ServerBuf.lpBuffer = NULL;
	}
	m_ServerBuf.nOffset = m_ServerBuf.nSize = 0;

	FreeRecvBuffers();

	//FreeSendBuffers();

	CancelRemainSendSessionBuffers();

	CloseAllSessions( TRUE );

	UninitSessions();

	m_boStarted = FALSE;
}

//创建一个Session
PRUNGATEUSERSESSION CRunDataProcesser::NewSession(SOCKET nSocket, SOCKADDRIN RemoteAddr)
{
	int i;
	PRUNGATEUSERSESSION pSession, pNewSession = NULL;
	/*
	if ( m_hPipe == INVALID_HANDLE_VALUE )
		return NULL;
	*/
	OutputMsg( rmTip, "=================================================Begin");
	OutputMsg( rmTip, "开始接入新的socket=%d", (int)nSocket);
	EnterCriticalSection( &m_SessionLock );
	pSession = m_Sessions;
	for ( i=0; i<m_nMaxSessionCount; ++i )
	{
		//这里安潮发现一个问题，在多线程的情况下一个线程要了一个session,然后睡眠了，另外一个线程可能依然获得这个session
		if ( pSession->nSocket == INVALID_SOCKET && 
			!pSession->boMarkToClose && 
			!pSession->boRemoteClosed && 
			!pSession->dwCloseTick  && 
			pSession->nSessionSatus ==enSessionStatusIdle)
		{
			pNewSession = pSession;
			pNewSession->SendBuf.nOffset = 0;
			pNewSession->dwCloseTick = 0;
			pNewSession->boMarkToClose = false;
			pNewSession->boRemoteClosed = false;
			pNewSession->nSessionSatus = enConn;
			break;
		}
		pSession++;
	}
	LeaveCriticalSection( &m_SessionLock );

	if ( pNewSession )
	{
		pNewSession->SockAddr		= RemoteAddr;
		pNewSession->nServerIdx		= 0;
		pNewSession->btPacketIdx	= 0;
		pNewSession->wPacketError	= 0;
		pNewSession->nRecvPacketCount=0;
		pNewSession->nSendPacketCount=0;
		pNewSession->boMarkToClose  = false;
		pNewSession->boRemoteClosed = false;
		pNewSession->boSendAvaliable= true;
		pNewSession->dwSendTimeOut	= 0;
		pNewSession->dwCloseTick	= 0;
		pNewSession->dwConnectTick	= _getTickCount();	
		pNewSession->dwClientMsgTick= pNewSession->dwConnectTick;
		pNewSession->dwServerMsgTick= pNewSession->dwConnectTick;
		pNewSession->nVerifyIdx		= InterlockedIncrement( (unsigned long long*)&m_nUserVerify );
		pNewSession->sLoginAccount[0] = 0;
		pNewSession->sLoginCharName[0]= 0;
		//pNewSession->nSessionId		= 0;
		if ( !pNewSession->RecvBuf.lpBuffer )
		{ 
			pNewSession->RecvBuf.nSize = SESSION_RECV_BUFSIZE ;
			pNewSession->RecvBuf.lpBuffer = (char*)malloc( pNewSession->RecvBuf.nSize );
		}
		pNewSession->RecvBuf.nOffset = 0;
		#ifdef WIN32
		pNewSession->Overlapped.WSABuf.buf = pNewSession->sRecvBuf;
		pNewSession->Overlapped.WSABuf.len = sizeof(pNewSession->sRecvBuf) - 1;
		#endif
		pNewSession->sRecvBuf[sizeof(pNewSession->sRecvBuf)] = 0;

		if ( !pNewSession->SendBuf.lpBuffer )
		{ 
			pNewSession->SendBuf.nSize = SESSION_SEND_BUFSIZE ;
			pNewSession->SendBuf.lpBuffer = (char*)malloc( pNewSession->SendBuf.nSize );
		}
		pNewSession->SendBuf.nOffset = 0;
		//最后设置会话的nSocket成员数据，
		//以防止其他线程通过nSocket != INVALID_SOCKET判断后对一个尚为初始化完毕的会话进行操作！
		pNewSession->nSocket = nSocket;
		pNewSession->nSessionSatus = enConn; //开始处于连接状态
		char * pIp = inet_ntoa(RemoteAddr.sin_addr);
	
		OutputMsg( rmTip, "[%s]接入新的socket=%d 完毕,nVerifyIdx=%ld",pIp, (int)nSocket,pNewSession->nVerifyIdx);

		m_nActiveUser++;	//增加活动用户计数
	}
	return pNewSession;
}

static bool gFindHttpParam(const char * param, const char * buf) {
	while (*param == *buf) {
		if (*(param + 1) == '\0') return true;
		++param; ++buf;
	}
	return false;
}

VOID CRunDataProcesser::AddRecvBuf(PRUNGATEUSERSESSION pSession,unsigned long long nVerify, char *pBuffer, int BufferSize)
{
	PCLIENTRECVBUF pRecvBuf;
	char *pData;

	if ( pSession->nSocket == INVALID_SOCKET || pSession->boMarkToClose || pSession->boRemoteClosed || !nVerify || pSession->nVerifyIdx != nVerify )
		return;

	pRecvBuf = (PCLIENTRECVBUF)malloc( sizeof(*pRecvBuf) + BufferSize + 1 );
	pRecvBuf->pSession	= pSession;
	pRecvBuf->nVerify	  = nVerify;
	pRecvBuf->nBufferSize = BufferSize;
	pData = (char*)(pRecvBuf + 1);

	memcpy( pData, pBuffer, BufferSize );
	pData[BufferSize] = 0;

	EnterCriticalSection( &m_RecvQueueLock );
	m_pRecvAppendList->add( pRecvBuf );
	LeaveCriticalSection( &m_RecvQueueLock );
}
