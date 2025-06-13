#ifndef _DBENGINE_H_
#define _DBENGINE_H_

#include "CFilter.h"

class CDBGateManager;
class CDBSessionClient;
class LogSender;
class CNameSyncClient;
class CDBDataServer;
class CDBCenterClient;
class CEsqlManager;


class CDBServer
{
public:
	CDBServer();
	~CDBServer();

	inline LPCSTR getServerName(){ return m_sServerName; };
	
	//inline CDBGateManager* getGateManager(){ return m_pGateManager; };
	//inline CDBSessionClient* getSessionClient(){ return m_pSSClient; };
	//inline LogSender* getLogClient(){ return m_pLogClient; };

	inline CNameSyncClient* getNameSyncClient(){ return m_pNameSyncClient; }
	//inline CDBCenterClient* getDBCenterClient() { return m_pDBCenterClient; }
	inline CDBDataServer* getDataServer(){ return m_pDataServer; }
	inline BOOL getSelectCountryEnabledOnCreateFirstChar(){ return m_boFirstCharEnableSelCountry; }
	inline INT	getServerIndex() { return m_nServerIndex;}
	BOOL	getGameReady(int nSrvIdx);
	//配置数据库连接对象
	VOID SetupSQLConnection(CSQLConenction *lpConnection);
	//将名称字符串中首个字符之后的字母全部转换为小写
	LPCSTR LowerCaseNameStr(LPSTR sNameStr,INT_PTR nSize);
	//加载游戏网关路由表，返回-1表示发生错误，否则表示加载的路由信息数量
	INT LoadGameServerRoute();
	// 输出游戏路由信息
	void TraceGameServerRoute();
	//为网关用户选择角色网关路由地址，选择路由地址成功则返回TRUE并将地址保存在lpGateHost中（会添加终止字符），将端口保存在lpGatePort中
	BOOL SelectGameServerRoute(const int nServerIndex, LPSTR lpGateHost, const SIZE_T dwHostLen, PINT_PTR lpGatePort);

	//获取会话数据
	inline BOOL GetSessionData(const int nSessionId, OUT jxSrvDef::PGLOBALSESSIONOPENDATA pSessionData)
	{
		//return m_pSSClient->GetSessionData(nSessionId, pSessionData);
		return FALSE;
	}
	//依据会话ID关闭连接到网关中的用户
	
	inline VOID PostCloseGateUserBySessionId(const int nSessionId)
	{
		//m_pGateManager->PostCloseGateUserByGlobalSessionId(nSessionId);
	}
	//异步检查网关中是否存在会话为特定值的用户
	inline VOID PostQueryUserExistsBySessionId(const int nSessionId)
	{
		//m_pGateManager->PostQueryGateUserExistsBySessionId(nSessionId);
	}
	//网关向DBSSClient返回对用户是否存在的查询的消息
	
	inline VOID PostQueryUserExistsBySessionIdResult(const int nSessionId, const BOOL boExists)
	{
		//m_pSSClient->PostQuerySessionExistsResult(nSessionId, boExists);
	}
	//网关向DBSSClient投递改变会话状态的消息
	inline VOID PostChangeSessionState(const int nSessionId, const jxSrvDef::GSSTATE nState)
	{
		//m_pSSClient->PostChangeSessionState(nSessionId, nState);
	}
	//当网关用户关闭的时候，向会话模块投递用户关闭的消息，以便决定是否应该删除会话
	inline VOID PostSSClientUserClosed(const int nSessionId)
	{
		//m_pSSClient->PostCloseSession(nSessionId);
	}
	//向特定的数据服务器投递打开角色加载认证数据
	inline VOID PostDBServerOpenCharLoad(const int nServerIndex, UINT64 nSessionId, UINT64 nCharId,UINT64 nCurrentTick)
	{
		m_pDataServer->PostOpenCharLoad(nServerIndex, nSessionId, nCharId,nCurrentTick);
	}

	//设置服务器名称
	VOID SetServerName(LPCSTR sSrvName);
	//设置网关服务的地址和端口
	VOID SetGateServiceAddress(LPCTSTR sHost, const INT_PTR nBindPort);
	//设置会话服务器地址和端口，设置会在会话客户端下次连接的时候生效
	VOID SetSessionServerAddress(LPCTSTR sHost, const INT_PTR nPort);
	//设置日志服务器地址和端口
	VOID SetLogServerAddress(LPCTSTR sHost, const INT_PTR nPort);
	// 设置DBCenter服务器地址和断开
	VOID SetDBCenterAddress(LPCTSTR sHost, const INT_PTR nPort);
	//设置数据库连接配置
	VOID SetSQLConfig(LPCSTR sHost, const INT_PTR nPort, LPCSTR sDBName, LPCSTR sDBUser, LPCSTR sUserPassword, bool boUtf8);
	//设置是否允许在创建第一个角色的时候选择国家
	VOID SetSelectCountryEnabledOnCreateFirstChar(const BOOL boEnabled);
	//设置名称服务器地址和端口，设置会在名称客户端下次连接的时候生效
	VOID SetNameSyncServerAddress(LPCTSTR sHost, const INT_PTR nPort);
	//设置数据服务的绑定地址和端口
	VOID SetDataServiceAddress(LPCTSTR sBindHost, const INT_PTR nBindPort);

	VOID SetServerIndex(INT nServerIndex);
	//启动数据服务器的服务
	BOOL Startup();
	//停止数据服务器的服务
	VOID Shutdown();
	//读入关键字敏感词
	BOOL LoadFilterNames();
	//增加关键字敏感词
	void AddFilterName(char* sName);
	//获取utf8格式的字符串的长度，1个汉字和1个英文字符都是1
	static INT_PTR GetStrLenUtf8(LPCTSTR sName);

	void CheckNameFilter();
	//检查名字里是否有敏感词，如果有，返回false
	BOOL CheckNameStr(LPCSTR sNameStr);

	/* 
	* Comments:加载文件
	* Param LPCSTR sPrifixList:
	* Param LPCSTR sConnectList:
	* Param LPCSTR sStuffixList:
	* @Return bool:
	*/
	bool LoadNameGroup(LPCSTR sPrifixList, /*LPCSTR sConnectList,*/ LPCSTR sStuffixList);

	//获取一个随机生成的名字
	INT_PTR GetRandomName(LPCTSTR& sName,INT_PTR nSex);

	bool GetRandomGroupName(char * sName, int nSize);

	//判断名称字符串中是否含有非法字符
	bool hasFilterWordsInclude(LPCSTR sNameStr);
	

	// 更新逻辑网关用户信息
	void UpdateLogicGateUserInfo(CDataPacketReader& packet);

	//设置数据服务器启动标志
	void SetDbStartFlag(bool bFlag) {m_bStartDbServer = bFlag;}

	//返回数据服务器关闭的标志
	bool GetDbStartFlag() {return m_bStartDbServer;}

	//运行esql文件
	void RunEsqlFile(char* sToolPath,char* sDbName);

	//追踪内存使用情况
	void Trace();

	bool IsUtf8() 
	{
		return m_boUtf8;	
	}
private:
	//释放游戏网关路由数据
	VOID FreeGameServerRoute();
	INT_PTR SelecteGameGateRouteIndex(jxSrvDef::PGAMESERVERROUTE pRoute);
		
private:
	CHAR			m_sServerName[128];			//服务器名称
	CHAR			m_sSQLHost[128];			//数据库地址
	INT_PTR			m_nSQLPort;					//数据库端口
	CHAR			m_sSQLDBName[128];			//数据库名称
	CHAR			m_sSQLUser[128];			//数据库用户名
	CHAR			m_sSQLPassword[128];		//数据库用户密码
	TCHAR			m_sNameSyncServerHost[128];	//名称服务器地址(如果地址不为空字符串，则通过名称服务器申请角色和帮会ID)
	INT_PTR			m_nNameSyncServerPort;		//名称服务器端口
	BOOL			m_boFirstCharEnableSelCountry;//账号创建第一个角色时是否允许选择国家，默认值为FALSE
	bool			m_boUtf8;					//使用utf8数据编码

	CFilter         m_filter;                      //


	//CDBGateManager		*m_pGateManager;	//网关管理器
	//CDBSessionClient	*m_pSSClient;		//会话客户端
	CNameSyncClient		*m_pNameSyncClient;	//名称客户端
	CDBDataServer		*m_pDataServer;		//数据服务器
	//LogSender			*m_pLogClient;		//日志客户端
	//CDBCenterClient		*m_pDBCenterClient;	// DBCenter客户端
	CCSLock				m_GameServerRouteListLock;//游戏网关路由表锁
	CLockList<jxSrvDef::PGAMESERVERROUTE>	m_GameServerRouteList;//游戏网关路由表

	CEsqlManager		m_pEsqlMgr;					//运行esql文件
	// zac
	int						m_nServerIndex;
	//PFT_MBCS_CHAR			m_pFilterWords;		//屏蔽词列表
	CCSLock					m_NameFilterLock;

	CCSLock							m_RandNameLock;	

	CLoadString	*					m_pListPrefix;
	//CLoadString	*					m_pListConnect;
	CLoadString	*					m_pListStuffix;
	bool							m_bStartDbServer;							
public:
	static CDBServer				*s_pDBEngine;
};

inline CDBServer* GetGlobalDBEngine()
{
	return CDBServer::s_pDBEngine;
}

#endif

