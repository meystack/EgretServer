#include "StdAfx.h"
#include "../interface/RankingExport.h"


namespace Ranking
{
	void* getRanking( int rankId )
	{
		return GetGlobalLogicEngine()->GetRankingMgr().GetRanking(rankId);
	}

	void* add( int rankId,char* sName,int nMax,int boDisplay, int nBroadCount)
	{
		return GetGlobalLogicEngine()->GetRankingMgr().Add(rankId,sName,nMax,boDisplay, nBroadCount);
	}

	bool load( void* pRank,char* sFile )
	{
		if (pRank)
		{
			return ((CRanking*)pRank)->Load(sFile);
		}

		return false;
	}

	bool save( void* pRank,char* sFile,bool bForceSave)
	{
		if (pRank)
		{
			((CRanking*)pRank)->Save(sFile, bForceSave);
			return true;
		}

		return false;
	}

	void addColumn( void* pRank,char* sTitle, int nIndex /*= -1*/, bool bUnique )
	{
		if (pRank)
		{
			((CRanking*)pRank)->AddColumn(sTitle,nIndex,bUnique);
		}
	}

	void setColumnTitle(void* pRank, char* sTitle, int nIndex)
	{
		if (pRank)
		{
			((CRanking*)pRank)->SetColumnTitle(sTitle, nIndex);
		}
	}

	void* addItem( void* pRank,unsigned int nId, int nPoint, bool bRankFlag)
	{
		if (pRank)
		{
			return ((CRanking*)pRank)->AddItem(nId,nPoint,bRankFlag);
		}
		return NULL;
	}

	int getItemIndexFromId( void* pRank,unsigned int nId )
	{
		if (pRank)
		{
			return (int)(((CRanking*)pRank)->GetIndexFromId(nId));
		}
		return -1;
	}

	void* getItemPtrFromId( void* pRank,unsigned int nId )
	{
		if (pRank)
		{
			return ((CRanking*)pRank)->GetPtrFromId(nId);
		}
		return NULL;
	}

	void* updateItem( void* pRank,unsigned int nId,int nPoint )
	{
		if (pRank)
		{
			return ((CRanking*)pRank)->Update(nId,nPoint);
		}
		return NULL;
	}

	void* setItem( void* pRank,unsigned int nId,int nPoint )
	{
		if (pRank)
		{
			return ((CRanking*)pRank)->Set(nId,nPoint);
		}
		return NULL;
	}

	void setSub( void* pRankItem,int nIndex, char* sData )
	{
		if (pRankItem)
		{
			return ((CRankingItem*)pRankItem)->SetSub(nIndex,sData);
		}
	}

	void addRef( void* pRank )
	{
		if (pRank)
		{
			((CRanking*)pRank)->addRef();
		}
	}

	void release( void* pRank )
	{
		if (pRank)
		{
			((CRanking*)pRank)->release();
		}
	}

	int getRef( void* pRank )
	{
		if (pRank)
		{
			return ((CRanking*)pRank)->getRef();
		}
		return 0;
	}

	void removeRanking( int rankId )
	{
		GetGlobalLogicEngine()->GetRankingMgr().Remove(rankId);
	}

	int getRankItemCount( void* pRank )
	{
		if (pRank)
		{
			return (int)(((CRanking*)pRank)->GetList().count());
		}
		return 0;
	}

	int getRankItemCountByLimit(void* pRank, int nLimit)
	{
		if (pRank)
		{
			return GetGlobalLogicEngine()->GetMiscMgr().GetRankLimitCount((CRanking*)pRank, nLimit);
		}
		return 0;
	}

	int getPoint( void* pRankItem )
	{
		if (pRankItem)
		{
			return ((CRankingItem*)pRankItem)->GetPoint();
		}
		return 0;
	}

	unsigned int getId( void* pRankItem )
	{
		if (pRankItem)
		{
			return ((CRankingItem*)pRankItem)->GetId();
		}
		return 0;
	}

	const char* getSub( void* pRankItem,int nIndex )
	{
		if (pRankItem)
		{
			const char* p = ((CRankingItem*)pRankItem)->GetSubData(nIndex);
			return p;
		}
		return "-";
	}

	void clearRanking( void* pRank )
	{
		if (pRank)
		{
			return ((CRanking*)pRank)->clear();
		}
	}

	void* getItemFromIndex( void* pRank,int nIndex )
	{
		if (pRank)
		{
			/*
			CVector<CRankingItem*>& itemList = ((CRanking*)pRank)->GetList();
			if (nIndex >= 0 && nIndex < itemList.count())
			{
				return itemList[nIndex];
			}
			*/
			return (void *)(((CRanking*)pRank)->GetItem(nIndex));
		}
		return NULL;
	}

	void setColumnDisplay( void* pRank, int nIndex ,int boDisplay )
	{
		if (pRank)
		{
			((CRanking*)pRank)->SetColDisplay(nIndex, boDisplay > 0 );
		}
	}

	int getIndexFromPtr( void* pRankItem )
	{
		if (pRankItem)
		{
			return ((CRankingItem*)pRankItem)->GetIndex();
		}
		return -100000;//
	}

	void setDisplayName( void* pRank, const char* sName )
	{
		if (pRank)
		{
			((CRanking*)pRank)->SetDisplayName(sName);
		}
	}

	void setIdTitle( void* pRank,const char* sName )
	{
		if (pRank)
		{
			((CRanking*)pRank)->SetIdTitle(sName);
		}
	}

	void setPointTitle( void* pRank, const char* sName )
	{
		if (pRank)
		{
			((CRanking*)pRank)->SetPointTitle(sName);
		}
	}

	void removeId( void* pRank, unsigned int id )
	{
		if (pRank)
		{
			((CRanking*)pRank)->RemoveId(id);
		}
	}

	void UpdateCsRank(int rankId)
	{
		GetGlobalLogicEngine()->GetRankingMgr().NotifyRankUpdateCs(rankId);
	}

	void setRankPropConfig(void* pRank, int nRankPropIndex)
	{
		if (pRank)
		{
			((CRanking*)pRank)->SetRankPropConfig(nRankPropIndex);
		}
	}

	int GetRankColumnCount(void* pRank){
		int count = 0;
		if (pRank)
		{
			count = ((CRanking*)pRank)->ColumnCount();
		}
		return (int)count;
	}

	void forceSave(void* pRank,char* sFile)
	{
		if (pRank)
		{
			((CRanking*)pRank)->ForceSave(sFile);
		}

	}

	bool CheckActorIdInRank(void* pRank,unsigned int id)
	{
		if (pRank)
		{
			return ((CRanking*)pRank)->CheckActorIdInRank(id);
		}
		return false;
	}
}
