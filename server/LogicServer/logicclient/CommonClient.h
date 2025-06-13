#ifndef COMMON_CLIENT_H_
#define COMMON_CLIENT_H_

/*
	此类实现了普通逻辑服务器连接公共逻辑服务器的客户端逻辑功能。普通逻辑服务器在引擎启动后创建此类对象，
	设置公共服务器地址信息并且连接到公共逻辑服务器。一旦建立到公共服务器的连接后，就可以发送和接受消息。
	发送消息给公共逻辑服务器：
		CDataPacket &packet = allocProtoPacket(nCmdId);
		// 填充消息数据包
		fluashProtoPacket(packet);
	引擎调用方式：
		CComClient *pCClient = GetGlobalEngine()->GetCommonClient();
		CDataPacket &packet = pCClient->allocProtoPacket(nCmdId);
		// 填充消息数据包
		pCClient->fluashProtoPacket(packet);

	接受来自公共服务器的消息：
		OnDispatchRecvPacket接受消息并且将消息转发给NetWorkHandler里头，供逻辑线程处理。在NetWorkHandler
	类里头实现对应消息的派发处理即可。
*/
class CCommonClient : public CCustomJXClientSocket
{
public:
	typedef CCustomJXClientSocket Inherited;
	CCommonClient();
	virtual ~CCommonClient();
		
	/*
	* Comments: 回收空闲的CDataPacket类
	* Param CDataPacket * pPacket:
	* @Return void:
	* @Remark: CommonClient收到来自CommonServer的数据包之后，本地分配数据包对象将内容缓存起来，然后通过逻辑线程处理。
			   逻辑线程处理完数据后，调用此接口释放数据包对象。
	*/
	void FreeBackUserDataPacket(CDataPacket* pPacket);

protected:
	// 以下是实现基类的虚函数
	virtual VOID OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket);

	virtual jxSrvDef::SERVERTYPE getLocalServerType();

	/* ★查询本地服务器的名称，以便正确的发送注册数据 */
	virtual LPCSTR getLocalServerName();
	/* ★查询本地服务器ID，以便正确的发送注册数据，默认的返回值是0 */
	virtual int getLocalServerIndex();

	virtual void OnRountine()
	{
		CCustomJXClientSocket::OnRountine();
	}

	/*
	* Comments: 分配数据包，用于存储接收到的数据
	* @Return CDataPacket*:
	* @Remark: 接收线程接收到数据并非立即处理，而是缓存起来，交给逻辑线程统一处理
	*/
	CDataPacket* AllocSendPacket();

private:
	CQueueList<CDataPacket*>			m_vFreeList;	//用来存放空闲的数据包
	CCSLock								m_vFreeListLock;
};

#endif
