#pragma once

class CCustomServerSocket;
class CSQLConenction;
class CSrvConfig;
extern std::string g_ConfigPath;


class CLoggerServerSocket:
	public CCustomServerSocket
{
public:
	typedef CCustomServerSocket ServerInherited ;

	const static int CREATE_TABLE_INTERVAL = 86400000;  //1天调用一次

	//重载父类的函数
	virtual CCustomServerClientSocket* CreateClientSocket(SOCKET nSocket, PSOCKADDR_IN pAddrIn);

	inline CSQLConenction* GetSqlConnection() { return &Db;}

	//发送给逻辑服开启聊天发送
	void SendOpenChatLogFlag(int nServerIndex,int nFlag);

	void AddServerClient(CLoggerClientSocket* pClient,int Flag = 0);

	//发送禁言消息
	void SendShutUpMsg(int nServerIndex,int nType,char* sName,int nTime);
public://构造函数和析构函数
	CLoggerServerSocket(void);
	~CLoggerServerSocket(void);

	
protected:
	//提供用于子类进行覆盖的开启服务的函数，如果函数返回FALSE，则启动将会失败
	//函数将在进入监听后，启动接受连接和连接处理线程前调用
	virtual BOOL DoStartup();
	//提供用于族类进行覆盖的停止服务的函数
	//函数将在停止接受连接以及处理数据线程并关闭监听套接字后调用
	virtual VOID DoStop();


	VOID CreateTables();

	//每次调用
	VOID SingleRun(); 

	BOOL ConnectSQL();
private:
	//设置数据库的相关参数
	VOID SetDbConnection();

	/*
	* 执行插入数据库操作，注意：如果执行出错，需要记录到文件日志里
	* szSql:sql语句
	*/
	BOOL ExecSql(const char* szSql);
public:
	CSrvConfig		Config;
protected:
	CSQLConenction	Db;
	TICKCOUNT       m_dwReconnectSQLTick; //当前的tickcount

	volatile LONG	m_boCreateTableStoped;				//是否停止工作线程
	HANDLE			m_hCTThread;
	UINT64       m_nextCreateTtableTick;            //上一次的调用的tickCount

	CQueueList<CLoggerClientSocket*>	m_LogClientList;//客户端列表
};

/**************  SQL语句  **************************/
// static LPCSTR m_szLoginLogSql = 
// 	_T("insert into log_login_%d%02d%02d(logdate,logid,userid,account,serverindex,loginip,servertype,logindescr,spid,level,yb,onlinetime) values(FROM_UNIXTIME(%d),%d,%u,'%s',%d,'%s',%d,'%s','%s',%d,%d,%u);");
static LPCSTR m_szLoginLogSql = 
	_T("insert into log_login_%d%02d%02d(logdate,logid,userid,account,serverindex,loginip,servertype,logindescr,spid,level,yb,onlinetime) values(now(),%d,%u,'%s',%d,'%s',%d,'%s','%s',%d,%d,%u);");
	
// static LPCSTR m_szLoginLogSql = 
	// _T("insert into log_login_%d%02d%02d(logdate,logid,userid,account,serverindex,loginip,servertype,logindescr,spid,level,yb,onlinetime) values('%s',%d,%u,'%s',%d,'%s',%d,'%s','%s',%d,%d,%u);");

static LPCSTR m_szConsumeLogSql = 
	_T("insert into log_consume_%d%02d%02d(logdate,logid,userid,account,serverindex,charname,moneytype,consumecount,paymentcount,consumedescr,reser1,balance,spid) values(now(),%d,%u,'%s',%d,'%s',%d,%d,%d,'%s',%d,%d,'%s');");

// static LPCSTR m_szConsumeLogSql = 
// 	_T("insert into log_consume_%d%02d%02d(logdate,logid,userid,account,serverindex,charname,moneytype,consumecount,paymentcount,consumedescr,reser1,balance,spid ) values('%s',%d,%u,'%s',%d,'%s',%d,%d,%d,'%s',%d,%d,'%s');");

static LPCSTR m_szOnlineLogSql = 
	_T("insert into log_onlinecount_%d%02d%02d(logdate,serverindex,onlinecount,spid) values(now(),%d,%d,'%s');");

// static LPCSTR m_szOnlineLogSql = 
// 	_T("insert into log_onlinecount_%d%02d%02d(logdate,serverindex,onlinecount,spid) values('%s',%d,%d,'%s');");

static LPCSTR m_szDealLogSql = 
	_T("insert into log_deal_%d%02d%02d(serverindex,logid,miniTime,inActorId,outActorId,item_id,item_num,money_type,money_num,dues,spid) values(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,'%s');");

static LPCSTR m_szItemLogSql = 
	_T("insert into log_item_%d%02d%02d(logdate,logid,userid,account,serverindex,charname,type,itemid,itemcount,consumedescr,spid) values(now(),%d,%u,'%s',%d,'%s',%d,%d,%d,'%s','%s');");

// static LPCSTR m_szItemLogSql = 
// 	_T("insert into log_item_%d%02d%02d(logdate,logid,userid,account,serverindex,charname,type,itemid,itemcount,consumedescr,spid ) values('%s',%d,%u,'%s',%d,'%s',%d,%d,%d,'%s','%s');");

/*
static LPCSTR m_szSuggestLogSql = 
	_T("insert into log_suggest_%d%02d%02d(logdate,account,serverindex,charname,suggesttype,titlestr,sugguststr,state) values(now(),'%s',%d,'%s',%d,'%s','%s',%d);");
*/

static LPCSTR m_szdropLogSql = 
	_T("insert into log_drop_%d%02d%02d(logdate,userid,account,serverindex,charname,monname,scenename,itemname,count,time,posx,posy) values(now(),%u,'%s',%d,'%s','%s','%s','%s',%d,%d,%d,%d);");

static LPCSTR m_szAtvLogSql = 
	_T("insert into log_joinatv_%d%02d%02d(logdate,userid,account,serverindex,charname,atvid,time,nindex) values(now(),%u,'%s',%d,'%s',%d,%d,%d);");

//建表语句

static LPCSTR m_szCreateLogin = "create table  if not exists `log_login_%d%02d%02d` ( "
	"`id` bigint(20) NOT NULL AUTO_INCREMENT,"
	"`logdate` datetime not null,"
	"`logid` int(10)  not null,"
	"`userid` int(10) unsigned default null,"
	"`account` varchar(80) default null,"
	"`serverindex` int(10)  default null,"
	"`loginip` varchar(20) default null,"
	"`servertype` int(10)  default null,"
	"`logindescr` varchar(32) default null,"
	"`level` int(10)  default null,"
	"`yb` int(10)  default null,"
	"`onlinetime` int(10) unsigned  default null,"
	"`spid` varchar(32) default null, "
	"`oldsrvid` int(10) unsigned  DEFAULT '0',"
	"`arg1` int(10) unsigned DEFAULT '0',"
	"`arg2` int(10) unsigned  DEFAULT '0',"
	"`arg3` int(10) unsigned  DEFAULT '0',"
	"`arg4` int(10) unsigned  DEFAULT '0',"
	"PRIMARY KEY (`id`),"
	"KEY `logid` (`logid`),"
	"KEY `userid` (`userid`),"
	"KEY `serverindex` (`serverindex`)"
	") engine=myisam default charset=utf8;";

static LPCSTR m_szCreateOnline = 
	"create table  if not exists `log_onlinecount_%d%02d%02d` ( "
	"`id` bigint(20) NOT NULL AUTO_INCREMENT,"
	"`logdate` datetime not null,"
	"`serverindex` int(10)  default null,"
	"`onlinecount` int(10)  default null,"
	"`spid` varchar(32) default null, "
	"`oldsrvid` int(10) unsigned  DEFAULT '0',"
	"`arg1` int(10) unsigned DEFAULT '0',"
	"`arg2` int(10) unsigned  DEFAULT '0',"
	"`arg3` int(10) unsigned  DEFAULT '0',"
	"`arg4` int(10) unsigned  DEFAULT '0',"
	"PRIMARY KEY (`id`)"
	") engine=myisam default charset=utf8;";

static LPCSTR m_szCreateConsume = 
	"create table  if not exists `log_consume_%d%02d%02d` ("
	"`id` bigint(20) NOT NULL AUTO_INCREMENT,"
	"`logdate` datetime not null,"
	"`logid` int(10)  not null,"
	"`userid` int(10) unsigned default null,"
	"`account` varchar(80) default null,"
	"`serverindex` int(10)  default null,"
	"`charname` varchar(32) default null,"
	"`moneytype` tinyint(32) default 3,"
	"`consumecount` int(10)  default null,"			//兼容已有的后台查询sql,保留该字段
	"`paymentcount` int(10)  default null,"
	"`consumedescr` varchar(32) character set utf8 collate utf8_bin default null,"
	"`reser1` int(10)  default null,"				//兼容已有的后台查询sql,保留该字段不改名
	"`balance` int(10)  default null,"				//余额字段
	"`spid` varchar(32) default null, "
	"`oldsrvid` int(10) unsigned  DEFAULT '0',"
	"`arg1` int(10) unsigned DEFAULT '0',"
	"`arg2` int(10) unsigned  DEFAULT '0',"
	"`arg3` int(10) unsigned  DEFAULT '0',"
	"`arg4` int(10) unsigned  DEFAULT '0',"
	"PRIMARY KEY (`id`),"
	"INDEX `logdate` (`logdate` ASC),"
	"KEY `logid` (`logid`),"
	"KEY `userid` (`userid`),"
	"KEY `serverindex` (`serverindex`)"
	") engine=myisam default charset=utf8;";

static LPCSTR m_szCreateDeal = 
	"create table  if not exists `log_deal_%d%02d%02d` ("
	"`id` bigint(20) NOT NULL AUTO_INCREMENT,"
	"`serverindex` int(10)  default null,"
	"`logid` int(10)  not null,"
	"`miniTime` int(10)  not null,"
	"`inActorId` int(10) unsigned default null,"
	"`outActorId` int(10) unsigned default null,"
	"`item_id` int(40) NOT NULL,"
	"`item_num` int(8) NOT NULL,"
	"`money_type` tinyint(8) NOT NULL,"
	"`money_num` int(8) NOT NULL,"
	"`dues` int(8) NOT NULL,"
	"`spid` varchar(32) default null, "
	"`oldsrvid` int(10) unsigned  DEFAULT '0',"
	"`arg1` int(10) unsigned DEFAULT '0',"
	"`arg2` int(10) unsigned  DEFAULT '0',"
	"`arg3` int(10) unsigned  DEFAULT '0',"
	"`arg4` int(10) unsigned  DEFAULT '0',"
	"PRIMARY KEY (`id`)"
	") engine=myisam default charset=utf8;";

static LPCSTR m_szCreateItem = 
	"create table  if not exists `log_item_%d%02d%02d` ("
	"`id` bigint(20) NOT NULL AUTO_INCREMENT,"
	"`logdate` datetime not null,"
	"`logid` int(10)  not null,"
	"`userid` int(10) unsigned default null,"
	"`account` varchar(80) default null,"
	"`serverindex` int(10)  default null,"
	"`charname` varchar(32) default null,"
	"`type` tinyint(32) default 3,"
	"`itemid` int(10)  default null,"			//兼容已有的后台查询sql,保留该字段
	"`itemcount` int(10)  default null,"
	"`consumedescr` varchar(32) character set utf8 collate utf8_bin default null,"
	"`spid` varchar(32) default null, "
	"`oldsrvid` int(10) unsigned  DEFAULT '0',"
	"`arg1` int(10) unsigned DEFAULT '0',"
	"`arg2` int(10) unsigned  DEFAULT '0',"
	"`arg3` int(10) unsigned  DEFAULT '0',"
	"`arg4` int(10) unsigned  DEFAULT '0',"
	"PRIMARY KEY (`id`),"
	"INDEX `logdate` (`logdate` ASC)"
	") engine=myisam default charset=utf8;";

/*
//玩家的建议，发言,这里
//suggestType 1表示是提交bug,2表示投诉，3表示游戏建议，4表示其他，5表示聊天
//
static LPCSTR m_szCreateSuggest= 
	"create table  if not exists `log_suggest_%d%02d%02d` ("
	"`logdate` datetime not null,"
	"`account` varchar(80) default null,"
	"`serverindex` int(10)  default 0,"
	"`state` int(10)  default 0,"
	"`gmcomment` varchar(128) character set utf8 collate utf8_bin default null,"
	"`charname` varchar(32) default null,"
	"`suggesttype` int(10)  default null,"
	"`titlestr` varchar(128) character set utf8 collate utf8_bin default null,"
	"`sugguststr` varchar(512) character set utf8 collate utf8_bin default null"
	") engine=myisam default charset=utf8;";

*/


static LPCSTR m_szCreateDrop = 
	"create table  if not exists `log_drop_%d%02d%02d` ("
	"`id` bigint(20) NOT NULL AUTO_INCREMENT,"
	"`logdate` datetime not null,"
	"`userid` int(10) unsigned default null,"
	"`account` varchar(80) default null,"
	"`serverindex` int(10)  default null,"
	"`charname` varchar(32) default null,"
	"`monname` varchar(80) default null,"
	"`scenename` varchar(80) default null,"
	"`itemname` varchar(80) default null,"
	"`count` int(10)  default null,"			//
	"`time` int(10)  default null,"
	"`posx` int(10)  default null,"				//
	"`posy` int(10)  default null,"				//
	"`oldsrvid` int(10) unsigned  DEFAULT '0',"
	"`arg1` int(10) unsigned DEFAULT '0',"
	"`arg2` int(10) unsigned  DEFAULT '0',"
	"`arg3` int(10) unsigned  DEFAULT '0',"
	"`arg4` int(10) unsigned  DEFAULT '0',"
	"PRIMARY KEY (`id`),"
	"INDEX `logdate` (`logdate` ASC),"
	"KEY `userid` (`userid`),"
	"KEY `serverindex` (`serverindex`)"
	") engine=myisam default charset=utf8;";


static LPCSTR m_szCreateJoinAtv = 
	"create table  if not exists `log_joinatv_%d%02d%02d` ("
	"`id` bigint(20) NOT NULL AUTO_INCREMENT,"
	"`logdate` datetime not null,"
	"`userid` int(10) unsigned default null,"
	"`account` varchar(80) default null,"
	"`serverindex` int(10)  default null,"
	"`charname` varchar(32) default null,"
	"`atvid` int(10)  default null,"			//
	"`time` int(10)  default null,"
	"`nindex` int(10)  default null,"				//
	"`oldsrvid` int(10) unsigned  DEFAULT '0',"
	"`arg1` int(10) unsigned DEFAULT '0',"
	"`arg2` int(10) unsigned  DEFAULT '0',"
	"`arg3` int(10) unsigned  DEFAULT '0',"
	"`arg4` int(10) unsigned  DEFAULT '0',"
	"PRIMARY KEY (`id`),"
	"INDEX `logdate` (`logdate` ASC),"
	"KEY `userid` (`userid`),"
	"KEY `serverindex` (`serverindex`),"
	"KEY `atvid` (`atvid`)"
	") engine=myisam default charset=utf8;";