#pragma once

/************************************************************************
                         网络数据包队列缓冲池

   设计宗旨：
      1、管理网络数据包的发送，按提交数据包的顺序进行发送；
	  2、通过数据包申请和提交的方式免除构造数据包后发送时进行数据包内容拷贝的开销；
	  3、使用内存池（常驻内存）优化内存管理；

   设计原理：
      将网络数据包保存到列表中，发送时循环从列表中逐个发送。会处理发送过程中对一个
   数据包只发送了一部分的情况，这种情况下将在下次发送时继续发送一个数据包未发送完成
   的部分。
      希望发送网络数据包的调用者需调用allocSendPacketList或allocSendPacketList
   函数取得一个或多个数据包对象。申请后的数据包内容已被清空，可直接写入数据。当数据
   包数据写入完成后，调用flushSendPacket或flushSendPacketList函数将数据包提
   交到发送队列中。
      ★调用者申请一个发送数据包对象后必须在队列缓冲池被销毁前提交数据包；
      数据发送者需调用sendToSocket函数将发送队列中的数据包通过socket对象发送，
   在一次sendToSocket的调用中并不一定会将发送队列的数据包全部发送完成，除非socket
   是一个阻塞式的套接字。当一次调用无法发完所有数据包时，则会在下次调用中继续发送后
   续的部分。

   能够最大化提升效率的方式至少有以下两种：
	   1）  在会提交发送数据的线程内部建立一个数据包列表，使用函数
	     AllocSendPacketList一次性申请多个发送数据包，在一定数量的
		 数据包填充完毕后，调用FlushSendPacketList一次性全部提交一个
		 列表中的数据包，这种方法能够提升对发送数据包申请/提交的的效率为
		 申请/提交的“数量”倍以上，将多次的获取锁的操作合并为一次！
		   使用此方法需要注意：提交的发送数据包的队列中，每个数据包的读写
		 偏移量必须为0，FlushSendPacketList函数不会修改提交数据包列表
		 中任何数据包的偏移指针！

	   2）  在一个能够产生多次数据包申请/提交的操作前（逻辑循环最外层的
	     部分）申请一个数据包对象，所有的发送数据全部追加到这个唯一的数
		 据包中，并在操作后提交这个数据包。能够提升对发送数据包申请/提交
		 的效率为向整个数据包中写入的通信数据的次数倍以上。

************************************************************************/

using namespace wylib::container;

class CSendPacketPool
{
public:
	CSendPacketPool();
	~CSendPacketPool();
	
	/* 申请一份发送数据包
	 * 当发送数据包填充内容后应当调用FlushSendPacket提交发送数据
	 * ★调用者申请一个发送数据包对象后必须在队列缓冲池被销毁前提交数据包；
	 */
	inline CDataPacket& allocSendPacket()
	{
		CDataPacket *pPacket;
		allocSendPacketList(*((CBaseList<CDataPacket*>*)NULL), 0, &pPacket);	
		pPacket->setLength(0);
		return *pPacket;
	}
	/* 提交一份数据包到发送队列中 
	 */
	inline VOID flushSendPacket(CDataPacket& packet)
	{
		//检查数据包内存是否被破坏
		packet._Assert();
		//如果数据包中被写入了数据则提交到发送队列，否则还原数据包到空闲队列
		if (packet.getLength() > 0)
		{
			//调整数据包偏移为0，才能在发送数据的从数据包头部开始发送
			packet.setPosition(0);
			//将数据包追加到发送队列中
			m_SendingPacketList.append(&packet);
		}
		else
		{
			m_FreeSendPackList.lock();
			m_FreeSendPackList.add(&packet);
			m_FreeSendPackList.unlock();
		}
	}
	/* 申请一批发送数据包
	 * 当发送数据包填充内容后应当调用FlushSendPacket提交单个数据包或FlushSendPacketList提交多个数据包到发送队列
	 *
	 * packetList		申请的多个发送数据包将被保存在此列表中
	 * nAllocCount		要申请的发送数据包的数量
	 * ppSingleAlloc	用于申请单个数据包对象，如果参数非空，会实际申请nAllocCount+1个数据包，其中nAllocCount
	 *					个数据包保存在nAllocCount中并单独向*ppSingleAlloc中保存一个
	 * ★remarks：申请后的数据包（包括ppSingleAlloc和packetList中存储的数据包）均已经被清空数据长度值，可以直接写入内容
	 * ★调用者申请一个发送数据包对象后必须在队列缓冲池被销毁前提交数据包；
	 */
	VOID allocSendPacketList(CBaseList<CDataPacket*>& packetList, INT_PTR nAllocCount, CDataPacket **ppSingleAlloc = NULL);
	/* 提交一批发送数据包
	 *
	 * packetList	数据包列表
	 * ★★★注意★★★  每个数据包将从其当前的Postion处发送到Length处，并非整个数据包的数据都被发送
	 */
	inline VOID flushSendPacketList(CBaseList<CDataPacket*>& packetList)
	{
		m_SendingPacketList.appendArray(packetList, packetList.count());
	}
	/* 情况发送队列中的数据包
	 * 通常需要在套接字断开连接的时候调用
	 *                ★★★注意★★★ 
	 * 该函数只能由负责调用sendToSocket函数的线程调用，如果被其他线程调用则
	 * 调用者必须妥善处理发送线程与调用线程之间关于数据发送处理的数据安全
	 */
	VOID clearSendList();
	/* 将发送数据队列中的数据包通过套接字发送
	 *
	 * socket	发送数据包的套接字对象
	 * @return	返回发送的总字节数
	 */
	size_t sendToSocket(wylib::inet::socket::CCustomSocket& socket);
	/* 获取队列中等待发送的数据包数量
	 * @return 还有多少个数据包等待发送
	 */
	inline size_t getPacketCount(){ return m_SendingPacketList.count() + m_SendingPacketList.appendCount(); }

	/*
	* Comments: 在数据包内存分配池cache的自由内存太多的时候，执行内存释放。
	* Param bool bIgnoreInterval: 是否强制（忽略内存分配器的定时清理间隔）清理
	* Param int nMaxFreeMemorySize: 需要清理的内存的大小，超过了这个内存大小就会调用清理
	* @Return void:
	* @Remark: 针对短期高频调用数据包分配内存，造成大量内存无法得到释放引起的服务器当机问题。
	*/
	inline void CheckMemory(bool bIgnoreInterval = false,int nMaxMemorySize = 30 * 1024 * 1024) 
	{
		
		static const size_t sMinSize =1* 1024 * 1024; //最小的检测内存的下限，防止参数有问题，小于这个就不
		
		CBufferAllocator::ALLOCATOR_MEMORY_INFO mi;
		m_Allocator.GetMemoryInfo(&mi);
		size_t sumSize = mi.SmallBuffer.dwFreeSize + mi.MiddleBuffer.dwFreeSize 
						 + mi.LargeBuffer.dwFreeSize + mi.SuperBuffer.dwFreeSize;
		if(sumSize < sMinSize) return; //如果少于1m，没有必要

		if (sumSize > (size_t)nMaxMemorySize)
		{
			//OutputMsg(rmTip,"SendPackPool check memory,free size=%d",)
			m_Allocator.CheckFreeBuffers(bIgnoreInterval);
		}	

	}
	//获取内存管理器，用于分析内存使用情况
	CBufferAllocator &GetBuffAlloc()
	{
		return m_Allocator;
	}

	
	/*
	* Comments:获取整个内存的占用
	* Param size_t * pSendingSize:发送的数据包的总长度,指针为0就不需要返回，否则返回长度
	* Param size_t * pFreeSendSize:空闲的总长度，指针为0就不需要返回，否则返回长度
	* Param size_t * pMaxSize:最大的包的长度，指针为0就不需要返回，否则返回长度
	* @Return size_t:
	*/
	size_t GetMemoryUsage(size_t *pSendingSize=NULL,size_t* pFreeSendSize=NULL,size_t* pMaxSize=NULL );
	

private:
	//销毁并释放所有的数据包
	VOID FreeAllPackets();
	
private:
	CBufferAllocator			m_Allocator;		//内存块管理器

protected:	
	CCSLock						m_SendingPacketLock;//发送中的数据包队列锁
	CCSLock						m_FreeSendPackLock;	//空闲的数据包队列锁
	INT_PTR						m_nSendingPacketIdx;//发送队列中的起始发送位置
	CQueueList<CDataPacket*>	m_SendingPacketList;//发送中的数据包队列
	CLockList<CDataPacket*>		m_FreeSendPackList;	//空闲的数据包队列
	INT_PTR                     m_dwPackAllocCount;  //这批次申请的包的数目

	
	//CBaseList<CDataPacket*>		m_SendPacketMemList;//数据包内存块指针列表
	//CBaseList<CDataPacket*>		m_SinglePacketList;	//在申请单个数据包的时候临时用的列表
};
