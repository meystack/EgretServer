#pragma once
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
	#include <crtdbg.h>
	#include <tchar.h>
	#include <Windows.h>
#endif

#include "_osdef.h"
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

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}


#include <_ast.h>
#include <_memchk.h>
#include <Tick.h>
#include <Lock.h>
#include <Stream.h>
#include <QueueList.h>

#include "ShareUtil.h"

#include "BufferAllocator.h"
#include "AppItnMsg.h"
#include <RefClass.hpp>
#include <RefString.hpp>
//#include "EDPass.h"
#include <CustomSocket.h>
#include "DataPacket.hpp"
#include "DataPacketReader.hpp"
#include "SendPackPool.h"
#include "CustomWorkSocket.h"
#include "CustomServerSocket.h"
#include "CustomClientSocket.h"
#include "CustomServerClientSocket.h"
#include "ServerDef.h"
#include "CustomJXServerClientSocket.h"
#include "SQL.h"
#include "CustomLuaScript.h"
#include "CustomLuaConfig.h"

//#include "Container.h"

#include <string>
using namespace wylib::stream;
#include "LogConfig.h"
#include "LocalSender.h"
#include "CustomLuaPreProcess.h"
#include "SrvConfig.h"
#include "LoggerClientSocket.h"
#include "LoggerServerSocket.h"
#include "ChatLogClientSocket.h"
#include "ChatLogServerSocket.h"
#include "FileLogger.h"


#include "LogType.h"
#include "DefExceptHander.h"
#include "LogServer.h"
#include "FDOP.h"



#define DBEGN_KN_VERION			MAKEFOURCC(17, 3, 20, 2)
