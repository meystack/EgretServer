#pragma once
//////////////////////////////////////////////////////////////////////////
/// 宠物的AI
class CPetAi : public CAnimalAI
{
public:
	typedef CAnimalAI Inherid;

	virtual void  UpdateAI(TICKCOUNT nCurrentTick);	

	virtual void initialize(PAICONFIG pAiconfig);

	/// 宠物不回归的
	virtual void EnterEvadeMode();
	
	/// 玩家选择从站立状态到跟随状态，需要选择
	void  OnChangeStayToFollow();

	/// 被攻击
	virtual void AttackedBy(CAnimal * pAttacker, bool boInherit = true);

	std::map<Uint64,CEntity*>& GetAvoidMap(){return m_avoidMap;}
	bool IsInAvoidMap(Uint64 handle) {return m_avoidMap.find(handle) != m_avoidMap.end();}
	void AddAvoidMap(CEntity* pEntity)
	{ 
		if (pEntity)
		{
			m_avoidMap[pEntity->GetHandle()] = pEntity;
		}
	}
	void RmvAvoidMap(Uint64 handle){
		typedef std::map<Uint64,CEntity*>::iterator Itr;
		Itr iter = m_avoidMap.find(handle);
		if (iter != m_avoidMap.end())
		{
			m_avoidMap.erase(iter);
		}
	}
	void ClearAvoidMap() {m_avoidMap.clear();}

protected:
	int m_petTransferDisSquare; 	///<  宠物离开主人的距离的平方，超过这个将自动传送到主人的身边
	int m_petBattleFollowDis ; 		///<  宠物离开主人超过这个距离，将向玩家移动
	TICKCOUNT m_followMasterTime; 	///< 脱离战斗的时间
	std::map<Uint64,CEntity*> m_avoidMap;
	CTimer<60000>			m_t1minute ;
};
