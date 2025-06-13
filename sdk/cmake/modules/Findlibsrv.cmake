if ( _DEBUG  )
    set( libsrv "srv_d" )
else()
    set( libsrv "srv_r" )
endif()

message( status " using bundled ${libsrv}..." )

find_path(
	LIBSRV_INCLUDE_DIR
	NAMES AdvanceAstar.h AMClient.h AppItnMsg.h BufferAllocatorEx.h BufferAllocator.h CBPTypes.h CFilter.h CLVariant.h cp.h CRC16.h CustomClientSocket.h CustomFileDB.h CustomHashTable.h CustomLuaClass.h CustomLuaConfig.h CustomLuaPreProcess.h CustomLuaProfiler.h CustomLuaScript.h CustomServerClientSocket.h CustomServerSocket.h CustomSortList.h CustomWorkSocket.h DataPacket.hpp DataPacketReader.hpp DefExceptHander.h EDPass.h FDOP.h FileDB_test.h LangTextSection.h LuaCLVariant.h LuaLangTextSection.h LuaToBP.h LuaVariantClass.h MiniDateTime.h NameAllocator.h ObjectAllocator.hpp ObjectAllocStat.hpp PathThreadLocale.h SendPackPool.h ShareUtil.h SingleObjectAllocator.hpp Singleton.hpp SQL.h SQLiteDB.h StackWalker.h TimeCall.h TimeStat.h utf8_output.h VirturalMap.h WinService.h EDCode.h
	
	#NAMES *.h *.hpp
	PATHS ${CMAKE_CURRENT_LIST_DIR}/../../../sdk/srvlib/include 
)

find_library(
	LIBSRV_LIBRARIES
	NAMES ${libsrv}
	PATHS ${CMAKE_CURRENT_LIST_DIR}/../../../libs
)
