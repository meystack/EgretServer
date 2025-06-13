#pragma once

/************************************************************************/
/* 
/*                提供默认的lua虚拟机内存管理函数的虚拟机内存管理类
/* 
/************************************************************************/
class CLogicLuaAllocator
{
public:
	//初始化静态内存管理器
	static VOID InitializeAllocator();
	//析构静态内存管理器
	static VOID UninitializeAllocator();
	//检查并释放内存
	static VOID CheckMemory();
	//创建虚拟机对象
	static lua_State* createLuaVM();

protected:
	static LPVOID alloc(void *ud, void *ptr, size_t osize, size_t nsize);

protected:
	static CBufferAllocator *s_pBufferAllocator;	//统一的脚本内存管理器
	static LONG				s_nAllocatorInitCount;//内存管理器初始化计数
};
