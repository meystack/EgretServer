#pragma once

#include "CommonDef.h"

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
		struct tm   *curTime = localtime(&szClock);		
		_tcsftime(szDestArray, size-1, szFormat, curTime);
		szDestArray[size-1] = _T('\0');
	}

	// 根据当前位置以及移动步长、方向，计算下个位置坐标
	static void NewPosition(INT_PTR& nX,INT_PTR& nY,INT_PTR nDir,INT_PTR nSpeed)
	{
		switch(nDir)
		{
		case DIR_UP:
			nY = nY - nSpeed;
			break;
		case DIR_UP_LEFT:
			nY = nY - nSpeed;
			nX = nX - nSpeed;
			break;
		case DIR_UP_RIGHT:
			nY = nY - nSpeed;
			nX = nX + nSpeed;
			break;
		case DIR_DOWN:
			nY = nY + nSpeed;
			break;
		case DIR_DOWN_LEFT:
			nY = nY + nSpeed;
			nX = nX - nSpeed;
			break;
		case DIR_DOWN_RIGHT:
			nY = nY + nSpeed;
			nX = nX + nSpeed;
			break;
		case DIR_RIGHT:
			nX = nX + nSpeed;
			break;
		case DIR_LEFT:
			nX = nX - nSpeed;
			break;
		default:
			break;
		}
	}

	//获取方向
	static int GetDir(int nSourceX, int nSourceY, int nTargetX, int nTargetY);

};
