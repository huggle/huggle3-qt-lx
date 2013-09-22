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

class ScoreWord
{
public:
    ScoreWord(QString Word, int Score);
    QString word;
    int score;
};

class Configuration
{
public:
    ////////////////////////////////////////////
    // System
    ////////////////////////////////////////////
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
    static int ProviderCache;
    static int RingLogMaxSize;
    //! Path where huggle contains its data
    static QString HomePath;
    static QString WikiDB;
    static QString GetConfigurationPath();
    static QStringList WhiteList;
    static QString GlobalConfigurationWikiAddress;
    static bool TrackInfoApiQ;
    //! Number of seconds for which the processed queries remain in list of processes
    static int QueryListTimeLimit;
    //! Number of edits to keep in history stack
    static int HistorySize;

    //////////////////////////////////////////////
    // Local config
    //////////////////////////////////////////////

    static QString LocalConfig_MinimalVersion;
    static bool LocalConfig_UseIrc;
    static bool LocalConfig_RequireRollback;
    static bool LocalConfig_RequireAdmin;
    static bool LocalConfig_EnableAll;
    static int LocalConfig_RequireEdits;

    // Reverting

    static QString LocalConfig_ManualRevertSummary;
    static QString LocalConfig_MultipleRevertSummary;
    static QStringList LocalConfig_RevertSummaries;
    static QStringList LocalConfig_TemplateSummary;
    static bool LocalConfig_RollbackPossible;
    static QString LocalConfig_RollbackSummary;
    static QString LocalConfig_DefaultSummary;
    static QString LocalConfig_SingleRevert;
    static QString LocalConfig_UndoSummary;

    // Warnings

    static QString LocalConfig_AgfRevert;
    static QString LocalConfig_WarnSummary;
    static QString LocalConfig_WarnSummary2;
    static QString LocalConfig_WarnSummary3;
    static QString LocalConfig_WarnSummary4;
    static QStringList LocalConfig_WarningTemplates;
    static QStringList LocalConfig_WarningDefs;

    // Definitions

    static QList<ScoreWord> LocalConfig_ScoreParts;
    static QList<ScoreWord> LocalConfig_ScoreWords;
    static int LocalConfig_ScoreFlag;
    static QStringList LocalConfig_Ignores;
    static QStringList LocalConfig_RevertPatterns;
    static QStringList LocalConfig_Assisted;
    static QStringList LocalConfig_Templates;
    static QStringList LocalConfig_IgnorePatterns;
    static int LocalConfig_TalkPageWarningScore;

    static int LocalConfig_BotScore;
    static int LocalConfig_IPScore;
    static int LocalConfig_WarningScore;
    static QStringList LocalConfig_WarningTypes;
    static QStringList LocalConfig_WelcomeTypes;

    //////////////////////////////////////////////
    // Global config
    //////////////////////////////////////////////
    static bool GlobalConfig_EnableAll;
    static QString GlobalConfig_MinVersion;
    static QString GlobalConfig_LocalConfigWikiPath;
    static QString GlobalConfig_DocumentationPath;
    static QString GlobalConfig_FeedbackPath;
    static QString GlobalConfig_UserConf;

    //////////////////////////////////////////////
    // Login
    //////////////////////////////////////////////

    //! User name
    static QString UserName;
    //! If SSL is being used
    static bool UsingSSL;
    //! Consumer key
    static QString WmfOAuthConsumerKey;
    //! Password
    static QString Password;

    //////////////////////////////////////////////
    // IRC
    //////////////////////////////////////////////

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

    //////////////////////////////////////////////
    // Friends
    //////////////////////////////////////////////
    //! Suffix used by huggle
    static QString EditSuffixOfHuggle;
    //! Regexes that other tools can be identified with
    static QStringList EditRegexOfTools;

    //////////////////////////////////////////////
    // Reverting
    //////////////////////////////////////////////

    static QString DefaultRevertSummary;
    static QString GetDefaultRevertSummary(QString source);
    //! Warn you in case you want to revert a user page
    static bool WarnUserSpaceRoll;

};

#endif // CONFIGURATION_H
