#pragma once

class CCustomServerSocket;
class CSQLConenction;
class CSrvConfig;



class CChatLogServerSocket:
	public CCustomServerSocket
{
public:
	typedef CCustomServerSocket ServerInherited ;

	const static int CREATE_TABLE_INTERVAL = 86400000;  //1天调用一次

	//重载父类的函数
	virtual CCustomServerClientSocket* CreateClientSocket(SOCKET nSocket, PSOCKADDR_IN pAddrIn);

	inline CSQLConenction* GetSqlConnection() { return &Db;}

	void AddLogServerClient(CChatLogClientSocket* pClient,int Flag = 0);

	//判断是否还有日志查看器连接这个服务器
	bool IsLogOnServerIndex(int nIndex);

	void SendChatRecordToAll(int nServerIndex,int nChannleID,int nActorId,char* sActorName,char* msg);
	//新服连接后自动连后台获取最新服务器配置，已打开聊天开关
	void SendChatGetNewServerList();
public://构造函数和析构函数
	CChatLogServerSocket();
	~CChatLogServerSocket(void);

	
protected:
	//提供用于子类进行覆盖的开启服务的函数，如果函数返回FALSE，则启动将会失败
	//函数将在进入监听后，启动接受连接和连接处理线程前调用
	virtual BOOL DoStartup();
	//提供用于族类进行覆盖的停止服务的函数
	//函数将在停止接受连接以及处理数据线程并关闭监听套接字后调用
	virtual VOID DoStop();


	//每次调用
	VOID SingleRun(); 

	
private:
	//设置数据库的相关参数
	//VOID SetDbConnection();

	/*
	* 执行插入数据库操作，注意：如果执行出错，需要记录到文件日志里
	* szSql:sql语句
	*/
	BOOL ExecSql(const char* szSql);
public:
	CSrvConfig		Config;
protected:
	CSQLConenction	Db;
	
	volatile LONG	m_boCreateTableStoped;				//是否停止工作线程
	HANDLE			m_hCTThread;
	UINT64       m_nextCreateTtableTick;            //上一次的调用的tickCount

	CQueueList<CChatLogClientSocket*>	m_LogClientList;//客户端列表
};


