#ifndef _MSG_ALLOCATOR_H_
#define	_MSG_ALLOCATOR_H_

/*
#include <_osdef.h>
#include <Tick.h>
#include <string.h>

#include "Lock.h"


#include "Lock.h"

#ifdef _DEBUG
#define _BUFFER_TRACE
#endif

using namespace wylib::time::tick64;

class CBufferAllocator
{

public:
	static const DWORD SmallBufferSize	= 64;	//小块内存块大小
	static const DWORD MiddleBufferSize	= 256;	//中等内存块大小
	static const DWORD LargeBufferSize	= 1024;	//大块内存块大小

#pragma pack(push, 1)
	typedef struct tagBuffer
	{
		size_t		dwSize;		//内存数据段大小
		tagBuffer*	pPrevBuffer;//指向上一个内存块的指针
		TICKCOUNT	dwFreeTick;	//即将被销毁的时间
#ifdef _BUFFER_TRACE
		tagBuffer*	pNextBuffer;//指向上下个内存块的指针，用于调试
		struct
		{
			const char* lpFile;		//申请内存块的代码位置，用于调试
			int nLine;
		}al, fl;
		BOOL		boUsing;	//是否使用的标记
#endif
	}BUFFER, *PBUFFER;
#pragma pack(pop)

	struct BufferStatic
	{
		size_t		dwAllocSize;
		size_t		dwFreeSize;
	};

	typedef struct tagAllocatorMemoryInfo
	{
		BufferStatic	SmallBuffer;
		BufferStatic	MiddleBuffer;
		BufferStatic	LargeBuffer;
		BufferStatic	SuperBuffer;
	}ALLOCATOR_MEMORY_INFO, *PALLOCATPR_MEMORY_INFOR;

private:
	//CRITICAL_SECTION	m_BufferLock;			//内存块申请锁
	wylib::sync::lock::CCSLock                 m_BufferLock;                   //加锁

	PBUFFER				m_pLastSmallBuffer;		//指向最后一个空闲的小型内存块的指针
	PBUFFER				m_pLastMiddleBuffer;	//指向最后一个空闲的中型内存块的指针
	PBUFFER				m_pLastLargeBuffer;		//指向最后一个空闲的大型内存块的指针
	PBUFFER				m_pLastSuperBuffer;		//指向最后一个空闲的超级内存块的指针
#ifdef _BUFFER_TRACE
	PBUFFER				m_pLastAllocedBuffer;	//指向最后一个被申请的内存块
#endif
	size_t				m_dwAllocedSmallSize;	//已被申请的小型内存块大小
	size_t				m_dwAllocedMiddleSize;	//已被申请的中型内存块大小
	size_t				m_dwAllocedLargeSize;	//已被申请的大型内存块大小
	size_t				m_dwAllocedSuperSize;	//已被申请的超级内存块大小
	size_t				m_dwFreedSmallSize;		//已被释放的小型内存块大小
	size_t				m_dwFreedMiddleSize;	//已被释放的中型内存块大小
	size_t				m_dwFreedLargeSize;		//已被释放的大型内存块大小
	size_t				m_dwFreedSuperSize;		//已被释放的超级内存块大小
public:
	CBufferAllocator();
	~CBufferAllocator();

#ifndef _BUFFER_TRACE
	PVOID AllocBuffer(size_t dwSize);
	PVOID ReallocBuffer(LPVOID ptr, size_t dwNewSize);
	VOID FreeBuffer(PVOID ptr);
#else
	PVOID _AllocBuffer(size_t dwSize, const char* lpFile, int nLine);
	PVOID _ReallocBuffer(LPVOID ptr, size_t dwNewSize, const char* lpFile, int nLine);
	VOID _FreeBuffer(PVOID ptr, const char* lpFile, int nLine);
#define AllocBuffer(dwSize) _AllocBuffer(dwSize, __FILE__, __LINE__)
#define ReallocBuffer(ptr, dwNewSize) _ReallocBuffer(ptr, dwNewSize, __FILE__, __LINE__)
#define FreeBuffer(ptr) _FreeBuffer(ptr, __FILE__, __LINE__)
#endif

	VOID GetMemoryInfo(CBufferAllocator::PALLOCATPR_MEMORY_INFOR pMemoryInfo);
	// bIgnoreInterval:是否忽略间隔。如果忽略时间间隔，不管是否时间间隔到，都强制删除
	VOID CheckFreeBuffers(bool bIgnoreInterval = false);
};

class DataBuffer
{
public:
	CBufferAllocator *m_pAllocator;
	size_t			  m_dwSize;
	char			 *pData;//内存块指针
	char			 *ptr;//写内存指针
	char			 *pEnd;//内存块结束指针
	char			 *pReadPtr;//读内存指针
	
#ifdef _BUFFER_TRACE
	DataBuffer(CBufferAllocator *pAllocator, size_t dwSize, const char *_FILE, const int _LINE);
#else
	DataBuffer(CBufferAllocator *pAllocator, size_t dwSize);
#endif
	~DataBuffer();
};

#ifdef _BUFFER_TRACE
#define DATA_BUFFER_VAR(_allocator, _name, _size)	DataBuffer _name((_allocator), (_size), __FILE__, __LINE__)
#else
#define DATA_BUFFER_VAR(_allocator, _name, _size)	DataBuffer _name((_allocator), (_size))
#endif
*/

#endif
