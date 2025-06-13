#ifndef _CUSTOM_SERVER_GATE_H_
#define _CUSTOM_SERVER_GATE_H_

#include"Define.h"
class CCustomGateManager;
class CCustomServerGateUser;



/*******************************************************
*
*			《剑啸江湖》服务器通用网关连接类
*
*    外部逻辑使用时需注意连接断开后解除对网关用户的使用以免因对
*  象被复用而导致的数据过期的问题。
*
********************************************************/

/* ★ 定义_USE_GATE_SEND_PACK_LIST_宏将使用发送列表来处理发送数据包
     当使用发送列表管理发送数据包时，可免去将发送数据拷贝到发送缓冲的内存
	 拷贝操作（这将是取消此宏定义的发送数据处理方式）。但使用发送队列的弊
	 端是在申请发送数据包对象以及提交发送数据包对象的时候都需要获取临界区
	 锁，即前后获取两次锁。

   ★ 对于不使用发送列表来处理要发送的数据包，将只在提交发送数据的时候获取
     一次数据发送数据缓冲区交换锁，但免除一次锁的代价是需要进行内存的拷贝，
	 将提交的发送数据内容拷贝到发送提交缓冲区中。

   ★ 使用_USE_GATE_SEND_PACK_LIST_宏来使用发送列表时，能够最大化提
     升效率的方式至少有以下两种：
	   1）  在会提交发送数据的线程内部建立一个数据包列表，使用函数
	     AllocGateSendPacketList一次性申请多个发送数据包，在一定数量的
		 数据包填充完毕后，调用FlushGateSendPacketList一次性全部提交一个
		 列表中的数据包，这种方法能够提升对发送数据包申请/提交的的效率为
		 申请/提交的“数量”倍以上，将多次的获取锁的操作合并为一次！
		   使用此方法需要注意：提交的发送数据包的队列中，每个数据包的读写
		 偏移量必须为0，FlushGateSendPacketList函数不会修改提交数据包列表
		 中任何数据包的偏移指针！此外每个数据包的内容必须是以一个网关通信
		 协议头（GATEMSGHDR）结构开始的数据且协议头必须被填充必要的成员！

	   2）  在一个能够产生多次数据包申请/提交的操作前（逻辑循环最外层的
	     部分）申请一个数据包对象，所有的发送数据全部追加到这个唯一的数
		 据包中，并在操作后提交这个数据包。能够提升对发送数据包申请/提交
		 的效率为向整个数据包中写入的通信数据的次数倍以上。
		   使用此方法需要注意：每次向数据包中写入一份通信数据内容前必须写入
		 一个网关通信协议头（GATEMSGHDR）且协议头必须被填充必要的成员！
	  
*/
#define _USE_GATE_SEND_PACK_LIST_

using namespace wylib::time::tick64;
using namespace wylib::container;

class CCustomServerGate 
	: public CCustomServerClientSocketEx
{
	friend class CCustomGateManager;
public:
	typedef CCustomServerClientSocket Inherited;

public:
#ifndef _USE_GATE_SEND_PACK_LIST_
	/* 添加向网关发送的数据，数据必须包含了网关通信协议头 */
	VOID AddGateBuffer(LPCVOID pBuffer, SIZE_T nBufferSize);
	/* 添加向网关发送的用户数据，数据为不包含网关通信协议头的，函数会自动追加网关通信协议头 
	 * pUser				网用户套接字句柄
	 * nGateSessionIndex	用户在网关中的会话索引
	 */
	VOID AddUserBuffer(UINT64 nUserSocket, INT_PTR nGateSessionIndex, LPCVOID pBuffer, SIZE_T nBufferSize);
#else
	/* 申请一份发送数据包
	 * ★申请的数据包中已经写入了网关通信协议头（GATEMSGHDR）且头中必要的成员也已经被填充值，
	 *  在使用此数据包的过程中，可以调整的数据包的读写偏移量最小为sizeof(GATEMSGHDR)，如果
	 *  小于此值将会覆盖数据包中的网关通信协议头，使用时请注意！★
	 * 当发送数据包填充内容后应当调用FlushSendPacket提交发送数据
	 *
	 * nUserSocket			网用户套接字句柄
	 * nGateSessionIndex	用户在网关中的会话索引
	 */
	CDataPacket& AllocGateSendPacket(UINT64 nUserSocket, int nGateSessionIndex,int nServerIndex =0);
	/* 提交一份数据包到发送队列中 
	 * 函数会依据数据包的数据长度计算用户数据长度并填充到数据包开头的网关协议头中数据长度的字段中
	 */
	VOID FlushGateSendPacket(CDataPacket& packet);
	//发送一个连串的数据包
	VOID FlushGateBigPacket(CDataPacket& packet);
	/* 申请一批发送数据包
	 * 当发送数据包填充内容后应当调用FlushSendPacket提交单个数据包或FlushSendPacketList提交多个数据包到发送队列
	 * ★★★attention★★★ 
	 * 申请的发送数据包的队列中，每个数据包的起始内存中均没有包含网关协议头，
	 * 在使用数据包之前必须自行向数据包中填充网关协议头！
	 * 每个数据包的内容必须是以一个GATEMSGHDR（网关通信协议头
	 * 结构开始的数据且协议头必须被填充必要的成员！
	 *
	 * packetList		申请的多个发送数据包将被保存在此列表中
	 * nAllocCount		要申请的发送数据包的数量
	 * ppSingleAlloc	用于申请单个数据包对象，如果参数非空，会实际申请nAllocCount+1个数据包，其中nAllocCount
	 *					个数据包保存在nAllocCount中并单独向*ppSingleAlloc中保存一个
	 */
	inline VOID AllocGateSendPacketList(CBaseList<CDataPacket*>& packetList, INT_PTR nAllocCount)
	{
		allocSendPacketList(packetList, nAllocCount);
	}
	/* 提交一批发送数据包
	 * ★★★attention★★★ 
	 * 提交的发送数据包的队列中，每个数据包的读写偏移量必须为0，
	 * 本函数不会修改提交数据包列表中任何数据包的偏移指针！此外
	 * 每个数据包的内容必须是以一个GATEMSGHDR（网关通信协议头
	 * 结构开始的数据且协议头必须被填充必要的成员！
	 *
	 * packetList	申请的多个发送数据包将被保存在此列表中
	 * nAllocCount	要申请的发送数据包的数量
	 */
	inline VOID FlushGateSendPacketList(CBaseList<CDataPacket*>& packetList)
	{
		flushSendPacketList(packetList);
	}
#endif
	/* 投递关闭网关用户的消息
	 * 依据套接字句柄关闭用户，效率较慢，需要遍历查找与nSocket对应的用户 
	 * 注意：关闭用户的过程是异步的，是出于多线程之间免锁数据安全考虑。
	 * 网关用户不会再函数执行期间内被关闭，函数只是向网关投递一个关闭用户的消息，
	 * 关闭用户的具体实现依赖于网关对象对内部消息的处理。
	 */
	inline VOID PostCloseUser(UINT64 nUserSocket)
	{
		PostInternalMessage(SGIM_CLOSE_USER_SOCKET, (UINT_PTR)nUserSocket, 0, 0);
	}
	/* 投递关闭网关用户的消息
	 * 依据套接字句柄以及用户在服务器的索引关闭用户，效率很快 
	 * 注意：关闭用户的过程是异步的，是出于多线程之间免锁数据安全考虑。
	 * 网关用户不会再函数执行期间内被关闭，函数只是向网关投递一个关闭用户的消息，
	 * 关闭用户的具体实现依赖于网关对象对内部消息的处理。
	 */
	inline VOID PostCloseUser(UINT64 nUserSocket, int nServerSessionIndex)
	{
		PostInternalMessage(SGIM_CLOSE_USER_SOCKET_SIDX, (UINT_PTR)nUserSocket, nServerSessionIndex, 0);
	}
	
	/*
	* Comments:
		* 投递按全局会话ID关闭网关用户
		* 依据全局会话ID关闭用户，效率较慢，需要遍历查找与nSessionID对应的用户 
		* 注意：关闭用户的过程是异步的，是出于多线程之间免锁数据安全考虑。
		* 网关用户不会再函数执行期间内被关闭，函数只是向网关投递一个关闭用户的消息，
		* 关闭用户的具体实现依赖于网关对象对内部消息的处理。
		*
	* Param const int nSessionID:会话的id
	* @Return VOID:
	*/
	inline VOID PostCloseUserByGlobalSessionId(const int nSessionID)
	{
		PostInternalMessage(SGIM_CLOSE_USER_SOCKET_GSID, nSessionID,0, 0);
	}

	/*
	* Comments:踢人下线
	* Param const int nSessionID:会话的di
	* @Return VOID:
	*/
	inline VOID PostKickUserByGlobalSessionId(const int nSessionID)
	{
		PostInternalMessage(SGIM_KICK_USER_SOCKET_GSID, nSessionID,0, 0);
	}

	/* 获取网关名称 */
	inline LPCSTR getGateName(){ return m_sGateName; }
	/* 获取网关编号 */
	inline INT_PTR getGateIndex(){ return m_nGateIndex; }
	// 获取网关用户信息
	inline GATEUSERINFO getGateUserInfo() { return m_GateUserInfo; }
protected:
	/*** 本protected函数集为子类的逻辑处理有必要覆盖的函数集 ***/
	/* ★创建网关用户对象，默认的返回值是new一个CCustomGateUser的实例 */
	virtual CCustomServerGateUser* CreateGateUser();
	/* ★销毁网关用户对象，默认的操作是delete此对象 */
	virtual VOID DestroyGateUser(CCustomServerGateUser* pUser);
	/* ★当一个用户连接后回调此函数 */
	virtual VOID OnOpenUser(CCustomServerGateUser *pUser);
	/* ★当一个网关用户关闭时回调此函数 */
	virtual VOID OnCloseUser(CCustomServerGateUser *pUser);
	/* ★当网关连接断开时调用此函数 */
	virtual VOID OnGateClosed();
	/* ★当需要处理网关转发给用户的一个完整的通信包时调用 
	 *pUser		数据包所属的网关用户对象
	 *pBuffer	通信数据包，数据包不是动态数据指针，在函数调用完成后指针所指向的数据内容不再有效。如要将数据保存，则必须将数据拷贝到单独的内存中。
	 */
	virtual VOID OnDispatchUserMsg(CCustomServerGateUser *pUser, char* pBuffer, SIZE_T nBufferSize) = 0;
	
	
	//关闭会话
	virtual VOID KickUserByGlobalSessionId(const int nGlobalSessionId);
	
protected:
	//处理网关套接字错误
	VOID SocketError(INT nErrorCode);
	//处理网关连接断开
	VOID Disconnected();
	//当长时间没有通信后发送保持连接的消息
	VOID SendKeepAlive();
	//处理接受到的服务器数据包
	VOID ProcessRecvBuffers(PDATABUFFER pDataBuffer);
	
	
	//VOID DispatchInternalMessage(UINT uMsg, UINT_PTR uParam1, UINT_PTR uParam2, UINT_PTR uParam3,UINT_PTR uParam4=0);
	//进行内部消息的处理分派
	virtual VOID DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4);
	
	
	//分派网关接收到的数据
	VOID DispathRecvMessage(PGATEMSGHDR pMsgHdr, char *pBuffer, SIZE_T nBufSize);
	//每次逻辑循环的例行逻辑处理函数，将实现数据缓冲中的数据发送到网关的功能
	VOID SingleRun();

	

	
	
protected:
	/*打开新用户
	 *nSocket		用户连接到网关的套接字句柄
	 *nSessionIndex 用户在网关中的会话索引
	 *sIPAddr		用户的IP地址字符串
	 */
	INT_PTR OpenNewUser(UINT64 nSocket, int nGateSessionIndex, char *sIPAddr);

	

	/*
	* Comments:依据套接字句柄关闭用户，效率很慢，需要遍历查找与nSocket对应的用户
	* Param const UINT64 nSocket: socketID
	* Param bool bNeedTellGate: 是否需要通过网关，true
	* @Return VOID:
	*/
	VOID CloseUser(const UINT64 nSocket,bool bNeedTellGate=true);
	
	/* 依据套接字句柄以及用户在服务器的索引关闭用户，效率很快 */
	VOID CloseUser(const UINT64 nSocket, const int nServerSessionIndex,bool bNeedTellGate=true);

	/* 依据全局会话ID关闭用户，效率很慢，需要遍历查找与nSessionId对应的用户 */
	VOID CloseUserByGlobalSessionId(const int nGlobalSessionId);

	/* 依据全局会话ID查找网关用户对象，效率很慢，需要遍历查找与nSessionId对应的用户
	 *pIndex	如果参数非空，当找到匹配的网关用户时会向其中填充用户在网关列表中的索引
	*/
	CCustomServerGateUser* UserExistsOfGlobalSessionId(const int nGlobalSessionId, PINT_PTR pIndex);
	/* 关闭所有用户 */
	VOID CloseAllUser();
	/* 将网关转发的用户数据发送给用户 */
	VOID PostUserData(PGATEMSGHDR pMsgHdr, char *pData, SIZE_T nDataSize);

	//关闭网关连接的程序
	VOID PostCloseGateServer();

#ifndef _USE_GATE_SEND_PACK_LIST_
	/* 将发送缓冲中的数据发送到网关中 */
	VOID SendGateBuffers();
#endif
	/* 处理关闭网关用户的事宜 */
	VOID GateUserClosed(CCustomServerGateUser *pGateUser,bool bNeedTellGate =true);

	/* 向网关发送关闭用户的消息 */
	VOID SendGateCloseUser(UINT64 nSocket, int nGateSessionIndex,int nServerIndex);
	/* 向网关发送新用户在本网关中被分配的会话索引 */
	VOID SendAcceptUser(UINT64 nSocket, int nGateSessionIndex, int nServerSessionIndex);
	/* 释放各个缓冲区内存 */
	VOID FreeBuffers();
public:
	CCustomServerGate();
	~CCustomServerGate();

private:
	CHAR					m_sGateName[32];		//网关名称
#ifndef _USE_GATE_SEND_PACK_LIST_
	DATABUFFER				m_SendBufQueue[2];		//两个数据缓冲区，当发送缓冲区为空则交换发送和追加缓冲区
	PDATABUFFER				m_pSendAppendBuffer;	//当前正在追加的管道数据缓冲区指针
	PDATABUFFER				m_pSendProcBuffer;		//当前正在写入管道的数据缓冲区指针
	CRITICAL_SECTION		m_WriteLock;			//数据写入锁
#endif
protected:
	INT_PTR					m_nGateIndex;			//网关编号
	CBaseList<CCustomServerGateUser*>	m_UserList;	//用户列表
	bool					m_bHasPrintErrInfo;		// 是否已经输出了错误信息（为降低性能开销，只输出一次）
	GATEUSERINFO			m_GateUserInfo;			// 网关用户信息
private:
	/*	
	 * 定义线程内部消息号
	 *
	*/
	//按用户SOCKET关闭用户,Param1 = Socket
	static const UINT SGIM_CLOSE_USER_SOCKET		= 101;
	//按用户SOCKET以及用户在服务器中的会话编号关闭用户，Param1=Socket, Param2=ServerSessionIndex
	static const UINT SGIM_CLOSE_USER_SOCKET_SIDX	= 102;
	//按用户的全局会话ID关闭用户，Param1=全局会话ID
	static const UINT SGIM_CLOSE_USER_SOCKET_GSID	= 103;
	
	//会话踢人下线，在跨服处理的时候和103有点差别
	static const UINT SGIM_KICK_USER_SOCKET_GSID	= 104;

};

#endif
