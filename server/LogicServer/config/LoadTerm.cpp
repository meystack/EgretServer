#include "StdAfx.h"
#include "LogicDataProvider.h"

//定义CLogicDataProvider的加载项列表
const CLogicDataProvider::DataProviderLoadTerm CLogicDataProvider::LoadTermList[] = 
{
	
	{ TRUE, enCDType_ChgVoc, _T("TransferConfig"),_T("data/config/Transfer/Transfer.config"), &CLogicDataProvider::LoadChgVocConfig},
	{ TRUE, enCDType_OFFICE, _T("OfficeConfig"),_T("data/config/Office/Office.config"), &CLogicDataProvider::LoadOfficeConfig},
	{ TRUE, enCDType_Notice, _T("NoticeConfig"),_T("data/config/WholeNotice/WholeNotice.config"), &CLogicDataProvider::LoadNoticeConfig},
	//{ TRUE, enCDType_NSNNotice, _T("NSNNoticeConfig"),_T("data/config/WholeNotice/NSNNotice.config"), &CLogicDataProvider::LoadNSNNoticeConfig},
	{ TRUE, enCDType_NSNNotice, _T("NSNNoticeConfig"),_T("data/config/WholeNotice/NSNotice.config"), &CLogicDataProvider::LoadNSNNoticeConfig},
	{ TRUE, enCDType_SETTING, _T("SettingConfig"),_T("data/config/setting/Setting.config"), &CLogicDataProvider::LoadSettingConfig},
	//{ TRUE, enCDType_AchieveConfig, _T("AchievePhasecfg"), _T("data/config/achieve/AchieveCfg.txt"), &CLogicDataProvider::LoadAchieveConfig},
	{ TRUE, enCDType_AchieveConfig, _T("AchievePhasecfg"), _T("data/config/achieve/AchieveConfig.config"), &CLogicDataProvider::LoadAchieveConfig},
	{ TRUE, enCDType_WingConfig, _T("WingsAngelPhasecfg"), _T("data/config/AlmirahSystem/WingCfg.txt"), &CLogicDataProvider::LoadWingsAngelConfig},
	{ TRUE, enCDType_RankProp, _T("RankProp"), _T("data/config/rank/RankProp.txt"), &CLogicDataProvider::LoadRankPropConfig},	//需要在其他配置之前读取完成以便初始化排行榜
	{ TRUE, enCDType_AI, _T("Ai"), _T("data/config/monster/AI.config"), &CLogicDataProvider::LoadAiConfig },
	{ TRUE, enCDType_SceneNpcDynPro, _T("SceneNpcDynProp"), _T(""), &CLogicDataProvider::LoadSceneNpcDynPropConfig },
	{ TRUE, enCDType_Property, _T("Property"), _T(""), &CLogicDataProvider::LoadPropertyConfig },
	{ TRUE, enCDType_Monster, _T("Monster"),  _T("data/config/monster/StdMonster.config"),&CLogicDataProvider::LoadMonsterConfig },
	{ TRUE, enCDType_NPC, _T("Npc"),  _T("data/config/npc/StdNpc.txt"),&CLogicDataProvider::LoadNpcConfig },
	{ TRUE, enCDType_Vocation, _T("Vocation"), _T(""), &CLogicDataProvider::LoadVocationConfig },
	{ TRUE, enCDType_Item, _T("Item"), _T("data/config/item/StdItem.txt"), &CLogicDataProvider::LoadStdItems },
	{ TRUE, enCDType_EquipDerive, _T("Item_derive"), _T("data/config/item/EquipDerive.txt"), &CLogicDataProvider::LoadEquipDeriveData },
	{ TRUE, enCDType_Skill, _T("Skill"), _T("data/config/skill/SkillConfig.config"), &CLogicDataProvider::LoadSkillConfig },
	{ TRUE, enCDType_Bag, _T("Bag"), _T("data/config/item/BagConfig.config"), &CLogicDataProvider::LoadBagConfig },
	{ TRUE, enCDType_BuffConfig, _T("BuffConfig"), _T("data/config/buff/StdBuff.txt"), &CLogicDataProvider::LoadBuffConfig},
	{ TRUE, enCDType_Scene, _T("Scene"), _T("data/envir/staticZone.txt"), &CLogicDataProvider::LoadSceneConfig },
	{ TRUE, enCDType_Quest, _T("Quest"), _T("data/config/task/TaskDisplay.config"), &CLogicDataProvider::LoadQuestConfig },
	{ TRUE, enCDType_Guild, _T("Guild"), _T("data/config/guild/Guild.txt"), &CLogicDataProvider::LoadGuildConfig },
	// { TRUE, enCDType_Suit, _T("Suit"), _T("data/config/item/SuitConfigs.txt"), &CLogicDataProvider::LoadSuitConfig },	
	{ TRUE, enCDType_PK, _T("PK"), _T("data/config/PKConfig.config"), &CLogicDataProvider::LoadPkConfig },
	{ TRUE, enCDType_Smith, _T("Smith"), _T("data/config/item/Smiths.txt"), &CLogicDataProvider::LoadSmithData },	
	{ TRUE, enCDType_Title, _T("Title"), _T("data/config/achieve/titles.txt"), &CLogicDataProvider::LoadTitles },
	{ TRUE, enCDType_TopTitle, _T("TopTitle"), _T("data/config/rank/HeadTitle.txt"),&CLogicDataProvider::LoadTopTitles },
	{ TRUE, enCDType_PetExp, _T("pet_exp"), _T("data/config/pet/PetExp.txt"), &CLogicDataProvider::LoadPetExp },
	{ TRUE, enCDType_ItemEval, _T("item_eval"), _T("data/config/item/EquipValuation.config"), &CLogicDataProvider::LoadItemEval },
	{ TRUE, enCDType_TimerConfig, _T("timer"), _T("data/config/misc/ScriptTimer.txt"), &CLogicDataProvider::LoadScriptTimerConfig },
	{ TRUE, enCDType_StoreRefresh, _T("store_refresh"), _T("data/config/store/RefreshStore.txt"), &CLogicDataProvider::LoadStoreRrereshData },
	{ TRUE, enCDType_SalaryConfig, _T("SalaryConfig"), _T("data/config/guide/SignInAndSalary.txt"), &CLogicDataProvider::LoadSalaryConfig},
	{ TRUE, enCDType_WILDBOSSConfig, _T("WILDBOSSConfig"), _T("data/config/boss/WildBossConfig.txt"), &CLogicDataProvider::LoadWildBossConfig},
	{ TRUE, enCDType_WORLDBOSSConfig, _T("WORLDBOSSConfig"), _T("data/config/boss/newboss.txt"), &CLogicDataProvider::LoadWorldBossConfig},
	{ TRUE, enCDType_NewTitleConfig, _T("TitleConfig"), _T("data/config/Title/Titles.txt"), &CLogicDataProvider::LoadNewTitleConfig},	
	{ TRUE, enCDType_CircleConfig, _T("CircleConfig"), _T("data/config/circle/CircleConfig.config"), &CLogicDataProvider::LoadCircleConfig},		
	// { TRUE, enCDType_EquipStarConfig, _T("EquipStar"), _T("data/config/Equip/EquipStar.txt"), &CLogicDataProvider::LoadEquipStarConfig},
	{ TRUE, enCDType_EquipInscriptConfig, _T("EquipInscript"), _T("data/config/Equip/EquipInscriptConfig.txt"), &CLogicDataProvider::LoadEquipInscriptConfig},
	{ TRUE, enCDType_MonthCard, _T("MonthCard"), _T("data/config/MonthCard.config"), &CLogicDataProvider::LoadMonthCardConfig},
	{ TRUE, enCDType_MonthCard, _T("MonthCard"), _T("data/config/Rage/Rageconst.config"), &CLogicDataProvider::LoadFrenzyConfig},
	

	{ TRUE, enCDType_GodStoveConfig, _T("GodStoveConfig"), _T("data/config/godstove/GodStove.config"), &CLogicDataProvider::LoadGodStoveConfig},	
	{ TRUE, enCDType_ConsignementCondfig, _T("ConsignmentConfig"), _T("data/config/tradebank/tradebank.txt"), &CLogicDataProvider::LoadConsignmentConfig},
	{ TRUE, enCDType_LevelConfig, _T("LevelConfig"), _T("data/config/LevelConfig.txt"), &CLogicDataProvider::LoadLevelConfig},
	{ TRUE, enCDType_HeroConfig, _T("HeroConfig"), _T("data/config/Hero/HeroConfig.txt"), &CLogicDataProvider::LoadHeroConfig},
	{ TRUE, enCDType_CombatConfig, _T("Combat"), _T("data/config/combat/combatRank.txt"), &CLogicDataProvider::LoadCombatConfig },
	{ TRUE, enCDType_GlobalConfig, _T("Global"), _T("data/config/GlobalConf.config"), &CLogicDataProvider::LoadGlobalConfig },
	
	{ TRUE, enCDType_DropConfig, _T("drop"), _T("data/config/drop/drop.txt"), &CLogicDataProvider::LoadDropGroupsConfig },
	{ TRUE, enCDType_ChatSysConfig, _T("ChatSystemConf"), _T("data/config/ChatSystemConfig.config"), &CLogicDataProvider::LoadChatSystemConfig },
	{ TRUE, enCDType_DropItemConfig, _T("DropItemConst"), _T("data/config/DropItemConst.config"), &CLogicDataProvider::LoadDropItemConfig },
	{ TRUE, enCDType_HookExpConfig, _T("hoopExpConfig"), _T("data/config/hookexp.config"), &CLogicDataProvider::LoadHookExpConfig },
	{ TRUE, enCDType_BagRemainConfig, _T("enCDType_BagRemainConfig"), _T("data/config/BagConfigRemain.config"), &CLogicDataProvider::LoadBagRemainConfig },
	{ TRUE, enCDType_LevelUpConfig, _T("LevelUp"), _T("data/config/level/LevelUpExp.config"), &CLogicDataProvider::LoadLevelUpExpConfig },
	{ TRUE, enCDType_ComposeConfig, _T("Forge"), _T("data/config/Forge/Forge.txt"), &CLogicDataProvider::LoadForgeConfig },
	{ TRUE, enCDType_ForgeConfig, _T("Compose"), _T("data/config/Merge/ItemMerge.config"), &CLogicDataProvider::LoadComposeConfig },
	{ TRUE, enCDType_SopConfig, _T("Shop"), _T("data/config/shop/shop.config"), &CLogicDataProvider::LoadGameStoreConfig },
	{ TRUE, enCDType_DeathDropConfig, _T("DeathDrop"), _T("data/config/DeathDropConfig.config"), &CLogicDataProvider::LoadDeathDropConfig },
	{ TRUE, enCDType_MeridianUpCfg, _T("MeridianUpCfg"), _T("data/config/meridians/Meridians.config"), &CLogicDataProvider::LoadMeridianUpConfig },
	{ TRUE, enCDType_FlyShoesIndex, _T("FlyTable"), _T("data/config/flyshoes/FlyTable.config"), &CLogicDataProvider::LoadFlyShoesIndex },
	{ TRUE, enCDType_FlyShoesTable, _T("FlyLevel"), _T("data/config/flyshoes/FlyLevel.config"), &CLogicDataProvider::LoadFlyShoesTable },
	{ TRUE, enCDType_BlessTable, _T("Bless"), _T("data/config/bless/bless.txt"), &CLogicDataProvider::LoadBlessConfig },
	{ TRUE, enCDType_ExpLmtOpenday, _T("ExpLmtOpenday"), _T("data/config/level/ExpLmtOpenday.config"), &CLogicDataProvider::LoadExpLmtOpendayConfig },
	{ TRUE, enCDType_ChangeName, _T("ChangeName"), _T("data/config/ChangeName.config"), &CLogicDataProvider::LoadChangeNameConfig },
	{ TRUE, enCDType_FirstLogin, _T("FirstLogin"), _T("data/config/FirstLogin.config"), &CLogicDataProvider::LoadFirstLoginConfig },
	{ TRUE, enCDType_Suit, _T("SuitItem"), _T("data/config/suit/suitItem.config"), &CLogicDataProvider::LoadSuitItemConfig },
	{ TRUE, enCDType_NumericalConfig, _T("Numerical"), _T("data/config/item/NumericalIcon.config"), &CLogicDataProvider::LoadNumericalConfig },
	{ TRUE, enCDType_StrengthenConfig, _T("Strengthen"), _T("data/config/strengthen/strengthen.txt"), &CLogicDataProvider::LoadStrengthenConfig },
	{ TRUE, enCDType_FashionConfig, _T("Fashion"), _T("data/config/fashion/fashion.txt"), &CLogicDataProvider::LoadFashionConfig },
	{ TRUE, enCDType_MailConfig, _T("mail"), _T("data/config/mail/mail.config"), &CLogicDataProvider::LoadMailConfig },
	{ TRUE, enCDType_RefiningConfig, _T("refining"), _T("data/config/refining/refining.config"), &CLogicDataProvider::LoadRefiningConfig },
	{ TRUE, enCDType_UpStarConfig, _T("upStart"), _T("data/config/upstar/Upstarconfig.config"), &CLogicDataProvider::LoadItemUpStarConfig },
	{ TRUE, enCDType_DemonConfig, _T("demons"), _T("data/config/demons/demons.config"), &CLogicDataProvider::LoadDemonConfig },
	{ TRUE, enCDType_RankConfig, _T("ranks"), _T("data/config/rank/ranks.config"), &CLogicDataProvider::LoadRankConfig },
	{ TRUE, enCDType_WarehouseConfig, _T("warehouse"), _T("data/config/Warehouse/Warehouse.config"), &CLogicDataProvider::LoadWarehousemConfig },
	
	{ TRUE, enCDType_ZSLevelExpPercentConfig, _T("ZSLevelExpPercentConfig"), _T("data/config/level/ZSLevelExpPercent.config"), &CLogicDataProvider::LoadZSLevelExpPercentConfig },
	{ TRUE, enCDType_MergeTimesConfig, _T("MergeTimes"), _T("data/MergeTimes.txt"), &CLogicDataProvider::LoadMergeTimesConfig },
	{ TRUE, enCDType_Activity, _T("ActivityConfig"), _T("data/config/activity/Activity.txt"), &CLogicDataProvider::LoadActivityConfig},
	{ TRUE, enCDType_Hallows, _T("HallowsConfig"), _T("data/config/holyitem/HallowsConfig.config"), &CLogicDataProvider::LoadHallowsConfig },
	{ TRUE, enCDType_ReviveDurationConfig, _T("ReviveDurationConfig"), _T("data/config/Revive/ReviveDuration.config"), &CLogicDataProvider::LoadReviveDurationConfig},
	{ TRUE, enCDType_LootPet, _T("LootPetConfig"), _T("data/config/lootPet/lootPet.config"), &CLogicDataProvider::LoadLootPetConfig},
	{ TRUE, enCDType_ResonanceItem, _T("ResonanceItem"), _T("data/config/suit/ResonanceItem.config"), &CLogicDataProvider::LoadResonanceItemConfig},
	{ TRUE, enCDType_Rebate, _T("Rebate"), _T("data/config/Rebate.txt"), &CLogicDataProvider::LoadRebateConfig},
	{ TRUE, enCDType_Edition, _T("edition"), _T("data/config/edition.txt"), &CLogicDataProvider::LoadEditionConfig},   
};

const INT_PTR CLogicDataProvider::LoadTermCount = ArrayCount(CLogicDataProvider::LoadTermList);


const CLogicDataProvider::DataProviderLoadTerm* CLogicDataProvider::GetDataProviderLoadConfig(const ConfigDataType cdt) 
{
	for (size_t i = 0; i < ArrayCount(LoadTermList); i++)
	{
		if (LoadTermList[i].cdType == cdt)
			return &LoadTermList[i];
	}

	return NULL;
}
