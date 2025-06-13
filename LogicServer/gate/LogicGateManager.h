#pragma once

/***************************************************************/
/*
/*                      逻辑服务器网关管理器
/*
/***************************************************************/

class CLogicServer;

class CLogicGateManager :
	public CCustomGateManager
{
public:
	typedef CCustomGateManager Inherited;

public:
	CLogicGateManager(CLogicServer* lpLogicServer);
	~CLogicGateManager();
	
	
	/* ★初始化网关队列 */
	virtual VOID Initialize();
	/* ★销毁网关队列 */
	virtual VOID Uninitialize();

	/* 启动网关服务器 */
	BOOL Startup();
	/* 停止网关服务器 */
	VOID Stop();

	//根据角色ID以及key删除玩家
	void KickUser(ACCOUNTDATATYPE accountId,UINT64 lKey );

	//关闭所有的网关程序
	void CloseAllGates();

	
	CLogicGateUser * GetUserPtr(INT_PTR  nGateIndex, UINT64 lKey )
	{
		if(nGateIndex <0 || nGateIndex >= MaxGateCount) return NULL;
		if(m_Gates[nGateIndex].connected())
		{
			return m_Gates[nGateIndex].GetGateUserByKey(lKey);
		}
		else
		{
			return NULL;
		}
	}


protected:
	/*** 覆盖父类的相关逻辑处理函数 ***/
	/* 返回异步查询网关中是否存在指定会话ID的用户的结果 */
	VOID OnResultGateUserExists(int nSessionId, const BOOL boOnline);

	//返回数据
	virtual VOID DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4 );

	
private:
	CLogicServer			*m_pLogicServer;	//所属逻辑服务器
	CLogicGate				m_Gates[MaxGateCount];//静态网关连接对象列表
};