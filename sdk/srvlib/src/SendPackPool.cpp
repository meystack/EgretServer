#include <new>
#include <stdio.h>

#ifdef WIN32
#include <tchar.h>
#include <Windows.h>
#endif

#include <_ast.h>
#include <_memchk.h>
#include <Tick.h>
#include <Lock.h>
#include <QueueList.h>
#include <CustomSocket.h>

#include "BufferAllocator.h"
#include "../include/DataPacket.hpp"

using namespace wylib::container;
using namespace wylib::inet::socket;

#include "../include/SendPackPool.h"

CSendPacketPool::CSendPacketPool()
{
	m_nSendingPacketIdx = 0;
	m_SendingPacketList.setLock(&m_SendingPacketLock);
	m_FreeSendPackList.setLock(&m_FreeSendPackLock);
	m_dwPackAllocCount =0;  //申请内存的次数
}

CSendPacketPool::~CSendPacketPool()
{
	FreeAllPackets();
}

size_t CSendPacketPool::GetMemoryUsage(size_t *pSendingSize,size_t* pFreeSendSize,size_t* pMaxSize  )
{
	size_t nTotalSize =0,nSendingSize =0,freeSendSize =0,nSize,nMaxSize =0;
	CDataPacket **pPacketList;
	
	

	pPacketList = m_SendingPacketList;


	for (INT_PTR i=m_SendingPacketList.count()-1; i>-1; --i)
	{
		nSize = pPacketList[i]->getMemorySize();
		if(nSize >nMaxSize)
		{
			nMaxSize = nSize;
		}
		nTotalSize += nSize;
		nSendingSize += nSize;
	}

	pPacketList = m_FreeSendPackList;

	for (INT_PTR i=m_FreeSendPackList.count()-1; i>-1; --i)
	{
		nSize = pPacketList[i]->getMemorySize();
		if(nSize >nMaxSize)
		{
			nMaxSize = nSize;
		}
		nTotalSize += nSize;
		freeSendSize += nSize;
	}
	if(pSendingSize )
	{
		*pSendingSize = nSendingSize;
	}
	if(pFreeSendSize )
	{
		*pFreeSendSize = freeSendSize;
	}
	if(pMaxSize )
	{
		*pMaxSize = nMaxSize;
	}
	return nTotalSize;
}

VOID CSendPacketPool::FreeAllPackets()
{
	INT_PTR i;
	CDataPacket **pPacketList;

	m_SendingPacketList.flush();

	//销毁发送中的数据包
	m_SendingPacketList.lock();



	pPacketList = m_SendingPacketList;
	for (i=m_SendingPacketList.count()-1; i>-1; --i)
	{
		pPacketList[0]->~CDataPacket();
		pPacketList++;
	}
	m_SendingPacketList.clear();
	m_SendingPacketList.unlock();

	m_FreeSendPackList.lock();

	//销毁空闲的数据包
	pPacketList = m_FreeSendPackList;
	for (i=m_FreeSendPackList.count()-1; i>-1; --i)
	{
		pPacketList[0]->~CDataPacket();
		pPacketList++;
	}
	m_FreeSendPackList.clear();

	//释放所有数据包内存
	/*
	for (i=m_SendPacketMemList.count()-1; i>-1; --i)
	{
		m_Allocator.FreeBuffer(m_SendPacketMemList[i]);
	}
	m_SendPacketMemList.clear();
	*/


	m_FreeSendPackList.unlock();

	//让内存管理器释放内存
	m_Allocator.CheckFreeBuffers();
}

VOID CSendPacketPool::allocSendPacketList(CBaseList<CDataPacket*>& packetList, INT_PTR nAllocCount, CDataPacket **ppSingleAlloc)
{
	static const INT_PTR PacketBlockCountAlgin = 512;
	INT_PTR nCount, nRemainCount;

	m_FreeSendPackList.lock();

	nCount = m_FreeSendPackList.count();

	//如果空闲包多过100个，则释放掉50个，保持50个包空闲使用就好了
	
	m_dwPackAllocCount ++;
	//过1千个则重新设置为0，触发1轮的检测,并且保留前面的前10个包，经常用到的，不进行释放，供后面使用
	if(m_dwPackAllocCount >= 1000 && nCount > 100)  
	{
		
		for( INT_PTR i= nCount -30 ; i >=0; i-- )
		{
			CDataPacket * pPack = m_FreeSendPackList[i];
			if(pPack && pPack->getMemorySize() >0)
			{
				pPack->~CDataPacket();  //是否内存 
			}
			else
			{
				break; //后面没必要检查了
			}
		}
		m_dwPackAllocCount =0;
	}
	
	//从当前空闲列表中拷贝
	if ( nCount > 0 && nAllocCount > 0 )
	{
		nRemainCount = nCount - nAllocCount;
		if ( nRemainCount > 0 )
		{
			packetList.addArray(&m_FreeSendPackList[nRemainCount], nAllocCount);
			m_FreeSendPackList.trunc(nRemainCount);
			nAllocCount = 0;
		}
		else
		{
			//添加现有的
			packetList.addArray(m_FreeSendPackList, nCount);
			m_FreeSendPackList.trunc(0);
			nCount = 0;
		}
		nCount = m_FreeSendPackList.count();
	}
	if ( ppSingleAlloc && nCount > 0 )
	{
		nCount--;
		*ppSingleAlloc = m_FreeSendPackList[nCount];
		m_FreeSendPackList.trunc(nCount);
		ppSingleAlloc = NULL;//将参数置空，以便后续的操作不需要多申请一个
	}

	//仍不足需求数量则继续申请
	if ( ppSingleAlloc || nAllocCount > 0 )
	{
		//申请数据包对象内存块
		INT_PTR i, nNewCount = __max(PacketBlockCountAlgin, nAllocCount);
		//如果还需要申请一个单独的数据包，则增加需求数量
		if ( ppSingleAlloc ) nNewCount ++;
		nNewCount = (nNewCount + PacketBlockCountAlgin) & (~(PacketBlockCountAlgin - 1));
		CDataPacket *pPacket = (CDataPacket*)m_Allocator.AllocBuffer(sizeof(*pPacket) * nNewCount);
		//讲数据包集的内存块添加到数据内存头列表中
		//m_SendPacketMemList.add(pPacket);
		//如果空闲发送送数据列表的剩余空间不足以放下新申请的数据包，则增长空闲发送数据包列表的保留长度
		if ( m_FreeSendPackList.maxCount() < nCount + nNewCount )
			m_FreeSendPackList.reserve(nCount + nNewCount);
		//循环调用构造函数
		for (i=0; i<nNewCount; ++i)
		{
			new(pPacket)CDataPacket(&m_Allocator);
			m_FreeSendPackList[nCount] = pPacket;//将数据包保存到空闲发送送数据列表
			pPacket++;
			nCount++;
		}
		//将剩余申请的数据包拷贝到申请列表中
		if ( nAllocCount > 0 ) 
		{
			nCount -= nAllocCount;
			packetList.addArray(&m_FreeSendPackList[nCount], nAllocCount);
			m_FreeSendPackList.trunc(nCount);
		}
		//需要申请一个单独的数据包
		if ( ppSingleAlloc )
		{
			nCount--;
			*ppSingleAlloc = m_FreeSendPackList[nCount];
			m_FreeSendPackList.trunc(nCount);
		}
	}
	m_FreeSendPackList.unlock();
}

void CSendPacketPool::clearSendList()
{
	INT_PTR i;
	CDataPacket **pPacketList;

	m_SendingPacketList.flush();

	m_SendingPacketList.lock();
	//情况所有发送队列中数据包的长度
	pPacketList = m_SendingPacketList;
	for (i=m_SendingPacketList.count()-1; i>-1; --i)
	{
		pPacketList[i]->setLength(0);
	}
	m_SendingPacketList.unlock();


	m_FreeSendPackList.lock();

	//将发送队列中的数据包还原到空闲队列中
	m_FreeSendPackList.addArray(m_SendingPacketList, m_SendingPacketList.count());

	//重置发送队列索引为0
	m_nSendingPacketIdx = 0;
	m_FreeSendPackList.unlock();

}

size_t CSendPacketPool::sendToSocket(CCustomSocket& socket)
{
	INT_PTR nCount, nAvalLength, nBytesWriten;
	CDataPacket **pPacketList, *pPacket;
	size_t nTotalSent = 0;

	//提交追加到发送队列中的数据包
	m_SendingPacketList.flush();

	//循环发送数据包
	pPacketList = m_SendingPacketList;
	nCount = m_SendingPacketList.count();
	for (; m_nSendingPacketIdx<nCount; ++m_nSendingPacketIdx)
	{
		pPacket = pPacketList[m_nSendingPacketIdx];
		nAvalLength = pPacket->getAvaliableLength();
		if (nAvalLength > 0)
		{
			//发送数据
			nBytesWriten = socket.send(pPacket->getOffsetPtr(), (INT)nAvalLength);
			if ( nBytesWriten <= 0 )
				break;
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
	if ( m_nSendingPacketIdx >= nCount )
	{
		m_nSendingPacketIdx = 0;
		m_FreeSendPackList.lock();
		m_FreeSendPackList.addArray(m_SendingPacketList, m_SendingPacketList.count());
		m_SendingPacketList.trunc(0);
		m_FreeSendPackList.unlock();
	}
	return nTotalSent;
}
