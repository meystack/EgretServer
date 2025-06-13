

//定义机器人，这个不要去掉！！
#ifndef __ROBOT_CLIENT__
	#define  __ROBOT_CLIENT__
#endif 
#include<signal.h>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
//#include <crtdbg.h>
//#include <tchar.h>
//#include <Windows.h>
//#include <CommCtrl.h>
#include <_ast.h>
#include <_memchk.h>
#include <Lock.h>
#include <Tick.h>
#include <QueueList.h>
#include <Thread.h>
#include <CustomSocket.h>
#include <Stream.h>
#include <ClassedWnd.h>
#include <RefString.hpp>
#include "wrand.h"
#include <vector> 
extern "C"
{
#include <lua.h>
#include <lauxlib.h>
}

#include "Encrypt.h"
#include "CommonDef.h"
#include "md5.h"
#include "ShareUtil.h"
#include "BufferAllocator.h"
#include "AppItnMsg.h"
#include "DataPacket.hpp"
#include "SQL.h"

#include "MiniDateTime.h"
#include "CustomHashTable.h"
#include "ObjectAllocator.hpp"
#include "CustomLuaScript.h"
#include "CustomLuaConfig.h"
#include "CustomLuaPreProcess.h"
#include "LangTextSection.h"
#include "LuaLangTextSection.h"
#include "DefExceptHander.h"
#include "SendPackPool.h"
#include "CustomWorkSocket.h"
#include "CustomClientSocket.h"
#include "CustomServerClientSocket.h"
#include "CustomServerSocket.h"
#include "CustomServerClientSocketEx.h"
#include "ServerDef.h"
#include "InterServerComm.h"
#include "CustomJXClientSocket.h"
#include "CustomJXServerClientSocket.h"
#include "GateProto.h"
#include "CustomServerGateUser.h"
#include "CustomServerGate.h"
#include "CustomGateManager.h"
//#include "CRC.h"
#include "crc.h"

#include "config/LogicLuaAllocator.h"
#include "config/CustomLogicLuaConfig.h"

//#include "NetworkDataHandler.h" 
#include "LogType.h"
#include "LogSender.h"
#include "../LogicServer/base/BuffAllocStatHelper.h"
#include "../LogicServer/script/interface/SystemParamDef.h"
#include "../LogicServer/config/FileReader.h"
#include "../LogicServer/base/Container.hpp"
#include "FileLogger.h"
#include "RobotDef.h"
//#include "../SessionServer/SSProto.h"  //Session服务器用到的通信协议
#include "DBProto.h"       //Db服务器用到的通信协议
#include "../LogicServer/attr/AttrDef.h"   
#include "UserItem.h"
#include "../LogicServer/config/ConfigData.h"
#include "../LogicServer/LogicDef.h"   //LOGIC服务器用到的通信协议
#include "../LogicServer/networkdata/LogicServerCmd.h"  //逻辑的网络消息的定义
#include "../LogicServer/script/interface/SystemParamDef.h"  //逻辑的一些基本的定义
#include "../LogicServer/base/TimerMgr.h"
#include "../LogicServer/script/interface/ActorEventDef.h"
#include "../LogicServer/script/interface/PropertyIDDef.h"
#include "../LogicServer/script/interface/MapAreaAttribute.h"
#include "../LogicServer/property/PropertyDef.h"
#include "../LogicServer/property/PropertySet.h"
#include "../LogicServer/config/AchieveCfg.h"
#include "../LogicServer/config/SceneConfig.h"
#include "../LogicServer/attr/AttrCalc.h"
#include "../LogicServer/item/StdItem.h"


#include "CommonDef.h"
#include "config/VSPDef.h"
#include "config/LanguageTextProvider.h"
#include "config/LogicLuaAllocator.h"
#include "config/CustomLogicLuaConfig.h"
#include "Scene/ClientMap.h"
#include "Scene/ClientSceneConfig.h"

#include "AI/AITree.h"
#include "AI/AITreeConfigLoader.h"
#include "AI/AStar.h"

#include "Entity/ClientEntity.h"
#include "SwapQueue.h"
#include "util/Utility.h"
#include "LogicSystem/NetMsgDef.h"
#include "Agent/Agent.h"     
#include "Scene/ClientScene.h"
#include "Scene/SceneConfigLoader.h"
#include "config/ItemConfig.h"
#include "config/ConfigLoader.h"


#include "LogicSystem/EntitySystem.h"     //子系统
#include "LogicSystem/DefaultSystem.h"   //默认子系统
#include "LogicSystem/SkillSystem.h"
#include "LogicSystem/ItemSystem.h"
#include "LogicSystem/EquipSystem.h"

#include "Agent/LogicAgent.h"
#include "RobotClient.h"

#include "RobotGroup.h"
#include "RobotManager.h"
#include "ObjectCounter.h"
#include "TimeStat.h"

#include "RobotConfig.h"


using namespace jxSrvDef;
using namespace jxInterSrvComm;
