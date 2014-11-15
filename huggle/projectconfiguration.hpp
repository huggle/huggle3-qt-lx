//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef PROJECTCONFIGURATION_H
#define PROJECTCONFIGURATION_H

// Include file with all global defines
#include "definitions.hpp"

#include <QList>
#include <QDateTime>
#include <QStringList>
#include <QHash>
#include <QString>

// Private key names
// these need to be stored in separate variables so that we can
// 1. Change them on 1 place
// 2. Track them (we need to be able to find where these options
//    are being used)
#define                 ProjectConfig_IPScore_Key "score-ip"

namespace Huggle
{
    class WikiPage;

    enum Headings
    {
        HeadingsStandard,
        HeadingsPageName,
        HeadingsNone
    };

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

    //! Project configuration, each project needs to have own instance of this
    class ProjectConfiguration
    {
        public:
            ProjectConfiguration(QString project_name);
            ~ProjectConfiguration();
            QString ProjectName;
            QDateTime ServerTime();
            //! Parse all information from local config, this function is used in login
            bool Parse(QString config, QString *reason);
            void RequestLogin();
            //! \todo This needs to be later used as a default value for user config, however it's not being ensured
            //!       this value is loaded before the user config right now
            bool AutomaticallyResolveConflicts = false;
            bool IsSane = false;
            QString     EditToken = "";
            QStringList Months;
            //! Pointer to AIV page
            WikiPage    *AIVP = nullptr;
            //! Pointer to UAA page
            WikiPage    *UAAP = nullptr;
            //! Set to false when you are logged out for some reason
            bool            IsLoggedIn = false;
            bool            RequestingLogin = false;
            //! Minimal version of huggle required to use it
            QString         MinimalVersion = HUGGLE_VERSION;
            bool            UseIrc = false;
            //! If admin rights are required to use huggle
            bool            RequireAdmin = false;
            //! If autoconfirmed is required to use huggle
            bool            RequireAutoconfirmed = false;
            bool            RequireConfig = false;
            //! Amount of edits required to use huggle
            int             RequireEdits = 0;
            //! If rollback right is required to use huggle
            bool            RequireRollback = false;
            bool            EnableAll = false;
            byte_ht         WarningLevel = 4;
            bool            AIV = false;
            bool            AIVExtend = true;
            bool            RFPP = false;
            unsigned int    RFPP_Section;
            QString         RFPP_Template = "";
            QString         RFPP_TemplateUser = "";
            QString         RFPP_Summary = "Sending request to protect a page";
            bool            RFPP_PlaceTop = false;
            QString         RFPP_Mark = "";
            QString         RFPP_Regex = "";
            QString         RFPP_Page = "";
            QString         ReportAIV = "";
            QString         Feedback = "";
            //! Section of report page to append template to
            int             ReportSt = 0;
            //! User flags on current project, this may be empty if you fail to login
            QStringList     Rights;
            //! IP vandals
            QString         IPVTemplateReport = "User $1: $2$3 ~~~~";
            //! Regular users
            QString         RUTemplateReport = "User $1: $2$3 ~~~~";
            QString         ReportDefaultReason = "vandalism";
            QString         WelcomeSummary = "Welcoming user";
            Headings        MessageHeadings;
            int             TemplateAge = -30;
            /// \todo move the following confirms to UserConfig, probably shouldn't read at all (initially) from ProjectConfig
            bool            ConfirmTalk = true;
            bool            ConfirmWL = true;
            bool            ConfirmOnSelfRevs = true;
            bool            ConfirmMultipleEdits = false;
            /// \todo implement or remove: also commented out on configuration read/write
            // bool            ConfirmRange = false;
            // bool            ConfirmPage = false;
            // bool            ConfirmSame = false;
            // bool            ConfirmWarned = false;
            bool            Patrolling = false;
            bool            PatrollingFlaggedRevs = false;
            int             IPScore = 20;
            // Reverting
            QString         MultipleRevertSummary = "Reverted,edit by,edits by,and,other users,to last revision by,to an older version by";
            QStringList     RevertSummaries;
            QStringList     Goto;
            QString         SoftwareRevertDefaultSummary;
            /// \todo use rollback summary at least at mw-rollback
            QString         RollbackSummary = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to last revision by $2";
            //! This is a token returned by tokens query from mw which should be same for all rollback execs
            QString         RollbackToken;
            QString         RollbackSummaryUnknownTarget = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])";
            QString         DefaultSummary = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to last revision by $2";
            QString         SingleRevert = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])";
            QString         UndoSummary = "Undid edit by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])";
            QString         ClearTalkPageTemp = "{{Huggle/Cleared}}";
            QString         WelcomeAnon = "{{subst:Welcome-anon}}";
            QString         WelcomeTitle = "Welcome";

            // Deleting
            QString         DeletionTitle;
            QStringList     DeletionSummaries;
            QString         AssociatedDelete = "G8. Page dependent on a non-existent or deleted page.";
            // Warnings
            QString         AgfRevert = "Reverted good faith edits";
            QString         WarnSummary = "Warning (level 1)";
            QString         WarnSummary2 = "Warning (level 2)";
            QString         WarnSummary3 = "Warning (level 3)";
            QString         WarnSummary4 = "Warning (level 4)";
            QStringList     WarningTemplates;
            QStringList     WarningDefs;
            //! Data of wl (list of users)
            QStringList     WhiteList;
            QStringList     NewWhitelist;
            QString         ReportSummary;
            QString         RestoreSummary = "Restored revision $1 made by $2";
            bool            WelcomeGood = true;

            // Blocking users
            QStringList     BlockExpiryOptions;
            QString         BlockTime = "indefinite";
            QString         BlockTimeAnon = "31 hours";
            QString         BlockMessage = "{{subst:huggle/block|1=$1|2=$2}}";
            QString         BlockMessageIndef = "{{subst:huggle/block-indef|1=$1}}";
            QString         BlockReason = "[[WP:VAND|Vandalism]]";
            QString         BlockSummary = "Notification: Blocked";

            // Protecting pages
            QString         ProtectReason;

            // Templates
            QString         SharedIPTemplateTags = "";
            QString         SharedIPTemplate = "";

            // Definitions
            QList<ScoreWord>        ScoreParts;
            QList<ScoreWord>        ScoreWords;
            int                     ScoreFlag = -60;
            int                     ForeignUser = 800;
            int                     ScoreTalk = -200;
            //! Score that is added for every edit that has really big size
            int                     ScoreChange = 100;
            int                     LargeRemoval = 400;
            int                     ScoreRemoval = 800;
            int                     ScoreUser = -600;
            //! This is a number that can be used to get a current server time
            qint64                  ServerOffset = 0;
            QStringList             Ignores;
            QStringList             RevertPatterns;
            QStringList             Assisted;
            QStringList             Templates;
            QStringList             IgnorePatterns;
            QString                 Parser_Date_Prefix = ",";
            QStringList             Parser_Date_Suffix;
            int                     TalkPageWarningScore = -800;
            bool                    GlobalRequired = true;
            // Tagging
            QString                 TaggingSummary;
            QStringList             Tags;
            //! Where the welcome message is stored
            QString                 WelcomeMP = "Project:Huggle/Message";
            // This is internal only do not prefix it!!
            QList<QRegExp>          _revertPatterns;
            int                     BotScore = -200;
            int                     WarningScore = 2000;
            QStringList             WarningTypes;
            QString                 SpeedyEditSummary = "Tagging page for deletion";
            QString                 SpeedyWarningSummary = "Sending user a notification regarding deletion of their page";
            QHash<int,QStringList>  AlternativeMonths;
            QStringList             SpeedyTemplates;
            QStringList             WelcomeTypes;
            long                    WhitelistScore = -800;
            QString                 WatchlistToken = "";
            // UAA
            QString                 UAAPath = "Project:Usernames for administrator attention";
            bool                    UAAavailable = false;
            QString                 UAATemplate = "* {{user-uaa|1=$1}} $2 ~~~~";
            //! Suffix used by huggle
            QString                 EditSuffixOfHuggle = "([[WP:HG|HG 3]])";
            //! Regexes that other tools can be identified with
            QStringList             EditRegexOfTools;
    };

    inline void ProjectConfiguration::RequestLogin()
    {
        if (this->IsLoggedIn && !this->RequestingLogin)
            this->IsLoggedIn = false;
    }
}

#endif // PROJECTCONFIGURATION_H
