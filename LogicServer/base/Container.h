
#pragma once
#include "List.h"

/***************************************************************/
/*
/*                     容器,使用小内存管理
/*
/***************************************************************/
template<class DATA>
class CList:
	public wylib::container::CBaseLinkedList<DATA>
{
public:
	CList(CBufferAllocator* pAllocator=NULL);

	virtual ~CList();

	virtual CLinkedNode<DATA>* allocNode();

	virtual VOID freeNode(CLinkedNode<DATA> *pNode);

protected:
	CBufferAllocator* m_pAllocator;
};


template<class DATA>
class CVector:
	public wylib::container::CBaseList<DATA>
{
public:
	CVector(CBufferAllocator* pAllocator=NULL);

	virtual~CVector();
public:
	//重载一下
	virtual void empty();

	virtual void reserve(INT_PTR count);
protected:
	CBufferAllocator* m_pAllocator;
};
