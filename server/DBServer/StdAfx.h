#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <crtdbg.h>
#include <tchar.h>
#include <Windows.h>
#endif
#include <signal.h>
#include <_ast.h>
#include <_memchk.h>
#include <Thread.h>
#include <CustomSocket.h>
#include <Lock.h>
#include <Tick.h>
#include <QueueList.h>
#include"Define.h"

#define  StrToInt _StrToInt

#include "ShareUtil.h"
#include "BufferAllocator.h"
#include "AppItnMsg.h"

//#include "db/SqlHelper.h"
#include "DataPacket.hpp"
#include "DataPacketReader.hpp"
#include "SendPackPool.h"
#include "CustomWorkSocket.h"
#include "CustomClientSocket.h"
#include "CustomServerClientSocket.h"
#include "CustomServerClientSocketEx.h"
#include "CustomServerSocket.h"
#include "ServerDef.h"
#include "ServerMailDef.h"
#include "CustomJXClientSocket.h"
#include "GateProto.h"
#include "../../LogicServer/script/interface/PropertyIDDef.h"
#include "../../LogicServer/property/PropertyDef.h"
#include "../../LogicServer/property/PropertySet.h"
#include "ActorOfflineMsg.h"
#include "QuestData.h"


#include "LoadString.h"
#include "SQL.h"
#include "ObjectCounter.h"
#include "TimeStat.h"
#include "../../LogicServer/script/interface/SystemParamDef.h"
#include "CustomServerGateUser.h"
#include "CustomServerGate.h"
#include "CustomGateManager.h"
#include "CustomGlobalSession.h"
#include "CustomSessionClient.h"
#include "InterServerComm.h"
#include "ActorCacheDef.h"
#include "DBSessionClient.h"
#include "DBProto.h"
#include "JobZyCountMgr.h"
#include "EsqlMgr.h"
#include "GateDBRequestHandler.h"
#include "DBGateUser.h"
#include "DBGate.h"
#include "DBGateManager.h"
#include <Stream.h>
#include <MBCSDef.h> //关键字过滤用的
#include "DBDataServer.h"
#include "DBServer.h"
#include "../../LogicServer/attr/AttrDef.h"
#include "DBNSClient.h"
#include "CustomJXServerClientSocket.h"
#include "LinkedList.h"
#include "HandleStat.h"


#include "HandleMgr.h"
#include "LogicDBRequestHostInterface.h"
#include "LogicDBRequestHandler.h"
#include "DBDataCache.h"
#include "DBDataClientHandler.h"
#include "DBDataClient.h"



#include "MiniDateTime.h" //时间用的
using namespace jxSrvDef;
#include "PropertyDef.h"
#include "ActorDbData.h"
#include "SkillDbData.h"
#include "UserItem.h"
#include "PetData.h"
#include "HeroData.h"
#include "HallowsDef.h"
#include "GameSets.h"
#include "GhostDef.h"
//#include "encrypt/CRC.h"
//#include "encrypt/Encrypt.h"
//#include "dataProcess/NetworkDataHandler.h"
#include "FileLogger.h"
#include "LogSender.h"
#include "LogType.h"
#include "DBCenterClient.h"

#include <RefClass.hpp>
#include <RefString.hpp>
#include <time.h>

//#include "EDPass.h"



using namespace wylib::stream;
using namespace wylib::string;
using namespace jxInterSrvComm::DbServerProto;
using namespace jxInterSrvComm::NameServerProto;
extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#include "CustomLuaMyLoad.h"
#include "CustomLuaScript.h"
#include "CustomLuaConfig.h"
#include "DBConfig.h"
#include "StackWalker.h"
#include "DefExceptHander.h"
#include "CustomExceptHander.h"


//#include "WinService.h"

#ifdef _DEBUG

#else
/*
	#ifndef _USE_TRY_CATCH
		#define _USE_TRY_CATCH 　　
	#endif
*/

#endif

//#include "./misc/AllocForSTL.hpp"


