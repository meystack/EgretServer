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
#include "CustomServerClientSocket.h"
#include "ServerDef.h"
#include "CustomJXServerClientSocket.h"
#include "SQL.h"
#include "CustomLuaMyLoad.h"
#include "CustomLuaScript.h"
#include "CustomLuaConfig.h"

#include "SrvConfig.h"
#include "LocalLogClientSocket.h"
#include "LocalLogServerSocket.h"
#include "FileLogger.h"

#include "LogType.h"
#include "DefExceptHander.h"
#include "FDOP.h"
