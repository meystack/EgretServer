#pragma once

class CBufferAllocator;
class CBuffAllocStatHelper
{
public:	

	static void WriteStrToStream(wylib::stream::CBaseStream& stream, LPCTSTR szContent);
	static void Dump(wylib::stream::CBaseStream& stream, 
					CBufferAllocator* pAlloc, 
					LPCTSTR szAllocaterName = _T("Unknown"),
					bool bIncTimeStamp = true);
	static void Dump(wylib::stream::CBaseStream& stream, 
					CBufferAllocator::ALLOCATOR_MEMORY_INFO& mi, 
					LPCTSTR szAllocaterName = _T("Unknown"),
					bool bIncTimeStamp = true);

protected:
	static void DumpHeader(wylib::stream::CBaseStream& stream, LPCTSTR szAllocatorName, bool bIncTimeStamp = true);
	static void DumpImpl(wylib::stream::CBaseStream& stream, CBufferAllocator::ALLOCATOR_MEMORY_INFO& mi);

private:	
	static LPCTSTR s_szLineFeed;
};