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
#include "huggleoption.hpp"
#include "wikisite.hpp"
#include "userconfiguration.hpp"
#include "projectconfiguration.hpp"

//! Huggle namespace contains all objects that belongs to huggle only so that they don't colide with other objects
namespace Huggle
{
    class WikiSite;
    class HuggleQueueFilter;
    class HuggleQueue;
    class WikiPage;
    class Syslog;
    class HuggleQueueParser;
    class HuggleOption;

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
            HuggleOption *GetOption(QString key);
            /*!
             * \brief SetOption lookup for a key in config file, if there is no such a key, insert a default one
             * \param key_ Name of configuration key in user config file
             * \param config_ Config file text
             * \param default_ Value that is used in case there is no such a key
             */
            QVariant SetOption(QString key_, QString config_, QVariant default_);
            QStringList SetUserOptionList(QString key_, QString config_, QStringList default_, bool CS = false);
            int GetSafeUserInt(QString key_, int default_value = 0);
            bool GetSafeUserBool(QString key_, bool default_value = false);
            QString GetSafeUserString(QString key_, QString default_value = "");
            void NormalizeConf();
            QString GenerateSuffix(QString text);
            //! Parse all information from global config on meta
            bool ParseGlobalConfig(QString config);
            //! Parse all information from local config, this function is used in login
            bool ParseProjectConfig(QString config);
            bool ParseUserConfig(QString config);
            QDateTime ServerTime();
            ////////////////////////////////////////////
            // System
            ////////////////////////////////////////////

            //! Verbosity for debugging to terminal etc, can be switched with parameter --verbosity
            unsigned int    Verbosity = 0;
            //! Version
            QString         HuggleVersion;
            //! currently selected project
            WikiSite        *Project = nullptr;
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
            QString         TemporaryConfig_EditToken = "";
            //! This is a number that can be used to get a current server time
            qint64          ServerOffset = 0;

            //////////////////////////////////////////////
            // User
            //////////////////////////////////////////////
            UserConfiguration *UserConfig = nullptr;

            // Private key names
            // these need to be stored in separate variables so that we can
            // 1. Change them on 1 place
            // 2. Track them (we need to be able to find where these options
            //    are being used)
            #define                 ProjectConfig_IPScore_Key "score-ip"

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
            WikiPage    *AIVP = nullptr;
            //! Pointer to UAA page
            WikiPage    *UAAP = nullptr;
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
            static QString Bool2ExcludeRequire(bool b);
            // USER

    };
}

#endif // CONFIGURATION_H
