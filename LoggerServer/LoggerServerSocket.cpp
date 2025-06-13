#include "stdafx.h"

CLoggerServerSocket::CLoggerServerSocket(void)
{
	Config.ReadConfig(g_ConfigPath.c_str());

	// 设置服务器的基本属性
	SetServiceName(Config.SrvConf.szServiceName);
	SetServiceHost(Config.SrvConf.szAddr);
	SetServicePort(Config.SrvConf.nPort);
	
	SetDbConnection();

	m_boCreateTableStoped = TRUE;
	m_nextCreateTtableTick =0;
	m_dwReconnectSQLTick =0;
}


CLoggerServerSocket::~CLoggerServerSocket(void)
{
}

CCustomServerClientSocket* CLoggerServerSocket::CreateClientSocket(SOCKET nSocket, PSOCKADDR_IN pAddrIn)
{
	if (pAddrIn == NULL)
	{
		return NULL;
	}
	OutputMsg( rmTip, "接受客户端连接!" );
	CLoggerClientSocket* pClientSocket = new CLoggerClientSocket(this,nSocket,pAddrIn);

	return pClientSocket;
}

BOOL CLoggerServerSocket::ExecSql(const char* szSql)
{
	return TRUE;
}

VOID CLoggerServerSocket::SetDbConnection()
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

BOOL CLoggerServerSocket::DoStartup()
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

VOID CLoggerServerSocket::DoStop()
{
	/*
	if ( FALSE == InterlockedCompareExchange(&m_boCreateTableStoped, TRUE, FALSE) )
	{
		CloseThread(m_hCTThread);
	}
	*/
}

//每次调用
VOID CLoggerServerSocket::SingleRun()
{
	ConnectSQL();
	CreateTables();
	ServerInherited::SingleRun();
	
}

BOOL CLoggerServerSocket::ConnectSQL()
{
	if ( !Db.Connected() )
	{
		TICKCOUNT nTick = _getTickCount();
		if ( nTick >= m_dwReconnectSQLTick )
		{
			SetDbConnection();

			//如果连接SQL失败则将在5秒后重试
			m_dwReconnectSQLTick =nTick + 3 * 1000;
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	return TRUE;
}

 VOID CLoggerServerSocket::CreateTables()
{

	UINT64 nCurrentTick = _getTickCount();
	
	if(nCurrentTick < m_nextCreateTtableTick)
	{
		return;
	}

	int nCount = 0;
	const int TOTAL = 3600;
	
	SYSTEMTIME Systime;
	GetLocalTime( &Systime );
	WORD nDay = Systime.wDay;
	WORD nMonth = Systime.wMonth;
	WORD nYear = Systime.wYear;

	OutputMsg(rmNormal,"create table year=%d,month=%d day= %d",(int)nYear,(int)nMonth,(int)nDay);

	bool isSucceed= true; //是否都执行成功
#ifdef WIN32
	__try
#endif

	{
		
		
		//生成未来30天的表格
		for(int i = 0; i < 30; i++)
		{
			char date[50];
			sprintf(date,"%d%d%d",nYear,nMonth,nDay);
			if (Db.Exec(m_szCreateLogin,nYear,nMonth,nDay) != 0)
			{
				//失败
				OutputMsg(rmError,_T("建表失败！sql:%s"),m_szCreateLogin);
				isSucceed =false;
			}
			else
			{
				Db.ResetQuery();
			}

			if (Db.Exec(m_szCreateOnline,nYear,nMonth,nDay) != 0)
			{
				//失败
				OutputMsg(rmError,_T("建表失败！sql: m_szCreateOnline"));
				isSucceed =false;
			}
			else
			{
				Db.ResetQuery();
			}
			if (Db.Exec(m_szCreateConsume,nYear,nMonth,nDay) != 0)
			{
				//失败
				OutputMsg(rmError,_T("建表失败！sql:m_szCreateConsume"));
				isSucceed =false;
			}
			else
			{
				Db.ResetQuery();
			}

			if (Db.Exec(m_szCreateDeal,nYear,nMonth,nDay) != 0)
			{
				//失败
				OutputMsg(rmError,_T("建表失败！sql:m_szCreateDeal"));
				isSucceed =false;
			}
			else
			{
				Db.ResetQuery();
			}


			if (Db.Exec(m_szCreateItem,nYear,nMonth,nDay) != 0)
			{
				//失败
				OutputMsg(rmError,_T("建表失败！sql:m_szCreateItem"));
				isSucceed =false;
			}
			else
			{
				Db.ResetQuery();
			}
			if (Db.Exec(m_szCreateDrop,nYear,nMonth,nDay) != 0)
			{
				//失败
				OutputMsg(rmError,_T("建表失败！sql:m_szCreateDrop"));
				isSucceed =false;
			}
			else
			{
				Db.ResetQuery();
			}

			if (Db.Exec(m_szCreateJoinAtv,nYear,nMonth,nDay) != 0)
			{
				//失败
				OutputMsg(rmError,_T("建表失败！sql:m_szCreateJoinAtv"));
				isSucceed =false;
			}
			else
			{
				Db.ResetQuery();
			}
			
			/*
			if (Db.Exec(m_szCreateSuggest,nYear,nMonth,nDay) != 0)
			{
				//失败
				OutputMsg(rmError,_T("建表失败！sql:m_szCreateSuggest"));
				isSucceed =false;
			}
			else
			{
				Db.ResetQuery();
			}
			*/
			//下一天
			nDay++;
			if (nDay > 31)
			{
				nDay = 1;
				nMonth++;
				if (nMonth > 12)
				{
					nMonth = 1;
					nYear++;
				}
			}

		}
		if(isSucceed)
		{
			m_nextCreateTtableTick = nCurrentTick + CREATE_TABLE_INTERVAL ;  //下次调用是10天以后的事情
			OutputMsg(rmNormal,"CreateTables finish,next=%lld,current=%lld",m_nextCreateTtableTick,nCurrentTick);

		}
		else
		{
			m_nextCreateTtableTick = nCurrentTick + 30000 ; //10秒以后重新创建表
			OutputMsg(rmError,"error,CreateTables !!!");
		}
	}
#ifdef WIN32
	__except(DefaultExceptHandler(GetExceptionInformation()))
	{
		Db.ResetQuery();
		OutputMsg(rmError,"error,CreateTables");
		m_nextCreateTtableTick = nCurrentTick +30000;
	}
#endif	

}

void CLoggerServerSocket::SendOpenChatLogFlag(int nServerIndex,int nFlag)
{
	CLoggerClientSocket *pClient;

	//OutputMsg(rmTip,"SendOpenChatLogFlag,nServerIndex=%d,nFlag=%d",nServerIndex,nFlag);
	int nCount = (int)m_LogClientList.count();
	for ( int i= nCount - 1; i>-1; --i )
	{
		pClient = (CLoggerClientSocket*)m_LogClientList[i];
		//如果连接对象是一个逻辑服务器
		//OutputMsg(rmTip,"CLoggerClientSocket,nServerIndex=%d",pClient->GetServerIndex());

		if ( pClient && pClient->connected() && pClient->registed() && pClient->GetServerIndex() == nServerIndex)
		{
			//if(	pClient->getClientType() == GameServer && pClient->getClientServerIndex() > 0 )
			//{
				CDataPacket &pdata = pClient->allocProtoPacket(LOG_LOOKCHATMSG_CMD); //分配一个 网络包
				pdata << (int)nFlag;
				pClient->flushProtoPacket(pdata);
			//}
		}	
	}
}

void CLoggerServerSocket::AddServerClient(CLoggerClientSocket* pClient,int Flag)
{
	if(pClient)
	{
		if(Flag == 0)
		{
			m_LogClientList.add(pClient);
			int nServerIndex = pClient->GetServerIndex();
			if (GetLogServer()->pChatLogSrv)
			{
				if (GetLogServer()->pChatLogSrv->IsLogOnServerIndex(nServerIndex))
				{
					SendOpenChatLogFlag(nServerIndex,1);
				}
				else
				{
					//新服链接，聊天监控获取下远程配置信息
					GetLogServer()->pChatLogSrv->SendChatGetNewServerList();
				}
			}
			
		}
		else
		{
			CLoggerClientSocket *tmpClient;
			INT_PTR nCount = m_LogClientList.count();
			for ( INT_PTR i= nCount - 1; i>-1; --i )
			{
				tmpClient = (CLoggerClientSocket*)m_LogClientList[i];
				if(tmpClient && tmpClient == pClient)
				{
					m_LogClientList.lock();
					m_LogClientList.remove(i);
					m_LogClientList.unlock();
					break;
				}
			}
		}
	}
}

void CLoggerServerSocket::SendShutUpMsg(int nServerIndex,int nType,char* sName,int nTime)
{
	CLoggerClientSocket *pClient;
	INT_PTR nCount = m_LogClientList.count();
	for ( INT_PTR i= nCount - 1; i>-1; --i )
	{
		pClient = (CLoggerClientSocket*)m_LogClientList[i];
		//如果连接对象是一个逻辑服务器

		if ( pClient && pClient->connected() && pClient->registed() && pClient->GetServerIndex() == nServerIndex)
		{
			CDataPacket &pdata = pClient->allocProtoPacket(LOG_SENDCHATRECORD_CMD); //分配一个 网络包
			pdata << (int)nServerIndex;
			pdata << (int)nType;
			pdata.writeString(sName?sName:"");
			pdata << (int)nTime;
			pClient->flushProtoPacket(pdata);
		}	
	} 
}
