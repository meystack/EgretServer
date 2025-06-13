#include "StdAfx.h"

CLocalLogServerSocket::CLocalLogServerSocket(char *pszFileName)
{
	Config.ReadConfig(pszFileName);

	// 设置服务器的基本属性
	SetServiceName(Config.SrvConf.szServiceName);
	SetServiceHost(Config.SrvConf.szAddr);
	SetServicePort(Config.SrvConf.nPort);
	
	SetDbConnection();

	m_boCreateTableStoped = TRUE;
	m_nextCreateTtableTick =0;
}


CLocalLogServerSocket::~CLocalLogServerSocket(void)
{
}

CCustomServerClientSocket* CLocalLogServerSocket::CreateClientSocket(SOCKET nSocket, PSOCKADDR_IN pAddrIn)
{
	if (pAddrIn == NULL)
	{
		return NULL;
	}
	OutputMsg( rmTip, "接受客户端连接!" );
	CLocalLogClientSocket* pClientSocket = new CLocalLogClientSocket(this,nSocket,pAddrIn);
	return pClientSocket;
}

BOOL CLocalLogServerSocket::ExecSql(const char* szSql)
{
	return TRUE;
}

VOID CLocalLogServerSocket::SetDbConnection()
{
	Db.SetServerHost(Config.DbConf.szHost);
	Db.SetServerPort(Config.DbConf.nPort);
	Db.SetDataBaseName(Config.DbConf.szDbName);
	Db.SetUserName(Config.DbConf.szUser);
	Db.SetPassWord(Config.DbConf.szPassWord);//这个要解密
	//多个客户端连接，数据库操作类要加锁
	//Db.SetMultiThread(TRUE);
	Db.SetConnectionFlags(CLIENT_FOUND_ROWS | CLIENT_MULTI_RESULTS);
	BOOL boConnected = Db.Connect();

	if (boConnected)
	{
		if(Config.DbConf.m_bUtf8)
		{
			//mysql_options(sql.GetMySql(), MYSQL_SET_CHARSET_NAME, "utf8");
			mysql_set_character_set(Db.GetMySql(),"utf8");
			OutputMsg( rmTip, _T("设置字符编码为utf8"));
		}

		OutputMsg( rmTip, _T("连接数据库成功"));
	}
	else
	{
		OutputMsg( rmError, _T("连接数据库失败"));
	}
}

BOOL CLocalLogServerSocket::DoStartup()
{
	/*
	if ( TRUE == InterlockedCompareExchange(&m_boCreateTableStoped, FALSE, TRUE) )
	{	
		m_hCTThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CreateTablesThreadRoutine, this, 0, NULL);
		if ( !m_hCTThread )
		{
			OutputError( GetLastError(), _T("创建检查数据表线程失败！") );
			return FALSE;
		}
		OutputMsg( rmTip, _T("创建检查数据表线程成功！"));
	}
	*/
	return TRUE;
}

VOID CLocalLogServerSocket::DoStop()
{
	/*
	if ( FALSE == InterlockedCompareExchange(&m_boCreateTableStoped, TRUE, FALSE) )
	{
		CloseThread(m_hCTThread);
	}
	*/
}

//每次调用
VOID CLocalLogServerSocket::SingleRun()
{
	ServerInherited::SingleRun();
}
