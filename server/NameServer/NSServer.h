#pragma once

class CNameServerClientConnection;

extern std::string g_ConfigPath;


class CNameServer
	: public CCustomServerSocket
{
public:
	typedef CCustomServerSocket Inherited;

public:
	CNameServer();
	~CNameServer();

	//获取数据库连接对象
	inline CSQLConenction& getSQLConnection(){ return m_SQLConnection; };
	//判断名称字符串中是否含有非法字符
	bool hasFilterWordsInclude(LPCSTR sNameStr);
	//设置数据库连接配置
	VOID SetSQLConfig(LPCSTR sHost, INT_PTR nPort, LPCSTR sDBName, LPCSTR sDBUser, LPCSTR sDBPass);

	VOID SetServerName(const char* sServerName) {_asncpytA(m_sServerName,sServerName);}
	char* GetServerName()	{return m_sServerName;}

	//设置运营商的id
	inline void SetSpId(INT_PTR nSpid) {m_spId = nSpid; }

	//获取运营商的id
	inline INT_PTR GetSpId() { return m_spId ; }

protected:
	//当客户端连接后会调用此函数以便创建一个连接对象，子类应当覆盖此函数
	CCustomServerClientSocket* CreateClientSocket(SOCKET nSocket, PSOCKADDR_IN pAddrIn);
	//销毁一个已经建立的客户端连接对象，默认的操作是进行delete销毁
	VOID DestroyClientSocket(CCustomServerClientSocket *pClientSocket);
	//客户端连接接收的通知函数
	VOID OnClientConnect(CCustomServerClientSocket *pClientSocke);
	//例行执行的通知函数
	VOID OnRun();
	//覆盖启动服务回调函数
	BOOL DoStartup();
	//覆盖停止服务回调函数
	VOID DoStop();

	//初始化数据库的actorid
	//根据运营商的spid,  spid <<24 +1 是最小的actorid，如果角色表里则插入一个这样的actorid作为占位
	bool InitActorId();

	//初始化帮派的ID,
	//根据运营商的spid,  spid <<24 +1 是最小的guild，如果角色表里则插入一个这样的actorid作为占位
	bool InitGuildId();
	
private:
	CSQLConenction		m_SQLConnection;	//数据库连接对象，所有建立连接的客户端需要查询数据库的时候都使用此连接
	CBaseList<CNameServerClientConnection*>	m_FreeConnectionList;//空闲客户端连接对象池
	PFT_MBCS_CHAR		m_pFilterWords;		//屏蔽词列表
	char				m_sServerName[128];
	INT_PTR             m_spId;             //spid
};
