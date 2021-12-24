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

include_directories(libs)
#include_directories(libs/fmt)
include_directories(wxWidgets_INCLUDE_DIRS)

add_subdirectory(libs)
add_subdirectory(src) 
 
message("PROJECT_NAME: " ${PROJECT_NAME})
message("src_SOURCES: " ${src_SOURCES})
message("gui_SOURCES: " ${gui_SOURCES})
message("libs_SOURCES: " ${libs_SOURCES})
message("wxWidgets_LIBRARIES: " ${wxWidgets_LIBRARIES})
message("Boost_LIBRARIES: " ${Boost_LIBRARIES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-PIE")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fno-pie")

add_executable(${PROJECT_NAME} $<TARGET_OBJECTS:libs> $<TARGET_OBJECTS:src> $<TARGET_OBJECTS:gui> $<TARGET_OBJECTS:utils>)
target_link_libraries(${PROJECT_NAME} libs fmt::fmt-header-only src ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_DL_LIBS} gui)


install (TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
