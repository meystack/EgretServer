#include <stdarg.h>
#include <stdio.h>

#ifdef WIN32
	#include <Windows.h>
	#include <dbghelp.h>
	#include <TCHAR.h>
	#include <locale.h>
#endif

#include "_osdef.h"
#include <Tick.h>
#include "Lock.h"

#include <string.h>

#include "ShareUtil.h"

//RTL_CRITICAL_SECTION	g_OutputMsgLock;
wylib::sync::lock::CCSLock g_OutputMsgLock;

SHAREOUTPUTMSGFN		g_lpOutputMsg;
static TCHAR sTempMsgBuf[8192 * 4];
#define TMPBUFSIZE	(sizeof(sTempMsgBuf)/sizeof(sTempMsgBuf[0]))


#ifdef WIN32

static int wExceptionDumpFlag  = -1 ; //这个是生成dump的标记







void SetMiniDumpFlag(int nFlag)
{
	wExceptionDumpFlag =nFlag;
}






LONG WINAPI DefaultUnHandleExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo)
{
	TCHAR sDumpFileName[256];
	_tcscpy(sDumpFileName,szExceptionDumpFile);
	INT_PTR nDotPos = _tcslen(sDumpFileName);
	for(; nDotPos>=0; nDotPos--)
	{
		if(sDumpFileName[nDotPos] =='.')
		{
			break;;
		}
	}
	if(nDotPos <0)
	{
		nDotPos =0;
	}

	SYSTEMTIME Systime;
	GetLocalTime( &Systime );
	
	_stprintf(&sDumpFileName[nDotPos], _T("-%d-%d-%d-%d-%d-%d"), Systime.wYear - 2000, Systime.wMonth, Systime.wDay, Systime.wHour, Systime.wMinute, Systime.wSecond );
	_tcscat(sDumpFileName, _T(".dmp"));
	
	HANDLE	hFile = CreateFile(	sDumpFileName, GENERIC_READ | GENERIC_WRITE,
			0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );
	MINIDUMP_TYPE wDumpFlag = MiniDumpWithFullMemory;
	if(wExceptionDumpFlag >=0)
	{
		wDumpFlag = (MINIDUMP_TYPE)wExceptionDumpFlag;
	}
	if ( hFile != INVALID_HANDLE_VALUE )
	{
        MINIDUMP_EXCEPTION_INFORMATION ExInfo;

        ExInfo.ThreadId				= GetCurrentThreadId();
        ExInfo.ExceptionPointers	= ExceptionInfo;
        ExInfo.ClientPointers		= NULL;

        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), 
			hFile, wDumpFlag, &ExInfo, NULL, NULL );
		CloseHandle(hFile);
	}

	
	ExitProcess( -1 );
	return 0;
}



LPCTSTR GetSysErrorMessage(const INT_PTR ErrorCode,OUT LPTSTR sBuffer, size_t dwBufferSize, size_t *dwBufferNeeded)
{
	DWORD dwLen;
	TCHAR Buf[2048];

	dwLen = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
		FORMAT_MESSAGE_ARGUMENT_ARRAY, NULL, (DWORD)ErrorCode, 0, Buf, sizeof(Buf), NULL );
	while ( dwLen > 0 )
	{
		if ( (BYTE)Buf[dwLen-1] <= 0x32 )
			dwLen--;
		else break;
	}
	Buf[dwLen] = 0;
	if ( sBuffer )
	{
		if ( dwBufferSize > dwLen )
		{
			memcpy( sBuffer, Buf, dwLen * sizeof(Buf[0]) );
			sBuffer[dwLen] = 0;
		}
		else
		{
			memcpy( sBuffer, Buf, dwBufferSize * sizeof(Buf[0]) );
			sBuffer[dwBufferSize-1] = 0;
		}
	}
	*dwBufferNeeded = dwLen;
	return sBuffer;
}


#endif

int SNPRINTFA(char* dst, int len, const char* format, ...)
{
	if (NULL == dst || len <= 0 || NULL == format) return -1;

	va_list va;
	va_start(va, format);

	int ret = vsnprintf(dst, len, format, va);

	if (ret >= len) ret = -1;

	va_end(va);

	dst[len - 1] = '\0';

	return ret;
}

int VSNPRINTFA(char* dst, int len, const char* format, va_list args)
{
	if (NULL == dst || len <= 0 || NULL == format) return -1;

	int ret = vsnprintf(dst, len, format, args);

	if (ret >= len) ret = -1;

	dst[len - 1] = '\0';

	return ret;
}

// 本函数代码出自 openbsd 的 strlcpy.c 文件，用于非bsd并且不提供strlcpy函数的系统
size_t
	strlcpy(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0) {
		while (--n != 0) {
			if ((*d++ = *s++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);	/* count does not include NUL */
}

// 本函数代码出自 openbsd 的 strlcat.c 文件，用于非bsd并且不提供strlcat函数的系统
size_t
	strlcat(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;
	size_t dlen;

	/* Find the end of dst and adjust bytes left but don't go past end */
	while (n-- != 0 && *d != '\0')
		d++;
	dlen = d - dst;
	n = siz - dlen;

	if (n == 0)
		return(dlen + strlen(s));
	while (*s != '\0') {
		if (n != 1) {
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';

	return(dlen + (s - src));	/* count does not include NUL */
}


void GetSystemTime(SystemTime& sysTime)
{
#ifdef _MSC_VER
	SYSTEMTIME nowSysTime; // 服务器的开启时间
	GetLocalTime(&nowSysTime);
	sysTime.sec_ = nowSysTime.wSecond;
	sysTime.min_ = nowSysTime.wMinute;
	sysTime.hour_ = nowSysTime.wHour;
	sysTime.mday_ = nowSysTime.wDay;
	sysTime.mon_ = nowSysTime.wMonth;
	sysTime.year_ = nowSysTime.wYear;
	sysTime.wday_ = nowSysTime.wDayOfWeek;
#else
	time_t lcurtime;
	time(&lcurtime);
	struct tm curTime;
	localtime_r(&lcurtime,&curTime);
	sysTime.sec_ = curTime.tm_sec;
	sysTime.min_ = curTime.tm_min;
	sysTime.hour_ = curTime.tm_hour;
	sysTime.mday_ = curTime.tm_mday;
	sysTime.mon_ = curTime.tm_mon + 1;
	sysTime.year_ = curTime.tm_year + 1900;
	sysTime.wday_ = curTime.tm_wday;
#endif
}

//默认消息输出函数
INT_PTR	STDCALL StdDefOutputMsg(SHAREOUTPUTMSGTYPE MsgType, LPCTSTR lpMsg, INT_PTR nMsgLen)
{
	SYSTEMTIME Systime;
	INT_PTR Result;

	GetLocalTime( &Systime );
	Result = _tprintf( _T("[%d-%d-%d %d:%d:%d]"), Systime.wYear - 2000, Systime.wMonth, Systime.wDay, Systime.wHour, Systime.wMinute, Systime.wSecond );
	switch ( MsgType )
	{
	case rmWaning: Result += _tprintf( _T("[WRN]:") ); break;
	case rmTip: Result += _tprintf( _T("[TIP]:") ); break;
	case rmError: Result += _tprintf( _T("[ERR]:") ); break;
	default: Result += _tprintf( _T(":") ); break;
	}

	//	Result += (int)fwrite( lpMsg, 1, nMsgLen * sizeof(*lpMsg), stdout );
	//	Result += (int)fwrite( _T("\n"), 1, 1 * sizeof(*lpMsg), stdout );
	_tprintf( _T("%s\n"), lpMsg );

	return Result;
}


//全局消息输出函数
INT_PTR	STDCALL OutputMsg(SHAREOUTPUTMSGTYPE MsgType, LPCTSTR sFormat, ...)
{
	va_list	args;
	INT_PTR	Result = 0;
	if (g_lpOutputMsg)
	{
		//EnterCriticalSection( &g_OutputMsgLock );
		g_OutputMsgLock.Lock();
		va_start(args, sFormat);
		Result = _vsntprintf(sTempMsgBuf, TMPBUFSIZE - 1, sFormat, args);
		va_end(args);

		Result = g_lpOutputMsg( MsgType, sTempMsgBuf, Result );

		//LeaveCriticalSection( &g_OutputMsgLock );
		g_OutputMsgLock.Unlock();
	}
	return Result;
}


INT _StrToInt(LPCTSTR sText)
{
	int nResult = 0;
	_stscanf(sText, _T("%d"), &nResult);
	return nResult;
}


INT64 StrToInt64(LPCTSTR sText)
{
	INT64 nResult = 0;
	_stscanf(sText, _T("%lld"), &nResult);//Unix中的libc库使用%lld
	return nResult;
}


void STDCALL UninitDefMsgOut()
{
	//DeleteCriticalSection( &g_OutputMsgLock );
}

//设置全局消息输出函数,返回当前的消息输出函数
SHAREOUTPUTMSGFN STDCALL SetOutputMsgFn(SHAREOUTPUTMSGFN lpFn)
{
	//EnterCriticalSection( &g_OutputMsgLock );
	g_OutputMsgLock.Lock();
	SHAREOUTPUTMSGFN lpCurFn = g_lpOutputMsg;
	g_lpOutputMsg = lpFn;
	//LeaveCriticalSection( &g_OutputMsgLock );
	g_OutputMsgLock.Unlock();
	return lpCurFn;
}


void STDCALL InitDefMsgOut()
{
	//InitializeCriticalSection( &g_OutputMsgLock );

	g_lpOutputMsg = StdDefOutputMsg;

	//设置CTYPE为本地代码页，防止出现无法打印UNICODE 255之后的字符的问题。
#ifdef WIN32
	char sACP[16];
	sprintf(sACP, ".%d", GetACP());
	setlocale(LC_CTYPE, sACP);
#endif

}


INT_PTR  STDCALL OutputError(const INT_PTR nErrorCode, LPCTSTR sFormat, ...)
{
	INT_PTR Result = 0;
	LPTSTR sptr;
	size_t dwLen;
	va_list	args;

	if (g_lpOutputMsg)
	{
		//EnterCriticalSection( &g_OutputMsgLock );
		g_OutputMsgLock.Lock();
		sptr = sTempMsgBuf;

		va_start(args, sFormat);
		sptr += _vsntprintf(sTempMsgBuf, TMPBUFSIZE - 1, sFormat, args);
		va_end(args);

		dwLen = TMPBUFSIZE - 1 - (sptr - sTempMsgBuf);
		sptr += _sntprintf( sptr, dwLen, _T(" %d "), (int)nErrorCode );

		Result = TMPBUFSIZE - 1 - (sptr - sTempMsgBuf);

#ifdef WIN32
		GetSysErrorMessage( nErrorCode, sptr, Result, &dwLen );
#else
		dwLen =0;
#endif
		sptr += __min((INT_PTR)dwLen, Result);

		Result = DWORD(sptr - sTempMsgBuf);
		Result = g_lpOutputMsg( rmError, sTempMsgBuf, Result );

		//LeaveCriticalSection( &g_OutputMsgLock );
		g_OutputMsgLock.Unlock();
	}
	return Result;
}
