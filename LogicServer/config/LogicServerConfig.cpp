#include "StdAfx.h"
#include "LogicServerConfig.h" 
#include "util_func_extend.h"  

#define CROSSSERVER_CS_LOCAL_FILE "./data/crossserver.config"//跨服服务器配置  监听端口
#define CROSSSERVER_SOURCE_CONNECT_FILE "./data/config/Cross/CrossSourceServer.config"//跨服服务器配置  原服
#define CROSSSERVER_CROSSSE_CONNECT_FILE "./data/config/Cross/CrossServer.config"//跨服服务器配置  跨服

#define CROSSSERVER_SVR_ID_FILE "./data/config/Cross/LocalServer.config"//跨服标记服务器ID 
#define CROSSSERVER_CALCULATE_PARAM 100000 //跨服识别原服务器计算码
 

const TCHAR CLogicServerConfig::ConfigFileName[] = _T("LogicServerLinux.txt");

CLogicServerConfig::CLogicServerConfig()
	:Inherited()
{

}

CLogicServerConfig::~CLogicServerConfig()
{

}

void CLogicServerConfig::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString s = _T("[Config Error]");
	s += sError;
	throw s;
}

bool CLogicServerConfig::loadServerConfig(CLogicServer *lpLogicServer, const char *pszFileName)
{
	bool boResult = false;	
	if (!pszFileName)
	{
		pszFileName = ConfigFileName;
	}
 
	try
	{
		boResult = throwLoadConfig(lpLogicServer, pszFileName);
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, (LPCSTR)s);
	}
	catch(...)
	{
		OutputMsg(rmError, _T("unexpected error on load - read ServerConfig "));
	}
	OutputMsg(rmTip, _T("load server config complete"));
	return boResult;
}

bool CLogicServerConfig::throwLoadConfig(CLogicServer *lpLogicServer, const char *pszFileName)
{
	wylib::stream::CMemoryStream ms;	
	if ( ms.loadFromFile(pszFileName) <= 0 )
	{
		showErrorFormat(_T("unable to load from %s"), pszFileName);
		return false;
	}

	if ( !setScript((LPSTR)ms.getMemory()) )
		return false;

	return readServerConfig(lpLogicServer);
}

bool CLogicServerConfig::readServerConfig(CLogicServer *lpLogicServer)
{
	if ( !openGlobalTable("LogicServer") )
		return false;

	LPCSTR sVal;
	INT nVal;
	char sServerName[128]; // 

	bool isCommonServer = false;	
	isCommonServer = getFieldBoolean("IsCommonServer", &isCommonServer);	
	GetLogicServer()->SetCommonServer(isCommonServer);

	//服务器名字和index
	getFieldStringBuffer("ServerName", sServerName,ArrayCount(sServerName)); //name
#ifdef WIN32
	CUtility::convertToACP(sServerName,strlen(sServerName), sServerName, ArrayCount(sServerName));
#endif
	SYSTEMTIME startSysTime; //服务器的开启时间
	CMiniDateTime nStart;

	BOOL IsValid =TRUE;
	LPCTSTR sName = getFieldString("ServerOpenTime", "",&IsValid);
	if( IsValid &&  sName != NULL && strlen(sName) >10) //是个基本合法的名字
	{
		sscanf(sName, "%d-%d-%d %d:%d:%d", &startSysTime.wYear, &startSysTime.wMonth, &startSysTime.wDay, &startSysTime.wHour, &startSysTime.wMinute, &startSysTime.wSecond);
	}
	else
	{
		GetLocalTime(&startSysTime);
	}
	nStart.encode(startSysTime);


	SYSTEMTIME combineTime; //服务器的合服时间
	CMiniDateTime nCombine;

	BOOL boValid =TRUE;
	LPCTSTR sTime = getFieldString("ServerCombineTime", "",&boValid);
	if( boValid &&  sTime != NULL && strlen(sTime) >10) //是个基本合法的名字
	{
		sscanf(sTime, "%d-%d-%d %d:%d:%d", &combineTime.wYear, &combineTime.wMonth, &combineTime.wDay, &combineTime.wHour, &combineTime.wMinute, &combineTime.wSecond);
		nCombine.encode(combineTime);
	}
	else
	{
		nCombine = 0;
	}
	
	nVal = getFieldInt("ServerIndex"); //serverindex

	INT nServerIndex =nVal;

	BOOL fiValid =TRUE;
	char strFilePath[64] = "./data/runtime";
	//sprintf_s(strFilePath,sizeof(strFilePath),"./data/runtime",nServerIndex);
	if (lpLogicServer)
	{
		lpLogicServer->SetServerIndex(nVal);
		lpLogicServer->SetServerName(sServerName);
		lpLogicServer->SetServerOpenTime(nStart); //设置开区时间
		lpLogicServer->SetServerCombineTime(nCombine);	// 设置合区时间
		lpLogicServer->SetStrFilePath(strFilePath);		
	}
	
	nVal = 0;
	INT_PTR nSpid = getFieldInt("spguid",&nVal); //读取spid
	lpLogicServer->SetSpid(nSpid); //设置spid

	/*
	bool bStartCommonServer = true;	
	bStartCommonServer = getFieldBoolean("IsStartCommonServer", &bStartCommonServer);	

	nVal = 0;
	nVal = getFieldInt("CommonServerId", &nVal);
	if (lpLogicServer)
		lpLogicServer->SetCommonServerId(nVal);
	if ((!isCommonServer) && (nVal == 0 || bStartCommonServer == false))
		lpLogicServer->SetStartCommonServer(false);

	*/

	
	//网关服务配置
	if ( openFieldTable("GateService") )
	{
		sVal = getFieldString("Address");
		nVal = getFieldInt("Port");
		if (NULL != sVal)
		{
			CLogicGateManager *pLogicManager = lpLogicServer->GetGateManager();
			if (pLogicManager)
			{
				pLogicManager->SetServiceHost(sVal);
				pLogicManager->SetServicePort(nVal);
			}
		}
		closeTable();
	}

	//会话服务配置
	if ( openFieldTable("SessionServer") )
	{
		sVal = getFieldString("Address");
		nVal = getFieldInt("Port");
		
		CLogicSSClient * pSessionClient = lpLogicServer->GetSessionClient();
		if (pSessionClient)
		{
			pSessionClient->SetServerHost(sVal);
			pSessionClient->SetServerPort(nVal);
		}
		closeTable();
	}
	
	//日志服务器地址配置
	if ( openFieldTable("LogServer") )
	{
		sVal = getFieldString("Address");
		nVal = getFieldInt("Port");
		
		LogSender * pLogClient  =lpLogicServer->GetLogClient(); 
		if (pLogClient)
		{
			pLogClient->SetServerHost(sVal);
			pLogClient->SetServerPort(nVal);
			pLogClient->SetServerIndex(nServerIndex);
			pLogClient->SetServerName(sServerName);
			pLogClient->SetServerType(jxSrvDef::GameServer);
		}
		closeTable();
	}

	//公共日志服务器地址配置
	if ( openFieldTable("LocalLogServer") )
	{
		sVal = getFieldString("Address");
		nVal = getFieldInt("Port");

		CLocalSender * pLocalLogClient  =lpLogicServer->GetLocalClient(); 
		if (pLocalLogClient)
		{
			pLocalLogClient->SetServerHost(sVal);
			pLocalLogClient->SetServerPort(nVal);
			pLocalLogClient->SetServerIndex(nServerIndex);
			pLocalLogClient->SetServerName(sServerName);
			pLocalLogClient->SetServerType(jxSrvDef::GameServer);
		}
		closeTable();
	}

	//数据client配置
	if (openFieldTable("DbServer"))
	{
		sVal = getFieldString("Address");
		nVal = getFieldInt("Port");
		CDataClient * pDbClient  =lpLogicServer->GetDbClient(); 
		if (pDbClient)
		{
			pDbClient->SetServerHost(sVal);
			pDbClient->SetServerPort(nVal);
		}
		closeTable();
	}
	//好友服务器配置
	//if (openFieldTable("FriendServer"))
	//{
	//	CFriendClient *pClient = lpLogicServer->GetFriendClient();
	//	//内部服务器地址配置
	//	if (openFieldTable("Server"))
	//	{
	//		sVal = getFieldString("Host");
	//		nVal = getFieldInt("Port");
	//		pClient->SetServerHost(sVal);
	//		pClient->SetServerPort(nVal);
	//		closeTable();
	//	}
	//	//用户连接的好友服务器网关地址配置
	//	if (openFieldTable("Gate"))
	//	{
	//		sVal = getFieldString("Host");
	//		nVal = getFieldInt("Port");
	//		pClient->SetFriendGateHost(sVal);
	//		pClient->SetFriendGatePort(nVal);
	//		closeTable();
	//	}
	//	closeTable();
	//}
	if (openFieldTable("MgrServer"))
	{
		sVal = getFieldString("Host");
		nVal = getFieldInt("Port");
		CBackStageSender * pClient = lpLogicServer->GetBackClient();
		if(pClient)
		{
			pClient->SetServerHost(sVal);
			pClient->SetServerPort(nVal);
			pClient->SetServerIndex(nServerIndex);
			pClient->SetServerName(sServerName);
			pClient->SetServerType(jxSrvDef::GameServer);
		}
		closeTable();
	}

	//if (feildTableExists("CommonServer") && openFieldTable("CommonServer"))
	//{
	//	char szServerIP[32];
	//	WORD wPort;
	//	LPCSTR szField[2] = {"Server", "Client"};
	//	int nIndex = isCommonServer ? 0 : 1;
	//	if (feildTableExists(szField[nIndex]) && openFieldTable(szField[nIndex]))
	//	{
	//		getFieldStringBuffer("Address", szServerIP, ArrayCount(szServerIP));
	//		wPort = (WORD)getFieldInt("Port");
	//		GetLogicServer()->SetCommonServerAddr(szServerIP, wPort);
	//		closeTable();
	//	}
	//	closeTable();
	//}

	closeTable();

	return true;
}

// //读取跨服配置表
// bool CLogicServerConfig::LoadCrossServerConfig(CLogicServer *lpLogicServer,bool bReload)
// {
// 	if(!lpLogicServer) return false;

// 	bool result = false;
// 	CMemoryStream ms;
// 	try
// 	{
// 		if ( ms.loadFromFile("crossserver.txt") <= 0 )
// 		{
// 			showError(_T("unable to load config from file crossserver.txt"));
// 			return result;
// 		}
// 		if ( !setScript((LPCSTR)ms.getMemory()) )
// 		{
// 			showError(_T("parse config script failed"));
// 			return false;
// 		}
// 		if ( openGlobalTable("crossserver"))
// 		{
// 			int nServerIndex = GetLogicServer()->GetServerIndex();

// 			int nCenterServerIndex =0;
// 			if(lpLogicServer->IsCommonServer())
// 			{
// 				nCenterServerIndex = nServerIndex;
// 			}
// 			else
// 			{
// 				nCenterServerIndex= lpLogicServer->GetCommonServerId();
// 			}
 
// 			//当前的公告服的ID
// 			OutputMsg(rmTip,"LoadCrossServerConfig Common server index = %d",nCenterServerIndex); //当前

// 			LPCSTR sSPID = GetLogicServer()->GetVSPDefine().GetDefinition(CVSPDefinition::SPID);
// 			if(nCenterServerIndex <= 0 && bReload)
// 			{
// 				if ( openFieldTable("map"))
// 				{	
// 					size_t nCount = lua_objlen(m_pLua,-1);

// 					if ( enumTableFirst() )
// 					{
// 						do 
// 						{
// 							int nId = getFieldInt("id");
// 							LPCSTR tmpSpid = getFieldString("spid");
// 							if ( openFieldTable("servers"))
// 							{
// 								INT_PTR j =0;
// 								if (enumTableFirst())
// 								{
// 									do
// 									{
// 										int nServerid = (int)getFieldNumber(NULL);
// 										if(nServerid == nServerIndex && strcmp(tmpSpid,sSPID) == 0)
// 										{
// 											lpLogicServer->SetCommonServer(false);
// 											lpLogicServer->SetCommonServerId(nId);
// 											lpLogicServer->SetStartCommonServer(true);
// 											nCenterServerIndex = nId;
// 										}
// 										j++;
// 									} while (enumTableNext());
// 								}
// 								closeTable();
// 							}

// 							//if(nServerIndex == nCenterid)
// 							//{
// 							//	lpLogicServer->SetCommonServer(true);
// 							//	nCenterServerIndex = nCenterid;
// 							//	//break;
// 							//}

// 							//if(nServerIndex >= nStart && nServerIndex <= nEnd && nServerIndex != nCenterid)
// 							//{
// 							//	lpLogicServer->SetCommonServer(false);
// 							//	lpLogicServer->SetCommonServerId(nCenterid);
// 							//	lpLogicServer->SetStartCommonServer(true);
// 							//	nCenterServerIndex = nCenterid;
// 							//	//break;
// 							//}
// 						}while (enumTableNext());
// 					}

// 					closeTable();
// 				}
// 			}

// 			bool bHasServer =false; 

// 			if ( openFieldTable("center"))
// 			{		
// 				size_t nCount = lua_objlen(m_pLua,-1);

// 				if ( enumTableFirst() )
// 				{

// 					INT_PTR nIdx = 0;
// 					do 
// 					{		
// 						// 读取DBCenter服务器配置
// 						int nServerId = getFieldInt("id");
// 						char szClientServerIP[64]; //客户端连接的ip，给客户端用的
// 						char szCommonServerIp[64]; //服务器连接的ip

// 						int  nPort =0;
// 						int nMinPort = 0;
// 						int nMaxPort = 0;
// 						if(nServerId == nCenterServerIndex)
// 						{
// 							getFieldStringBuffer("loginsrv", szClientServerIP, ArrayCount(szClientServerIP));      //连接的公共服的域名
// 							getFieldStringBuffer("commondbsrv", szCommonServerIp, ArrayCount(szClientServerIP));   //连接公共数据服的域名

// 							nPort =getFieldInt("dbport");
// 							nMinPort = getFieldInt("minport");
// 							nMaxPort = getFieldInt("maxport");
// 							GetLogicServer()->SetCommonServerAddr(szClientServerIP,szCommonServerIp, nPort,nMinPort,nMaxPort);

// 							bHasServer =true;

// 						}

// 						nIdx++;
// 					}while (enumTableNext());
// 				}
// 				closeTable();//GateService
// 			}


// 			closeTable();

// 			//这没有开启跨服
// 			if(!bHasServer)
// 			{
// 				GetLogicServer()->SetCommonServerAddr("","", 0,0,0);
// 			}
// 		}
// 	}
// 	catch(RefString &s)
// 	{
// 		OutputMsg( rmNormal, s.rawStr() );
// 	}
// 	catch(...)
// 	{
// 		OutputMsg( rmNormal, _T("unexpected error on load config") );
// 	}
// 	return true;
// } 

//读取跨服配置表
bool CLogicServerConfig::LoadNewCrossClientConfig(CLogicServer *lpLogicServer, bool bReload)
{
	if(!lpLogicServer) 
	{
		return false;
	}
	bool result = false;
	CMemoryStream ms;
	try
	{
		if ( ms.loadFromFile(CROSSSERVER_SVR_ID_FILE) <= 0 )
		{
			showError(_T("unable to load config from file crossserver.config"));
			return result;
		}
		if ( !setScript((LPCSTR)ms.getMemory()) )
		{
			showError(_T("parse config script failed"));
			return false;
		}
		
		LPCSTR sVal;
		INT nVal = 0;
		int nDef = 0;
		
		if ( openGlobalTable("LocalServerConfig"))
		{ 
			int nServerIndex = GetLogicServer()->GetServerIndex();
			// if ( feildTableExists("server") && openFieldTable("server"))
			// {	
			// 	sVal = getFieldString("Address");
			// 	nVal = getFieldInt("Port");
			// 	if (NULL != sVal)
			// 	{
			// 		auto *pCrossServer = lpLogicServer->GetCrossServerManager();
			// 		if (pCrossServer)
			// 		{
			// 			pCrossServer->SetCrossServiceAddress(sVal, nVal);
			// 			lpLogicServer->SetCrossServer(true);
			// 		}
					
			// 	}
			// 	closeTable();
			// }
			//当前服的ID
			int localId = getFieldInt("localId", &nDef);
			localId = nServerIndex + localId * CROSSSERVER_CALCULATE_PARAM;
			lpLogicServer->SetCrossServerId(localId);
			
			// bool bHasServer =false; 
			// if ( feildTableExists("client") && openFieldTable("client"))
			// {		
			// 	sVal = getFieldString("Address");
			// 	nVal = getFieldInt("Port");
			// 	int nCrossServerIndex = getFieldInt("croessServerId",&nDef);
			// 	if (NULL != sVal && nCrossServerIndex != 0)
			// 	{
			// 		nCrossServerIndex = nCrossServerIndex*10000 + nServerIndex;
			// 		lpLogicServer->SetCrossServerId(nCrossServerIndex);
			// 		auto *pCrossClient = lpLogicServer->GetCrossClient();
			// 		if (pCrossClient)
			// 		{
			// 			pCrossClient->SetServerHost(sVal);
			// 			pCrossClient->SetServerPort(nVal);
			// 			// lpLogicServer->SetCrossServer(true);
			// 		}
					
			// 	}
			// 	closeTable();
			// }
			closeTable();
		}
	}
	catch(RefString &s)
	{
		OutputMsg( rmNormal, s.rawStr() );
	}
	catch(...)
	{
		OutputMsg( rmNormal, _T("unexpected error on load config") );
	}
	return true;
}

//读取跨服配置表
bool CLogicServerConfig::LoadNewCrossServerConfig(CLogicServer *lpLogicServer, bool bReload)
{
	if(!lpLogicServer) 
	{ 
		return false;
	}
	// if (!lpLogicServer->IsCrossServer())
	// {
	// 	return false;//原服服务器不处理
	// }

	bool result = false;
	CMemoryStream ms;
	try
	{
		//if ( ms.loadFromFile("./data/crossserver.config") <= 0 )
		if ( ms.loadFromFile(CROSSSERVER_CS_LOCAL_FILE) <= 0 )
		{
			showError(_T("unable to load config from file crossserver.config"));
			return result;
		}
		if ( !setScript((LPCSTR)ms.getMemory()) )
		{
			showError(_T("parse config script failed"));
			return false;
		}
		
		LPCSTR sVal;
		INT nVal = 0;
		int nDef = 0;
		
		if ( openGlobalTable("crossserver"))
		{
			int nServerIndex = GetLogicServer()->GetServerIndex();
			if ( feildTableExists("server") && openFieldTable("server"))
			{	
				sVal = getFieldString("Address");
				nVal = getFieldInt("Port");
				if (NULL != sVal)
				{
					auto *pCrossServer = lpLogicServer->GetCrossServerManager();
					if (pCrossServer)
					{
						pCrossServer->SetCrossServiceAddress(sVal, nVal);
						lpLogicServer->SetCrossServer(true);
						OutputMsg( rmTip, "[CSConfig] 为跨服服务器 CrossIP=[%s], Port=[%d]", sVal, nVal);
					}
				}
				closeTable();
			}
			closeTable();
		}
	}
	catch(RefString &s)
	{
		OutputMsg( rmNormal, s.rawStr() );
	}
	catch(...)
	{
		OutputMsg( rmNormal, _T("unexpected error on load config") );
	}
	return true;
} 

//跨服读取配置表
bool CLogicServerConfig::LoadLocalCSConfigByCross(CLogicServer *lpLogicServer, bool bReload)
{
	if(!lpLogicServer)
	{ 
	 	return false;
	}

	bool result = false;
	CMemoryStream ms;
	try
	{
		if ( ms.loadFromFile(CROSSSERVER_CROSSSE_CONNECT_FILE) <= 0 )
		{
			showError(_T("unable to load config from file crossserver.config"));
			return result;
		}
		if ( !setScript((LPCSTR)ms.getMemory()) )
		{
			showError(_T("parse config script failed"));
			return false;
		}
		
		LPCSTR sVal;
		INT nVal = 0;
		int nDef = 0;
		
		//if (lpLogicServer->IsCrossServer())
		//{
		//	return false;//跨服服务器不处理
		//}

		if ( openGlobalTable("CrossServerConfig"))
		{
			
			int nServerIndex = lpLogicServer->GetServerIndex();
			int CsId = lpLogicServer->GetCrossServerId();
			
			if(enumTableFirst()) {

				do{
					if(enumTableFirst()) {
						do{
							CROSSSERVERCFG cfg;
							cfg.nPFid = getFieldInt("pfid",&nDef);
							cfg.nServerId = getFieldInt("serverid",&nDef);
							int nKeyCfg = cfg.nServerId + cfg.nPFid * CROSSSERVER_CALCULATE_PARAM;
							//cfg.nPort = getFieldInt("gate", &nDef);
							int nClientPort = getFieldInt("port", &nDef);
							int nSwitch = getFieldInt("switch", &nDef);
							getFieldStringBuffer(("localNamePR"), cfg.sLocalNamePR, sizeof(cfg.sLocalNamePR));
							// getFieldStringBuffer(("srvaddr"), cfg.sClientNet, sizeof(cfg.sClientNet));
							// getFieldStringBuffer(("CrossIP"), cfg.nCrossIP, sizeof(cfg.nCrossIP));
							//跨服不需要连接了
							// if(!nSwitch
							//  && cfg.nPort != 0 
							//  && nKeyCfg == CsId) 
							// {
							// 	auto *pCrossClient = lpLogicServer->GetCrossClient();
							// 	if (pCrossClient)
							// 	{
							// 		pCrossClient->SetServerHost(cfg.nCrossIP);
							// 		pCrossClient->SetServerPort(cfg.nPort);
							// 		OutputMsg( rmTip, "[CSConfig] 连接到跨服服务器 CrossIP=[%s], Port=[%d],  nPFid=[%d], nServerId=[%d], SID=[%d], CSID=[%d]", cfg.nCrossIP, cfg.nPort, cfg.nPFid, cfg.nServerId, nServerIndex, CsId);
							// 	}
							// 	lpLogicServer->m_nCSGatePort = nClientPort;
							// 	memcpy(lpLogicServer->sClientNet, cfg.sClientNet, sizeof(cfg.sClientNet));
 
							// 	// CSSourceInfo info;
							// 	// info.nkfid = getFieldInt("kfid", &nDef); //编号
							// 	// info.nServerId = nServerIndex;//getFieldInt("serverid", &nDef); 
							// 	// //int nSwitch = getFieldInt("switch", &nDef);
							// 	// getFieldStringBuffer(("srvaddr"), info.sClientNet, sizeof(info.sClientNet));
							// 	// int nClientPort = getFieldInt("port", &nDef);
							// 	// info.gate = nClientPort;
							// 	// //getFieldStringBuffer(("localNamePR"), info.sLocalNamePR, sizeof(info.sLocalNamePR));
							// 	// getFieldStringBuffer(("CrossIP"), info.nCrossIP, sizeof(info.nCrossIP));
							// 	// info.nPort = getFieldInt("gate", &nDef);
								
							// 	// lpLogicServer->GetLogicServerConfig()->m_mCSConfigSource[1] = info; 
							// } 
							// else
							// {
							// 	OutputMsg( rmTip, "过滤跨服服务器 CrossIP=[%s], Port=[%d], nPFid=[%d], nServerId=[%d],", cfg.nCrossIP, cfg.nPort, cfg.nPFid, cfg.nServerId);
							// }
							if (feildTableExists("map") && openFieldTable("map")){
								cfg.nDestination = getFieldInt("id");
								//打开子表
								if (feildTableExists("range") && openFieldTable("range"))
								{
									if(enumTableFirst())
									{
										int  i = 0;
										do
										{
											if (i >= sizeof(cfg.nPointRange)/sizeof(int)) break;
											
											cfg.nPointRange[i] = getFieldInt(NULL);
											i++;
										} while (enumTableNext());

									}
									closeTable();
								} 
								closeTable();
							}
							
							lpLogicServer->GetLogicServerConfig()->m_mCSConfigCross[nKeyCfg] = cfg;
						}while(enumTableNext());
					}
				}while(enumTableNext());
			}
			closeTable();
		}
	}
	catch(RefString &s)
	{
		OutputMsg( rmNormal, s.rawStr() );
	}
	catch(...)
	{
		OutputMsg( rmNormal, _T("unexpected error on load config") );
	}
	return true;
} 

//原服读取配置表
bool CLogicServerConfig::LoadLocalCSConfigBySource(CLogicServer *lpLogicServer, bool bReload)
{
	if(!lpLogicServer)
	{ 
	 	return false;
	}

	bool result = false;
	CMemoryStream ms;
	try
	{
		if ( ms.loadFromFile(CROSSSERVER_SOURCE_CONNECT_FILE) <= 0 )
		{
			showError(_T("unable to load config from file crossserver.config"));
			return result;
		}
		if ( !setScript((LPCSTR)ms.getMemory()) )
		{
			showError(_T("parse config script failed"));
			return false;
		}
		
		LPCSTR sVal; 
		int nDef = 0;
		 
		if ( openGlobalTable("CrossSourceServerConfig"))
		{
			
			int nServerIndex = lpLogicServer->GetServerIndex();
			int CsId = lpLogicServer->GetCrossServerId();
			
			if(enumTableFirst()) 
			{ 
				do
				{
					CSSourceInfo info;
					info.nkfid = getFieldInt("kfid", &nDef); //编号
					info.nServerId = nServerIndex;//getFieldInt("serverid", &nDef);
					int nKeyCfg = info.nServerId + info.nkfid * CROSSSERVER_CALCULATE_PARAM;
					//int nSwitch = getFieldInt("switch", &nDef);
					getFieldStringBuffer(("GateAddr"), info.cGateAddr, sizeof(info.cGateAddr));
					int nGatePort = getFieldInt("GatePort", &nDef);
					info.nGatePort = nGatePort;
					//getFieldStringBuffer(("localNamePR"), info.sLocalNamePR, sizeof(info.sLocalNamePR));
					getFieldStringBuffer(("CrossIP"), info.cCrossIP, sizeof(info.cCrossIP));
					info.nCrossPort = getFieldInt("CrossPort", &nDef); 
					if (feildTableExists("openday") && openFieldTable("openday"))
					{ 
						if(enumTableFirst())
						{
							int  i = 0;
							do
							{
								if (i >= 2)
								{
									break;
								}
								
								if(i == 0)
								{ 
									info.nOpenDayBegin = getFieldInt(NULL); 
								}
								
								if(i == 1)
								{
									info.nOpenDayEnd = getFieldInt(NULL);  
								}

								i++;
							} while (enumTableNext());

						}
						closeTable(); 
					}
					 
					if (feildTableExists("map") && openFieldTable("map"))
					{
						info.nDestination = getFieldInt("id");
						//打开子表
						if (feildTableExists("range") && openFieldTable("range"))
						{
							if(enumTableFirst())
							{
								int  i = 0;
								do
								{
									if (i >= sizeof(info.nPointRange)/sizeof(int)) break;
									
									info.nPointRange[i] = getFieldInt(NULL);
									i++;
								} while (enumTableNext());

							}
							closeTable();
						} 
						closeTable();
					}
					
					lpLogicServer->GetLogicServerConfig()->m_mCSConfigSource[info.nkfid] = info; 
					 
				}while(enumTableNext());
			}
			closeTable();
		}
	}
	catch(RefString &s)
	{
		OutputMsg( rmNormal, s.rawStr() );
	}
	catch(...)
	{
		OutputMsg( rmNormal, _T("unexpected error on load config") );
	} 
	int nOpenServerDay = GetLogicServer()->GetDaysSinceOpenServer(); 
	CSSourceInfo* info = GetLogicServer()->GetLogicServerConfig()->getCSConfigSourceByOpenday(nOpenServerDay);
 
	if(!info)
	{ 
		OutputMsg( rmTip, "[CSConfig] 1跨服服务器 原服配置错：%d", nOpenServerDay); 
		return true;
	}
	CLocalCrossClient *pCSClient = lpLogicServer->GetCrossClient();
	
	if(!pCSClient)
	{ 
		OutputMsg( rmTip, "[CSConfig] 2跨服服务器 连接启动失败"); 
		return true;
	}
 
	pCSClient->OnSetSrvHostPort(info->cCrossIP, info->nCrossPort); 
	pCSClient->OnSetCsGateAddr(info->cGateAddr);
	pCSClient->OnSetCsGatePort(info->nGatePort); 
	
	std::string str = info->cGateAddr;
	str += ":";
	str += utility::toString(info->nGatePort);
	str += _T("跨服逻辑");
	pCSClient->SetClientName(str);
	// pCSClient->SetServerHost(info.nCrossIP);
	// pCSClient->SetServerPort(info.nPort);
	OutputMsg( rmTip, "[CSConfig] 连接到跨服服务器 CrossIP=[%s], Port=[%d], nServerId=[%d] ", info->cCrossIP, info->nCrossPort, info->nServerId);
 
	lpLogicServer->m_nCSGatePort = info->nGatePort;
	lpLogicServer->m_cGateAddr = info->cGateAddr;  

	return true;
} 
//原服配置 
CSSourceInfo* CLogicServerConfig::getCSConfigSourceByOpenday(int tDaytime)
{
	std::map<int, CSSourceInfo>::iterator it = m_mCSConfigSource.begin();
	for (; it != m_mCSConfigSource.end(); it++)
	{
		CSSourceInfo &temp = it->second;
		if (tDaytime >= temp.nOpenDayBegin)
		{
			if (tDaytime <= temp.nOpenDayEnd
				|| temp.nOpenDayEnd == -1)
			{
				return &temp;
			}
		} 
	}  
	return NULL; 
}

