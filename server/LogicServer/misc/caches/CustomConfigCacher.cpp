#include "StdAfx.h"
#include "CustomConfigCacher.h"

using namespace wylib::stream;
using namespace FDOP;

CCustomConfigCacher::CCustomConfigCacher()
{

}

CCustomConfigCacher::~CCustomConfigCacher()
{

}

bool CCustomConfigCacher::LoadFromCache(LPCTSTR sCacheFile, DWORD dwSourceCRC, 
	CObjectAllocator<char> &allocator, OUT void** pElements, INT_PTR &nElementCount)
{
	if ( !FDOP::FileExists(sCacheFile) )
		return false;

	PCONF_DATA_CACHE_HDR pHdr;

	//加载缓存文件到内存
	if (m_CacheStream.loadFromFile(sCacheFile) <= 0)
		return false;
	//如果无法读取文件头则不是有效的缓存文件
	if (m_CacheStream.getSize() < sizeof(*pHdr))
		return false;

	pHdr = (PCONF_DATA_CACHE_HDR)m_CacheStream.getMemory();
	//如果效验文件头失败则返回false
	if (!ValidateHeader(*pHdr))
		return false;
	//如果验证数据CRC失败则表明缓存数据错误或原始数据已经修改了
	if (!ValidateCacheDataCRC(dwSourceCRC))
		return false;
	//读取缓存数据
	return ReadCacheFile(allocator, pElements, nElementCount);
}

bool CCustomConfigCacher::SaveToCache(LPCTSTR sCacheFile, DWORD dwSourceCRC, 
	const CObjectAllocator<char> &allocator, const void *pElements, INT_PTR nElementCount)
{
	CONF_DATA_CACHE_HDR hdr;
	MEM_BLOCK_REC memRec;
	TCHAR sSavePath[1024];

	//获取保存文件的目录路径，如果目录路径字符长度超出缓存长度则报错
	if ( ExtractFileDirectory(sCacheFile, sSavePath, ArrayCount(sSavePath)) >= ArrayCount(sSavePath) )
	{
		OutputMsg(rmError, _T("unable to save Config Cache Data to %s, path to long"), sCacheFile);
		return false;
	}
	//逐层判断目录是否存在，如果不存在则创建
	if ( !DeepCreateDirectory(sSavePath) )
	{
		OutputError(GetLastError(), _T("unable to create cache directory %s "), sSavePath);
		return false;
	}

	//清空数据
	m_CacheStream.setSize(0);

	//填充文件头
	ZeroMemory(&hdr, sizeof(hdr));
	hdr.dwMemBlockCount = (DWORD)allocator.blockCount();
	hdr.dwElementCount = (DWORD)nElementCount;
	hdr.dwDataCRC32 = 0xFFFFFFFF;
	hdr.dwSourceCRC32 = dwSourceCRC;
	hdr.btPlatFormIdent = sizeof(void*);
	FillHeaderData(hdr);
	
	//保留文件头空间
	m_CacheStream.setPosition(sizeof(hdr));

	//写入对象指针列表
	m_CacheStream.write(pElements, hdr.dwSizeElement * nElementCount);
	//写入内存块描述表
	LPCVOID lpKey = NULL;
	CObjectAllocator<char>::DataBlockDesc desc;
	while (lpKey = allocator.enumBlockDesc(lpKey, desc))
	{
		memRec.lpMemoryBase = (char*)desc.lpBaseAddress;
		memRec.dwMemorySize = desc.dwBlockSize;
		//TRACE(_T("MEMBLOCK-DESC:%X,%u\n"), desc.lpBaseAddress, desc.dwBlockSize);
		m_CacheStream.write(&memRec, sizeof(memRec));
	}
	//写入内存数据段
	while (lpKey = allocator.enumBlockDesc(lpKey, desc))
	{
		m_CacheStream.write(desc.lpBaseAddress, desc.dwBlockSize);
	}

	//计算数据段CRC32值
	hdr.dwDataCRC32 = ~CRC32Update(hdr.dwDataCRC32, 
		((const char*)m_CacheStream.getMemory()) + sizeof(hdr), 
		(int)m_CacheStream.getSize() - sizeof(hdr));
	//最后写入文件头
	m_CacheStream.setPosition(0);
	m_CacheStream.write(&hdr, sizeof(hdr));

	//保存到文件
	m_CacheStream.setPosition(0);
	return m_CacheStream.saveToFile(sCacheFile) >= m_CacheStream.getSize();
}

bool CCustomConfigCacher::ReadCacheFile(CObjectAllocator<char> &allocator, OUT void** pElements, INT_PTR &nElementCount)
{
	/*
	/*+-----+--------+----------+-----------+--------------+
	/*|文件头|对象数据段|内存块描述段|内存块1数据段|内存块N数据段... |
	/*+-----+--------+----------+-----------+--------------+
	*/
	CacheReadEnvir ev;
#ifdef WIN32
	__try
#endif	
	{
		ev.pHdr = (PCONF_DATA_CACHE_HDR)m_CacheStream.getMemory();
		ev.pElements = (void*)(ev.pHdr + 1);
		ev.pMemBlocks = (PMEM_BLOCK_REC)(((char*)ev.pElements) + ev.pHdr->dwSizeElement * ev.pHdr->dwElementCount);
		ev.pDataSegment = ((char*)ev.pMemBlocks) + sizeof(*ev.pMemBlocks) * ev.pHdr->dwMemBlockCount;
		m_pCacheReadEnvir = &ev;
		ev.pNewMemory = allocator.allocObjects(CalcDataSize());
		CopyBlockMemorys(ev.pNewMemory);

		INT_PTR i;
		char *pElement = (char*)ev.pElements;
		size_t dwSizeElement = ev.pHdr->dwSizeElement;

		for (i=(INT_PTR)ev.pHdr->dwElementCount-1; i>-1; --i)
		{
			if ( !AdjustElementPointers(pElement) )
				return false;
			pElement += dwSizeElement;
		}
		*pElements = ev.pElements;
		nElementCount = ev.pHdr->dwElementCount;
	}
#ifdef WIN32
	__finally
#endif
	{
		m_pCacheReadEnvir = NULL;
	}
	return true;
}

inline bool CCustomConfigCacher::ValidateCacheDataCRC(DWORD dwSourceCRC)
{
	PCONF_DATA_CACHE_HDR pHdr = (PCONF_DATA_CACHE_HDR)m_CacheStream.getMemory();

	//判断缓存生成平台
	if (sizeof(void*) != pHdr->btPlatFormIdent)
		return false;
	//-1表示强制读取缓存
	if (dwSourceCRC == -1)
		return true;
	//判断原数据的CRC32值
	if (dwSourceCRC != pHdr->dwSourceCRC32)
		return false;
	DWORD dwCRC = 0xFFFFFFFF;
	//计算并对比缓存文件整个数据段的CRC32值
	dwCRC = ~CRC32Update(dwCRC, pHdr+1, (int)m_CacheStream.getSize() - sizeof(*pHdr));
	if ( dwCRC != pHdr->dwDataCRC32 )
		return false;

	return true;
}

void* CCustomConfigCacher::GetNewMemoryPtr(LPCVOID lpAddress)
{
	INT_PTR i, nCount = (INT_PTR)m_pCacheReadEnvir->pHdr->dwMemBlockCount;
	PMEM_BLOCK_REC pMemBlock = m_pCacheReadEnvir->pMemBlocks;
	char* pNewMemPtr = m_pCacheReadEnvir->pNewMemory;

	for (i=0; i<nCount; ++i)
	{
		if (pMemBlock->lpMemoryBase <= (char*)lpAddress)
		{
			if (pMemBlock->lpMemoryBase + pMemBlock->dwMemorySize > (char*)lpAddress)
			{
				return pNewMemPtr + (((char*)lpAddress) - pMemBlock->lpMemoryBase);
			}
		}
		pNewMemPtr += pMemBlock->dwMemorySize;
		pMemBlock++;
	}
	return NULL;
}

size_t CCustomConfigCacher::CalcDataSize()
{
	INT_PTR i, nCount = (INT_PTR)m_pCacheReadEnvir->pHdr->dwMemBlockCount;
	PMEM_BLOCK_REC pMemBlock = m_pCacheReadEnvir->pMemBlocks;
	size_t result = 0;

	for (i=0; i<nCount; ++i)
	{
		result += pMemBlock->dwMemorySize;
		pMemBlock++;
	}
	return result;
}

void CCustomConfigCacher::CopyBlockMemorys(char* lpNewMemory)
{
	INT_PTR i, nCount = (INT_PTR)m_pCacheReadEnvir->pHdr->dwMemBlockCount;
	PMEM_BLOCK_REC pMemBlock = m_pCacheReadEnvir->pMemBlocks;
	char *lpCacheMemory = m_pCacheReadEnvir->pDataSegment;

	for (i=0; i<nCount; ++i)
	{
		memcpy(lpNewMemory, lpCacheMemory, pMemBlock->dwMemorySize);
		lpNewMemory += pMemBlock->dwMemorySize;
		lpCacheMemory += pMemBlock->dwMemorySize;
		pMemBlock++;
	}
}

