cmake_minimum_required (VERSION 2.8.1)
set( CMAKE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../sdk) # include 文件夹

#头文件
include_directories(
	${CMAKE_INCLUDE_DIR}/public
	${CMAKE_INCLUDE_DIR}/system
	${CMAKE_INCLUDE_DIR}/utils 
	
	${CMAKE_INCLUDE_DIR}/lua-5.1/src 
	${CMAKE_INCLUDE_DIR}/tolua++/src
	./
)

# 源文件及头文件
file(GLOB_RECURSE Common_LIST "${PROJECT_SOURCE_DIR}/common/*.cc" "${PROJECT_SOURCE_DIR}/common/*.h")

# 项目文件分组
group_by_dir( ${PROJECT_SOURCE_DIR}/common ${Common_LIST} )

# 输出目录
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${Root_Dir}/../libs/gateway)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${Root_Dir}/../libs/gateway)

#生成目标
add_library(common STATIC ${Common_LIST})