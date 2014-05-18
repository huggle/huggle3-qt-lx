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
#include <QString>
#include "hugglequeuefilter.hpp"
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
            //! Helper function that will return URL of project in question
            /*!
             * \param Project Site
             * \return String with url
             */
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
            unsigned int    Verbosity = 0;
            //! Version
            QString         HuggleVersion;
            //! currently selected project
            WikiSite        *Project = NULL;
            //! List of projects
            QList<WikiSite *> ProjectList;
            //! When this is true most of functions will not work
            bool            Restricted = false;
            //! Where the welcome message is stored
            QString         WelcomeMP = "Project:Huggle/Message";
            //! This is used in combination with --login option, so that huggle knows if it should
            //! login automatically or wait for user to fill in their user information
            bool            Login = false;
            //! Maximum number of queue stuff
            int             SystemConfig_QueueSize = 200;
            //! Whether python is available
            bool            PythonEngine;
            //! Size of feed
            int             SystemConfig_ProviderCache = 200;
            //! Maximum size of ringlog
            int             SystemConfig_RingLogMaxSize = 2000;
            //! Path where huggle contains its data
            QString         HomePath;
            //! Path to a file where information about wikis are stored
            QString         WikiDB = "";
            //! Data of wl (list of users)
            QStringList     WhiteList;
            QStringList     NewWhitelist;
            //! URL of wiki that contains a global config
            QString         GlobalConfigurationWikiAddress = "meta.wikimedia.org/w/";
            //! Number of seconds for which the processed queries remain in list of processes
            int             SystemConfig_QueryListTimeLimit = 2;
            //! Number of edits to keep in history stack
            int             SystemConfig_HistorySize = 20;
            //! Ask user if they really want to report someone
            bool            AskUserBeforeReport = true;
            //! This is experimental feature that removes the old templates from talk pages when they are being read
            bool            TrimOldWarnings = true;
            //! User flags on current project, this may be empty if you fail to login
            QStringList     Rights;
            //! Whether new edits go to top or bottom (if true, they go to up)
            bool            SystemConfig_QueueNewEditsUp = false;
            //! If this is true some functionalities will be disabled
            bool            SystemConfig_SafeMode = false;
            //! Resolve edit conflict without asking user
            bool            UserConfig_AutomaticallyResolveConflicts = false;
            /// \todo This option needs to be implemented to browser so that font size is different when this is changed by user
            //! Size of fonts in diff
            int             SystemConfig_FontSize = 10;
            //! Timeout for queries
            int             SystemConfig_ReadTimeout = 60;
            //! Timeout for write / update queries
            int             SystemConfig_WriteTimeout = 200;
            //! Whitelist is not useable
            bool            SystemConfig_WhitelistDisabled = false;
            //! If this is true huggle will always use software rollback even if user has the rollback privileges
            bool            EnforceManualSoftwareRollback = false;
            //! List of characters that separate words from each other, like dot, space etc, used by score words
            QStringList     SystemConfig_WordSeparators;
            //! This is affecting if columns are auto-sized or not
            bool            SystemConfig_DynamicColsInList = false;
            //! Huggle will auto revert all edits that were made by same user on auto conflict resolution
            bool            RevertOnMultipleEdits = false;
            QString         SystemConfig_GlobalConfigWikiList = "Project:Huggle/List";
            //! Changing this to true will make the Syslog write all data to a file
            bool            SystemConfig_Log2File = false;
            //! This path is used when Log2File is true to write the logs to
            QString         SystemConfig_SyslogPath = "huggle.log";
            //! Whether huggle check for an update on startup
            bool            SystemConfig_UpdatesEnabled = true;
            bool            SystemConfig_LanguageSanity = false;
            bool            SystemConfig_RequestDelay = false;
            unsigned int    SystemConfig_DelayVal = 0;
            //! This is a size of cache used by HAN to keep data about other user messages

            //! HAN need this so that changes that are first announced on there, but parsed from slower
            //! mediawiki later, can be synced. If this cache is too low, some actions reported on HAN
            //! may be lost and never applied on actual edits, because these are parsed later
            int             SystemConfig_CacheHAN = 100;
            //! Debug mode
            bool            SystemConfig_Dot = false;
            bool            SystemConfig_InstantReverts = true;
            int             SystemConfig_RevertDelay = 0;
            //! This is index for login form so that we can remember which was last wiki user logged to

            //! We are storing index instead of wiki name, because in case it was a wiki that later
            //! was removed from the list, we would have nonexistent wiki in list
            int             IndexOfLastWiki = 0;
            QString         TemporaryConfig_EditToken = "";

            //////////////////////////////////////////////
            // User
            //////////////////////////////////////////////
            bool                    UserConfig_EnforceMonthsAsHeaders = true;
            unsigned int            UserConfig_TalkPageFreshness = 20;
            //! If history and user info should be automatically loaded for every edit
            bool                    UserConfig_HistoryLoad = true;
            //! Defines what should be done on next edit
            Configuration_OnNext    UserConfig_GoNext = Configuration_OnNext_Next;
            bool                    UserConfig_DeleteEditsAfterRevert = true;
            //! Fetch only the last edit of page, that means if there is a newer edit
            //! it get automatically loaded instead of cached version
            bool                    UserConfig_LastEdit = false;
            bool                    UserConfig_SectionKeep = true;
            unsigned int            UserConfig_HistoryMax = 50;
            bool                    UserConfig_TruncateEdits = false;
            bool                    UserConfig_RevertNewBySame = true;
            //! If this is set to false the warning will be selected by huggle when user decide to
            //! use the "warn only" feature in huggle (W) for example, it doesn't affect reverting
            bool                    UserConfig_ManualWarning = false;
            //! Large title of every page in top of diff
            bool                    UserConfig_DisplayTitle = false;
            //! Result of "Stop feed, Remove old edits" in main form
            bool                    UserConfig_RemoveOldQueueEdits = false;
            bool                    UserConfig_CheckTP = false;
            QString                 UserConfig_QueueID = "default";
            //! Display messages from users in vandal window
            bool                    UserConfig_HAN_DisplayUser = true;
            bool                    UserConfig_HAN_DisplayBots = true;
            bool                    UserConfig_HAN_DisplayUserTalk = true;

            //////////////////////////////////////////////
            // Global config
            //////////////////////////////////////////////

            bool        GlobalConfig_EnableAll = true;
            QString     GlobalConfig_MinVersion = HUGGLE_VERSION;
            QString     GlobalConfig_LocalConfigWikiPath = "Project:Huggle/Config";
            QString     GlobalConfig_DocumentationPath = "https://www.mediawiki.org/wiki/Manual:Huggle";
            QString     GlobalConfig_FeedbackPath = "http://en.wikipedia.org/wiki/Wikipedia:Huggle/Feedback";
            QString     GlobalConfig_UserConf = "User:$1/huggle3.css";
            QString     GlobalConfig_UserConf_old = "User:$1/huggle.css";
            QString     GlobalConfig_Whitelist = "http://huggle.wmflabs.org/data/";
            bool        GlobalConfigWasLoaded = false;

            //////////////////////////////////////////////
            // Local config
            //////////////////////////////////////////////

            //! Minimal version of huggle required to use it
            QString         ProjectConfig_MinimalVersion = HUGGLE_VERSION;
            bool            ProjectConfig_UseIrc = false;
            //! If admin rights are required to use huggle
            bool            ProjectConfig_RequireAdmin = false;
            //! If autoconfirmed is required to use huggle
            bool            ProjectConfig_RequireAutoconfirmed = false;
            bool            ProjectConfig_RequireConfig = false;
            //! Amount of edits required to use huggle
            int             ProjectConfig_RequireEdits = 0;
            //! If rollback right is required to use huggle
            bool            ProjectConfig_RequireRollback = false;
            bool            ProjectConfig_EnableAll = false;
            byte_ht         ProjectConfig_WarningLevel = 4;
            bool            ProjectConfig_AIV = false;
            bool            ProjectConfig_AIVExtend = true;
            bool            ProjectConfig_RFPP = false;
            unsigned int    ProjectConfig_RFPP_Section;
            QString         ProjectConfig_RFPP_Template = "";
            QString         ProjectConfig_RFPP_TemplateUser = "";
            QString         ProjectConfig_RFPP_Summary = "Sending request to protect a page";
            bool            ProjectConfig_RFPP_PlaceTop = false;
            QString         ProjectConfig_RFPP_Regex = "";
            QString         ProjectConfig_RFPP_Page = "";
            QString         ProjectConfig_ReportAIV = "";
            //! Section of report page to append template to
            int             ProjectConfig_ReportSt = 0;
            //! IP vandals
            QString         ProjectConfig_IPVTemplateReport = "User $1: $2$3 ~~~~";
            //! Regular users
            QString         ProjectConfig_RUTemplateReport = "User $1: $2$3 ~~~~";
            QString         ProjectConfig_ReportDefaultReason = "vandalism";
            QString         ProjectConfig_WelcomeSummary = "Welcoming user";
            Headings        ProjectConfig_Headings;
            int             ProjectConfig_TemplateAge = -30;
            bool            ProjectConfig_ConfirmTalk = true;
            bool            ProjectConfig_ConfirmWL = true;
            bool            ProjectConfig_ConfirmOnSelfRevs = true;
            bool            ProjectConfig_ConfirmMultipleEdits = false;
            bool            ProjectConfig_ConfirmRange = false;
            bool            ProjectConfig_ConfirmPage = false;
            bool            ProjectConfig_ConfirmSame = false;
            bool            ProjectConfig_ConfirmWarned = false;
            bool            ProjectConfig_Patrolling = false;

            // Reverting
            QString         ProjectConfig_MultipleRevertSummary = "Reverted,edit by,edits by,and,other users,to last revision by,to an older version by";
            QStringList     ProjectConfig_RevertSummaries;
            QString         ProjectConfig_SoftwareRevertDefaultSummary;
            QString         ProjectConfig_RollbackSummary = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to last revision by $2";
            QString         ProjectConfig_RollbackSummaryUnknownTarget = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])";
            QString         ProjectConfig_DefaultSummary = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to last revision by $2";
            QString         ProjectConfig_SingleRevert = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])";
            QString         ProjectConfig_UndoSummary = "Undid edit by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])";
            QString         ProjectConfig_ClearTalkPageTemp = "{{Huggle/Cleared}}";
            QString         ProjectConfig_WelcomeAnon = "{{subst:Welcome-anon}} ~~~~";
            QString         ProjectConfig_WelcomeTitle = "Welcome";

            // Deleting
            QString         ProjectConfig_DeletionTitle;
            QStringList     ProjectConfig_DeletionSummaries;
            QString         ProjectConfig_AssociatedDelete = "G8. Page dependent on a non-existent or deleted page.";

            // Warnings
            QString         ProjectConfig_AgfRevert = "Reverted good faith edits";
            QString         ProjectConfig_WarnSummary = "Warning (level 1)";
            QString         ProjectConfig_WarnSummary2 = "Warning (level 2)";
            QString         ProjectConfig_WarnSummary3 = "Warning (level 3)";
            QString         ProjectConfig_WarnSummary4 = "Warning (level 4)";
            QStringList     ProjectConfig_WarningTemplates;
            QStringList     ProjectConfig_WarningDefs;
            QString         ProjectConfig_ReportSummary;
            QString         ProjectConfig_RestoreSummary = "Restored revision $1 made by $2";
            bool            ProjectConfig_WelcomeGood = true;

            // Blocking users
            QStringList     ProjectConfig_BlockExpiryOptions;
            QString         ProjectConfig_BlockTime = "indefinite";
            QString         ProjectConfig_BlockTimeAnon = "31 hours";
            QString         ProjectConfig_BlockMessage = "{{subst:huggle/block|1=$1|2=$2}}";
            QString         ProjectConfig_BlockMessageIndef = "{{subst:huggle/block-indef|1=$1}}";
            QString         ProjectConfig_BlockReason = "[[WP:VAND|Vandalism]]";
            QString         ProjectConfig_BlockSummary = "Notification: Blocked";

            // Protecting pages
            QString         ProjectConfig_ProtectReason;

            // Templates
            QString         ProjectConfig_SharedIPTemplateTags = "";
            QString         ProjectConfig_SharedIPTemplate = "";

            // Definitions
            QList<ScoreWord> ProjectConfig_ScoreParts;
            QList<ScoreWord> ProjectConfig_ScoreWords;
            int              ProjectConfig_ScoreFlag = -60;
            int              ProjectConfig_ForeignUser = 800;
            int              ProjectConfig_ScoreTalk = -200;
            //! Score that is added for every edit that has really big size
            int              ProjectConfig_ScoreChange = 100;
            int              ProjectConfig_ScoreUser = -600;
            QStringList      ProjectConfig_Ignores;
            QStringList      ProjectConfig_RevertPatterns;
            QStringList      ProjectConfig_Assisted;
            QStringList      ProjectConfig_Templates;
            QStringList      ProjectConfig_IgnorePatterns;
            int              ProjectConfig_TalkPageWarningScore = -800;
            bool             ProjectConfig_GlobalRequired = true;
            // This is internal only do not prefix it!!
            QList<QRegExp>   RevertPatterns;
            int              ProjectConfig_BotScore = -200;
            int              ProjectConfig_IPScore = 800;
            int              ProjectConfig_WarningScore = 2000;
            QStringList      ProjectConfig_WarningTypes;
            QString          ProjectConfig_SpeedyEditSummary = "Tagging page for deletion";
            QString          ProjectConfig_SpeedyWarningSummary = "Sending user a notification regarding deletion of their page";
            QStringList      ProjectConfig_SpeedyTemplates;
            QStringList      ProjectConfig_WelcomeTypes;
            long             ProjectConfig_WhitelistScore = -800;
            // UAA
            QString          ProjectConfig_UAAPath = "Project:Usernames for administrator attention";
            bool             ProjectConfig_UAAavailable = false;
            QString          ProjectConfig_UAATemplate = "* {{user-uaa|1=$1}} $2 ~~~~";

            //////////////////////////////////////////////
            // Login
            //////////////////////////////////////////////

            //! User name
            QString     SystemConfig_Username = "User";
            //! If SSL is being used
            bool        SystemConfig_UsingSSL = true;
            //! Consumer key
            QString     WmfOAuthConsumerKey;
            //! This is automatically changed to false everytime when new edit is loaded
            //! changing it to true will disable auto-jump to newer edit.
            //! Typical usage for this is when you want to display a diff that may not be
            //! latest revision and you want to force huggle to not load a latest version
            //! even if user wants that (used by toolbar when user info is loaded).
            bool        ForcedNoEditJump = false;
            //! Password
            QString     TemporaryConfig_Password = "";

            //////////////////////////////////////////////
            // IRC
            //////////////////////////////////////////////

            //! Whether IRC is being used
            bool    UsingIRC = true;
            //! Server
            QString IRCServer = "irc.wikimedia.org";
            //! Nick
            QString IRCNick = "huggle";
            //! Ident
            QString IRCIdent = "huggle";
            //! Port
            int     IRCPort = 6667;
            int     SystemConfig_IRCConnectionTimeOut = 2;

            //////////////////////////////////////////////
            // Friends
            //////////////////////////////////////////////

            //! Suffix used by huggle
            QString     ProjectConfig_EditSuffixOfHuggle = "([[WP:HG|HG 3]])";
            //! Regexes that other tools can be identified with
            QStringList ProjectConfig_EditRegexOfTools;

            //////////////////////////////////////////////
            // Reverting
            //////////////////////////////////////////////

            //! Warn you in case you want to revert a user page
            bool        WarnUserSpaceRoll = true;
            QStringList Months;
            //! Send a message to user on good edit
            bool        WelcomeEmpty = true;
            //! This is changed to true in case that someone send a message to user
            bool        NewMessage = false;
            QString     VandalNw_Server = "irc.tm-irc.org";
            QString     VandalNw_Ident;
            bool        VandalNw_Login = true;
            //! Pointer to AIV page
            WikiPage    *AIVP = NULL;
            //! Pointer to UAA page
            WikiPage    *UAAP = NULL;
            //! Operating system that is sent to update server
            QString     Platform;
        private:
            QHash       <QString, Option> Options;
    };
}

#endif // CONFIGURATION_H
