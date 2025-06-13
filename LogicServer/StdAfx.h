#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <crtdbg.h>
#include <tchar.h>
#include <Windows.h>
#endif
#include<signal.h>
#include <cstring>
#include <algorithm>
#include <_ast.h>
#include <_memchk.h>
#include <Thread.h>
#include <CustomSocket.h>
#include <Lock.h>
#include <Tick.h>
#include <RefString.hpp>
#include <Stream.h>
#include <wrand.h>
#include <bzhash.h>
#include <math.h>
#include <deque>
#include<string>
#include "except/CustomExceptHander.h"
extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <ltm.h>
#include <tolua++.h>
}
#define nullptr NULL
//定义逻辑服务器，这个不要去掉！！
#ifndef __LOGIC_SERVER__
	#define  __LOGIC_SERVER__
#endif 



//!!!是否开启try_catch,在需要跟问题的时候把下面的屏蔽
//开启try_catch 主要用捕获一些偶然出现的异常，如果出现异常并无法找到原因，就把下面的定义屏蔽，生成dump
//服务器将停止运行



#ifndef _DEBUG

#ifndef _USE_TRY_CATCH
	#define _USE_TRY_CATCH
#endif
#endif 


#ifndef USE_HASK_FILTER
	#define  USE_HASK_FILTER
#endif 

//#ifdef _DEBUG
#define USE_OPTIMIZED //使用优化
//#endif

#define GRID_USE_STL_MAP

#ifdef USE_OPTIMIZED
#define ENTITY_MANAGER_USE_STL_MAP //实体管理器从 CList 换成 std::map
#define MONSTER_MANAGER_BY_SCENE //怪物实体run 从实体管理器换成scene
#endif

#include <_osdef.h>
#include "CommonDef.h"
#include "NamedPipe.h"
#include <md5.h>
#include <crc.h>
#include <QueueList.h>
#include <LinkedList.h>
#include "ShareUtil.h"
#include "base/SaveChatMsg.h"
#include "BufferAllocator.h"
#include "base/BuffAllocStatHelper.h"
#include "AppItnMsg.h"
#include "DataPacket.hpp"
#include "ObjectAllocator.hpp"
#include "SingleObjectAllocator.hpp"
#include "FDOP.h"
#include "CustomHashTable.h"
//#include "EDPass.h"
#include "DefExceptHander.h"
#include "SendPackPool.h"
#include "CustomWorkSocket.h"
#include "CustomClientSocket.h"
#include "CustomServerClientSocket.h"
#include "CustomServerSocket.h"
#include "CustomServerClientSocketEx.h"
#include "CustomLuaMyLoad.h"
#include "CustomLuaScript.h"
#include "CustomLuaConfig.h"
#include "CustomLuaPreProcess.h"
#include "CLVariant.h"
#include "LuaCLVariant.h"
#include "CustomLuaProfiler.h"
#include "LangTextSection.h"
#include "LuaLangTextSection.h"
#include "MiniDateTime.h"
#include "TimeCall.h"
#include "AMClient.h"
//#include "WinService.h"
#include "ObjectCounter.h"
#include "base/Utility.h"
#include "TimeStat.h"
#include "misc/RandHit.h"
#include "script/interface/AchieveEvent.h" //成就事件的定义
#include "script/interface/CommonActivityDef.h"
#include "ServerDef.h"
#include "CommonDef.h"
#include "ServerMailDef.h"

#include "InterServerComm.h"
#include "networkdata/LogicServerCmd.h"
#include "CustomJXClientSocket.h"
#include "CustomJXServerClientSocket.h"
#include "GateProto.h"
#include "CustomServerGateUser.h"
#include "CustomServerGate.h"
#include "CustomGateManager.h"
#include "CustomGlobalSession.h"
#include "CustomSessionClient.h"
#include <MBCSDef.h> //关键字过滤用的

#include "LogicDef.h"
#include "LogType.h"
#include "Locallog/LocalSender.h"
#include "LogSender.h"
#include "FileLogger.h"

#include "AdvanceAstar.h"

#include "base/LogicLuaAllocator.h"
#include "base/CustomLogicLuaConfig.h"
#include "base/BaseScriptExportObject.h"

#include "base/EntityAttriFlag.h"
#include "base/EntityMsg.h"
#include "QuestData.h"


#include "misc/FHDefs.h"
#include "misc/AreaRegion.h"
#include "misc/caches/CustomConfigCacher.h"
#include "misc/ScoreRecorder.h"

#include "base/Container.h"
#include "base/TimerMgr.h"
#include "gate/LogicGateUser.h"
#include "gate/LogicGate.h"
#include "gate/LogicGateManager.h"
#include "session/LogicSession.h"
#include "session/LogicSSClient.h"

#include "config/VSPDef.h"
#include "config/TipmsgConfig.h"
#include "language/LanguageTextProvider.h"

#include "script/hlp/LuaHlp.h"
#include "script/interface/SystemParamDef.h"

//属性、物品和BUFF
#include "script/interface/BuffType.h"


#include "attr/AttrDef.h"
#include "attr/AttrCalc.h"
#include "attr/AttrEval.h"


#include "script/interface/TipmsgID.h"
#include "script/interface/staticType.h"


#include "config/FileReader.h"
#include "config/ConfigData.h"
#include "config/ScriptTimerConfig.h"


//配置文件
#include  "config/LogicServerConfig.h"
#include "property/PropertyDef.h"
#include "script/interface/PropertyIDDef.h"
#include "property/PropertySet.h"
#include "script/interface/ActorEventDef.h"
#include "script/interface/MapAreaAttribute.h"
#include "ActorDbData.h"
#include "SkillDbData.h"
#include "GuildDb.h"
#include "ActivityDb.h"
#include "FeeDb.h"
#include "Fuben.h"
#include "InterAction.h"
#include "Msg.h"
#include "HeroData.h"
#include "AchieveData.h"
#include "BossData.h"

#include "ActorOfflineMsg.h"

#include "item/StdItem.h"
#include "item/StdItemProvider.h"


#include "store/GameStoreRank.h"
#include "store/GameStore.h"

//这里放读取文件的数据结构的定义
#include "config/AchieveCfg.h"								//成就
#include "config/QuestConfig.h"
#include "config/QuestProvider.h"
#include "config/PkProvider.h" 
#include "config/GuildProvider.h"
#include "config/FriendProvider.h"
#include "config/BuffProvider.h"
#include "config/NewTitleProvider.h"
#include "config/ConsignmentProvider.h"
#include "config/ReviveDurationProvider.h"		//复活特权配置


#include "config/SoulConfig.h"
#include "config/AlmirahConfig.h"	// 衣橱系统配置
#include "config/WingsAngelcfg.h"							//翅膀
#include "config/HeroProvider.h"							//英雄
#include "config/DropGroupCfg.h"
#include "config/GameSettingCfg.h"
#include "config/StrengthConfig.h"
#include "config/FashionCfg.h"
//#include "config/HallowsConfig.h"
#include "config/LootPetProvider.h"
#include "config/RebateProvider.h"			//返利
#include "config/EditionProvider.h"			//返利
#include "config/ResonanceItemProvider.h"
#include "config/SuitConfig.h"
#include "config/BagConfig.h"
#include "config/SceneConfig.h"
#include "config/MonsterConfig.h"
#include "config/VocationConfig.h"
#include "config/EnvirConfig.h"
#include "config/PropertyConfig.h"
#include "config/SkillConfig.h"
#include "config/SkillProvider.h"
#include "config/MonsterProvider.h"
#include "config/AiConfig.h"
#include "config/AiProvider.h"
#include "config/AchieveProvider.h"
#include "config/SceneNpcDynPropConfig.h"
#include "misc/ActorNameColor.h"
#include "config/PetProvider.h"
#include "config/CombatProvider.h"
#include "config/FlyShoesConfig.h"
#include "config/BlessCfg.h"
#include "config/ActivityProvider.h"
#include "config/NoticeProvider.h"
#include "config/NSNNoticeProvider.h"
#include "config/mailCfg.h"
#include "config/Refining.h"
#include "config/ItemUpStarConfig.h"
#include "config/WingsAngelcfg.h"
#include "config/RankProvider.h"  
//#include "config/RebateConfig.h"
#include "config/MiscProvider.h"

#include "config/LogicDataProvider.h"//这个要在加载所有配置之后

#include "base/Component.h" 
#include "script/interface/ItemExportFun.h"
#include "script/ext/ScriptMemoryManager.h"
#include "script/value/ScriptValue.hpp"
#include "script/value/ScriptValueList.h"
#include "script/LogicScript.h"
#include "base/TimeSpan.h"      //时间管理器
#include "script/ext/ScriptTimeCall.h"
#include "base/TimerMgr.h"

//#include "gameLog/GameLogDef.h"
#include "opt/EntityOPCollect.hpp"

#include "base/EntitySubSystem.h"
#include "entity/Entity.h"
#include "item/SuitCal.hpp"

#include "base/StringBuff.hpp"

#include "UserItem.h"

#include "item/UserItemAllocator.h"
#include "item/cnt/UserItemContainer.h"
#include "item/cnt/UserBag.h"
#include "item/cnt/UserStorage.h"
#include "item/cnt/DepotBag.h"
#include "item/cnt/UserDepot.h"

#include "team/DropBag.h" //掉落的定义
#include "team/Team.h"     //队伍的基础类
#include "script/interface/LogDef.h"

#include "attr/RandAttrSelector.h"
#include "attr/RandAttrReader.h"

#include "db/DataClient.h"
#include "mgrserv/BackStage.h"

#include "JXAbsGameMap.h"
#include "HandleStat.h"
#include "HandleMgr.h"
#include "base/HandleList.h"

#include "base/Guild.h"
//仇恨管理器
#include "ai/BattleHate.h" 

// 脚本的协议分发系统
#include "base/ScriptNetmsgDispatcher.h"
// 脚本的事件分发系统
#include "base/ScriptEventDispatcher.h"

#include "base/BossManager.h"//boss管理

//子系统的列表
#include "entitysystem/MotionStack.hpp" //移动堆栈
#include "buff/DynamicBuff.h"
#include "buff/BuffSystem.h"    //buff子系统
#include "quest/Quest.h"
#include "entitysystem/ObserverSystem.h" //观察者子系统
#include "entitysystem/MovementSystem.h" //移动子系统
#include "entitysystem/PropertySystem.h" //玩家的属性子系统

#include "item/cnt/EquipVessel.h"  //装备子系统

#include "item/cnt/UserEquip.h"  //装备子系统
#include "item/cnt/HeroEquip.h"  //装备

#include "quest/QuestSystem.h"   //任务子系统
#include "entitysystem/ChatSystem.h"  //聊天子系统
#include "skill/SkillCondition.h"
#include "skill/SkillResult.h"
#include "skill/EffectSystem.h"
#include "skill/SkillSubSystem.h" 		//技能子系统
#include "store/GameStoreBuySystem.h"	//购买商城物品的子系统
#include "entitysystem/DealSystem.h"  	//交易子系统
#include "entitysystem/GuildSystem.h"	//帮派子系统
#include "entitysystem/BasicFunctionsSystem.h" //基础功能子系统
#include "entitysystem/LootSystem.h" 	//拾取子系统
#include "team/TeamSystem.h" 			//队伍子系统
#include "entitysystem/FubenSystem.h"
#include "entitysystem/MsgSystem.h"
#include "entitysystem/PkSystem.h"
#include "misc/AsyncOpCollector.h"
#include "entitysystem/MiscSystem.h"  		//杂乱子系统

#include "entitysystem/AchieveSystem.h" 	//成就子系统
#include "entitysystem/EntityVarSubSystem.h"

#include "entitysystem/FriendSystem.h"
#include "entitysystem/HeroSystem.h"

#include "entitysystem/BossSystem.h"		//boss子系统
#include "entitysystem/MailSystem.h"		//邮件子系统
#include "entitysystem/AlmirahSystem.h"		// 衣橱子系统(翅膀)
//传奇系统
#include "entitysystem/StallData.h"
#include "entitysystem/StallSystem.h"
#include "entitysystem/NewTitleSystem.h"
#include "entitysystem/CGameSetsSystem.h"  		//游戏设置系统
#include "entitysystem/CheckSpeedSystem.h"			//加速检测外挂子系统
#include "entitysystem/TreasureSystem.h" //玩家宝物
#include "entitysystem/CombatSystem.h"		//职业竞技子系统
#include "entitysystem/CStaticCountSystem.h" //玩家计数器
#include "entitysystem/PetSystem.h" //玩家的宠物子系统
#include "entitysystem/ConsignmentSystem.h"			//交易行
#include "entitysystem/ActivitySystem.h"		//活动系统
#include "entitysystem/CStrengthenSystem.h"		//强化
#include "GhostDef.h"
#include "entitysystem/GhostSystem.h"		//神魔
#include "entitysystem/HallowsSystem.h"		//圣物
#include "entitysystem/CrossSystem.h"		//跨服
#include "entitysystem/ReviveDurationSystem.h"		//复活特权
#include "entitysystem/LootPetSystem.h"		//
#include "entitysystem/RebateSystem.h"		// 返利
#include "ai/BehaviorAI.h"
#include "ai/AnimalAI.h"
#include "ai/AggressorAI.h"
#include "ai/PassiveAI.h"
#include "ai/RandomFleeAI.h"
#include "ai/BaseAI.h"
#include "ai/NPCAI.h"
#include "ai/PetAI.h"
#include "ai/HeroAI.h"
#include "ai/WalkRideAi.h"
#include "ai/SpecialAI.h"
#include "ai/AIMgr.h"

//实体的列表
#include "entity/NonLive.h"
#include "entity/Creature.h"
#include "entity/NoAnimal.h"
#include "entity/Animal.h"
#include "entity/AdvanceAnimal.h"
#include "entity/Transfer.h"
#include "entity/Pet.h"
#include "entity/Fire.h"
#include "entity/Landscape.h"
#include "entity/Hero.h"
#include "entity/WalkRide.h"


#include "base/ActorDataPacketStatAlloc.h"
#include "base/ActorDataPacketStat.h"
#include "datadef/ActorDef.h"
#include "entity/Actor.h"
#include "entity/Monster.h"
#include "entity/Npc.h"
#include "entity/DropItemEntity.h"



#include "mgrserv/InterMgrServProto.h"

//管理器的列表
#include "base/GuildManger.h"  //工会管理器
#include "base/ActivityManager.h"  //活动管理器

#include "base/EntityManager.h" //实体管理器
#include "team/TeamManager.h"  //队伍管理器
#include "base/ChatManager.h"  //聊天管理器
#include "base/NoticeManager.h"  //公告管理器

#include "base/RankingMgr.h"	//排行榜管理器
#include "base/SimulatorMgr.h"
#include "base/GlobalVarMgr.h"		//给脚本保存数据用的

#include "base/TopTitleMgr.h"	//头衔管理器
#include "base/MiscMgr.h"		//杂七杂八的管理器
#include "base/OfflineUserMgr.h"  //离线玩家管理器
#include "base/OfflineCenterUserMgr.h"
#include "base/EncryptFunctionName.h"
#include "base/WorldLevelMgr.h"		//世界等级
#include "base/ConsignmentMgr.h"	//交易行
#include "base/MailMgr.h"	//  邮件


#include "scene/AStar.h"

//conponent
#include "scene/SceneDef.h"
#include "scene/EntityGrid.h"
#include "scene/Scene.h"
#include "scene/FuBenManager.h"
#include "scene/FuBen.h"
#include "networkdata/NetWorkMsgHandle.h" 

#include "script/interface/BoxDropDef.h"
#include "base/Ranking.h"
#include "base/BoxDrop.h"
#include "base/BackResLoadModule.h"
#include "transmit/TransmitMgr.h"
#include "Cross/CrossActorMgr.h"
#include "commonserver/CommonServer.h"
#include "commonserver/CommonServerClient.h"
#include "logicclient/CommonClient.h"

#include "CrossServer/CrossProto.h"
#include "CrossServer/CrossDataHandle.h"
#include "CrossServer/CrossServer.h"
#include "CrossServer/CrossClient.h"
#include "CrossServer/CrossServerManager.h"
#include "Crossclient/LocalCrossClient.h"


#include "base/MessageFilter.h"
#include "AsyncWorker.h"
#include "LogicEngine.h"
#include "LogicServer.h"
#include<map>
#include "script/interface/IntMap.h"

#ifndef ASSERT_POINTER
#define ASSERT_POINTER(p, type) assert((p) && sizeof(*(p)) == sizeof((type)))
#endif
std::vector<std::string> SplitStr(std::string str,const std::string pattern);



unsigned int getNowZeroTime();

//return  nLeftExtra + nInt + nRightExtra
std::string intToString(int nInt, std::string nLeftExtra = "", std::string nRightExtra = "");





