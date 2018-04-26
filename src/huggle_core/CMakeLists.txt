# This is a build file for Huggle (used with cmake)
cmake_minimum_required (VERSION 2.8.7)

if (NOT HUGGLE_CMAKE)
    message(FATAL_ERROR "This cmake file can't be used on its own, it must be included from parent folder")
endif()

project(huggle_core)
execute_process(COMMAND "${CMAKE_SOURCE_DIR}/update.sh" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

set(CMAKE_include_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Script REQUIRED)
set(QT_INCLUDES ${Qt5Network_INCLUDE_DIRS} ${Qt5Script_INCLUDE_DIRS} ${Qt5Xml_INCLUDE_DIRS})
if (AUDIO)
    find_package(Qt5Multimedia REQUIRED)
    set(QT_INCLUDES ${QT_INCLUDES} ${Qt5Multimedia_INCLUDE_DIRS})
endif()
include_directories(${QT_INCLUDES})

file(GLOB srcx "*.cpp")
file(GLOB resx "*.qrc")
file(GLOB headers "*.hpp")

set(huggle_core_SOURCES ${srcx})
set(huggle_core_RESOURCES ${resx})

QT5_WRAP_CPP(huggle_core_HEADERS_MOC ${huggle_core_HEADERS})
QT5_ADD_RESOURCES(huggle_core_RESOURCES_RCC ${huggle_core_RESOURCES})

add_definitions(${QT_DEFINITIONS})
add_definitions( -DQT_USE_QSTRINGBUILDER )

add_library(huggle_core SHARED ${huggle_core_SOURCES} ${headers} ${huggle_core_HEADERS_MOC} ${huggle_core_RESOURCES_RCC})
target_link_libraries(huggle_core ${QT_LIBRARIES})
set_target_properties(huggle_core PROPERTIES COMPILE_FLAGS -DHUGGLE_LIBRARY_CORE)

if (WIN32)
    target_link_libraries(huggle_core dbghelp)
endif()

target_link_libraries(huggle_core irc ircclient yaml-cpp)

qt5_use_modules(huggle_core Core Script Network Xml)

if (AUDIO)
    qt5_use_modules(huggle_core Multimedia)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})
IF (NOT WIN32)
    INSTALL(TARGETS huggle_core LIBRARY DESTINATION lib)
ENDIF()
