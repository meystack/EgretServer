#include "StdAfx.h"
#include "DataProcess.h"
#include "SockProcess.h"
#include "SelectSockProcess.h"
#include "IOCPSockProcess.h"
#include "GateServer.h"
#include "GateServerConfig.h"

const char szExceptionDumpFile[] = ".\\RunGate.dmp";






#ifdef _USRDLL
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	static LPTOP_LEVEL_EXCEPTION_FILTER lpOldExceptonFilter;

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		lpOldExceptonFilter = SetUnhandledExceptionFilter( DefaultUnHandleExceptionFilter );
		InitDefMsgOut();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		UninitDefMsgOut();
		 SetUnhandledExceptionFilter( lpOldExceptonFilter );
		break;
	}
    return TRUE;
}
#endif


/*************************作为插件的导出函数*************************/

CRunSockProcesser* EXPORTCALL CreateRunGate(RUNSOCKTYPE SockType, int MaxSession)
{
	CRunSockProcesser *pRunSock;
	CRunDataProcesser *pRunData;

	if ( SockType == rsSelect )
		pRunSock = new CSelectRunSockProcesser();
#ifdef WIN32
	else if ( SockType == rsIOCP )
		pRunSock = new CIOCPRunSockProcesser();
#endif
	else return NULL;
	
	pRunData = new CRunDataProcesser( MaxSession );
	pRunSock->SetDataProcesser( pRunData );

	return pRunSock;
}

VOID EXPORTCALL FreeRunGate(CRunSockProcesser *pRunSock)
{
	if ( pRunSock )
	{
		pRunSock->Stop();
		pRunSock->GetDataProcesser()->Stop();
		delete pRunSock->GetDataProcesser();
		delete pRunSock;
	}
}

VOID EXPORTCALL RunGateSetBindAddress(CRunSockProcesser *pRunSock, LPCSTR sBindAddress)
{
	pRunSock->SetBindAddress( sBindAddress );
}

VOID EXPORTCALL RunGateSetBindPort(CRunSockProcesser *pRunSock, INT nBindPort)
{
	pRunSock->SetBindPort( nBindPort );
}

INT EXPORTCALL RunGateGetIOWorkThreadCount(CRunSockProcesser *pRunSock)
{
#ifdef WIN32
	if ( pRunSock->GetRunSockType() == rsIOCP )
		return ((CIOCPRunSockProcesser*)pRunSock)->GetWorkThreadCount();
	else
#endif
		return -1;
}

BOOL EXPORTCALL RunGateSetIOWorkThreadCount(CRunSockProcesser *pRunSock, INT nThreadCount)
{
#ifdef WIN32
	if (pRunSock->GetRunSockType() == rsIOCP)
	{
		((CIOCPRunSockProcesser*)pRunSock)->SetWorkThreadCount(nThreadCount);
		return TRUE;
	}
#endif
	return FALSE;
}

BOOL EXPORTCALL RunGateStartup(CRunSockProcesser *pRunSock)
{
	CRunDataProcesser *pRunData = pRunSock->GetDataProcesser();

	if ( pRunData->Startup() )
	{
		if ( pRunSock->InitBase() )
		{
			if ( pRunSock->Start() )
				return TRUE;
			else pRunSock->UninitBase();
		}
		pRunData->Stop();
	}

	return FALSE;
}

VOID EXPORTCALL RunGateStop(CRunSockProcesser *pRunSock)
{
	pRunSock->Stop();
	pRunSock->UninitBase();
	pRunSock->GetDataProcesser()->Stop();
}

INT EXPORTCALL RunGateGetActiveSessionCount(CRunSockProcesser *pRunSock)
{
	return pRunSock->GetDataProcesser()->GetActiveUserCount();
}

VOID EXPORTCALL RunGateSetName(CRunSockProcesser *pRunSock, LPCSTR sName)
{
	pRunSock->GetDataProcesser()->SetName( sName );
}

VOID EXPORTCALL RunGateSetServerIP(CRunSockProcesser *pRunSock, LPCSTR sServerName)
{
	pRunSock->GetDataProcesser()->SetServerHost( sServerName );
}


VOID EXPORTCALL RunGateSetServerPort(CRunSockProcesser *pRunSock, int port)
{
	pRunSock->GetDataProcesser()->SetServerPort( port );
}


INT EXPORTCALL RunGateGetSendThreadCount(CRunSockProcesser *pRunSock)
{
	return pRunSock->GetDataProcesser()->GetSendThreadCount();
}

VOID EXPORTCALL RunGateSetSendThreadCount(CRunSockProcesser *pRunSock, INT nThreadCount)
{
	pRunSock->GetDataProcesser()->SetSendThreadCount( nThreadCount );
}

DATAPROCESSERTYPE	RunGateGetDataProcessType(CRunSockProcesser *pRunSock)
{
	return pRunSock->GetDataProcesser()->GetDataProcessType();
}

VOID EXPORTCALL RunGateSetDataProcessType(CRunSockProcesser *pRunSock, DATAPROCESSERTYPE ProcessType)
{
	pRunSock->GetDataProcesser()->SetDataProcessType( ProcessType );
}

VOID EXPORTCALL RunGateSetDataProcessPrintMsgPacket(CRunSockProcesser *pRunSock, BOOL boPrintC2S, BOOL boPrintS2C)
{
	pRunSock->GetDataProcesser()->SetPrintC2SMessage(boPrintC2S);
	pRunSock->GetDataProcesser()->SetPrintS2CMessage(boPrintS2C);
}


BOOL EXPORTCALL RunGateQuerySendThreadInfo(CRunSockProcesser *pRunSock, INT nThreadIdx, PRUNGATESENDTHREADQUERYSTRUCT pQueryStruct)
{
	PRUNDATASENDTHREAD pSendThread;

	pSendThread = pRunSock->GetDataProcesser()->GetSendThreadInfo( nThreadIdx );
	if ( pSendThread )
	{
		pQueryStruct->hThread		= pSendThread->hThread;
		pQueryStruct->dwThreadId	= pSendThread->dwThreadId;
		pQueryStruct->dwProcTick	= pSendThread->dwProcTick;
		pQueryStruct->dwSleepTick	= pSendThread->dwSleepTick;
		return TRUE;
	}
	return FALSE;
}

INT EXPORTCALL RunGateEnumActiveSessions(CRunSockProcesser *pRunSock, RUNGATE_ENUMSESSION_ROUTINE lpCallBack)
{
	int nCount = 0;
	TICKCOUNT dwCurTick;
	PRUNGATEUSERSESSION pSession, pLastSession;
	RUNGATEQUERYSESSIONSTRUCT SessionInfo;

	dwCurTick = _getTickCount();
	pSession = pRunSock->GetDataProcesser()->GetFirstSession();
	pLastSession = pSession + pRunSock->GetDataProcesser()->GetMaxSessionCount();

	while ( pSession < pLastSession )
	{
		if ( pSession->nSocket != INVALID_SOCKET && !pSession->boMarkToClose && !pSession->boRemoteClosed )
		{
			SessionInfo.nSocket		= pSession->nSocket;
			SessionInfo.pSession	= pSession;
			SessionInfo.SockAddr	= pSession->SockAddr;
			SessionInfo.nRecvPacketCount = pSession->nRecvPacketCount;
			SessionInfo.nSendPacketCount = pSession->nSendPacketCount;
			SessionInfo.wPacketError= pSession->wPacketError;
			SessionInfo.btPacketIdx = pSession->btPacketIdx;
			SessionInfo.boSendAvaliable	= pSession->boSendAvaliable;
			SessionInfo.nVerifyIdx	= pSession->nVerifyIdx;
			SessionInfo.nRecvBufSize	= pSession->RecvBuf.nOffset;
			SessionInfo.nRecvCacheSize	= pSession->RecvBuf.nSize;
			SessionInfo.nSendBufSize	= pSession->SendBuf.nOffset;
			SessionInfo.nSendCacheSize	= pSession->SendBuf.nSize;
			SessionInfo.dwActiveTick	= dwCurTick - pSession->dwConnectTick;
			SessionInfo.dwClientMsgTick	= dwCurTick - pSession->dwClientMsgTick;
			SessionInfo.dwServerMsgTick	= dwCurTick - pSession->dwServerMsgTick;
			//memcpy( SessionInfo.sLoginAccount, pSession->sLoginAccount, __min(sizeof(SessionInfo.sLoginAccount), sizeof(pSession->sLoginAccount)) );
			//memcpy( SessionInfo.sLoginCharName, pSession->sLoginCharName, __min(sizeof(SessionInfo.sLoginCharName), sizeof(pSession->sLoginCharName)) );
			//SessionInfo.nSessionId		= pSession->nSessionId;
			if ( !lpCallBack( pRunSock, &SessionInfo ) )
				break;
			nCount++;
		}
		pSession++;
	}

	return nCount;
}

BOOL EXPORTCALL RunGateCloseSession(CRunSockProcesser *pRunSock, PRUNGATEUSERSESSION pSession)
{
	PRUNGATEUSERSESSION pFirstSession;

	if ( !pRunSock || !pSession )
		return FALSE;

	pFirstSession = pRunSock->GetDataProcesser()->GetFirstSession();
	if ( pSession < pFirstSession )
		return FALSE;
	if ( pSession >= pFirstSession + pRunSock->GetDataProcesser()->GetMaxSessionCount() )
		return FALSE;
	if ( (((size_t)pSession) - ((size_t)pFirstSession)) % sizeof(*pSession) != 0 )
		return FALSE;

	if ( pSession->nSocket == INVALID_SOCKET || pSession->boMarkToClose || pSession->boRemoteClosed )
		return FALSE;

	pRunSock->GetDataProcesser()->SendCloseSession( pSession, TRUE, 9999 );
	return TRUE;
}

VOID EXPORTCALL RunGateQueryDataStatics(CRunSockProcesser *pRunSock, PRUNDATASTATICS pQueryStruct)
{
	CRunDataProcesser *pRunData = pRunSock->GetDataProcesser();

	pQueryStruct->dwProcessRecvSize		= pRunData->m_dwProcessRecvSize;
	pQueryStruct->dwWaitSendUserSize	= pRunData->m_dwWaitSendUserSize;
	pQueryStruct->dwWaitSendQueueSize	= pRunData->m_dwWaitSendQueueSize;
	pQueryStruct->dwRecvSeverSize		= pRunData->m_dwRecvSeverSize;
	pQueryStruct->dwSendUserSize		= pRunData->m_dwSendUserSize;
	pQueryStruct->dwLastProcUsrMsgTime	= pRunData->m_dwLastProcSrvMsgTime;
	pQueryStruct->dwLastProcSrvMsgTime	= pRunData->m_dwLastProcSrvMsgTime;
	pQueryStruct->dwLastRecvSrvMsgTime	= pRunData->m_dwLastRecvSrvMsgTime;
	pQueryStruct->dwProcSrvThreadSleep	= pRunData->m_dwProcSrvThreadSleep;
	pQueryStruct->dwSendQueueSize		= pRunData->m_dwSendQueueSize;
}

VOID EXPORTCALL RunGateResetDataStatics(CRunSockProcesser *pRunSock)
{
	CRunDataProcesser *pRunData = pRunSock->GetDataProcesser();

	InterlockedExchange( (LONG*)&pRunData->m_dwProcessRecvSize, 0 );
	InterlockedExchange( (LONG*)&pRunData->m_dwRecvSeverSize, 0 );
	InterlockedExchange( (LONG*)&pRunData->m_dwSendUserSize, 0 );
}

INT EXPORTCALL RunGateGetKernalVersion(LPVOID lpReseved)
{
	return	RUNGATE_KRN_VERSION;
}
