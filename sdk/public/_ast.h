﻿#ifndef	_MDBG_H_
#define	_MDBG_H_

/*	定义Win32应用程序的Assert宏	*/



#include "_osdef.h"
#ifdef Assert
	#undef Assert
#endif

// #ifdef WIN32
	#ifdef	_DEBUG
	
		#include <assert.h>
		#define	Assert(exp)	(exp)
		#define DbgAssert(exp)
	#else
		#define	Assert(exp)	(exp)
		#define DbgAssert(exp)	//什么都不做
	#endif
// #else	
// 	#define	Assert(exp)	(exp)
// 	#define DbgAssert(exp)	//什么都不做
// #endif	// --> #ifndef Assert


/*	定义Win32应用程序的TRACE宏	*/
#ifndef WIN32
	#define	TRACE(...) OutputMsg(rmTip, __VA_ARGS__)
	//#define	TRACE
#else

	#ifdef _DEBUG
		#include <stdarg.h>
		#include <tchar.h>
		#define	TRACE	_mdbgOutputDbgStr
		static void _mdbgOutputDbgStr(LPCTSTR szFormat, ...)
		{
			static TCHAR szStaticBuf[4096];

			va_list	args;
			va_start( args, szFormat );
			_vstprintf( szStaticBuf, szFormat, args );
			va_end (args );

			OutputDebugString( szStaticBuf );
		}
	#else
		#define	TRACE
	#endif

#endif	// --> #ifndef	TRACE


/*	定义当前文件以及行的字符串获取的宏(获取的字符串形如：abc.cpp(2000))	*/
#ifndef __FILE_LINE__


#define	_TLN(LN)		#LN
#define	__TLINE__(LN)	_TLN(LN)
#define	__FILE_LINE__	__FILE__"("__TLINE__(__LINE__)")"

#endif  // --> #ifndef	__FILE_LINE__

/*	定义在编译器输出窗口输出内容的宏，但内容中会自动增加文件以及行号并在双击输出窗口后可以直接定位到消息所在行 */
#ifndef __CPMSG__

#define	__CPMSG__(MSG)	message( __FILE_LINE__ " : " #MSG )

#endif	// --> ifndef __CPMSG__


/* 定义SafeDelete、SafeDeleteArray、SafeFree以及SafeRelease宏 */
#ifndef	SafeDelete
#define	SafeDelete(p)	if (p) { delete p; p = NULL; }
#endif

#ifndef	SafeDeleteArray
#define	SafeDeleteArray(p)	if (p) { delete [] p; p = NULL; }
#endif

#ifndef	SafeFree
#define	SafeFree(p)	if (p) { free(p); p = NULL; }
#endif

#ifndef SafeRelease
#define	SafeRelease(p)	if (p){ p->Release(); p = NULL; }
#endif

#define ArrayCount(a)	(sizeof(a) / sizeof((a)[0]))

#endif
