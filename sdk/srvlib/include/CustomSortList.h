#ifndef _BASE_SORT_LIST_H_
#define _BASE_SORT_LIST_H_

/**************************************************************
                                              
                     支持排序的列表类                

排序的算法采用快速怕徐法，在列表中进行的查找则使用二分查找法。    
                                                 
    模板的参数类型T必须是一个重载了==和<运算的类型。
         模板的参数K表示进行查找时的键类型。
         
  类型T的排序实现示范：
	class CSortedListItemType
	{
		int		m_nID;
		//在排序列表中的排序对比函数，与相同的类型进行对比，
		//返回0表示相等，返回负数表示比another小，否则表示比another大
		inline INT_PTR compre (const CSortedListItemType & another) const
		{ return m_nID - another.m_nID; }
		//在排序列表中的搜索对比函数，与搜索时的键类型对比
		//返回0表示相等，返回负数表示比key小，否则表示比key大
		inline INT_PTR compareKey(const INT nID) const
		{ return m_nID - nID; }
	};
**************************************************************/

#include <List.h>

template <typename T, typename K>
class CCustomSortList
	: public wylib::container::CBaseList<T>
{
public:
	typedef wylib::container::CBaseList<T> Inherited;

private:
	BOOL m_boSorted;	//是否启用排序，默认值为TRUE

public:
	CCustomSortList()
		:Inherited()
	{
		m_boSorted = TRUE;
	}
	//判断列表是否启用了排序
	inline BOOL getSorted() const { return m_boSorted; }
	//设置列表是否启用排序
	inline VOID setSorted(const BOOL boSorted)
	{
		if ( m_boSorted != boSorted )
		{
			if ( boSorted )
				sort();
			m_boSorted = boSorted;
		}
	}
	/***
		通过key搜索列表中与Key相等的数据
		如果列表中找不到与key相等的数据，函数返回-1，否则返回值表示数据在
		列表中的索引。
		如果列表启用排序，则会通过基于key与数据对比的搜索算法快速查找，如
		果未启用排序，则循环逐个对比
	***/
	INT_PTR search(const K& key) const
	{
		if ( m_boSorted )
		{
			return getIndexByKey(key);
		}
		else
		{
			INT_PTR i;
			T *DataList = *this;

			for ( i=count()-1; i>-1; --i )
			{
				if (DataList[i].compareKey(key) == 0)
					return i;
			}

			return -1;
		}
	}
	/***
		通过key搜索列表中与Key相等或大于key的最小项
		如在已排序列表{ 1, 2, 7, 9 }中查找大于等于6的最小项，则查找到的项为7。
		如果列表中找不到则函数返回-1，否则返回值表示数据在列表中的索引。
		如果列表启用排序，则会通过基于key与数据对比的搜索算法快速查找。
		★★注意★★
		如果未启用排序，则循环逐个对比并且仅当数据完全匹配的时候才返回有效索
		引而不会查找最相近的项。
	***/
	INT_PTR searchMiniGreater(const K& key) const
	{
		if ( m_boSorted )
		{
			return getMiniGreaterIndexByKey(key);
		}
		else
		{
			INT_PTR i;
			T *DataList = *this;

			for ( i=count()-1; i>-1; --i )
			{
				if (DataList[i].compareKey(key) == 0)
					return i;
			}

			return -1;
		}
	}
	/***
		覆盖插入的函数
		如果列表启用排序，则不允许插入，在调用次函数时将返回-1,否则将调用父类的插入函数并返回index
	***/
	INT_PTR insert(const INT_PTR index, const T& data)
	{
		if ( m_boSorted )
		{
			DbgAssert(FALSE);//排序列表不允许插入
			return -1;
		}
		return Inherited::insert(index, data);
	}
	/***
		覆盖添加函数
		如果列表启用排序，则会计算新添加的元素应当插入的位置并插入到列表中
		即：会为新添加的元素进行排序
	***/
	INT_PTR add(const T& data)
	{
		if ( !m_boSorted )
			return Inherited::add(data);

		//如果启用排序，则搜索到应当插入到列表中的位置
		INT_PTR newIndex = 0;
		getIndex(data, &newIndex);
		if ( newIndex > -1 )
			Inherited::insert(newIndex, data);
        return newIndex;
	}
	/***
		覆盖按索引改变数据的函数
		如果列表启用排序，则操作会被忽略，否则将改变指定索引处的数据
	***/
	inline void set(const INT_PTR index, const T &item)
	{
		if ( !m_boSorted )
		{
			DbgAssert(FALSE);//排序列表不允许修改项
			Assert( index > -1 && index < m_tCount );
			return m_pData[index] = item;
		}
	}
	/***
		覆盖获取数据在列表中的指针的函数
		如果列表启用排序，则会使用搜索函数基于算法查找，如果未启用排序，则从头到尾循环查找
	***/
	INT_PTR index(const T& data) const
	{
		if ( m_boSorted )
			return getIndex(data, NULL);
		else return Inherited::index(data);
	}
	/***
		覆盖添加新列表的函数
		如果列表启用排序，在将列表的数据添加到自身后会重新进行排序
	***/
	inline void addArray(T* data, INT_PTR length)
	{
		Inherited::addArray(data, length);
		if ( m_boSorted )
		{
			sort();
		}
	}
private:
	/***
		通过对比函数快速搜索data在列表中的索引。
		参数pInsertIndex用于输data数据可插入的位置的索引，此参数可以为空。
		如果列表中找不到与data相等的数据，函数返回-1，否则返回值表示数据在
		列表中的索引。
		无论返回值是多少，如果pInsertIndex非空，都会向其中填充data数据可插
		入的位置的索引。
	***/
	INT_PTR	getIndex(const T& data, INT_PTR *pInsertIndex) const
	{
		INT_PTR nLow = 0, nHigh = count() - 1, nIndex = 0, nValue, nResult;
		T *ListData = *this;

		nResult = -1;
		while ( nLow <= nHigh )
		{
			nIndex = (nLow + nHigh) >> 1;//nLow + (nHigh - nLow)/2
			nValue = data.compare(ListData[nIndex]);
			if ( nValue != 0 )
			{
				if ( nValue < 0 )
					nHigh = nIndex - 1;
				else nLow = ++nIndex;
			}
			else 
			{
				nResult = nIndex;
				break;
			}
		}

		if ( pInsertIndex )
			*pInsertIndex = nIndex;
		return nResult;
	}
	/***
		通过针对T的key从列表中搜索大于等于key值最小的数据在列表中的索引。
		如在已排序列表{ 1, 2, 7, 9 }中查找6的最接近项，则查找到的项为7。
	***/
	INT_PTR	getMiniGreaterIndexByKey(const K key) const
	{
		INT_PTR nLow = 0, nHigh = count() - 1, nGreaterIndex = -1, nIndex = 0, nValue;
		T *ListData = *this;

		while ( nLow <= nHigh )
		{
			nIndex = (nLow + nHigh) >> 1;//nLow + (nHigh - nLow)/2
			nValue = ListData[nIndex].compareKey( key );
			if ( nValue != 0 )
			{
				if ( nValue > 0 )
				{
					nHigh = nIndex - 1;
					nGreaterIndex = nIndex;
				}
				else nLow = ++nIndex;
			}
			else break;
		}
		return nGreaterIndex;
	}
	/***
		通过针对T的key从列表中搜索与key相等的数据
		如果列表中找不到与key相等的数据，函数返回-1，否则返回值表示数据在
		列表中的索引。
	***/
	INT_PTR getIndexByKey(const K key) const
	{
		INT_PTR nLow = 0, nHigh = count() - 1, nIndex = 0, nValue, nResult;
		T *ListData = *this;

		nResult = -1;
		while ( nLow <= nHigh )
		{
			nIndex = (nLow + nHigh) >> 1;//nLow + (nHigh - nLow)/2
			nValue = ListData[nIndex].compareKey( key );
			if ( nValue != 0 )
			{
				if ( nValue > 0 )
					nHigh = nIndex - 1;
				else nLow = ++nIndex;
			}
			else 
			{
				nResult = nIndex;
				break;
			}
		}
		return nResult;
	}
	/***
		通过对比函数对列表中的某段数据进行快速排序的排序函数
	***/
	void quickSort(INT_PTR nLow, INT_PTR nHigh)
	{
		INT_PTR i, j, p;
		T *ListData = *this;
		T temp;

		do
		{
			i = nLow;
			j = nHigh;
			p = (nLow + nHigh) >> 1;

			do
			{
				while ( ListData[i].compare( ListData[p] ) < 0 ) i++;
				while ( ListData[j].compare( ListData[p] ) > 0 ) j--;
				if ( i <= j )
				{
					memcpy(&temp, &ListData[j], sizeof(T));
					memcpy(&ListData[j], &ListData[i], sizeof(T));
					memcpy(&ListData[i], &temp, sizeof(T));
					if ( p == i )
						p = j;
					else if ( p == j )
						p = i;
					i++;
					j--;
				}
			}
			while ( i <= j );

			if ( nLow < j )
				quickSort( nLow, j );
			nLow = i;
		}
		while ( i < nHigh );
	}
	/***
		对整个列表中的数据重新排序
	***/
	inline void sort()
	{
		if (count() > 1)
		{
			quickSort(0, count()-1);
		}
	}
};

#endif

