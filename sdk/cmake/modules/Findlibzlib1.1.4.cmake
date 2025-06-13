if ( _DEBUG  )
    set( libzlib1.1.4 "zlib1.1.4_d" )
else()
    set( libzlib1.1.4 "zlib1.1.4_r" )
endif()

message( status " using bundled ${libzlib1.1.4}..." )

find_path(
	LIBZLIB1.1.4_INCLUDE_DIR
	NAMES zlib.h inffast.h infblock.h deflate.h zconf.h inftrees.h infcodes.h trees.h inffixed.h zutil.h infutil.h
	#NAMES *.h *.hpp
	PATHS ${CMAKE_CURRENT_LIST_DIR}/../../../sdk/zlib\ 1.1.4/src 
)
 
find_library(
	LIBZLIB1.1.4_LIBRARIES
	NAMES ${libzlib1.1.4}
	PATHS ${CMAKE_CURRENT_LIST_DIR}/../../../libs/bin/
)
