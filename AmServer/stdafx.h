// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <map>
#ifdef WIN32
	#include <crtdbg.h>
	#include <tchar.h>
	#include <Windows.h>
#endif
#include "_osdef.h"
#include "_ast.h"
#include <_memchk.h>
#include <Thread.h>
#include <CustomSocket.h>
#include <Lock.h>
#include <Tick.h>
#include <RefString.hpp>
#include <Stream.h>
#include <wrand.h>
#include <bzhash.h>
#include <vector>
#include <string>
using namespace std;


extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#include <stdlib.h>
#include <_ast.h>
#include <_memchk.h>
#include <Tick.h>
#include <Lock.h>
#include <Stream.h>
#include <QueueList.h>
#include "ShareUtil.h"
#include "BufferAllocator.h"
#include "ObjectAllocator.hpp"
#include "SingleObjectAllocator.hpp"
#include "AppItnMsg.h"
#include <RefClass.hpp>
#include <RefString.hpp>
#include "EDPass.h"
#include <CustomSocket.h>
#include "DataPacket.hpp"
#include "DataPacketReader.hpp"
#include "SendPackPool.h"
#include "CustomWorkSocket.h"
#include "CustomServerSocket.h"
#include "CustomServerClientSocket.h"
#include "ServerDef.h"
#include "CustomJXServerClientSocket.h"
#include "SQL.h"
#include "CustomLuaScript.h"
#include "CustomLuaConfig.h"
#include "DefExceptHander.h"
#include "FileLogger.h"
#include "MiniDateTime.h"

#include "HttpParamParser.h"
#include "SrvConfig.h"
#include "AMClientSocket.h"
#include "AMServerSocket.h"
#include "Product.h"
#include "md5.h"
#include "EDCode.h"
#include "FDOP.h"

namespace HttpUtility
{
   
    typedef unsigned char BYTE;
 
    BYTE toHex(const BYTE &x);

    BYTE fromHex(const BYTE &x);
 
    string URLEncode(const string &sIn);

    string URLDecode(const string &sIn);
}

extern bool bAMEngineStartSucceed ;

// TODO: reference additional headers your program requires here
