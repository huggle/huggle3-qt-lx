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

namespace YAML
{
    class Node;
}

namespace Huggle
{
    class WikiPage;
    class WikiSite;

    enum Headings
    {
        HeadingsStandard,
        HeadingsPageName,
        HeadingsNone
    };

    enum ReportType
    {
        ReportType_DefaultManual,
        ReportType_DefaultAuto,
        ReportType_StrictManual,
        ReportType_StrictAuto
    };

    /*!
     * \brief The ScoreWord class
     *
     * Every score word is represented by this class, a score word is a pattern
     * that has some score and score of edit is incremented by sum of all scores
     * of all score words matched in edit
     */
    class HUGGLE_EX_CORE ScoreWord
    {
        public:
            ScoreWord(QString Word, int Score);
            ScoreWord(ScoreWord *word);
            ScoreWord(const ScoreWord &word);
            QString word;
            int score;
    };

    //! Project configuration, each project needs to have own instance of this
    class HUGGLE_EX_CORE ProjectConfiguration
    {
        public:
            //! Speedy option - used for speedy deletions
            class HUGGLE_EX_CORE SpeedyOption
            {
                public:
                    //! Name of tag
                    QString Tag;
                    QString Template;
                    //! Description
                    QString Info;
                    //! Template used to message user
                    QString Msg;
                    //! Whether user should be notified when tag is applied
                    bool Notify = false;
                    QString Parameter;

            };

            static QList<ProjectConfiguration::SpeedyOption> Yaml_FetchSpeedyOptions(YAML::Node &node);
            static QHash<QString, int> Yaml_FetchScoreTags(YAML::Node &node);

            ProjectConfiguration(QString project_name);
            ~ProjectConfiguration();
            QDateTime ServerTime();
            //! Parse all information from local config, this function is used in login
            bool Parse(QString config, QString *reason, WikiSite *site);
            bool ParseYAML(QString yaml_src, QString *reason, WikiSite *site);
            void RequestLogin();
            QString GetConfig(QString key, QString dv = "");
            //! \todo This needs to be later used as a default value for user config, however it's not being ensured
            //!       this value is loaded before the user config right now
            bool AutomaticallyResolveConflicts = false;
            //! If true, parsing will be done using YAML parser
            bool UsingYAML = false;
            QString ReportAutoSummary;
            bool ReadOnly = false;
            bool IsSane = false;
            bool Approval = false;
            bool UserlistSync = false;
            QString UserlistUpdateSummary = "Adding [[Special:Contributions/$1|$1]]";
            QString ApprovalPage = "Project:Huggle/Users";
            QString ProjectName;
            WikiSite    *Site = nullptr;
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
            //! Number of days for user since registration that is needed for login
            int             RequireTime = 0;
            //! Amount of edits required to use huggle
            int             RequireEdits = 0;
            //! If rollback right is required to use huggle
            bool            RequireRollback = false;
            bool            EnableAll = false;
            byte_ht         WarningLevel = 4;
            bool            AIV = false;
            bool            RFPP = false;
            unsigned int    RFPP_Section;
            QString         RFPP_Template = "";
            QString         RFPP_TemplateUser = "";
            QString         RFPP_Summary = "Sending request to protect a page";
            bool            RFPP_PlaceTop = false;
            QString         RFPP_Mark = "";
            QString         RFPP_Regex = "";
            QString         RFPP_Page = "";
            QString         ReportUserCheckPattern = ".*$username.*";
            ReportType      ReportMode = ReportType_DefaultAuto;
            QString         ReportAIV = "";
            QString         Feedback = "";
            //! Section of report page to append template to
            int             ReportSection = 0;
            //! User flags on current project, this may be empty if you fail to login
            QStringList     Rights;
            //! IP vandals
            QString         IPVTemplateReport = "User $1: $2$3 ~~~~";
            //! Regular users
            QString         RUTemplateReport = "User $1: $2$3 ~~~~";
            QString         ReportDefaultReason = "vandalism";
            QString         WelcomeSummary = "Welcoming user";
            QString         TemplateHeader;
            Headings        MessageHeadings;
            QHash<int, QString> WarningSummaries;
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
            score_ht        EditScore = -2;
            score_ht        IPScore = 20;
            // Reverting
            QString         MultipleRevertSummary = "Reverted,edit by,edits by,and,other users,to last revision by,to an older version by";
            bool            RevertingEnabled = true;
            QStringList     RevertSummaries;
            QStringList     Goto;
            QString         SoftwareRevertDefaultSummary;
            //! This is used to identify edits made by huggle, if empty no tags are inserted to meta information of edit
            QString         Tag;
            QString         Token_Csrf;
            //! This is a token returned by tokens query from mw which should be same for all rollback execs
            QString         Token_Rollback;
            QString         Token_Watch;
            QString         Token_Patrol;
            /// \todo use rollback summary at least at mw-rollback
            QString         RollbackSummary = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to last revision by $2";
            QString         RollbackSummaryUnknownTarget = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])";
            QString         DefaultSummary = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to last revision by $2";
            QString         DefaultTemplate = "warning";
            QString         SingleRevert = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])";
            QString         UndoSummary = "Undid edit by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])";
            QString         ClearTalkPageTemp = "{{Huggle/Cleared}}";
            //! Template for anon users, leave empty if there is none for current wiki
            QString         WelcomeAnon;
            //! Welcome template
            QString         Welcome;
            QString         WelcomeTitle = "Welcome";

            // Deleting
            QString         DeletionTitle;
            QStringList     DeletionReasons;
            QString         AssociatedDelete = "G8. Page dependent on a non-existent or deleted page.";
            // Warnings
            QString         AgfRevert = "Reverted good faith edits";
            QStringList     WarningTemplates;
            //! Instant level - last warning messages supported
            bool            InstantWarnings = false;
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
            QHash<QString, int>     ScoreTags;
            QList<ScoreWord>        ScoreParts;
            QList<ScoreWord>        ScoreWords;
            //! Score for warning level
            QHash<int, score_ht>    ScoreLevel;
            QList<ScoreWord>        NoTalkScoreWords;
            QList<ScoreWord>        NoTalkScoreParts;
            score_ht                ScoreFlag = -60;
            score_ht                ForeignUser = 800;
            score_ht                ScoreTalk = -200;
            //! Score that is added for every edit that has really big size
            score_ht                ScoreChange = 100;
            score_ht                LargeRemoval = 400;
            score_ht                ScoreRemoval = 800;
            score_ht                ScoreUser = -600;
            //! This is a number that can be used to get a current server time
            qint64                  ServerOffset = 0;
            QStringList             Ignores;
            QStringList             RevertPatterns;
            QStringList             Assisted;
            QStringList             Templates;
            QStringList             IgnorePatterns;
            QString                 Parser_Date_Prefix = ",";
            QStringList             Parser_Date_Suffix;
            score_ht                TalkPageWarningScore = -800;
            bool                    GlobalRequired = true;
            // Tagging
            QString                 TaggingSummary;
            QStringList             Tags;
            //! Help for tags
            QHash<QString,QString>  TagsDesc;
            QHash<QString,QString>  TagsArgs;
            //! Where the welcome message is stored
            QString                 WelcomeMP = "Project:Huggle/Message";
            // This is internal only do not prefix it!!
            QList<QRegExp>          _revertPatterns;
            score_ht                BotScore = -200;
            score_ht                WarningScore = 2000;
            QStringList             WarningTypes;
            QString                 SpeedyEditSummary = "Tagging page for deletion";
            QString                 SpeedyWarningSummary = "Sending user a notification regarding deletion of their page";
            QHash<int,QStringList>  AlternativeMonths;
            QList<SpeedyOption>     SpeedyTemplates;
            QStringList             WelcomeTypes;
            long                    WhitelistScore = -800;
            // UAA
            QString                 UAAPath = "Project:Usernames for administrator attention";
            bool                    UAAavailable = false;
            QString                 UAATemplate = "* {{user-uaa|1=$1}} $2 ~~~~";
            //! Suffix used by huggle
            QString                 EditSuffixOfHuggle = "([[WP:HG|HG 3]])";
            //! Regexes that other tools can be identified with
            QStringList             EditRegexOfTools;

        private:
            void Sanitize();
            QHash<QString, QString> cache;
            // We keep the config cached here just in case we needed to ever access it later
            QString                 configurationBuffer;
            YAML::Node              *yaml_node = nullptr;
    };

    inline void ProjectConfiguration::RequestLogin()
    {
        if (this->IsLoggedIn && !this->RequestingLogin)
        {
            this->IsLoggedIn = false;
            this->Token_Csrf = "";
            this->Token_Rollback = "";
            this->Token_Watch = "";
        }
    }
}

#endif // PROJECTCONFIGURATION_H
