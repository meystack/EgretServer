#include "StdAfx.h"
#include "StackWalker.h"

#ifdef WIN32
DWORD CustomExceptHandler(const LPEXCEPTION_POINTERS lpPointers)
{
	SHOW_CALLSTACK();
	return DefaultExceptHandler(lpPointers);
}
#endif
