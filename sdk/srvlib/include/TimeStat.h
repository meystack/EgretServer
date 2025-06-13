#pragma once
//////////////////////////////////////////////////////////////////////////
//							时间统计分析类
// 功	能：用于追踪程序中调用的函数执行时间消耗情况，定位出性能瓶颈
// 说	明：统计按照设定好的时间间隔来输出，例如2分钟输出一次，将所有函数调用
//			节点信息按照树形结果来呈现（序列化到文件）处理，可以快速的分析出
//			2分钟内，每个执行单元消耗了多久时间，找出瓶颈所在。
// 例如：在逻辑线程，在逻辑线程每次循环加一个追踪节点，在下面的网络处理、实体管理器、
//		 副本管理器等等都增加追踪节点，这样就能详细的跟踪到每个节点耗时占整个耗时的
//		 比例。
//			因为逻辑引擎本身采取分时，对每个子（子子）单元都限制了执行时间，这样就对
//		 统计分析有影响！比如正常情况下实体管理器刷新占据每次例行50%左右的时间，结果
//		 因为分时，就算实体刷新出问题，也检测不出来！！！
//			对于此问题，要么是暂时过滤分时；要么是在实体管理器内部设置分时追踪结点。
//////////////////////////////////////////////////////////////////////////
#include <new>
#include "ObjectCounter.h"
#include "Lock.h"
#include "Tick.h"
#include "_osdef.h"
#ifndef  WIN32
	#include <pthread.h>
#endif

class CTimeProfDummy  : public Counter<CTimeProfDummy>
{
public:
	CTimeProfDummy(LPCTSTR szUnitName, unsigned int nHashCode);
	~CTimeProfDummy();
	static void SetOpenFlag(bool isOpen)
	{
		g_isOpen =isOpen;
	}
	static bool g_isOpen;   //是否是关闭的
};

// 执行时间记录节点
class CTimeProfRecord : public Counter<CTimeProfRecord>
{
public:
	enum
	{
		MAX_EXEC_UNIT_NAME_LEN	= 64,								// 最大执行单元名称
	};
	CTimeProfRecord(){}
	CTimeProfRecord(LPCTSTR szUnitName, unsigned int nHashCode);
	~CTimeProfRecord();
	//设置节点名称
	void setName(LPCTSTR szName);
	// 添加一个子执行结点
	inline void addChild(CTimeProfRecord* record)
	{
		m_childrenNode.add(record);
	}
	// 查询节点
	//CTimeProfRecord* getChild(LPCTSTR szUnitName);
	inline CTimeProfRecord* getChildByHash(unsigned int nHashCode)
	{
		for (INT_PTR i = 0; i < m_childrenNode.count(); i++)
		{
			if (nHashCode == m_childrenNode[i]->m_nNameHashCode)
				return m_childrenNode[i];
		}
		return NULL;
	}
	// 输出结点执行时间详细信息
	void dump(wylib::stream::CBaseStream& stream, int level = 0);
	// 重置节点数据
	inline void reset(bool bIncOccuData = false)
	{
		m_nTotalCount	= 0;
		m_nMaxTime		= 0;
		m_nMinTime		= 0;
		m_nTotalTime	= 0;
		if (bIncOccuData)
		{
			m_nOccuTotalCount	= 0;
			m_nOccuTotalTime	= 0;
			m_nOccuMaxTime		= 0;
			m_nOccuMinTime		= 0;
		}
	}
	inline void reInitBasicData(LPCTSTR szUnitName, unsigned int nHashCode)
	{
		ZeroMemory(this, offsetof(CTimeProfRecord, m_childrenNode));
		_tcscpy_s(m_szExecUnitName, szUnitName);
		for (INT_PTR i = 0; i < (INT_PTR)_tcslen(m_szExecUnitName); i++)
		{
			if (m_szExecUnitName[i] == _T('\"')		|| m_szExecUnitName[i] == _T(',') || m_szExecUnitName[i] == _T('<') 
				|| m_szExecUnitName[i] == _T('>')	|| m_szExecUnitName[i] == _T(':') || m_szExecUnitName[i] == _T(' '))
				m_szExecUnitName[i] = _T('_');
		}
		m_nNameHashCode = nHashCode;			
	}
	inline void startProf()
	{
		m_nStartTime = wylib::time::tick64::GetTickCountEx();		
		m_nTotalCount++;	
		m_nOccuTotalCount++;
	}
	inline void endProf()
	{
		m_nEndTime = wylib::time::tick64::GetTickCountEx();		
		unsigned long long nTimeConsume = m_nEndTime - m_nStartTime;
		if (m_nMaxTime < nTimeConsume)
			m_nMaxTime = nTimeConsume;
		if (m_nMinTime > nTimeConsume)
			m_nMinTime = nTimeConsume;
		if (m_nOccuMaxTime < nTimeConsume)
			m_nOccuMaxTime = nTimeConsume;
		if (m_nOccuMinTime > nTimeConsume)
			m_nOccuMinTime = nTimeConsume;
		m_nTotalTime += nTimeConsume;
		m_nOccuTotalTime += nTimeConsume;	
	}

	void clear();

	//字符串hash函数，需要提供长度
	inline static unsigned int hashlstr(const char* str, size_t len)
	{
		unsigned int h = (unsigned int)len;
		size_t step = (len>>5)+1;  /* if string is too long, don't hash all its chars */
		size_t l1;
		for (l1=len; l1>=step; l1-=step)  /* compute hash */
			h = h ^ ((h<<5)+(h>>2)+(unsigned char)str[l1-1]);
		return h;
	}

protected:		
	unsigned int				m_nNameHashCode;							// 名字Hash码
	unsigned long long			m_nTotalCount;								// 执行次数
	unsigned long long			m_nOccuTotalCount;							// 累积执行次数
	unsigned long long			m_nMaxTime;									// 本次统计最大执行耗时
	unsigned long long			m_nMinTime;									// 本次统计最小执行耗时
	unsigned long long			m_nOccuMaxTime;								// 累积的最大执行耗时
	unsigned long long			m_nOccuMinTime;								// 累积的最小执行耗时
	unsigned long long			m_nTotalTime;								// 整体执行耗时（用于统计平均耗时）
	unsigned long long			m_nOccuTotalTime;							// 累积执行时间
	unsigned long long			m_nStartTime;								// 开始执行时间	
	unsigned long long			m_nEndTime;									// 结束执行时间
	TCHAR						m_szExecUnitName[MAX_EXEC_UNIT_NAME_LEN];	// 执行单位名称	
	wylib::container::CBaseList<CTimeProfRecord*>		m_childrenNode;		// 所有子节点列表
	static   bool               g_isOpen;                  
};


// 执行时间管理器
class CTimeProfMgr
{
public:
	CTimeProfMgr();
	~CTimeProfMgr();
	enum
	{
		ALLOC_RECORD_NUM = 200,
	};
	
	


	static CTimeProfMgr& getSingleton()
	{
		static CTimeProfMgr mgr;
		return mgr;
	}
	bool InitMgr();
	CTimeProfMgr* getThreadInst();
	
	// 开始执行计时
	inline void OnTimeProfRecordStart(CTimeProfRecord *record)
	{
		m_execUnitStack.push(record);
	}
	// 结束执行计时
	inline void OnTimeProfRecordEnd(CTimeProfRecord *record)
	{
		m_execUnitStack.pop();		
	}

	//获取数据
	inline INT_PTR GetExeUnitCount()
	{
		return m_execUnitStack.count();
	}


	// 获取当前执行单元结点
	inline CTimeProfRecord* getCurrTimeProfRecord()
	{
		INT_PTR count = m_execUnitStack.count();
		if (count > 0)
			return m_execUnitStack[count-1];

		return NULL;
	}

	// 输出结点执行时间详细信息
	void dump();

	inline CTimeProfRecord* allocRecord()
	{
		if (m_freeRecordList.count() <= 0)
			allocTimeProfRecord();
		return m_freeRecordList.pop();
	}

	inline void freeRecord(CTimeProfRecord *record)
	{
		m_freeRecordList.add(record);
	}

	void clear();

protected:
	// 输出时间头
	void dumpDateTimeHeader(wylib::stream::CBaseStream& stream);
	// 输出分隔符
	void dumpDateTimeSep(wylib::stream::CBaseStream& stream);
	// 获取当前时间
	void getCurrentTm(tm &t);
	// dump
	void dumpImpl(wylib::stream::CBaseStream& stream);
	// 格式化时间
	void formatTimeStr(TCHAR* szDataBuff, size_t nLen, tm *t);
	// 执行一次分配（每次分配ALLOC_RECORD_NUM个）
	inline void allocTimeProfRecord()
	{		
		for (INT_PTR i = 0; i < ALLOC_RECORD_NUM; i++)
		{
			CTimeProfRecord* pRecords = (CTimeProfRecord *)malloc(sizeof(CTimeProfRecord));
			if(pRecords)
			{
				new (pRecords)CTimeProfRecord();
				m_freeRecordList.add(pRecords);
			}
			
		}
	}

	// set thread id
	inline void setThreadId(DWORD dwThreadId) { m_dwThreadId = dwThreadId; }

protected:
	wylib::container::CBaseList<CTimeProfRecord *>	m_execUnitStack;	// 执行单元结点栈
	CTimeProfRecord									m_execUnitRoot;		// 执行单元树的根节点	
	tm												m_lastDumpTime;		// 上次dump结束时间
	wylib::container::CBaseList<CTimeProfRecord*>	m_freeRecordList;


#ifdef WIN32
	static DWORD									s_dwTlsIdx;			// Tls Slot Index
#else
	static pthread_key_t 							s_dwTlsIdx;                     //linux key	
#endif

	wylib::sync::lock::CCSLock						m_lock;				// Sync for multithread 

	wylib::container::CBaseList<CTimeProfMgr*>		m_vecTimeProfMgr;	// TimeProfMgr list
	DWORD											m_dwThreadId;		// Current ThreadId
};
#ifdef _DEBUG
#define DECLARE_TIME_PROF(name)   static unsigned int _STATIC_FUNCTION_HASH_VALUE_= CTimeProfRecord::hashlstr(name,strlen(name)); \
	CTimeProfDummy LocalTimeProfRecord(name, _STATIC_FUNCTION_HASH_VALUE_ );
#define DECLARE_FUN_TIME_PROF()  DECLARE_TIME_PROF(__FUNCTION__)
#else
#define DECLARE_TIME_PROF(name)
#define DECLARE_FUN_TIME_PROF()
#endif

