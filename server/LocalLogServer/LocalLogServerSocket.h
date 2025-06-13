#pragma once

class CCustomServerSocket;
class CSQLConenction;
class CSrvConfig;



class CLocalLogServerSocket:
	public CCustomServerSocket
{
public:
	typedef CCustomServerSocket ServerInherited ;

	const static int CREATE_TABLE_INTERVAL = 86400000;  //1天调用一次

	//重载父类的函数
	virtual CCustomServerClientSocket* CreateClientSocket(SOCKET nSocket, PSOCKADDR_IN pAddrIn);

	inline CSQLConenction* GetSqlConnection() { return &Db;}

public://构造函数和析构函数
	CLocalLogServerSocket(char *pszFileName);
	~CLocalLogServerSocket(void);

	
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
	
	volatile LONG	m_boCreateTableStoped;				//是否停止工作线程
	HANDLE			m_hCTThread;
	TICKCOUNT       m_nextCreateTtableTick;            //上一次的调用的tickCount
};

/**************  SQL语句  **************************/
static LPCSTR m_szCommonLogSql = 
	_T("insert into log_common_%d_%d_%d%02d%02d(logdate,logid,userid,account,accountname,serverindex,para0,para1,para2,shortstr0,shortstr1,midstr0,midstr1,midstr2,longstr0,longstr1,longstr2,spid) values(now(),%d,%u,'%s','%s',%d,%d,%d,%d,'%s','%s','%s','%s','%s','%s','%s','%s','%s');");

//删表语句
static LPCSTR m_szDeleteCommonSql =
	_T("drop table if exists `log_common_%d_%d_%d%02d%02d`");

//统计的
static LPCSTR m_szCommonCountSql =
	_T("select `logid`,count(1) from `log_common_%d_%d_%d%02d%02d` group by logid desc");


//资产日志表
static LPCSTR m_szCreateCurrency =
	"CREATE TABLE IF NOT EXISTS `currency_%d%02d` ("
	"`time` int(8) NOT NULL,"					// 操作时间的时间戳
	"`log_id` int(64) NOT NULL,"				// 日志ID（消费点ID）
	"`log_type` tinyint(8) NOT NULL,"			// 日志类型（1产出 2消费）
	"`currency_type` tinyint(8) NOT NULL,"		// 货币类型（1金币 2绑金 3银两 4元宝 ）
	"`amount` int(8) NOT NULL,"					// 产出/消费货币数量
	"`balance` int(8) NOT NULL,"				// 当前余额
	"`is_shop` tinyint(8) NOT NULL DEFAULT '2',"// 是否商城消费（1是 2否）
	"`accountname` varchar(80) NOT NULL,"		// 账号
	"`actorid` int(10) NOT NULL,"				// 角色id
	"`actorname` varchar(32) NOT NULL,"			// 角色名
	"`date` datetime NOT NULL,"					// 日期
	"`srvid` int(8) NOT NULL DEFAULT '0',"		// 服务器id
	"`oldsrvid` int(10) unsigned  DEFAULT '0',"
	"`arg1` int(10) unsigned DEFAULT '0',"
	"`arg2` int(10) unsigned  DEFAULT '0',"
	"`arg3` int(10) unsigned  DEFAULT '0',"
	"`arg4` int(10) unsigned  DEFAULT '0',"
	"KEY `time` (`time`)"
	") ENGINE=InnoDB DEFAULT CHARSET=utf8;";

static LPCSTR m_szDeleteCurrency =
	_T("DROP TABLE IF EXISTS `currency_%d%02d`");

// static LPCSTR m_szInsertCurrency = 
// 	_T("insert into currency_%d%02d(time,log_id,log_type,currency_type,amount,balance,is_shop,accountname,actorid,actorname,date,srvid,oldsrvid) values(unix_timestamp(),%d,%d,%d,%lld,%lld,%d,'%s',%u,'%s',NOW(),%d,%d);");

static LPCSTR m_szInsertCurrency = 
	_T("insert into currency_%d%02d(time,log_id,log_type,currency_type,amount,balance,is_shop,accountname,actorid,actorname,date,srvid) values(unix_timestamp(),%d,%d,%d,%lld,%lld,%d,'%s',%u,'%s',NOW(),%d);");


//商城日志表
static LPCSTR m_szCreateShop =
	"CREATE TABLE IF NOT EXISTS `shop_%d%02d` ("
	"`time` int(8) NOT NULL,"					// 操作时间的时间戳
	"`shop_type` int(8) NOT NULL DEFAULT '2',"	// 商城类型
	"`item_id` int(40) NOT NULL,"				// 道具ID
	"`item_type` int(8) NOT NULL,"				// 道具类型ID
	"`item_num` int(8) NOT NULL,"				// 购买道具数
	"`currency_type` tinyint(8) NOT NULL,"		// 货币类型（1金币 2绑金 3银两 4元宝 ）
	"`consume_num` int(8) NOT NULL,"			// 消耗货币总额
	"`accountname` varchar(80) NOT NULL,"		// 账号
	"`actorid` int(10) NOT NULL,"				// 角色id
	"`actorname` varchar(32) NOT NULL,"			// 角色名
	"`date` datetime NOT NULL,"					// 日期
	"`srvid` int(8) NOT NULL DEFAULT '0',"		// 服务器id
	"`oldsrvid` int(10) unsigned  DEFAULT '0',"
	"`arg1` int(10) unsigned DEFAULT '0',"
	"`arg2` int(10) unsigned  DEFAULT '0',"
	"`arg3` int(10) unsigned  DEFAULT '0',"
	"`arg4` int(10) unsigned  DEFAULT '0',"
	"KEY `time` (`time`)"
	") ENGINE=InnoDB DEFAULT CHARSET=utf8;";

static LPCSTR m_szDeleteShop =
	_T("DROP TABLE IF EXISTS `shop_%d%02d`");

// static LPCSTR m_szInsertShop = 
// 	_T("insert into shop_%d%02d(time,shop_type,item_id,item_type,item_num,currency_type,consume_num,accountname,actorid,actorname,date,srvid,oldsrvid) values(unix_timestamp(),%d,%d,%d,%d,%d,%lld,'%s',%u,'%s',NOW(),%d,%d);");

static LPCSTR m_szInsertShop = 
	_T("insert into shop_%d%02d(time,shop_type,item_id,item_type,item_num,currency_type,consume_num,accountname,actorid,actorname,date,srvid) values(unix_timestamp(),%d,%d,%d,%d,%d,%lld,'%s',%u,'%s',NOW(),%d);");

//道具日志表
static LPCSTR m_szCreateItem =
	"CREATE TABLE IF NOT EXISTS `item_%d%02d` ("
	"`time` int(8) NOT NULL,"					// 操作时间的时间戳
	"`item_id` int(40) NOT NULL,"				// 道具ID
	"`item_type` int(8) NOT NULL,"				// 道具类型ID
	"`item_num` int(8) NOT NULL,"				// 购买道具数
	"`oper_type` int(8) NOT NULL DEFAULT '1',"	// 操作类型（1获得，2消耗）
	"`reason` varchar(64) NOT NULL,"			// 获得/消耗道具的原因
	"`accountname` varchar(80) NOT NULL,"		// 账号
	"`actorid` int(10) NOT NULL,"				// 角色id
	"`actorname` varchar(32) NOT NULL,"			// 角色名
	"`date` datetime NOT NULL,"					// 日期
	"`srvid` int(8) NOT NULL DEFAULT '0',"		// 服务器id
	"`oldsrvid` int(10) unsigned  DEFAULT '0',"
	"`arg1` int(10) unsigned DEFAULT '0',"
	"`arg2` int(10) unsigned  DEFAULT '0',"
	"`arg3` int(10) unsigned  DEFAULT '0',"
	"`arg4` int(10) unsigned  DEFAULT '0',"
	"KEY `time` (`time`)"
	") ENGINE=InnoDB DEFAULT CHARSET=utf8;";

static LPCSTR m_szDeleteItem =
	_T("DROP TABLE IF EXISTS `item_%d%02d`");

// static LPCSTR m_szInsertItem = 
// 	_T("insert into item_%d%02d(time,item_id,item_type,item_num,oper_type,reason,accountname,actorid,actorname,date,srvid,oldsrvid) values(unix_timestamp(),%d,%d,%d,%d,'%s','%s',%u,'%s',NOW(),%d,%d);");


static LPCSTR m_szInsertItem = 
	_T("insert into item_%d%02d(time,item_id,item_type,item_num,oper_type,reason,accountname,actorid,actorname,date,srvid) values(unix_timestamp(),%d,%d,%d,%d,'%s','%s',%u,'%s',NOW(),%d);");

//任务日志表
static LPCSTR m_szCreateTask =
	"CREATE TABLE IF NOT EXISTS `task_%d%02d` ("
	"`time` int(8) NOT NULL,"					// 接收/完成时间戳
	"`task_id` int(64) NOT NULL,"				// 任务ID
  	"`task_type` tinyint(8) NOT NULL DEFAULT '1',"// 任务类型（1主线 2其他）
  	"`status` tinyint(8) NOT NULL,"				// 状态（1接收，2完成）
  	"`accountname` varchar(80) NOT NULL,"		// 账号
  	"`actorid` int(10) NOT NULL,"				// 角色id
  	"`actorname` varchar(32) NOT NULL,"			// 角色名
  	"`date` datetime NOT NULL,"					// 日期
	"`srvid` int(8) NOT NULL DEFAULT '0',"		// 服务器id
	"`oldsrvid` int(10) unsigned  DEFAULT '0',"
	"`arg1` int(10) unsigned DEFAULT '0',"
	"`arg2` int(10) unsigned  DEFAULT '0',"
	"`arg3` int(10) unsigned  DEFAULT '0',"
	"`arg4` int(10) unsigned  DEFAULT '0',"
	"KEY `time` (`time`)"
	") ENGINE=MyISAM DEFAULT CHARSET=utf8;";

static LPCSTR m_szDeleteTask =
	_T("DROP TABLE IF EXISTS `task_%d%02d`");

// static LPCSTR m_szInsertTask = 
// 	_T("insert into task_%d%02d(time,task_id,task_type,status,accountname,actorid,actorname,date,srvid,oldsrvid) values(unix_timestamp(),%d,%d,%d,'%s',%u,'%s',NOW(),%d,%d);");

static LPCSTR m_szInsertTask = 
	_T("insert into task_%d%02d(time,task_id,task_type,status,accountname,actorid,actorname,date,srvid) values(unix_timestamp(),%d,%d,%d,'%s',%u,'%s',NOW(),%d);");

//活动日志表
static LPCSTR m_szCreateActivity =
	"CREATE TABLE IF NOT EXISTS `activity_%d%02d` ("
	"`time` int(8) NOT NULL,"					// 接收/完成时间戳
	"`atv_id` int(32) NOT NULL,"				// 活动ID
  	"`atv_type` int(8) NOT NULL DEFAULT '1',"	// 活动类型
  	"`status` tinyint(8) NOT NULL,"				// 状态（1接收，2完成）
  	"`accountname` varchar(80) NOT NULL,"		// 账号
  	"`actorid` int(10) NOT NULL,"				// 角色id
  	"`actorname` varchar(32) NOT NULL,"			// 角色名
  	"`date` datetime NOT NULL,"					// 日期
	"`srvid` int(8) NOT NULL DEFAULT '0',"		// 服务器id
	"`oldsrvid` int(10) unsigned  DEFAULT '0',"
	"`arg1` int(10) unsigned DEFAULT '0',"
	"`arg2` int(10) unsigned  DEFAULT '0',"
	"`arg3` int(10) unsigned  DEFAULT '0',"
	"`arg4` int(10) unsigned  DEFAULT '0',"
	"KEY `time` (`time`)"
	") ENGINE=MyISAM DEFAULT CHARSET=utf8;";

static LPCSTR m_szDeleteActivity =
	_T("DROP TABLE IF EXISTS `activity_%d%02d`");

// static LPCSTR m_szInsertActivity = 
// 	_T("insert into activity_%d%02d(time,atv_id,atv_type,status,accountname,actorid,actorname,date,srvid,oldsrvid) values(unix_timestamp(),%d,%d,%d,'%s',%u,'%s',NOW(),%d,%d);");

static LPCSTR m_szInsertActivity = 
	_T("insert into activity_%d%02d(time,atv_id,atv_type,status,accountname,actorid,actorname,date,srvid) values(unix_timestamp(),%d,%d,%d,'%s',%u,'%s',NOW(),%d);");



static LPCSTR m_szCreateRechargeAll =
	"CREATE TABLE IF NOT EXISTS `recharge_all` ("
	"`id` bigint(11) NOT NULL AUTO_INCREMENT,"
	"`order_id` varchar(64) COLLATE utf8_bin DEFAULT '' COMMENT '订单号',"
	"`role_id` varchar(32) COLLATE utf8_bin DEFAULT '' COMMENT '角色id',"
	"`rolename` varchar(32) COLLATE utf8_bin DEFAULT '' COMMENT '角色名',"
	"`uid` varchar(32) COLLATE utf8_bin DEFAULT '' COMMENT '玩家游戏账号',"
	"`charge_time` int(11) DEFAULT '0' COMMENT '时间戳(充值时间)',"
	"`charge_currency` int(11) DEFAULT '0' COMMENT '游戏币总数(物品总数)',"
	"`money` int(11) DEFAULT '0' COMMENT '充值的人民币(分)',"
	"`currency_type` tinyint(4) DEFAULT '1' COMMENT '币种类型',"
	"`level` int(11) DEFAULT '0' COMMENT '充值等级',"
	"`channel` varchar(32) COLLATE utf8_bin DEFAULT '' COMMENT '充值渠道名称',"
	"`type` int(11) COLLATE utf8_bin DEFAULT '1' COMMENT '充值类型(1:正常充值,2:充值测试,3:发放福利)',"
	"`status` tinyint(4) DEFAULT '1' COMMENT '1:成功,2:角色不存在,3:失败',"
	"`last_update_time` int(11) DEFAULT '0' COMMENT '默认值等于charge_time',"
	"`oldsrvid` int(10) unsigned  DEFAULT '0',"
	"`arg1` int(10) unsigned DEFAULT '0',"
	"`arg2` int(10) unsigned  DEFAULT '0',"
	"`arg3` int(10) unsigned  DEFAULT '0',"
	"`arg4` int(10) unsigned  DEFAULT '0',"
	"PRIMARY KEY (`id`),"
	"KEY `last_update_time` (`last_update_time`),"
	"KEY `order_id` (`order_id`),"
	"KEY `role_id` (`role_id`,`rolename`,`uid`),"
	"KEY `channel` (`channel`)"
	") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;";

static LPCSTR m_szCreateRoleAll =
	"CREATE TABLE IF NOT EXISTS `role_all` ("
	"`id` bigint(11) NOT NULL AUTO_INCREMENT,"
	"`type` int(11) DEFAULT '0' COMMENT '上报类型',"
	"`uid` varchar(32) COLLATE utf8_bin DEFAULT '' COMMENT '用户帐号ID',"
	"`role_id` varchar(32) COLLATE utf8_bin DEFAULT '' COMMENT '角色ID',"
	"`log_time` int(11) DEFAULT '0' COMMENT '上报时间戳',"
	"`ip` varchar(32) COLLATE utf8_bin DEFAULT '' COMMENT '请求ip地址',"
	"PRIMARY KEY (`id`),"
	"KEY `type` (`type`),"
	"KEY `log_time` (`log_time`),"
	"KEY `uid` (`uid`,`role_id`)"
	") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;";

