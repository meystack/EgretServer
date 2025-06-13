

set( ROOT_PATH  ${CMAKE_CURRENT_SOURCE_DIR} )
set( CMAKE_MODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/../cmake/modules)
   
# find_package( libtcmalloc_minimal REQUIRED )
# mark_as_advanced(
#        LIBTCMALLOC_MINIMAL_LIBRARIES
# )
 
if( LIBTCMALLOC_MINIMAL_LIBRARIES )
        target_link_libraries( ${EXE_NAME} ${LIBTCMALLOC_MINIMAL_LIBRARIES} ) 
        #message( status " --------------${EXE_NAME}-w1-------------------W-------------W----------> ${LIBTCMALLOC_MINIMAL_LIBRARIES}" ) 
endif( LIBTCMALLOC_MINIMAL_LIBRARIES )

target_link_libraries( ${EXE_NAME} pthread )

find_package( libev REQUIRED )
mark_as_advanced(
        LIBEV_INCLUDE_DIR
        LIBEV_LIBRARIES
)
if( LIBEV_INCLUDE_DIR AND LIBEV_LIBRARIES )
        include_directories( ${LIBEV_INCLUDE_DIR} )
        target_link_libraries( ${EXE_NAME} ${LIBEV_LIBRARIES} )
        
#message( status " --------------${EXE_NAME}-w1-------------------W-------------W----------> ${LIBEV_INCLUDE_DIR}" ) 
#message( status " --------------${EXE_NAME}-w2-------------------W-------------W----------> ${LIBEV_LIBRARIES}" ) 
 
endif( LIBEV_INCLUDE_DIR AND LIBEV_LIBRARIES )

find_package( libtolua++ REQUIRED )
mark_as_advanced(
        LIBTOLUA++_INCLUDE_DIR
        LIBTOLUA++_LIBRARIES
)
if( LIBTOLUA++_INCLUDE_DIR AND LIBTOLUA++_LIBRARIES )
        include_directories( ${LIBTOLUA++_INCLUDE_DIR} )
        target_link_libraries( ${EXE_NAME} ${LIBTOLUA++_LIBRARIES} )
        
##message( status " --------------${EXE_NAME}-w1-------------------W-------------W----------> ${LIBTOLUA++_INCLUDE_DIR}" ) 
#message( status " --------------${EXE_NAME}-w2-------------------W-------------W----------> ${LIBTOLUA++_LIBRARIES}" ) 
 
endif( LIBTOLUA++_INCLUDE_DIR AND LIBTOLUA++_LIBRARIES )

find_package( libmysqlclient REQUIRED )
mark_as_advanced(
        LIBMYSQLCLIENT_INCLUDE_DIR
        LIBMYSQLCLIENT_LIBRARIES
)
if( LIBMYSQLCLIENT_INCLUDE_DIR AND LIBMYSQLCLIENT_LIBRARIES )
        include_directories( ${LIBMYSQLCLIENT_INCLUDE_DIR} )
        target_link_libraries( ${EXE_NAME} ${LIBMYSQLCLIENT_LIBRARIES} )
        
#message( status " --------------${EXE_NAME}-w1-------------------W-------------W----------> ${LIBMYSQLCLIENT_INCLUDE_DIR}" ) 
#message( status " --------------${EXE_NAME}-w2-------------------W-------------W----------> ${LIBMYSQLCLIENT_LIBRARIES}" ) 
 
endif( LIBMYSQLCLIENT_INCLUDE_DIR AND LIBMYSQLCLIENT_LIBRARIES )

find_package( libzlib1.1.4 REQUIRED )
mark_as_advanced(
        LIBZLIB1.1.4_INCLUDE_DIR
        LIBZLIB1.1.4_LIBRARIES
)
if( LIBZLIB1.1.4_INCLUDE_DIR AND LIBZLIB1.1.4_LIBRARIES )
        include_directories( ${LIBZLIB1.1.4_INCLUDE_DIR} )
        target_link_libraries( ${EXE_NAME} ${LIBZLIB1.1.4_LIBRARIES} )
        
#message( status " --------------${EXE_NAME}-w2-------------------W-------------W----------> ${LIBZLIB1.1.4_INCLUDE_DIR}" ) 
 
#message( status " --------------${EXE_NAME}-w1-------------------W-------------W----------> ${LIBZLIB1.1.4_LIBRARIES}" ) 
endif( LIBZLIB1.1.4_INCLUDE_DIR AND LIBZLIB1.1.4_LIBRARIES )

find_package( libservercommon REQUIRED )
mark_as_advanced(
        LIBSERVERCOMMON_INCLUDE_DIR
        LIBSERVERCOMMON_LIBRARIES
)
if( LIBSERVERCOMMON_INCLUDE_DIR AND LIBSERVERCOMMON_LIBRARIES )
        include_directories( ${LIBSERVERCOMMON_INCLUDE_DIR} )
        target_link_libraries( ${EXE_NAME} ${LIBSERVERCOMMON_LIBRARIES} )
        
#message( status " --------------${EXE_NAME}-w1-------------------W-------------W----------> ${LIBSERVERCOMMON_INCLUDE_DIR}" ) 
#message( status " --------------${EXE_NAME}-w2-------------------W-------------W----------> ${LIBSERVERCOMMON_LIBRARIES}" ) 
 
endif( LIBSERVERCOMMON_INCLUDE_DIR AND LIBSERVERCOMMON_LIBRARIES )


find_package( libsrv REQUIRED )
mark_as_advanced(
        LIBSRV_INCLUDE_DIR
        LIBSRV_LIBRARIES
)

if( LIBSRV_INCLUDE_DIR AND LIBSRV_LIBRARIES )

        include_directories( ${LIBSRV_INCLUDE_DIR} )
        target_link_libraries( ${EXE_NAME} ${LIBSRV_LIBRARIES} )
        
#message( status " --------------${EXE_NAME}-w1-------------------W-------------W----------> ${LIBSRV_INCLUDE_DIR}" ) 
#message( status " --------------${EXE_NAME}-w2-------------------W-------------W----------> ${LIBSRV_LIBRARIES}" ) 
 
endif( LIBSRV_INCLUDE_DIR AND LIBSRV_LIBRARIES )

find_package( libbreakpad REQUIRED )
mark_as_advanced(
        LIBBREAKPAD_INCLUDE_DIR
        LIBBREAKPAD_LIBRARIES
)
if( LIBBREAKPAD_INCLUDE_DIR AND LIBBREAKPAD_LIBRARIES )
        include_directories( ${LIBBREAKPAD_INCLUDE_DIR} )
        target_link_libraries( ${EXE_NAME} ${LIBBREAKPAD_LIBRARIES} )
        
#message( status " --------------${EXE_NAME}-w2-------------------W-------------W----------> ${LIBBREAKPAD_INCLUDE_DIR}" ) 
 
#message( status " --------------${EXE_NAME}-w1-------------------W-------------W----------> ${LIBBREAKPAD_LIBRARIES}" ) 
endif( LIBBREAKPAD_INCLUDE_DIR AND LIBBREAKPAD_LIBRARIES )


find_package( liblua-5.1 REQUIRED )
mark_as_advanced(
        LIBLUA-5.1_INCLUDE_DIR
        LIBLUA-5.1_LIBRARIES
)
if( LIBLUA-5.1_INCLUDE_DIR AND LIBLUA-5.1_LIBRARIES )
        include_directories( ${LIBLUA-5.1_INCLUDE_DIR} )
        target_link_libraries( ${EXE_NAME} ${LIBLUA-5.1_LIBRARIES} ) 
#message( status " --------------${EXE_NAME}-w2-------------------W-------------W----------> ${LIBLUA-5.1_INCLUDE_DIR}" ) 
#message( status " --------------${EXE_NAME}-w1-------------------W-------------W----------> ${LIBLUA-5.1_LIBRARIES}" ) 
endif( LIBLUA-5.1_INCLUDE_DIR AND LIBLUA-5.1_LIBRARIES )


find_package( libcurl REQUIRED )
mark_as_advanced(
       CURL_INCLUDE_DIR_PATH
       CURL_LIBRARIE_DIR_PATH
)
if( CURL_INCLUDE_DIR_PATH AND CURL_LIBRARIE_DIR_PATH )
        include_directories( ${CURL_INCLUDE_DIR_PATH} )
        target_link_libraries( ${EXE_NAME} ${CURL_LIBRARIE_DIR_PATH} )
        
#message( status " --------------${EXE_NAME}-w1-------------------W-------------W----------> ${CURL_INCLUDE_DIR_PATH}" ) 
#message( status " --------------${EXE_NAME}-w2-------------------W-------------W----------> ${CURL_LIBRARIE_DIR_PATH}" ) 
 
endif( CURL_INCLUDE_DIR_PATH AND CURL_LIBRARIE_DIR_PATH )


