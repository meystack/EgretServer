#pragma once


/************************************************************************
 
                  LnagTextSection向脚本导出的注册以及实现
 
************************************************************************/

namespace LuaLangTextSection
{
	//将CLangTextSection类导出到脚本
	void regist(lua_State *L);
	//将CLangTextSection类作为指定的类名导出到脚本
	void registTo(lua_State *L, const char *className);
	//向脚本返回CLangTextSection实例
	int returnValue(lua_State *L, CLangTextSection &section);
	//向脚本设置CLangTextSection实例为全局变量
	void setGlobalValue(lua_State *L, CLangTextSection &section, const char* sGlobalName);
}
