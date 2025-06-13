#if ( _DEBUG  )
#    set( libwylib   "wylib_d" )
#else()
#    set( libwylib   "wylib_r" )
#endif()
#
#message( status " using bundled ${libwylib}..." )
#
#find_path(
#	LIBWYLIB_INCLUDE_DIR
#	NAMES aes.h AnsiString.hpp _ast.h AtomOPDef.h AtomOP.h base64.h bzhash.h ClassedWnd.h crc.h CustomSocket.h fastFT.h LinkedList.h List.h Lock.h LockList.h MBCSDef.h md5.h _memchk.h NamedPipe.h NPComm.h ObjectCounter.h _osdef.h Platform.h  PrintMsg.h Property.h QueueList.h RefClass.hpp RefString.hpp seh.h SocketConfig.h Stream.h SysApi.h Thread.h Tick.h TypeDef.h UCSDef.h WideString.hpp wrand.h wyString.h ZStream.h
#	#NAMES *.h *.hpp
#	PATHS ${CMAKE_CURRENT_LIST_DIR}/../../../../common/cpp/libs/src/wylib/include/ ${CMAKE_CURRENT_LIST_DIR}/../../../../common/cpp/libs/src/wylib/include/3thpart/
#)
#
#find_library(
#	LIBWYLIB_LIBRARIES
#	NAMES ${libwylib}
#	PATHS ${CMAKE_CURRENT_LIST_DIR}/../../../../common/cpp/libs/3rd/bin
#)
