#pragma once
#include <map>
#include <vector>
class CGlobalVarMgr;
#define BOSSSOCAIL_MUTIPLY 100000
#define  BOSS_ID_MUTIPLY   1000
typedef struct tagKillBossCost
{
	int nId; //
	int nType; //
	int nCount;//
	tagKillBossCost()
	{
		memset(this, 0, sizeof(*this));
	}
	
}KILLBOSSCOST;
typedef struct tagBossOpenConfig
{
	int nOpenlevel;//开启等级
	int nOpenday;//开放天数
	int nId;//boss 计数器id
	tagBossOpenConfig()
	{
		memset(this, 0, sizeof(*this));
	}
}BossOpenConfig;

struct BossTimePair
{
	int nStartTime;
    int nEndTime;
};
//boss信息
typedef struct tagBossConfigInfo
{	
	int nBossId; //bossid
	int nSerial; //
	int nTab;   //页签类型
	int nReborn; //刷新类型
	int nMaxNum; //数量
	std::vector<int> nReferTime; //刷新时间
	int nMapId ;//刷新地图
	int nBossX;     //x坐标
	int nBossY;     //y坐标
	int nBossRange; //范围
	int nNotice; //公告
	char nContent[200];// 公告
	int nOpenCircle; //转生等级
	int nOpenDay;//开服天数
	int nDelivery; //传送
	int nEndmap;// 飞鞋地图
	std::vector<tagKillBossCost> nCosts;//消耗
	int nDeliveryX; //传送坐标x
	int nDeliveryY;//传送坐标y
	int nDeliveryRange; //传送范围
	int nDaliyLimit; //每日限制次数
	int nDalitStaticId; //计时器id
	int nJiontime;//传入时间
	int nShowDay;//显示天数
	int nFubenId; //副本id
	int nLevellimit; // 等级限制
	int nBerebornlimit; //转生限制
	int nNodeliver;//死亡可传送
	int nNoexpel;//boss死亡踢出玩家
	int nIsOpen;// 是否开启
	int nNpcId;// 是否开启
	int nVip;// 是否开启
	int nTips;//tips
	int nShowwindow;//
	BossTimePair* pTimeSlotDetail;//活动时间段
	short nTimeSlotCount;
	short nTimeSlotIdx;
	tagBossConfigInfo()
	{
		nBossId = 0; //bossid
		nSerial = 0;
		nTab = 0;   //页签类型
		nReborn = 0; //刷新类型
		nMaxNum = 0; //数量
		nReferTime.clear(); //刷新时间
		nMapId = 0;//刷新地图
		nBossX = 0;     //x坐标
		nBossY = 0;     //y坐标
		nBossRange = 0; //范围
		nNotice = 0; //公告
		memset(&nContent, 0, sizeof(nContent));
		nOpenCircle = 0; //转生等级
		nOpenDay = 0;//开服天数
		nDelivery = 0; //传送
		nEndmap = 0;// 飞鞋地图
		nCosts.clear();//消耗
		nDeliveryX = 0; //传送坐标x
		nDeliveryY = 0;//传送坐标y
		nDeliveryRange = 0; //传送范围
		nDalitStaticId = 0;
		nDaliyLimit = 0;
		nJiontime = 0;
		nShowDay = 0;
		nFubenId = 0;
		nBerebornlimit = 0;
		nVip = 0;
		nNodeliver = 0;
		nIsOpen = 0;
		nNoexpel = 0;
		nNpcId = 0;
		nTips = 0;
		nShowwindow = 0;
		pTimeSlotDetail = nullptr;
		nTimeSlotCount = 0;
		nTimeSlotIdx = 0;
	}
	
	int GetNextReferTime(int nLastReferTime);

}BOSSCONFIGINFO;



typedef struct tagForbiddenAreaCfg
{
	int nMapid = 0;//地图id
	int nOpenserverDay = 0;//开服天数
	int nLimitLv = 0; //等级限制
	int nLimitCircle = 0;//转生限制
	int nJoinTime = 0;//进入时间
	int nLimittimes = 0;//进入次数限制
	std::vector<tagKillBossCost> nCosts;//消耗
	int nJoinX = 0;//x坐标
	int nJoinY = 0;//y坐标
	int nRange = 0;//范围
}FORBIDDENAREAN;
//boss成长等级限制条件
typedef struct tagMaxHardLevel
{
	int nLimit ;//
	int nLimitNum ;//
	int nLevel;//
	tagMaxHardLevel()
	{
		memset(this, 0, sizeof(*this));
	}
}MAXHARDLEVEL;
//boss成长等级限制条件
typedef struct tagBossMaxHardLevel
{
	int nType ;//
	std::vector<MAXHARDLEVEL> nGrowLv; //等级
	tagBossMaxHardLevel()
	{
		nType = 0;
		nGrowLv.clear();
	}
}BOSSMAXHANDLEVEL;
//boss成长线
typedef struct tagBossGrowData
{
	int nBossId; //bossid
	int nFDropA; //系数A
	int nFDropB; //系数B
	int nFDropC; //系数C
	BOSSMAXHANDLEVEL mHandLvs;
	tagBossGrowData()
	{
		nBossId = 0;
		nFDropA = 0;
		nFDropB = 0;
		nFDropC = 0;
	}

	int GetBuffAddValue(int nBosslv)
	{
		return 100 + nBosslv*nBosslv*nFDropA + nBosslv*nFDropB + nFDropC;
	}

	int GetBossGrowLv(int OpenserverDay, int nMaxLevel, CGlobalVarMgr* pGGlobVar);
}BOSSGROWDATA;


class CMonsterProvider :
	protected CVector<MONSTERCONFIG>,
	protected CCustomLogicLuaConfig
{
public:
	typedef CVector<MONSTERCONFIG>		Inherited;
	typedef CCustomLogicLuaConfig		Inherited2;
	typedef CObjectAllocator<char>	CDataAllocator;
	static LPCTSTR MonsterCacheFile;	//配置数据缓存文件路径

	typedef struct tagBossGrowHeader
	{
		UINT32 uLength;
	}BossGrowHeader;

	//成长BOSS的成长数据
	typedef struct tagBossGrowData
	{
		INT nMonsterId;
		INT nHardLevel;
		INT nDeadCount;
	}BossGrowData;

	//击杀BOSS的时间和角色
	typedef struct tagBossFlushTime
	{
		unsigned int nTime;
		ACTORNAME sName;
		unsigned int nMyId;
	}BOSSFLUSHTIME, *PBOSSFLUSHTIME;

	//所有BOSS信息
	typedef struct tagBossInfo
	{
		WORD nId;
		byte btType;
		WORD nSceneId;
		unsigned int nFlushDt;
		CVector<BOSSFLUSHTIME> KillList;
	}BOSSINFO, *PBOSSINFO;

	//野外BOSS（XGame）
	typedef struct tagWildBoss
	{
		WORD nBossId;
		WORD nSceneId;
	}WILDBOSS, *PWILDBOSS;

	//世界BOSS（XGame）
	typedef struct tagWorldBoss
	{
		WORD nBossId;
		WORD nSceneId;
	}WORLDBOSS, *PWORLDBOSS;

	//BOSS击杀信息（普通怪不要用这个）
	typedef struct tagBossKill
	{
		WORD nBossId;
		unsigned int nLastKillTime;
	}BOSSKILL, *PBOSSKILL;

	//BOSS击杀信息（普通怪不要用这个）
public:
	CMonsterProvider();
	~CMonsterProvider();

	//转换函数
	inline operator const MONSTERCONFIG* () const
	{
		CMonsterProvider *pProvider = (CMonsterProvider*)this;
		return pProvider->Inherited::operator MONSTERCONFIG*();
	}


	/*
	* Comments:通过怪物ID返回怪物数据
	* Param const INT_PTR nId:怪物ID
	* @Return const MONSTERCONFIG*: 怪物的数据指针
	*/
	inline const PMONSTERCONFIG GetMonsterData(const UINT_PTR nId) const
	{
		//ID为0的
		if ( nId <=0 || nId >= UINT_PTR(Inherited::count()) )
			return NULL;
		else return (PMONSTERCONFIG)&(this->operator const MONSTERCONFIG*()[nId]);
	}
	
	inline const PNPCCONFIG GetNpcData(const INT_PTR nId) const
	{
		if (nId <= 0 || nId > m_npcList.count())
		{
			return NULL;
		}
		return &m_npcList[nId-1];
	}

	/*
	* Comments:通过怪物的名称获取配置信息
	* Param const INT_PTR nId:
	* @Return const PMONSTERCONFIG:
	*/
	inline const PMONSTERCONFIG GetMonsterDataByName(const char* sName) const
	{
		//ID为0的
		for (int i = 0; i < Inherited::count(); i++)
		{
			PMONSTERCONFIG ret = (PMONSTERCONFIG)&(this->operator const MONSTERCONFIG*()[i]);
			if (strcmp(ret->szName,sName) == 0) return ret;
		}
		return NULL;
	}

	inline char* GetMonsterNameById(int nId) 
	{
		const PMONSTERCONFIG pConfig = GetMonsterData(nId);
		if (pConfig)
		{
			_asncpytA( sNameNoShowNumber, pConfig->szName);
			INT_PTR nLen = strlen(sNameNoShowNumber);
			if (nLen > 2)
			{
				if (sNameNoShowNumber[nLen-1] >='0' && sNameNoShowNumber[nLen-1] <='9')
				{
					sNameNoShowNumber[nLen-1]=0;
				}
				if (sNameNoShowNumber[nLen - 2] >= '0' && sNameNoShowNumber[nLen - 2] <='9')
				{
					sNameNoShowNumber[nLen - 2] = 0;
				}
			}
			return sNameNoShowNumber;
		}
		return NULL;
	}
	
	
	/*
	* Comments:获取怪物的数量
	* @Return INT_PTR: 怪物的数量
	*/
	inline INT_PTR count() const{ return Inherited::count(); }

	
	/*
	* Comments: 从文件里装载怪物的数据
	* Param LPCTSTR sFilePath: 路径的名称
	* @Return bool: 失败返回false
	*/
	bool LoadMonsters(LPCTSTR sFilePath, bool bBackLoad = false);

	bool LoadBossConfig(LPCTSTR sFilePath);

	//boss列表
	bool LoaddBossInfoConfig(LPCTSTR sFilePath);
	//boss成长
	bool LoaddBossGrowConfig();

	/*
	* Comments:加载npc配置
	* Param LPCTSTR sFilePath:
	* @Return bool:
	*/
	bool LoadNpcs(LPCTSTR sFilePath);

	/*
	* Comments:从二进制中加载怪成长数据
	* @Return bool:成功返回true
	*/
	bool LoadBossGrowData(LPCTSTR sFilePath);

	bool LoaddShenZhuangBossConfig();


	/*
	* Comments:保存怪成长数据到二进制
	* @Return bool:成功返回true
	*/
	bool SaveBossGrowData(LPCTSTR sFilePath);

	void SetBossGrowFlag(bool boFlag)
	{
		m_bossGrowupHasChange = boFlag;
	}


	//清空数据
	void ClearBossGrow();

	std::map<int, BOSSCONFIGINFO> & GetBossInfoList()
	{
		return m_bossInfoList;
	}

	// CVector<WILDBOSS> & GetWildBossList()
	// {
	// 	return m_wildBossList;
	// }

	// CVector<WORLDBOSS> & GetWorldBossList()
	// {
	// 	return m_worldBossList;
	// }

	inline BOSSCONFIGINFO * GetBossInfoById(int nId)
	{
		std::map<int, BOSSCONFIGINFO>::iterator it = m_bossInfoList.find(nId);
		if(it != m_bossInfoList.end())
		{
			return &(it->second);
		}
		return NULL;
	}

	//获取某个成长BOSS数据
	inline BOSSGROWDATA * GetBossGrowDataById(WORD nId)
	{
		std::map<int, BOSSGROWDATA>::iterator it = m_growBossList.find(nId);
		if(it != m_growBossList.end())
		{
			return &(it->second);
		}
		return NULL;
	}

	//装载野外BOSS
	bool LoadWildBossConfig(LPCTSTR sFilePath);
	//装载世界BOSS
	bool LoadWorldBossConfig(LPCTSTR sFilePath);

	void SetBossLastKillTime( WORD nBossId, unsigned int nKillTime );
	unsigned int GetBossLastKillTime( WORD nBossId );

	//更新后台加载的怪物数据
	void UpdateMonsterConfig();

	void AddBossTeleCfgTimes(int nId, int nValue)
	{
		std::map<int, int>::iterator it = m_BossTeleCfg.find(nId);
		if(it != m_BossTeleCfg.end())
		{
			m_BossTeleCfg[nId] += nValue;
		}
		else
		{
			m_BossTeleCfg[nId] = nValue;
		}
	}

	int GetBossTeleCfgTimes(int nId)
	{
		std::map<int, int>::iterator it = m_BossTeleCfg.find(nId);
		if(it != m_BossTeleCfg.end())
		{
			return m_BossTeleCfg[nId];
		}
		
		return 0;
	}
protected:
	//以下函数为覆盖父类的相关数据处理函数
	void showError(LPCTSTR sError);

private:

	//************************************
	// Method:    completeRead
	// FullName:  CMonsterProvider::completeRead
	// Access:    private 
	// Returns:   void
	// Qualifier: 读取怪物数据配置完成后，将读取的数据保存到自身中
	// Parameter: MONSTERCONFIG * pMonsters
	// Parameter: const INT_PTR nMonsterCount
	// Parameter: CDataAllocator & dataAllocator
	//************************************
	void completeRead(MONSTERCONFIG *pMonsters, const INT_PTR nMonsterCount, CDataAllocator &dataAllocator, bool bBackLoad = false);

	/*
	* Comments:装载全部怪物的数据
	* @Return bool: 失败返回false
	*/
	bool ReadAllMonsters();


	/*
	* Comments: 读取一个怪物的数据
	* Param CDataAllocator & dataAllocator: 内存分配器
	* Param MONSTERCONFIG * oneMonster:  怪物的指针
	* @Return bool:
	*/
	bool ReadOneMonster(CDataAllocator &dataAllocator,MONSTERCONFIG *oneMonster);
	
	bool ReadMonsterSkills( CDataAllocator &dataAllocator,MONSTERCONFIG *oneMonster);

	/*
	* Comments: 
	* Param OneSkillData * oneSkill: 一个技能的数据指针
	* @Return bool: 成功返回true ，否则返回false
	*/
	/*
	* Comments:读取怪物的技能
	* Param CDataAllocator & dataAllocator:
	* Param MONSTERCONFIG * oneMonster:
	* @Return bool:
	*/
	//bool ReadMonsterSkills( CDataAllocator &dataAllocator,MONSTERCONFIG *oneMonster);

	/*
	* Comments:读取怪物的掉落组
	* Param CDataAllocator & dataAllocator:
	* Param MONSTERCONFIG * oneMonster:
	* @Return bool:
	*/
	bool ReadMonsterDropRroup(CDataAllocator &dataAllocator,MONSTERCONFIG *oneMonster);

	/*
	* Comments:读取怪物的属性标志位
	* Param CDataAllocator & dataAllocator:
	* Param MONSTERCONFIG * oneMonster:
	* @Return bool:
	*/
	bool ReadMonsterFlags(CDataAllocator &dataAllocator,MONSTERCONFIG *oneMonster);

	/*
	* Comments: 读取怪物发言信息
	* Param CDataAllocator & dataAllocator
	* Param MONSTERCONFIG * oneMonster: 怪物配置
	* @Return bool:成功返回true；失败返回false
	*/
	bool ReadMonsterSayInfo(CDataAllocator &dataAllocator, MONSTERCONFIG *oneMonster);
	

	/*
	* Comments: 读取怪物的优先攻击目标列表配置
	* Param CDataAllocator & dataAllocator:
	* Param MONSTERCONFIG * oneMonster:
	* @Return bool:
	*/
	bool ReadMonsterPriorAttackTarget(CDataAllocator &dataAllocator, MONSTERCONFIG *oneMonster);
	
	/*
	* Comments: 读取怪物颜色配置
	* Param MONSTERCONFIG * pMonster:
	* @Return bool:
	*/
	bool ReadMonsterColor(MONSTERCONFIG *pMonster);

	/*
	* Comments:预加载
	* Param LPCTSTR sFilePath:文件路径
	* @Return bool:成功返回true
	*/
	bool PreLoad(LPCTSTR sFilePath);
	
	/*
	* Comments: 读取怪物的一个掉落
	* Param CDataAllocator & dataAllocator:
	* Param PDROPGROUP oneDrop:
	* @Return bool:
	*/
	//bool ReadMonsterDrop(CDataAllocator &dataAllocator,PDROPGROUP oneDrop);

	

private:
	//从缓存文件中读怪物配置数据，如果缓存数据有效且源数据文件未经修改且读取成功则返回true。
	bool readCacheData(DWORD dwSrcCRC32);
	//将配置数据写入缓存文件中
	bool saveCacheData(DWORD dwSrcCRC32);

	bool m_bossGrowupHasChange ; //boss升级的是否改变过
private:
	CDataAllocator m_DataAllocator; //内存分配器
	CDataAllocator	m_DataAllocatorBack;	// 怪物属性对象申请器(用于后台加载)	
	CVector<MONSTERCONFIG> m_MonsterBack;			// 后台加载的怪物数据
	INT_PTR				   m_nMonsterCount;		// 分配的静态怪物对象数量（用于后台加载）
	CCSLock			m_Lock;

	// 怪物属性组
	static int			  s_nPropCount;
	static CREATURBATTLEEDATA*  s_pPropConfig;
	// 怪物标志组
	static int			  s_nFlagCount;
	static EntityFlags*   s_pFlagConfig;

	CVector<NPCCONFIG> m_npcList;
	// CVector<BOSSINFO> m_bossInfoList;
	// CVector<WILDBOSS> m_wildBossList;		//野外BOSS
	// CVector<WORLDBOSS> m_worldBossList;		//世界BOSS
	// CVector<BOSSKILL> m_bossKillList;		//BOSS的击杀信息（普通怪不要用这个）
	// CVector<BossGrowData>  m_growBossList;	//成长BOSS列表（配置CanGrowUp = true）
	char sNameNoShowNumber[33];
	std::map<int, BOSSGROWDATA> m_growBossList; //成长系数
	std::map<int, BOSSCONFIGINFO> m_bossInfoList; //boss列表
	std::map<int ,int > m_BossTeleCfg;//快传次数配置 --配置很乱统一处理
public:
	std::vector<BossOpenConfig> m_openCfgs; //开启动力
	// int nOpenlevel;//开启等级
	// int nOpenday;//开放天数
	// int nId;//神装boss 计数器id
	// int nPersonOpenlevel;//个人开启等级
	// int nPersonOpenday;//个人开放天数
	// int nPersonId;//个人boss 计数器id
};
