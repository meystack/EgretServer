
#ifdef WIN32
	#include <Windows.h>
	#include <tchar.h>
#endif

#include <stdio.h>
#include <_ast.h>
#include <_memchk.h>
#include <Tick.h>
#include "ShareUtil.h"
#include "DefExceptHander.h"

inline LPCTSTR GetAccessViolationOperationName(ULONG_PTR info)
{
	if ( info == 0 )
		return _T("read");
	else if ( info == 1 )
		return _T("write");
	else if ( info == 8 )
		return _T("execute");
	else return _T("unknown operation");
}

#ifdef WIN32
DWORD DefaultExceptHandler(const LPEXCEPTION_POINTERS lpPointers)
{
#define eprt(fmt, ...) sptr += _sntprintf(sptr, sErrBuf + ArrayCount(sErrBuf) - sptr - 1, fmt, __VA_ARGS__)
	
	TCHAR sErrBuf[1024], *sptr = sErrBuf;
	
	switch(lpPointers->ExceptionRecord->ExceptionCode)
	{
	case EXCEPTION_ACCESS_VIOLATION:
		eprt(_T("Access Violation at Address %0.16LX %s of Address %0.16LX"),
			lpPointers->ExceptionRecord->ExceptionAddress,
			GetAccessViolationOperationName(lpPointers->ExceptionRecord->ExceptionInformation[0]),
			lpPointers->ExceptionRecord->ExceptionInformation[1]);
		break;
	case EXCEPTION_DATATYPE_MISALIGNMENT:
		eprt(_T("Misaligment Address Access at Address %0.16LX Access of Address %0.16LX"),
			lpPointers->ExceptionRecord->ExceptionAddress,
			lpPointers->ExceptionRecord->ExceptionInformation[1]);
		break;
	case EXCEPTION_BREAKPOINT:
		eprt(_T("A BreakPoint was encountered at Address %0.16LX"),
			lpPointers->ExceptionRecord->ExceptionAddress);
		break;
	case EXCEPTION_SINGLE_STEP:
		eprt(_T("A Single-Step instruction has been executed at Address %0.16LX"),
			lpPointers->ExceptionRecord->ExceptionAddress);
		break;
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		eprt(_T("Array Access Bounds-Out at Address %0.16LX"),
			lpPointers->ExceptionRecord->ExceptionAddress);
		break;
	case EXCEPTION_FLT_DENORMAL_OPERAND:
		eprt(_T("Denormaled floating-point operand at Address %0.16LX"),
			lpPointers->ExceptionRecord->ExceptionAddress);
		break;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		eprt(_T("Division by Zero at Address %0.16LX"),
			lpPointers->ExceptionRecord->ExceptionAddress);
		break;
	case EXCEPTION_FLT_INEXACT_RESULT:
		eprt(_T("Cannot represent a decimal fraction of a floating-point operation at Address %0.16LX"),
			lpPointers->ExceptionRecord->ExceptionAddress);
		break;
	case EXCEPTION_FLT_INVALID_OPERATION:
		eprt(_T("Invalid floating-point operation at Address %0.16LX"),
			lpPointers->ExceptionRecord->ExceptionAddress);
		break;
	case EXCEPTION_FLT_OVERFLOW:
		eprt(_T("Floating-point operation Overflow at Address %0.16LX"),
			lpPointers->ExceptionRecord->ExceptionAddress);
		break;
	case EXCEPTION_FLT_STACK_CHECK:
		eprt(_T("Floating-point stack Overflow or Underflow at Address %0.16LX"),
			lpPointers->ExceptionRecord->ExceptionAddress);
		break;
	case EXCEPTION_FLT_UNDERFLOW:
		eprt(_T("Floating-point operation Underflow at Address %0.16LX"),
			lpPointers->ExceptionRecord->ExceptionAddress);
		break;
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		eprt(_T("Integer Division By Zero at Address %0.16LX"),
			lpPointers->ExceptionRecord->ExceptionAddress);
		break;
	case EXCEPTION_INT_OVERFLOW:
		eprt(_T("Integer operation Overflow at Address %0.16LX"),
			lpPointers->ExceptionRecord->ExceptionAddress);
		break;
	case EXCEPTION_PRIV_INSTRUCTION:
		eprt(_T("Privileged Instuction at Address %0.16LX"),
			lpPointers->ExceptionRecord->ExceptionAddress);
		break;
	case EXCEPTION_IN_PAGE_ERROR:
		eprt(_T("Page Error at Address %0.16LX %s of Address %0.16LX"),
			lpPointers->ExceptionRecord->ExceptionAddress,
			GetAccessViolationOperationName(lpPointers->ExceptionRecord->ExceptionInformation[0]),
			lpPointers->ExceptionRecord->ExceptionInformation[1]);
		break;
	case EXCEPTION_ILLEGAL_INSTRUCTION:
		eprt(_T("Illegal Instuction at Address %0.16LX"),
			lpPointers->ExceptionRecord->ExceptionAddress);
		break;
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		eprt(_T("None-Continuable Exception at Address %0.16LX, Going to Abort"),
			lpPointers->ExceptionRecord->ExceptionAddress);
		break;
	case EXCEPTION_STACK_OVERFLOW:
		eprt(_T("Stack Overflow at Address %0.16LX"),
			lpPointers->ExceptionRecord->ExceptionAddress);
		break;
	case EXCEPTION_INVALID_DISPOSITION:
		eprt(_T("Invalid Disposition at Address %0.16LX"),
			lpPointers->ExceptionRecord->ExceptionAddress);
		break;
	case EXCEPTION_INVALID_HANDLE:
		eprt(_T("Invalid Handle at Address %0.16LX"),
			lpPointers->ExceptionRecord->ExceptionAddress);
		break;
	case 0xe06d7363://C++ exception
		eprt(_T("Unhandled C++ Exception at Address %0.16LX"), lpPointers->ExceptionRecord->ExceptionAddress);
		//ExceptionRecord->ExceptionInformation[1] = throw 的对象实例
		//ExceptionRecord->ExceptionInformation[2] = throw 的对象类型信息
		break;
	default:
		eprt(_T("Unexpected Exception Code %0.16LX at Address %0.16LX\r\n [0]=%0.16LX [1]=%0.16LX [2]=%0.16LX [3]=%0.16LX"),
			lpPointers->ExceptionRecord->ExceptionAddress,
			lpPointers->ExceptionRecord->ExceptionInformation[0],
			lpPointers->ExceptionRecord->ExceptionInformation[1],
			lpPointers->ExceptionRecord->ExceptionInformation[2],
			lpPointers->ExceptionRecord->ExceptionInformation[3]);
		break;
	}

#undef eprt
	TRACE(_T("%s\r\n"), sErrBuf);
	OutputMsg(rmError, sErrBuf);
	return EXCEPTION_EXECUTE_HANDLER;//继续从异常处理函数中恢复执行
}
#endif
