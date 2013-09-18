//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

// comment this if you want to disable python support
//#define PYTHONENGINE

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QList>
#include <QStringList>
#include <QDir>
#include <QString>
#include "wikisite.h"

class Configuration
{
public:
    ///////////////////////////
    // System
    ///////////////////////////
    //! Verbosity for debugging to terminal etc, can be switched with parameter --verbosity
    static unsigned int Verbosity;
    //! Version
    static QString HuggleVersion;
    //! currently selected project
    static WikiSite Project;
    //! List of projects
    static QList<WikiSite> ProjectList;
    //! When this is true most of functions will not work
    static bool Restricted;
    //! Return a prefix for url
    static QString GetURLProtocolPrefix();
    //! Where the welcome message is stored
    static QString WelcomeMP;
    //! Size of info cache
    static int Cache_InfoSize;
    //! Whether python is available
    static bool PythonEngine;
    //! Size of feed
    static unsigned int ProviderCache;
    static unsigned int RingLogMaxSize;
    //! Path where huggle contains its data
    static QString HomePath;
    static QString WikiDB;
    static QString GetConfigurationPath();
    static QStringList WhiteList;
    static QString GlobalConfigurationWikiAddress;

    ///////////////////////////
    // Login
    ///////////////////////////

    //! User name
    static QString UserName;
    //! If SSL is being used
    static bool UsingSSL;
    //! Consumer key
    static QString WmfOAuthConsumerKey;
    //! Password
    static QString Password;

    /////////////////////////////
    // IRC
    /////////////////////////////

    //! Whether IRC is being used
    static bool UsingIRC;
    //! Server
    static QString IRCServer;
    //! Nick
    static QString IRCNick;
    //! Ident
    static QString IRCIdent;
    //! Port
    static quint16 IRCPort;
    Configuration();

    ////////////////////////////
    // Friends
    ////////////////////////////
    //! Suffix used by huggle
    static QString EditSuffixOfHuggle;
    //! Regexes that other tools can be identified with
    static QStringList EditRegexOfTools;

    ///////////////////////////////
    // Reverting
    ///////////////////////////////

    static QString DefaultRevertSummary;
    static QString GetDefaultRevertSummary(QString source);

};

#endif // CONFIGURATION_H
