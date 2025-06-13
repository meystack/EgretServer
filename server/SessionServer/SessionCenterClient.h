#ifndef SESSION_CENTER_CLIENT_H_
#define SESSION_CENTER_CLIENT_H_

/*
	连接DBCenter的Client封装类。
*/



class CSessionCenterClient : public CCustomJXClientSocket,
						public CSessionDataHandle

{
public:
	typedef CCustomJXClientSocket Inherited;

public:
	CSessionCenterClient();
	virtual ~CSessionCenterClient();
		



	/*
	* Comments: 回收空闲的CDataPacket类
	* Param CDataPacket * pPacket:
	* @Return void:
	* @Remark: CommonClient收到来自CommonServer的数据包之后，本地分配数据包对象将内容缓存起来，然后通过逻辑线程处理。
			   逻辑线程处理完数据后，调用此接口释放数据包对象。
	*/
	void FreeBackUserDataPacket(CDataPacket* pPacket);


	
	
	virtual CDataPacket& AllocDataPacket(const jxSrvDef::INTERSRVCMD nCmd)
	{
		return allocProtoPacket(nCmd);
	}

	void FlushDataPacket(CDataPacket &packet)
	{
		return flushProtoPacket(packet);
	}


protected:	
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Virtual Function Of CCustomJXClientSocket
	virtual LPCSTR getLocalServerName();
	virtual jxSrvDef::SERVERTYPE getLocalServerType();	
	//TODO. 临时测试用。DBServer不需要这个，而是需要自己维护DB连接的逻辑服务器ID的列表
	virtual int getLocalServerIndex();
	virtual VOID OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket);
	virtual VOID DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3, UINT64 uParam4);
	
	/*
	* Comments: 处理DBCenter转发的逻辑请求。
	* Param CDataPacketReader & inPacket:
	* @Return void:
	* @Remark:
	*/
	void HandleLogicRequest(CDataPacketReader &inPacket);
			
	/*
	* Comments: 分配数据包，用于存储接收到的数据
	* @Return CDataPacket*:
	* @Remark: 接收线程接收到数据并非立即处理，而是缓存起来，交给逻辑线程统一处理
	*/
	CDataPacket* AllocSendPacket();

private:
	static const int DBC_INTERNAL_UPDATECLIENTLIST = 101;	// 更新连接的逻辑客户端列表内部消息

private:
	

	CSQLConenction						*m_pSQLConn;
	CQueueList<CDataPacket*>			m_vFreeList;
	CCSLock								m_vFreeListLock;
};

#endif