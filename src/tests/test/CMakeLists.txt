# This is a build file for tst_testmain (used with cmake)
# you will need to update it by hand
cmake_minimum_required (VERSION 3.5)

if (NOT HUGGLE_CMAKE)
    message(FATAL_ERROR "This cmake file can't be used on its own, it must be included from parent folder")
endif()

PROJECT(tst_testmain)
SET(CMAKE_AUTOMOC ON)

if (QT6_BUILD)
    find_package(Qt6Core REQUIRED)
    find_package(Qt6Gui REQUIRED)
    find_package(Qt6Test REQUIRED)
    find_package(Qt6Xml REQUIRED)
    find_package(Qt6Widgets REQUIRED)
    find_package(Qt6Network REQUIRED)
    set(QT_INCLUDES
        ${Qt6Gui_INCLUDE_DIRS}
        ${Qt6Widgets_INCLUDE_DIRS}
        ${Qt6Network_INCLUDE_DIRS}
        ${Qt6Xml_INCLUDE_DIRS}
        ${Qt6Test_INCLUDE_DIRS}
        ${Qt6Svg_INCLUDE_DIRS}
    )
    if (NOT WEB_ENGINE)
        find_package(Qt6WebKitWidgets REQUIRED)
        find_package(Qt6WebKit REQUIRED)
        set(QT_INCLUDES ${QT_INCLUDES} ${Qt6WebKit_INCLUDE_DIRS})
    else()
        find_package(Qt6WebEngine REQUIRED)
        find_package(Qt6WebEngineWidgets REQUIRED)
        set(QT_INCLUDES ${QT_INCLUDES} ${Qt6WebEngine_INCLUDE_DIRS})
    endif()
elseif (QT5_BUILD)
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
        find_package(Qt5WebKitWidgets REQUIRED)
        find_package(Qt5WebKit REQUIRED)
        set(QT_INCLUDES ${QT_INCLUDES} ${Qt5WebKit_INCLUDE_DIRS})
    else()
        find_package(Qt5WebEngine REQUIRED)
        find_package(Qt5WebEngineWidgets REQUIRED)
        set(QT_INCLUDES ${QT_INCLUDES} ${Qt5WebEngine_INCLUDE_DIRS})
    endif()
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

if (QT6_BUILD)
    QT6_WRAP_CPP(tst_testmain_HEADERS_MOC ${tst_testmain_HEADERS})
    QT6_WRAP_UI(tst_testmain_FORMS_HEADERS ${tst_testmain_FORMS})
    QT6_ADD_RESOURCES(tst_testmain_RESOURCES_RCC ${tst_testmain_RESOURCES})
elseif (QT5_BUILD)
    QT5_WRAP_CPP(tst_testmain_HEADERS_MOC ${tst_testmain_HEADERS})
    QT5_WRAP_UI(tst_testmain_FORMS_HEADERS ${tst_testmain_FORMS})
    QT5_ADD_RESOURCES(tst_testmain_RESOURCES_RCC ${tst_testmain_RESOURCES})
endif()

ADD_DEFINITIONS(${QT_DEFINITIONS})

ADD_EXECUTABLE(tst_testmain ${tst_testmain_SOURCES} ${tst_testmain_HEADERS_MOC} ${tst_testmain_FORMS_HEADERS} ${tst_testmain_RESOURCES_RCC})
TARGET_LINK_LIBRARIES(tst_testmain ${QT_LIBRARIES})
TARGET_LINK_LIBRARIES(tst_testmain huggle_core irc ircclient yaml-cpp)

if (QT6_BUILD)
    if (NOT WEB_ENGINE)
        TARGET_LINK_LIBRARIES(tst_testmain Qt6::Core Qt6::Test Qt6::Gui Qt6::Widgets Qt6::WebKitWidgets Qt6::WebKit Qt6::Network Qt6::Xml)
    else()
        TARGET_LINK_LIBRARIES(tst_testmain Qt6::Core Qt6::Test Qt6::Gui Qt6::Widgets Qt6::WebEngineWidgets Qt6::WebEngine Qt6::Network Qt6::Xml)
    endif()
elseif (QT5_BUILD)
    if (NOT WEB_ENGINE)
        TARGET_LINK_LIBRARIES(tst_testmain Qt5::Core Qt5::Test Qt5::Gui Qt5::Widgets Qt5::WebKitWidgets Qt5::WebKit Qt5::Network Qt5::Xml)
    else()
        TARGET_LINK_LIBRARIES(tst_testmain Qt5::Core Qt5::Test Qt5::Gui Qt5::Widgets Qt5::WebEngineWidgets Qt5::WebEngine Qt5::Network Qt5::Xml)
    endif()
endif()
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
