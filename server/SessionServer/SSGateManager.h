#ifndef _SS_GATEMANAGER_H_
#define _SS_GATEMANAGER_H_

/*
class CSSManager;

class CSSGateManager
	: public CCustomGateManager
{
public:
	typedef CCustomGateManager Inherited;
private:
	CSSGate				m_Gates[MaxGateCount];	//网关列表
	CSSManager*			m_pSSManager;			//所属DBEngine
	CSQLConenction		m_SQLConnection;		//网关数据库连接对象
	TICKCOUNT			m_dwReconnectSQLTick;	//下次重新连接数据库的时间
	TICKCOUNT           m_dwDumpTime;           //写日志的时间

private:
	BOOL ConnectSQL();
	//确认会话是否在线的结果处理
	VOID ResultUserSessionConfim(const int nSessionId, const BOOL boOnline);
protected:
	//返回异步查询网关中是否存在指定会话ID的用户的结果 
	VOID OnResultGateUserExists(int nSessionId, const BOOL boOnline);

	//收到了确认opensession的数据包
	VOID OnResultConformOpenSession(int nSessionId,int nServerIndex);

	//进行内部消息的处理分派
	VOID DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4);
	VOID OnRun();
public:
	CSSGateManager(CSSManager *lpSSManager);
	~CSSGateManager();

	VOID Initialize();
	VOID Uninitialize();
};
*/


#endif

