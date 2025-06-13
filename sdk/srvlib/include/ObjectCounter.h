#pragma once


#include <time.h>
#include "Stream.h"

#define DISALLOW_CONSTRUCTORS(TypeName)    \
	TypeName(const TypeName&);             \
	void operator=(const TypeName&);

#define DISALLOW_COPY_AND_ASSIGN(TypeName) DISALLOW_CONSTRUCTORS(TypeName)

typedef uint64 (*GetCounterProc)();



//////////////////////////////////////////////////////////////////////////
// 计数管理器：管理所有计数对象，提供同一模块输出
class CounterManager
{
public:
	enum {
		MAX_COUNTER_NAME_LEN = 62,
		MAX_COUNTER_COUNT = 2048,
	};

	struct CounterNode
	{
		void	*m_fn;									// 取Counter的函数指针
		size_t	m_objectSize;							// 目标对象大小
		TCHAR	m_szCounterName[MAX_COUNTER_NAME_LEN+1];	// 计数器名称

		CounterNode() : m_fn(0), m_objectSize(0) { m_szCounterName[0] = _T('\0');}
		CounterNode(LPCTSTR lpszCounterName, void *fn, size_t objectSize)
		{
			size_t nLen = 0;
			const TCHAR *pBegin = 0, *pEnd = 0;
			if (lpszCounterName)
			{
				pBegin	= _tcsstr(lpszCounterName, _T("class "));
				if (!pBegin)
				{
					pBegin = _tcsstr(lpszCounterName, _T("struct "));
					if (pBegin)
						pBegin += _tcslen(_T("struct "));
				}
				else
				{
					pBegin += _tcslen(_T("class "));
				}
				pEnd	= _tcsstr(lpszCounterName, _T("::addToStat"));								
				if (pBegin && pEnd && (pBegin < --pEnd))
				{					
					nLen = __max(0, pEnd-pBegin);
				}
			}	
			nLen = __min(MAX_COUNTER_NAME_LEN, nLen);
			if (nLen > 0)
				memcpy(m_szCounterName, pBegin, nLen * sizeof(TCHAR));	
			m_szCounterName[nLen] = _T('\0');
			m_fn = fn;
			m_objectSize = objectSize;
		}

		CounterNode& operator=(const CounterNode &rhs)
		{
			memcpy(this, &rhs, sizeof(CounterNode));
			return *this;
		}
	};

public:
	static CounterManager& getSingleton()
	{
		static CounterManager mgr;
		return mgr;
	}

	CounterManager()
	{
		ZeroMemory(this, sizeof(*this));
	}
	void clear()
	{		
		m_counterCnt = 0;
	}

	inline void addObjectCounter(LPCTSTR lpszCounterName, void *fn, size_t objectSize)
	{
		if (!lpszCounterName || !fn)
			return;
		if (m_counterCnt >= MAX_COUNTER_COUNT) return;
		CounterNode node(lpszCounterName, fn, objectSize);		
		m_counterList[m_counterCnt] = node;
		m_counterCnt++;
	}
	
	inline void logToFile()
	{
		using namespace wylib::stream;	
		
		char name[128];
		
		
		time_t      szClock;	
		time(&szClock);
		struct tm curTime;
		localtime_r(&szClock,&curTime);

		sprintf(name,"OCProf%d_%d_%d.log",(int)curTime.tm_year,(int)curTime.tm_mon,(int)curTime.tm_mday);

		CFileStream fs(name, CFileStream::faWrite, CFileStream::AlwaysOpen);
		fs.setPosition(fs.getSize());
		const TCHAR szSep[] = _T("-------------------------------------------------------------------------------------------\r\n");
		
		TCHAR szDateTime[250] = {0};	
		_tcsftime(szDateTime, _tcslen(szDateTime)-1, _T("%Y-%m-%d %H:%M:%S\r\n"), &curTime);	
		fs.write(szSep, _tcslen(szSep));
		fs.write(szDateTime, _tcslen(szDateTime));
		TCHAR szDesc[256] = {0};
		_stprintf(szDesc, _T("%-84s%-20s%-20s\r\n"), _T("Object"), _T("Count"), _T("TotalSize"));
		fs.write(szDesc, _tcslen(szDesc));

		TCHAR szContent[256] = {0};
		TCHAR szContentFmt[256] = {0};
		_tcscpy(szContentFmt, _T("%-84s%-20I64u%-20d\r\n"));		
		for (INT_PTR i = 0; i < m_counterCnt; i++)
		{
			const CounterNode &cn = m_counterList[i];
			uint64 nCount = ((GetCounterProc)(cn.m_fn))();
			_stprintf(szContent, szContentFmt, cn.m_szCounterName, nCount, nCount*cn.m_objectSize);
			fs.write(szContent, _tcslen(szContent));
		}
		fs.write(szSep, _tcslen(szSep));
	}

private:	
	CounterNode									m_counterList[MAX_COUNTER_COUNT];
	INT_PTR										m_counterCnt;
};

#ifdef _ENABLE_OBJ_COUNT_STATICS_
template <class T>
class Counter
{
public:
	Counter() {
		addToStat();
	}

	Counter(const Counter &rhs)
	{
		addToStat();
	}

	~Counter() {
		m_nCount--;
	}
		
	static uint64 getCount() { return m_nCount; }

protected:
	inline void addToStat()
	{
		if (InterlockedCompareExchange(&m_bHasAddedToStat, 1, 0) == 0)
		{
			CounterManager::getSingleton().addObjectCounter(_T(__FUNCTION__), (void *)&getCount, sizeof(T));
		}
		m_nCount++;
	}
private:
	//DISALLOW_COPY_AND_ASSIGN(Counter)
	static uint64 m_nCount;
	static long m_bHasAddedToStat;
};

template<class T> uint64 Counter<T>::m_nCount = 0;
template<class T> long Counter<T>::m_bHasAddedToStat = 0;



#else	//_ENABLE_OBJ_COUNT_STATICS_

template <class T>
class Counter
{
//public:
//	static unsigned __int64 getCount() { return 0; }

public:
	Counter() {
		//addToStat();
	}

	Counter(const Counter &rhs)
	{
		//addToStat();
	}

	~Counter() {
		//m_nCount--;
	}

};
#endif	//_ENABLE_OBJ_COUNT_STATICS_

#define DECLARE_OBJECT_COUNTER(OBJECT) Counter<OBJECT> __counter;