
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
	#include <Windows.h>
	#include <tchar.h>
#else
	#include "unistd.h"
	#include <sys/stat.h> 
#endif



#include <_memchk.h>
#include <_ast.h>
#include "../include/FDOP.h" 

using namespace FDOP;

//isFile or Dir
bool FDOP::FileExists(LPCTSTR sFilePath)
{

#ifdef WIN32
	DWORD dwAttr = GetFileAttributes(sFilePath);
	if ( dwAttr == (DWORD)-1 )
		return false;
	else return true;

#else
	struct stat info;
	int result= stat (sFilePath,&info);
        if(  result !=0) return false;
	
	if( ((info.st_mode & S_IFMT) == S_IFREG  ) || ( info.st_mode & S_IFDIR) )
        {
       		return true;
        }
	else
	{
		return false;	
	}
#endif

}

bool FDOP::IsCommonFile(LPCTSTR sDirPath)
{

#ifdef WIN32
	DWORD dwAttr = GetFileAttributes(sDirPath);
	if ( dwAttr == INVALID_FILE_ATTRIBUTES )
		return false;
	if ( dwAttr & FILE_ATTRIBUTE_DIRECTORY )
		return false;
	return true;
#else
	struct stat info;
	int result= stat (sDirPath,&info);
        if(  result !=0) return false;
	if( (info.st_mode & S_IFMT) == S_IFREG)
        {
       		return true;
        }
	else
	{
		return false;	
	}
#endif
}


bool FDOP::IsArchive(LPCTSTR sFilePath)
{
#ifdef WIN32
	DWORD dwAttr = GetFileAttributes(sFilePath);
	if ( dwAttr == (DWORD)-1 || (dwAttr & FILE_ATTRIBUTE_ARCHIVE) == 0 )
		return false;
	else return true;
#else
	return false;  //there is no achieve in linux
#endif
}

bool FDOP::IsDirectory(LPCTSTR sDirPath)
{
#ifdef WIN32
	DWORD dwAttr = GetFileAttributes(sDirPath);
	if ( dwAttr == (DWORD)-1 || (dwAttr & FILE_ATTRIBUTE_DIRECTORY) == 0 )
		return false;
	else return true;
#else
	struct stat info;
	int result= stat (sDirPath,&info);
        if(  result !=0) return false;
	
	return (   info.st_mode & S_IFDIR ) ?true:false;
        
#endif


}

SIZE_T FDOP::ExtractFileName(LPCTSTR sFilePath, LPTSTR sNameBuf, SIZE_T dwBufLen)
{
	LPCTSTR sNameStart, sNameEnd = sFilePath + _tcslen(sFilePath) - 1;
	//跳过目录名称后连续的'/'或'\'
	while ( sNameEnd >= sFilePath && (*sNameEnd == '/' || *sNameEnd == '\\') )
	{
		sNameEnd--;
	}
	sNameStart = sNameEnd;
	sNameEnd++;
	//定位目录名称起始的位置
	while ( sNameStart >= sFilePath )
	{
		if ( *sNameStart == '/' || *sNameStart == '\\' )
			break;
		sNameStart--;
	}
	sNameStart++;
	//拷贝目录名称
	if ( sNameStart < sNameEnd )
	{
		SIZE_T dwNameLen = sNameEnd - sNameStart;
		if ( dwBufLen > 0 )
		{
			if ( dwBufLen > dwNameLen )
				dwBufLen = dwNameLen;
			else dwBufLen--;
			memcpy(sNameBuf, sNameStart, sizeof(*sNameStart) * dwBufLen);
			sNameBuf[dwBufLen] = 0;
		}
		return dwNameLen;
	}
	return 0;
}

SIZE_T FDOP::ExtractFileNameOnly(LPCTSTR sFileName, LPTSTR sNameBuf, SIZE_T dwBufLen)
{
	//文件名为空则直接返回0
	if ( !*sFileName )
	{
		if ( dwBufLen > 0 )
			sNameBuf[0] = 0;
		return 0;
	}
	LPCTSTR sNameStart;
	LPCTSTR sNameEnd = sFileName + _tcslen(sFileName) - 1;
	//如果文件是目录
	if ( *sNameEnd == '/' || *sNameEnd == '\\' )
	{
		//跳过目录名称后连续的'/'或'\'
		while ( sNameEnd >= sFileName && (*sNameEnd == '/' || *sNameEnd == '\\') )
		{
			sNameEnd--;
		}
		sNameEnd++;
	}
	else
	{
		LPCTSTR sPtr = sNameEnd;
		//找到文件后缀部分的起始位置
		while ( sPtr >= sFileName )
		{
			if (*sPtr == '.')
			{
				sNameEnd = sPtr;
				break;
			}
			if (*sPtr == '/' || *sPtr == '\\')
				break;
			sPtr--;
		}
		if( *sPtr != '.' )
            sNameEnd++ ;
	}

	sNameStart = sNameEnd - 1;
	//定位目录名称起始的位置
	while ( sNameStart >= sFileName )
	{
		if ( *sNameStart == '/' || *sNameStart == '\\' )
			break;
		sNameStart--;
	}
	sNameStart++;
	//拷贝目录名称
	if ( sNameStart < sNameEnd )
	{
		SIZE_T dwNameLen = sNameEnd - sNameStart;
		if ( dwBufLen > 0 )
		{
			if ( dwBufLen > dwNameLen )
				dwBufLen = dwNameLen;
			else dwBufLen--;
			memcpy(sNameBuf, sNameStart, sizeof(*sNameStart) * dwBufLen);
			sNameBuf[dwBufLen] = 0;
		}
		return dwNameLen;
	}
	return 0;
}

LPCTSTR FDOP::ExtractFileExt(LPCTSTR sFileName)
{
	LPCTSTR sResult = NULL;
	while (*sFileName)
	{
		if (*sFileName == '.')
		{
			sResult = sFileName;
			break ;
		}
		sFileName++;
	}
	return sResult;
}

SIZE_T FDOP::ExtractFileDirectory(LPCTSTR sFilePath, LPTSTR sDirBuf, SIZE_T dwBufLen)
{
	LPCTSTR sDirEnd = sFilePath + _tcslen(sFilePath) - 1;
	while (sDirEnd >= sFilePath && *sDirEnd != '/' && *sDirEnd != '\\')
	{
		sDirEnd--;
	}
	if ( sDirEnd > sFilePath )
	{
		SIZE_T dwNameLen = sDirEnd - sFilePath + 1;
		if ( dwBufLen > 0 )
		{
			if ( dwBufLen > dwNameLen )
				dwBufLen = dwNameLen;
			else dwBufLen--;

			memcpy(sDirBuf, sFilePath, sizeof(*sDirBuf) * dwBufLen);
			sDirBuf[dwBufLen] = 0;
		}
		return dwNameLen;
	}
	return 0;
}

SIZE_T FDOP::ExtractTopDirectoryName(LPCTSTR sDirPath, OUT LPCTSTR *ppChildDirPath, LPTSTR sDirName, SIZE_T dwBufLen)
{
	LPCTSTR sNameEnd;
	//跳过目录名称前连续的'/'或'\'
	while ( *sDirPath && (*sDirPath == '/' || *sDirPath == '\\') )
	{
		sDirPath++;
	}
	sNameEnd = sDirPath;

	//定位目录名称起始的位置
	while ( *sNameEnd )
	{
		if ( *sNameEnd == '/' || *sNameEnd == '\\' )
			break;
		sNameEnd++;
	}
	//拷贝目录名称
	if ( sNameEnd > sDirPath )
	{
		SIZE_T dwNameLen = sNameEnd - sDirPath;
		if ( dwBufLen > 0 )
		{
			if ( dwBufLen > dwNameLen )
				dwBufLen = dwNameLen;
			else dwBufLen--;

			memcpy(sDirName, sDirPath, sizeof(*sDirPath) * dwBufLen);
			sDirName[dwBufLen] = 0;

			if (ppChildDirPath)
				*ppChildDirPath = sNameEnd;
		}
		return dwNameLen;
	}
	return 0;
}

bool FDOP::DeepCreateDirectory(LPCTSTR sDirPath)
{
	TCHAR sPath[256];
	LPTSTR sPathPtr = sPath;
	SIZE_T dwNameLen, dwBufLen = ArrayCount(sPath) - 1;

	while (true)
	{
		dwNameLen = ExtractTopDirectoryName(sDirPath, &sDirPath, sPathPtr, dwBufLen);
		//如果目录名称长度超过目录缓冲区长度则放弃
		if ( dwNameLen >= dwBufLen )
			return false;
		//如果目录名称长度为0则表示所有目录均已创建完成
		if (dwNameLen == 0)
			return true;
		sPathPtr += dwNameLen;
		//如果目录名称不是驱动器名称则检查和创建目录

#ifdef WIN32
		if (sPathPtr[-1] != ':')
		{
			//如果目录不存在则创建此目录
			DWORD dwAttr  = GetFileAttributes(sPath);
			if ( (dwAttr == (DWORD)-1 && GetLastError() == ERROR_FILE_NOT_FOUND) )
			{
				if (!CreateDirectory(sPath, NULL))
					return false;
			}
			//如果文件存在且文件不是目录则返回false
			else if ( !(dwAttr & FILE_ATTRIBUTE_DIRECTORY) )
			{
				return false;
			}
		}
		sPathPtr[0] = '\\';

#else
		if (sPathPtr != sDirPath) //Not The begin
		{
			struct stat info;
			int result= stat (sPath,&info);
			if(  result !=0) 
			{
				if( mkdir(sPath,S_IWRITE | S_IREAD | S_IEXEC) !=0 ) return false;
			}
			else
			{
				if(! (info.st_mode & S_IFDIR) ) return false;
			}
	
			
		}
		sPathPtr[0] = '/';
#endif
		
		
		sPathPtr++;
		if ( dwBufLen > dwNameLen )
			dwBufLen -= dwNameLen + 1;
		else dwBufLen = 0;
	}
	return false;
}
