#pragma once

class CCustomServerSocket;
class CSQLConenction;
class CSrvConfig;
 

class CBackServerSocket:
	public CCustomServerSocket
{
	  
public:
	typedef CCustomServerSocket ServerInherited ;

	//重载父类的函数
	virtual CCustomServerClientSocket* CreateClientSocket(SOCKET nSocket, PSOCKADDR_IN pAddrIn);

	inline CSQLConenction* GetSqlConnection() { return &Db;}


	//删除服务器
	void DelClient(CBackClientSocket* pClient);

public://构造函数和析构函数
	CBackServerSocket(char *pszFileName);
	~CBackServerSocket(void);

	CBackHttpServer& GetHttpServer(){ return m_HttpServer; };
	
protected:

	//启动创建表
	VOID CreateTables();

	//每次调用
	VOID SingleRun(); 

	
private:
	//设置数据库的相关参数
	VOID SetDbConnection();

	/*
	* 执行插入数据库操作，注意：如果执行出错，需要记录到文件日志里
	* szSql:sql语句
	*/
	BOOL ExecSql(const char* szSql);

	void HandleHttpCommand(SocketInfo& socketInfo);
public:
	CSrvConfig		Config;
protected:
	CSQLConenction	Db;
	volatile LONG	m_boCreateTableStoped;				//是否停止工作线程
	HANDLE			m_hCTThread;
	TICKCOUNT       m_nextGetNoticeTick;				//每30分钟请求公告
	CQueueList<CBackClientSocket*>	m_BackClientList;		//客户端列表
	CBackHttpServer	m_HttpServer;
	//上一次的调用的tickCount
	typedef struct tagCommand
	{
		int id;
		int serverindex;
		int operindex;
		char strcom[512];
		char username[32];
		char time[24];
	}COMMAND,*PCOMMAND;

	vector<COMMAND> vcCommandCopy;
	
};

//--后台操作历史命令表
static LPCSTR m_szCommandHistory = "create table  if not exists `commandhistory` ( "
	"`id` int(10)  not null," //命令的序号id，是自增的
	"`serverindex` int(10)  default null,"  //要发送到的服务器id  如果是发所有服务器写0就可以
	"`operindex` int(10)  default null,"    //操作码
	"`strcom` varchar(1024) default null,"   //命令类容
	"`nstate` int(10)  default null,"       //状态
	"`username` varchar(32) default null,"  //用户名称
	"`logdate` datetime NOT NULL,"          //操作时间
	"`reser1` int(10)  default null"        //预留
	") engine=myisam default charset=utf8;";

static LPCSTR m_szCreateNotice = "create table  if not exists `notice` ( "
	"`id` int(10)  not null ," //公告id
	"`serverindex` int(10)  default null,"  //服务器id
	"`operindex` int(10)  default null,"    //操作码
	"`strmsg` varchar(1024) default null,"   //公告类容
	"`reser` int(10)  default null,"        //sid
	"`starttime` int(10)  default 0,"     //开始时间戳
	"`endtime` int(10)  default 0,"        //结束时间戳
	"`mins` int(10)  default 0,"        //时间间隔(分钟)
	"`pos` int(10)  default 0"        //位置
	") engine=myisam default charset=utf8;";


static LPCSTR  szSQLSP_LoadCommand = "select id,serverindex,operindex,strcom ,username ,logdate,reser1 from command";		//查询命令表
static LPCSTR  szSQLSP_LoadCommandAll = _T("select id,serverindex,operindex,strcom,username,logdate from command");		//查询命令表
static LPCSTR  szSQLSP_AddCommandHistory="insert into commandhistory(id,serverindex,operindex,strcom,nstate,username,reser1,logdate) values(%d,%d,%d,\"%s\",%d,\"%s\",%d,now())";//插入数据到历史表
//static LPCSTR  szSQLSP_UpdateCommandHistory="update commandhistory set nstate = &d where id = %d";//更新历史表
static LPCSTR  szSQLSP_GetHistory=_T("select serverindex,operindex,strcom ,username,logdate from commandhistory where id=%d");

static LPCSTR  szSQLSP_AddCommandCopy="insert into commandcopy(id, serverindex,operindex,strcom,nstate,username,logdate) values(%d,%d,%d,\"%s\",0,\"%s\",\"%s\")";//插入数据到copy表中



static LPCSTR  szSQLSP_DelServerInfo = "delete from serverstate where serverindex=%d";		//删除服务器的信息
static LPCSTR  szSQLSP_AddServerInfo = "insert into serverstate(serverindex,strstate) values(%d,\"%s\")";		//插入服务器的信息

static LPCSTR  szSQLSP_DropNoticTable = "drop table  if exists notice";		//删除公告类容
static LPCSTR  szSQLSP_DelServerNotice = "delete from notice where serverindex=%d and reser=%d";		//删除公告类容
static LPCSTR  szSQLSP_AddServerNotice = "insert into notice(id,serverindex,operindex,strmsg,reser,starttime,endtime,mins,pos) values(%d,%d,%d,\"%s\",%d,%u,%u,%d,%d)";		//插入公告类容
