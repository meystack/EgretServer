#include <stdarg.h>
#include <stdio.h>

#ifdef WIN32
	#include <Windows.h>
	#include <dbghelp.h>
	#include <TCHAR.h>
	#include <locale.h>
#endif

#include "ObjectAllocStat.hpp"
#include <Tick.h>
#include "_ast.h"
#include "ShareUtil.h"
#include "LinkedList.h"
#include "List.h"
#include "Stream.h"

#include <time.h>


//#ifdef _ENABLE_OBJ_COUNT_STATICS_





void CSingleObjectAllocStatMgr::addAllocStat(const CObjectAllocatorStat *stat)
{
	if (stat)
	{
		using namespace wylib::container;
		CLinkedListIterator<const CObjectAllocatorStat*> iter(m_AllocStatList);
		CLinkedNode<const CObjectAllocatorStat*> *pNode;
		for (pNode = iter.first(); pNode; pNode = iter.next())
		{
			if (pNode->m_Data == stat)
				return;
		}
				
		m_AllocStatList.linkAfter(stat);
	}
}

void CSingleObjectAllocStatMgr::removeAllocStat(const CObjectAllocatorStat *stat)
{	
	if (!stat) return;	
	using namespace wylib::container;
	CLinkedListIterator<const CObjectAllocatorStat*> iter(m_AllocStatList);
	CLinkedNode<const CObjectAllocatorStat*> *pNode;
	for (pNode = iter.first(); pNode; pNode = iter.next())
	{
		if (pNode->m_Data == stat)
		{
			iter.remove(pNode, true);
			break;
		}
	}
}

void CSingleObjectAllocStatMgr::logToFile()
{
	/* 文件格式：
	---------------------------------------------------------------------------
	DateTime:2010-10-20 19:30 
	AllocDesc		UsedNum		AllocedNum		TotalByte
	***
	---------------------------------------------------------------------------
	*/
	/*
	using namespace wylib::stream;
	using namespace wylib::container;
	CFileStream fs(_T("SOAProf.log"), CFileStream::faWrite, CFileStream::AlwaysOpen);
	fs.setPosition(fs.getSize());
	const TCHAR szSep[] = _T("-------------------------------------------------------------------------------------------\r\n");
	time_t      szClock;	
	time(&szClock);
	struct tm   *curTime = localtime(&szClock);
	TCHAR szDateTime[250] = {0};	
	_tcsftime(szDateTime, _tcslen(szDateTime)-1, _T("%Y-%m-%d %H:%M:%S\r\n"), curTime);	
	fs.write(szSep, _tcslen(szSep));
	fs.write(szDateTime, _tcslen(szDateTime));
	TCHAR szDesc[256] = {0};
	_stprintf(szDesc, _T("%-34s%-15s%-15s%-15s%-20s\r\n"), _T("AllocDesc"), _T("AllocType"), _T("UsedNum"), _T("AllocedNum"), _T("TotalByte"));
	fs.write(szDesc, _tcslen(szDesc));

	TCHAR szContent[256] = {0};
	TCHAR szContentFmt[256] = {0};
	_tcscpy(szContentFmt, _T("%-34s%-15d%-15I64u%-15I64u%-20I64u\r\n"));
	CLinkedListIterator<const CObjectAllocatorStat*> iter(m_AllocStatList);
	CLinkedNode<const CObjectAllocatorStat*> *pNode;
	for (pNode = iter.first(); pNode; pNode = iter.next())
	{
		const CObjectAllocatorStat* stat = pNode->m_Data;
		if (stat)
		{
			unsigned __int64 usedObjects, allocObjects;
			unsigned __int64 totalBytes = stat->getAllocStat(usedObjects, allocObjects);
			_stprintf(szContent, szContentFmt, stat->getObjectDesc(), stat->getAllocType(), usedObjects, allocObjects, totalBytes);
			fs.write(szContent, _tcslen(szContent));
		}
	}
	fs.write(szSep, _tcslen(szSep));
	*/

}
//#else

/*
void CObjectAllocatorStat::removeFromStat() const
{
}

void CObjectAllocatorStat::addToStat() const
{
}
*/


/*
void CSingleObjectAllocStatMgr::logToFile()
{
}

void CSingleObjectAllocStatMgr::removeAllocStat(const CObjectAllocatorStat *stat)
{

}
void CSingleObjectAllocStatMgr::addAllocStat(const CObjectAllocatorStat *stat)
{

}

#endif
*/


