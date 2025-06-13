#include "StdAfx.h"
#include "StdItemConfigCacher.h"

const FILEIDENT CStdItemConfigCacher::CacheFileIdent = { MAKEFOURCC('S', 'I', 'C', 0) };
const FILEVERSION CStdItemConfigCacher::CacheVersion = { MAKEFOURCC(1, 11, 2, 17) };

bool CStdItemConfigCacher::ValidateHeader(const CONF_DATA_CACHE_HDR& hdr)
{
	//验证文件标识
	if (hdr.Ident.uIdent != CacheFileIdent.uIdent)
		return false;
	//验证文件版本
	if (hdr.Version.uVersion != CacheVersion.uVersion)
		return false;
	//验证StdItem结构大小
	if (sizeof(CStdItem) != hdr.dwSizeElement)
		return false;
	return true;
}

bool CStdItemConfigCacher::AdjustElementPointers(LPVOID lpElement)
{
	INT_PTR i;
	CStdItem *pStdItem = (CStdItem*)lpElement;
#define AdjustPtr(ptr, T) if (ptr && !(ptr = (T)GetNewMemoryPtr(ptr))) return false;
#define AdjustPtrArray(a, name, n, T) for (i=n-1; i>-1; --i) if ( (a)[i].name && !((a)[i].name = (T)GetNewMemoryPtr((a)[i].name)) ) return false;

	AdjustPtr(pStdItem->m_StaticAttrs.pAttrs, PGAMEATTR);
	AdjustPtrArray(pStdItem->m_QualityAttrs, pAttrs, ArrayCount(pStdItem->m_QualityAttrs), PGAMEATTR);
	AdjustPtrArray(pStdItem->m_StrongAttrs, pAttrs, ArrayCount(pStdItem->m_StrongAttrs), PGAMEATTR);
	//AdjustPtr(pStdItem->m_UnitedAttrs.pGroups, CStdItem::UnitedAttributeGroup*);
	//AdjustPtrArray(pStdItem->m_UnitedAttrs.pGroups, pUnitedItemIds, pStdItem->m_UnitedAttrs.nCount, PWORD);
	//AdjustPtr(pStdItem->m_SmithAttrs.pAttrs, PRANDOMATTRTERM);
	AdjustPtr(pStdItem->m_Conditions.pConds, CStdItem::ItemUseCondition*);
	AdjustPtr(pStdItem->m_existScenes.pSceneId, int*);

#undef AdjustPtr
#undef AdjustPtrArray
	return true;
}

void CStdItemConfigCacher::FillHeaderData(CONF_DATA_CACHE_HDR& hdr)
{
	hdr.Ident.uIdent = CacheFileIdent.uIdent;
	hdr.Version.uVersion = CacheVersion.uVersion;
	hdr.dwSizeElement = sizeof(CStdItem);
}