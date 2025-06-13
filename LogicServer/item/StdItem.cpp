#include "StdAfx.h"
#include "StdItem.h"

CAttrCalc& CStdItem::CalcAttributes(CAttrCalc &calc, const INT_PTR nQuality, const INT_PTR nStrong, const CUserItem* pUserItem, CAnimal* pMaster) const
{
	INT_PTR i;
	const AttributeGroup *pAttrGroup;

	//写入静态属性
	if ( m_StaticAttrs.nCount > 0 )
	{
		for (i=m_StaticAttrs.nCount-1; i>-1; --i)
		{
			calc << m_StaticAttrs.pAttrs[i];
			
		}
	}
	//写入品质属性
	DbgAssert(nQuality >= 0 && nQuality <= MaxItemQuality);
	if(nQuality >0 && nQuality < MaxItemQuality)
	{
		pAttrGroup = &m_QualityAttrs[nQuality -1];
		if ( pAttrGroup->nCount > 0 )
		{
			for (i=pAttrGroup->nCount-1; i>-1; --i)
			{
				calc << pAttrGroup->pAttrs[i];
			}
		}
	}

	
	//写入强化属性**没有用了
	//DbgAssert(nStrong >= 0 && nStrong <= MaxItemStrong);
	
	return calc;
}

/*
角色道具根据配置进行实例化
*/
void CStdItem::AssignInstance(CUserItem *pUserItem, const CStdItem * pItem)
{
}
