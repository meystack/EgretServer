#ifndef _STR_HASH_TABLE_H_
#define _STR_HASH_TABLE_H_
/************************************************************************
*                                ��ϣ��
*
*   ��ϣ�㷨�������������ı�ѩ��ϣ�㷨��ÿ�����ڵ���3����ϣֵ�����жϣ�һ���̶��Ͽ��Լ�
* ��ķ�ֹ���ֳ�ͻ��
*
*   ������������ݵ�ʱ�����һ��hash������ʹ�ã�����������������������������ͷ��
* ʼ���ҿ�λ��ֱ֪���ҵ���λ�����һ�α�������
*
*   �ӱ��в���������ʱ�����hashֵ����������λ������2����ϣֵ��ͬ������������������
* ������������ͷ��ʼ���ҿ�λ��ֱ֪���ҵ������һ�α�������
*
*
************************************************************************/

#include "bzhash.h"

template <typename T>
class StrHashTable;

template <typename T>
class StrHashTableIterator
{
public:
	StrHashTableIterator()
	{
		m_pTable = NULL;
		m_nIndex = 0;
	}
	StrHashTableIterator(const StrHashTable<T>& table)
	{
		setTable(table);
	}
	inline void setTable(const StrHashTable<T>& table)
	{
		m_pTable = &table;
		m_nIndex = 0;
	}
	inline T* first()
	{
		int nLen = (int)(m_pTable->m_nLen);

		m_nIndex = 0;

		while (m_nIndex < nLen)
		{
			typename StrHashTable<T>::NodeType* pNode =
			    &m_pTable->m_pTable[m_nIndex];

			m_nIndex++;

			if (pNode->hash1)
				return &pNode->value;
		}

		return NULL;
	}
	inline T* next()
	{
		int nLen = (int)(m_pTable->m_nLen);

		while (m_nIndex < nLen)
		{
			typename StrHashTable<T>::NodeType* pNode = 
				&m_pTable->m_pTable[m_nIndex];

			m_nIndex++;

			if (pNode->hash1) return &pNode->value;
		}

		return NULL;
	}
private:
	const StrHashTable<T>*	m_pTable;
	int							m_nIndex;
};

template <typename T>
class StrHashTable
{
	friend class StrHashTableIterator<T>;
public:
	typedef StrHashTable<T> ClassType;

public:
	StrHashTable(size_t len = 0)
	{
		m_pTable = NULL;
		m_nLen = m_nFree = 0;
		m_nInitSize = len;

		if (len > MiniSize)
		{
			// ���Ƴ��ȱ�����2�Ĵη����������ϣ�±��㷨�޷�����
			size_t val;

			for (int i = 0; i < 32; ++i)
			{
				val = size_t(1 << i);

				if (len <= val)
				{
					m_nInitSize = val;
					break;
				}
			}
		}
		else
		{
			m_nInitSize = MiniSize;	// ���Ƴ��ȱ�����2�Ĵη����������ϣ�±��㷨�޷�����
		}
	}
	virtual ~StrHashTable()
	{
		clear();
	}
	//��չ�ϣ��
	void clear()
	{
		//ѭ��������������
		for (int i = (int)m_nLen - 1; i > -1; --i)
		{
			if (m_pTable[i].hash1)
			{
				m_pTable[i].value.~T();
			}
		}

		//�ͷ��ڴ�
		if (m_pTable) realloc(m_pTable, 0);

		m_pTable = NULL;
		m_nLen = m_nFree = 0;
	}
	//��ȡ��Ч��������
	inline size_t count() const
	{
		return m_nLen - m_nFree;
	}
protected:
	/** �����ڲ�ʹ�õĹ�ϣ�ڵ����ݽṹ **/
	template <typename TA>
	class HashNode
	{
	public:
		unsigned int hash1;	//��ϣֵ1
		unsigned int hash2;	//��ϣֵ2
		unsigned int hash3;	//��ϣֵ3
		TA value;			//����ֵ
	};

	typedef HashNode<T> NodeType;
public:
	//ͨ��������ֵ
	inline T* get(const char* sKey)
	{
		int idx = getIndex(sKey);
		return (idx >= 0) ? &m_pTable[idx].value : NULL;
	}
	//ͨ��������ֵ
	inline const T* get(const char* sKey) const
	{
		int idx = getIndex(sKey);
		return (idx >= 0) ? &m_pTable[idx].value : NULL;
	}
	/* ͨ��������ֵ
	* ���һ��hash������ʹ�ã������������������������
	* �����ͷ��ʼ���ҿ�λ��ֱ֪���ҵ���λ�����һ�α�������
	*/
	inline T* put(const char* sKey)
	{
		unsigned int hash1, idx, start;
#ifdef _MSC_VER
		unsigned int hash2, hash3;
#else
		unsigned int __attribute__ ((unused)) hash2, hash3;
#endif
		//�ڴ�ռ䲻�㣬�����ڴ�ռ�
		if (m_nFree <= 0)
		{
			size_t oldlen = m_nLen;
			m_nLen = (oldlen <= 0) ? m_nInitSize : m_nLen << 1;//�����ȱ�����2�Ĵη�
			m_nFree = m_nLen - oldlen;
			m_pTable = (NodeType*)realloc(m_pTable, m_nLen * sizeof(m_pTable[0]));
			memset(&m_pTable[oldlen], 0, m_nFree * sizeof(m_pTable[0]));
		}

		hash1 = ::bzhashstr(sKey, 0);
		hash2 = ::bzhashstr(sKey, 1);
		hash3 = ::bzhashstr(sKey, 2);
		start = idx = hash1 & ((unsigned int)m_nLen - 1);//�����ȱ�����2�Ĵη�

		do
		{
			NodeType* pNode = &m_pTable[idx];

			//�����λ��û��ֵ�������õ���λ�ã���������ҵ�һ����λ��
			if (!pNode->hash1)
			{
				pNode->hash1 = hash1;
				pNode->hash2 = ::bzhashstr(sKey, 1);
				pNode->hash3 = ::bzhashstr(sKey, 2);
				m_nFree--;
				new(&pNode->value)T();
				return &pNode->value;
			}

#ifdef _DEBUG
			else if (pNode->hash1 == hash1 && pNode->hash2 == hash2 && pNode->hash3 == hash3)
			{
				//�������ظ����ӣ�����ȷʵ���ִ���
				//DebugBreak();
			}
#endif
			idx = (idx + 1) & ((unsigned int)m_nLen - 1);//�����ȱ�����2�Ĵη�
		}
		while (start != idx);

		return NULL;
	}
	//ͨ��������ֵ
	inline int update(const char* sKey, const T& value)
	{
		int idx = getIndex(sKey);

		if (idx >= 0)
			m_pTable[idx].value = value;

		return idx;
	}
	//ͨ�����Ƴ�ֵ��û���ҵ��򷵻�-1
	inline int remove(const char* sKey)
	{
		int idx = getIndex(sKey);

		if (idx >= 0)
		{
			NodeType* pNode = &m_pTable[idx];
			pNode->hash1 = pNode->hash2 = pNode->hash3 = 0;
			m_nFree++;
			pNode->value.~T();
			return idx;
		}

		return -1;
	}
	//��ȡ���ڱ��е�����
	int getIndex(const char* sKey) const
	{
		unsigned int idx, start;
		size_t len;

		if (m_nLen <= 0)
			return -1;

		unsigned int hash1 = ::bzhashstr(sKey, 0);
		unsigned int hash2 = ::bzhashstr(sKey, 1);
		unsigned int hash3 = ::bzhashstr(sKey, 2);

		//���ȿ�ʼ�۰����
		len = m_nLen;

		while (len >= m_nInitSize)
		{
			idx = hash1 & ((unsigned int)len - 1);//�����ȱ�����2�Ĵη�
			NodeType* pNode = &m_pTable[idx];

			if (pNode->hash1 == hash1 && pNode->hash2 == hash2 && pNode->hash3 == hash3)
			{
				return idx;
			}

			len >>= 1;
		}

		//�۰���Ҳ������hashλ�ÿ�ʼ����������
		start = idx = hash1 & ((unsigned int)m_nLen - 1);//�����ȱ�����2�Ĵη�

		do
		{
			NodeType* pNode = &m_pTable[idx];

			if (pNode->hash1 == hash1 && pNode->hash2 == hash2 && pNode->hash3 == hash3)
			{
				return idx;
			}

			idx = (idx + 1) & ((unsigned int)m_nLen - 1);//�����ȱ�����2�Ĵη�
		}
		while (start != idx);

		return -1;
	}

protected:
	//�ڴ����뺯����������c�����е�reallocʵ����ͬ��ʵ�����롢��չ�Լ��ͷ��ڴ�
	virtual void* realloc(void* p, size_t s)
	{
#ifdef _MSC_VER
		static BaseAllocator alloc("bzhashtable");
		if (s > 0)
		{
			return alloc.ReAllocBuffer(p, s);
		}
		else
		{
			if (p)
			{
				alloc.FreeBuffer(p);
			}
			return NULL;
		}
		return alloc.ReAllocBuffer(p, s);
#else
		return ::realloc(p, s);
#endif
	}
protected:
	size_t		m_nInitSize;//����ʼ����
	size_t		m_nLen;		//��ϣ���ĳ���,������2�Ĵη������ϣ�±��㷨�޷�����
	size_t		m_nFree;	//���нڵ�����
	HashNode<T>*	m_pTable;	//��ϣ��

public:
	static const size_t MiniSize = 16;//��ϣ����С���ȣ�������2�Ĵη������ϣ�±��㷨�޷�����
};
#endif

