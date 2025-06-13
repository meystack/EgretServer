#include "StdAfx.h"
//#include "SSManager.h"

CSSManager::CSSManager()
{
	m_sServerName[0] = 0;
	SetSQLConfig("localhost", 3306, "gamedb", "root", "123456",false);

	m_dwQueryCharTimeOut = 30 * 1000;
	m_dwEntryGameTimeOut = 30 * 1000;

	m_CharGateRouteList.setLock(&m_CharGateRouteListLock);
	m_AdministLoginList.setLock(&m_AdministLoginListLock);
	m_LogicServerInfoList.setLock(&m_LogicServerInfoLIstLock);

	//去掉网关服务器
	//m_pGateManager = new CSSGateManager(this);
	//m_pGateManager->Initialize();

	m_pSessionServer = new CSessionServer(this);
	m_pLogClient =new LogSender(jxSrvDef::SessionServer,0,m_sServerName); //创建一个日志服务器的连接对象
	m_pAMClient = new CAMClient();
	m_pSessionCenter = new CSessionCenter(jxSrvDef::SessionServer,0,m_sServerName,this);
	m_pAMClient->SetProductName(_T("djrm"));
	m_isFcmOpen = false;
	m_nLoginIpLimit =10; //默认10个
	m_bCardmd5 =true;
	m_spidMd5[0]=0;
	m_logIpCount =0;
	
	m_phpTime =180;
	m_kickCount =5;
	m_gmIpLogin =false;
}

CSSManager::~CSSManager()
{
	Shutdown();

	//m_pGateManager->Stop();
	//m_pGateManager->Uninitialize();
	//SafeDelete(m_pGateManager);
	SafeDelete(m_pSessionServer);
	SafeDelete(m_pLogClient);
	SafeDelete(m_pAMClient);
	SafeDelete(m_pSessionCenter);
}

VOID CSSManager::ClearRouteList()
{
	INT_PTR i;
	m_CharGateRouteList.lock();
	for ( i=m_CharGateRouteList.count()-1; i>-1; --i )
	{
		delete m_CharGateRouteList[i];
	}
	m_CharGateRouteList.clear();
	m_CharGateRouteList.unlock();
}

VOID CSSManager::ClearAdministLoginList()
{
	INT_PTR i;
	m_AdministLoginList.lock();
	for ( i=m_AdministLoginList.count()-1; i>-1; --i )
	{
		delete m_AdministLoginList[i];
	}
	m_AdministLoginList.clear();
	m_AdministLoginList.unlock();
}

VOID CSSManager::SetupSQLConnection(CSQLConenction *lpConnection)
{
	lpConnection->SetServerHost(m_sSQLHost);
	lpConnection->SetServerPort(m_nSQLPort);
	lpConnection->SetDataBaseName(m_sSQLDBName);
	lpConnection->SetUserName(m_sSQLUser);
	lpConnection->SetPassWord(m_sSQLPassword);
	lpConnection->SetConnectionFlags(CLIENT_FOUND_ROWS | CLIENT_MULTI_RESULTS);
	lpConnection->SetMultiThread(FALSE);
}

VOID CSSManager::SetServerName(LPCSTR sSrvName)
{
	_asncpytA(m_sServerName, sSrvName);
	//SetConsoleTitle( sSrvName );

}

VOID CSSManager::SetGateServiceAddress(LPCTSTR sHost, const int nPort)
{
	//m_pGateManager->SetServiceHost(sHost);
	//m_pGateManager->SetServicePort(nPort);
}

VOID CSSManager::SetSessionServiceAddress(LPCTSTR sHost, const int nPort)
{
	m_pSessionServer->SetServiceHost(sHost);
	m_pSessionServer->SetServicePort(nPort);
}

VOID CSSManager::SetAMServerAddr(LPCTSTR sHost, const int nPort)
{
	m_pAMClient->SetServerHost(sHost);
	m_pAMClient->SetServerPort(nPort);
}


//计算hash值
VOID CSSManager::SetSPID(LPCTSTR sName) 
{ 
	_asncpyt(m_sSPID, sName);	
	m_pAMClient->SetSPID(m_sSPID);

	//计算一下hash值的md5

	char sTotal[128];
	_asncpyt(sTotal, sName);	
	char num[20];
	num[0]=0;

	INT_PTR nSpguid= m_nSpGuid;

	//itoa(nSpguid,num,10); //计算出spguid
	sprintf(num,"%d",nSpguid);
	strncat(sTotal,num,sizeof(sTotal) );	


	MD5_CTX   ctx; 
	//使用 MD5加密玩家的密码
	MD5Init(&ctx); 
	MD5Update(&ctx,  (unsigned char *)sTotal,   (unsigned int)strlen(sTotal) );
	MD5Final(m_spidMd5,&ctx);
	m_spidMd5[32]=0;
	OutputMsg(rmTip,"spid=%s,spguid=%s,spguidmd5=%s,sTotal=%s",m_sSPID,num,m_spidMd5,sTotal);
}


VOID CSSManager::SetLogServerAddress(LPCTSTR sHost, const int nPort)
{
	m_pLogClient->SetServerHost(sHost);
	m_pLogClient->SetServerPort(nPort);
}

VOID CSSManager::SetSQLConfig(LPCSTR sHost, const int nPort, LPCSTR sDBName, LPCSTR sDBUser, LPCSTR sUserPassword,bool isUtf8)
{
	_asncpytA(m_sSQLHost, sHost);
	m_nSQLPort = nPort;
	_asncpytA(m_sSQLDBName, sDBName);
	_asncpytA(m_sSQLUser, sDBUser);
	_asncpytA(m_sSQLPassword, sUserPassword);
	m_bUtf8 = isUtf8;

}

PCHARGATEROUTE getServerRoute(CBaseList<PCHARGATEROUTE> &list, const int nServerIndex)
{
	INT_PTR i;
	for ( i=list.count()-1; i>-1; --i )
	{
		if (list[i]->nServerIndex == nServerIndex )
		{
			return list[i];
		}
	}
	return NULL;
}

INT_PTR CSSManager::LoadCharGateRouteTable()
{
	int nRouteCount = 0;
	int nValue;
	PCHARGATEROUTE pRoute;
	MYSQL_ROW pRow;
	CBaseList<PCHARGATEROUTE> RouteList;
	CSQLConenction sql;

	//配置数据库连接
	SetupSQLConnection(&sql);
	if ( !sql.Connect() )
		return -1;

	//从数据库加载角色网关路由表
	if ( sql.Query( szLoadCharGateRouteTable ) )
	{
		sql.Disconnect();
		return -2;
	}

	//读取角色网关路由表
	pRow = sql.CurrentRow();
	while ( pRow )
	{
		sscanf(pRow[1], "%d", &nValue); //server_id

		pRoute = getServerRoute(RouteList, nValue);
		if ( !pRoute )
		{
			pRoute = new CHARGATEROUTE;
			ZeroMemory(pRoute, sizeof(*pRoute));
			pRoute->nServerIndex = nValue;
			RouteList.add(pRoute);
		}

		if ( pRoute->nRouteCount < ArrayCount(pRoute->RouteTable) )
		{
			_asncpytA(pRoute->RouteTable[pRoute->nRouteCount].sHost, pRow[2]); //名字
			sscanf(pRow[3], "%d", &nValue);   //端口
			pRoute->RouteTable[pRoute->nRouteCount].nPort = nValue;
			OutputMsg( rmNormal, _T("添加路由表，Host=%s,port=%d,serverid=%d"),
				pRoute->RouteTable[pRoute->nRouteCount].sHost,nValue,pRoute->nServerIndex);

			pRoute->nRouteCount++;
			nRouteCount++;
		}

		pRow = sql.NextRow();
	}
	//断开数据库连接
	sql.ResetQuery();
	sql.Disconnect();

	//将加载的新的角色网关路由数据保存到角色网关路由表中
	m_CharGateRouteList.lock();
	ClearRouteList();
	m_CharGateRouteList.addList(RouteList);
	m_CharGateRouteList.unlock();

	return nRouteCount;
}

INT_PTR CSSManager::LoadAdministLoginTable()
{
	PADMINLOGINRECORD pRecord;
	MYSQL_ROW pRow;
	CBaseList<PADMINLOGINRECORD> AdministLoginList;
	CSQLConenction sql;

	//配置数据库连接
	SetupSQLConnection(&sql);
	if ( !sql.Connect() )
		return -1;

	//从数据库加载管理员登录表
	if ( sql.Query( szLoadAdministLoginTable ) )
	{
		sql.Disconnect();
		return -2;
	}

	//读取管理员登录表
	pRow = sql.CurrentRow();
	while ( pRow )
	{
		pRecord = new ADMINLOGINRECORD;
		sscanf(pRow[0], "%d", &pRecord->eRecType);
		_asncpytA(pRecord->sRecData, pRow[1]);
		AdministLoginList.add(pRecord);
		OutputMsg( rmNormal, _T("添加管理员，类型为%d,Name=%s"),pRecord->eRecType,pRecord->sRecData);
		pRow = sql.NextRow();
	}
	//断开数据库连接
	sql.ResetQuery();
	sql.Disconnect();

	//将加载的新的管理员登录数据保存到管理员登录表中
	m_AdministLoginList.lock();
	ClearAdministLoginList();
	m_AdministLoginList.addList(AdministLoginList);
	m_AdministLoginList.unlock();

	return m_AdministLoginList.count();
}

BOOL CSSManager::SelectCharGateRoute(const INT_PTR nServerIndex, LPSTR lpGateHost, const SIZE_T dwHostLen, PINT lpGatePort)
{
	INT_PTR i, nRouteIndex;
	BOOL boResult = FALSE;
	PCHARGATEROUTE pRoute;

	m_CharGateRouteList.lock();
	for ( i=m_CharGateRouteList.count()-1; i>-1; --i )
	{
		pRoute = m_CharGateRouteList[i];
		if ( pRoute->nServerIndex == nServerIndex )
		{
			nRouteIndex = (_getTickCount() / 1000) % pRoute->nRouteCount;
			strncpy(lpGateHost, pRoute->RouteTable[nRouteIndex].sHost, dwHostLen-1);
			lpGateHost[dwHostLen-1] = 0;
			*lpGatePort = pRoute->RouteTable[nRouteIndex].nPort;
			boResult = TRUE;
			break;
		}
	}
	m_CharGateRouteList.unlock();

	return boResult;
}

INT_PTR CSSManager::LoadLogicServerInfoList()
{
	INT_PTR nResult = 0;
	LOGICSERVERINFO si;
	CBaseList<LOGICSERVERINFO>	serverList;	
	CSQLConenction sql;	
	SetupSQLConnection(&sql);
	if (!sql.Connect())
	{
		OutputMsg(rmError, _T("%s connect sql server failed"), __FUNCTION__);
		return -1;
	}

	if (sql.Query(szLoadServerInfoList))
	{
		OutputMsg(rmError, _T("%s exec sp[%s] failed"), __FUNCTION__, szLoadServerInfoList);
		sql.Disconnect();
		return -2;
	}

	MYSQL_ROW pRow = sql.CurrentRow();
	while ( pRow )
	{
		sscanf(pRow[0], "%d", &si.server_id);
		sscanf(pRow[1], "%d", &si.cserver_id);		
		if (pRow[2])
		{
			_asncpytA(si.serverName, pRow[2]);
		}
		else
		{
			_asncpytA(si.serverName, "Unknown");
		}

		serverList.add(si);
		/*
		OutputMsg(rmNormal, 
				  _T("添加服务器信息，[serverid=%d, cserverid=%d, Name=%s"),
				  si.server_id,
				  si.cserver_id,
				  si.serverName);

		*/

		pRow = sql.NextRow();
		nResult++; 
	}	
	sql.ResetQuery();
	sql.Disconnect();

	m_LogicServerInfoList.lock();
	m_LogicServerInfoList.clear();
	m_LogicServerInfoList.addList(serverList);
	// 更新m_CommServerClientList
	m_CommServerClientList.Clear();
	for (INT_PTR i = 0; i < m_LogicServerInfoList.count(); i++)
	{
		m_CommServerClientList.AddClient(m_LogicServerInfoList[i].cserver_id, m_LogicServerInfoList[i].server_id);
	}
	m_CommServerClientList.Trace();
	m_LogicServerInfoList.unlock();

	return nResult;
}

bool CSSManager::GetLogicServerInfo(const int nServerId, LOGICSERVERINFO &info)
{
	m_LogicServerInfoList.lock();
	for (INT_PTR i = 0; i < m_LogicServerInfoList.count(); i++)
	{
		if (nServerId == m_LogicServerInfoList[i].server_id)
		{
			CopyMemory(&info, &m_LogicServerInfoList[i], sizeof(LOGICSERVERINFO));
			return true;
		}
	}
	m_LogicServerInfoList.unlock();

	return false;
}

void CSSManager::GetClientLogicServerList(const int nCommServerId, CBaseList<int> &clientList)
{
	m_LogicServerInfoList.lock();
	CommServerClient *pSC = m_CommServerClientList.GetServerClient(nCommServerId);
	if (pSC)
	{
		clientList.addList(pSC->ClientList);
	}
	m_LogicServerInfoList.unlock();
}

BOOL CSSManager::IsAdministLogin(LPCSTR sAccount, LPCSTR sIPAddr)
{
	INT_PTR i;
	BOOL boResult = FALSE;
	PADMINLOGINRECORD pRecord;

	m_AdministLoginList.lock();
	for ( i=m_AdministLoginList.count()-1; i>-1; --i )
	{
		pRecord = m_AdministLoginList[i];
		if ( pRecord->eRecType == ADMINLOGINRECORD::alAccount )
		{
			if ( _stricmp(pRecord->sRecData, sAccount) == 0 )
			{
				boResult = TRUE;
				break;
			}
		}
		if ( pRecord->eRecType == ADMINLOGINRECORD::alIPAddress )
		{
			if ( _stricmp(pRecord->sRecData, sIPAddr) == 0 )
			{
				boResult = TRUE;
				break;
			}
		}
	}
	m_AdministLoginList.unlock();

	return boResult;
}

BOOL CSSManager::Startup()
{
	//int nError;

	//初始化网络套接字
	
	int nError = CCustomWorkSocket::InitSocketLib();
	if ( nError )
	{
		OutputError( nError, _T("初始化网络库失败") );
		return FALSE;
	}
	

	
	//加载角色网关路由表
	/*
	if ( LoadCharGateRouteTable() < 0 )
	{
		OutputMsg( rmError, _T("加载角色网关路由表失败"));
		return FALSE;
	}
	*/

	//加载管理员登录表
	/*
	if ( LoadAdministLoginTable() < 0 )
	{
		OutputMsg( rmError, _T("加载管理员表失败"));
		return FALSE;
	}

	// 加载逻辑服务器信息列表
	if (LoadLogicServerInfoList() < 0)
	{
		OutputMsg(rmError, _T("加载逻辑服务器列表失败"));
	}
	*/

	//启动日志客户端
	if ( !m_pLogClient->Startup() )
	{
		OutputMsg(rmError, _T("启动日志失败"));
		return FALSE;
	}

	if (m_pLogClient)
	{
		m_pLogClient->SetServerSpid(m_sSPID);
	}
	
	if (!m_pAMClient->Startup())
	{
		OutputMsg(rmError, _T("启动AM失败"));
		return FALSE;
	}

	//启动会话客户端
	if ( !m_pSessionServer->Startup() )
	{
		OutputMsg(rmError, _T("!m_pSessionServer->Startup() 失败"));
		return FALSE;
	}

	//启动网关管理器
	//m_pGateManager->Startup();
	
	return TRUE;
}

VOID CSSManager::Shutdown()
{
	//停止网关
	//m_pGateManager->Stop();
	//停止会话客户端
	m_pSessionServer->Stop();
	//停止日志客户端

	m_pLogClient->Stop();
	// 关闭AMClient
	m_pAMClient->Stop();

	if(m_pSessionCenter->connected())
	{
		m_pSessionCenter->Stop();
	}
	//销毁角色网关路由表
	//ClearRouteList();
	//销毁管理员登录列表
	//ClearAdministLoginList();
	//卸载网络库
	//CSSGateManager::UnintSocketLib();
}


VOID CSSManager::UpdateAMC()
{
	if (!m_pAMClient) return;
	wylib::container::CBaseList<CAMClient::AMOPDATA> msg_list;
	INT_PTR msg_count = m_pAMClient->GetAMOPResults(msg_list);

	msg_count = __min(msg_count, msg_list.count());

	for (INT_PTR i = 0; i < msg_count; i++)
	{
		m_pSessionServer->PostAMCMsg(msg_list[i]);
	}	
}

VOID CSSManager::UpdateTask()
{
	if (!m_pAMClient) return;
	wylib::container::CBaseList<CAMClient::TASKMSG> msg_list;
	INT_PTR msg_count = m_pAMClient->GetTaskOPResults(msg_list);

	msg_count = __min(msg_count, msg_list.count());

	for (INT_PTR i = 0; i < msg_count; i++)
	{
		m_pSessionServer->PostAMCTaskMsg(msg_list[i]);
	}	
}
