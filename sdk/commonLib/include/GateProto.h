#pragma once

/*******

   网关与内部服务器通信协议以及基础数据结构定义

**********************************************************/

#define	RUNGATECODE				0xAA55AA55		// 网关通信
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
