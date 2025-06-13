cmake_minimum_required(VERSION 2.6)
project(srvlibs)
# set(PROJECTNAME "srvlibs") 
# project(${PROJECTNAME} CXX)
# project(${PROJECTNAME} C CXX) 

# Avoid warnings in higher versions
# if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" GREATER 2.6)
# CMAKE_POLICY(VERSION 2.8)
# endif()
MESSAGE(STATUS "Running cmake version ${CMAKE_VERSION}")

if( NOT CMAKE_BUILD_TYPE )
 set(CMAKE_BUILD_TYPE "Release")	
endif()

ADD_DEFINITIONS(-D_GNU_SOURCE -D_REENTRANT)

set(BUILD_SHARED_LIBS ON)

if( BUILD_SHARED_LIBS  ) 
   SET ( LIBS_TYPE SHARED)
else()
   SET ( LIBS_TYPE STATIC)
endif()

set( ROOT_PATH  ${CMAKE_CURRENT_SOURCE_DIR} )

# set( CMAKE_MODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
# set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_MODULE_DIR}/cmake/modules)

set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../libs)
set(BINARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../libs) 
set( CMAKE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../sdk) # include 文件夹

# set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -fPIC -pthread -D_DEBUG")
# set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -ggdb -g -Wall -pthread -fPIC")
 
# message( "main ---- CMAKE_SOURCE_DIR 1******************************-> ${CMAKE_INCLUDE_DIR}" )
# message( "main ---- CMAKE_CURRENT_SOURCE_DIR 1******************************-> ${CMAKE_CURRENT_SOURCE_DIR}" )


set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -fPIC -D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -ggdb -g -Wall  -fPIC")
 
include_directories( 
  ${CMAKE_INCLUDE_DIR}/public
  ${CMAKE_INCLUDE_DIR}/system
  ${CMAKE_INCLUDE_DIR}/utils 
  ${CMAKE_INCLUDE_DIR}/lua-5.1/src 
  ${CMAKE_INCLUDE_DIR}/tolua++/src
  ${CMAKE_INCLUDE_DIR}/mysql-5.2/include
  ${CMAKE_INCLUDE_DIR}/zlib 1.1.4/src
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  #${CMAKE_CURRENT_SOURCE_DIR}/../lua-5.1/src
  #${CMAKE_CURRENT_SOURCE_DIR}/../tolua++/src
  #${CMAKE_CURRENT_SOURCE_DIR}/../zlib 1.1.4/src 
  #${CMAKE_CURRENT_SOURCE_DIR}/../common/libs/src/wylib/include
  #${CMAKE_CURRENT_SOURCE_DIR}/../mysql-5.2/include
)
file(GLOB_RECURSE sources *.cpp *.c)

file(GLOB_RECURSE sources2 ../system/*.cpp ../system/*.c)
file(GLOB_RECURSE sources3 ../utils/*.cpp ../utils/*.c)

# file(GLOB sources1 *.cpp *.c)
# file(GLOB sources2 */*.cpp */*.c)
# file(GLOB sources3 */*/*.cpp */*/*.c)
# file(GLOB sources4 */*/*/*.cpp */*/*/*.c)
# file(GLOB sources5 */*/*/*/*.cpp */*/*/*/*.c)

# link_directories (
#    ${CMAKE_CURRENT_SOURCE_DIR}/../common/cpp/libs/3rd/bin
#)

#add_subdirectory("src")

if ( ${CMAKE_BUILD_TYPE} STREQUAL "Release" )
   set( _DEBUG  0 )
else ()
    set( _DEBUG  1 )
endif()

if ( _DEBUG  )
  set( EXE_NAME srv_d )
else()
  set( EXE_NAME srv_r )
endif()
 
# add_library("${EXE_NAME}" STATIC ${sources1} ${sources2} ${sources3}  ${sources4} ${sources5})
 
add_library(${EXE_NAME} STATIC ${sources} ${sources2} ${sources3})
 
# include(../cmake/libs.cmake)
# add_library("${EXE_NAME}" SHARED ${Npc_include} ${sources} )
# add_library("${EXE_NAME}" SHARED ${sources})


# SET_TARGET_PROPERTIES(nyutil PROPERTIES CLEAN_DIRECT_OUTPUT 1)  
# SET_TARGET_PROPERTIES(nyutil_static PROPERTIES OUTPUT_NAME "nyutil") 
# SET_TARGET_PROPERTIES(nyutil PROPERTIES CLEAN_DIRECT_OUTPUT 1) 
# SET_TARGET_PROPERTIES(nyutil_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)