#include "StdAfx.h"
#include "../interface/TestExportFun.h"

namespace TestSys
{
	void setLevel(void* sysarg, int nLevel)
	{
		CEntity* pEntity = (CEntity *)sysarg;
		if (!pEntity || pEntity->GetType() != enActor)
			return;

		((CActor *)pEntity)->SetLevel(nLevel);
	}

	int addItemById(void* sysarg, int nItemId, int nCount, int nQuality, int nStrong, bool bBind)
	{
		CEntity* pEntity = (CEntity *)sysarg;
		if (!pEntity || pEntity->GetType() != enActor)
			return 0;

		CUserItemContainer::ItemOPParam itemData;
		itemData.wItemId	= (WORD)nItemId;
		itemData.wCount	= (BYTE)nCount;
		itemData.btQuality	= (BYTE)nQuality;
		itemData.btStrong	= (BYTE)nStrong;		
		itemData.btBindFlag = (BYTE)bBind;		
		return (int)((CActor *)sysarg)->GetBagSystem().AddItem(itemData,
														 pEntity->GetEntityName(), 
														 (int)GameLog::clGMAddItem);
	}
}