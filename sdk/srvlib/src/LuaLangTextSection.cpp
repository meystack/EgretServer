#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#include <Windows.h>
#include <tchar.h>
#endif

#include <time.h>

#include <_ast.h>
#include <_memchk.h>
extern "C"
{
#include <lua.h>
#include <ltm.h>
};
#include "../include/CLVariant.h"
#include <new>
#include "../include/ObjectAllocator.hpp"
#include "../include/CustomHashTable.h"
#include "../include/LangTextSection.h"
#include "../include/LuaLangTextSection.h"

namespace CLangTextSectionExport
{
	static const unsigned int RegistedTypeId = CCLVariant::hashstr("CLangTextSection");
	static const unsigned int RegistedRefTypeId = CCLVariant::hashstr("CLangTextSectionRef");

	static CLangTextSection* getDataPtr(lua_State *L, int index = 1)
	{
		if (lua_type(L, index) != LUA_TUSERDATA)
			return NULL;
		CLangTextSection *result = NULL;
		/*
		int lua_getmetatable (lua_State *L, int index);
		把给定索引指向的值的元表压入堆栈。如果索引无效，或是这个值没有元表，函数将返回 0 并且不会向栈上压任何东西。
		*/
		if (lua_getmetatable(L, index))
		{
			lua_pushlstring(L, "_typeid", 7);
			lua_rawget(L, -2);
			if (lua_isnumber(L, -1))
			{
				unsigned int tpid = (unsigned int)lua_tonumber(L, -1);
				if (RegistedRefTypeId == tpid)
					result = *(CLangTextSection**)lua_touserdata(L, (index > 0) ? index : index - 2);
				else if (RegistedTypeId == tpid)
					result = (CLangTextSection*)lua_touserdata(L, (index > 0) ? index : index - 2);
			}
			//弹出_typeid以及metatable
			lua_pop(L, 2);
		}
		return result;
	}
	static void setType(lua_State *L, const char *classeName, int tidx = -1)
	{
		lua_getglobal(L, classeName);
		if (lua_istable(L, -1))
			lua_setmetatable(L, (tidx > 0) ? tidx : tidx - 1);
		else lua_pop(L, 1);
	}
	static int newRef(lua_State *L, CLangTextSection &section)
	{
		CLangTextSection **pSection = (CLangTextSection**)lua_newuserdata(L, sizeof(*pSection));
		//设置类型表
		setType(L, "CLangTextSectionRef");
		*pSection = &section;
		return 1;
	}
	static void setGlobal(lua_State *L, CLangTextSection &section, const char* sGlobalName)
	{
		CLangTextSection **pSection = (CLangTextSection**)lua_newuserdata(L, sizeof(*pSection));
		//设置类型表
		setType(L, "CLangTextSectionRef");
		*pSection = &section;
		//设为全局变量
		lua_setglobal(L, sGlobalName);
	}
	static int get(lua_State *L)
	{
		CLangTextSection *pSection = getDataPtr(L);
		if (!pSection)
			return 0;

		const char* sName = lua_tostring(L, 2);
		pSection = pSection->getSection(sName);
		if (!pSection)
			return 0;

		//返回字符串
		if (pSection->getType() == CLangTextSection::eText)
		{
			lua_pushstring(L, pSection->getText());
			return 1;
		}
		//返回文字包引用
		return newRef(L, *pSection);
	}

	static void setMetaNameClosure(lua_State *L, const char* metaName, const void* fn, int tidx = -1)
	{  
		lua_pushstring(L, metaName);
		lua_pushcfunction(L, (lua_CFunction)fn);
		lua_rawset(L, (tidx < 0) ? tidx - 2 : tidx);
	}

	static void registTo(lua_State *L, const char *className)
	{
		lua_createtable(L, 0, 3);
		//typename
		lua_pushlstring(L, "_typename", 9);
		lua_pushstring(L, className);
		lua_rawset(L, -3);
		//typeid
		lua_pushlstring(L, "_typeid", 7);
		lua_pushnumber(L, CCLVariant::hashstr(className));
		lua_rawset(L, -3);
		//__index
		setMetaNameClosure(L, "__index", (const void *)get);

		lua_setglobal(L, className);
	}

	void regist(lua_State *L)
	{
		registTo(L, "CLangTextSectionRef");
	}
}

namespace LuaLangTextSection
{
	void regist(lua_State *L)
	{
		CLangTextSectionExport::regist(L);
	}

	void registTo(lua_State *L, const char *className)
	{
		CLangTextSectionExport::registTo(L, className);
	}

	int returnValue(lua_State *L, CLangTextSection &section)
	{
		return CLangTextSectionExport::newRef(L, section);
	}

	void setGlobalValue(lua_State *L, CLangTextSection &section, const char* sGlobalName)
	{
		return CLangTextSectionExport::setGlobal(L, section, sGlobalName);
	}
}
