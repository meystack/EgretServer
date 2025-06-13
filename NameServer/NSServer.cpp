#include "StdAfx.h"

CNameServer::CNameServer()
	:Inherited()
{

}

CNameServer::~CNameServer()
{
	INT_PTR i;
	for (i=m_FreeConnectionList.count()-1; i>-1; --i)
	{
		delete m_FreeConnectionList[i];
	}
	m_FreeConnectionList.empty();
}

CCustomServerClientSocket* CNameServer::CreateClientSocket(SOCKET nSocket, PSOCKADDR_IN pAddrIn)
{
	CNameServerClientConnection* pClient = NULL;
	INT_PTR nCount = m_FreeConnectionList.count();
	//如果空闲连接对象池中有空闲的链接对象，则直接返回一个对象
	if ( nCount > 0 )
	{
		nCount--;
		pClient = m_FreeConnectionList[nCount];
		m_FreeConnectionList.trunc(nCount);
	}
	//空闲连接对象池中没有对象则创建新的链接对象
	if ( !pClient )
	{
		pClient = new CNameServerClientConnection(this);
	}
	pClient->SetClientSocket(nSocket, pAddrIn);
	return pClient;
}

VOID CNameServer::DestroyClientSocket(CCustomServerClientSocket *pClientSocket)
{
	//将已经断开的连接对象添加到空闲连接对象池中
	m_FreeConnectionList.add((CNameServerClientConnection*)pClientSocket);
}

VOID CNameServer::OnClientConnect(CCustomServerClientSocket *pClientSocke)
{
	Inherited::OnClientConnect(pClientSocke);
	OutputMsg(rmTip, _T("Name client %s:%d connected"), pClientSocke->GetRemoteHost(), pClientSocke->GetRemotePort());
}

VOID CNameServer::OnRun()
{
	//如果数据库连接尚未就绪则连接数据库
	if ( !m_SQLConnection.Connected() )
	{
		if (m_SQLConnection.Connect())
		{
			mysql_options(m_SQLConnection.GetMySql(), MYSQL_SET_CHARSET_NAME, "utf8");
		}
	}
}

BOOL CNameServer::DoStartup()
{
	if ( !m_SQLConnection.Connected() )
	{
		if (m_SQLConnection.Connect())
		{
			mysql_options(m_SQLConnection.GetMySql(), MYSQL_SET_CHARSET_NAME, "utf8");
		}else
		{
			OutputMsg(rmError, _T("can not connect db"));
			return FALSE;
		}
		
	}

	//从数据库加载屏蔽文字列表  NSSQL_LoadFilterNames

	/*
	if ( m_SQLConnection.Query(NSSQL_LoadFilterNames) )
	{
		OutputMsg(rmError, _T("can not load filter names, database call error"));
		return FALSE;
	}
	//创建屏蔽词匹配工具
	m_pFilterWords = CreateMBCSFilter(NULL);
	int nCount = 0;
	MYSQL_ROW pRow = m_SQLConnection.CurrentRow();
	while ( pRow )
	{
		if ( pRow[0] && pRow[0][0] )
		{
			AddMBCSFilterStrToTable(m_pFilterWords, pRow[0]);
			nCount++;
		}
		pRow = m_SQLConnection.NextRow();
	}
	//释放查询结果集
	m_SQLConnection.ResetQuery();
	*/
	
	if( !InitActorId() )
	{
		return FALSE;
	}
	
	if( !InitGuildId() )
	{
		return FALSE;
	}
	//OutputMsg(rmTip, _T("%d filter name loaded"), nCount);
	return TRUE;
}

//初始化actorid
bool CNameServer::InitActorId()
{
	unsigned int nSpidMinActoridId = (unsigned int)(m_spId << 24) ; //该运营商最小的actorid

	unsigned int nDbMaxActorId =0;   //数据库里最大的角色的id
	unsigned int nFixActorId =0; //固定的这个actorid
	bool hasFixActorId =false;   //是否有固定的acttorid

	if ( m_SQLConnection.Query("select max(charid) from charnames") )
	{
		OutputMsg(rmError, _T("can not call select max(charid) from charnames"));
		return false;
	}
	else
	{
		
		bool bNeedInsert =false; //是否需要插入初始化记录
		MYSQL_ROW pRow = m_SQLConnection.CurrentRow();
		if ( pRow )
		{
			if ( pRow[0]  )
			{
				nDbMaxActorId=(unsigned int) strtoul(pRow[0],0,10);
				OutputMsg(rmTip, _T(" max actorid=%u"),nDbMaxActorId);
			}
		}
		m_SQLConnection.ResetQuery();
		nFixActorId =__max(nSpidMinActoridId,nDbMaxActorId ); //这个取2者之间的最大值
		
		
		unsigned int nCurrentFixActorId =0; //当前的数据库里的最大的id

		if( m_SQLConnection.Query("select charid from charnames where serverindex =0")  ==0)
		{
			MYSQL_ROW pRow = m_SQLConnection.CurrentRow();
			if ( pRow )
			{	
				if ( pRow[0]  )
				{
					nCurrentFixActorId=(unsigned int) strtoul(pRow[0],0,10);
					OutputMsg(rmTip, _T("nCurrentFixActorId  =%u"),nFixActorId);
					hasFixActorId =true;
				}
			}
			m_SQLConnection.ResetQuery();
		}
		else
		{
			OutputMsg(rmError, _T("select charid from charnames where serverindex =0 fail"));
			return false;
		}

		
		if(hasFixActorId ==false) //需要初始化db，往里边插入空记录
		{
			char buff[256];
			char name[32];
			sprintf(name,"__init_actorid_%u",m_spId);
			
			sprintf(buff,"insert into charnames (charid,charname,serverindex) values (%u,\"%s\",0)",(nFixActorId +1),name);
			if(m_SQLConnection.Exec(buff))
			{
				OutputMsg(rmError, _T("init actorid  spid fail"));
			}
			else
			{
				m_SQLConnection.ResetQuery();
				OutputMsg(rmTip, _T("init actorid  spid succeed, actorid=%u"),nFixActorId);
			}
			
		}
		else
		{
			if(nCurrentFixActorId < nFixActorId)
			{
				//char name[64];
				//sprintf(name,"__init_actorid_%d",m_spId);
				char buff[256];
				sprintf(buff,"update charnames set charid =%u where serverindex=0 limit 1",(nFixActorId +1));
				if(m_SQLConnection.RealExec(buff,strlen(buff)))
				{
					OutputMsg(rmError, _T("init actorid  spid fail"));
					return false;
				}
				else
				{
					m_SQLConnection.ResetQuery();
					OutputMsg(rmTip, _T("init actorid  spid succeed, actorid=%u"),nFixActorId);
				}
			}
			OutputMsg(rmTip, _T("actorid has init,max=%u"),nFixActorId);
		}
	}
	return true;

}

//初始化guildid
bool CNameServer::InitGuildId()
{
	unsigned int nSpidMinActoridId = (unsigned int)(m_spId << 24) ; //该运营商最小的actorid

	unsigned int nDbMaxActorId =0;   //数据库里最大的角色的id
	unsigned int nFixActorId =0; //固定的这个actorid
	bool hasFixActorId =false;   //是否有固定的acttorid

	if ( m_SQLConnection.Query("select max(guildid) from guildnames") )
	{
		OutputMsg(rmError, _T("can not call select max(guildid) from guildnames"));
		return false;
	}
	else
	{
		
		bool bNeedInsert =false; //是否需要插入初始化记录
		MYSQL_ROW pRow = m_SQLConnection.CurrentRow();
		if ( pRow )
		{
			if ( pRow[0]  )
			{
				nDbMaxActorId=(unsigned int) strtoul(pRow[0],0,10);
				OutputMsg(rmTip, _T(" max guildid=%u"),nDbMaxActorId);
			}
		}
		m_SQLConnection.ResetQuery();
		nFixActorId =__max(nSpidMinActoridId,nDbMaxActorId ); //这个取2者之间的最大值
		
		
		unsigned int nCurrentFixActorId =0; //当前的数据库里的最大的id

		if( m_SQLConnection.Query("select guildid from guildnames where serverindex =0")  ==0)
		{
			MYSQL_ROW pRow = m_SQLConnection.CurrentRow();
			if ( pRow )
			{	
				if ( pRow[0]  )
				{
					nCurrentFixActorId=(unsigned int) strtoul(pRow[0],0,10);
					OutputMsg(rmTip, _T("nCurrentFixGuildId  =%u"),nFixActorId);
					hasFixActorId =true;
				}
			}
			m_SQLConnection.ResetQuery();
		}
		else
		{
			OutputMsg(rmError, _T("select guildid from guildnames where serverindex =0 fail"));
			return false;
		}

		
		if(hasFixActorId ==false) //需要初始化db，往里边插入空记录
		{
			char buff[256];
			char name[32];
			sprintf(name,"__init_guild_%u",m_spId);
			
			sprintf(buff,"insert into guildnames (guildid,guildname,serverindex) values (%u,\"%s\",0)",(nFixActorId +1),name);
			if( m_SQLConnection.RealExec(buff,strlen(buff)) )
			{
				OutputMsg(rmError, _T("init guildid  spid fail"));
			}
			else
			{
				m_SQLConnection.ResetQuery();
				OutputMsg(rmTip, _T("init guildid  spid succeed, guildid=%u"),nFixActorId);
			}
			
		}
		else
		{
			if(nCurrentFixActorId < nFixActorId)
			{
				//char name[64];
				//sprintf(name,"__init_actorid_%d",m_spId);
				char buff[256];
				sprintf(buff,"update guildnames set guildid =%u where serverindex=0 limit 1",(nFixActorId +1));
				if(m_SQLConnection.RealExec(buff,strlen(buff)))
				{
					OutputMsg(rmError, _T("init guildid  spid fail"));
				}
				else
				{
					m_SQLConnection.ResetQuery();
					OutputMsg(rmTip, _T("init guildid  spid succeed, actorid=%u"),nFixActorId);
				}
			}
			OutputMsg(rmTip, _T("guildid has init,max=%u"),nFixActorId);
		}
	}
	return true;
}

VOID CNameServer::DoStop()
{
	//销毁屏蔽词
	if ( m_pFilterWords )
	{
		FreeMBCSFilter(m_pFilterWords);
		m_pFilterWords = NULL;
	}
}

bool CNameServer::hasFilterWordsInclude(LPCSTR sNameStr)
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
	return MatchMBCSFilterWord(m_pFilterWords, sNameStr, NULL) != NULL;
}

VOID CNameServer::SetSQLConfig(LPCSTR sHost, INT_PTR nPort, LPCSTR sDBName, LPCSTR sDBUser, LPCSTR sDBPass)
{
	m_SQLConnection.SetServerHost(sHost);
	m_SQLConnection.SetServerPort(nPort);
	m_SQLConnection.SetDataBaseName(sDBName);
	m_SQLConnection.SetUserName(sDBUser);
	m_SQLConnection.SetPassWord(sDBPass);
	m_SQLConnection.SetConnectionFlags(CLIENT_FOUND_ROWS | CLIENT_MULTI_RESULTS);
}
