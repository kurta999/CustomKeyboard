cmake_minimum_required(VERSION 3.7)

#target_sources(myapp PUBLIC
#	src/struct_parser.cpp
#)

set(gui_SOURCES 
	${CMAKE_CURRENT_SOURCE_DIR}/Configuration.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ConfigurationBackup.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Editor.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/EscaperPanel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/FilePanel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/LogPanel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/main_frame.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/MainPanel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ParserPanel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TrayIcon.cpp
	CACHE FORCE ""
)

add_library(gui ${gui_SOURCES} )

#add_executable(${PROJECT_NAME} ${PROG_SRCS})
target_precompile_headers(gui PRIVATE pch.h)
#target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES})


