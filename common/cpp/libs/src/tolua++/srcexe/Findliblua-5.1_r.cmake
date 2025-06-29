message( status " using bundled Findliblua-5.1_r.cmake..." )

find_path(
        LIBLUA-5.1_R_INCLUDE_DIR
        NAMES lualib.h lauxlib.h ldebug.h lmem.h lzio.h lvm.h lapi.h lstring.h lgc.h ldo.h lfunc.h llimits.h lua.h ltable.h lopcodes.h lcode.h lparser.h ltm.h luaconf.h lundump.h llex.h lobject.h lstate.h
        #NAMES *.h *.hpp
        PATHS ${CMAKE_SOURCE_DIR}/../../lua-5.1/src/
)

find_library(
        LIBLUA-5.1_R_LIBRARIES
        NAMES lua-5.1
        PATHS ${CMAKE_SOURCE_DIR}/../../../../../../GameCommon/cpp/libs
)

