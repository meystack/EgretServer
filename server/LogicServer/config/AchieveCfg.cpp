#include "StdAfx.h"
#include "AchieveCfg.h"
#include "../base/Container.hpp"
CAchieve::CAchieve()
	:Inherited(),m_NewDataAllocator(_T("AchieveDataAlloc"))
{
}

CAchieve::~CAchieve()
{
}

bool CAchieve::LoadConfig(LPCTSTR sFilePath)
{
	bool Result = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;
	try
	{
		//从文件加载配置脚本
		if ( ms.loadFromFile(sFilePath) <= 0 )
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		//对配置脚本进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
		//设置脚本内容
		if ( !setScript(sText) )
		{
			showError(_T("syntax error on RideGame config"));
			FILE* fp = fopen("scripterror.txt", "wb");
			if (fp)
			{
				fputs(sText, fp);
				fclose(fp);
			}
		}
		//读取标准物品配置数据
		Result = readAchieveTaskConfigs();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load CAchieve config error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load CAchieve config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

bool CAchieve::readConfigs()
{
	return true;
}



bool CAchieve::readMedalConfigs()
{
	if ( !openGlobalTable("MedalConfig") )
		return FALSE;
	m_MedalMap.clear();
	int nDefault = 0;
	if(enumTableFirst())
	{
		do
		{
			
			ELEM_MEDALCFG tmp;
			tmp.level = getFieldInt("level", &nDefault);
			tmp.need_popularity = getFieldInt("popularity", &nDefault);
			tmp.need_level = getFieldInt("nextlevel", &nDefault);
			tmp.need_openday = getFieldInt("openday", &nDefault);
			tmp.need_circle = getFieldInt("circle", &nDefault);
			tmp.now_medal_id = getFieldInt("medal", &nDefault);
			tmp.next_medal_id = getFieldInt("nextmedal", &nDefault);

			
			if (feildTableExists("achievementid") && openFieldTable("achievementid"))
			{
				if(enumTableFirst())
				{
					do
					{
						tmp.need_achieve_id_vec.push_back(getFieldInt(NULL)) ;
						
					} while (enumTableNext());
				}
				closeTable();
			}

			if (feildTableExists("needitem") && openFieldTable("needitem"))
			{
				if(enumTableFirst())
				{
					do
					{
						GOODS_TAG local_tmp ;
						local_tmp.type = getFieldInt("type", &nDefault) ;
						local_tmp.id = getFieldInt("id", &nDefault) ;
						local_tmp.count = getFieldInt("count", &nDefault) ;

						tmp.need_goods_vec.push_back( local_tmp) ;
						
					} while (enumTableNext());
				}
				closeTable();
			}
			
			m_MedalMap[tmp.level] = tmp;
	
		}while(enumTableNext());
	}
	closeTable();
	return true;
}




bool CAchieve::readAchieveTypeConfigs()
{
	if ( !openGlobalTable("AchieveTypeConfig") )
		return FALSE;
	m_nAchieveTypes.clear();
	int nDefault = 0;
	if(enumTableFirst())
	{
		do
		{
			if(enumTableFirst())
			{
				do
				{
					if(enumTableFirst())
					{
						do
						{

							ACHIEVETYPECFG cfg;
							cfg.nType = getFieldInt("type", &nDefault);
							cfg.nSubType = getFieldInt("Subtype", &nDefault);
							cfg.nValueType = getFieldInt("ValueType", &nDefault);
							cfg.nLog = getFieldInt("log", &nDefault);
							cfg.nWay = getFieldInt("getway", &nDefault);
							if (feildTableExists("activityid") && openFieldTable("activityid"))
							{
								if(enumTableFirst())
								{
									do
									{
										int id = getFieldInt(NULL) ;
										cfg.activityId.push_back(id) ;
										
									} while (enumTableNext());
								}
								closeTable();
							}
							int nType = cfg.nType*1000 + cfg.nSubType ; //处理 成就类型
							m_nAchieveTypes[nType][cfg.nWay] = cfg;
						}while(enumTableNext());
					}
				}while(enumTableNext());
			}
		}while(enumTableNext());
	}
	closeTable();
	return readMedalConfigs();
}

ACHIEVETASKCONFIG* CAchieve::GetAchieveTask(int nTask)
{
	std::map<int, ACHIEVETASKCONFIG>::iterator it = m_AchieveTasks.find(nTask);
	if(it != m_AchieveTasks.end())
	{
		return &(it->second);
	}
	return NULL;
}




bool CAchieve::readAchieveTaskConfigs()
{
	if ( !openGlobalTable("AchieveConfig") )
		return FALSE;

	int nDefault=0;
	if ( enumTableFirst() )
	{
		do 
		{
			if(enumTableFirst())
			{
				do
				{
					ACHIEVETASKCONFIG task;
					task.nId = getFieldInt("id",&nDefault);
					task.nTab = getFieldInt("page",&nDefault);
					task.nParameter1 = getFieldInt("parameter1",&nDefault);
					task.nParameter2 = getFieldInt("parameter2",&nDefault);
					task.nLevel = getFieldInt("level",&nDefault);
					task.nCircleLv = getFieldInt("zhuanshenglvl",&nDefault);
					task.nOpenday = getFieldInt("openday",&nDefault);
					task.nPreachievement = getFieldInt("preachievement",&nDefault);
					task.nAwards.clear();
					if (feildTableExists("award") && openFieldTable("award"))
					{
						if(enumTableFirst())
						{
							do
							{
								ACHIEVEAWARDCONFIG award;
								award.nId = getFieldInt("id",&nDefault);
								award.nCount = getFieldInt("count", &nDefault);
								award.nType = getFieldInt("type", &nDefault);
								task.nAwards.push_back(award);
							}while(enumTableNext());
						}
						closeTable();
					}
					if (feildTableExists("type1") && openFieldTable("type1"))
					{

						ACHIEVECONDITIONCONFIG& condition = task.nConditionOne;
						condition.nType = getFieldInt("type", &nDefault);
						if(feildTableExists("value")&& openFieldTable("value"))
						{
							if(enumTableFirst())
							{
								do
								{
									int nValue = getFieldInt(NULL);
									if(condition.nType > 0)
									{
										condition.nLimitValue = nValue;
									}else
									{
										condition.nValue.push_back(nValue);
									}
								} while (enumTableNext());
							}
							closeTable();
						}
						closeTable();
					}
					if (feildTableExists("type2") && openFieldTable("type2"))
					{
						ACHIEVECONDITIONCONFIG& condition = task.nConditionTwo;
						condition.nType = getFieldInt("type", &nDefault);
						if(feildTableExists("value")&& openFieldTable("value"))
						{
							if(enumTableFirst())
							{
								do
								{
									int nValue = getFieldInt(NULL);
									if(condition.nType > 0)
									{
										condition.nLimitValue = nValue;
									}else
									{
										condition.nValue.push_back(nValue);
									}
									
								} while (enumTableNext());
							}
							closeTable();
						}
						closeTable();
					}
				m_AchieveTasks[task.nId] = task;
				}while(enumTableNext());
			}
		}while(enumTableNext());
	}
	closeTable();
	return readAchieveTypeConfigs();
}
