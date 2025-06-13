#pragma once

template <class T>
class HashNode
{
public:
	unsigned int hash1;	//哈希值1
	unsigned int hash2;	//哈希值2
	unsigned int hash3;	//哈希值3
	T value;			//数据值
	
};

#include "bzhash.h"
template <class T>
class CCustomHashTable
{
public:
	typedef CCustomHashTable<T> ClassType;
	typedef HashNode<T> NodeType;
	
public:
	CCustomHashTable(size_t len = 0)
	{
		m_pTable = NULL;
		m_nLen = m_nFree = 0;
		m_nInitSize = len;
		if (len > MiniSize)
		{
			//限制长度必须是2的次方数，否则哈希下标算法无法工作
			size_t val;
			for (int i=0; i<32; ++i)
			{
				val = size_t(1 << i);
				if (len <= val)
				{
					m_nInitSize = val;
					break;
				}
			}
		}
		else m_nInitSize = MiniSize;//限制长度必须是2的次方数，否则哈希下标算法无法工作
	}
	virtual ~CCustomHashTable()
	{
		clear();
	}
	//清空哈希表
	void clear()
	{
		//循环调用析构函数
		for (INT_PTR i=m_nLen-1; i>-1; --i)
		{
			if (m_pTable[i].hash1)
			{
				m_pTable[i].value.~T();
			}
		}
		//释放内存
		if (m_pTable) realloc(m_pTable, 0);
		m_pTable = NULL;
		m_nLen = m_nFree = 0;
	}
	//获取有效对象数量
	inline size_t count() const { return m_nLen - m_nFree; }

public:
	//通过键查找值
	inline T* get(const char* sKey)
	{
		INT_PTR idx = getIndex(sKey);
		return (idx >= 0) ? &m_pTable[idx].value : NULL;
	}
	//通过键查找值
	inline const T* get(const char* sKey) const
	{
		INT_PTR idx = getIndex(sKey);
		return (idx >= 0) ? &m_pTable[idx].value : NULL;
	}
	/* 通过键添加值
	* 如果一个hash索引被使用，则向后递推索引，到达最大索
	* 引则从头开始查找空位，知直到找到空位或完成一次表遍历。
	*/
	inline T* put(const char* sKey)
	{
		unsigned int hash1, hash2, hash3, idx, start;
		NodeType *pNode;

		//内存空间不足，增长内存空间
		if (m_nFree <= 0)
		{
			size_t oldlen = m_nLen;
			m_nLen = (oldlen <= 0) ? m_nInitSize : m_nLen << 1;//表长度必须是2的次方
			m_nFree = m_nLen - oldlen;
			m_pTable = (NodeType *)realloc(m_pTable, m_nLen * sizeof(m_pTable[0]));
			memset(&m_pTable[oldlen], 0, m_nFree * sizeof(m_pTable[0]));
		}

		hash1 = bzhashstr(sKey, 0);
		hash2 = bzhashstr(sKey, 1);
		hash3 = bzhashstr(sKey, 2);
		start = idx = hash1 & (m_nLen - 1);//表长度必须是2的次方
		do 
		{
			pNode = &m_pTable[idx];
			//如果该位置没有值，则设置到该位置，否则向后找到一个空位置
			if (!pNode->hash1)
			{
				pNode->hash1 = hash1;
				pNode->hash2 = bzhashstr(sKey, 1);
				pNode->hash3 = bzhashstr(sKey, 2);
				m_nFree--;
				new (&pNode->value)T();
				return &pNode->value;
			}
#ifdef _DEBUG
			else if (pNode->hash1 == hash1 && pNode->hash2 == hash2 && pNode->hash3 == hash3)
			{
				//调用者重复添加，或者确实出现错误！
				DebugBreak();
			}
#endif
			idx = (idx + 1) & (m_nLen - 1);//表长度必须是2的次方
		} 
		while (start != idx);

		return NULL;
	}
	//通过键更新值
	inline INT_PTR update(const char* sKey, const T &value)
	{
		INT_PTR idx = getIndex(sKey);
		if (idx >= 0)
			m_pTable[idx].value = value;
		return idx;
	}
	//通过键移除值，没有找到则返回-1
	inline INT_PTR remove(const char* sKey)
	{
		INT_PTR idx = getIndex(sKey);
		NodeType *pNode;
		if (idx >= 0)
		{
			pNode = &m_pTable[idx];
			pNode->hash1 = pNode->hash2 = pNode->hash3 = 0;
			m_nFree++;
			pNode->value.~T();
			return idx;
		}
		return -1;
	}
	//获取键在表中的索引
	INT_PTR getIndex(const char* sKey) const
	{
		unsigned int hash1, hash2, hash3;
		unsigned int idx, start;
		size_t len;
		NodeType *pNode;

		if (m_nLen <= 0)
			return -1;

		hash1 = bzhashstr(sKey, 0);
		hash2 = bzhashstr(sKey, 1);
		hash3 = bzhashstr(sKey, 2);

		//首先开始折半查找
		len = m_nLen;
		while (len >= m_nInitSize)
		{
			idx = hash1 & (len - 1);//表长度必须是2的次方
			pNode = &m_pTable[idx];
			if (pNode->hash1 == hash1 && pNode->hash2 == hash2 && pNode->hash3 == hash3)
			{
				return idx;
			}
			len >>= 1;
		}

		//折半查找不到则从hash位置开始遍历整个表
		start = idx = hash1 & (m_nLen - 1);//表长度必须是2的次方
		do 
		{
			pNode = &m_pTable[idx];
			if (pNode->hash1 == hash1 && pNode->hash2 == hash2 && pNode->hash3 == hash3)
			{
				return idx;
			}
			idx = (idx + 1) & (m_nLen - 1);//表长度必须是2的次方
		}
		while (start != idx);
		return -1;
	}

protected:
	//内存申请函数，作用与c函数中的realloc实现相同，实现申请、扩展以及释放内存
	virtual void* realloc(void* p, size_t s)
	{
		return ::realloc(p, s);
	}
protected:
	size_t		m_nInitSize;//表初始长度
	
	size_t		m_nFree;	//空闲节点数量
	
	
public:
	static const size_t MiniSize = 16;//哈希表最小长度，必须是2的次方否则哈希下标算法无法工作
	HashNode<T>	*m_pTable;	//哈希表
	size_t		m_nLen;		//哈希表的长度,必须是2的次方否则哈希下标算法无法工作
};



template <typename T>
class CHashTableIterator
{
	friend class CCustomHashTable<T>;
public:
	CHashTableIterator()
	{
		m_pTable = NULL;
		m_nIndex = 0;
	}
	CHashTableIterator(const CCustomHashTable<T> &table)
	{
		setTable(table);
	}
	inline void setTable(const CCustomHashTable<T> &table)
	{
		m_pTable = &table;
		m_nIndex = 0;
	}
	inline T* first()
	{
		INT_PTR nLen = m_pTable->m_nLen;
		HashNode<T> *pNode;

		m_nIndex = 0;
		while (m_nIndex < nLen)
		{
			pNode = &m_pTable->m_pTable[m_nIndex];
			m_nIndex++;
			if (pNode->hash1)
				return &pNode->value;
		}
		return NULL;
	}
	inline T* next()
	{
		INT_PTR nLen = m_pTable->m_nLen;
		HashNode<T> *pNode;

		while (m_nIndex < nLen)
		{
			pNode = &m_pTable->m_pTable[m_nIndex];
			m_nIndex++;
			if (pNode->hash1)
				return &pNode->value;
		}
		return NULL;
	}
private:
	const CCustomHashTable<T>*m_pTable;
	INT_PTR				m_nIndex;
};



