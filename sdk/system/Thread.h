#ifndef _WYL_THREAD_H_
#define _WYL_THREAD_H_

/******************************************************************
 *
 *							wyLib库	2008 - 2010
 *
 *	$ 线程基础类 $
 *  
 *  - 主要功能 - 
 *
 *	实现线程的基本封装操作。支持暂停、恢复线程，等待线程以及设
 *	置线程优先级等基本操作。
 *  2013-10-22 修改：
	1、母线程启动子线程的时候，自身休眠，需要等子线程启动完毕以后，唤醒母线程；
	2、在关闭线程的时候，子线程完全退出以后，唤醒母线程，将外边的线程调用全部整合到一起；
 *****************************************************************/

#include "_osdef.h"
namespace wylib
{
	namespace thread
	{

		#ifdef WIN32
		#define THREAD_HANDLE HANDLE
		#else
			#define THREAD_HANDLE pthread_t
			typedef void*( *thread_proc)(void*);
		#endif		

		/*	线程基础类

		*/

		class CBaseThread
		{

			 //线程的回调函数,在linux和windows下的实现
			typedef void (*THREAD_CALLBACK)( void*) ;
			
		private:
			THREAD_HANDLE m_hThread;			//自身的线程句柄
			
			volatile long m_boWorking;          //是否已经启动了，如果没有启动，需要
			
			volatile long m_boHasStoped;          //是否已经停止了

#ifndef WIN32
			pthread_cond_t m_Cond;
			pthread_mutex_t m_Mutex;
			volatile bool m_boSuspend;
#else	
			volatile HANDLE  m_hWorkThreadEvent  ;  //用于线程同步的事件
#endif


			DWORD	m_dwThreadId;		//线程ID
			char    m_threadName[64];   //线程的名字
			THREAD_CALLBACK m_callFun;  //调用函数
			void *  m_pCallBackData;    //回调参数
			
		private:

			THREAD_HANDLE CreateThreadHandle(const DWORD dwFlags);	


			//启动线程的时候调用
			//static void OnStartThread(void *lpThread);

#ifdef WIN32
			static unsigned int CALLBACK ThreadRoutine(void *lpThread);
#else
			static void* ThreadRoutine(void * lpThread);
#endif

		protected:
			//terminated函数用于判断现成是否被设置了终止标记
			inline bool terminated(){ return m_boWorking ? false:true; }
			
			
			//线程启动了
			virtual void OnThreadStarted(){}


			//实时调用，每次调用
			virtual void OnRountine() ;

			//线程被终止后的通知函数，nSuspendCount参数表示线程被终止的次数
			virtual void OnSuspend(int nSuspendCount);

			//线程被恢复后的通知函数，nSuspendCount参数表示线程还需恢复多少次后才能恢复执行,为0表示线程已恢复执行
			virtual void OnResume(int nSuspendCount);
			
			//线程例程终止后的通知函数
			virtual void OnTerminated();
		public:

		
			/*
			* Comments:  线程类的构造函数，支持2种方式构造线程，一个是继承本类，一种是传调用函数和参数进来，在构造函数中，不会启动线程，需要显示调用StartWorkThread
			* Parameter: char * sThreadName:线程的名字，用于调试
			* Parameter: THREAD_CALLBACK callFun:回调函数，如果不使用，就使用OnRountine调用，否则使用 callFun(pData)
			* Parameter: void * pData:如果callFun不为空的时候，是用户自己设置的调用函数，这个时候，运行附加一个参数pData
			* @Return  :
			*/
			CBaseThread(char *sThreadName ="Thread",THREAD_CALLBACK callFun=NULL,void * pData =NULL );

		
			~CBaseThread();

			//线程正式开始工作
			void StartWorkThread();

			//停止工作
			void StopWorkThread();


			//获取线程句柄
			inline THREAD_HANDLE getHandle(){ return m_hThread; }

			//获取线程ID
			inline DWORD getThreadId(){ return m_dwThreadId; }
			
			//获取线程的名字
			inline char *GetThreadName() {return m_threadName;}
			

#ifndef WIN32
			// 启动时候是否需要挂起。因为linux没有此线程api，只能手动挂起，兼容wylib的window特性
			inline bool isSuspend(){return m_boSuspend;}
#endif
			//暂停线程的执行，返回值表示线程到本次暂停操作后为止总计被暂停的次数。若返回-1则表示暂停线程失败。
			int suspend();

			//恢复线程的执行，返回值表示线程到本次恢复操作后，线程仍需恢复多少次才能恢复执行。
			//若返回-1则表示恢复线程失败，返回0表示线程已经完全恢复执行。
			int resume();
			
			//获取线程优先级,失败会返回THREAD_PRIORITY_ERROR_RETURN，否则返回值表示线程优先级
			int getPriority();
			
			//设置线程优先级，成功则返回true
			bool setPriority(int nPriority);
			
			//等待线程执行完毕，dwWaitLong参数表示等待的最大毫秒数，INFINITE表示无限等待。
			//注意，调用此函数的线程在此线程执行完毕后会一直处于阻塞状态
			//参数boWaitAlertAble表示调用线程在阻塞期间是否允许进入警告状态（仅对于windows有效)
#ifdef WIN32
			int waitFor(DWORD dwWaitLong = INFINITE, bool boWaitAlertAble = true);
#else
			int waitFor(DWORD dwWaitLong = 0, bool boWaitAlertAble = true);
#endif

			//标记线程的终止标记
			void terminate();
		};
	};
};

#endif

