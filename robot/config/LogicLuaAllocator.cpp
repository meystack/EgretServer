#include "stdafx.h"
#include "LogicLuaAllocator.h"


CBufferAllocator* CLogicLuaAllocator::s_pBufferAllocator = NULL;	//统一的脚本内存管理器
LONG CLogicLuaAllocator::s_nAllocatorInitCount = 0;//内存管理器初始化计数

//初始化静态内存管理器
VOID CLogicLuaAllocator::InitializeAllocator()
{
	if (1 == InterlockedIncrement(&s_nAllocatorInitCount))
	{
		s_pBufferAllocator = new CBufferAllocator();
	}
}

VOID CLogicLuaAllocator::CheckMemory()
{
	//SF_TIME_CHECK(); //检测性能	
	s_pBufferAllocator->CheckFreeBuffers();
}

//析构静态内存管理器
VOID CLogicLuaAllocator::UninitializeAllocator()
{
	if (0 == InterlockedDecrement(&s_nAllocatorInitCount))
	{
		delete s_pBufferAllocator;
		s_pBufferAllocator = NULL;
	}
}

lua_State* CLogicLuaAllocator::createLuaVM()
{
	return s_pBufferAllocator 
		? lua_newstate((lua_Alloc)alloc, s_pBufferAllocator)
		: luaL_newstate();
}

LPVOID CLogicLuaAllocator::alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
	CBufferAllocator *pAllocator = (CBufferAllocator*)ud;
	return pAllocator->ReallocBuffer(ptr, nsize);
}


bool CCustomLogicLuaConfig::registLocalLibs()
{
	//CLogicServer *pLogicServer = GetLogicServer();
	CRobotManager* pRM = GetRobotMgr();

	if (pRM)
	{
		//导出CLangTextSection类
		LuaLangTextSection::regist(m_pLua);

		//导出语言包实例
		LuaLangTextSection::setGlobalValue(m_pLua, pRM->GetTextProvider(), "Lang");
	}

	return true;
}
