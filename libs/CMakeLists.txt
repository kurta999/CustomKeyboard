cmake_minimum_required(VERSION 3.7)

set(libs_SOURCES 
	${CMAKE_CURRENT_SOURCE_DIR}/lodepng/lodepng.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/sha256/sha256.c
	${CMAKE_CURRENT_SOURCE_DIR}/sqlite/sqlite3.c
#	${CMAKE_CURRENT_SOURCE_DIR}/bitfield/8byte.c
	${CMAKE_CURRENT_SOURCE_DIR}/bitfield/bitarray.c
	${CMAKE_CURRENT_SOURCE_DIR}/bitfield/bitfield.c
#	${CMAKE_CURRENT_SOURCE_DIR}/bitfield/read.c
#	${CMAKE_CURRENT_SOURCE_DIR}/bitfield/write.c
	${CMAKE_CURRENT_SOURCE_DIR}/isotp/isotp.c
	CACHE FORCE ""
)

if(MSVC)
	list(APPEND libs_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/enumser/enumser.cpp)
endif()

add_library(libs ${libs_SOURCES} )
