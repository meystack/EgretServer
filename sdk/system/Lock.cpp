#include "_osdef.h"
#include "Lock.h"

using namespace wylib::sync::lock;

#ifdef WIN32

CCSLock::CCSLock()
{
	InitializeCriticalSection( &m_CriticalSection );
	m_pFileName =NULL;
	m_nLine =0;
}

CCSLock::~CCSLock()
{
	DeleteCriticalSection( &m_CriticalSection );
	
}

#else
#define recursive_mutex_flag PTHREAD_MUTEX_RECURSIVE_NP

//bool CCSLock::attr_initalized = false;
//pthread_mutexattr_t CCSLock::attr;

CCSLock::CCSLock()
{
	/*
	if(!attr_initalized)
	{
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, recursive_mutex_flag);
		attr_initalized = true;
	}

	pthread_mutex_init(&mutex, &attr);
	*/
	pthread_mutex_init(&mutex,NULL);
	
}

CCSLock::~CCSLock() { pthread_mutex_destroy(&mutex); }
#endif



void CCSLock::_Lock( char *sFile,int nLine)
{
#ifndef WIN32
	pthread_mutex_lock(&mutex);
#else
	EnterCriticalSection(&m_CriticalSection);
#endif
	m_pFileName =sFile;
	m_nLine= nLine;
}

BOOL CCSLock::TryLock()
{
#ifndef WIN32
	return (pthread_mutex_trylock(&mutex) == 0);
#else
	return TryEnterCriticalSection(&m_CriticalSection);
#endif
}

void CCSLock::_Unlock( char *sFile,int nLine)
{
#ifndef WIN32
	pthread_mutex_unlock(&mutex);
#else
	LeaveCriticalSection(&m_CriticalSection);
#endif
	m_pFileName =0;
	m_nLine= 0;
}


//#ifdef WIN32
CAtomLock::CAtomLock()
{
	m_uLockFlag = Unlocked;
	m_dwLockThread = 0;
	m_dwLockCount = 0;
}

CAtomLock::~CAtomLock()
{
}

void CAtomLock::_Lock( char *sFile,int nLine)
{
#ifdef WIN32
	DWORD dwThreadId = GetCurrentThreadId();
#else
	DWORD dwThreadId = pthread_self();
#endif

	if ( dwThreadId == m_dwLockThread )
	{
		m_dwLockCount++;
	}
	else
	{

		while ( Unlocked != InterlockedCompareExchange( &m_uLockFlag, Locked, Unlocked ) )
		{
			OnLockWait();
		}
		m_dwLockThread = dwThreadId;
		m_dwLockCount++;
	}
	m_pFileName =sFile;
	m_nLine= nLine;
}

BOOL CAtomLock::TryLock()
{
#ifdef WIN32
	DWORD dwThreadId = GetCurrentThreadId();
#else
	DWORD dwThreadId = pthread_self();
#endif

	if ( dwThreadId == m_dwLockThread )
	{
		m_dwLockCount++;
	}
	else
	{

		if ( Unlocked != InterlockedCompareExchange( &m_uLockFlag, Locked, Unlocked ) )
			return false;
		m_dwLockThread = dwThreadId;
		m_dwLockCount++;
	}
	return true;
}

void CAtomLock::_Unlock( char *sFile,int nLine)
{
#ifdef WIN32
	if ( GetCurrentThreadId() == m_dwLockThread )
#else
	if ( pthread_self() == m_dwLockThread ) 
#endif
	{
		m_dwLockCount--;
		if ( !m_dwLockCount )
		{
			m_uLockFlag = Unlocked;
			m_dwLockThread = 0;
		}
	}
	m_pFileName =0;
	m_nLine= 0;
}

void CAtomLock::OnLockWait()
{
}
//#endif
