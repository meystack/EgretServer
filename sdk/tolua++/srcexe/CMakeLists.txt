cmake_minimum_required(VERSION 2.6)
project(tolua++)

# Avoid warnings in higher versions
if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" GREATER 2.6)
 CMAKE_POLICY(VERSION 2.8)
endif()
MESSAGE(STATUS "Running cmake version ${CMAKE_VERSION}")

if( NOT CMAKE_BUILD_TYPE )
  set(CMAKE_BUILD_TYPE "Debug")
endif()

add_definitions(-D_GNU_SOURCE -D_REENTRANT)

set( CMAKE_SOURCE_DIR .)
set( ROOT_PATH  ${CMAKE_SOURCE_DIR} )
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

set(LIBRARY_OUTPUT_PATH ${ROOT_PATH}/libs)
set(BINARY_OUTPUT_PATH ${ROOT_PATH}/libs)

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -lm -ldl")

file(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../../lua-5.1/src/
)

add_executable(tolua++  ${sources})

find_package( liblua-5.1_r REQUIRED )
mark_as_advanced(
        LIBLUA-5.1_R_INCLUDE_DIR
        LIBLUA-5.1_R_LIBRARIES
)
if( LIBLUA-5.1_R_INCLUDE_DIR AND LIBLUA-5.1_R_LIBRARIES )
        include_directories( ${LIBLUA-5.1_R_INCLUDE_DIR} )
        target_link_libraries( tolua++ ${LIBLUA-5.1_R_LIBRARIES} )
endif( LIBLUA-5.1_R_INCLUDE_DIR AND LIBLUA-5.1_R_LIBRARIES )

#if ( _DEBUG  )
#  add_library(tolua++_d ${LIBS_TYPE} ${sources})	
#else()
#   add_library(tolua++_r ${LIBS_TYPE} ${sources})
#endif()



