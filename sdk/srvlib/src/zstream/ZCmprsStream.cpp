#include "_osdef.h"
#include "_memchk.h"
#include "Stream.h"
#include "zlib.h"
#include "ZStream.h"

using namespace wylib::zstream;

CZCompressionStream::CZCompressionStream(CBaseStream &destStream, CompressionLevel CmprsLvl)
:Inherited(destStream)
{
	static const int Levels[clMax + 1] = { Z_NO_COMPRESSION, Z_BEST_SPEED, Z_DEFAULT_COMPRESSION, Z_BEST_COMPRESSION };
	
	m_ZStrm.next_out = m_sBuffer;
	m_ZStrm.avail_out = sizeof(m_sBuffer);
	m_nLastCode = deflateInit_(&m_ZStrm, Levels[CmprsLvl], zlib_version, sizeof(m_ZStrm));
}

CZCompressionStream::~CZCompressionStream()
{
	finish();
	m_nLastCode = deflateEnd(&m_ZStrm);
}

LONGLONG CZCompressionStream::seek(const LONGLONG tOffset, const int Origin)
{
	if ( tOffset == 0 && Origin == CBaseStream::soCurrent )
		return m_ZStrm.total_in;
	return 0;
}

LONGLONG CZCompressionStream::write(LPCVOID lpBuffer, const LONGLONG tSizeToWrite)
{
	m_ZStrm.next_in = (Bytef*)lpBuffer;
	m_ZStrm.avail_in = (uInt)tSizeToWrite;
	if ( m_pStrm->getPosition() != m_nStrmPos )
	{
		m_pStrm->setPosition( m_nStrmPos );
	}

	while ( m_ZStrm.avail_in > 0 )
	{
		m_nLastCode = deflate(&m_ZStrm, 0);
		if ( !succeeded() )
			break;
		if ( m_ZStrm.avail_out == 0 )
		{
			m_pStrm->write( m_sBuffer, sizeof(m_sBuffer) );
			m_ZStrm.next_out = m_sBuffer;
			m_ZStrm.avail_out = sizeof(m_sBuffer);
			m_nStrmPos = m_pStrm->getPosition();
			DoProgress();
		}
	}

	return tSizeToWrite;
}

void CZCompressionStream::finish()
{
	if ( !m_ZStrm.next_in )
		return;

	m_ZStrm.next_in = NULL;
	m_ZStrm.avail_in = 0;
	if ( m_pStrm->getPosition() != m_nStrmPos )
	{
		m_pStrm->setPosition(m_nStrmPos);
	}

	while ( 1 )
	{
		m_nLastCode = deflate(&m_ZStrm, Z_FINISH);
		if ( !succeeded() )
			break;
		if ( m_nLastCode == Z_STREAM_END || m_ZStrm.avail_out != 0 )
			break;
		m_pStrm->write(m_sBuffer, sizeof(m_sBuffer));
		m_ZStrm.next_out = m_sBuffer;
		m_ZStrm.avail_out = sizeof(m_sBuffer);
	}
	if ( m_ZStrm.avail_out < sizeof(m_sBuffer) )
	{
		m_pStrm->write( m_sBuffer, sizeof(m_sBuffer) - m_ZStrm.avail_out );
	}
}
