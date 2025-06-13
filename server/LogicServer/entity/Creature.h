
#pragma once

/***************************************************************/
/*
/*                     生物,包括玩家,NPC, 怪物等
/*
/***************************************************************/


class CCreature:
	public CEntity
{
public:
	typedef CEntity Inherited;

	inline bool Init(void * data, size_t size){ m_nDestoryTime = 0;return true;} //创建一个实体

	//例行逻辑准备函数，当例行逻辑函数调用前被调用
	virtual VOID BeforeLogicRun(TICKCOUNT nCurrentTime)
	{
		Inherited::BeforeLogicRun(nCurrentTime);
	}
	//例行逻辑处理函数
	virtual VOID LogicRun(TICKCOUNT nCurrentTime)
	{
		Inherited::LogicRun(nCurrentTime);
	}
	//例行逻辑收尾函数，当例行逻辑函数调用后被调用
	virtual VOID AfterLogicRun(TICKCOUNT nCurrentTime)
	{
		Inherited::AfterLogicRun(nCurrentTime);
	}

	/*
	* Comments: 修改实体的HP
	* Param int nValue: 改变量，可以为正数也可以为负数
	* Param CEntity * pKiller: 杀手的指针
	* @Return VOID:
	*/
	virtual void ChangeHP(int nValue,CEntity * pKiller=NULL,bool bIgnoreDamageRedure=false, bool bIgnoreMaxDropHp=false, bool boSkillResult = false, int btHitType = 0);

	//复活
	virtual void OnEntityRelive(){}
	
	/*
	* Comments:修改实体的MP
	* Param int nValue:改变量，可以为正数也可以为负数
	* @Return VOID:
	*/
	virtual VOID ChangeMP(int nValue)
	{
		nValue += (int) GetProperty<unsigned int >(PROP_CREATURE_MP);
		int nMaxValue = GetProperty<unsigned int >(PROP_CREATURE_MAXMP);
		nValue = nValue < 0 ? 0 : __min( nValue,nMaxValue); //如果小于0，那么就为0
		SetProperty<unsigned int>(PROP_CREATURE_MP,(unsigned int)nValue);
	}

	/*
	* Comments:实体死亡后，需要等待数秒后才清除，这里设置它清除的时间,注意：对于玩家（CActor)来说，这个值表示复活倒计时，超过这个时间，自动回城复活
	* nTime:	死亡后显示的时间，单位：秒
	* @Return VOID:
	*/
	VOID  SetDestoryTime(UINT nTime);

	/*
	* Comments:把m_nDestoryTime置0
	* @Return void:
	*/
	inline void ClearDestoryTime(){m_nDestoryTime = 0;}
	/*
	* Comments:判断是否已经放入场景的死亡列表中
	* @Return bool:
	*/
	inline bool IsInDeathList()
	{
		return m_nDestoryTime != 0;
	}

	
	inline UINT GetDeathTime() {return m_nDestoryTime;}


protected:
	//当生物死亡时，需要等一段时间（数秒后）才从场景中删除,这个变量表示删除的时间
	//初始值是0，在非0并且比现在时间小，表示这个实体可以清除掉了
	UINT	m_nDestoryTime;
	
};


