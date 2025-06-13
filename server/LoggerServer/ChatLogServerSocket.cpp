#include "stdafx.h"

CChatLogServerSocket::CChatLogServerSocket()
{
	
	Config.ReadConfig(g_ConfigPath.c_str());

	// 设置服务器的基本属性
	SetServiceName(Config.ChatConf.szServiceName);
	SetServiceHost(Config.ChatConf.szAddr);
	SetServicePort(Config.ChatConf.nPort);
	

	/*
	char *pBuff  = (char*)malloc(256);
	in_addr ia;
	ia.S_un.S_addr = DBEGN_KN_VERION;
	strcpy(pBuff,"xhwr-");

	strcat(pBuff, Config.ChatConf.szServiceName);
	strcat(pBuff,"-V");
	strcat(pBuff,inet_ntoa(ia));
	//SetConsoleTitle(pBuff);
	free(pBuff);
		pBuff =NULL;
	*/



	//SetConsoleTitle(Config.ChatConf.szServiceName );

	//SetDbConnection();

	m_boCreateTableStoped = TRUE;
	m_nextCreateTtableTick =0;
}


CChatLogServerSocket::~CChatLogServerSocket(void)
{
}

CCustomServerClientSocket* CChatLogServerSocket::CreateClientSocket(SOCKET nSocket, PSOCKADDR_IN pAddrIn)
{
	if (pAddrIn == NULL)
	{
		return NULL;
	}
	OutputMsg( rmTip, "接受客户端连接!" );
	CChatLogClientSocket* pClientSocket = new CChatLogClientSocket(this,nSocket,pAddrIn);

	if(pClientSocket)
	{
		m_LogClientList.add(pClientSocket);
	}

	return pClientSocket;
}

BOOL CChatLogServerSocket::ExecSql(const char* szSql)
{
	return TRUE;
}


BOOL CChatLogServerSocket::DoStartup()
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

VOID CChatLogServerSocket::DoStop()
{
	/*
	if ( FALSE == InterlockedCompareExchange(&m_boCreateTableStoped, TRUE, FALSE) )
	{
		CloseThread(m_hCTThread);
	}
	*/
}

//每次调用
VOID CChatLogServerSocket::SingleRun()
{
	
	//CreateTables();
	ServerInherited::SingleRun();
	
}

void CChatLogServerSocket::AddLogServerClient(CChatLogClientSocket* pClient,int Flag)
{
	if(pClient)
	{
		if(Flag == 0)
		{
			m_LogClientList.add(pClient);
		}
		else
		{
			CChatLogClientSocket *tmpClient;
			INT_PTR nCount = m_LogClientList.count();
			for ( INT_PTR i= nCount - 1; i>-1; --i )
			{
				tmpClient = (CChatLogClientSocket*)m_LogClientList[i];
				if(tmpClient && tmpClient == pClient)
				{
					for(int j=0;j<pClient->m_nCount;j++)
					{
						if (!IsLogOnServerIndex(pClient->m_ServerList[j]))
						{
							if(GetLogServer() && GetLogServer()->pLoggerSrv)
							{
								GetLogServer()->pLoggerSrv->SendOpenChatLogFlag(pClient->m_ServerList[j],0);
							}	
						}
					}
					m_LogClientList.lock();
					m_LogClientList.remove(i);
					m_LogClientList.unlock();
					break;
				}
			}
		}
	}
}

bool CChatLogServerSocket::IsLogOnServerIndex(int nServerIndex)
{
	CChatLogClientSocket *pClient;
	INT_PTR nCount = m_LogClientList.count();
	for ( INT_PTR i= nCount - 1; i>-1; --i )
	{
		pClient = (CChatLogClientSocket*)m_LogClientList[i];
		if(pClient)
		{
			for(int j=0;j<pClient->m_nCount;j++)
			{
			
				if(pClient->m_ServerList[j] == nServerIndex)
				{
					 return true;
				}
			}
		}
	}
	return false;
}
void CChatLogServerSocket::SendChatGetNewServerList()
{
	CChatLogClientSocket *pClient;
	INT_PTR nCount = m_LogClientList.count();
	for ( INT_PTR i= nCount - 1; i>-1; --i )
	{
		pClient = (CChatLogClientSocket*)m_LogClientList[i];
		if(pClient)
		{
			CDataPacket &pdata = pClient->allocProtoPacket(SEND_GET_NEW_SERVER_LIST); //分配一个 网络包
			pClient->flushProtoPacket(pdata);
		}
	}
}
void CChatLogServerSocket::SendChatRecordToAll(int nServerIndex,int nChannleID,int nActorId,char* sActorName,char* msg)
{
	if(sActorName == NULL || msg == NULL) return;

	CChatLogClientSocket *pClient;
	INT_PTR nCount = m_LogClientList.count();
	for ( INT_PTR i= nCount - 1; i>-1; --i )
	{
		pClient = (CChatLogClientSocket*)m_LogClientList[i];
		if(pClient)
		{
			for(int j=0;j<pClient->m_nCount;j++)
			{
				//OutputMsg(rmTip,"SendChatRecordToAll,nServerIndex=%d,nChannleID=%d",pClient->m_ServerList[j],nServerIndex);
				if(pClient->m_ServerList[j] == nServerIndex)
				{
					OutputMsg(rmTip,"serverid=%d, channleid=%d, actorid=%d, actorname=[%s], msg=[%s]", nServerIndex, nChannleID, nActorId, sActorName, msg);
					CDataPacket &pdata = pClient->allocProtoPacket(SEND_CHATLOG_CMD); //分配一个 网络包
					pdata << (int)nServerIndex;
					pdata << (int)nChannleID;
					pdata << (int)nActorId;
					pdata.writeString(sActorName);
					pdata.writeString(msg);
					pClient->flushProtoPacket(pdata);
				}
			}
		}
	}
}
