//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "projectconfiguration.hpp"
#include "configuration.hpp"
#include "generic.hpp"
#include "exception.hpp"
#include "huggleparser.hpp"
#include "localization.hpp"
#include "syslog.hpp"
#include "version.hpp"
#include "wikipage.hpp"
#include "wikisite.hpp"

// Nasty hack to get yaml-cpp to work on OSX
#undef _l

#include <yaml-cpp/yaml.h>

using namespace Huggle::Generic;
using namespace Huggle;

QList<ProjectConfiguration::SpeedyOption> ProjectConfiguration::Yaml_FetchSpeedyOptions(YAML::Node &node)
{
    QList<ProjectConfiguration::SpeedyOption> results;
    try
    {
        if (!node)
            throw new Huggle::NullPointerException("YAML::Node *node", BOOST_CURRENT_FUNCTION);

        if (!node["speedy-options"])
            return results;

        YAML::Node seq = node["speedy-options"];
        for (YAML::const_iterator it = seq.begin(); it != seq.end(); ++it)
        {
            SpeedyOption speedy_opt;
            speedy_opt.Tag = QString::fromStdString(it->first.as<std::string>());
            YAML::Node s = it->second;
            QStringList options = HuggleParser::YAML2QStringList(s);
            if (options.count() < 3)
            {
                HUGGLE_DEBUG1("Invalid speedy option: " + speedy_opt.Tag);
            } else
            {
                speedy_opt.Info = options[0];
                QString tmp = options[1];
                if (tmp.contains("|"))
                {
                    speedy_opt.Parameter = tmp.mid(tmp.indexOf("|") + 1);
                    speedy_opt.Template = tmp.mid(0, tmp.indexOf("|"));
                } else
                {
                    speedy_opt.Template = tmp;
                }
                speedy_opt.Msg = options[2];
                speedy_opt.Notify = options.size() > 3 && options[3] == "notify";
                results.append(speedy_opt);
            }
        }
        return results;
    } catch (YAML::Exception exception)
    {
        HUGGLE_ERROR("YAML Parsing error (speedy-options): " + QString(exception.what()));
    }
    return results;
}

QHash<QString, int> ProjectConfiguration::Yaml_FetchScoreTags(YAML::Node &node)
{
    try
    {
        if (!node)
            throw new Huggle::NullPointerException("YAML::Node *node", BOOST_CURRENT_FUNCTION);

        if (!node["score-tags"])
            return QHash<QString, int>();

        QHash<QString, int> results;
        YAML::Node seq = node["score-tags"];

        for (YAML::const_iterator it = seq.begin(); it != seq.end(); ++it)
        {
            QString tag = QString::fromStdString(it->first.as<std::string>());
            int score = it->second.as<int>();
            if (results.contains(tag))
            {
                Syslog::HuggleLogs->DebugLog("Multiple definitions of score-tag: " + tag);
                continue;
            }
            results.insert(tag, score);
        }
        return results;
    } catch (YAML::Exception exception)
    {
        HUGGLE_ERROR("YAML Parsing error (score-tags): " + QString(exception.what()));
    }
    return QHash<QString, int>();
}

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
    this->DeletionReasons << "Deleted page using Huggle";
    this->ProjectName = project_name;
    this->SoftwareRevertDefaultSummary = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to"\
            " last revision by $2 using huggle software rollback (reverted by $3 revisions to revision $4)";
}

ProjectConfiguration::~ProjectConfiguration()
{
    delete this->AIVP;
    delete this->UAAP;
    delete this->yaml_node;
}

//! This is just a compatibility hack, to be removed ASAP!!
QStringList temp_compat_hash2list(QHash<QString, QString> hash, bool sort = false)
{
    QStringList result;
    QStringList keys = hash.keys();
    if (sort)
    {
        keys.sort();
    }
    foreach (QString k, keys)
    {
        result.append(k + ";" + hash[k]);
    }
    return result;
}

bool ProjectConfiguration::Parse(QString config, QString *reason, WikiSite *site)
{
    this->configurationBuffer = config;
    this->cache.clear();
    this->UsingYAML = false;
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
    this->ApprovalPage = HuggleParser::ConfigurationParse("userlist", config, this->ApprovalPage);
    this->ReportAIV = HuggleParser::ConfigurationParse("aiv", config);
    this->ReportSection = HuggleParser::ConfigurationParse("aiv-section", config).toInt();
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
        Syslog::HuggleLogs->WarningLog(Huggle::Localizations::HuggleLocalizations->Localize("read-only", site->Name));
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
    this->WarningLevel = static_cast<byte_ht>(HuggleParser::ConfigurationParse("warning-mode", config, "4").toInt());
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
    this->RestoreSummary = HuggleParser::ConfigurationParse("restore-summary", config, this->RestoreSummary);
    this->SpeedyTemplates.clear();
    QStringList speedies = HuggleParser::ConfigurationParse_QL("speedy-options", config);
    foreach (QString speedy, speedies)
    {
        SpeedyOption speedy_option;
        QStringList speedy_parsed = speedy.split(";");
        if (speedy_parsed.count() < 4)
        {
            Huggle::Syslog::HuggleLogs->DebugLog("Invalid csd: " + speedy);
            continue;
        }

        speedy_option.Tag = speedy_parsed[0];
        speedy_option.Info = speedy_parsed[1];
        speedy_option.Template = speedy_parsed[2];
        speedy_option.Msg = speedy_parsed[3];
        speedy_option.Notify = speedy_parsed.count() > 4 && (speedy_parsed[4] == "notify" || speedy_parsed[4] == "notify,");

        this->SpeedyTemplates.append(speedy_option);
    }
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
            Syslog::HuggleLogs->DebugLog("Multiple tag info: " + tag);
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
    this->DeletionReasons = HuggleParser::ConfigurationParseTrimmed_QL("deletion-reasons", config, false);
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
    this->RFPP_Section = static_cast<unsigned int>(HuggleParser::ConfigurationParse("rfpp-section", config, "0").toInt());
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
    this->ReportUserCheckPattern = HuggleParser::ConfigurationParse("report-user-check-pattern", config, this->ReportUserCheckPattern);
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
    this->AIVP = new WikiPage(this->ReportAIV, site);
    HuggleParser::ParsePatterns(config, site);
    HuggleParser::ParseNoTalkPatterns(config, site);
    HuggleParser::ParseNoTalkWords(config, site);
    HuggleParser::ParseWords(config, site);
    if (this->UAAP != nullptr)
        delete this->UAAP;
    this->UAAP = new WikiPage(this->UAAPath, site);
    // templates
    int CurrentTemplate=0;
    while (CurrentTemplate<this->WarningTypes.count())
    {
        QString type = HuggleParser::GetKeyFromSSItem(this->WarningTypes.at(CurrentTemplate));
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

    this->Sanitize();
    return true;
}

bool ProjectConfiguration::ParseYAML(QString yaml_src, QString *reason, WikiSite *site)
{
    this->UsingYAML = true;
    this->configurationBuffer = yaml_src;
    this->cache.clear();
    this->Site = site;

    // Fetch the YAML
    std::string config_std = HuggleParser::FetchYAML(yaml_src).toStdString();
    YAML::Node yaml;
    try
    {
        yaml = YAML::Load(config_std);
    } catch (YAML::Exception exception)
    {
        *reason = QString(exception.what());
        return false;
    }
    if (yaml.IsNull())
    {
        *reason = "document is NULL";
        return false;
    }

    // Make a copy of YAML node so that we can fetch keys later without need to reparse whole config file
    delete this->yaml_node;
    this->yaml_node = new YAML::Node(yaml);

    // Check if version of Huggle is allowed on this wiki
    Version version(HuggleParser::YAML2String("min-version", yaml, "3.0.0"));
    Version huggle_version(HUGGLE_VERSION);

    if (huggle_version < version)
    {
        if (reason)
            *reason = "your huggle is too old, " + this->ProjectName + " supports only " + version.ToString() + " or newer.";
        return false;
    }

    /////////////////////////////////////////////
    // Access control
    /////////////////////////////////////////////
    this->Approval = HuggleParser::YAML2Bool("approval", yaml, false);
    this->ApprovalPage = HuggleParser::YAML2String("userlist", yaml, this->ApprovalPage);
    this->EnableAll = HuggleParser::YAML2Bool("enable-all", yaml, false);
    this->RequireAdmin = HuggleParser::YAML2Bool("require-admin", yaml, this->RequireAdmin);
    this->RequireAutoconfirmed = HuggleParser::YAML2Bool("require-autoconfirmed", yaml, this->RequireAutoconfirmed);
    this->RequireConfig = HuggleParser::YAML2Bool("require-config", yaml, false);
    this->RequireEdits = HuggleParser::YAML2Int("require-edits", yaml);
    this->RequireTime = HuggleParser::YAML2Int("require-time", yaml, this->RequireTime);
    this->RequireRollback = HuggleParser::YAML2Bool("require-rollback", yaml, this->RequireRollback);
    this->ReadOnly = HuggleParser::YAML2Bool("read-only", yaml, this->ReadOnly);
    if (this->ReadOnly)
        Syslog::HuggleLogs->WarningLog(Huggle::Localizations::HuggleLocalizations->Localize("read-only", site->Name));

    //AIV
    this->AIV = HuggleParser::YAML2Bool("aiv-reports", yaml);
    this->ReportAIV = HuggleParser::YAML2String("aiv", yaml);
    this->ReportSection = HuggleParser::YAML2Int("aiv-section", yaml);

    // we use these to understand which format they use on a wiki for dates
    this->Parser_Date_Suffix = HuggleParser::YAML2QStringList("parser-date-suffix", yaml, this->Parser_Date_Suffix);
    this->Parser_Date_Prefix = HuggleParser::YAML2String("parser-date-prefix", yaml, this->Parser_Date_Prefix);
    this->UserlistUpdateSummary = HuggleParser::YAML2String("userlist-update-summary", yaml, this->UserlistUpdateSummary);
    this->UserlistSync = HuggleParser::YAML2Bool("userlistsync", yaml, false);
    this->IPVTemplateReport = HuggleParser::YAML2String("aiv-ip", yaml, "User $1: $2$3 ~~~~");
    this->RUTemplateReport = HuggleParser::YAML2String("aiv-user", yaml, "User $1: $2$3 ~~~~");
    this->ReportDefaultReason = HuggleParser::YAML2String("vandal-report-reason", yaml, "Persistent vandalism and/or "\
                                                                 "unconstructive edits found with [[WP:HG|Huggle 3]].");

    this->LargeRemoval = HuggleParser::YAML2Int("large-removal", yaml, 400);
    // IRC
    this->UseIrc = HuggleParser::YAML2Bool("irc", yaml, this->UseIrc);
    // Ignoring
    this->Ignores = HuggleParser::YAML2QStringList("ignore", yaml);
    if (!this->Ignores.count())
        HUGGLE_DEBUG1(this->ProjectName + " conf: 0 records for ignore");
    this->IgnorePatterns = HuggleParser::YAML2QStringList("ignore-patterns", yaml);
    if (!this->IgnorePatterns.count())
        HUGGLE_DEBUG1(this->ProjectName + " conf: 0 records for ignore-patterns");

    /////////////////////////////////////////////
    // Prediction
    /////////////////////////////////////////////

    // Scoring
    this->WhitelistScore = HuggleParser::YAML2Int("score-wl", yaml, -800);
    this->IPScore = HuggleParser::YAML2Int(ProjectConfig_IPScore_Key, yaml, 800);
    this->ScoreFlag = HuggleParser::YAML2Int("score-flag", yaml);
    this->ForeignUser = HuggleParser::YAML2Int("score-foreign-user", yaml, 200);
    this->BotScore = HuggleParser::YAML2Int("score-bot", yaml, -200000);
    this->ScoreUser = HuggleParser::YAML2Int("score-user", yaml, -200);
    this->ScoreTalk = HuggleParser::YAML2Int("score-talk", yaml, -800);
    this->ScoreRemoval = HuggleParser::YAML2Int("score-remove", yaml, 800);
    this->ScoreTags = ProjectConfiguration::Yaml_FetchScoreTags(yaml);
    this->DefaultSummary = HuggleParser::YAML2String("default-summary", yaml, "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to last revision by $2");
    this->WelcomeSummary = HuggleParser::YAML2String("welcome-summary", yaml, this->WelcomeSummary);
    this->AgfRevert = HuggleParser::YAML2String("agf", yaml, "Reverted good faith edits by [[Special:Contributions/$2|$2]] ([[User talk:$2|talk]]): $1");
    this->EditSuffixOfHuggle = HuggleParser::YAML2String("summary", yaml, "[[Project:Huggle|HG]]") + " (" + HUGGLE_VERSION + ")";
    this->Goto = temp_compat_hash2list(HuggleParser::YAML2QStringHash("go", yaml));
    this->InstantWarnings = HuggleParser::YAML2Bool("warning-im", yaml);
    this->RevertSummaries = temp_compat_hash2list(HuggleParser::YAML2QStringHash("revert-summaries", yaml));
    if (!this->RevertSummaries.count())
    {
        Syslog::HuggleLogs->WarningLog("revert-summaries for " + site->Name + " contain no data, default summary will be used for all of them, you need to fix project settings!!");
    }
    this->RollbackSummary = HuggleParser::YAML2String("rollback-summary", yaml,
                                                      "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to last revision by $2");
    this->SingleRevert = HuggleParser::YAML2String("single-revert-summary", yaml, "Undid edit by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])");
    this->UndoSummary = HuggleParser::YAML2String("undo-summary", yaml);
    this->SoftwareRevertDefaultSummary = HuggleParser::YAML2String("manual-revert-summary", yaml, "Reverted edits by [[Special:Contributions/$1|$1]] to last revision by $2");
    this->MultipleRevertSummary = HuggleParser::YAML2String("multiple-revert-summary-parts", yaml, "Reverted,edit by,edits by,and,other users,to last revision by,to an older version by");
    this->RollbackSummaryUnknownTarget = HuggleParser::YAML2String("rollback-summary-unknown", yaml, "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])");
    QHash<QString, QString> score_level = HuggleParser::YAML2QStringHash("score-level", yaml);
    if (!score_level.isEmpty())
    {
        foreach (QString level, score_level.keys())
        {
            this->ScoreLevel.insert(level.toInt(), score_level[level].toLongLong());
        }
    }

    // Warning types
    this->WarningTypes = temp_compat_hash2list(HuggleParser::YAML2QStringHash("warning-types", yaml), true);
    if (!this->WarningTypes.count())
    {
        if (reason)
            *reason = "warning-types contains no data (no revert summaries)";

        return false;
    }
    this->WarningLevel = static_cast<byte_ht>(HuggleParser::YAML2Int("warning-mode", yaml, 4));
    this->WarningDefs = HuggleParser::YAML2QStringList("warning-template-tags", yaml);
    if (this->WarningDefs.count() == 0)
        Syslog::HuggleLogs->WarningLog("There are no warning tags defined for " + this->ProjectName + " warning parser will not work");
    // Reverting
    this->ConfirmWL = HuggleParser::YAML2Bool("confirm-ignored", yaml, true);
    this->ConfirmMultipleEdits = HuggleParser::YAML2Bool("confirm-multiple", yaml, false);
    this->ConfirmTalk = HuggleParser::YAML2Bool("confirm-talk", yaml, true);
    // this->ConfirmRange = HuggleParser::YAML2Bool("confirm-range", yaml, true);
    // this->ConfirmPage = HuggleParser::YAML2Bool("confirm-page", yaml, "true"));
    // this->ConfirmSame = HuggleParser::YAML2Bool("confirm-same", yaml, "true"));
    this->ConfirmOnSelfRevs = HuggleParser::YAML2Bool("confirm-self-revert", yaml, true);
    // this->ProjectConfig->ConfirmWarned = SafeBool(ConfigurationParse("confirm-warned", config, "true"));
    this->AutomaticallyResolveConflicts = HuggleParser::YAML2Bool("automatically-resolve-conflicts", yaml, false);
    // Welcoming
    this->Welcome = HuggleParser::YAML2String("welcome", yaml);
    this->WelcomeMP = HuggleParser::YAML2String("startup-message-location", yaml, "Project:Huggle/Message");
    this->WelcomeGood = HuggleParser::YAML2Bool("welcome-on-good-edit", yaml, true);
    this->WelcomeAnon = HuggleParser::YAML2String("welcome-anon", yaml, "{{subst:welcome-anon}}");
    this->WelcomeTypes = temp_compat_hash2list(HuggleParser::YAML2QStringHash("welcome-messages", yaml), true);
    // Reporting
    this->SpeedyEditSummary = HuggleParser::YAML2String("speedy-summary", yaml, "Tagging page for deletion");
    this->SpeedyWarningSummary = HuggleParser::YAML2String("speedy-message-summary", yaml, "Notification: [[$1]] has been listed for deletion");
    this->Patrolling = HuggleParser::YAML2Bool("patrolling-enabled", yaml);
    this->PatrollingFlaggedRevs = HuggleParser::YAML2Bool("patrolling-flaggedrevs", yaml, false);
    this->Speedy_EnableWarnings = HuggleParser::YAML2Bool("speedy-enable-warnings", yaml, this->Speedy_EnableWarnings);
    this->Speedy_WarningOnByDefault = HuggleParser::YAML2Bool("speedy-warning-on-by-default", yaml, this->Speedy_WarningOnByDefault);
    // Get report mode
    QString report = HuggleParser::YAML2String("report", yaml);
    if (report.isEmpty())
    {
        HUGGLE_WARNING("Project " + this->Site->Name + " doesn't contain report mode (report), falling back to default");
    } else
    {
        report = report.toLower();
        if (report == "defaultauto")
            this->ReportMode = ReportType_DefaultAuto;
        else if (report == "strictauto")
            this->ReportMode = ReportType_StrictAuto;
        else if (report == "strictmanual")
            this->ReportMode = ReportType_StrictManual;
        else if (report == "defaultmanual")
            this->ReportMode = ReportType_DefaultManual;
        else
            HUGGLE_WARNING("Uknown report mode, falling back to default");
    }
    this->ReportSummary = HuggleParser::YAML2String("report-summary", yaml);
    this->ReportAutoSummary = HuggleParser::YAML2String("report-auto-summary", yaml, "This user was automatically reported by Huggle due to reverted vandalism after four warnings, please verify their"\
                                                                                     " contributions carefully, it may be a false positive");
    this->SpeedyTemplates = Yaml_FetchSpeedyOptions(yaml);
    // Templates
    this->TemplateAge = HuggleParser::YAML2Int("template-age", yaml, this->TemplateAge);
    this->DefaultTemplate = HuggleParser::YAML2String("default-template", yaml, this->DefaultTemplate);
    // UAA
    this->UAAPath = HuggleParser::YAML2String("uaa", yaml);
    this->UAATemplate = HuggleParser::YAML2String("uaa-template", yaml);
    this->TaggingSummary = HuggleParser::YAML2String("tag-summary", yaml, "Tagging page");
    this->Tags.clear();
    QStringList tags_temp = HuggleParser::YAML2QStringList("tags", yaml);
    this->TagsArgs.clear();
    this->TagsDesc.clear();
    foreach (QString item, tags_temp)
    {
        if (item.contains("|"))
        {
            QString pm = item.mid(item.indexOf("|") + 1);
            QString key = item.mid(0, item.indexOf("|"));
            this->Tags.append(key);
            if (!this->TagsArgs.contains(key))
                this->TagsArgs.insert(key, pm);
        } else
        {
            this->Tags.append(item);
        }
    }
    QHash<QString,QHash<QString, QString>> TagsInfo = HuggleParser::YAML2QHashOfHash("tags-info", yaml);
    foreach (QString tag, TagsInfo.keys())
    {
        if (this->TagsDesc.contains(tag))
        {
            Syslog::HuggleLogs->DebugLog("Multiple taginfo: " + tag);
            continue;
        }
        if (!this->Tags.contains(tag))
            this->Tags.append(tag);
        if (TagsInfo[tag].contains("args"))
        {
            if (!this->TagsArgs.contains(tag))
                this->TagsArgs.insert(tag, TagsInfo[tag]["args"]);
        }
        if (TagsInfo[tag].contains("info"))
            this->TagsDesc.insert(tag, TagsInfo[tag]["info"]);
    }
    // Blocking
    this->BlockMessage = HuggleParser::YAML2String("block-message", yaml);
    this->BlockReason = HuggleParser::YAML2String("block-reason", yaml);
    this->BlockExpiryOptions.clear();
    // Feedback
    this->Feedback = HuggleParser::YAML2String("feedback", yaml);
    // Templates
    this->MessageHeadings = HeadingsStandard;
    QString headings = HuggleParser::YAML2String("headings", yaml, "standard");
    if (headings == "page")
    {
        this->MessageHeadings = HeadingsPageName;
        //this->UserConfig->EnforceMonthsAsHeaders = false;
    } else if(headings == "none")
    {
        this->MessageHeadings = HeadingsNone;
        //this->UserConfig->EnforceMonthsAsHeaders = false;
    }
    QString Options = HuggleParser::YAML2String("block-expiry-options", yaml);
    QStringList list = Options.split(",");
    while (list.count() > 0)
    {
        QString item = list.at(0);
        item = item.trimmed();
        this->BlockExpiryOptions.append(item);
        list.removeAt(0);
    }
    this->Tag = HuggleParser::YAML2String("tag", yaml);
    this->DeletionReasons = HuggleParser::YAML2QStringList("deletion-reasons", yaml);
    this->BlockSummary = HuggleParser::YAML2String("block-summary", yaml, "Notification: Blocked");
    this->BlockTime = HuggleParser::YAML2String("blocktime", yaml, "indef");
    this->ClearTalkPageTemp = HuggleParser::YAML2String("template-clear-talk-page", yaml, "{{Huggle/Cleared}}");
    this->Assisted = HuggleParser::YAML2QStringList("assisted-summaries", yaml);
    this->SharedIPTemplateTags = HuggleParser::YAML2String("shared-ip-template-tag", yaml);
    this->SharedIPTemplate = HuggleParser::YAML2String("shared-ip-template", yaml);
    this->ProtectReason =  HuggleParser::YAML2String("protection-reason", yaml, "Excessive [[Wikipedia:Vandalism|vandalism]]");
    this->RevertingEnabled = HuggleParser::YAML2Bool("reverting-enabled", yaml, this->RevertingEnabled);
    this->RFPP_Reason = HuggleParser::YAML2String("protection-request-reason", yaml, this->RFPP_Reason);
    this->RFPP_PlaceTop = HuggleParser::YAML2Bool("protection-request-top", yaml);
    this->RFPP_Regex = HuggleParser::YAML2String("rfpp-verify", yaml);
    this->RFPP_Section = static_cast<unsigned int>(HuggleParser::YAML2Int("rfpp-section", yaml, 0));
    this->RFPP_Page = HuggleParser::YAML2String("protection-request-page", yaml);
    this->RFPP_Template = HuggleParser::YAML2String("rfpp-template", yaml);
    this->TemplateHeader = HuggleParser::YAML2String("template-header", yaml, "Your edits to $1");
    this->RFPP_Mark = HuggleParser::YAML2String("rfpp-mark", yaml);
    this->RFPP_Summary = HuggleParser::YAML2String("protection-request-summary", yaml, "Request to protect page");
    this->RFPP = (this->RFPP_Template.length() && this->RFPP_Regex.length());
    this->RFPP_TemplateUser = HuggleParser::YAML2String("rfpp-template-user", yaml);
    this->RFPP_Temporary = HuggleParser::YAML2String("rfpp-temporary", yaml, this->RFPP_Temporary);
    this->RFPP_Permanent = HuggleParser::YAML2String("rfpp-permanent", yaml, this->RFPP_Permanent);
    this->RestoreSummary = HuggleParser::YAML2String("restore-summary", yaml, this->RestoreSummary);
    this->ReportUserCheckPattern = HuggleParser::YAML2String("report-user-check-patter", yaml, this->ReportUserCheckPattern);
    this->WarningSummaries.clear();
    this->WarningSummaries.insert(1, HuggleParser::YAML2String("warn-summary", yaml, "Message re. [[$1]]"));
    this->WarningSummaries.insert(2, HuggleParser::YAML2String("warn-summary-2", yaml, "Level 2 re. [[$1]]"));
    this->WarningSummaries.insert(3, HuggleParser::YAML2String("warn-summary-3", yaml, "Level 3 re. [[$1]]"));
    this->WarningSummaries.insert(4, HuggleParser::YAML2String("warn-summary-4", yaml, "Level 4 re. [[$1]]"));
    this->GroupTag = HuggleParser::YAML2String("group-tag", yaml, this->GroupTag);

    // Month headers
    QStringList MonthsHeaders_ = HuggleParser::YAML2QStringList("months", yaml);
    if (MonthsHeaders_.count() == 0)
    {
        Syslog::HuggleLogs->WarningLog("Configuration for " + this->ProjectName + " contains 0 months, falling back to English month names!");
    } else if (MonthsHeaders_.count() != 12)
    {
        Syslog::HuggleLogs->WarningLog("Configuration for " + this->ProjectName + " contains " + QString::number(MonthsHeaders_.count()) +
                                       " months, which is weird and I will not use them");
    } else
    {
        this->Months = MonthsHeaders_;
    }

    this->AlternativeMonths.clear();
    QList<QStringList> AMH_ = HuggleParser::YAML2QListOfQStringList("alternative-months", yaml);
    if (!AMH_.count())
    {
        HUGGLE_DEBUG1("Configuration for " + this->ProjectName + " contains 0 alternative months, falling back to English month names!");
    } else if (AMH_.count() != 12)
    {
        Syslog::HuggleLogs->WarningLog("Configuration for " + this->ProjectName + " contains " + QString::number(MonthsHeaders_.count()) +
                                       " alternative month signatures, which is weird and I will not use them");
    } else
    {
        int month_ = 1;
        foreach (QStringList months, AMH_)
        {
            this->AlternativeMonths.insert(month_, months);
            month_++;
        }
    }

    this->RevertPatterns = HuggleParser::YAML2QStringList("revert-patterns", yaml);
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
    (*HuggleQueueFilter::Filters[site]) += HuggleParser::ConfigurationParseQueueList_YAML(yaml, true);
    if (this->AIVP != nullptr)
        delete this->AIVP;
    this->AIVP = new WikiPage(this->ReportAIV, site);
    HuggleParser::ParsePatterns_yaml(yaml, site);
    HuggleParser::ParseNoTalkPatterns_yaml(yaml, site);
    HuggleParser::ParseNoTalkWords_yaml(yaml, site);
    HuggleParser::ParseWords_yaml(yaml, site);
    if (this->UAAP != nullptr)
        delete this->UAAP;
    this->UAAP = new WikiPage(this->UAAPath, site);
    // templates
    int CurrentTemplate=0;
    while (CurrentTemplate<this->WarningTypes.count())
    {
        QString type = HuggleParser::GetKeyFromSSItem(this->WarningTypes.at(CurrentTemplate));
        int CurrentWarning = 1;
        while (CurrentWarning <= 4)
        {
            QString xx = HuggleParser::YAML2String(type + QString::number(CurrentWarning), yaml);
            if (!xx.isEmpty())
            {
                this->WarningTemplates.append(type + QString::number(CurrentWarning) + ";" + xx);
            }
            CurrentWarning++;
        }
        CurrentTemplate++;
    }

    this->Sanitize();
    return true;
}

QString ProjectConfiguration::GetConfig(QString key, QString dv)
{
    if (this->cache.contains(key))
        return this->cache[key];

    QString value;

    if (!this->UsingYAML)
    {
        value = HuggleParser::ConfigurationParse(key, this->configurationBuffer, dv);
    } else
    {
        value = HuggleParser::YAML2String(key, *this->yaml_node, dv);
    }
    this->cache.insert(key, value);
    return value;
}

void ProjectConfiguration::Sanitize()
{
    if (this->ReportAIV.size() == 0)
        this->AIV = false;
    if (this->ScoreLevel.isEmpty())
    {
        this->ScoreLevel.insert(1, 200);
        this->ScoreLevel.insert(2, 400);
        this->ScoreLevel.insert(3, 600);
        this->ScoreLevel.insert(4, 800);
    }
    // Do the same for UAA as well
    this->UAAavailable = this->UAAPath.size() > 0;
    this->IsSane = true;
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
