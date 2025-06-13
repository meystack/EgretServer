#ifndef DBCENTER_CLIENT_H_
#define DBCENTER_CLIENT_H_

/*
	连接DBCenter的Client封装类。
*/

class CDBServer;
class CDBGateManager;
class CDBDataServer;

class CDBCenterClient : public CCustomJXClientSocket,
						public ILogicDBRequestHost
{
public:
	typedef CCustomJXClientSocket Inherited;

public:
	CDBCenterClient(CDBServer *pDBEngine);
	virtual ~CDBCenterClient();
		
	void Init();
	void Stop();

	/*
	* Comments: 投递更新连接的逻辑服务器列表消息
	* Param const CBaseList<int> & serverIdList:
	* @Return void:
	* @Remark:
	*/
	void PostUpdateLogicClientList(const CBaseList<int> &serverIdList);

	/*
	* Comments: 更新本DBServer连接的逻辑客户端列表
	* Param INT_PTR nCount: 连接的逻辑服务器客户端数量
	* Param int *idList: 连接的逻辑服务器ID列表
	* @Return void:
	* @Remark:
	*/
	void UpdateLogicClientList(INT_PTR nCount, int *idList);

	/*
	* Comments: 回收空闲的CDataPacket类
	* Param CDataPacket * pPacket:
	* @Return void:
	* @Remark: CommonClient收到来自CommonServer的数据包之后，本地分配数据包对象将内容缓存起来，然后通过逻辑线程处理。
			   逻辑线程处理完数据后，调用此接口释放数据包对象。
	*/
	void FreeBackUserDataPacket(CDataPacket* pPacket);

	//////////////////////////////////////////////////////////////////////////
	// ILogicDBRequestHost	
	virtual CDataPacket& AllocDataPacket(const jxSrvDef::INTERSRVCMD nCmd);
	virtual void FlushDataPacket(CDataPacket &packet);	
	virtual int CatchCreateGuild(int nActor,
								 LPCSTR sGuildName,	
								 LPCSTR sFoundName,
								 int nIndex,
								 Uint64	hActor,
								 BYTE nZY,
								 INT_PTR nIcon)
	{
		return 0;
	}
	//追踪内存
	void Trace()
	{
		OutputMsg(rmNormal,"---------------CDBCenterClient Trace...------");
		m_sDataHandler.Trace();
		OutputMsg(rmNormal,"---------------CDBCenterClient Trace end-----");
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
	* Comments: 处理DBCenter转发的网关请求
	* Param CDataPacketReader & inPacket:
	* @Return void:
	* @Remark: 来自DBCenter转发的网关请求包括选择角色、创建角色、删除角色等等。是用户登录游戏和进入游戏之间的
			   DB请求。
	*/
	void HandleDBGateRequest(CDataPacketReader &inPacket);
	
	/*
	* Comments: 接收查询角色列表请求
	* Param CDataPacketReader & inPacket:
	* @Return void:
	* @Remark:
	*/
	void OnRecvQueryActorList(CDataPacketReader &inPacket);

	/*
	* Comments: 接收进入游戏请求
	* Param CDataPacketReader & inPacket:
	* @Return void:
	* @Remark:
	*/
	void OnRecvEnterGame(CDataPacketReader &inPacket);


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
	CDBServer							*m_pDBEngine;
	CDBGateManager						*m_pGateMgr;		
	CGateDBReqestHandler				m_gateDBReqHandler;
	CDBDataClientHandler				m_sDataHandler;			// DB请求处理类
	CGateDBReqestHandler                m_loginHandler;

	//CLogicDBReqestHandler				m_logicDBReqHandler;
	CSQLConenction						*m_pSQLConn;
	CQueueList<CDataPacket*>			m_vFreeList;
	CCSLock								m_vFreeListLock;
};

#endif
