# This is a build file for Huggle (used with cmake)
cmake_minimum_required (VERSION 3.5)

if (NOT HUGGLE_CMAKE)
    message(FATAL_ERROR "This cmake file can't be used on its own, it must be included from parent folder")
endif()

project(huggle_core)
set(HUGGLE_BUILD_REVISION "production build")
set(HUGGLE_BUILD_HASH "")
find_package(Git QUIET)
if (GIT_FOUND)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse --is-inside-work-tree
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        RESULT_VARIABLE HUGGLE_GIT_REPOSITORY_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )
    if (HUGGLE_GIT_REPOSITORY_RESULT EQUAL 0)
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" rev-list HEAD --count
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            RESULT_VARIABLE HUGGLE_GIT_COUNT_RESULT
            OUTPUT_VARIABLE HUGGLE_BUILD_REVISION
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" describe --always --tags
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            RESULT_VARIABLE HUGGLE_GIT_DESCRIBE_RESULT
            OUTPUT_VARIABLE HUGGLE_BUILD_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if (NOT HUGGLE_GIT_COUNT_RESULT EQUAL 0 OR NOT HUGGLE_GIT_DESCRIBE_RESULT EQUAL 0)
            message(FATAL_ERROR "Unable to read the Huggle Git revision")
        endif()
    endif()
endif()
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/version.txt"
     "build: ${HUGGLE_BUILD_REVISION} ${HUGGLE_BUILD_HASH}\n")

include(GNUInstallDirs)

set(CMAKE_include_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

if (QT6_BUILD)
    find_package(Qt6Core REQUIRED)
    find_package(Qt6Gui REQUIRED)
    find_package(Qt6Xml REQUIRED)
    find_package(Qt6Network REQUIRED)
    find_package(Qt6Qml REQUIRED)
    set(QT_INCLUDES ${Qt6Gui_INCLUDE_DIRS} ${Qt6Network_INCLUDE_DIRS} ${Qt6Qml_INCLUDE_DIRS} ${Qt6Xml_INCLUDE_DIRS})
    if (AUDIO)
        find_package(Qt6Multimedia REQUIRED)
        set(QT_INCLUDES ${QT_INCLUDES} ${Qt6Multimedia_INCLUDE_DIRS})
    endif()
elseif (QT5_BUILD)
    find_package(Qt5Core REQUIRED)
    find_package(Qt5Gui REQUIRED)
    find_package(Qt5Xml REQUIRED)
    find_package(Qt5Network REQUIRED)
    find_package(Qt5Qml REQUIRED)
    set(QT_INCLUDES ${Qt5Gui_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS} ${Qt5Qml_INCLUDE_DIRS} ${Qt5Xml_INCLUDE_DIRS})
    if (AUDIO)
        find_package(Qt5Multimedia REQUIRED)
        set(QT_INCLUDES ${QT_INCLUDES} ${Qt5Multimedia_INCLUDE_DIRS})
    endif()
endif()
include_directories(${QT_INCLUDES})

file(GLOB srcx "*.cpp" "scripting/*.cpp")
file(GLOB resx "*.qrc" "scripting/*.qrc")
file(GLOB headers "*.hpp" "scripting/*.hpp")

set(huggle_core_SOURCES ${srcx})
set(huggle_core_RESOURCES ${resx})

if (QT6_BUILD)
    QT6_WRAP_CPP(huggle_core_HEADERS_MOC ${huggle_core_HEADERS})
    QT6_ADD_RESOURCES(huggle_core_RESOURCES_RCC ${huggle_core_RESOURCES})
elseif (QT5_BUILD)
    QT5_WRAP_CPP(huggle_core_HEADERS_MOC ${huggle_core_HEADERS})
    QT5_ADD_RESOURCES(huggle_core_RESOURCES_RCC ${huggle_core_RESOURCES})
endif()

add_definitions(${QT_DEFINITIONS})
add_definitions( -DQT_USE_QSTRINGBUILDER )

add_library(huggle_core SHARED ${huggle_core_SOURCES} ${headers} ${huggle_core_HEADERS_MOC} ${huggle_core_RESOURCES_RCC})
target_link_libraries(huggle_core ${QT_LIBRARIES})
set_target_properties(huggle_core PROPERTIES COMPILE_FLAGS -DHUGGLE_LIBRARY_CORE)

if (WIN32)
    target_link_libraries(huggle_core dbghelp)
endif()

target_link_libraries(huggle_core huggle_l10n irc ircclient yaml-cpp)

if (QT6_BUILD)
    target_link_libraries(huggle_core Qt6::Core Qt6::Gui Qt6::Network Qt6::Xml Qt6::Qml)
    if (AUDIO)
        target_link_libraries(huggle_core Qt6::Multimedia)
    endif()
elseif (QT5_BUILD)
    target_link_libraries(huggle_core Qt5::Core Qt5::Gui Qt5::Network Qt5::Xml Qt5::Qml)
    if (AUDIO)
        target_link_libraries(huggle_core Qt5::Multimedia)
    endif()
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})
IF (NOT WIN32)
    INSTALL(TARGETS huggle_core LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
ENDIF()
