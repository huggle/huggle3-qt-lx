# This is a build file for Huggle (used with cmake)
cmake_minimum_required (VERSION 3.5)

if (NOT HUGGLE_CMAKE)
    message(FATAL_ERROR "This cmake file can't be used on its own, it must be included from parent folder")
endif()

project(huggle_l10n)
include(GNUInstallDirs)

set(CMAKE_include_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

if (QT6_BUILD)
    find_package(Qt6Core REQUIRED)
    find_package(Qt6Xml REQUIRED)
    set(QT_INCLUDES ${Qt6Xml_INCLUDE_DIRS})
    include_directories(${QT_INCLUDES})
elseif (QT5_BUILD)
    find_package(Qt5Core REQUIRED)
    find_package(Qt5Xml REQUIRED)
    set(QT_INCLUDES ${Qt5Xml_INCLUDE_DIRS})
    include_directories(${QT_INCLUDES})
endif()

file(GLOB srcx "*.cpp")
file(GLOB resx "*.qrc")
file(GLOB headers "*.hpp")

set(huggle_l10n_SOURCES ${srcx})
set(huggle_l10n_RESOURCES ${resx})

if(QT6_BUILD)
QT6_WRAP_CPP(huggle_l10n_HEADERS_MOC ${huggle_l10n_HEADERS})
QT6_ADD_RESOURCES(huggle_l10n_RESOURCES_RCC ${huggle_l10n_RESOURCES})
elseif(QT5_BUILD)
QT5_WRAP_CPP(huggle_l10n_HEADERS_MOC ${huggle_l10n_HEADERS})
QT5_ADD_RESOURCES(huggle_l10n_RESOURCES_RCC ${huggle_l10n_RESOURCES})
endif()

add_definitions(${QT_DEFINITIONS})
add_definitions( -DQT_USE_QSTRINGBUILDER )

ADD_LIBRARY(huggle_l10n SHARED ${huggle_l10n_SOURCES} ${headers} ${huggle_l10n_HEADERS_MOC} ${huggle_l10n_FORMS_HEADERS} ${huggle_l10n_RESOURCES_RCC})
target_link_libraries(huggle_l10n ${QT_LIBRARIES})
set_target_properties(huggle_l10n PROPERTIES COMPILE_FLAGS -DHUGGLE_LIBRARY_L10N)

if (WIN32)
    target_link_libraries(huggle_l10n dbghelp)
endif()

if (QT6_BUILD)
target_link_libraries(huggle_l10n Qt6::Core Qt6::Xml)
elseif (QT5_BUILD)
target_link_libraries(huggle_l10n Qt5::Core Qt5::Xml)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})
IF (NOT WIN32)
    INSTALL(TARGETS huggle_l10n LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
ENDIF()
