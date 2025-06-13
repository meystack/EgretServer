
#pragma once

/***************************************************************/
/* 宠物的类，玩家的宠物，实际存在的宠物
/***************************************************************/

class CPet:
	public CAdvanceAnimal
{
public:
	typedef CAdvanceAnimal Inherited;

public:
	
	const static int PET_BATTLE_STATUE_STAY =1;		 //休息
	const static int PET_BATTLE_STATUE_FOLLOW =2;	//战斗

	static int GETTYPE() {return enPet;}

	CPet();
	//初始化
	bool Init(void * data, size_t size)
	{  
		m_pMaster =NULL;
		m_nId =0;
		m_nMaxExp =0;
		m_nExp =0;
		m_nColor =0;
		m_nLiveTime = 0;
		m_boNoticeOnce = false;
		m_nBattleStatus = PET_BATTLE_STATUE_FOLLOW;
		
		m_boAlwaysMoveFlag = false;
		return Inherited::Init(data,size);
	} 
	//删除的一些内存消耗操作
	inline void Destroy()
	{ 
		Inherited::Destroy(); 
	}
	//处理LogicRun，实现对脚本的内存定时回收
	void LogicRun(TICKCOUNT nCurrentTime);
	
	//处理实体消息
	void ProcessEntityMsg(const CEntityMsg &msg);
	
	//获取属性集的大小
	inline INT_PTR GetPropertySize() const
	{
		return sizeof(CPetProperty) - sizeof(CPropertySet);
	}
	//获取属性集的指针
	virtual CPropertySet * GetPropertyPtr() {return &m_property ;}
	virtual const CPropertySet * GetPropertyPtr() const {return &m_property ;}

	virtual int GetPropertySetTotalSize()const { return sizeof(m_property);}

	//获取怪物更新的mask
	inline CUpdateMask * GetBroadCastMask() {return s_pPetMask;}

	/******* 静态函数集 ********/
	static void InitPetBroadcastmask();

	//删除mask
	static void DestroyPetBroadcastMask()
	{
		SafeDelete(CPet::s_pPetMask);
	}

	//设置宠物的主人
	void SetMaster(CActor *pActor);

	//获取宠物的主人
	inline CActor * GetMaster(){return m_pMaster;}

	void OnKilledByEntity(CEntity * pKillerEntity);

	//设置宠物的ID
	inline void SetPetId(int nPetId) {m_nId =nPetId; } 

	//获取宠物的id
	inline INT_PTR GetPetId(){ return m_nId;}

	virtual void OnEntityDeath(); //死亡

	//获取宠物的指针
	//获取宠物的指针
	//const CPetSystem::PETDATA * CPet::GetPetPtr();
	
	void  InitAi( int nAiId );

	virtual void ChangeHP(int nValue,CEntity * pKiller=NULL,bool bIgnoreDamageRedure=false, bool bIgnoreMaxDropHp=false, bool boSkillResult = false,int btHitType = 0)
	{
		return Inherited::ChangeHP(nValue,pKiller,bIgnoreDamageRedure,bIgnoreMaxDropHp,boSkillResult, btHitType);
	}
	
	
	//宠物增加经验，杀一个怪增加一点经验，满了就升级
	void  AddExp(int nValue);
	
	//设置当前的
	void SetCurrentExp(unsigned int nValue);

	//设置颜色
	void SetColor(unsigned int color);
	
	//获取颜色
	unsigned int GetColor (){return m_nColor;}

	//设置战斗的状态
	void SetBattleStatus (int nStatus);

	//获取战斗状态
	inline int GetBattleStatus() {return m_nBattleStatus; }
	
	//设置宝宝的等级
	void SetLevel(unsigned int nLevel);

	virtual void OnLevelUp(int nUpdateVal);

	/*
	* Comments:设置宠物生存时间
	* Param int nSecs:生存秒数
	* @Return bool:成功返回true
	*/
	bool SetLiveTime(int nSecs);

	/*
	* Comments:获取生存时间 如果是大于0，表示偶遇宠物
	* @Return unsigned int:
	*/
	inline unsigned int GetLiveTime()
	{
		return m_nLiveTime;
	}

	inline bool IsLiveTime(unsigned int nNow)
	{
		if (m_nLiveTime > 0)
		{
			return m_nLiveTime > nNow;
		}
		return true;
	}

	inline void SetNoticeOnce( bool boResult )
	{
		m_boNoticeOnce = boResult;
	}

	inline bool GetNoticeOnce() { return m_boNoticeOnce;}
	inline void SetInheritPercent(int nInheritPercent){m_nInheritPercent = nInheritPercent;}
	inline int GetInheriPercent(){return m_nInheritPercent;}
	inline bool GetAlwaysMoveFlag(){return m_boAlwaysMoveFlag;}
	inline void SetAlwaysMoveFlag(bool flag){m_boAlwaysMoveFlag = flag;}

protected:
	CPetProperty	m_property;
	static CUpdateMask *s_pPetMask; //怪物的属性的mask

	CActor * m_pMaster;   //宠物的主人的指针
	int  m_nId;      //宠物的ID
	int m_nExp; //当前的经验
	int m_nMaxExp; //最大的经验  
	unsigned int m_nColor; //宠物的颜色
	int m_nBattleStatus; //战斗状态
	unsigned int m_nLiveTime;
	int m_nInheritPercent;//继承主人属性百分比
	bool m_boNoticeOnce;

	bool m_boAlwaysMoveFlag;//这个处理宠物一直等玩家停下才寻路的问题
    CTimer<3000> m_3sTimer;
};
