//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//! This file exist only for compiler options that can be changed before you build huggle
//! Please do not commit any changes in this file

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

typedef char byte_ht;

#define HUGGLE_VERSION                  "3.0.5"
#define HUGGLE_BYTE_VERSION_MAJOR       0x3
#define HUGGLE_BYTE_VERSION_MINOR       0x0
#define HUGGLE_BYTE_VERSION_RELEASE     0x5

// we are using translatewiki and if this is not defined there is a huge overhead of Qt code
#ifndef QT_NO_TRANSLATION
    #define QT_NO_TRANSLATION
#endif

// comment this out to disable multithreaded garbage collector
// this can be useful for debugging as multithreaded GC is not able to delete Qt objects, so if your code
// is crashing with it only, it means your code suck and need a fix in destructor :))
#ifdef __APPLE__
    #include <cstddef>
    #include "TargetConditionals.h"
    #ifdef TARGET_OS_MAC
        #define HUGGLE_MACX true
        #define HUGGLE_NO_MT_GC
    #endif
#endif
// this is a nasty hack that will disable multi threaded gc on MacOS as we had some report that
// it has problems there (need to be fixed though)
#ifndef HUGGLE_NO_MT_GC
    #define HUGGLE_USE_MT_GC               "mt"
#endif

// #define HUGGLE_PROFILING

// uncomment this if you want to enable python support
#ifndef PYTHONENGINE
//    #define PYTHONENGINE
#endif

// Uncomment this in order to disable breakpad, this is useful when you are having troubles
// linking or building its libraries
#define DISABLE_BREAKPAD

// this is a nasty workaround that exist because python is written by noobs
#ifdef PYTHONENGINE
  #ifdef _WIN32
  // workaround for http://bugs.python.org/issue11566
  // remove at least 8 months after the bug is fixed
  #include <cmath>
  #endif
#endif

#define HUGGLE_SUCCESS                     1
#define PRODUCTION_BUILD                   0
#define MEDIAWIKI_DEFAULT_NS_MAIN               ""
#define MEDIAWIKI_DEFAULT_NS_TALK               "Talk:"
#define MEDIAWIKI_DEFAULT_NS_USER               "User:"
#define MEDIAWIKI_DEFAULT_NS_USERTALK           "User talk:"
#define MEDIAWIKI_DEFAULT_NS_PROJECT            "Project:"
#define MEDIAWIKI_DEFAULT_NS_PROJECTTALK        "Project talk:"
#define MEDIAWIKI_DEFAULT_NS_FILE               "File:"
#define MEDIAWIKI_DEFAULT_NS_FILETALK           "File talk:"
#define MEDIAWIKI_DEFAULT_NS_MEDIAWIKI          "Mediawiki:"
#define MEDIAWIKI_DEFAULT_NS_MEDIAWIKITALK      "Mediawiki talk:"
#define MEDIAWIKI_DEFAULT_NS_TEMPLATE           "Template:"
#define MEDIAWIKI_DEFAULT_NS_TEMPLATETALK       "Template talk:"
#define MEDIAWIKI_DEFAULT_NS_HELP               "Help:"
#define MEDIAWIKI_DEFAULT_NS_HELPTALK           "Help talk:"
#define MEDIAWIKI_DEFAULT_NS_CATEGORY           "Category:"
#define MEDIAWIKI_DEFAULT_NS_CATEGORYTALK       "Category talk:"
#define MEDIAWIKI_DEFAULT_NS_PORTAL             "Portal:"
#define MEDIAWIKI_DEFAULT_NS_PORTALTALK         "Portal talk:"
#define MEDIAWIKI_NSID_MAIN                     0
#define MEDIAWIKI_NSID_TALK                     1
#define MEDIAWIKI_NSID_USER                     2
#define MEDIAWIKI_NSID_USERTALK                 3
#define MEDIAWIKI_NSID_PROJECT                  4
#define MEDIAWIKI_NSID_PROJECTTALK              5
#define MEDIAWIKI_NSID_FILE                     6
#define MEDIAWIKI_NSID_FILETALK                 7
#define MEDIAWIKI_NSID_MEDIAWIKI                8
#define MEDIAWIKI_NSID_MEDIAWIKITALK            9
#define MEDIAWIKI_NSID_TEMPLATE                 10
#define MEDIAWIKI_NSID_TEMPLATETALK             11
#define MEDIAWIKI_NSID_HELP                     12
#define MEDIAWIKI_NSID_HELPTALK                 13
#define MEDIAWIKI_NSID_CATEGORY                 14
#define MEDIAWIKI_NSID_CATEGORYTALK             15
#define MEDIAWIKI_NSID_PORTAL                   100
#define MEDIAWIKI_NSID_PORTALTALK               101
//! Minimal score the edit can have
#define MINIMAL_SCORE                   -999999
#define HUGGLE_CONF                     "huggle3.xml"
//! Path where the extensions are located
#define EXTENSION_PATH                  "extensions"
//! Value that is used by default for timers that are used on various places
//! lower this is, more your CPU will work but faster the huggle will be
#ifndef HUGGLE_TIMER
    #define HUGGLE_TIMER                   200
#endif
//! Change this to DEBIAN / UBUNTU / WINDOWS to get automatic updates for selected channels
#ifdef __linux__
    #define HUGGLE_UPDATER_PLATFORM_TYPE            "linux"
    #define HUGGLE_GLOBAL_EXTENSION_PATH            "/usr/share/huggle/extensions"
#elif _WIN32
    #define HUGGLE_UPDATER_PLATFORM_TYPE            "windows"
#elif defined HUGGLE_MACX
    #define HUGGLE_UPDATER_PLATFORM_TYPE            "mac"
#else
    #define HUGGLE_UPDATER_PLATFORM_TYPE            "unknown"
#endif
//! Revid of edit that doesn't exist
#define WIKI_UNKNOWN_REVID -1

#endif // CONFIG_H
