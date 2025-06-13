#include <stdio.h>
#include<stdlib.h>
#ifdef WIN32
#include <tchar.h>
#include <Windows.h>
#endif
#include <_ast.h>
#include <_memchk.h>
#include <zlib.h>
#include "Stream.h"
#include "ZStream.h"
#include "JXAbsGameMap.h"
#include <BufferAllocator.h>

using namespace jxcomm::gameMap;
using namespace wylib::stream;
using namespace wylib::zstream;

//地图文件头标志
const DWORD MapFileHeaderIdent		= 0x00504D57; 
//地图版本号定义
const DWORD MapFileVersion_101116	= 0x000A0B10;
const DWORD MapFileVersion_101206	= 0x000A0C06;

//使用战将2的资源
const DWORD MapFileVersion_120320	= 0x000A0302;


//当前地图版本号
const DWORD MapFileVersion_Current	= MapFileVersion_120320;
//不可移动标记

const UINT_PTR MAPFLAG_MOVEABLE  = 0x8000;

#define USE_NEW_MAPFILE_HEADER

/* 
 * 地图文件头结构
 */
typedef struct tagJXMapFileHeader
{
	/*
	DWORD	dwIdent;		//文件头标识，固定为0x004D584A
	DWORD	dwVersion;		//文件版本，固定为0x000A0B0F
	INT		nWidth;			//地图宽度
	INT		nHeight;		//地图高度
	INT		nBackground;	//地图背景图片编号
	DWORD	dwDataSize;		//地图坐标数据段字节长度
	BYTE	btReseve[40];
	*/
#ifdef USE_NEW_MAPFILE_HEADER
	//char 			hdr_[5];		// "map"
	//int 			dwVersion;		// 文件版本，固定为0x000A0B0F
	//unsigned char 	dwDataSize;		// 常量，64像素
	//int 			pxi_width_;		// 地图的像素大小
	//int 			pix_height_;				
	int				nWidth;			// 地图宽度
	int				nHeight;		// 地图高度
#else
	unsigned int	dwVersion;		// 文件版本0x0101
	int				nWidth;			// 地图宽度
	int				nHeight;		// 地图高度
	int				bg_img_;		// 地图背景图片编号
	unsigned int	dwDataSize;		// 地图坐标数据段字节长度
	unsigned char	reseve_[32];
#endif
}MAPFILE_HEADER, *PMAPFILE_HEADER;


enum GripFlag
{
	gfBlock = 0, //阻挡
	gfCanMove = 1, //行走区
	gfThrough = 2, //透明区
	gfNofly = 3, //阻挡且不能飞跃
};


/* 
 * 地图坐标完整结构
 */
typedef struct tagJXMapCell
{
	unsigned char	 wFlag;		// 0=阻档，1=行走区,2=透明区,3=阻挡且不能飞跃
}MAPCELL, *PMAPCELL;

CAbstractMap::CAbstractMap()
{
	m_dwWidth = 0;
	m_dwHeight = 0;
	m_pGridData = NULL;
	m_pMoveableIndex = NULL;
	m_nMoveableCount = 0;
	
	m_nBBoxT	= 0;
	m_nBBoxL	= 0;
	m_nBBoxR	= 0;
	m_nBBoxB	= 0;	
	m_pAllocator = new CBufferAllocator();

	//m_pWayPointMap =NULL;
	//m_nWayPointCount =0;
	
}

CAbstractMap::~CAbstractMap()
{
	if ( m_pMoveableIndex )
		m_pAllocator->FreeBuffer(m_pMoveableIndex);

	if (m_pGridData)
		m_pAllocator->FreeBuffer(m_pGridData);
	
	if (m_pAllocator)
		free(m_pAllocator);

	
	/*
	if(m_pWayPointMap)
	{
		free(m_pWayPointMap);
	}
	*/
}

/*
int CAbstractMap::GetMapPosValue(int x,int y)
{
	return getMoveableIndex(x,y);
}
*/


bool CAbstractMap::LoadFromStream(CBaseStream& stream)
{
	MAPFILE_HEADER hdr;
	MAPCELL* map_grid, *grid_ptr;

	//将压缩过的坐标数据段读取到内存流
	CMemoryStream ms;
	ms.copyFrom(stream, stream.getSize());
	ms.setPosition(0);

	//从内存流中创建解压流，以便解压坐标数据
	//CZDecompressionStream deStream(ms);
	CMemoryStream& deStream = ms;
	int max = 0;

	//读取并检查文件头
	if (deStream.read(&hdr, sizeof(hdr)) != sizeof(hdr))
		return false;
	// 地图打包文件，按照大头的字节序打包了
	//hdr.nWidth = ntohl(hdr.nWidth);
	//hdr.nHeight = ntohl(hdr.nHeight);

	size_t grid_count = sizeof(MAPCELL) *  hdr.nWidth * hdr.nHeight;
	if (m_pGridData) m_pAllocator->FreeBuffer(m_pGridData);
	m_pGridData = grid_ptr = map_grid = (MAPCELL*)m_pAllocator->AllocBuffer(grid_count);
	if (grid_count != (const size_t)deStream.read(map_grid, (const int)grid_count))
		return false;
	max =  hdr.nWidth * hdr.nHeight;
	
	m_nMoveableCount = 0;
	if (m_pMoveableIndex) m_pAllocator->FreeBuffer(m_pMoveableIndex);
	BYTE* index_ptr = m_pMoveableIndex = (BYTE*)m_pAllocator->AllocBuffer(max * sizeof(*index_ptr));
	memset(index_ptr, 0, max * sizeof(*index_ptr));		// 如果是memset其他值会有问题

	// 这个索引值跟旧版本的不一样
	for (int i = 0; i < max; ++i)
	{
		if (grid_ptr->wFlag != gfBlock /*&& grid_ptr->wFlag != gfNofly*/)//不是明确的不可走点,都认为可走就行了
		{
			int x = i / hdr.nHeight;
			int y = i % hdr.nHeight;

			*(index_ptr + (y * hdr.nWidth + x)) = grid_ptr->wFlag;
			m_nMoveableCount++;
		}
		grid_ptr++;
	}

	//保存地图宽度以及高度数据
	m_dwWidth = hdr.nWidth;
	m_dwHeight = hdr.nHeight;
	return true;
}

/*
bool CAbstractMap::LoadFromStream(CBaseStream& stream)
{
	MAPFILE_HEADER hdr;
	size_t dwCellSize;
	PMAPCELL pMapCell, pMapCellPtr;

	//读取并检查文件头
	if ( stream.read(&hdr, sizeof(hdr)) != sizeof(hdr) )
	{
		
		return false;
	}
	if ( hdr.dwIdent != MapFileHeaderIdent )
	{
		return false;
	}
	if ( hdr.dwVersion != MapFileVersion_Current )
	{
		return false;
	}
	INT_PTR nTotalSize= stream.getSize(); //获取文件的大小
	INT_PTR nDataSize = nTotalSize - sizeof(hdr); //减去头的长度就是数据的长度


	//将压缩过的坐标数据段读取到内存流
	CMemoryStream ms;
	ms.copyFrom(stream, nDataSize);
	ms.setPosition(0);
	
	//从内存流中创建解压流，以便解压坐标数据
	CZDecompressionStream deStream(ms);

	dwCellSize = sizeof(*pMapCell) * hdr.nWidth * hdr.nHeight;
	pMapCellPtr = pMapCell = (PMAPCELL)malloc(dwCellSize);
	if ( dwCellSize != deStream.read(pMapCell, dwCellSize) )
		return false;
	
	//生成抽象灰度地图
	int i, max = hdr.nWidth * hdr.nHeight;

	m_nMoveableCount = 0;
	
	int nIntCount = max/32 +1; //占用多少个Int

	BYTE* pMoveIndex = m_pMoveableIndex = (BYTE*)  realloc(m_pMoveableIndex, max * sizeof(*pMoveIndex));
	memset(pMoveIndex, 0, max * sizeof(*pMoveIndex));
	
	
	m_nBBoxL = hdr.nWidth;
	m_nBBoxT = hdr.nHeight;
	for ( i=0; i<max; ++i)
	{
		if ((pMapCellPtr->wFlag & MAPFLAG_MOVEABLE) == 0)
		{
			*pMoveIndex = 1;
			m_nMoveableCount++;

			int nY = i / hdr.nWidth; //y坐标
			int nX = i % hdr.nWidth; //x坐标
			if (m_nBBoxL > nX)
				m_nBBoxL = nX;
			else if (nX > m_nBBoxR)
				m_nBBoxR = nX;

			if (m_nBBoxT > nY)
				m_nBBoxT = nY;
			else if (nY > m_nBBoxB)
				m_nBBoxB = nY;
		}
		pMoveIndex++;
		pMapCellPtr++;
	}	
	//保存地图宽度以及高度数据
	m_dwWidth = hdr.nWidth;
	m_dwHeight = hdr.nHeight;

	free(pMapCell);
	
	return true;
}
*/

bool CAbstractMap::LoadFromFile(LPCTSTR sFileName)
{
	CFileStream fs(sFileName, CFileStream::faRead || CFileStream::faShareRead);
	return LoadFromStream(fs);
}


void CAbstractMap::initDefault(DWORD dwWidth, DWORD dwHeight)
{
	//生成抽象灰度地图
	int i, max = dwWidth * dwHeight;

	m_nMoveableCount = 0;
	if (m_pMoveableIndex) m_pAllocator->FreeBuffer(m_pMoveableIndex);
	BYTE* pMoveIndex = m_pMoveableIndex = (BYTE*)m_pAllocator->AllocBuffer(max * sizeof(*pMoveIndex));
	
	for ( i=0; i<max; ++i)
	{
		(*pMoveIndex) = 1;
		m_nMoveableCount++;
		pMoveIndex++;
	}
	//保存地图宽度以及高度数据
	m_dwWidth = dwWidth;
	m_dwHeight = dwHeight;
}


