#pragma once


/****************************************************
 *
 *					全局会话抽象类 
 *
 ***************************************************/

class CCustomGlobalSession
{
public:
	INT					nGateIndex;     //处于网关的编号
	unsigned int		nSessionId;		//全局会话ID
	int					nServerIndex;	//会话登录的服务器ID
	int					nRawServerId;// 角色原始创建时的服务器ID
	LONGLONG			nClientIPAddr;	//会话登录的IP地址（支持IPv6）
	DWORD				dwFCMOnlineSec;	//防沉迷系统中不满足下线休息时间而产生的连续在线时间，单位是秒
	jxSrvDef::GSSTATE	nState;			//会话状态（登录、查询角色还是已经进入游戏）
	jxSrvDef::ACCOUNT	sAccount;		//会话账号
	unsigned long long	dwSessionTick;	//会话开启的时间
	int                 nGmLevel;     //GM等级
	UINT64              lKey;        //key
	
};