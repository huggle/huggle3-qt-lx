# This is a build file for huggle sdk (used with cmake)
cmake_minimum_required (VERSION 2.8.9)
PROJECT(huggle_core)
ADD_DEFINITIONS(-DHUGGLE_SDK)
option(QT5_BUILD "Build huggle using the Qt5 framework" FALSE)
SET(QT_USE_QTNETWORK TRUE)
SET(QT_USE_QTXML TRUE)
SET(CMAKE_BUILD_TYPE Release)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(CMAKE_AUTOMOC ON)
if(WIN32)
    if(MINGW)
	SET(CMAKE_CXX_FLAGS "-mwindows -std=c++11")
    endif()
else()
	SET(CMAKE_CXX_FLAGS "-std=c++11")
endif()
#### Qt 4 and 5 ####
if(QT5_BUILD)
    find_package(Qt5Core REQUIRED)
    find_package(Qt5Gui REQUIRED)
    find_package(Qt5Xml REQUIRED)
    find_package(Qt5Network REQUIRED)
    set(QT_INCLUDES
        ${Qt5Gui_INCLUDE_DIRS}
        ${Qt5Network_INCLUDE_DIRS}
        ${Qt5Xml_INCLUDE_DIRS}
        ${Qt5Svg_INCLUDE_DIRS}
    )
    include_directories(${QT_INCLUDES})
else()
    find_package(Qt4 REQUIRED)
endif()

file(GLOB srcx
    "*.cpp"
)
file (GLOB resx
    "*.qrc"
)
file (GLOB uixx
    "*.ui"
)

FILE (STRINGS "sources.list" source_list)

SET(huggle_SOURCES ${srcx} ${source_list})

SET(huggle_FORMS ${uixx})
SET(huggle_RESOURCES ${resx})
if(QT5_BUILD)
    QT5_WRAP_CPP(huggle_HEADERS_MOC ${huggle_HEADERS})
    QT5_WRAP_UI(huggle_FORMS_HEADERS ${huggle_FORMS})
    QT5_ADD_RESOURCES(huggle_RESOURCES_RCC ${huggle_RESOURCES})
else()
    QT4_WRAP_CPP(huggle_HEADERS_MOC ${huggle_HEADERS})
    QT4_WRAP_UI(huggle_FORMS_HEADERS ${huggle_FORMS})
    QT4_ADD_RESOURCES(huggle_RESOURCES_RCC ${huggle_RESOURCES})
endif()
if (NOT QT5_BUILD)
  INCLUDE(${QT_USE_FILE})
endif()
ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS( -DQT_USE_QSTRINGBUILDER )
ADD_LIBRARY(huggle_core SHARED ${huggle_SOURCES} ${huggle_HEADERS_MOC} ${huggle_FORMS_HEADERS} ${huggle_RESOURCES_RCC})
TARGET_LINK_LIBRARIES(huggle_core ${QT_LIBRARIES})
SET_TARGET_PROPERTIES(huggle_core PROPERTIES COMPILE_FLAGS -DHUGGLE_LIBRARY)
if (QT5_BUILD)
    qt5_use_modules(huggle_core Core Network Xml)
endif()
if (WIN32)
    TARGET_LINK_LIBRARIES(huggle_core dbghelp)
endif()

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
