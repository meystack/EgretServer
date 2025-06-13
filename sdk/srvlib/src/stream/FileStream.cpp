//#include "StdAfx.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#ifdef WIN32
#include <tchar.h>
#endif
#include "_osdef.h"
#include "_memchk.h"
#include "_ast.h"
#include "Stream.h"

using namespace wylib::stream;

CFileStream::CFileStream(LPCTSTR lpFileName, DWORD dwAccessType)
:Inherited(INVALID_HANDLE_VALUE)
{
	OpenDisposition CreationDisposition;
#ifdef WIN32
	if ( dwAccessType & faCreate || AlwaysCreate &dwAccessType)
		CreationDisposition = static_cast<OpenDisposition>(CREATE_ALWAYS);
	else CreationDisposition = static_cast<OpenDisposition>(OPEN_EXISTING);
#else
	if ( dwAccessType & faCreate || AlwaysCreate &dwAccessType )
		CreationDisposition = static_cast<OpenDisposition>(O_CREAT);
	else CreationDisposition = static_cast<OpenDisposition>(O_EXCL);
#endif
	construct( lpFileName, dwAccessType, CreationDisposition );
}

CFileStream::CFileStream(LPCTSTR lpFileName, DWORD dwAccessType, OpenDisposition eWin32CreateionDisposition)
:Inherited(INVALID_HANDLE_VALUE)
{
	construct( lpFileName, dwAccessType, eWin32CreateionDisposition );
}

CFileStream::~CFileStream()
{
	if ( m_hHandle != INVALID_HANDLE_VALUE )
	{
#ifdef WIN32
		CloseHandle( m_hHandle );
#else
		close(m_hHandle);
#endif
	}
	SafeFree(m_sFileName);
}

void CFileStream::setFileName(LPCTSTR lpFileName)
{
#ifdef WIN32
	m_sFileName = (LPTSTR)malloc((_tcslen(lpFileName) + 1) * sizeof(lpFileName[0]));
	_tcscpy( m_sFileName, lpFileName );
#else
	m_sFileName = (LPTSTR)malloc((strlen(lpFileName) + 1) * sizeof(lpFileName[0]));
	strcpy( m_sFileName, lpFileName );
#endif
}

void CFileStream::construct(LPCTSTR lpFileName, DWORD dwAccessType, DWORD dwWin32CreationDisposition)
{
	DWORD dwDesiredAccess, dwShareMode ;

	setFileName( lpFileName );

	dwDesiredAccess = dwShareMode = 0;

#ifdef WIN32
	if ( dwAccessType & faCreate  )
	{
		dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
	}
	else
	{
		if ( dwAccessType & faRead ) dwDesiredAccess |= GENERIC_READ;
		if ( dwAccessType & faWrite ) dwDesiredAccess |= GENERIC_WRITE;
		if ( dwAccessType & faShareRead ) 
		{
			dwShareMode |= FILE_SHARE_READ;
			dwDesiredAccess |= GENERIC_READ;
		}
		if ( dwAccessType & faShareWrite )
		{
			dwShareMode |= FILE_SHARE_WRITE;
			dwDesiredAccess |= GENERIC_WRITE;
		}
		if ( dwAccessType & faShareDelete ) dwShareMode |= FILE_SHARE_DELETE;
	}

	setHandle( CreateFile( m_sFileName, dwDesiredAccess, dwShareMode, NULL, dwWin32CreationDisposition, 0, NULL ) );
#else
	if ( (dwAccessType & faCreate)  | (dwAccessType & CreateIfNotExists) )
	{
		dwDesiredAccess = O_RDWR |  O_CREAT;
		dwShareMode = S_IRUSR|S_IWUSR;
	}
	else
	{
		if ( dwAccessType & faRead ) 
		{
			dwDesiredAccess |= O_RDONLY;	
			dwShareMode |= S_IRUSR;
		}
		if ( dwAccessType & faWrite ) 
		{
			dwDesiredAccess |= O_WRONLY;
			dwShareMode |= S_IWUSR;
		}
		if ( dwAccessType & faShareRead ) 
		{
			dwShareMode |= S_IROTH;
			dwDesiredAccess |= O_RDONLY;
		}
		if ( dwAccessType & faShareWrite )
		{
			dwShareMode |= S_IWOTH;
			dwDesiredAccess |= O_WRONLY;
		}
		//if ( dwAccessType & faShareDelete ) dwShareMode |= FILE_SHARE_DELETE;
	}
	
	if( CreateIfNotExists == dwWin32CreationDisposition )
	{
		dwDesiredAccess |=O_CREAT ;
	}
	else if( AlwaysCreate ==dwWin32CreationDisposition )
	{
		dwDesiredAccess |=O_CREAT ;
	}
	else if( OpenExistsOnly == dwWin32CreationDisposition )
	{
		dwDesiredAccess |=O_CREAT ;
		dwDesiredAccess |=O_EXCL;
	}
	else if( AlwaysOpen == dwWin32CreationDisposition )
	{
		dwDesiredAccess |=O_APPEND ;
	}
	else if( TruncExistsOnly == dwWin32CreationDisposition )
	{
		dwDesiredAccess |=O_TRUNC;
	}
	
	int hFd = open(m_sFileName, dwDesiredAccess, dwShareMode);

	setHandle(hFd );
#endif

}
