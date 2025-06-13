#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <tchar.h>
#include <WinSock2.h>
#include <Windows.h>
#endif

#include <_ast.h>
#include <_memchk.h>
#include <CustomSocket.h>
#include <Tick.h>
#include <Lock.h>
#include "Thread.h"

#include <QueueList.h>
#include "ShareUtil.h"
#include "BufferAllocator.h"
#include "AppItnMsg.h"
#include "DataPacket.hpp"
#include "SendPackPool.h"
#include "CustomWorkSocket.h"
#include "CustomServerClientSocket.h"


CCustomServerClientSocket::CCustomServerClientSocket()
	:Inherited()
{
	ZeroMemory(&m_RemoteAddr, sizeof(m_RemoteAddr));
	m_boActiveKeepAlive = TRUE;
}

CCustomServerClientSocket::CCustomServerClientSocket(SOCKET nSocket, SOCKADDR_IN *pClientAddr):Inherited()
{
	SetClientSocket(nSocket, pClientAddr);
	m_boActiveKeepAlive = TRUE;
}

CCustomServerClientSocket::~CCustomServerClientSocket()
{

}

VOID CCustomServerClientSocket::SetClientSocket(SOCKET nSocket, SOCKADDR_IN *pClientAddr)
{
	m_dwConnectTick = _getTickCount();
	setSocket(nSocket);
	if ( pClientAddr )
	{
		m_RemoteAddr = *pClientAddr;

#ifdef WIN32
		int nLen = _sntprintf(m_sRemoteHost, sizeof(m_sRemoteHost) - 1, "%d.%d.%d.%d", 
			pClientAddr->sin_addr.S_un.S_un_b.s_b1, pClientAddr->sin_addr.S_un.S_un_b.s_b2, 
			pClientAddr->sin_addr.S_un.S_un_b.s_b3, pClientAddr->sin_addr.S_un.S_un_b.s_b4);
		m_sRemoteHost[nLen] = 0;
#else
		//DO Do In Linux
#endif

		m_nRemotePort = htons(pClientAddr->sin_port);
	}
	else
	{
		ZeroMemory(&m_RemoteAddr, sizeof(m_RemoteAddr));
		m_sRemoteHost[0] = 0;
		m_nRemotePort = 0;
	}
}

VOID CCustomServerClientSocket::SendKeepAlive()
{
}

VOID CCustomServerClientSocket::SingleRun()
{
	Inherited::SingleRun();

	//发送保持连接消息
	if ( m_boActiveKeepAlive && connected() )
	{
		if ( _getTickCount() - m_dwMsgTick >= 10 * 1000 )
		{
			SendKeepAlive();
		}
	}
}
