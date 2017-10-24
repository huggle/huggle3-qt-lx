# This is a build file for tst_testmain (used with cmake)
# you will need to update it by hand
cmake_minimum_required (VERSION 2.8.0)
PROJECT(tst_testmain)
execute_process(COMMAND "${CMAKE_SOURCE_DIR}/prepare_cmake.sh" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
#  cmake <folder> -DQT5_BUILD=true
option(QT5_BUILD "Build tst_testmain using the Qt5 framework" FALSE)
option(WEB_ENGINE "Use Qt WebEngine instead of WebKit library" FALSE)
option(AUDIO "Enable audio backend" FALSE)
if (NOT AUDIO)
    add_definitions(-DHUGGLE_NOAUDIO)
endif()

if (WEB_ENGINE)
    add_definitions(-DHUGGLE_WEBEN)
endif()

if(WIN32)
    if(MINGW)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows -std=c++11")
    endif()
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()
SET(QT_USE_QTNETWORK TRUE)
SET(QT_USE_QTTEST TRUE)
SET(QT_USE_QTXML TRUE)
set(QT_USE_QTWEBKIT TRUE)
SET(CMAKE_BUILD_TYPE Release)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(CMAKE_AUTOMOC ON)
#### Qt 4 and 5 ####
if(QT5_BUILD)
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
    "../../*.ui"
)

if (NOT WEB_ENGINE)
    file(GLOB srcx_web "../../webkit/*.cpp")
    file(GLOB resx_web "../../webkit/*.qrc")
    file(GLOB uixx_web "../../webkit/*.ui")
    file(GLOB headers_web "../../webkit*.hpp")
else()
    file(GLOB srcx_web "../../web_engine/*.cpp")
    file(GLOB resx_web "../../web_engine/*.qrc")
    file(GLOB uixx_web "../../web_engine/*.ui")
    file(GLOB headers_web "../../web_engine*.hpp")
endif()

add_subdirectory(../../libs/libirc/libirc libirc)
add_subdirectory(../../libs/libirc/libircclient libircclient)

# YAML
option(YAML_CPP_BUILD_TOOLS "Disable tests" OFF)
option(BUILD_SHARED_LIBS "Build as shared" ON)
add_subdirectory(../../libs/yaml-cpp yaml-cpp)
include_directories("../../libs/yaml-cpp/include/")

SET(tst_testmain_SOURCES ${srcx} ${srcx_web}
../../aboutform.cpp
../../apiqueryresult.cpp
../../collectable_smartptr.cpp
../../configuration.cpp
../../editbaritem.cpp
../../editbar.cpp
../../huggleoption.cpp
../../hw.cpp
../../wikipagetagsform.cpp
../../scorewordsdbform.cpp
../../core.cpp
../../wikiuser.cpp
../../historyitem.cpp
../../mediawiki.cpp
../../mediawikiobject.cpp
../../wikiutil.cpp
../../querypool.cpp
../../configuration.cpp
../../localization.cpp
../../wikisite.cpp
../../wikipage.cpp
../../resources.cpp
../../core.cpp
../../generic.cpp
../../queryresult.cpp
../../warninglist.cpp
../../huggleparser.cpp
../../query.cpp
../../syslog.cpp
../../gc.cpp
../../exception.cpp
../../deleteform.cpp
../../updateform.cpp
../../collectable.cpp
../../blockuser.cpp
../../apiquery.cpp
../../message.cpp
../../mainwindow.cpp
../../login.cpp
../../ignorelist.cpp
../../iextension.cpp
../../reloginform.cpp
../../huggletool.cpp
../../genericbrowser.cpp
../../hugglequeueitemlabel.cpp
../../hugglequeuefilter.cpp
../../hugglequeue.cpp
../../warnings.cpp
../../hugglelog.cpp
../../hugglefeedproviderwiki.cpp
../../hugglefeedproviderxml.cpp
../../hugglefeedproviderirc.cpp
../../hugglefeed.cpp
../../hooks.cpp
../../historyform.cpp
../../version.cpp
../../history.cpp
../../exceptionwindow.cpp
../../editquery.cpp
../../sleeper.cpp
../../wlquery.cpp
../../loadingform.cpp
../../wikisite.cpp
../../wikiedit.cpp
../../vandalnw.cpp
../../speedyform.cpp
../../ircchattextbox.cpp
../../whitelistform.cpp
../../speedyform.cpp
../../processlist.cpp
../../protectpage.cpp
../../proxy.cpp
../../waitingform.cpp
../../terminalparser.cpp
../../reportuser.cpp
../../preferences.cpp
../../uaareport.cpp
../../userinfoform.cpp
../../pythonengine.cpp
../../sessionform.cpp
../../requestprotect.cpp
../../revertquery.cpp
../../xmlutils.cpp
../../gc_thread.cpp
../../welcomeinfo.cpp
../../webserverquery.cpp
../../userconfiguration.cpp
../../projectconfiguration.cpp
../../queuehelp.cpp
../../custommessage.cpp)

SET(tst_testmain_FORMS ${uixx} ${uixx_web})
SET(tst_testmain_RESOURCES ${resx} ${resx_web})
if(QT5_BUILD)
    QT5_WRAP_CPP(tst_testmain_HEADERS_MOC ${tst_testmain_HEADERS})
    QT5_WRAP_UI(tst_testmain_FORMS_HEADERS ${tst_testmain_FORMS})
    QT5_ADD_RESOURCES(tst_testmain_RESOURCES_RCC ${tst_testmain_RESOURCES})
else()
    QT4_WRAP_CPP(tst_testmain_HEADERS_MOC ${tst_testmain_HEADERS})
    QT4_WRAP_UI(tst_testmain_FORMS_HEADERS ${tst_testmain_FORMS})
    QT4_ADD_RESOURCES(tst_testmain_RESOURCES_RCC ${tst_testmain_RESOURCES})
endif()
if (NOT QT5_BUILD)
  INCLUDE(${QT_USE_FILE})
endif()
ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_EXECUTABLE(tst_testmain ${tst_testmain_SOURCES} ${tst_testmain_HEADERS_MOC} ${tst_testmain_FORMS_HEADERS} ${tst_testmain_RESOURCES_RCC})
TARGET_LINK_LIBRARIES(tst_testmain ${QT_LIBRARIES})
TARGET_LINK_LIBRARIES(tst_testmain irc ircclient yaml-cpp)
if (QT5_BUILD)
    if (NOT WEB_ENGINE)
        qt5_use_modules(tst_testmain Core Test Gui Widgets WebKitWidgets WebKit Network Xml)
    else()
        qt5_use_modules(tst_testmain Core Test Gui Widgets WebEngineWidgets WebEngine Network Xml)
    endif()
endif()
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
