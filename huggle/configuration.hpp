//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef CONFIGURATION_H
#define CONFIGURATION_H
// Include file with all global defines
#include "config.hpp"
// now we need to ensure that python is included first, because it
// simply suck :P
#ifdef PYTHONENGINE
#include <Python.h>
#endif

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

#include <QList>
#include <QStringList>
#include <QHash>
#include <QDir>
#include <QtXml>
#include <QString>
#include "hugglequeuefilter.hpp"
#include "syslog.hpp"
#include "huggleparser.hpp"
#include "localization.hpp"
#include "wikipage.hpp"
#include "wikisite.hpp"

//! Huggle namespace contains all objects that belongs to huggle only so that they don't colide with other objects
namespace Huggle
{
    //! This enum defines what action should be done when revert etc
    enum Configuration_OnNext
    {
        //! Display same edit
        Configuration_OnNext_Stay,
        //! Display next
        Configuration_OnNext_Next,
        //! Display revert
        Configuration_OnNext_Revert
    };

    enum Headings
    {
        HeadingsStandard,
        HeadingsPageName,
        HeadingsNone
    };

    enum OptionType
    {
        OptionType_String,
        OptionType_List,
        OptionType_Dictionary
    };

    class WikiSite;
    class HuggleQueueFilter;
    class HuggleQueue;
    class WikiPage;
    class Syslog;
    class HuggleQueueParser;
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

    /*!
     * \brief The Option class is one option that is parsed from configuration no matter of its type
     */
    class Option
    {
        public:
            Option();
            Option(QString name, OptionType type);
            Option(Option *option);
            Option(const Option &option);
            QString Name;
            OptionType Type;
        private:
            QString ContainerString;
    };

    //! Run time configuration of huggle
    class Configuration
    {
        public:
            static QString GetLocalizationDataPath();
            static QString GetExtensionsRootPath();
            //! Return a prefix for url
            static QString GetURLProtocolPrefix();
            //! Return a configuration path
            static QString GetConfigurationPath();
            static QString Bool2ExcludeRequire(bool b);
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
             * \brief GetDefaultRevertSummary Retrieve default summary
             * \param source User who should be replaced instead of $1
             * \return Default revert summary
             */
            static QString GetDefaultRevertSummary(QString source);
            static Configuration *HuggleConfiguration;

            Configuration();
            ~Configuration();
            /*!
             * \brief Returns a data for given key
             * \param key
             * \return New instance of data or NULL in case there is no such an option
             */
            Option *GetOption(QString key);
            QString GenerateSuffix(QString text);
            ////////////////////////////////////////////
            // System
            ////////////////////////////////////////////

            //! Verbosity for debugging to terminal etc, can be switched with parameter --verbosity
            unsigned int    Verbosity;
            //! Version
            QString         HuggleVersion;
            //! currently selected project
            WikiSite        *Project;
            //! List of projects
            QList<WikiSite *> ProjectList;
            //! When this is true most of functions will not work
            bool            Restricted;
            //! Where the welcome message is stored
            QString         WelcomeMP;
            //! Maximum number of queue stuff
            int             SystemConfig_QueueSize;
            //! Whether python is available
            bool            PythonEngine;
            //! Size of feed
            int             SystemConfig_ProviderCache;
            //! Maximum size of ringlog
            int             RingLogMaxSize;
            //! Path where huggle contains its data
            QString         HomePath;
            //! Path to a file where information about wikis are stored
            QString         WikiDB;
            //! Data of wl (list of users)
            QStringList     WhiteList;
            //! URL of wiki that contains a global config
            QString         GlobalConfigurationWikiAddress;
            //! Number of seconds for which the processed queries remain in list of processes
            int             QueryListTimeLimit;
            //! Number of edits to keep in history stack
            int             HistorySize;
            //! Ask user if they really want to report someone
            bool            AskUserBeforeReport;
            //! This is experimental feature that removes the old templates from talk pages when they are being read
            bool            TrimOldWarnings;
            //! User flags on current project, this may be empty if you fail to login
            QStringList     Rights;
            //! Whether new edits go to top or bottom (if true, they go to up)
            bool            QueueNewEditsUp;
            //! If this is true some functionalities will be disabled
            bool            _SafeMode;
            //! Resolve edit conflict without asking user
            bool            AutomaticallyResolveConflicts;
            /// \todo This option needs to be implemented to browser so that font size is different when this is changed by user
            //! Size of fonts in diff
            int             FontSize;
            //! Timeout for queries
            int             ReadTimeout;
            //! Timeout for write / update queries
            int             SystemConfig_WriteTimeout;
            //! Whitelist is not useable
            bool            SystemConfig_WhitelistDisabled;
            //! If this is true huggle will always use software rollback even if user has the rollback privileges
            bool            EnforceManualSoftwareRollback;
            //! List of characters that separate words from each other, like dot, space etc, used by score words
            QStringList     SystemConfig_WordSeparators;
            //! Huggle will auto revert all edits that were made by same user on auto conflict resolution
            bool            RevertOnMultipleEdits;
            //! Changing this to true will make the Syslog write all data to a file
            bool            SystemConfig_Log2File;
            //! This path is used when Log2File is true to write the logs to
            QString         SystemConfig_SyslogPath;
            //! Whether huggle check for an update on startup
            bool            SystemConfig_UpdatesEnabled;
            bool            SystemConfig_LanguageSanity;
            //! This is a size of cache used by HAN to keep data about other user messages

            //! HAN need this so that changes that are first announced on there, but parsed from slower
            //! mediawiki later, can be synced. If this cache is too low, some actions reported on HAN
            //! may be lost and never applied on actual edits, because these are parsed later
            int             SystemConfig_CacheHAN;
            //! Debug mode
            bool            SystemConfig_Dot;
            //! This is index for login form so that we can remember which was last wiki user logged to

            //! We are storing index instead of wiki name, because in case it was a wiki that later
            //! was removed from the list, we would have nonexistent wiki in list
            int             IndexOfLastWiki;
            QString         SystemConfig_EditToken;

            //////////////////////////////////////////////
            // User
            //////////////////////////////////////////////
            bool            UserConfig_EnforceMonthsAsHeaders;
            //! If history and user info should be automatically loaded for every edit
            bool            UserConfig_HistoryLoad;
            //! Defines what should be done on next edit
            Configuration_OnNext    UserConfig_GoNext;
            bool                    UserConfig_DeleteEditsAfterRevert;
            //! Fetch only the last edit of page, that means if there is a newer edit
            //! it get automatically loaded instead of cached version
            bool                    UserConfig_LastEdit;
            bool                    UserConfig_SectionKeep;
            unsigned int            UserConfig_HistoryMax;
            bool                    UserConfig_TruncateEdits;
            bool                    UserConfig_RevertNewBySame;

            //////////////////////////////////////////////
            // Local config
            //////////////////////////////////////////////

            //! Minimal version of huggle required to use it
            QString LocalConfig_MinimalVersion;
            bool    LocalConfig_UseIrc;
            bool    LocalConfig_RequireRollback;
            bool    LocalConfig_RequireConfig;
            bool    LocalConfig_RequireAdmin;
            bool    LocalConfig_RequireAutoconfirmed;
            bool    LocalConfig_EnableAll;
            int     LocalConfig_RequireEdits;

            bool            LocalConfig_AIV;
            bool            LocalConfig_AIVExtend;
            QString         LocalConfig_ReportPath;
            //! Section of report page to append template to
            int             LocalConfig_ReportSt;
            //! IP vandals
            QString         LocalConfig_IPVTemplateReport;
            //! Regular users
            QString         LocalConfig_RUTemplateReport;
            QString         LocalConfig_WelcomeSummary;
            QString         LocalConfig_NSTalk;
            QString         LocalConfig_NSUserTalk;
            QString         LocalConfig_NSProject;
            QString         LocalConfig_NSUser;
            QString         LocalConfig_NSProjectTalk;
            QString         LocalConfig_NSFile;
            QString         LocalConfig_NSFileTalk;
            QString         LocalConfig_NSMediaWiki;
            QString         LocalConfig_NSMediaWikiTalk;
            QString         LocalConfig_NSTemplate;
            QString         LocalConfig_NSTemplateTalk;
            QString         LocalConfig_NSHelp;
            QString         LocalConfig_NSHelpTalk;
            QString         LocalConfig_NSCategory;
            QString         LocalConfig_NSCategoryTalk;
            QString         LocalConfig_NSPortal;
            QString         LocalConfig_NSPortalTalk;
            Headings        LocalConfig_Headings;
            int             LocalConfig_TemplateAge;
            bool            LocalConfig_ConfirmTalk;
            bool            LocalConfig_ConfirmWL;
            bool            LocalConfig_ConfirmOnSelfRevs;
            bool            LocalConfig_ConfirmMultipleEdits;
            bool            LocalConfig_ConfirmRange;
            bool            LocalConfig_ConfirmPage;
            bool            LocalConfig_ConfirmSame;
            bool            LocalConfig_ConfirmWarned;
            bool            LocalConfig_Patrolling;

            // Reverting
            QString         LocalConfig_MultipleRevertSummary;
            QStringList     LocalConfig_RevertSummaries;
            QString         LocalConfig_SoftwareRevertDefaultSummary;
            QString         LocalConfig_RollbackSummary;
            QString         LocalConfig_RollbackSummaryUnknownTarget;
            QString         LocalConfig_DefaultSummary;
            QString         LocalConfig_SingleRevert;
            QString         LocalConfig_UndoSummary;
            QString         LocalConfig_ClearTalkPageTemp;
            QString         LocalConfig_WelcomeAnon;
            QString         LocalConfig_WelcomeTitle;

            // Deleting
            QString         LocalConfig_DeletionTitle;
            QStringList     LocalConfig_DeletionSummaries;
            QString         LocalConfig_AssociatedDelete;

            // Warnings
            QString         LocalConfig_AgfRevert;
            QString         LocalConfig_WarnSummary;
            QString         LocalConfig_WarnSummary2;
            QString         LocalConfig_WarnSummary3;
            QString         LocalConfig_WarnSummary4;
            QStringList     LocalConfig_WarningTemplates;
            QStringList     LocalConfig_WarningDefs;
            QString         LocalConfig_ReportSummary;
            QString         LocalConfig_RestoreSummary;
            bool            LocalConfig_WelcomeGood;

            // Blocking users
            QStringList     LocalConfig_BlockExpiryOptions;
            QString         LocalConfig_BlockTime;
            QString         LocalConfig_BlockTimeAnon;
            QString         LocalConfig_BlockMessage;
            QString         LocalConfig_BlockMessageIndef;
            QString         LocalConfig_BlockReason;
            QString         LocalConfig_BlockSummary;

            // Protecting pages
            QString         LocalConfig_ProtectReason;

            // Templates
            QString         LocalConfig_SharedIPTemplateTags;
            QString         LocalConfig_SharedIPTemplate;

            // Definitions
            QList<ScoreWord> LocalConfig_ScoreParts;
            QList<ScoreWord> LocalConfig_ScoreWords;
            int              LocalConfig_ScoreFlag;
            int              LocalConfig_ForeignUser;
            int              LocalConfig_ScoreTalk;
            //! Score that is added for every edit that has really big size
            int              LocalConfig_ScoreChange;
            int              LocalConfig_ScoreUser;
            QStringList      LocalConfig_Ignores;
            QStringList      LocalConfig_RevertPatterns;
            QStringList      LocalConfig_Assisted;
            QStringList      LocalConfig_Templates;
            QStringList      LocalConfig_IgnorePatterns;
            int              LocalConfig_TalkPageWarningScore;
            bool             LocalConfig_GlobalRequired;
            // This is internal only do not prefix it!!
            QList<QRegExp>   RevertPatterns;

            int              LocalConfig_BotScore;
            int              LocalConfig_IPScore;
            int              LocalConfig_WarningScore;
            QStringList      LocalConfig_WarningTypes;
            QStringList      LocalConfig_SpeedyTemplates;
            QStringList      LocalConfig_WelcomeTypes;
            int              LocalConfig_WhitelistScore;

            // UAA
            QString          LocalConfig_UAAPath;
            bool             LocalConfig_UAAavailable;
            QString          LocalConfig_UAATemplate;


            //////////////////////////////////////////////
            // Global config
            //////////////////////////////////////////////

            bool        GlobalConfig_EnableAll;
            QString     GlobalConfig_MinVersion;
            QString     GlobalConfig_LocalConfigWikiPath;
            QString     GlobalConfig_DocumentationPath;
            QString     GlobalConfig_FeedbackPath;
            QString     GlobalConfig_UserConf;
            QString     GlobalConfig_UserConf_old;
            bool        GlobalConfigWasLoaded;

            //////////////////////////////////////////////
            // Login
            //////////////////////////////////////////////

            //! User name
            QString     UserName;
            //! If SSL is being used
            bool        UsingSSL;
            //! Consumer key
            QString     WmfOAuthConsumerKey;
            //! Password
            QString     Password;

            //////////////////////////////////////////////
            // IRC
            //////////////////////////////////////////////

            //! Whether IRC is being used
            bool    UsingIRC;
            //! Server
            QString IRCServer;
            //! Nick
            QString IRCNick;
            //! Ident
            QString IRCIdent;
            //! Port
            int     IRCPort;
            int     IRCConnectionTimeOut;

            //////////////////////////////////////////////
            // Friends
            //////////////////////////////////////////////

            //! Suffix used by huggle
            QString     LocalConfig_EditSuffixOfHuggle;
            //! Regexes that other tools can be identified with
            QStringList LocalConfig_EditRegexOfTools;

            //////////////////////////////////////////////
            // Reverting
            //////////////////////////////////////////////

            //! Warn you in case you want to revert a user page
            bool        WarnUserSpaceRoll;
            QStringList Months;
            //! Send a message to user on good edit
            bool        WelcomeEmpty;
            //! This is changed to true in case that someone send a message to user
            bool        NewMessage;
            QString     VandalNw_Server;
            QString     VandalNw_Ident;
            bool        VandalNw_Login;
            //! Pointer to AIV page
            WikiPage    *AIVP;
            //! Pointer to UAA page
            WikiPage    *UAAP;
            //! Operating system that is sent to update server
            QString     Platform;
        private:
            QHash       <QString, Option> Options;
    };
}

#endif // CONFIGURATION_H
