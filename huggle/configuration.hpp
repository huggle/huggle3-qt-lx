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

#include <QList>
#include <QStringList>
#include <QByteArray>
#include <QHash>
#include <QString>
#include "userconfiguration.hpp"
#include "projectconfiguration.hpp"
class QXmlStreamWriter;

#define HUGGLE_ACCEL_NONE ""
#define HUGGLE_ACCEL_MAIN_EXIT                  0
#define HUGGLE_ACCEL_MAIN_REVERT_AND_WARN       1
#define HUGGLE_ACCEL_MAIN_REVERT                2
#define HUGGLE_ACCEL_MAIN_WARN                  3
#define HUGGLE_ACCEL_NEXT                       4
#define HUGGLE_ACCEL_SUSPICIOUS_EDIT            5
#define HUGGLE_ACCEL_MAIN_FORWARD               6
#define HUGGLE_ACCEL_MAIN_BACK                  7
#define HUGGLE_ACCEL_MAIN_REVERT_AND_WARN0      8
#define HUGGLE_ACCEL_MAIN_REVERT_AND_WARN1      9
#define HUGGLE_ACCEL_MAIN_REVERT_AND_WARN2      10
#define HUGGLE_ACCEL_MAIN_REVERT_AND_WARN3      11
#define HUGGLE_ACCEL_MAIN_REVERT_AND_WARN4      12
#define HUGGLE_ACCEL_MAIN_REVERT_AND_WARN5      13
#define HUGGLE_ACCEL_MAIN_REVERT_AND_WARN6      14
#define HUGGLE_ACCEL_MAIN_REVERT_AND_WARN7      15
#define HUGGLE_ACCEL_MAIN_REVERT_AND_WARN8      16
#define HUGGLE_ACCEL_MAIN_REVERT_AND_WARN9      17
#define HUGGLE_ACCEL_MAIN_WARN0                 18
#define HUGGLE_ACCEL_MAIN_WARN1                 19
#define HUGGLE_ACCEL_MAIN_WARN2                 20
#define HUGGLE_ACCEL_MAIN_WARN3                 21
#define HUGGLE_ACCEL_MAIN_WARN4                 22
#define HUGGLE_ACCEL_MAIN_WARN5                 23
#define HUGGLE_ACCEL_MAIN_WARN6                 24
#define HUGGLE_ACCEL_MAIN_WARN7                 25
#define HUGGLE_ACCEL_MAIN_WARN8                 26
#define HUGGLE_ACCEL_MAIN_WARN9                 27
#define HUGGLE_ACCEL_MAIN_REVERT_0              28
#define HUGGLE_ACCEL_MAIN_REVERT_1              29
#define HUGGLE_ACCEL_MAIN_REVERT_2              30
#define HUGGLE_ACCEL_MAIN_REVERT_3              31
#define HUGGLE_ACCEL_MAIN_REVERT_4              32
#define HUGGLE_ACCEL_MAIN_REVERT_5              33
#define HUGGLE_ACCEL_MAIN_REVERT_6              34
#define HUGGLE_ACCEL_MAIN_REVERT_7              35
#define HUGGLE_ACCEL_MAIN_REVERT_8              36
#define HUGGLE_ACCEL_MAIN_REVERT_9              37
#define HUGGLE_ACCEL_MAIN_TALK                  38
#define HUGGLE_ACCEL_REVERT_STAY                39
#define HUGGLE_ACCEL_MAIN_OPEN_IN_BROWSER       40
#define HUGGLE_ACCEL_MAIN_GOOD                  41
#define HUGGLE_ACCEL_MAIN_MYTALK_PAGE           42
#define HUGGLE_ACCEL_REVW_STAY                  43
#define HUGGLE_ACCEL_MAIN_REVERT_AGF_ONE_REV    44
#define HUGGLE_ACCEL_MAIN_WATCH                 45
#define HUGGLE_ACCEL_MAIN_UNWATCH               46
#define HUGGLE_ACCEL_MAIN_OPEN                  48
#define HUGGLE_ACCEL_CLOSE_TAB                  200
#define HUGGLE_ACCEL_CREATE_NEW_TAB             206

#define hcfg Huggle::Configuration::HuggleConfiguration

//! Huggle namespace contains all objects that belongs to huggle only so that they don't colide with other objects
namespace Huggle
{
    class WikiSite;
    class HuggleQueueFilter;
    class HuggleQueue;
    class Syslog;
    class HuggleQueueParser;
    class HuggleOption;

    //! This is used to handle the shortcuts for the main form
    class Shortcut
    {
        public:
            Shortcut();
            Shortcut(QString name, QString description);
            Shortcut(const Shortcut &copy);
            QString Name;
            QString Description;
            QString QAccel;
            int ID;
            bool Modified = false;
    };

    //! Used to store the configuration per extension so that each extension can create own private keys with options
    class ExtensionConfig
    {
        public:
            void SetOption(QString name, QString value);
            /*!
             * \brief GetOption Retrieve an option from local config
             * \param name Name of key
             * \param md Value that is returned when key is missing
             * \return Value that is associated with the key
             */
            QString GetOption(QString name, QString md = "");
        private:
            QHash<QString, QString> Options;
            friend class Configuration;
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
            static QString GetProjectWikiURL(WikiSite *Project);
            //! Return a script url like http://en.wikipedia.org/w/
            static QString GetProjectScriptURL(WikiSite *Project);
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
            static QString GetProjectURL(WikiSite *Project);
            static QString GetLocalizationDataPath();
            //! Extension path (typically HR/extensions) where .py and .so files are in
            static QString GetExtensionsRootPath();
            //! Return a prefix for url
            static QString GetURLProtocolPrefix(WikiSite *s = nullptr);
            //! Returns full configuration path suffixed with slash
            static QString GetConfigurationPath();
            static QString ReplaceSpecialUserPage(QString PageName);

            //! Save the local configuration to file
            static void SaveSystemConfig();
            //! Load the local configuration from disk
            static void LoadSystemConfig(QString fn);
            //! This function appends the huggle suffix to a edit summary
            static QString GenerateSuffix(QString text, ProjectConfiguration *conf);
            static Configuration *HuggleConfiguration;

            Configuration();
            ~Configuration();
            void NormalizeConf(WikiSite *site);
            //! Parse all information from global config on meta
            bool ParseGlobalConfig(QString config);
            QString GetExtensionConfig(QString extension, QString name, QString ms);
            ////////////////////////////////////////////
            // System
            ////////////////////////////////////////////

            QHash<QString, ExtensionConfig*> ExtensionData;
            QHash<QString, Shortcut> Shortcuts;
            //! If it's needed to reload config of main form
            bool                     ReloadOfMainformNeeded = true;
            //! Verbosity for debugging to terminal etc, can be switched with parameter --verbosity
            unsigned int      Verbosity = 0;
            //! Version
            QString           HuggleVersion;
            QString           HANMask = "$feed.huggle";
            QByteArray        WebqueryAgent;
            bool              Multiple = false;
            QList<WikiSite *> Projects;
            //! currently selected project
            WikiSite         *Project = nullptr;
            //! List of projects
            QList<WikiSite *> ProjectList;
            QStringList       ProjectString;
            //! When this is true most of functions will not work
            bool            Restricted = false;
            //! This is used in combination with --login option, so that huggle knows if it should
            //! login automatically or wait for user to fill in their user information
            bool            Login = false;
            bool            SystemConfig_DryMode = false;
            //! Maximum number of queue stuff
            int             SystemConfig_QueueSize = 200;
            //! Whether python is available
            bool            PythonEngine;
            bool            Fuzzy = false;
            //! Size of feed
            int             SystemConfig_ProviderCache = 200;
            //! Maximum size of ringlog
            int             SystemConfig_RingLogMaxSize = 2000;
            //! Path where huggle contains its data
            QString         HomePath;
            //! Path to a file where information about wikis are stored
            QString         WikiDB = "";
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
            //! Whether new edits go to top or bottom (if true, they go to up)
            bool            SystemConfig_QueueNewEditsUp = false;
            //! If this is true some functionalities will be disabled
            bool            SystemConfig_SafeMode = false;
            //! Timeout for queries
            int             SystemConfig_ReadTimeout = 60;
            //! Timeout for write / update queries
            int             SystemConfig_WriteTimeout = 200;
            //! Whitelist is not useable
            bool            SystemConfig_WhitelistDisabled = false;
            //! List of characters that separate words from each other, like dot, space etc, used by score words
            QStringList     SystemConfig_WordSeparators;
            //! This is affecting if columns are auto-sized or not
            bool            SystemConfig_DynamicColsInList = false;
            QString         SystemConfig_GlobalConfigWikiList = "Project:Huggle/List";
            //! Changing this to true will make the Syslog write all data to a file
            bool            SystemConfig_Log2File = false;
            //! This path is used when Log2File is true to write the logs to
            QString         SystemConfig_SyslogPath = "huggle.log";
            //! Whether huggle check for an update on startup
            bool            SystemConfig_UpdatesEnabled = true;
            bool            SystemConfig_NotifyBeta = false;
            bool            SystemConfig_LanguageSanity = false;
            bool            SystemConfig_RequestDelay = false;
            bool            SystemConfig_SuppressWarnings = true;
            unsigned int    SystemConfig_DelayVal = 0;
            unsigned int    SystemConfig_WikiRC = 200;
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

            //////////////////////////////////////////////
            // User
            //////////////////////////////////////////////
            UserConfiguration *UserConfig = nullptr;

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

            ProjectConfiguration *ProjectConfig = nullptr;

            //////////////////////////////////////////////
            // Login
            //////////////////////////////////////////////

            //! User name
            QString     SystemConfig_Username = "";
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
            // Reverting
            //////////////////////////////////////////////

            //! Warn you in case you want to revert a user page
            bool        WarnUserSpaceRoll = true;
            //! Send a message to user on good edit
            bool        WelcomeEmpty = true;
            //! This is changed to true in case that someone send a message to user
            bool        NewMessage = false;
            QString     VandalNw_Server = "irc.tm-irc.org";
            QString     VandalNw_Ident;
            bool        VandalNw_Login = true;
            QString     QueryDebugPath = "querydump.dat";
            bool        QueryDebugging = false;
            //! Operating system that is sent to update server
            QString     Platform;
        private:
            /*!
             * \brief InsertConfig
             * \param key Configuration key
             * \param value Value of key
             * \param s Stream writer
             */
            static void InsertConfig(QString key, QString value, QXmlStreamWriter *s);

            void MakeShortcut(QString name, QString description, QString default_accel = HUGGLE_ACCEL_NONE);
    };
}

#endif // CONFIGURATION_H
