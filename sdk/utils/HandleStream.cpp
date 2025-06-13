#include "_osdef.h"
#include "Stream.h"

using namespace wylib::stream;

#ifdef WIN32
CHandleStream::CHandleStream(HANDLE StreamHandle)
#else
CHandleStream::CHandleStream(int StreamHandle)
#endif
:Inherited()
{
	m_hHandle = StreamHandle;
}

#ifdef WIN32
void CHandleStream::setHandle(HANDLE Handle)
#else
void CHandleStream::setHandle(int Handle)
#endif
{
	m_hHandle = Handle;
}

bool CHandleStream::setSize(LONGLONG tSize)
{
#ifdef WIN32
	seek( tSize, soBeginning );
	return SetEndOfFile( m_hHandle ) != FALSE;
#else
	return	ftruncate(m_hHandle, lseek(m_hHandle, tSize, SEEK_SET)) == 0;
#endif
}

LONGLONG CHandleStream::seek(const LONGLONG tOffset, const int Origin)
{
#ifdef WIN32
	LARGE_INTEGER li, nNewPointer;
	li.QuadPart = tOffset;

	if ( SetFilePointerEx( m_hHandle, li, &nNewPointer, Origin ) )
		return (LONGLONG)nNewPointer.QuadPart;
	else return -1;
#else
	if( Origin == soBeginning )
                return ( LONGLONG )lseek( m_hHandle, tOffset, SEEK_SET ) ;
        else if( Origin == soCurrent )
                return ( LONGLONG )lseek( m_hHandle, tOffset, SEEK_CUR ) ;
        else if( Origin == soEnd )
                return ( LONGLONG )lseek( m_hHandle, tOffset, SEEK_END ) ;
			else
				return -1;
#endif

}

LONGLONG CHandleStream::read(LPVOID lpBuffer, const LONGLONG tSizeToRead)
{
	DWORD dwBytesReaded;
#ifdef WIN32
	if ( ReadFile( m_hHandle, lpBuffer, (DWORD)tSizeToRead, &dwBytesReaded, NULL ) )
		return dwBytesReaded;
	else return -1;
#else
	dwBytesReaded = ::read( m_hHandle, lpBuffer, (int)tSizeToRead) ;
	if(dwBytesReaded > 0)
		return dwBytesReaded;
	else return -1;
#endif
}

LONGLONG CHandleStream::write(LPCVOID lpBuffer, const LONGLONG tSizeToWrite)
{
	DWORD dwBytesWriten;
#ifdef WIN32
	if ( WriteFile( m_hHandle, lpBuffer, (DWORD)tSizeToWrite, &dwBytesWriten, NULL ) )
		return dwBytesWriten;
	else return -1;
#else
	dwBytesWriten = ::write( m_hHandle, lpBuffer, (int)tSizeToWrite);
	if(dwBytesWriten > 0)
		return dwBytesWriten;
	else return -1;
#endif
}
