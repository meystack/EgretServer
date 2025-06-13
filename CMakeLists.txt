cmake_minimum_required (VERSION 2.8.1)

# Avoid warnings in higher versions
if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" GREATER 2.6)
 CMAKE_POLICY(VERSION 2.8)
endif()
MESSAGE(STATUS "Creating Project MMOServer; Running cmake version ${CMAKE_VERSION}")

# 项目名
PROJECT(MMOServer)

# 当前路径
set( Root_Dir ${PROJECT_SOURCE_DIR} )

#is or not shared type
if( BUILD_SHARED_LIBS  ) 
   SET ( LIBS_TYPE SHARED)
else()
   SET ( LIBS_TYPE STATIC)
endif()

if ( ${CMAKE_BUILD_TYPE} STREQUAL "Release" )
   set( _DEBUG  0 )
else ()
   set( _DEBUG  1 )
endif()


message( "main ---- ROOT_PATH 1******************************-> ${ROOT_PATH}" )  
message( "main ---- ROOT_PATH 1******************************-> ${ROOT_PATH}/../sdk/commonLib" )  

set( ROOT_PATH  ${CMAKE_CURRENT_SOURCE_DIR} )

# 子路径
#add_subdirectory(srvlib) 
if(NOT SDK_DIR)
   set( SDK_DIR ${CMAKE_SOURCE_DIR}/../sdk)
endif()
if( IS_DIRECTORY ${SDK_DIR} )
   # 第二个cassdk.out参数用于指定外部文件夹在输出文件夹中的位置
   add_subdirectory( ${SDK_DIR}/srvlib ../B/srvlib.out)
   add_subdirectory( ${SDK_DIR}/commonLib ../B/commonLib.out)
else()
   message(FATAL_ERROR   "INVALID FOLDER 'SDK_DIR'=${SDK_DIR}" )
endif() 
 
#add_subdirectory(${ROOT_PATH}/../sdk/commonLib)
add_subdirectory(LogicServer ../B/LogicServer.out)
add_subdirectory(DBServer ../B/DBServer.out)
add_subdirectory(LocalLogServer ../B/LocalLogServer.out)
add_subdirectory(LoggerServer ../B/LoggerServer.out)
add_subdirectory(NameServer ../B/NameServer.out)
add_subdirectory(BackStageServer ../B/BackStageServer.out)
add_subdirectory(SessionServer ../B/SessionServer.out)
add_subdirectory(robot ../B/robot.out)

set( ROOT_PATH  ${CMAKE_CURRENT_SOURCE_DIR} )

if ( _DEBUG  ) 
#    message( status "ROOT_PATH ******************************-> ${ROOT_PATH}/../Exec/cp_d.sh" ) 
#    set(SCRIPT_SETUP_SAMPLE "${ROOT_PATH}/../Exec/cp_d.sh") 
else() 
#    message( status "ROOT_PATH -------------------------------> ${ROOT_PATH}/../Exec/cp.sh" ) 
#    set(SCRIPT_SETUP_SAMPLE "${ROOT_PATH}/../Exec/cp.sh") 
endif()
 
# execute_process(
#   COMMAND ${SCRIPT_SETUP_SAMPLE}
#   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#   )
 
# message( status "ROOT_PATH -------------------------------> ${ROOT_PATH}/../Exec/ln.sh" ) 
# set(SCRIPT_SETUP_SAMPLE "${ROOT_PATH}/../Exec/ln.sh")  

# execute_process(
#  COMMAND ${SCRIPT_SETUP_SAMPLE}
#  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#  )
