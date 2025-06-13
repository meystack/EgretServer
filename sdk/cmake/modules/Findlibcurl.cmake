

if ( _DEBUG  )
    set( libcurl   "curl_d" )
else()
    set( libcurl   "curl_r" )
endif()


message( status " using bundled ${libcurl}" )

find_path(
	LIBCURL_INCLUDE_DIR
	NAMES  curl.h
	PATHS ${CMAKE_CURRENT_LIST_DIR}/../../../../common/cpp/libs/3rd/inc/curl
)

find_library(
	CURL_LIBRARIE_DIR_PATH
	NAMES ${libcurl} 
	PATHS  ${CMAKE_CURRENT_LIST_DIR}/../../../../common/cpp/libs/3rd/bin
)
