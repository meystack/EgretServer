#include "stdafx.h"

CProduct::CProduct(PlatForm* config)
{
	SetConfig(config);
	if (!SetDBConnection())
	{
		OutputMsg( rmError, _T("连接MYSQL数据库(%s:%d)失败"),config->db,config->port);
		bAMEngineStartSucceed = false;
	}
	else
	{
		OutputMsg( rmTip, _T("连接MYSQL数据库(%s:%d)成功"),config->db,config->port);
	}
}
void CProduct::SetConfig(PlatForm* config)
{
	m_Config = config;
	_asncpy(m_spid,m_Config->spid);
	_asncpy(m_code,m_Config->code);
}
BOOL CProduct::CheckConnection()
{
	SYSTEMTIME	m_CurSysTime;
	GetLocalTime(&m_CurSysTime);
	CMiniDateTime CurMiniDateTime;
	CurMiniDateTime = CurMiniDateTime.encode(m_CurSysTime);
	BOOL boConnected = m_Mysql.Connected();
	if (!boConnected || CurMiniDateTime.tv - m_ConnectMiniDateTime.tv > 60*60)
	{
		m_Mysql.Disconnect();
		boConnected = m_Mysql.Connect();
		if( boConnected )
		{
			if( mysql_set_character_set(m_Mysql.GetMySql(),m_Config->bianma) )
				OutputMsg( rmError, _T("设置utf8编码出错 !!!") );
		}
	}
	return boConnected;
}

BOOL CProduct::SetDBConnection()
{
	m_Mysql.SetServerHost(m_Config->host);
	m_Mysql.SetServerPort(m_Config->port);
	m_Mysql.SetDataBaseName(m_Config->db);
	m_Mysql.SetUserName(m_Config->user);
	m_Mysql.SetPassWord(m_Config->pass);
	//多个客户端连接，数据库操作类要加锁
	//m_Mysql.SetMultiThread(TRUE);
	m_Mysql.SetConnectionFlags(CLIENT_FOUND_ROWS | CLIENT_MULTI_RESULTS);
	BOOL boConnected = m_Mysql.Connect();
	if (boConnected)
	{
		if( mysql_set_character_set(m_Mysql.GetMySql(),m_Config->bianma) )
			OutputMsg( rmError, _T("设置utf8编码出错 !!!") );
		OutputMsg(rmTip, _T("mysql connection character set: %s"), mysql_character_set_name(m_Mysql.GetMySql()));
		SYSTEMTIME	m_CurSysTime;
		GetLocalTime(&m_CurSysTime);
		m_ConnectMiniDateTime = m_ConnectMiniDateTime.encode(m_CurSysTime);
	}
	return boConnected;
}

int CProduct::GetContractID(const char * sContractID )
{
	size_t len = strlen(sContractID);
	if( len <= 0 )   return 0;

	for(int i=0; i<m_Config->taskcount; ++i)
	{
		if(strncmp(m_Config->constractid[i], sContractID, len) == 0 )
		{
			return (i+1);
		}
	}
	return 0;
}

int CProduct::Pay( const char* sOPID,const char* sAccount,UINT nMoney,double dRmb,UINT ServerIndex,UINT nType,UINT &uUserId,UINT nActorId, const char * actorname, int level, const char * channel)
{
	if (!CheckConnection())
	{
		return ERROR_NOT_CONNECT_DB;
	}
	int nResult = ERROR_QUERY_DB;
	m_UserDBLock.Lock();
	int nErrorId = m_Mysql.Query(sPay_SQL, sOPID,sAccount,nMoney,dRmb,ServerIndex,m_Config->spid,nType, nActorId, actorname, level,channel);
	if (!nErrorId)
	{
		MYSQL_ROW pRow = m_Mysql.CurrentRow();
		if (pRow)
		{
			sscanf(pRow[0], "%d", &nResult);
			if( pRow[1] )
				sscanf(pRow[1], "%u", &uUserId);
		}
		m_Mysql.ResetQuery();
	}
	m_UserDBLock.Unlock();
	return nResult;
}

int CProduct::QueryAmount(UINT uUserId, UINT& nActorId)
{
	if (!CheckConnection())
	{
		return ERROR_NOT_CONNECT_DB;
	}
	int nResult = ERROR_QUERY_DB;
	m_UserDBLock.Lock();
	
	int nErrorId = m_Mysql.Query(sQuery_SQL, uUserId, 0, nActorId);
	if (!nErrorId)
	{
		MYSQL_ROW pRow = m_Mysql.CurrentRow();
		if (pRow)
		{
		   //查角色或账号元宝，nActorId > 0,则为角色元宝，nActorId=0则为账号元宝
		   sscanf(pRow[0], "%d", &nResult);
		   sscanf(pRow[1], "%d", &nActorId);
		}
		m_Mysql.ResetQuery();
	}
	else
	{
		OutputMsg( rmError,sError_Query_SQL, uUserId, 0 );
	}
	m_UserDBLock.Unlock();
	return nResult;
}

int CProduct::Consume(UINT uUserId,UINT nAmount,char* sCharName,UINT nServerId, UINT nActorId, BYTE nLevel)
{
	if (!CheckConnection())
	{
		return ERROR_NOT_CONNECT_DB;
	}
	int nResult = ERROR_QUERY_DB;
	m_UserDBLock.Lock();
	int nErrorId = m_Mysql.Query(sConsume_SQL, uUserId, nAmount, nActorId,sCharName,nServerId, nLevel);
	if (!nErrorId)
	{
		MYSQL_ROW pRow = m_Mysql.CurrentRow();
		if (pRow)
		{
			int nTmp = -1;
			sscanf(pRow[0], "%d", &nTmp);
			//int nRowCount= m_Mysql.GetFieldCount();

			//for(int i=0; i< nRowCount; i++)
			//{
			//	OutputMsg( rmTip, "param [%d =%d]", i ,atoi(pRow[i])) ;
			//}


			if (nTmp == 0)
			{
				sscanf(pRow[1], "%d", &nResult);
			}
			else if(nTmp == -1 )
			{
				OutputMsg( rmTip, "余额不足" ) ;
			}
		}
		m_Mysql.ResetQuery();
	}
	else
	{
		OutputMsg( rmError,_T(sError_Consume_SQL), uUserId, nAmount,sCharName,nServerId );
	}
	m_UserDBLock.Unlock();
	return nResult;
}
