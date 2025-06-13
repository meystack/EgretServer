#pragma once

/*****************************************************/
/*
/*             在逻辑服务器中的全局会话类
/*
/*****************************************************/

class CActor;

class CLogicSession :
	public CCustomGlobalSession
{
public:
	typedef CCustomGlobalSession Inherited;

	CActor*	pGamePlayer;	//全局会话关联的用户玩家对象
};