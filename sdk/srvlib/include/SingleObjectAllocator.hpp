#pragma once

/*
                          对象内存申请器（单个对象申请）

                   ★采用对象池的技术队大量的小内存对象的申请优化★
                       ★！此内存管理器不是多线程安全的！★

   用于对连续的相同对象的小块内存申请进行优化。申请器只提供申请对象的功能并且能够将内存
 申请的效率进行最大化提升，提供释放对象所占用内存的功能。所有对象所占用的内存将在申请
 器销毁时才被释放。

*/
#include "ObjectAllocStat.hpp"

template <class T>
class CSingleObjectAllocator :	
	protected wylib::container::CBaseList<T*>,
	public CObjectAllocatorStat
	
{
public:

	typedef wylib::container::CBaseList<T*> Inherited;
	typedef CObjectAllocatorStat Inherited2;

private:
	struct DataBlock
	{
		DataBlock*	pPrev;
	}		*m_pDataBlock;	
public:	
	enum
	{
		AllocObjectCountOneTime = 4096,
	};
	//申请属性指针
	T* allocObject()
	{
		T* result;
		INT_PTR nCount = this->count();

		if ( nCount <= 0 )
		{
			//static const INT_PTR nAllocCount = 4096;
			DataBlock *pBlock = (DataBlock*)malloc(sizeof(DataBlock) + sizeof(T) * AllocObjectCountOneTime);
			memset(pBlock, 0, sizeof(DataBlock) + sizeof(T) * AllocObjectCountOneTime);
//#ifdef _ENABLE_OBJ_COUNT_STATICS_
			m_nAllocObjNum += AllocObjectCountOneTime;
//#endif
			//将对象指针全部添加到空闲列表中
			result = (T*)(pBlock + 1);
			if ( this->maxCount() < AllocObjectCountOneTime )
				this->reserve(AllocObjectCountOneTime);
			for (INT_PTR i=0; i<AllocObjectCountOneTime; ++i)
			{
				//(*this)[i] = result;
				this->add( result ) ;
				result++;
			}
			nCount = AllocObjectCountOneTime;
			//标记上内存块的上一个内存块
			pBlock->pPrev = m_pDataBlock;
			m_pDataBlock = pBlock;
		}
		//返回列表中的最后一个对象
		//nCount--;
		//result = (*this)[nCount];
		result = (T*)this->pop() ;
		//trunc(nCount);
//#ifdef _ENABLE_OBJ_COUNT_STATICS_
		m_nUsedObjNum++;
//#endif
		return result;
	}
	//获得元素的数目
	inline INT_PTR GetCount()
	{
		return this->count();
	}
	//获得第index的指针
	T* GetIndex(INT_PTR nIndex)
	{
		if(nIndex <0 || nIndex >= this->count()) return NULL;
		return (*this)[nIndex];
	}


	//释放一个对象
	inline void freeObject(T* pObject)
	{
		//add(pObject);
		this->add(pObject);
//#ifdef _ENABLE_OBJ_COUNT_STATICS_
		m_nUsedObjNum--;
//#endif
	}
//#ifdef _ENABLE_OBJ_COUNT_STATICS_
	CSingleObjectAllocator(LPCTSTR lpszDesc)
		:Inherited(), Inherited2(sizeof(T), 2, lpszDesc)
	{
		m_pDataBlock = NULL;
	}
/*
#else
	CSingleObjectAllocator(LPCTSTR lpszDesc):Inherited()
	{
		m_pDataBlock = NULL;
	}
*/

//#endif
	~CSingleObjectAllocator()
	{
		DataBlock *pBlock, *pPrev; 
		pBlock = m_pDataBlock;
		while (pBlock)
		{
			pPrev = pBlock->pPrev;
			free(pBlock);
			pBlock = pPrev;
		}
		m_pDataBlock = NULL;
	}		
};
