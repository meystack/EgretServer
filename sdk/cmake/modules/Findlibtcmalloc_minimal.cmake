 
message( status " using bundled Findlibtcmalloc_minimal.cmake..." )

#if ( _DEBUG  )
#    set( libtcmalloc   "libtcmalloc_minimal.so" )
#else()
    set( libtcmalloc   "libtcmalloc_minimal.a" )
#endif()
set( LIBRARIE_DIR_PATH ${CMAKE_CURRENT_LIST_DIR}/../../../libs/bin/ )

find_library(
	LIBTCMALLOC_MINIMAL_LIBRARIES
	NAMES ${libtcmalloc}
	PATHS ${LIBRARIE_DIR_PATH}
)
