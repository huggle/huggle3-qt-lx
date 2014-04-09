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
#include "definitions.hpp"
// now we need to ensure that python is included first, because it
// simply suck :P
#ifdef PYTHONENGINE
#include <Python.h>
#endif

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

    //! Some interesting information regarding configuration:
    //! Huggle is using different configurations, they are divided to several sections which are described
    //! in table here

    //! System config:
    //! That is configuration which is related to selected computer, like fonts, sizes and layout of GUI
    //! this configuration is not stored on wiki, it's only in local configuration file

    //! Global config:
    //! Is configuration used for all wikimedia projects, stored on meta, it can't be overriden neither by users
    //! nor by project configs

    //! Project config:
    //! Is configuration local to projects, which can't be overriden by user config

    //! Shared config:
    //! Is configuration local to projects, which contains definitions for templates and so on, this can be
    //! overriden by user config

    //! User config:
    //! Maintained by user and stored in huggle3.css on wiki, this is only wiki-side configuration that is being
    //! updated directly by huggle

    //! Temporary config:
    //! Is maintained accross 1 huggle session
    class Configuration
    {
        public:
            //! Return a full url like http://en.wikipedia.org/wiki/
            static QString GetProjectWikiURL(WikiSite Project);
            //! Return a script url like http://en.wikipedia.org/w/
            static QString GetProjectScriptURL(WikiSite Project);
            //! Return a base url of current project
            static QString GetProjectURL();
            //! Return a full url like http://en.wikipedia.org/wiki/
            static QString GetProjectWikiURL();
            //! Return a script url like http://en.wikipedia.org/w/
            static QString GetProjectScriptURL();
            static QString GetProjectURL(WikiSite Project);
            static QString GetLocalizationDataPath();
            //! Extension path (typically HR/extensions) where .py and .so files are in
            static QString GetExtensionsRootPath();
            //! Return a prefix for url
            static QString GetURLProtocolPrefix();
            //! Returns full configuration path suffixed with slash
            static QString GetConfigurationPath();
            static QString ReplaceSpecialUserPage(QString PageName);
            static QString Bool2ExcludeRequire(bool b);
            /*!
             * \brief Bool2String Convert a bool to string
             * \param b bool
             * \return string
             */
            static QString Bool2String(bool b);
            //! Save the local configuration to file
            static void SaveSystemConfig();
            //! Load the local configuration from disk
            static void LoadSystemConfig(QString fn);
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
            //! Parse a string from configuration which has format used by huggle 2x
            /*!
             * \param key Key
             * \param content Text to parse from
             * \param missing Default value in case this key is missing in text
             * \return Value of key, in case there is no such a key content of missing is returned
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
            void NormalizeConf();
            QString GenerateSuffix(QString text);
            //! Parse all information from global config on meta
            bool ParseGlobalConfig(QString config);
            //! Parse all information from local config, this function is used in login
            bool ParseProjectConfig(QString config);
            bool ParseUserConfig(QString config);
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
            int             SystemConfig_RingLogMaxSize;
            //! Path where huggle contains its data
            QString         HomePath;
            //! Path to a file where information about wikis are stored
            QString         WikiDB;
            //! Data of wl (list of users)
            QStringList     WhiteList;
            //! URL of wiki that contains a global config
            QString         GlobalConfigurationWikiAddress;
            //! Number of seconds for which the processed queries remain in list of processes
            int             SystemConfig_QueryListTimeLimit;
            //! Number of edits to keep in history stack
            int             SystemConfig_HistorySize;
            //! Ask user if they really want to report someone
            bool            AskUserBeforeReport;
            //! This is experimental feature that removes the old templates from talk pages when they are being read
            bool            TrimOldWarnings;
            //! User flags on current project, this may be empty if you fail to login
            QStringList     Rights;
            //! Whether new edits go to top or bottom (if true, they go to up)
            bool            SystemConfig_QueueNewEditsUp;
            //! If this is true some functionalities will be disabled
            bool            SystemConfig_SafeMode;
            //! Resolve edit conflict without asking user
            bool            UserConfig_AutomaticallyResolveConflicts;
            /// \todo This option needs to be implemented to browser so that font size is different when this is changed by user
            //! Size of fonts in diff
            int             SystemConfig_FontSize;
            //! Timeout for queries
            int             SystemConfig_ReadTimeout;
            //! Timeout for write / update queries
            int             SystemConfig_WriteTimeout;
            //! Whitelist is not useable
            bool            SystemConfig_WhitelistDisabled;
            //! If this is true huggle will always use software rollback even if user has the rollback privileges
            bool            EnforceManualSoftwareRollback;
            //! List of characters that separate words from each other, like dot, space etc, used by score words
            QStringList     SystemConfig_WordSeparators;
            //! This is affecting if columns are auto-sized or not
            bool            SystemConfig_DynamicColsInList;
            //! Huggle will auto revert all edits that were made by same user on auto conflict resolution
            bool            RevertOnMultipleEdits;
            QString         SystemConfig_GlobalConfigWikiList;
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
            QString         TemporaryConfig_EditToken;

            //////////////////////////////////////////////
            // User
            //////////////////////////////////////////////
            bool                    UserConfig_EnforceMonthsAsHeaders;
            unsigned int            UserConfig_TalkPageFreshness;
            //! If history and user info should be automatically loaded for every edit
            bool                    UserConfig_HistoryLoad;
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
            //! If this is set to false the warning will be selected by huggle when user decide to
            //! use the "warn only" feature in huggle (W) for example, it doesn't affect reverting
            bool                    UserConfig_ManualWarning;
            //! Large title of every page in top of diff
            bool                    UserConfig_DisplayTitle;

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
            // Local config
            //////////////////////////////////////////////

            //! Minimal version of huggle required to use it
            QString         ProjectConfig_MinimalVersion;
            bool            ProjectConfig_UseIrc;
            //! If admin rights are required to use huggle
            bool            ProjectConfig_RequireAdmin;
            //! If autoconfirmed is required to use huggle
            bool            ProjectConfig_RequireAutoconfirmed;
            bool            ProjectConfig_RequireConfig;
            //! Amount of edits required to use huggle
            int             ProjectConfig_RequireEdits;
            //! If rollback right is required to use huggle
            bool            ProjectConfig_RequireRollback;
            bool            ProjectConfig_EnableAll;
            byte_ht         ProjectConfig_WarningLevel;
            bool            ProjectConfig_AIV;
            bool            ProjectConfig_AIVExtend;
            bool            ProjectConfig_RFPP;
            unsigned int    ProjectConfig_RFPP_Section;
            QString         ProjectConfig_RFPP_Template;
            QString         ProjectConfig_RFPP_Summary;
            QString         ProjectConfig_RFPP_Regex;
            QString         ProjectConfig_RFPP_Page;
            QString         ProjectConfig_ReportAIV;
            //! Section of report page to append template to
            int             ProjectConfig_ReportSt;
            //! IP vandals
            QString         ProjectConfig_IPVTemplateReport;
            //! Regular users
            QString         ProjectConfig_RUTemplateReport;
            QString         ProjectConfig_ReportDefaultReason;
            QString         ProjectConfig_WelcomeSummary;
            QString         ProjectConfig_NSTalk;
            QString         ProjectConfig_NSUserTalk;
            QString         ProjectConfig_NSProject;
            QString         ProjectConfig_NSUser;
            QString         ProjectConfig_NSProjectTalk;
            QString         ProjectConfig_NSFile;
            QString         ProjectConfig_NSFileTalk;
            QString         ProjectConfig_NSMediaWiki;
            QString         ProjectConfig_NSMediaWikiTalk;
            QString         ProjectConfig_NSTemplate;
            QString         ProjectConfig_NSTemplateTalk;
            QString         ProjectConfig_NSHelp;
            QString         ProjectConfig_NSHelpTalk;
            QString         ProjectConfig_NSCategory;
            QString         ProjectConfig_NSCategoryTalk;
            QString         ProjectConfig_NSPortal;
            QString         ProjectConfig_NSPortalTalk;
            Headings        ProjectConfig_Headings;
            int             ProjectConfig_TemplateAge;
            bool            ProjectConfig_ConfirmTalk;
            bool            ProjectConfig_ConfirmWL;
            bool            ProjectConfig_ConfirmOnSelfRevs;
            bool            ProjectConfig_ConfirmMultipleEdits;
            bool            ProjectConfig_ConfirmRange;
            bool            ProjectConfig_ConfirmPage;
            bool            ProjectConfig_ConfirmSame;
            bool            ProjectConfig_ConfirmWarned;
            bool            ProjectConfig_Patrolling;

            // Reverting
            QString         ProjectConfig_MultipleRevertSummary;
            QStringList     ProjectConfig_RevertSummaries;
            QString         ProjectConfig_SoftwareRevertDefaultSummary;
            QString         ProjectConfig_RollbackSummary;
            QString         ProjectConfig_RollbackSummaryUnknownTarget;
            QString         ProjectConfig_DefaultSummary;
            QString         ProjectConfig_SingleRevert;
            QString         ProjectConfig_UndoSummary;
            QString         ProjectConfig_ClearTalkPageTemp;
            QString         ProjectConfig_WelcomeAnon;
            QString         ProjectConfig_WelcomeTitle;

            // Deleting
            QString         ProjectConfig_DeletionTitle;
            QStringList     ProjectConfig_DeletionSummaries;
            QString         ProjectConfig_AssociatedDelete;

            // Warnings
            QString         ProjectConfig_AgfRevert;
            QString         ProjectConfig_WarnSummary;
            QString         ProjectConfig_WarnSummary2;
            QString         ProjectConfig_WarnSummary3;
            QString         ProjectConfig_WarnSummary4;
            QStringList     ProjectConfig_WarningTemplates;
            QStringList     ProjectConfig_WarningDefs;
            QString         ProjectConfig_ReportSummary;
            QString         ProjectConfig_RestoreSummary;
            bool            ProjectConfig_WelcomeGood;

            // Blocking users
            QStringList     ProjectConfig_BlockExpiryOptions;
            QString         ProjectConfig_BlockTime;
            QString         ProjectConfig_BlockTimeAnon;
            QString         ProjectConfig_BlockMessage;
            QString         ProjectConfig_BlockMessageIndef;
            QString         ProjectConfig_BlockReason;
            QString         ProjectConfig_BlockSummary;

            // Protecting pages
            QString         ProjectConfig_ProtectReason;

            // Templates
            QString         ProjectConfig_SharedIPTemplateTags;
            QString         ProjectConfig_SharedIPTemplate;

            // Definitions
            QList<ScoreWord> ProjectConfig_ScoreParts;
            QList<ScoreWord> ProjectConfig_ScoreWords;
            int              ProjectConfig_ScoreFlag;
            int              ProjectConfig_ForeignUser;
            int              ProjectConfig_ScoreTalk;
            //! Score that is added for every edit that has really big size
            int              ProjectConfig_ScoreChange;
            int              ProjectConfig_ScoreUser;
            QStringList      ProjectConfig_Ignores;
            QStringList      ProjectConfig_RevertPatterns;
            QStringList      ProjectConfig_Assisted;
            QStringList      ProjectConfig_Templates;
            QStringList      ProjectConfig_IgnorePatterns;
            int              ProjectConfig_TalkPageWarningScore;
            bool             ProjectConfig_GlobalRequired;
            // This is internal only do not prefix it!!
            QList<QRegExp>   RevertPatterns;

            int              ProjectConfig_BotScore;
            int              ProjectConfig_IPScore;
            int              ProjectConfig_WarningScore;
            QStringList      ProjectConfig_WarningTypes;
            QString          ProjectConfig_SpeedyEditSummary;
            QString          ProjectConfig_SpeedyWarningSummary;
            QStringList      ProjectConfig_SpeedyTemplates;
            QStringList      ProjectConfig_WelcomeTypes;
            int              ProjectConfig_WhitelistScore;

            // UAA
            QString          ProjectConfig_UAAPath;
            bool             ProjectConfig_UAAavailable;
            QString          ProjectConfig_UAATemplate;

            //////////////////////////////////////////////
            // Login
            //////////////////////////////////////////////

            //! User name
            QString     SystemConfig_Username;
            //! If SSL is being used
            bool        SystemConfig_UsingSSL;
            //! Consumer key
            QString     WmfOAuthConsumerKey;
            //! Password
            QString     TemporaryConfig_Password;

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
            int     SystemConfig_IRCConnectionTimeOut;

            //////////////////////////////////////////////
            // Friends
            //////////////////////////////////////////////

            //! Suffix used by huggle
            QString     ProjectConfig_EditSuffixOfHuggle;
            //! Regexes that other tools can be identified with
            QStringList ProjectConfig_EditRegexOfTools;

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
