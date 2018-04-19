# This is a build file for Huggle (used with cmake)
cmake_minimum_required (VERSION 2.8.7)

set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug or Release (or any other documented for CMAKE_BUILD_TYPE")
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
option(HUGGLE_EXT "Build Huggle extensions" false)
option(HUGGLE_TEST "Unit tests" false)
option(WEB_ENGINE "Use Qt WebEngine instead of WebKit library" false)
option(AUDIO "Enable audio backend" true)

set(HUGGLE_CMAKE true)

if (NOT AUDIO)
    add_definitions(-DHUGGLE_NOAUDIO)
endif()

if (WEB_ENGINE)
    add_definitions(-DHUGGLE_WEBEN)
endif()

if (LINUX_SNAP)
    add_definitions(-DHUGGLE_SNAP)
endif()

# Enable c++11
if(WIN32)
    if(MINGW)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows -std=c++11")
    endif()
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# This will ensure that produced binaries go to right place on windows
if(NOT MINGW AND WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/Release)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/Debug)
endif()

add_subdirectory(3rd/libirc/libirc libirc)
add_subdirectory(3rd/libirc/libircclient libircclient)

# YAML
option(YAML_CPP_BUILD_TOOLS "Disable tests" OFF)
option(BUILD_SHARED_LIBS "Build as shared" ON)
include_directories("3rd/yaml-cpp/include/")
add_subdirectory(3rd/yaml-cpp)
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

# Huggle core
add_subdirectory(huggle_core)

# Qt desktop
add_subdirectory(huggle_ui)

# Resources
add_subdirectory(huggle_res)

# Localizations
add_subdirectory(huggle_l10n)

# Huggle executable
add_subdirectory(huggle)

if (HUGGLE_TEST)
    add_subdirectory(tests)
endif()

include(${CMAKE_SOURCE_DIR}/CPackInfo)
install(FILES ${CMAKE_SOURCE_DIR}/huggle_core/Resources/huggle3_newlogo.png DESTINATION share/huggle)
include(CPack)
