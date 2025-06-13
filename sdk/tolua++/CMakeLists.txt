file(GLOB sources src/*.c)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_definitions(-D_GNU_SOURCE -D_REENTRANT)

if ( _DEBUG  )
  add_library(tolua++_d ${LIBS_TYPE} ${sources})	
else()
   add_library(tolua++_r ${LIBS_TYPE} ${sources})
endif()



