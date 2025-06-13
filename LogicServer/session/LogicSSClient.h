#pragma once

/***************************************************************/
/*
/*                      逻辑服务器会话管理器
/*
/*     实现逻辑服务器与会话服务器的通信以及逻辑服务器中的全局会话管理
/*
/***************************************************************/
class CLogicServer;
class CActor;

class CLogicSSClient :
	public CCustomSessionClient
{
public:
	typedef CCustomSessionClient Inherited;

public:

	CLogicSSClient(CLogicServer *lpLogicServer);
	~CLogicSSClient();

	typedef struct tagAccountPasswd
	{
		ACCOUNT account;  //玩家的账户
		char    passwd[MAX_PASSWD_LENGTH];
	}ACCOUNTPASSWD,*PACCOUNTPASSWD;

	/*
	* Comments: 公共服务器广播数据给普通服务器用
	* Param const LPCVOID pData:
	* Param const SIZE_T nLen:
	* @Return void:
	* @Remark:
	*/
	void BroadMsgToCommonLogicClient(LPCVOID pData, const SIZE_T nLen);

	/*
	* Comments: 发送数据给目标逻辑服务器
	* Param const LPCVOID pData:
	* Param const SIZE_T nLen:
	* @Return void:
	* @Remark:
	*/
	void SendMsgToDestLogicClient(int nCmd, int nDestServerId, LPCVOID pData, const SIZE_T nLen);

	/*
	* Comments: 逻辑服务器广播数据给其他服务器用
	* Param const LPCVOID pData:
	* Param const SIZE_T nLen:
	* @Return void:
	* @Remark:
	*/
	void BroadMsgToLogicClient(int nCmd, LPCVOID pData, const SIZE_T nLen);

	virtual void OnRountine()
	{
		Inherited::OnRountine();
	}

public:
	/*
	* Comments: 投递设置会话ID相关连的游戏角色的
	* Param INT_PTR nSessionId: 会话ID
	* Param CActor * pActor: 游戏角色对象
	* @Return VOID:
	*/
	inline VOID PostSetSessionPlayer(INT_PTR nSessionId, CActor *pActor,UINT64 lKey)
	{
		PostInternalMessage(SCIM_SET_SESSION_PLAYER, nSessionId,(UINT64)pActor,lKey);
	}

	
	/*
	* Comments:向会话服务器发送在线人数
	* @Return VOID:
	*/
	VOID SendOnlinePlayerCount(INT_PTR nCount);

	// 回收空闲的CDataPacket类
	void FreeBackUserDataPacket(CDataPacket* pDataPacket);


	//玩家发数据包过来，准备进入游戏 
	void PostLogin(char *sAccountName,char *sPasswd);

	
	//向会话服务器报告自己的服务器的ID
	VOID SendServerIndex();

	
protected:
	//继承基类的，处理网络消息
	VOID OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket);

	/* 查询本地服务器的名称，以便正确的向会话服务器注册连接 */
	LPCSTR getLocalServerName() ;
	/* 创建一个会话对象，默认的操作是使用m_Allocator申请一份会话对象内存 */
	CCustomGlobalSession* CreateSession();
	/* 销毁一个会话对象，默认的操作是将会话对象释放回m_Allocator中 */
	VOID DestroySession(CCustomGlobalSession* pSession);
	/* 当会话服务器告知有新会话打开的时,当创建会话对象并填充基本数据后调用
	 *pSession			会话对象
	 *boIsNewSession	是否是新的会话（一个会话可能已经存在与本地会话列表中，但会话实际是不在线的，此情况下用此参数区分是否是完全新打开的会话）
	*/
	VOID OnOpenSession(CCustomGlobalSession* pSession, BOOL boIsNewSession);
	/* 当一个会话被关闭时调用
	 *nSessionID	会话ID，当调用此函数时会话对象已经被删除，因此不能提供会话对象参数
	*/
	VOID OnCloseSession(CCustomGlobalSession* pSession);
	/* 当被动（由会话服务器广播而进行的）更新会话状态的时候调用此函数
	 *pSession	会话对象
	 *oldState	会话更新前的状态
	 *newState	会话更新后的状态
	*/
	VOID OnUpdateSessionState(CCustomGlobalSession* pSession, jxSrvDef::GSSTATE oldState, jxSrvDef::GSSTATE newState);
	/* 查询全局会话关联的用户是否在线
	 *nSessionId	全局会话ID
	 * 此函数内不应当做阻塞式的操作，例如对其他对象加锁以求立刻查询会话ID关联的用户是否存在，
	 * 必须使用内部消息PostInternalMessage的形式向其他线程的对象发送消息，并在该线程中处理消息查询会话关联的用户是否存在，
	 * 最后，使用PostQuerySessionExistsResult返回查询用户是否在线的结果
	*/
	VOID OnQuerySessionExists(INT_PTR nSessionId);
	//覆盖工作线程停止时的操作
	VOID OnWorkThreadStop();
		 
	jxSrvDef::SERVERTYPE getLocalServerType()
	{
		return jxSrvDef::GameServer;
	}
	int getLocalServerIndex();

	//连接起来的时候向会话发送本服的index和公共服的index
	virtual VOID OnConnected();



protected:
	//处理内部消息
	VOID DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4);

private:
	//销毁所有全局回话对象所占的内存
	VOID FreeSessionBuffers();
	// 分配一个CDataPacket
	CDataPacket* AllocSendPacket();

private:
	CLogicServer				*m_pLogicServer;	//所属逻辑服务器
	CBaseList<CCustomGlobalSession*>m_FreeSessionList;	//空闲全局会话列表
	CBaseList<void*>			m_SessionMemList;	//全局会话内存块头部指针列表

	CQueueList<CDataPacket*>			m_vFreeList;	//用来存放空闲的数据包
	CCSLock								m_vFreeListLock;

	CQueueList<ACCOUNTPASSWD>          m_accountPasswd; //用户名密码信息，避免频繁的申请

private:
	
	/** 内部消息列表 **/
	static const INT_PTR SCIM_SET_SESSION_PLAYER = 201; //设置会话游戏角色对象，Param1=会话ID，Param2=角色对象


};
