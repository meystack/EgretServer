#include <stdlib.h>
#include <stdio.h>
#ifdef WIN32
#include <tchar.h>
#include <Windows.h>
#endif
#include <_ast.h>
#include <_memchk.h>
#include <Thread.h>
#include <Tick.h>
#include <Lock.h>
#include <QueueList.h>
#include <CustomSocket.h>
#include "ShareUtil.h"
#include "BufferAllocator.h"
#include "GateProto.h"
#include "AppItnMsg.h"
#include "DataPacket.hpp"
#include "SendPackPool.h"
#include "CustomWorkSocket.h"
#include "CustomServerClientSocket.h"
#include "CustomServerClientSocketEx.h"
#include "CustomServerSocket.h"
#include "CustomServerGate.h"
#include "CustomGateManager.h"

CCustomGateManager::CCustomGateManager()
{
	ZeroMemory(m_GateList, sizeof(m_GateList));
	SetSingleLoopCount(8);
}

CCustomGateManager::~CCustomGateManager()
{
	Stop();
}

BOOL CCustomGateManager::Startup()
{
	//初始化网关列表
	for (INT_PTR i=0; i<ArrayCount(m_GateList); ++i)
	{
		m_GateList[i]->m_nGateIndex = i;
	}
	return Inherited::Startup();
}

VOID CCustomGateManager::Stop()
{
	Inherited::Stop();
}

VOID CCustomGateManager::PostCloseGateUser(SOCKET nUserSocket)
{
	INT_PTR i;
	//向每个已经连接的网关对象发送关闭用户的消息
	for ( i=0; i<ArrayCount(m_GateList); ++i )
	{
		if ( !m_GateList[i]->connected() )
		{
			m_GateList[i]->PostCloseUser(nUserSocket);
		}
	}
}

VOID CCustomGateManager::PostCloseGateUserByGlobalSessionId(int nSessionId)
{
	INT_PTR i;
	//向每个已经连接的网关对象发送关闭用户的消息
	for ( i=0; i<ArrayCount(m_GateList); ++i )
	{
		if ( m_GateList[i]->connected() )
		{
			m_GateList[i]->PostCloseUserByGlobalSessionId(nSessionId);
		}
	}
}

VOID CCustomGateManager::PostKickGateUserByGlobalUser(int nSessionId)
{
	INT_PTR i;
	//向每个已经连接的网关对象发送关闭用户的消息
	for ( i=0; i<ArrayCount(m_GateList); ++i )
	{
		if ( m_GateList[i]->connected() )
		{
			m_GateList[i]->PostKickUserByGlobalSessionId(nSessionId);
		}
	}
}

CCustomServerClientSocket* CCustomGateManager::CreateClientSocket(SOCKET nSocket, PSOCKADDR_IN pAddrIn)
{
	//从预初始的网关列表中返回一个没有使用的网关连接对象
	INT_PTR i;
	for ( i=0; i<ArrayCount(m_GateList); ++i )
	{
		if ( !m_GateList[i]->connected() )
		{
			OutputMsg( rmTip, _T("接受网关客户端连接!") );
			m_GateList[i]->SetClientSocket(nSocket, pAddrIn);
			m_GateList[i]->ClearSendBuffers();
			m_GateList[i]->StartWork();
			return m_GateList[i];
		}
	}	
	return NULL;
}

VOID CCustomGateManager::DestroyClientSocket(CCustomServerClientSocket *pClientSocket)
{	
	//什么都不处理	
	((CCustomServerGate *)pClientSocket)->StopWork();
}

 VOID CCustomGateManager::DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4)
{
	//按全局会话ID查询网关用户是否存在（Param1=全局会话ID）
	if (uMsg == SGIM_QUERY_GSID_USER_EXISTS)
	{
		INT_PTR i;
		int nSessionId = (int)uParam1;
		BOOL boExists = FALSE;
		for ( i=0; i<ArrayCount(m_GateList); ++i )
		{
			if ( m_GateList[i]->connected() )
			{
				if ( m_GateList[i]->UserExistsOfGlobalSessionId(nSessionId, NULL) )
				{
					boExists = TRUE;
					break;
				}
			}
		}
		OnResultGateUserExists(nSessionId, boExists);
	}
	else Inherited::DispatchInternalMessage(uMsg, uParam1, uParam2, uParam3,uParam4);
}

