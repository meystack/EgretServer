#pragma once
/*
*动物的AI的基类
*宠物，怪物和NPC都可能执行AI的操作，故在Creature里放一个CCreatureAI的指针
*在执行一定的操作性做一些操作
*/
typedef Handle<BYTE>AiIndex; //ai的指针
#define MAX_RESEL_TARGET_RATE 100

class CAnimalAI
{
public: 
	CAnimalAI()
	{
		m_pSelf =NULL;
		m_pAiConfig =NULL;
		m_wReSelTargetRate = 0;		
		m_bInFollowingStats = false;		
	}

	/*
	* Comments: 初始化一些工作
	* @Return void:
	*/
	virtual void initialize(PAICONFIG pAiconfig);

	/*
	* Comments: 进入初始AI行为
	* @Return void:
	*/
	virtual void EnterInitAI();
	
	/*
	* Comments: 更新怪物的AI
	* Param TICKCOUNT nCurrentTik:当前的tick
	* @Return void:
	*/
	virtual void  UpdateAI(TICKCOUNT nCurrentTick);

	/*
	* Comments: 实体销毁
	* @Return void:
	*/
	void EntityDestroy();



	/*
	* Comments: 开始攻击其他实体
	* Param CCreature * pTarget:目标实体的指针
	* @Return void:
	*/
	virtual void AttackStart(CAnimal * pTarget);
	
	/*
	* Comments:结束攻击其他实体
	* Param CAnimal * pTarget:目标实体的指针
	* @Return void:
	*/
	virtual void AttackStop(CAnimal * pTarget);
	
	/*
	* Comments:受到其他实体的攻击
	* Param CAnimal * pAttacker:攻击者的指针
	* @Return void:
	*/
	virtual void AttackedBy(CAnimal * pAttacker, bool boInherit = true) ;

	/*
	* Comments: 自身刚刚被杀死
	* Param CAnimal *pAttacker: 杀死自己的实体指针
	* @Return void:
	*/
	virtual void KillBy(CAnimal  *pAttacker =NULL) ;

	/*
	* Comments: 杀死另外一个实体
	* Param CAnimal *: 被杀死者的指针
	* @Return void:
	*/
	virtual void KillOther(CAnimal *){}
	
	/*
	* Comments: 一个实体在自己的视野里移动
	* Param CAnimal *: 实体指针
	* @Return void:
	*/
	virtual void MoveInSight(CAnimal *pEntity,INT_PTR nPosX,INT_PTR nPosY ){}
	
	/*
	* Comments: 受到伤害
	* Param CAnimal * pAttacker: 攻击者的指针
	* Param nDamageValue: 伤害的值
	* @Return void:
	*/
	virtual void DamageBy(CAnimal * pAttacker,INT_PTR nDamageValue ){}

	/*
	* Comments: 伤害输出给一个实体
	* Param CAnimal * pTarget: 目标实体
	* Param INT_PTR nDamageValue:伤害的值
	* @Return void:
	*/
	virtual void DamageTo(CAnimal * pTarget,INT_PTR nDamageValue){}

	/*
	* Comments: 进入逃避模式，开始逃避，也就是回归模式
	* @Return void:
	*/
	virtual void EnterEvadeMode();

	/*
	* Comments: 目标是否可见
	* Param CAnimal * pTarget:目标
	* @Return bool: 目标是否可见
	*/
	virtual bool IsVisible(CAnimal * pTarget){return true;}

	/*
	* Comments: 是否需要定时检查ai
	* @Return bool:
	*/
	virtual bool NeedOntimeCheck(TICKCOUNT nCurrentTick);
	
	/*
	* Comments: 检测战斗目标
	* @Return bool:
	*/
	virtual bool CheckEnemy();
	
	/*
	* Comments: 设置怪物被攻击重选目标的概率
	* Param INT_PTR nRate:概率百分比【0-100】。0表示第一次选择目标之后再也不会改变。
	* @Return void:
	*/
	inline void SetReSelTargetRate(INT_PTR nRate)
	{
		if (m_wReSelTargetRate != nRate)
		{
			nRate = __min(nRate, MAX_RESEL_TARGET_RATE);
			m_wReSelTargetRate = (WORD)nRate;			
		}
	}


	/*
	* Comments: 怪物被攻击时检测是否需要改变当前目标为攻击者
	* @Return bool: 需要改变目标就返回true；否则返回false
	*/
	virtual bool NeedReSelTarget();

	/*
	* Comments: 怪物改变目标
	* Param CAnimal * pTarget:
	* @Return void:
	*/
	virtual void ChangeTarget(CAnimal *pTarget);

	/*
	* Comments: 设置跟随状态
	* Param bool bEnter:
	* @Return void:
	*/
	virtual void SetFollowStatus(bool bEnter){
		m_bInFollowingStats = bEnter;
	}

	void RefreshSkillDis();

	//获取移动的停留间隔
	inline int GetMoveStopInterval()
	{
		if(m_pAiConfig)
		{
			return m_pAiConfig->nMoveStopInterval;
		}
		else
		{
			return 0;
		}
	}

	//获取移动路径
	inline DataList<int>& GetPathPoints()
	{
		return m_pAiConfig->pathPoints;
	}

protected:

	/*
	* Comments: 处理进入战斗立刻进入CD的技能
	* @Return void:
	*/
	void HandleSkillCDWhenCombatStart();
	void SetNextCheckEnemyTimer();

public:
	BYTE m_btAiType; 	//AI的类型
	BYTE m_bReserver;   //技能释放的最小的距离
	WORD m_wID;      	//AI的ID
	AiIndex m_handle; 	//ai的指针
	CAnimal * m_pSelf;  //自己的指针

protected:
	//WORD m_wCurrentSkill;			//当前的技能的ID
	WORD m_wMinSkillDis;      		//最小的技能的作用距离
	WORD m_wReserver;       		//保留
	int  m_nSkillIdx;  				//当前技能索引
	
	PAICONFIG  m_pAiConfig;
	CTimer<1000> m_skillTimer; //释放技能的定时器,怪物不能不停的释放技能，释放完以后需要停一段时间才能放	
	WORD	m_wReSelTargetRate;	// 怪物被攻击时重新选择目标的概率，默认为0，表明第一次选择目标后不会再次选择。	
	bool	m_bInFollowingStats;	// 是否处于跟随状态
	CTimer<1000> m_1sCheckEnemyInCombat; // 战斗中检测敌人
	
};
