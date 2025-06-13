#ifdef WIN32
	#include <Windows.h>
	#include <tchar.h>
#endif

#include <stdio.h>
#include <_ast.h>
#include <_memchk.h>
#include <List.h>
#include <Stream.h>
#include <wrand.h>
#include "../include/NameAllocator.h"
#include <string.h>

CNameAllocator::CNameAllocator()
:Inherited()
{
	m_nUsedCount = 0;
}

CNameAllocator::~CNameAllocator()
{

}

INT_PTR CNameAllocator::loadFromFile(LPCTSTR sFileName)
{
    if (m_NameStream.loadFromFile(sFileName) <= 0)
        return 0;
    LPCTSTR sPtr = (LPCTSTR)m_NameStream.getMemory();
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
#ifdef WIN32
		sTag = _tcschr(sPtr, '\n');
#else
		sTag = strchr(sPtr, '\n');
#endif

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
    m_NameUseList.reserve(nCount);
    m_NameUseList.trunc(nCount);
    ZeroMemory((bool*)m_NameUseList, nCount);

    //
    m_nAllocNameIndex = nCount-1;
    return nCount;
}

INT_PTR CNameAllocator::saveToFile(LPCTSTR sFileName)
{
    INT_PTR Result = 0;
    LPCTSTR *lpNamePtr = *this;
    wylib::stream::CMemoryStream ms;
	
	INT_PTR nSize = 0;
	static const TCHAR* szSep		= _T("\r\n");
	static const INT_PTR nSepSize	= _tcslen(szSep) * sizeof(TCHAR);
	for (INT_PTR i=0; i < count(); i++)
	{
		if (!m_NameUseList[i])
			nSize += _tcslen(*lpNamePtr)*sizeof(TCHAR) + nSepSize;
		lpNamePtr++;
	}
	ms.setSize(nSize);  // 预分配内存，避免频繁分配内存降低效率
	lpNamePtr = *this;
    for (INT_PTR i=0; i < count(); i++)
    {
        if (!m_NameUseList[i])
        {
			INT_PTR nNameSize = _tcslen(*lpNamePtr)*sizeof(TCHAR);
            ms.write(*lpNamePtr, nNameSize);
            ms.write(szSep, nSepSize);			
            Result++;
        }
        lpNamePtr++;
    }	
	ms.saveToFile(sFileName);
    return Result;
}

INT_PTR CNameAllocator::getName(LPCTSTR &lpNamePtr)
{
    //没有任何名字可供使用了,调用者必须先尝试调用repack重新整理名字库!
//    if (m_nAllocNameIndex <= 0)
//        return -1;

    //返回名字索引
//    INT_PTR nResult = m_nAllocNameIndex;
//    m_nAllocNameIndex--;
//    lpNamePtr = (*this)[nResult];
//    return nResult;

	//如果已经用完了列表的一轮,则看看是否还有未被使用的名字,有的话再继续一轮
	if (m_nAllocNameIndex < 0)
	{
		if (count() - m_nUsedCount <= 0) //无名字可用
		{
			return -1;
		}
		m_nAllocNameIndex = count() - 1; //从新滚动到底部
	}
	INT_PTR nResult = -1;
	while (true)
	{
		nResult = m_nAllocNameIndex;
		if ( !m_NameUseList[nResult] ) //此名字没被使用
		{
			lpNamePtr = (*this)[nResult];
			m_nAllocNameIndex--;
			break;
		}
		m_nAllocNameIndex--;
		if (m_nAllocNameIndex < 0)
		{
			if (count() - m_nUsedCount <= 0) //无名字可用
			{
				return -1;
			}
			m_nAllocNameIndex = count() - 1; //从新滚动到底部
		}
	}

	return nResult;
}

bool CNameAllocator::allocName(INT_PTR index)
{
    if (index >= 0 && index < count() && index > m_nAllocNameIndex)
    {
        m_NameUseList[index] = true;
		m_nUsedCount++; //增加已使用的名字计数
        return true;
    }
    return false;
}

INT_PTR CNameAllocator::repack()
{
    for (INT_PTR i=count()-1; i>-1; --i)
    {
        if (m_NameUseList[i])
        {
            remove(i);
        }
    }
    INT_PTR nCount = count();
    m_NameUseList.trunc(nCount);
    ZeroMemory((bool*)m_NameUseList, nCount);
    m_nAllocNameIndex = nCount-1;

    return nCount;
}
