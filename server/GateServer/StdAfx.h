#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <stdarg.h>

#ifndef _USE_TRY_CATCH
	#define _USE_TRY_CATCH 　　
#endif 

#ifdef WIN32
#include <crtdbg.h>
#include <tchar.h>
#include <Windows.h>
#include <WinSock2.h>
#include <MSWSock.h>
#endif


#include <_ast.h>
#include <_memchk.h>
#include <Tick.h>
#include <Lock.h>
#include <Thread.h>
#include <QueueList.h>
#include "ServerDef.h"
#include "GateProto.h"
#include "CommonDef.h"
#include "ShareUtil.h"
#include "BufferAllocator.h"
#include "AppItnMsg.h"
#include <time.h>
#include <Stream.h>
#include "TimeStat.h"
#include "sha1.h"
#include "base64.h"

#include <CustomSocket.h>
#include "DataPacket.hpp"
#include "SendPackPool.h"
#include "CustomWorkSocket.h"
#include "CustomClientSocket.h"
#include "CustomServerClientSocket.h"
#include "CustomServerSocket.h"
#include "wrand.h"

#include "StackWalker.h"
#include "DefExceptHander.h"
#include "CustomExceptHander.h"


#include "crc.h"
#include "Encrypt.h"
//#include "NetworkDataHandler.h" 
#include "GateServerProto.h"
#include "CustomGateClient.h"

//这些是Lua读写配置需要添加的函数
extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#include <Stream.h>
#include <wyString.h>

#include <RefClass.hpp>

#include <RefString.hpp>
#include "CustomLuaMyLoad.h"
#include "CustomLuaScript.h"
#include "CustomLuaConfig.h"
#include "LuaToBP.h"

//using namespace wylib;
using namespace wylib::stream;
using namespace jxSrvDef;







