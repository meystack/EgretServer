#include "_osdef.h"
#ifndef WIN32
#include <malloc.h>
#endif
#include "Stream.h"

using namespace wylib::stream;

LONGLONG CBaseStream::getSize()
{
	LONGLONG nPos = seek( 0, soCurrent );
	LONGLONG nSize = seek( 0, soEnd );
	seek( nPos, soBeginning );
	return nSize;
}

LONGLONG CBaseStream::setPosition(const LONGLONG tPosition)
{
	return seek( tPosition, soBeginning );
}

LONGLONG CBaseStream::copyFrom(CBaseStream& stream, LONGLONG tSizeToCopy)
{
	static const LONGLONG OnceReadBytes = 8192;
	LONGLONG nSizeCopyed, nSizeToRead, nSizeReaded, nOldPosition;

	//保存原始流当前的指针位置
	nOldPosition = stream.seek( 0, soCurrent );

	if ( tSizeToCopy == 0 )
	{
		stream.seek( 0, soBeginning );
		tSizeToCopy = stream.getSize();
	}

	if ( tSizeToCopy <= 0 )
		return tSizeToCopy;

	nSizeCopyed = 0;
	void *pBuffer = malloc( OnceReadBytes );

	while ( nSizeCopyed < tSizeToCopy )
	{
		nSizeToRead = tSizeToCopy - nSizeCopyed;
		if ( nSizeToRead > OnceReadBytes )
			nSizeToRead = OnceReadBytes;
		nSizeReaded = stream.read( pBuffer, (DWORD)nSizeToRead );
		if ( nSizeReaded <= 0 )
			break;
		write( pBuffer, (DWORD)nSizeReaded );
		nSizeCopyed += nSizeReaded;
	}

	free( pBuffer );

	//恢复原始流的指针位置
	stream.seek( nOldPosition, soBeginning );
	return nSizeCopyed;
}

