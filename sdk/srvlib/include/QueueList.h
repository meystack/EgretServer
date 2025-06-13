#ifndef _MBASE_QUEUELIST_H_
#define _MBASE_QUEUELIST_H_

#include "LockList.h"

namespace wylib
{
	namespace container
	{
		template <typename T>
		class CQueueList :
			public CLockList<T>
		{
		public:
			typedef CLockList<T> Inherited;
			typedef CQueueList<T> ListClass;
		private:
			CBaseList<T>	m_AppendList;	//数据追加列表
		public:
			//添加数据，数据将在调用flush或tryFlush是被提交到自身列表中
			inline void append(const T& data)
			{
				Inherited::lock();
				m_AppendList.add(data);
				Inherited::unlock();
			}
			inline void appendList(CBaseList<T> &list)
			{
				Inherited::lock();
				m_AppendList.addArray(list, list.count());
				Inherited::unlock();
			}
			inline void appendArray(T* data, INT_PTR length)
			{
				Inherited::lock();
				m_AppendList.addArray(data, length);
				Inherited::unlock();
			}
			//获取追加数据数量
			inline INT_PTR appendCount()
			{
				return m_AppendList.count();
			}
			//提交由append调用添加的数据
			inline void flush()
			{
				
				if ( m_AppendList.count() > 0 )
				{
					Inherited::lock();
					Inherited::addList(m_AppendList);
					m_AppendList.trunc(0);
					Inherited::unlock();
				}
				
			}
			//尝试提交由append调用添加的数据，如果获取锁失败，则无法提交
			inline BOOL tryFlush()
			{
				if ( m_AppendList.count() > 0 )
				{
					if ( Inherited::trylock() )
					{
						Inherited::addList(m_AppendList);
						m_AppendList.trunc(0);	
						Inherited::unlock();	
						return TRUE;				
					}
					else
					{
						return FALSE;
					}				
					
				}
				else
				{
					return TRUE;				
				}
			}
		};
	};
};

#endif

