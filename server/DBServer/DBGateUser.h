#ifndef _GATEUSER_H_
#define _GATEUSER_H_

/**
	网关用户类
	此类不可再继承
**/
class CDBGateUser
	: public CCustomServerGateUser
{
public:
	typedef CCustomServerGateUser Inherited;

public:
	PCREATEACTORDATAREQ		pCreateCharStruct;	//创建角色的数据（仅当启用名称服务器的时候有意义，如果值为NULL则表示没有在创建角色）

	TICKCOUNT				dwCreateCharTimeOut;//创建角色时通过名称服务器申请角色名称的超时时间

	INT_PTR					nNameLibIndex;      //随机名字在名字库中的索引
	char					sRandomName[32];    //系统发给玩家的随机名字

	WORD					nZyList;		//发给客户端的可选的阵营列表，用位表示，比如第一位是1表示阵营1可选，如果是0表示都不可选，由系统分配
	char 	szIP[128];		//登陆的ip
	int 	nPort;           //登陆的端口

public:
	CDBGateUser():Inherited()
	{
		nGlobalSessionId = 0;
		sAccount[0] = 0;
		nZyList = -1;
		szIP[0]=0;
		nPort=0;
	}
};

#endif

