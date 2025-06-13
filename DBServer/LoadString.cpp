
#include "StdAfx.h"
#include "wrand.h"

CLoadString::CLoadString()
{

}

CLoadString::~CLoadString()
{

}

INT_PTR CLoadString::loadFromFile( LPCSTR sFileName )
{
	if (m_FileStream.loadFromFile(sFileName) <= 0)
		return 0;
	LPCTSTR sPtr = (LPCTSTR)m_FileStream.getMemory();
	LPCTSTR sTag;

	//跳过 UTF8 文件的头三个字节BOM
	BYTE *pHead = (BYTE*)sPtr;
	if (pHead[0]==0xEF && pHead[1]==0xBB && pHead[2]==0xBF)
	{
		sPtr += 3;
	}

	while (*sPtr)
	{
		//跳过行首的空白字符
		while (*sPtr && (unsigned int)*sPtr <= 0x20)
			sPtr++;

		//将名字指针添加到名字列表中
		if (*sPtr)
		{
			add(sPtr);
		}
		//搜索换行符
		sTag = _tcschr(sPtr, '\n');
		if (sTag)
		{
			*(LPTSTR)(sTag - 1) = 0; //把 '\r'也填充0
			*(LPTSTR)sTag = 0;
			sTag++;
			//跳过行尾的空白字符
			while (*sTag && (unsigned int)*sTag <= 0x20)
			{
				*(LPTSTR)sTag = 0;
				sTag++;
			}
			if (*sTag)
			{
				sPtr = sTag;
			}
		}
		if (NULL == sTag || !(*sTag))
		{
			break;
		}
	}
	//重新填充名称使用情况指针
	INT_PTR nCount = count();
	OutputMsg(rmNormal, _T("加载了%s，数量%d"), sFileName, nCount);
	return nCount;
}

bool CLoadString::GetName(LPCSTR & sName, INT_PTR nIdx)
{
	if (nIdx < 0 || nIdx >= count())
	{
		return false;
	}
	sName = (*this)[nIdx];
	return true;
}

const char * CLoadString::GetRandomName()
{
	INT_PTR nCount = count();
	if (nCount <= 0)
	{
		return NULL;
	}
	INT_PTR nRandomVal = wrand((unsigned long)nCount);
	return (*this)[nRandomVal];
}
