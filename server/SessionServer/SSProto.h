#ifndef _SS_PROTO_H_
#define _SS_PROTO_H_




//账号状态标志位

/*
enum tagSessionCmd
{
	//效验用户名和密码，以及是否能够登陆 （角色名称：shortString,密码：shortString）
	cLoginSyn = 1,		
	//选择服务器(errorid：Short int)
	cSelectServer =2,

	//创建一个账户，用于创建账户测试的，账户的名字和密码
	cCreateAccount =3,

	//效验用户名和密码，以及是否能够登陆的结果 (serverid：int)
	sLoginAck = 1,

	//选择服务器的结果  (serverid：int,res：short int,Host: short string, port:short int)
	sSelectServer = 2,

	//创建一个账户的结果
	sCreateAccount =3,
};
*/



/**
全局会话结构
在修改此结构的时候需要注意一下结构大小，通过内存池申请的中等包的大小是256字节，
如果此结构大于256字节且小于64K则会造成较多的内存浪费
**/
typedef	struct tagGameSession
{
	Uint64				nSessionID;		//会话ID
	int					nServerIndex;	//会话登录的服务器ID
	int                 nIsRawLogin;    //是否在原始服务器登录

	LONGLONG			nIPAddr;		//会话登录的IP地址（支持IPv6）
	TICKCOUNT			dwSessionTick;	//会话建立的时间
	jxSrvDef::GSSTATE	nState;			//会话状态（登录、查询角色还是已经进入游戏）
	jxSrvDef::ACCOUNT	sAccount;	//会话账号
	TICKCOUNT			dwTimeOut;		//会话超时时间（在等待查询角色以及等待进入游戏状态下有效）
	int				nConfimSrvCount;//确认会话是否登录，需要等待回应的服务器数量
	int             nGmLevel;       //GM等级
	UINT64              lKey;      //登陆的随机key
}GAMESESSION, *PGAMESESSION;

/**
	角色网关路由地址结构
**/
typedef struct tagCharGateRoute
{
	int		nServerIndex;	//服务器ID
	int		nRouteCount;	//路由地址数量
	struct  
	{
		char	sHost[124];	//服务器地址（客户端支持域名）
		int		nPort;		//服务器端口
	}		RouteTable[16];	//路由表
}CHARGATEROUTE, *PCHARGATEROUTE;

/**
	管理员登录结构
	管理员登录不受服务器人员已满的限制且会自动剔除已经在线的会话
**/
typedef struct tagAdminLoginRecord
{
	enum eRecordType
	{
		alAccount = 0,		//记录是一个账号
		alIPAddress,		//记录是一个IP地址
	}		eRecType;
	char	sRecData[124];	//如果eRecType为alAccount则表示账号，为alIPAddress则表示IP地址
}ADMINLOGINRECORD, *PADMINLOGINRECORD;

/*
**	记录逻辑服务器ID、服务器名称以及连接的公共服务器ID等信息
*/
typedef struct tagLogicServerInfo
{
	int		server_id;					// 服务器ID
	int		cserver_id;					// 连接的公共服务器ID
	char	serverName[32];				// 服务器名称
}LOGICSERVERINFO, *PLOGICSERVERINFO;

/*
** 定义公共服务器连接的普通逻辑服务器列表
*/
class CommServerClient
{
public:
	CommServerClient()
	{			
	}
	~CommServerClient()
	{
		Clear();
	}
	void Clear()
	{
		ClientList.trunc(0);
	}
	void Add(const int nClientServerId)
	{
		bool bRepeat = false;
		for (INT_PTR i = 0; i < ClientList.count(); i++)
		{
			if (nClientServerId == ClientList[i])
			{
				OutputMsg(rmError, _T("发现重复的ClientServerId配置[cserverid=%d serverid=%d]"),
						  cserver_id,
						  nClientServerId);
				bRepeat = true;
				break;
			}
		}

		if (!bRepeat)
			ClientList.add(nClientServerId);
	}

	void Trace()
	{
		//OutputMsg(rmTip, _T("公共服务器[%d]连接的客户端数量为："), cserver_id, (int)ClientList.count());
		for (INT_PTR  i = 0; i < ClientList.count(); i++)
		{
			OutputMsg(rmTip, _T("客户端_%2d服务器ID:%d"), (int)(i+1), (int)ClientList[i]);
		}
	}
	int					cserver_id;		// 公共服务器ID
	CBaseList<int>		ClientList;		// 连接的客户逻辑服务器ID列表
};

class CommServerClientList
{
public:
	~CommServerClientList()
	{
		Clear();
	}

	void Clear()
	{
		for (INT_PTR i = 0; i < m_ClientList.count(); i++)
		{
			m_ClientList[i].Clear();
		}
	}

	void AddClient(const int nCommServerId, const int nClientServerId)
	{
		if (nCommServerId == nClientServerId)
			return;

		CommServerClient *pServerClient = GetServerClient(nCommServerId);
		if (!pServerClient)
		{
			CommServerClient sc;
			sc.cserver_id = nCommServerId;
			INT_PTR nIdx = m_ClientList.add(sc);
			pServerClient = (CommServerClient *)&m_ClientList.get(nIdx);
		}	

		pServerClient->Add(nClientServerId);
	}

	CommServerClient* GetServerClient(const int nCommServerId) const
	{
		for (INT_PTR i = 0; i < m_ClientList.count(); i++)
		{
			if (nCommServerId == m_ClientList[i].cserver_id)
				return &m_ClientList[i];
		}

		return NULL;
	}

	void Trace()
	{
		for (INT_PTR i = 0; i < m_ClientList.count(); i++)
		{
			m_ClientList[i].Trace();
		}
	}

private:
	CBaseList<CommServerClient>		m_ClientList;
};

//客户端登录失败错误号
#define	LOGINERR_USRNOTEXISTS			-1	//用户不存在
#define	LOGINERR_PSWDFAILURE			-2	//密码错误
#define	LOGINERR_USERDISABLED			-3	//账号已被禁用
#define	LOGINERR_USERNOTAVAILABLE		-4	//尚未激活游戏
#define	LOGINERR_LOGINLOCKCHECKFAIL		-5	//
#define	LOGINERR_DISABLELOGINTHISGAME	-6	//
#define	LOGINERR_ALREADYLOGIN			-7
#define	LOGINERR_SERVERBUSY				-8
#define	LOGINERR_LOGINONWAITING			-9
#define	LOGINERR_INPUT_SECURENUMBER		-10
#define	LOGINERR_USERFULL				-11

/*	网关模块内部消息定义
*******************************************/
#define GTIM_CONFIM_SESSION_RESULT			10001	//会话服务器向网关用户投递确认会话是否在线的结果(Param1=会话ID，Param2=是否在线(0:1))
#define GTIM_CONFIRM_OPEN_SESSION           10012   //确认已经打开了会话

/*  会话服务器模块内部消息定义
*******************************************/
#define SSIM_POST_CLOSE_SESSION				20001	//按会话ID关闭会话(Param1=会话ID,Param2=会话连续在线时间（用于防沉迷中统计在线时间）)
#define SSIM_USER_EXISTS_OF_SESSION_RESULT	20002	//网关按会话ID检查用户是否存在后向DBSSClient返回的消息(Param1=会话ID,Param2=BOOL)
#define SSIM_CHANGE_SESSION_STATE			20003	//网关向SessionServer投递改变会话状态的消息(Param1=会话ID,Param2=会话状态)
#define SSIM_GATE_USER_CLOSED				20004	//网关向SessionServer投递网关用户已关闭的消息(Param1=会话ID)
#define SSIM_POST_ALL_CLIENT_MSG			20005	//向所有会话服务器的客户端发送消息(Param1=服务器类型,Param2=数据包,Param3=数据包大小)
#define SSIM_CONFIM_SESSION_ONLINE			20006	//确认会话是否在线(Param1=会话ID,Param2=服务器ID)
#define SSIM_CONFIM_SESSION_RESULT			20007	//数据或引擎客户端返回会话是否在线(Param1=会话ID,Param2=是否在线（0:1）)
#define SSIM_CLOSE_SESSION_BY_ACCOUNT		20008	//通过账号字符串查找并关闭会话(Param1=字符串指针)
#define SSIM_DEBUG_PRINT_SESSIONS			20009	//调试消息：打印全局会话(Param1=会话数量)
#define SSIM_QUERY_YUANBAO_COUNT			20010	// 查询元宝数量
#define SSIM_WITHDRAW_YUANBAO				20011	// 提取元宝
#define SSIM_CLOSE_SESSION_BY_ACCOUNTID     20012   //通过accountid关闭会话

#define SSIM_KICK_ONLINE_CROSSSERVER_ACTOR     20013   //踢掉在线的跨服用户


/*  SQL查询语句定义
*******************************************/
// 此处定义各种SQL查询语句，使用存储过程调用SQL查询

//加载角色网关路由表(QUERY)
static LPCSTR szLoadCharGateRouteTable		= "call loadcharactorserveraddress()";
//加载管理员登录表(QUERY)
static LPCSTR szLoadAdministLoginTable		= "call loadadministlogintable()";
// 加载服务器信息列表
static LPCSTR szLoadServerInfoList			= "call loadlogicserverinfolist()";

static LPCSTR szUserLoginGetGlobalUser		=  "call logingetglobaluser(\"%s\")";//用户登录时依据账号查询数据(QUERY)

static LPCSTR szUpdateUserLogin= "call updateGlobaluserLogin(%u,%lu)";

static LPCSTR szUpdateUsernlineTime="call updateGlobaluserOnlineTime(%u,%u)";

static LPCSTR szGetFcmTime= "call getFcmTime(%u)";  //获取防沉迷的时间

static LPCSTR szCreateAccount=  "call createglobaluser(\"%s\",\"%s\",\"%s\")";    //创建一个账户，参数为名字，密码和身份证号码

static LPCSTR szSessionLoadUserItem = "call loaduseritem(%u,%u,%d)";//获取角色的活动背包内容,[账户id，角色id,serverindex]
static LPCSTR szSessionGetUserItem = "call getuseritem(%u,%u,%lld,%d)";//领取物品[账户id，角色id,消息id]
static LPCSTR szSessionDelUserItem = "call deleteuseritem(%lld)";//删除用户的活动背包
static LPCSTR szGetSASeriesInfo = "call getvaseriesinfo(%llu)"; // 查询增值服务器序列号信息
static LPCSTR szDeleteSASeries = "call deletevaseries(%llu, %u, %u, \"%s\")"; // 删除增值序列号

static LPCSTR szDeleteCSRank = "call deletecsrank(%d)";	//删除跨服排行[客户端服务器ID]
static LPCSTR szSaveCsRank = "insert into csrank(actorid,actorname,sex,job,serverindex,value) values";	//更新跨服战力排行[角色ID,服务器ID,职业,战力]
static LPCSTR szLoadCsRank = "call loadcsrank()";	//获取每个职业战力第一名单

#endif

