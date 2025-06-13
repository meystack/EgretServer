
#pragma once

/***************************************************************/
/*  法师的火，法师施放火墙术的时候，在地上释放出一5个火团
/*  火团有持续时间，另外一个人在这个点释放了火以后，这团火就归属为另外一个人了
/***************************************************************/

class CFire:
	public CEntity
{
public:
	typedef CEntity Inherited;

	static int GETTYPE() {return enFire;}

	//初始化
	bool Init(void * data, size_t size);

	//删除的一些内存消耗操作
	void Destroy(); //销毁一个实体

	//执行逻辑循环
	virtual VOID LogicRun(TICKCOUNT nCurrentTime);

	inline INT_PTR GetPropertySize() const {return sizeof(CFireProperty) -sizeof(CPropertySet);}


	 //获取属性集
	 virtual CPropertySet * GetPropertyPtr() {return &m_property ;}

	 virtual  const CPropertySet* GetPropertyPtr() const {return &m_property ;}

	 virtual int GetPropertySetTotalSize() const{ return sizeof(m_property); }

	 //设置过期的时间，从当前往后面多少毫秒
	 void SetExpireTime(unsigned int nMiSecond);

	 //设置下火的主人的handle
	 void SetMasterHandle(EntityHandle  hd)
	 {
		 m_ownerHandle =hd;
	 }
	 
	 /* 
	 * Comments: 设置攻击的参数
	 * Param float fRate: 百分比
	 * Param int nAddValue: 附加的数值
	 * @Return void: 
	 */
	 inline void SetAttackParam( int  nRate, int nAddValue)
	 {
		 m_nAttackRate = nRate;
		 m_nAttackValueAdd     =nAddValue;
	 }

	 /* 
	 * Comments: 创建一团火
	 * Param CScene * pScene:场景的指针 
	 * Param int nPosX: 位置x
	 * Param int nPosY: 位置y
	 * Param CCreature * pMaster:主人的指针 
	 * Param int nModelId: 模型的id
	 * Param unsigned int nTime: 火的存活时间，单位ms
	 * Param int nAttackRate: 攻击的附加的比例
	 * Param int nAttackValue: 攻击的输出
	 * @Return CFire *: 返回指针
	 */
	 static CFire * CreateFire( CScene *pScene, int nPosX,int nPosY,CCreature *pMaster,int nModelId,unsigned int nTime,int nAttackRate,int nAttackValue);
	 
	 //获得主人的指针
	 inline CAnimal *GetMasterPtr();
	
	 //当实体碰撞了火
	 void OnCollideFire(CAnimal *pAnimal);

	

private:
	void OnTimeDamage(CAnimal *pOwner); //定时伤害

	//计算火墙的输出
	void OnFireDamage(CAnimal * pMaster ,CAnimal *pAnimal);

private:
	CFireProperty m_property; //属性集合

	TICKCOUNT   m_expireTime;		 //过期的时间，低于这个过期时间，这个箱子就要删除
	CTimer<3000>   m_attackTimer;    //每3秒烧一次

	//TICKCOUNT  m_attackTime; //上次的攻击时间
	EntityHandle      m_ownerHandle;      //所属人的handle

	//法师的输出的 m_fAttackRate * 魔法攻击 + 附加数值
	int         m_nAttackRate;       //攻击的输出的百分比,万分之1
	int         m_nAttackValueAdd;   //攻击的附加数值
};
