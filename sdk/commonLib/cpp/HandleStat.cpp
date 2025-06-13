//#include "Stdafx.h"


#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <crtdbg.h>
#include <tchar.h>
#include <Windows.h>
#endif
#include <_ast.h>
#include <_memchk.h>
//#include <Thread.h>
#include <CustomSocket.h>
#include <Lock.h>
#include <Tick.h>

#include <Stream.h>

//#include <QueueList.h>
#include <LinkedList.h>
#include "ShareUtil.h"
#include <RefString.hpp>


//#include <math.h>


#include "HandleStat.h"

void HandleMgrCollector::addHangleMgrStat(const HandleMgrStat *stat)
{
	if (!stat) return;	
	m_handleMgrCollector.linkAfter(stat);
}

void HandleMgrCollector::removeHandleMgrStat(const HandleMgrStat *stat)
{
	if (!stat) return;
	using namespace wylib::container;
	CLinkedListIterator<const HandleMgrStat*> iter(m_handleMgrCollector);
	CLinkedNode<const HandleMgrStat*> *pNode = 0;
	for (pNode = iter.first(); pNode; pNode = iter.next())
	{
		if (pNode->m_Data == stat)
		{			
			iter.remove(pNode);
			return;
		}
	}
}

void HandleMgrCollector::logToFile()
{
	/* 文件格式：
	---------------------------------------------------------------------------
	DateTime:2010-10-20 19:30 
	HandleMgrName	UsedHandleCount		AllocedBlockNum		HandlePerBlock
	***
	---------------------------------------------------------------------------
	*/
	using namespace wylib::stream;
	using namespace wylib::container;
	
	char name[128];

	time_t      szClock;	
	time(&szClock);
	struct tm curTime;
	localtime_r(&szClock, &curTime);


	sprintf(name,"HandleMgrProf_%d_%d_%d.log",(int)curTime.tm_year,(int)curTime.tm_mon,(int)curTime.tm_mday);
	CFileStream fs(name, CFileStream::faWrite, CFileStream::AlwaysOpen);
	fs.setPosition(fs.getSize());
	const TCHAR szSep[] = _T("-------------------------------------------------------------------------------------------\r\n");

	TCHAR szDateTime[250] = {0};	
	_tcsftime(szDateTime, _tcslen(szDateTime)-1, _T("%Y-%m-%d %H:%M:%S\r\n"), &curTime);	
	fs.write(szSep, _tcslen(szSep));
	fs.write(szDateTime, _tcslen(szDateTime));
	TCHAR szDesc[256] = {0};
	_stprintf(szDesc, _T("%-40s%-25s%-25s%-25s%-25s\r\n"), _T("HandleMgrName"), _T("UsedHandleCount"), _T("MaxUsedHdlCount"), _T("AllocedBlockNum"), _T("HandlePerBlock"));
	fs.write(szDesc, _tcslen(szDesc));

	TCHAR szContent[256] = {0};
	TCHAR szContentFmt[256] = {0};
	_tcscpy(szContentFmt, _T("%-40s%-25I64u%-25I64u%-25I64u%-25I64u\r\n"));
	CLinkedListIterator<const HandleMgrStat*> iter(m_handleMgrCollector);
	CLinkedNode<const HandleMgrStat*> *pNode = 0;
	for (pNode = iter.first(); pNode; pNode = iter.next())
	{
		const HandleMgrStat *stat = pNode->m_Data;
		if (stat)
		{			
			unsigned long long nCntPerChunk, nChunkCount, nUsedCnt, nMaxUsedCnt;
			stat->getStatInfo(nCntPerChunk, nChunkCount, nUsedCnt, nMaxUsedCnt);
			_stprintf(szContent, szContentFmt, stat->getObjectDesc(), nUsedCnt, nMaxUsedCnt, nChunkCount, nCntPerChunk);
			fs.write(szContent, _tcslen(szContent));
		}
	}
	fs.write(szSep, _tcslen(szSep));
}
