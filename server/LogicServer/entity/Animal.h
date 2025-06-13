
#pragma once

/***************************************************************/
/*
/* 动物,继承于生物，动物有Buff系统，背包系统，观察者子系统,移动子系统 
/*
/***************************************************************/

//附加的战斗属性，放在这里，便于管理
struct CreatureBattleProp
{

	int                     m_selfAttackAppend; ///< 技能攻击的时候 攻击伤害追加n点（以固定值的方式影响角色造成的内功与外功攻击伤害）

	float					m_deductDamagePower;	//伤害低扣倍率

	int                     m_nAddPhysicalDamageRate;       ///< 物理攻击时候附加物理攻击输出的概率,整数，1点表示万分之一
	int                    	m_nAddPhysicalDamageValue;      ///< 物理攻击时附加攻击输出的数值增加
	int                     m_hp2MpRate;                    ///< 护身效果

	unsigned int            m_nDizzyValue;                  ///< 麻痹的概率，单位1%
	unsigned int            m_nDeductDizzyValue;            ///< 抗麻痹概率，万分比
	unsigned int            m_nDizzyTimeAdd;              	///< 麻痹时长增加 万分比
	unsigned int            m_nDizzyTimeAbsorbAdd;        	///< 麻痹时长减免 万分比
	unsigned int            m_nGuardRate;                  	///< 守护概率，万分比
	unsigned int            m_nGuardValue;                  	///< 守护值
	unsigned int            m_nCuttingRate;                 ///< 切割概率，万分比
	unsigned int            m_nDoubleAtkRate;                 ///< 倍攻率，万分比

	int                     m_DieRefreshHpPro;              ///< 死亡以后立刻回复的HP的比例,1点表示百分之1(复活戒指使用)
	TICKCOUNT               m_nextHpReliveCd;               ///< 下一个死亡复活的cd时间

	WORD                    m_nFireDefenseRate;				///< 抗火率 使用1点表示1万分之1

	float                   m_fDamgeAbsorbRate;             ///< 伤害减免比例

	int						m_nWarriorTargetDamageValue;               //固定值增加对战士的伤害
	int						m_nWarriorTargetDamageRate;                //百分比增加对战士的伤害

	int		m_nMagicianTargetDamageValue;             //固定值增加对法师的伤害
	int		m_nMagicianTargetDamageRate;			   //固定比增加对法师的伤害

	int		m_nWizardTargetDamageValue;             //固定值增加对道士的伤害
	int		m_nWizardTargetDamageRate;			   //固定比增加对法师的伤害

	int		m_nMonsterTargetDamageValue;             //固定值增加对怪物的伤害
	int		m_nMonsterTargetDamageRate;			   //固定比增加对怪物的伤害

	unsigned int		m_nAddAllDamageRate;					//全职业攻击伤害追加概率
	unsigned int		m_nAddAllDamageRateEx;					//会心一击追加概率 

	unsigned int		m_nCritDamageRate;			//新版暴击几率，万分比，区别于m_nAddAllDamageRate
	unsigned int		m_nCritDamageValue;			//新版暴击力,int
	unsigned int		m_nCritDamagePower;			///新版暴击倍率 万分比 int
	unsigned int		m_nDeductCritDamageRate;	//抗暴
	
	float	m_fAddAllDamagePower;					//全职业攻击时附加攻击输出的倍率增加

	unsigned int		m_nDamageReboundRate;					//伤害反弹 概率
	float	m_fDamageReboundPower;					//伤害反弹 倍率 

	int     m_nIgnorDefenceRatio;	   //无视防御的比例
	int     m_nDamageAbsorbRatio;  //伤害减免 比例 ---暂时没用

	int     m_nPkDamageAbsorbRatio;  //pk伤害减免 比例
	int     m_nSuckBloodRatio;       //吸血比例
	int		m_nSaviorTime;			 //救主灵刃-CD时间
	int 	m_nSaviorRate;			 //救主灵刃-回复率
	int 	m_nSaviorValue;			 //救主灵刃-回复值
	int 	m_nToxicRate;			 //剧毒裁决-淬毒几率
	int 	m_nToxicDamage;			 //剧毒裁决-淬毒伤害
	int 	m_nToxicEffect;			 //剧毒裁决-效果
	int     m_nFrozenStrength;		 //冰冻强度
	int		m_nHpRenewAdd;			 //生命恢复值增加

	CreatureBattleProp()
	{
		memset(this,0,sizeof(*this));
	}
};


#ifdef _DEBUG
extern std::string s_HurtValueMsg;
extern char s_HurtValueBuff[100];
#endif

static LPCTSTR  szJZLRCDTime	= "JZLRCDTime";		//救主灵刃CD时间
static const int nJZBuffId = 244;

class CAnimal:
	public CCreature,
	public CreatureBattleProp
{
public:	

	typedef CCreature Inherited;
	static const int BATTLE_LEFT_TIME = 10000; ///< 战斗脱离时间
	CAnimal():Inherited()
	{
		m_pAI =NULL;
		m_stateMask =0;
		m_attackInterval =0;	///< 攻击的间隔，这个是不会改的，不作为属性
		m_maxDropHp =0;			///< 每次掉的最大的血
		
		m_nCamp =0;
		m_nAttackType =0;	
		m_battleLeftTime =0;
		m_killHandler =0;
		m_AttackerHandler = 0;
		m_GuildId = 0;
		m_nAttackLevel =1;		///< 攻击优先级
		m_MonsterDieHpPro = 0;
		m_nDizzyBuffInterval = 0;
		m_nDizzyTime = 0;
		m_nJZLRCDTime = 0;
		m_nToxicAttckValue = 0;
		m_nSZASAttckValue = 0;
	}

	bool Init(void * data, size_t size); ///< 初始化
	inline CObserverSystem* GetObserverSystem()	{return &m_observerSystem;} ///< 获取观察者子系统
	inline  CMovementSystem* GetMoveSystem()	{return &m_moveSystem;}		///< 获取移动子系统
	inline CBuffSystem *	GetBuffSystem()		{return &m_buffSystem ;}	///< Buff子系统
	
	inline CSkillSubSystem & GetSkillSystem()	{return m_skillSystem;}		///< 获取技能子系统
	inline CPropertySystem &GetPropertySystem()	{return m_propertySystem;}	///< 属性子系统
	inline CEffectSystem &	GetEffectSystem()	{return m_effectSystem;}	///< 特效子系统 TO DELETE
	
	inline void Destroy()
	{
		ClearAllEntityMsg();
		
		m_observerSystem.Destroy();
		m_moveSystem.Destroy();
		m_propertySystem.Destroy();
		m_buffSystem.Destroy();
		
		m_skillSystem.Destroy();
		DestroyAi();
		m_effectSystem.Destroy();
		Inherited::Destroy();
	}

	//设置汇总处理标记
	inline void CollectOperate(const CEntityOPCollector::CollecteOPType eType)
	{
		m_OPCollector.Collect(eType);
	}
	//进行一项汇总收集的操作
	virtual VOID DoCollectedOperation(CEntityOPCollector::CollecteOPType eOPType);

	/******* 覆盖父类的函数集 *******/
	//例行逻辑准备函数，当例行逻辑函数调用前被调用
	virtual VOID BeforeLogicRun(TICKCOUNT nCurrentTime);
	//例行逻辑处理函数
	virtual VOID LogicRun(TICKCOUNT nCurrentTime);
	//例行逻辑收尾函数，当例行逻辑函数调用后被调用
	virtual VOID AfterLogicRun(TICKCOUNT nCurrentTime);

	//处理实体消息
	void ProcessEntityMsg(const CEntityMsg &msg);

	//TODELETE 能否攻击 pPet:如果存在则target为主人
	bool _CanAttack(CAnimal *pTargetEntity, CAnimal *pPet=NULL, bool boAttackNotice = true);

	//能否攻击
	bool CanAttack(CAnimal *pTargetEntity, CAnimal * = NULL, bool = true);

	bool IsFriend(CAnimal *pTargetEntity);
	

	/*
	* Comments: 修改实体的HP
	* Param int nValue: 改变量，可以为正数也可以为负数
	* @Return VOID:
	*/
	virtual void ChangeHP(int nValue,CEntity * pKiller=NULL,bool bIgnoreDamageRedure=false, bool bIgnoreMaxDropHp=false, bool boSkillResult = false, int btHitType = 0);

	/*
	* Comments:修改实体的MP
	* Param int nValue:改变量，可以为正数也可以为负数
	* @Return VOID:
	*/
	virtual VOID ChangeMP(int nValue); 
	
	//当等级提升的时候
	virtual void OnLevelUp(int nUpdateVal);
	
	virtual void OnEntityDeath();
	

	//复活
	virtual void OnEntityRelive();

	/*
	* Comments: 被其他实体杀死的时候调用
	* Param CEntity * pKillerEntity:  杀死怪物的实体
	* @Return void:
	*/
	virtual void OnKilledByEntity(CEntity * pKillerEntity)
	{
		if(pKillerEntity && pKillerEntity->IsInited())
		{
			m_killHandler = pKillerEntity->GetHandle();
		}
	}

	//获取一个bool 标记位，免得写很多垃圾代码
	inline static bool GetFlag(unsigned int nValue,int nBitId)
	{
		return (nValue & (0x1 << nBitId) ) ? true:false;
	}

	//设置一个bit位
	template<class T>
	inline static void SetFlag(T &nValue,int nBitId,bool flag)
	{
		T newValue;
		if(flag)
		{
			newValue = (T)(0x1 << nBitId);
			nValue |= newValue;
		}
		else
		{
			newValue = (T)(~(0x1 << nBitId));
			nValue &= newValue;
		}
	}

	//重刷一下状态的禁止位
	inline void  RefreshStateMask()
	{
		if( CAnimal::s_stateForbidMask ==NULL) return;
		m_stateMask =0;
		for(INT_PTR i =0 ; i < esMaxStateCount; i++)
		{
			if(HasState(i))
			{
				m_stateMask |= CAnimal::s_stateForbidMask[i];
			}
		}
	}
	

	/*
	* Comments:是否有这个状态
	* Param INT_PTR nStateID:状态ID
	* @Return bool:有返回true，否则返回false
	*/
	inline bool HasState(INT_PTR nStateID)
	{
		if(nStateID <0 || nStateID >=esMaxStateCount) return false;
		return !!(GetProperty<unsigned int>(PROP_CREATURE_STATE) & (0x1 << nStateID) );
	}

	/*
	* Comments: 设置实体的状态，实体的状态是按位存储的
	* Param INT_PTR nStateID:状态id，比如死亡，骑乘等
	* @Return bool:成功返回true,否则返回false
	*/
	inline bool AddState(INT_PTR nStateID)
	{
		if(nStateID <0 || nStateID >=esMaxStateCount) return false;
		if(HasState(nStateID) ) return false; //已经有这个状态了
		unsigned int nMask = 0x1 << nStateID;
		nMask |= GetProperty<unsigned int>(PROP_CREATURE_STATE);
		SetProperty<unsigned int>(PROP_CREATURE_STATE,nMask);
		RefreshStateMask();
		if(nStateID == esStateBattle)
		{
			OnEnterBattle();
		}
		return true;
	}

	//进入战斗状态
	void OnEnterBattle();
	
	/*
	* Comments:去除实体的状态
	* Param INT_PTR nStateID: 成功返回true，否则返回false
	* @Return bool:成功返回true，否则返回false
	*/
	inline bool RemoveState(INT_PTR nStateID)
	{
		if(nStateID <0 || nStateID >=esMaxStateCount) return false;
		if(!HasState(nStateID )  ) return false; //没有这个状态
		unsigned int nMask = 0x1 << nStateID;
		nMask =~ nMask; //取反
		nMask &=  GetProperty<unsigned int>(PROP_CREATURE_STATE);
		SetProperty<unsigned int>(PROP_CREATURE_STATE,nMask);
		RefreshStateMask();
		return true;
	}


	/*
	* Comments:去除实体的麻痹状态
	* @Return bool:成功返回true，否则返回false
	*/
	inline bool RemoveDizzyState()
	{
		SetProperty<unsigned int>(PROP_CREATURE_DIZZY_STATUS,0);
		RefreshStateMask();
		return true;
	}

	/*
	* Comments:设置实体的麻痹状态
	* @Return bool:成功返回true，否则返回false
	*/
	inline bool SetDizzyState()
	{
		int nValue = GetProperty<unsigned int>(PROP_CREATURE_DIZZY_STATUS);
		if(nValue <= 0)
			nValue = 10000;
		nValue--;
		SetProperty<unsigned int>(PROP_CREATURE_DIZZY_STATUS,nValue);
		RefreshStateMask();
		return true;
	}
	//麻痹状态
	inline bool GetDizzyState()
	{
		return GetProperty<unsigned int>(PROP_CREATURE_DIZZY_STATUS);
	}

	//获取阵营的id
	inline int GetCampId()
	{
		if(GetType() == enActor)
		{
			return GetProperty<UINT>(PROP_ACTOR_ZY);
		}
		else
		{
			return m_nCamp;
		}
	}

	void SetCamp(int nCampId);


	inline void SetNpcGuildId(unsigned int nGuildId) {m_GuildId = nGuildId;}

	inline unsigned int GetNpcGuildId() {return m_GuildId;}

	//获取AI的指针 
	inline CAnimalAI * GetAI()
	{
		return m_pAI;
	}

	//被别人攻击 bSetVest:是否设置归属
	virtual void OnAttacked(CAnimal * pEntity, bool bSetVest=true);
	
	//攻击别人 bChgDura:是否改变装备耐久
	virtual void OnAttackOther(CAnimal * pEntity, bool bChgDura=true);
	
	//取附近的玩家列表
	void GetNearActorList(CVector<EntityHandle> &vecEntityList);

	//取场景的玩家的列表
	void GetSceneActorList(CScene *pScene, CVector<EntityHandle> &vecEntityList);

	//取副本的玩家列表
	void GetFubenActorList(CVector<EntityHandle> &vecEntityList);

	//世界的玩家列表
	void GetWorldActorList(CVector<EntityHandle> &vecEntityList);

	//获取阵营的玩家的列表
	void GetCampActorList(CVector<EntityHandle> &vecEntityList);

	
	/*
	* Comments:说话
	* Param MonsterSayBCType nStyle:说话的类型
	* Param const char * pMsg:说话内容
	* Param tagTipmsgType tipType: 说话提示类型。指定说话内容在客户端如何显示。默认在聊天频道显示
	* Param nLimitLev:显示此内容的玩家等级限制
	* @Return void:
	*/
	void Say(MonsterSayBCType nStyle, const char *pMsg, int tipType = ttChatWindow, unsigned int nLimitLev = 0);

	/*
	* Comments: 推迟观察者系统刷新时间
	* Param INT_PTR nTime: 推迟的时间，单位是毫秒
	* @Return void: 此功能用于希望延迟或提前刷新可见列表的场合，例如跳跃时希望在跳跃完成前不刷新观察者列表
	*/
	inline void PostponeRefViewTime(INT_PTR nTime){ m_tRefViewTimer.SetNextHitTimeFromNow(nTime); }

	//初始化状态静止表
	static void InitStateForbidMask();
	
	//消耗状态表
	static void DestroyStateForbidMask();

	//能否进入一个状态,状态定义在enum tagEntityState定义
	inline bool CanEnterState(INT_PTR nStateID)
	{
		return !GetFlag(m_stateMask,(int)nStateID);
	}

	/*
	* Comments: 检测公共操作的定时器,如果失败了会发一个包给客户端通知操作失败
	* Param TICKCOUNT nTick: 时间
	* Param bool bFailSendmsg:如果检测失败是否发送消息
	* Param bool bMoveOp: 是否是移动操作。如果是移动操作，不用每步检测，目的是保证走路的流畅性
	* @Return bool: 检测通过返回true,否则返回false
	*/
	bool CheckCommonOpTick(TICKCOUNT nTick, bool bFailSendmsg =true, bool bMoveOp = false);
	
	// 玩家的m_tOpsTimer因未知原因导致数值极大，从而造成发射技能在公共CD检查时失败
	bool CheckCommonOpTickBefore(TICKCOUNT nTick);

	/*
	* Comments:设置下次操作的时间间隔
	* Param INT_PTR nNextTime: 下次可以操作的时间
	* Param bool bSendResult2Client: 是否需要发包告诉客户端操作结果
	* Param bool bResult: 操作的结果
	* Param unsigned int nStep: 对于移动操作，保存移动步长，对其他情况，都为0
	* Param unsigned int nSendPkgTick: 客户端发包时间，只针对移动操作	
	* Param bool bUseGateTime: 为true表明是使用网关收到数据包时间来对时；否则使用逻辑服当前时
	* Param bool bUseCheckCd: 是否需要检测cd
	* Param bool bNearAttackSuc: 近战攻击是否成功
	* @Return void:
	*/
	void SetCommonOpNextTime(INT_PTR nNextTime,bool bSendResult2Client =false,bool bResult =true, 
		unsigned int nStep = 0, 
		unsigned int nSendPkgTick = 0,
		bool bUseGateTime = true,
		bool bUseCheckCd=true,
		bool bNearAttackSuc = true);



	void DestroyAi();

	//怪物重用时，重置所有相关属性
	virtual void Reuse();

	/*
	* Comments: 获取实体的战斗等级
	* @Return int:
	*/
	inline int GetAttackLevel()
	{
		return m_nAttackLevel;
	}

	//设置攻击等级 攻击等级，用于决定怪物被攻击的优先等级,玩家是100，等级越高的越容易被攻击，数值没有上限
	inline void SetAttackLevel(int nLevel)
	{
		m_nAttackLevel = nLevel;
	}

	/*
	* Comments: 脚本延迟消息处理
	* Param unsigned int nMsgIdx: 消息编号。标识此玩家消息号的唯一标识。
	* Param int nParam1: 参数
	* Param int nParam2: 参数
	* Param int nParam3: 参数
	* Param int nParam4: 参数
	* Param int nParam5: 参数
	* @Return void:
	* @Note: 
	*/
	virtual void OnEntityScriptDelayMsgCallback(unsigned int nMsgIdx, int nParam1, int nParam2, int nParam3, int nParam4, int nParam5){}

	inline void SetObserverSysNextHitTime(TICKCOUNT nTickCount)
	{
		//m_tRefViewTimer.SetNextHitTime(nTickCount);
	}

	inline int GetDistance(int nPosX,int nPosy)
	{
		int nCurPosx,nCurPoyY;
		GetPosition(nCurPosx,nCurPoyY);
		nCurPosx -= nPosX;
		nCurPoyY -= nPosy;
		double nDis = nCurPosx*nCurPosx + nCurPoyY * nCurPoyY;
		return (int)(sqrt (nDis));
	}

//////////////////////////////////下面这些函数是战斗用的
	//设置获取攻击间隔
	inline void SetAttackInterval(int nInterval){m_attackInterval = nInterval;}
	inline int   GetAttackInterval(){return m_attackInterval;}

	//设置和读取每次掉落的maxHP
	inline int   GetMaxDropHp() {return m_maxDropHp; }
	inline void  SetMaxDropHp(int nValue){ m_maxDropHp = nValue; }

	/* 
	* Comments:获取抵扣伤害的百分比
	* @Return float:
	*/
	inline float GetDeductDamagePower()
	{
		return m_deductDamagePower;
	}

	/* 
	* Comments:设置抵扣伤害的百分比
	* Param float rate:新的百分比
	* @Return void:
	*/
	inline void SetDeductDamagePower(float rate)
	{
		m_deductDamagePower = rate;
	}

	//设置/获取 物理攻击时候附加物理攻击输出的概率,整数，1点表示万分之一
	// inline void SetAddPhysicalDamageRate(int nValue) {m_nAddPhysicalDamageRate =nValue;}
	// inline int GetAddPhysicalDamageRate() {return m_nAddPhysicalDamageRate ;}
	//设置/获取 物理攻击时附加攻击输出的数值增加
	// inline void SetAddPhysicalDamageValue(int nValue) {m_nAddPhysicalDamageValue =nValue;}
	// inline int GetAddPhysicalDamageValue() {return m_nAddPhysicalDamageValue ;}

	//血转换为蓝的比例
	inline void SetHp2MpRate( int nValue) {m_hp2MpRate =nValue; }
	inline int GetHp2MpRate () { return m_hp2MpRate;}

	inline float GetFastMedicamentRenew()
	{
		return (((float)GetProperty<int>(PROP_ACTOR_MEDIC_RENEW))/10000.0);
	}

	
	//更改名字
	void ChangeName(const char* sName);

	void ChangeShowName(int nRadius, LPCTSTR sName);
	
	/*
	* Comments:改变自己视野中实体的模型
	* Param int nRadius:半径范围
	* Param int nModelId:新的模型id
	* @Return void:
	*/
	void ChangeModel(int nRadius, int nModelId);

	//获取攻击类型
	inline int GetAttackType() { return m_nAttackType; }

	//设置攻击类型
	void SetAttackType(int nType,bool bNeedBroadCast =false);
	
	//获取离开战斗的时间
	inline TICKCOUNT GetLeftBattleTickCount(){return m_battleLeftTime;}

	//设置击杀自己的人的handle
	inline void  SetKillerHandler(EntityHandle hd)
	{
		m_killHandler = hd;
	}

	//获取杀死者的指针
	inline EntityHandle& GetKillHandle( ) {return m_killHandler;}

	void SetDizzyTime(int time);
	inline void  SetAttackerHandler(EntityHandle hd) { m_AttackerHandler = hd; }
	inline EntityHandle& GetAttackerHandle( ) {return m_AttackerHandler;}

	//更新救主灵刃特效以及当前CD时间
	void UpdateJZLR();
	/*
	* Comments:获取攻击值，幸运，诅咒决定攻击的输出
	* Param INT_PTR nAttackType:攻击的类型，物理，魔法，道术攻击
	* @Return int:
	*/
	int GetAttackValue(INT_PTR nAttackType);

	//死亡以后立刻回复的HP的比例
	void SetDieRefreshHpPro(int nValue) {m_MonsterDieHpPro = nValue;}
	
	//获取公共操作下一次tick
	TICKCOUNT GetCommonOpTick(){return m_tOpsTimer.GetNextTime();}

	//发送复活戒指的db时间
	void SendReliveRingCd();
private:
	virtual int getLeaveCombatTime() { return BATTLE_LEFT_TIME; }
	void postponeLeaveCombatTimer();
public: 
	//状态禁
	static                  unsigned int *s_stateForbidMask; //
	int                     m_nAttackType; //攻击类型
	unsigned int  			m_nDizzyBuffInterval; //麻痹间隔时间
	TICKCOUNT               m_nDizzyTime;  //麻痹时间
	int						m_nJZLRCDTime;
	int						m_nToxicAttckValue;//剧毒裁决造成的绝对值伤害
	int						m_nSZASAttckValue;//霜之哀伤造成的绝对值伤害
protected:
	CObserverSystem			m_observerSystem;	// 观察者子系统
	CMovementSystem			m_moveSystem;		//移动的子系统
	CPropertySystem			m_propertySystem;	//属性子系统
	CBuffSystem				m_buffSystem;		//Buff子系统
	
	CEntityOPCollector		m_OPCollector;		//汇总操作记录对象
	CSkillSubSystem			m_skillSystem;		//技能子系统

	CEffectSystem           m_effectSystem;    //特效子系统

	CTimer<2000>			m_t2s ;			//回蓝回血定时器
	CTimer<500>			    m_t500ms ;		//500ms定时器
	CTimer<1000>            m_t1S;			//1秒定时器
	CTimer<700>				m_tRefViewTimer;//观察者系统刷新定时器
	CTimer<3000>			m_tKfRefViewTimer;//跨服观察者系统刷新定时器
	CTimer<3000>			m_t3sInner;		//内劲恢复
	
	
	CAnimalAI              * m_pAI; //怪物的AI指针
	unsigned int            m_stateMask; //状态禁止的模板
	CTimer<1000>            m_tOpsTimer;     //公共操作的定时器
	TICKCOUNT               m_battleLeftTime;  //战斗状态的进入状态
	
	//下面这些是战斗中用到的,不想每个都是以属性做，集中到一起
	int						m_nAttackLevel;		// 战斗等级
	int                     m_attackInterval;   //攻击间隔
	int                     m_maxDropHp;        //最大的掉落的hp,默认是0

	int                     m_nCamp;		// 阵营Id

	unsigned int			m_GuildId;		//设置行会id

	int                     m_MonsterDieHpPro;                //死亡以后立刻回复的HP的比例,1点表示百分之1(针对怪物用的)
        

	EntityHandle			m_killHandler;			//击杀者的指针
	EntityHandle			m_AttackerHandler;		//攻击自己的动物指针
};
 