#include "StdAfx.h"
#include "../base/Container.hpp"
CMiscProvider::CMiscProvider()
{

}

CMiscProvider::~CMiscProvider()
{

}

bool CMiscProvider::LoadNpcStatueConfig(LPCTSTR sFilePath)
{
	bool bResult = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;
	try
	{
		//从文本加载配置脚本
		if (ms.loadFromFile(sFilePath) <= 0)
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		//对配置脚本进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
		//设置脚本内容
		if (!setScript(sText))
			showError(_T("syntax error on ai config"));
		bResult = ReadAllNPCStatue();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load %s config error: %s"),sFilePath, s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load NPCStatue config "));
	}

	//销毁脚本虚拟机
	setScript(NULL);

	return bResult;
}

bool CMiscProvider::ReadAllNPCStatue()
{
	if (!openGlobalTable("NpcStatue"))
		return false;

	CBaseList<NPCSTATUE> dataList;
	const INT_PTR nCount = lua_objlen(m_pLua, -1);
	if (nCount <= 0)
	{
		showError(_T("no data on NPCStatue config"));
		return true;
	}

	dataList.reserve(nCount);
	NPCSTATUE *pData = dataList;
	ZeroMemory(pData, sizeof(NPCSTATUE) * nCount);

	//循环读取
	if (enumTableFirst())
	{
		INT_PTR nIdx = 0;
		CDataAllocator dataAllocator(_T("NPCStatueTmpAlloc"));
		do 
		{
			NPCSTATUE *pOneData = &dataList[nIdx];
			int nDef = 0;
			pOneData->btType = (BYTE)getFieldInt("type");
			pOneData->btParam = (BYTE)getFieldInt("param");
			getFieldStringBuffer("npcScene", pOneData->sSceneName, sizeof(pOneData->sSceneName));
			getFieldStringBuffer("npcName", pOneData->sNpcName, sizeof(pOneData->sNpcName));
			pOneData->nMaleModelId = getFieldInt("maleModelid");
			pOneData->nFemaleModelId = getFieldInt("femaleModelid");
			nIdx++;
		} while (enumTableNext());

		//读取配置完成，将临时列表中的数据全部拷贝到自身中
		m_npcStatue.reserve(nCount);
		m_npcStatue.trunc(0);
		m_npcStatue.addArray(pData, nCount);
		//调用自身的申请器的析构函数，以便释放之前的内存块
		m_npcStatueAllocator.~CObjectAllocator();
		//将临时的申请器的内存数据拷贝到自身申请器中
		m_npcStatueAllocator = dataAllocator;
		//清空临时申请器的内存数据，防止新读取的数据被销毁
		ZeroMemory(&dataAllocator, sizeof(dataAllocator));
	}

	closeTable();

	return true;
}

bool CMiscProvider::ReadAttributeTable(CObjectAllocator<char> &dataAllocator, CStdItem::AttributeGroup *pAttrGroup)
{
	PGAMEATTR pAttr;
	pAttrGroup->nCount = lua_objlen(m_pLua, -1);

	if (enumTableFirst())
	{
		pAttr = pAttrGroup->pAttrs = (PGAMEATTR)dataAllocator.allocObjects(sizeof(*(pAttrGroup->pAttrs)) * pAttrGroup->nCount);
		do 
		{
			pAttr->type = getFieldInt("type");
			//判断属性类型是否有效
			if ( pAttr->type < aUndefined || pAttr->type >= GameAttributeCount )
			{
				showErrorFormat(_T("attr type config error %d"),pAttr->type);
			}
			switch(AttrDataTypes[pAttr->type])
			{
			case adSmall:
			case adShort:
			case adInt:
				pAttr->value.nValue = getFieldInt("value");
				break;
			case adUSmall:
			case adUShort:
			case adUInt:
				pAttr->value.uValue = (UINT)getFieldInt64("value");
				break;
			case adFloat:
				pAttr->value.fValue = (float)getFieldNumber("value");
				break;
			}
			pAttr++;
		}
		while (enumTableNext());
	}
	return true;
}


bool CMiscProvider::LoadRankPropConfig(LPCTSTR sFilePath)
{
	bool bResult = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;
	try
	{
		//从文本加载配置脚本
		if (ms.loadFromFile(sFilePath) <= 0)
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		//对配置脚本进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
		//设置脚本内容
		if (!setScript(sText))
			showError(_T("syntax error on rankprop config"));
		bResult = ReadAllRankProp();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load %s config error: %s"),sFilePath, s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load rankprop config "));
	}

	//销毁脚本虚拟机
	setScript(NULL);

	return bResult;
}


bool CMiscProvider::ReadAllRankProp()
{
	// if (!openGlobalTable("RankProp"))
	// 	return false;

	// //获取配置中的物品数量
	// const INT_PTR nCount = lua_objlen(m_pLua, -1);
	// if (nCount <= 0)
	// {
	// 	m_RankProps.count = 0;
	// }
	// else
	// {
	// 	m_RankProps.count = nCount;
	// 	m_RankProps.pData = (RANKPROP *)m_DataAllocator.allocObjects(nCount * sizeof(RANKPROP));
	// 	if (enumTableFirst())
	// 	{
	// 		INT_PTR nIndex =0; //当前遍历的index
	// 		RANKPROP * pBaseData =(RANKPROP *)m_RankProps.pData;
	// 		RANKPROP * pOneData = NULL;
	// 		do 
	// 		{
	// 			pOneData = pBaseData + nIndex;

	// 			strncpy(pOneData->rankName, getFieldString("rankName",""), sizeof(pOneData->rankName)-1);
	// 			int nDef = 0;
	// 			pOneData->nValue = getFieldInt("value", &nDef);
	// 			//pOneData->pRanking =NULL;
				
	// 			//获取下排行榜
	// 			CRanking *pRank =GetGlobalLogicEngine()->GetRankingMgr().GetRanking(pOneData->rankName);
	// 			pOneData->pRanking = pRank;

	// 			if (feildTableExists("rankProp") && openFieldTable("rankProp"))
	// 			{
	// 				const INT_PTR nPropCount = lua_objlen(m_pLua, -1); //
	// 				if (nPropCount <= 0)
	// 				{
	// 					pOneData->props.count = 0;
	// 				}
	// 				else
	// 				{
	// 					pOneData->props.count = nPropCount;
	// 					pOneData->props.pData = (CStdItem::AttributeGroup *)m_DataAllocator.allocObjects(nPropCount * sizeof(CStdItem::AttributeGroup));
	// 					if ( enumTableFirst() )
	// 					{
	// 						INT_PTR nIdx = 0;

	// 						CStdItem::AttributeGroup * pBaseProp = (CStdItem::AttributeGroup *)pOneData->props.pData;
	// 						CStdItem::AttributeGroup * pOneProp = NULL;
	// 						do 
	// 						{					
	// 							pOneProp = pBaseProp + nIdx;
	// 							if (!ReadAttributeTable(m_DataAllocator, pOneProp))
	// 							{
	// 								OutputMsg(rmError, "Read rank prop error!!");
	// 								return false;
	// 							}

	// 							nIdx++;
	// 						}while (enumTableNext());
	// 					}
	// 				}
	// 				closeTable();
	// 			}

	// 			nIndex ++;
	// 		}while (enumTableNext());
	// 	}
		
	// }

	// closeTable();
	return true;
}


bool CMiscProvider::LoadSalaryConfig(LPCTSTR sFilePath)
{
	bool bResult = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;
	try
	{
		//从文本加载配置脚本
		if (ms.loadFromFile(sFilePath) <= 0)
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		//对配置脚本进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
		//设置脚本内容
		if (!setScript(sText))
			showError(_T("syntax error on rankprop config"));
		bResult = ReadAllSalaryConfig();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load %s config error: %s"),sFilePath, s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load salary config "));
	}

	//销毁脚本虚拟机
	setScript(NULL);

	return bResult;
}


bool CMiscProvider::ReadAllSalaryConfig()
{
	if (!openGlobalTable("SignInAndSalary"))
		return false;

	if (feildTableExists("salary") && openFieldTable("salary"))
	{
		const INT_PTR nCount = lua_objlen(m_pLua, -1); //
		if (nCount <= 0)
		{
			m_SalayConfig.count = 0;
		}
		else
		{
			m_SalayConfig.count = nCount;
			m_SalayConfig.pData = (SALARY *)m_DataAllocator.allocObjects(nCount * sizeof(SALARY));
			int nDef = 0;
			if ( enumTableFirst() )
			{
				INT_PTR nIdx = 0;
				
				SALARY * pBaseData = (SALARY *)m_SalayConfig.pData;
				SALARY * pOneData = NULL;
				do 
				{					
					pOneData = pBaseData + nIdx;

					pOneData->nBegin = getFieldInt("begin", &nDef);
					pOneData->nOver = getFieldInt("over", &nDef);
					pOneData->nRate = getFieldInt("rate", &nDef);

					nIdx++;
				}while (enumTableNext());
			}
		}
		closeTable();
	}
	return true;
}

RANKPROP* CMiscProvider::GetRankPropByRankName(LPCSTR sName)
{
	for (INT_PTR i = 0; i < m_RankProps.count; i++)
	{
		RANKPROP *pProps = m_RankProps.pData + i;
		if (pProps == NULL || pProps->rankName[0] == '\0') continue;

		if (strcmp(sName, pProps->rankName) == 0)
		{
			return pProps;
		}
	}
	return nullptr;
}


bool CMiscProvider::LoadCircleConfig(LPCTSTR sFilePath)
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
			showError(_T("syntax error on SpecialRingTrain config"));
		//读取标准物品配置数据
		Result = ReadCircleConfig();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load SpecialRingTrain config error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load SpecialRingTrain config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}
// bool CMiscProvider::ReadCircleConfig()
// {
// 	if (!openGlobalTable("CircleConfig"))
// 	{
// 		return false;
// 	}
// 	int nDef = 0;
// 	int nMaxLevel = 0;
// 	nMaxLevel = getFieldInt("circle0Maxlevel", &nDef);
// 	m_CircleMaxLevelConfig.add(nMaxLevel);
// 	//Circles table
// 	if (feildTableExists("Circles") && openFieldTable("Circles"))
// 	{
// 		if (enumTableFirst())
// 		{
// 			int nIndex = 0;
// 			do 
// 			{
// 				nMaxLevel = getFieldInt("level", &nDef);
// 				m_CircleMaxLevelConfig.add(nMaxLevel);
// 				nIndex++;
// 			} while (enumTableNext());
// 		}
// 		closeTable();
// 	}
//     //CircleLevel
// 	if (feildTableExists("CircleLevel") && openFieldTable("CircleLevel"))
// 	{
// 		const INT_PTR nCount =(INT_PTR) lua_objlen(m_pLua, -1) ;
// 		m_CircleLevelConfig.starlist.count = (int)lua_objlen(m_pLua,-1);
// 		m_CircleLevelConfig.starlist.pData =(PCIRCLEATTR)m_DataAllocator.allocObjects(m_CircleLevelConfig.starlist.count * sizeof(CIRCLEATTR));
// 		if (enumTableFirst())
// 		{
// 			int nIdx = 0;
// 			do 
// 			{
// 			    CIRCLEATTR& level = m_CircleLevelConfig.starlist[nIdx];
// 				if ( feildTableExists("attrs") && openFieldTable("attrs") )
// 				{
// 					int  nCount = (int)lua_objlen(m_pLua,-1);
// 					PGAMEATTR pAttr = NULL;
// 					level.attri.nCount = nCount;
// 					if(nCount > 0)
// 					{
// 					    level.attri.pAttrs = (PGAMEATTR)m_DataAllocator.allocObjects(sizeof(GAMEATTR) * nCount);
// 						ZeroMemory(level.attri.pAttrs, sizeof(GAMEATTR) * nCount);
// 						pAttr = level.attri.pAttrs;
// 					}
// 					if(enumTableFirst())
// 					{
// 					    if(!LoadAttri(pAttr))
// 						{
// 						    return false;
// 						}
// 						pAttr++;
// 					}
// 					closeTable();
// 				}
// 				nIdx++;
// 			} while (enumTableNext());
// 		}
// 		closeTable();
// 	}
// 	closeTable();
// 	return true;
// }

bool CMiscProvider::ReadCircleConfig()
{
	if (!openGlobalTable("CircleConfig"))
	{
		return false;
	}
	int nDef = 0;

    //CircleLevel
	if (feildTableExists("CircleLevel") && openFieldTable("CircleLevel"))
	{
		const INT_PTR nCount =(INT_PTR) lua_objlen(m_pLua, -1) ;
		// m_CircleLevelConfig.m_lCircleLv.count = (int)lua_objlen(m_pLua,-1);
		// m_CircleLevelConfig.m_lCircleLv.pData =(PCIRCLELEVEL)m_DataAllocator.allocObjects(m_CircleLevelConfig.m_lCircleLv.count * sizeof(CIRCLELEVEL));
		if (enumTableFirst())
		{
			int nIdx = 0;
			do 
			{
			    // CIRCLELEVEL& level = m_CircleLevelConfig.m_lCircleLv[nIdx];
				int nlevel = getFieldInt("level", &nDef);
				CIRCLELEVEL level;
				level.nlevel =nlevel;
				level.nServerOpenDay = getFieldInt("openday", &nDef);
				level.nLevelLimit = getFieldInt("levellimit", &nDef);
				// level.ncost = getFieldInt("cost", &nDef);
				// level.nLastCircleNum = getFieldInt("lastcircleNum", &nDef);
				level.nCostLevel = getFieldInt("costlevel", &nDef);
				level.nLevelBlock = getFieldInt("levelblock", &nDef);
				level.nTipId = getFieldInt("tipsid",&nDef);
				// level.m_vSubDay.clear();
				if(feildTableExists("item1") && openFieldTable("item1"))
				{
					ACTORAWARD cfg ;
					cfg.btType = (BYTE)getFieldInt("type", &nDef);
					cfg.wId = (WORD)getFieldInt("id", &nDef);
					cfg.wCount = getFieldInt("count", &nDef);
					level.cost.push_back(cfg);
					closeTable();
				}
				if(feildTableExists("item2") && openFieldTable("item2"))
				{
					ACTORAWARD cfg ;
					cfg.btType = (BYTE)getFieldInt("type", &nDef);
					cfg.wId = (WORD)getFieldInt("id", &nDef);
					cfg.wCount = getFieldInt("count", &nDef);
					level.cost.push_back(cfg);
					closeTable();
				}
				if(feildTableExists("item3") && openFieldTable("item3"))
				{
					ACTORAWARD cfg ;
					cfg.btType = (BYTE)getFieldInt("type", &nDef);
					cfg.wId = (WORD)getFieldInt("id", &nDef);
					cfg.wCount = getFieldInt("count", &nDef);
					level.cost.push_back(cfg);
					closeTable();
				}
				// level.nSubDay1 = getFieldInt("subDay1", &nDef);
				// level.nSubDay2 = getFieldInt("subDay2",&nDef);
				// level.nSubDay2 =getFieldInt("subDay3", &nDef);
				if ( feildTableExists("attrs") && openFieldTable("attrs") )
				{
					int  nCount = (int)lua_objlen(m_pLua,-1);
					PGAMEATTR pAttr = NULL;
					level.attri.nCount = nCount;
					if(nCount > 0)
					{
					    level.attri.pAttrs = (PGAMEATTR)m_DataAllocator.allocObjects(sizeof(GAMEATTR) * nCount);
						ZeroMemory(level.attri.pAttrs, sizeof(GAMEATTR) * nCount);
						pAttr = level.attri.pAttrs;
					}
					if(enumTableFirst())
					{
					    if(!LoadAttri(pAttr))
						{
						    return false;
						}
						pAttr++;
					}
					closeTable();
				}

				nIdx++;
				m_CircleLevelConfig.m_lCircleLv[nlevel] = level;
			} while (enumTableNext());
		}
		closeTable();
	}

	if (feildTableExists("ExchangCircleConfig") && openFieldTable("ExchangCircleConfig"))
	{
		const INT_PTR nCount =(INT_PTR) lua_objlen(m_pLua, -1) ;
		m_CircleLevelConfig.m_lExSoul.count = nCount;
		m_CircleLevelConfig.m_lExSoul.pData = (PEXCHANGECIRCLE)m_DataAllocator.allocObjects(m_CircleLevelConfig.m_lExSoul.count * sizeof(EXCHANGECIRCLE));
		if (enumTableFirst())
		{
			int nIdx = 0;
			do 
			{
			    
				int nType = getFieldInt("type", &nDef);
				EXCHANGECIRCLE& info = m_CircleLevelConfig.m_lExSoul[nType-1];
				info.nType = nType;
				info.nCost = getFieldInt("cost", &nDef);
				info.nValue = getFieldInt("value",&nDef);
				info.nUseLimit = getFieldInt("useLimit", &nDef);
				info.nStaticType = getFieldInt("statictype", &nDef);
				nIdx++;
			} while (enumTableNext());
		}
		closeTable();
	}

	if(feildTableExists("CircleConstConfig") && openFieldTable("CircleConstConfig"))
	{

		m_CircleLevelConfig.nOpenLv = getFieldInt("openlv", &nDef);
		m_CircleLevelConfig.nOpenServerDay = getFieldInt("openserverday", &nDef);
		closeTable();
	}
	closeTable();
	return true;
}
int CMiscProvider::GetPlayerMaxLevel(int nCirlce, int nState)
{
	if(nState == 0) //默认直接返回最大值
	{
		return 999;
	}
	std::map<int, CIRCLELEVEL>::iterator it = m_CircleLevelConfig.m_lCircleLv.find(nCirlce);
	if (it!= m_CircleLevelConfig.m_lCircleLv.end())
	{
		return it->second.nLevelBlock;
	}
	return 0;
}

bool CMiscProvider::LoadLevelConfig(LPCTSTR sFilePath)
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
			showError(_T("syntax error on SoldierSoulConfigg"));
		//读取标准物品配置数据
		Result = ReadLevelConfig();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load SoldierSoulConfig error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load SoldierSoulConfig"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}
POPENLEVELCONFIG CMiscProvider::GetOpenLevel(int nId)
{
	if (nId > 0 && nId <= m_openLevel.count())
	{
		return &m_openLevel[nId-1];
	}
	return  NULL;
}
bool CMiscProvider::ReadLevelConfig()
{
	if (!openGlobalTable("LevelConfig"))
	{
		return false;
	}
	int nDefault = 0;
	if (enumTableFirst())
	{
		do 
		{
			OPENLEVELCONFIG levelCfg;
			levelCfg.circle = getFieldInt("circle",&nDefault);
			levelCfg.level = getFieldInt("level",&nDefault);
			m_openLevel.add(levelCfg);
		}while(enumTableNext());
	}
	return true;
}

//装载装备强化（升星）的配置
bool CMiscProvider::LoadEquipInscriptConfig(LPCTSTR sFilePath)
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
			showError(_T("syntax error on EquipInscriptConfigg"));
		//读取标准物品配置数据
		Result = ReadEquipInscriptConfig();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load EquipInscriptConfig error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load EquipInscriptConfig"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

//按部位读取的属性
bool CMiscProvider::ReadEquipPosConfig(AttriGroup& stAttriGroup)
{
	stAttriGroup.nCount = (int)lua_objlen(m_pLua,-1);
	//OutputMsg(rmWaning,"ReadEquipPosConfig, nCount=%d",stAttriGroup.nCount);
	PGAMEATTR pAttr = NULL;
	if( stAttriGroup.nCount > 0 )
	{
		stAttriGroup.pAttrs  = (PGAMEATTR)m_DataAllocator.allocObjects(sizeof(GAMEATTR) * stAttriGroup.nCount);
		ZeroMemory(stAttriGroup.pAttrs,	sizeof(GAMEATTR) * stAttriGroup.nCount);
		pAttr = stAttriGroup.pAttrs;
	}

	int nDef = 0;
	if (enumTableFirst())
	{
		if( !LoadEquipAttri(pAttr) )
		{
			return false;
		}
		pAttr++;
	}

	return true;
}

bool CMiscProvider::ReadEquipInscriptConfig()  
{
	if (!openGlobalTable("EquipInscriptCfg"))
	{
		return false;
	}

	if( feildTableExists("inscript") && openFieldTable("inscript") )
	{
		int nInscriptNum = (int)lua_objlen(m_pLua,-1);
		if( enumTableFirst() )
		{
			do 
			{
				EQUIPSTARCONFIG  cfg;
				if( feildTableExists("inscriptLevel") && openFieldTable("inscriptLevel") )
				{
					cfg.starlist.count = (int)lua_objlen(m_pLua,-1);
					cfg.starlist.pData =(PEQUIPINSCRIPT)m_DataAllocator.allocObjects(cfg.starlist.count * sizeof(EQUIPINSCRIPT));
					if( enumTableFirst() )
					{
						int nIndex = 0;	
						do 
						{
							EQUIPINSCRIPT& stInscript = cfg.starlist[nIndex];
							if( feildTableExists("attrs") && openFieldTable("attrs") )
							{
								int nPosNum = (int)lua_objlen(m_pLua,-1);
								if( enumTableFirst() )	//按部位读取属性
								{
									int nPos = 1;
									bool boFlag2EndEnum = false;
									do
									{
										//OutputMsg(rmWaning,"ReadEquipInscriptConfig, nPos=%d", nPos);
										if(nPos >= Item::itEquipMax)
										{
											boFlag2EndEnum = true;
											break;
										}
										AttriGroup& stAttriGroup = stInscript.posAttri[nPos++];
										ReadEquipPosConfig(stAttriGroup);

									}while(enumTableNext());

									if (boFlag2EndEnum)
									{
										endTableEnum();
									}
								}

								closeTable();
							}
							nIndex++;	
						}while (enumTableNext());
					}

					closeTable();
				}
				m_EquipInscript.add(cfg);
			}  while (enumTableNext());
		}

		closeTable();
	}
	
	closeTable();
	return true;
}

bool CMiscProvider::LoadEquipAttri( PGAMEATTR pAttr )
{
	int nDef = 0;
	do
	{
		pAttr->type = getFieldInt("type");
		pAttr->reserve[0] = (char)getFieldInt("job",&nDef);			//属性分职业
		if ( pAttr->type < aUndefined || pAttr->type >= GameAttributeCount )
		{
			showErrorFormat(_T("Acupoint type config error %d"),pAttr->type);
		}
		switch(AttrDataTypes[pAttr->type])
		{
		case adSmall:
		case adShort:
		case adInt:
			pAttr->value.nValue = getFieldInt("value");
			break;
		case adUSmall:
		case adUShort:
		case adUInt:
			pAttr->value.uValue = (UINT)getFieldInt64("value");
			break;
		case adFloat:
			pAttr->value.fValue = (float)getFieldNumber("value");
			break;
		}
		++pAttr;
	}while(enumTableNext());
	pAttr ++;
	return true;
}



//经脉升级
bool CMiscProvider::LoadMeridiansCfg(LPCTSTR sFilePath)
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
			showError(_T("syntax error on LoadMeridiansCfg"));
		Result = ReadMeridiansCfg();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load LoadMeridiansCfg error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load LoadMeridiansCfg"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

bool CMiscProvider::ReadMeridiansCfg() 
{
	if (!openGlobalTable("MeridiansConfig"))
	{
		return false;
	}
	int nDef = 0;
	if(enumTableFirst())
	{
		do
		{
			int nlevel = getFieldInt("level", &nDef);
			if(nlevel ==0)
				continue;

			MeridianUpCfg cfg;
			cfg.nLevel =nlevel;
			cfg.nServerOpenDay = getFieldInt("serveropenday", &nDef);
			cfg.nLevelLimit = getFieldInt("levellimit", &nDef);
			cfg.nCircle = getFieldInt("circle", &nDef);
			// cfg.nCostExp = getFieldInt("costexp", &nDef);
			// cfg.nCostBindCoin = getFieldInt("costbindcoin", &nDef);
			// cfg.nCostBook = getFieldInt("cost", &nDef);
			getFieldStringBuffer("tips", cfg.nTips, sizeof(cfg.nTips));
			if ( feildTableExists("cost") && openFieldTable("cost") )
			{
				if(enumTableFirst())
				{
					do
					{
						/* code */
						ACTORAWARD cost;
						cost.btType = getFieldInt("type", &nDef);
						cost.wId = getFieldInt("id", &nDef);
						cost.wCount = getFieldInt("count", &nDef);
						cfg.costs.push_back(cost);
					} while (enumTableNext());
					
				}
				closeTable();
			}

			if ( feildTableExists("attrs") && openFieldTable("attrs") )
			{
				int  nCount = (int)lua_objlen(m_pLua,-1);
				PGAMEATTR pAttr = NULL;
				cfg.attri.nCount = nCount;
				if(nCount > 0)
				{
					cfg.attri.pAttrs = (PGAMEATTR)m_DataAllocator.allocObjects(sizeof(GAMEATTR) * nCount);
					ZeroMemory(cfg.attri.pAttrs, sizeof(GAMEATTR) * nCount);
					pAttr = cfg.attri.pAttrs;
				}
				if(enumTableFirst())
				{
					if(!LoadAttri(pAttr))
					{
						return false;
					}
					pAttr++;
				}
				closeTable();
			}
			m_MeridianUpCfg[nlevel] = cfg;
		} while (enumTableNext());
		
	}
	closeTable();
	return true;
}


bool CMiscProvider::LoadMonthCardConfig(LPCTSTR sFilePath)
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
			showError(_T("syntax error on LoadMeridiansCfg"));
		Result = ReadMonthCardConfig();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load LoadMeridiansCfg error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load LoadMeridiansCfg"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

bool CMiscProvider::ReadMonthCardConfig()
{
	if (!openGlobalTable("MonthCardConfig"))
	{
		return false;
	}
	int nDef = 0;
	int nDef_f1 = -1;
	const INT_PTR nCount =(INT_PTR) lua_objlen(m_pLua, -1) ;
	if (enumTableFirst())
	{
		memset(&m_SuperRightMap, 0, sizeof(m_SuperRightMap));
		do 
		{
			int nIdx = getFieldInt("idx", &nDef);
			if (nIdx > MAXMONTHCARDCFGNUM ) return false;
			MONTHCARDCONFIG& config = m_MonthCardConfig[nIdx - 1];

			config.nSuperRightLV = getFieldInt("superRightLV", &nDef);
			config.nMaxExpDampNum = getFieldInt("MaxExpDamp", &nDef);
			config.nJiShouNum = getFieldInt("JiShouCount", &nDef);
			config.nAtvPaoDianPlus = getFieldInt("AtvPaoDianPlus", &nDef);
			if(config.nSuperRightLV >0 && config.nSuperRightLV <= MAXCOLORCARDLEVEL)
				m_SuperRightMap[config.nSuperRightLV ] = nIdx - 1 ;
			else if(config.nSuperRightLV == -1 )
				m_DefaultPaoDianPlusKey = nIdx - 1;

			// 消耗
			memset(&config.Consume, -1, sizeof(config.Consume));
			if (feildTableExists("consume") && openFieldTable("consume"))
			{
				config.Consume.type = getFieldInt("type", &nDef_f1);
				config.Consume.id = getFieldInt("id", &nDef_f1);
				config.Consume.count = getFieldInt("count", &nDef_f1);
				closeTable();
			}
			
			// 开通奖励
			memset(config.BuyAwardList, -1, sizeof(config.BuyAwardList));
			if (feildTableExists("buyAward") && openFieldTable("buyAward"))
			{
				if (enumTableFirst())
				{
					int i = 0;
					do 
					{
						if (i >= 6) 
						{
							continue;
						} 
						
						config.BuyAwardList[i].type = getFieldInt("type", &nDef_f1);
						config.BuyAwardList[i].id = getFieldInt("id", &nDef_f1);
						config.BuyAwardList[i].count = getFieldInt("count", &nDef_f1);
						i++;
					} while (enumTableNext());
				}
				closeTable();
			}
			config.m_recoverGolds.clear();
			if (feildTableExists("goldget") && openFieldTable("goldget"))
			{
				if (enumTableFirst())
				{
					do 
					{
						GOODS_TAG cfg;
						cfg.type = getFieldInt("type", &nDef_f1);
						cfg.id = getFieldInt("id", &nDef_f1);
						cfg.count = getFieldInt("count", &nDef_f1);
						config.m_recoverGolds.emplace_back(cfg);

					} while (enumTableNext());
				}
				closeTable();
			}

			// 每日奖励
			memset(config.DailyAwardList, -1, sizeof(config.DailyAwardList));
			if (feildTableExists("dailyAward") && openFieldTable("dailyAward"))
			{
				if (enumTableFirst())
				{
					int i = 0;
					do 
					{
						if (i >= 6) 
						{
							continue;
						} 
						
						config.DailyAwardList[i].type = getFieldInt("type", &nDef_f1);
						config.DailyAwardList[i].id = getFieldInt("id", &nDef_f1);
						config.DailyAwardList[i].count = getFieldInt("count", &nDef_f1);
						i++;
					} while (enumTableNext());
				}
				closeTable();
			}
			
		} while (enumTableNext());
	}
	closeTable();
	return true;
}

bool CMiscProvider::LoadFrenzyCfg(LPCTSTR sFilePath)
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
			showError(_T("syntax error on LoaddemonCfg"));
		Result = ReadFrenzy();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load LoaddemonCfg error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load LoaddemonCfg"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}
bool CMiscProvider::ReadFrenzy()
{
	if (openGlobalTable("RageconstConfig"))
	{
		int ndef_0 = 0;

		// 狂暴条件
		if ( feildTableExists("condition") && openFieldTable("condition") )
		{
			m_FrenzyCfg.nCardLv = getFieldInt("CardLv", &ndef_0);
			closeTable();
		}
		// 狂暴属性
		if ( feildTableExists("RageAttr") && openFieldTable("RageAttr") )
		{
			if(enumTableFirst())
			{
				do
				{
					int type =getFieldInt("type", &ndef_0);
					int value =getFieldInt("value", &ndef_0);
					m_FrenzyCfg.vecPropertys.push_back(std::make_tuple(type,value));
				}
				while (enumTableNext());
			}
			closeTable();
		}
		// 狂暴购买价格
		if ( feildTableExists("RagePrice") && openFieldTable("RagePrice") )
		{
			if(enumTableFirst())
			{
				do
				{
					int type =getFieldInt("type", &ndef_0);
					int id =getFieldInt("id", &ndef_0);
					int count =getFieldInt("count", &ndef_0);
					m_FrenzyCfg.vecConsum.push_back(std::make_tuple(type,id,count));
				}
				while (enumTableNext());
			}
			closeTable();
		}
		// 击杀狂暴者获得奖励
		if ( feildTableExists("KillerRewards") && openFieldTable("KillerRewards") )
		{
			if(enumTableFirst())
			{
				do
				{
					int type =getFieldInt("type", &ndef_0);
					int id =getFieldInt("id", &ndef_0);
					int count =getFieldInt("count", &ndef_0);
					m_FrenzyCfg.vecKillAward.push_back(std::make_tuple(type,id,count));
				}
				while (enumTableNext());
			}
			closeTable();
		}
		// 击杀者的邮件
		m_FrenzyCfg.vecKillerTitleHead = getFieldString("KillerMailTitle", "KillerMailTitle");
		m_FrenzyCfg.vecKillerTitleContent = getFieldString("KillerMailText", "KillerMailText");
	}
	return true;
}

bool CMiscProvider::LoadOfficeCfg(LPCTSTR sFilePath)
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
			showError(_T("syntax error on LoaddemonCfg"));
		Result = ReadOffice();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load LoaddemonCfg error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load LoaddemonCfg"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}
bool CMiscProvider::ReadOffice()
{
	if (openGlobalTable("OfficeConfig"))
	{
		int ndef_0 = 0, ndef_1 = 1;
		const INT_PTR nCount = lua_objlen(m_pLua, -1);
		m_OfficeCfg.resize(nCount+1);

		if(enumTableFirst())
		{
			do
			{
				int lvl =getFieldInt("officelevel", &ndef_0);
				if (lvl == 0 || lvl > nCount)
				{
					OutputMsg(rmError, _T("load OfficeConfig error: lvl=%d"), lvl);
					return false;
				}
				
				m_OfficeCfg[lvl].nLevel = lvl;
				m_OfficeCfg[lvl].nIsNotice = getFieldInt("notice", &ndef_0);
				m_OfficeCfg[lvl].nLvlLimit = getFieldInt("levellimit", &ndef_0);
				m_OfficeCfg[lvl].nCircleLimit = getFieldInt("circle", &ndef_0);
			
				char * pContent=(char *)getFieldString("show");
				strncpy(m_OfficeCfg[lvl].sName, pContent, sizeof(m_OfficeCfg[lvl].sName));
				
				// 升级消耗
				if ( feildTableExists("consume") && openFieldTable("consume") )
				{
					if(enumTableFirst())
					{
						do
						{
							int type =getFieldInt("type", &ndef_0);
							int id =getFieldInt("id", &ndef_0);
							int count =getFieldInt("count", &ndef_0);
							m_OfficeCfg[lvl].vecConsum.push_back(std::make_tuple(type,id,count));
						}
						while (enumTableNext());
					}
					closeTable();
				}

				// 加成属性
				if ( feildTableExists("attribute") && openFieldTable("attribute") )
				{
					if(enumTableFirst())
					{
						do
						{
							int type =getFieldInt("type", &ndef_0);
							int value =getFieldInt("value", &ndef_0);
							m_OfficeCfg[lvl].vecPropertys.push_back(std::make_tuple(type,value));
						}
						while (enumTableNext());
					}
					closeTable();
				}
			}
			while (enumTableNext());
		}
		closeTable();
	}
	return true;
}

bool CMiscProvider::LoadChangeVocCfg(LPCTSTR sFilePath)
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
			showError(_T("syntax error on LoaddemonCfg"));
		Result = ReadChangeVoc();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load LoaddemonCfg error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load LoaddemonCfg"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}
bool CMiscProvider::ReadChangeVoc()
{
	if (openGlobalTable("TransferConfig"))
	{
		int nDefault = 0;

		if (feildTableExists("ChangeVocationforever") && openFieldTable("ChangeVocationforever"))
		{
			if (enumTableFirst())
			{
				do 
				{
					int type =getFieldInt("type",&nDefault);
					int id =getFieldInt("id",&nDefault);
					int count =getFieldInt("count",&nDefault);
					m_ChgVocCfg.vecChangeVocationUnConsum.push_back( std::make_tuple(type,id,count) );
				} while (enumTableNext());
			}		
			closeTable();	
		}

		if (feildTableExists("ChangeVocation") && openFieldTable("ChangeVocation"))
		{
			if (enumTableFirst())
			{
				do 
				{
					int type =getFieldInt("type",&nDefault);
					int id =getFieldInt("id",&nDefault);
					int count =getFieldInt("count",&nDefault);
					m_ChgVocCfg.vecChangeVocationConsum.push_back( std::make_tuple(type,id,count) );
				} while (enumTableNext());
			}		
			closeTable();	
		}

		int nChangeVocCD = 0;
		m_ChgVocCfg.nChangeVocCD = getFieldInt("CD",&nChangeVocCD);
		closeTable();
	}
	return true;
}

//经脉升级
bool CMiscProvider::LoaddemonCfg(LPCTSTR sFilePath)
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
			showError(_T("syntax error on LoaddemonCfg"));
		Result = Readdemonsbody();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load LoaddemonCfg error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load LoaddemonCfg"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

bool CMiscProvider::Readdemonsbody() 
{
	ReadCommonDemons("demonsbodyConfig", m_nGhostCfg.m_gHostUpLvCfg);
	readDemonCost();
	ReadCommonDemons("demonslevelConfig", m_nGhostCfg.m_DemonslevelConfig);
	return true;
}

void CMiscProvider::ReadCommonDemons(const char* table, std::map<int, std::vector<GhostConfig> >& m_maps)
{
	if (openGlobalTable(table))
	{
		int nDef = 0;
		m_maps.clear();
		if(enumTableFirst())
		{
			do
			{
				if(enumTableFirst())
				{
					do
					{

						GhostConfig cfg;
						cfg.nId =getFieldInt("pos", &nDef);
						cfg.nLevel =getFieldInt("level", &nDef);
						memset(cfg.name,0,sizeof(cfg.name));
						getFieldStringBuffer("name", cfg.name, sizeof(cfg.name));
						cfg.nTipId = getFieldInt("tipsid",&nDef);
						cfg.nProbability = getFieldInt("probability",&nDef);
						if ( feildTableExists("cost") && openFieldTable("cost") )
						{
							if(enumTableFirst()) {
								do
								{
									ACTORAWARD cost;
									cost.btType = (BYTE)getFieldInt("type", &nDef);
									cost.wId = (WORD)getFieldInt("id", &nDef);
									cost.wCount = getFieldInt("count", &nDef);
									cfg.cost.push_back(cost);
								} while (enumTableNext());
								
							}
							closeTable();
						}
						if ( feildTableExists("opelimit") && openFieldTable("opelimit") )
						{

							cfg.nServerOpenDay = getFieldInt("serveropenday", &nDef);
							cfg.nLevelLimit = getFieldInt("levellimit", &nDef);
							cfg.nCircle = getFieldInt("zsLevel", &nDef);

							closeTable();
						}
						memset(&cfg.attri, 0, sizeof(cfg.attri));
						if ( feildTableExists("attr") && openFieldTable("attr") )
						{
							int  nCount = (int)lua_objlen(m_pLua,-1);
							PGAMEATTR pAttr = NULL;
							cfg.attri.nCount = nCount;
							if(nCount > 0)
							{
								cfg.attri.pAttrs = (PGAMEATTR)m_DataAllocator.allocObjects(sizeof(GAMEATTR) * nCount);
								ZeroMemory(cfg.attri.pAttrs, sizeof(GAMEATTR) * nCount);
								pAttr = cfg.attri.pAttrs;
							}
							if(enumTableFirst())
							{
								if(!LoadAttri(pAttr))
								{
									return ;
								}
								pAttr++;
							}
							closeTable();
						}
						m_maps[cfg.nId].push_back(cfg);
					} while (enumTableNext());
				}

			} while (enumTableNext());
			
		}
		closeTable();
	}
	
}


void CMiscProvider::readDemonCost()
{
	int nDef = 0;
	if (openGlobalTable("demonsconstConfig"))
	{

		m_nGhostCfg.nAddBless = getFieldInt("blessadd", &nDef);
		m_nGhostCfg.nMaxBless = getFieldInt("maxbless", &nDef);
		m_nGhostCfg.nAddProbability = getFieldInt("probabilityup", &nDef);
		m_nGhostCfg.nProbability = getFieldInt("blessup", &nDef);
		m_nGhostCfg.nMaxLv = getFieldInt("maxlevel", &nDef);
		m_nGhostCfg.nMaxCount = getFieldInt("maxcount", &nDef);
		memset(m_nGhostCfg.sTitle,0,sizeof(m_nGhostCfg.sTitle));
		memset(m_nGhostCfg.sContent,0,sizeof(m_nGhostCfg.sContent));
		getFieldStringBuffer("mailtitle", m_nGhostCfg.sTitle, sizeof(m_nGhostCfg.sTitle));
		getFieldStringBuffer("mailtext", m_nGhostCfg.sContent, sizeof(m_nGhostCfg.sContent));
		m_nGhostCfg.awards.clear();
		if ( feildTableExists("awards") && openFieldTable("awards") )
		{
			if(enumTableFirst()) {
				do
				{
					ACTORAWARD cost;
					cost.btType = (BYTE)getFieldInt("type", &nDef);
					cost.wId = (WORD)getFieldInt("id", &nDef);
					cost.wCount = getFieldInt("count", &nDef);
					m_nGhostCfg.awards.push_back(cost);
				} while (enumTableNext());
				
			}
			closeTable();
		}
		closeTable();
	}

	if(openGlobalTable("DemonsReplaceConfig"))
    {
        m_nReplaces.clear();

        if ( enumTableFirst() )
        {
            do 
            {
                gHostReplaceCfg attCfg;
                attCfg.nItemId = getFieldInt("itemid");
                attCfg.nId = getFieldInt("id");
                attCfg.nPrice = getFieldInt("price",&nDef);
                m_nReplaces[attCfg.nItemId] = attCfg;
            }while(enumTableNext());
        }
        closeTable();
    }
	
}
