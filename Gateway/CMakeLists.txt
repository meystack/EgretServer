cmake_minimum_required (VERSION 2.8.1)
PROJECT(Server)

set( CMAKE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../sdk) # include 文件夹
#头文件
include_directories(
	${CMAKE_INCLUDE_DIR}/lua-5.1/src 
	${CMAKE_INCLUDE_DIR}/tolua++/src
	srvlib/include/
	srvlib/3rd/lua/
	srvlib/3rd/zlib/
	srvlib/3rd/tolua++/
	srvlib/3rd/mysql/
	common/
)


# Win10 / Linux 环境下的处理
if(MSVC)
	# 头文件
	include_directories(srvlib/3rd/mysql)

	# 工具路径
	SET(PATH_LUA "${CMAKE_SOURCE_DIR}/srvlib/bin/lua514")
	SET(PATH_TOLUA "${CMAKE_SOURCE_DIR}/srvlib/bin/tolua++")

	# 编译选项
	ADD_DEFINITIONS(/W3 /wd"4996" /wd"4819" /wd"4091" /wd"4099" /wd"4800" /wd"4996")
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /MP4")
  	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
else()
	# 头文件
	include_directories(/usr/local/mysql/include)

	# 编译选项
	set(CMAKE_C_FLAGS "-O0 -g3 -gdwarf-2 -std=c++0x -c -fmessage-length=0 -MMD -MP")
	SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -Wall -Wextra -Wfatal-errors -std=c++11 -fpermissive -D_DEBUG -g -ggdb")
	SET(CMAKE_CXX_FLAGS_RELEASE "-O0 -g3 -Wall -std=c++11 -fpermissive -g -ggdb -DNDEBUG")
	#add_definitions("-O0 -g3 -gdwarf-2 -std=c++11 -w -c -fmessage-length=0 -MMD -MP")
endif(MSVC)

# 项目内文件分组工具
function(group_by_dir source_dir)
	message(STATUS "${ARGV0}")
    foreach(FILE ${ARGN})
    	message(STATUS "${FILE}")

        # 获取文件绝对路径
        get_filename_component(FULL_NAME "${FILE}" ABSOLUTE)
        # 获取文件父路径
        get_filename_component(PARENT_DIR "${FULL_NAME}" PATH)
        # 移除父路径中的源码根路径
        string(REPLACE "${ARGV0}" "" GROUP "${PARENT_DIR}")
           
        # 确保路径使用windows路径符号
        string(REPLACE "/" "\\" GROUP "${GROUP}")
        # 分组
        source_group("${GROUP}" FILES "${FILE}")
    endforeach()
endfunction(group_by_dir)

set( Root_Dir ${PROJECT_SOURCE_DIR} )
set( Libs_Dir
	${PROJECT_SOURCE_DIR}/libs/gateway
	${PROJECT_SOURCE_DIR}/../common/cpp/libs/3rd/bin
)
set( Bin_Dir ${PROJECT_SOURCE_DIR} )

SET(EXECUTABLE_OUTPUT_PATH ${Bin_Dir})

# 子路径
add_subdirectory(srvlib)
add_subdirectory(common)
add_subdirectory(gateway)