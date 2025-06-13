#include "StdAfx.h"
#include "LuaHlp.h"

using namespace wylib::stream;

VOID LuaHelp::PushDataPointerToTable(lua_State *L, LPCVOID lpData, SIZE_T sizeElement, SIZE_T elementCount)
{
	if (elementCount > 0)
	{
		LPCBYTE pBytes = (LPCBYTE)lpData;

		lua_createtable(L, (int)elementCount, 0);
		for (SIZE_T i=1; i<=elementCount; ++i)
		{
			lua_pushlightuserdata( L, (LPVOID)pBytes );
			lua_rawseti( L, -2, (int)i );
			pBytes += sizeElement;
		}
	}
	else lua_pushnil(L);
}

VOID LuaHelp::PushDataPointerToTable( lua_State *L, void** lpData, SIZE_T elementCount )
{
	if (elementCount > 0)
	{

		lua_createtable(L, (int)elementCount, 0);
		for (SIZE_T i=1; i<=elementCount; ++i)
		{
			lua_pushlightuserdata( L, (LPVOID)(*lpData) );
			lua_rawseti( L, -2, (int)i );
			lpData++;
		}
	}
	else lua_pushnil(L);
}



VOID LuaHelp::PushStringsToTable(lua_State *L, LPCSTR *ppStrList, SIZE_T strCount)
{
	if (strCount > 0)
	{
		lua_createtable(L, (int)strCount, 0);
		for (SIZE_T i=1; i<=strCount; ++i)
		{
			lua_pushstring( L, *ppStrList );
			lua_rawseti( L, -2, (int)i );
			ppStrList++;
		}
	}
	else lua_pushnil(L);
}

VOID WriteIdentChars(CBaseStream &stm, INT_PTR nIdent)
{
	static const LPCSTR sIdentStrs[] = { NULL, 
		"  ", 
		"    ", 
		"      ",
		"        ", 
		"          ", 
		"            ", 
		"              ", 
		"                ", 
		"                  ", 
		"                    ", 
		"                      ", 
		"                        ", 
		"                          ", 
		"                            ", 
		"                              ", 
		"                                ", 
	};
	INT_PTR nCount;
	while (nIdent > 0)  
	{
		nCount = nIdent & 15;
		stm.write(sIdentStrs[nCount], nCount * 2);
		nIdent -= nCount;
	}
}

SIZE_T WriteTableA(lua_State *L, INT_PTR stackId, CBaseStream &stm, INT_PTR nIdent)
{
	INT64 dwPos = stm.getPosition();
	LPCSTR sKey, sValue;
	INT_PTR nValueType, nFieldIndex = 1, nTableLen;

	WriteIdentChars(stm, nIdent);
	stm.write("{\r\n",3);

	nTableLen = lua_objlen(L, (int)stackId);
	if (stackId < 0) stackId--;

	lua_pushnil(L); /* first key */
	if (lua_next(L, (int)stackId)) 
	{
		nIdent++;

		do 
		{
			nValueType = lua_type(L, -1);

			if ( lua_type(L, -2) == LUA_TSTRING )
			{
				WriteIdentChars(stm, nIdent);
				sKey = lua_tostring(L, -2);
				stm.write(sKey, strlen(sKey));
				stm.write(" = ", 3);
			}
			else  
			{
				sKey = NULL;

				//为纯数组中的bool、number以及string写缩进字符串
				if ( nFieldIndex == 1 )
				{
					switch(nValueType)
					{
					case LUA_TBOOLEAN:
					case LUA_TNUMBER:
					case LUA_TSTRING:
						if (!sKey)
							WriteIdentChars(stm, nIdent);
						break;
					}
				}
			}

			switch(nValueType)
			{
			case LUA_TBOOLEAN:
				if ( lua_toboolean(L, -1) )
					stm.write( "true", 4 );
				else stm.write( "false", 5 );
				break;
			case LUA_TNUMBER:
				sValue = lua_tostring(L, -1);
				stm.write(sValue, strlen(sValue));
				break;
			case LUA_TSTRING:
				sValue = lua_tostring(L, -1);
				stm.write("\"", 1);
				stm.write(sValue, strlen(sValue));
				stm.write("\"", 1);
				break;
			case LUA_TTABLE:
				if ( sKey )
					stm.write("\r\n", 2);
				WriteTableA(L, -1, stm, nIdent);
				break;
			default:
				stm.write("nil", 3);
				break;
			}

			lua_pop(L, 1);
			nFieldIndex++;

			if (sKey || nValueType == LUA_TTABLE || nFieldIndex > nTableLen)
				stm.write(",\r\n", 3);
			else stm.write(", ", 2);
		} 
		while (lua_next(L, (int)stackId));

		nIdent--;
	}
	WriteIdentChars(stm, nIdent);
	stm.write("}", 1);
	return (SIZE_T)(stm.getPosition() - dwPos);
}

SIZE_T LuaHelp::FormatTableA(lua_State *L, INT_PTR stackId, CBaseStream &stm)
{
	if ( !lua_istable(L, (int)stackId) )
		return 0;

	return WriteTableA(L, stackId, stm, 0);
}

