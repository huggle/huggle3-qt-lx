# This is a build file for Huggle (used with cmake)
cmake_minimum_required (VERSION 2.8.7...4.0.0)

if (NOT HUGGLE_CMAKE)
    message(FATAL_ERROR "This cmake file can't be used on its own, it must be included from parent folder")
endif()

project(huggle)
include(GNUInstallDirs)

if (QT6_BUILD)
    find_package(Qt6Core REQUIRED)
    find_package(Qt6Gui REQUIRED)
    find_package(Qt6Widgets REQUIRED)
    set(QT_INCLUDES ${Qt6Gui_INCLUDE_DIRS} ${Qt6Widgets_INCLUDE_DIRS})
    include_directories(${QT_INCLUDES})
elseif (QT5_BUILD)
    find_package(Qt5Core REQUIRED)
    find_package(Qt5Gui REQUIRED)
    find_package(Qt5Widgets REQUIRED)
    set(QT_INCLUDES ${Qt5Gui_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS})
    include_directories(${QT_INCLUDES})
endif()

file(GLOB srcx "*.cpp")
file(GLOB resx "*.qrc")
file(GLOB uixx "*.ui")
file(GLOB headers "*.hpp")

set(huggle_SOURCES ${srcx})
set(huggle_FORMS ${uixx})
set(huggle_RESOURCES ${resx})

if (QT6_BUILD)
    QT6_WRAP_CPP(huggle_HEADERS_MOC ${huggle_HEADERS})
    QT6_WRAP_UI(huggle_FORMS_HEADERS ${huggle_FORMS})
    QT6_ADD_RESOURCES(huggle_RESOURCES_RCC ${huggle_RESOURCES})
elseif (QT5_BUILD)
    QT5_WRAP_CPP(huggle_HEADERS_MOC ${huggle_HEADERS})
    QT5_WRAP_UI(huggle_FORMS_HEADERS ${huggle_FORMS})
    QT5_ADD_RESOURCES(huggle_RESOURCES_RCC ${huggle_RESOURCES})
endif()

add_definitions(${QT_DEFINITIONS})

add_executable(huggle ${huggle_SOURCES} ${headers} ${huggle_HEADERS_MOC} ${huggle_FORMS_HEADERS} ${huggle_RESOURCES_RCC})
target_link_libraries(huggle ${QT_LIBRARIES})

if (WIN32)
    target_link_libraries(huggle dbghelp)
endif()

target_link_libraries(huggle huggle_l10n huggle_res huggle_core huggle_ui)

if (QT6_BUILD)
target_link_libraries(huggle Qt6::Core Qt6::Gui Qt6::Widgets)
elseif (QT5_BUILD)
target_link_libraries(huggle Qt5::Core Qt5::Gui Qt5::Widgets)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})
install(FILES ${CMAKE_SOURCE_DIR}/huggle/man/huggle.1 DESTINATION share/man/man1)
install(TARGETS huggle DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_SOURCE_DIR}/build/huggle.desktop DESTINATION share/applications)
