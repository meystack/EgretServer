cmake_minimum_required(VERSION 3.0.0)
project(Robot VERSION 0.1.0)
add_definitions(-std=c++11)
if( NOT CMAKE_BUILD_TYPE )
  set(CMAKE_BUILD_TYPE "Release")
endif()

add_definitions(-D_GNU_SOURCE -D_REENTRANT)

set( ROOT_PATH  ${CMAKE_CURRENT_SOURCE_DIR} )
set( CMAKE_MODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../common)
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_MODULE_DIR}/cmake/modules)
set( CMAKE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../sdk) # include 文件夹 

set(LIBRARY_OUTPUT_PATH ${ROOT_PATH}/libs) 
set(BINARY_OUTPUT_PATH ${ROOT_PATH}/libs) 

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -w -g2 -ggdb -fPIC -z muldefs -lpthread -lrt -lz -ldl -lm -D_DEBUG -fpermissive")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -w -fPIC -z muldefs -lpthread -lrt -lz -ldl -lm -fpermissive")

#include_directories(
#  ${CMAKE_CURRENT_SOURCE_DIR}/../srvlib/include
#  ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cpp/libs/src/wylib/include
#  ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cpp/libs/src/lua-5.1/src
#  ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cpp/libs/src/tolua++/src
#  ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cpp/libs/3rd/inc/mysql-5.2/include
#  ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cpp/libs/3rd/inc/breakpad/
#  ${CMAKE_CURRENT_SOURCE_DIR}/../common/include
#  ${CMAKE_CURRENT_SOURCE_DIR}/../common/def
#  ${CMAKE_CURRENT_SOURCE_DIR}/../LogicServer
#  ${CMAKE_CURRENT_SOURCE_DIR}
#)

include_directories( 
  ${CMAKE_INCLUDE_DIR}/public
  ${CMAKE_INCLUDE_DIR}/system
  ${CMAKE_INCLUDE_DIR}/utils 

 # ${CMAKE_CURRENT_SOURCE_DIR}/src/wylib/include
  ${CMAKE_INCLUDE_DIR}/lua-5.1/src
  ${CMAKE_INCLUDE_DIR}/tolua++/src
  ${CMAKE_INCLUDE_DIR}/mysql-5.2/include
  ${CMAKE_INCLUDE_DIR}/breakpad/
  ${CMAKE_INCLUDE_DIR}/commonLib/include
  ${CMAKE_INCLUDE_DIR}/commonLib/def
  ${CMAKE_INCLUDE_DIR}/srvlib/include
  ${CMAKE_INCLUDE_DIR} 
  ${CMAKE_CURRENT_SOURCE_DIR}/../LogicServer
  ${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB sources1 *.cpp)
file(GLOB sources2 */*.cpp)
file(GLOB sources3 */*/*.cpp)

if ( ${CMAKE_BUILD_TYPE} STREQUAL "Release" )
   set( _DEBUG  0 )
else ()
   set( _DEBUG  1 )
endif()

if ( _DEBUG  )
  set( EXE_NAME robot_d )
else()
  set( EXE_NAME robot_r )
endif()

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../../Exec/LogicServer)

add_executable(${EXE_NAME} ${sources1} ${sources2} ${sources3})

#include(../common/cmake/libs.cmake)
include(../../sdk/cmake/libs.cmake)#ccc

