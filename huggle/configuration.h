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
//! Minimal score the edit can have
#define MINIMAL_SCORE -999999
//! Path where the extensions are located
#define EXTENSION_PATH "extensions"
#define MEDIAWIKI_DEFAULT_NS_MAIN ""
#define MEDIAWIKI_DEFAULT_NS_TALK "Talk:"
#define MEDIAWIKI_DEFAULT_NS_PROJECT "Project:"
#define MEDIAWIKI_DEFAULT_NS_PROJECTTALK "Project talk:"
#define MEDIAWIKI_DEFAULT_NS_USER "User:"
#define MEDIAWIKI_DEFAULT_NS_USERTALK "User talk:"

#define MEDIAWIKI_NSID_MAIN 0
#define MEDIAWIKI_NSID_TALK 1
#define MEDIAWIKI_NSID_USER 2
#define MEDIAWIKI_NSID_USERTALK 3
#define MEDIAWIKI_NSID_PROJECT 4
#define MEDIAWIKI_NSID_PROJECTTALK 5
#define MEDIAWIKI_NSID_FILE 6
#define MEDIAWIKI_NSID_FILETALK 7
#define MEDIAWIKI_NSID_MEDIAWIKI 8
#define MEDIAWIKI_NSID_MEDIAWIKITALK 9

#include <QList>
#include <QStringList>
#include <QDir>
#include <QString>
#include "wikisite.h"

namespace Huggle
{
    /*!
     * \brief The ScoreWord class
     *
     * Every score word is represented by this class, a score word is a pattern
     * that has some score and score of edit is incremented by sum of all scores
     * of all score words matched in edit
     */
    class ScoreWord
    {
    public:
        ScoreWord(QString Word, int Score);
        QString word;
        int score;
    };

    //! Run time configuration of huggle
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
        static QList<WikiSite *> ProjectList;
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
        //! Maximum size of ringlog
        static int RingLogMaxSize;
        //! Path where huggle contains its data
        static QString HomePath;
        //! Path to a file where information about wikis are stored
        static QString WikiDB;
        //! Return a configuration path
        static QString GetConfigurationPath();
        //! Data of wl
        static QStringList WhiteList;
        //! URL of wiki that contains a global config
        static QString GlobalConfigurationWikiAddress;
        //! Number of seconds for which the processed queries remain in list of processes
        static int QueryListTimeLimit;
        //! Number of edits to keep in history stack
        static int HistorySize;
        //! Language selected by user this is only a language of interface
        static QString Language;
        //! Number of edits made since you logged in
        static double EditCounter;
        //! Number of reverts made since you logged in
        static double RvCounter;
        //! Ask user if they really want to report someone
        static bool AskUserBeforeReport;
        //! This is experimental feature that removes the old templates from talk pages when they are being read
        static bool TrimOldWarnings;
        static QStringList Rights;
        //! Whether new edits go to top or bottom
        static bool QueueNewEditsUp;
        //! If this is true some functionalities will be disabled
        static bool _SafeMode;

        //Layout

        static QByteArray Geometry;
        static QByteArray Position;

        //////////////////////////////////////////////
        // Local config
        //////////////////////////////////////////////

        //! Minimal version of huggle required to use it
        static QString LocalConfig_MinimalVersion;
        static bool LocalConfig_UseIrc;
        static bool LocalConfig_RequireRollback;
        static bool LocalConfig_RequireAdmin;
        static bool LocalConfig_EnableAll;
        static int LocalConfig_RequireEdits;

        static bool LocalConfig_AIV;
        static bool LocalConfig_AIVExtend;
        static QString LocalConfig_ReportPath;
        //! Section of report page to append template to
        static int LocalConfig_ReportSt;
        //! IP vandals
        static QString LocalConfig_IPVTemplateReport;
        //! Regular users
        static QString LocalConfig_RUTemplateReport;
        static QString LocalConfig_WelcomeSummary;
        static QString LocalConfig_NSTalk;
        static QString LocalConfig_NSUserTalk;
        static QString LocalConfig_NSProject;
        static QString LocalConfig_NSUser;
        static QString LocalConfig_NSProjectTalk;
        static int LocalConfig_TemplateAge;

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
        static QString LocalConfig_ClearTalkPageTemp;
        static QString LocalConfig_WelcomeAnon;
        static QString LocalConfig_WelcomeTitle;

        // Warnings

        static QString LocalConfig_AgfRevert;
        static QString LocalConfig_WarnSummary;
        static QString LocalConfig_WarnSummary2;
        static QString LocalConfig_WarnSummary3;
        static QString LocalConfig_WarnSummary4;
        static QStringList LocalConfig_WarningTemplates;
        static QStringList LocalConfig_WarningDefs;
        static QString LocalConfig_ReportSummary;

        // Blocking users
        static QStringList LocalConfig_BlockExpiryOptions;
        static QString LocalConfig_BlockTime;
        static QString LocalConfig_BlockTimeAnon;
        static QString LocalConfig_BlockMessage;
        static QString LocalConfig_BlockMessageIndef;
        static QString LocalConfig_BlockReason;
        static QString LocalConfig_BlockSummary;

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
        static bool LocalConfig_GlobalRequired;

        static int LocalConfig_BotScore;
        static int LocalConfig_IPScore;
        static int LocalConfig_WarningScore;
        static QStringList LocalConfig_WarningTypes;
        static QStringList LocalConfig_DeletionTemplates;
        static QStringList LocalConfig_WelcomeTypes;
        static int LocalConfig_WhitelistScore;

        //////////////////////////////////////////////
        // Global config
        //////////////////////////////////////////////

        static bool GlobalConfig_EnableAll;
        static QString GlobalConfig_MinVersion;
        static QString GlobalConfig_LocalConfigWikiPath;
        static QString GlobalConfig_DocumentationPath;
        static QString GlobalConfig_FeedbackPath;
        static QString GlobalConfig_UserConf;
        static bool GlobalConfigWasLoaded;

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
        /*!
         * \brief GetDefaultRevertSummary Retrieve default summary
         * \param source User who should be replaced instead of $1
         * \return Default revert summary
         */
        static QString GetDefaultRevertSummary(QString source);
        //! Warn you in case you want to revert a user page
        static bool WarnUserSpaceRoll;
        static bool NextOnRv;
        //! Send a message to user on good edit
        static bool WelcomeEmpty;
        /*!
         * \brief Bool2String Convert a bool to string
         * \param b bool
         * \return string
         */
        static QString Bool2String(bool b);
    };
}

#endif // CONFIGURATION_H
