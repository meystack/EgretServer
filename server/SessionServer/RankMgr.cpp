
//实现排行榜的一个文件

#include "StdAfx.h"

bool CRankMgr::AddRankItem(CRankItem &rankItem,int nRankItemCount)
{
	CRankItem * pItem;
	int nKey = rankItem.nKey;
	unsigned int nActorid= rankItem.nActorid;

	if(nKey <=0 || nActorid ==0) return false;

	//锁定，防治重复操作
	m_rankItems.trylock();

	for(int i=0;i < m_rankItems.count(); i++)
	{
		pItem =& m_rankItems[i]; 
		//if(pItem->nKey != nKey) continue;
		//如果自己在这个榜单上有个名次，就删除自己的名次
		//当前写死一下，如果职业排行榜的有同样的actorid过来，则删除掉掉以前的actorid
		if(pItem->nActorid == nActorid && pItem->nKey <=3)
		{
			m_rankItems.remove(i);
			break;
		}
	}
	
	INT_PTR nInsertPos =-1;

	int nAboveCount=0;  //超过这个玩家的角色数目
	
	for(INT_PTR i=0; i< m_rankItems.count(); i++)
	{
		pItem =& m_rankItems[i];
		if(pItem->nKey != nKey) continue;
		
		if(pItem->nValue < rankItem.nValue ) 
		{
			nInsertPos = i;
			break;
		}
		else if (pItem->nValue == rankItem.nValue)
		{
			if(pItem->lAuxValue1 < rankItem.lAuxValue1)
			{
				nInsertPos = i;
				break;
			}
			else if(pItem->lAuxValue1 == rankItem.lAuxValue1)
			{
				if(pItem->lAuxValue2 < rankItem.lAuxValue2)
				{
					nInsertPos = i;
					break;
				}
			}
		}
		nAboveCount ++;
	}

	//能够替换掉1个
	if(nInsertPos >=0)
	{
		m_rankItems[nInsertPos] = rankItem;
	}
	else
	{
		//如果还不够位置的话，就在后面添加1个
		if(nAboveCount <nRankItemCount )
		{
			m_rankItems.add(rankItem);
		}
	}
	m_rankItems.unlock(); //解锁一下

	return true;
	
}

