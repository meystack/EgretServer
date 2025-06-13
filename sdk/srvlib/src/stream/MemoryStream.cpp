#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <tchar.h>
#endif
#include "_osdef.h"
#include "_memchk.h"
#include "_ast.h"
#include "Stream.h"

using namespace wylib::stream;

CMemoryStream::CMemoryStream()
{
	m_pMemory = m_pPointer = m_pMemoryEnd = m_pStreamEnd = NULL;
}

CMemoryStream::~CMemoryStream()
{
	SafeFree( m_pMemory );
}

char* CMemoryStream::Alloc(char* ptr, const LONGLONG size)
{
	if ( size == 0 && ptr )
	{
		free( ptr );
		return NULL;
	}
	return (char*)realloc( ptr, (size_t)size );
}

bool CMemoryStream::setSize(LONGLONG tSize)
{
	LONGLONG nOldPos = m_pPointer - m_pMemory;

	m_pMemory = Alloc( m_pMemory, tSize + sizeof(INT_PTR) );
	m_pStreamEnd = m_pMemoryEnd = m_pMemory + tSize;
	*(PINT_PTR)m_pStreamEnd = 0;//在流末尾写入字符0，以便将流内容当做字符串使用
	m_pPointer = m_pMemory + nOldPos;
	if ( m_pPointer > m_pStreamEnd )
		m_pPointer = m_pStreamEnd;

	return true;
}

LONGLONG CMemoryStream::seek(const LONGLONG tOffset, const int Origin)
{
	if ( Origin == soBeginning )
		m_pPointer = m_pMemory + tOffset;
	else if ( Origin == soCurrent )
		m_pPointer += tOffset;
	else if ( Origin == soEnd )
		m_pPointer = m_pStreamEnd + tOffset;
	return m_pPointer - m_pMemory;
}

LONGLONG CMemoryStream::read(LPVOID lpBuffer, const LONGLONG tSizeToRead)
{
	LONGLONG nRemain = m_pStreamEnd - m_pPointer;
	LONGLONG tReadBytes = tSizeToRead;

	if ( nRemain <= 0 || !tReadBytes )
		return 0;

	if ( tReadBytes > nRemain )
		tReadBytes = nRemain;

	memcpy( lpBuffer, m_pPointer, (size_t)tReadBytes );
	m_pPointer += tReadBytes;
	return tReadBytes;
}

LONGLONG CMemoryStream::write(LPCVOID lpBuffer, const LONGLONG tSizeToWrite)
{
	char *pStreamEnd = m_pPointer + tSizeToWrite;
	
	if ( pStreamEnd > m_pMemoryEnd )
	{
		setSize( pStreamEnd - m_pMemory );
	}

	memcpy( m_pPointer, lpBuffer, (size_t)tSizeToWrite );
	m_pPointer += tSizeToWrite;
	return tSizeToWrite;
}

LONGLONG CMemoryStream::loadFromFile(LPCTSTR lpFileName)
{
	LONGLONG nFileSize;
	CFileStream stm(lpFileName, CFileStream::faRead | CFileStream::faShareRead);

	nFileSize = stm.getSize();
	if ( nFileSize > 0 )
	{
		setSize( nFileSize );
		stm.read( m_pMemory, nFileSize );
	}
	return nFileSize;
}

LONGLONG CMemoryStream::saveToFile(LPCTSTR lpFileName)
{
#ifdef WIN32
	CFileStream stm(lpFileName, CFileStream::faCreate  );
#else
	CFileStream stm(lpFileName, CFileStream::faCreate , CFileStream::TruncExistsOnly );
#endif

	return stm.write( m_pMemory, getSize() );
}

