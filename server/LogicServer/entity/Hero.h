
#pragma once

/***************************************************************/
/* 英雄的类，玩家的宠物，实际存在的宠物
/***************************************************************/

class CHero:
	public CAdvanceAnimal
{
public:
	typedef CAdvanceAnimal Inherited;

	static int GETTYPE() {return enHero;}

public:
	
	const static int Hero_BATTLE_STATUE_STAY =1;		 //呆在原地
	const static int Hero_BATTLE_STATUE_FOLLOW =2;	//跟随

	CHero();
	//初始化
	bool Init(void * data, size_t size)
	{  
	
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
		return sizeof(CHeroProperty) - sizeof(CPropertySet);
	}
	//获取属性集的指针
	virtual CPropertySet * GetPropertyPtr() {return &m_property ;}
	virtual const CPropertySet * GetPropertyPtr() const {return &m_property ;}

	virtual int GetPropertySetTotalSize() const{return sizeof(m_property);}

	//获取怪物更新的mask
	inline CUpdateMask * GetBroadCastMask() {return s_pHeroMask;}

	/******* 静态函数集 ********/
	static void InitHeroBroadcastmask();

	//删除mask
	static void DestroyHeroBroadcastMask()
	{
		SafeDelete(CHero::s_pHeroMask);
	}

	//设置宠物的主人
	inline void SetMaster(CActor *pActor)
	{
		m_pMaster =pActor;
	}

	//设置宠物的ID
	//inline void SetHeroId(int nHeroId) {m_nId =nHeroId; } 


	//获取宠物的主人
	inline CActor * GetMaster(){return m_pMaster;}

	//获取宠物的id
	//inline INT_PTR GetHeroId(){ return m_nId;}

	virtual void OnEntityDeath(); //死亡

	//获取宠物的指针
	//获取宠物的指针
	const CHeroSystem::HERODATA * GetHeroPtr();
	
	void  InitAi();

	
	
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
	void SetLevel(unsigned int nLevel)
	{
		Inherited::SetLevel(nLevel);
	}

	virtual void OnLevelUp(int nUpdateVal);

	inline bool SetSocialMask(INT_PTR nMaskID,bool flag)
	{
		unsigned int nMask = GetProperty<unsigned int >(PROP_ACTOR_SOCIALMASK);
		if(flag)
		{
			nMask |= (1<< nMaskID);
		}
		else
		{
			nMask &= ~( 1<< nMaskID ); //取反
		}
		SetProperty<unsigned int >(PROP_ACTOR_SOCIALMASK,nMask );

		return true;
	}
protected:
	CHeroProperty	m_property;
	static CUpdateMask *s_pHeroMask; //怪物的属性的mask

	CActor * m_pMaster;   //宠物的主人的指针
	//int  m_nId;      //宠物的ID
	int m_nExp; //当前的经验
	int m_nMaxExp; //最大的经验  
	unsigned int m_nColor; //宠物的颜色
	int m_nBattleStatus; //战斗状态
	
};
