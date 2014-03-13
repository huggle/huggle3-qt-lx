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

#ifndef CONFIG_H
#define CONFIG_H

#define HUGGLE_VERSION                  "3.0.0.0"
#define HUGGLE_BYTE_VERSION             0x3000

// uncomment this if you want to enable python support
//#define PYTHONENGINE

// Uncomment this in order to disable breakpad, this is useful when you are having troubles
// linking or building its libraries
//#define DISABLE_BREAKPAD

#define PRODUCTION_BUILD                0
//! Minimal score the edit can have
#define MINIMAL_SCORE                   -999999
//! Path where the extensions are located
#define EXTENSION_PATH                  "extensions"
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
//! Change this to DEBIAN / UBUNTU / WINDOWS to get automatic updates for selected channels
#define HUGGLE_UPDATER_PLATFORM_TYPE            "unknown"
//! Revid of edit that doesn't exist
#define WIKI_UNKNOWN_REVID -1

#endif // CONFIG_H
