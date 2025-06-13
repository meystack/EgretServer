#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
	#include <tchar.h>
	#include <Windows.h>
#endif

#include <bzhash.h>
#include "../include/ObjectAllocator.hpp"
#include "../include/CustomHashTable.h"
#include "../include/LangTextSection.h"
#include <new>

static void* allocFromAllocator(CObjectAllocator<char> *pAllocator, void* p, size_t s)
{
	if (s > 0)
	{
		size_t *pBlock, *pOldBlock = NULL;
		if (p)
		{
			pOldBlock = (size_t*)p;
			pOldBlock--;
			//内存块长度足够则直接返回
			if (*pOldBlock >= s)
				return p;
		}
		pBlock = (size_t *)pAllocator->allocObjects(s + sizeof(*pBlock));
		*pBlock = s;
		if (pOldBlock)
		{
			memcpy(pBlock + 1, pOldBlock + 1, *pOldBlock);
			//CObjectAllocator<char> 没有提供释放内存的功能
		}
		return pBlock + 1;
	}
	//CObjectAllocator<char> 没有提供释放内存的功能
	return NULL;
}

CLangTextSection::CLangTextSection()
{
	m_pAllocator = NULL;
	m_nType = eInvalid;
	m_Data.m_pStr = NULL;
}

CLangTextSection::CLangTextSection(CObjectAllocator<char> *pAllocator)
{
	m_pAllocator = pAllocator;
	m_nType = eInvalid;
	m_Data.m_pStr = NULL;
}

CLangTextSection::~CLangTextSection()
{
	clear();
}

CLangTextSection* CLangTextSection::getSection(LPCTSTR sName)
{
	if (m_nType == ePacket)
	{
		return m_Data.m_pPack->get(sName);
	}
	return NULL;
}

void CLangTextSection::setText(LPCTSTR str)
{
	clear();
	m_nType = eText;
	size_t len = sizeof(*str) *(strlen(str) + 1);
	m_Data.m_pStr = (LPTSTR)realloc(NULL, len);
	memcpy(m_Data.m_pStr, str, len);
}

void CLangTextSection::setPacket(size_t len)
{
	clear();
	m_Data.m_pPack = (CLangTextPacket*)realloc(NULL, sizeof(*m_Data.m_pPack));
	new(m_Data.m_pPack)CLangTextPacket(m_pAllocator, len);
	m_nType = ePacket;
}

CLangTextSection* CLangTextSection::addSection(LPCTSTR sName)
{
	if (m_nType != ePacket)
		return NULL;

	CLangTextSection* pSection = m_Data.m_pPack->get(sName);
	if (!pSection)
	{
		pSection = m_Data.m_pPack->put(sName);
		pSection->m_pAllocator = m_pAllocator;
	}
	return pSection;
}

void CLangTextSection::clear()
{
	if (m_nType == eText)
	{
		realloc(m_Data.m_pStr, 0);
		m_Data.m_pStr = NULL;
	}
	else
	{
		if (m_Data.m_pPack)
		{
			m_Data.m_pPack->~CLangTextPacket();
			realloc(m_Data.m_pPack, 0);
			m_Data.m_pPack = NULL;
		}
	}
	m_nType = eInvalid;
}

void* CLangTextSection::realloc(void *p, size_t s)
{
	return allocFromAllocator(m_pAllocator, p, s);
}


CLangTextPacket::CLangTextPacket(CObjectAllocator<char> *pAllocator, size_t len)
	:Inherited(len)
{
	m_pAllocator = pAllocator;
}

CLangTextPacket::~CLangTextPacket()
{
	clear();
}

void* CLangTextPacket::realloc(void* p, size_t s)
{
	return allocFromAllocator(m_pAllocator, p, s);
}
