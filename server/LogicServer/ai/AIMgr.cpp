#include "StdAfx.h"
#include "AIMgr.h"
#include "../base/Container.hpp"
//AI管理器,用于创建各种AI实体

CAnimalAI * CAiMgr::CreateAi(INT_PTR nAiId,CAnimal * pAnimal)
{
	if (pAnimal ==NULL) return NULL;
	AICONFIG * pConfig= GetLogicServer()->GetDataProvider()->GetAiConfig().GetAiData(nAiId);	
	if(pConfig ==NULL) return NULL;
	INT_PTR nAiType = pConfig->nAIType;
	CAnimalAI * pAI= NULL;
	AiIndex handle;
	switch(nAiType)
	{
	case atAiTypePassive:
		pAI= m_passiveAiMgr.Acquire(handle);
		break;
	case atAiTypeAggressor:
		pAI= m_aggressorAiMgr.Acquire(handle);
		break;
	case atAiTypeRandomFlee:
		pAI = m_randomFleeAiMgr.Acquire(handle);
		break;
	case atAiTypeNeutrally:
		pAI = m_neutralAiMgr.Acquire(handle);
		break;
	case atAiTypeNPC:
		pAI = m_NPCAIMgr.Acquire(handle);
		break;	
	case atAiTypePet:
		pAI =m_petAiMgr.Acquire(handle);
		break;
	case atAiTypeHero:
		pAI =m_heroAiMgr.Acquire(handle);
		break;
	default:
		break;
	}
	if(pAI ==NULL) return NULL;
	pAI->m_btAiType = (BYTE)nAiType;
	pAI->m_handle = handle;
	pAI->m_wID = (WORD)nAiId;
	pAI->m_pSelf = pAnimal;
	pAI->initialize(pConfig);	
	
	// 设置怪物AI属性：被攻击改变目标的概率
	// if(pAnimal->IsMonster())
	// {
	// 	unsigned int nID = pAnimal->GetId();
	// 	PMONSTERCONFIG pMonsterConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nID);
	// 	if (pMonsterConfig)
	// 		pAI->SetReSelTargetRate(pMonsterConfig->wReSelTargetRate);
	// }
	return pAI;
}

void  CAiMgr::DestroyAi(CAnimalAI * pAi)
{
	if(pAi ==NULL) return;
	switch(pAi->m_btAiType )
	{
	case atAiTypePassive:
		m_passiveAiMgr.Release(pAi->m_handle);
		break;
	case atAiTypeAggressor:
		m_aggressorAiMgr.Release(pAi->m_handle);
		break;
	case atAiTypeRandomFlee:
		m_randomFleeAiMgr.Release(pAi->m_handle);
		break;
	case atAiTypeNeutrally:
		m_neutralAiMgr.Release(pAi->m_handle);
		break;
	case atAiTypeNPC:
		m_NPCAIMgr.Release(pAi->m_handle);
		break;	
	case atAiTypePet:
		m_petAiMgr.Release(pAi->m_handle);
		break;	
	case atAiTypeHero:
		m_heroAiMgr.Release(pAi->m_handle);
		break;
	case atAiTypeSpecial:
		m_SpecialAiMgr.Release(pAi->m_handle);
		break;
	
	default:
		break;
	}
}
