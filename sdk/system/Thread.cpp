#include "_osdef.h"
#include "Thread.h"
#include "stdio.h"
using namespace wylib::thread;




CBaseThread::CBaseThread(char *sThreadName,THREAD_CALLBACK callFun,void * pData)
{
	m_hThread = 0;
	m_dwThreadId = 0;
	m_boWorking =0;  //没有启动
	m_boHasStoped =0;  //是否已经停止完成了
	//m_boTerminated = false;

	m_callFun =callFun;
	m_pCallBackData =pData;
	
	if(sThreadName !=NULL)
	{
		strncpy(m_threadName,sThreadName,sizeof(m_threadName));
	}
	else
	{
		m_threadName[0]=0;
	}

	#ifdef WIN32
		m_hWorkThreadEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);  //等待线程启动事件发生
	#else
		m_boSuspend = false;
		pthread_mutex_init(&m_Mutex, NULL);
		pthread_cond_init(&m_Cond, NULL);
	#endif
}

CBaseThread::~CBaseThread()
{
	StopWorkThread();
	
#ifndef WIN32
	pthread_mutex_destroy(&m_Mutex);
	pthread_cond_destroy(&m_Cond);
#endif

}

THREAD_HANDLE CBaseThread::CreateThreadHandle(const DWORD dwFlags)
{
#ifdef WIN32
	m_hThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)ThreadRoutine, this, dwFlags, &m_dwThreadId);
#else
	pthread_create(&m_hThread, NULL, ThreadRoutine, (void*)this);
	//suspend();
	m_dwThreadId = m_hThread;
	printf("[thread%s-%u]create\n\r",m_threadName,(unsigned int)m_dwThreadId);
#endif
	return m_hThread;
}


#ifdef WIN32
unsigned int CALLBACK CBaseThread::ThreadRoutine(void *lpThread)
{
	if(lpThread ==NULL) return 0 ;

	CBaseThread *pThread = (CBaseThread *)lpThread; 
	
	//唤醒线程
	if(pThread->m_hWorkThreadEvent != INVALID_HANDLE_VALUE)
	{
		
		SetEvent(pThread->m_hWorkThreadEvent);
	}
	
	InterlockedCompareExchange(&pThread->m_boWorking, TRUE, FALSE );
	pThread->OnThreadStarted();

	//如果不传参数进来，则使用本身的继承的方式
	if(pThread->m_callFun ==NULL)
	{
		
		while(pThread->m_boWorking)
		{
			pThread->OnRountine();
		}
	}
	else
	{
		while(pThread->m_boWorking )
		{
			pThread->m_callFun(pThread->m_pCallBackData); //调用
		}
	}
	
	if(pThread)
	{
		pThread->OnTerminated();
		
		SetEvent(pThread->m_hWorkThreadEvent); //让等待的人退出
		InterlockedCompareExchange(&pThread->m_boHasStoped, TRUE, FALSE ); //表示已经停止了
	}
	

	ExitThread(0);
	return 0;
}
#else
void * CBaseThread::ThreadRoutine(void * lpThread)
{
	if(lpThread ==NULL) return 0;
	CBaseThread *pThread = (CBaseThread *)lpThread; 
	char * sName = pThread->m_threadName;
	unsigned int nThreadId =  pThread->m_dwThreadId;

	printf("[thread %s-%u] ThreadRoutine\n\r",sName,nThreadId);
	
	//唤醒母线程
	pthread_mutex_t * pMutex = &( pThread->m_Mutex);
	pthread_cond_t * pCond = &( pThread->m_Cond);

	pthread_mutex_lock(pMutex);
	pthread_cond_signal(pCond);
	pthread_mutex_unlock(pMutex);

	InterlockedCompareExchange(&pThread->m_boWorking, TRUE, FALSE );
	pThread->OnThreadStarted();
	if(pThread->m_callFun ==NULL)
	{
		while(pThread->m_boWorking)
		{
			pThread->OnRountine();
		}
	}
	else
	{
		while(pThread->m_boWorking)
		{
			pThread->m_callFun(pThread->m_pCallBackData); //调用
		}
	}
	pThread->OnTerminated();
	
	pMutex = &( pThread->m_Mutex);
	pCond = &( pThread->m_Cond);

	pthread_mutex_lock(pMutex);
	pthread_cond_signal(pCond);
	pthread_mutex_unlock(pMutex);
	InterlockedCompareExchange(&pThread->m_boHasStoped, TRUE, FALSE ); //表示已经停止了
	
	printf("[thread %s-%u]exit\n\r",sName,nThreadId);
	//InterlockedCompareExchange(&pThread->m_boWorking, FALSE, TRUE );
	
	pthread_exit(0);
}
#endif



void CBaseThread::OnRountine()
{
}



void CBaseThread::OnSuspend(int nSuspendCount)
{
}

void CBaseThread::OnResume(int nSuspendCount)
{
}

void CBaseThread::OnTerminated()
{
}

int CBaseThread::suspend()
{
	
	int dwSuspended;
#ifdef WIN32
	dwSuspended = SuspendThread( m_hThread );
#else
	if(m_boSuspend ) return -1;
	pthread_mutex_lock(&m_Mutex);
	m_boSuspend =true;
	dwSuspended = pthread_cond_wait(&m_Cond, &m_Mutex);
	pthread_mutex_unlock(&m_Mutex);

#endif
	if ( dwSuspended != -1 )
	{
		OnSuspend(dwSuspended + 1);
		return dwSuspended + 1;
	}
	return dwSuspended;
}

int CBaseThread::resume()
{
	printf("[thread %s-%u]resume\n\r",m_threadName,(unsigned int)m_dwThreadId);
	int dwSuspended;
#ifdef WIN32
	dwSuspended = ResumeThread( m_hThread );
#else

	pthread_mutex_lock(&m_Mutex);
	m_boSuspend =false;	
	dwSuspended = pthread_cond_signal(&m_Cond);
	pthread_mutex_unlock(&m_Mutex);

	printf("[thread %s-%u]resume end\n\r",m_threadName,(unsigned int)m_dwThreadId);

#endif
	if ( dwSuspended != -1 )
	{
		OnResume(dwSuspended - 1);
		return dwSuspended - 1;
	}
	return dwSuspended;
}

int CBaseThread::getPriority()
{
#ifdef WIN32
	return GetThreadPriority( m_hThread );
#else
	return getpriority(PRIO_PROCESS, m_dwThreadId);
#endif
}

bool CBaseThread::setPriority(int nPriority)
{
#ifdef WIN32
	return SetThreadPriority( m_hThread, nPriority ) != 0;
#else
	return setpriority(PRIO_PROCESS, m_dwThreadId, nPriority);
#endif
}

int CBaseThread::waitFor(DWORD dwWaitLong, bool boWaitAlertAble)
{
	if(FALSE== InterlockedCompareExchange(&m_boHasStoped, FALSE, FALSE) )  //如果引擎已经启动得话
	{

#ifdef WIN32
	return WaitForSingleObjectEx( m_hWorkThreadEvent, dwWaitLong, boWaitAlertAble );
#else
	return pthread_join(m_hThread, NULL); //等待一个线程的结束
#endif
	}
	return 0;
}

void CBaseThread::terminate()
{
	InterlockedCompareExchange(&m_boWorking, FALSE, TRUE); //

}

// 开始工作，创建工作线程发送数据
void CBaseThread::StartWorkThread()
{
	if( m_hThread !=0 )return;

	if( CreateThreadHandle( 0 ) <=0 ) //创建线程失败
	{
		printf("Create thread %s Fail",m_threadName);
		return;
	}
	printf("Start thread id=%d\n",(int)m_hThread);
	
	//如果线程还没有启动完成，需要等待一段时间
	if(FALSE== InterlockedCompareExchange(&m_boWorking, FALSE, FALSE) )  //如果引擎已经启动得话
	{
			
	#ifdef WIN32
		if(m_hWorkThreadEvent != INVALID_HANDLE_VALUE)
		{
			if( WAIT_TIMEOUT == WaitForSingleObject(m_hWorkThreadEvent, 300000) ) //等待工作线程启动，等待5秒，如果没启动工作线程的话
			{
				printf("Create thread time out");
			}
			ResetEvent(m_hWorkThreadEvent);
		}
	#else
		struct timespec nowTimer;
		nowTimer.tv_sec = time(NULL) +5;
		nowTimer.tv_nsec=0;
		pthread_mutex_lock(&m_Mutex);
		//dwSuspended = pthread_cond_wait(&m_Cond, &m_Mutex);
		//这里不使用长等待，使用限时的等待，避免线程启动失败，把母线程卡死的现象
		int ret= pthread_cond_timedwait(&m_Cond,&m_Mutex,&nowTimer ); //等待5秒钟看效果
		if(ret == ETIMEDOUT)
		{
			printf("Create thread time out");
		}
		pthread_mutex_unlock(&m_Mutex);
	#endif
	}	
	else
	{
	#ifdef WIN32
		if(m_hWorkThreadEvent != INVALID_HANDLE_VALUE)
		{
			if(m_hWorkThreadEvent != INVALID_HANDLE_VALUE)
			{
				ResetEvent(m_hWorkThreadEvent);
			}
		}
	#endif
	}

	

}


// 停止工作线程发送数据
void CBaseThread::StopWorkThread()
{
	if(FALSE== InterlockedCompareExchange(&m_boWorking, FALSE, FALSE) ) 
	{

	}
	else
	{
		terminate();
		waitFor(300000);
		printf("Stop thread id=%d",(int)m_hThread);
		if ( m_hThread )
		{
#ifdef WIN32
			if(m_hThread != INVALID_HANDLE_VALUE)
			{
				CloseHandle( m_hThread );
			}
#else
			//pthread_mutex_destroy(&m_Mutex);
			//pthread_cond_destroy(&m_Cond);
#endif
			m_hThread = 0;
			m_boWorking =FALSE;

		}	
	}
	
	
}



