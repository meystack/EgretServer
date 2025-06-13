#ifndef _GATE_H_
#define	_GATE_H_

using namespace wylib::time::tick64;
using namespace wylib::container;

class CDBGateManager;
class CDBServer;

/*******************************************************
	
		网关类
	
********************************************************/
class CDBGate 
	: public CCustomServerGate
{
	friend class CDBGateManager;

public:
	CDBGate();
	virtual ~CDBGate();
	
	/* 
	* Comments: 逻辑服务器返回收到了玩家的数据
	* Param const unsigned int nSessionID: sessionID
	* Param const unsigned int nActorId: 玩家的的actorID
	* @Return void: 
	*/
	void OnLogicRsponseSessionData(const unsigned int nSessionID,const unsigned int nActorId);

	static bool m_bGableIsClose;   //盗梦空间是否关闭
	
	virtual void OnRountine()
	{
		CCustomServerGate::OnRountine();
	}

protected:
	/*** 覆盖父类的逻辑处理函数集 ***/
	CCustomServerGateUser* CreateGateUser();
	VOID DestroyGateUser(CCustomServerGateUser* pUser);
	VOID OnOpenUser(CCustomServerGateUser *pGateUser);
	VOID OnCloseUser(CCustomServerGateUser *pGateUser);
	VOID OnGateClosed();
	VOID OnDispatchUserMsg(CCustomServerGateUser *pUser, char* pBuffer, SIZE_T nBufferSize);
	VOID OnRun();
	virtual VOID DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4);

private:
	//处理用户列表
	VOID ProcessUsers();
	VOID AddAllGateUsersToFreeList();
	//通过全局会话ID查找网关用户，如果找到用户且lpIndex非空则会想*lpIndex填充网关用户的索引
	CDBGateUser* GetGateUserPtrBySessionId(const int nSessionId, PINT_PTR lpIndex = NULL);
	//处理用户选择角色进入游戏的消息
	int UserSelCharEntryGame(CDBGateUser *pGateUser, int nCharId,
		char* sGateHost,SIZE_T nHostLen,PINT_PTR nGatePort);
	//处理名称服务器返回申请角色名称的结果
	BOOL NameServerAllocCharIdResult(const int nSessionId, const INT_PTR nError, const int nCharId);

//处理各个数据包
private:
	//根据帐户ID查询出相关的角色列表,结果会添加到pPacket中
	CDataPacket* QueryActorList(CDBGateUser *pGateUser, char* lpData, SIZE_T nSize);

	//处理创建角色包
	CDataPacket* CreateActor(CDBGateUser *pGateUser, char* lpData,SIZE_T nSize);

	CDataPacket* DeleteActor(CDBGateUser *pGateUser, char* lpData,SIZE_T nSize);

	CDataPacket* EntryGameReq(CDBGateUser *pGateUser, char* lpData,SIZE_T nSize);

	//随机生成名字
	CDataPacket* RandNameReq(CDBGateUser *pGateUser, char* lpData,SIZE_T nSize);
	//查询最少人选择的职业
	CDataPacket* QueryLessJobReq(CDBGateUser *pGateUser, char* lpData,SIZE_T nSize);
	BYTE QueryLessJobReq( BYTE &bJob, int serverindex );

	CDataPacket* QueryZYReq(CDBGateUser *pGateUser, char* lpData,SIZE_T nSize);
	BYTE QueryZYReq( BYTE& bZY ,int serverindex);

	INT_PTR QueryActorCount(INT_PTR nAccountId, INT_PTR nServerIndex);
	WORD QueryZyList(INT_PTR nServerIndex);

	// 当名字服务器返回创建角色结果时，执行本函数
	// 1.数据库中插入新的角色信息 2.返回客户端结果
	VOID ProcessNameSrvResult(CDBGateUser* pGateUser,int nCharId, INT_PTR nError);

	void SendClientLogin(const unsigned int nSessionID,const unsigned int nActorId);
	// 设置网关管理器
	void SetGateManager(CDBGateManager* mgr){ m_pGateMgr = mgr; }
	//inline void InitDBReqHandler() { m_reqHandler.Init(m_pSQLConnection, m_pGateMgr); }

	//// 封装DB请求处理
	///*
	//* Comments: 获取指定账号ID的所有角色列表信息
	//* Param const int nActorId: 玩家账号ID
	//* Param const int nServerId: 角色所在的逻辑服务器ID
	//* Param CDataPacket &: 输出数据包
	//* @Return void:
	//* @Remark: 输出内容包括：错误码（1Byte) + 角色数目（1Byte） + 角色信息列表。如果查询失败，只有错误码。
	//*/
	//void GetActorList(const int nActorId, const int nServerId, CDataPacket &packet);

	//有可能玩家连接了2个连接，或者连接没有关闭，导致逻辑服发过来的时候，发给了另外一个连接
	//所以玩家登陆的时候，根据账户id，把以前的连接需要关闭，避免发错连接，也避免僵尸连接
	VOID DeleteOldActor(CDBGateUser *pGateUser);

public:
	typedef CCustomServerGate Inherited;
	static const INT_PTR MAX_GATE_USER = 8192;	//最大网关用户数
	static const INT_PTR SSM_LOGIC_RESPONSE_SESSION_DATA =40001;

private:
	CDBGateManager*			m_pGateMgr;					// 网关管理器
	CDBGateUser				m_GateUsers[MAX_GATE_USER];	//网关用户数组
	CSQLConenction*			m_pSQLConnection;			//数据库连接对象
	CDBServer*				m_pDBServer;				//所属数据引擎
	CBaseList<CCustomServerGateUser*>	m_FreeUserList;	//空闲网关用户列表
	PERFORMANCEINFO			m_MaxTimeUserMsgPerformance;//处理用户单个消息最高的时间
	CGateDBReqestHandler	m_reqHandler;				// GateDBReqHandler
};

#endif
