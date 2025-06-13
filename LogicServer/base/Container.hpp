#pragma once
#include "Container.h"
/***************************************************************

                     容器,使用小内存管理

***************************************************************/

template<class DATA>
CList<DATA>::CList(CBufferAllocator* pAllocator)
{
	wylib::container::CBaseLinkedList<DATA>();
	if (pAllocator)
		m_pAllocator = pAllocator;
#ifndef __ROBOT_CLIENT__
	else
		m_pAllocator = GetLogicServer()->GetBuffAllocator();
#endif
}

template<class DATA>
CList<DATA>::~CList()
{
	CBaseLinkedList<DATA>::clear();
}

template<class DATA>
CLinkedNode<DATA>* CList<DATA>::allocNode()
{
	CLinkedNode<DATA>* result = (CLinkedNode<DATA>*)m_pAllocator->AllocBuffer(sizeof(CLinkedNode<DATA>));
	if(result == NULL ) return NULL;
	if(result)
	{
		new(&result->m_Data)DATA();//placement new
	}
	return result;
}

template<class DATA>
VOID CList<DATA>::freeNode(CLinkedNode<DATA> *pNode)
{
	if(pNode == NULL) return;
	if(pNode)
	{
		(&pNode->m_Data)->~DATA();
	}
	m_pAllocator->FreeBuffer(PVOID(pNode));
}

template<class DATA>
CVector<DATA>::CVector(CBufferAllocator* pAllocator)
{
	wylib::container::CBaseList<DATA>() ;
	if (pAllocator)
		m_pAllocator = pAllocator;
#ifndef __ROBOT_CLIENT__
	else
		m_pAllocator = GetLogicServer()->GetBuffAllocator();
#endif
}

template<class DATA>
CVector<DATA>::~CVector()
{
	empty();
}

template<class DATA>
void CVector<DATA>::empty()
{
	CBaseList<DATA>::clear();
	this->m_tMaxCount = 0;
	if(this->m_pData)
	{
		m_pAllocator->FreeBuffer(PVOID(this->m_pData));
	}
	this->m_pData = NULL;
}

template<class DATA>
void CVector<DATA>::reserve(INT_PTR count)
{
	if ( count > this->m_tCount && count != this->m_tMaxCount )
	{
		PVOID *pData=(PVOID *) m_pAllocator->AllocBuffer(sizeof(DATA) * count);
		if(this->m_pData && this->m_tMaxCount >0)
		{
			memcpy(pData,this->m_pData,sizeof(DATA)*this->m_tMaxCount);
			m_pAllocator->FreeBuffer(PVOID(this->m_pData));
		}
		if (this->m_pData && this->m_tMaxCount <= 0)
		{
			Assert(FALSE);
		}
		this->m_pData = (DATA*)pData;
		this->m_tMaxCount = count;
	}
}
