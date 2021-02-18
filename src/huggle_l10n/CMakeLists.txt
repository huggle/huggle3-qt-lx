# This is a build file for Huggle (used with cmake)
cmake_minimum_required (VERSION 2.8.7)

if (NOT HUGGLE_CMAKE)
    message(FATAL_ERROR "This cmake file can't be used on its own, it must be included from parent folder")
endif()

project(huggle_l10n)
include(GNUInstallDirs)

set(CMAKE_include_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Xml REQUIRED)
set(QT_INCLUDES ${Qt5Xml_INCLUDE_DIRS})
include_directories(${QT_INCLUDES})

file(GLOB srcx "*.cpp")
file(GLOB resx "*.qrc")
file(GLOB headers "*.hpp")

set(huggle_l10n_SOURCES ${srcx})
set(huggle_l10n_RESOURCES ${resx})

QT5_WRAP_CPP(huggle_l10n_HEADERS_MOC ${huggle_l10n_HEADERS})
QT5_ADD_RESOURCES(huggle_l10n_RESOURCES_RCC ${huggle_l10n_RESOURCES})

add_definitions(${QT_DEFINITIONS})
add_definitions( -DQT_USE_QSTRINGBUILDER )

ADD_LIBRARY(huggle_l10n SHARED ${huggle_l10n_SOURCES} ${headers} ${huggle_l10n_HEADERS_MOC} ${huggle_l10n_FORMS_HEADERS} ${huggle_l10n_RESOURCES_RCC})
target_link_libraries(huggle_l10n ${QT_LIBRARIES})
set_target_properties(huggle_l10n PROPERTIES COMPILE_FLAGS -DHUGGLE_LIBRARY_L10N)

if (WIN32)
    target_link_libraries(huggle_l10n dbghelp)
endif()

target_link_libraries(huggle_l10n Qt5::Core Qt5::Xml)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
IF (NOT WIN32)
    INSTALL(TARGETS huggle_l10n LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
ENDIF()
