#include "StdAfx.h"
#include "wrand.h"

CDBServer* CDBServer::s_pDBEngine = NULL;

CDBServer::CDBServer()
{
	m_nServerIndex = 0;

	m_sServerName[0] = 0;
	m_sNameSyncServerHost[0] = 0;
	m_boFirstCharEnableSelCountry = FALSE;
//	SetSQLConfig("localhost", 3306, "gamedb", "root", "123456");

	m_GameServerRouteList.setLock(&m_GameServerRouteListLock);

	/*
	m_pGateManager = new CDBGateManager(this);
	m_pGateManager->Initialize();
	m_pSSClient = new CDBSessionClient(this);
	*/

	//m_pFilterWords = NULL;

//	m_pLogClient->SetLocalServerType(DBServer);
	m_pNameSyncClient = new CNameSyncClient(this);
	m_pDataServer = new CDBDataServer(this);
	//m_pLogClient = new LogSender();
	//m_pLogClient->SetServerType(jxSrvDef::DBServer);
	
	//m_pDBCenterClient = new CDBCenterClient(this);

	m_pListPrefix = new CLoadString;
	//m_pListConnect = new CLoadString;
	m_pListStuffix = new CLoadString;

	m_bStartDbServer = FALSE;

	m_boUtf8 = false;
}


CDBServer::~CDBServer()
{
	OutputMsg(rmNormal, _T("Start Save Name File"));
	OutputMsg(rmNormal, _T("Save Name File End"));
	OutputMsg(rmNormal, _T("Start ShutDown DBServer"));
	Shutdown();
	OutputMsg(rmNormal, _T("ShutDown DBServer End"));	
	//m_pGateManager->Uninitialize();
	OutputMsg(rmNormal, _T("End GateUnitialize...."));
	
	//SafeDelete(m_pGateManager);
	//OutputMsg(rmNormal, _T("Wait Del SessionClient...."));
	
	//SafeDelete(m_pSSClient);
	//OutputMsg(rmNormal, _T("Wait Del LogClient...."));
	//SafeDelete(m_pLogClient);	
	//OutputMsg(rmNormal, _T("Wait Del DBCenterClient...."));
	
	//SafeDelete(m_pDBCenterClient);
	OutputMsg(rmNormal, _T("Wait Del NameSynClient...."));
	SafeDelete(m_pNameSyncClient);
	OutputMsg(rmNormal, _T("Wait Del DataServer...."));
	SafeDelete(m_pDataServer);
	OutputMsg(rmNormal, _T("Wait Del Name Group..."));
	SafeDelete(m_pListPrefix);
	//SafeDelete(m_pListConnect);
	SafeDelete(m_pListStuffix);
	OutputMsg(rmNormal, _T("End...."));
}

void CDBServer::Trace()
{
	/*
	if(m_pDBCenterClient)
	{
		m_pDBCenterClient->Trace();
	}
	*/
		
	if(m_pDataServer)
	{
		m_pDataServer->Trace();
	}
	

}


VOID CDBServer::FreeGameServerRoute()
{
	INT_PTR i;
	for ( i=m_GameServerRouteList.count()-1; i>-1; --i )
	{
		delete m_GameServerRouteList[i];
	}
	m_GameServerRouteList.clear();
}

VOID CDBServer::SetupSQLConnection(CSQLConenction *lpConnection)
{
	lpConnection->SetServerHost(m_sSQLHost);
	lpConnection->SetServerPort(m_nSQLPort);
	lpConnection->SetDataBaseName(m_sSQLDBName);
	lpConnection->SetUserName(m_sSQLUser);
	lpConnection->SetPassWord(m_sSQLPassword);
	lpConnection->SetConnectionFlags(CLIENT_FOUND_ROWS | CLIENT_MULTI_RESULTS);
}

//修改了字符串越界的问题
/*
	utf-8编码规则，如果是1个字节是
	1个字节0xxxxxxx
	2个字节110xxxxx 10xxxxxx
	3个字节1110xxxx 10xxxxxx 10xxxxxx
	4个字节11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
	5个字节111110xx 10xxxxxx 10xxxxxx 10xxxxxx
*/
LPCSTR CDBServer::LowerCaseNameStr(LPSTR sNameStr,INT_PTR nSize)
{
	LPCSTR sResult = sNameStr;
	INT_PTR count=0;
	while ( *sNameStr && count < nSize)
	{
		/*
		if ( (unsigned int)*sNameStr >= 0x7F )
		{
			sNameStr+=3;
			continue;
		}
		*/
		//如果是多个字节的
		if( (*sNameStr & 0x80))
		{
			
		}
		else
		{
			if ( *sNameStr >= 'A' && *sNameStr <= 'Z' )
			{
				*sNameStr |= 0x20;
			}
		}

		
		sNameStr++;
		count ++;
	}
	return sResult;
}

PGAMESERVERROUTE getServerRoute(CBaseList<PGAMESERVERROUTE> &list, const int nServerIndex)
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

INT CDBServer::LoadGameServerRoute()
{
	int Result = 0;
	int nValue =0;
	
	MYSQL_ROW pRow;
	PGAMESERVERROUTE pRoute;
	CBaseList<PGAMESERVERROUTE> RouteList;

	CSQLConenction sql;
	//配置数据库连接
	SetupSQLConnection(&sql);
	//连接数据库
	if ( !sql.Connect() )
	{
		return -1;
	}

	//从数据库角色网关路由表
	if ( !sql.Query(szSQLSP_LoadGameSrvRouteTable) )
	{
		//读取角色网关路由表
// 		+-------------+---------------+------------+---------------+
// 		| serverindex | serveraddress | serverport | selgateaddr   |
// 		+-------------+---------------+------------+---------------+
		pRow = sql.CurrentRow();
		while ( pRow )
		{
			if(pRow[0])
			{
				sscanf(pRow[0], "%d", &nValue);
			}
			else
			{
				OutputMsg(rmWaning,_T("%s serverindex is NULL"),pRow[0]);
			}

			pRoute = getServerRoute(RouteList, nValue);
			if ( !pRoute )
			{
				pRoute = new GAMESERVERROUTE;
				ZeroMemory(pRoute, sizeof(*pRoute));
				pRoute->nServerIndex = nValue;
				RouteList.add(pRoute);
			}

			if ( pRoute->nRouteCount < ArrayCount(pRoute->RouteTable) )
			{
				_asncpytA(pRoute->RouteTable[pRoute->nRouteCount].sHost, pRow[1]);
				sscanf(pRow[2], "%d", &nValue);
				pRoute->RouteTable[pRoute->nRouteCount].nPort = nValue;
				pRoute->nRouteCount++;
				TRACE(_T("路由:%d %s:%d\n"), pRoute->nServerIndex, pRow[1], nValue);
				Result++;
			}

			pRow = sql.NextRow();
		}

		//将从数据库中加载的游戏网关路由表保存到类数据中
		m_GameServerRouteList.lock();
		FreeGameServerRoute();
		m_GameServerRouteList.addList(RouteList);
		m_GameServerRouteList.unlock();

		sql.ResetQuery();
	}	
	//断开与数据的连接
	sql.Disconnect();

	return Result;
}

INT_PTR CDBServer::SelecteGameGateRouteIndex(jxSrvDef::PGAMESERVERROUTE pRoute)
{
	int nMinCount = 1000000;
	INT_PTR nMinCountIdx = 0;
	if (pRoute->nRouteCount <= 0)
		return 0;

	for (INT_PTR i = 0; i < pRoute->nRouteCount; i++)
	{
		if (pRoute->RouteTable[i].nUserCount < nMinCount)
		{
			nMinCount = pRoute->RouteTable[i].nUserCount;
			nMinCountIdx = i;
		}
	}
	pRoute->RouteTable[nMinCountIdx].nUserCount++;
	return nMinCountIdx;
}

void CDBServer::TraceGameServerRoute()
{
	m_GameServerRouteList.lock();
	PGAMESERVERROUTE pRoute;
	for (INT_PTR i = 0; i < m_GameServerRouteList.count(); i++)
	{
		pRoute = m_GameServerRouteList[i];		
		for (INT_PTR j = 0; j < pRoute->nRouteCount; j++)
		{
			OutputMsg(rmTip, _T("路由[%s:%6d] 用户人数:%d"), 
					pRoute->RouteTable[j].sHost, 
					pRoute->RouteTable[j].nPort,
					pRoute->RouteTable[j].nUserCount);
		}
	}
	m_GameServerRouteList.unlock();
}
void CDBServer::UpdateLogicGateUserInfo(CDataPacketReader& packet)
{
	DECLARE_FUN_TIME_PROF()
	int nServerIdx = 0;
	WORD nCount = 0;
	packet >> nServerIdx >> nCount;
	PGAMESERVERROUTE pRoute;
	GATEUSERINFO gi;
	if (nCount > 0)
	{
		m_GameServerRouteList.lock();
		// 找到对应服务器的路由配置
		bool bValidServerRoute = false;
		for (INT_PTR routeIdx = m_GameServerRouteList.count()-1; routeIdx > -1; --routeIdx)
		{
			pRoute = m_GameServerRouteList[routeIdx];
			if (pRoute->nServerIndex == nServerIdx)  
			{	
				bValidServerRoute = true;
				break;
			}
		}

		if (!bValidServerRoute)
		{
			OutputMsg(rmError, _T("找不到对应服务器[server_index=%d]的路由配置信息"), nServerIdx);
		}
		else
		{
			INT_PTR nCurRouteCount = pRoute->nRouteCount;
			for (INT_PTR i = 0; i < nCount; i++)
			{
				packet >> gi;
				for (INT_PTR ri = 0; ri < nCurRouteCount; ri++)
				{
					if (pRoute->RouteTable[ri].nPort == gi.nGatePort)
					{
						pRoute->RouteTable[ri].nUserCount = gi.nUserCount;
						OutputMsg(rmTip, _T("更新逻辑网关[port:%d]用户数目： %d"), (int)gi.nGatePort,  gi.nUserCount);
					}
				}
			}
		}
		
		m_GameServerRouteList.unlock();
	}
}

BOOL CDBServer::SelectGameServerRoute(const int nServerIndex, LPSTR lpGateHost, const SIZE_T dwHostLen, PINT_PTR lpGatePort)
{
	INT_PTR i;
	PGAMESERVERROUTE pRoute;
	BOOL boResult = FALSE;

	m_GameServerRouteList.lock();
	for ( i=m_GameServerRouteList.count()-1; i>-1; --i )
	{
		pRoute = m_GameServerRouteList[i];
		if ( pRoute->nServerIndex == nServerIndex )
		{
			//随机挑选一个路由地址
			//int nIndex = (_getTickCount() / 1000) % pRoute->nRouteCount;			
			int nIndex = (int)SelecteGameGateRouteIndex(pRoute);
			strncpy(lpGateHost, pRoute->RouteTable[nIndex].sHost, dwHostLen-1);
			lpGateHost[dwHostLen-1] = 0;
			*lpGatePort = pRoute->RouteTable[nIndex].nPort;
			boResult = TRUE;
			break;
		}
	}
	m_GameServerRouteList.unlock();

	return boResult;
}

VOID CDBServer::SetServerName(LPCSTR sSrvName)
{
	_asncpytA(m_sServerName, sSrvName);
	//m_pLogClient->SetServerName((char*)(sSrvName));
}

VOID CDBServer::SetGateServiceAddress(LPCTSTR sHost, const INT_PTR nBindPort)
{
	//m_pGateManager->SetServiceHost(sHost);
	//m_pGateManager->SetServicePort(nBindPort);
}
	
VOID CDBServer::SetSessionServerAddress(LPCTSTR sHost, const INT_PTR nPort)
{
	//m_pSSClient->SetServerHost(sHost);
	//m_pSSClient->SetServerPort(nPort);
}
	
VOID CDBServer::SetLogServerAddress(LPCTSTR sHost, const INT_PTR nPort)
{
	//m_pLogClient->SetServerHost(sHost);
	//m_pLogClient->SetServerPort(nPort);
}

VOID CDBServer::SetDBCenterAddress(LPCTSTR sHost, const INT_PTR nPort)
{
	/*
	m_pDBCenterClient->SetServerHost(sHost);
	m_pDBCenterClient->SetServerPort(nPort);
	*/
}

VOID CDBServer::SetSQLConfig(LPCSTR sHost, const INT_PTR nPort, LPCSTR sDBName, LPCSTR sDBUser, LPCSTR sUserPassword, bool boUtf8)
{
	_asncpytA(m_sSQLHost, sHost);
	m_nSQLPort = nPort;
	_asncpytA(m_sSQLDBName, sDBName);
	_asncpytA(m_sSQLUser, sDBUser);
	_asncpytA(m_sSQLPassword, sUserPassword);
	m_boUtf8 = boUtf8;
}

VOID CDBServer::SetSelectCountryEnabledOnCreateFirstChar(const BOOL boEnabled)
{
	m_boFirstCharEnableSelCountry = boEnabled;
}

VOID CDBServer::SetNameSyncServerAddress(LPCTSTR sHost, const INT_PTR nPort)
{
	_asncpyt(m_sNameSyncServerHost, sHost);
	m_nNameSyncServerPort = nPort;
	m_pNameSyncClient->SetServerHost(sHost);
	m_pNameSyncClient->SetServerPort(nPort);
}

VOID CDBServer::SetDataServiceAddress(LPCTSTR sBindHost, const INT_PTR nBindPort)
{
	m_pDataServer->SetServiceHost(sBindHost);
	m_pDataServer->SetServicePort(nBindPort);
}

VOID CDBServer::SetServerIndex(INT nServerIndex)
{
	m_nServerIndex = nServerIndex;
	//m_pLogClient->SetServerIndex(nServerIndex);
}
BOOL CDBServer::Startup()
{
	
	m_bStartDbServer = true;

	CTimeProfMgr::getSingleton();

	//初始化网络套接字
	/*
	nError = m_pSSClient->InitSocketLib();
	if ( nError )
	{
		OutputError( nError, _T(__FUNCTION__)_T("初始化网络库失败") );
		return FALSE;
	}
	*/

	//加载角色网关路由表
	//LoadGameServerRoute();
	LoadFilterNames();
	CheckNameFilter();
	// 启动日志客户端
	/*
	if ( !m_pLogClient->Startup() )
		return FALSE;
	
	//启动会话客户端
	if ( !m_pSSClient->Startup() )
		return FALSE;
	*/

	//启动名称客户端
	if ( !m_pNameSyncClient->Startup() )
		return FALSE;
	
	//启动数据服务器
	if ( !m_pDataServer->Startup() )
		return FALSE;
	
	//启动网关管理器
	//m_pGateManager->Startup();

	// 启动DBCenter客户端，需要在GateMgr和DataServer初始化之后！
	/*
	if (m_pDBCenterClient->GetServerPort() != 0 && !m_pDBCenterClient->Startup())
	{
		OutputMsg(rmError, _T("启动连向DBCenter的客户端失败"));
		return FALSE;
	}
	*/

	//m_pDBCenterClient->Init();

	return TRUE;
}

VOID CDBServer::Shutdown()
{
	//停止网关
	//OutputMsg(rmNormal, _T("Prepare to Stop Gate Manager"));
	//m_pGateManager->Stop();
	//停止会话客户端
	//OutputMsg(rmNormal, _T("Prepare to Stop SessionClient"));
	///m_pSSClient->Stop();
	//停止数据服务器
	OutputMsg(rmNormal, _T("Prepare to Stop DataServer"));
	m_pDataServer->Stop();
	//停止名称客户端（必须在数据服务器停止后进行）
	OutputMsg(rmNormal, _T("Prepare to Stop NameSyncClient"));
	m_pNameSyncClient->Stop();
	//停止日志客户端（必须在数据服务器停止后进行）
	OutputMsg(rmNormal, _T("Prepare to Stop LogClient"));
	//m_pLogClient->Stop();
	// 停止DBCenterClient
	//m_pDBCenterClient->Stop();
	//释放角色网关路由表	
	FreeGameServerRoute();
	//销毁屏蔽词
	/*
	if ( m_pFilterWords )
	{
		FreeMBCSFilter(m_pFilterWords);
		m_pFilterWords = NULL;
	}
	*/
	m_filter.Destroy();

	CTimeProfMgr::getSingleton().clear();

	//卸载网络库
	//m_pSSClient->UnintSocketLib();
}
BOOL CDBServer::getGameReady(int nSrvIdx)
{
	return m_pDataServer->getGameReady(nSrvIdx);
}
void CDBServer::AddFilterName(char* sName)
{
	m_filter.AddFilterWord(sName);
}
BOOL CDBServer::LoadFilterNames()
{
	CSQLConenction sql;
	//配置数据库连接
	SetupSQLConnection(&sql);

	if ( !sql.Connected() )
	{
		if (sql.Connect())
		{
			//mysql_options(sql.GetMySql(), MYSQL_SET_CHARSET_NAME, "utf8");
			if (mysql_set_character_set(sql.GetMySql(),"utf8"))
			{
				OutputMsg( rmError, _T("设置utf8编码出错 !!!") );
			}
		}else
		{
			OutputMsg(rmError, _T("can not load filter names"));
			return FALSE;
		}

	}

	//从数据库加载屏蔽文字列表  NSSQL_LoadFilterNames
	if ( sql.Query(NSSQL_LoadFilterNames) )
	{
		OutputMsg(rmError, _T("can not load filter names, database call error"));
		return FALSE;
	}
	//创建屏蔽词匹配工具
	//m_pFilterWords = CreateMBCSFilter(NULL);
	if(m_filter.Init() ==false) 
	{
		OutputMsg(rmError,"m_filter.Init() fail");
		return FALSE;
	}
	int nCount = 0;
	MYSQL_ROW pRow = sql.CurrentRow();
	while ( pRow )
	{
		if ( pRow[0] && pRow[0][0] )
		{
			//AddMBCSFilterStrToTable(m_pFilterWords, pRow[0]);
			m_filter.AddFilterWord(pRow[0]);
			nCount++;
		}
		pRow = sql.NextRow();
	}
	//释放查询结果集
	sql.ResetQuery();
	OutputMsg(rmTip, _T("%d filter name loaded"), nCount);
	
	return TRUE;
}

//修改了字符串越界的问题
/*
	utf-8编码规则，如果是1个字节是
	1个字节0xxxxxxx
	2个字节110xxxxx 10xxxxxx
	3个字节1110xxxx 10xxxxxx 10xxxxxx
	4个字节11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
	5个字节111110xx 10xxxxxx 10xxxxxx 10xxxxxx
*/
INT_PTR CDBServer::GetStrLenUtf8(LPCTSTR sName)
{
	LPCSTR sptr;
	INT_PTR nLen = 0;
	unsigned char ch;
	for ( sptr = sName; *sptr; ++sptr )
	{
		ch = (unsigned char)*sptr;
		if((ch & 0xF8) == 0xF8) //5个字节111110xx 10xxxxxx 10xxxxxx 10xxxxxx
		{
			sptr += 4;
		}
		else if( (ch & 0xF0) == 0xF0 )//4个字节11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
		{
			sptr += 3;
		}
		else if( (ch & 0xE0) == 0xE0)//3个字节1110xxxx 10xxxxxx 10xxxxxx
		{
			sptr += 2;
		}
		else if( (ch & 0xC0) == 0xC0 )//2个字节110xxxxx 10xxxxxx
		{
			sptr += 1;
		}
		nLen++;

		//增加一个判断，防止恶意输入
		if(nLen >1000)
		{
			break;
		}
	}
	return nLen;
}
void CDBServer::CheckNameFilter()
{
	for (int i=0; i <m_pListPrefix->Count(); i++)
	{
		LPCTSTR name = m_pListPrefix->get(i);
		if (!CheckNameStr(name))
		{
			OutputMsg(rmWaning,"name filter error:%s",name);
		}
	}
	for (int i=0; i <m_pListStuffix->Count(); i++)
	{
		LPCTSTR name = m_pListStuffix->get(i);
		if (!CheckNameStr(name))
		{
			OutputMsg(rmWaning,"name filter error:%s",name);
		}
	}
}
BOOL CDBServer::CheckNameStr( LPCSTR sNameStr )
{
	//检查是否包含控制字符以及英文标点符号，这些字符是不被允许的
	LPCSTR sptr;
	unsigned int ch;
	DECLARE_FUN_TIME_PROF()
	for ( sptr = sNameStr; *sptr; ++sptr )
	{
		//此处允许中文
		ch = (unsigned int)*sptr;
		if ( ch >= 0x7F )
		{
			sptr += 2;//UTF-8
			continue;
		}
		//此处允许a-z
		if ( ch >= 'a' && ch <= 'z' )
			continue;
		//此处允许A-Z
		if ( ch >= 'A' && ch <= 'Z' )
			continue;
		//此处允许0-9
		if ( ch >= '0' && ch <= '9' )
			continue;
		return FALSE;
	}

	//从名称过滤表中进行屏蔽字检查
	//m_NameFilterLock.Lock();
	/*
	if ( m_pFilterWords )
	{
		sptr = MatchMBCSFilterWord(m_pFilterWords, sNameStr, NULL);
	}
	else
	{
		OutputMsg(rmWaning,"No Filter name m_pFilterWords");
		sptr = NULL;
	}
	*/
	int nLen;
	sptr = m_filter.Filter((char*)sNameStr,&nLen);

	//m_NameFilterLock.Unlock();

	return sptr == NULL;
}

bool CDBServer::hasFilterWordsInclude(LPCSTR sNameStr)
{
	LPCSTR sptr = sNameStr;

	for ( sptr--; *sptr; )
	{
		sptr++;
		if (*sptr >= 'a' && *sptr <= 'z')
			continue;
		if (*sptr >= 'A' && *sptr <= 'Z')
			continue;
		if (*sptr >= '0' && *sptr <= '9')
			continue;
		if (*sptr >= 0x7F)
			continue;
		return true;
	}
	//return MatchMBCSFilterWord(m_pFilterWords, sNameStr, NULL) != NULL;
	int nLen;
	return m_filter.Filter((char*)sNameStr,&nLen) != NULL;
}

bool CDBServer::LoadNameGroup( LPCSTR sPrifixList, /*LPCSTR sConnectList,*/ LPCSTR sStuffixList )
{
	if(!m_pListPrefix->loadFromFile(sPrifixList))
	{
		OutputMsg(rmError, _T("%s load %s file error"), __FUNCTION__, sPrifixList);
		return false;
	}
	/*if(!m_pListConnect->loadFromFile(sConnectList))
	{
	OutputMsg(rmError, _T("%s load %s file error"), __FUNCTION__, sConnectList);
	return false;
	}*/
	if(!m_pListStuffix->loadFromFile(sStuffixList))
	{
		OutputMsg(rmError, _T("%s load %s file error"), __FUNCTION__, sStuffixList);
		return false;
	}
	return true;
}

bool CDBServer::GetRandomGroupName( char * sName, int nNameSize )
{
	LPCSTR sPrefix = m_pListPrefix->GetRandomName();
	//LPCSTR sConnect = m_pListConnect->GetRandomName();
	LPCSTR sStuffix = m_pListStuffix->GetRandomName();
	if (sPrefix != NULL /*&& sConnect != NULL*/ && sStuffix != NULL)
	{
		sprintf_s(sName, nNameSize, "%s%s",  sPrefix, sStuffix);
		return true;
	}
	return false;
}

void CDBServer::RunEsqlFile(char* sToolPath,char* sDbName)
{
	if(sToolPath && sDbName)
	{
		m_pEsqlMgr.EsqlMain(sToolPath,sDbName);
	}
}