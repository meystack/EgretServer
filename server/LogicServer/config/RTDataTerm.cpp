#include "StdAfx.h"
#include "LogicDataProvider.h"


//定义CLogicDataProvider的加载项列表
const CLogicDataProvider::RunTimeDataTerm CLogicDataProvider::RunTimeDataTermList[] = 
{
	//{ TRUE, _T("GameStore Rank"), _T("./data/runtime/store/rank.rtd"), &CLogicDataProvider::LoadGameStoreRank, &CLogicDataProvider::SaveStoreDynamicData  },
	//{ TRUE, _T("Script BootCall"), _T("./data/runtime/script/bootcall.rtd"), &CLogicDataProvider::LoadScriptBootCall, &CLogicDataProvider::SaveScriptBootCall  },

	{ TRUE, _T("GameStore_refresh"), _T("./data/runtime/store/dynamic.rtd"), &CLogicDataProvider::LoadStoreDynamicData, &CLogicDataProvider::SaveStoreDynamicData  },

	// { TRUE, _T("Hard Boss info"), _T("./data/runtime/store/bosshard.rtd"), &CLogicDataProvider::LoadBossGrowData, &CLogicDataProvider::SaveBossGrowData  },

};

const INT_PTR CLogicDataProvider::RunTimeDataTermCount = ArrayCount(CLogicDataProvider::RunTimeDataTermList);