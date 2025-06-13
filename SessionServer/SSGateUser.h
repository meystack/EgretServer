#ifndef _SS_GATEUSER_H_
#define _SS_GATEUSER_H_

/**
	会话网关用户类
	此类不可再继承
**/

class CSSGateUser
	: public CCustomServerGateUser
{
public:
	typedef CCustomServerGateUser Inherietd;

	//网关用户状态
	enum eGateUserState
	{
		guUnavailable = 0,	//未登录
		guConfimLogin,		//正在确认账号是否在线
		guLoginOK,			//登录成功
		guLoginFail,		//登录失败
		guSelectServer		//选择了服务器
	};

	//检查会话是否在线的最终结果
	enum eGateCheckSessionResult
	{
		crNoResult,			//尚未有结果
		crOnline,			//账号已在线
		crNotOnline,		//账号不在线，可以登录
		crTimedOut,			//检查超时
	};

public:
	INT					nUserSessionId;			//全局会话ID
	int				    nTodayOnlineSec;		//账号今日在线时间，主要用于防沉迷系统
	long long		    nLastLoginIP;			//上次登录IP
	eGateUserState		nUserState;				//网关用户状态
	eGateCheckSessionResult	nCheckSessionResult;//登录确认的结果
	TICKCOUNT			dwLoginConfimTimeOut;	//登录确认超时时间
	int                 nGmLevel;               //账户的GM等级
	int                 nServerIndex;           //要登陆的服务器的编号
	bool                bHasReplySelServer;     //是否回复了选择服务器，避免重复回复
};

#endif

