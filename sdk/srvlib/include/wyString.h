#ifndef _WYLIB_STRING_H_
#define	_WYLIB_STRING_H_

/******************************************************************
 *
 *							wyLib库	2008 - 2010
 *
 *	$ 字符串类 $
 *  
 *  - 主要功能 - 
 *
 *	包含UCS-2和ASCII字符串处理类
 *
 *****************************************************************/

#include "WideString.hpp"
#include "AnsiString.hpp"

#ifdef UNICODE
	typedef wylib::string::CWideString	String;
#else
	typedef wylib::string::CAnsiString String;
#endif

#endif
