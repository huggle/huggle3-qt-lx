//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "projectconfiguration.hpp"
#include "generic.hpp"
#include "huggleparser.hpp"
#include "localization.hpp"
#include "syslog.hpp"
#include "version.hpp"
#include "wikipage.hpp"
#include "wikisite.hpp"

using namespace Huggle::Generic;
using namespace Huggle;

ProjectConfiguration::ProjectConfiguration(QString project_name)
{
    // these headers are parsed by project config so don't change them
    // no matter if there is a nice function to retrieve them
    this->Months << "January"
                 << "February"
                 << "March"
                 << "April"
                 << "May"
                 << "June"
                 << "July"
                 << "August"
                 << "September"
                 << "October"
                 << "November"
                 << "December";

    this->Parser_Date_Suffix << "(CET)" << "(UTC)" << "(CEST)";

    // defaults
    this->ProtectReason = "Persistent [[WP:VAND|vandalism]]";
    this->BlockExpiryOptions.append("indefinite");
    this->DeletionSummaries << "Deleted page using Huggle";
    this->ProjectName = project_name;
    this->SoftwareRevertDefaultSummary = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to"\
            " last revision by $2 using huggle software rollback (reverted by $3 revisions to revision $4)";
}

ProjectConfiguration::~ProjectConfiguration()
{
    delete this->AIVP;
    delete this->UAAP;
}

bool ProjectConfiguration::Parse(QString config, QString *reason, WikiSite *site)
{
    this->configurationBuffer = config;
    this->cache.clear();
    Version version(HuggleParser::ConfigurationParse("min-version", config, "3.0.0"));
    Version huggle_version(HUGGLE_VERSION);
    this->Site = site;
    if (huggle_version < version)
    {
        if (reason)
            *reason = "your huggle is too old, " + this->ProjectName + " supports only " + version.ToString() + " or newer.";
        return false;
    }
    this->Approval = SafeBool(HuggleParser::ConfigurationParse("approval", config, "false"));
    //AIV
    this->AIV = SafeBool(HuggleParser::ConfigurationParse("aiv-reports", config));
    this->AIVExtend = SafeBool(HuggleParser::ConfigurationParse("aiv-extend", config));
    this->ApprovalPage = HuggleParser::ConfigurationParse("userlist", config, this->ApprovalPage);
    this->ReportAIV = HuggleParser::ConfigurationParse("aiv", config);
    this->ReportSt = HuggleParser::ConfigurationParse("aiv-section", config).toInt();
    // we use these to understand which format they use on a wiki for dates
    this->Parser_Date_Suffix = HuggleParser::ConfigurationParse_QL("parser-date-suffix", config, this->Parser_Date_Suffix, true);
    this->Parser_Date_Prefix = HuggleParser::ConfigurationParse("parser-date-prefix", config, this->Parser_Date_Prefix);
    this->UserlistUpdateSummary = HuggleParser::ConfigurationParse("userlist-update-summary", config, this->UserlistUpdateSummary);
    this->UserlistSync = SafeBool(HuggleParser::ConfigurationParse("userlistsync", config, "false"));
    this->IPVTemplateReport = HuggleParser::ConfigurationParse("aiv-ip", config, "User $1: $2$3 ~~~~");
    this->RUTemplateReport = HuggleParser::ConfigurationParse("aiv-user", config, "User $1: $2$3 ~~~~");
    this->ReportDefaultReason = HuggleParser::ConfigurationParse("vandal-report-reason", config, "Persistent vandalism and/or "\
                                                                 "unconstructive edits found with [[WP:HG|Huggle 3]].");
    // Restrictions
    this->EnableAll = SafeBool(HuggleParser::ConfigurationParse("enable-all", config));
    this->RequireAdmin = SafeBool(HuggleParser::ConfigurationParse("require-admin", config));
    this->RequireAutoconfirmed = SafeBool(HuggleParser::ConfigurationParse("require-autoconfirmed", config, "false"));
    this->RequireConfig = SafeBool(HuggleParser::ConfigurationParse("require-config", config, "false"));
    this->RequireEdits = HuggleParser::ConfigurationParse("require-edits", config, "0").toInt();
    this->RequireTime = HuggleParser::ConfigurationParse("require-time", config, "0").toInt();
    this->RequireRollback = SafeBool(HuggleParser::ConfigurationParse("require-rollback", config));
    this->ReadOnly = SafeBool(HuggleParser::ConfigurationParse("read-only", config), this->ReadOnly);
    if (this->ReadOnly)
        Syslog::HuggleLogs->WarningLog(_l("read-only", site->Name));
    this->LargeRemoval = HuggleParser::ConfigurationParse("large-removal", config, "400").toInt();
    // IRC
    this->UseIrc = SafeBool(HuggleParser::ConfigurationParse("irc", config));
    // Ignoring
    this->Ignores = HuggleParser::ConfigurationParse_QL("ignore", config, true);
    this->IgnorePatterns = HuggleParser::ConfigurationParse_QL("ignore-patterns", config, true);
    // Scoring
    this->IPScore = HuggleParser::ConfigurationParse(ProjectConfig_IPScore_Key, config, "800").toInt();
    this->ScoreFlag = HuggleParser::ConfigurationParse("score-flag", config).toInt();
    this->ForeignUser = HuggleParser::ConfigurationParse("score-foreign-user", config, "200").toInt();
    this->BotScore = HuggleParser::ConfigurationParse("score-bot", config, "-200000").toInt();
    this->ScoreUser = HuggleParser::ConfigurationParse("score-user", config, "-200").toInt();
    this->ScoreTalk = HuggleParser::ConfigurationParse("score-talk", config, "-800").toInt();
    this->ScoreRemoval = HuggleParser::ConfigurationParse("score-remove", config, "800").toInt();
    QStringList tags = HuggleParser::ConfigurationParseTrimmed_QL("score-tags", config);
    foreach (QString tx, tags)
    {
        QStringList parts = tx.split(";");
        if (parts.count() != 2)
        {
            Syslog::HuggleLogs->DebugLog("Ignoring malformed score-tag: " + tx);
            continue;
        }
        if (this->ScoreTags.contains(parts[0]))
        {
            Syslog::HuggleLogs->DebugLog("Multiple definitions of score-tag " + parts[0]);
            continue;
        }
        this->ScoreTags.insert(parts[0], parts[1].toInt());
    }
    this->DefaultSummary = HuggleParser::ConfigurationParse("default-summary", config,
              "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to last revision by $2");
    this->WelcomeSummary = HuggleParser::ConfigurationParse("welcome-summary", config, this->WelcomeSummary);
    this->AgfRevert = HuggleParser::ConfigurationParse("agf", config, "Reverted good faith edits by [[Special:Contributions/$2|$2]]"\
                                                       " [[User talk:$2|talk]]: $1");
    this->EditSuffixOfHuggle = HuggleParser::ConfigurationParse("summary", config, "[[Project:Huggle|HG]]") + " (" + HUGGLE_VERSION + ")";
    this->Goto = HuggleParser::ConfigurationParse_QL("go", config);
    this->InstantWarnings = SafeBool(HuggleParser::ConfigurationParse("warning-im", config));
    this->RevertSummaries = HuggleParser::ConfigurationParse_QL("template-summ", config);
    if (!this->RevertSummaries.count())
    {
        Syslog::HuggleLogs->WarningLog("RevertSummaries for " + site->Name + " contain no data, default summary will be used for all of them, you need to fix project settings!!");
    }
    this->RollbackSummary = HuggleParser::ConfigurationParse("rollback-summary", config,
              "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to last revision by $2");
    this->SingleRevert = HuggleParser::ConfigurationParse("single-revert-summary", config,
              "Undid edit by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])");
    this->UndoSummary = HuggleParser::ConfigurationParse("undo-summary", config);
    this->SoftwareRevertDefaultSummary = HuggleParser::ConfigurationParse("manual-revert-summary", config,
              "Reverted edits by [[Special:Contributions/$1|$1]] to last revision by $2");
    this->MultipleRevertSummary = HuggleParser::ConfigurationParse("multiple-revert-summary-parts", config,
              "Reverted,edit by,edits by,and,other users,to last revision by,to an older version by");
    this->RollbackSummaryUnknownTarget = HuggleParser::ConfigurationParse("rollback-summary-unknown",
              config, "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])");
    // Warning types
    this->WarningTypes = HuggleParser::ConfigurationParse_QL("warning-types", config);
    if (!this->WarningTypes.count())
    {
        if (reason)
            *reason = "warning-types contains no data (no revert summaries)";

        return false;
    }
    this->WarningLevel = (byte_ht)HuggleParser::ConfigurationParse("warning-mode", config, "4").toInt();
    this->WarningDefs = HuggleParser::ConfigurationParse_QL("warning-template-tags", config);
    if (this->WarningDefs.count() == 0)
        Syslog::HuggleLogs->WarningLog("There are no warning tags defined for " + this->ProjectName + " warning parser will not work");
    // Reverting
    this->ConfirmWL = SafeBool(HuggleParser::ConfigurationParse("confirm-ignored", config, "true"));
    this->ConfirmMultipleEdits = SafeBool(HuggleParser::ConfigurationParse("confirm-multiple", config, ""));
    this->ConfirmTalk = SafeBool(HuggleParser::ConfigurationParse("confirm-talk", config, "true"));
    // this->ConfirmRange = SafeBool(HuggleParser::ConfigurationParse("confirm-range", config, "true"));
    // this->ConfirmPage = SafeBool(HuggleParser::ConfigurationParse("confirm-page", config, "true"));
    // this->ConfirmSame = SafeBool(HuggleParser::ConfigurationParse("confirm-same", config, "true"));
    this->ConfirmOnSelfRevs = SafeBool(HuggleParser::ConfigurationParse("confirm-self-revert", config, "true"));
    // this->ProjectConfig->ConfirmWarned = SafeBool(ConfigurationParse("confirm-warned", config, "true"));
    this->AutomaticallyResolveConflicts = SafeBool(HuggleParser::ConfigurationParse("automatically-resolve-conflicts", config), false);
    // Welcoming
    this->Welcome = HuggleParser::ConfigurationParse("welcome", config);
    this->WelcomeMP = HuggleParser::ConfigurationParse("startup-message-location", config, "Project:Huggle/Message");
    this->WelcomeGood = SafeBool(HuggleParser::ConfigurationParse("welcome-on-good-edit", config, "true"));
    this->WelcomeAnon = HuggleParser::ConfigurationParse("welcome-anon", config, "{{subst:welcome-anon}}");
    this->WelcomeTypes = HuggleParser::ConfigurationParse_QL("welcome-messages", config);
    // Reporting
    this->SpeedyEditSummary = HuggleParser::ConfigurationParse("speedy-summary", config, "Tagging page for deletion");
    this->SpeedyWarningSummary = HuggleParser::ConfigurationParse("speedy-message-summary", config, "Notification: [[$1]] has been listed for deletion");
    this->Patrolling = SafeBool(HuggleParser::ConfigurationParse("patrolling-enabled", config));
    this->PatrollingFlaggedRevs = SafeBool(HuggleParser::ConfigurationParse("patrolling-flaggedrevs", config, "false"));
    this->ReportSummary = HuggleParser::ConfigurationParse("report-summary", config);
    this->ReportAutoSummary = HuggleParser::ConfigurationParse("report-auto-summary", config, "This user was automatically reported by Huggle due to reverted vandalism after four warnings, please verify their"\
                                                                                              " contributions carefully, it may be a false positive");
    this->SpeedyTemplates = HuggleParser::ConfigurationParse_QL("speedy-options", config);
    // Parsing
    this->TemplateAge = HuggleParser::ConfigurationParse("template-age", config, QString::number(this->TemplateAge)).toInt();
    // UAA
    this->UAAPath = HuggleParser::ConfigurationParse("uaa", config);
    this->UAATemplate = HuggleParser::ConfigurationParse("uaa-template", config);
    this->TaggingSummary = HuggleParser::ConfigurationParse("tag-summary", config, "Tagging page");
    this->Tags = HuggleParser::ConfigurationParse_QL("tags", config, true);
    QStringList tags_copy(this->Tags);
    this->TagsArgs.clear();
    this->TagsDesc.clear();
    foreach (QString item, tags_copy)
    {
        if (item.contains("|"))
        {
            QString pm = item.mid(item.indexOf("|") + 1);
            QString key = item.mid(0, item.indexOf("|"));
            int index = this->Tags.indexOf(item);
            this->Tags.removeAt(index);
            this->Tags.insert(index, key);
            if (!this->TagsArgs.contains(key))
                this->TagsArgs.insert(key, pm);
        }
    }
    QStringList TagsInfo = HuggleParser::ConfigurationParse_QL("tags-info", config);
    foreach (QString tag, TagsInfo)
    {
        if (tag.endsWith(","))
            tag = tag.mid(0, tag.size() - 1);
        QStringList info = tag.split(QChar(';'));
        if (info.count() < 2)
        {
            Syslog::HuggleLogs->DebugLog("Ignoring invalid tag info: " + tag);
            continue;
        }
        if (this->TagsDesc.contains(info[0]))
        {
            Syslog::HuggleLogs->DebugLog("Multiple taginfo: " + tag);
            continue;
        }
        if (!this->Tags.contains(info[0]))
            this->Tags.append(info[0]);
        if (!this->TagsArgs.contains(info[0]))
            this->TagsArgs.insert(info[0],info[1]);
        this->TagsDesc.insert(info[0],info[2]);
    }
    // Blocking
    this->WhitelistScore = HuggleParser::ConfigurationParse("score-wl", config, "-800").toInt();
    this->BlockMessage = HuggleParser::ConfigurationParse("block-message", config);
    this->BlockReason = HuggleParser::ConfigurationParse("block-reason", config);
    this->BlockExpiryOptions.clear();
    // Feedback
    this->Feedback = HuggleParser::ConfigurationParse("feedback", config);
    // Templates
    this->MessageHeadings = HeadingsStandard;
    QString headings = HuggleParser::ConfigurationParse("headings", config, "standard");
    if (headings == "page")
    {
        this->MessageHeadings = HeadingsPageName;
        //this->UserConfig->EnforceMonthsAsHeaders = false;
    } else if(headings == "none")
    {
        this->MessageHeadings = HeadingsNone;
        //this->UserConfig->EnforceMonthsAsHeaders = false;
    }
    QString Options = HuggleParser::ConfigurationParse("block-expiry-options", config);
    QStringList list = Options.split(",");
    while (list.count() > 0)
    {
        QString item = list.at(0);
        item = item.trimmed();
        this->BlockExpiryOptions.append(item);
        list.removeAt(0);
    }
    this->Tag = HuggleParser::ConfigurationParse("tag", config);
    this->DeletionSummaries = HuggleParser::ConfigurationParseTrimmed_QL("deletion-reasons", config, false);
    this->BlockSummary = HuggleParser::ConfigurationParse("block-summary", config, "Notification: Blocked");
    this->BlockTime = HuggleParser::ConfigurationParse("blocktime", config, "indef");
    this->ClearTalkPageTemp = HuggleParser::ConfigurationParse("template-clear-talk-page", config, "{{Huggle/Cleared}}");
    this->Assisted = HuggleParser::ConfigurationParse_QL("assisted-summaries", config, true);
    this->SharedIPTemplateTags = HuggleParser::ConfigurationParse("shared-ip-template-tag", config, "");
    this->SharedIPTemplate = HuggleParser::ConfigurationParse("shared-ip-template", config, "");
    this->ProtectReason =  HuggleParser::ConfigurationParse("protection-reason", config, "Excessive [[Wikipedia:Vandalism|vandalism]]");
    this->RevertPatterns = HuggleParser::ConfigurationParse_QL("revert-patterns", config, true);
    this->RevertingEnabled = HuggleParser::ConfigurationParseBool("reverting-enabled", config, true);
    this->RFPP_PlaceTop = SafeBool(HuggleParser::ConfigurationParse("protection-request-top", config));
    this->RFPP_Regex = HuggleParser::ConfigurationParse("rfpp-verify", config);
    this->RFPP_Section = (unsigned int)HuggleParser::ConfigurationParse("rfpp-section", config, "0").toInt();
    this->RFPP_Page = HuggleParser::ConfigurationParse("protection-request-page", config);
    this->RFPP_Template = HuggleParser::ConfigurationParse("rfpp-template", config);
    this->TemplateHeader = HuggleParser::ConfigurationParse("template-header", config, "Your edits to $1");
    this->RFPP_Mark = HuggleParser::ConfigurationParse("rfpp-mark", config);
    this->RFPP_Summary = HuggleParser::ConfigurationParse("protection-request-summary", config, "Request to protect page");
    this->RFPP = (this->RFPP_Template.length() && this->RFPP_Regex.length());
    this->RFPP_TemplateUser = HuggleParser::ConfigurationParse("rfpp-template-user", config);
    this->WarningSummaries.clear();
    this->WarningSummaries.insert(1, HuggleParser::ConfigurationParse("warn-summary", config, "Message re. [[$1]]"));
    this->WarningSummaries.insert(2, HuggleParser::ConfigurationParse("warn-summary-2", config, "Level 2 re. [[$1]]"));
    this->WarningSummaries.insert(3, HuggleParser::ConfigurationParse("warn-summary-3", config, "Level 3 re. [[$1]]"));
    this->WarningSummaries.insert(4, HuggleParser::ConfigurationParse("warn-summary-4", config, "Level 4 re. [[$1]]"));
    QStringList MonthsHeaders_ = HuggleParser::ConfigurationParse_QL("months", config);
    if (MonthsHeaders_.count() != 12)
    {
        Syslog::HuggleLogs->WarningLog("Configuration for this project contains " + QString::number(MonthsHeaders_.count()) +
                                       " months, which is weird and I will not use them");
    } else
    {
        this->Months.clear();
        int i = 0;
        while (i < 12)
        {
            QString month_ = MonthsHeaders_.at(i);
            if (month_.contains(";"))
            {
                month_ = month_.mid(month_.indexOf(";") + 1);
            }
            if (month_.endsWith(','))
            {
                month_ = month_.mid(0, month_.length() - 1);
            }
            this->Months.append(month_);
            i++;
        }
    }
    this->AlternativeMonths.clear();
    QStringList AMH_ = HuggleParser::ConfigurationParse_QL("alternative-months", config);
    int month_ = 1;
    foreach (QString months, AMH_)
    {
        if (months.endsWith(","))
            months = months.mid(0, months.size() - 1);
        this->AlternativeMonths.insert(month_, months.split(';'));
        month_++;
    }
    while (month_ < 13)
    {
        Syslog::HuggleLogs->WarningLog("Project config for " + this->ProjectName + " is missing alternative month names for month "
                                       + QString::number(month_) + " the warning parser may not work properly");
        this->AlternativeMonths.insert(month_, QStringList());
        month_++;
    }
    this->_revertPatterns.clear();
    int xx = 0;
    while (xx < this->RevertPatterns.count())
    {
        this->_revertPatterns.append(QRegExp(this->RevertPatterns.at(xx)));
        xx++;
    }
    if (!HuggleQueueFilter::Filters.contains(site))
    {
        HuggleQueueFilter::Filters.insert(site, new QList<HuggleQueueFilter*>());
    } else
    {
        // we need to delete these
        foreach (HuggleQueueFilter* filter_p, *HuggleQueueFilter::Filters[site])
            delete filter_p;
        // flush
        HuggleQueueFilter::Filters[site]->clear();
    }
    HuggleQueueFilter::Filters[site]->clear();
    HuggleQueueFilter::Filters[site]->append(HuggleQueueFilter::DefaultFilter);
    (*HuggleQueueFilter::Filters[site]) += HuggleParser::ConfigurationParseQueueList(config, true);
    if (this->AIVP != nullptr)
        delete this->AIVP;
    this->AIVP = new WikiPage(this->ReportAIV);
    HuggleParser::ParsePats(config, site);
    HuggleParser::ParseNoTalkPats(config, site);
    HuggleParser::ParseNoTalkWords(config, site);
    HuggleParser::ParseWords(config, site);
    if (this->UAAP != nullptr)
        delete this->UAAP;
    this->UAAP = new WikiPage(this->UAAPath);
    // templates
    int CurrentTemplate=0;
    while (CurrentTemplate<this->WarningTypes.count())
    {
        QString type = HuggleParser::GetKeyFromValue(this->WarningTypes.at(CurrentTemplate));
        int CurrentWarning = 1;
        while (CurrentWarning <= 4)
        {
            QString xx = HuggleParser::ConfigurationParse(type + QString::number(CurrentWarning), config);
            if (!xx.isEmpty())
            {
                this->WarningTemplates.append(type + QString::number(CurrentWarning) + ";" + xx);
            }
            CurrentWarning++;
        }
        CurrentTemplate++;
    }
    // sanitize
    if (this->ReportAIV.size() == 0)
        this->AIV = false;
    // Do the same for UAA as well
    this->UAAavailable = this->UAAPath.size() > 0;
    this->IsSane = true;
    return true;
}

QString ProjectConfiguration::GetConfig(QString key, QString dv)
{
    if (this->cache.contains(key))
        return this->cache[key];

    QString value = HuggleParser::ConfigurationParse(key, this->configurationBuffer, dv);
    this->cache.insert(key, value);
    return value;
}

QDateTime ProjectConfiguration::ServerTime()
{
    return QDateTime::currentDateTime().addSecs(this->ServerOffset);
}

ScoreWord::ScoreWord(QString Word, int Score)
{
    this->score = Score;
    this->word = Word.toLower();
}

ScoreWord::ScoreWord(ScoreWord *word)
{
    this->score = word->score;
    this->word = word->word;
}

ScoreWord::ScoreWord(const ScoreWord &word)
{
    this->score = word.score;
    this->word = word.word;
}
