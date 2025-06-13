extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "lundump.h"
#include "lstate.h"
}

#ifdef WIN32
#include <Windows.h>
#include <TCHAR.h>
#endif

#include <Tick.h>
#include <stdarg.h>
#include <_ast.h>
#include <RefString.hpp>
#include "ShareUtil.h"
#include "CustomLuaScript.h"
#include "CustomLuaMyLoad.h"
#include "DataPacket.hpp"

CCustomLuaScript::CCustomLuaScript()
{
	m_pLua = NULL;
	m_nLastError = 0;
	m_sLastFnName[0] = 0;
}

CCustomLuaScript::~CCustomLuaScript()
{
	setScript(NULL);
}

bool CCustomLuaScript::setScript(LPCSTR sText)
{
	if ( m_pLua )
	{
		//调用析构函数
		callFinal();
		//销毁虚拟机
		lua_close(m_pLua);
		m_pLua = NULL;
	}

	if ( sText )
	{
		//跳过UTF-8 BOM
		if ( (*(PINT)sText & 0x00FFFFFF) == 0xBFBBEF )
			sText += 3;
	}

	if ( sText && sText[0] )
	{
		//创建虚拟机
		m_pLua = createLuaVM();
		//打开基本函数库
		openBaseLibs();
		lua_initVersion(m_pLua);
		//lua_registerMyRequire(m_pLua);
		//注册本地函数库
		registLocalLibs();
		//保存错误函数名称
		strcpy(m_sLastFnName, "<LOADER>");

		//加载脚本
		try
		{
			if ( !lcCheck(luaL_loadstring( m_pLua, sText )) )
			{
				FILE* fp = fopen("scripterror.txt", "wb");
				if (fp)
				{
					fputs(sText, fp);
					fclose(fp);
				}
				return false;
			}
		}
		catch(...)
		{
			FILE* fp = fopen("scripterror.txt", "wb");
			if (fp)
			{
				fputs(sText, fp);
				fclose(fp);
			}
			throw;
		}
		
		//初始化脚本		
		if ( !pcall( 0, 0, 0 ) )
		{
			FILE* fp = fopen("scripterror.txt", "wb");
			if (fp)
			{
				fputs(sText, fp);
				fclose(fp);
			}
			return false;	
		}	
		//调用初始化函数
		return callInit();
	}
	return true;
}

const char* CCustomLuaScript::GetCallStack()
{
	if (m_pLua)
	{
		lua_getglobal(m_pLua, "debug");
		lua_getfield(m_pLua, -1, "traceback");
		int iError = lua_pcall(m_pLua,//VMachine
			0,//Argument Count
			1,//Return Value Count
			0);
		return lua_tostring(m_pLua, -1);
	}
	return NULL;
}

int CCustomLuaScript::StreamWriter(lua_State* L, const void* p, size_t size, void* u)
{
	UNUSED(L); 
	CDataPacket *packet = (CDataPacket *)u;
	if (!packet)
		return 1;
	packet->writeBuf(p, size);
	return 0;
}

bool CCustomLuaScript::CompileLua(lua_State* L, const char* szContent, CDataPacket &packet)
{
	if (!L || !szContent)
		return false;

	if ( !lcCheck(luaL_loadstring(L, szContent)) )
		return false;

	packet.setLength(0);
	const Proto* proto = clvalue(L->top-1)->l.p;

//#ifdef WIN32
	luaU_dump(L, proto, CCustomLuaScript::StreamWriter, &packet, 0);
//#endif
	return true;
}

bool CCustomLuaScript::resetBinScript(CDataPacket& packet)
{	
	if (!m_pLua)
		return false;
	callFinal();
	if ( !lcCheck( luaL_loadbuffer(m_pLua, packet.getMemoryPtr(), packet.getPosition(), "") ) )	
		return false;
	if ( !pcall( 0, 0, 0 ) )
		return false;
	bool bRet = callInit();	
	return bRet;
}

bool CCustomLuaScript::setBinScript(LPCTSTR szScript, CDataPacket& packet, LPCTSTR name, bool bCompile)
{	
	if (m_pLua)
	{
		//调用析构函数		
		callFinal();
		//销毁虚拟机		
		lua_close(m_pLua);
		m_pLua = NULL;
	}

	if ( (bCompile && szScript) || ( (!bCompile) && (packet.getPosition() > 0) ))
	{
		//创建虚拟机			
		m_pLua = createLuaVM();
		//打开基本函数库		
		openBaseLibs();
		lua_initVersion(m_pLua);
		lua_registerMyRequire(m_pLua);
		//注册本地函数库		
		registLocalLibs();
		//保存错误函数名称
		strcpy(m_sLastFnName, "<LOADER>");		
		//加载脚本		
		if (bCompile)
		{			
			if (!CompileLua(m_pLua, szScript, packet))
				return false;		
		}		
		if ( !lcCheck( luaL_loadbuffer(m_pLua, packet.getMemoryPtr(), packet.getPosition(), name) ) )	
			return false;				
		//初始化脚本		
		if ( !pcall( 0, 0, 0 ) )
			return false;		
		//调用初始化函数		
		return callInit();
	}
	return true;
}

int CCustomLuaScript::getAvaliableMemorySize()
{
	if ( !m_pLua ) return 0;
	int n = lua_gc(m_pLua , LUA_GCCOUNT, 0) * 1024;
	n |= lua_gc( m_pLua, LUA_GCCOUNTB, 0 );
	return n;
}

int CCustomLuaScript::gc()
{
	if ( !m_pLua ) return 0;
	int n = getAvaliableMemorySize();
	lua_gc(m_pLua , LUA_GCCOLLECT, 0);
	return n - getAvaliableMemorySize();
}

lua_State* CCustomLuaScript::createLuaVM()
{
	return luaL_newstate();
}

bool CCustomLuaScript::openBaseLibs()
{
	if ( !lcCheck( lua_cpcall( m_pLua, luaopen_base, NULL ) ) )
		return false;
	if ( !lcCheck( lua_cpcall( m_pLua, luaopen_string, NULL ) ) )
		return false;
	if ( !lcCheck( lua_cpcall( m_pLua, luaopen_math, NULL ) ) )
		return false;
	if ( !lcCheck( lua_cpcall( m_pLua, luaopen_table, NULL ) ) )
		return false;
	if ( !lcCheck( lua_cpcall( m_pLua, luaopen_io, NULL ) ) )
		return false;
	if ( !lcCheck( lua_cpcall( m_pLua, luaopen_os, NULL ) ) )
		return false;
	if ( !lcCheck( lua_cpcall( m_pLua, luaopen_debug, NULL ) ) )
		return false;
	if ( !lcCheck( lua_cpcall( m_pLua, luaopen_package, NULL ) ) )
		return false;
	return true;
}

bool CCustomLuaScript::registLocalLibs()
{
	return true;
}

bool CCustomLuaScript::callInit()
{
	return true;
}

bool CCustomLuaScript::callFinal()
{
	return true;
}

void CCustomLuaScript::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	_tprintf(sError);
}

void CCustomLuaScript::showErrorFormat(LPCTSTR sFmt, ...)
{
	TCHAR sBuf[1024];
	va_list	args;

	va_start(args, sFmt);
	_vsntprintf(sBuf, ArrayCount(sBuf) - 1, sFmt, args);
	va_end(args);

	showError(sBuf);
}

bool CCustomLuaScript::lcCheck(int nError)
{
	if ( !nError )
		return true;
	m_nLastError = nError;
	
	TCHAR sErrDesc[256];
	LPCTSTR pErrDesc = NULL;
    if ( lua_gettop( m_pLua ) > 0 )
	{
#ifdef UNICODE
		wylib::string::CAnsiString sError = lua_tostring(m_pLua, -1 );
		wylib::string::CWideString swError;
		sError.toWStr(&swError);
		_asncpy(sErrDesc, swError);
#else
		pErrDesc = lua_tostring( m_pLua, -1 );
		_asncpy(sErrDesc, pErrDesc);
#endif
		lua_pop( m_pLua, 1 );
	}
    else pErrDesc = _T("falt system error: lua_gettop <= 0");
    if ( !pErrDesc || !pErrDesc[0] )
		pErrDesc = _T("undefined error");


	//std::string strLastFnName = m_sLastFnName;
	//std::string strErrDesc = pErrDesc; 
	showErrorFormat(_T("function:%s,error:%s"), m_sLastFnName, pErrDesc );
	return false;
}

bool CCustomLuaScript::pcall(const int args, const int results, const int nErrFunc)
{
	bool result = true;

	int nTop = lua_gettop( m_pLua ) - args - 1;//-1是排除函数名称所占的栈空间
	result = lcCheck(lua_pcall(m_pLua, args, results, nErrFunc));
	int nTop2 = lua_gettop( m_pLua ) - results;

	if ( results != LUA_MULTRET && nTop != nTop2 )
	{
		showErrorFormat(_T("function:%s,the stack before call was:%d,the stack after call is:%d,stack difference value is:%d"), m_sLastFnName, nTop, nTop2, nTop2 - nTop);
	}
	return result;
}
