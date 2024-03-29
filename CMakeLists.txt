cmake_minimum_required(VERSION 3.7)
project(CustomKeyboard)

set (CMAKE_CXX_STANDARD 20)

set(wxWidgets_USE_DEBUG ON)
set(wxWidgets_USE_STATIC OFF)
find_package(wxWidgets COMPONENTS core base stc propgrid aui REQUIRED)
include(${wxWidgets_USE_FILE})

set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost REQUIRED system program_options)

find_package(fmt REQUIRED)

set(USE_HIDAPI true CACHE BOOL "Use HIDAPI library?")
if(USE_HIDAPI)
	find_package(hidapi REQUIRED)
	add_definitions(-DUSE_HIDAPI)
endif()

include_directories(libs)
#include_directories(libs/bitfield)
#include_directories(libs/isotp)
include_directories(src/interface)
include_directories(wxWidgets_INCLUDE_DIRS)

add_subdirectory(libs)

set(src_SOURCES_pch
	#${CMAKE_CURRENT_SOURCE_DIR}/src/pch.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/AntiLock.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/CanEntryHandler.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/CanDeviceLawicel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/CanDeviceStm32.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/CanSerialPort.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/CryptoPrice.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/CmdExecutor.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/CustomKeyboard.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/CustomMacro.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/CorsairHid.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/DatabaseImpl.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/DatabaseLogic.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/DirectoryBackup.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/ExceptionHandler.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/GuiEditor.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/IdlePowerSaver.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MacroRecorder.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/PathSeparator.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/PrintScreenSaver.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Sensors.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Session.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Settings.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/StructParser.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/SymlinkCreator.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/TerminalHotkey.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/TcpMessageExecutor.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/ModbusMasterSerialPort.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MapConverter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/SerialPort.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/SerialTcpBackend.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/StringToCEscaper.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Utils.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/CanPanel/BitEditorDialog.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/CanPanel/CanLogPanel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/CanPanel/CanPanel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/CanPanel/CanScriptPanel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/CanPanel/CanSenderPanel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/CanPanel/CanUdsRawDialog.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/Configuration.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/ConfigurationBackup.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/CmdExecutorPanel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/Editor.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/EscaperPanel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/FilePanel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/LogPanel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/MainFrame.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/MainPanel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/ModbusMasterPanel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/MapConverterPanel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/ParserPanel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/DebugPanel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/TrayIcon.cpp
)
 
 set(src_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/src/utils/AsyncSerial.cpp
)
if(MSVC)
	add_definitions(-DWXUSINGDLL=1 -D_CRT_SECURE_NO_WARNINGS -DWIN32 -D_WINDOWS -D_WIN32_WINNT=0x0601)
	#target_precompile_headers(${PROJECT_NAME} PRIVATE src/pch.hpp)

	set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/pch.cpp
        PROPERTIES
        COMPILE_FLAGS "/Ycpch.hpp"
        )
    foreach( myproject_SOURCE_FILES ${src_SOURCES_pch} )
		message("src file: "${myproject_SOURCE_FILES})
        set_source_files_properties(
            ${myproject_SOURCE_FILES}
            PROPERTIES
            COMPILE_FLAGS "/Yupch.hpp"
            )
    endforeach( myproject_SOURCE_FILES ${src_SOURCES_pch} )
    list(APPEND src_SOURCES_pch ${CMAKE_CURRENT_SOURCE_DIR}/src/pch.cpp)
	#add_precompiled_header( ${PROJECT_NAME} pch.hpp FORCEINCLUDE src_SOURCES_pch pch.cpp )
	
	# Create the source groups for source tree with root at VD_SRC.
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${src_SOURCES_pch})
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type")
endif()

#message("wxWidgets_LIBRARIES: " ${wxWidgets_LIBRARIES})

add_executable(${PROJECT_NAME} WIN32 ${src_SOURCES_pch} ${src_SOURCES})
if(USE_HIDAPI)
	target_link_libraries(${PROJECT_NAME} $<TARGET_OBJECTS:libs> ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_DL_LIBS} fmt::fmt hidapi::hidapi)
else()
	target_link_libraries(${PROJECT_NAME} $<TARGET_OBJECTS:libs> ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_DL_LIBS} fmt::fmt)
endif()

install (TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
