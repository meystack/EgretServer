#include "StdAfx.h"

void TalkAI::Initialize(CAnimal *pSelf, PAICONFIG pConfig)
{
	m_pSelf = pSelf;
	m_pAIConfig = pConfig;
	if (m_pAIConfig)
	{
		m_bHasNonCombatTalk = m_bHasCombatTalk = true;
		if (m_pAIConfig->m_combatTalkConfig.nIntervalLower <= 0 || 
			m_pAIConfig->m_combatTalkConfig.nIntervalLower <= 0 ||
			m_pAIConfig->m_combatTalkConfig.nTalkRate <= 0)
			m_bHasCombatTalk = false;

		if (m_pAIConfig->m_nonCombatTalkConfig.nIntervalLower <= 0 || 
			m_pAIConfig->m_nonCombatTalkConfig.nIntervalLower <= 0 ||
			m_pAIConfig->m_nonCombatTalkConfig.nTalkRate <= 0)
			m_bHasNonCombatTalk = false;
	}	
}

void TalkAI::Update(TICKCOUNT nCurrentTick)
{
	if (m_t2sTalk.CheckAndSet(nCurrentTick))
	{
		UpdateNPCTalk();
	}
}

void TalkAI::UpdateNPCTalk()
{
	if (m_bHasNonCombatTalk)
		UpdateNPCNonCombatTalkImpl();
}


void TalkAI::UpdateNPCNonCombatTalkImpl()
{	
	if (enNpc == m_pSelf->GetType())
		((CNpc *)m_pSelf)->NPCTalk();
		
	PostponeTalkTimer();
}

void TalkAI::PostponeTalkTimer()
{
	if (!m_pAIConfig) return;
	int nRange = m_pAIConfig->m_nonCombatTalkConfig.nIntervalUpper - m_pAIConfig->m_nonCombatTalkConfig.nIntervalLower+1;
	int nNextHit = wrand(nRange) + m_pAIConfig->m_nonCombatTalkConfig.nIntervalLower;
	m_t2sTalk.SetNextHitTimeFromNow(nNextHit * 1000);
}
