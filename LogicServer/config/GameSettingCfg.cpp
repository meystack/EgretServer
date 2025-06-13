#include "StdAfx.h"
#include "GameSettingCfg.h"
#include "../base/Container.hpp"
CGameSetting::CGameSetting()
	:Inherited(),m_NewDataAllocator(_T("GameSettingDataAlloc"))
{
	memset(&m_DefaultGameSetting, 0, sizeof(m_DefaultGameSetting));
}

CGameSetting::~CGameSetting()
{
}

bool CGameSetting::LoadConfig(LPCTSTR sFilePath)
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
			showError(_T("syntax error on RideGame config"));
		//读取标准物品配置数据
		Result = readConfigs();
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

bool CGameSetting::readConfigs()
{
	if ( !openGlobalTable("SettingConfig") )
		return FALSE;

	int nDefNumber_1 = 1;
	int nDefNumber_0 = 0;
	int nTabCount = (int)lua_objlen(m_pLua,-1);;
	int nMaxIdx = 1;

	int nValue = 0;

	// 基础设置
	if (openFieldTable("BasicsSettingmConfig")) 
	{
		int nCount = (int)lua_objlen(m_pLua,-1);
		int nPos = 0;
		for (size_t i = 1; i <= nCount; i++)
		{
			if (!getFieldIndexTable(i))
			{
				//OutputMsg(rmError, "open job attr failed. l:%d, j:%d", lv, i);
				return false;
			}
			nPos = i - 1;

			nValue = getFieldInt("value", &nDefNumber_0);
			if (nValue) (m_DefaultGameSetting.m_nBase) |= (1 << nPos); 

			closeTable();
		}

		const char* skills[] = {"skills1","skills2","skills3"};
		for (size_t i = 0; i < ArrayCount(skills); i++)
		{
			if (openFieldTable(skills[i])) 
			{
				int nSkillCount = (int)lua_objlen(m_pLua,-1);
				for (size_t j = 1; j <= nSkillCount; j++)
				{
					if (!getFieldIndexTable(j))
					{
						return false;
					}

					nPos++;
					nValue = getFieldInt("value", &nDefNumber_0);
					if (nValue) (m_DefaultGameSetting.m_nBase) |= (1 << nPos);
					
					closeTable();
				}
				closeTable();
			}
		}

		if (openFieldTable("showPet")) 
		{
			nPos++;
			nValue = getFieldInt("value", &nDefNumber_0);
			if (nValue) (m_DefaultGameSetting.m_nBase) |= (1 << nPos);
			closeTable();
		}

		closeTable();		
	}
	
	// 系统设置
	if (openFieldTable("SystemSettingmConfig")) 
	{
		int nCount = (int)lua_objlen(m_pLua,-1);
		int nPos = 0;
		for (size_t i = 1; i <= nCount; i++)
		{
			if (!getFieldIndexTable(i))
			{
				//OutputMsg(rmError, "open job attr failed. l:%d, j:%d", lv, i);
				return false;
			}
			nPos = i - 1;
			nValue = getFieldInt("value", &nDefNumber_0);
			if (nValue) (m_DefaultGameSetting.m_nSystem) |= (1 << nPos); 

			closeTable();
		}
	
		closeTable();
	}

	// 药品设置
	if (openFieldTable("MedicineSettingConfig")) 
	{
		m_DefaultGameSetting.m_NmHp = getFieldInt("NmHp", &nDefNumber_0);
		m_DefaultGameSetting.m_NmMp = getFieldInt("NmMp", &nDefNumber_0);
		m_DefaultGameSetting.m_ImHp = getFieldInt("ImHp", &nDefNumber_0);
		m_DefaultGameSetting.m_ImMp = getFieldInt("ImMp", &nDefNumber_0);
		m_DefaultGameSetting.m_PerAdd1 = getFieldInt("PerAdd1", &nDefNumber_0);
		m_DefaultGameSetting.m_NmHpVal = getFieldInt("NmHpVal", &nDefNumber_0);
		m_DefaultGameSetting.m_NmHpTime = getFieldInt("NmHpTime", &nDefNumber_0);
		m_DefaultGameSetting.m_NmMpVal = getFieldInt("NmMpVal", &nDefNumber_0);
		m_DefaultGameSetting.m_NmMpTime = getFieldInt("NmMpTime", &nDefNumber_0);
		m_DefaultGameSetting.m_ImHpVal = getFieldInt("ImHpVal", &nDefNumber_0);
		m_DefaultGameSetting.m_ImHpTime = getFieldInt("ImHpTime", &nDefNumber_0);
		m_DefaultGameSetting.m_ImMpVal = getFieldInt("ImMpVal", &nDefNumber_0);
		m_DefaultGameSetting.m_ImMpTime = getFieldInt("ImMpTime", &nDefNumber_0);
		m_DefaultGameSetting.m_ValAdd1 = getFieldInt("ValAdd1", &nDefNumber_0);
		m_DefaultGameSetting.m_TimeAdd1 = getFieldInt("TimeAdd1", &nDefNumber_0);
		closeTable();
	}

	// 保护设置
	if (openFieldTable("ProtectSettingConfig")) 
	{
		m_DefaultGameSetting.m_nHp1Val = getFieldInt("Hp1Val", &nDefNumber_0);
		m_DefaultGameSetting.m_nHp1Item = getFieldInt("Hp1Item", &nDefNumber_0);
		m_DefaultGameSetting.m_nHp2Val = getFieldInt("Hp2Val", &nDefNumber_0);
		m_DefaultGameSetting.m_nHp2Item = getFieldInt("Hp2Item", &nDefNumber_0);
		closeTable();
	}

	// 挂机设置
	if (openFieldTable("HookSettingConfig")) 
	{
		int idx = 0;
		if (getFieldInt("MaxHpMonster", &nDefNumber_1) > 0)
		{
			m_DefaultGameSetting.m_nHook |= (1 << idx);
		}
		++idx;
		if (getFieldInt("DotPickItem", &nDefNumber_1) > 0)
		{
			m_DefaultGameSetting.m_nHook |= (1 << idx);
		}
		++idx;
		if (getFieldInt("PickItem", &nDefNumber_1) > 0)
		{
			m_DefaultGameSetting.m_nHook |= (1 << idx);
		}
		++idx;
		if (getFieldInt("Huofu", &nDefNumber_1) > 0)
		{
			m_DefaultGameSetting.m_nHook |= (1 << idx);
		}
		++idx;
		if (getFieldInt("CallPet", &nDefNumber_1) > 0)
		{
			m_DefaultGameSetting.m_nHook |= (1 << idx);
		}
		++idx;
		if (getFieldInt("HpMin", &nDefNumber_1) > 0)
		{
			m_DefaultGameSetting.m_nHook |= (1 << idx);
		}
		++idx;
		if (getFieldInt("Hemophagy", &nDefNumber_1) > 0)
		{
			m_DefaultGameSetting.m_nHook |= (1 << idx);
		}
		++idx;
		if (getFieldInt("Thunderbolt", &nDefNumber_1) > 0)
		{
			m_DefaultGameSetting.m_nHook |= (1 << idx);
		}
		++idx;
		if (getFieldInt("Poisoning", &nDefNumber_1) > 0)
		{
			m_DefaultGameSetting.m_nHook |= (1 << idx);
		}
		++idx;
		if (getFieldInt("Hailstorm", &nDefNumber_1) > 0)
		{
			m_DefaultGameSetting.m_nHook |= (1 << idx);
		}
		++idx;
		if (getFieldInt("RoFstorm", &nDefNumber_1) > 0)
		{
			m_DefaultGameSetting.m_nHook |= (1 << idx);
		}
		++idx;
		if (getFieldInt("ThunderClap1", &nDefNumber_1) > 0)
		{
			m_DefaultGameSetting.m_nHook |= (1 << idx);
		}
		++idx;
		if (getFieldInt("ThunderClap2", &nDefNumber_1) > 0)
		{
			m_DefaultGameSetting.m_nHook |= (1 << idx);
		}
		++idx;
		if (getFieldInt("ThunderClap3", &nDefNumber_1) > 0)
		{
			m_DefaultGameSetting.m_nHook |= (1 << idx);
		}

		m_DefaultGameSetting.m_nAutoSkillID = getFieldInt("AutoSkillID", &nDefNumber_0);
		m_DefaultGameSetting.m_nPet = getFieldInt("Pet", &nDefNumber_0);
		m_DefaultGameSetting.m_nHpLess = getFieldInt("HpLess", &nDefNumber_0);
		m_DefaultGameSetting.m_nHpLessSkill = getFieldInt("HpLessSkill", &nDefNumber_0);
		closeTable();
	}

	// 物品设置
	if (openFieldTable("ItemSettingConfig")) 
	{
		const INT_PTR nCount = lua_objlen(m_pLua, -1) + 1;
		if (nCount > GAMESET_ITEMS_COUNT)
		{
			showErrorFormat(_T("物品设置超过预定最大Size，最大:%d, 配置数:%d"), GAMESET_ITEMS_COUNT, nCount);
			return false;
		}

		memset(m_DefaultGameSetting.m_nItems,0x55555555,sizeof(m_DefaultGameSetting.m_nItems));
		m_DefaultGameSetting.m_nItems[0] = 0x55555554;

		if (enumTableFirst())
		{
			do 
			{
				bool nDef_false = false;
				int nIndex = getFieldInt("idx"); 
				int nValue1 = getFieldBoolean("value1",&nDef_false); 
				int nValue2 = getFieldBoolean("value2",&nDef_false); 

				int nPos = (nIndex)	* 2;
				
				// 自动捡取
				int nIntOffs = nPos / 32;
				int nBitOffs = nPos % 32;
				int* pStart = m_DefaultGameSetting.m_nItems + nIntOffs;
				if (nValue1) (*pStart) |= (1 << nBitOffs);
				else (*pStart) &= ~(1 << nBitOffs);	

				// 物品标红
				nPos++;
				nIntOffs = nPos / 32;
				nBitOffs = nPos % 32;
				pStart = m_DefaultGameSetting.m_nItems + nIntOffs;
				if (nValue2) (*pStart) |= (1 << nBitOffs);
				else (*pStart) &= ~(1 << nBitOffs);	

			}while (enumTableNext());

		}
		closeTable();//ItemSettingConfig
	}

	//m_DefaultGameSetting.m_nRecycle = 0;
	memset(m_DefaultGameSetting.m_nRecycle,0,sizeof(m_DefaultGameSetting.m_nRecycle));

	closeTable();
	return true;
}
