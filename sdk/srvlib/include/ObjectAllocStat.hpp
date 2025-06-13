#pragma once

/************************************************************************
                         对象分配内存统计
************************************************************************/
#include "List.h"
#include "LinkedList.h"


//#ifdef WIN32
class CObjectAllocatorStat;
//#endif


class CSingleObjectAllocStatMgr
{
public:
	static CSingleObjectAllocStatMgr& getSingleton()
	{
		static CSingleObjectAllocStatMgr mgr;
		return mgr;
	}

	~CSingleObjectAllocStatMgr()
	{
		clear();
	}

	void clear()
	{
		m_AllocStatList.clear();
	}

	/*
	* Comments:添加一个统计项
	* @Return void:
	*/
	void addAllocStat(const CObjectAllocatorStat *stat);
	
	/*
	* Comments: 移除一个统计项
	* Param CSingleObjectAllocatorStat * stat:
	* @Return void:
	*/
	void removeAllocStat(const CObjectAllocatorStat *stat);

	/*
	* Comments: 统计信息到日志
	* @Return void:
	*/
	void logToFile();

private:	
	wylib::container::CBaseLinkedList<const CObjectAllocatorStat *> m_AllocStatList;
};

//#ifdef _ENABLE_OBJ_COUNT_STATICS_

class CObjectAllocatorStat
{
public:
	enum 
	{
		MAX_DESC_LENGTH = 32,			// 最大描述字符串长度
	};
	enum AllocatorType
	{
		enAT_ObjectAlloc		= 1,	// 对应ObjectAllocator(生存期内只分配不释放)
		enAT_SingleObjectAlloc	= 2,	// 对应SingleObjectAllocator(生存期可以分配、释放对象)
	};

	//CObjectAllocatorStat(size_t nObjectSize,int at = 2, LPCTSTR szDesc = NULL);
	CObjectAllocatorStat(size_t nObjectSize, int at = 2, LPCTSTR lpszDesc= NULL) 
		: m_nObjectSize(nObjectSize)
	{
		m_allocType =(AllocatorType)at;
		m_nUsedObjNum	= 0;
		m_nAllocObjNum	= 0;
		size_t nLen = 0;
		if (lpszDesc)
			nLen = _tcslen(lpszDesc);
		nLen = __min(MAX_DESC_LENGTH, nLen);
		if (nLen > 0)
			memcpy(m_szObjectDesc, lpszDesc, nLen * sizeof(TCHAR));	
		m_szObjectDesc[nLen] = _T('\0');
		addToStat();

	}

	/*
	* Comments: 返回分配器当前分配的统计信息
	* @Param unsigned __int64 &usedObjects: 返回使用的对象数目
	* @Param unsigned __int64 &allocObjects: 返回分配的对象数目
	* @Return unsigned __int64: 返回总共分配的字节数
	*/
	inline uint64 getAllocStat(uint64 &usedObjects, uint64 &allocObjects) const
	{
		usedObjects		= m_nUsedObjNum;
		allocObjects	= m_nAllocObjNum;
		return ((uint64)m_nAllocObjNum) * m_nObjectSize;
	}

	inline AllocatorType getAllocType() const { return m_allocType; }

	inline LPCTSTR getObjectDesc() const { return m_szObjectDesc; }

	inline CObjectAllocatorStat& operator = (const CObjectAllocatorStat &rhs)
	{
		m_nUsedObjNum	= rhs.m_nUsedObjNum;
		m_nAllocObjNum	= rhs.m_nAllocObjNum;
		m_nObjectSize	= m_nObjectSize;
		return *this;
	}

	~CObjectAllocatorStat()
	{
		removeFromStat();
	}
	


public:


	
	/*
	* Comments: 添加到统计列表
	* @Return void:
	*/
	//void addToStat() ;
	

	/*
	* Comments: 将自己从统计列表中删除
	* @Return void:
	*/
	//void removeFromStat() ;
	
	//CObjectAllocatorStat::~CObjectAllocatorStat()
	//{
		
	//}

	void addToStat() 
	{
		if (m_szObjectDesc[0] != _T('\0'))
			CSingleObjectAllocStatMgr::getSingleton().addAllocStat(this);
	}

	void removeFromStat() 
	{
		if (m_szObjectDesc[0] != _T('\0'))
			CSingleObjectAllocStatMgr::getSingleton().removeAllocStat(this);
	}



protected:
	uint64			m_nUsedObjNum;							// 已经使用的对象数目
	uint64			m_nAllocObjNum;							// 分配对象的数目
	size_t			m_nObjectSize;							// 对象大小
	AllocatorType		m_allocType;							// 分配器类型
	TCHAR			m_szObjectDesc[MAX_DESC_LENGTH+1];				// 对象描述
};

/*
#endif

#ifndef _ENABLE_OBJ_COUNT_STATICS_

class CObjectAllocatorStat
{
public:
	CObjectAllocatorStat(size_t nObjectSize,int at = 2, LPCTSTR szDesc = NULL){}
	inline void addToStat() const {}
	inline void removeFromStat() const {} 
	inline uint64 getAllocStat(uint64 &usedObjects, uint64 &allocObjects) const
	{
		return 0;
	}
	inline CObjectAllocatorStat& operator = (const CObjectAllocatorStat &rhs)
	{
		return *this;
	}
	~CObjectAllocatorStat() {}
};
#endif
*/








