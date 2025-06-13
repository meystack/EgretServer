#include <stdio.h>
#include <stdlib.h>



#ifdef WIN32
#include <Windows.h>
#include <tchar.h>
//#include <WinSock2.h>
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
#include "CustomServerClientSocket.h"
#include "CustomServerClientSocketEx.h"


CCustomServerClientSocketEx::CCustomServerClientSocketEx()
	:Inherited(),
	InheritedThread("CCustomServerClientSocketEx")
{	
	m_bStop = false;	
	//m_hSendDataThread = INVALID_HANDLE_VALUE;
	//m_hStopEvent = INVALID_HANDLE_VALUE;
	SetSendDataInSingleRun(false);
}

CCustomServerClientSocketEx::CCustomServerClientSocketEx(SOCKET nSocket, SOCKADDR_IN *pClientAddr)
{
	SetClientSocket(nSocket, pClientAddr);
}

CCustomServerClientSocketEx::~CCustomServerClientSocketEx()
{
	//if (m_hSendDataThread != INVALID_HANDLE_VALUE)
	{
		m_bStop = true;
		OutputMsg(rmNormal, _T("ServerClientSocketEx发送线程退出"));
		//WaitForSingleObject(m_hStopEvent, INFINITE);
		//OutputMsg(rmNormal, _T("ServerClientSocketEx发送线程退出"));
		//CloseHandle(m_hSendDataThread);
	}	
}

void CCustomServerClientSocketEx::StartWork()
{
	/*
	if (m_hSendDataThread != INVALID_HANDLE_VALUE)
		return;	
	m_bStop = false;
	if (INVALID_HANDLE_VALUE == m_hStopEvent)
		m_hStopEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hSendDataThread = ::CreateThread(NULL, 0, staticSendDataRountine, this, 0, NULL);
	*/
	OutputMsg(rmNormal, _T("ServerClientSocketEx发送线启动"));
	StartWorkThread();
	//InheritedThread::resume();

}

void CCustomServerClientSocketEx::StopWork()
{
	/*
	if (m_hSendDataThread != INVALID_HANDLE_VALUE)
	{
		m_bStop = true;
		OutputMsg(rmNormal, _T("等待ServerClientSocketEx发送线程退出"));
		WaitForSingleObject(m_hStopEvent, INFINITE);
		OutputMsg(rmNormal, _T("ServerClientSocketEx发送线程退出"));
		CloseHandle(m_hSendDataThread);
	}
	*/
	OutputMsg(rmNormal, _T("ServerClientSocketEx发送线程退出"));
	StopWorkThread();
	
}

/*
DWORD CCustomServerClientSocketEx::staticSendDataRountine(LPVOID pParam)
{
	CCustomServerClientSocketEx* pThis = (CCustomServerClientSocketEx *)pParam;
	pThis->SendDataRoutine();
	return 0;
}
*/

//void OnRountine();
void CCustomServerClientSocketEx::OnRountine()
{
	INT_PTR nMaxLoopSendCount = 2;
	static INT_PTR nLoopCount = 0;
	fd_set fdwrite;
	struct timeval tv = {1, 0};
	//while (!m_bStop)
	{
		if (!connected())
		{
			Sleep(50);
			return;
		}
		
		if (sendToSocketEx(*this) > 0)
		{
			m_dwMsgTick = _getTickCount();
		}
		
		if (m_SendingPacketList.count() > 0)
		{	
			FD_ZERO(&fdwrite); 
			SOCKET fd = getSocket() ;
			if( fd < 0 )  return ;
			FD_SET(fd, &fdwrite);   
			#ifdef WIN32
			int nRet = select(0, NULL, &fdwrite, NULL, &tv);
			#else
			int nRet = select(fd+1, NULL, &fdwrite, NULL, &tv);  
			#endif
			if (nRet < 0)
			{				
				OutputMsg(rmError, _T("select except: %d"), nRet);
				Sleep(10);
			}

			// 不管是出错还是等待超时，还是缓冲区可以写数据，都返回
			return;			
		}
		else
		{
			nLoopCount++;
			if (nLoopCount >= nMaxLoopSendCount)
			{
				nLoopCount = 0;
				Sleep(30);
			}
		}
	}

	//SetEvent(m_hStopEvent);
}

size_t CCustomServerClientSocketEx::sendToSocketEx(CCustomSocket& socket)
{
	INT_PTR nCount, nAvalLength, nBytesWriten;
	CDataPacket **pPacketList, *pPacket;
	size_t nTotalSent = 0;

	//提交追加到发送队列中的数据包
	if (m_SendingPacketList.count() <= 0)
		m_SendingPacketList.flush();

	//循环发送数据包
	nCount = m_SendingPacketList.count();
	if (nCount <= 0)
		return 0;
	pPacketList = m_SendingPacketList;	
	for (; m_nSendingPacketIdx<nCount; ++m_nSendingPacketIdx)
	{
		pPacket = pPacketList[m_nSendingPacketIdx];
		nAvalLength = pPacket->getAvaliableLength();
		if (nAvalLength > 0)
		{
			//发送数据
			nBytesWriten = socket.send(pPacket->getOffsetPtr(), (INT)nAvalLength);
			if ( nBytesWriten <= 0 )
			{
				break;
			}
		}
		else nBytesWriten = 0;
		nTotalSent += nBytesWriten;
		//该数据包中的数据是否已经发完
		if (nBytesWriten >= nAvalLength)
		{
			//清空数据包的数据长度
			pPacket->setLength(0);
		}
		else
		{
			//调整偏移到下次继续发送的位置
			pPacket->adjustOffset(nBytesWriten);
			break;
		}
	}
	//如果发送队列中的数据已经全部发送完毕，则将数据包全部移动到空闲数据包列表中
	// 计算已经发送完毕的数据包的数量	
	if ( m_nSendingPacketIdx >= nCount || m_nSendingPacketIdx >= 20)	
	{
		INT_PTR nSendCount = m_nSendingPacketIdx;
		//OutputMsg(rmTip, _T(" sendPackIdx=%d nCount=%d"), nSendCount, nCount);
		m_nSendingPacketIdx = 0;
		m_FreeSendPackList.lock();
		m_FreeSendPackList.addArray(m_SendingPacketList, nSendCount);		
		m_FreeSendPackList.unlock();
		m_SendingPacketList.remove(0, nSendCount);		
	}
	return nTotalSent;
}

//VOID CCustomServerClientSocketEx::SendSocketBuffers()
//{
//	//Inherited::SendSocketBuffers();
//}

