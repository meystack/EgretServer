#ifndef _DATA_CLIENT_H_
#define	_DATA_CLIENT_H_

/***
	本类主要用于逻辑服务器连接数据服务器，并发送信息	
*/

class CCustomJXClientSocket;
class CLogicServer;

//保存认证信息的时间  10分钟超时超时
#define KEEPSESSIONDATA		600000

//保存加载用户认证信息的结构
typedef struct 
{
	unsigned int	nSeesionId;
	unsigned int 	nActorid;
	unsigned long long   dwTime;
} LOADACTORSTRUCT;

class CDataClient:
	public CCustomJXClientSocket
{
	typedef VOID (CDataClient::*OnHandleSockPacket)(CDataPacketReader &packet);
public:
	CDataClient(CLogicServer * pLogicServer);
	~CDataClient();
public:
	// 以下函数用于逻辑服务器向数据服务器发送查询信息

	//数据服务器返回角色的状态
	BOOL OnQuerySessionData(UINT_PTR nSessionId,unsigned int nActorId);

	/*
	* Comments: 向db服务器发送简单的数据
		如果是复杂数据使用 allocProtoPacket，然后写入数据，再flushProtoPacket
	* Param INT_PTR nCmd: 命令码,如dcLoadQuest
	* Param T data: 数据，可以是任何类型的
	* @Return VOID:
	*/
	template<class T>
	inline VOID SendDbServerData( INT_PTR nCmd ,T & data)
	{
		CDataPacket& DataPacket = allocProtoPacket((jxSrvDef::INTERSRVCMD)nCmd);
		DataPacket << data;
		flushProtoPacket(DataPacket);
	}

	//等待数据发送或转储完成
	void WaitFor();
	
public:
	// 回收空闲的CDataPacket类
	void FreeBackUserDataPacket(CDataPacket* pDataPacket);
	

protected:

	VOID OnLoadActorMsg(CDataPacketReader &inPacket);

	// 以下是实现基类的虚函数
	virtual VOID OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket);

	virtual jxSrvDef::SERVERTYPE getLocalServerType();

	/* ★查询本地服务器的名称，以便正确的发送注册数据 */
	virtual LPCSTR getLocalServerName();
	/* ★查询本地服务器ID，以便正确的发送注册数据，默认的返回值是0 */
	virtual int getLocalServerIndex();

	CLogicServer* m_pLogic;

	//提供向最外层逻辑通知连接建立的事件
	virtual VOID OnConnected();

	virtual void OnRountine()
	{
		CCustomJXClientSocket::OnRountine();
	}

	////提供向最外层逻辑通知连接断开的事件
	//virtual VOID OnDisconnected()
	//{

	//}

private:
	// 分配一个CDataPacket
	CDataPacket* AllocSendPacket();

private:
	static const OnHandleSockPacket		SockPacketHandlers[];	//结构化网络数据包处理函数列表

	CQueueList<CDataPacket*>			m_vFreeList;	//用来存放空闲的数据包
	CCSLock								m_vFreeListLock;

	CBaseList<LOADACTORSTRUCT>			m_vSeesionData;	//用来保存用户认证信息的队列
	RTL_CRITICAL_SECTION				m_SessionLock;
};
#endif
