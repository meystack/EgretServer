#ifndef _SS_MANAGER_H_
#define _SS_MANAGER_H_

class CSSGateManager;
class LogSender;

class CSSManager
{
private:
	CHAR			m_sServerName[128];	//服务器名称
	
	INT_PTR         m_nSpGuid;          //运营商的整形的ID

	char			m_sSPID[128];		// 运营商ID

	char            m_sPhpKey[128];  //php用来计算秘钥的key
	bool            m_bIsPhpSignCheck; //是否检查php的秘钥
	bool            m_bOnlyWhitePass;   //只要白名单的才能登陆

	int             m_phpTime;         //php时间过期读多久算失效


	CHAR			m_sSQLHost[128];	//数据库地址
	int				m_nSQLPort;			//数据库端口
	CHAR			m_sSQLDBName[128];	//数据库名称
	CHAR			m_sSQLUser[128];	//数据库用户名
	CHAR			m_sSQLPassword[128];//数据库用户密码
	TICKCOUNT		m_dwQueryCharTimeOut;//会话查询角色的状态超时
	TICKCOUNT		m_dwEntryGameTimeOut;//会话进入游戏的状态超时
	CSSGateManager	*m_pGateManager;	//网关管理器
	LogSender		*m_pLogClient;		//日志客户端
	CSessionServer	*m_pSessionServer;	//会话服务器
	CAMClient		*m_pAMClient;		// 金融管理客户端

	CSessionCenter	*m_pSessionCenter;	//会话中心客户端

	CLockList<PCHARGATEROUTE>	m_CharGateRouteList;//角色网关路由表
	CCSLock			m_CharGateRouteListLock;		//角色网关路由表锁
	CLockList<PADMINLOGINRECORD>m_AdministLoginList;//管理员登录列表
	CCSLock			m_AdministLoginListLock;		//角色网关路由表锁
	CLockList<LOGICSERVERINFO>	m_LogicServerInfoList;		// 逻辑服务器信息列表
	CommServerClientList		m_CommServerClientList;	// 公共服务器连接的逻辑服务器列表
	CCSLock						m_LogicServerInfoLIstLock;	// 逻辑服务器信息列表锁
	bool            m_isFcmOpen;                    //防沉迷是否开启，默认是false表示防沉迷关闭

	CBaseList<unsigned long long>	m_gmIps;                //GM能够登陆的IP
	CBaseList<unsigned long long>	m_createAccountIps;     //能够创建账户的IP列表
	CBaseList<unsigned long long>	m_blackIps;     //禁止的IP列表
	CBaseList<unsigned long long>	m_whiteIps;     //能进去的IP列表

	
	int                    m_nLoginIpLimit;        //获取1个ip最多登陆多少个账户

	bool                   m_bCardmd5;              //卡是否用md5自动生成

	bool                   m_bAutoAccount;          //自动创建账户

	unsigned char          m_spidMd5[128];          //spid的md5的值

	bool                   m_bUtf8 ;                //是否是utf8编码

	int                    m_logIpCount;            //单ip超过了多少个就发消息到日志服去记录下
	
	int                    m_forbidIpInterval;      //封号的IP的间隔

	int                    m_kickCount ;            //单ip登陆间隔小于这个则踢

	bool                   m_gmIpLogin;             //GM只能在特定的ip登陆

	bool                   m_autoSealIp;            //是否自动封IP


	bool                   m_sealAccountSealIp;    //是否封账户的登陆了直接封IP
	     
	bool                   m_bCheckServerIndex;		//是否密码的是否检测serverid，手机这边因为拿不到serverid，所以验证的地方要改下

private:
	VOID ClearRouteList();
	VOID ClearAdministLoginList();

public:
	CSSManager();
	~CSSManager();

	inline LPCSTR getServerName(){ return m_sServerName; };
	inline CSSGateManager* getGateManager(){ return m_pGateManager; };
	inline LogSender* getLogClient(){ return m_pLogClient; };
	inline CSessionServer* getSessionServer(){ return m_pSessionServer; };
	inline CAMClient*	getAMClient() { return m_pAMClient; }
	inline CSessionCenter* getSessionCenter() {return m_pSessionCenter;}
	inline TICKCOUNT getSessionQueryCharTimeOut(){ return m_dwQueryCharTimeOut; }
	inline TICKCOUNT getSessionEntryGameTimeOut(){ return m_dwEntryGameTimeOut; }
	
	inline bool    isFcmOpen(){return m_isFcmOpen;}  //防沉迷是否开启
	inline void    SetFcmOpen(bool flag){m_isFcmOpen =flag;} //设置防沉迷是否开启
	
	inline int GetPhpTime() {return m_phpTime;}  //获取php时间
	inline void SetPhpTime(int nTime) {m_phpTime= nTime;} //设置php时间

	//获取是否需要检测服务器index
	inline bool GetCheckServerIndex() { return m_bCheckServerIndex ; }

	//获取是否需要检测服务器index
	inline void SetCheckServerIndex(bool bFlag) { m_bCheckServerIndex =bFlag; }

	//是否使用utf8编码
	inline bool IsUtf8(){return m_bUtf8;}

	//是否检查php传入的参数
	inline bool isCheckPhpSign()
	{
		return m_bIsPhpSignCheck;
	}

	//设置是否开启php验证
	inline void SetCheckPhpSign(bool flag)
	{
		m_bIsPhpSignCheck =flag;
	}

	//读取key
	inline void SetPhpKey(char *pStr)
	{
		_tcscpy(m_sPhpKey,pStr);
	}

	//获取key
	inline char *GetPhpKey()
	{
		return m_sPhpKey;
	}

	//获取卡是否开启md5
	inline bool GetCardMd5()
	{
		return m_bCardmd5;
	}

	//是否是自动账户
	inline bool IsAutoAccount()
	{
		return m_bAutoAccount;
	}

	//是否是自动封IP的
	inline bool IsAutoSealIp()
	{
		return m_autoSealIp;
	}
	
	//设置是否自动封IP
	inline void SetAutoSealIp(bool flag)
	{
		m_autoSealIp = flag;
	}
	
	//设置自动账户功能
	inline void SetAutoAccount(bool flag)
	{
		m_bAutoAccount =flag;
	}


	//设置卡是否开启md5
	inline void SetCardMd5(bool bFlag)
	{
		m_bCardmd5 =bFlag;
	}

	//返回spid的md5的值
	inline unsigned char * GetSpidMd5()
	{
		return m_spidMd5; 
	}

	//单ip登陆超过多少个就记录日志
	inline int GetIpLogCount()
	{
		return m_logIpCount;
	}

	
	//设置单ip的登陆个数
	inline void SetIpLogCount(int nCount)
	{
		m_logIpCount = nCount;
	}
	
	//设置gm只能在特定的ip登陆 
	inline void SetGmIpLogin(bool flag)
	{
		m_gmIpLogin = flag;
	}

	//获取GM是否只能在特定的IP登陆
	inline bool GetGmIpLogin()
	{
		return m_gmIpLogin;
	}

	//设置封账户直接封IP
	inline void SetSealAccountSealIp(bool flag)
	{
		m_sealAccountSealIp = flag;
	}

	//返回封停账户直接封IP
	inline bool GetSealAccountSealIp()
	{
		return m_sealAccountSealIp;
	}

	//GM登陆的IP列表
	CBaseList<unsigned long long>& GetGmIpList(){return m_gmIps;}

	//能够创建账户的ip列表
	CBaseList<unsigned long long>& GetCreateAcountIpList(){return m_createAccountIps;}

	//黑名ip限制
	CBaseList<unsigned long long>& GetBlackIpList(){return m_blackIps;}

	//获取白名ip限制
	CBaseList<unsigned long long>& GetWhiteIpList(){return m_whiteIps;}

	//获取1个ip能够登陆多少个用户
	inline int  GetIpLoginCount(){return m_nLoginIpLimit;}

	//设置1个ip能登陆多少个
	inline void SetIpLoginCount(int nCount) {m_nLoginIpLimit =nCount;}
	
	//设置是否只要白名单的才能登陆
	inline void SetWhitePass(bool bFlag){ m_bOnlyWhitePass = bFlag ;}

	//获取是否只要白名单才能登陆
	inline bool GetWhitePass() {return m_bOnlyWhitePass;}


	//设置禁止的间隔
	inline void SetForbidInterval(int nInterval)
	{
		m_forbidIpInterval = nInterval;
	}
	
	//返回禁止的间隔
	inline int GetForbidInterval()
	{
		return m_forbidIpInterval;
	}



	//设置禁止的间隔
	inline void SetKickCount(int nInterval)
	{
		m_kickCount = nInterval;
	}

	//返回禁止的间隔
	inline int GetKickCount()
	{
		return m_kickCount;
	}


	//配置数据库连接对象
	VOID SetupSQLConnection(CSQLConenction *lpConnection);

	//依据会话ID关闭连接到网关中的用户
	/*
	inline VOID PostCloseGateUserBySessionId(const int nSessionId)
	{
		m_pGateManager->PostCloseGateUserByGlobalSessionId(nSessionId);
	}
	*/

	//会话服务器回应网关用户确认会话是否在线的结果

	/*
	inline VOID PostGateUserConfimSessionResult(const int nSessionId, const BOOL boOnline)
	{
		m_pGateManager->PostInternalMessage(GTIM_CONFIM_SESSION_RESULT, nSessionId, boOnline, 0);
	}
	*/

	//网关向SessionServer投递改变会话状态的消息
	inline VOID PostChangeSessionState(const int nSessionId, const jxSrvDef::GSSTATE nState)
	{
		return m_pSessionServer->PostInternalMessage(SSIM_CHANGE_SESSION_STATE, nSessionId, nState, 0);
	}
	//当网关用户关闭的时候，向会话模块投递用户关闭的消息，以便决定是否应该删除会话
	inline VOID PostSessionServerUserClosed(const INT_PTR nSessionId)
	{
		return m_pSessionServer->PostInternalMessage(SSIM_GATE_USER_CLOSED, nSessionId, 0, 0);
	}

	//设置服务器名称
	VOID SetServerName(LPCSTR sSrvName);
	// 设置运营商ID
	VOID SetSPID(LPCTSTR sName);

	//设置运营商的唯一的编号
	inline void SetSpGUID(INT_PTR nguid){m_nSpGuid= nguid;}
	
	//获取运营商的唯一编号
	inline INT_PTR GetSPGUID(){return m_nSpGuid;}
	

	// 获取运营商ID
	inline char* GetSPID()  { return m_sSPID;}

	//设置网关服务器的监听地址和端口
	VOID SetGateServiceAddress(LPCTSTR sHost, const int nPort);
	//设置会话服务器地址和端口
	VOID SetSessionServiceAddress(LPCTSTR sHost, const int nPort);
	// 设置AMServer服务器地址和端口
	VOID SetAMServerAddr(LPCTSTR sHost, const int nPort);
	//设置日志服务器地址和端口
	VOID SetLogServerAddress(LPCTSTR sHost, const int nPort);
	//设置数据库连接配置
	VOID SetSQLConfig(LPCSTR sHost, const int nPort, LPCSTR sDBName, LPCSTR sDBUser, LPCSTR sUserPassword,bool bUtf8);
	//设置会话查询角色的超时时间
	inline VOID SetSessionQueryCharTimeOut(TICKCOUNT dwTimeOutMilSec){ m_dwQueryCharTimeOut = dwTimeOutMilSec; };
	//设置会话进入游戏的超时时间
	inline VOID SetSessionEntryGameTimeOut(TICKCOUNT dwTimeOutMilSec){ m_dwEntryGameTimeOut = dwTimeOutMilSec; };
	//加载角色网关路由表，函数返回加载的路由信息数量
	INT_PTR LoadCharGateRouteTable();
	//加载管理员登录表，函数返回加载的记录数量
	INT_PTR LoadAdministLoginTable();
	//为网关用户选择角色网关路由地址，选择路由地址成功则返回TRUE并将地址保存在lpGateHost中（会添加终止字符），将端口保存在lpGatePort中
	BOOL SelectCharGateRoute(const INT_PTR nServerIndex, LPSTR lpGateHost, const SIZE_T dwHostLen, PINT lpGatePort);
	//判断用户是否属于管理员登录列表中的用户
	BOOL IsAdministLogin(LPCSTR sAccount, LPCSTR sIPAddr);

	/*
	* Comments: 加载逻辑服务器列表信息
	* @Return INT_PTR: 返回已经加载的逻辑服务器信息数量
	* @Remark: 读取数据库serverinfo表
	*/
	INT_PTR LoadLogicServerInfoList();

	/*
	* Comments: 获取指定逻辑服务器配置信息
	* Param const int nServerId: 要查询的逻辑服务器ID
	* Param LOGICSERVERINFO &info: 返回查询的逻辑服务器信息。这里返回的结果是数据拷贝过的，线程安全
	* @Return bool: 如果找到对一个服务器的信息返回true；否则返回false
	* @Remark:
	*/
	bool GetLogicServerInfo(const int nServerId, LOGICSERVERINFO &info);

	/*
	* Comments: 获取连接到指定公共逻辑服务器的普通逻辑服务器ID列表
	* Param const int nCommServerId: 公共服务器ID
	* Param CBaseList<int> & clientList: 返回连接到nCommServerId公共服务器的普通逻辑服务器列表
	* @Return void:
	* @Remark:
	*/
	void GetClientLogicServerList(const int nCommServerId, CBaseList<int> &clientList);

	BOOL Startup();
	VOID Shutdown();
	VOID UpdateAMC();

	VOID UpdateTask();


};

#endif

