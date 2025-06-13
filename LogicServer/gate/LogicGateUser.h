#pragma once

/*********************************************************/
/*
/*              逻辑服务器网关用户对象
/*
/*********************************************************/

class CLogicGateUser :	public CCustomServerGateUser, public Counter<CLogicGateUser>
{
public:
	typedef CCustomServerGateUser Inherited;

	CLogicGateUser()
	{
		nCharId = 0;
		sCharName[0] = 0;
		nHandleIndex =0;
		lLogicKey =0;
		nLastRandNameIndex =-1;
		sLastRandName[0]=0;
		dwFlag =0;
	}

	unsigned int    nCharId;		//网关登录用户的角色ID
	jxSrvDef::ACTORNAME	sCharName;	//网关登录用户的角色名称
	Uint64  lLogicKey;              //逻辑初始化的key，用于查找
	int     nLastRandNameIndex;     //数据服返回的上一次的随机的名字
	
	jxSrvDef::ACTORNAME	sLastRandName;	//上次登陆随机出来的名字
	
	
	//一些逻辑用到的标记，以后就随便增加，64个以内
	union 
	{
		Uint64 dwFlag;	//标志组1
		struct
		{
			bool	recordLog:1;      //是否记录流通日志
			bool    boHasSecPsw:1;                //是否有2级密码
			bool	boCheckSecPswSucess:1;        //2级密码的监测是否成功
			bool    boWhiteIpLogin:1;            //是否是白名单登陆的
		};
	} ;

	unsigned nHandleIndex;  //这个是玩家实体的句柄的index，保存这个为了发送到逻辑服务器的数据能够快速定位到一个玩家身上
};