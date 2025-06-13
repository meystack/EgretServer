#include "StdAfx.h"
#include "AiConfig.h"
#include "AiProvider.h"
#include "../base/Container.hpp"
using namespace wylib::stream;

//////////////////////////////////////////////////////////////////////////
// AI Talk Config Label
const char *pszAITalkLabel			= "talk";
const char *pszNonCombatTalkLable	= "noncombat_talk";
const char *pszCombatTalkLable		= "combat_talk";
const char *pszTalkIntervalLower	= "interval_lower";
const char *pszTalkIntervalUpper	= "interval_upper";
const char *pszTalkRate				= "talk_rate";

CAiProvider::CAiProvider()
	:Inherited(), Inherited2(), m_DataAllocator(_T("AIDataAlloc"))
{

}

CAiProvider::~CAiProvider()
{
}

bool CAiProvider::LoadAI(LPCTSTR sFilePath)
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
			showError(_T("syntax error on ai config"));
		//读取标准物品配置数据
		Result = ReadAllAI();
		
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load monster config error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load LoadAI config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);

	return Result;

}
bool  CAiProvider::ReadAllAI()
{
	
	if ( !openGlobalTable("AiConf") )
		return false;
	

	CBaseList<AICONFIG> aiList;

	const INT_PTR nCount = lua_objlen(m_pLua, -1);
	//如果物品数量为0则输出未配置任何物品的错误
	if ( nCount <= 0 )
	{
		showError(_T("no ai data on ai config"));
		return false;
	}

	aiList.reserve(nCount);
	AICONFIG *pAi= aiList;
	ZeroMemory(pAi, sizeof(AICONFIG) * nCount);

	//循环读取技能配置数据
	if ( enumTableFirst() )
	{
		INT_PTR nIdx = 0;
		CDataAllocator dataAllocator;
		do 
		{
			nIdx = getFieldInt("idx") - 1;
			if (nIdx < 0 || nIdx >= nCount)
			{
				showErrorFormat(_T("AI config Idx error, nIdx=%d "),nIdx );
			}
			
			AICONFIG * pAIConf = &aiList[nIdx];
			
			//读取1个技能的配置配置数据
			if(ReadOneAI( dataAllocator,pAIConf) ==false)
			{
				showErrorFormat(_T("AI config error, AI=%d "),nIdx );
				return false;
			}
		}
		while (enumTableNext());

		//读取物品配置完成，将临时列表中的物品数据全部拷贝到自身中
		reserve(nCount);
		trunc(0);
		addArray(pAi, nCount);
		for (size_t i = 1; i <= nCount;)
		{
			AICONFIG * pConfig= GetAiData(i);
			int nAIType = pConfig->nAIType;
			i++;
		}

		//调用自身的物品属性申请器的析构函数，以便释放之前的物品属性的内存块
		m_DataAllocator.~CObjectAllocator();
		//将临时的物品属性申请器的内存数据拷贝到自身申请器中
		m_DataAllocator = dataAllocator;
		//情况临时物品属性申请器的内存数据，防止新读取的物品属性数据被销毁
		ZeroMemory(&dataAllocator, sizeof(dataAllocator));
	}

	closeTable();
	OutputMsg(rmTip,_T("Load ai Finished"));
	return true;
}

bool CAiProvider::ReadOneAI(CDataAllocator &dataAllocator, AICONFIG *pAIConf)
{
	pAIConf->nId = getFieldInt("idx");
	pAIConf->nAIType = (BYTE)getFieldInt("aiType");
	pAIConf->patrolRadius = (BYTE)getFieldInt("patrolRadius");
	pAIConf->pursuitDistance = (WORD)getFieldInt("pursuitDistance");
	pAIConf->watchDistance = (BYTE)getFieldInt("watchDistance");
	pAIConf->returnHomeRenewRate = (BYTE)getFieldInt("returnHomeRenewRate");
	int nDis = getFieldInt("leftHomeMaxDistance");
	pAIConf->leftHomeMaxDistanceSquare = nDis * nDis;
	
	int nDef = 2000;
	pAIConf->nCheckEnemyIntervalInCombat = getFieldInt("checkenemyInterval", &nDef);
	
	nDef = 10;
	pAIConf->nStopMinTime = getFieldInt("stopMinTime", &nDef);
	
	nDef = pAIConf->nStopMinTime +10;
	pAIConf->nStopMaxTime = getFieldInt("stopMaxTime", &nDef);

	 //宠物离开主人超过这个距离，将向玩家移动
	nDef =6;
	pAIConf->bBattleFollowDis =(BYTE) getFieldInt("battleFollowDis", &nDef);

	nDef =20;
	//宠物离开主人的距离，超过这个将自动传送到主人的身边
	pAIConf->bTransferDis =(BYTE) getFieldInt("transferDis", &nDef);
	
	nDef =0;
	pAIConf->nMoveStopInterval =(int) getFieldInt("moveStopInterval", &nDef);

	nDef =0; 
	pAIConf->hitSetTargetRate=(int) getFieldInt("hitSetTargetRate", &nDef);

	nDef =3000;
	pAIConf->petFollowMasterTime =(int) getFieldInt("petFollowMasterTime", &nDef);

	ReadMonsterTalk(dataAllocator, pAIConf);
	if (!ReadPathPoints(dataAllocator, pAIConf))
	{
		return false;
	}
	return true;
}

void CAiProvider::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString s = _T("[Config Error]");
	s += sError;
	throw s;
}

bool CAiProvider::ReadMonsterTalk(CDataAllocator &allocator, AICONFIG *pAIConfig)
{
	if (!pAIConfig) return false;
	if (feildTableExists(pszAITalkLabel) && openFieldTable(pszAITalkLabel))
	{
		if (feildTableExists(pszNonCombatTalkLable) && openFieldTable(pszNonCombatTalkLable))
		{			
			ReadTalkConfigImpl(pAIConfig->m_nonCombatTalkConfig);			
		}

		if (feildTableExists(pszCombatTalkLable) && openFieldTable(pszCombatTalkLable))
		{			
			ReadTalkConfigImpl(pAIConfig->m_combatTalkConfig);
		}

		closeTable();
	}

	return true;
}

void CAiProvider::ReadTalkConfigImpl(MonsterTalkConfig &cfg)
{
	cfg.nIntervalLower	= (unsigned int)getFieldNumber(pszTalkIntervalLower);
	cfg.nIntervalUpper	= (unsigned int)getFieldNumber(pszTalkIntervalUpper);
	cfg.nTalkRate		= (unsigned int)getFieldNumber(pszTalkRate);
	closeTable();
}

bool CAiProvider::ReadPathPoints( CDataAllocator &dataAllocator,AICONFIG *pAiConfig)
{

	//怪物的技能列表
	if (feildTableExists("movePath") && openFieldTable("movePath"))
	{
		const INT_PTR nCount = (int)lua_objlen(m_pLua, -1);

		if (nCount % 2 != 0)
		{
			OutputMsg(rmError,_T("AI movePath config error! Path Point Count Error! Count must is Power of 2!"));
			return false;
		}
		
		pAiConfig->pathPoints.count = nCount;

		if (nCount > 0)
		{
			pAiConfig->pathPoints.pData = (int *)dataAllocator.allocObjects(nCount * sizeof(int));

			if (enumTableFirst())
			{
				int nIdx = 0;
				do 
				{
					pAiConfig->pathPoints[nIdx++] = ((int)getFieldInt(NULL));
				}while (enumTableNext());
			}
		}
		
		closeTable();
	}
	else
	{
		pAiConfig->pathPoints.count = 0;
	}

	return true;
}
