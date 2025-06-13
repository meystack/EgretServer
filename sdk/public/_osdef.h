#ifndef _WYL_OS___DEFINITION_H_
#define _WYL_OS___DEFINITION_H_

/* 去掉MFC的定义
#ifdef	_WIN32_WINNT
	#if (_WIN32_WINNT < 0x0400)
		#undef	_WIN32_WINNT
		#define	_WIN32_WINNT 0x0400
	#endif //
#else
	#define	_WIN32_WINNT 0x0400
#endif
*/

#ifdef WIN32
	#ifndef CONFIG_USE_WIN_SOCKET
		#define CONFIG_USE_WIN_SOCKET
	#endif

	typedef __int64            int64;
	typedef __int32            int32;
	typedef __int16            int16;
	typedef __int8             int8;
	typedef unsigned __int64   uint64;
	typedef unsigned __int32   uint32;
	typedef unsigned __int16   uint16;
	typedef unsigned __int8    uint8;
	#include <windows.h>

	#ifndef	STDCALL
		#define	STDCALL __stdcall
	#endif
	
	
	#define PTHREADRET void
	
	#define I64FORMAT "%I64d"
	#define U64FORMAT "%I64u"
#else
	#ifndef	_MTICK64
		#define	_MTICK64
	#endif
	
	#ifndef	TOLUA_RELEASE
		#define	TOLUA_RELEASE
	#endif
	
	#ifndef	_ENABLE_OBJ_COUNT_STATICS_
		#define	_ENABLE_OBJ_COUNT_STATICS_
	#endif
	
	#include <sys/syscall.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <sys/socket.h>
	#include <dirent.h>
	#include <unistd.h>
	#include <sys/time.h>
	#include <sys/resource.h>
	#include <pthread.h>
	#include <semaphore.h>
	#include <string.h>
	#include <stdio.h>
	#include <sched.h>
	#include <termios.h>
	#include <fcntl.h>
	#include <errno.h>
	#include <assert.h>
	#include <stddef.h>
	#include <netinet/tcp.h>
	#include <stdlib.h>

	//#include <system.h>

	#ifndef CONFIG_USE_LINUX_SOCKET
		#define CONFIG_USE_LINUX_SOCKET
	#endif

	#ifndef __max
		#define __max(x,y) ((x) < (y) ? (y):(x))
	#endif

	#ifndef __min
		#define __min(x,y) ((x) < (y) ? (x):(y))
	#endif

	#ifndef _tcslen
		#define _tcslen strlen
	#endif
	
	
	#ifndef _stprintf
		#define _stprintf sprintf
	#endif

	#ifndef sprintf_s
		#define sprintf_s snprintf
	#endif
	
	#ifndef _tcscpy
		#define _tcscpy strcpy
	#endif
	
	#define OutputDebugString(s) fprintf( stderr, "%s", s)
	
	#define SetConsoleTitle(input) printf( "%c]0;%s%c", '\033', input, '\007');


	#ifndef _stprintf_s
		#define _stprintf_s sprintf
	#endif

	
	
	#ifndef _tcscpy_s
		#define _tcscpy_s strcpy
	#endif
	
	#include <stdint.h>
		#ifndef uint64_t
		#ifdef __linux__
			#include <linux/types.h>
		#endif
	#endif
	
	#define CoInitialize(x);
	
	#define CoUninitialize(x); 

	//#define UninitDefMsgOut(x); 
	#define _setmbcp(x); 
	

	typedef int64_t            int64;
	typedef int32_t            int32;
	typedef int16_t            int16;
	typedef int8_t             int8;
	typedef uint64_t           uint64;
	typedef uint32_t           uint32;
	typedef uint16_t           uint16;
	typedef uint8_t            uint8;

	//typedef int64 __int64            ;
	typedef int32 __int32            ;
	typedef int16 __int16            ;
	typedef int8  __int8             ;

	typedef void *HANDLE;
	typedef HANDLE *PHANDLE;

	//#undef FAR
	#undef  NEAR
	//#define FAR                 far
	#define NEAR                near
	#ifndef CONST
	#define CONST               const
	#endif
	#define VOID				void

	#ifndef IN
	#define IN
	#endif

	#ifndef OUT
	#define OUT
	#endif

	#ifndef TRUE
		#define TRUE 1
	#endif

	#ifndef NULL
		#define NULL 0
	#endif

	#ifndef FALSE
		#define FALSE 0
	#endif

	#define INFINITE 0xFFFFFFFF
 
	// cross platform for wylib
	typedef unsigned int       DWORD;
	typedef int                 BOOL;
	typedef unsigned char       BYTE;
	typedef unsigned char       byte;
	typedef unsigned short      WORD;
	typedef float               FLOAT;
	typedef FLOAT               *PFLOAT;
	typedef BOOL *PBOOL;
	typedef BOOL *LPBOOL;
	typedef BYTE *PBYTE;
	typedef BYTE *LPBYTE;
	typedef int *PINT;
	typedef int *LPINT;
	typedef WORD *PWORD;
	typedef WORD *LPWORD;
	typedef long *LPLONG;
	typedef DWORD *PDWORD;
	typedef DWORD *LPDWORD;
	typedef DWORD *DWORD_PTR;
	typedef DWORD COLORREF;
	typedef void *LPVOID;
	typedef CONST BYTE *LPCBYTE;
	typedef CONST void *LPCVOID;
	typedef void *PVOID;

	typedef char                CHAR;
	typedef signed char         INT8;
	typedef unsigned char       UCHAR;
	typedef unsigned char       UINT8;
	typedef unsigned char       BYTE;
	typedef short               SHORT;
	typedef signed short        INT16;
	typedef unsigned short      USHORT;
	typedef unsigned short      UINT16;
	typedef unsigned short      WORD;
	typedef int                 INT;
	typedef signed int          INT32;
	typedef unsigned int        UINT;
	typedef unsigned int        UINT32;
	typedef long long               LONG;
	typedef unsigned long long       ULONG;
	

	typedef int64_t             LONGLONG;
	typedef int64_t             LONG64;
	typedef int64_t	            INT64;
	typedef uint64_t			ULONGLONG;
	typedef uint64_t			DWORDLONG;
	typedef uint64_t			ULONG64;
	typedef uint64_t			DWORD64;
	typedef uint64_t			UINT64;

	
	typedef int64_t             LONG_PTR;
	typedef uint64_t			ULONG_PTR;
	typedef unsigned int        *PUINT;

	typedef double   DOUBLE; 
	
	typedef long long *   PINT64; 
	typedef CHAR * LPSTR    ;
	typedef long long  __int64 ;
	
	typedef ULONGLONG *    PULONGLONG; 

	typedef struct _POINTL
	{
		DWORD x;
		DWORD y;
	} POINT, *PPOINT, *NPPOINT, *LPPOINT;

	//#define _ENABLE_OBJ_COUNT_STATICS_ 1

	#define GetLastError() errno
	#define WSAGetLastError() errno

	typedef uint64_t UINT_PTR, *PUINT_PTR;
	typedef int64_t             INT_PTR, *PINT_PTR;
	
	
	#ifdef __x86_64__
		//typedef int64_t INT_PTR, *PINT_PTR;
		
		//#define offsetof(s,m)   (size_t)( (ptrdiff_t)&reinterpret_cast<const volatile char&>((((s *)0)->m)) )
	#elif  __i386__
		//#define offsetof(s,m)   (size_t)&reinterpret_cast<const volatile char&>((((s *)0)->m))
		//typedef int INT_PTR, *PINT_PTR;
		//typedef unsigned int UINT_PTR, *PUINT_PTR;	 
	#endif


	#ifdef  UNICODE                  
		typedef  wchar_t TCHAR       ;
		#define _vsntprintf  vswprintf  
		//#define  wstrchr         _tcschr;
	#else   
		typedef char* LPTSTR;
		typedef CONST CHAR *LPCSTR, *PCSTR;
		typedef LPCSTR LPCTSTR;
	
		typedef  char TCHAR        ;
		#define  _vsntprintf  vsnprintf 
		#define  _tcscmp  strcmp 
		#define  _tcschr  strchr
		#define  _tcsncmp  strncmp
		#define  _tcscat  strcat
		#define  _tcscpy  strcpy
		#define  _stscanf sscanf
		#define  _tcsncpy strncpy
		#define  _tcsnicmp strncasecmp
		#define  _tcsncicmp strncasecmp
		#define  _tcsicmp strcasecmp
		#define  _stricmp strcasecmp
		#define  _tcsstr  strstr 
		//#define  strchr         _tcschr;
	#endif
	
	#define ExitThread pthread_exit 
	#define __forceinline inline

	#include<time.h>
	#ifdef __x86_64__
		#define __time64_t time_t
	#else
		#define __time32_t time_t
	#endif

	#ifdef __x86_64__
                #define _mktime64 mktime
        #else
                #define _mktime32 mktime
        #endif

	#ifdef __x86_64__
                #define _localtime64 localtime_r
        #else
                #define _localtime32 localtime_r
        #endif

	#define LOBYTE(w) ((BYTE)(((DWORD)(w)) & 0xff))
	#define HIBYTE(w) ((BYTE)((((DWORD)(w)) >> 8) & 0xff))
	#define HIWORD(l) ((WORD)((((DWORD)(l)) >> 16) & 0xffff))
    #define LOWORD(l) ((WORD)(((DWORD)(l)) & 0xffff))
    #define MAKELONG(a, b) ((LONG)(((WORD)(((DWORD)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD)(b)) & 0xffff))) << 16))
	#define MAKEWORD(a, b) ((WORD)(((BYTE)(((DWORD)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD)(b)) & 0xff))) << 8))

	#define ULONG64    UINT64
	#define LONG64     INT64

	#define MAXULONG64 ((ULONG64)~((ULONG64)0))
	#define MAXLONG64  ((LONG64)(MAXULONG64 >> 1))
	#define MINLONG64  ((LONG64)~MAXLONG64)

	#define MAXUINT    ((UINT)~((UINT)0))
	#define MAXINT     ((INT)(MAXUINT >> 1))
	#define MININT     ((INT)~MAXINT)

	#define MAXBYTE    0xff

	#ifndef	STDCALL
		#define	STDCALL 
	#endif

	#ifndef	WINAPI
		#define	WINAPI 
	#endif

	#define _T(x) x
	
	#define ZeroMemory(Destination,Length) memset((Destination),0,(Length))
	
	#define _tcsftime   strftime
	#define TLS_OUT_OF_INDEXES ((DWORD)0xFFFFFFFF)

	#ifndef	CHAR_BIT
		#define	CHAR_BIT 8
	#endif

	#define _tprintf  printf  
	#define _sntprintf  snprintf  
	#define _snprintf   snprintf

	#ifndef MAX_PATH
		#define MAX_PATH 260
	#endif

	#define	InterlockedIncrement(x) __sync_add_and_fetch((x),1)
	#define	InterlockedDecrement(x) __sync_sub_and_fetch((x),1)
	
	#define	InterlockedCompareExchange(x, y,z)  __sync_val_compare_and_swap((x),(z),(y))
	#define InterlockedExchange(x, y)  __sync_lock_test_and_set((x), (y))

	#define InterlockedExchangeAdd(x,y)  __sync_add_and_fetch((x),(y))
	
	typedef unsigned int (*PTHREAD_START_ROUTINE )(void * lpThreadParameter );
	typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;
	

	#define __try try
	#define __catch catch
	#define __finally finally
	 
	#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
                ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))

	#define PTHREADRET void*
	
		//Some Api From
	
	
	#define INVALID_HANDLE_VALUE ((long)-1)
	
	#define CopyMemory(Destination,Source,Length) memcpy((Destination),(Source),(Length))
	#define	IsDebuggerPresent(x) 0 
	#define	DebugBreak(x)  
	#define GetCurrentThreadId() syscall(SYS_gettid)
	#define _atoi64(v) strtoll(v, NULL, 10)
	#define _getts gets

	typedef CHAR* PCHAR;

	#ifndef INT_MAX
		#define INT_MAX 2147483647 /* maximum (signed) int value */
	#endif
	
	#define SetCurrentDirectory chdir
	#define DeleteFileA remove

	#define DUMMYSTRUCTNAME
	#if defined(MIDL_PASS)
	typedef struct _LARGE_INTEGER
	{
	#else // MIDL_PASS
	typedef union _LARGE_INTEGER
	{
		struct
		{
			DWORD LowPart;
			DWORD HighPart;
		} DUMMYSTRUCTNAME;
		struct
		{
			DWORD LowPart;
			DWORD HighPart;
		} u;
	#endif // MIDL_PASS
		LONGLONG QuadPart;
	} LARGE_INTEGER;

	#define  __declspec(dllexport) 

	#define TOLUA_RELEASE

	#define RTL_CRITICAL_SECTION   pthread_mutex_t
	#define CRITICAL_SECTION   pthread_mutex_t
	#define LPCRITICAL_SECTION pthread_mutex_t *
	#define EnterCriticalSection   pthread_mutex_lock
	#define LeaveCriticalSection    pthread_mutex_unlock
	#define TryEnterCriticalSection(x) (!pthread_mutex_trylock(x))
	#define InitializeCriticalSection(x) pthread_mutex_init((x), (NULL))
	#define DeleteCriticalSection pthread_mutex_destroy

	#define CALLBACK  __attribute__((stdcall)) 
	#define EXPORTCALL  __attribute__((stdcall))
	#define __cdecl   __attribute__((cdecl))
	#define __stdcall __attribute__((stdcall))

	#define closesocket(x) close((x))

	typedef struct _COORD {
    		SHORT X;
    		SHORT Y;
	} COORD, *PCOORD;

	#define ioctlsocket ioctl
	#define ERROR_SUCCESS 0
	typedef struct sockaddr SOCKADDR;

	#ifndef SOCKET_ERROR
		#define SOCKET_ERROR -1
	#endif

	#define GetCurrentProcessId() getpid()
	
	#define I64FORMAT "%lld"
	#define U64FORMAT "%llu"
#endif

#include "../../system/SysApi.h"  
#include "../../system/LinuxTimer.h"  
#endif   // _WYL_OS_DEFINITION_H_
