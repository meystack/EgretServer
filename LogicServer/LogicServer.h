#pragma once

class CLogicEngine;
class CLogicGateManager;
class CLogicSSClient;
class CDataClient;
class CFriendClient;
class CLogicServerConfig;
/***************************************************************/
/*
/*                      逻辑服务器中心
/*
/*     实现逻辑服务器中各个组件模块之间的管理以及模块之间功能的衔接
/*
/***************************************************************/

class CLogicServer
{
public:
	
	CLogicServer();
	~CLogicServer();

	//启动逻辑服务器
	BOOL StartServer();
	//停止逻辑服务器
	VOID StopServer(bool bSaveFile = false);

	//获取服务器名称
	inline LPCSTR getServerName(){ return m_sServerName; }


	//设置服务器的名字
	VOID SetServerName(LPCSTR sName);


	//获取文件存储的路径
	inline LPCSTR getStrFilePath(){ return m_strFilePath; }

	//设置服务器的名字
	inline VOID SetStrFilePath(LPCSTR sName){ _asncpyt(m_strFilePath, sName);}	


	//获取逻辑服的版本号
	static char *GetLogicVersion();
	
	/*
	* Comments:设置公共服务器IP地址和端口
	* Param LPCSTR sLoginSrvIp:登陆服务器的ip
	* Param LPCSTR sDbSrvIp:公共数据服务器的ip
	* Param int nPort:公共数据服务器的端口
	* Param int nMinPort:逻辑服务器的端口小
	* Param int nMaxPort:逻辑服务器的端口大
	* @Return VOID:
	*/
	VOID SetCommonServerAddr(LPCSTR sLoginSrvIp,LPCSTR sDbSrvIp, int nPort,int nMinPort,int nMaxPort);

	inline LPCSTR GetCommonServerAddr() {return m_szCommonLoginSrvIP;}
	/*
	/* Comments: 设置服务器的ID
	/* int nServerIndex: 服务器的ID
	/* Return VOID: 
	*/
	inline VOID SetServerIndex(int nServerIndex){m_nServerIndex = nServerIndex;}

	/*
	* Comments: 获取公共服务器ID
	* @Return int:
	* @Remark:
	*/
	inline int GetCommonServerId() const { return m_nCommonServerId; }

	//获取登陆到跨服的最小端口
	inline int GetCommonMinPort() {return m_minCommonPort;}

	//获取登陆到跨服的最大端口
	inline int GetCommonMaxPort() {return m_maxCommonPort;}

	//获取登陆到跨服的随机端口
	int GetCommonServerRandPort();

	/*
	* Comments: 设置公共服务器Id
	* Param const int nServerId:
	* @Return VOID:
	* @Remark:
	*/
	VOID SetCommonServerId(const int nServerId) ;
	

	//设置服务器的开区时间
	inline VOID SetServerOpenTime(CMiniDateTime &time){m_serverOpenTime =time; }

	// 设置夫妻的合区时间
	inline VOID SetServerCombineTime(const CMiniDateTime &time) {m_serverCombineTime = time;}

	/*
	* Comments:获取当前时间是开服零点以来的第几天，0表示还没到开服时间，1表示开服当天，2表示开服的第2天，比如1号1开服，2号返回2
	* @Return int:开服以来的第几天
	*/
	int GetDaysSinceOpenServer();  

	/*
	* Comments:获取当前时间是合服以来的第几天，0表示还没到合服时间，1表示合服当天，2表示合服的第2天，比如1号1合服，2号返回2
	* @Return int:合服以来的第几天
	*/
	int GetDaysSinceCombineServer();  

	//获取服务器的开区时间
	inline CMiniDateTime & GetServerOpenTime(){return m_serverOpenTime;} 

	//获取服务器的合区时间
	inline CMiniDateTime & GetServerCombineTime(){return m_serverCombineTime;} 

	//向网关管理器投递按全局会话ID关闭会话的消息
	inline void postCloseGateUserByGlobalSessionId(int nSessionId)
	{
		m_pGateManager->PostCloseGateUserByGlobalSessionId(nSessionId);
	}
	
	//按sessionid去踢玩家下线
	inline void PostKickGateUserByGlobalSessionId(int nSessionId)
	{
		m_pGateManager->PostKickGateUserByGlobalUser(nSessionId);
	}

	//获取网关管理器
	inline CLogicGateManager * GetGateManager() {return m_pGateManager;}
	
	// 获取公共服务器（针对公共逻辑服务器）
	//inline CCommonServer* GetCommonServer() const { return m_pCommonServer; }


	// 获取连接公告服务器的客户端（针对普通逻辑服务器）
	inline CCommonClient* GetCommonClient() const { return m_pCommonClient; }

	inline CLogicSSClient *  GetSessionClient() {return m_pSSClient; }

	inline LogSender * GetLogClient(  ){return m_pLogClient;}

	VOID SetLogClient( LogSender * pLogClient ); 

	inline CLocalSender *GetLocalClient() {return m_pLocalClient;}

	inline int GetServerIndex(){return m_nServerIndex;} 

	inline void SetCommonServer(bool flag) { m_bCommonServer = flag; }
	inline bool IsCommonServer() const { return m_bCommonServer; }
	CLogicServerConfig* GetLogicServerConfig() {return m_pDataConfig;}
	//跨服相关
	inline void SetCrossServer(bool flag) { m_bCrossServer = flag; }
	inline bool IsCrossServer() const { return m_bCrossServer; }
	inline int GetCrossServerId() const { return m_nCrossServerId; }
	inline int SetCrossServerId(int nSrvId) { m_nCrossServerId = nSrvId; }
	//跨服连接客户端
	inline CLocalCrossClient* GetCrossClient() { return m_pCrossClient; }
	inline CCrossServerManager* GetCrossServerManager() const { return m_pCrossServer; }
	//

	inline CLogicEngine * GetLogicEngine( ){ return  s_pLogicEngine;}

	inline CDataClient * GetDbClient( ){ return m_pDBClient;}

	inline CLogicDataProvider * GetDataProvider(){return m_pDataProvider; }

	inline CBufferAllocator *GetBuffAllocator(){ return &m_dataBuff; } //获取逻辑线程的内存管理器
	//获取版本以及运营商定义对象
	inline CVSPDefinition& GetVSPDefine(){ return m_pVSPDefines; }
	//获取语言包
	inline CLanguageTextProvider& GetTextProvider(){ return m_LanguageText; }

	inline CBackStageSender * GetBackClient() { return m_pBackClient;}

	inline BOOL IsStart() { return m_boStopServ; }

	inline void GMStopServer() 
	{ 
		//关闭网关
		if(m_pGateManager)
		{
			m_pGateManager->CloseAllGates();
		}
		m_boStopServ = FALSE; 
	}//GM强制关闭服务器

	inline void GMCancelStopServer() {m_boStopServ = TRUE; }//取消关机

	void SetStartCommonServer(bool flag);
	inline bool IsStartCommonServer() const { return m_boStartCommonServer; }

	/*
	* Comments:设置运营商的编号
	* Param INT_PTR nSpid:运营商的编号ID
	* @Return void:
	*/
	inline void SetSpid(INT_PTR nSpid) { m_nSpID =nSpid; }

	
	/*
	* Comments:获取运营商的ID
	* @Return INT_PTR:运营商的ID
	*/
	inline INT_PTR GetSpid(){return m_nSpID;}

	//
	void SetGmCreateIndex(int nIndex) {m_CreateIndex = nIndex;}

	int GetGmCreateIndex() {return m_CreateIndex;}

	//后台发送重新加载跨服配置
	void ReloadCrossConfig(bool bReload = true);

	//设置是否开起改名功能
	void SetChangeNameFlag(bool bState) {m_bChangeNameFlag = bState;}

	//获取开启改名功能的状态
	bool GetchangeNameFlag() {return m_bChangeNameFlag;}

	//设置任务是否存日志
	void SetQuestSaveLog(bool bState) {m_bQuestSaveLog = bState;}

	//获得任务是否存日志
	bool GetQuestSveLog() {return m_bQuestSaveLog;}

	//设置消费金币是否存日志
	void SetCoinSaveLog(bool bState) {m_bCoinSaveLog = bState;}

	//获得消费金币是否存日志
	bool GetCoinSaveLog() {return m_bCoinSaveLog;}
	//判断当前开服天数是否满足
	bool CheckOpenServer(int nDay);

	inline AsyncWorker* GetAsyncWorker() { return m_pAsyncWorker; }

	inline void SetAsyncWorker(AsyncWorker* worker) { m_pAsyncWorker = worker; }
	
	/**
	 * @brief 发送资产变动日志
	 * @param nLogId 消费点ID
	 * @param nActorId 角色id
	 * @param sActorName 角色名
	 * @param sAccount 账号
	 * @param nLogType 日志类型（1产出，2消费）
	 * @param nCurrencyType 货币类型（1金币 2绑金 3银两 4元宝 ）
	 * @param nAmount 消费金额
	 * @param nBalance 当前余额
	 * @param nIsShop 是否商城消费（1是 2否）
	 */
	VOID SendCurrencyLocalLog(WORD nLogId,
		unsigned int nActorId,LPCTSTR sActorName,LPCTSTR sAccount,
		WORD nLogType,WORD nCurrencyType,INT64 nAmount,INT64 nBalance,WORD nIsShop, int nSrvId = 0);
	VOID SendCurrencyLocalLog(WORD nLogId,CActor *pActor,WORD nLogType,WORD nCurrencyType,INT64 nAmount,INT64 nBalance,WORD nIsShop,int nSrvId = 0);
	
	/**
	 * @brief 发送商城购买日志
	 * @param nItemId 道具ID
	 * @param nActorId 角色id
	 * @param sActorName 角色名
	 * @param sAccount 账号
	 * @param nShopType 商城类型
	 * @param nItemType 道具类型ID
	 * @param nItemNum 购买道具数
	 * @param nCurrencyType 货币类型（1金币 2绑金 3银两 4元宝 ）
	 * @param nConsumeNum 消耗货币总额
	 */
	VOID SendShopLocalLog(INT nItemId,
		unsigned int nActorId,LPCTSTR sActorName,LPCTSTR sAccount,
		WORD nShopType,WORD nItemType,WORD nItemNum,WORD nCurrencyType,INT64 nConsumeNum,int nSrvId = 0);
	VOID SendShopLocalLog(INT nItemId,CActor *pActor,WORD nShopType,WORD nItemType,WORD nItemNum,WORD nCurrencyType,INT64 nConsumeNum,int nSrvId = 0);
	
	/**
	 * @brief 发送道具获得/消耗日志
	 * @param nLogId 日志ID
	 * @param nActorId 角色id
	 * @param nAccountId 账号ID
	 * @param sActorName 角色名
	 * @param sAccount 账号
	 * @param nItemId 道具ID
	 * @param nItemType 道具类型ID
	 * @param nItemNum 获得/消耗道具数
	 * @param nOperType 操作类型（1获得，2消耗）
	 */
	VOID SendItemLocalLog(WORD nLogId, 
		unsigned int nActorId,		//玩家id
		unsigned int nAccountId,	//角色ID
		LPCTSTR sActorName,LPCTSTR sAccount,
		INT nItemId,WORD nItemType,WORD nItemNum,WORD nOperType, int nSrvId = 0);
	VOID SendItemLocalLog(WORD nLogId,CActor *pActor,INT nItemId,WORD nItemType,WORD nItemNum,WORD nOperType,int nSrvId = 0);
	
	/**
	 * @brief 发送任务接受完成日志
	 * @param nTaskId 任务ID
	 * @param nActorId 角色id
	 * @param sActorName 角色名
	 * @param sAccount 账号
	 * @param nTaskType 任务类型
	 * @param nStatu 状态（1接收，2完成）
	 */
	VOID SendTaskLocalLog(INT nTaskId,
		unsigned int nActorId,LPCTSTR sActorName,LPCTSTR sAccount,
		WORD nTaskType,WORD nStatu,int nSrvId = 0);
	VOID SendTaskLocalLog(INT nTaskId,CActor *pActor,WORD nTaskType,WORD nStatu,int nSrvId = 0);
	
	/**
	 * @brief 发送活动参与日志
	 * @param nAtvId 活动ID
	 * @param nActorId 角色id
	 * @param sActorName 角色名
	 * @param sAccount 账号
	 * @param nAtvType 活动类型
	 * @param nStatu 状态（1接收，2完成）
	 */
	VOID SendActivityLocalLog(INT nAtvId,
		unsigned int nActorId,LPCTSTR sActorName,LPCTSTR sAccount,
		WORD nAtvType,WORD nStatu,int nSrvId = 0);
	VOID SendActivityLocalLog(INT nAtvId,CActor *pActor,WORD nAtvType,WORD nStatu,int nSrvId = 0);
	
private:
	//做全局变量的初始化工作
	VOID InitStaticVar();
	//释放程序中的全局变量
	VOID ReleaseStaticVar();

	//启好服务器做一些工作
	VOID OnStartedServer();
public:
	static CLogicEngine		*s_pLogicEngine;	//逻辑引擎

	//获取utf字符串长度  中文和英文都为1
	static INT_PTR GetStrLenUtf8(LPCTSTR sName);
	int                     m_nCrosssKey ;//跨服
private:

	CVSPDefinition			m_pVSPDefines;		//版本以及运营商常量配置定义
	CLanguageTextProvider	m_LanguageText;		//语言包
	CLogicGateManager		*m_pGateManager;	//网关管理器
	//CCommonServer			*m_pCommonServer;	// 公共逻辑服务器Server端
	CCommonClient			*m_pCommonClient;	// 普通逻辑服务器Client端

	CLogicSSClient			*m_pSSClient;		//会话客户端
	LogSender				*m_pLogClient;		//日志客户端

	CLocalSender			*m_pLocalClient;		//公共日志客户端

	CDataClient				*m_pDBClient;		//数据客户端
	//CFriendClient			*m_pFrdClient;		//好友客户端
	//CMgrServClient			*m_pMgrClient;	//连接管理后台的客户端
	CBackStageSender		*m_pBackClient;		//后台客户端
	AsyncWorker				*m_pAsyncWorker;	//异步工作线程


	CLogicServerConfig		*m_pDataConfig;    //config data io center

	CLogicDataProvider		*m_pDataProvider;  //逻辑数据中心
	
	CHAR					m_sServerName[128];	//游戏服务器名称	
	int						m_nServerIndex;     //server index
	CMiniDateTime           m_serverOpenTime;   //服务器的开始时间
	BOOL					m_boStarted;		//逻辑服务器是否已经启动
	BOOL					m_boStopServ;		//停止服务器的标志
	bool					m_bCommonServer;	// 为true表明是公共逻辑服务器
	int						m_nCommonServerId;	// 公共服务器ID
	
	//公共登陆服务器的配置
	char					m_szCommonLoginSrvIP[64];	// 对于普通逻辑服务器，这里保存连接到公共逻辑服的ip和端口；对于公共服务器，记录其监听地址
	int						m_minCommonPort;		//表示玩家登录进去使用的最小端口
	int						m_maxCommonPort;		//表示玩家登录进去使用的最大端口

	//公共数据服务器的配置
	char					m_szCommonDbSrvIp[64];	    //公共服务器数据服的ip
	int						m_nCommonDbSrvPort;        //公共数据服务器的地址

	
	INT_PTR                 m_nSpID;            //运营商的编号
	bool					m_boStartCommonServer;	//是否连接到跨服服务器
	CMiniDateTime			m_serverCombineTime;	// 服务器合区时间

	int						m_CreateIndex;			//gm设置的创号的入口id

	CHAR					m_strFilePath[128];			//文件的存储路径

	bool					m_bChangeNameFlag;			//是否开启改名功能

	bool					m_bQuestSaveLog;			//任务是否存日志
	bool					m_bCoinSaveLog;				//消费金币是否村日志

public:
	CBufferAllocator        m_dataBuff; //内存管理器

	//跨服
	bool                  	m_bCrossServer;//是否是跨服服务器
	int                    	m_nCrossServerId;//跨服服务器ServerID
	CCrossServerManager     *m_pCrossServer;	//	逻辑服务器Server端
	CLocalCrossClient     	*m_pCrossClient;	//	逻辑服务器Client端

	std::string				m_cGateAddr;		//用不到了
	int                     m_nCSGatePort;		//用不到了 
};

class GameServerEntry
{
public:
	static CLogicServer * g_pLogicServer ;
};

inline  CLogicEngine *GetGlobalLogicEngine(){return CLogicServer::s_pLogicEngine; }
inline  CLogicServer* GetLogicServer( ){return GameServerEntry::g_pLogicServer;} 
inline CEntity * GetEntityFromHandle(const EntityHandle & handle){	
	return GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(handle) ;
}

inline TICKCOUNT GetLogicCurrTickCount() {return CLogicEngine::nCurrTickCount;}

//定义个全局的分配内存的函数
//使用CLogicServer的内存分配器
inline PVOID GAllocBuffer(size_t dwSize)
{
	return GameServerEntry::g_pLogicServer->m_dataBuff.AllocBuffer(dwSize);
}

inline VOID	GFreeBuffer(PVOID ptr)
{
	return GameServerEntry::g_pLogicServer->m_dataBuff.FreeBuffer(ptr);
}
