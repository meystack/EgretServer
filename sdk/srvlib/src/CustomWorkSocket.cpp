#include <stdlib.h>
#include <stdio.h>

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
#include <QueueList.h>
#include "ShareUtil.h"
#include "BufferAllocator.h"
#include "AppItnMsg.h"
#include "DataPacket.hpp"
#include "SendPackPool.h"
#include "CustomWorkSocket.h"


CCustomWorkSocket::CCustomWorkSocket()
	:Inherited(),CSendPacketPool()
{
	m_dwConnectTick = 0;
	m_dwMsgTick = 0;
	m_bSendData = true;
	ZeroMemory(&m_RecvBuffers, sizeof(m_RecvBuffers));
	m_pRecvBuffer = &m_RecvBuffers[0];
	m_pProcRecvBuffer = &m_RecvBuffers[1];

	m_ItnMsgList.setLock(&m_ItnMsgListLock);
}

CCustomWorkSocket::~CCustomWorkSocket()
{
	free(m_pRecvBuffer->pBuffer);
	free(m_pProcRecvBuffer->pBuffer);

	INT_PTR i, nCount;
	PAPPINTERNALMSG pAppMsg;

	m_ItnMsgList.flush();

	nCount = m_ItnMsgList.count();
	for ( i = 0; i < nCount; ++i )
	{
		pAppMsg = m_ItnMsgList[i];
		m_Allocator.FreeBuffer(pAppMsg);
	}
	m_ItnMsgList.clear();
}

VOID CCustomWorkSocket::ReadSocket()
{
	static const int OnceRecvSize = 4096;
	int nError;
	TICKCOUNT dwCurTick = _getTickCount();	

	while ( TRUE )
	{
		//增长接收缓冲区大小
		if ( m_pRecvBuffer->nSize - m_pRecvBuffer->nOffset < OnceRecvSize * 2 )
		{
			size_t nPointer = m_pRecvBuffer->pPointer - m_pRecvBuffer->pBuffer;
			m_pRecvBuffer->nSize += OnceRecvSize * 2;
			m_pRecvBuffer->pBuffer = (char*)realloc(m_pRecvBuffer->pBuffer, m_pRecvBuffer->nSize);
			m_pRecvBuffer->pPointer = m_pRecvBuffer->pBuffer + nPointer;
		}
		//从套接字读取数据
		nError = recv(&m_pRecvBuffer->pBuffer[m_pRecvBuffer->nOffset], OnceRecvSize);
		if ( nError <= 0 )
			break;

		m_pRecvBuffer->nOffset += nError;
		m_pRecvBuffer->pBuffer[m_pRecvBuffer->nOffset] = 0;
		m_dwMsgTick = dwCurTick;		
	}
}

VOID CCustomWorkSocket::ProcessRecvBuffers(PDATABUFFER pDataBuffer)
{
	//对接受数据的默认处理是什么都不做，只清空数据
	pDataBuffer->nOffset = 0;
	pDataBuffer->pPointer = pDataBuffer->pBuffer;
}

VOID CCustomWorkSocket::SendSocketBuffers()
{
	if ( sendToSocket(*this) > 0 )
	{
		m_dwMsgTick = _getTickCount();
	}
}

VOID CCustomWorkSocket::DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4)
{
}

VOID CCustomWorkSocket::ProcessInternalMessages()
{
	INT_PTR i, nCount;
	PAPPINTERNALMSG pAppMsg;

	m_ItnMsgList.flush();

	nCount = m_ItnMsgList.count();
	for ( i = 0; i < nCount; ++i )
	{
		pAppMsg = m_ItnMsgList[i];
		DispatchInternalMessage(pAppMsg->uMsg, pAppMsg->uParam1, pAppMsg->uParam2, pAppMsg->uParam3,pAppMsg->uParam4);
		m_Allocator.FreeBuffer(pAppMsg);
	}
	m_ItnMsgList.clear();
}

VOID CCustomWorkSocket::SingleRun()
{
	//TICKCOUNT dwCurTick = _getTickCount();

	//接收数据
	if ( connected() )
		ReadSocket();

	//处理接受到的数据包
	if ( connected() )
		ProcessRecvBuffers(m_pProcRecvBuffer);

	//处理内部消息
	ProcessInternalMessages();

	//调用例行函数
	OnRun();

	//发送数据
	if ( connected() && m_bSendData)
	{
		SendSocketBuffers();
	}
}

VOID CCustomWorkSocket::Disconnected()
{
	Inherited::Disconnected();
	//断开连接后清空接收到的数据
	m_pRecvBuffer->pPointer = m_pRecvBuffer->pBuffer;
	m_pRecvBuffer->nOffset = 0;

	m_pProcRecvBuffer->pPointer = m_pProcRecvBuffer->pBuffer;
	m_pProcRecvBuffer->nOffset = 0;
}

VOID CCustomWorkSocket::SwapRecvProcessBuffers()
{
	INT_PTR dwSize;

	//数据包处理完毕
	dwSize = (int)(m_pProcRecvBuffer->pPointer - m_pProcRecvBuffer->pBuffer);
	if ( dwSize >= m_pProcRecvBuffer->nOffset )
	{
		PDATABUFFER pDataBuffer = m_pProcRecvBuffer;
		m_pProcRecvBuffer = m_pRecvBuffer;
		m_pRecvBuffer = pDataBuffer;

		m_pProcRecvBuffer->pPointer = m_pProcRecvBuffer->pBuffer;
		m_pRecvBuffer->pPointer = m_pRecvBuffer->pBuffer;
		m_pRecvBuffer->nOffset = 0;
	}
	//有新的数据
	else if ( m_pRecvBuffer->nOffset > 0 )
	{
		//将剩余数据移动到头部
		dwSize = m_pProcRecvBuffer->nOffset - (INT_PTR)(m_pProcRecvBuffer->pPointer - m_pProcRecvBuffer->pBuffer);
		if ( m_pProcRecvBuffer->pPointer > m_pProcRecvBuffer->pBuffer )
		{
			//memcpy(m_pProcRecvBuffer->pBuffer, m_pProcRecvBuffer->pPointer, dwSize);
			memmove(m_pProcRecvBuffer->pBuffer, m_pProcRecvBuffer->pPointer, dwSize);
			m_pProcRecvBuffer->nOffset = dwSize;
			m_pProcRecvBuffer->pBuffer[m_pProcRecvBuffer->nOffset] = 0;
		}
		//拷贝新数据
		if ( m_pProcRecvBuffer->nSize <= m_pRecvBuffer->nOffset + m_pProcRecvBuffer->nOffset )
		{
			m_pProcRecvBuffer->nSize += __max(8192, m_pRecvBuffer->nOffset + 1);
			m_pProcRecvBuffer->pBuffer = (char*)realloc(m_pProcRecvBuffer->pBuffer, m_pProcRecvBuffer->nSize);
		}
		memcpy(&m_pProcRecvBuffer->pBuffer[m_pProcRecvBuffer->nOffset], m_pRecvBuffer->pBuffer, m_pRecvBuffer->nOffset);
		m_pProcRecvBuffer->nOffset += m_pRecvBuffer->nOffset;
		m_pProcRecvBuffer->pBuffer[m_pProcRecvBuffer->nOffset] = 0;
		m_pProcRecvBuffer->pPointer = m_pProcRecvBuffer->pBuffer;

		m_pRecvBuffer->nOffset = 0;
		m_pRecvBuffer->pPointer = m_pRecvBuffer->pBuffer;
	}
}

VOID CCustomWorkSocket::ClearSendBuffers()
{
	clearSendList();
}

VOID CCustomWorkSocket::OnRun()
{
}

VOID CCustomWorkSocket::AppendSendBuffer(LPCVOID lpData, const size_t dwSize)
{
	CDataPacket &packet = allocSendPacket();
	packet.writeBuf(lpData, dwSize);
	flushSendPacket(packet);
}

VOID CCustomWorkSocket::PostInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4)
{
	PAPPINTERNALMSG pAppMsg = (PAPPINTERNALMSG)m_Allocator.AllocBuffer(sizeof(*pAppMsg));
	pAppMsg->uMsg = uMsg;
	pAppMsg->uParam1 = uParam1;
	pAppMsg->uParam2 = uParam2;
	pAppMsg->uParam3 = uParam3;
	pAppMsg->uParam4 = uParam4;
	m_ItnMsgList.append(pAppMsg);
}

INT CCustomWorkSocket::InitSocketLib()
{
#ifdef WIN32
	WSADATA Wsad;
	if ( WSAStartup(0x0202, &Wsad) )
		return GetLastError();
#endif

	return 0;
}

VOID CCustomWorkSocket::UnintSocketLib()
{

#ifdef WIN32
	WSACleanup();
#endif
}
