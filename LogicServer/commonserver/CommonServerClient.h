#ifndef COMMON_SERVER_CLIENT_H_
#define COMMON_SERVER_CLIENT_H_

/*
	公共服务器负责处理和客户端连接的数据发送接受逻辑类。公共辅服务器接受客户端（普通逻辑服务器）
	连接后就创建一个CCommonServerClient对象，用于和客户端通信。
*/
class CCommonServerClient : public CCustomJXServerClientSocket
{
	friend class CCommonServer;
public:	
	typedef CCustomJXServerClientSocket Inherited;

	CCommonServerClient(CCommonServer *lpCommonServer, SOCKET nSocket, PSOCKADDR_IN pAddrIn);
	virtual ~CCommonServerClient();

	/*
	* Comments: 回收空闲的CDataPacket类
	* Param CDataPacket * pPacket:
	* @Return void:
	* @Remark: 收到数据包之后，本地分配数据包对象将内容缓存起来，然后通过逻辑线程处理。
			   逻辑线程处理完数据后，调用此接口释放数据包对象。
	*/
	void FreeBackUserDataPacket(CDataPacket* pPacket);

protected:
	// CCustomSocket 虚函数
	VOID OnDisconnected();
	VOID OnError(INT errorCode);
	// CCustomWorkSocket 虚函数
	VOID OnRun();

	// CCustomJXServerClientSocket
	VOID OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket);
	bool OnValidateRegData(const jxSrvDef::PSERVER_REGDATA pRegData);

	// 由CommonServer调用的例行执行函数
	inline VOID Run() { SingleRun(); }

	CDataPacket* AllocSendPacket();

private:
	CQueueList<CDataPacket*>			m_vFreeList;	//用来存放空闲的数据包
	CCSLock								m_vFreeListLock;
};

#endif