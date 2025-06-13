#pragma once

#ifndef WIN32

#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <setjmp.h>
#include <assert.h>
//#include <execinfo.h>
#include <bits/sigcontext.h>
#define EXCEPTION_CONTINUE_SEARCH			0
#define EXCEPTION_CONTINUE_EXECUTION		      (-1)
#define EXCEPTION_EXECUTE_HANDLER			1

#define MAX_FRAMES					128

#if __WORDSIZE == 32
	#define jmpstr                 "jmpl *%0\n"
#else
	#define jmpstr                 "jmpq *%0\n"
#endif

#ifdef __cplusplus
extern "C" 
{
#endif

typedef struct _except_frame {
	sigjmp_buf		jmpbuf;
} except_frame;

typedef struct _thread_except {
	
	except_frame				frames[MAX_FRAMES];
	size_t					stacktop;
	unsigned long				exceptid;	// current exception id
	void*					exceptaddr;	// exception addr

} thread_except;

extern __thread thread_except* 		gthread_except;
#define _thread_frames			gthread_except->frames
#define _thread_exceptid		gthread_except->exceptid
#define _thread_exceptaddr		gthread_except->exceptaddr
#define _thread_stacktop		gthread_except->stacktop

void seh_init();
void seh_uninit();
void seh_raise(unsigned long exceptid);
void seh_raise2(unsigned long excepttid, void* exceptaddr);
#define seh_exceptid()			_thread_exceptid
#define seh_exceptaddr()		_thread_exceptaddr

#define _seh_try
//	assert(_thread_stacktop < MAX_FRAMES - 1);						\
//	if (sigsetjmp(_thread_frames[_thread_stacktop ++].jmpbuf, 1) == 0) {

#define _seh_finally
//	}  

#define _seh_except(expr)
//	} else switch ( (expr) ) {								\
//		default:									\
//		case EXCEPTION_CONTINUE_SEARCH:							\
//		{										\
//			seh_raise2(_thread_exceptid, _thread_exceptaddr);			\
//		}										\
//		break;										\
//		case EXCEPTION_CONTINUE_EXECUTION:						\
//			__asm__(								\
//				jmpstr								\
//				:								\
//				: "r"(_thread_exceptaddr)					\
//			); 									\
//			break;									\
//		case EXCEPTION_EXECUTE_HANDLER:

#define _seh_end_except
//		break;										\
//	};

#ifdef __cplusplus
}
#endif

#endif
