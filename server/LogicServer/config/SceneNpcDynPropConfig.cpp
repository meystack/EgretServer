#include "StdAfx.h"
#include "../base/Container.hpp"

bool CLogicDataProvider::LoadSceneNpcDynPropConfig(LPCTSTR sFilePath)
{
	SceneNpcDynPropConfig &cfg = GetLogicServer()->GetDataProvider()->GetNpcDynPropConfig();
	if (!openGlobalTable("DynPropConfig"))
	{
		OutputMsg(rmError, _T("装置副本NPC动态属性配置表失败，找不到全局表DynPropConfig"));
		return false;
	}

	if (enumTableFirst())
	{
		INT_PTR nIndex = 0;
		INT_PTR nCount = 0;
		do 
		{
			nCount = lua_objlen(m_pLua, -1);
			/*if (nCount > NPC_DYN_PROP_COUNT)
			{
				OutputMsg(rmWaning, _T("装置副本NPC动态属性配置表数据警告，等级[%d]属性配置过多(max=%d)"), 
									(nIndex+1), NPC_DYN_PROP_COUNT);
			}*/

			NpcDynPropListRate dynPropListRate;
			if (nCount <= 0)
			{				
				for (int i = 0; i < NPC_DYN_PROP_COUNT; i++)
				{
					dynPropListRate.fRate[i] = 0.0f;						
				}
			}
			else
			{
				if (enumTableFirst())
				{					
					CVector<float> vecList;
					do {						
						vecList.push((float)getFieldNumber(NULL));
					}while(enumTableNext());
													
					nCount = __min(nCount, NPC_DYN_PROP_COUNT);
					for (INT_PTR i = 0; i < nCount; i++)
					{
						dynPropListRate.fRate[i] = vecList.get(i);
					}
					for (INT_PTR i = nCount; i < NPC_DYN_PROP_COUNT; i++)
					{
						dynPropListRate.fRate[i] = 0.0f;						
					}
				}
			}
			cfg.vecDynPropRate.push(dynPropListRate);
		} while (enumTableNext());
	}

	closeTable();


	// 读取队伍人数对于副本怪物属性的影响系数
	if (!openGlobalTable("TeamNumAdjustRate"))
	{
		OutputMsg(rmError, _T("装置副本NPC动态属性配置表失败，找不到全局表TeamNumAdjustRate"));
		return false;
	}
		
	INT_PTR nCount = lua_objlen(m_pLua, -1);
	if (nCount > 0)
	{
		if (enumTableFirst())
		{		
			INT_PTR nIdx = 0;
			do 
			{
				NpcDynPropListRate adjustInfo; // 队伍人数调整配置信息				
				INT_PTR nPropCount = lua_objlen(m_pLua, -1);	
				if (nPropCount > 0)
				{
					if (enumTableFirst())
					{	
						CVector<float> vecList;
						do {						
							vecList.push((float)getFieldNumber(NULL));
						}while(enumTableNext());

						nPropCount = __min(nPropCount, NPC_DYN_PROP_COUNT);
						for (int i = 0; i < nPropCount; i++)
						{
							adjustInfo.fRate[i] = vecList.get(i);
						}
						for (int i = (int)nPropCount; i < NPC_DYN_PROP_COUNT; i++)
						{
							adjustInfo.fRate[i] = 0.0f;						
						}
					}
				}
				else
				{
					for (int i = 0; i < NPC_DYN_PROP_COUNT; i++)
					{
						adjustInfo.fRate[i] = 0.0f;						
					}
				}				
				if (nIdx < MAX_TEAM_MEMBER_COUNT)
					cfg.teamNumAdjustRateInfo[nIdx++] = adjustInfo;
				
			}while(enumTableNext());
		}
	}
	
	closeTable();
	return true;
}

