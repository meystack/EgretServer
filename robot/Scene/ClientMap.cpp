// #include <stdio.h>
// #include <tchar.h>
// #include <Windows.h>
// #include <_ast.h>
// #include <_memchk.h>
// #include <zlib.h>
// #include "Stream.h"
// #include "ZStream.h"
// #include "ClientMap.h"

// using namespace wylib::stream;
// using namespace wylib::zstream;

// //地图文件头标志
// const DWORD MapFileHeaderIdent		= 0x00504D57; 
// //地图版本号定义
// const DWORD MapFileVersion_101116	= 0x000A0B10;
// const DWORD MapFileVersion_101206	= 0x000A0C06;

// //使用战将2的资源
// const DWORD MapFileVersion_120320	= 0x000A0302;


// //当前地图版本号
// const DWORD MapFileVersion_Current	= MapFileVersion_120320;
// //不可移动标记

// const UINT_PTR MAPFLAG_MOVEABLE  = 0x8000;

// /* 
//  * 地图文件头结构
//  */
// typedef struct tagJXMapFileHeader
// {
// 	DWORD	dwIdent;		//文件头标识，固定为0x004D584A
// 	DWORD	dwVersion;		//文件版本，固定为0x000A0B0F
// 	INT		nWidth;			//地图宽度
// 	INT		nHeight;		//地图高度
// 	INT		nBackground;	//地图背景图片编号
// 	DWORD	dwDataSize;		//地图坐标数据段字节长度
// 	BYTE	btReseve[40];
// }JXMAPFILE_HEADER, *PJXMAPFILE_HEADER;




// /* 
//  * 地图坐标完整结构
//  */
// typedef struct tagJXMapCell
// {
// 	WORD	wBkImg;			//背景图片编号
// 	WORD	wFtImg;			//前景图片编号
// 	WORD	wFlag;			//地图坐标标志位
// 	BYTE	btObjRoot;		//素材分类索引
// 	BYTE	btReseve;
// }JXMAPCELL, *PJXMAPCELL;

// CAbstractMap::CAbstractMap()
// {
// 	m_dwWidth = 0;
// 	m_dwHeight = 0;
// 	m_pMoveableIndex = NULL;
// 	m_nMoveableCount = 0;
// }

// CAbstractMap::~CAbstractMap()
// {
// 	if ( m_pMoveableIndex )
// 		free(m_pMoveableIndex);
// }

// bool CAbstractMap::LoadFromStream(CBaseStream& stream)
// {
	

// 	JXMAPFILE_HEADER hdr;
// 	size_t dwCellSize;
// 	PJXMAPCELL pMapCell, pMapCellPtr;

// 	//读取并检查文件头
// 	if ( stream.read(&hdr, sizeof(hdr)) != sizeof(hdr) )
// 		return false;
// 	if ( hdr.dwIdent != MapFileHeaderIdent )
// 		return false;
// 	if ( hdr.dwVersion != MapFileVersion_Current )
// 		return false;

// 	INT_PTR nTotalSize= stream.getSize(); //获取文件的大小
// 	INT_PTR nDataSize = nTotalSize - sizeof(hdr); //减去头的长度就是数据的长度
// 	//将压缩过的坐标数据段读取到内存流
// 	CMemoryStream ms;
// 	ms.copyFrom(stream, nDataSize);
// 	ms.setPosition(0);

// 	//从内存流中创建解压流，以便解压坐标数据
// 	CZDecompressionStream deStream(ms);

// 	dwCellSize = sizeof(*pMapCell) * hdr.nWidth * hdr.nHeight;
// 	pMapCellPtr = pMapCell = (PJXMAPCELL)malloc(dwCellSize);
// 	if ( dwCellSize != deStream.read(pMapCell, dwCellSize) )
// 		return false;

// 	//生成抽象灰度地图
// 	int  max = hdr.nWidth * hdr.nHeight;

// 	m_nMoveableCount = 0;
// 	PINT pMoveIndex = m_pMoveableIndex = (PINT)realloc(m_pMoveableIndex, max * sizeof(*pMoveIndex));
// 	memset(pMoveIndex, -1, max * sizeof(*pMoveIndex));

// 	for ( int i=0; i<max; ++i)
// 	{
// 		if ((pMapCellPtr->wFlag & MAPFLAG_MOVEABLE) == 0)
// 		{
// 			(*pMoveIndex) = 0;
// 			m_nMoveableCount++;
// 		}
// 		pMoveIndex++;
// 		pMapCellPtr++;
// 	}	

// 	//保存地图宽度以及高度数据
// 	m_dwWidth = hdr.nWidth;
// 	m_dwHeight = hdr.nHeight;

// 	free(pMapCell);
// 	return true;
// }

// bool CAbstractMap::LoadFromFile(LPCTSTR sFileName)
// {
// 	CFileStream fs(sFileName, CFileStream::faRead || CFileStream::faShareRead);
// 	return LoadFromStream(fs);
// }

// void CAbstractMap::initDefault(DWORD dwWidth, DWORD dwHeight)
// {
// 	//生成抽象灰度地图
// 	int i, max = dwWidth * dwHeight;

// 	m_nMoveableCount = 0;
// 	PINT pMoveIndex = m_pMoveableIndex = (PINT)realloc(m_pMoveableIndex, max * sizeof(*pMoveIndex));

// 	for ( i=0; i<max; ++i)
// 	{
// 		*pMoveIndex = m_nMoveableCount;
// 		m_nMoveableCount++;
// 		pMoveIndex++;
// 	}
// 	//保存地图宽度以及高度数据
// 	m_dwWidth = dwWidth;
// 	m_dwHeight = dwHeight;
// }
