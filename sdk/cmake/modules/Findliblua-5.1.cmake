if ( _DEBUG  )
    set( liblua-5.1 "lua-5.1_d" )
else()
    set( liblua-5.1 "lua-5.1_r" )
endif()

message( status " using bundled ${liblua-5.1}..." )

find_path(
	LIBLUA-5.1_INCLUDE_DIR
	NAMES lualib.h lauxlib.h ldebug.h lmem.h lzio.h lvm.h lapi.h lstring.h lgc.h ldo.h lfunc.h llimits.h lua.h ltable.h lopcodes.h lcode.h lparser.h ltm.h luaconf.h lundump.h llex.h lobject.h lstate.h
	#NAMES *.h *.hpp
	PATHS ${CMAKE_CURRENT_LIST_DIR}/../../../sdk/lua-5.1/src 
)

find_library(
	LIBLUA-5.1_LIBRARIES
	NAMES ${liblua-5.1}
	PATHS ${CMAKE_CURRENT_LIST_DIR}/../../../libs/bin/
)
