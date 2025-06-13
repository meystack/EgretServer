#ifndef _CUSTOM_WORK_SOCKET_H_
#define _CUSTOM_WORK_SOCKET_H_

/************************************************************
*
*	工作套接字连接模型类
*
*		数据的发送使用队列堆积的形式，而不是使用缓冲区堆积
*	的方式，所有提交的待发送的数据将被保存到一个列表中排队
*	发送，在添加发送数据的时候，可以指定是否优先发送以便将
*	数据插入到待发队列的头部而优先发送。
*
*       凡是当链接断开的时候，所有已经提交到发送队列中的数据包均
*   不会被销毁或丢弃，会在链接建立后继续发送。因此，如果希望在连
*   接断开后立刻清空发送队列，则子类必须覆盖Disconnected函数或
*   在OnDisconnected通知函数中调用clearSendList。
*
***********************************************************/

using namespace wylib::time::tick64;
using namespace wylib::container;
using namespace wylib::sync::lock;

class CCustomWorkSocket : 
	public wylib::inet::socket::CCustomSocket,
	public CSendPacketPool
{
public:
	typedef CCustomSocket Inherited;

public:
	CCustomWorkSocket();
	virtual ~CCustomWorkSocket();

	//发送内部消息
	VOID PostInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4=0);
	//获取连接的时间
	TICKCOUNT getConnectTick(){ return m_dwConnectTick;}

	//初始化网络库（调用WSAStartup）,返回0表示成功
	static INT InitSocketLib();
	//卸载网络库（调用WSACleanup）
	static VOID UnintSocketLib();

protected:
	/**** 以下函数集为子类可能需要覆盖的函数 ****/
	//处理接受到的服务器数据包
	virtual VOID ProcessRecvBuffers(PDATABUFFER pDataBuffer);
	//进行内部消息的处理分派
	virtual VOID DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4);
	//每次逻辑循环的例行逻辑处理函数
	virtual VOID OnRun();
	//例行执行函数
	virtual VOID SingleRun();

protected:
	//交换接收、处理的缓冲包
	VOID SwapRecvProcessBuffers();
	//清除所有待发数据
	VOID ClearSendBuffers();
	//添加待发送数据
	//★★应当尽量使用allocSendPacket以及flushSendPacket进行数据发送★★
	VOID AppendSendBuffer(LPCVOID lpData, const size_t dwSize);
	//读取套接字数据
	VOID ReadSocket();
	//发送数据队列中的数据
	VOID SendSocketBuffers();
	//处理内部消息列表
	VOID ProcessInternalMessages();
	//获取接收数据缓冲，仅为子类提供
	inline const PDATABUFFER getRecvBuffer(){ return m_pRecvBuffer; }
	//获取接受处理缓冲，仅为子类提供
	inline const PDATABUFFER getProcRecvBuffer(){ return m_pProcRecvBuffer; }
	//获取内部消息队列中的消息数量，仅为子类提供
	inline INT_PTR getInternalMessageCount(){ return m_ItnMsgList.count(); }
	inline void SetSendDataInSingleRun(bool bFlag) { m_bSendData = bFlag; }
protected:
	//覆盖父类断开连接的处理函数
	VOID Disconnected();

protected:
	TICKCOUNT				m_dwMsgTick;			//上次通信的时间
	CBufferAllocator		m_Allocator;			//内存池
	TICKCOUNT				m_dwConnectTick;		//连接建立的时间
	TICKCOUNT				m_dwFreeMemoryTime;		//清理内存的时间
	 
private:
	DATABUFFER				m_RecvBuffers[2];		//网络数据接收和处理缓冲器
	PDATABUFFER				m_pRecvBuffer;			//接收数据缓冲区
	PDATABUFFER				m_pProcRecvBuffer;		//处理接收到的数据缓冲区
	CQueueList<PAPPINTERNALMSG> m_ItnMsgList;		//内部消息列表
	CCSLock					m_ItnMsgListLock;		//内部消息列表锁
	bool					m_bSendData;			// 标记在SingleRun是否发送数据
};


#endif

