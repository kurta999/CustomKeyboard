cmake_minimum_required(VERSION 3.7)

set(utils_SOURCES 
	${CMAKE_CURRENT_SOURCE_DIR}/AsyncSerial.cpp
	CACHE FORCE ""
)

add_library(utils ${utils_SOURCES})