#include <stdlib.h>

#include <locale.h>

#ifdef WIN32
#include <tchar.h>
#include <Windows.h>
#endif

#ifdef WIN32

#include <Tick.h>
#include "ShareUtil.h"

/**
 * 跳转指令结构
 *************************/
#pragma pack(push, 1)
struct JMPInstruct32
{
	unsigned char code;
	unsigned int offset;
};

struct JMPInstruct64
{
	unsigned short code; 
	unsigned int loAddrCST;
	unsigned __int64 offset;
};

union JMPInstruct
{
	JMPInstruct32 c32;
	JMPInstruct64 c64;
};
#pragma pack(pop)


union JMPInstruct CodeSource;
static char sCP[128];

static HANDLE WINAPI _CreateThread_Path_(
	__in_opt  LPSECURITY_ATTRIBUTES lpThreadAttributes,
	__in      SIZE_T dwStackSize,
	__in      LPTHREAD_START_ROUTINE lpStartAddress,
	__in_opt __deref __drv_aliasesMem LPVOID lpParameter,
	__in      DWORD dwCreationFlags,
	__out_opt LPDWORD lpThreadId
	);


//计算跳转偏移地址
#define CalcJmpOffset32(s, d)	((SIZE_T)(d) - ((SIZE_T)(s) + 5))
#define CalcJmpOffset64(s, d)	((SIZE_T)(d))

static int PathCreateThreadCode(const JMPInstruct* pCode)
{
	JMPInstruct* fn = (JMPInstruct*)&CreateThread;
	DWORD dwOldProtect;
	MEMORY_BASIC_INFORMATION mbi;

	if ( VirtualQuery(fn, &mbi, sizeof(mbi)) != sizeof(mbi) )
		return GetLastError();

	//修改_output_l函数的内存保护模式，增加可读写保护
	if ( !VirtualProtect(fn, sizeof(*fn), PAGE_EXECUTE_READWRITE, &dwOldProtect) )
		return GetLastError();

	//改写跳转代码
	if (sizeof(void*) == 8)
		fn->c64 = pCode->c64;
	else if (sizeof(void*) == 4)
		fn->c32 = pCode->c32;

	//还原_output_l函数的内存保护模式
	if ( !VirtualProtect(fn, sizeof(*fn), dwOldProtect, &dwOldProtect) )
		return GetLastError();
	
	return 0;
}

//设置代码跳转补丁
static int PathCreateThread()
{
	JMPInstruct pc;
	if (sizeof(void*) == 8)
	{
		//JMP [OFFSET]
		pc.c64.code = 0x25FF;
		pc.c64.loAddrCST = 0;//固定为0
		pc.c64.offset = CalcJmpOffset64(&CreateThread, &_CreateThread_Path_);
	}
	else if (sizeof(void*) == 4)
	{
		pc.c32.code = 0xE9;
		pc.c32.offset = (UINT)CalcJmpOffset32(&CreateThread, &_CreateThread_Path_);
	}
	return PathCreateThreadCode(&pc);
}

//解除代码跳转补丁
static int UnPathCreateThread()
{
	return PathCreateThreadCode(&CodeSource);
}

int InstallThreadLocalePath(const char *lc)
{
	//保存原始代码字节
	if (!CodeSource.c32.code) CodeSource = *((JMPInstruct*)&CreateThread);
	//保存设定的数据
	_asncpytA(sCP, lc);
	//设置跳转
	return PathCreateThread();
}

static void SetupThreadLocale()
{
	//参见：http://msdn.microsoft.com/zh-cn/library/ms235302(VS.80).aspx
	//★★★setlocal是针对线程的！
	//设置locale为C，设置mbcp为_MB_CP_SBCS，以便支持libc中文字处理相关的函数支持UTF-8
	setlocale(LC_ALL, sCP);
}


struct PTD
{
	LPTHREAD_START_ROUTINE routine;
	LPVOID param;
};

static INT_PTR WINAPI _Path_ThreadLocale_Rountine(PTD *pd)
{
	SetupThreadLocale();
	PTD td = *pd;
	
	INT_PTR result = td.routine(td.param);
	//free(pd);
	return result;
}

static HANDLE WINAPI _CreateThread_Path_(
	__in_opt  LPSECURITY_ATTRIBUTES lpThreadAttributes,
	__in      SIZE_T dwStackSize,
	__in      LPTHREAD_START_ROUTINE lpStartAddress,
	__in_opt __deref __drv_aliasesMem LPVOID lpParameter,
	__in      DWORD dwCreationFlags,
	__out_opt LPDWORD lpThreadId
	)
{
	UnPathCreateThread();

	PTD *p = (PTD*)malloc(sizeof(*p));
	p->routine = lpStartAddress;
	p->param = lpParameter;

	HANDLE result = CreateThread(lpThreadAttributes, dwStackSize, 
		(LPTHREAD_START_ROUTINE)_Path_ThreadLocale_Rountine, 
		p, dwCreationFlags, lpThreadId);

	PathCreateThread();
	return result;
}
#endif
