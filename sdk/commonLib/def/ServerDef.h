#ifndef _SERVER_DEF_H_
#define _SERVER_DEF_H_

/*****************************************************
*
*	服务端通用定义文件
*
*****************************************************/
typedef unsigned long long Uint64; //
typedef unsigned int ACCOUNTDATATYPE;   //账户的类型 
typedef unsigned int ACTORDATATYPE;     //角色的类型 

namespace jxSrvDef
{
	/* 服务器类型定义 */
	typedef enum tagServerType
	{
		InvalidServer = 0,	//未定义服务器类型
		GateServer = 1,		//网关服务器
		LogServer = 2,		//日志服务器
		NameServer = 3,		//名称服务器
		SessionServer = 4,	//会话服务器
		DBServer = 5,		//数据服务器
		GameServer = 6,		//游戏服务器	
	}SERVERTYPE;

	
	/* 获取服务器类型名称字符串 */
	inline LPCTSTR getServerTypeName(tagServerType eServerType)
	{
		switch(eServerType)
		{
		case GateServer: return _T("网关");
		case LogServer: return _T("日志");
		case NameServer: return _T("名称");
		case SessionServer: return _T("会话");
		case DBServer: return _T("数据");
		case GameServer: return _T("逻辑");
		default: return _T("未知");
		}
	}

	/* 全局会话状态定义 */
	typedef enum tagGlobalSessionState
	{
		gsWaitQueryChar = 1,	//等待连接到DB服务器查询角色
		gsSelChar = 2,			//已经连接到DB服务器，出于创建、选择角色状态
		gsWaitEntryGame = 3,	//等待连接游戏服务器
		gsInGame = 4,			//已经连接游戏服务器开始游戏
	}GSSTATE;

	/* 
		DBCenter转发消息类型。注意，不要用0作为请求号，默认的心跳包消息ID就是0
	*/
	typedef enum tagDBCenterRequest
	{
		DBCenterHeartBeat = 0,			// DBCenterClient和DBCenterServer之间的心跳包
		DBCenterGateRequest = 100,		// DBCenter转发的网关请求，包括选择角色、创建角色、删除角色等请求
		DBCenterLogicRequest			// DBCenter转发的逻辑请求，包括获取角色基本数据、装备数据等请求
	}DBCENTERREQUEST;

	/******
	 * 服务器注册结构，当服务器连接到另一个服务器，
	 * 需要发送注册服务器的数据结构以便注册服务器，
	 * 注册服务器成功后才可正常通信。
	****************************************************/
	typedef struct tagServerRegistData
	{
		static const INT GT_JianXiaoJiangHu = 0x484A584A;//《剑啸江湖》的游戏ID
		INT			GameType;		//游戏ID，《剑啸江湖》中固定为0x484A584A
		INT			ServerType;		//服务器类型，值为SERVERTYPE枚举类型中的枚举值
		INT			ServerIndex;	//服务器唯一编号（服务器编号仅对游戏服务器有意义）
		INT			Recv1;
		CHAR		ServerName[64];	//服务器名称，需包含0终止符
	}SERVER_REGDATA, *PSERVER_REGDATA;

	/*** 全局异步操作返回值定义 **/
	typedef enum tagASyncOPError
	{
		aeSuccess = 0,				//成功
		aeLocalInternalError = 1,	//本地服务器内部错误
		aeServerInternalError = 2,	//远程服务器内部错误
		aeInvalidName = 3,			//名称字符串无效
		aeLocalDataBaseError = 4,	//本地服务器数据库操作错误
		aeServerDataBaseError = 5,	//远程服务器数据库操作错误
		aeNameInuse = 6,			//名称已被使用
		aeTimedOut = 0xFFFF,		//操作超时（本地定义的错误，异步操作实现的服务器返回的错误）
	}ASYNC_OPERROR;
		
	
	/** 登陆的时候返回的错误码**/
	typedef enum tagLoginOPError
	{
		enSuccess = 0,	     //操作成功
		enPasswdError =1,    //密码错误
		enNoAccount=2,       //没有这个账号
		enIsOnline =3,       //已经在线
		enServerBusy =4,     //服务器忙
		enServerClose =5,    //服务器没有开放 
		enSessionServerError =6 , //session服务器有问题，比如db没有连接好
		enServerNotExisting =7, //不存在这个服务器
		enFcmLimited =8 ,      //账户纳入防沉迷
		enSessionServerClose =9, //会话服务器处于关闭状态
		enDbServerClose =10,     //数据服务器处于关闭状态
		enGMLoginFailError =11,    //gm在非法的ip登陆
		enIpError =12,    //ip收到了限制
		enIpTooManyConnect =13,    //ip连接了太多
		enIpMd5Error =14,    //Md5计算错误
		enSignOutofDate =15,    //发过来的时间已经过期了
		enTimeFormatError =16,    //前面的格式错误
		enAccountSeal =17,    //登陆过封停的账户
		enAccountFull = 18,//当前服务器过载,请选择其他服务器
		enServerOnLineCountMAx = 126, //在线达到最大

	}SESSION_SERVER_OPERROR;



	/*** 定义服务器之间通信消息号的数据类型 **/
	typedef WORD	INTERSRVCMD;


	//定义字符串数据长度
	static const int MAX_ACCOUNT_NAME_LENGTH  = 80;	//最大的账户名字长度
	static const int ACCOUNT_NAME_BUFF_LENGTH = 81;  //账户的buff长度

	typedef char ACCOUNT[ACCOUNT_NAME_BUFF_LENGTH];

	static const int MAX_ACTOR_NAME_LENGTH  = 32;   //最大的玩家名字的长度
	static const int ACTOR_NAME_BUFF_LENGTH = 33;   //玩家名字的buff长度
	static const int ENOUNCE_BUFF_LENGTH = 100;		//最大宣言长度	

	static const int MAX_TOKEN_LENGTH = 48;			//运营商的渠道统计的最大长度
	
	typedef char ACTORNAME[ACTOR_NAME_BUFF_LENGTH]; //玩家的名字

	typedef char ENOUNCE[ENOUNCE_BUFF_LENGTH];		 //帝王宣言

	static const int MAX_PASSWD_LENGTH		= 64;    //最大的密码长度
	static const int MAX_GUILD_NAME_LENGTH	= 32;    //最大的公会的名字

	/***
	 * 逻辑服务器向数据服务器发送创建帮会的数据结构
	 * 包含创建帮会的必要数据
	 ***************************************************/
	typedef struct tagCreateGuildData
	{
		CHAR	sGuildName[33];		//帮会名称，包含0终止字符
		CHAR	sFoundName[33];		//创始人的名字，插入数据库的时候需要用到
		BYTE	nZY;
		WORD	wIcon;
		int		nIndex;
		Uint64	hActor;
		unsigned int		nActorid;
		int		nServerIndex;
	}CREATEGUILD_DATA, *PCREATEGUILD_DATA;

	/*** 会话服务器广播打开会话的数据结构 ***/
	typedef struct tagGlobalSessionOpenData
	{
		INT         nGateIndex;     //处于网关的编号
		INT			nSessionId;		//全局会话ID		
		INT			nServerIndex;	//会话登录的目标服务器
		INT			nRawServerId;	// 角色创建时的服务器ID
		LONGLONG	nClientIPAddr;	//客户端登陆的IP地址，支持IPv6
		DWORD		dwFCMOnlineSec;	//防沉迷系统中不满足下线休息时间而产生的连续在线时间，单位是秒
		GSSTATE		eState;			//会话状态
		ACCOUNT		sAccount;		//会话账号字符串
		int         nGmLevel;       //账户的GM等级
		UINT64      lKey;           //生效的key
		
	}GLOBALSESSIONOPENDATA, *PGLOBALSESSIONOPENDATA;

	/***
	/*  游戏网关路由表结构
	*******************************************/
	typedef struct tagGameServerRoute
	{
		int		nServerIndex;	//服务器ID
		int		nRouteCount;	//路由地址数量
		struct  
		{
			char	sHost[124];	//服务器地址（支持域名）
			int		nPort;		//服务器端口
			int		nUserCount;	// 此路由上的用户数目
		}	RouteTable[16];		//路由地址表
	}GAMESERVERROUTE, *PGAMESERVERROUTE;
}


	
#endif
