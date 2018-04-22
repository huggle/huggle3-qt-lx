# This is a build file for Huggle (used with cmake)
cmake_minimum_required (VERSION 2.8.7)

if (NOT HUGGLE_CMAKE)
    message(FATAL_ERROR "This cmake file can't be used on its own, it must be included from parent folder")
endif()

project(huggle_res)
execute_process(COMMAND "${CMAKE_SOURCE_DIR}/update.sh" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

set(CMAKE_include_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Xml REQUIRED)
set(QT_INCLUDES ${Qt5Gui_INCLUDE_DIRS} ${Qt5Xml_INCLUDE_DIRS} ${Qt5Svg_INCLUDE_DIRS})
include_directories(${QT_INCLUDES})

file(GLOB srcx "*.cpp")
file(GLOB resx "*.qrc")
file(GLOB headers "*.hpp")

set(huggle_res_SOURCES ${srcx})
set(huggle_res_RESOURCES ${resx})

QT5_WRAP_CPP(huggle_res_HEADERS_MOC ${huggle_res_HEADERS})
QT5_ADD_RESOURCES(huggle_res_RESOURCES_RCC ${huggle_res_RESOURCES})

add_definitions(${QT_DEFINITIONS})
add_definitions( -DQT_USE_QSTRINGBUILDER )

add_library(huggle_res SHARED ${huggle_res_SOURCES} ${headers} ${huggle_res_HEADERS_MOC} ${huggle_res_FORMS_HEADERS} ${huggle_res_RESOURCES_RCC})
target_link_libraries(huggle_res ${QT_LIBRARIES})
set_target_properties(huggle_res PROPERTIES COMPILE_FLAGS -DHUGGLE_LIBRARY)

if (WIN32)
    target_link_libraries(huggle_res dbghelp)
endif()

qt5_use_modules(huggle_res Core Xml)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
INSTALL(TARGETS huggle_res DESTINATION lib)
