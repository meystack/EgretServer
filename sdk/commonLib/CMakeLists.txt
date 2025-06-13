cmake_minimum_required(VERSION 2.6)
project(servercommon)

# Avoid warnings in higher versions
if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" GREATER 2.6)
 CMAKE_POLICY(VERSION 2.8)
endif()
MESSAGE(STATUS "Running cmake version ${CMAKE_VERSION}")

if( NOT CMAKE_BUILD_TYPE )
  set(CMAKE_BUILD_TYPE "Release")
endif()

add_definitions(-std=c++11)
add_definitions(-D_GNU_SOURCE -D_REENTRANT) 
 
set( ROOT_PATH  ${CMAKE_CURRENT_SOURCE_DIR} )
#set( CMAKE_MODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
#set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_MODULE_DIR}/cmake/modules)
 
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../libs) 
set(BINARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../libs) 
set( CMAKE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../sdk) # include 文件夹

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -w -g2 -ggdb -fPIC -fpermissive -z muldefs -lpthread -lrt -lz -ldl -lm -D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -w -fPIC -z muldefs -lpthread -lrt -lz -ldl -lm")

if ( ${CMAKE_BUILD_TYPE} STREQUAL "Release" )
   set( _DEBUG  0 )
else ()
   set( _DEBUG  1 )
endif()
 
if ( _DEBUG  )
  set( EXE_NAME servercommon_d )
else()
  set( EXE_NAME servercommon_r )
endif()

include_directories(
  
  ${CMAKE_INCLUDE_DIR}/public
  ${CMAKE_INCLUDE_DIR}/system
  ${CMAKE_INCLUDE_DIR}/utils 
  ${CMAKE_INCLUDE_DIR}/lua-5.1/src 
  ${CMAKE_INCLUDE_DIR}/tolua++/src
  ${CMAKE_INCLUDE_DIR}/mysql-5.2/include
  ${CMAKE_INCLUDE_DIR}/breakpad/
  ${CMAKE_INCLUDE_DIR}/srvlib/include

  # ${CMAKE_CURRENT_SOURCE_DIR}/../srvlib/include
  # ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cpp/libs/src/wylib/include    
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/def
)

file(GLOB sources1 cpp/*.cpp )


ADD_LIBRARY(${EXE_NAME} STATIC ${sources1})
include(../cmake/libs.cmake)
