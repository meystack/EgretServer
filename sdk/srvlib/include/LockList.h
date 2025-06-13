#ifndef _MBASE_LOCKLIST_H_
#define _MBASE_LOCKLIST_H_

#include "List.h"
#include "Lock.h"

namespace wylib
{
	namespace container
	{
		using namespace wylib::sync::lock;

		template <typename T>
		class CLockList :
			public CBaseList<T>
		{
		public:
			typedef CBaseList<T> Inherited;
			typedef CLockList<T> ListClass;
		private:
			CBaseLock	*m_pLock;//数据锁
		public:
			CLockList(CBaseLock *plock = NULL):Inherited()
			{
				m_pLock = plock;
			}
			//获取列表锁对象
			inline CBaseLock* getLock(){ return m_pLock; }
			//设置列表锁对象，函数返回旧的列表锁
			CBaseLock* setLock(CBaseLock *pLock)
			{
				CBaseLock *pOldLock = m_pLock;
				m_pLock = pLock;
				return pOldLock;
			}
			//对列表加锁
			inline void lock()
			{
				if ( m_pLock ) m_pLock->Lock();
			}
			//尝试对列表加锁，加锁失败则返回FALSE，否则返回TRUE
			inline BOOL trylock()
			{
				return m_pLock ? m_pLock->TryLock() : TRUE;
			}
			//对列表解锁
			inline void unlock()
			{
				if ( m_pLock ) m_pLock->Unlock();
			}
		};
	};
};

#endif

