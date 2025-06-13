#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#include <Windows.h>
#include <tchar.h>


#include <_ast.h>
#include <_memchk.h>
#include "sqlite3.h"
#include "../include/SQLiteDB.h"

CSQLiteDB::CSQLiteDB()
{
	m_pDB = NULL;
}

CSQLiteDB::~CSQLiteDB()
{
	close();
}

int CSQLiteDB::copen(const char * sDBFile)
{
	sqlite3 *pDB;
	//打开数据库
	int nErr = sqlite3_open(sDBFile, &pDB);
	return checkOpenResult(nErr, pDB);
}

int CSQLiteDB::wopen(const wchar_t * wsDBFile)
{
	sqlite3 *pDB;
	//打开数据库
	int nErr = sqlite3_open16(wsDBFile, &pDB);
	return checkOpenResult(nErr, pDB);
}

void CSQLiteDB::close()
{
	if (m_pDB)
	{
		sqlite3_close(m_pDB);
		m_pDB = NULL;
	}
}

unsigned short CSQLiteDB::getPageSize()
{
	if (!opened()) 
		return 0;
	CSQLiteStatement stmt(this, "PRAGMA page_size;");
	if (stmt.step())
		return stmt.val_int(0);
	else return 0;
}

int CSQLiteDB::getErrorCode()
{
	if (!opened())
		return 0;
	return sqlite3_errcode(m_pDB);
}

const wchar_t * CSQLiteDB::getErrorMessage()
{
	if (!opened())
		return NULL;
	return (const wchar_t *)sqlite3_errmsg16(m_pDB);
}

int CSQLiteDB::checkOpenResult(int nErr, sqlite3 *pdb)
{
	if (nErr == SQLITE_OK)
	{
		//打开数据成功则关闭现有数据库并保持新的数据库连接对象
		close();
		m_pDB = pdb;
	}
	return nErr;
}
#endif
