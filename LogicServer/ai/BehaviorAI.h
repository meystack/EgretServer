#pragma once

class BehaviorAI
{
public:
	virtual void Initialize(CAnimal *pSelf, PAICONFIG pConfig) = 0;

	virtual void Update(TICKCOUNT nCurrentTick) = 0;
};

class TalkAI : public BehaviorAI
{
public:
	typedef TalkAI Inherited;

	TalkAI() : m_bHasNonCombatTalk(false), m_bHasCombatTalk(false){}
	virtual void Initialize(CAnimal *pSelf, PAICONFIG pConfig);
	virtual void Update(TICKCOUNT nCurrentTick);

protected:
	void UpdateNPCTalk();
	void UpdateNPCNonCombatTalkImpl();
	void PostponeTalkTimer();

private:
	CAnimal			*m_pSelf;				// 实体指针
	PAICONFIG		m_pAIConfig;			// AI配置
	bool			m_bHasNonCombatTalk;	// 是否有非战斗发言
	bool			m_bHasCombatTalk;		// 是否有战斗发言
	CTimer<2000>	m_t2sTalk;				// 发言定时器
};