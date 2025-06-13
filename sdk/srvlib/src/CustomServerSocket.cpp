#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
	#include <tchar.h>
	#include <Windows.h>
#endif


#include <_ast.h>
#include <_memchk.h>
#include <Thread.h>
#include <CustomSocket.h>
#include <Tick.h>
#include <Lock.h>
#include <QueueList.h>
#include "ShareUtil.h"
#include "BufferAllocator.h"
#include "../include/AppItnMsg.h"
#include "../include/DataPacket.hpp"
#include "../include/SendPackPool.h"
#include "../include/CustomWorkSocket.h"
#include "../include/CustomServerClientSocket.h"
#include "../include/CustomServerSocket.h"

CCustomServerSocket::CCustomServerSocket()
:Inherited()
{
	m_pAcceptThread = new CBaseThread("ServerAcceptThread",&CCustomServerSocket::ServerSocketAcceptThreadRoutine,(void*)this )  ;
	
	m_pDataThread = new CBaseThread("ServerDataThread",&CCustomServerSocket::ServerSocketDataThreadRoutine,(void*)this ) ;

	m_sServiceName[0] = 0;
	_tcscpy(m_sBindHost, _T("0.0.0.0"));//默认绑定到所有网卡
	m_nBindPort = 0;
	m_boStoped = TRUE;
	//m_boAcceptThreadStoped = TRUE;
	//m_hAcceptThread = NULL;
	//m_hDataThread = NULL;
	m_nLoopCountLimit = 2;
	m_uLoopTimeLimit = 4;
	m_ClientList.setLock(&m_ClientListLock);
}

CCustomServerSocket::~CCustomServerSocket()
{
	
	Stop();
	//销毁内存
}

void CCustomServerSocket::ServerSocketAcceptThreadRoutine(void *pData)
{
	CCustomServerSocket *lpServer = (CCustomServerSocket *)pData;

	int nError;
	SOCKET nSock;
	CCustomServerClientSocket *pClient;
	SOCKADDR_IN addr_In;

	nError = lpServer->accept(&nSock, 3000, &addr_In);
	//成功
	if ( nError == 0 )
	{
		//创建客户端连接类
		pClient = lpServer->CreateClientSocket(nSock, &addr_In);
		//创建连接类失败，则关闭套接字
		if ( !pClient )
		{
		#ifdef WIN32
			closesocket(nSock);
		#else
			::close(nSock);
		#endif
			return;
		}
		//设置为非阻塞模式
		nError = pClient->setBlockMode(false);
		if ( nError )
		{
			OutputError(nError, _T("无法将%s套接字接受的连接设置为非阻塞模式"), lpServer->m_sServiceName );
			//delete pClient;
			return;
		}
		//调整接收缓冲区大小
		nError = pClient->setRecvBufSize(32 * 1024);
		if ( nError )
		{
			//此操作的错误可以忽略
			OutputError(nError, _T("无法将%s套接字接受的连接调整接受缓冲区大小"), lpServer->m_sServiceName );
		}
		//调整发送缓冲区大小
		nError = pClient->setSendBufSize(32 * 1024);
		if ( nError )
		{
			//此操作的错误可以忽略
			OutputError(nError, _T("无法将%s套接字接受的连接调整发送缓冲区大小"), lpServer->m_sServiceName );
		}
		lpServer->OnClientConnect(pClient);
		if (pClient->connected())
			lpServer->m_ClientList.append(pClient);
		else lpServer->DestroyClientSocket(pClient);
		Sleep(1);
			
	}
	else
	{
		//超时
		if ( nError != SOCKET_ERROR - 1 )
		{
			OutputError(nError, _T(__FUNCTION__),_T("错误"));
		}
		Sleep(1000);
	}
}

void CCustomServerSocket::ServerSocketDataThreadRoutine(void *pData)
{
	CCustomServerSocket *lpServer = (CCustomServerSocket *)pData;
	TICKCOUNT dwStartTick, dwCurTick, dwRestaticTick = 0;
	int nLoopCount;
	TICKCOUNT dwSleepTick;

	//lpServer->OnSocketDataThreadStart();


	lpServer->m_MainProcPerformance.dwTickBegin = dwStartTick = _getTickCount();
	//是否重新统计循环性能
	if ( dwStartTick >= dwRestaticTick )
	{
		dwRestaticTick = dwStartTick + 60 * 1000;
		lpServer->m_LoopPerformance.nMaxLoop = 0;
		lpServer->m_LoopPerformance.nMinLoop = 99999;

		lpServer->m_MainProcPerformance.dwMaxTick = 0;
		lpServer->m_MainProcPerformance.dwMinTick = 9999;
	}

	nLoopCount = 0;
	while ( TRUE )
	{
		lpServer->SingleRun();
		nLoopCount++;
		dwCurTick = _getTickCount();
		if ( dwCurTick - dwStartTick >= lpServer->m_uLoopTimeLimit )
		{
			dwSleepTick = 1;
			break;
		}
		if ( nLoopCount >= lpServer->m_nLoopCountLimit )
		{
			dwSleepTick = (DWORD)(lpServer->m_uLoopTimeLimit - (dwCurTick - dwStartTick));
			//如果实际休眠值为此值则说明上一步的运算溢出了
			if ( dwSleepTick > lpServer->m_uLoopTimeLimit + 2 ) dwSleepTick = (DWORD)(lpServer->m_uLoopTimeLimit + 2); 
			break;
		}
	}

	lpServer->m_LoopPerformance.nLastLoop = nLoopCount;
	lpServer->m_MainProcPerformance.dwLastTick = dwCurTick - lpServer->m_MainProcPerformance.dwTickBegin;
	lpServer->m_MainSleepPerformance.dwLastTick = dwSleepTick;

	if ( lpServer->m_LoopPerformance.nMaxLoop < nLoopCount )
	{
		lpServer->m_LoopPerformance.nMaxLoop = nLoopCount;
	}
	if ( lpServer->m_LoopPerformance.nMinLoop > nLoopCount )
	{
		lpServer->m_LoopPerformance.nMinLoop = nLoopCount;
	}
	if ( lpServer->m_MainProcPerformance.dwMaxTick < lpServer->m_MainProcPerformance.dwLastTick )
	{
		lpServer->m_MainProcPerformance.dwMaxTick = lpServer->m_MainProcPerformance.dwLastTick;
	}
	if ( lpServer->m_MainProcPerformance.dwMinTick > lpServer->m_MainProcPerformance.dwLastTick )
	{
		lpServer->m_MainProcPerformance.dwMinTick = lpServer->m_MainProcPerformance.dwLastTick;
	}
	if ( lpServer->m_MainSleepPerformance.dwMaxTick < dwSleepTick )
	{
		lpServer->m_MainSleepPerformance.dwMaxTick = dwSleepTick;
	}
	if ( lpServer->m_MainSleepPerformance.dwMinTick > dwSleepTick )
	{
		lpServer->m_MainSleepPerformance.dwMinTick = dwSleepTick;
	}
	Sleep(dwSleepTick);
	//ExitThread(0);//设置线程退出返回值
}

VOID CCustomServerSocket::OnClientConnect(CCustomServerClientSocket *pClientSocke)
{

}

BOOL CCustomServerSocket::DoStartup()
{
	return TRUE;
}

VOID CCustomServerSocket::DoStop()
{

}

VOID CCustomServerSocket::OnSocketDataThreadStart()
{

}

VOID CCustomServerSocket::OnSocketDataThreadStop()
{
}

VOID CCustomServerSocket::DestroyClientSocket(CCustomServerClientSocket *pClientSocket)
{
	delete pClientSocket;
}

VOID CCustomServerSocket::ProcessClients()
{
	INT_PTR i;
	CCustomServerClientSocket *pClient;

	//循环处理每个客户端连接
	m_ClientList.flush();
	//必须降序循环，因为列表中的数据可能在循环中被移除
	for ( i=m_ClientList.count() - 1; i>-1; --i )
	{
		pClient = m_ClientList[i];
		if(pClient) {
			//处理活动的连接
			pClient->Run();
			//删除断开的连接
			if ( !pClient->connected() )
			{
				m_ClientList.lock();
				m_ClientList.remove(i);
				m_ClientList.unlock();
				DestroyClientSocket(pClient);
				continue;
			}
		}
		
	}
}

VOID CCustomServerSocket::CloseAllClients()
{
	INT_PTR i;
	CCustomServerClientSocket *pClient;

	m_ClientList.flush();
	
	for ( i=m_ClientList.count() - 1; i>-1; --i )
	{
		pClient = m_ClientList[i];
		if ( pClient )
		{
			pClient->close();
			DestroyClientSocket(pClient);
		}
	}
	m_ClientList.lock();
	m_ClientList.clear();
	m_ClientList.unlock();
}

VOID CCustomServerSocket::SingleRun()
{
	//处理连接的客户端
	ProcessClients();

	//处理内部消息
	ProcessInternalMessages();

	//调用例行函数
	OnRun();
}

VOID CCustomServerSocket::SetServiceName(LPCTSTR sName)
{
	_asncpyt(m_sServiceName, sName);
}

VOID CCustomServerSocket::SetSingleLoopCount(INT nSingleLoopCount)
{
	m_nLoopCountLimit = nSingleLoopCount;
}

VOID CCustomServerSocket::SetSingleLoopTime(UINT uSingleLoopTimeInMilSec)
{
	m_uLoopTimeLimit = uSingleLoopTimeInMilSec;
}

VOID CCustomServerSocket::SetServiceHost(LPCTSTR sHost)
{
	_asncpyt(m_sBindHost, sHost);
}

VOID CCustomServerSocket::SetServicePort(const INT_PTR nPort)
{
	m_nBindPort = nPort;
}

BOOL CCustomServerSocket::Startup()
{
	int nError;
	if ( TRUE == InterlockedCompareExchange(&m_boStoped, FALSE, TRUE) )
	{
		//创建用于接受连接的套接字
		nError = createSocket(&m_nSocket);
		if ( nError )
		{
			OutputError(nError, _T("创建%s服务套接字失败"), m_sServiceName);
			return FALSE;
		}
		//绑定套接字到地址和端口
		nError = bind(m_sBindHost, (INT)m_nBindPort);
		if ( nError )
		{
 			nError = errno;
			OutputError(nError, _T("绑定%s服务器套接字到%s:%d失败"), m_sServiceName, m_sBindHost, m_nBindPort);
			return FALSE;
		}
		//监听
		nError = listen();
		if ( nError )
		{
 			nError = errno;
			OutputError(nError, _T("无法在%s:%d端口监听%s服务"), m_sBindHost, m_nBindPort, m_sServiceName);
			return FALSE;
		}

		//调用启动函数
		if ( !DoStartup() )
		{
			return FALSE;
		}

		//启动接受连接的线程
		//m_boAcceptThreadStoped = FALSE;

		if(m_pAcceptThread)
		{
			m_pAcceptThread->StartWorkThread();
		}
		OnSocketDataThreadStart();
		if(m_pDataThread)
		{
			m_pDataThread->StartWorkThread();
		}
	}
	return TRUE;
}

VOID CCustomServerSocket::Stop()
{
	if ( FALSE == InterlockedCompareExchange(&m_boStoped, TRUE, FALSE) )
	{
	
		
		//关闭所有连接
		
		OnSocketDataThreadStop();
		
		CloseAllClients();

		if(m_pDataThread)
		{
			m_pDataThread->StopWorkThread();
		}

		if(m_pAcceptThread)
		{
			m_pAcceptThread->StopWorkThread();
		}

		if (connected() )
		{
			close();
		}

		//CloseThread(m_hAcceptThread);

		//关闭监听套接字
		if ( m_nSocket != INVALID_SOCKET )
			close();

		//CloseThread(m_hDataThread);

		//调用停止服务函数
		DoStop();

		if(m_pAcceptThread)
		{
			delete m_pAcceptThread;
			m_pAcceptThread =NULL;
		}
		if(m_pDataThread )
		{
			delete m_pDataThread;
			m_pDataThread =NULL;
		}
	}
}
