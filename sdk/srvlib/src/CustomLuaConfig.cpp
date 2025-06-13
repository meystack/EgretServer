extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#ifdef WIN32
	#include <Windows.h>
	#include <TCHAR.h>
#endif

#include <_ast.h>
#include <Tick.h>
#include <List.h>
#include <stdarg.h>
#include <wyString.h>
#include <RefClass.hpp>
#include <RefString.hpp>
#include "ShareUtil.h"
#include "CustomLuaScript.h"
#include "CustomLuaConfig.h"

#include "Define.h"

using namespace wylib::string;

CCustomLuaConfig::CCustomLuaConfig()
	:Inherited()
{
}

CCustomLuaConfig::~CCustomLuaConfig()
{
	INT_PTR i;
	for ( i=m_TableStacks.count()-1; i>-1; --i )
	{
		m_TableStacks[i]->release();
	}
	m_TableStacks.clear();
}

void CCustomLuaConfig::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	String sErr = _T("[Configuration Error]");
	sErr += sError;
	_tprintf(sErr);
}

void CCustomLuaConfig::showTableNotExists(LPCSTR sTableName)
{
	LPCTSTR sTableNamePtr;
	String s;
	getTablePath(s);

#ifndef UNICODE
	sTableNamePtr = sTableName;
#else
	CAnsiString cs = sTableName;
	CWideString ws;
	cs.toWStr(&ws);
	sTableNamePtr = ws;
#endif

	LPCTSTR sFmt = (sTableName && !s.isEmpty())
		? _T("table \"%s.%s\" not exists")
		: _T("table \"%s%s\" not exists");
	showErrorFormat(sFmt, (LPCTSTR)s, sTableNamePtr);
}
	
void CCustomLuaConfig::showFieldDoesNotContainValue(LPCSTR sFieldName, LPCTSTR sValueType)
{
	LPCTSTR sFieldNamePtr;
	String s;
	getTablePath(s);
	
#ifndef UNICODE
	sFieldNamePtr = sFieldName;
#else
	CAnsiString cs = sFieldName;
	CWideString ws;
	cs.toWStr(&ws);
	sFieldNamePtr = ws;
#endif

	LPCTSTR sFmt = (sFieldName && !s.isEmpty())
		? _T("field \"%s.%s\" does not contain valid %s value")
		: _T("field \"%s%s\" does not contain valid %s value");
	showErrorFormat(sFmt, (LPCTSTR)s, sFieldNamePtr, sValueType);
}

CCustomLuaConfig::TableName* CCustomLuaConfig::createTableName(LPCSTR sName)
{
	TableName *tn = new TableName();
#ifdef UNICODE
	CAnsiString as = sName;
	as.toWStr(tn);
#else
	*((String*)tn) = sName;
#endif
	tn->addRef();
	return tn;
}

bool CCustomLuaConfig::openGlobalTable(LPCSTR sTableName)
{
	lua_getglobal(m_pLua, sTableName);
	bool result = lua_istable( m_pLua, -1 );
	if ( !result )
	{
		lua_pop( m_pLua, 1 );
		showTableNotExists(sTableName);
	}
	else
	{
		m_TableStacks.add(createTableName(sTableName));
	}
	return result;
}

bool CCustomLuaConfig::openFieldTable(LPCSTR sTableName)
{
	lua_getfield(m_pLua, -1, sTableName);
	bool result = lua_istable( m_pLua, -1 );
	if ( !result )
	{
		lua_pop( m_pLua, 1 );
		showTableNotExists(sTableName);
	}
	else
	{
		m_TableStacks.add(createTableName(sTableName));
	}
	return result;
}

bool CCustomLuaConfig::enumTableFirst()
{
	lua_pushnil(m_pLua);  /* first key */
	bool result = lua_next(m_pLua, -2) != 0;
	if ( result )
	{
		m_TableStacks.add(createTableName("[1]"));
	}
	return result;
}

bool CCustomLuaConfig::enumTableNext()
{
	lua_pop(m_pLua, 1);	/* remove value, reserve key for next iterate */
	bool result = lua_next(m_pLua, -2) != 0;   
	INT_PTR nListIndex = m_TableStacks.count()-1;
	//如果表存在则改变表名称列表中的表名称，表不存在则完成遍历并从表名称列表中删除当前的枚举表名称
	if ( result )
	{
		int nTableIndex;
		TableName *tn = m_TableStacks[nListIndex];
		_stscanf((LPCTSTR)*((String*)tn), _T("[%d]"), &nTableIndex );
		nTableIndex++;
		tn->format(_T("[%d]"), nTableIndex);
	}
	else
	{
		m_TableStacks[nListIndex]->release();
		m_TableStacks.remove(nListIndex);
	}
	return result;
}

void CCustomLuaConfig::endTableEnum()
{
	INT_PTR nListIndex = m_TableStacks.count()-1;
	if ( nListIndex > -1 )
	{
		TableName *tn = m_TableStacks[nListIndex];
		if ( !tn->isEmpty() && tn->rawStr()[0] == '[' )
		{
			m_TableStacks[nListIndex]->release();
			m_TableStacks.remove(nListIndex);
			lua_pop(m_pLua, 2);  /* remove value and key */
		}
		else
		{
			showErrorFormat(_T("table is not under enumeration"));
		}
	}
	else
	{
		showErrorFormat(_T("table stack was empty"));
	}
}

bool CCustomLuaConfig::getFieldIndexTable(const int IndexStartFromOne)
{
	lua_pushinteger( m_pLua, IndexStartFromOne );
	lua_rawget( m_pLua, -2 );
	bool result = lua_istable( m_pLua, -1 );
	if ( !result )
	{
		lua_pop( m_pLua, 1 );

		String s;
		getTablePath(s);
		LPCTSTR sFmt = !s.isEmpty()
			? _T("table \"%s.[%d]\" does not exists")
			: _T("table \"%s[%d]\" does not exists");
		showErrorFormat(sFmt, (LPCTSTR)s, IndexStartFromOne);
	}
	else
	{
		CHAR s[64];
		sprintf(s, "[%d]", IndexStartFromOne);
		m_TableStacks.add(createTableName(s));
	}
	return result;
}

bool CCustomLuaConfig::feildTableExists(LPCSTR sTableName)
{
	lua_getfield(m_pLua, -1, sTableName);
	bool result = lua_istable( m_pLua, -1 );
	lua_pop(m_pLua, 1);
	return result;
}

bool CCustomLuaConfig::globalTableExists(LPCSTR sTableName)
{
	lua_getglobal(m_pLua, sTableName);
	bool result = lua_istable( m_pLua, -1 );
	lua_pop( m_pLua, 1 );
	return result;
}

void CCustomLuaConfig::closeTable()
{
	if ( lua_gettop(m_pLua) > 0 )
	{
		lua_pop(m_pLua, 1);
		INT_PTR nIndex = m_TableStacks.count() - 1;
		m_TableStacks[nIndex]->release();
		m_TableStacks.remove(nIndex);
	}
	else
	{
		showErrorFormat(_T("table stack was empty"));
	}
}

String& CCustomLuaConfig::getTablePath(String& Retval)
{
	INT_PTR i, nCount;
	Retval = _T("");

	nCount = m_TableStacks.count();
	if ( nCount > 0 )
	{
		for ( i=0; i<nCount-1; ++i )
		{
			Retval += m_TableStacks[i]->rawStr();
			Retval += _T(".");
		}
		Retval += m_TableStacks[nCount-1]->rawStr();
	}
	return Retval;
}

CAnsiString& CCustomLuaConfig::getKeyName(CAnsiString& Retval)
{
	/*
		★attention★
			不得使用lua_tostring来直接获取键名称！
		因为在遍历数组表的时候，键的类型为number，这个时候
		使用lua_tostring会将键的数据类型修改为string，从而
		导致后续的遍历将无法进行！！！
	*/
	switch ( lua_type(m_pLua, -2) )
	{
	case LUA_TNUMBER:
		{
			lua_Integer n = lua_tointeger(m_pLua, -2) - 1;//LUA中数组索引是从1开始，基于CBP规范，数组索引应当从0开始。
			Retval.format("%d", (int)n);
		}
		break;
	case LUA_TSTRING:
		{
			LPCSTR s = lua_tostring(m_pLua, -2);
			Retval = s;
		}
		break;
	default:
		Retval = NULL;
		break;
	}
	return Retval;
}

bool CCustomLuaConfig::getFieldBoolean(LPCSTR sFieldName, const bool *pDefValue, BOOL* pIsValid)
{
	if ( sFieldName )
	{
		lua_getfield(m_pLua, -1, sFieldName);
	}

	bool result;
	BOOL isValid = lua_isboolean( m_pLua, -1 );
	if (pIsValid) *pIsValid = isValid;
	if ( isValid )
	{
		result = lua_toboolean( m_pLua, -1 ) != 0;
	}
	else if ( pDefValue )
	{
		result = *pDefValue;
	}
	else
	{
		result = false;
		showFieldDoesNotContainValue(sFieldName, _T("boolean"));
	}

	if ( sFieldName )
	{
		lua_pop(m_pLua, 1);
	}
	return result;
}

DOUBLE CCustomLuaConfig::getFieldNumber(LPCSTR sFieldName, const DOUBLE *pDefValue, BOOL* pIsValid)
{
	if ( sFieldName )
	{
		lua_getfield(m_pLua, -1, sFieldName);
	}

	DOUBLE result;
	BOOL isValid = lua_isnumber( m_pLua, -1 );
	if (pIsValid) *pIsValid = isValid;
	if ( isValid )
	{
		result = lua_tonumber( m_pLua, -1 );
	}
	else if ( pDefValue )
	{
		result = *pDefValue;
	}
	else 
	{
		result = 0;
		showFieldDoesNotContainValue(sFieldName, _T("numeric"));
	}

	if ( sFieldName )
	{
		lua_pop(m_pLua, 1);
	}
	return result;
}

INT64 CCustomLuaConfig::getFieldInt64(LPCSTR sFieldName, const PINT64 pDefValue, BOOL* pIsValid)
{
	if ( pDefValue )
	{
		DOUBLE defValue = (DOUBLE)*pDefValue;
		return (INT64)getFieldNumber(sFieldName, &defValue, pIsValid);
	}
	return (INT64)getFieldNumber(sFieldName, NULL, pIsValid);
}

INT CCustomLuaConfig::getFieldInt(LPCSTR sFieldName, const PINT pDefValue, BOOL* pIsValid)
{
	if ( pDefValue )
	{
		DOUBLE defValue = *pDefValue;
		return (INT)getFieldNumber(sFieldName, &defValue, pIsValid);
	}
	return (INT)getFieldNumber(sFieldName, NULL, pIsValid);
}

LPCSTR CCustomLuaConfig::getFieldString(LPCSTR sFieldName, const LPCSTR pDefValue, BOOL* pIsValid)
{
	if ( sFieldName )
	{
		lua_getfield(m_pLua, -1, sFieldName);
	}

	LPCSTR result;
	BOOL isValid = lua_isstring( m_pLua, -1 );
	if (pIsValid) *pIsValid = isValid;
	if ( isValid )
	{
		result = lua_tostring( m_pLua, -1 );
	}
	else if ( pDefValue )
	{
		result = pDefValue;
	}
	else
	{
		result = NULL;
		showFieldDoesNotContainValue(sFieldName, _T("string"));
	}

	if ( sFieldName )
	{
		lua_pop(m_pLua, 1);
	}
	return result;
}

INT_PTR CCustomLuaConfig::getFieldStringBuffer(LPCSTR sFieldName, OUT LPSTR sBuffer, IN SIZE_T dwBufLen, const LPCSTR sDefBuff)
{
	LPCSTR s;
	BOOL boIsValid;
	if ( dwBufLen <= 0 )
		return -1;

	s = getFieldString(sFieldName, sDefBuff, &boIsValid);
	if ( !boIsValid )
		return false;

	SIZE_T dwLen = strlen(s);
	dwBufLen--;
	dwLen = __min(dwLen, dwBufLen);

	memcpy(sBuffer, s, dwLen * sizeof(sBuffer[0]));
	sBuffer[dwLen] = 0;
	return dwLen;
}


bool CCustomLuaConfig::getIndexBoolean(LUA_INTEGER _index, const bool *pDefValue, BOOL* pIsValid)
{
	lua_pushinteger(m_pLua, _index);
	lua_gettable(m_pLua, -2);

	bool result;
	BOOL isValid = lua_isboolean( m_pLua, -1 );
	if (pIsValid) *pIsValid = isValid;
	if ( isValid )
	{
		result = lua_toboolean( m_pLua, -1 ) != 0;
	}
	else if ( pDefValue )
	{
		result = *pDefValue;
	}
	else
	{
		result = NULL;
		char str[25];
		sprintf(str, "%x", _index);
		showFieldDoesNotContainValue(str, _T("boolean"));
	}

	lua_pop(m_pLua, 1);//Value
	return result;
}

DOUBLE CCustomLuaConfig::getIndexNumber(LUA_INTEGER _index, const DOUBLE *pDefValue, BOOL* pIsValid)
{
	lua_pushinteger(m_pLua, _index);
	lua_gettable(m_pLua, -2);

	DOUBLE result;
	BOOL isValid = lua_isnumber( m_pLua, -1 );
	if (pIsValid) *pIsValid = isValid;
	if ( isValid )
	{
		result = lua_tonumber( m_pLua, -1 );
	}
	else if ( pDefValue )
	{
		result = *pDefValue;
	}
	else 
	{
		result = NULL;
		char str[25];
		sprintf(str, "%x", _index);
		showFieldDoesNotContainValue(str, _T("numeric"));
	}

	lua_pop(m_pLua, 1);
	return result;
}

INT64 CCustomLuaConfig::getIndexINT64(LUA_INTEGER _index, const PINT pDefValue, BOOL* pIsValid)
{
	if ( pDefValue )
	{
		DOUBLE defValue = (DOUBLE)*pDefValue;
		return (INT64)getIndexNumber(_index, &defValue, pIsValid);
	}
	return (INT64)getIndexNumber(_index, NULL, pIsValid);
}

INT CCustomLuaConfig::getIndexInt(LUA_INTEGER _index, const PINT pDefValue, BOOL* pIsValid)
{
	if ( pDefValue )
	{
		DOUBLE defValue = *pDefValue;
		return (INT)getIndexNumber(_index, &defValue, pIsValid);
	}
	return (INT)getIndexNumber(_index, NULL, pIsValid);
}

LPCSTR CCustomLuaConfig::getIndexString(LUA_INTEGER _index, const LPCSTR pDefValue, BOOL* pIsValid)
{
	lua_pushinteger(m_pLua, _index);
	lua_gettable(m_pLua, -2);

	LPCSTR result;
	BOOL isValid = lua_isstring( m_pLua, -1 );
	if (pIsValid) *pIsValid = isValid;
	if ( isValid )
	{
		result = lua_tostring( m_pLua, -1 );
	}
	else if ( pDefValue )
	{
		result = pDefValue;
	}
	else
	{
		result = NULL;
		char str[25];
		sprintf(str, "%x", _index);
		showFieldDoesNotContainValue(str, _T("string"));
	}

	lua_pop(m_pLua, 1);
	return result;
}

INT_PTR CCustomLuaConfig::getIndexStringBuffer(LUA_INTEGER _index, OUT LPSTR sBuffer, IN SIZE_T dwBufLen)
{
	LPCSTR s;
	BOOL boIsValid;
	if ( dwBufLen <= 0 )
		return -1;

	s = getIndexString(_index, NULL, &boIsValid);
	if ( !boIsValid )
		return false;

	SIZE_T dwLen = strlen(s);
	dwBufLen--;
	dwLen = __min(dwLen, dwBufLen);

	memcpy(sBuffer, s, dwLen * sizeof(sBuffer[0]));
	sBuffer[dwLen] = 0;
	return dwLen;
}

bool CCustomLuaConfig::isExistString(LPCSTR sFieldName)
{
	if ( sFieldName )
	{
		lua_getfield(m_pLua, -1, sFieldName);
		BOOL isValid = lua_isstring( m_pLua, -1 );
		lua_pop(m_pLua, 1);
		if (isValid) return true;
	}
	return false;
}
