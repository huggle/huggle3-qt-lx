# This is a build file for Huggle (used with cmake)
cmake_minimum_required (VERSION 3.5)

# So that MacOS builds are working on older versions as well
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug or Release (or any other documented for CMAKE_BUILD_TYPE" FORCE)
endif()
set(CMAKE_DEBUG_POSTFIX "" CACHE STRING "Suffix for debug versions of built targets")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_DATA_DIR share/${PROJECT_NAME} CACHE PATH "Installation directory for data files")

#set(CMAKE_CXX_STANDARD 11)
project(HuggleProject)
execute_process(COMMAND "${CMAKE_SOURCE_DIR}/build/prepare_cmake.sh" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

if (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.0)
    # Visual Studio
    # Enable multi threaded build for speed
    add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)
endif()

option(LINUX_SNAP "Enable support for snap, use this option only if you are building snapshotable version" false)
option(QT5_BUILD "Build Huggle using the Qt5 framework" true)
option(QT6_BUILD "Build Huggle using the Qt6 framework" false)
option(HUGGLE_PROFILING "Enable profiler" false)
option(HUGGLE_EXT "Build Huggle extensions" false)
option(HUGGLE_TEST "Unit tests" false)
option(WEB_ENGINE "Use Qt WebEngine instead of WebKit library" false)
option(AUDIO "Enable audio backend" true)

if (QT6_BUILD)
    set (QT5_BUILD false)
    set (WEB_ENGINE true)
    add_definitions( -DQT6_BUILD )
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version" FORCE)
endif()

set(HUGGLE_CMAKE true)

if (NOT AUDIO)
    add_definitions(-DHUGGLE_NOAUDIO)
endif()

if (WEB_ENGINE)
    add_definitions(-DHUGGLE_WEBEN)
endif()

if (HUGGLE_PROFILING)
    add_definitions(-DHUGGLE_PROFILING)
endif()

if (LINUX_SNAP)
    add_definitions(-DHUGGLE_SNAP)
endif()

# Enable c++11 or c++17 in case of Qt6
if(WIN32)
    if(MINGW)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows -std=c++11")
    endif()
else()
    if (QT6_BUILD)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()
endif()

# This will ensure that produced binaries go to right place on windows
if(NOT MINGW AND WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/Release)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/Debug)
endif()

add_subdirectory(3rd/libirc/libirc libirc)
add_subdirectory(3rd/libirc/libircclient libircclient)

# YAML
find_package(YAML-CPP 0.6.3 QUIET)
if (YAML-CPP_FOUND)
    message(STATUS "Using system yaml-cpp")
    include_directories(YAML_CPP_INCLUDE_DIR)
else()
    option(YAML_CPP_BUILD_TOOLS "Disable tests" OFF)
    option(YAML_CPP_BUILD_TESTS "Enable testing" OFF)
    option(YAML_CPP_BUILD_TOOLS "Enable parse tools" OFF)
    option(BUILD_SHARED_LIBS "Build as shared" ON)
    include_directories("3rd/yaml-cpp/include/")
    add_subdirectory(3rd/yaml-cpp)
endif()

if (HUGGLE_EXT)
    if(NOT MINGW AND WIN32)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/Release/extensions)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/Debug/extensions)
    endif()
    add_subdirectory(extensions/extension-scoring/)
    add_subdirectory(extensions/extension-thanks/)
    add_subdirectory(extensions/enwiki/)
    add_subdirectory(extensions/mass-delivery/)
    add_subdirectory(extensions/extension-splitter-helper/)
    add_subdirectory(extensions/extension-mass-delete/)
    add_subdirectory(extensions/extension-flow/)
    if(WIN32 AND NOT MINGW)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE)
    endif() 
endif()

add_definitions( -DQT_USE_QSTRINGBUILDER )
include_directories("3rd/")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Resources
add_subdirectory(huggle_res)

# Localizations
add_subdirectory(huggle_l10n)

# Huggle core
add_subdirectory(huggle_core)

# Qt desktop
add_subdirectory(huggle_ui)

# Huggle executable
add_subdirectory(huggle)

if (HUGGLE_TEST)
    add_subdirectory(tests)
endif()

include(${CMAKE_SOURCE_DIR}/CPackInfo)
install(FILES ${CMAKE_SOURCE_DIR}/huggle_res/Resources/huggle3_newlogo.png DESTINATION share/huggle)
if (LINUX_SNAP)
    install(DIRECTORY scripts/ DESTINATION bin/extensions FILES_MATCHING PATTERN "*.js")
else()
    install(DIRECTORY scripts/ DESTINATION share/huggle/extensions FILES_MATCHING PATTERN "*.js")
endif()

include(CPack)
