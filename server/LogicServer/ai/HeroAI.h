#pragma once
//////////////////////////////////////////////////////////////////////////
// 宠物的AI
//
class CHeroAi : public CAnimalAI
{
public:
	typedef CAnimalAI Inherid;

	virtual void  UpdateAI(TICKCOUNT nCurrentTick);	

	virtual void initialize(PAICONFIG pAiconfig);


	//宠物不回归的
	virtual void EnterEvadeMode();
	
	//玩家选择从站立状态到跟随状态，需要选择
	void  OnChangeStayToFollow();

	
protected:
	int m_heroTransferDisSquare; // 宠物离开主人的距离的平方，超过这个将自动传送到主人的身边
	int m_heroBattleFollowDis ; // 宠物离开主人超过这个距离，将向玩家移动
	TICKCOUNT m_followMasterTime; //脱离战斗的时间
};