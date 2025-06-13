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
	static const DWORD SmallBufferSize	= 64;	//С���ڴ���С
	static const DWORD MiddleBufferSize	= 256;	//�е��ڴ���С
	static const DWORD LargeBufferSize	= 1024;	//����ڴ���С

#pragma pack(push, 1)
	typedef struct tagBuffer
	{
		size_t		dwSize;		//�ڴ����ݶδ�С
		tagBuffer*	pPrevBuffer;//ָ����һ���ڴ���ָ��
		TICKCOUNT	dwFreeTick;	//���������ٵ�ʱ��
#ifdef _BUFFER_TRACE
		tagBuffer*	pNextBuffer;//ָ�����¸��ڴ���ָ�룬���ڵ���
		struct
		{
			const char* lpFile;		//�����ڴ��Ĵ���λ�ã����ڵ���
			int nLine;
		}al, fl;
		BOOL		boUsing;	//�Ƿ�ʹ�õı��
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
	//CRITICAL_SECTION	m_BufferLock;			//�ڴ��������
	wylib::sync::lock::CCSLock                 m_BufferLock;                   //����

	PBUFFER				m_pLastSmallBuffer;		//ָ�����һ�����е�С���ڴ���ָ��
	PBUFFER				m_pLastMiddleBuffer;	//ָ�����һ�����е������ڴ���ָ��
	PBUFFER				m_pLastLargeBuffer;		//ָ�����һ�����еĴ����ڴ���ָ��
	PBUFFER				m_pLastSuperBuffer;		//ָ�����һ�����еĳ����ڴ���ָ��
#ifdef _BUFFER_TRACE
	PBUFFER				m_pLastAllocedBuffer;	//ָ�����һ����������ڴ��
#endif
	size_t				m_dwAllocedSmallSize;	//�ѱ������С���ڴ���С
	size_t				m_dwAllocedMiddleSize;	//�ѱ�����������ڴ���С
	size_t				m_dwAllocedLargeSize;	//�ѱ�����Ĵ����ڴ���С
	size_t				m_dwAllocedSuperSize;	//�ѱ�����ĳ����ڴ���С
	size_t				m_dwFreedSmallSize;		//�ѱ��ͷŵ�С���ڴ���С
	size_t				m_dwFreedMiddleSize;	//�ѱ��ͷŵ������ڴ���С
	size_t				m_dwFreedLargeSize;		//�ѱ��ͷŵĴ����ڴ���С
	size_t				m_dwFreedSuperSize;		//�ѱ��ͷŵĳ����ڴ���С
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
	// bIgnoreInterval:�Ƿ���Լ�����������ʱ�����������Ƿ�ʱ����������ǿ��ɾ��
	VOID CheckFreeBuffers(bool bIgnoreInterval = false);
};

class DataBuffer
{
public:
	CBufferAllocator *m_pAllocator;
	size_t			  m_dwSize;
	char			 *pData;//�ڴ��ָ��
	char			 *ptr;//д�ڴ�ָ��
	char			 *pEnd;//�ڴ�����ָ��
	char			 *pReadPtr;//���ڴ�ָ��
	
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
