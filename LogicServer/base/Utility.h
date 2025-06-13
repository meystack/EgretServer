#pragma once
#include <stdio.h>
#include <time.h>
class CUtility
{
public:
	template<size_t size>
	static void CopyCharArrayS(TCHAR (&szDestArray)[size], const TCHAR* srcData)
	{
		size_t nMaxLen = __min(size-1, _tcslen(srcData));
		CopyMemory(szDestArray, srcData, nMaxLen * sizeof(TCHAR));
		szDestArray[nMaxLen] = _T('\0');
	}

	template<size_t size>
	static void GetDateTimeDesc(TCHAR (&szDestArray)[size], LPCTSTR szFormat = "%Y-%m-%d %H:%M:%S")
	{
		time_t      szClock;	
		time(&szClock);
		struct tm curTime;
		localtime_r(&szClock,&curTime);		
		_tcsftime(szDestArray, size-1, szFormat, &curTime);
		szDestArray[size-1] = _T('\0');
	}
#ifdef WIN32
	//转换文字编码为ASC
	static size_t convertToACP(const char* lpMsg, const size_t nMsgLen, char *pBuffer, const size_t BufferLen)
	{
		WCHAR sWCBuf[4096];
		DWORD cch = (DWORD)MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, lpMsg, (int)nMsgLen, sWCBuf, ArrayCount(sWCBuf)-1);
		if ( cch > 0 )
		{
			cch = (DWORD)WideCharToMultiByte(CP_ACP, 0, sWCBuf, cch, pBuffer, int(BufferLen-1), NULL, NULL);
			if ( cch > 0 )
			{
				pBuffer[cch] = 0;
				return cch;
			}
		}
		//本身就是ACP
		else 
		{
			cch = (DWORD)__min(BufferLen-1, nMsgLen);
			memcpy(pBuffer, lpMsg, cch);
			pBuffer[cch] = 0;
			return cch;
		}
		return 0;
	}
#endif
};