#pragma once


#define	MAXFBPARAM	32
#define	MAXBOSSCOUNT 6
#define	MAXPROGRESS 8



class CFuBenManager;


class CFuBen : public Counter<CFuBen>
{
public:
	typedef CVector<CScene*> SceneList;
public:
	CFuBen();
	virtual ~CFuBen();
public:
	//初始化的工作
	VOID Init(bool boRun=true);

	//重用那个时必须调用的函数
	void Reset();

	//从空闲列表中取出后必须做的一些工作
	void OnReuse();

	//判断是否空闲的内存
	bool IsFree() { return m_boFree; }
	void SetFreeFlag(bool boFlag);
	
	inline VOID SetFbId(int id) {m_nFbId = id;}
	inline int GetFbId(){return m_nFbId;}
	
	inline int GetFbType() {return m_bType;}
	inline void SetFbType(BYTE nType)  { m_bType = nType;}

	/*
	* Comments: 是否是副本
	* @Return bool: 是副本返回true，否则返回false，那么就是普通场景
	*/
	inline bool IsFb(){return m_nFbId !=0;}

	//是否按照场景配置里的刷怪方式刷怪开关
	//默认只有主场景使用默认刷怪方式
	//副本刷怪可以手动启用
	bool create_monster_flag =  0 ;
	bool change_create_monster_flag(bool flag=0);
	inline bool is_use_default_create_monster() {return create_monster_flag;}



	VOID SetHandle(const CFuBenHandle& hHandle){m_hFbHandle = hHandle;}
	CFuBenHandle GetHandle() {return m_hFbHandle;}
	
	SceneList& GetSceneList() {return m_vSceneList;}

	inline void AddScene(CScene* pScene){ m_vSceneList.add(pScene); }
	
	inline void SetConfig(COMMONFUBENCONFIG* pConfig) {m_pConfig = pConfig;}

	//每次逻辑循环都需要执行的函数
	//每次执行只能有固定的时间内，超出时间必须停止，返回false表示正常停止，true是超时停止
	int RunOne();

	//输出本副本的信息
	void Trace();

	/*
	* Comments: 进入一个副本
	* Param INT_PTR nScenceID: 场景的ID
	* Param INT_PTR nPosX: 位置x
	* Param INT_PTR nPosY:位置y
	* @Return BOOL: 成功返回true，否则返回false
	*/
	bool Enter(CEntity * pEntity,INT_PTR nScenceID,INT_PTR nPosX, INT_PTR nPosY, INT_PTR nWeight = 0, INT_PTR nHeight = 0, int nType = 0,
		int nValue = 0, int nParam=-1, CScene* pLastScene = NULL, CFuBen* pLastFb=NULL);

	//克隆一个跟本副本一样的新的副本,DstFuBen是克隆本
	void Clone(CFuBen& DstFuBen,bool boRun =true);
	
	//往副本增加场景实例，这里的场景不会被释放，其他场景都从这里的场景clone出来
	//本函数只应该在程序启动时读取配置数据的时候执行
	BOOL AddStaticScene(SCENECONFIG* SceneData,CFuBenManager* pFbMgr,CObjectAllocator<char>& dataAlloc);

	inline SceneConfigList* GetSceneConfig() {return m_vSceneConfig;}

	inline COMMONFUBENCONFIG* GetConfig() { return m_pConfig;}

	/*
	* Comments: 返回本副本中指定场景id的场景指针
	* Param INT_PTR nScenceID:场景ID,没有指定的场景，返回第一个场景
	* @Return CScene*:场景的指针
	*/
	CScene* GetScene(INT_PTR nScenceID);

	CScene* GetSceneByName(LPCTSTR sName);
	
	//重置副本的位置
	void RefreshFbPos();



	//判断是否含有这个id的场景
	bool HasScene(int nSceneId);

	/*
	* Comments:增加一个被拒绝进入的人
	* Param UINT nActorid:角色id
	* @Return void:
	*/
	void AddRefuseActor(UINT nActorid);

	/*
	* Comments:判断是否被拒绝进入的人
	* Param UINT nActorid:角色id
	* @Return bool:
	*/
	bool IsRefuseActor(UINT nActorid);

	unsigned int GetGuildId() {return m_nGuildId;};
	void SetGuildId(unsigned int nGuildId) { m_nGuildId = nGuildId; }

	/*
	* Comments:踢出所有的玩家，玩家将返回到普通场景，如果本副本是常规副本，这函数不起作用
	* @Return void:
	*/
	void ExitAllActor();

	//获取副本玩家的数量
	int GetFubenActorCount();

	/**
	* @brief:获取副本队伍玩家平均等级	
	*/
	int  getPlayerAverageLvl() const;

	/*
	* Comments: 更新副本队伍玩家数量
	* Param int nCount: 玩家数量
	* @Return void:
	*/
	void updateFubenPlayerCount(int nCount);

	/*
	* Comments: 查询副本玩家数量
	* @Return int:
	*/
	int getPlayerCount() const;

	//脚本设置副本的相关的值
	inline void SetFbValue(INT_PTR nIndex,int nValue)
	{
		if (nIndex < 0 || nIndex >= MAXFBPARAM) return;
		m_nValues[nIndex] = nValue;
	}

	inline int GetFbValue(INT_PTR nIndex) 
	{
		if (nIndex < 0 || nIndex >= MAXFBPARAM) return 0;
		return m_nValues[nIndex];
	}
	
	//设置和获取副本剩余时间
	UINT GetFubenTime();

	void _SetFubenTime( UINT nTime, LPCSTR file, INT_PTR line  );

	#define SetFubenTime(nTime) _SetFubenTime(nTime, __FILE__, __LINE__)

	/*
	* Comments: 更新副本玩家数量和平均等级
	* Param CEntity * pEntity: 实体对象
	* Param bool bAdd: 进入副本还是离开副本。
	* @Return void:
	*/
	void UpdateFbPlayerCountAndAverageLevel(CEntity *pEntity, bool bAdd);

	/*
	* Comments: 副本玩家等级变化
	* Param int updateLevel: 等级变化值
	* @Return void:
	*/
	void OnFbPlayerLevelChanged(int updateLevel);

	//设置和获取在副本中角色死亡的次数
	inline INT_PTR GetActorDieCount() { return m_ActorDieTime; }
	inline void AddActorDieCount()
	{
		m_ActorDieTime++;
	}

	//设置杀死boss的玩家的名字
	inline void AddKillBossName(LPCSTR sName)
	{
		if (m_KillNameCount < (MAXBOSSCOUNT-1))
		{
			_asncpytA(m_szKillBossName[m_KillNameCount],sName);
			m_KillNameCount++;
		}
	}

	//获取杀死boss的玩家的个数
	inline INT_PTR GetKillBossCount()
	{
		return m_KillNameCount;
	}
	//获取杀死过boss的玩家的名字
	LPCSTR GetKillBossActorName(INT_PTR nIndex)
	{
		if (nIndex >=0 && nIndex < m_KillNameCount)
		{
			return m_szKillBossName[nIndex];
		}
		return "";
	}

	//获取副本总共出现的怪物的总数
	inline INT_PTR GetMonsterTotal() { return m_MonsterTotal; }
	inline void AddMonsterTotal() { m_MonsterTotal++; }
	unsigned int GetMonsterCount(int nMonsterId, int nScenId);

	//获取和设置副本内被杀死的怪的数量
	inline INT_PTR GetKillMonsterCount() { return m_KillMonsterCount; }
	inline void AddKillMonsterCount() {m_KillMonsterCount++;}
	
	/*
	* Comments: 向副本中所有在线的玩家广播数据的兼容接口
	* Param LPCVOID lpData:
	* Param SIZE_T dwSize:
	* @Return void:
	*/
	void SendData(LPCVOID lpData, SIZE_T dwSize);

	/*
	* Comments:重新读入npc的脚本
	* Param LPCTSTR szSceneName:指定的场景名，如果为NULL，则寻找所有的场景
	* Param LPCTSTR szNpcName:指定的NPC名，如果为NULL，则重新读入所有npc的脚本
	* @Return void:
	*/
	INT_PTR ReloadNpcScript(LPCTSTR szSceneName,LPCTSTR szNpcName);

	/*
	* Comments: 将副本中所有场景的怪物都创建出来
	* @Return void:
	* @Remark: 
	*/
	void RefreshMonster();

	/*
	* Comments: 统计场景已创建出来的怪物数
	* @Return void:
	* @Remark: 
	*/
	unsigned int TotalSceneMonsterCount();

	//获取变量对象
	inline CCLVariant& GetVar() { return m_DynamicVar; }

private:
	/*
	* Comments:判断角色能否进入这个副本，这个只有角色实体才需要执行这个函数
	* Param CEntity * pEntity:
	* @Return bool:
	*/
	bool CanEnter(CActor * pActor);
	
private:
	// 副本基本属性数据
	int					m_nFbId;					// 副本Id（副本Id须保证是连续配置）
	CFuBenHandle		m_hFbHandle;				// 副本句柄
	COMMONFUBENCONFIG*	m_pConfig;					// 副本静态配置
	SceneList			m_vSceneList;				// 副本的场景对象列表
	UINT				m_DestoryTime;				// 副本删除的时间，由于玩家掉线，副本会为他保留一段时间才删除
	UINT				m_restTime;					// 副本的剩余时间，用于限时的副本，默认是0，表示无限制
	bool				m_boFree;					// 副本是否是自由状态。只针对动态副本（提高动态进入FB效率，采取预分配和池策略）
	CEntityList			m_RefuseList;				// 被拒绝进入的玩家列表，主要用于结婚副本中限定某些人进入,实际上这里保存的是角色的id
	BYTE				m_bType;					// 副本的类型，比如结婚副本、擂台副本

	// 用于FB内怪物属性的动态刷新数据
	int					m_nPlayerLevelSum;			// 副本玩家等级和
	int					m_nPlayerCount;				// 当前副本玩家人数，用于副本怪物动态刷新	
	
	// FB相关的统计数据
	WORD				m_ActorDieTime;				// 玩家打副本过程中一共死亡的次数
	int					m_KillMonsterCount;			// 一共杀死的怪物的数量
	char				m_szKillBossName[MAXBOSSCOUNT][32];		// 保存杀死boss的玩家的名字 这里有问题！！！如果玩家不是顺序杀boss的话
	BYTE				m_KillNameCount;			// 杀死boss的玩家的个数，不能超过6个（MAXBOSSCOUNT）
	int					m_MonsterTotal;				// 副本中一共出现过的怪物的总数

	// 提供给脚本临时保存副本相关数据用的数据
	CCLVariant			m_DynamicVar;				// 实体动态变量	
	int					m_nValues[MAXFBPARAM];		// 给脚本使用的存储空间

    CTimer<1000>					m_1sTimer;

	unsigned int		m_nGuildId;					//此副本是因为某个行会而创建出来的，0-不属于某个行会
public:
	//对应的场景配置信息
	SceneConfigList*		m_vSceneConfig;	
};
