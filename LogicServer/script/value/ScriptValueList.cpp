#include "StdAfx.h"
#include "ScriptValueList.h"

void CScriptValueList::pushArgs(lua_State *pLua, INT_PTR nCount)
{
	for (INT_PTR i=0; i<nCount; ++i)
	{
		CScriptValue &value = m_Values[i];
		switch(value.getType())
		{
		case CScriptValue::vBool:
			lua_pushboolean(pLua, value.operator bool());
			break;
		case CScriptValue::vLightData:
		case CScriptValue::vUserData:
			{
				LPCVOID ptr = value;
				if ( ptr )
					lua_pushlightuserdata(pLua, (LPVOID)ptr);
				else lua_pushnil(pLua);
			}
			break;
		case CScriptValue::vNumber:
			lua_pushnumber(pLua, value.operator double());
			break;
		case CScriptValue::vString:
			lua_pushlstring(pLua, value.operator LPCSTR(), value.getSize());
			break;
		case CScriptValue::vCFunction:
			{
				lua_CFunction pfn = value;
				if ( pfn )
					lua_pushcfunction(pLua, pfn);
				else lua_pushnil(pLua);
			}
			break;
		default:
			lua_pushnil(pLua);
			break;
		}
	}
}

INT_PTR CScriptValueList::getResults(lua_State *pLua, int nCount)
{
	INT_PTR nType, Result = 0;
	while ( nCount > 0 )
	{
		if ( m_nCount >= MaxValueCount )
			break;
		nType = lua_type(pLua, -nCount);
		switch(nType)
		{
		case LUA_TBOOLEAN: m_Values[m_nCount++] = lua_toboolean(pLua, -nCount) != FALSE; break;
		case LUA_TLIGHTUSERDATA: m_Values[m_nCount++] = lua_touserdata(pLua, -nCount); break;
		case LUA_TNUMBER: m_Values[m_nCount++] = lua_tonumber(pLua, -nCount); break;
		case LUA_TSTRING: m_Values[m_nCount++] = lua_tostring(pLua, -nCount); break;
		case LUA_TUSERDATA: m_Values[m_nCount++].SetUserData(lua_touserdata(pLua, -nCount), lua_objlen(pLua, -nCount)); break;
		default: m_Values[m_nCount++].clear(); break;
		}
		nCount--;
		Result++;
	}
	lua_pop(pLua, (int)(Result + nCount));
	return Result;
}

INT_PTR CScriptValueList::getArguments(lua_State *pLua, INT_PTR nStackIdx)
{
	INT_PTR nType, Result = 0;
	INT_PTR nCount = lua_gettop(pLua);

	clear();
	while ( nStackIdx <= nCount )
	{
		if ( m_nCount >= MaxValueCount )
			break;
		nType = lua_type(pLua, (int)nStackIdx);
		switch(nType)
		{
		case LUA_TBOOLEAN: m_Values[m_nCount++] = lua_toboolean(pLua, (int)nStackIdx) != FALSE; break;
		case LUA_TLIGHTUSERDATA: m_Values[m_nCount++] = lua_touserdata(pLua, (int)nStackIdx); break;
		case LUA_TNUMBER: m_Values[m_nCount++] = lua_tonumber(pLua, (int)nStackIdx); break;
		case LUA_TSTRING: m_Values[m_nCount++] = lua_tostring(pLua, (int)nStackIdx); break;
		case LUA_TUSERDATA: m_Values[m_nCount++].SetUserData(lua_touserdata(pLua, (int)nStackIdx), lua_objlen(pLua, (int)nStackIdx)); break;
		default: m_Values[m_nCount++].clear(); break;
		}
		nStackIdx++;
		Result++;
	}
	return Result;
}
