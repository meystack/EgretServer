cmake_minimum_required(VERSION 2.6)
project(logicserver)
add_definitions(-std=c++11)
# Avoid warnings in higher versions
if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" GREATER 2.6)
 CMAKE_POLICY(VERSION 2.8)
endif()
MESSAGE(STATUS "Running cmake version ${CMAKE_VERSION}")



if( NOT CMAKE_BUILD_TYPE )
  set(CMAKE_BUILD_TYPE "Release")
endif()

add_definitions(-D_GNU_SOURCE -D_REENTRANT)


#is or not shared type
if( BUILD_SHARED_LIBS  ) 
   SET ( LIBS_TYPE SHARED)
else()
   SET ( LIBS_TYPE STATIC)
endif()

set( ROOT_PATH  ${CMAKE_CURRENT_SOURCE_DIR} )
set( CMAKE_MODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../common)
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_MODULE_DIR}/cmake/modules)
set( CMAKE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../sdk) # include 文件夹

set(LIBRARY_OUTPUT_PATH ${ROOT_PATH}/libs) 
set(BINARY_OUTPUT_PATH ${ROOT_PATH}/libs) 

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -w -g2 -ggdb -fPIC -z muldefs -lpthread -lrt -lz -ldl -lm -D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O0 -w  -g2 -ggdb -fPIC -z muldefs -lpthread -lrt -lz -ldl -lm")

#include_directories(
#  ${CMAKE_CURRENT_SOURCE_DIR}/../srvlib/include
#  ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cpp/libs/src/wylib/include
#  ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cpp/libs/src/lua-5.1/src
#  ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cpp/libs/src/tolua++/src
#  ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cpp/libs/3rd/inc/mysql-5.2/include
#  ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cpp/libs/3rd/inc/breakpad/
#  ${CMAKE_CURRENT_SOURCE_DIR}/../common/include
#  ${CMAKE_CURRENT_SOURCE_DIR}/../common/def
#  ${CMAKE_CURRENT_SOURCE_DIR}
#)

message( status "LogicServer ---- ROOT_PATH 1******************************-> ${ROOT_PATH}" ) 
message( status "LogicServer ---- ROOT_PATH 2******************************-> ${CMAKE_INCLUDE_DIR}" ) 
message( status "LogicServer ---- ROOT_PATH 3******************************-> ${CMAKE_INCLUDE_DIR}/commonLib/include" ) 
message( status "LogicServer ---- ROOT_PATH 3******************************-> ${CMAKE_INCLUDE_DIR}/utils" ) 

include_directories(
 # ${CMAKE_CURRENT_SOURCE_DIR}/src/wylib/include
  ${CMAKE_INCLUDE_DIR}/public
  ${CMAKE_INCLUDE_DIR}/system
  ${CMAKE_INCLUDE_DIR}/utils 
  
  ${CMAKE_INCLUDE_DIR}/lua-5.1/src
  ${CMAKE_INCLUDE_DIR}/tolua++/src
  ${CMAKE_INCLUDE_DIR}/mysql-5.2/include
  ${CMAKE_INCLUDE_DIR}/breakpad/
  ${CMAKE_INCLUDE_DIR}/commonLib/include
  ${CMAKE_INCLUDE_DIR}/commonLib/def
  ${CMAKE_INCLUDE_DIR}/srvlib/include
  ${CMAKE_INCLUDE_DIR}/system
  ${CMAKE_INCLUDE_DIR} 
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
  set( EXE_NAME logicserver_d )
else()
  set( EXE_NAME logicserver_r )
endif()
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../../Exec/LogicServer)

add_executable(${EXE_NAME} ${sources1} ${sources2} ${sources3} )

#include(../common/cmake/libs.cmake)
include(../../sdk/cmake/libs.cmake)#ccc

MESSAGE(${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(MakeServerInterface ALL)

#add_dependencies(${EXE_NAME}  MakeServerInterface)

#add_custom_command(
#  TARGET MakeServerInterface PRE_BUILD
#  COMMAND echo -e "\t开始生成 serverInterface.cpp 文件！"
#  COMMAND rm -rf serverInterface.cpp
#  COMMAND ./../../../../common/cpp/libs/src/tolua++/tolua++ -n server -o serverInterface.cpp serverInterface.scp
#  WORKING_DIRECTORY
#      ${CMAKE_CURRENT_SOURCE_DIR}/script/tolua++
#  DEPENDS
#      ${CMAKE_CURRENT_SOURCE_DIR}/script/tolua++/serverInterface.scp
#  VERBATIM
#  )

# add_custom_target(CleanLog ALL)
# add_custom_command(
#   TARGET CleanLog PRE_BUILD
#   COMMAND echo -e "\t开始生成清除日志文件！"
#   COMMAND find . -iname "*.txt*" -exec rm {} +;
#   WORKING_DIRECTORY  ${EXECUTABLE_OUTPUT_PATH}/../log/
#   VERBATIM
#   )

add_custom_target(DOSOME
  WORKING_DIRECTORY
    ${EXECUTABLE_OUTPUT_PATH}/script/tolua++
  COMMENT
    ./../../../../common/cpp/libs/src/tolua++/tolua++ -n server -o serverInterface.cpp serverInterface.scp
  )
ADD_DEPENDENCIES(DOSOME ${EXE_NAME})
