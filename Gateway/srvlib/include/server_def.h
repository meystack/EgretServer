#ifndef _SERVER_DEF_H_
#define _SERVER_DEF_H_

/*****************************************************
*
*	服务端通用定义文件
*
*****************************************************/
#include "os_def.h"


#define TAG_VALUE 0xCCEE		   //默认的包的开头的2个字节
#define SMALL_HEAD

#pragma pack(push,4)
typedef struct
{
	int tag;
	int len;
	struct 
	{
		uint16_t dataSum;
		uint16_t hdrSum;
	} EncodePart;
#ifndef SMALL_HEAD
	union
	{
		struct
		{
			int recvTime;	// 网关接收消耗时间
			int sendTime;	// 网关发送消耗时间
		}s;
		int64_t reserve; // 时间点，用于网关接受到客户端数据后打上的时间戳。
	}u;
#endif
}DATAHEADER;


typedef struct
{
	uint64_t socket_;
	uint16_t index_;
	uint16_t gate_id_;
}NetId;

#pragma pack(pop)

/*******

   网关与内部服务器通信协议以及基础数据结构定义

**********************************************************/

#define RUNTEMPCODE             0XEEFF          // temp部分，也用来校验
#define	GM_OPEN					1				// 打开新用户会话，当客户端连接到网关时网关向服务器发送此消息
#define	GM_CLOSE				2				// 关闭用户会话，当客户端与网关的链接断开时网关向服务器发送此消息
#define	GM_CHECKSERVER			3				// 网关回应服务器的心跳包消息
#define	GM_CHECKCLIENT			4				// 服务器向网关发送心跳包消息
#define	GM_DATA					5				// 网关转发用户通信数据到服务器，服务器向网关发送用户通信数据也使用此消息
#define	GM_SERVERUSERINDEX		6				// 服务器回应网关，新连接的用户在服务器上分配的用户ID号
#define GM_APPKEEPALIVE			7				// 服务器给网关发的应用层心跳包
#define GM_APPKEEPALIVEACK		8				// 网关回应服务器的应用层心跳包
#define GM_APPBIGPACK			9				// 应用层大包
#define GM_GATEUSERINFO			10				// 网关通知服务器网关用户数量
#define GM_CLOSE_SERVER         0xEEF1          // 关闭网关程序

#define	RUNGATECODE				0xAA55AA55		// 网关通信
typedef unsigned short      WORD;
typedef unsigned int       DWORD;

/* 网关通信协议头 */

typedef struct tagGateMsgHdr
{
	unsigned long long	nSocket;		//网关连接套接字标识（用户套接字）
	unsigned int		dwGateCode;		//标识码，总是为RUNGATECODE
	WORD				wSessionIdx;	//网关用户ID（由网关生成）
	WORD				wIdent;			//通信消息号
	WORD				wServerIdx;		//服务器用户ID（由服务器返回）
	WORD				wTemp;			//保留
	int					nDataSize;		//通信数据大小（不包括协议头）
	long long 			tickCount; 		//收到该数据包的时间

}GATEMSGHDR, *PGATEMSGHDR;


typedef struct tagGateUserInfo
{
	int					nGatePort;		// 网关监听用户连接的端口号
	int					nUserCount;		// 网关当前建立用户连接的数量
}GATEUSERINFO, *PGATEUSERINFO;

typedef int ActorId;
typedef int64_t FuBenUId;

// 用于测试网络传包性能的特殊id

#define 	TEST_GATEAY_CODE  0xfabc1254adee
#define		TEST_SERVER_CODE  0xffbb56facedd
#define		TEST_SERVER_CODE1 0x12698745200d
namespace SrvDef
{
	/* 服务器类型定义 */
	typedef enum tagServerType
	{
		InvalidServer = 0,	//未定义服务器类型
		LogServer = 2,		//日志服务器
		SessionServer = 4,	//会话服务器
		T_DBServer = 5,		//数据服务器
		T_GameServer = 6,	//游戏服务器	
		GateServer = 1,		//网关服务器
	}SERVERTYPE;

	
	/* 获取服务器类型名称字符串 */
	inline const char* getServerTypeName(int eServerType)
	{
		switch(eServerType)
		{
		case GateServer: return ("gateway");
		case LogServer: return ("log");
		case SessionServer: return ("login");
		case T_DBServer: return ("db");
		case T_GameServer: return ("game");
		default: return ("unknow");
		}
	}

	/******
	 * 服务器注册结构，当服务器连接到另一个服务器，
	 * 需要发送注册服务器的数据结构以便注册服务器，
	 * 注册服务器成功后才可正常通信。
	****************************************************/
	typedef struct tagServerRegistData
	{
		static const int GT_ID = 0x12AA66CA;
		int			GameType;		//游戏ID，固定为0x12AA66CA
		int			ServerType;		//服务器类型，值为SERVERTYPE枚举类型中的枚举值
		int			ServerIndex;	//服务器唯一编号（服务器编号仅对游戏服务器有意义）
		int			Recv1;
		char		ServerName[64];	//服务器名称，需包含0终止符
	}SERVER_REGDATA, *PSERVER_REGDATA;


	//定义字符串数据长度
	static const int MAX_ACCOUNT_NAME_LENGTH  = 80;	//最大的账户名字长度
	static const int ACCOUNT_LENGTH = 81;  //账户的buff长度

	typedef char ACCOUNT[ACCOUNT_LENGTH];

	static const int MAX_ACTOR_NAME_LENGTH  = 32;   //最大的玩家名字的长度
	static const int ACTOR_NAME_BUFF_LENGTH = 33;   //玩家名字的buff长度
	
	typedef char ACTORNAME[ACTOR_NAME_BUFF_LENGTH]; //玩家的名字

	static const int MAX_PASSWD_LENGTH		= 32;    //最大的密码长度
	static const int MAX_GUILD_NAME_LENGTH	= 32;    //最大的公会的名字

	
	/***
	*  游戏网关路由表结构
	*******************************************/
	typedef struct tagGameServerRoute
	{
		int		nServerIndex;	//服务器ID
		int		nRouteCount;	//路由地址数量
		struct
		{
			char	sHost[124];	//服务器地址（支持域名）
			int		nPort;		//服务器端口
		}	RouteTable[16];		//路由地址表
	}GAMESERVERROUTE, *PGAMESERVERROUTE;

}


	
#endif
