#include "StdAfx.h"
#include "AreaRegion.h"

CAreaRegion::CAreaRegion()
{
	m_pMapData = NULL;
	m_nMapWdith = m_nMapHeight = 0;
}

CAreaRegion::~CAreaRegion()
{
	SafeFree(m_pMapData);
}

void CAreaRegion::setSize(INT_PTR uMapWidth, INT_PTR uMapHeight)
{
	if (uMapWidth != m_nMapWdith || uMapHeight != m_nMapHeight)
	{
		SafeFree(m_pMapData);
		m_nMapWdith = uMapWidth;
		m_nMapHeight = uMapHeight;
	}  
}

bool CAreaRegion::addArea(LPPOINT lpPoints, int nNumPoints, int nAreaIndex, LPBYTE lpMapData)
{
	if (nNumPoints < 3) return false;
	if(nAreaIndex < 0 || nAreaIndex > MAXBYTE) return false;

	//如果调用者没有提供缓冲区则自己创建一个缓冲区
	if (!lpMapData)
	{
		if (!m_pMapData)
		{
			size_t dwDataSize = m_nMapWdith * m_nMapHeight * sizeof(m_pMapData[0]);
			m_pMapData = (PBYTE)malloc(dwDataSize);
			ZeroMemory(m_pMapData, dwDataSize);
		}
		lpMapData = m_pMapData;
	}

	AreaDesc area;
	area.lpPoints = lpPoints;
	area.nPointCount = nNumPoints;
	area.btIndex = (BYTE)nAreaIndex;
	area.lpMapData = lpMapData;

	//计算区域范围
	calcAreaBounds(area);
	//绘制区域轮廓
	drawAreaOutline(area);
//	traceAreaShape(area);
	//填充区域内部
	fillAreaRegion(area);
//	traceAreaShape(area);
	return true;
}

void CAreaRegion::calcAreaBounds(IN OUT AreaDesc &area)
{	
	area.nLeft = m_nMapWdith + 1;
	area.nTop = m_nMapHeight + 1;
	area.nRight = -1;
	area.nBottom = -1;

	for (INT_PTR i=0; i<area.nPointCount; ++i)
	{
		if (area.lpPoints[i].x < area.nLeft)
			area.nLeft = area.lpPoints[i].x;
		if (area.lpPoints[i].x > area.nRight)
			area.nRight = area.lpPoints[i].x;
		if (area.lpPoints[i].y < area.nTop)
			area.nTop = area.lpPoints[i].y;
		if (area.lpPoints[i].y > area.nBottom)
			area.nBottom = area.lpPoints[i].y;
	}	

	if (area.nLeft < 0) area.nLeft = 0;
	if (area.nTop < 0) area.nTop = 0;
	if (area.nRight >= m_nMapWdith) area.nRight = m_nMapWdith -1;
	if (area.nBottom >= m_nMapHeight) area.nBottom = m_nMapHeight -1;

	area.nWidth = area.nRight - area.nLeft + 1;
	area.nHeight = area.nBottom - area.nTop + 1;
}

void CAreaRegion::drawAreaOutline(const AreaDesc area)
{
	INT_PTR i, j, dist, nCount, ax, ay;
	double px, py, dx, dy;
	INT_PTR nX, nY, nMapWidth, nMapHeight;
	PBYTE pData;
	POINT pts[2];

	nMapWidth = m_nMapWdith;
	nMapHeight = m_nMapHeight;
	pData = area.lpMapData;

	nCount = area.nPointCount;
	if (*PINT64(area.lpPoints) == *PINT64(&area.lpPoints[nCount-1]))
		nCount--;
	pts[1] = area.lpPoints[0];

	for (i=1; i<=nCount; ++i)
	{
		pts[0] = pts[1];
		if (i == nCount)
			pts[1] = area.lpPoints[0];
		else pts[1] = area.lpPoints[i];

		dx = (int)pts[1].x - (int)pts[0].x;
		dy = (int)pts[1].y - (int)pts[0].y;
		ax = abs((int)dx);
		ay = abs((int)dy);
		dist = __max(ax, ay);
		px = dx / dist;
		py = dy / dist;

		dx = pts[0].x;
		dy = pts[0].y;
		for (j=dist-1; j>=0;--j)
		{
			nX = (int)(dx + 0.5);
			if (nX < 0) 
				nX = 0;
			else if (nX >= nMapWidth) 
				nX = nMapWidth - 1;

			nY = (int)(dy + 0.5);
			if (nY < 0) 
				nY = 0;
			else if (nY >= nMapHeight) 
				nY = nMapHeight - 1;

			pData[nMapWidth * nY + nX] = area.btIndex;

			dx += px;
			dy += py;
		}
	}

}

void CAreaRegion::fillAreaRegion(const AreaDesc area)
{
	INT_PTR nMapWidth = m_nMapWdith;
	PBYTE pRow = &area.lpMapData[area.nTop * nMapWidth + area.nLeft];
	PBYTE pColStart, pColEnd;

	for (INT_PTR i=area.nBottom; i>=area.nTop; --i)
	{
		pColStart = pRow;
		pColEnd = pColStart + area.nWidth;
		//确定该行中填充的起始位置
		while (pColStart < pColEnd)
		{
			if (*pColStart == area.btIndex)
			{
				break;
			}
			pColStart++;
		}
		//确定该行中填充的结束位置
		while (pColEnd > pColStart)
		{
			pColEnd--;
			if (*pColEnd == area.btIndex)
			{
				break;
			}
		}
		//如果填充起始位置小于填充结束位置，则从起始处填充到结束处
		while (pColStart < pColEnd)
		{
			*pColStart = area.btIndex;
			pColStart++;
		}
		//调整填充位置为下一行区域起始处
		pRow += nMapWidth;
	}
}

void CAreaRegion::traceAreaShape(const AreaDesc area)
{
	INT_PTR nMapWidth = m_nMapWdith;
	PBYTE pRow = &area.lpMapData[area.nTop * nMapWidth + area.nLeft];
	PBYTE pColStart, pColEnd;

	LPSTR sBuf, sChar, pDestRow;
	size_t dwBufSize = (area.nWidth + 1) * area.nHeight;
	pDestRow = sBuf = (LPSTR)malloc(dwBufSize + sizeof(INT_PTR));
	memset(sBuf, ' ', dwBufSize);
	sBuf[dwBufSize] = 0;

	for (INT_PTR i=area.nBottom; i>=area.nTop; --i)
	{
		pColStart = pRow;
		pColEnd = pColStart + area.nWidth;
		while (pColStart < pColEnd)
		{
			if (*pColStart == area.btIndex)
				break;
			pColStart++;
		}
		sChar = pDestRow + (pColStart - pRow);
		while (pColStart < pColEnd)
		{
			if (*pColStart != area.btIndex)
				break;
			*sChar = 'X';
			pColStart++;
			sChar++;
		}
		//下面这个循环用于支持在没有填充区域的情况下输出区域的右侧的边界
		while (pColStart < pColEnd)
		{
			if (*pColStart == area.btIndex)
			{
				*sChar = 'X';
				break;
			}
			pColStart++;
			sChar++;
		}
		pRow += nMapWidth;
		pDestRow += area.nWidth;
		pDestRow[0] = '\n';
		pDestRow++;
	}
	OutputDebugString(sBuf);
	OutputDebugString("\r\n");
	free(sBuf);
}
