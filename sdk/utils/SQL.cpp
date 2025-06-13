#include <stdio.h>

#ifdef WIN32
	#include <tchar.h>
	#include <config-win.h>
#else
	#include <stdarg.h>
#endif

#include "SQL.h"
#include <Tick.h>
#include <wyString.h>
#include "ShareUtil.h"

CSQLConenction::CSQLConenction()
{
	ZeroMemory( &m_MySQL, sizeof(m_MySQL) );

	m_pSqlResult = NULL;
	m_uSqlRows = 0;
	m_pSqlRows = NULL;
	m_pSqlFields = NULL;
	m_uSqlFields = NULL;

	m_sServerHost[0] = 0;
	m_nServerPort = 3306;
	m_sUserName[0] = 0;
	m_sPassWord[0] = 0;
	m_sDataBase[0] = 0;
	m_nConnectFlags = CLIENT_FOUND_ROWS | CLIENT_MULTI_RESULTS;

	m_sQueryBuffer[0] = 0;
	//InitializeCriticalSection( &m_QueryLock );

	m_boConnected = FALSE;
	m_boMultiThread = FALSE;
}

CSQLConenction::~CSQLConenction()
{
	Disconnect();
	//DeleteCriticalSection( &m_QueryLock );
}

VOID CSQLConenction::SetServerHost(const char *sServerHost)
{
	ZeroMemory( m_sServerHost, sizeof(m_sServerHost) );
	strncpy( m_sServerHost, sServerHost, sizeof(m_sServerHost) - 1 );
}

VOID CSQLConenction::SetServerPort(INT_PTR nServerPort)
{
	m_nServerPort = nServerPort;
}

VOID CSQLConenction::SetDataBaseName(const char *sDataBaseName)
{
	ZeroMemory( m_sDataBase, sizeof(m_sDataBase) );
	strncpy( m_sDataBase, sDataBaseName, sizeof(m_sDataBase) - 1 );
}

VOID CSQLConenction::SetUserName(const char* sUserName)
{
	ZeroMemory( m_sUserName, sizeof(m_sUserName) );
	strncpy( m_sUserName, sUserName, sizeof(m_sUserName) - 1 );
}

VOID CSQLConenction::SetPassWord(const char* sPassWord)
{
	ZeroMemory( m_sPassWord, sizeof(m_sPassWord) );
	strncpy( m_sPassWord, sPassWord, sizeof(m_sPassWord) - 1 );
}

VOID CSQLConenction::SetConnectionFlags(const UINT_PTR nFlags)
{
	m_nConnectFlags = nFlags;
}

VOID CSQLConenction::SetMultiThread(const BOOL boMultiThread)
{
	m_boMultiThread = boMultiThread;
}


BOOL CSQLConenction::Connect()
{
	BOOL Result = TRUE;

	if ( !m_boConnected )
	{
		if ( mysql_init(&m_MySQL) )
		{
			m_MySQL.reconnect = TRUE;
			if (mysql_real_connect(&m_MySQL, m_sServerHost, m_sUserName, m_sPassWord, m_sDataBase, (UINT)m_nServerPort,NULL, (UINT)m_nConnectFlags) )
			{
				m_boConnected = TRUE;
				//ResetQuery();
			}
			else
			{
				Result = FALSE;
				OutputMsg( rmError, _T("无法连接到数据库：%s"), mysql_error(&m_MySQL) );
				mysql_close(&m_MySQL);
			}
		}
		else
		{
			OutputMsg( rmError, _T("无法初始化数据库连接程序") );
			Result	= FALSE;
		}
	}

	return	Result;
}

VOID CSQLConenction::Disconnect()
{
	if (m_boConnected)
	{
		m_boConnected = FALSE;
		ResetQuery();
		mysql_close(&m_MySQL);
	}
}

VOID CSQLConenction::AfterQueryed(int nError)
{
	if (!nError)
	{
		m_pSqlResult = mysql_store_result(&m_MySQL);
		if (m_pSqlResult)
		{
			mysql_data_seek( m_pSqlResult, 0 );
			mysql_field_seek( m_pSqlResult, 0 );
			m_uSqlRows	= mysql_num_rows(m_pSqlResult);
			m_uSqlFields	= mysql_num_fields(m_pSqlResult);
			m_pSqlRows    = mysql_fetch_row(m_pSqlResult);
		}
		else
		{
			m_uSqlFields = 0;
			m_pSqlFields = NULL;
			m_uSqlRows = 0;
			m_pSqlRows = NULL;
			m_sQueryBuffer[128] = 0;
			OutputMsg(rmError, _T("%s(%s)"), mysql_error(&m_MySQL), m_sQueryBuffer );
		}
	}
	else
	{
		m_uSqlFields = 0;
		m_pSqlFields = NULL;
		m_uSqlRows = 0;
		m_pSqlRows = NULL;
		m_pSqlResult = NULL;
#ifdef UNICODE
		wylib::string::CAnsiString as(mysql_error(&m_MySQL));
		wylib::string::CWideString *ws = as.toWStr();
		OutputMsg(rmError, *ws);
		delete ws;
#else
		OutputMsg(rmError, mysql_error(&m_MySQL));
#endif
		if ( m_boMultiThread )  m_QueryLock.Unlock();
			// LeaveCriticalSection( &m_QueryLock );
	}
}

VOID CSQLConenction::AfterExeced(int nError)
{
	if (!nError)
	{
		m_pSqlResult = NULL;
		m_uRowsAffected = mysql_affected_rows( &m_MySQL );
	}
	else
	{
		m_uSqlFields = 0;
		m_pSqlFields = NULL;
		m_uSqlRows = 0;
		m_pSqlRows = NULL;
		m_pSqlResult = NULL;
		m_uRowsAffected = 0;
#ifdef UNICODE
		wylib::string::CAnsiString as(mysql_error(&m_MySQL));
		wylib::string::CWideString *ws = as.toWStr();
		OutputMsg(rmError, *ws);
		delete ws;
#else
		OutputMsg(rmError, mysql_error(&m_MySQL));
#endif
		if ( m_boMultiThread )   m_QueryLock.Unlock();
			//LeaveCriticalSection( &m_QueryLock );
	}
}

int CSQLConenction::Query(const char* sQueryFormat, ...)
{
	int	Result;
	va_list	args;
	//OutputMsg(rmError,_T("Query :%s, CurrentThreadId[%d]"),sQueryFormat,GetCurrentThreadId());
	if ( m_boMultiThread )   m_QueryLock.Lock();
	// EnterCriticalSection( &m_QueryLock );

	va_start(args, sQueryFormat);
	Result = vsprintf(m_sQueryBuffer, sQueryFormat, args);
	va_end(args);

	Result = mysql_real_query(&m_MySQL, m_sQueryBuffer, Result);
	AfterQueryed( Result );

	return	Result;
}

int CSQLConenction::RealQuery(const char* sQueryText, const size_t nTextLen)
{
	int	Result;

	//if ( m_boMultiThread ) EnterCriticalSection( &m_QueryLock );
	OutputMsg(rmError,_T("RealQuery :%s, CurrentThreadId[%d]"),sQueryText,GetCurrentThreadId());
	if ( m_boMultiThread )   m_QueryLock.Lock();

	Result = mysql_real_query(&m_MySQL, sQueryText, (UINT)nTextLen);
	AfterQueryed( Result );

	return	Result;
}

int CSQLConenction::Exec(const char* sQueryFormat, ...)
{
	int	Result;
	va_list	args;

	//if ( m_boMultiThread ) EnterCriticalSection( &m_QueryLock );
	//OutputMsg(rmError,_T("Exec CurrentThreadId[%d]"),GetCurrentThreadId());
	if ( m_boMultiThread )   m_QueryLock.Lock();

	va_start(args, sQueryFormat);
	Result = vsprintf(m_sQueryBuffer, sQueryFormat, args);
	va_end(args);

	Result = mysql_real_query(&m_MySQL, m_sQueryBuffer, Result);
	AfterExeced( Result );

	return	Result;
}

int CSQLConenction::RealExec(const char* sExecText, const size_t nTextLen)
{
	int	Result;

	//if ( m_boMultiThread ) EnterCriticalSection( &m_QueryLock );
	//OutputMsg(rmError,_T("RealExec CurrentThreadId[%d]"),GetCurrentThreadId());
	if ( m_boMultiThread )   m_QueryLock.Lock();

	Result = mysql_real_query(&m_MySQL, sExecText, (UINT)nTextLen);
	AfterExeced( Result );

	return	Result;
}

VOID CSQLConenction::ResetQuery()
{
	if (m_pSqlResult)
	{
		mysql_free_result(m_pSqlResult);
		//Commands out of sync; you can't run this command now
		//get back all results
		while (!mysql_next_result(&m_MySQL));
	}

	m_uSqlFields = 0;
	m_pSqlFields = NULL;
	m_uSqlRows = 0;
	m_pSqlRows = NULL;
	m_pSqlResult = NULL;

	//if ( m_boMultiThread ) LeaveCriticalSection( &m_QueryLock );
	//OutputMsg(rmError,_T("ResetQuery CurrentThreadId[%d]"),GetCurrentThreadId());
	if ( m_boMultiThread )   m_QueryLock.Unlock();
}
