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
#define PRODUCTION_BUILD        0
//! Minimal score the edit can have
#define MINIMAL_SCORE           -999999
#define HUGGLE_VERSION          "3.0.0.0"
#define HUGGLE_BYTE_VERSION     0x3000
//! Path where the extensions are located
#define EXTENSION_PATH          "extensions"
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
#define MEDIAWIKI_NSID_MAIN             0
#define MEDIAWIKI_NSID_TALK             1
#define MEDIAWIKI_NSID_USER             2
#define MEDIAWIKI_NSID_USERTALK         3
#define MEDIAWIKI_NSID_PROJECT          4
#define MEDIAWIKI_NSID_PROJECTTALK      5
#define MEDIAWIKI_NSID_FILE             6
#define MEDIAWIKI_NSID_FILETALK         7
#define MEDIAWIKI_NSID_MEDIAWIKI        8
#define MEDIAWIKI_NSID_MEDIAWIKITALK    9
#define MEDIAWIKI_NSID_TEMPLATE         10
#define MEDIAWIKI_NSID_TEMPLATETALK     11
#define MEDIAWIKI_NSID_HELP             12
#define MEDIAWIKI_NSID_HELPTALK         13
#define MEDIAWIKI_NSID_CATEGORY         14
#define MEDIAWIKI_NSID_CATEGORYTALK     15
#define MEDIAWIKI_NSID_PORTAL           100

#include <QList>
#include <QStringList>
#include <QDir>
#include <QtXml>
#include <QString>
#include "core.hpp"
#include "wikisite.hpp"

//! Huggle namespace contains all objects that belongs to huggle only so that they don't colide with other objects
namespace Huggle
{
    class Core;
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
            ScoreWord(ScoreWord *word);
            ScoreWord(const ScoreWord &word);
            QString word;
            int score;
    };

    //! Run time configuration of huggle
    class Configuration
    {
        public:
            Configuration();
            static Configuration *HuggleConfiguration;
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
            //! Resolve edit conflict without asking user
            static bool AutomaticallyResolveConflicts;
            //! Size of fonts in diff
            static int FontSize;
            //! Timeout for queries
            static int ReadTimeout;
            //! Timeout for write / update queries
            static int WriteTimeout;
            //! Whitelist is not useable
            static bool WhitelistDisabled;
            //! If this is true huggle will always use software rollback even if user has the rollback privileges
            static bool EnforceManualSoftwareRollback;
            static QStringList Separators;
            //! Huggle will auto revert all edits that were made by same user on auto conflict resolution
            static bool RevertOnMultipleEdits;

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
            static QString LocalConfig_NSFile;
            static QString LocalConfig_NSFileTalk;
            static QString LocalConfig_NSMediaWiki;
            static QString LocalConfig_NSMediaWikiTalk;
            static QString LocalConfig_NSTemplate;
            static QString LocalConfig_NSTemplateTalk;
            static QString LocalConfig_NSHelp;
            static QString LocalConfig_NSHelpTalk;
            static QString LocalConfig_NSCategory;
            static QString LocalConfig_NSCategoryTalk;
            static QString LocalConfig_NSPortal;
            static QString LocalConfig_NSPortalTalk;
            static int LocalConfig_TemplateAge;
            static bool LocalConfig_ConfirmMultipleEdits;
            static bool LocalConfig_ConfirmRange;
            static bool LocalConfig_ConfirmPage;
            static bool LocalConfig_ConfirmSame;
            static bool LocalConfig_ConfirmWarned;

            // Reverting

            static QString LocalConfig_MultipleRevertSummary;
            static QStringList LocalConfig_RevertSummaries;
            static QString LocalConfig_SoftwareRevertDefaultSummary;
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
            static bool LocalConfig_WelcomeGood;

            // Blocking users
            static QStringList LocalConfig_BlockExpiryOptions;
            static QString LocalConfig_BlockTime;
            static QString LocalConfig_BlockTimeAnon;
            static QString LocalConfig_BlockMessage;
            static QString LocalConfig_BlockMessageIndef;
            static QString LocalConfig_BlockReason;
            static QString LocalConfig_BlockSummary;

            // Protecting pages
            static QString LocalConfig_ProtectReason;

            // Definitions

            static QList<ScoreWord> LocalConfig_ScoreParts;
            static QList<ScoreWord> LocalConfig_ScoreWords;
            static int LocalConfig_ScoreFlag;
            //! Score that is added for every edit that has really big size
            static int LocalConfig_ScoreChange;
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

            // UAA
            static QString LocalConfig_UAAPath;
            static bool LocalConfig_UAAavailable;
            static QString LocalConfig_UAATemplate;


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

            static QString VandalNw_Server;
            static QString VandalNw_Ident;
            static bool VandalNw_Login;


            /*!
             * \brief Bool2String Convert a bool to string
             * \param b bool
             * \return string
             */
            static QString Bool2String(bool b);

            //! Save the local configuration to file
            static void SaveConfig();
            //! Load the local configuration from disk
            static void LoadConfig();
            static void NormalizeConf();
            //! This function creates a user configuration that is stored on wiki
            static QString MakeLocalUserConfig();
            /*!
             * \brief InsertConfig
             * \param key Configuration key
             * \param value Value of key
             * \param s Stream writer
             */
            static void InsertConfig(QString key, QString value, QXmlStreamWriter *s);
            static bool SafeBool(QString value, bool defaultvalue = false);
            //! Parse all information from global config on meta
            static bool ParseGlobalConfig(QString config);
            //! Parse all information from local config, this function is used in login
            static bool ParseLocalConfig(QString config);
            static bool ParseUserConfig(QString config);
            //! Parse a string from configuration which has format used by huggle 2x
            /*!
             * \param key Key
             * \param content Text to parse from
             * \param missing Default value in case this key is missing in text
             * \return Value of key
             */
            static QString ConfigurationParse(QString key, QString content, QString missing = "");
            /*!
             * \brief ConfigurationParse_QL Parses a QStringList of values for a given key
             * \param key Key
             * \param content Text to parse key from
             * \param CS Whether the values are separated by comma
             * \return List of values from text or empty list
             */
            static QStringList ConfigurationParse_QL(QString key, QString content, bool CS = false);
            static QStringList ConfigurationParse_QL(QString key, QString content, QStringList list, bool CS = false);
    };
}

#endif // CONFIGURATION_H
