#include <stdlib.h>
#include <stdio.h>
#ifdef WIN32
	#include <Windows.h>
	#include <tchar.h>
#endif
#include <_ast.h>
#include <_memchk.h>
#include <Tick.h>
#include "Lock.h"
#include "ShareUtil.h"
#include "BufferAllocator.h"



static const DWORD dwBufferFreeTick = 30 * 1000;//标记为释放的的内存块的延时销毁时间

CBufferAllocator::CBufferAllocator()
{
	m_pLastSmallBuffer = NULL;
	m_pLastMiddleBuffer = NULL;
	m_pLastLargeBuffer = NULL;
	m_pLastSuperBuffer = NULL;
#ifdef _BUFFER_TRACE
	m_pLastAllocedBuffer = NULL;
#endif
	m_dwAllocedSmallSize = 0;
	m_dwAllocedMiddleSize = 0;
	m_dwAllocedLargeSize  = 0;
	m_dwAllocedSuperSize = 0;
	m_dwFreedSmallSize = 0;
	m_dwFreedMiddleSize = 0;
	m_dwFreedLargeSize = 0;
	m_dwFreedSuperSize = 0;
	//InitializeCriticalSection( &m_BufferLock );
}

CBufferAllocator::~CBufferAllocator()
{
	PBUFFER pBuffer, pPrevBuffer;
	
	pBuffer = m_pLastSmallBuffer;
	while ( pBuffer )
	{
		pPrevBuffer = pBuffer->pPrevBuffer;
		free( pBuffer );
		pBuffer = pPrevBuffer;
	}

	pBuffer = m_pLastMiddleBuffer;
	while ( pBuffer )
	{
		pPrevBuffer = pBuffer->pPrevBuffer;
		free( pBuffer );
		pBuffer = pPrevBuffer;
	}

	pBuffer = m_pLastLargeBuffer;
	while ( pBuffer )
	{
		pPrevBuffer = pBuffer->pPrevBuffer;
		free( pBuffer );
		pBuffer = pPrevBuffer;
	}

	pBuffer = m_pLastSuperBuffer;
	while ( pBuffer )
	{
		pPrevBuffer = pBuffer->pPrevBuffer;
		free( pBuffer );
		pBuffer = pPrevBuffer;
	}

#ifdef _BUFFER_TRACE
	pBuffer = m_pLastAllocedBuffer;
	while ( pBuffer )
	{
		pPrevBuffer = pBuffer->pPrevBuffer;
		TRACE( _T("%s(%d) : 申请的内存块没有释放！pointer Addr=%d\n"), pBuffer->al.lpFile, pBuffer->al.nLine,(INT_PTR)pBuffer );
		free( pBuffer );
		pBuffer = pPrevBuffer;
		//Assert(FALSE);//有内存泄漏，提醒
	}
#endif
	//DeleteCriticalSection( &m_BufferLock );
}

#ifndef _BUFFER_TRACE
PVOID CBufferAllocator::AllocBuffer(size_t dwSize)
#else
PVOID CBufferAllocator::_AllocBuffer(size_t dwSize, const char *lpFile, int nLine)
#endif
{
	if(m_dwFreedSuperSize >100000000 || m_dwFreedLargeSize > 100000000)
	{
		CheckFreeBuffers(true);
	}
	PBUFFER pBuffer, pPrev, pResult = NULL;

	//EnterCriticalSection( &m_BufferLock );
	m_BufferLock.Lock();

	if ( dwSize <= SmallBufferSize )
	{
		if ( m_pLastSmallBuffer )
		{
			pResult = m_pLastSmallBuffer;
			m_pLastSmallBuffer = pResult->pPrevBuffer;
			pResult->pPrevBuffer = NULL;
			m_dwFreedSmallSize -= SmallBufferSize;
		}
		else 
		{
#ifdef _BUFFER_TRACE
			pResult = (PBUFFER)malloc( SmallBufferSize + sizeof(*pResult) + sizeof(INT) );
#else
			pResult = (PBUFFER)malloc( SmallBufferSize + sizeof(*pResult) );
#endif
			pResult->dwSize = SmallBufferSize;
			pResult->pPrevBuffer = NULL;
#ifdef _BUFFER_TRACE
            pResult->boUsing = FALSE;
            pResult->pNextBuffer = NULL;
#endif
		}
		m_dwAllocedSmallSize += SmallBufferSize;
	}
	else if ( dwSize <= MiddleBufferSize )
	{
		if ( m_pLastMiddleBuffer )
		{
			pResult = m_pLastMiddleBuffer;
			m_pLastMiddleBuffer = pResult->pPrevBuffer;
			pResult->pPrevBuffer = NULL;
			m_dwFreedMiddleSize -= MiddleBufferSize;
		}
		else 
		{
#ifdef _BUFFER_TRACE
			pResult = (PBUFFER)malloc( MiddleBufferSize + sizeof(*pResult) + sizeof(INT) );
#else
			pResult = (PBUFFER)malloc( MiddleBufferSize + sizeof(*pResult) );
#endif
			pResult->dwSize = MiddleBufferSize;
			pResult->pPrevBuffer = NULL;
#ifdef _BUFFER_TRACE
            pResult->boUsing = FALSE;
            pResult->pNextBuffer = NULL;
#endif
		}
		m_dwAllocedMiddleSize += MiddleBufferSize;
	}
	else if ( dwSize <= LargeBufferSize )
	{
		if ( m_pLastLargeBuffer )
		{
			pResult = m_pLastLargeBuffer;
			m_pLastLargeBuffer = pResult->pPrevBuffer;
			pResult->pPrevBuffer = NULL;
			m_dwFreedLargeSize -= LargeBufferSize;
		}
		else 
		{
#ifdef _BUFFER_TRACE
			pResult = (PBUFFER)malloc( LargeBufferSize + sizeof(*pResult) + sizeof(INT) );
#else
			pResult = (PBUFFER)malloc( LargeBufferSize + sizeof(*pResult) );
#endif
			pResult->dwSize = LargeBufferSize;
			pResult->pPrevBuffer = NULL;
#ifdef _BUFFER_TRACE
            pResult->boUsing = FALSE;
            pResult->pNextBuffer = NULL;
#endif
		}
		m_dwAllocedLargeSize += LargeBufferSize;
	}
	else
	{
        pPrev = NULL;
		pBuffer = m_pLastSuperBuffer;
		while ( pBuffer )
		{
			if ( pBuffer->dwSize >= dwSize )
			{
				pResult = pBuffer;
#ifdef _BUFFER_TRACE
                Assert( !pResult->boUsing );
#endif
                if ( pResult == m_pLastSuperBuffer )
				{
					m_pLastSuperBuffer = pResult->pPrevBuffer;
					pResult->pPrevBuffer = NULL;
				}
                else  if ( pPrev )
                {
                    pPrev->pPrevBuffer = pResult->pPrevBuffer;
                    pResult->pPrevBuffer = NULL;
                }
				m_dwFreedSuperSize -= pResult->dwSize;
				break;
            }
            pPrev = pBuffer;
			pBuffer = pBuffer->pPrevBuffer;
		}
		if ( !pResult )
		{
			dwSize = (dwSize + 511) & (~511);
#ifdef _BUFFER_TRACE
			pResult = (PBUFFER)malloc( dwSize + sizeof(*pResult) + sizeof(INT) );
#else
			pResult = (PBUFFER)malloc( dwSize + sizeof(*pResult) );
#endif
			pResult->dwSize = dwSize;
			pResult->pPrevBuffer = NULL;
#ifdef _BUFFER_TRACE
            pResult->boUsing = FALSE;
            pResult->pNextBuffer = NULL;
#endif
		}
		m_dwAllocedSuperSize += pResult->dwSize;
	}

#ifdef _BUFFER_TRACE
	if ( pResult )
	{
		Assert( !pResult->boUsing );
		pResult->boUsing = TRUE;
		pResult->al.lpFile = lpFile;
		pResult->al.nLine = nLine;
        pResult->fl.lpFile = NULL;
        pResult->fl.nLine = 0;
        pResult->dwFreeTick = 0;
		
		*((PINT)((char*)(pResult + 1) + pResult->dwSize)) = 0xCC55EE33;

		if ( m_pLastAllocedBuffer ) m_pLastAllocedBuffer->pNextBuffer = pResult;
		pResult->pPrevBuffer = m_pLastAllocedBuffer;
		pResult->pNextBuffer = NULL;
		m_pLastAllocedBuffer = pResult;
	}
#endif
	//LeaveCriticalSection( &m_BufferLock );
	m_BufferLock.Unlock();

	return pResult ? pResult + 1 : NULL;
}


#ifndef _BUFFER_TRACE
PVOID CBufferAllocator::ReallocBuffer(PVOID ptr, size_t dwNewSize)
#else
PVOID CBufferAllocator::_ReallocBuffer(PVOID ptr, size_t dwNewSize, const char* lpFile, int nLine)
#endif
{
	if (dwNewSize == 0)
	{
		FreeBuffer(ptr);
		return NULL;
	}

	PBUFFER pBuffer;

	if (ptr)
	{
		pBuffer = ((PBUFFER)ptr) - 1;
		if (dwNewSize <= pBuffer->dwSize)
			return ptr;
	}

	PVOID pResult;
#ifndef _BUFFER_TRACE
	pResult = AllocBuffer(dwNewSize);
	if (ptr) 
	{
		memcpy(pResult, ptr, pBuffer->dwSize);
		FreeBuffer(ptr);
	}
#else
	pResult = _AllocBuffer(dwNewSize, lpFile, NULL);
	if (ptr)
	{
		memcpy(pResult, ptr, pBuffer->dwSize);
		_FreeBuffer(ptr, lpFile, NULL);
	}
#endif
	return pResult;
}

#ifndef _BUFFER_TRACE
VOID CBufferAllocator::FreeBuffer(PVOID ptr)
#else
VOID CBufferAllocator::_FreeBuffer(PVOID ptr, const char* lpFile, int nLine)
#endif
{
	PBUFFER pBuffer;

	if (!ptr) return;

	pBuffer = ((PBUFFER)ptr) - 1;

	//EnterCriticalSection( &m_BufferLock );
	m_BufferLock.Lock();
#ifdef _BUFFER_TRACE
	if ( !pBuffer->boUsing )
	{
		//TRACE( _T(__FUNCTION__)_T(" 试图释放已经被标记为释放的内存块(%X,大小:%d)，申请位置：%s(%d)，释放位置：%s(%d)\n"), 
		//	pBuffer, pBuffer->dwSize, pBuffer->al.lpFile, pBuffer->al.nLine,
		//	pBuffer->fl.lpFile, pBuffer->fl.nLine );
		OutputMsg(rmError, _T(" %s试图释放已经被标记为释放的内存块(%X,大小:%d)，申请位置：%s(%d)，释放位置：%s(%d)\n"), 
			__FUNCTION__,pBuffer, pBuffer->dwSize, pBuffer->al.lpFile, pBuffer->al.nLine,
			pBuffer->fl.lpFile, pBuffer->fl.nLine );
	}
	Assert( pBuffer->boUsing );
	pBuffer->boUsing = FALSE;
	Assert(*((PINT)((char*)(pBuffer + 1) + pBuffer->dwSize)) == 0xCC55EE33);

	if ( pBuffer->pPrevBuffer )
		pBuffer->pPrevBuffer->pNextBuffer = pBuffer->pNextBuffer;
	if ( pBuffer->pNextBuffer )
		pBuffer->pNextBuffer->pPrevBuffer = pBuffer->pPrevBuffer;
	if ( pBuffer == m_pLastAllocedBuffer )
		m_pLastAllocedBuffer = pBuffer->pPrevBuffer;
    
    pBuffer->pPrevBuffer = NULL;
    pBuffer->pNextBuffer = NULL;
	pBuffer->fl.lpFile = lpFile;
	pBuffer->fl.nLine = nLine;
#endif

	pBuffer->dwFreeTick = _getTickCount() + dwBufferFreeTick;

	if ( pBuffer->dwSize == SmallBufferSize )
	{
		pBuffer->pPrevBuffer = m_pLastSmallBuffer;
		m_pLastSmallBuffer = pBuffer;
		m_dwAllocedSmallSize -= SmallBufferSize;
		m_dwFreedSmallSize += SmallBufferSize;
	}
	else if ( pBuffer->dwSize == MiddleBufferSize )
	{
		pBuffer->pPrevBuffer = m_pLastMiddleBuffer;
		m_pLastMiddleBuffer = pBuffer;
		m_dwAllocedMiddleSize -= MiddleBufferSize;
		m_dwFreedMiddleSize += MiddleBufferSize;
	}
	else if ( pBuffer->dwSize == LargeBufferSize )
	{
		pBuffer->pPrevBuffer = m_pLastLargeBuffer;
		m_pLastLargeBuffer = pBuffer;
		m_dwAllocedLargeSize -= LargeBufferSize;
		m_dwFreedLargeSize += LargeBufferSize;
	}
	else
	{
		pBuffer->pPrevBuffer = m_pLastSuperBuffer;
		m_pLastSuperBuffer = pBuffer;
		m_dwAllocedSuperSize -= pBuffer->dwSize;
		m_dwFreedSuperSize += pBuffer->dwSize;
	}

	//LeaveCriticalSection( &m_BufferLock );
	m_BufferLock.Unlock();
}

VOID CBufferAllocator::GetMemoryInfo(CBufferAllocator::PALLOCATPR_MEMORY_INFOR pMemoryInfo)
{
	pMemoryInfo->SmallBuffer.dwAllocSize = m_dwAllocedSmallSize;
	pMemoryInfo->SmallBuffer.dwFreeSize = m_dwFreedSmallSize;
	pMemoryInfo->MiddleBuffer.dwAllocSize = m_dwAllocedMiddleSize;
	pMemoryInfo->MiddleBuffer.dwFreeSize = m_dwFreedMiddleSize;
	pMemoryInfo->LargeBuffer.dwAllocSize = m_dwAllocedLargeSize;
	pMemoryInfo->LargeBuffer.dwFreeSize = m_dwFreedLargeSize;
	pMemoryInfo->SuperBuffer.dwAllocSize = m_dwAllocedSuperSize;
	pMemoryInfo->SuperBuffer.dwFreeSize = m_dwFreedSuperSize;
}

VOID CBufferAllocator::CheckFreeBuffers(bool bIgnoreInterval)
{
	PBUFFER pBuffer, pPrevBuffer, pLastBuffer;
	LONGLONG dwTick = _getTickCount();

	//EnterCriticalSection( &m_BufferLock );
	m_BufferLock.Lock();
	pBuffer = m_pLastSmallBuffer;
	pLastBuffer = NULL;
	while ( pBuffer )
	{
		pPrevBuffer = pBuffer->pPrevBuffer;
		if (bIgnoreInterval || dwTick >= pBuffer->dwFreeTick)
		{
			if ( pBuffer == m_pLastSmallBuffer )
				m_pLastSmallBuffer = pPrevBuffer;
			if ( pLastBuffer )
				pLastBuffer->pPrevBuffer = pPrevBuffer;
			m_dwFreedSmallSize -= pBuffer->dwSize;
			free( pBuffer );
		}
		else pLastBuffer = pBuffer;
		pBuffer = pPrevBuffer;
	}

	pBuffer = m_pLastMiddleBuffer;
	pLastBuffer = NULL;
	while ( pBuffer )
	{
		pPrevBuffer = pBuffer->pPrevBuffer;
		if (bIgnoreInterval || dwTick >= pBuffer->dwFreeTick)
		{
			if ( pBuffer == m_pLastMiddleBuffer )
				m_pLastMiddleBuffer = pPrevBuffer;
			if ( pLastBuffer )
				pLastBuffer->pPrevBuffer = pPrevBuffer;
			m_dwFreedMiddleSize -= pBuffer->dwSize;
			free( pBuffer );
		}
		else pLastBuffer = pBuffer;
		pBuffer = pPrevBuffer;
	}

	pBuffer = m_pLastLargeBuffer;
	pLastBuffer = NULL;
	while ( pBuffer )
	{
		pPrevBuffer = pBuffer->pPrevBuffer;
		if (bIgnoreInterval || dwTick >= pBuffer->dwFreeTick)
		{
			if ( pBuffer == m_pLastLargeBuffer )
				m_pLastLargeBuffer = pPrevBuffer;
			if ( pLastBuffer )
				pLastBuffer->pPrevBuffer = pPrevBuffer;
			m_dwFreedLargeSize -= pBuffer->dwSize;
			free( pBuffer );
		}
		else pLastBuffer = pBuffer;
		pBuffer = pPrevBuffer;
	}

	pBuffer = m_pLastSuperBuffer;
	pLastBuffer = NULL;
	while ( pBuffer )
	{
		pPrevBuffer = pBuffer->pPrevBuffer;
		if (bIgnoreInterval || dwTick >= pBuffer->dwFreeTick)
		{
			if ( pBuffer == m_pLastSuperBuffer )
				m_pLastSuperBuffer = pPrevBuffer;
			if ( pLastBuffer )
				pLastBuffer->pPrevBuffer = pPrevBuffer;
			m_dwFreedSuperSize -= pBuffer->dwSize;
			free( pBuffer );
		}
		else pLastBuffer = pBuffer;
		pBuffer = pPrevBuffer;
	}

	//LeaveCriticalSection( &m_BufferLock );
	m_BufferLock.Unlock();
}












#ifdef _BUFFER_TRACE
DataBuffer::DataBuffer(CBufferAllocator *pAllocator, size_t dwSize, const char *_FILE, const int _LINE)
#else
DataBuffer::DataBuffer(CBufferAllocator *pAllocator, size_t dwSize)
#endif
{
	m_pAllocator = pAllocator;
	m_dwSize = dwSize;
#ifdef _BUFFER_TRACE
	pData = (char*)pAllocator->_AllocBuffer(dwSize, _FILE, _LINE);
#else
	pData = (char*)pAllocator->AllocBuffer(dwSize);
#endif
	ptr = pReadPtr = pData;
	pEnd = pData + dwSize;
}

DataBuffer::~DataBuffer()
{
	m_pAllocator->FreeBuffer( pData );
}

