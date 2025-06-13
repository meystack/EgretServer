#ifndef _MLOCK_H_
#define	_MLOCK_H_

/******************************************************************
 *
 *	wyLib库数互斥锁类
 *
 *  主要功能  *
 *		进程内部的互斥锁。Windows中提供临界区锁(CCSLock)和原子锁(
 *		CAtomLock)两种互斥实现方式。另外提供一个可在函数内部安全
 *		使用的CSafeLock类。
 *
 *****************************************************************/

namespace wylib
{
	namespace sync
	{
		namespace lock
		{
			/*	基础锁类
				抽象类
			*/
			class CBaseLock
			{
			public:


				virtual void _Lock( char * sFile, int nLine) = 0;
				virtual BOOL TryLock() = 0;
				virtual void _Unlock( char * sFile, int nLine) = 0;


				//为了定位死锁问题，增加了锁定者的位置
				#define Lock() _Lock( (char *)__FILE__, __LINE__)
				#define Unlock() _Unlock( (char *)__FILE__, __LINE__)

			protected:
				char *          m_pFileName;  //锁定者的文件名
				int             m_nLine;      //锁定者的行数
			};

			class CCSLock
				: public CBaseLock
			{
			public:
				typedef CBaseLock Inherited;
			private:
#ifdef WIN32
				CRITICAL_SECTION m_CriticalSection;
#else
				//static bool attr_initalized;
				//static pthread_mutexattr_t attr;
				pthread_mutex_t mutex;
				
				
#endif
			public:
				CCSLock();
				virtual ~CCSLock();
				
				void _Lock( char * sFile, int nLine);
				
				void _Unlock( char * sFile, int nLine);

				BOOL  TryLock();
			
				
			};
//#ifdef WIN32
			class CAtomLock
				: public CBaseLock
			{
			public:
				typedef CBaseLock Inherited;
				enum AtomLockState
				{
					Unlocked = 0,
					Locked = 1,
				};
			private:
				LONG volatile m_uLockFlag;
				DWORD volatile m_dwLockThread;
				DWORD volatile m_dwLockCount;
			public:
				CAtomLock();
				virtual ~CAtomLock();
				void _Lock( char * sFile, int nLine);
				BOOL TryLock();
				void _Unlock( char * sFile, int nLine);
				virtual void OnLockWait();
			};
//#else
//	#define CAtomLock CCSLock
//#endif
			class CSafeLock
			{
			private:
				CBaseLock *m_pLock;
			public:
				inline CSafeLock(CBaseLock *pLock)
				{
					m_pLock = pLock;
					pLock->Lock();
				}
				inline ~CSafeLock()
				{
					m_pLock->Unlock();
				}
			};
		};
	};
};

#endif

