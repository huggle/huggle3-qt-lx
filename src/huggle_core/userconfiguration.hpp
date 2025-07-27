//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef USERCONFIGURATION_H
#define USERCONFIGURATION_H

// Include file with all global defines
#include "definitions.hpp"

#include <QString>
#include <QList>
#include <QStringList>
#include <QHash>

namespace YAML
{
    class Node;
}

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

    enum WatchlistOption
    {
        WatchlistOption_Watch = 0,
        WatchlistOption_Unwatch = 1,
        WatchlistOption_Preferences = 2,
        WatchlistOption_NoChange = 3
    };

    class HuggleOption;
    class ProjectConfiguration;
    class Version;

    //! User configuration, for a user per project
    //! This class was historically used to parse the huggle3.css config file, since 3.4.0 it's used to parse the huggle.yaml file by default
    class HUGGLE_EX_CORE UserConfiguration
    {
        public:
            static WatchlistOption WatchlistOptionFromString(const QString& string);
            static QString WatchListOptionToString(WatchlistOption option);

            UserConfiguration();
            ~UserConfiguration();
            //! In case there is no user config on wiki yet, Parse is not called at all so we need to initialize this user config with defaults from project
            void SetDefaults(ProjectConfiguration *ProjectConfig);
            //! This function is obsolete and used only to read the old format huggle3.css file, new versions should only use YAML formats
            bool Parse(const QString& config, ProjectConfiguration *ProjectConfig, bool IsHome);
            bool ParseYAML(const QString &config, ProjectConfiguration *ProjectConfig, bool IsHome, QString *error);
            /*!
             * \brief Returns a data for given key
             * \param key
             * \return New instance of data or NULL in case there is no such an option
             */
            HuggleOption *GetOption(const QString& key);
            /*!
             * \brief SetOption lookup for a key in config file, if there is no such a key, insert a default one
             * \param key_ Name of configuration key in user config file
             * \param config_ Config file text
             * \param default_ Value that is used in case there is no such a key
             */
            QVariant SetOption(const QString& key, const QString& config, const QVariant& def);
            QVariant SetOptionYAML(const QString& key, YAML::Node &config, const QVariant& def);
            QStringList SetUserOptionList(const QString& key, const QString& config, const QStringList& def, bool CS = false);
            QStringList SetUserOptionListYAML(const QString& key, YAML::Node &config, const QStringList& def);
            int GetSafeUserInt(const QString& key, int default_value = 0);
            bool GetSafeUserBool(const QString& key, bool default_value = false);
            QString GetSafeUserString(const QString& key, QString default_value = "");
            //! This function creates a user configuration in YAML format that is stored on wiki
            QString MakeLocalUserConfig(ProjectConfiguration *Project);
            bool            EnforceSoftwareRollback();
            QHash<QString, HuggleOption*> UserOptions;
            //! Whether edits by same user should be grouped in history widget
            bool            AutomaticallyGroup = false;
            bool            AutomaticReports = false;
            //! Resolve edit conflict without asking user
            bool            AutomaticallyResolveConflicts = true;
            bool            AutomaticRefresh = true;
            bool            AutomaticallyWatchlistWarnedUsers = false;
            //! When user you want to send a warning to received another warning recently, ask if you really want to send it
            bool            ConfirmOnRecentWarning = true;
            //! If warning was sent less than N seconds ago it's considered too recent
            int             RecentWarningTimeSpan = 120;
            //! When old edit is reverted, ask if you really want to send a warning for it
            bool            ConfirmWarningOnVeryOldEdits = true;
            //! In case edit is old or user recently got a warning, just write this into logs and don't ask user anything
            bool            SkipWarningOnConfirm = true;
            //! default-summary inherited from project config so there is no default here
            QString         DefaultSummary;
            QString         RollbackSummary;
            QString         RollbackSummaryUnknownTarget;
            //! If this is true huggle will always use software rollback even if user has the rollback privileges
            bool            EnforceManualSoftwareRollback = false;
            bool            EnforceMonthsAsHeaders = true;
            //! This will temporarily make huggle use software rollback as a workaround for bug where
            //! mediawiki reject token with no reason
            bool            EnforceManualSRT = false;
            int             PreferredProvider = 2;
            unsigned int    TalkPageFreshness = 20;
            bool            ShowWarningIfNotOnLastRevision = true;
            //! Huggle will auto revert all edits that were made by same user on auto conflict resolution
            bool            RevertOnMultipleEdits = true;
            //! Whether a founder of every page should be retrieved or not
            bool            RetrieveFounder = false;
            bool            HtmlAllowedInIrc = false;
            //! Removes the edit from queue in case that some trusted user edited the same page
            bool            RemoveAfterTrustedEdit = true;
            bool            HuggleSuffix = true;
            QString         ShortcutHash;
            //! If history and user info should be automatically loaded for every edit
            bool                    HistoryLoad = true;
            //! Defines what should be done on next edit
            Configuration_OnNext    GoNext = Configuration_OnNext_Next;
            //! Check if the edit that was reverted was in queue and if yes, remove it
            bool                    DeleteEditsAfterRevert = true;
            //! Fetch only the last edit of page, that means if there is a newer edit
            //! it get automatically loaded instead of cached version
            bool                    LastEdit = false;
            bool                    SectionKeep = true;
            bool                    NumberDropdownMenuItems = true;
            unsigned int            HistoryMax = 50;
            bool                    TruncateEdits = false;
            //! By default Huggle highlights "Missing summary", if this is set to true, it's highlighted instead when it's there
            bool                    HighlightSummaryIfExists = false;
            //! TODO: this probably should be merged with RevertOnMultipleEdits
            bool                    RevertNewBySame = true;
            //! If this is set to false the warning will be selected by huggle when user decide to
            //! use the "warn only" feature in huggle (W) for example, it doesn't affect reverting
            bool                    ManualWarning = false;
            //! Large title of every page in top of diff
            bool                    DisplayTitle = false;
            //! Result of "Stop feed, Remove old edits" in main form
            bool                    RemoveOldQueueEdits = false;
            //! Check for new messages on your talk page
            bool                    CheckTP = true;
            //! Default queue ID
            QString                 QueueID = "default";
            //! Display messages from users in vandal window
            bool                    HAN_DisplayUser = true;
            //! Display message from bots in vandal window
            bool                    HAN_DisplayBots = false;
            bool                    HAN_DisplayUserTalk = true;
            //! Welcome new users on a good edit
            bool                    WelcomeGood = false;
            //! Set to true if you want to ignore queue edits that are higher than specific score (useful to filter only good edits)
            bool                    EnableMaxScore = false;
            score_ht                MaxScore = 0;
            bool                    EnableMinScore = false;
            score_ht                MinScore = 0;
            QString                 PageEmptyQueue;
            Version*                Previous_Version;
            WatchlistOption         Watchlist;
            //! Insert top revision edits of user when we rollback some of their edits
            bool                    InsertEditsOfRolledUserToQueue = true;
        private:
            YAML::Node              *yaml_node = nullptr;
    };
}

#endif
