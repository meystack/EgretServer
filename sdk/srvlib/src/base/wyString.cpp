#include <stdio.h>
#ifdef WIN32
#include <tchar.h>
#else
#include <stdlib.h>
#include <wchar.h>
#endif
#include "_osdef.h"
#include "_ast.h"
#include "_memchk.h"
#include "WideString.hpp"
#include "AnsiString.hpp"
#include <stdarg.h>


const wchar_t* wylib::string::CWideString::EMPTY_STR = L"\0";
const char* wylib::string::CAnsiString::EMPTY_STR = "\0";

wylib::string::CAnsiString* wylib::string::CWideString::toAStr(wylib::string::CAnsiString* sOutput) const
{
	if ( !m_sStr || !m_sStr[0] )
	{
		if ( sOutput ) *sOutput = NULL;
		return sOutput;
	}

	if ( sOutput == NULL ) 
		sOutput = new CAnsiString();

	int srcLen = (int)length();
	sOutput->setLength(srcLen * 2);
	int destLen =0;

#ifdef WIN32
	destLen = WideCharToMultiByte(CP_ACP, 0, m_sStr, srcLen, (LPSTR)sOutput->rawStr(), srcLen * 2, NULL, NULL);
#else
	
#endif
	if ( 0 >= destLen )
	{
		sOutput->setLength(0);
	}
	else sOutput->rawStr()[destLen] = 0;

	return sOutput;
}


wylib::string::CWideString* wylib::string::CAnsiString::toWStr(wylib::string::CWideString* sOutput) const
{
	if ( !m_sStr || !m_sStr[0] )
	{
		if ( sOutput ) *sOutput = NULL;
		return sOutput;
	}

	if ( sOutput == NULL ) 
		sOutput = new CWideString();

	int srcLen = (int)length();
	sOutput->setLength(srcLen);
	int destLen =0;
	
#ifdef WIN32
	destLen = MultiByteToWideChar(CP_ACP, 0, m_sStr, srcLen, (LPWSTR)sOutput->rawStr(), srcLen);
#else
	
#endif
	if ( 0 >= destLen )
	{
		sOutput->setLength(0);
	}
	else sOutput->rawStr()[destLen] = 0;

	return sOutput;
}


size_t wylib::string::CWideString::format(const wchar_t* fmt, ...)
{
	va_list	args;
	size_t Result = 0;

	va_start(args, fmt);
	Result = format_args(fmt, args);
	va_end(args);

	return Result;
}

size_t wylib::string::CWideString::format_args(const wchar_t* fmt, va_list _Args)
{
	CharType buffer[4096/sizeof(CharType)];
	size_t Result = 0;

#ifdef WIN32
	Result = _vsnwprintf(buffer, ArrayCount(buffer), fmt, _Args);
#else
	Result = vswprintf(buffer, ArrayCount(buffer), fmt, _Args);
#endif

	buffer[Result] = 0;

	setData( buffer );
	return Result;
}

size_t wylib::string::CAnsiString::format(const char* fmt, ...)
{
	va_list	args;
	size_t Result = 0;

	va_start(args, fmt);
	Result = format_args(fmt, args);
	va_end(args);
	return Result;
}

size_t wylib::string::CAnsiString::format_args(const char* fmt, va_list _Args)
{
	CharType buffer[4096/sizeof(CharType)];
	size_t Result = 0;

#ifdef WIN32
	Result = _vsnprintf(buffer, ArrayCount(buffer), fmt, _Args);
#else
	Result = vsnprintf(buffer, ArrayCount(buffer), fmt, _Args);
#endif

	buffer[Result] = 0;

	setData( buffer );
	return Result;
}




