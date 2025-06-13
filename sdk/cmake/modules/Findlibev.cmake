message( status " using bundled Findlibev.cmake..." )

set( INCLUDE_DIR_PATH ${CMAKE_CURRENT_LIST_DIR}/../../../../common/cpp/libs/3rd/inc/libev/include )
set( LIBRARIE_DIR_PATH ${CMAKE_CURRENT_LIST_DIR}/../../../../common/cpp/libs/3rd/bin )

find_path(
	LIBEV_INCLUDE_DIR
	NAMES event.h ev.h ev++.h 
	PATHS ${INCLUDE_DIR_PATH}
)

find_library(
	LIBEV_LIBRARIES
	NAMES libev.a 
	PATHS ${LIBRARIE_DIR_PATH}
)
