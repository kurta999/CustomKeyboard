cmake_minimum_required(VERSION 3.7)

#target_sources(myapp PUBLIC
#	src/struct_parser.cpp
#)


set(libs_SOURCES 
	${CMAKE_CURRENT_SOURCE_DIR}/lodepng/lodepng.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/sha256/sha256.c
	${CMAKE_CURRENT_SOURCE_DIR}/sqlite/sqlite3.c
	CACHE FORCE ""
)
add_library(libs ${libs_SOURCES} )


#add_executable(${PROJECT_NAME} ${PROG_SRCS})
#target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES})


