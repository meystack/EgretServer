#pragma once

#define	GUILD_LEVEL_MAX		20			//行会等级最大到20级

typedef struct tagGuildLevelConfig
{
	int nLevel;			//等级id	
	int nMaxSecLeader;	//副会长数量的上限
	int nMaxelders;     //副长老数量的上限
	int nMaxElite;			//精英的上限
	int nMaxMember;		//该等级帮派所有成员的人数上限
	int nNeedYb;		//这个是升级到下一级需要的元宝
	int nWeekDecCoin;	//每周维护资金，每周日凌晨扣除，不足则删除行会
	int nFruitLevel;	//该等级行会对于的果实等级(从1-N)
	int nDepotPage;		//当前等级行会的仓库可以开放到页数
}GuildLevelConfig;

typedef struct tagGuildLevelConfigList
{
	int	nCount;
	GuildLevelConfig* pList;
}GuildLevelConfigList;

typedef struct tagFirstNewHundredAwardList
{
	int nCount;
	ACTORAWARD* pList;
	tagFirstNewHundredAwardList()
	{
		memset(this,0,sizeof(*this));
	}
}FirstNewHundredAwardList;
typedef struct tagReturnSiegeScene
{
	int nSceneId;//返回场景id
	int nPosX;
	int nPosY;
	int nRadius;//坐标半径范围
	tagReturnSiegeScene()
	{
		memset(this,0,sizeof(*this));
	}
}ReturnSiegeScene;
//每日捐献限额
typedef struct tagDailyDonateLimit
{
	int		m_nCoinLimit;		//每日捐献金币限额
	int		m_nItemLimit;		//每日捐献道具限额
}DailyDonateLimit;

//捐献数额
typedef struct tagDonateCfg
{
	int   nType;
	int   nCost;
	int   nAddCoin;
	int   nLimitTimes;
	int   nStaticCountType;
	int   nAddDonate;
	tagDonateCfg()
	{
		memset(this, 0, sizeof(*this));
	}
}DonateCfg;

typedef struct tagGuildUpgrade
{
	int		m_nUpgradeItem1;		//行会升级道具1
	int		m_nUpgradeItem2;	//行会升级道具2
	int		m_nUpgradeItem3;	//行会升级道具3
}GuildUpgrade;

//神树果实等级配置
typedef struct tagGuildFruitLevel
{
	int		m_nFruitId;				//果实id
	int		m_nFruitNum;			//神树果实数量
	int		m_nBackGuildCoin[2];	//返回资金
	char	m_szQualityName[16];
	tagGuildFruitLevel()
	{
		memset(this,0,sizeof(*this));
	}
}GuildFruitLevel;

typedef struct tagGuildTree
{
	int		m_nDailyChargeLimit;		//每人每天充能次数限制
	int		m_nDailyPickLimit;			//每人每天摘果实次数限制
	int		m_nFruitProtectTime;		//结果保护期
	CVector<GuildFruitLevel>  m_FruitLevelList;
}GuildTree;


typedef struct tagGuildRelation
{
	int nUnionNeedCoin;			//联盟所需费用
	int nDeclareLastTime;		//宣战持续的时间 单位秒
	int	nDeclareNeedCoin;		//宣战需要的行会资金
	int nBidMoney;				//行会竞价排名所需要的元宝
}GuildRelation;


//行会任务(这里的任务与角色任务，截然不同)
typedef struct tagGuildTask
{
	short	m_nTaskType;			//任务类型
	int		m_nTaskId;				//任务id
	int		m_nTaskObjId;			//任务目标id
	int		m_nTaskObjNum;			//任务目标完成数量
	tagGuildTask()
	{
		memset(this,0,sizeof(*this));
	}
}GuildTask;

typedef struct tagGuildLevelTask
{
	CVector<GuildTask> m_GuildTaskList; 
}GuildLevelTask;

//放入行会仓库
typedef struct tagGuildDepotPutIn
{
	int nItemCircle;			//要求物品转数
	int nItemLevel[2];			//要求物品等级
	CVector<int> addGxList;		//投入转数+等级+部位的物品成功增加的行会贡献
}GuildDepotPutIn;

//从行会仓库取出
typedef struct tagGuildDepotGetOut
{
	int nItemCircle;			//物品转数
	int nItemLevel[2];			//物品等级
	CVector<int> needGxList;	//兑换转数+等级+部位的物品需要的行会贡献
}GuildDepotGetOut;

//行会仓库
typedef struct tagGuildDepot
{
	int nDepotMaxPage;					//最大页数
	int nDailyDepotPutInNum;			//每人每日可投入操作的上限
	int nExchangeNeedCheckCircleMin;	//需要审核的装备的最低转数
	int nExchangeWaitCheckHour;			//审核等待时间（单位小时）
	CVector<GuildDepotPutIn> m_ItemPutInList;
	CVector<GuildDepotGetOut> m_ItemGetOutList;
}GuildDepot;


//行会建筑
typedef struct tagGuildBuildCfg
{
	int nType;
	int nLevel;
	int nCost;
	int nLimit;
	tagGuildBuildCfg()
	{
		memset(this, 0, sizeof(*this));
	}
}GuildBuildCfg;


//跨服沙巴克行会奖励
typedef struct tagCSSbkGuildRank
{
	int nValue;
	int nnIndex;//
	std::vector<ACTORAWARD> awards;
	tagCSSbkGuildRank()
	{
		awards.clear();
		nnIndex = 0;
		nValue = 0;
	}
	
}CSSBKGUILDRANK;

//行会关闭、行会清理
typedef struct tagGuildData
{
	int nTimeType;		// 时间类型
	int nHefuTimes;		// 第几次合服生效
	int nAfterSrvDay;	// 开服N天后开
	int nBeforeSrvDay;	// 开服前N天开
	CMiniDateTime nStartTime;	// 开始时间
	CMiniDateTime nEndTime;		// 结束时间
	CMiniDateTime nRealTimeLt;	// 该日期前开（时间戳）
	CMiniDateTime nRealTimeGt;	// 该日期后开（时间戳）
	CMiniDateTime nHefuTimeLt;	// 该日期前合服不开（时间戳）
	CMiniDateTime nHefuTimeGt;	// 该日期后合服不开（时间戳）
	OneTimePair* pTimeDetail;	// 时间配置
	short nTimeCount;
	short nTimeIdx;
	CHAR nIsOpen;			//是否开启(行会关闭、行会清理)功能
	tagGuildData()
	{
		memset(this,0,sizeof(*this));
	}

} GUILDDATA,*PGUILDDATA;

class CGuildProvider :
	protected CCustomLogicLuaConfig 
{
public:
	typedef CObjectAllocator<char>	CDataAllocator;
	typedef CCustomLogicLuaConfig	Inherited;

public:
	CGuildProvider();
	~CGuildProvider();

	// const FirstNewHundredAwardList* GetNewHundredAwardSBK(BYTE btIndex) const
	// {
	// 	if (btIndex >= nNewHundredCount )
	// 	{
	// 		return NULL;
	// 	}
	// 	return pNewHundredList + btIndex;
	// }

	//通过帮派等级配置对象
	inline const GuildLevelConfig* GetLevelConfig(INT_PTR nLevel) const
	{
		if ( nLevel <= 0)
			nLevel = 1;
		else if (nLevel > pLevelList->nCount) 
			nLevel = pLevelList->nCount;//如果等级超过了配置文件的等级，就按最大的等级配置
		nLevel--;
		return pLevelList->pList + nLevel;
	}

	const GuildBuildCfg* GetBuildCfgPtr(int type, int level)
	{
		std::map<int, std::map<int, GuildBuildCfg> >::iterator it = m_GuildBuilds.find(type);
		if(it != m_GuildBuilds.end())
		{
			std::map<int, GuildBuildCfg>& info = it->second;
			std::map<int, GuildBuildCfg>::iterator ot = info.find(level);
			if(ot != info.end())
			{
				return &(ot->second);
			}
		}
		return NULL;
	}
	DonateCfg* GetGuildDonateCfg(int type)
	{
		std::map<int, DonateCfg>::iterator it = m_nDonate.find(type);
		if(it != m_nDonate.end())
		{
			return &(it->second);
		}
		return NULL;
	}

	//从文件加载配置
	bool LoadGuildConfig(LPCTSTR sFilePath);

	inline int GetMaxLevel(){return nMaxLevel;}

	// CVector<ACTORAWARD> & GetCoinAwardList()
	// {
	// 	return m_CoinAward;
	// }
	//获取皇城职位名字
	const char* GetGuildCityPosName(int nPos);

	//物品放入到行会仓库增加的行会贡献
	int GetGuildDepotPutInAddGx(int nItemCircle, int nItemLevel, int nItemPos=0);

	//物品从行会仓库取出所需的行会贡献
	int GetGuildDepotGetOutNeedGx(int nItemCircle, int nItemLevel, int nItemPos=0);
protected:
	//以下函数为覆盖父类的相关数据处理函数
	void showError(LPCTSTR sError);
	bool readConfigs();
private:
	bool readGuildSiegeConfig();
	
	bool readGuildFruitLevelConfig();

	bool readGuildTaskConfig();

	bool readguildLevelConfig();

	bool readGuilBuildConfig();	
public:
	//攻城战相关
	// int nOpenSvrSingDays;			//开服前三天自动报名下次攻城，那第四天就是报当天的名	
	// CVector<BYTE> vSiegeDayList;	//攻城时间
	// char  sCityPosName[stMaxGuildCityPos][64];//皇城职位名称
	// int nAutoSignMainHallLevel;		//自动报名攻城需要主殿等级
	// int nDragonRoleBuffId;			//龙袍buffId
	// int nRealMasterModelId[2];			//君主模型Id
	// ReturnSiegeScene returnSiegeScene;//返回攻城场景
	// int nPalaceSceneId;				//沙皇宫场景
	// int nSiegeSceneId;				//攻城所在场景
public:
	int nLevelLimit;				//创建帮派时，角色需要达到的最小等级
	//int nCreateNeedItem;			//创建行会需要的物品id
	unsigned int nCreateNeedYb;	//创建行会需要的游戏币

	int nAwardGuildMoney;			//创建行会时赠送的行会资金

	int nLeftTimeLimit;				//恢复玩家主动脱离帮派的限制创建帮派加入其他帮派，这个是配置这个时间，以小时为单位（24小时）

	//int nCoinLimit;					//捐献行会钱票获得的行会资金
	int nAddCoinItemId;				//行会钱票的物品id

	// int nMinGuildCoin;				//行会资金降为1000删除行会

	// int nNoticeGuildCoin;			//行会资金低于2000的时候提示删除行会
	// int nDecGuildCoin;				//每天定时扣除行会资金
	int nMaxLevel;					//帮派的最大等级
	int nImpeachcost;             //弹劾消耗

	//int nTipsLevel[5];				//给行会提示的等级

	// int nHelpNeedCoin;				//求救需要的行会资金

	// int nVipDays;
	// int nVipType;
	// int nVipPrice;

	// int nSbkOwnerTitleId;			//沙城霸主的头衔id
	// int nSbkMemberTitleId;			//沙城英雄的头衔id


	// int nGuildCoinGxRate;			//捐献1000行会获得1贡献


	int nGuildLeaderLogoutDay;		//行会掌门未上线时间，单位：天
	int nGuildLeaderImpeachHour;	//检测弹劾行会掌门的时间，小时
	int nGuildLeaderImpeachMin;		//检测弹劾行会掌门的时间，分

	// int	nRewardCoffie;				//奖励系数

	// int nCallNeedCoin;				//召集所需行会资金
	// int nCallDoneTime;				//召集间隔

	int nProtectDay;				//职位保护期
	int nBuildNum;                //行会建筑数量

	std::map<int, DonateCfg>      m_nDonate; //捐献
	std::map<int, std::map<int, GuildBuildCfg> > m_GuildBuilds;
	//这个是每个等级的配置
	GuildLevelConfigList*	pLevelList;

	CVector<DailyDonateLimit> m_DailyDonateLimitList;
	//int m_nMaxMainHallCoin[MAXMAINHALLLEVEL];
	//CVector<ACTORAWARD> m_CoinAward;

	// FirstNewHundredAwardList*	pNewHundredList;
	// int							nNewHundredCount;

	// int nCallMemberNeed;
	std::vector<CSSBKGUILDRANK>  sbkguild;//
	std::vector<ACTORAWARD> czrewards;
	char czrewardsMailTT[50]; //
	char czrewardsMailCT[50];

	char rankAwardMailTT[50];
	char rankAwardMailCT[50];
	GuildRelation		m_GuildRelation;			//行会关系

	// GuildUpgrade		m_GuildUpgrade;				//行会升级
	// GuildTree			m_GuildTree;				//行会神树
	// GuildLevelTask		m_GuildLevelTaskList[GUILD_LEVEL_MAX];			//行会任务列表
	// GuildDepot			m_GuildDepot;				//行会仓库
	int nApproval;//行会创建后是否需要自动审批
	GUILDDATA m_GuildClose;//行会关闭
	GUILDDATA m_GuildClear;//行会清理
	

private:
	CDataAllocator			m_DataAllocator;	//对象申请器
	CDataAllocator			m_DataAward;
public:
	//跨服相关
	void GetSbkGuildAward(int nRankId, std::vector<ACTORAWARD>& award);

public:
	bool UpdateTime(lua_State * pLuaState, GUILDDATA& guildData, bool nNextTime = false);
	//获取行会关闭配置
	GUILDDATA& GetGuildCloseConfig(){ return m_GuildClose; }
	//获取行会清理配置
	GUILDDATA& GetGuildClearConfig(){ return m_GuildClear; }
	// 检测该日期后不开，true为可开
	bool CheckOpenTimeLT(GUILDDATA& guildData);
	// 检测该日期前不开，true为可开
	bool CheckOpenTimeGt(GUILDDATA& guildData);
	// 检测该日期前合服开，true为可开
	bool CheckHefuTimeLt(GUILDDATA& guildData);
	// 检测该日期后合服开，true为可开
	bool CheckHefuTimeGt(GUILDDATA& guildData);
	// 检测第几次合服生效，true为可开
	bool CheckHefuTimes(GUILDDATA& guildData);
private:
	bool ReadTime(lua_State * pLuaState, int nTimeType, LPCTSTR sKey, int nKeyLen, CMiniDateTime& out);
	bool ReadFixedTimeValue(lua_State * pLuaState, LPCTSTR sKey, int nKeyLen, SYSTEMTIME& out);
};
