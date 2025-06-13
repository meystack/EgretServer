#include "_osdef.h"
#include "_memchk.h"
#include "Stream.h"
#include "zlib.h"
#include "ZStream.h"

using namespace wylib::zstream;

CBaseZStream::CBaseZStream(CBaseStream &stream)
:Inherited()
{
	m_pStrm = &stream;
	m_nStrmPos = stream.getPosition();

	//ZeroMemory( &m_ZStrm, sizeof(m_ZStrm) );
        memset(&m_ZStrm, 0,sizeof(m_ZStrm)); 
	m_ZStrm.zalloc = (alloc_func)zlibAllocMem;
	m_ZStrm.zfree = (free_func)zlibFreeMem;
	m_ZStrm.opaque = this;

	m_nLastCode = Z_OK;
}

CBaseZStream::~CBaseZStream()
{
}

void* CBaseZStream::zlibAllocMem(CBaseZStream &ZStream, uInt Items, uInt Size)
{
	return ZStream.Alloc( Items * Size );
}

void CBaseZStream::zlibFreeMem(CBaseZStream &ZStream, void *lpBlock)
{
	ZStream.Free( lpBlock );
}

void* CBaseZStream::Alloc(LONGLONG size)
{
	return malloc((size_t)size);
}

void CBaseZStream::Free(void* ptr)
{
	free(ptr);
}

void CBaseZStream::DoProgress()
{
}

