#ifndef _CUSTOM_GATE_MANAGER_H_
#define _CUSTOM_GATE_MANAGER_H_

/**************************************************************

			  《剑啸江湖》服务器网关通用处理类

		该服务器网关模型采用网关连接对象预初始化的方式来处理网关对象
	实现连接对象在运行期间不会删除从而减少可能出现野指针的情况，同时
	对一个链接对象的复用也提高了效率，减少内存分配频率。一个网关连接
	对象CCustomServerGate类的指针可以长期保存，直到网关管理器被销毁
	后指针才无效，期间网关连接断开也不会使网关对象指针失效。

**************************************************************/

class CCustomServerGate;

using namespace wylib::container;

class CCustomGateManager
	: public CCustomServerSocket
{
public:
	typedef CCustomServerSocket Inherited;
	static const int MaxGateCount = 10;//定义最大网关数量

	enum tagGateCode
	{
		SGIM_QUERY_GSID_USER_EXISTS =301, //是否存在
		SGIM_RECV_LOGIN_DATA = 302,		  //收到登陆的数据,数据包比较大的时候使用buff数据
		SGIM_RECV_LOGIN_CMD = 303,		  //收到了消息包形式 
		SGIM_POST_KICK_USER =304,         //关闭用户
	};
	
public:
	CCustomGateManager();
	~CCustomGateManager();

	/* ★初始化网关队列 */
	virtual VOID Initialize() = 0;
	/* ★销毁网关队列 */
	virtual VOID Uninitialize() = 0;

	/* 启动网关服务器 */
	BOOL Startup();
	/* 停止网关服务器 */
	VOID Stop();
	
	/* 投递关闭网关用户的消息
	 * 依据套接字句柄以及用户在服务器的索引关闭用户，效率很快 
	 * 注意：关闭用户的过程是异步的，是出于多线程之间免锁数据安全考虑。
	 * 网关用户不会再函数执行期间内被关闭，函数只是向网关投递一个关闭用户的消息，
	 * 关闭用户的具体实现依赖于网关对象对内部消息的处理。
	 */
	VOID PostCloseGateUser(SOCKET nUserSocket);
	/* 投递按全局会话ID关闭网关用户
	 * 依据全局会话ID关闭用户，效率较慢，需要遍历查找与nSessionID对应的用户 
	 * 注意：关闭用户的过程是异步的，是出于多线程之间免锁数据安全考虑。
	 * 网关用户不会再函数执行期间内被关闭，函数只是向网关投递一个关闭用户的消息，
	 * 关闭用户的具体实现依赖于网关对象对内部消息的处理。
	 */
	VOID PostCloseGateUserByGlobalSessionId(int nSessionId);
	/* 投递检查网关中是否存在指定全局会话ID的用户
	 * 该操作是异步的，向网关管理器自身发送一个检查所有网关中是否存在指定会话ID的用户
	 * 当网关处管理器理内部消息并检查到对应会话ID的用户存在的时候，会调用网关管理器自身的的OnResultGateUserExists函数
	 */

	//根据账号的ID把一个玩家踢下线，和关闭在跨服的处理有点差别
	VOID PostKickGateUserByGlobalUser(int nSessionId);
	
	inline VOID PostQueryGateUserExistsBySessionId(INT_PTR nSessionId)
	{
		PostInternalMessage(SGIM_QUERY_GSID_USER_EXISTS, nSessionId, 0, 0);
	}
	/* 通过网关编号获取网关对象 */
	inline CCustomServerGate* getGate(INT_PTR nIndex)
	{
		if ( nIndex >= 0 && nIndex < ArrayCount(m_GateList) )
			return m_GateList[nIndex];
		return NULL;
	}
protected:
	/** 以下保护函数为子类可能必要覆盖的函数集**/
	/* ★返回异步查询网关中是否存在指定会话ID的用户的结果 */
	virtual VOID OnResultGateUserExists(int nSessionId, const BOOL boOnline) = 0;

protected:
	/** 以下保护函数为覆盖父类的相关处理函数 **/
	/* 当客户端连接后会调用此函数以便创建一个连接对象，子类应当覆盖此函数 */
	CCustomServerClientSocket* CreateClientSocket(SOCKET nSocket, PSOCKADDR_IN pAddrIn);
	/* 销毁一个已经建立的客户端连接对象，默认的操作是进行delete销毁 */
	VOID DestroyClientSocket(CCustomServerClientSocket *pClientSocket);
protected:
	//覆盖处理内部消息的函数
	VOID DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4);

protected:
	CCustomServerGate*		m_GateList[MaxGateCount];	//网关列表


	
};


#endif

