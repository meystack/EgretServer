#include "stdafx.h"




CBackServerSocket::CBackServerSocket(char *pszFileName)
{
	Config.ReadConfig(pszFileName);
	SetServiceName(Config.SrvConf.szServiceName);
	SetServiceHost(Config.SrvConf.szAddr);
	SetServicePort(Config.SrvConf.nPort);
	SetDbConnection();
	m_boCreateTableStoped = TRUE;
	m_nextGetNoticeTick = 0;
	m_HttpServer.DoHttpServerCreate("0.0.0.0",Config.SrvConf.nHttpPort);
}


CBackServerSocket::~CBackServerSocket(void)
{
}


CCustomServerClientSocket* CBackServerSocket::CreateClientSocket(SOCKET nSocket, PSOCKADDR_IN pAddrIn)
{
	if (pAddrIn == NULL)
	{
		return NULL;
	}
	OutputMsg( rmTip, "接受客户端连接!" );
	CBackClientSocket* pClientSocket = new CBackClientSocket(this,nSocket,pAddrIn);

	if(pClientSocket)
	{
		m_BackClientList.lock();
		m_BackClientList.add(pClientSocket);
		m_BackClientList.unlock();
	}
	else
	{
		OutputMsg(rmError,"连接初始化失败");
	}

	return pClientSocket;
}

BOOL CBackServerSocket::ExecSql(const char* szSql)
{
	return TRUE;
}

VOID CBackServerSocket::SetDbConnection()
{
	Db.SetServerHost(Config.DbConf.szHost);
	Db.SetServerPort(Config.DbConf.nPort);
	Db.SetDataBaseName(Config.DbConf.szDbName);
	Db.SetUserName(Config.DbConf.szUser);
	//这个要解密
	Db.SetPassWord(Config.DbConf.szPassWord);
	//多个客户端连接，数据库操作类要加锁
	//Db.SetMultiThread(TRUE);
	Db.SetConnectionFlags(CLIENT_FOUND_ROWS | CLIENT_MULTI_RESULTS);
	BOOL boConnected = Db.Connect();

	if (boConnected)
	{
		OutputMsg( rmTip, _T("连接数据库成功"));

		if (Config.DbConf.nUseUtf8 )
		{
			if (mysql_set_character_set(Db.GetMySql(),"utf8"))
			{
				OutputMsg( rmError, _T("设置utf8编码出错 !!!") );
			}
		}
		OutputMsg(rmTip, _T("mysql connection character set: %s"), mysql_character_set_name(Db.GetMySql()));
		CreateTables();
	}
	else
	{
		OutputMsg( rmError, _T("连接数据库失败"));
	}
}

void CBackServerSocket::DelClient(CBackClientSocket* pClient)
{
	CBackClientSocket *tmpClient;
	m_BackClientList.lock();
	for ( INT_PTR i=m_BackClientList.count() - 1; i>-1; --i )
	{
		tmpClient = (CBackClientSocket*)m_BackClientList[i];
		if(tmpClient && tmpClient == pClient)
		{		
			m_BackClientList.remove(i);
			break;
		}
	}
	m_BackClientList.unlock();
}

void CBackServerSocket::CreateTables()
{
	if (Db.Exec(m_szCommandHistory) != 0)
	{
		//失败
		OutputMsg(rmError,_T("建表失败！sql:%s"),m_szCommandHistory);
	}
	else
	{
		Db.ResetQuery();
	}

	if (Db.Exec(szSQLSP_DropNoticTable) != 0)
	{
		OutputMsg(rmError,_T("建表失败！sql:%s"),szSQLSP_DropNoticTable);
	}
	else
	{
		Db.ResetQuery();
	}

	if (Db.Exec(m_szCreateNotice) != 0)
	{
		//失败
		OutputMsg(rmError,_T("建表失败！sql:%s"),m_szCreateNotice);
	}
	else
	{
		Db.ResetQuery();
	}
}

//每次调用
VOID CBackServerSocket::SingleRun()
{
	ServerInherited::SingleRun();
	TICKCOUNT tcNow = _getTickCount();
	if(m_nextGetNoticeTick <= 0)
	{
		m_nextGetNoticeTick = tcNow + 20 * 60 * 1000;
	}

	//30分钟
	if(tcNow>m_nextGetNoticeTick)
	{
		CBackClientSocket *pClient;
		m_BackClientList.lock();
		for(int i=0;i<m_BackClientList.count();i++)
		{
			pClient = (CBackClientSocket*)m_BackClientList[i];
			if(pClient)
			{
				pClient->GetNoticeMsg();
			}
		}
		m_BackClientList.unlock();
		m_nextGetNoticeTick=tcNow+60 * 60 * 1000;
	}
	
	
	m_HttpServer.HttpGetData();

	for (int i = m_HttpServer.m_fdSetCount-1; i >0; i--)
	{
		SocketInfo& sInfo = m_HttpServer.m_SocketInfoArr[i];
		if (strlen(sInfo.data)>0)
		{
			OutputMsg(rmTip, _T("%s"), BackHttpUtility::URLDecode(sInfo.data).c_str());
			m_HttpServer.ClearParams();
			HandleHttpCommand(sInfo);
		}
	}
	
}

void CBackServerSocket::HandleHttpCommand(SocketInfo& socketInfo)
{
	if (!m_HttpServer.DocumentParser(socketInfo.data))
	{
		memset(socketInfo.data,0, BUFSIZE);
		m_HttpServer.SendResponse(socketInfo.socket, "Get Http Param Parser Error");
		OutputMsg( rmError, "Get Http Param Parser Error");
		return;
	}


	int nOperIndex = m_HttpServer.GetParamIntValue("operid");
	LPCTSTR sUserName = m_HttpServer.GetParamCharValue("user");
	if (!sUserName || nOperIndex < 0)
	{
		m_HttpServer.SendResponse(socketInfo.socket,"Http Param Error");
		OutputMsg( rmError, "Http Param Error");
		return;
	}
	int nServerIndex = m_HttpServer.GetParamIntValue("server_num");
	int nSpid = m_HttpServer.GetParamIntValue("spid");
	int nId = m_HttpServer.GetParamIntValue("auto_id");
	LPCTSTR sCommand = m_HttpServer.GetParamCharValue("command");
	CBackClientSocket *pClient;
	bool bFindServer = false;

	if(nOperIndex > 0 && nServerIndex == 0)		
	{
		m_BackClientList.lock();
		OutputMsg(rmTip, _T("发送命令%d到全服,spid:%d"), nOperIndex, nSpid);
		for(int i=0;i<m_BackClientList.count();i++)
		{
			pClient = (CBackClientSocket*)m_BackClientList[i];

			if(pClient && pClient->GetSpid() == nSpid)
			{	
				
				pClient->SendCommandToLogic(nServerIndex,nOperIndex,(char*)sCommand,(char*)sUserName,nId);
				m_HttpServer.AddToSocketInfo(socketInfo, nId, nSpid, nServerIndex);
				bFindServer = true;
			}
		}
		m_BackClientList.unlock();
	}
	else if(nOperIndex > 0 && nServerIndex > 0)						
	{
		m_BackClientList.lock();
		for(int i=0;i<m_BackClientList.count();i++)
		{
			pClient = (CBackClientSocket*)m_BackClientList[i];
			if(pClient )
			{
				int nIndex = pClient->GetnServerIndex();
				int nidd = pClient->GetSpid();
				if ( nIndex == nServerIndex && nidd == nSpid)
				{
					OutputMsg(rmTip, _T("发送命令%d到%d,spid:%d"), nOperIndex, nServerIndex, nSpid);
					pClient->SendCommandToLogic(nServerIndex,nOperIndex,(char*)sCommand,(char*)sUserName,nId);
					m_HttpServer.AddToSocketInfo(socketInfo, nId, nSpid, nServerIndex);
					bFindServer = true;
					break;
				}
			}
		}
		m_BackClientList.unlock();
	}	
	if (!bFindServer)
	{	
		m_HttpServer.SendResponse(socketInfo.socket, "Cannot Find Server");
		OutputMsg(rmError,"Cannot Find Server spid:%d,serverindex:%d",nSpid,nServerIndex);
	}
	
}
