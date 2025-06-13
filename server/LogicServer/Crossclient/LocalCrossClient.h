#ifndef LOCAL_CENTER_CLIENT_H_
#define LOCAL_CENTER_CLIENT_H_

/*
	CopyRight@2021 跨服客户端模块

	此类实现了普通逻辑服务器连接跨服逻辑服务器的客户端逻辑功能。普通逻辑服务器在引擎启动后创建此类对象，
	设置公共服务器地址信息并且连接到公共逻辑服务器。一旦建立到跨服服务器的连接后，就可以发送和接受消息。
	发送消息给公共逻辑服务器：
	CDataPacket &packet = allocProtoPacket(nCmdId);
	// 填充消息数据包
	fluashProtoPacket(packet);
	引擎调用方式：
	CComClient *pCClient = GetGlobalEngine()->GetCommonClient();
	CDataPacket &packet = pCClient->allocProtoPacket(nCmdId);
	// 填充消息数据包
	pCClient->fluashProtoPacket(packet);

	接受来自跨服服务器的消息：
		OnDispatchRecvPacket接受消息并且将消息转发给NetWorkHandler里头，供逻辑线程处理。在NetWorkHandler
	类里头实现对应消息的派发处理即可。
	author: 马海龙
	time: 2021-04-28
*/
class CLocalCrossClient : public CCustomJXClientSocket
{
public:
	typedef CCustomJXClientSocket Inherited;
	CLocalCrossClient();
	virtual ~CLocalCrossClient();
		
	/*
	* Comments: 回收空闲的CDataPacket类
	* Param CDataPacket * pPacket:
	* @Return void:
	* @Remark: CommonClient收到来自CommonServer的数据包之后，本地分配数据包对象将内容缓存起来，然后通过逻辑线程处理。
			   逻辑线程处理完数据后，调用此接口释放数据包对象。
	*/
	void FreeBackUserDataPacket(CDataPacket* pPacket);
	//发送数据到跨服线程
	VOID SendRawServerData(const jxSrvDef::INTERSRVCMD nCmd,  LPCVOID pData, SIZE_T size);
 
	//重置服务器
	void OnStopServer();
	bool OnRunServer(); 
	void OnSetSrvHostPort(std::string SrvHost, int64_t Port); 
	std::string OnGetSrvHost(){return m_SrvHost;};
	int64_t OnGetSrvPort(){return m_Port;};
	
	void OnSetCsIndex(int CurIndex){m_CurIndex = CurIndex;}; 
	int OnGetCsIndex(){ return m_CurIndex;}; 

	//给客户端使用的信息
	void OnSetCsGateAddr(std::string SrvHost){ m_CsGateAddr = SrvHost;}; 
	void OnSetCsGatePort(int Port){m_nCsGatePort = Port; }; 
	std::string OnGetCsGateAddr(){ return m_CsGateAddr;}; 
	int OnGetCsGatePort(){ return m_nCsGatePort; };  
 
	void SetClientName(std::string name){ CCustomClientSocket::SetClientName(name.c_str());};
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
		if(m_isRun)
		{ 
			CCustomJXClientSocket::OnRountine();
		}
	}
	void OnRecvMessage(int nMsgId, CDataPacketReader &reader);

	/*
	* Comments: 分配数据包，用于存储接收到的数据
	* @Return CDataPacket*:
	* @Remark: 接收线程接收到数据并非立即处理，而是缓存起来，交给逻辑线程统一处理
	*/
	CDataPacket* AllocSendPacket();

private:
	CQueueList<CDataPacket*>			m_vFreeList;	//用来存放空闲的数据包
	CCSLock								m_vFreeListLock;

	std::string m_SrvHost;
	int64_t m_Port;
	bool m_isRun;
	int m_CurIndex;
	
	//给客户端的数据信息
	std::string	m_CsGateAddr;	 
	int	m_nCsGatePort; 
};

#endif//LOCAL_CENTER_CLIENT_H_
