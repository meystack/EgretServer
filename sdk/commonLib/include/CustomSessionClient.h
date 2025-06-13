#ifndef _DB_SS_CLIENT_H_
#define _DB_SS_CLIENT_H_

using namespace wylib::sync::lock;


/************************************************************
 *
 * 数据引擎连接到会话服务器的客户端类
 *
 ***********************************************************/

class CCustomSessionClient : 
	public CCustomJXClientSocket
{
public:
	typedef CCustomJXClientSocket Inherited;
	typedef VOID (CCustomSessionClient::*OnHandleSockPacket)(CDataPacketReader &packet);
	
public:
	CCustomSessionClient();
	~CCustomSessionClient();

	/* 获取会话基本数据
	 * 如果找到会话，则将数据填充到pSessionData指向的内存中并返回true，否则返回false
	*/
	bool GetSessionData(const int nSessionId, UINT64 lKey,OUT jxSrvDef::PGLOBALSESSIONOPENDATA pSessionData);
	/* 投递关闭全局会话的消息
	 * 会话不会被立刻关闭，对此功能的实现首先是判断会话是否存在，如果会话存在则向会话服务器发送关闭会话的消息。
	 * 当收到会话服务器发回的关闭会话的消息后才会真正的关闭本地会话列表的一个会话
	*/
	inline void PostCloseSession(unsigned int nSessionId,Uint64 lKey)
	{
		OutputMsg(rmTip,"SessionClient,post close session=%u",nSessionId);
		PostInternalMessage(SSIM_CLOSE_SESSION, nSessionId, lKey, 0);
	}

	/* 投递改变会话状态的消息
	 * 如果会话存在，会话状态会立刻被改变并向会话服务器发送改变会话状态的消息
	*/
	inline void PostChangeSessionState(int nSessionId, jxSrvDef::GSSTATE eState)
	{
		PostInternalMessage(SSIM_CHG_SESSION_STATE, nSessionId, eState, 0);
	}
	/* 投递查询会话是否在线结果的消息
	 * 会向会话服务器回应查询会话是否在线的结果
	 * 参数boOnline的值为TRUE表示与会话相关的用户已经连接到本服务器，否则表示不在线
	*/
	inline void PostQuerySessionExistsResult(int nSessionId, BOOL boOnline)
	{
		PostInternalMessage(SSIM_RESULT_SESSION_EXISTS, nSessionId, boOnline, 0);
	}
protected:
	/*** 下列函数为子类可能有必要覆盖的函数集 ***/
	/* ★创建一个会话对象，默认的操作是使用m_Allocator申请一份会话对象内存 */
	virtual CCustomGlobalSession* CreateSession();
	/* ★销毁一个会话对象，默认的操作是将会话对象释放回m_Allocator中 */
	virtual VOID DestroySession(CCustomGlobalSession* pSession);
	/* ★当会话服务器告知有新会话打开的时,当创建会话对象并填充基本数据后调用
	 *pSession			会话对象
	 *boIsNewSession	是否是新的会话（一个会话可能已经存在与本地会话列表中，但会话实际是不在线的，此情况下用此参数区分是否是完全新打开的会话）
	*/
	virtual VOID OnOpenSession(CCustomGlobalSession* pSession, BOOL boIsNewSession)=0;
	/* ★当一个会话被关闭时调用
	 *nSessionID	会话ID，当调用此函数时会话对象已经被删除，因此不能提供会话对象参数
	*/
	virtual VOID OnCloseSession(CCustomGlobalSession *pSession)=0;
	/* ★当被动（由会话服务器广播而进行的）更新会话状态的时候调用此函数
	 *pSession	会话对象
	 *oldState	会话更新前的状态
	 *newState	会话更新后的状态
	*/
	virtual VOID OnUpdateSessionState(CCustomGlobalSession* pSession, jxSrvDef::GSSTATE oldState, jxSrvDef::GSSTATE newState)=0;
	/* ★查询全局会话关联的用户是否在线
	 *nSessionId	全局会话ID
	 * 此函数内不应当做阻塞式的操作，例如对其他对象加锁以求立刻查询会话ID关联的用户是否存在，
	 * 必须使用内部消息PostInternalMessage的形式向其他线程的对象发送消息，并在该线程中处理消息查询会话关联的用户是否存在，
	 * 最后，使用PostQuerySessionExistsResult返回查询用户是否在线的结果
	*/
	virtual VOID OnQuerySessionExists(INT_PTR nSessionId)=0;

	//连接好了

	virtual VOID OnConnected();
protected:
	//覆盖通信数据包分派函数
	VOID OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket);
	//覆盖父类分派内部消息的函数
	VOID DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4);
	//覆盖当连接处理线程停止前调用的函数
	VOID OnWorkThreadStop();
protected:
	//获取会话指针，如果pIndex参数不不为空，则会在找到会话数据的时候将会话数据在会话列表中的索引保存在pIndex中
	CCustomGlobalSession* GetSessionPtr(const INT_PTR nSessionID, PINT_PTR pIndex,UINT64 lKey);
	
	/* 处理单个通信数据包 */
	VOID DispatchRecvPacket(CDataPacketReader &inPacket);
	/* 发送心跳包消息 */
	//VOID SendKeepAlive();

private:
	VOID CatchDefaultPacket(CDataPacketReader &inPacket);
	VOID CatchOpenSession(CDataPacketReader &inPacket);
	VOID CatchCloseSession(CDataPacketReader &inPacket);
	VOID CatchUpdateSession(CDataPacketReader &inPacket);
	VOID CatchQuerySessionExists(CDataPacketReader &inPacket);
	//清空所有会话数据
	VOID ClearSessionList();

protected:
	CBaseList<CCustomGlobalSession*>	m_SessionList;	//全局会话列表
	CCSLock								m_SessionLock;	//会话列表锁
private:
	static const OnHandleSockPacket		SSSockPacketHandlers[];	//结构化网络数据包处理函数列表
	/*** 定义类内部消息处理的消息号  ***/
	//关闭会话 uParam1=全局会话ID
	static const UINT SSIM_CLOSE_SESSION		= 101;
	//改变会话状态 uParam1=全局会话ID，uParam2=会话状态，值为GSSTATE的枚举值
	static const UINT SSIM_CHG_SESSION_STATE	= 102;
	//返回查询会话是否在线的结果 uParam1=全局会话ID，uParam2=在线?1:0
	static const UINT SSIM_RESULT_SESSION_EXISTS= 103;
};

#endif

