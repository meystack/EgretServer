#pragma once


/************************************************************************/
/*
/*						    脚本内存管理器
/*
/*                 目前的实现方式是采用单独的堆进行内存管理。
/*
/************************************************************************/

namespace ScriptMemoryManager
{
	//LUA虚拟机内存申请函数
	void* luaAlloc(void *ud, void* ptr, size_t osize, size_t newsize);
	//获取已申请内存块数量
	INT64 getAvaliableMemBlockCount();
	//获取已申请内存总字节大小
	INT64 getAvaliableMemSize();

	 void UninitalizeMemHeap();

}