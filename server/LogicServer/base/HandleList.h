#pragma once
#include <set>
/*
	使用内存管理器扩展了一下List链表,
*/
typedef Handle<UINT>  LinkNodeHandle;


typedef Handle<UINT>  CEntityListHandle;

typedef CVector<EntityHandle>	EntityVector;

template<typename DATA, typename TAG>
class CHandleList:
	public CBaseLinkedList<DATA>
{
public:
	//typedef HandleMgr<CLinkedNode<DATA>,LinkNodeHandle> LinkNodeMgr;
	typedef CSingleObjectAllocator< CLinkedNode<DATA> > LinkNodeMgr;
	virtual ~CHandleList()
	{
		this->clear();
	}
	static  LinkNodeMgr * GetAlloc()
	{
		static LinkNodeMgr  *pMgr = new LinkNodeMgr("CHandleList")  ;
		return pMgr;
	}
protected:
	virtual CLinkedNode<DATA>* allocNode()
	{
		//if(g_LinkNodeMgr ==NULL) return;
		//LinkNodeHandle hHandle;
		//CLinkedNode<DATA>* result = g_LinkNodeMgr->allocObject(); //g_LinkNodeMgr->Acquire(hHandle);//这里可能返回NULL
		//if(result ==NULL ) return NULL;
		//return result;
		return GetAlloc()->allocObject();
	}

	virtual VOID freeNode(CLinkedNode<DATA> *pNode)
	{
		//if(g_LinkNodeMgr ==NULL) return; //
		//g_LinkNodeMgr->freeObject(pNode);//放回内存池
		GetAlloc()->freeObject(pNode);//放回内存池
	}
//public:
//	CEntityListHandle	m_hHandle;

public:
	//static LinkNodeMgr*	g_LinkNodeMgr;	
};

typedef CHandleList<EntityHandle,UINT> CEntityList;
//template<> CEntityList::LinkNodeMgr*	CEntityList::g_LinkNodeMgr = new LinkNodeMgr("CHandleList") ;

// 有序的实体列表
typedef std::set<Uint64>	SequenceEntityList;
typedef SequenceEntityList::iterator SequenceEntityListIter;
