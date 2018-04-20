# This is a build file for Huggle (used with cmake)
cmake_minimum_required (VERSION 2.8.7)

if (NOT HUGGLE_CMAKE)
    message(FATAL_ERROR "This cmake file can't be used on its own, it must be included from parent folder")
endif()

project(huggle)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
set(QT_INCLUDES ${Qt5Gui_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS})
include_directories(${QT_INCLUDES})

file(GLOB srcx "*.cpp")
file(GLOB resx "*.qrc")
file(GLOB uixx "*.ui")
file(GLOB headers "*.hpp")

set(huggle_SOURCES ${srcx} ${srcx_web})
set(huggle_FORMS ${uixx} ${uixx_web})
set(huggle_RESOURCES ${resx} ${resx_web})

QT5_WRAP_CPP(huggle_HEADERS_MOC ${huggle_HEADERS})
QT5_WRAP_UI(huggle_FORMS_HEADERS ${huggle_FORMS})
QT5_ADD_RESOURCES(huggle_RESOURCES_RCC ${huggle_RESOURCES})

add_definitions(${QT_DEFINITIONS})

add_executable(huggle ${huggle_SOURCES} ${headers} ${headers_web} ${huggle_HEADERS_MOC} ${huggle_FORMS_HEADERS} ${huggle_RESOURCES_RCC})
target_link_libraries(huggle ${QT_LIBRARIES})

if (WIN32)
    target_link_libraries(huggle dbghelp)
endif()

target_link_libraries(huggle huggle_core huggle_ui)

qt5_use_modules(huggle Core Gui Widgets)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
install(FILES ${CMAKE_SOURCE_DIR}/huggle/man/huggle.1 DESTINATION share/man/man1)
install(TARGETS huggle DESTINATION bin)
install(FILES ${CMAKE_SOURCE_DIR}/build/huggle.desktop DESTINATION share/applications)
