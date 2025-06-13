#include "StdAfx.h"
#include "LogicDataProvider.h"

/*

bool CLogicDataProvider::LoadMonsterConfig(LPCTSTR sFilePath)
{
	if (!openGlobalTable("Monster"))
	{
		return false;
	}
	m_vMonsterList.nCount = lua_objlen(m_pLua, -1);
	if (m_vMonsterList.nCount > 0)
	{
		int nSize = sizeof(MONSTERCONFIG)*m_vMonsterList.nCount;
		m_vMonsterList.pList = (MONSTERCONFIG*)m_DataAllocator.allocObjects(nSize);
		ZeroMemory(m_vMonsterList.pList,nSize);
	}
	int	nId = 0;
	if (enumTableFirst())
	{
		do 
		{
			MONSTERCONFIG* config = m_vMonsterList.pList + nId;
			config->nEntityId = getFieldInt("entityid");
			if (nId != config->nEntityId)
			{
				OutputMsg(rmError,_T("非期待的实体ID配置值，应为%d却配置为%d"), nId, config->nEntityId);
				delete config;
				return FALSE;
			}
			config->nLevel = getFieldInt("level");
			getFieldStringBuffer("name",config->szName,ArrayCount(config->szName));
			//config->nIcon = getFieldInt("icon");
			config->cal.reset(); //重置计算器
			CREATUREDATA data;
			data.nMaxHp = getFieldInt("maxhp");
			data.nMaxMp = getFieldInt("maxmp");
			data.nInnerAttack = getFieldInt("innerattack");
			data.nOutAttack = getFieldInt("outattack");
			data.fOutCriticalStrikesRate = (float)getFieldNumber("outcriticalstrikesrate");
			data.fOutDodgeRate = (float)getFieldNumber("outdodgerate");
			data.fOutHitRate = (float)getFieldNumber("outhitrate");
			data.nOutDefence = getFieldInt("outdefence");
			data.nInnerDefence = getFieldInt("innerdefence");
			data.fInnerCriticalStrikesRate = (float)getFieldNumber("innercriticalstrikesrate");
			data.fInnerDodgeRate = (float)getFieldNumber("innerdodgerate");
			data.fInnerHitRate = (float)getFieldNumber("innerhitrate");
			data.nCure = getFieldInt("cure");
			data.nHpRateRenew = (float)getFieldNumber("hprenew");
			data.nMpRateRenew = (float)getFieldNumber("mprenew");
			data.nMoveOneSlotTime = getFieldInt("moveoneslottime");
			CPropertySystem::InitMonsterCalculator(data,config->cal);

			config->nModelId = getFieldInt("modelid");
			int nDef = 10;//默认值
			config->nAttackSpeed = getFieldInt("attackSpeed",&nDef);
			nId++;
		} while (enumTableNext());
	}
	closeTable();//close Monster

	return true;
}

*/
