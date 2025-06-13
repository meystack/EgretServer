#include "_osdef.h"
#include "_memchk.h"
#include "Stream.h"
#include "zlib.h"
#include "ZStream.h"

using namespace wylib::zstream;

CZDecompressionStream::CZDecompressionStream(CBaseStream &source)
:Inherited(source)
{
	m_ZStrm.next_in = m_sBuffer;
	m_ZStrm.avail_in = 0;
	m_nLastCode = inflateInit_(&m_ZStrm, zlib_version, sizeof(m_ZStrm));
}

CZDecompressionStream::~CZDecompressionStream()
{
	m_pStrm->seek( -(signed)m_ZStrm.avail_in, CBaseStream::soCurrent );
	m_nLastCode = inflateEnd(&m_ZStrm);
}

LONGLONG CZDecompressionStream::seek(const LONGLONG Offset, const int Origin)
{
	LONGLONG i;
	char Buf[4096];
	LONGLONG tOffset = Offset;
	
	if ( !succeeded() )
		return -1;
	if ( tOffset == 0 && Origin == CBaseStream::soBeginning )
	{
		m_nLastCode = inflateReset(&m_ZStrm);
		if ( !succeeded() )
			return -1;
		m_ZStrm.next_in = m_sBuffer;
		m_ZStrm.avail_in = 0;
		m_pStrm->setPosition(0);
		m_nStrmPos = 0;
	}
	else if ( (tOffset >= 0 && Origin == CBaseStream::soCurrent) || (tOffset - m_ZStrm.total_out > 0 && Origin == CBaseStream::soBeginning) )
	{
		if ( Origin == CBaseStream::soBeginning )
			tOffset -= m_ZStrm.total_out;
		if ( tOffset > 0 )
		{
			for ( i=tOffset / sizeof(Buf) - 1; i>-1; --i )
			{
				read( Buf, sizeof(Buf) );
			}
			read( Buf, tOffset % sizeof(Buf) );
		}
	}
	return m_ZStrm.total_out;
}

LONGLONG CZDecompressionStream::read(LPVOID lpBuffer, const LONGLONG tSizeToRead)
{
	LONGLONG nSizeReaded;

	m_ZStrm.next_out = (Bytef*)lpBuffer;
	m_ZStrm.avail_out = (uInt)tSizeToRead;

	if ( !succeeded() )
		return -1;

	if ( m_pStrm->getPosition() != m_nStrmPos )
		m_pStrm->setPosition( m_nStrmPos);
	while ( m_ZStrm.avail_out > 0 )
	{
		if ( m_ZStrm.avail_in == 0 )
		{
			nSizeReaded = m_pStrm->read( m_sBuffer, sizeof(m_sBuffer) );
			if ( nSizeReaded <= 0 )
			{
				return tSizeToRead - m_ZStrm.avail_out;
			}
			m_ZStrm.avail_in = (uInt)nSizeReaded;
			m_ZStrm.next_in = m_sBuffer;
			m_nStrmPos = m_pStrm->getPosition();
			DoProgress();
		}
		m_nLastCode = inflate(&m_ZStrm, 0);
		if ( !succeeded() )
		{
			return -1;
		}
	}
	return tSizeToRead;
}
