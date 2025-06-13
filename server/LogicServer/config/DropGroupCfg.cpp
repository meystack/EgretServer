
#include "StdAfx.h"
DropGroupCfg::DropGroupCfg()
{

}

DropGroupCfg::~DropGroupCfg()
{

}


bool DropGroupCfg::LoadDropGroupsConfig( LPCTSTR sFilePath )
{
	bool boResult = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;

	try
	{
		//从文件加载配置脚本
		if ( ms.loadFromFile(sFilePath) <= 0 )
			showErrorFormat(_T("%s unable to load from %s"), __FUNCTION__, sFilePath);
		//对配置脚本进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
		//设置脚本内容
		if ( !setScript(sText) )
			showErrorFormat(_T("%s syntax error on Load config:%s"), __FUNCTION__, sFilePath);
		//读取标准物品配置数据
		boResult = ReadDropGroupsConfig();
		ReadDropInfosConfig();
		ReadDropAttributeConfig();

	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load %s config error: %s"),sFilePath, s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load %s config"), sFilePath);
	}
	//销毁脚本虚拟机
	setScript(NULL);

	return boResult;
}

bool DropGroupCfg::ReadDropGroupsConfig()
{
	if (!openGlobalTable("DropGroupConfig"))
	{
		return false;
	}
	// size_t nCount = lua_objlen(m_pLua, -1);
    int nDefValue = 0;
    m_GiftDropGroupMaps.clear();
	if (enumTableFirst())
	{		
		do 
		{
			int nId = getFieldInt("id");
            GIFTDROPGROUP group;
            group.nType =  (WORD)getFieldInt("type");
            group.vGiftDropGroups.clear();
			group.maxRate = ReadTableConfigByTable(nId,"group", group.vGiftDropGroups);
            m_GiftDropGroupMaps.insert(std::make_pair(nId, group));
		} while (enumTableNext());
	}
	closeTable();
	return true;
}

int DropGroupCfg::ReadTableConfigByTable(int nDropid, LPCSTR sTableName, std::vector<DROPINFO>& vInfos)
{
	int nDefValue = 0;
	int maxRate = 0;
	if(feildTableExists(sTableName) && openFieldTable(sTableName))
	{
		size_t nCount = lua_objlen(m_pLua, -1);
		// if (nCount <= 0)
		// {
		// 	closeTable();
		// 	return maxRate;
		// }
		if(enumTableFirst())
		{
			do
			{
				DROPINFO info;
				info.nRate = getFieldInt("rate", &nDefValue);
				info.nId = getFieldInt("id", &nDefValue);
				info.nCount = getFieldInt("count", &nDefValue);
				info.nType = getFieldInt("type", &nDefValue);
				info.bNoDrop = getFieldInt("nodrop", &nDefValue);
				info.nDropId = nDropid;
				maxRate += info.nRate;
				vInfos.push_back(info);

			} while (enumTableNext());
		}
		closeTable();

	}
	return maxRate;
}

bool DropGroupCfg::ReadDropInfosConfig()
{
	if (!openGlobalTable("DropTableConfig"))
	{
		return false;
	}
	// size_t nCount = lua_objlen(m_pLua, -1);
    int nDefValue = 0;
    m_GiftDropTableMaps.clear();
	if (enumTableFirst())
	{		
		do 
		{
			int nId = getFieldInt("id");
            GIFTDROPTABLE group;
            group.nType =  (WORD)getFieldInt("type", &nDefValue);
			group.isDropbest = getFieldInt("isDropbest",&nDefValue);
			group.nLimitServerOpenDay = getFieldInt("limitopenserverDay",&nDefValue);
			group.nCloseOpenDay = getFieldInt("closeopenday",&nDefValue);
            group.vGiftDropTables.clear();
			group.maxRate = ReadTableConfigByTable(nId,"table", group.vGiftDropTables);
			m_GiftDropTableMaps[nId] = group;
			char time[50];
			memset(&time, 0, sizeof(time));
			getFieldStringBuffer("timeLimit",time, sizeof(time));
			int nDefV = 0;
			group.nLootType =  (WORD)getFieldInt("loottype", &nDefV);
			for(int i = 0; i < sizeof(time); i++)
			{
				if(time[i] == '*')
					time[i] = '0';
			}
			sscanf(time, "%d-%d-%d %d:%d ^ %d-%d-%d %d:%d", &group.startTime.wYear, &group.startTime.wMonth,&group.startTime.wDay,&group.startTime.wHour,&group.startTime.wMinute,
			&group.endTime.wYear, &group.endTime.wMonth, &group.endTime.wDay,&group.endTime.wHour,&group.endTime.wMinute);
			//group.timmLimit = time;
			if(feildTableExists("weeklimit") && openFieldTable("weeklimit"))
            {

                // size_t nCount = lua_objlen(m_pLua, -1);
                // if (nCount <= 0)
                // {
                //     closeTable();
                //     break;
                // }
                if(enumTableFirst())
                {
                    do
                    {
						int week = getFieldInt(NULL);
                    	group.vWeekLimt.push_back(week);

                    } while (enumTableNext());
                    
                }
				closeTable();
            
            }
            m_GiftDropTableMaps[nId] = group;
		} while (enumTableNext());
	}
	closeTable();
	return true;
}


bool DropGroupCfg::ReadDropAttributeConfig()
{
	if (!openGlobalTable("dropAttributeConfig"))
	{
		return false;
	}
	// size_t nCount = lua_objlen(m_pLua, -1);
	// if (nCount <= 0)
	// {
	// 	return false;
	// }
    int nDefValue = 0;
    m_DropAttributeMaps.clear();
	if (enumTableFirst())
	{		
		do 
		{
			int nId = getFieldInt("type",&nDefValue);
            DROPATTRIBUTECFG group;
            group.nRate =  (WORD)getFieldInt("rate",&nDefValue);
            group.m_vAttribute.clear();
			if(feildTableExists("drop") && openFieldTable("drop"))
			{
				if(enumTableFirst())
				{
					do
					{
						DROPATTRIBUTE info;
						info.nRate = getFieldInt("rate", &nDefValue);
						info.nType = getFieldInt("type", &nDefValue);
						info.nValue = getFieldInt("value", &nDefValue);
						info.nFollowrate = getFieldInt("followrate", &nDefValue);
						info.nFollowcount = getFieldInt("followcount", &nDefValue);
						group.m_vAttribute.push_back(info);

					} while (enumTableNext());

					closeTable();
				}

			}

			m_DropAttributeMaps[nId] = group;
		} while (enumTableNext());
	}
	closeTable();
	return true;
}

void DropGroupCfg::getGiftDropGroup(int dropGroupid, std::vector<int>& result)
{
	std::map<int, GIFTDROPGROUP>::iterator it = m_GiftDropGroupMaps.find(dropGroupid);
	if(it != m_GiftDropGroupMaps.end())
	{
		GIFTDROPGROUP& info = it->second;
		int maxRand = 100; // nType：1 为随机
		if(info.nType == 2) //权重
		{
			maxRand = info.maxRate;
		}

		//std::vector<DROPINFO>::iterator pInfo = info.vGiftDropGroups.begin();
		
		int count = info.vGiftDropGroups.size();
		int nRate = 0;
		int rand = wrand(maxRand);
		for(int i = 0; i < count; i++)
		{
			if(info.nType == 1)
			{
				nRate = info.vGiftDropGroups[i].nRate;
				rand = wrand(maxRand);

			}else
			{
				nRate += info.vGiftDropGroups[i].nRate;
			}
			if(nRate >= rand)
			{
				result.push_back(info.vGiftDropGroups[i].nId);
				if (info.nType == 2)
					return;
			}
		}
		
	}

}
void DropGroupCfg::GetGiftDropInfoByDropGroupId(int dropGroupid, std::vector<DROPGOODS>& result)
{
	std::vector<int> dropIds;
	dropIds.clear();
	// void * p = (void *)::malloc(sizeof(int)*100);
	// dropIds.reserve(sizeof(int)*100);
	getGiftDropGroup(dropGroupid, dropIds);
	int count = dropIds.size();
	if(count <= 0)
	{
		//OutputMsg(rmError, _T("GetGiftDropInfoByDropGroupId size is zero dropGroupid:%d"),dropGroupid);
		return;
	}
	for(int i= 0; i < count; i++)
	{
		GetGiftDropInfoByDropId(dropIds[i], result);
	}
}

void DropGroupCfg::GetGiftDropInfoByDropId(int dropid, std::vector<DROPGOODS>& result)
{
	std::map<int, GIFTDROPTABLE>::iterator it = m_GiftDropTableMaps.find(dropid);
	if(it != m_GiftDropTableMaps.end())
	{
		
		GIFTDROPTABLE& info = it->second;
		if(!CheckTimeLimit(info))
		{
			return;
		}
		int maxRand = 100; // nType：1 为随机
		if(info.nType == 2) //权重
		{
			maxRand = info.maxRate;
		}

		//std::vector<DROPINFO>::iterator pInfo = info.vGiftDropTables.begin();
		int count = info.vGiftDropTables.size();
		int nRate = 0;//
		int rand = wrand(maxRand);
		for(int i = 0; i < count; i++)
		{
			if(info.nType == 1)
			{
				nRate = info.vGiftDropTables[i].nRate;
				rand = wrand(maxRand);
			}
			else
			{
				nRate += info.vGiftDropTables[i].nRate;
			}

			if(rand <= nRate)
			{
				DROPGOODS item;
				item.info = info.vGiftDropTables[i];
				if(!item.info.bNoDrop) {
					
					if(item.info.nType == 0 && info.isDropbest)
					{
						const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(item.info.nId);
						if(pStdItem &&  Item::itUndefinedType < pStdItem->m_btType < Item::itEquipMax && pStdItem->nJpdrop)
						{
							GetDropAttribute(pStdItem->m_btType, item);
						}
					}
					result.push_back(item);
				}
				if(info.nType == 2)
					return;
			}

		}	
	}
}

GIFTDROPTABLE* DropGroupCfg::GetDropTableConfig(int droptableid)
{
	std::map<int, GIFTDROPTABLE>::iterator droptableCfg = m_GiftDropTableMaps.find(droptableid);
	if(droptableCfg != m_GiftDropTableMaps.end())
	{
		return &(droptableCfg->second);
	}
	return NULL;
}
void DropGroupCfg::GetDropAttribute(int type, DROPGOODS& item)
{
	std::map<int, DROPATTRIBUTECFG>::iterator dropAttrCfg = m_DropAttributeMaps.find(type);
	if(dropAttrCfg != m_DropAttributeMaps.end())
	{
		DROPATTRIBUTECFG cfg = dropAttrCfg->second;
		int rand = wrand(100);
		if(rand <= cfg.nRate)
		{
			int count = cfg.m_vAttribute.size();
			for(int i = 0; i < count; i++)
			{
				DropAttribute info = cfg.m_vAttribute[i];
				int rand_ = wrand(100);
				// printf("GetDropAttribute: %d, %d\n",rand_,info.nRate);
				if(rand_ <= info.nRate)
				{
					// printf("GetDropAttribute: %d, %d\n",rand_,info.nRate);
					item.AddDropItemAttr(info);
					if(info.nFollowcount)
					{
						for(int j = 0; j < info.nFollowcount; j++)
						{
							int rand = wrand(100);
							if(rand <= info.nFollowrate)
							{
								item.AddDropItemAttr(info);
								continue;
							}
							break;
						}
					}
				}

				// if(info.nFollowcount)
				// {
				// 	for(int j = 0; j < info.nFollowcount; j++)
				// 	{
				// 		int rand = wrand(100);
				// 		if(rand <= info.nFollowrate)
				// 		{
				// 			item.AddDropItemAttr(info);
				// 		}
				// 	}
				// }
			} 
		}
	}
}

bool DropGroupCfg::CheckWeekLimit(GiftDropTable& dropTable, int week)
{
	if(dropTable.vWeekLimt.size() <= 0)
		return true;
	
	int count = dropTable.vWeekLimt.size();
	for(int i= 0 ; i< count; i++)
	{
		if(dropTable.vWeekLimt[i] == week)
		{
			return true;
		}
	}
	return false;
}

bool DropGroupCfg::CheckTimeLimit(GiftDropTable& dropTable)
{
	/*
	* 表示不限制
	[]是星期几，不填或[*] 或 []表示不限
	| 表示确定的两个时间点之间内
	*/
	SYSTEMTIME nowTime;
	GetLocalTime(&nowTime);
	int nDays = GetLogicServer()->GetDaysSinceOpenServer();
	if( CheckLimit(dropTable.startTime.wYear, dropTable.endTime.wYear, nowTime.wYear) && 
	CheckLimit(dropTable.startTime.wMonth, dropTable.endTime.wMonth, nowTime.wMonth) &&
	CheckLimit(dropTable.startTime.wDay, dropTable.endTime.wDay, nowTime.wDay) &&
	CheckLimit(dropTable.startTime.wHour, dropTable.endTime.wHour, nowTime.wHour) &&
	CheckLimit(dropTable.startTime.wMinute, dropTable.endTime.wMinute, nowTime.wMinute) &&
	CheckWeekLimit(dropTable, nowTime.wDayOfWeek) && nDays >= dropTable.nLimitServerOpenDay &&(!dropTable.nCloseOpenDay || nDays < dropTable.nCloseOpenDay)) 
	{
		return true;
	}


	return false;

}

bool DropGroupCfg::CheckLimit(int start, int end, int now)
{
	if(start != 0 && end != 0 && (now < start || now > end))
	{
		return false;
	}
	else if((start != 0 && end == 0 && now < start) || (start == 0 && end != 0 && now > end)) //start > end? 不太可能
	{
		return false;
	}else
	{
		return true;
	}

	return true;
}


void DropGroupCfg::getDropItemOPParam(int nDropGroupId, std::vector<CUserItem>& dropItems)
{
	std::vector<DROPGOODS> dropInfos;
	GetGiftDropInfoByDropGroupId(nDropGroupId, dropInfos);
	int count  = dropInfos.size();
	if(count <= 0)
	{
		return;
	}
	else
	{
		for(int i = 0; i < count; i++)
		{
			CUserItem iParam;
			iParam.wItemId = dropInfos[i].info.nId;
			iParam.wCount = (WORD)dropInfos[i].info.nCount;
			iParam.btQuality =1;
			iParam.btStrong = 0;
			iParam.nType = dropInfos[i].info.nType ;
			if(dropInfos[i].info.nType  == 0)
			{
				const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(dropInfos[i].info.nId);
				if(!pStdItem)
					continue;
				iParam.btQuality = pStdItem->b_showQuality;
				int num = dropInfos[i].v_bestAttr.size();

				for(int j = 0; j < num; j++)
				{
					char buf[10];
					if(j != 0)
						strcat(iParam.cBestAttr, "|");

					DropAttribute data = dropInfos[i].v_bestAttr[j];
					sprintf(buf, "%d,%d", data.nType, data.nValue);
					strcat(iParam.cBestAttr, buf);
				}
				iParam.cBestAttr[sizeof(iParam.cBestAttr)-1]= '\0';	
			}
			dropItems.push_back(iParam);
		}
	}
}
