#pragma once

//////////////////////////////////////////////////////////////////////////
// 说  明：统计句柄管理器分配详细，包括：
//		   当前分配的句柄Block数量、每个Block包含的句柄的数目
//		   已经使用的句柄数目
//////////////////////////////////////////////////////////////////////////

class HandleMgrStat;
#include "_osdef.h"
class HandleMgrCollector
{
public:
	static HandleMgrCollector& getSingleton()
	{
		static HandleMgrCollector collector;
		return collector;
	}

	void addHangleMgrStat(const HandleMgrStat *stat);

	void removeHandleMgrStat(const HandleMgrStat *stat);

	void logToFile();

private:
	wylib::container::CBaseLinkedList<const HandleMgrStat*>		m_handleMgrCollector;
};

// 句柄管理器统计类
// 记录每个管理器名称和管理器的指针
class HandleMgrStat
{
public:
	enum 
	{
		MAX_DESC_LENGTH = 32,			// 最大描述字符串长度
	};
		
	HandleMgrStat(LPCTSTR lpszDesc, __int64 nCountPerBlock){
		size_t nLen = 0;
		if (lpszDesc)
			nLen = __min(MAX_DESC_LENGTH, _tcslen(lpszDesc));
		if (nLen > 0)
			memcpy(m_szObjectDesc, lpszDesc, nLen * sizeof(TCHAR));
		m_szObjectDesc[nLen] = _T('\0');	
		if (nLen > 0)
			HandleMgrCollector::getSingleton().addHangleMgrStat(this);		

		m_nCountPerChunk		= nCountPerBlock;		
		m_nAllocedChunkCount	= 0;
		m_nUsedHdlCount			= 0;
		m_nMaxUsedHandleCount	= 0;
	}

	~HandleMgrStat()
	{
		HandleMgrCollector::getSingleton().removeHandleMgrStat(this);
	}

	inline void getStatInfo(unsigned long long & nCountPerChunk, unsigned long long & nAllocedChunkCnt, unsigned long long & nUsedCnt, unsigned long long & nMaxUsedCnt) const
	{
		nCountPerChunk		= m_nCountPerChunk;
		nAllocedChunkCnt	= m_nAllocedChunkCount;
		nUsedCnt			= m_nUsedHdlCount;
		nMaxUsedCnt			= m_nMaxUsedHandleCount;
	}

	inline LPCTSTR getObjectDesc() const { return m_szObjectDesc; }

protected:
	unsigned long long	m_nCountPerChunk;						// 每个Chunk中包含的句柄数量
	unsigned long long	m_nAllocedChunkCount;					// 已经分配的Chunk数量
	unsigned long long	m_nUsedHdlCount;						// 已经使用的句柄数量
	unsigned long long	m_nMaxUsedHandleCount;					// 最大使用的句柄数量
	TCHAR				m_szObjectDesc[MAX_DESC_LENGTH+1];		// 对象描述
};
