#include <stdio.h>
#include <stdarg.h>

#ifdef WIN32
	#include <tchar.h>
#else
	#include <stdlib.h>
	#include <wchar.h>
#endif

#include "_osdef.h"
#include <_ast.h>
#include <_memchk.h>
#include "PrintMsg.h"

using namespace wylib::printmsg;

MSGHANDLEPROC g_lpMsgHandleProc;

MSGHANDLEPROC wylib::printmsg::SetMsgHandler(MSGHANDLEPROC lpProc)
{
	MSGHANDLEPROC lpOldProc = g_lpMsgHandleProc;
	g_lpMsgHandleProc = lpProc;
	return lpOldProc;
}

MSGHANDLEPROC wylib::printmsg::GetMsgHandler()
{
	return g_lpMsgHandleProc; 
}

int wylib::printmsg::PrintMsg(ePrintMsgType msgType, eMsgLevel msgLevel, LPCTSTR format, ...)
{
	TCHAR sBuffer[4096/sizeof(TCHAR) + 1];
	va_list	args;
	int	Result = 0;

	if (g_lpMsgHandleProc)
	{
		sBuffer[0] = 0;
		va_start(args, format);
		Result = _vsntprintf(sBuffer, ArrayCount(sBuffer), format, args);
		va_end(args);
		
		Result = g_lpMsgHandleProc( msgType, msgLevel, sBuffer, Result );
	}
	return Result;
}


