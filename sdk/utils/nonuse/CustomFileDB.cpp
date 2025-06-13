#ifdef WIN32

#include <stdio.h>
#include <stdlib.h>
#include <crtdbg.h>
#include <tchar.h>
#include <Windows.h>
#include <_ast.h>
#include <_memchk.h>
#include <QueueList.h>
#include <Tick.h>
#include <wyString.h>
#include "ShareUtil.h"
#include "../include/FDOP.h"
#include "../include/ObjectAllocator.hpp"
#include "../include/CustomSortList.h"
#include "CustomFileDB.h"
#include <new>

const TCHAR CCustomFileDB::DataFileExt[] = _T(".fdb");
const TCHAR CCustomFileDB::IndexFileExt[] = _T(".fdi");

using namespace wylib::sync::lock;

CCustomFileDB::CCustomFileDB()
{
	m_hIndexFile = m_hDataFile = INVALID_HANDLE_VALUE;
	m_nNextIndexOffset = sizeof(m_Header);
	ZeroMemory(&m_Header, sizeof(m_Header));
	m_boFlushFileDataImmediately = FALSE;
}

CCustomFileDB::~CCustomFileDB()
{
	close();
}

class CFDBOpenHelper
{
public:
	HANDLE hIndex;
	HANDLE hData;
	CCustomFileDB::ChunkIndex::ChunkDesc *pChunkDescBuffer;

	CFDBOpenHelper()
	{
		hIndex = hData = INVALID_HANDLE_VALUE;
		pChunkDescBuffer = NULL;
	}
	~CFDBOpenHelper()
	{ 
		if (hIndex != INVALID_HANDLE_VALUE)
			CloseHandle(hIndex);
		if (hData != INVALID_HANDLE_VALUE)
			CloseHandle(hData);
		if (pChunkDescBuffer)
			free(pChunkDescBuffer);
	}
};

bool CCustomFileDB::open(LPCTSTR sDBName)
{
	CFDBOpenHelper openHelper;
	//以共享读方式打开数据文件
	String sPath = sDBName;
	String sFilePath = sPath + DataFileExt;
	openHelper.hData = CreateFile(sFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
	if (openHelper.hData == INVALID_HANDLE_VALUE)
	{
		OutputError(GetLastError(), _T("Can not open DataFile \"%s\""), sFilePath.rawStr());
		return false;
	}
	//以共享读方式打开索引文件
	sFilePath = sPath + IndexFileExt;
	openHelper.hIndex = CreateFile(sFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
	if (openHelper.hIndex == INVALID_HANDLE_VALUE)
	{
		OutputError(GetLastError(), _T("Can not open IndexFile \"%s\""), sFilePath.rawStr());
		return false;
	}
	//读取文件头
	SetFilePointer(openHelper.hIndex, 0, NULL, 0);
	FDBHeader hdr;
	DWORD dwBytesReaded;
	if (!ReadFile(openHelper.hIndex, &hdr, sizeof(hdr), &dwBytesReaded, NULL) || dwBytesReaded != sizeof(hdr))
	{
		OutputError(GetLastError(), _T("Can not read IndexHeader"));
		return false;
	}
	//检查文件头标志
	if (hdr.dwIdent != FDBHeader::IDENT)
	{
		OutputMsg(rmError, _T("Invalid IndexHeader %08X"), hdr.dwIdent);
		return false;
	}
	//检查文件版本
	if (hdr.dwVersion != FDBHeader::VERSION)
	{
		OutputMsg(rmError, _T("Invalid IndexVersion %08X"), hdr.dwVersion);
		return false;
	}
	//申请索引数据临时内容缓冲区
	DWORD dwDataIndexSize = sizeof(openHelper.pChunkDescBuffer[0]) * hdr.nRecordCount;
	openHelper.pChunkDescBuffer = (ChunkIndex::ChunkDesc*)malloc(dwDataIndexSize);
	if (!openHelper.pChunkDescBuffer)
	{
		OutputMsg(rmError, _T("Out of memory to alloc indexbuffer %08X"), hdr.dwVersion);
		return false;
	}
	//读取索引数据内容
	if (!ReadFile(openHelper.hIndex, openHelper.pChunkDescBuffer, dwDataIndexSize, &dwBytesReaded, NULL) || dwBytesReaded != dwDataIndexSize)
	{
		OutputMsg(rmError, _T("Invalid IndexVersion %08X"), hdr.dwVersion);
		return false;
	}

	//关闭当前文件
	close();

	//将文件句柄以及文件头保存到类中
	m_hIndexFile = openHelper.hIndex;
	m_hDataFile = openHelper.hData;
	m_nNextIndexOffset = sizeof(m_Header) + dwDataIndexSize;
	openHelper.hIndex = openHelper.hData = INVALID_HANDLE_VALUE;
	m_Header = hdr;

	//保留读取内存空间
	m_IndexRecordList.reserve(hdr.nRecordCount);
	m_DataList.setSorted(FALSE);
	m_DataList.reserve(hdr.nRecordCount);
	m_FreeDataSizeList.setSorted(FALSE);
	m_FreeDataSizeList.reserve(hdr.nRecordCount);
	m_FreeDataOffsetList.setSorted(FALSE);
	m_FreeDataOffsetList.reserve(hdr.nRecordCount);
	m_NullIndexList.reserve(hdr.nRecordCount);

	//开始读取索引数据到内存
	ChunkIndex::ChunkDesc *pChunkDesc = openHelper.pChunkDescBuffer;
	ChunkIndex *pIndex = m_IndexAllocator.allocObjects(hdr.nRecordCount);
	AvaliableDataIndex avalIndex;
	FreeDataSizeIndex sizeIndex;
	FreeDataOffsetIndex offsetIndex;

	INT64 nOffset = sizeof(hdr);
	for (INT_PTR i=hdr.nRecordCount-1; i>-1; --i)
	{
		pIndex->chunk = *pChunkDesc;
		pIndex->nIndexOffset = nOffset;
		m_IndexRecordList.add(pIndex);

		//如果记录块大小值为0则表示该记录为没有任何意义，需将记录存储在空闲块列表中。
		if (pChunkDesc->nChunkSize == 0)
		{
			m_NullIndexList.add(pIndex);
		}
		//如果记录ID值为零则表示该记录为一个空闲的数据块，需将记录存储在空闲块列表中。
		else if (pChunkDesc->nId == 0)
		{
			sizeIndex.pIndex = pIndex;
			m_FreeDataSizeList.add(sizeIndex);
			offsetIndex.pIndex = pIndex;
			m_FreeDataOffsetList.add(offsetIndex);
		}
		else
		{
			//数据有效，添加到有效数据列表中
			avalIndex.pIndex = pIndex;
			m_DataList.add(avalIndex);
		}
		nOffset += sizeof(*pChunkDesc);
		pChunkDesc++;
		pIndex++;
	}
	m_DataList.setSorted(TRUE);
	m_FreeDataSizeList.setSorted(TRUE);
	m_FreeDataOffsetList.setSorted(TRUE);

	return true;
}

void CCustomFileDB::close()
{
	CSafeLock sl1(&m_IndexFileLock);
	CSafeLock sl2(&m_DataFileLock);

	if (m_hIndexFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hIndexFile);
		m_hIndexFile = INVALID_HANDLE_VALUE;
	}
	if (m_hDataFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hDataFile);
		m_hDataFile = INVALID_HANDLE_VALUE;
	}
	m_IndexRecordList.trunc(0);
	m_DataList.trunc(0);
	m_FreeDataSizeList.trunc(0);
	m_FreeDataOffsetList.trunc(0);
	m_NullIndexList.trunc(0);
	//释放索引块内存管理器中的内存并重新初始化内存管理器
	m_IndexAllocator.~CObjectAllocator<ChunkIndex>();
	new (&m_IndexAllocator) IndexAllocator();
}

bool CCustomFileDB::create(LPCTSTR sDBName)
{
	CFDBOpenHelper helper;
	String sPath = sDBName;
	String sDataFile = sPath + DataFileExt;
	String sIndexFile = sPath + IndexFileExt;

	//数据文件已经存在则不能再创建
	if (FDOP::IsArchive(sDataFile))
	{
		OutputMsg(rmError, _T("Can not create new database, data file \"%s\" already exists"), sDataFile.rawStr());
		return false;
	}
	//索引文件已经存在则不能再创建
	if (FDOP::IsArchive(sIndexFile))
	{
		OutputMsg(rmError, _T("Can not create new database, index file \"%s\" already exists"), sIndexFile.rawStr());
		return false;
	}
	//逐层创建数据库目录，例如指定数据库./FDB/db1/char，则需要创建目录./FDB以及./FDB/db1。
	FDOP::ExtractFileDirectory(sDBName, sPath, MAXINT);
	if (!FDOP::DeepCreateDirectory(sPath))
	{
		OutputMsg(rmError, _T("Can not create new database, index file \"%s\" already exists"), sIndexFile.rawStr());
		return false;
	}
	//创建数据库索引文件
	helper.hIndex = CreateFile(sIndexFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, 0, NULL);
	if (helper.hIndex == INVALID_HANDLE_VALUE)
	{
		OutputMsg(rmError, _T("Can not create new index file \"%s\")"), sIndexFile.rawStr());
		return false;
	}
	//向索引文件中写入文件头
	FDBHeader hdr;
	DWORD dwSizeWritten;
	ZeroMemory(&hdr, sizeof(hdr));
	hdr.dwIdent = FDBHeader::IDENT;
	hdr.dwVersion = FDBHeader::VERSION;
	if (!WriteFile(helper.hIndex, &hdr, sizeof(hdr), &dwSizeWritten, NULL) || sizeof(hdr) != dwSizeWritten)
	{
		OutputMsg(rmError, _T("Can not write new index file \"%s\""), sIndexFile.rawStr());
		return false;
	}
	//创建数据文件
	helper.hData = CreateFile(sDataFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, 0, NULL);
	if (helper.hData == INVALID_HANDLE_VALUE)
	{
		OutputMsg(rmError, _T("Can not create new data file \"%s\""), sDataFile.rawStr());
		return false;
	}

	//关闭当前数据库
	close();
	//保存当前的数据库文件句柄为新打开的句柄
	m_Header = hdr;
	m_hIndexFile = helper.hIndex;
	m_hDataFile = helper.hData;
	//
	helper.hIndex = helper.hData = INVALID_HANDLE_VALUE;
	return true;
}

bool CCustomFileDB::put(INT64 nDataId, LPCVOID lpData, INT64 dwSize)
{
	//不允许数据ID值为0，因为在索引中记录ID值为零用于表示索引记录包含一个空闲的数据块
	if (nDataId == 0)
	{
		DbgAssert(FALSE);
		return false;
	}

	ChunkIndex *pIndex, *pNewIndex;
	DataHeader dh;

	dh.dwIdent = DataHeader::VALID_IDENT;
	dh.nId = nDataId;
	dh.nDataSize = dwSize;

	dwSize += sizeof(dh);

	INT_PTR nIdx = m_DataList.search(nDataId);
	if (nIdx > -1)
	{
		pIndex = m_DataList[nIdx];
		/*如果当前块空间足够容纳新数据，则检查是否因当前块的数据减少了从而需要将一
		个大的块拆解为两个独立的记录块。如果当前快空间不足容纳新数据，则必须申请一
		个新的块来存放数据并且将当前块添加为空闲块。*/
		if (pIndex->chunk.nChunkSize >= dwSize)
		{
			INT64 nOldDataSize = pIndex->chunk.nDataSize;
			pIndex->chunk.nDataSize = dwSize;
			if (!writeData(pIndex, &dh, lpData))
				return false;
			//计算可以从这个大的数据块中分出多大的新的数据快
			INT64 dwSplitOutSize = getChunkSplitOutSize(pIndex->chunk.nChunkSize, dwSize);
			//如果可以分出新的数据块则拆分数据块
			if (dwSplitOutSize > 0)
			{
				if (!splitLargeChunk(pIndex, dwSplitOutSize))
					return false;
			}
			else 
			{
				//如果数据块没有进行拆分则检查是否需要更新改块的索引数据
				if (nOldDataSize != dwSize)
				{
					if (!flushIndex(pIndex))
						return false;
				}
			}
		}
		else
		{
			//重新申请新记录块，并向新的块中写入数据
			pNewIndex = reallocChunk(pIndex, dwSize);
			if (!pNewIndex)
				return false;
			//向数据文件中更新数据
			if (!writeData(pNewIndex, &dh, lpData))
				return false;
			//将新记录块索引写入到索引文件中。
			//必须在writeData发生后才能写，因为当pNewIndex是全新的索引时chunk.nDataOffset为-1
			//用于表示在writeData发生时再确定数据块索引
			if (!flushIndex(pNewIndex))
				return NULL;
			//将新的记录索引替换到数据索引列表中
			m_DataList[nIdx].pIndex = pNewIndex;
		}
	}
	else
	{
		//申请新的记录块，并向新的块中写入数据
		pNewIndex = allocChunk(nDataId, dwSize);
		if (!writeData(pNewIndex, &dh, lpData))
			return false;
		//将新记录块索引写入到索引文件中
		if (!flushIndex(pNewIndex))
			return false;
		//将新索引添加到有效数据列表中
		AvaliableDataIndex ChunkIndex;
		ChunkIndex.pIndex = pNewIndex;
		m_DataList.add(ChunkIndex);
	}
	return true;
}

INT64 CCustomFileDB::get(INT64 nDataId, LPVOID lpBuffer, INT64 dwBufferSize) const
{
	static const DWORD DataHeaderSize = sizeof(DataHeader);

	INT_PTR nIdx = m_DataList.search(nDataId);
	if (nIdx < 0)
		return 0;

	ChunkIndex *pIndex = m_DataList[nIdx].pIndex;
	DbgAssert(pIndex->chunk.nDataSize >= DataHeaderSize);

	//dwBufferSize为0则仅表示获取数据长度
	if (dwBufferSize == 0)
	{
		if (pIndex->chunk.nDataSize > DataHeaderSize)
			return pIndex->chunk.nDataSize - DataHeaderSize;
		else return 0;
	}
	else
	{
		INT64 nOffset = pIndex->chunk.nDataOffset + DataHeaderSize;
		INT64 nSize = pIndex->chunk.nDataSize - DataHeaderSize;

		if (dwBufferSize > nSize)
			dwBufferSize = nSize;
		if (!readDataFile(nOffset, lpBuffer, dwBufferSize))
			return 0;
		return dwBufferSize;
	}
}

bool CCustomFileDB::remove(INT64 nDataId)
{
	INT_PTR nIdx = m_DataList.search(nDataId);
	if (nIdx > -1)
	{
		ChunkIndex *pIndex = m_DataList[nIdx];
		//将索引记录从数据列表移除
		m_DataList.remove(nIdx);
		//将旧的索引记录添加到空闲记录列表中
		pIndex->chunk.nId = 0;
		addFreeChunk(pIndex);
		//新索引写入到索引文件中
		if (!flushIndex(pIndex))
			return false;
		return true;
	}
	return false;
}

INT64 CCustomFileDB::getTotalDataSize() const
{
	INT_PTR i;
	INT64 nSize = 0;
	AvaliableDataIndex *pIndexList = m_DataList;

	AvaliableDataIndex *pIndex = pIndexList;
	for (i=m_DataList.count()-1; i>-1; --i)
	{
		nSize += pIndex->pIndex->chunk.nDataSize;
		pIndex++;
	}
	return nSize;
}

INT64 CCustomFileDB::getTotalFreeChunkSize() const
{
	INT_PTR i;
	INT64 nSize = 0;
	FreeDataSizeIndex *pIndexList = m_FreeDataSizeList;

	FreeDataSizeIndex *pIndex = pIndexList;
	for (i=m_FreeDataSizeList.count()-1; i>-1; --i)
	{
		nSize += pIndex->pIndex->chunk.nChunkSize;
		pIndex++;
	}
	return nSize;
}


INT64 CCustomFileDB::getDataFileSize() const
{
	LARGE_INTEGER lSize;

	if (m_hDataFile == INVALID_HANDLE_VALUE)
		return 0;

	if (GetFileSizeEx(m_hDataFile, &lSize))
		return lSize.QuadPart;
	else return 0;
}

INT64 CCustomFileDB::getIdList(INT64 nIndex, PINT64 pIdList, INT64 nMaxCount)
{
	PINT64 pIdPtr = pIdList;
	INT_PTR nListCount = m_DataList.count();

	for (INT_PTR i=(INT_PTR)nIndex; i<nListCount && nMaxCount>0; ++i)
	{
		*pIdPtr = m_DataList[i].pIndex->chunk.nId;
		pIdPtr++;
		nMaxCount--;
	}

	return pIdPtr - pIdPtr;
}

CCustomFileDB::ChunkIndex* CCustomFileDB::allocChunk(INT64 nId, INT64 dwDataSize)
{
	ChunkIndex *pResult = NULL;
	INT64 nChunkSize = 0;

	//当dwDataSize为0时表示仅申请一个索引对象，不预留空间。
	if (dwDataSize != 0)
	{
		//根据数据块单元大小调整nChunkSize
		if (m_Header.dwChunkSize > 0)
			nChunkSize = dwDataSize + (m_Header.dwChunkSize - (dwDataSize % m_Header.dwChunkSize));
		else nChunkSize = dwDataSize;

		//在空闲列表中找一个大小最接近的数据块索引
		INT_PTR nIdx = m_FreeDataSizeList.searchMiniGreater(nChunkSize);
		if (nIdx > -1)
		{
			FreeDataOffsetIndex offsetIndex;

			pResult = m_FreeDataSizeList[nIdx].pIndex;
			DbgAssert(pResult->chunk.nChunkSize >= nChunkSize);
			offsetIndex.pIndex = pResult;
			INT_PTR nOffsetIndex = m_FreeDataOffsetList.search(pResult->chunk.nDataOffset);
			if (nOffsetIndex > -1)
			{
				DbgAssert(m_FreeDataOffsetList[nOffsetIndex].pIndex == pResult);
				//分别从空闲偏移以及空闲大小列表中删除索引记录
				m_FreeDataSizeList.remove(nIdx);
				m_FreeDataOffsetList.remove(nOffsetIndex);
			}
			else
			{
				OutputMsg(rmError, _T("Missing SizeRecord of Free ChunkIndex at %08X"), pResult);
				DbgAssert(FALSE);
				pResult = NULL;
			}
		}
	}
	//空闲数据块列表中没有合适的记录，则在数据文件末尾写入
	if (!pResult)
	{
		if (m_NullIndexList.count() > 0)
		{
			pResult = m_NullIndexList.pop();
		}
		else
		{
			pResult = m_IndexAllocator.allocObjects(1);
			m_IndexRecordList.add(pResult);
			//将nIndexOffset置为-1，以便在写索引的时候再确定记录偏移位置
			pResult->nIndexOffset = -1;
		}
		//将nDataOffset置为-1，以便在写数据的时候再确定偏移位置
		//无论是在m_NullIndexList列表中的索引记录或新创建的索引记录，都是没有数据块的！
		pResult->chunk.nDataOffset = -1;
	}
	//填充新索引记录的所有有效字段
	pResult->chunk.nId = nId;
	pResult->chunk.nDataSize = dwDataSize;
	pResult->chunk.nChunkSize = nChunkSize;

	return pResult;
}

bool CCustomFileDB::flushIndex(ChunkIndex *pIndex)
{
	bool isNewIndex = pIndex->nIndexOffset == -1;
	//如果是新索引，则计算并确定索引偏移
	if (isNewIndex)
	{
		pIndex->nIndexOffset = m_nNextIndexOffset;
		//TRACE(_T("New IndexOffset is : %08X\r\n"), pIndex->nIndexOffset);
		m_nNextIndexOffset += sizeof(pIndex->chunk);
	}
	//向索引文件中写入索引记录
	if (!writeIndexFile(pIndex->nIndexOffset, &pIndex->chunk, sizeof(pIndex->chunk)))
		return false;
	//如果是新索引，则更新索引文件头
	if (isNewIndex)
	{
		m_Header.nRecordCount++;
		if (!writeIndexFile(0, &m_Header, sizeof(m_Header)))
			return false;
	}
	return true;		
}

void CCustomFileDB::addFreeChunk(ChunkIndex *pIndex)
{
	ChunkIndex *pExistsRecord;
	FreeDataOffsetIndex offsetIndex;
	FreeDataSizeIndex sizeIndex;
	INT_PTR nIdx, nListIndex, nMergeFrom, nMergedCount, nListCount;
	DataHeader dh;

	Assert(pIndex->chunk.nId == 0);
	Assert(pIndex->chunk.nDataOffset != -1);

	//将原有的数据头标记为无效数据，以防在通过数据文件重建索引时将无用的数据块再挖掘出来！
	dh.dwIdent = DataHeader::INVALID_IDENT;//标记数据无效
	writeDataFile(pIndex->chunk.nDataOffset, &dh.dwIdent, sizeof(dh.dwIdent));

	//将索引记录添加到数据块偏移以及大小排序列表中
	offsetIndex.pIndex = pIndex;
	sizeIndex.pIndex = pIndex;
	nIdx = m_FreeDataOffsetList.add(offsetIndex);
	m_FreeDataSizeList.add(sizeIndex);

	//尝试合并与改数据块前后相邻的空闲数据块
	nMergeFrom = -1;
	nMergedCount = 0;
	nListIndex = nIdx - 1;
	//向前合并连续的数据块为一个大块
	while (nListIndex >= 0)
	{
		pExistsRecord = m_FreeDataOffsetList[nListIndex].pIndex;
		if (pExistsRecord->chunk.nDataOffset + pExistsRecord->chunk.nChunkSize == pIndex->chunk.nDataOffset)
		{
			pExistsRecord->chunk.nChunkSize += pIndex->chunk.nChunkSize;
			pIndex->chunk.nChunkSize = 0;
			pIndex->chunk.nId = 0;
			pIndex = pExistsRecord;

			nMergeFrom = nListIndex;
			nMergedCount++;
			nListIndex--;
		}
		else break;
	}
	//向后合并连续的数据块为一个大块
	nListCount = m_FreeDataOffsetList.count();
	nListIndex = nIdx + 1;
	while (nListIndex < nListCount)
	{
		pExistsRecord = m_FreeDataOffsetList[nListIndex].pIndex;
		if (pIndex->chunk.nDataOffset + pIndex->chunk.nChunkSize == pExistsRecord->chunk.nDataOffset)
		{
			pIndex->chunk.nChunkSize += pExistsRecord->chunk.nChunkSize;
			pExistsRecord->chunk.nChunkSize = 0;
			pExistsRecord->chunk.nId = 0;

			if (nMergeFrom == -1)
				nMergeFrom = nIdx;
			nMergedCount++;
			nListIndex++;
		}
		else break;
	}

	if (nMergedCount > 0)
	{
		INT_PTR i;

		//将被合并的数据块的索引从块偏移以及块大小列表中移除
		for (i=nMergeFrom + nMergedCount; i> nMergeFrom; --i)
		{
			pIndex = m_FreeDataOffsetList[i].pIndex;
			//目前查找pIndex在m_FreeDataSizeList中索引的操作采取的是完全遍历的方式！
			//由于m_FreeDataSizeList使用数据块大小成员进行排序，而列表中可能存在多个大
			//小相同的块的索引记录，因而如果使用基于对比的快速查找有可能返回其他的指针的问题。
			nListIndex = getFreeSizeListIndex(pIndex);
			if (nListIndex > -1)
			{
				m_FreeDataSizeList.remove(nListIndex);
				m_NullIndexList.add(pIndex);
				flushIndex(pIndex);
			}
			else Assert(FALSE);
		}
		m_FreeDataOffsetList.remove(nMergeFrom + 1, nMergedCount);
		//向索引文件中写入被合并后的最终块索引
		pIndex = m_FreeDataOffsetList[nMergeFrom].pIndex;
		flushIndex(pIndex);
	}
	validateListCount();
}

bool CCustomFileDB::splitLargeChunk(ChunkIndex *pIndex, INT64 dwSplitOutSize)
{
	//减少当前数据块的大小并写入索引文件中
	Assert(pIndex->chunk.nChunkSize >= dwSplitOutSize);
	pIndex->chunk.nChunkSize -= dwSplitOutSize;
	if (!flushIndex(pIndex))
		return false;
	//申请新的记录块，并向将剩余空间保留给新记录块
	ChunkIndex *pNewIndex = allocChunk(0, 0);
	pNewIndex->chunk.nChunkSize = dwSplitOutSize;
	pNewIndex->chunk.nDataOffset= pIndex->chunk.nDataOffset + pIndex->chunk.nChunkSize;
	//将新的索引记录添加到空闲记录列表中
	addFreeChunk(pNewIndex);
	//新索引写入到索引文件中
	if (!flushIndex(pNewIndex))
		return false;
	return true;
}

CCustomFileDB::ChunkIndex* CCustomFileDB::reallocChunk(ChunkIndex *pIndex, INT64 dwNewDataSize)
{
	INT64 nMergedChunkSize;
	ChunkIndex *pNewIndex;

	//首先在尝试与该数据块后面相邻的空闲数据块进行合并
	INT_PTR nIdx = m_FreeDataOffsetList.search(pIndex->chunk.nDataOffset + pIndex->chunk.nChunkSize);
	pNewIndex = ((nIdx > -1) ? m_FreeDataOffsetList[nIdx].pIndex : NULL);
	//如果改数据块与下一个块的总大小可以容纳新的数据长度，则进行合并
	if (pNewIndex && (nMergedChunkSize = pIndex->chunk.nChunkSize + pNewIndex->chunk.nChunkSize) >= dwNewDataSize)
	{
		//如果完全合并后的块太大，将从空闲块中拆分一部分空间与当前块合并，剩余的空间继续作为一个独立的块
		//例如，我们需要存储10K的数据，而空闲块中有1GB的数据，如果将1G与当前快合并将造成过度的浪费
		INT64 nSplitOutSize = getChunkSplitOutSize(nMergedChunkSize, dwNewDataSize);
		if (nSplitOutSize > 0)
		{
			Assert(nSplitOutSize <= pNewIndex->chunk.nChunkSize);
			//减少空闲块的长度
			pNewIndex->chunk.nDataOffset += pNewIndex->chunk.nChunkSize - nSplitOutSize;
			pNewIndex->chunk.nChunkSize = nSplitOutSize;
			if (!flushIndex(pNewIndex))
				return NULL;
			//增加当前块的长度
			pIndex->chunk.nChunkSize = nMergedChunkSize - nSplitOutSize;
			pIndex->chunk.nDataSize = dwNewDataSize;
			if (!flushIndex(pIndex))
				return NULL;
		}
		else
		{
			//将空闲数据块调整为无效数据块
			pNewIndex->chunk.nChunkSize = 0;
			if (!flushIndex(pNewIndex))
				return NULL;
			//扩展当前块的长度
			pIndex->chunk.nChunkSize = nMergedChunkSize;
			pIndex->chunk.nDataSize = dwNewDataSize;
			if (!flushIndex(pIndex))
				return NULL;
			//将空闲块索引记录从空闲偏移以及大小列表中移除
			INT_PTR nSizeListIdx = getFreeSizeListIndex(pNewIndex);
			Assert(nSizeListIdx > -1);
			m_FreeDataSizeList.remove(nSizeListIdx);
			m_FreeDataOffsetList.remove(nIdx);
			//将空闲块索引记录添加到无效块列表
			m_NullIndexList.add(pNewIndex);
		}

		return pIndex;
	}
	else
	{
		//申请新的记录块，并向新的块中写入数据
		pNewIndex = allocChunk(pIndex->chunk.nId, dwNewDataSize);
		//向索引文件中更新旧索引
		pIndex->chunk.nId = 0;
		if (!flushIndex(pIndex))
			return NULL;
		//将旧的索引记录添加到空闲记录列表中
		addFreeChunk(pIndex);

		return pNewIndex;
	}
	return NULL;
}

bool CCustomFileDB::writeData(ChunkIndex *pIndex, const DataHeader *pDataHeader, LPCVOID lpDataBuffer)
{
	INT64 nOffset = pIndex->chunk.nDataOffset;
	LARGE_INTEGER lFileSize, lNewSize;
	INT64 nBlockDataSize = pDataHeader->nDataSize + sizeof(*pDataHeader);
	bool boIsNewData = nOffset == -1;

	DbgAssert(pIndex->chunk.nId == pDataHeader->nId);
	DbgAssert(pIndex->chunk.nChunkSize >= nBlockDataSize);
	DbgAssert(pIndex->chunk.nChunkSize >= pIndex->chunk.nDataSize);

	CSafeLock sl(&m_DataFileLock);

	//偏移为-1表示这是一个新的数据块，需要写入数据文件末尾
	if (boIsNewData)
	{
		if (!GetFileSizeEx(m_hDataFile, &lFileSize))
		{
			OutputError(GetLastError(), _T("Can not query data file size"));
			return false;
		}
		pIndex->chunk.nDataOffset = nOffset = lFileSize.QuadPart;
	}
	if (!writeDataFile(nOffset, pDataHeader, sizeof(*pDataHeader)))
		return false;
	nOffset += sizeof(*pDataHeader);
	if (!writeDataFile(nOffset, lpDataBuffer, pDataHeader->nDataSize))
		return false;
	//如果是新数据块且数据库头中指定了数据块单元大小，则调整数据文件以进行数据块大小对齐
	if (boIsNewData && m_Header.dwChunkSize > 0 && pIndex->chunk.nChunkSize != nBlockDataSize)
	{
		lFileSize.QuadPart = pIndex->chunk.nDataOffset + pIndex->chunk.nChunkSize;
		//调整文件指针到数据块大小单元处
		if (!SetFilePointerEx(m_hDataFile, lFileSize, &lNewSize, FILE_BEGIN) || lFileSize.QuadPart != lNewSize.QuadPart)
		{
			OutputError(GetLastError(), _T("Can not query data file size"));
			return false;
		}
		//设置文件结束位置
		if (!SetEndOfFile(m_hDataFile))
		{
			OutputError(GetLastError(), _T("Can not set end of data file"));
			return false;
		}
	}
	return true;
}

bool CCustomFileDB::writeDataFile(INT64 nOffset, LPCVOID lpBuffer, INT64 dwSize)
{
	static const DWORD OnceWriteBytes = 0x10000;//每次写文件的字节数
	CSafeLock sl(&m_DataFileLock);

	LONG nHighInt = HILONG(nOffset);
	if (SetFilePointer(m_hDataFile, (LONG)nOffset, &nHighInt, FILE_BEGIN) != nOffset)
	{
		OutputError(GetLastError(), _T("Fatal error can not set data file pointer"));
		return false; 
	}

	DWORD dwBytesToWrite, dwBytesWriten;
	const char* ptr = (const char*)lpBuffer;
	while (dwSize > 0)
	{
		if (dwSize > OnceWriteBytes)
			dwBytesToWrite = OnceWriteBytes;
		else dwBytesToWrite = (LONG)dwSize;
		if (!WriteFile(m_hDataFile, ptr, dwBytesToWrite, &dwBytesWriten, NULL))
		{
			OutputError(GetLastError(), _T("Fatal error can not write data file"));
			return false; 
		}
		ptr += dwBytesWriten;
		dwSize -= dwBytesWriten;
	}
	if (m_boFlushFileDataImmediately)
	{
		FlushFileBuffers(m_hDataFile);
	}
	return true;
}

bool CCustomFileDB::readDataFile(INT64 nOffset, LPVOID lpBuffer, INT64 dwSize) const
{
	static const DWORD OnceReadBytes = 0x10000;//每次读文件的字节数
	CSafeLock sl((CBaseLock *)&m_DataFileLock);

	LONG nHighInt = HILONG(nOffset);
	if (SetFilePointer(m_hDataFile, (LONG)nOffset, &nHighInt, FILE_BEGIN) != nOffset)
	{
		OutputError(GetLastError(), _T("Fatal error can not set data file pointer"));
		return false; 
	}

	DWORD dwBytesToRead, dwBytesReaded;
	INT64 dwTotalRead = 0;
	char* ptr = (char*)lpBuffer;

	while (dwSize > dwTotalRead)
	{
		if (dwSize > OnceReadBytes)
			dwBytesToRead = OnceReadBytes;
		else dwBytesToRead = (LONG)dwSize;
		if (!ReadFile(m_hDataFile, ptr, dwBytesToRead, &dwBytesReaded, NULL) || dwBytesReaded == 0)
		{
			OutputError(GetLastError(), _T("Fatal error can not read data file"));
			return false; 
		}
		ptr += dwBytesReaded;
		dwTotalRead += dwBytesReaded;
	}
	return true;
}

bool CCustomFileDB::writeIndexFile(INT64 nOffset, LPCVOID lpBuffer, INT64 dwSize)
{
	static const DWORD OnceWriteBytes = 8192;//每次写文件的字节数
	CSafeLock sl(&m_IndexFileLock);

	LONG nHighInt = HILONG(nOffset);
	if (SetFilePointer(m_hIndexFile, (LONG)nOffset, &nHighInt, FILE_BEGIN) != nOffset)
	{
		OutputError(GetLastError(), _T("Fatal error can not set index file pointer"));
		return false; 
	}

	DWORD dwBytesToWrite, dwBytesWriten;
	const char* ptr = (const char*)lpBuffer;
	while (dwSize > 0)
	{
		if (dwSize > OnceWriteBytes)
			dwBytesToWrite = OnceWriteBytes;
		else dwBytesToWrite = (LONG)dwSize;
		if (!WriteFile(m_hIndexFile, ptr, dwBytesToWrite, &dwBytesWriten, NULL))
		{
			OutputError(GetLastError(), _T("Fatal error can not write index file"));
			return false; 
		}
		ptr += dwBytesWriten;
		dwSize -= dwBytesWriten;
	}
	if (m_boFlushFileDataImmediately)
	{
		FlushFileBuffers(m_hIndexFile);
	}
	return true;
}

void CCustomFileDB::validateListCount() const
{
	Assert(m_FreeDataOffsetList.count() == m_FreeDataSizeList.count());
	Assert(m_FreeDataOffsetList.count() + m_NullIndexList.count() + m_DataList.count() == m_IndexRecordList.count());
}

INT64 CCustomFileDB::getChunkSplitOutSize(INT64 nChunkSize, INT64 dwDataSize) const
{
	INT64 nRemainSize = nChunkSize - dwDataSize;
	/*将大的数据块拆分为小数据块的条件为：
		1、数据库头中规定的数据块大小不低于128字节；
		2、数据块写入新数据后剩余空间大于数据库头中的数据块单位大小；
		3、新数据长度是数据块长度的一半以内；
	*/
	if (nRemainSize >= 128 && (nRemainSize > m_Header.dwChunkSize) && (dwDataSize <= nChunkSize / 2) )
	{
		//将新数据块的大小调整为头中规定的数据库单位大小的倍数
		return nRemainSize / m_Header.dwChunkSize * m_Header.dwChunkSize;
	}
	//返回0表示无法分割
	return 0;
}


INT_PTR CCustomFileDB::getFreeSizeListIndex(ChunkIndex *pIndex) const
{
	INT_PTR i;
	FreeDataSizeIndex *pSizeIndexList = m_FreeDataSizeList;

	for (i=m_FreeDataSizeList.count()-1; i>-1; --i)
	{
		if (pSizeIndexList[i] == pIndex)
		{
			return i;
		}
	}
	return -1;
}
#endif
