//#include "StdAfx.h"
#include <stdio.h>
#include <stdlib.h>


#ifdef WIN32
	#include <tchar.h>
	#include <Windows.h>

#endif

#include <_ast.h>
#include <_memchk.h>
#include <Thread.h>

#include <Lock.h>
#include <Tick.h>

#include <Stream.h>
#include <wrand.h>
#include <time.h>

#include <List.h>
#include "ShareUtil.h"
#include <Lock.h>
#include "MiniDateTime.h"



//#include <LinkedList.h>

#include "../include/TimeStat.h"
static const TCHAR s_szLogFileName[] = _T("TimeStat.xml");
static const TCHAR* s_szTimeProfIndent[] = {
	_T(""),									// level_1
	_T("  "),								// level_2
	_T("    "),								// level_3
	_T("      "),							// level_4
	_T("        "),							// level_5
	_T("          "),						// level_6
	_T("            "),						// level_7
	_T("              "),					// level_8
	_T("                "),					// level_9
	_T("                  "),				// level_10
	_T("                    "),				// level_11
	_T("                      "),			// level_12
	_T("                        "),			// level_13
	_T("                          "),		// level_14
	_T("                            ")		// level_15
};
const TCHAR s_szSep[] = _T("-------------------------------------------------------------------------------------------\r\n");
const TCHAR s_szStartRecordsLabel[] = _T("<Records>\r\n");
const TCHAR s_szEndRecordsLabel[] = _T("</Records>\r\n");
const TCHAR s_szStartProfRecord[] = _T("<ProfRecord timeSpan=\"");
const TCHAR s_szEndProfRecord[] = _T("</ProfRecord>\r\n");
const TCHAR s_szThreadLabel[] = _T(" threadId=%d");
const TCHAR szXmlEndLable[] = _T("\">\r\n");

#ifdef WIN32
	DWORD CTimeProfMgr::s_dwTlsIdx = TLS_OUT_OF_INDEXES;
#else
	pthread_key_t CTimeProfMgr::s_dwTlsIdx =0 ;
#endif

bool CTimeProfDummy::g_isOpen  =false;  //默认是不开启的

CTimeProfDummy::CTimeProfDummy(LPCTSTR szUnitName, unsigned int nHashCode)
{
	
	if(g_isOpen)
	{
		CTimeProfMgr& profMgr = *CTimeProfMgr::getSingleton().getThreadInst();
		CTimeProfRecord *pCurRecord = profMgr.getCurrTimeProfRecord();
		if (pCurRecord)
		{
			CTimeProfRecord* pChild = pCurRecord->getChildByHash(nHashCode);
			if (!pChild)
			{	
				pChild = profMgr.allocRecord();
				pChild->reInitBasicData(szUnitName, nHashCode);			
				pCurRecord->addChild(pChild);
			}

			if(pChild)
			{
				pChild->startProf();
				profMgr.OnTimeProfRecordStart(pChild);
			}
		}
	}
}
CTimeProfDummy::~CTimeProfDummy()
{
	if(g_isOpen)
	{
		CTimeProfMgr& mgr = *CTimeProfMgr::getSingleton().getThreadInst();
		CTimeProfRecord *pCurRecord = mgr.getCurrTimeProfRecord();
		if(pCurRecord ==NULL) return;
		if(mgr.GetExeUnitCount() >1)
		{
			pCurRecord->endProf();	
			mgr.OnTimeProfRecordEnd(pCurRecord);
		}
	}
		
}

void CTimeProfRecord::dump(wylib::stream::CBaseStream& stream, int level)
{	
	level = level >= (int)(ArrayCount(s_szTimeProfIndent)) ? (int)(ArrayCount(s_szTimeProfIndent)-1) : level;
	stream.write(s_szTimeProfIndent[level], _tcslen(s_szTimeProfIndent[level])*sizeof(TCHAR));
	TCHAR szData[512] = {0};
	float nAvgTime = 0;	
	if (m_nTotalCount > 0)
		nAvgTime = (float)m_nTotalTime / m_nTotalCount;

//in windows,%I64d,%I64u,in linux %lld, %llu
#ifdef WIN32
	_stprintf_s(szData, _T("<%s ttime=\"%I64u\" tcount=\"%I64u\" atime=\"%f\" maxtime=\"%I64u\" mintime=\"%I64u\" ottime=\"%I64u\" otcount=\"%I64u\" omaxtime=\"%I64u\" omintime=\"%I64u\">\r\n"), 
		m_szExecUnitName,m_nTotalTime, m_nTotalCount, nAvgTime, m_nMaxTime, m_nMinTime, 
		m_nOccuTotalTime, m_nOccuTotalCount, m_nOccuMaxTime, m_nOccuMinTime);
#else
	_stprintf_s(szData, _T("<%s ttime=\"%llu\" tcount=\"%llu\" atime=\"%f\" maxtime=\"%llu\" mintime=\"%llu\" ottime=\"%llu\" otcount=\"%llu\" omaxtime=\"%llu\" omintime=\"%llu\">\r\n"), 
		m_szExecUnitName,m_nTotalTime, m_nTotalCount, nAvgTime, m_nMaxTime, m_nMinTime, 
		m_nOccuTotalTime, m_nOccuTotalCount, m_nOccuMaxTime, m_nOccuMinTime);
#endif

	stream.write(szData, _tcslen(szData) * sizeof(szData[0]));
	reset();
	for (INT_PTR i = 0; i < m_childrenNode.count(); i++)
		m_childrenNode[i]->dump(stream, level+1);	
	_stprintf_s(szData, _T("%s</%s>\r\n"), s_szTimeProfIndent[level], m_szExecUnitName);
	stream.write(szData, _tcslen(szData) * sizeof(szData[0]));
}

void CTimeProfRecord::setName(LPCTSTR szName)
{
	_tcscpy_s(m_szExecUnitName, szName);
}

CTimeProfRecord::CTimeProfRecord(LPCTSTR szUnitName, unsigned int nHashCode)
{	
}

CTimeProfRecord::~CTimeProfRecord(){
	clear();
}

void CTimeProfRecord::clear()
{
	for (INT_PTR i = 0; i < m_childrenNode.count(); i++)
	{
		CTimeProfRecord *pRecord = m_childrenNode[i];
		if(pRecord) {
			pRecord->clear();
		}
		
		CTimeProfMgr::getSingleton().getThreadInst()->freeRecord(pRecord);
	}		
	m_childrenNode.empty();
}

CTimeProfMgr::CTimeProfMgr() : m_execUnitRoot("", 0), m_dwThreadId(0)
{
	//::timeBeginPeriod(1);
	m_execUnitRoot.setName("Root");
	m_execUnitRoot.reset(true);
	m_execUnitStack.reserve(500);
	getCurrentTm(m_lastDumpTime);	
	m_execUnitStack.push(&m_execUnitRoot);

	m_freeRecordList.reserve(500);
	allocTimeProfRecord();
}

CTimeProfMgr::~CTimeProfMgr()
{
	//::timeEndPeriod(1);	
	clear();
}

void CTimeProfMgr::getCurrentTm(tm &t)
{
	time_t      szClock;	
	time(&szClock);
	//struct tm   curTime;
	localtime_r(&szClock,&t);
	//t = *curTime;
}


CTimeProfMgr* CTimeProfMgr::getThreadInst()
{
	CTimeProfMgr* profMgr =0;

#ifdef WIN32
	profMgr= (CTimeProfMgr *)TlsGetValue(s_dwTlsIdx);
#else
	profMgr=  (CTimeProfMgr *)pthread_getspecific(s_dwTlsIdx);
#endif
	if (!profMgr)
	{
		profMgr = new CTimeProfMgr();

#ifdef WIN32
		TlsSetValue(s_dwTlsIdx, profMgr);
		profMgr->setThreadId(GetCurrentThreadId());
#else
		pthread_setspecific(s_dwTlsIdx,profMgr);
		profMgr->setThreadId( pthread_self() );
#endif
		m_lock.Lock();
		m_vecTimeProfMgr.add(profMgr);
		m_lock.Unlock();
	}

	return profMgr;
}



bool CTimeProfMgr::InitMgr()
{
#ifdef WIN32		
	if (TLS_OUT_OF_INDEXES == s_dwTlsIdx)
	{
		s_dwTlsIdx = TlsAlloc();
		return s_dwTlsIdx != TLS_OUT_OF_INDEXES ? true : false;
	}
#else
	//s_dwTlsIdx = TlsAlloc();
	pthread_key_create(&s_dwTlsIdx,0);
	return true;
#endif
	return false;		
}

void CTimeProfMgr::clear()
{
	m_execUnitRoot.clear();

	for (INT_PTR i = 0; i < m_freeRecordList.count(); i++)
	{
		CTimeProfRecord *pRecord = m_freeRecordList[i];
		if(pRecord)
		{
			pRecord->~CTimeProfRecord();
			free(pRecord);
		}

	}
	m_freeRecordList.empty();		
	m_execUnitStack.empty();

#ifdef WIN32
	if (s_dwTlsIdx != TLS_OUT_OF_INDEXES)
	{
		TlsFree(s_dwTlsIdx);
		s_dwTlsIdx = TLS_OUT_OF_INDEXES;
	}
#else
	pthread_key_delete(s_dwTlsIdx);
#endif

	for (INT_PTR i = 0; i < m_vecTimeProfMgr.count(); i++)
	{
		delete m_vecTimeProfMgr[i];
	}
	m_vecTimeProfMgr.empty();
}

void CTimeProfMgr::dumpImpl(wylib::stream::CBaseStream& stream)
{
	dumpDateTimeHeader(stream);
	m_execUnitRoot.dump(stream);
	stream.write(s_szEndProfRecord, _tcslen(s_szEndProfRecord)*sizeof(TCHAR));
}

void CTimeProfMgr::dump()
{	
	if(CTimeProfDummy::g_isOpen)
	{
		tm cuTm;

		char name[128];
		getCurrentTm(cuTm);
		sprintf(name,"TimeStat_%d_%d_%d.xml",(int)cuTm.tm_year,(int)cuTm.tm_mon,(int)cuTm.tm_mday);


		using namespace wylib::stream;
		CFileStream fs(name, CFileStream::faWrite |  CFileStream::faShareRead, CFileStream::AlwaysCreate);
		LONGLONG fileSize = fs.getSize();
		bool bFirstTime = true;
		if (fileSize > (LONGLONG)(_tcslen(s_szEndProfRecord)*sizeof(TCHAR)))
		{
			bFirstTime = false;
			fileSize -= _tcslen(s_szEndRecordsLabel)*sizeof(TCHAR);
			fs.setPosition(fileSize);
		}
		else
		{
			fs.setPosition(fileSize);
			fs.write(s_szStartRecordsLabel, _tcslen(s_szStartRecordsLabel)*sizeof(TCHAR));
		}
		for (INT_PTR i = 0; i < m_vecTimeProfMgr.count(); i++)
		{
			m_vecTimeProfMgr[i]->dumpImpl(fs);
		}
		////dumpDateTimeSep(fs);
		//dumpDateTimeHeader(fs);
		//m_execUnitRoot.dump(fs);
		//fs.write(s_szEndProfRecord, _tcslen(s_szEndProfRecord)*sizeof(TCHAR));
		////dumpDateTimeSep(fs);	
		fs.write(s_szEndRecordsLabel, _tcslen(s_szEndRecordsLabel)*sizeof(TCHAR));	
	}
	
}

void CTimeProfMgr::dumpDateTimeHeader(wylib::stream::CBaseStream& stream)
{	
	TCHAR szBeginDateTime[256] = {0};
	formatTimeStr(szBeginDateTime, ArrayCount(szBeginDateTime)-1, &m_lastDumpTime);
	szBeginDateTime[ArrayCount(szBeginDateTime)-1] = _T('\0');		
	stream.write(s_szStartProfRecord, _tcslen(s_szStartProfRecord)*sizeof(TCHAR));
	stream.write(szBeginDateTime, _tcslen(szBeginDateTime) * sizeof(TCHAR));	
	stream.write("--", 2);

	TCHAR szDateTime[256] = {0};
	getCurrentTm(m_lastDumpTime);
	formatTimeStr(szDateTime, ArrayCount(szDateTime)-1, &m_lastDumpTime);
	szDateTime[ArrayCount(szDateTime)-1] = _T('\0');
	stream.write(szDateTime, _tcslen(szDateTime) * sizeof(TCHAR));	

	// dump thread id	
	TCHAR szThreadInfo[64] = {0};
	_stprintf_s(szThreadInfo, s_szThreadLabel, m_dwThreadId);
	stream.write(szThreadInfo, _tcslen(szThreadInfo) * sizeof(TCHAR));	
	stream.write(szXmlEndLable, _tcslen(szXmlEndLable) * sizeof(TCHAR));
}

void CTimeProfMgr::formatTimeStr(TCHAR* szDataBuff, size_t nLen, tm *t)
{
	_tcsftime(szDataBuff, nLen-1, _T("%Y-%m-%d_%H:%M:%S"), t);		
}

void CTimeProfMgr::dumpDateTimeSep(wylib::stream::CBaseStream& stream)
{
	stream.write(s_szSep, _tcslen(s_szSep) * sizeof(TCHAR));
}
