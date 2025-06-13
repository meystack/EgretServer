message( status " using bundled Findlibmysqlclient.cmake..." )

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set( INCLUDE_DIR_PATH ${CMAKE_CURRENT_LIST_DIR}/../../../sdk/mysql-5.2/include )
	set( LIBRARIE_DIR_PATH ${CMAKE_CURRENT_LIST_DIR}/../../../libs/bin/ )
#	set( INCLUDE_DIR_PATH /usr/include )
#	set( LIBRARIE_DIR_PATH /usr/lib64/mysql )
else()
	set( INCLUDE_DIR_PATH ${CMAKE_CURRENT_LIST_DIR}/../../../sdk/mysql-5.2/include )
	set( LIBRARIE_DIR_PATH ${CMAKE_CURRENT_LIST_DIR}/../../../libs/bin/ )
#	set( INCLUDE_DIR_PATH /usr/include )
#	set( LIBRARIE_DIR_PATH /usr/lib/mysql )
endif()

find_path(
	LIBMYSQLCLIENT_INCLUDE_DIR
	NAMES config-netware.h config-win.h errmsg.h libmysql.def m_ctype.h m_string.h my_alloc.h my_dbug.h my_getopt.h my_global.h my_list.h my_pthread.h mysql_com.h mysqld_ername.h mysqld_error.h mysql_embed.h mysql.h mysql_time.h mysql_version.h my_sys.h typelib.h base64.h decimal.h hash.h keycache.h lf.h my_aes.h my_alarm.h my_atomic.h my_attribute.h 
	#NAMES *.h *.hpp
	PATHS ${INCLUDE_DIR_PATH}
)

find_library(
	LIBMYSQLCLIENT_LIBRARIES
	NAMES mysqlclient 
	PATHS ${LIBRARIE_DIR_PATH}
)
