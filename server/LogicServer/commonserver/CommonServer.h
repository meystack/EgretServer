#ifndef COMMON_SERVER_H_
#define COMMON_SERVER_H_

/*
	功能：此类是跨服服务器用于连接普通的逻辑服务器的连接Server类。公共逻辑服务器在启动引擎的时候
	创建此对象，并且在指定的地址上监听来自普通逻辑服务器的连接。建立和普通服务器的连接之后，就由
	CCommonServerClient类负责和客户端的连接处理。CCommonServer只是负责连接的建立和管理，不
	负责具体连接数据的处理。
		公共服务器发送给普通逻辑服务器的数据，需要找到对应的链接。可以根据ServerIndex来进行索引。
	虽然说用ServerIndex索引每次都需要遍历，但是考虑到逻辑服务器之间通信不是很频繁（定时存盘数据）
	并且断开后重新连上不会有太多影响等等，还是采取索引方式来连接。
	逻辑引擎调用CCommonServer::SendData(serverIndex, pData, nLen)来发送数据。接收数据在
	CCommonServerClient里头处理
		！！！！！！！！！！！！！！！！暂时作废
*/
class CCommonServerClient;
class CCommonServer : public CCustomServerSocket
{
public:
	typedef CCustomServerSocket Inherited;
	CCommonServer();
	~CCommonServer();

	//覆盖创建连接对象的函数
	CCustomServerClientSocket* CreateClientSocket(SOCKET nSocket, PSOCKADDR_IN pAddrIn);
	//覆盖销毁一个已经建立的客户端对象
	VOID DestroyClientSocket(CCustomServerClientSocket *pClientSocket);

	//覆盖处理客户端连接的函数
	VOID ProcessClients();	
	//覆盖内部消息分派函数
	VOID DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4);
	//覆盖例行执行函数
	VOID SingleRun();
	//覆盖父类调用例行RUN的函数
	VOID OnRun();

	/*
	* Comments: 往指定的服务器发送消息，供外部线程调用。最大数据包长度为81960字节
	* Param int nServerIndex: 目标服务器ServerIndex
	* Param const char * pData:
	* Param const size_t nLen:
	* @Return void:
	* @Remark: 遍历的方式效率很低，最好做成索引的方式！ TODO
	*/
	void SendData(int nServerIndex, const void *pData, const size_t nLen);

private:
	//处理已经关闭的数据客户端对象
	VOID ProcessClosedClients();

	//// 供外部线程调用，需要加锁
	//BOOL GetSession(const INT_PTR nSessionId, OUT PGAMESESSION pSession);
	//// 供内部调用，无需加锁
	//PGAMESESSION GetSessionPtr(const INT_PTR nSessionId, PINT_PTR lpIndex);
private:
	//CBaseList<CCommonServerClient*>		m_CloseClientList;	//已经关闭的数据客户端列表
	/*CQueueList<PGAMESESSION>	m_SessionList;
	CCSLock						m_SessionListLock;*/
};

#endif