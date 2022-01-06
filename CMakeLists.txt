cmake_minimum_required(VERSION 3.7)
project(CustomKeyboard)

set (CMAKE_CXX_STANDARD 17)

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 

find_package(wxWidgets COMPONENTS core base stc std propgrid aui REQUIRED)
include(${wxWidgets_USE_FILE})

find_package(Boost 1.78.0 COMPONENTS filesystem ) 

find_package(fmt)

add_subdirectory(libs)

include_directories(libs)
include_directories(wxWidgets_INCLUDE_DIRS)

if(MSVC)
	add_definitions(-DWXUSINGDLL=1 -D_CRT_SECURE_NO_WARNINGS -DWIN32 -D_WINDOWS -D_WIN32_WINNT=0x0601)
endif()

set(src_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/src/CustomKeyboard.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/CustomMacro.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Database.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/DirectoryBackup.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/GuiEditor.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MacroRecorder.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/PathSeparator.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/pch.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/PrintScreenSaver.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Sensors.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Session.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Settings.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/StructParser.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/SymlinkCreator.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Utils.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/Configuration.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/ConfigurationBackup.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/Editor.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/EscaperPanel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/FilePanel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/LogPanel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/main_frame.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/MainPanel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/ParserPanel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/TrayIcon.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/utils/AsyncSerial.cpp
)
 
#message("src_SOURCES: " ${src_SOURCES})

add_executable(${PROJECT_NAME} ${src_SOURCES})
target_link_libraries(${PROJECT_NAME} fmt::fmt-header-only $<TARGET_OBJECTS:libs> ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})

install (TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
