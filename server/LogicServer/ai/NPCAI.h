#pragma once
//////////////////////////////////////////////////////////////////////////
// NPCAI ： NPC AI实现类
// 目前因为只有针对NPC发言相关的需求,通过NPCAI来实现。后面经讨论说可以借用现有
// 的NPC Idle（实际上也是一个定时执行脚本）来实现。以后有什么NPC相关的AI需求可
// 以在这里开发。
class CNpcAI : public CAnimalAI
{
public:
	typedef CAnimalAI Inherid;

	virtual void  UpdateAI(TICKCOUNT nCurrentTick);	

	virtual void initialize(PAICONFIG pAiconfig);

protected:
	void UpdateNPCTalk();
	void UpdateNPCNonCombatTalkImpl();
	void PostponeTalkTimer();

};