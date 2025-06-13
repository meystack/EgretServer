#pragma once

class CDBServer;

class CDBSessionClient
	: public CCustomSessionClient
{
public:
	typedef CCustomSessionClient Inherited;

public:
	CDBSessionClient(CDBServer *lpDBServer);
	~CDBSessionClient();

protected:
	/*** 覆盖父类的会话逻辑处理函数 ***/
	/* ★查询本地服务器的类型，以便正确的向会话服务器注册连接 */
	jxSrvDef::SERVERTYPE getLocalServerType();

	// 返回serverindex
	virtual int getLocalServerIndex();

	/* ★查询本地服务器的名称，以便正确的向会话服务器注册连接 */
	LPCSTR getLocalServerName();
	/* ★创建一个会话对象，默认的操作是使用m_Allocator申请一份会话对象内存 */
	CCustomGlobalSession* CreateSession();
	/* ★销毁一个会话对象，默认的操作是将会话对象释放回m_Allocator中 */
	VOID DestroySession(CCustomGlobalSession* pSession);
	/* ★当会话服务器告知有新会话打开的时,当创建会话对象并填充基本数据后调用
	 *pSession			会话对象
	 *boIsNewSession	是否是新的会话（一个会话可能已经存在与本地会话列表中，但会话实际是不在线的，此情况下用此参数区分是否是完全新打开的会话）
	*/
	VOID OnOpenSession(CCustomGlobalSession* pSession, BOOL boIsNewSession);
	/* ★当一个会话被关闭时调用
	 *nSessionID	会话ID，当调用此函数时会话对象已经被删除，因此不能提供会话对象参数
	*/
	VOID OnCloseSession(int nSessionId);

	//实现父类的虚函数
	VOID OnCloseSession(CCustomGlobalSession *pSession){}
	/* ★当被动（由会话服务器广播而进行的）更新会话状态的时候调用此函数
	 *pSession	会话对象
	 *oldState	会话更新前的状态
	 *newState	会话更新后的状态
	*/
	VOID OnUpdateSessionState(CCustomGlobalSession* pSession, jxSrvDef::GSSTATE oldState, jxSrvDef::GSSTATE newState);
	/* ★查询全局会话关联的用户是否在线
	 *nSessionId	全局会话ID
	 * 此函数内不应当做阻塞式的操作，例如对其他对象加锁以求立刻查询会话ID关联的用户是否存在，
	 * 必须使用内部消息PostInternalMessage的形式向其他线程的对象发送消息，并在该线程中处理消息查询会话关联的用户是否存在，
	 * 最后，使用PostQuerySessionExistsResult返回查询用户是否在线的结果
	*/
	VOID OnQuerySessionExists(INT_PTR nSessionId);

private:
	CDBServer			*m_pDBServer;	//所属DBServer
};
