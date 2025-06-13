#include <stdio.h>
#include <stdlib.h>
#include <string>
#ifdef WIN32
#include <crtdbg.h>
#include <tchar.h>
#include <Windows.h>
#include <CommCtrl.h>
#endif

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
extern "C"
{
#include <lua.h>
#include <lauxlib.h>
}
#define StrToInt _StrToInt 

#define NO_USE_AMSERVER


#include "md5.h"
#include "ShareUtil.h"
#include "BufferAllocator.h"
#include "AppItnMsg.h"
#include "DataPacket.hpp"
#include "SQL.h"
#include "DataPacketReader.hpp"
#include "DataPacket.hpp"
//#include "EDPass.h"
#include "CustomLuaMyLoad.h"
#include "CustomLuaScript.h"
#include "CustomLuaConfig.h"
#include "SendPackPool.h"
#include "CustomWorkSocket.h"
#include "CustomClientSocket.h"
#include "CustomServerClientSocket.h"
#include "CustomServerSocket.h"
#include "CustomServerClientSocketEx.h"

#include "ServerDef.h"
#include "CommonDef.h"
#include "InterServerComm.h"
#include "CustomJXClientSocket.h"
#include "CustomJXServerClientSocket.h"
#include "GateProto.h"



#include "LogType.h"
#include "LogSender.h"
#include "FileLogger.h"
#include "CustomExceptHander.h"

//#include "AMClient.h"
#include "SSProto.h"

#include "RankMgr.h"

//#include "SSGateUser.h"
//#include "SSGate.h"
//#include "SSGateManager.h"
#include "SessionDBRequestHostInterface.h"
#include "SessionDataHandle.h"

#include "SessionClient.h"
#include "SessionServer.h"
#include "SessionCenter.h"
#include "SessionCenterClient.h"
#include "SSManager.h"
#include "ObjectCounter.h"
#include "TimeStat.h"

#include "PerformanceWatcher.h"
#include "PerformanceWatcher.h"
#include "SSConfig.h"
#include "wrand.h"
#include "FDOP.h"

using namespace jxSrvDef;
using namespace jxInterSrvComm;
