
#pragma once

#define MAXMONTHCARDCFGNUM  13
#define MAXCOLORCARDLEVEL  7

//NPC雕像类型
enum NpcStatueType 
{
	stNormal = 0,
	stRsCastellan = 1,	//本服沙巴克城主雕像
	stCsCastellan = 2,	//跨服沙巴克城主雕像
	stRsVocation = 3,	//本服职业雕像
	stCsVocation = 4,	//跨服职业雕像
};

typedef struct tagHostReplaceCfg{
    int nId;
    int nPrice;
    int nItemId;

    tagHostReplaceCfg()
    {
        memset(this, 0, sizeof(*this));
    }
}gHostReplaceCfg;



class CRanking;

//雕像NPC配置
typedef struct tagNPCStatue
{
	BYTE	btType;				//雕像类型 
	BYTE	btParam;			//职业
	char	sSceneName[32];		//场景名字
	char	sNpcName[32];		//NPC名字
	int		nMaleModelId;		//男性模型ID
	int		nFemaleModelId;		//女性模型ID
}NPCSTATUE,*PNPCSTATUE;


typedef struct tagSmileTalk
{
	char sTalk[200];
}SmileTalk, *PSmileTalk;

typedef struct tagSmileTalkList
{
	int nSmileId;
	int nCount;
	SmileTalk * pSmileTalk;
}SmileTalkList, *PSmileTalkList;

typedef struct tagAllSmileTalks
{
	int nCount;
	SmileTalkList * pSmileList;
}AllSmileTalkList, *PAllSmileTalkList;

/// 排行榜属性添加配置
typedef struct tagRankProp
{
	char rankName[32];	///< 关联的排行榜名字
	int  nValue;		///< 值限制, 大于等于该值才加进来显示
	DataList<CStdItem::AttributeGroup> props;	//属性
	CRanking *pRanking;	///< 关联哪一个排行榜
	tagRankProp()
	{
		rankName[0] = NULL;
		nValue		= 0;
		pRanking	= NULL;
	}
}RANKPROP, *PRANKPROP;

//工资加成配置配置
typedef struct tagSalary
{
	WORD	nBegin;		//开服天数区间开始
	WORD	nOver;		//开服天数区间结束 -1代表以后
	BYTE	nRate;		//倍率
}SALARY, *PSALARY;

typedef struct _ACTIVITYTIME 
{
	//unsigned short wDayOfWeek;
	//unsigned short wDay;
	unsigned short wStartHour;
	unsigned short wStartMinute;
	unsigned short wStartSecond;
	unsigned short wEndHour;
	unsigned short wEndMinute;
	unsigned short wEndSecond;
	_ACTIVITYTIME()
	{
		memset(this, 0, sizeof(*this));
	}
}ACTIVITYTIME, *PACTIVITYTIME;

//转生配置
typedef struct tagCircleAttri
{
	AttriGroup		attri;//属性
	tagCircleAttri()
	{
		memset(this, 0, sizeof(*this));
	}
}CIRCLEATTRI,*PCIRCLEATTRI;

//兵魂星星配置
typedef struct tagSoldierSoulStar
{
	AttriGroup		attri;//属性
	tagSoldierSoulStar()
	{
		memset(this, 0, sizeof(*this));
	}
}SOLDIERSOULSTAR, *PSOLDIERSOULSTAR;
//兵魂配置
typedef struct tagSoldierSoulConfig
{
	int nAppear;//外观id
	int skillId;//技能id
	DataList<SOLDIERSOULSTAR> starlist;//星星属性列表
}SOLDIERSOULCONFIG, *PSOLDIERSOULCONFIG;

//开启等级配置
typedef struct tagOpenLevelConfig
{
	int circle ;
	int level;
	tagOpenLevelConfig()
	{
		memset(this, 0, sizeof(*this));
	}
}OPENLEVELCONFIG,*POPENLEVELCONFIG;

//装备升星、铭刻的配置
typedef struct tagEquipStar
{
	//AttriGroup		attri;	//属性
	AttriGroup		posAttri[Item::itEquipMax];		//部位属性
	tagEquipStar()
	{
		memset(this, 0, sizeof(*this));
	}
}EQUIPSTAR,*PEQUIPSTAR,EQUIPINSCRIPT,*PEQUIPINSCRIPT;

typedef struct tagEquipStarConfig
{
	DataList<EQUIPSTAR> starlist;		//强化-星级，铭刻-铭刻等级(需要动态分配缓存)
}EQUIPSTARCONFIG,*PEQUIPSTARCONFIG,EQUIPINSCRIPTCONFIG,*PQUIPINSCRIPTCONFIG;

//转生属性
// typedef struct tagCircleAttr
// {
// 	AttriGroup		attri;	//属性
// 	tagCircleAttr()
// 	{
// 		memset(this, 0, sizeof(*this));
// 	}
// }CIRCLEATTR,*PCIRCLEATTR;

typedef struct tageSubDay
{
	int nDay = 0;
	int nRate = 0;
	tageSubDay()
	{
		memset(this, 0, sizeof(*this));
	}

}TAGSUBDAY, *PTAGSUBDAY;

typedef struct tagCircleLevel
{
	int nlevel;             //转生等级
	int nServerOpenDay;     //开服天数
	int nLevelLimit;        //等级限制
	std::vector<ACTORAWARD> cost;
	// int nLastCircleNum;     //上一转的人数
	int nLevelBlock;         //该转生等级上线
	int nCostLevel;// 消耗等级
	// std::vector<TAGSUBDAY> m_vSubDay; //
	AttriGroup		attri;	//属性
	int nTipId = 0;
	tagCircleLevel()
	{
		memset(this, 0, sizeof(*this));
	}

	// int getCost(int nOpenDay)
	// {
	// 	int ncount = m_vSubDay.size();
	// 	int nCost = ncost;
	// 	int nDay = 0;
	// 	for(int i = 0; i< ncount; i++)
	// 	{
	// 		TAGSUBDAY& day = m_vSubDay[i];

	// 		if(nOpenDay >= day.nDay && day.nDay > nDay)
	// 		{
	// 			nDay = day.nDay;
	// 			nCost = ncost*day.nRate/100;
	// 		}	
	// 	}

	// 	return nCost;
	// }
}CIRCLELEVEL,*PCIRCLELEVEL;


typedef struct tagExchangeCircle
{
	int nType;              //兑换类型
	int nCost;         	    //消耗
	int nValue;     	    //数值
	int nUseLimit;          //每日限制
	int nStaticType;        //计数器类型
	
	tagExchangeCircle()
	{
		memset(this, 0, sizeof(*this));
	}
}EXCHANGECIRCLE,*PEXCHANGECIRCLE;

typedef struct tagCircleCfg
{
	//DataList<CIRCLEATTR> starlist;
	std::map<int, CIRCLELEVEL> m_lCircleLv;
	DataList<EXCHANGECIRCLE> m_lExSoul;

	int nOpenLv = 0;  //开启天数
	int nOpenServerDay = 0; //开服天数
	
	tagCircleCfg()
	{
		nOpenLv = 0;
		nOpenServerDay = 0;
		m_lCircleLv.clear();
	}

	// int GetCost(int nLevel, int nDay)
	// {
	// 	std::map<int, CIRCLELEVEL>::iterator it = m_lCircleLv.find(nLevel);
	// 	if(it != m_lCircleLv.end())
	// 	{
	// 		return it->second.getCost(nDay);
	// 	}

	// 	return 0;
	// }

}CIRCLECFG,*PCIRCLECFG;

typedef struct TagMeridianUpCfg
{
	int nLevel;         //经脉等级
	int nServerOpenDay; //开服天数
	int nLevelLimit;    //等级限制
	int nCircle;        //转生要求
	AttriGroup	attri;	//属性
	// int nCostExp;		//升至本级消耗的经验
	// int nCostBindCoin;  //升至本级消耗的绑金
	// int nCostId;      //升至本级消耗的初阶秘籍id
	// int nCostBook;      //升至本级消耗的初阶秘籍
	std::vector<ACTORAWARD> costs;
	char  nTips[1024];
	TagMeridianUpCfg()
	{
		nLevel = 0;
		nServerOpenDay = 0;
		nLevelLimit = 0;
		nCircle = 0;
		costs.clear();
		memset(this->nTips, 0, sizeof(this->nTips));
	}
}MeridianUpCfg;
//神魔
typedef struct tagGhostConfig
{
	int nId;         //
	int nLevel;         //
	int nServerOpenDay; //开服天数
	int nLevelLimit;    //等级限制
	int nCircle;        //转生要求
	int nProbability;        //
	char	name[32];		//
	AttriGroup	attri;	//属性
	std::vector<ACTORAWARD> cost;
	int nTipId = 0;
}GhostConfig;

typedef struct tagGhostDataCfg
{
	std::map<int, std::vector<GhostConfig> >  m_gHostUpLvCfg;    			//神魔升级
	std::map<int, std::vector<GhostConfig> >  m_DemonslevelConfig ;    			//共鸣
	int nAddBless;
	int nMaxBless;
	int nAddProbability;
	int nProbability;
	int nMaxLv;
	int nMaxCount;
	char sTitle[1024];
	char sContent[1024];
	std::vector<ACTORAWARD> awards;
	GhostConfig* GetGhostLvCfg(int nPos, int nLevel) {
		if(m_gHostUpLvCfg.count(nPos)) {
			std::vector<GhostConfig>& v_lists = m_gHostUpLvCfg[nPos];
			for(auto & it : v_lists) {
				if(it.nLevel == nLevel)
					return &it;
			}
		}
		return NULL;
	}

	GhostConfig* GetGhostCommonCfg(int nType, int nLevel) {
		if(m_DemonslevelConfig.count(nType)) {
			std::vector<GhostConfig>& v_lists = m_DemonslevelConfig[nType];
			for(auto & it : v_lists) {
				if(it.nLevel == nLevel)
					return &it;
			}
		}
		return NULL;
	}
}GhostDataCfg;

typedef struct tagFrenzy
{
	BYTE nCardLv;
	std::vector<std::tuple<int,int>> vecPropertys;
	std::vector<std::tuple<int,int,int>> vecConsum;
	std::vector<std::tuple<int,int,int>> vecKillAward;
	std::string vecKillerTitleHead;
	std::string vecKillerTitleContent;
} FrenzyCfg;

typedef struct tagChgVoc
{
	std::vector<std::tuple<int,int,int>> vecChangeVocationUnConsum;	//专职变性道具不消耗
	std::vector<std::tuple<int,int,int>> vecChangeVocationConsum;	//专职变性道具消耗
	unsigned int nChangeVocCD;
} ChgVocCfg;

typedef struct tagOffice
{
	short nLevel;
	short nIsNotice;//是否走马灯公告
	short nLvlLimit;//等级限制
	short nCircleLimit;//转身限制
	char sName[32];//官阶名字
	std::vector<std::tuple<int,int>> vecPropertys;
	std::vector<std::tuple<int,int,int>> vecConsum;
} OfficeCfg;

typedef struct tagTeamFubenConfig
{
	int  nOpenLevel;  
	int  nAddTime;
	int  nAddForce;
	int  nAddForceOutline;
	int  nMaxForce;
	int  nExtraForce;
	int  nMaxTeamCount;
}TEAMFUBENCONFIG,*PTEAMFUBENCONFIG;

typedef struct tagMonthCardConfig
{
	GOODS_TAG		Consume;
	GOODS_TAG		BuyAwardList[6];
	GOODS_TAG		DailyAwardList[6];
	int 			nSuperRightLV;//特权卡等级，普通卡为0
	int 			nMaxExpDampNum;//衰减百分数，如70%则为70
	int 			nJiShouNum;    //寄售行寄售数目
	int 			nAtvPaoDianPlus;//特权活动泡点加成
	std::vector<GOODS_TAG> m_recoverGolds; //特权回收 金币加成
}MONTHCARDCONFIG,*PMONTHCARDCONFIG;

//杂七杂八配置
class CMiscProvider
	: protected CLuaConfigBase
{
public:
	typedef CObjectAllocator<char>	CDataAllocator;
	typedef CLuaConfigBase	Inherited;


	typedef struct tagActivityExp
	{
		int nCount;
		long long * llExps;
	}ACTIVITYEXP, *PACTIVITYEXP;

	typedef struct tagActivityExpList
	{
		int nCount;
		ACTIVITYEXP * pExpList;
	}ACTIVITYEXPLIST, *PACTIVITYEXPLIST;

	typedef struct tagBanneretBuy{
		int		favorableprice;	// 优惠价
		int		price;			// 价格
		int		nDays;
	}Banneret, *PBanneret;

	typedef struct tagtagBanneretBuyList{
		int count;
		PBanneret  data;
	}BanneretBuyList, *PBanneretBuyList;

	CMiscProvider();
	~CMiscProvider();

	//装载NPC雕像配置
	bool LoadNpcStatueConfig(LPCTSTR sFilePath);

	//装载排行榜属性配置
	bool LoadRankPropConfig(LPCTSTR sFilePath);

	//装载工资配置
	bool LoadSalaryConfig(LPCTSTR sFilePath);

	//获取对应职业的NPC雕像
	inline const PNPCSTATUE GetNpcStatue(INT_PTR nType, INT_PTR nParam)
	{
		for (INT_PTR i=0; i < m_npcStatue.count(); i++)
		{
			PNPCSTATUE pData = &m_npcStatue[i];
			if ((INT_PTR)pData->btType == nType)
			{
				if (nType == stRsVocation || nType == stCsVocation)
				{
					if ((INT_PTR)pData->btParam == nParam)
						return pData;
				}
				else
					return pData;
			}
		}
		return NULL;
	}

	//
	inline CVector<NPCSTATUE>* GetNpcStatueList() {return &m_npcStatue;}

	//获取移动路径
	inline DataList<RANKPROP>& GetRankProps()
	{
		return m_RankProps;
	}

	RANKPROP* GetRankPropByRankName(LPCSTR sName);

	inline const int GetSalaryRate(int nDay)
	{
		for (INT_PTR i=0; i < m_SalayConfig.count; i++)
		{
			SALARY *pData = m_SalayConfig.pData + i;

			if ((nDay >= pData->nBegin && nDay <= pData->nOver) || (nDay >= pData->nBegin && pData->nOver == -1))
			{
				return pData->nRate;
			}
		}
		return 0;
	}
	
	/* 
	* Comments: 获取转数对应的最大等级
	* Param int nCirlce: 转数
	* @Return int:  
	*/
	int GetPlayerMaxLevel(int nCirlce, int nState = 0);
	
	AttriGroup* GetCircleAttri(int nIndex);
	//获取某个兵魂的配置
	SOLDIERSOULCONFIG* GetSoldierSoulCfgByIndex(int nIndex);
	
	/*
		获取装备铭刻配置的属性
		这里就是取最后一段配置，以最后一段配置为准
	*/
	inline EQUIPINSCRIPTCONFIG*  GetEquipInscriptCfg()
	{
		INT_PTR  nCount  = m_EquipInscript.count() ;
		if(nCount > 0)
		{
			return  &m_EquipInscript[nCount - 1];
		}
		return NULL;
	}

	//获取开启等级
	POPENLEVELCONFIG GetOpenLevel(int nId);

	//通过月卡index获取配置
	MONTHCARDCONFIG* GetMonthCardConfig(int mtype)
	{
		if (mtype >= (sizeof(m_MonthCardConfig)/sizeof(MONTHCARDCONFIG)))
		{
			return NULL;
		}
		return &m_MonthCardConfig[mtype-1];
	}

	//通过特权等级获取配置
	MONTHCARDCONFIG* GetMonthCardConfigbySuperLV(int mLV)
	{
		if (mLV >= (sizeof(m_SuperRightMap)/sizeof(int)))
		{
			return NULL;
		}
		int key = m_SuperRightMap[mLV] ; 
		if ((key < 0) || (key > MAXMONTHCARDCFGNUM))
			return NULL;
		
		return &m_MonthCardConfig[key];
	}

	//获取首充卡配置
	MONTHCARDCONFIG* GetShouChongCardConfig()
	{
		if (m_DefaultPaoDianPlusKey ==0 )
		{
			return NULL;
		}
		return &m_MonthCardConfig[m_DefaultPaoDianPlusKey];
	}



public:
	bool ReadAllConfig(){return true;}
	bool LoadLevelConfig(LPCTSTR sFilePath);//开启等级
	bool LoadCircleConfig(LPCTSTR sFilePath);

	bool LoadEquipInscriptConfig(LPCTSTR sFilePath);		//装备铭刻
	//读取经脉升级
	bool LoadMeridiansCfg(LPCTSTR sFilePath);
	
	//读取月卡配置
	bool LoadMonthCardConfig(LPCTSTR sFilePath);
	
private:

	//读取月卡配置
	bool ReadMonthCardConfig();

	//装载雕像NPC配置
	bool ReadAllNPCStatue();

	//装载排行榜属性配置
	bool ReadAllRankProp();

	//装载工资配置
	bool ReadAllSalaryConfig();

	//读取属性表
	bool ReadAttributeTable(CDataAllocator &dataAllocator, CStdItem::AttributeGroup *pAttrGroup);

	//读取转生配置
	bool ReadCircleConfig();
	//读取兵魂配置
	bool ReadSoldierSoulConfig();
	//获取开启等级配置
	bool ReadLevelConfig();
	

	//读取装备铭刻配置
	bool ReadEquipInscriptConfig();

	//按装备部位读取属性
	bool ReadEquipPosConfig(AttriGroup& stAttriGroup);
	bool LoadEquipAttri( PGAMEATTR pAttr );

	//经脉
	bool ReadMeridiansCfg();
public:
	//神魔
	bool LoaddemonCfg(LPCTSTR sFilePath);
	bool Readdemonsbody();
	void ReadCommonDemons(const char* table, std::map<int, std::vector<GhostConfig> >& m_maps);
	void readDemonCost();

	//狂暴
	bool LoadFrenzyCfg(LPCTSTR sFilePath);
	bool ReadFrenzy();
	FrenzyCfg const& GetFrenzyCfg(){ return m_FrenzyCfg; }

	//官职
	bool LoadOfficeCfg(LPCTSTR sFilePath);
	bool ReadOffice();
	std::vector<OfficeCfg> const& GetOfficeCfg(){ return m_OfficeCfg; }

	//转职
	bool LoadChangeVocCfg(LPCTSTR sFilePath);
	bool ReadChangeVoc();
	ChgVocCfg const& GetChangeVocCfg(){ return m_ChgVocCfg; }

	
	OfficeCfg*  GetOfficeCfgPtrByLevel(int nLevel){
		if(nLevel < 0 || nLevel >= m_OfficeCfg.size() )
			return NULL;
		return &m_OfficeCfg[nLevel];
	 }

	gHostReplaceCfg* getGhostReplaceCfg(int nItemId)
    {
        std::map<int, gHostReplaceCfg>::iterator it = m_nReplaces.find(nItemId);
        if(it != m_nReplaces.end())
        {
            return &(it->second);
        }
        return NULL;
    }

private:
	CDataAllocator	m_DataAllocator;
	ACTIVITYEXPLIST m_ActivityExpList;
	
	std::vector<OfficeCfg> m_OfficeCfg;
	
	CDataAllocator		m_npcStatueAllocator;	//内存分配器
	CVector<NPCSTATUE>	m_npcStatue;	//雕像NPC配置

	DataList<RANKPROP>	m_RankProps;	//排行榜属性

	DataList<SALARY>	m_SalayConfig;	//工资配置
	
public:
	CVector<int>		m_CircleMaxLevelConfig;       //当前转数对应最大等级
	CIRCLECFG         m_CircleLevelConfig;          //转生属性
	CVector<SOLDIERSOULCONFIG>	m_soldierSoul;        //兵魂
	CVector<EQUIPINSCRIPTCONFIG>  m_EquipInscript;    //装备铭刻

	CVector<OPENLEVELCONFIG>  m_openLevel;            //开启等级配置
	TEAMFUBENCONFIG  m_TeamFubenConfig;               //团队副本
	CVector<int>  m_TeamFubenIDList;                  //团队副本ID列表
	std::map<int, MeridianUpCfg>  m_MeridianUpCfg;    			//经脉升级配置
	//神魔
	GhostDataCfg           m_nGhostCfg;
	MONTHCARDCONFIG m_MonthCardConfig[MAXMONTHCARDCFGNUM + 1];	//月卡配置
	int m_SuperRightMap[MAXCOLORCARDLEVEL + 1] ;				//存储 m_MonthCardConfig中的索引id
	int m_DefaultPaoDianPlusKey;								//激活充值卡后的活动泡点经验加成配置 m_MonthCardConfig[m_DefaultPaoDianPlusKey]
	//狂暴属性
	FrenzyCfg m_FrenzyCfg;
	//转职配置
	ChgVocCfg m_ChgVocCfg;
	 std::map<int, gHostReplaceCfg> m_nReplaces;//神魔替换
};
