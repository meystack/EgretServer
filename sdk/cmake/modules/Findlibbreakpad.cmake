message( status " using bundled Findlibbreakpad.cmake..." )



set( INCLUDE_DIR_PATH ${CMAKE_CURRENT_LIST_DIR}/../../../sdk/breakpad/client/linux/handler/ )
set( LIBRARIE_DIR_PATH ${CMAKE_CURRENT_LIST_DIR}/../../../libs/bin/ )
 

find_path(
	LIBBREAKPAD_INCLUDE_DIR
	NAMES exception_handler.h minidump_descriptor.h
	PATHS ${INCLUDE_DIR_PATH}
)

find_library(
	LIBBREAKPAD_LIBRARIES
	NAMES breakpad_client 
	PATHS ${LIBRARIE_DIR_PATH}
)
