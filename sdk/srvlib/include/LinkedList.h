#ifndef _LINKED_LIST_H_
#define _LINKED_LIST_H_
/******************************************************************
 *
 *							wyLib库	2008 - 2010
 *
 *	$ 链表类 $
 *  
 *  - 主要功能 - 
 *
 *	实现动态添加、插入节点。与CBaseList一样，链表类也是一个容器类，但与
 *  CBaseList比较，链表在进行删除节点的时候能够更大的发挥效率，缺点则是比列表
 *  更加浪费内存（内存开销为3~4倍）且在按索引访问的时候效率极低。
 *
 *****************************************************************/
#include "ObjectCounter.h"
#include "TypeDef.h"

#include "_ast.h"
namespace wylib
{
	namespace container
	{
		template <typename T>
		class CBaseLinkedList;

		template <typename T>
		class CLinkedListIterator;

		/*************************
		* 链表节点类
		**************************/
		template <typename T>
		class CLinkedNode : public Counter<CLinkedNode<T> >
		{
			friend class CBaseLinkedList<T>;
			friend class CLinkedListIterator<T>;
		public:
			typedef T InstancesType;
		public:
			inline operator T& (){ return m_Data; }
			inline CLinkedNode<T>* prev(){ return m_pPrevNode; }
			inline CLinkedNode<T>* next(){ return m_pNextNode; }
			inline CBaseLinkedList<T>* list(){ return m_pOwnList; }
			T				m_Data;
		private:			
			CLinkedNode<T>*	m_pPrevNode;
			CLinkedNode<T>* m_pNextNode;
			CBaseLinkedList<T>* m_pOwnList;
		};

		/*************************
		* 链表迭代器类
		**************************/
		template <typename T>
		class CLinkedListIterator
		{
			friend class CBaseLinkedList<T>;
		public:
			CLinkedListIterator()
			{
				m_pList = NULL;
				m_pEnuming = NULL;
				m_pPrevIterator = m_pNextIterator = NULL;
			}
			CLinkedListIterator(CBaseLinkedList<T> &list)
			{
				m_pList = NULL;
				m_pEnuming = NULL;
				m_pPrevIterator = m_pNextIterator = NULL;
				setList(list);
			}
			~CLinkedListIterator()
			{
				if (m_pList)
				{
					if (m_pPrevIterator) m_pPrevIterator->m_pNextIterator = m_pNextIterator;
					if (m_pNextIterator) m_pNextIterator->m_pPrevIterator = m_pPrevIterator;
					if (this == m_pList->m_pLastIterator)
						m_pList->m_pLastIterator = m_pPrevIterator;
					m_pList = NULL;
					m_pEnuming = NULL;
					m_pPrevIterator = m_pNextIterator = NULL;
				}
			}
			//重新设置遍历对象
			inline VOID setList(CBaseLinkedList<T> &list)
			{
				this->~CLinkedListIterator();
				m_pList = &list;
				m_pEnuming = NULL;
				m_pNextIterator = NULL;
				m_pPrevIterator = list.m_pLastIterator;
				if (m_pPrevIterator) m_pPrevIterator->m_pNextIterator = this;
				list.m_pLastIterator = this;
			}
			//移除一个节点，方便在迭代的过程中移除节点从而破坏迭代过程
			inline void remove(CLinkedNode<T>* pNode, BOOL boFree = TRUE)
			{
				if (pNode == m_pEnuming)
					m_pEnuming = pNode->m_pPrevNode;
				m_pList->remove(pNode, boFree);
			}
			//从头开始遍历节点
			inline CLinkedNode<T>* first(){ return m_pEnuming = m_pList->first(); }
			//遍历下一个节点
			inline CLinkedNode<T>* next(){ return m_pEnuming = (m_pEnuming ? m_pEnuming->m_pNextNode : m_pList->first()); }
			//当前遍历到的节点
			inline CLinkedNode<T>* current(){ return m_pEnuming; };
		private:
			CLinkedNode<T>*		m_pEnuming;
			CBaseLinkedList<T>*	m_pList;
			CLinkedListIterator<T>* m_pPrevIterator;
			CLinkedListIterator<T>* m_pNextIterator;
		};

		/*************************
		* 链表类
		**************************/
		template <typename T>
		class CBaseLinkedList
		{
			friend class CLinkedListIterator<T>;
		public:
			typedef T InstancesType;
			typedef CLinkedNode<T> NodeType;
			typedef CLinkedListIterator<T> Iterator;
		public:
			CBaseLinkedList()
			{
				m_pFirst = m_pLast = NULL;
				m_pLastIterator = NULL;
				m_nNodeCount = 0;
			}
			virtual ~CBaseLinkedList()
			{
				clear();
			}
			//获取第一个节点
			inline CLinkedNode<T>* first(){ return m_pFirst; }
			//获取最后一个节点
			inline CLinkedNode<T>* last(){ return m_pLast; }
			//获取节点数量
			inline INT_PTR count() const { return m_nNodeCount; }
			/* 在pNode节点的前面插入新节点
			 * 如果pNode为空则表示插入到链表的起始位置
			 */
			CLinkedNode<T> * linkBefore(const T& data, CLinkedNode<T> *pNode = NULL)
			{
				Assert(!pNode || pNode->m_pOwnList == this);

				if ( !pNode ) pNode = m_pFirst;
				CLinkedNode<T> *pPrev = pNode ? pNode->m_pPrevNode : NULL;
				CLinkedNode<T> *pNewNode = allocNode();

				pNewNode->m_Data = data;
				pNewNode->m_pPrevNode = pPrev;
				pNewNode->m_pNextNode = pNode;

				if ( pPrev )
					pPrev->m_pNextNode = pNewNode;
				else m_pFirst = pNewNode;

				if ( pNode ) 
					pNode->m_pPrevNode = pNewNode;
				else m_pLast = pNewNode;

				pNewNode->m_pOwnList = this;
				m_nNodeCount++;
				return pNewNode;
			}
			/* 在pNode节点的后面插入新节点
			 * 如果pNode为空则表示插入到链表的末尾
			 */
			CLinkedNode<T> * linkAfter(const T& data, CLinkedNode<T> *pNode = NULL)
			{
				//Assert(!pNode || pNode->m_pOwnList == this);

				if ( !pNode ) pNode = m_pLast;
				CLinkedNode<T> *pNext = pNode ? pNode->m_pNextNode : NULL;
				CLinkedNode<T> *pNewNode = allocNode();

				pNewNode->m_Data = data;
				pNewNode->m_pPrevNode = pNode;
				pNewNode->m_pNextNode = pNext;

				if ( pNext )
					pNext->m_pPrevNode = pNewNode;
				else m_pLast = pNewNode;

				if ( pNode ) 
					pNode->m_pNextNode = pNewNode;
				else m_pFirst = pNewNode;

				pNewNode->m_pOwnList = this;
				m_nNodeCount++;
				return pNewNode;
			}

			//重载linkAfer，插入一个已有的node
			CLinkedNode<T> * Transfer(CLinkedNode<T> *pNewNode, CLinkedNode<T> *pNode = NULL)
			{
				Assert(!pNode || pNode->m_pOwnList == this);
				Assert(pNewNode->m_pOwnList == NULL);

				if ( !pNode ) pNode = m_pLast;
				CLinkedNode<T> *pNext = pNode ? pNode->m_pNextNode : NULL;

				pNewNode->m_pPrevNode = pNode;
				pNewNode->m_pNextNode = pNext;

				if ( pNext )
					pNext->m_pPrevNode = pNewNode;
				else m_pLast = pNewNode;

				if ( pNode ) 
					pNode->m_pNextNode = pNewNode;
				else m_pFirst = pNewNode;

				pNewNode->m_pOwnList = this;
				m_nNodeCount++;
				return pNewNode;
			}
			//将数据插入到链表头部
			inline CLinkedNode<T>* linkAtFirst(const T& data)
			{
				return inkBefore(data, m_pFirst);
			}
			//将数据添加到链表尾部
			inline CLinkedNode<T>* linkAtLast(const T& data)
			{
				return linkAfter(data, m_pLast);
			}
			inline CLinkedNode<T>* TransferAtLast(CLinkedNode<T> *pNewNode)
			{
				return Transfer(pNewNode, m_pLast);
			}
			//通过索引获取链表节点
			CLinkedNode<T> *getNodeAt(INT_PTR nIndex)
			{
				for (CLinkedNode<T> *pNode = m_pFirst; pNode; pNode = pNode->m_pNextNode)
				{
					if ( nIndex <= 0 )
						return pNode;
					nIndex--;
				}
				return NULL;
			}
			/* 移除一个链表节点
			 * ★注意★ 如果正在通过迭代器遍历链表，则移除操作必须调用迭代器的CLinkedListIterator::remove函数
			 */
			inline VOID remove(CLinkedNode<T> *pNode,BOOL boFree=TRUE)
			{
				if ( pNode && pNode->m_pOwnList == this )
				{
					//移除节点
					if ( pNode->m_pPrevNode )
						pNode->m_pPrevNode->m_pNextNode = pNode->m_pNextNode;
					if ( pNode->m_pNextNode )
						pNode->m_pNextNode->m_pPrevNode = pNode->m_pPrevNode;
					if ( pNode == m_pFirst )
						m_pFirst = pNode->m_pNextNode;
					if ( pNode == m_pLast )
						m_pLast = pNode->m_pPrevNode;
					//从所有迭代器中测试并修正当前遍历节点
					if (m_pLastIterator)
					{
						Iterator *pIt = m_pLastIterator;
						while (pIt)
						{
							if (pNode == pIt->m_pEnuming)
								pIt->m_pEnuming = pNode->m_pPrevNode;
							//修复了如果迭代器本身出现问题的时候，可能引起死循环的问题
							if(pIt->m_pPrevIterator != pIt)
							{
								pIt = pIt->m_pPrevIterator;
							}
							else
							{
								break;
							}
						}
					}
					pNode->m_pOwnList = NULL;
					//销毁节点
					if (boFree)	freeNode(pNode);
					m_nNodeCount--;
					#ifdef WIN32
						Assert(m_nNodeCount >= 0);
					#endif
				}
			}
			//清空链表
			virtual VOID clear()
			{
				CLinkedNode<T> *pNode = m_pFirst, *pNextNode;
				while ( pNode )
				{
					pNextNode = pNode->m_pNextNode;
					freeNode(pNode);
					pNode = pNextNode;
				}
				m_pFirst = m_pLast = NULL;
				m_nNodeCount = 0;
			}
		protected:
			/* 申请一个链表节点对象，默认的操作时使用new申请内存并构造
			 * 子类可通过覆盖此方法实现链表节点的内存管理
			 */
			virtual CLinkedNode<T>* allocNode()
			{
				return new CLinkedNode<T>;
			}
			/* 销毁一个链表节点对象，默认的操作时使用delete析构并释放内存
			 * 子类可通过覆盖此方法实现链表节点的内存管理
			 */
			virtual VOID freeNode(CLinkedNode<T> *pNode)
			{
				delete pNode;
			}
		private:
			CLinkedNode<T> *m_pFirst;
			CLinkedNode<T> *m_pLast;
			Iterator	   *m_pLastIterator;
			INT_PTR			m_nNodeCount;
		};
	};
};

#endif

