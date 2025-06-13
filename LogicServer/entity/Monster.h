
#pragma once

/***************************************************************/
/*
/*                     monster实体，怪物，继承于动物，怪物有属性集合
/*
/***************************************************************/

#define MAX_OWNER 2


class CRanking;
class CDropItemEntity;
class CMonster:
	public CAnimal
{
public:
	typedef CAnimal Inherited;

	static int GETTYPE() {return enMonster;}

	//初始化
	bool Init(void * data, size_t size)
	{
		m_nTeamPlayerCnt	= 0;
		if( Inherited::Init(data,size) ==false) return false;		
		InitAi();
		m_liveTime = 0;
		m_BossLiveTime = 0;
		m_hVestEntity = EntityHandle();
		m_nForceVesterId = 0;				//无强制归属玩家
		m_nPreSayType		= mstInvalid;
		m_nPreSayContentIdx = -1;
		m_nOwnerActorId =0;
		m_nAttackedTime = 0;

		InitPriorAttackTarget(m_priorAttackMonsterList);
		//SetAttriFlag
		m_ranking = NULL;

		//memset(&m_sOwner, 0, sizeof(m_sOwner));

		//RegisterMonsterRank();	//注册怪物排行榜	在脚本上必须进行销毁
		m_hasDeathCall =false;
		m_bHasLiveTimeOut	= false;
		m_sVestEntityName[0] = 0;
		m_nVestAttackTime = 0;
		m_nBossReferId = 0;
		m_nBuffPercent = 0;
		return true;		
	}

	//删除的一些内存消耗操作
	void Destroy();
	
	//获取属性集的大小
	inline INT_PTR GetPropertySize() const
	{
		return sizeof(CMonsterProperty) - sizeof(CPropertySet);
	}
	//获取属性集的指针
	virtual CPropertySet * GetPropertyPtr() {return &m_property ;}
	virtual const CPropertySet * GetPropertyPtr() const {return &m_property ;}
	virtual int GetPropertySetTotalSize()const { return sizeof(m_property);}

	virtual bool CanSee(CEntity* pEntity);

	//获取怪物更新的mask
	inline CUpdateMask * GetBroadCastMask() {return s_monsterBroadcastMask;}

	//怪物死亡的时候
	virtual void OnEntityDeath(); 

	void OnEntityDestroy();	

	//例行逻辑处理函数
	virtual VOID LogicRun(TICKCOUNT nCurrentTime);

	virtual void OnKilledByEntity(CEntity * pKillerEntity);


	/*
	* Comments: 怪物被攻击
	* Param CAnimal * pEntity: 攻击者
	* Param bool bSetVest: 是否设置归属
	* @Return void:
	*/
	virtual void OnAttacked(CAnimal * pEntity, bool bSetVest=true);

	//初始化ai
	void InitAi();

	/*
	* Comments:设置怪物的存活时间，单位：秒
	* Param UINT nTime:存活的秒数，从生成开始算
	* @Return void:
	*/
	void SetLiveTime(UINT nTime);

	//重用，重置所有相关属性
	virtual void Reuse();

	/*
	* Comments: 设置怪物的归属者
	* Param EntityHandle handler: 归属者句柄
	* @Return void:
	*/
	void SetVestEntity(const EntityHandle& handler);
		
	/*
	* Comments:查询怪物的归属者
	* @Return EntityHandle:归属者句柄
	*/
	const EntityHandle& GetVestEntity()const;

	void SetVestEntityName(LPCTSTR sName);

	LPCSTR GetVestEntityName();

	/*
	* Comments: 设置怪物的被玩家攻击列表
	* Param unsigned int nActorId: 攻击怪物的玩家ID
	* @Return void:
	*/
	void SetBeAttackActorList(unsigned int nActorId);

	bool InBeAttackActorList(unsigned int nActorId);

	/*
	* Comments:通知附近玩家
	* @Return void:
	*/
	void QuestMonsterToPlayerNearby(CActor* pActor);
	/*
	* Comments:设置怪物的拥有者
	* Param EntityHandle & hHandle:玩家handle
	* @Return void:
	*/
	void SetOwner(EntityHandle hHandle){m_hOwner = hHandle;}

	/*
	* Comments:获取拥有者句柄
	* @Return EntityHandle:
	*/
	inline EntityHandle GetOwner(){return m_hOwner;}

	/*
	* Comments:设置怪物的强制归属
	* Param nForceVesterId 强制归属玩家ID
	* @Return void:
	*/
	void SetForceVesterId(unsigned int nForceVesterId);

	/*
	* Comments:设置怪物的拥有者
	* Param INT_PTR nIndex: index
	* Param LPCTSTR name: 角色name
	* @Return bool: 成功返回true
	*/
	bool SetOwnerName( LPCTSTR sName);
	
	
	//或者所属的actorid
	unsigned int GetOwnerActorId () { return m_nOwnerActorId;}

	//设置主人的actorid
	void SetOnwerActorId(unsigned int nActorId) { m_nOwnerActorId = nActorId;}


	//重设显示名字
	void ResetShowName(LPCTSTR sName);
	
	virtual void ChangeHP(int nValue,CEntity * pKiller=NULL,bool bIgnoreDamageRedure=false, bool bIgnoreMaxDropHp=false,bool boSkillResult = false,int btHitType = 0); 

	/*
	* Comments: 怪物发言
	* Param MonsterSayType sayType: 发言类型
	* Param INT_PTR nSayIdx: 发言索引。对于mstHpLow这种类型发言，可以配置多个，这里需要指定是哪个发言
	* @Return void: 
	* @Note: 读取怪物聊天配置数据进行发言
	*/
	void MonsterSay(MonsterSayType sayType, INT_PTR nSayIdx = 0);

	

	inline int GetMonsterType() { return m_nMonsterType; }

	

	inline int GetFubenTeammemberCount() const { return m_nTeamPlayerCnt; }

	//获的杀怪的经验的等级衰减
	static float GetKillMonsterExpRate(int nActorLevel,PMONSTERCONFIG pMonster);

	//击杀怪物获得怒气值 nLevel怪物的等级
	static int GetKillMonsterAwardAnger(CActor *pActor,PMONSTERCONFIG pMonster);

	/*
	 * Comments:怪物给玩家掉率物品和经验
	 * Param int nMonsterId:怪物的id
	 * Param CActor * pKiller:杀手的指针
	 * Param CScene *pScene:场景的指针
	 * Param int x:位置x
	 * Param int y:位置y
	 * Param iCMonster * pMonster :怪物的指针，喊话的时候需要，空就不能喊话了
	 * @Return void:
	 */
	 static void RealDropItemExp(int nMonsterId,CActor * pKiller, CScene *pScene, int x,int y,CMonster * pMonster =NULL, int nGrowLv = 0);

	/*
	 * Comments:指定位置按掉落组掉落物品
	 * Param int dropGroupId : 掉落组id
	 * Param CScene *pScene:场景的指针
	 * Param int x:位置x
	 * Param int y:位置y
	 * @Return bool 
	 */
	 static bool RealDropItemByDropGroupId(CScene *pScene, int nPosX,int nPosY,int dropGroupId,int pick_time = 120, int nDropTips = 0);

	 /* 
	 * Comments: 被玩家攻击列表
	 * @Return CVector<unsigned int>&:  
	 */
	 CVector<unsigned int>&	GetBeAccactedActorList(){return m_BeAccactedActorList;}

	 void SetVestAttackTime(TICKCOUNT dValue)
	 {
		 m_nVestAttackTime = dValue;
	 }

	 int GetHp()
	 {
		return GetProperty<unsigned int >(PROP_CREATURE_HP);
	 }
	 int GetMaxHp()
	 {
		return GetProperty<unsigned int >(PROP_CREATURE_MAXHP);
	 }
public:
	/******* 静态函数集 ********/
	static VOID InitMonsterBroadcastmask();

	//删除mask
	static void DestroyMonsterBroadcastMask()
	{
		SafeDelete(CMonster::s_monsterBroadcastMask);
	}
	//设置怪物的出生点
	inline void SetBornPoint(INT_PTR nPosX,INT_PTR nPosY)
	{
		m_wBornPointX =(WORD) nPosX;
		m_wBornPointY =(WORD) nPosY;
	}

	//获取怪物的出生点
	inline void GetBornPoint(INT_PTR& nPosX,INT_PTR& nPosY)
	{
		nPosX= m_wBornPointX  ;
		nPosY =m_wBornPointY ; 
	}

	inline unsigned int  GetAttackedTime()
	{
	    return m_nAttackedTime;
	}
	
	const inline CVector<int>& getMonsterPriorAttackList() { return m_priorAttackMonsterList; }

	//获取存活时间
	inline unsigned int GetLiveTime() { return m_liveTime;}

	//获取仇恨值
	INT_PTR GetMonsterHetred(CEntity* pEntity);

	inline unsigned int SetBossReferId(int nBossReferId) { m_nBossReferId = nBossReferId;}
	inline unsigned int GetBossReferId() { return m_nBossReferId;}
	// inline unsigned int SetBuffPercent(int nBuffPercent) { m_nBuffPercent = nBuffPercent;}
	// inline unsigned int GetBuffPercent() { return m_nBuffPercent;}

	bool isHasDrop()
	{
		return m_hasDrop;
	}
	void setHasDrop(bool v)
	{
		m_hasDrop = v;
	}

private: 

	/*
	* Comments:开始掉落物品和经验
	* Param CActor * pActor:杀死怪物的人，如果是宠物杀死的，就给宠物 的主人
	* @Return void:
	*/
	void DropItemExp(CActor * pActor, int nGrowLv);

	 /*
	 * Comments: 给玩家背包添加一个任务物品
	 * Param CActor * pActor: 玩家对象指针
	 * Param CUserItemContainer::ItemOPParam * pItem: 物品对象指针
	 * @Return bool:
	 */
	 bool AddDropTaskItemToVester(CActor *pActor, CUserItemContainer::ItemOPParam *pItem);
	
	 void PostponeMonsterIdleSay();
	 	
	 // 初始化怪物优先攻击列表
	 void InitPriorAttackTarget(CVector<int> &monsterList);

	 //创建怪物排行榜
	 //**必须在脚本上进行销毁
	 void RegisterMonsterRank();

	 //更新怪物排行榜
	 void UpdateMonsterRanking(CEntity *pEntity, INT_PTR nNewVal);
	 //更新仇恨
	 void UpdateMonsterHatred(CEntity *pEntity, INT_PTR nNewVal);
	 unsigned int  m_nAttackedTime;  //被攻击时间
protected:	
	CMonsterProperty	m_property;
	static CUpdateMask *s_monsterBroadcastMask; //那些属性需要发送到客户端
	WORD                m_wBornPointX; //出生点x
	WORD                m_wBornPointY; //出生点y
	UINT				m_liveTime;	//存活时间，默认是0，代表无限期（这个是到期时间）

	int					m_BossLiveTime;		//boss的存活时间
	EntityHandle		m_hVestEntity;	// 归属者
	unsigned int		m_nForceVesterId;	// 强制归属者（只要怪物死亡时，玩家在线，掉落就归他）	

	EntityHandle		m_hOwner;	// 任务怪物的拥有者，即为完成某任务而动态刷出来的怪物，其必须被拥有者kill了才执行相关逻辑
	//ACTORNAME			m_sOwner[MAX_OWNER];	//怪物拥有者
	unsigned int        m_nOwnerActorId;  //所属的玩家的角色



	bool					m_bHasHPChangeSayConfig;// 是否怪物配置了血量变化的发言配置
	bool 					m_hasDrop;//是否掉落物品
	TICKCOUNT				m_nNextIdleSayTime;		// 怪物Idle状态下次发言时间
	MonsterSayType			m_nPreSayType;			// 怪物上次发言类型
	int						m_nPreSayContentIdx;	// 上次发言内容索引，避免连续两次发言相同
	int						m_nMonsterType;			// 怪物类型
	//int					m_nAttackType;			// 攻击类型
	int						m_nTeamPlayerCnt;		// 怪物所在的副本队伍人数，用于副本怪物属性调整
	CTimer<500>				m_500Timer;				// 500ms定时器
	CTimer<5000>			m_t5s ;					//定时定量回血定时器
	CTimer<1000>			m_updateFBMonsterProp;	// 副本怪物属性动态调整定时器
	CVector<int>			m_priorAttackMonsterList; // 优先攻击怪物列表
	CRanking*				m_ranking;				//创建怪物触发排行榜
	bool					m_hasDeathCall;       //是否调用了死亡回调，在init的时候调用一下，防治
	CVector<unsigned int>	m_BeAccactedActorList;	//被攻击玩家的列表（此列表应该不会很长）
	CVector<MONSTERHATRED>  m_hatredList;		//仇恨列表
	bool					m_bHasLiveTimeOut;		//已经处理生命期到的逻辑
	char					m_sVestEntityName[64];	
	TICKCOUNT				m_nVestAttackTime;
	int                     m_nBossReferId;//boss刷新id
	int                     m_nBuffPercent = 0;//boss刷新id
};
