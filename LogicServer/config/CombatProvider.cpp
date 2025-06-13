#include "StdAfx.h"
#include "CombatProvider.h"

using namespace wylib::stream;

CCombatRankProvider::CCombatRankProvider()
	: Inherited(), m_DataAllocator(_T("CombatDataAlloc"))
{
	ZeroMemory(&m_CombatRankConfig,sizeof(m_CombatRankConfig));
	ZeroMemory(m_CombatGroupList,sizeof(m_CombatGroupList));
	m_Count = 0;
}

CCombatRankProvider::~CCombatRankProvider()
{
	//m_CombatGroupList.clear();
	//ZeroMemory(m_CombatGroupList,sizeof(m_CombatGroupList));
}

bool CCombatRankProvider::LoadCombatConfig(LPCTSTR sFilePath)
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
			showError(_T("syntax error on CombatRanking config"));
		//读取标准物品配置数据
		Result = ReadCombatConfigs();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load CombatRanking config error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load CombatRanking config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

bool CCombatRankProvider::ReadCombatConfigs()
{
	if ( !openGlobalTable("CombatRankConfig") )
		return false;

	m_CombatRankConfig.nShouCount = getFieldInt("ClinetShowCount");
	m_CombatRankConfig.nMaxRankNum = getFieldInt("MaxRankNum");
	m_CombatRankConfig.nLevLimit = getFieldInt("LevLimit");
	m_CombatRankConfig.nCircleLimit = getFieldInt("circleLimit");
	m_CombatRankConfig.nMaxChallangeRecord = getFieldInt("MaxChallangeRecord");
	m_CombatRankConfig.nMaxChallageTimes = getFieldInt("CanChanllangeTimes");
	m_CombatRankConfig.nMaxPickTimes = getFieldInt("maxPickCount");
	m_CombatRankConfig.nMaxCD = getFieldInt("maxCDTime");
	m_CombatRankConfig.nAddCD = getFieldInt("addCDTime");


	LPCSTR tempStr;
	INT_PTR i = 1;
	if ( openFieldTable("RankInfo") )
	{
		size_t nCount = lua_objlen(m_pLua,-1);

		if (nCount > 0 )
		{
			if ( enumTableFirst() )
			{
				do 
				{
					if (i >= enMaxVocCount)
					{
						endTableEnum();
						break;
					}

					tempStr = getFieldString("RankName");
					if (tempStr)
					{
						_asncpytA(m_CombatRankConfig.sRankInfo[i].nRankName, tempStr);
					}

					tempStr = getFieldString("RankFile");
					if (tempStr)
					{
						_asncpytA(m_CombatRankConfig.sRankInfo[i].nFileName, tempStr);
					}

					i++;
				}while (enumTableNext());
			}
		}
		closeTable();
	}

	i=0;
	if ( openFieldTable("PickRule") )
	{
		size_t nCount = lua_objlen(m_pLua,-1);

		if (nCount > 0 )
		{
			if ( enumTableFirst() )
			{
				do 
				{
					m_CombatGroupList[i].nStartRank = getFieldInt("StartRank");
					m_CombatGroupList[i].nEndRank = getFieldInt("EndRank");

					if (openFieldTable("PickRange"))
					{
						size_t nPickCount = lua_objlen(m_pLua, -1);
						if (nPickCount > 0)
						{
							INT_PTR j = 0;
							if (enumTableFirst())
							{
								do 
								{
									if (j >= MAXPICKITEMNUM)
									{
										endTableEnum();
										break;
									}
									m_CombatGroupList[i].sPickitems[j].nStartPickRank = getFieldInt("StartPick");
									m_CombatGroupList[i].sPickitems[j].nEndPIckRank = getFieldInt("EndPick");
									j++;
								} while (enumTableNext());
							}
						}
						closeTable();
					}

					i++;
					m_Count++;
				}while (enumTableNext());
			}
		}
		closeTable();

	}

	m_DataAllocator.~CObjectAllocator();
	closeTable();
	return true;
}
