#include <new>
#include <stdio.h>
#ifdef WIN32
#include <tchar.h>
#include <Windows.h>
#endif
#include <_ast.h>
#include <_memchk.h>
#include <Stream.h>
#include "BufferAllocator.h"
#include "BuffAllocStatHelper.h"
#include "Utility.h"
using namespace wylib::stream;

LPCTSTR CBuffAllocStatHelper::s_szLineFeed = _T("\r\n");
void CBuffAllocStatHelper::Dump(CBaseStream& stream, 
								CBufferAllocator* pAlloc, 
								LPCTSTR szAllocaterName,
								bool bIncTimeStamp)
{
	if (!pAlloc)
		return;

	CBufferAllocator::ALLOCATOR_MEMORY_INFO mi;
	pAlloc->GetMemoryInfo(&mi);
	DumpHeader(stream, szAllocaterName);	
	DumpImpl(stream, mi);
}

void CBuffAllocStatHelper::Dump(wylib::stream::CBaseStream& stream, 
								CBufferAllocator::ALLOCATOR_MEMORY_INFO& mi, 
								LPCTSTR szAllocaterName, 
								bool bIncTimeStamp)
{
	DumpHeader(stream, szAllocaterName);
	DumpImpl(stream, mi);
}

void CBuffAllocStatHelper::DumpHeader(wylib::stream::CBaseStream& stream, LPCTSTR szAllocatorName, bool bIncTimeStamp)
{
	TCHAR szDateTime[MAX_PATH] = {0};
	if (bIncTimeStamp)
	{
		CUtility::GetDateTimeDesc(szDateTime);
		WriteStrToStream(stream, szDateTime);
	}
	WriteStrToStream(stream, _T("----------------"));
	WriteStrToStream(stream, szAllocatorName);
	WriteStrToStream(stream, s_szLineFeed);
}

void CBuffAllocStatHelper::DumpImpl(wylib::stream::CBaseStream& stream, CBufferAllocator::ALLOCATOR_MEMORY_INFO& mi)
{
	TCHAR szBuff[1024] = {0};
	_stprintf(szBuff, _T("小块内存总分配字节数 = %-20d, 空闲字节数=%d\r\n"), mi.SmallBuffer.dwAllocSize, mi.SmallBuffer.dwFreeSize);
	WriteStrToStream(stream, szBuff);
	_stprintf(szBuff, _T("中块内存总分配字节数 = %-20d, 空闲字节数=%d\r\n"), mi.MiddleBuffer.dwAllocSize, mi.MiddleBuffer.dwFreeSize);
	WriteStrToStream(stream, szBuff);
	_stprintf(szBuff, _T("大块内存总分配字节数 = %-20d, 空闲字节数=%d\r\n"), mi.LargeBuffer.dwAllocSize, mi.LargeBuffer.dwFreeSize);
	WriteStrToStream(stream, szBuff);
	_stprintf(szBuff, _T("超大块内存总分配字节数 = %-20d, 空闲字节数=%d\r\n"), mi.SuperBuffer.dwAllocSize, mi.SuperBuffer.dwFreeSize);
	WriteStrToStream(stream, szBuff);
}

void CBuffAllocStatHelper::WriteStrToStream(wylib::stream::CBaseStream& stream, LPCTSTR szContent)
{
	stream.write(szContent, _tcslen(szContent) * sizeof(TCHAR));
}