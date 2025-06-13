#ifndef _CUSTOM_SERVER_GATEUSER_H_
#define _CUSTOM_SERVER_GATEUSER_H_

using namespace wylib::time::tick64;

/**
 *	网关用户类
 *	此类不可包含析构函数和任何虚函数
******************************************/
class CCustomServerGateUser
{
public:
	INT64					nSocket;			//连接到网关中的套接字
	INT						nGateSessionIndex;	//连接到网关中的用户索引
	INT						nServerSessionIndex;//在服务器中的用户索引
	INT						nRawServerIndex;	// 角色的原始服务器ID
	unsigned int			nGlobalSessionId;	//网关用户的全局会话ID
	char					sIPAddr[32];		//客户端地址
	char					sAccount[jxSrvDef::ACCOUNT_NAME_BUFF_LENGTH];		//帐号字符串
	INT64					dwConnectTick;		//连接时间
	INT64					dwLastMsgTick;		//上次通信时间
	INT64					dwDelayCloseTick;	//延时关闭的时间
	BOOL					boMarkToClose;		//是否标记为主动关闭
	unsigned long long      dwGateTickCount;    //网关发该包的时间
public:
	CCustomServerGateUser()
	{
		nSocket = 0;
		nGateSessionIndex = 0;
		nServerSessionIndex = 0;
		sIPAddr[0] = 0;
		dwConnectTick = 0;
		dwGateTickCount =0;
	}
	//延时关闭
	inline void DelayClose(const TICKCOUNT dwDelayTick)
	{
		dwDelayCloseTick = _getTickCount() + dwDelayTick;
	}
};

#endif

