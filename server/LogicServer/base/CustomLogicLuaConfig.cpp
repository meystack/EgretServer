/*
#include "StdAfx.h"
#include "CustomLogicLuaConfig.h"


CBufferAllocator* CCustomLogicLuaConfig::s_pBufferAllocator = NULL;	//统一的脚本内存管理器
LONG CCustomLogicLuaConfig::s_nAllocatorInitCount = 0;//内存管理器初始化计数

//初始化静态内存管理器
VOID CCustomLogicLuaConfig::InitializeAllocator()
{
	if (1 == InterlockedIncrement(&s_nAllocatorInitCount))
	{
		s_pBufferAllocator = new CBufferAllocator();
	}
}

//析构静态内存管理器
VOID CCustomLogicLuaConfig::UninitializeAllocator()
{
	if (0 == InterlockedDecrement(&s_nAllocatorInitCount))
	{
		delete s_pBufferAllocator;
		s_pBufferAllocator = NULL;
	}
}

lua_State* CCustomLogicLuaConfig::createLuaVM()
{
	return s_pBufferAllocator 
		? lua_newstate((lua_Alloc)alloc, s_pBufferAllocator)
		: luaL_newstate();
}

LPVOID CCustomLogicLuaConfig::alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
	CBufferAllocator *pAllocator = (CBufferAllocator*)ud;
	return pAllocator->ReallocBuffer(ptr, nsize);
}
*/