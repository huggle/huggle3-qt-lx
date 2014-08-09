# This is a build file for tst_testmain (used with cmake)
# you will need to update it by hand
cmake_minimum_required (VERSION 2.8.9)
PROJECT(tst_testmain)
execute_process(COMMAND "${CMAKE_SOURCE_DIR}/prepare_cmake.sh" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
#  cmake <folder> -DQT5_BUILD=true
option(QT5_BUILD "Build tst_testmain using the Qt5 framework" FALSE)
SET(CMAKE_CXX_FLAGS "-std=c++11")
SET(QT_USE_QTNETWORK TRUE)
SET(QT_USE_QTTEST TRUE)
SET(QT_USE_QTXML TRUE)
SET(QT_USE_QTWEBKIT TRUE)
SET(CMAKE_BUILD_TYPE Release)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(CMAKE_AUTOMOC ON)
#### Qt 4 and 5 ####
if(QT5_BUILD)
  find_package(Qt5Core REQUIRED)
  find_package(Qt5Gui REQUIRED)
  find_package(Qt5Test REQUIRED)
  find_package(Qt5WebKit REQUIRED)
  find_package(Qt5Xml REQUIRED)
  find_package(Qt5Widgets REQUIRED)
  find_package(Qt5Network REQUIRED)
  set(QT_INCLUDES
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${Qt5WebKit_INCLUDE_DIRS}
    ${Qt5Xml_INCLUDE_DIRS}
    ${Qt5Test_INCLUDE_DIRS}
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
    "../../*.ui"
)

SET(tst_testmain_SOURCES ${srcx} ../../aboutform.cpp ../../collectable_smartptr.cpp ../../huggleoption.cpp ../../configuration.cpp ../../wikipagetagsform.cpp ../../scorewordsdbform.cpp ../../core.cpp ../../wikiuser.cpp ../../historyitem.cpp ../../mediawiki.cpp ../../mediawikiobject.cpp ../../wikiutil.cpp ../../querypool.cpp ../../configuration.cpp ../../localization.cpp ../../wikisite.cpp ../../wikipage.cpp ../../resources.cpp ../../core.cpp ../../generic.cpp ../../queryresult.cpp ../../warninglist.cpp ../../huggleparser.cpp ../../query.cpp ../../syslog.cpp ../../gc.cpp ../../exception.cpp ../../deleteform.cpp ../../updateform.cpp ../../collectable.cpp ../../blockuser.cpp ../../apiquery.cpp ../../message.cpp ../../mainwindow.cpp ../../login.cpp ../../ignorelist.cpp ../../iextension.cpp ../../reloginform.cpp ../../huggletool.cpp ../../huggleweb.cpp ../../hugglequeueitemlabel.cpp ../../hugglequeuefilter.cpp ../../hugglequeue.cpp ../../warnings.cpp ../../hugglelog.cpp ../../hugglefeedproviderwiki.cpp ../../hugglefeedproviderirc.cpp ../../hugglefeed.cpp ../../hooks.cpp ../../historyform.cpp ../../history.cpp ../../exceptionwindow.cpp ../../editquery.cpp ../../sleeper.cpp ../../wlquery.cpp ../../loadingform.cpp ../../wikisite.cpp ../../wikiedit.cpp ../../networkirc.cpp ../../vandalnw.cpp ../../speedyform.cpp ../../whitelistform.cpp ../../speedyform.cpp ../../processlist.cpp ../../protectpage.cpp ../../proxy.cpp ../../waitingform.cpp ../../terminalparser.cpp ../../reportuser.cpp ../../preferences.cpp ../../uaareport.cpp ../../userinfoform.cpp ../../pythonengine.cpp ../../sessionform.cpp ../../requestprotect.cpp ../../revertquery.cpp ../../gc_thread.cpp ../../webserverquery.cpp ../../userconfiguration.cpp ../../projectconfiguration.cpp)


SET(tst_testmain_FORMS ${uixx})
SET(tst_testmain_RESOURCES ${resx})
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
if (QT5_BUILD)
    qt5_use_modules(tst_testmain Core Gui Widgets Test WebKitWidgets WebKit Network Xml)
endif()
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
