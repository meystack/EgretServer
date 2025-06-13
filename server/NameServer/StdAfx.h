#include <stdio.h>
#include <stdlib.h>
#include <string>
#ifdef WIN32
#include <WinSock2.h>
#include <MSWSock.h>
#include <tchar.h>
#include "WinService.h"
#endif

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
}
//#include "EDPass.h"
#include <mysql.h>


#include <_ast.h>
#include <_memchk.h>
#include <Tick.h>
#include <Lock.h>
#include <Thread.h>
#include <QueueList.h>
#include <CustomSocket.h>
#include <RefString.hpp>
#include <Stream.h>
#include <MBCSDef.h>
//#include "EDPass.h"

#include "ShareUtil.h"

#include "CustomLuaMyLoad.h"
#include "CustomLuaScript.h"
#include "CustomLuaConfig.h"

#include "BufferAllocator.h"
#include "AppItnMsg.h"
#include "SQL.h"
#include "DataPacket.hpp"
#include "SendPackPool.h"
#include "CustomWorkSocket.h"
#include "CustomServerClientSocket.h"
#include "CustomServerSocket.h"
#include "CustomServerClientSocketEx.h"



#include "ServerDef.h"
#include "InterServerComm.h"

#include "CustomJXServerClientSocket.h"
#include "GateProto.h"
#include "CustomServerGateUser.h"
#include "CustomServerGate.h"
#include "CustomGateManager.h"

#include "FileLogger.h"

#include "NSDef.h"
#include "NSConnection.h"
#include "NSServer.h"
#include "NSConfig.h"
#include "FDOP.h"
