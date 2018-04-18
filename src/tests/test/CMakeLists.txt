# This is a build file for tst_testmain (used with cmake)
# you will need to update it by hand
cmake_minimum_required (VERSION 2.8.0)

if (NOT HUGGLE_CMAKE)
    message(FATAL_ERROR "This cmake file can't be used on its own, it must be included from parent folder")
endif()

PROJECT(tst_testmain)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Test REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
set(QT_INCLUDES
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${Qt5Xml_INCLUDE_DIRS}
    ${Qt5Test_INCLUDE_DIRS}
    ${Qt5Svg_INCLUDE_DIRS}
)
if (NOT WEB_ENGINE)
    find_package(Qt5WebKit REQUIRED)
    set(QT_INCLUDES ${QT_INCLUDES} ${Qt5WebKit_INCLUDE_DIRS})
else()
    find_package(Qt5WebEngine REQUIRED)
    find_package(Qt5WebEngineWidgets REQUIRED)
    set(QT_INCLUDES ${QT_INCLUDES} ${Qt5WebEngine_INCLUDE_DIRS})
endif()
include_directories(${QT_INCLUDES})

file(GLOB srcx
    "*.cpp"
)
file (GLOB resx
    "*.qrc"
)
file (GLOB uixx
    "*.ui"
)

SET(tst_testmain_SOURCES ${srcx})
SET(tst_testmain_FORMS ${uixx})
SET(tst_testmain_RESOURCES ${resx})
QT5_WRAP_CPP(tst_testmain_HEADERS_MOC ${tst_testmain_HEADERS})
QT5_WRAP_UI(tst_testmain_FORMS_HEADERS ${tst_testmain_FORMS})
QT5_ADD_RESOURCES(tst_testmain_RESOURCES_RCC ${tst_testmain_RESOURCES})

ADD_DEFINITIONS(${QT_DEFINITIONS})

ADD_EXECUTABLE(tst_testmain ${tst_testmain_SOURCES} ${tst_testmain_HEADERS_MOC} ${tst_testmain_FORMS_HEADERS} ${tst_testmain_RESOURCES_RCC})
TARGET_LINK_LIBRARIES(tst_testmain ${QT_LIBRARIES})
TARGET_LINK_LIBRARIES(tst_testmain huggle_core irc ircclient yaml-cpp)

if (NOT WEB_ENGINE)
    qt5_use_modules(tst_testmain Core Test Gui Widgets WebKitWidgets WebKit Network Xml)
else()
    qt5_use_modules(tst_testmain Core Test Gui Widgets WebEngineWidgets WebEngine Network Xml)
endif()
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
