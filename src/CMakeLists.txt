cmake_minimum_required(VERSION 3.7)

#target_sources(myapp PUBLIC
#	src/struct_parser.cpp
#)


set(src_SOURCES 
	${CMAKE_CURRENT_SOURCE_DIR}/CustomKeyboard.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/CustomMacro.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Database.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/DirectoryBackup.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/GuiEditor.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Logger.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/MacroRecorder.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/PathSeparator.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/pch.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/PrintScreenSaver.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Sensors.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Session.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Settings.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/StructParser.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SymlinkCreator.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Utils.cpp
	CACHE FORCE ""
)
add_library(src ${src_SOURCES} )

find_package(wxWidgets COMPONENTS core base xrc REQUIRED)
include(${wxWidgets_USE_FILE})

target_include_directories(src PRIVATE
	${Boost_INCLUDE_DIR}
)

find_package(Boost 1.78.0 COMPONENTS filesystem regex) 

add_subdirectory(gui)
add_subdirectory(utils)

#add_executable(${PROJECT_NAME} ${PROG_SRCS})
target_precompile_headers(src PRIVATE pch.h)
#target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES})


