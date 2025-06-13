file(GLOB sources src/*.c)


include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if ( _DEBUG  )
  add_library(lua-5.1_d ${LIBS_TYPE} ${sources})	
else()
   add_library(lua-5.1_r ${LIBS_TYPE} ${sources})
endif()



