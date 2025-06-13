#pragma once

/*******************************************************/
/*
/*					逻辑数据配置提供类
/*
/*   本类实现对游戏逻辑服务器运行中需要的各种游戏配置数据的读取以及管
/* 理。例如各个等级的升级需求经验配置，货币上限配置，基础属性配置等。
/* 
*Note: 文件的读取规则
*  需要读取的文件自己定义一个头文件,比如XXConfig.h,里边定义自己用到的数据结构体
*  然后写一个XXConfig.cpp,里边实现CLogicDataProvider里一个函数,比如bool LoadXXConfig(),这里去读取数据
* CLogicDataProvider 里导出一个读取接口,比如  PXXCONFIGDATA GetXXConfig();读取数据的指针
/*******************************************************/

// 配置数据类型，刷新配置的时候，需要用到这里的ID
//!!注意，添加的时候再后面接着添加就行，不要重复ID
enum ConfigDataType
{
	enCDType_Unknown				= 0,	//暂位
	enCDType_RankProp				= 1,	//排行榜配置
	enCDType_SETTING				= 2,	//游戏设置
	enCDType_AI						= 3,	//AI配置
	enCDType_SceneNpcDynPro			= 4,	//属性加成配置
	enCDType_Property				= 5,	//属性配置
	enCDType_Monster				= 6,	//怪物配置
	enCDType_NPC					= 7,	//NPC配置
	enCDType_Vocation				= 8,	//职业配置
	enCDType_Item					= 9,	//物品配置
	enCDType_EquipDerive			= 10,	//装备衍生属性配置
	enCDType_Skill					= 11,	//技能配置
	enCDType_Activity				= 12,	//活动配置
	enCDType_Bag					= 13,	//背包配置
	enCDType_BuffConfig				= 14,	//BUFF配置
	enCDType_Scene					= 15,	//场景配置
	enCDType_Fuben					= 16,	//副本配置
	enCDType_Quest					= 17,	//任务配置
	enCDType_QuestTypeNames			= 18,	//任务类型名配置
	enCDType_Guild					= 19,	//行会配置
	enCDType_Suit					= 20,	//套装配置
	enCDType_PK						= 21,	//PK配置
	enCDType_Notice					= 22,	//公告配置
	enCDType_OFFICE					= 23,	//官职配置
	enCDType_ChgVoc					= 24,	//转职配置
	enCDType_Smith					= 25,	//精锻属性配置
	enCDType_Title					= 26,	//称号配置
	enCDType_TopTitle				= 27,	//称号配置
	enCDType_PetExp					= 28,	//宠物经验配置
	enCDType_ItemEval				= 29,	//装备评分配置
	enCDType_TimerConfig			= 30,	//定时功能配置
	enCDType_StoreRefresh			= 31,	//商城刷新配置
	enCDType_QuestEntrustRate		= 32,	//任务多倍配置
	enCDType_QuestTargetRate		= 33,	//任务目标配置
	enCDType_34			= 34,	//
	enCDType_35			= 35,	//
	enCDType_36				= 36,	//
	enCDType_37				= 37,	//
	enCDType_38						= 38,
	enCDType_39						= 39,
	enCDType_40						= 40,
	enCDType_SalaryConfig			= 41,	//工资配置
	enCDType_42				= 42,	//
	enCDType_NewTitleConfig			= 43,	//新称号配置
	enCDType_CircleConfig			= 44,	//转生配置
	enCDType_45		= 45,	//
	enCDType_EquipStarConfig        = 46,   //装备升星配置
	enCDType_GodStoveConfig			= 47,	//神炉系统 TO DELETE
	enCDType_TeamFubenConfig        = 48,   //团队副本配置
	enCDType_49						= 49,
	enCDType_WILDBOSSConfig			= 50,	//野外BOSS配置
	enCDType_WORLDBOSSConfig		= 51,	//世界BOSS配置
	enCDType_ConsignementCondfig	= 52,	//交易行
	enCDType_LevelConfig			= 54,	//开启等级配置
	enCDType_MonthCard				= 55,	//月卡
	enCDType_WingConfig			    = 56,	//翅膀属性
	enCDType_EquipInscriptConfig    = 57,   //装备铭刻配置
    enCDType_HeroConfig             = 58,   //英雄配置
	enCDType_Frenzy					= 59,	//狂暴配置
	enCDType_AchieveConfig			= 60,	//成就属性
	enCDType_61			= 61,	//
	enCDType_CombatConfig			= 62,	//职业竞技配置
	enCDType_GlobalConfig	 		= 63,	//全局配置
	enCDType_DropConfig			    = 64,	//掉落配置
	enCDType_ChatSysConfig			= 65,	//聊天全局配置
	enCDType_DropItemConfig			= 66,	//掉落物品全局配置
	enCDType_LevelUpConfig			= 67,	//升级配置
	enCDType_ComposeConfig          = 68, //合成
	enCDType_ForgeConfig            = 69, //锻造
	enCDType_SopConfig              = 70, //商城
	enCDType_DeathDropConfig        = 71, //死亡掉落
	enCDType_MeridianUpCfg          = 72, //经脉
	enCDType_FlyShoesIndex			= 73, //飞鞋消耗配置
	enCDType_FlyShoesTable			= 74, //飞鞋等级要求配置
	enCDType_BlessTable				= 75, //祝福
	enCDType_ExpLmtOpenday			= 76, //开服天数相关每日经验上限
	enCDType_ChangeName				= 77, //改变名字
	enCDType_FirstLogin				= 78, //新角色
	enCDType_BagRemainConfig		= 79, //剩余背包
	enCDType_StrengthenConfig		= 80, //强化
	enCDType_FashionConfig	     	= 81, //时装
	enCDType_MailConfig	         	= 82, //邮件
	enCDType_RefiningConfig	        = 83, //洗炼
	enCDType_HookExpConfig	        = 84, //泡点
	enCDType_UpStarConfig	        = 85, //升星
	enCDType_DemonConfig	        = 86, //神魔
	enCDType_RankConfig	       		= 87, //排行榜
	enCDType_NumericalConfig	    = 88, //数值资源
	enCDType_WarehouseConfig	    = 89, //仓库
	enCDType_ZSLevelExpPercentConfig	    = 90, //转生等级差-杀怪经验衰减
	enCDType_MergeTimesConfig	    = 91, //
	enCDType_NSNNotice				= 92,	//NSN公告配置
	enCDType_Hallows				= 93,	//圣物
	enCDType_ReviveDurationConfig	= 94,	//复活特权
	enCDType_LootPet	= 95,				//宠物捡取
	enCDType_ResonanceItem	= 96,			//共鸣属性 
	enCDType_Rebate	= 97,					//返利系统
	enCDType_Edition= 98,					//差异表
};
//判断背包类型
enum BagType
{
	BagDef    = 0, //默认                   提示 -- 
	BagNpc    = 1, //npc领奖活动               tmDefNoBagNum    
	BagShop = 2, //商城购物                     tmDefNoBagNum        
	BagDeal = 3, //交易行单次提取              tmDefNoBagNum
	BagMailOld =4, //邮件领取附件时                tmLeftBagNumNotEnough
	BagRecover =5, //回收(正常不会出现不足)       tmRecoverNoBagNum
	BagCompose =6, //合成                      tmLeftBagNumNotEnough
	BagRequest =7, //领取任务奖励               tmLeftBagNumNotEnough
	BagForge =8, //锻造武器1次                  tmForgeNoBagNum
	BagTenForge =9, //锻造武器10次              tmTenForgeNoBagNum
	BagGitfOld =10, //礼包              tmTenForgeNoBagNum
	BagCaiLiaoFB = 11,//材料副本      tmDefNoBagNum
	BagXunBao = 12,   //寻宝		  tmDefNoBagNum
	BagZhuShou = 13, //驻守任务       tmDefNoBagNum
	BagMail =14, //邮件领取附件时        tmLeftBagNumNotEnough
	BagGitf =15, //礼包              tmTenForgeNoBagNum
	BagAtvCommon = 16,//活动通用
	BagShouChong = 17,//首充领奖
	BagUnused	 = 18,//物品兑换活动
	BagComposes  = 19,//合成十次

};
struct ChangeNameList {
	int nNum; 
	LPSTR* pNameList;
};

struct ItemAward
{
	int nType;
	int nId;
	int nCount;
};

class CNoticeProvider;
class CNSNNoticeProvider;
class CLogicEngine;
class CFlyShoes;
 
class CHallowsthenCfg; 

// Lua 配置数据
class CLogicDataProvider :
	public CLuaConfigBase
{
public:
	typedef CLuaConfigBase Inherited;
	
	//定义数据提供器数据加载项结构
	static const struct DataProviderLoadTerm
	{
		BOOL    boLoad;			//此加载项是否需要加载
		ConfigDataType cdType;	// 配置数据类型
		TCHAR	sTermName[64];	//加载项的名称
		TCHAR	sFilePath[256];	//加载项的文件路径
		bool (CLogicDataProvider::*loadProc)(LPCTSTR);//加载项的加载函数
	}	LoadTermList[];//加载项列表
	//声明加载项的数量
	static const INT_PTR LoadTermCount;

	//定义数据提供器运行时数据项结构
	static const struct RunTimeDataTerm
	{
		BOOL    boLoad;			//此数据项是否需要加载和保存
		TCHAR	sTermName[64];	//数据项的名称
		TCHAR	sFilePath[256];	//数据项的文件路径
		bool (CLogicDataProvider::*loadProc)(LPCTSTR);//数据项的加载函数
		bool (CLogicDataProvider::*saveProc)(LPCTSTR);//数据项的保存函数
	}	RunTimeDataTermList[];//运行时数据数据项列表
	//声明运行时数据项的数量
	static const INT_PTR RunTimeDataTermCount;
public:
	static const DataProviderLoadTerm* GetDataProviderLoadConfig(const ConfigDataType cdt);

	//获取的接口
	inline CMonsterProvider& GetMonsterConfig() {return m_monsterProvider;}	//只有get方法
	inline CFlyShoes& GetFlyShoesConfig() {return m_FlyShoesConfig ;}   	//获取飞鞋配置信息
	inline PVOCATIONINITTABLE GetVocationConfig(){return &m_vocation;}   //获取职业的配置信息

	inline CStdItemProvider& GetStdItemProvider(){ return m_StdItemProvider; }//获取物品配置数据提供器
	inline  CSkillProvider* GetSkillProvider(){return &m_skillProvider;}       //获取技能的配置  
	inline CQuestProvider* GetQuestConfig() {return &m_QuestProvider;} //任务
	inline CRankProvider& GetRankConfig() {return m_RankProvider;} //排行榜
	inline TIPMSGCONFIG & GetOldTipmsgConfig(){ return m_oldTipmsg; } //获取系统提示的配置 TO DELETE
	inline NEWTIPMSGCONFIG & GetTipmsgConfig(){ return m_tipmsg; } //获取系统提示的配置
	inline TIPMSGCONFIG & GetActorMoodLangConfig(){ return m_ActorMoodLang; } //获取玩家心情语言包的配置
	inline TIPMSGCONFIG & GetMonsterShoutConfig() {return m_monsterShout;} //获取怪物说话的配置

	inline CActivityProvider& GetActivityConfigs() { return m_activityProvider; }
	inline CNoticeProvider& GetNoticeConfigs() { return m_noticeProvider; }
	inline CNSNNoticeProvider& GetNSNNoticeConfigs() { return m_NsnNoticeProvider; }
	inline CGameStore& GetGameStore(){ return m_GameStore; };//获取游戏商城
	VOID SetLogicEngine(CLogicEngine* pLogic){m_pLogicEngine = pLogic;}

	inline LEVELUPEXPTABLE& GetLevelUpExp(){return m_propertyConfig.expTable;} //玩家的升级经验表
	inline EXPLMTOPENDAY& GetExpLmtOpenday() {return m_ExpLmtOpendayConfig ; }  //与开服天数相关的每日经验上限
	inline BAGCONFIG & GetBagConfig() {return m_bagConfig;} //背包的配置

	inline CGuildProvider & GetGuildConfig() { return m_GuildConfig;}

	inline CFriendProvider &GetFriendConfig() {return m_FriendConfig;}
	inline CMailCfg & GetMailConfig() { return m_cMailCfg;} //邮件
	inline CRefiningCfg & GetRefiningConfig() { return m_Refining;} //洗炼
	inline KILLMONSTEREXPTABLE & GetKillMonsterExpReduce(){return m_propertyConfig.killMonsterExpTable;} //获取杀怪经验衰减
	inline GLOBALCONFIG &    GetGlobalConfig(){return m_propertyConfig.globalConfig;} //全局配置表
	inline TEAMKILLMONSTEREXP & GetTeamKillMonsterExp(){return m_propertyConfig.teamKillMonsterExp; } //组队杀怪
	// inline SUITCONFIG& GetSuitConfig(){return m_suitConfig;} //返回套装的
	inline CAiProvider & GetAiConfig(){return m_aiProvider;}

	inline CPkProvider& GetPkConfig(){return m_PkProvider;}
	inline CBlessCfg& GetBlessCfg(){return m_BlessConfig;}//祝福
	inline CHATSYSTEMCONFIG        & GetChatSystemConfig (){return m_propertyConfig.chatConfig;}
	inline DROPITEMCONFIG        & GetDropItemConfig (){return m_propertyConfig.dropItemConfig;}
	inline BAGREMAINCONFIG        & GetBagRemainConfig (){return m_propertyConfig.bagRemain;}
	inline HookExpConfig        & GethookExpCfg (){return m_propertyConfig.hookExpCfg;}
	inline DEATHDROPCONFIG        & GetDeathDropRateConfig (){return m_propertyConfig.deathDropCfg;}
	inline CAchieveProvider & GetAchieveConfig() {return m_achieveConfig;}
	inline SceneNpcDynPropConfig &GetNpcDynPropConfig() { return m_npcDynPropConfig; }
	inline CVector<unsigned int> &GetExpGetUpperLmtCfg() { return m_vecExpGetUpperLmt; }
	inline ACTOR_NAME_CLR_TABLE &GetActorNameClrCfg() { return m_nameClrTable; }

	inline RENOWNLIMIT& GetRenownConfig(){return m_renownLimit;} //获取每个等级的获得的声望的最大的数值
	inline CPetProvider& GetPetProvider () {return m_petProvider;} //宠物的配置
	inline CScriptTimerConfig & GetScriptTimerConfig() { return m_scriptTimerConfig;} //获取脚本定时回调的函数
	inline CMiscProvider & GetMiscConfig(){return m_miscProvider;}			//杂7杂8的配置
	inline CBuffProvider & GetBuffProvider(){return m_buffConfig;}		//BUFF配置
	inline CLootPetProvider & GetLootPetProvider(){return m_LootPetConfig;}		//BUFF配置
	inline CResonanceItemProvider & GetResonanceItemProvider(){return m_ResonanceItemConfig;}		//BUFF配置
	inline CRebateProvider & GetRebateProvider(){return m_RebateConfig;}		//返利系统
	inline CEditionProvider & GetEditionProvider(){return m_EditionConfig;}		//差异表
	inline CNewTitleProvider & GetNewTitlesConfig() {return m_NewTitleConfig;}
	inline CReviveDurationProvider & GetReviveDurationsConfig() {return m_reviveDurationConfig;}

	inline ConsignmentProvider & GetConsignmentProvider() {return m_ConsignmentProvider;}//交易行(寄卖配置)

	inline CWingsAngel& GetWingConfig(){return m_WingsAngel;}		//翅膀的配置

	inline CFashionCfg& GetFashionConfig(){return m_FashionCfg;}		//翅膀的配置

	inline  CHeroProvider&  GetHeroConfig(){return m_HeroConfig;}     //英雄的配置
	inline  DropGroupCfg&  GetDropCfg(){return m_DropCfg;}     //掉落的配置

	inline  CAchieve&  GetNewAchieveConfig(){return m_Achieve;}				//成就的配置

	inline CCombatRankProvider& GetCombatConfig() { return m_CombatConfig;} //职业竞技配置

	inline CGameSetting& GetGameSetting(){return m_SettingConfig;}//游戏设置配置

	inline CStrengthenCfg& GetStrengthen() {return m_StrengthenConfig; } //强化配置
	inline ItemUpStarConfig& GetUpStar() {return m_ItemUpStarCfg; } //升星配置
	
	inline CHallowsthenCfg& GetHallowthen() {return *m_pHallowsthenConfig; } //圣物配置
 
	LPCTSTR GetChangeName(int nIdx)
	{
		if ((nIdx >= 0) && (nIdx < m_ChangeNameList.nNum))
		{
			return m_ChangeNameList.pNameList[nIdx];
		}
		return NULL;
	}

	std::vector<int>& GetFirstLoginKeySet(){return m_FirstLoginKeySet;}
	std::vector<int>& GetFirstLoginDress(){return m_FirstLoginDress;}
	std::vector<ItemAward>& GetFirstLoginAwards(int vocation) {
		switch (vocation)
		{
		case enVocWarrior:
			return m_WarriorFirstLoginAwards;
			break;
		case enVocMagician:
			return m_MagicianFirstLoginAwards;
			break;
		case enVocWizard:
			return m_WizardFirstLoginAwards;
			break;
		}
		return m_WarriorFirstLoginAwards;
	}
	//获取设定背包剩余格数
	std::vector<int>& getBagRemainConfig(int nType ) {
		std::map<int, std::vector<int> >::iterator it = m_propertyConfig.bagRemain.m_nBagRemain.find(nType);
		if(it != m_propertyConfig.bagRemain.m_nBagRemain.end())
		{
			return it->second;
		}
		return m_propertyConfig.bagRemain.m_nBagRemain[0];
	}

	/*
	* Comments:重刷配置文件 
	* Param char * sConfigDesc:
	* @Return bool:
	*/
	bool ReloadConfig(char * sConfigDesc);

	bool ReloadConfig(INT_PTR nConfigId);



public:
	//加载所有基础配置
	bool LoadConfig();
	//加载动态生成的数据
	bool LoadRunTimeData();
	//保存动态生成的数据
	bool SaveRunTimeData();
	//加载游戏商城配置数据
	inline bool LoadGameStoreConfig(LPCTSTR sFilePath){ return m_GameStore.LoadGameStore(sFilePath); }

	bool ReadAttributeTable(CObjectAllocator<char> &dataAllocator, CStdItem::AttributeGroup *pAttrGroup);

public:
	CLogicDataProvider();
	~CLogicDataProvider();
	
protected:
	void showError(LPCTSTR sError);
	bool ReadAllConfig(){return true;}
private:
	bool LoadNoticeConfig(LPCTSTR sFilePath) { return m_noticeProvider.LoadAll(sFilePath); }
	bool LoadNSNNoticeConfig(LPCTSTR sFilePath) { return m_NsnNoticeProvider.LoadAll(sFilePath); }
	bool LoadActivityConfig(LPCTSTR sFilePath) { return m_activityProvider.LoadAll(sFilePath); }
	bool LoadMonsterConfig(LPCTSTR sFilePath) { return m_monsterProvider.LoadMonsters(sFilePath); }
	bool LoadBossConfig(LPCTSTR sFilePath) { return m_monsterProvider.LoadBossConfig(sFilePath);}
	bool LoadWildBossConfig(LPCTSTR sFilePath) { return m_monsterProvider.LoadWildBossConfig(sFilePath);}
	bool LoadWorldBossConfig(LPCTSTR sFilePath) { return m_monsterProvider.LoaddBossInfoConfig(sFilePath);}
	//小飞鞋配置表
	bool LoadFlyShoesIndex(LPCTSTR sFilePath) { return m_FlyShoesConfig.LoadIndex(sFilePath);}
	bool LoadFlyShoesTable(LPCTSTR sFilePath) { return m_FlyShoesConfig.LoadTable(sFilePath);}

	bool LoadNpcConfig(LPCTSTR sFilePath) { return m_monsterProvider.LoadNpcs(sFilePath);}
	//加载祝福
	bool LoadBlessConfig(LPCTSTR sFilePath) { return m_BlessConfig.LoadBlessCfg(sFilePath);}
	// 加载圣物 
	bool LoadHallowsConfig(LPCTSTR sFilePath); 
	//读取职业初始化列表
	bool LoadVocationConfig(LPCTSTR sFilePath);  
	//读取1级属性到2级属性的转换表
	bool LoadPropertyConfig(LPCTSTR sFilePath); 
	//读取物品配置数据
	inline bool LoadStdItems(LPCTSTR sFilePath)
	{ return m_StdItemProvider.LoadStdItems(sFilePath); }

	//锻造配置表
	inline bool LoadForgeConfig(LPCTSTR sFilePath)
	{ return m_StdItemProvider.LoadForgeConfig(sFilePath); }
	//合成配置表
	inline bool LoadComposeConfig(LPCTSTR sFilePath)
	{ return m_StdItemProvider.LoadComposeConfig(sFilePath); }
	//套装
	inline bool LoadSuitItemConfig(LPCTSTR sFilePath)
	{ return m_StdItemProvider.LoadSuitItemConfig(sFilePath); }
	
	//套装
	inline bool LoadWarehousemConfig(LPCTSTR sFilePath)
	{ return m_StdItemProvider.LoadWarehousemConfig(sFilePath); }
	//套装
	inline bool LoadNumericalConfig(LPCTSTR sFilePath)
	{ return m_StdItemProvider.LoadNumericalConfig(sFilePath); }

	//装载装备衍生的数据
	inline bool LoadEquipDeriveData(LPCTSTR sFilePath)
	{
		return m_StdItemProvider.LoadEquipDeriveData(sFilePath); 
	}
	
	//读取精锻的数据
	inline bool LoadSmithData(LPCTSTR sFilePath) 
	{ return m_StdItemProvider.LoadSmithData(sFilePath); }
	
	//读取技能配置数据
	inline bool LoadSkillConfig(LPCTSTR sFilePath){ return m_skillProvider.LoadSkills(sFilePath); }

	//读取任务配置数据
	inline bool LoadQuestConfig(LPCTSTR sFilePath){ return m_QuestProvider.LoadQuest(sFilePath); }
	
	//读取排行榜配置数据
	inline bool LoadRankConfig(LPCTSTR sFilePath){ return m_RankProvider.LoadRank(sFilePath); }

	//加载游戏场景配置
	inline bool LoadSceneConfig(LPCTSTR sFilePath){ return m_Envir.LoadConfig(sFilePath); }
	//加载游戏背包配置
	bool LoadBagConfig(LPCTSTR sFilePath);
	bool readBagConfig();
	//泡点
	bool LoadHookExpConfig(LPCTSTR sFilePath);
	bool LoadHookExpConfig();	
	
	bool LoadGuildConfig(LPCTSTR sFilePath) { return m_GuildConfig.LoadGuildConfig(sFilePath);}

	bool LoadAchieve(LPCTSTR sFilePath) {return m_achieveConfig.LoadAchieves(sFilePath);}

	bool LoadAchieveEvent(LPCTSTR sFilePath){return m_achieveConfig.LoadAchieveEvents(sFilePath);}

	bool LoadBadgeConfig(LPCTSTR sFilePath){return m_achieveConfig.LoadBadgeConfig(sFilePath);}
	// //装备
	// bool LoadSuitConfig(LPCTSTR sFilePath) ; 
	
	bool LoadAiConfig(LPCTSTR sFilePath) {return m_aiProvider.LoadAI(sFilePath);}
	
	bool LoadPkConfig(LPCTSTR sFilePath) {return m_PkProvider.LoadPkConfig(sFilePath);}

	bool LoadSceneNpcDynPropConfig(LPCTSTR sFilePath);
	// 加载阵营相关的配置数据
	bool LoadCampConfig(LPCTSTR sFilePath);

	//装载称号数据
	bool LoadTitles(LPCTSTR sFilePath) {return m_achieveConfig.LoadTitles(sFilePath);}

	//装载头衔排行配置
	inline bool LoadTopTitles(LPCTSTR sFilePath){return m_achieveConfig.LoadTopTitles(sFilePath);};

	// 加载宝石配置数据
	bool LoadGemConfig(LPCTSTR sFilePath);
	
	//装载宠物的属性
	bool LoadPetConfig(LPCTSTR sFilePath) {return m_petProvider.LoadPets(sFilePath);}

	//装载玩家宠物的洗资质的数据
	//bool LoadPetSmithConfig(LPCTSTR sFilePath) {return m_petProvider.LoadSmithData(sFilePath);}

	//装载宠物的全局性的数据
	//bool LoadPetGlobalConfig(LPCTSTR sFilePath){return m_petProvider.LoadPetGlobalConfig(sFilePath);}
	
	//装载宠物的经验
	bool LoadPetExp(LPCTSTR sFilePath){return m_petProvider.LoadPetExp(sFilePath);}

	//装载成就的分组的数据
	bool LoadAchieveGroup(LPCTSTR sFilePath){return m_achieveConfig.LoadAchieveGroup(sFilePath);}

	//装载装备评价数据
	bool LoadItemEval(LPCTSTR sFilePath){return m_StdItemProvider.LoadItemEval(sFilePath);}

	//装载宠物的评价的数据
	//bool LoadPetEval(LPCTSTR sFilePath){return m_petProvider.LoadPetScore(sFilePath);}
	
	//装载宠物的技能
	//bool LoadPetSkill(LPCTSTR sFilePath) {return m_petProvider.LoadPetSkill(sFilePath);}

	//装载脚本定时回调的配置
	bool LoadScriptTimerConfig(LPCTSTR sFilePath)  {return m_scriptTimerConfig.LoadScriptTimerData(sFilePath);}

	bool LoadStoreRrereshData(LPCTSTR sFilePath) {return m_GameStore.LoadGameRefresh(sFilePath);}

	//加载好友配置
	bool LoadFriendConfig(LPCTSTR sFilePath) {return m_FriendConfig.LoadFriendConfig(sFilePath);}

	//邮件
	bool LoadMailConfig(LPCTSTR sFilePath) {return m_cMailCfg.LoadConfig(sFilePath);}
	//洗炼
	bool LoadRefiningConfig(LPCTSTR sFilePath) {return m_Refining.LoadConfig(sFilePath);}
	//加载NPC雕像配置
	bool LoadNpcStatueConfig(LPCTSTR sFilePath) {return m_miscProvider.LoadNpcStatueConfig(sFilePath);}

	//加载神魔
	bool LoadDemonConfig(LPCTSTR sFilePath) {return m_miscProvider.LoaddemonCfg(sFilePath);}

	//
	bool LoadFrenzyConfig(LPCTSTR sFilePath) {return m_miscProvider.LoadFrenzyCfg(sFilePath);}

	bool LoadOfficeConfig(LPCTSTR sFilePath) {return m_miscProvider.LoadOfficeCfg(sFilePath);}

	bool LoadChgVocConfig(LPCTSTR sFilePath) { return m_miscProvider.LoadChangeVocCfg(sFilePath); }

	//加载排行榜属性
	bool LoadRankPropConfig(LPCTSTR sFilePath) {return m_miscProvider.LoadRankPropConfig(sFilePath);}

	//加载工资配置 TO Delete
	bool LoadSalaryConfig(LPCTSTR sFilePath) {return m_miscProvider.LoadSalaryConfig(sFilePath);}

	bool LoadNewTitleConfig(LPCTSTR sFilePath) {return m_NewTitleConfig.LoadNewTitleConfig(sFilePath);}
	//复活特权
	bool LoadReviveDurationConfig(LPCTSTR sFilePath) {return m_reviveDurationConfig.LoadReviveDurationConfig(sFilePath);}
	//加载BUFF数据
	bool LoadBuffConfig(LPCTSTR sFilePath)
	{
		return m_buffConfig.Load(sFilePath);
	}

	bool LoadLootPetConfig(LPCTSTR sFilePath)
	{
		return m_LootPetConfig.Load(sFilePath);
	}
	bool LoadResonanceItemConfig(LPCTSTR sFilePath)
	{
		return m_ResonanceItemConfig.Load(sFilePath);
	}
	bool LoadRebateConfig(LPCTSTR sFilePath)
	{
		return m_RebateConfig.Load(sFilePath);
	}
	bool LoadEditionConfig(LPCTSTR sFilePath)
	{
		return m_EditionConfig.Load(sFilePath);
	}
	//转生
	bool LoadCircleConfig(LPCTSTR sFilePath) {return m_miscProvider.LoadCircleConfig(sFilePath);}
	//经脉
	bool LoadMeridianUpConfig(LPCTSTR sFilePath) {return m_miscProvider.LoadMeridiansCfg(sFilePath);}

	bool LoadStrengthenConfig(LPCTSTR sFilePath) {return m_StrengthenConfig.LoadStrengthenConfig(sFilePath);}

	//加载装备升星
	// bool LoadEquipStarConfig(LPCTSTR sFilePath) {return m_miscProvider.LoadEquipStarConfig(sFilePath);} 

	//加载装备铭刻配置
	bool LoadEquipInscriptConfig(LPCTSTR sFilePath) {return m_miscProvider.LoadEquipInscriptConfig(sFilePath);} 

	//加载月卡配置
	bool LoadMonthCardConfig(LPCTSTR sFilePath) {return m_miscProvider.LoadMonthCardConfig(sFilePath);} 

	//加载神炉配置
	bool LoadGodStoveConfig(LPCTSTR sFilePath) {return m_StdItemProvider.LoadGodStoveConfig(sFilePath);} 

	//加载交易行配置
 	bool LoadConsignmentConfig(LPCTSTR sFilePath) {return m_ConsignmentProvider.LoadConsignConfig(sFilePath);} 
	//加载开启等级配置
	bool LoadLevelConfig(LPCTSTR sFilePath) {return m_miscProvider.LoadLevelConfig(sFilePath);} 
	//装载翅膀的配置
	bool LoadWingsAngelConfig(LPCTSTR sFilePath) {return m_WingsAngel.LoadConfig(sFilePath);}

	//装载时装的配置
	bool LoadFashionConfig(LPCTSTR sFilePath) {return m_FashionCfg.LoadFashionCfg(sFilePath);}

	//装载英雄的配置
	bool LoadHeroConfig(LPCTSTR sFilePath) {return m_HeroConfig.LoadConfig(sFilePath);}
	//掉落表
	bool LoadDropGroupsConfig(LPCTSTR sFilePath) {return m_DropCfg.LoadDropGroupsConfig(sFilePath);}
	//装载成就的配置
	bool LoadAchieveConfig(LPCTSTR sFilePath) {return m_Achieve.LoadConfig(sFilePath);}
	//职业竞技的配置
	bool LoadCombatConfig(LPCTSTR sFilePath) { return m_CombatConfig.LoadCombatConfig(sFilePath);}
	//游戏设置的配置
	bool LoadSettingConfig(LPCTSTR sFilePath) { return m_SettingConfig.LoadConfig(sFilePath);}
	//道具升星的配置
	bool LoadItemUpStarConfig(LPCTSTR sFilePath) { return m_ItemUpStarCfg.LoadConfig(sFilePath);}
	//全局配置
	bool LoadGlobalConfig(LPCTSTR sFilePath);
	//合服
	bool LoadMergeTimesConfig(LPCTSTR sFilePath);
	bool LoadChatSystemConfig(LPCTSTR sFilePath);
	bool LoadLevelUpExpConfig(LPCTSTR sFilePath);
	bool LoadExpLmtOpendayConfig(LPCTSTR sFilePath);
	bool LoadDropItemConfig(LPCTSTR sFilePath);
	bool LoadBagRemainConfig(LPCTSTR sFilePath);
	bool LoadDeathDropConfig(LPCTSTR sFilePath);
	
	bool LoadChangeNameList();
	bool LoadChangeNameConfig(LPCTSTR sFilePath);
	bool LoadFirstLogin();
	bool LoadFirstLoginConfig(LPCTSTR sFilePath);
	
private:
	//加载商城销售排行
	inline bool LoadGameStoreRank(LPCTSTR sFilePath)
	{
		return m_GameStore.LoadGameStoreRank(sFilePath); 
	}

	//保存商城销售排行
	inline bool SaveGameStoreRank(LPCTSTR sFilePath)
	{ 
		return m_GameStore.SaveGameStoreRank(sFilePath); 
	}
	

	//装载商城的动态数据，这里的文件名没有用
	bool LoadStoreDynamicData(LPCTSTR sFilePath){return m_GameStore.LoadDynamicData();}


	//保存商城的动态数据，这里的文件名没有用
	bool SaveStoreDynamicData(LPCTSTR sFilePath){ return m_GameStore.SaveDynamicData();}

	
	//加载脚本定时函数补充调用记录(Script BootCall)
	bool LoadScriptBootCall(LPCTSTR sFilePath);
	
	//保存脚本定时函数补充调用记录
	bool SaveScriptBootCall(LPCTSTR sFilePath);
	
	//加载怪成长难度数据
	bool LoadBossGrowData(LPCTSTR sFilePath)
	{
		return m_monsterProvider.LoadBossGrowData(sFilePath);
	}

	//保存怪难度数据
	bool SaveBossGrowData(LPCTSTR sFilePath)
	{
		return m_monsterProvider.SaveBossGrowData(sFilePath);
	}

private:
	//按照加载数据项数据选择性的加载配置
	bool LoadConfigTerms(const DataProviderLoadTerm *pTermList, const INT_PTR nTermCount = LoadTermCount);
	
	//装载经验升级表
	bool LoadLevelUpExpTable(); 

	//装载每日经验上限，与开服天数相关
	bool LoadExpLmtOpendayTable();

	/*
	* Comments: 加载每日杀怪获取经验上限
	* @Return bool:
	*/
	bool LoadExpGetUpperLmtConfig();

	/*
	* Comments: 加载玩家名称颜色配置表
	* @Return bool:
	*/
	bool LoadActorNameColorConfig();


	//装载全局配置表
	bool LoadGlobalConfig();

	bool LoadMergeTimesConfig();

	bool LoadDropItemConfig();
	bool LoadBagRemainConfig();
	
	bool LoadDeathDropConfig();//死亡掉落系数
	bool LoadChatSystemConfig();

	// 加载角色消息过滤器
	bool LoadActorMsgFilter();

	//杀怪经验衰减
	bool LoadKillMonsterExpReduceTable();

	//转生等级差-杀怪经验衰减
	bool LoadZSLevelExpPercentConfig(LPCTSTR sFilePath); 
	bool LoadZSLevelExpPercentConfig(); 
	
	//装载组队杀怪获得的经验加成
	bool LoadTeamKillMonsterExp();

	//装载组队杀怪获得的经验加成buf图标
	bool LoadTeamKillMonsterExpBuf();

	//装载各等级的最大的声望
	bool LoadRenownLimit();

	// 加载阵营基本配置数据
	bool LoadCampBasicConfig();
	// 加载阵营江湖地位配置
	bool LoadCampJHDWConfig();

	/*
	* Comments:装载语言包的配置
	* Param TIPMSGCONFIG & dataConfig:
	* Param LPCTSTR sTableName:
	* @Return bool:
	*/
	//bool LoadLangConfig(TIPMSGCONFIG & dataConfig,LPCTSTR sTableName);

	/*
	* Comments:装载属性转换表
	* Param PLEVEL1PROPERTYTRANSFER pConfig:属性转换表的指针
	* Param char * tableName:全局表的名字
	* @Return bool:成功返回true,否则返回false
	*/
	bool LoadProperty(PLEVEL1PROPERTYTRANSFER pConfig,char * tableName);

protected:
	 bool LoadNameColorItem(const char *pItemName, unsigned int &clr);

	 bool PreLoadFile(LPCSTR sFilePath);
private:
	static const TCHAR FileDirName[];	//文件目录名
	static const TCHAR IncludeFileName[];	//定义包含文件名称

private:
	CLogicServer*			m_pLogicServer;
	CLogicEngine*			m_pLogicEngine;
	
	VOCATIONINITTABLE       m_vocation; //职业的初始化

	CNoticeProvider			m_noticeProvider;
	CNSNNoticeProvider		m_NsnNoticeProvider;//NSN系统提示
	CActivityProvider		m_activityProvider;
	CStdItemProvider		m_StdItemProvider;//物品数据配置读取器
	CQuestProvider			m_QuestProvider;
	CRankProvider           m_RankProvider;
	CSkillProvider          m_skillProvider ; //技能数据
	CEnvirConfig			m_Envir;
	TIPMSGCONFIG            m_oldTipmsg;		//系统提示的消息 TO DELETE
	NEWTIPMSGCONFIG         m_tipmsg;		//系统提示的消息

	TIPMSGCONFIG            m_monsterShout; //怪物喊话的配置

	TIPMSGCONFIG            m_ActorMoodLang;	//玩家心情（签名）语言包
	
	CPetProvider            m_petProvider ; //宠物的读取类

	CGameStore				m_GameStore;//游戏商城

	BAGCONFIG               m_bagConfig; //背包的配置

	CGuildProvider			m_GuildConfig;//帮派的配置
	CMonsterProvider        m_monsterProvider;// 怪物的配置
	
	PROPERTYCONFIG          m_propertyConfig; //玩家的经验升级表，杀怪经验衰减表，属性转换表,背包，泡点

	// SUITCONFIG              m_suitConfig; //套装的属性

	CMiscProvider			m_miscProvider;

	CFriendProvider			m_FriendConfig;		//好友的配置

	CMailCfg				m_cMailCfg;		//邮件
	CRefiningCfg              m_Refining;//洗炼
	

	CAiProvider             m_aiProvider ; //
	ConsignmentProvider		m_ConsignmentProvider;	//交易行
	CPkProvider				m_PkProvider;
	CAchieveProvider         m_achieveConfig;
	SceneNpcDynPropConfig	m_npcDynPropConfig;		// 副本NPC动态属性刷新配置

	CObjectAllocator<char>		m_DataAllocator;

	CVector<unsigned int>		m_vecExpGetUpperLmt;	// 每日杀怪获取经验上限

	ACTOR_NAME_CLR_TABLE		m_nameClrTable;			// 角色名称颜色配置信息

	CNewTitleProvider m_NewTitleConfig;			//新的头衔
	CReviveDurationProvider 	m_reviveDurationConfig; //复活特权

	CBuffProvider	m_buffConfig;
	CLootPetProvider	m_LootPetConfig;
	CResonanceItemProvider	m_ResonanceItemConfig;		//共鸣装备
	CRebateProvider			m_RebateConfig;				//返利系统
	CEditionProvider		m_EditionConfig;			//差异表

	RENOWNLIMIT                 m_renownLimit;          // 每个等级的的声望的最大的值
	CScriptTimerConfig          m_scriptTimerConfig;    // 脚本定时回调的配置

	CWingsAngel					m_WingsAngel;			//翅膀的配置
	CFashionCfg					m_FashionCfg;			//时装的配置
	CHeroProvider               m_HeroConfig;           //英雄的配置
	CAchieve					m_Achieve;				//成就的配置
	CGameSetting				m_SettingConfig;		//游戏设置的配置
	CStrengthenCfg              m_StrengthenConfig;//强化配置

	DropGroupCfg                m_DropCfg;              //掉落配置

	CCombatRankProvider			m_CombatConfig;		//职业竞技配置

	CHallowsthenCfg*            m_pHallowsthenConfig;	//圣物配置  

	CFlyShoes					m_FlyShoesConfig;	//飞鞋配置
	CBlessCfg                   m_BlessConfig; //祝福
	EXPLMTOPENDAY 				m_ExpLmtOpendayConfig; //与开服天数相关的经验限制

	ChangeNameList  	m_ChangeNameList; //变更名字列表
	std::vector<int> 	m_FirstLoginKeySet;	//新角色的按键设置
	std::vector<int> 	m_FirstLoginDress;	//新角色的默认穿戴
	std::vector<ItemAward> 	m_WarriorFirstLoginAwards;	//新战士角色的默认物品
	std::vector<ItemAward> 	m_MagicianFirstLoginAwards;	//新法师角色的默认物品
	std::vector<ItemAward> 	m_WizardFirstLoginAwards;	//新道士角色的默认物品
	ItemUpStarConfig           m_ItemUpStarCfg;//道具升星配置
};
