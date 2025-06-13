#pragma once

//AI管理器,用于创建各种AI实体

enum tagAiType
{
	atAiTypePassive		= 1,	///< 被动怪
	atAiTypeAggressor	= 2,	///< 主动怪
	atAiTypePet			= 3,	///< 宠物怪
	atAiTypeHero		= 4,    ///< 英雄

	// TO DELETE
	atAiTypeRandomFlee	= 6,	///< 随机逃跑怪
	atAiTypeNeutrally	= 7,	///< 中立怪（不会攻击别人）
	atAiTypeNPC			= 8,	///< NPC
	atAiTypeSpecial		= 9,	///< 塔防类
};

class CAiMgr
{
public:
	
	typedef HandleMgr<CAggressorAI,AiIndex,1024> AggressorAIMgr;	///< 主动怪的ai
	typedef HandleMgr<CPassiveAi,AiIndex,1024> PassiveAIMgr;		///< 被动怪的ai
	typedef HandleMgr<CRandomFleeAI, AiIndex,32> RandomFleeAIMgr;	///< 随机逃跑怪的ai	
	typedef HandleMgr<CBaseAI, AiIndex,32> NeutralAIMgr;			///< 中立怪的ai	
	typedef HandleMgr<CNpcAI, AiIndex, 512> NPCAIMgr;				///< NPC AI
	typedef HandleMgr<CPetAi, AiIndex, 512> PETAIMgr;				///< 宠物的AI
	typedef HandleMgr<CHeroAi, AiIndex, 128> HEROAIMgr;				///< 英雄的AI
	typedef HandleMgr<CSpecialAI, AiIndex, 32> SPECIALAIMgr;

	CAiMgr():m_aggressorAiMgr(_T("AggrAIHdlMgr")), m_passiveAiMgr(_T("PassAIHdlMgr")), 
		m_randomFleeAiMgr(_T("RandomFleeAIHdlMgr")), m_neutralAiMgr(_T("NeutralAIHdlMgr")), m_NPCAIMgr(_T("NpcAIHdlMgr")),
		m_petAiMgr(_T("PetAIMgr")),m_heroAiMgr(_T("HeroAIMgr")),m_SpecialAiMgr(_T("SpecialAiMgr"))
	{}
	/*
	* Comments: 创建一个AI对象	
	* Param INT_PTR nAiId:  AI的ID
	* Param CAnimal * pAnimal: ai所有人的指针
	* @Return CAnimalAI *: ai的指针
	*/
	CAnimalAI * CreateAi(INT_PTR nAiId,CAnimal * pAnimal);

	
	/*
	* Comments: 删除一个ai
	* Param CAnimalAI * pAi: ai的指针
	* @Return void:
	*/
	void DestroyAi(CAnimalAI * pAi);


private:
	AggressorAIMgr	m_aggressorAiMgr;	///< 主动怪的ai的管理器
	PassiveAIMgr	m_passiveAiMgr;		///< 被动怪的ai管理器
	RandomFleeAIMgr m_randomFleeAiMgr;	///< 随机逃跑怪物ai
	NeutralAIMgr	m_neutralAiMgr;		///< 中立怪ai
	NPCAIMgr		m_NPCAIMgr;			///< NPC ai管理器
	PETAIMgr		m_petAiMgr;			///< 宠物的ai管理器
	HEROAIMgr       m_heroAiMgr;		///< 英雄的ai管理器
	SPECIALAIMgr	m_SpecialAiMgr;
};
