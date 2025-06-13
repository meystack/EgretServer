#pragma once
/*
   主动怪的实现类
*/

#ifdef WIN32
#define PRIORITY_FLAG_ATTACK_MASTER_TARGET (0x8000000000000000ui64)
#else
#define PRIORITY_FLAG_ATTACK_MASTER_TARGET (0x8000000000000000ULL)
#endif

class CAggressorAI:
	public CAnimalAI
{
public:
	typedef CAnimalAI Inherid;

	/**
	* Comments: 检测战斗目标
	* @Return void:
	*/
	virtual bool CheckEnemy();

	/*
	* Comments: 一个实体在自己的视野里移动
	* Param CAnimal *: 实体指针
	* @Return void:
	*/
	virtual void MoveInSight(CAnimal *,INT_PTR nPosX,INT_PTR nPosY );

private:

	UINT64 ComposePriority(bool bOwnerTarget, bool bPrior, int nPriorLevel, int nDist)
	{
		UINT64 result = bOwnerTarget ? (UINT64)PRIORITY_FLAG_ATTACK_MASTER_TARGET : 0;
		if (bPrior)
			result |= ((INT64)1 << 62);		
		result |= (INT64)nPriorLevel << 32;
		result |= nDist;
		return result;
	}

	/* 
	* Comments:计算目标被攻击的优先权值
	* Param CEntity * pTarget:目标对象
	* Param const CVector<int> *priorTargetList： 怪物自己的优选攻击列表
	* Param UINT64& priority:返回被攻击的优先权值。怪物选择目标优选被攻击权值高的
	* Param int nSelfX: 怪物自身位置X坐标
	* Param int nSelfY: 怪物自身位置Y坐标
	* @Return bool: 是合法目标，返回true；否则返回false
	* 按照：怪物配置的优选攻击怪物列表、 被击优先级、 距离
	* 第63位标记是否是拥有者的目标，
	* 第62位标记是否是优选攻击怪物，
	* 第32-61 标记目标被击优先级  
	* 第0-31标记距离
	*/
	bool CalcTargetPrior(CEntity *pTarget, const CVector<int> *priorTargetList, UINT64 &priority, int nSelfX, int nSelfY);

public:
	static EntityVector *s_pVisibleEntityList;	///< 主动怪周围实体列表容器，避免获取怪物周围实体列表频繁的分配释放内存

	CTimer<3000>        m_checkMonsterTimer;    ///< 观察怪物的定时器
};
