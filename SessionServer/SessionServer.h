#ifndef _SESSION_SERVER_H_
#define _SESSION_SERVER_H_

#include "AMProcto.h"
#include "AMClient_tx.h"


class CSessionClient;
class CSSManager;
class CRankMgr;

using namespace jxSrvDef;

class CSessionServer
	: public CCustomServerSocket
{
public:
	friend class CSessionClient;
	typedef CCustomServerSocket Inherited;
	//定义枚举客户连接的回调函数类型，第一个LPVOID类型的参数是一个穿透性的参数。函数返回0表示继续枚举，否则表示终止枚举
	typedef UINT (__stdcall *EnumConnectionFn) (LPCVOID, CSessionClient*);

private:
	CQueueList<PGAMESESSION>	m_SessionList;		//全局会话列表
	CCSLock						m_SessionListLock;	//全局会话列表锁
	CSSManager*					m_pSSManager;		//所属会话管理器
	TICKCOUNT					m_dwReconnectSQLTick;//下次重新连接数据的时间
	CSQLConenction				m_SQLConnection;	//数据库连接对象
	INT_PTR						m_nDBClientCount;	//数据客户端数量
	INT_PTR						m_nEngineClientCount;//引擎客户端数量
	TICKCOUNT					m_dwCheckOnlineLogTick;//下次检查记录在线人数的时间

	TICKCOUNT                   m_dwSaveRankTick;   //保存rank的tick

	BOOL						m_boOnlineLoged;	//是否已经记录在线人数
	BOOL                        m_boIpLoged;         //是否记录了IP在线

	CRankMgr                    m_rankMgr;           //获取排名管理器

	const static int           SAVE_RANK_INTERVAL= 120*3600*1000;  //2个小时存盘1次吧
	
	struct IPCount
	{
		long long lIp;         //用户的IP
		int       nCount;      //禁止登陆的次数
	};

	CBaseList<IPCount>       m_ipForbidCount;		//IP违规的次数，违规多次以后，就需要封IP,封账户

private:
	//发送记录各个服务器的在线人数
	VOID SendLogOnlineCount();
protected:
	//覆盖创建连接对象的函数
	CCustomServerClientSocket* CreateClientSocket(SOCKET nSocket, PSOCKADDR_IN pAddrIn);
	//覆盖处理客户端连接的函数
	VOID ProcessClients();
	//覆盖的开启服务的函数
	BOOL DoStart();
	//覆盖的停止服务的函数
	VOID DoStop();
	//覆盖内部消息分派函数
	VOID DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4);
	//覆盖例行执行函数
	VOID SingleRun();
private:
	//连接数据库
	BOOL ConnectSQL();

	
	
	
	//关闭会话，操作的依据是会话在列表中的索引，而非会话ID或账号字符串
	BOOL CloseSessionAtListIndex(const INT_PTR nIndex);

private:
	
	

	//向所有DB客户端以及指定ID的引擎客户端发送消息，消息将被立刻追加到客户端发的发送缓冲中。数据包必须是完成的格式（包含包头和包尾）
	INT_PTR SendAllDBAndIndexEngineMsg(const INT_PTR nServerIndex, LPCSTR sMsg, const size_t dwSize);
	
	//广播会话状态变更的消息
	INT_PTR BroadCastUpdateSessionState(PGAMESESSION pSession);
	
	
	/*
	* Comments:广播删除一个会话，如果nServerIndex=-1则所有的广播，否则只向特定的服务器广播
	* Param const UINT64 nSessionId:会话的id
	* Param const int nServerIndex:服务器的编号
	* @Return INT_PTR:
	*/
	INT_PTR BroadCastCloseSession(const UINT64 nSessionId,Uint64 lKey, const int nServerIndex=-1 );
	
	//广播确认会话是否在线
	INT_PTR BroadCastSessionConfim(const UINT64 nSessionId, const INT_PTR nServerIndex);
	
	//广播要删除这个玩家，玩家登陆的时候，如果在线，直接踢出这个角色
	//INT_PTR BroadcastKickUser(const INT_PTR nSessionId,const INT_PTR nServerIndex);
	
	//初始化账户的唯一，为了保证账户的唯一，account的最小值为 spguid << 24 +1 ，这样保证全球所有的账户都是唯一的
	bool InitACCountId();

	//初始化所有的数据
	bool InitRankMsg();

	//保存所有的rank数据
	bool SaveRankMsg();


	//向日志服发送当前ip的登陆情况
	void SendOnlineIpStatus();

	//获取int类型的ip
	long long  GetIntIp(char *sIp); //

public:

	//通过账号字符串获取会话指针，如果lpIndex参数非空，则会向指针指向的内存填充值为会话在列表中的索引
	//此函数非常的慢！
	PGAMESESSION GetSessionPtrByAccount(LPCSTR sAccount, PINT_PTR lpIndex = NULL);

	// 给连接到指定公共服务器的逻辑服务器广播消息
	INT_PTR SendGroupLogicClientMsg(const int nCommServerId, LPVOID data, const SIZE_T nSize);

	//向所有客户端送消息,消息将被立刻追加到客户端发的发送缓冲中。数据包必须是完成的格式（包含包头和包尾）
	//参数eServerType表示服务器类型，InvalidServer表示向所有类型的服务器发送，否则向类型为eServerType的所有服务器发送
	INT_PTR SendAllClientMsg(const SERVERTYPE eServerType, LPCSTR sMsg, const size_t dwSize);
	
	//向指定ID的逻辑户端发送消息，消息将被立刻追加到客户端发的发送缓冲中。数据包必须是完成的格式（包含包头和包尾）
	INT_PTR SendLogicClientMsg(const INT_PTR nServerIndex, LPCSTR sMsg, const size_t dwSize);

	//获取会话指针，如果lpIndex参数非空，则会向指针指向的内存填充值为会话在列表中的索引
	PGAMESESSION GetSessionPtr(const unsigned int nSessionId,PINT_PTR lpIndex = NULL,Uint64 lKey= (Uint64)-1,int nServerIndex =-1);


	CSessionServer(CSSManager *lpSSManager);
	~CSessionServer();

	//向所有服务器投递消息,消息将在逻辑循环中被发送而不是立即发送。数据包必须是完成的格式（包含包头和包尾）
	//参数eServerType表示服务器类型，stInvalid表示向所有类型的服务器发送，否则向类型为eServerType的所有服务器发送
	VOID PostAllClientMsg(const SERVERTYPE eServerType, LPCSTR sMsg);
	
	
	//获取会话数据，如果会话存在则返回TRUE并为pSession填充会话数据内容
	BOOL GetSession(const INT_PTR nSessionId,OUT PGAMESESSION pSession,int nServerIndex =-1);


	//查找某个特定服会话的指针
	PGAMESESSION  GetSpecialServerSession(const INT_PTR nSessionId, const int nServerIndex);

	//踢掉在线的用户
	VOID PostKickCrossActor(const INT_PTR nSessionId, const int nServerIndex);

	//处理剔除跨服用户
	void ProcessKickCrossActor(const unsigned int nSessionId, const int nServerIndex);

	//获取1个ip登陆的个数
	int GetLoginAccountCount(LONGLONG lIp); 


	//获取当前全局会话数量
	inline INT_PTR GetSessionCount(){ return m_SessionList.count(); }
	//获取已经连接的数据客户端数量
	inline INT_PTR GetDBClientCount(){ return m_nDBClientCount; };
	//获取已经连接的逻辑服务器客户端数量，如果nServerIndex参数为0，则统计所有逻辑客户端数量
	INT_PTR GetLogicClientCount(const INT_PTR nServerIndex);
	//判断用户是否可以登录到目的服务器，返回0表示可以登录。如果服务器的会话客户端未连接则返回-1，如果服务器人员已满，则返回-2,
	INT_PTR UserCanLoginToServer(const INT_PTR nServerIndex);
	
	//投递开启新会话的消息
	BOOL PostOpenSession(const INT_PTR nServerIndex,		// 登录的逻辑服务器ID
						 const INT_PTR nRawServerIndex,		// 原始的服务器ID
						 LPCSTR sAccount,					// 登录用户名
						 const unsigned int nSessionId,			// 会话ID（账号ID）
						 const INT_PTR nIPAddr,				// 登录IP地址
						 const INT_PTR dwTodayOnlineSec,	// 今天在线时间
						 INT_PTR nGmLevel,					// GM等级
						 UINT64  lKey ,                     // 在逻辑服的key
						 INT_PTR nGateIndex,                //在网关的编号 ,
						 bool   bIsWhiteLogin,              // 是否是白名单登陆的
						 bool	bCommonMsg=false			//是否中心会话服转发过来的消息					
							 );

	//投递按账号字符串关闭会话的消息，用于管理用
	VOID PostCloseSessionByAccount(const LPCSTR sAccount);
	
	//关闭一个会话
	VOID PostCloseSessionByAccountId(Uint64 nAccountId,UINT64 lKey);



	//遍历所有的连接,lpFn是毁掉函数指针，lpParam是穿透参数会传递给对调函数。
	VOID EnumConnections(EnumConnectionFn lpFn, LPCVOID lpParam);

	CSSManager* GetSSManager() { return m_pSSManager; }

	void PostAMCMsg(const CAMClient::AMOPData &data);

	//转发集市任务的消息给逻辑服
	void PostAMCTaskMsg(CAMClient::tagTASKMsg &data);

	CSQLConenction* GetSql() {return &m_SQLConnection;}

	/*
	* Comments:  往列表里添加1个成员
	* Parameter: CRankItem & rankItem:rank的值
	* Parameter: int nRankItemCount:排行榜最大的数额限制，
	* @Return  void:
	*/
	void PostRankMsg(CRankItem& rankItem,int nRankItemCount=1)
	{
		m_rankMgr.AddRankItem(rankItem,nRankItemCount);
	}


	/*
	* Comments:  获取某个排行榜
	* Parameter: int nKey:排行榜的id,如果是所有的，可以传入-1
	* Parameter: wylib::container::CBaseList<CRankItem> & itemList:元素列表
	* @Return  void:
	*/
	void GetRankMsg(int nKey,wylib::container::CBaseList<CRankItem> & itemList)
	{
		m_rankMgr.GetRankList(nKey,itemList);
	}

	/*
	* Comments: 封掉 ip
	* Parameter: long long lIp:ip的整数
	* @Return  void:
	*/
	void SealIp(char *sIp);

	
	


	/*
	* Comments:  解封ip
	* Parameter: long long lIp:
	* @Return  void:
	*/
	void UnSealIp(char *sIp);

	

	
	/*
	* Comments:  禁止ip并且封掉所有的账户
	* Parameter: long long lIP:IP
	* Parameter: bool bDirectSeal:是不是直接封号
	* @Return  void:
	*/
	void ForbidIpAndSealAll(long long lIP,bool bDirectSeal );


	

	//获取IP禁止的次数
	inline INT_PTR GetIpForbidCount(long long lIp)
	{
		for( INT_PTR i=m_ipForbidCount.count() -1; i>=0 ; i -- )
		{
			if(m_ipForbidCount[i].lIp == lIp)
			{
				return m_ipForbidCount[i].nCount;
			}
		}
		return 0;
	}

	//设置IP禁止的次数
	inline  void SetIpForbidCount(long long lIp,int nCount)
	{
		for( INT_PTR i=m_ipForbidCount.count() -1; i>=0 ; i -- )
		{
			if(m_ipForbidCount[i].lIp == lIp)
			{
				m_ipForbidCount[i].nCount = nCount;
				return ;
			}
		}
		IPCount data;
		data.lIp = lIp;
		data.nCount =nCount;
		m_ipForbidCount.add(data);

	}
	
	//从会话中心服回来的跨平台请求结果
	void OnSendPlatformResultToClient(int nServerIndex,CDataPacketReader &inPacket);
};

#endif

