#ifndef	_SHAREUTIL_H_
#define	_SHAREUTIL_H_
#include "LinuxPortable.h"

//貌似vc里没有snprintf这个函数,提供了_snprintf代替,但gcc的snprintf和_snprintf返回值是有差异的，这个要注意
//如果格式化的长度大于提供的字节长度，_snprintf返回-1，而snprintf是返回实际的长度
#ifndef _MSC_VER
#define SNPRINTF snprintf
#define STRNCASECMP strncasecmp
#else
#define SNPRINTF _snprintf	
#define STRNCASECMP _strnicmp
#endif

//超出指定长度，len,即返回值在[0-len]，需要对snprintf返回长度做判断的统一用这个函数
int SNPRINTFA(char *dst, int len, const char *format, ...);

int VSNPRINTFA(char *dst, int len, const char *format, va_list args);

size_t strlcpy(char *dst, const char *src, size_t siz);
size_t strlcat(char *dst, const char *src, size_t siz);

//等待并关闭线程
#ifdef WIN32
	#define	CloseThread(ht)		if ( ht ) { WaitForSingleObject( ht, 2000 ); CloseHandle( ht ); (ht) = NULL; }
#else
	#define	CloseThread(ht)		if ( ht ) { struct timespec time; time.tv_sec = 2, time.tv_nsec = 0;    pthread_timedjoin_np(ht, NULL, &time);  (ht) = NULL; }
#endif

#ifndef SafeDelete
#define	SafeDelete(p)		if ( p ){ delete p; p = NULL; }
#endif

//获取数组长度
#ifndef ArrayCount
#define	ArrayCount(a)		(sizeof(a)/sizeof((a)[0]))
#endif
//拷贝字符串到字符数组（支持MBCS和UCS）
#ifndef _asncpy
#define	_asncpy(dest, src)	_tcsncpy(dest, src, sizeof(dest)/sizeof(TCHAR)-1)
#endif
//拷贝字符串到字符数组并添加终止字符（支持MBCS和UCS）
#ifndef _asncpyt
#define	_asncpyt(dest, src)	{ _tcsncpy(dest, src, sizeof(dest)/sizeof(TCHAR)-1); dest[sizeof(dest)/sizeof(TCHAR)-1] = 0;}
#endif
//拷贝MBCS字符串到MBCS字符数组并添加终止字符
#ifndef _asncpytA
#define	_asncpytA(dest, src)	{ strncpy(dest, src, sizeof(dest)/sizeof(CHAR)-1); dest[sizeof(dest)/sizeof(CHAR)-1] = 0;}
#endif
//将字符串终止于指定长度
#ifndef STRNTERM
#define STRNTERM(s, l)	s[l] = 0;
#endif
//取64位整数的低32位整数
#ifndef LOLONG
#define LOLONG(S)           ((DWORD)((ULONG64)(S) & 0xffffffff))
#endif
//取64位整数的高32位整数
#ifndef HILONG
#define HILONG(S)           ((DWORD)((ULONG64)(S) >> 32))
#endif

#define _getTickCount		wylib::time::tick64::GetTickCountEx

//#define _getTickCount		Port_GetTickCount
#define _timeGetTime		wylib::time::tick64::GetTickCountEx

struct SystemTime
{
	int sec_;     /* seconds after the minute - [0,59] */
	int min_;     /* minutes after the hour - [0,59] */
	int hour_;    /* hours since midnight - [0,23] */
	int mday_;    /* day of the month - [1,31] */
	int mon_;     /* months since January - [1,12] */
	int year_;    /* years */
	int wday_;    /* days since Sunday[0--6], Sunday:0, Monday:1 ...*/
};

void GetSystemTime(SystemTime& sysTime);

/// 输出消息类型
typedef enum tagShareOutputMsgType
{
	rmNormal = 0,
	rmWaning,
	rmTip,
	rmError,
}SHAREOUTPUTMSGTYPE;

//消息输出函数类型
typedef	INT_PTR (STDCALL *SHAREOUTPUTMSGFN)	(SHAREOUTPUTMSGTYPE MsgType, LPCTSTR lpMsg, INT_PTR nMsgLen);

/*	数据缓冲区结构	*/
typedef struct tagDataBuffer
{
	char	*pBuffer;
	char	*pPointer;
	INT_PTR	nOffset;
	INT_PTR	nSize;
}DATABUFFER, *PDATABUFFER;

/*	性能统计结构	*/
typedef struct tagPerformanceInfo
{
	LONGLONG	dwTickBegin;
	LONGLONG	dwLastTick;
	LONGLONG	dwMinTick;
	LONGLONG	dwMaxTick;
public:
	//开始一次性能统计
	inline void start(){ dwTickBegin = _getTickCount(); }
	//结束本次性能统计
	inline void end()
	{
		dwLastTick = _getTickCount() - dwTickBegin;
		if ( dwLastTick < dwMinTick ) 
		{
			dwMinTick = dwLastTick;
		}
		if ( dwLastTick > dwMaxTick )
		{
			dwMaxTick = dwLastTick;
		}
	}
}PERFORMANCEINFO, *PPERFORMANCEINFO;

/*	循环统计结构	*/
typedef struct tagLoopPerformance
{
	INT_PTR	nLastLoop;
	INT_PTR nMinLoop;
	INT_PTR nMaxLoop;
}LOOPPERFORMANCE, *PLOOPPERFORMANCE;


//全局消息输出函数
extern INT_PTR	 STDCALL OutputMsg(SHAREOUTPUTMSGTYPE MsgType, LPCTSTR sFormat, ...);
//全局错误输出函数
//删除的错误格式为：sprintf(sFormat, ...) + 错误码 + 错误描述
INT_PTR  STDCALL OutputError(const INT_PTR nErrorCode, LPCTSTR sFormat, ...);
//初始化全局消息输出
void STDCALL InitDefMsgOut();

//反初始化全局消息输出
void STDCALL UninitDefMsgOut();

//设置全局消息输出函数,返回当前的消息输出函数
SHAREOUTPUTMSGFN STDCALL SetOutputMsgFn(SHAREOUTPUTMSGFN lpFn);

//定义异常转出文件名称
extern const TCHAR szExceptionDumpFile[];
//static int wExceptionDumpFlag;

//默认的异常处理钩子
LONG WINAPI DefaultUnHandleExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo);

//获取系统提供的错误描述内容
LPCTSTR GetSysErrorMessage(const INT_PTR ErrorCode, OUT LPTSTR sBuffer, size_t dwBufferSize, size_t *dwBufferNeeded);

//字符串转32位整数
INT _StrToInt(LPCTSTR sText);

//设置写堆栈时候的标记，是全堆栈还是部分堆栈等
/*
typedef enum _MINIDUMP_TYPE {
MiniDumpNormal                         = 0x00000000,
MiniDumpWithDataSegs                   = 0x00000001,
MiniDumpWithFullMemory                 = 0x00000002,
MiniDumpWithHandleData                 = 0x00000004,
MiniDumpFilterMemory                   = 0x00000008,
MiniDumpScanMemory                     = 0x00000010,
MiniDumpWithUnloadedModules            = 0x00000020,
MiniDumpWithIndirectlyReferencedMemory = 0x00000040,
MiniDumpFilterModulePaths              = 0x00000080,
MiniDumpWithProcessThreadData          = 0x00000100,
MiniDumpWithPrivateReadWriteMemory     = 0x00000200,
MiniDumpWithoutOptionalData            = 0x00000400,
MiniDumpWithFullMemoryInfo             = 0x00000800,
MiniDumpWithThreadInfo                 = 0x00001000,
MiniDumpWithCodeSegs                   = 0x00002000,
MiniDumpWithoutAuxiliaryState          = 0x00004000,
MiniDumpWithFullAuxiliaryState         = 0x00008000,
MiniDumpWithPrivateWriteCopyMemory     = 0x00010000,
MiniDumpIgnoreInaccessibleMemory       = 0x00020000,
MiniDumpWithTokenInformation           = 0x00040000,
MiniDumpValidTypeFlags                 = 0x0007ffff,
} MINIDUMP_TYPE;
*/
void SetMiniDumpFlag(int nFlag);


//字符串转64位整数
INT64 StrToInt64(LPCTSTR sText);


#endif
