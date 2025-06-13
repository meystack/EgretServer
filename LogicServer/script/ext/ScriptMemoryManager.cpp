#include "StdAfx.h"
#include "ScriptMemoryManager.h"


//全局脚本内存堆
#ifdef WIN32
HANDLE g_hScriptMemHeap = NULL;
#else
BOOL g_hScriptMemHeap = false;
#endif
//总计活动内存块数量
INT64 g_nMemBlockCount = 0;
//总计活动内存大小，以字节为单位
INT64 g_nTotalMemSize = 0;

//初始化脚本内存管理器
void InitializeMemHeap()
{
	if (!g_hScriptMemHeap)
	{
#ifdef WIN32
		g_hScriptMemHeap = HeapCreate(0,20*1024*1024, 0);
#else
		g_hScriptMemHeap = true;
#endif
		if (!g_hScriptMemHeap)
			OutputError(GetLastError(), "Failed to create Script Alone Heap!");
		else g_nMemBlockCount = g_nTotalMemSize = 0;
	}
}


namespace ScriptMemoryManager
{

	//析构脚本内存管理器
	void UninitalizeMemHeap()
	{
		if (g_hScriptMemHeap)
		{
#ifdef WIN32
			HeapDestroy(g_hScriptMemHeap);
			g_hScriptMemHeap = NULL;
#else
			g_hScriptMemHeap = false;
#endif
			g_nMemBlockCount = g_nTotalMemSize = 0;
		}
	}


	void* luaAlloc(void *ud, void* ptr, size_t osize, size_t newsize)
	{
		void *result;

		//内存堆尚未初始化
		if (!g_hScriptMemHeap)
			InitializeMemHeap();
		//申请内存
		if (newsize > 0)
		{
			if (osize == 0)
			{
#ifdef WIN32
				result = HeapAlloc(g_hScriptMemHeap, 0, newsize);
#else
				result = malloc(newsize);
#endif
				//result = GetLogicServer()->GetBuffAllocator()->AllocBuffer(newsize);
				//result=malloc(newsize);
				if (result)
				{
					g_nMemBlockCount++;
					g_nTotalMemSize += newsize;
				}
			}
			else
			{
#ifdef WIN32
				result = HeapReAlloc(g_hScriptMemHeap, 0, ptr, newsize);
#else
				result = realloc(ptr, newsize);
#endif

				//result = GetLogicServer()->GetBuffAllocator()->ReallocBuffer(ptr,newsize);
				//result=realloc(ptr,newsize);
				if (result)
				{
					g_nTotalMemSize += (INT64)newsize - (INT64)osize;

				}
			}
		}
		//释放内存
		else if (newsize <= 0)
		{
			if (ptr) 
			{
#ifdef WIN32
				HeapFree(g_hScriptMemHeap, 0, ptr);
#else
				free(ptr);
#endif
				// GetLogicServer()->GetBuffAllocator()->FreeBuffer(ptr);
				//free(ptr);
				g_nTotalMemSize -= osize;
				g_nMemBlockCount--;
				if (g_nMemBlockCount <= 0)
				{
					if (g_nTotalMemSize > 0)
					{
						OutputMsg(rmError, "Script Memroy Manager Detected Memory Leaks for %lld bytes", g_nTotalMemSize);
					}
					//UninitalizeMemHeap();
				}
			}
			else
			{
				//OutputMsg(rmNormal,"ptr =NULL");
			}
			result = NULL;
		}
		return result;
	}

	INT64 getAvaliableMemBlockCount()
	{
		return g_nMemBlockCount;
	}

	INT64 getAvaliableMemSize()
	{
		return g_nTotalMemSize;
	}
}
