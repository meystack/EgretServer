#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#include <tchar.h>
#include <WinSock2.h>
#include <Windows.h>
#include <process.h>
#endif

#include <_ast.h>
#include <_memchk.h>
#include <CustomSocket.h>
#include <Tick.h>
#include <Lock.h>
#include <QueueList.h>




#include "ShareUtil.h"
#include "BufferAllocator.h"
#include "AppItnMsg.h"
#include "DataPacket.hpp"
#include "SendPackPool.h"
#include "CustomWorkSocket.h"
#include "CustomClientSocket.h"


CCustomClientSocket::CCustomClientSocket()
	:Inherited()
{
	m_sSrvHost[0] = 0;
	m_nSrvPort = 0;
	m_sClientName[0] = 0;
	//m_hThread = NULL;
	//m_boStoped = TRUE;
	m_dwReconnectTick = 0;
	//m_dwThreadId = 0;
	m_bNeedCheckMemory =true;
	ZeroMemory( &m_LoopPerformance, sizeof(m_LoopPerformance) );
	ZeroMemory( &m_MainProcPerformance, sizeof(m_MainProcPerformance) );
	ZeroMemory( &m_MainSleepPerformance, sizeof(m_MainSleepPerformance) );
	m_dwCheckMemoryTick = _getTickCount() +CHECK_MEMORY_INTERVAL; //后检测内存
	
}

CCustomClientSocket::~CCustomClientSocket()
{
	Stop();
}

VOID CCustomClientSocket::SingleRun()
{
	TICKCOUNT dwCurTick = _getTickCount();

	//连接到服务器
	if ( !connected() )
		ConnectToServer();
	
	Inherited::SingleRun();

	//发送保持连接消息
	if ( connected() )
	{

		if ( dwCurTick - m_dwMsgTick >= 10 * 1000 )
		{
			SendKeepAlive();
		}

		//5分钟检测一次内存，释放一下
		if( dwCurTick >= m_dwCheckMemoryTick && m_bNeedCheckMemory)
		{
			CheckMemory(false);
			m_dwCheckMemoryTick = dwCurTick + CHECK_MEMORY_INTERVAL; 
		}
		
	}
}

DWORD CCustomClientSocket::GetWorkThreadId()
{
	return InheritedThread::getThreadId();
}

VOID CCustomClientSocket::Connected()
{
	Inherited::Connected();
	OutputMsg( rmTip, _T("%s服务器连接成功"), m_sClientName );
	SendRegisteClient();
}

VOID CCustomClientSocket::Disconnected()
{
	Inherited::Disconnected();
	OutputMsg( rmWaning, _T("%s服务器连接已断开"), m_sClientName );
}

VOID CCustomClientSocket::SocketError(INT errorCode)
{
	Inherited::SocketError(errorCode);
	OutputError( errorCode, _T("%s客户端套接字发生错误"), m_sClientName );
}

VOID CCustomClientSocket::OnWorkThreadStart()
{
}

VOID CCustomClientSocket::OnWorkThreadStop()
{
}

VOID CCustomClientSocket::SetClientName(LPCTSTR sClientName)
{
	_asncpyt(m_sClientName, sClientName);
}

BOOL CCustomClientSocket::ConnectToServer()
{
	int nErr;

	//如果已连接则直接返回
	if ( connected() )
	{
		return TRUE;
	}

	TICKCOUNT dwCurTick = _getTickCount();

	//创建套接字
	if ( getSocket() == INVALID_SOCKET )
	{
		SOCKET sock;
		nErr = createSocket(&sock);
		if ( nErr )
		{
			OutputError( nErr, _T("创建%s客户端套接字失败"), m_sClientName );
			return FALSE;
		}
		m_nSocket = sock;
		//调整发送和接收缓冲大小
		nErr = setSendBufSize(32 * 1024);
		if ( nErr )
		{
			OutputError( nErr, _T("调整%s套接字发送缓冲长度失败"), m_sClientName );
			return FALSE;
		}
		nErr = setRecvBufSize(32 * 1024);
		if ( nErr )
		{
			OutputError( nErr, _T("调整%s套接字接收缓冲长度失败"), m_sClientName );
			return FALSE;
		}
	}
	//连接到会话服务器
	if ( dwCurTick >= m_dwReconnectTick )
	{
		m_dwReconnectTick = dwCurTick + 5 * 1000;
		nErr = connect(m_sSrvHost, (INT) m_nSrvPort);
		if ( nErr )
		{
			OutputError( nErr, _T("连接到%s服务器失败"), m_sClientName );
			return FALSE;
		}
		//调整为非阻塞模式
		nErr = setBlockMode(FALSE);
		if ( nErr )
		{
			OutputError( nErr, _T("调整%s套接字接为非阻塞模式失败"), m_sClientName );
			return FALSE;
		}
		m_dwConnectTick = m_dwMsgTick = dwCurTick;
		return TRUE;
	}

	return FALSE;
}

VOID CCustomClientSocket::SendRegisteClient()
{

}

VOID CCustomClientSocket::SendKeepAlive()
{

}
void CCustomClientSocket::OnThreadStarted()
{
	OnWorkThreadStart();
}

void CCustomClientSocket::OnRountine()
{
	static const int MaxLoop = 2;	//最大循环次数
	static const TICKCOUNT MaxCPU = 4;	//最大循环时间
	TICKCOUNT dwStartTick, dwCurTick, dwRestaticTick = 0;
	int nLoopCount;
	DWORD dwSleepTick;


	m_MainProcPerformance.dwTickBegin = dwStartTick = _getTickCount();
	//是否重新统计循环性能
	if ( dwStartTick >= dwRestaticTick )
	{
		dwRestaticTick = dwStartTick + 60 * 1000;
		m_LoopPerformance.nMaxLoop = 0;
		m_LoopPerformance.nMinLoop = 99999;

		m_MainProcPerformance.dwMaxTick = 0;
		m_MainProcPerformance.dwMinTick = 9999;
	}

	nLoopCount = 0;
	while ( TRUE )
	{
		SingleRun();
		nLoopCount++;
		dwCurTick = _getTickCount();
		if ( dwCurTick - dwStartTick >= MaxCPU )
		{
			dwSleepTick = 1;
			break;
		}
		if ( nLoopCount >= MaxLoop )
		{
			dwSleepTick = (DWORD)(MaxCPU - (dwCurTick - dwStartTick));
			if ( dwSleepTick > MaxCPU + 2 ) dwSleepTick = MaxCPU + 2; //如果实际休眠值为此值则说明上一步的运算溢出了
			break;
		}
	}

	m_LoopPerformance.nLastLoop = nLoopCount;
	m_MainProcPerformance.dwLastTick = dwCurTick -m_MainProcPerformance.dwTickBegin;
	m_MainSleepPerformance.dwLastTick = dwSleepTick;

	if (m_LoopPerformance.nMaxLoop < nLoopCount )
	{
		m_LoopPerformance.nMaxLoop = nLoopCount;
	}
	if ( m_LoopPerformance.nMinLoop > nLoopCount )
	{
		m_LoopPerformance.nMinLoop = nLoopCount;
	}
	if ( m_MainProcPerformance.dwMaxTick < m_MainProcPerformance.dwLastTick )
	{
		m_MainProcPerformance.dwMaxTick =m_MainProcPerformance.dwLastTick;
	}
	if ( m_MainProcPerformance.dwMinTick > m_MainProcPerformance.dwLastTick )
	{
		m_MainProcPerformance.dwMinTick = m_MainProcPerformance.dwLastTick;
	}
	if ( m_MainSleepPerformance.dwMaxTick < dwSleepTick )
	{
		m_MainSleepPerformance.dwMaxTick = dwSleepTick;
	}
	if ( m_MainSleepPerformance.dwMinTick > dwSleepTick )
	{
		m_MainSleepPerformance.dwMinTick = dwSleepTick;
	}
	Sleep(dwSleepTick);

}


void CCustomClientSocket::OnTerminated()
{
	if (connected() )
	{
		close();
	}
	OnWorkThreadStop();
}

/*
VOID STDCALL CCustomClientSocket::CustomClientSocketWorkRoutine(CCustomClientSocket *lpClient)
{
	static const int MaxLoop = 2;	//最大循环次数
	static const TICKCOUNT MaxCPU = 4;	//最大循环时间
	TICKCOUNT dwStartTick, dwCurTick, dwRestaticTick = 0;
	int nLoopCount;
	DWORD dwSleepTick;

	lpClient->OnWorkThreadStart();

	while ( !lpClient->m_boStoped )
	{
		lpClient->m_MainProcPerformance.dwTickBegin = dwStartTick = _getTickCount();
		//是否重新统计循环性能
		if ( dwStartTick >= dwRestaticTick )
		{
			dwRestaticTick = dwStartTick + 60 * 1000;
			lpClient->m_LoopPerformance.nMaxLoop = 0;
			lpClient->m_LoopPerformance.nMinLoop = 99999;

			lpClient->m_MainProcPerformance.dwMaxTick = 0;
			lpClient->m_MainProcPerformance.dwMinTick = 9999;
		}

		nLoopCount = 0;
		while ( TRUE )
		{
			lpClient->SingleRun();
			nLoopCount++;
			dwCurTick = _getTickCount();
			if ( dwCurTick - dwStartTick >= MaxCPU )
			{
				dwSleepTick = 1;
				break;
			}
			if ( nLoopCount >= MaxLoop )
			{
				dwSleepTick = (DWORD)(MaxCPU - (dwCurTick - dwStartTick));
				if ( dwSleepTick > MaxCPU + 2 ) dwSleepTick = MaxCPU + 2; //如果实际休眠值为此值则说明上一步的运算溢出了
				break;
			}
		}

		lpClient->m_LoopPerformance.nLastLoop = nLoopCount;
		lpClient->m_MainProcPerformance.dwLastTick = dwCurTick - lpClient->m_MainProcPerformance.dwTickBegin;
		lpClient->m_MainSleepPerformance.dwLastTick = dwSleepTick;

		if ( lpClient->m_LoopPerformance.nMaxLoop < nLoopCount )
		{
			lpClient->m_LoopPerformance.nMaxLoop = nLoopCount;
		}
		if ( lpClient->m_LoopPerformance.nMinLoop > nLoopCount )
		{
			lpClient->m_LoopPerformance.nMinLoop = nLoopCount;
		}
		if ( lpClient->m_MainProcPerformance.dwMaxTick < lpClient->m_MainProcPerformance.dwLastTick )
		{
			lpClient->m_MainProcPerformance.dwMaxTick = lpClient->m_MainProcPerformance.dwLastTick;
		}
		if ( lpClient->m_MainProcPerformance.dwMinTick > lpClient->m_MainProcPerformance.dwLastTick )
		{
			lpClient->m_MainProcPerformance.dwMinTick = lpClient->m_MainProcPerformance.dwLastTick;
		}
		if ( lpClient->m_MainSleepPerformance.dwMaxTick < dwSleepTick )
		{
			lpClient->m_MainSleepPerformance.dwMaxTick = dwSleepTick;
		}
		if ( lpClient->m_MainSleepPerformance.dwMinTick > dwSleepTick )
		{
			lpClient->m_MainSleepPerformance.dwMinTick = dwSleepTick;
		}
		Sleep(dwSleepTick);
	}

	if ( lpClient->connected() )
	{
		lpClient->close();
	}

	lpClient->OnWorkThreadStop();

	ExitThread(0);//设置线程退出返回值
}
*/


VOID CCustomClientSocket::SetServerHost(LPCTSTR sHost)
{
	_tcsncpy(m_sSrvHost, sHost, ArrayCount(m_sSrvHost) - 1);
	m_sSrvHost[ArrayCount(m_sSrvHost) - 1] = 0;
}

VOID CCustomClientSocket::SetServerPort(const INT_PTR nPort)
{
	m_nSrvPort = nPort;
}

BOOL CCustomClientSocket::Startup()
{
	//if ( TRUE == InterlockedCompareExchange(&m_boStoped, FALSE, TRUE) )
	{
		
		/*
		m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CustomClientSocketWorkRoutine, this, 0, &m_dwThreadId);
		
		if ( !m_hThread )
		{
			OutputError( GetLastError(), _T("创建%s通信线程失败"), m_sClientName );
			return FALSE;
		}
		*/
		//return CBaseThread::startThread((void*)CustomClientSocketWorkRoutine, &m_hThread, &m_dwThreadId,(void*)this,(char*)m_sClientName );	
		
	}

	InheritedThread::StartWorkThread(); //开始工作
	return TRUE;

}

VOID CCustomClientSocket::Stop()
{
	
	/*
	if ( FALSE == InterlockedCompareExchange(&m_boStoped, TRUE, FALSE) )
	{
		
		CloseThread(m_hThread);


		ClearSendBuffers();
		m_dwThreadId = 0;
	}
	*/
	InheritedThread::StopWorkThread(); //停止工作
}

