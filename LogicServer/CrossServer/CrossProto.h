#ifndef _CROSS_PROTO_H_
#define _CROSS_PROTO_H_
/*
	CopyRight@2021 跨服通用结构
	author: 马海龙
	time: 2021-04-28
*/

/**
全局会话结构
在修改此结构的时候需要注意一下结构大小，通过内存池申请的中等包的大小是256字节，
如果此结构大于256字节且小于64K则会造成较多的内存浪费
**/
typedef	struct tagGameCross
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
	UINT64              lKey;      //登陆的随机key
}GAMECROSS, *PGAMECROSS;

/*
**	记录逻辑服务器ID、服务器名称以及连接的公共服务器ID等信息
*/
typedef struct tagLogicServerInfo
{
	int		server_id;					// 服务器ID
	int		cserver_id;					// 连接的公共服务器ID
	char	serverName[32];				// 服务器名称
}LOGICSERVERINFO, *PLOGICSERVERINFO;


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

/*  跨服服务器模块内部消息定义
*******************************************/
#define CSIM_POST_CLOSE_SESSION				20001	//按会话ID关闭会话(Param1=会话ID,Param2=会话连续在线时间（用于防沉迷中统计在线时间）)
#define CSIM_USER_EXISTS_OF_SESSION_RESULT	20002	//网关按会话ID检查用户是否存在后向DBSSClient返回的消息(Param1=会话ID,Param2=BOOL)
#define CSIM_CHANGE_SESSION_STATE			20003	//网关向SessionServer投递改变会话状态的消息(Param1=会话ID,Param2=会话状态)
#define CSIM_GATE_USER_CLOSED				20004	//网关向SessionServer投递网关用户已关闭的消息(Param1=会话ID)
#define CSIM_POST_ALL_CLIENT_MSG			20005	//向所有会话服务器的客户端发送消息(Param1=服务器类型,Param2=数据包,Param3=数据包大小)
#define CSIM_CONFIM_SESSION_ONLINE			20006	//确认会话是否在线(Param1=会话ID,Param2=服务器ID)
#define CSIM_CONFIM_SESSION_RESULT			20007	//数据或引擎客户端返回会话是否在线(Param1=会话ID,Param2=是否在线（0:1）)
#define CSIM_CLOSE_SESSION_BY_ACCOUNT		20008	//通过账号字符串查找并关闭会话(Param1=字符串指针)
#define CSIM_CLOSE_SESSION_BY_ACCOUNTID     20012   //通过accountid关闭会话

#define CSIM_KICK_ONLINE_CROSSSERVER_ACTOR     20013   //踢掉在线的跨服用户

#endif //_CROSS_PROTO_H_

