# This is a build file for Huggle (used with cmake)
cmake_minimum_required (VERSION 2.8.7)

if (NOT HUGGLE_CMAKE)
    message(FATAL_ERROR "This cmake file can't be used on its own, it must be included from parent folder")
endif()

project(huggle_l10n)

set(CMAKE_include_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Xml REQUIRED)
set(QT_INCLUDES ${Qt5Xml_INCLUDE_DIRS})
include_directories(${QT_INCLUDES})

file(GLOB srcx "*.cpp")
file(GLOB resx "*.qrc")
file(GLOB uixx "*.ui")
file(GLOB headers "*.hpp")

if (NOT WEB_ENGINE)
    file(GLOB srcx_web "webkit/*.cpp")
    file(GLOB resx_web "webkit/*.qrc")
    file(GLOB uixx_web "webkit/*.ui")
    file(GLOB headers_web "webkit*.hpp")
else()
    file(GLOB srcx_web "web_engine/*.cpp")
    file(GLOB resx_web "web_engine/*.qrc")
    file(GLOB uixx_web "web_engine/*.ui")
    file(GLOB headers_web "web_engine*.hpp")
endif()

set(huggle_l10n_SOURCES ${srcx} ${srcx_web})
set(huggle_l10n_FORMS ${uixx} ${uixx_web})
set(huggle_l10n_RESOURCES ${resx} ${resx_web})

QT5_WRAP_CPP(huggle_l10n_HEADERS_MOC ${huggle_l10n_HEADERS})
QT5_WRAP_UI(huggle_l10n_FORMS_HEADERS ${huggle_l10n_FORMS})
QT5_ADD_RESOURCES(huggle_l10n_RESOURCES_RCC ${huggle_l10n_RESOURCES})

add_definitions(${QT_DEFINITIONS})
add_definitions( -DQT_USE_QSTRINGBUILDER )

add_library(huggle_l10n SHARED ${huggle_l10n_SOURCES} ${headers} ${headers_web} ${huggle_l10n_HEADERS_MOC} ${huggle_l10n_FORMS_HEADERS} ${huggle_l10n_RESOURCES_RCC})
target_link_libraries(huggle_l10n ${QT_LIBRARIES})
set_target_properties(huggle_l10n PROPERTIES COMPILE_FLAGS -DHUGGLE_LIBRARY)

if (WIN32)
    target_link_libraries(huggle_l10n dbghelp)
endif()

target_link_libraries(huggle_l10n)

qt5_use_modules(huggle_l10n Core Xml)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
INSTALL(TARGETS huggle_l10n DESTINATION lib)
