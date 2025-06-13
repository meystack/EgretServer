if ( _DEBUG  )
    set( libtolua++ "tolua++_d" )
else()
    set( libtolua++ "tolua++_r" )
endif()

message( status " using bundled ${libtolua++}..." )

find_path(
	LIBTOLUA++_INCLUDE_DIR
	NAMES toluabind.h tolua_event.h tolua++.h
	#NAMES *.h *.hpp
	PATHS ${CMAKE_CURRENT_LIST_DIR}/../../../sdk/tolua++/src 
)

find_library(
	LIBTOLUA++_LIBRARIES
	NAMES ${libtolua++}
	PATHS ${CMAKE_CURRENT_LIST_DIR}/../../../libs/bin/
)
