//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "userconfiguration.hpp"
#include "generic.hpp"
#include "configuration.hpp"
#include "exception.hpp"
#include "hooks.hpp"
#include "huggleoption.hpp"
#include "huggleparser.hpp"
#include "syslog.hpp"
#include "localization.hpp"
#include "projectconfiguration.hpp"
#include "version.hpp"
#include <QKeySequence>

// Nasty hack to get yaml-cpp to work on OSX
#define _hl Huggle::Localizations::HuggleLocalizations->Localize
#undef _l

#include <string>
#include <yaml-cpp/yaml.h>

using namespace Huggle;
using namespace Huggle::Generic;
using namespace Huggle::HuggleParser;

static QString Bool2ExcludeRequire(HuggleQueueFilterMatch match)
{
    switch (match)
    {
        case HuggleQueueFilterMatchRequire:
            return "require";
        case HuggleQueueFilterMatchExclude:
            return "exclude";
        case HuggleQueueFilterMatchIgnore:
            return "ignore";
    }
    throw Huggle::Exception("Invalid enum", BOOST_CURRENT_FUNCTION);
}

WatchlistOption UserConfiguration::WatchlistOptionFromString(QString string)
{
    if (string == "nochange")
        return WatchlistOption_NoChange;
    if (string == "preferences")
        return WatchlistOption_Preferences;
    if (string == "unwatch")
        return WatchlistOption_Unwatch;
    if (string == "watch")
        return WatchlistOption_Watch;

    return WatchlistOption_Preferences;
}

QString UserConfiguration::WatchListOptionToString(WatchlistOption option)
{
    switch (option)
    {
        case WatchlistOption_NoChange:
            return "nochange";
        case WatchlistOption_Preferences:
            return "preferences";
        case WatchlistOption_Unwatch:
            return "unwatch";
        case WatchlistOption_Watch:
            return "watch";
    }

    return "preferences";
}

Huggle::UserConfiguration::UserConfiguration()
{
    this->Previous_Version = new Version(HUGGLE_VERSION);
}

Huggle::UserConfiguration::~UserConfiguration()
{
    delete this->Previous_Version;
    delete this->yaml_node;
    QStringList ol = this->UserOptions.keys();
    while (ol.count())
    {
        HuggleOption *option = this->UserOptions[ol[0]];
        this->UserOptions.remove(ol[0]);
        delete option;
        ol.removeAt(0);
    }
}

HuggleOption *UserConfiguration::GetOption(QString key)
{
    if (this->UserOptions.contains(key))
    {
        return this->UserOptions[key];
    }
    return nullptr;
}

QVariant UserConfiguration::SetOption(QString key, QString config, QVariant def)
{
    if (this->UserOptions.contains(key))
    {
        // we must not add 2 same
        throw new Huggle::Exception("This option is already in a list you can't have multiple same keys in it", BOOST_CURRENT_FUNCTION);
    }
    QString d_ = def.toString();
    QString value = ConfigurationParse(key, config, d_);
    HuggleOption *h;
    switch (def.type())
    {
        case QVariant::Int:
            h = new HuggleOption(key, value.toInt(), value == d_);
            break;
        case QVariant::Bool:
            h = new HuggleOption(key, Generic::SafeBool(value), value == d_);
            break;
        default:
            h = new HuggleOption(key, value, value == d_);
            break;
    }
    this->UserOptions.insert(key, h);
    return h->GetVariant();
}

QVariant UserConfiguration::SetOptionYAML(QString key, YAML::Node &config, QVariant def)
{
    if (this->UserOptions.contains(key))
    {
        // we must not add 2 same
        throw new Huggle::Exception("This option is already in a list you can't have multiple same keys in it", BOOST_CURRENT_FUNCTION);
    }
    QString default_str = def.toString();
    QString value = YAML2String(key, config, default_str);
    HuggleOption *h;
    switch (def.type())
    {
        case QVariant::Int:
            h = new HuggleOption(key, value.toInt(), value == default_str);
            break;
        case QVariant::Bool:
            h = new HuggleOption(key, Generic::SafeBool(value), value == default_str);
            break;
        default:
            h = new HuggleOption(key, value, value == default_str);
            break;
    }
    this->UserOptions.insert(key, h);
    return h->GetVariant();
}

QString SanitizeString(QString s)
{
    s.replace("\\", "\\\\");
    s.replace("\"", "\\\"");
    return s;
}

void AppendConf(QString *conf, QString key, QString value)
{
    conf->append(key + ": \"" + SanitizeString(value) + "\"\n");
}

void AppendConf(QString *conf, QString key, bool value)
{
    conf->append(key + ": " + Bool2String(value) + "\n");
}

void AppendConf(QString *conf, QString key, int value)
{
    conf->append(key + ": " + QString::number(value) + "\n");
}

void AppendConf(QString *conf, QString key, unsigned int value)
{
    conf->append(key + ": " + QString::number(value) + "\n");
}

void AppendComment(QString *conf, QString text)
{
    conf->append("# // " + text + "\n");
}

void AppendConf(QString *conf, QString key, long long value)
{
    conf->append(key + ": " + QString::number(value) + "\n");
}

QString UserConfiguration::MakeLocalUserConfig(ProjectConfiguration *Project)
{
    QString configuration = "---\n# // This is a configuration of huggle, do not change it unless you know what you are doing.\n";
    AppendConf(&configuration, "enable", true);
    AppendComment(&configuration, "Last version of huggle that wrote into this configuration file (sanity check)");
    AppendConf(&configuration, "version", hcfg->HuggleVersion);
    AppendConf(&configuration, "report-summary", Project->ReportSummary);
    AppendConf(&configuration, "confirm-multiple", Project->ConfirmMultipleEdits);
    AppendConf(&configuration, "confirm-talk", Project->ConfirmTalk);
    AppendConf(&configuration, "confirm-self-revert", Project->ConfirmOnSelfRevs);
    AppendConf(&configuration, "confirm-whitelist", Project->ConfirmWL);
    // Save this only if it differs from project config so that project config can change in future and it's reflected
    if (Project->DefaultSummary != this->DefaultSummary)
        AppendConf(&configuration, "default-summary", this->DefaultSummary);
    if (Project->RollbackSummary != this->RollbackSummary)
        AppendConf(&configuration, "rollback-summary", this->RollbackSummary);
    if (Project->RollbackSummaryUnknownTarget != this->RollbackSummaryUnknownTarget)
        AppendConf(&configuration, "rollback-summary-unknown", this->RollbackSummaryUnknownTarget);
    AppendComment(&configuration, "This option will change the behaviour of automatic resolution, be carefull");
    AppendConf(&configuration, "revert-on-multiple-edits", this->RevertOnMultipleEdits);
    AppendConf(&configuration, "automatically-resolve-conflicts", this->AutomaticallyResolveConflicts);
    AppendConf(&configuration, "software-rollback", this->EnforceManualSoftwareRollback);
    AppendConf(&configuration, "enforce-months-as-headers", this->EnforceMonthsAsHeaders);
    AppendConf(&configuration, "history-load", this->HistoryLoad);
    AppendConf(&configuration, "on-next", static_cast<int>(this->GoNext));
    AppendConf(&configuration, "delete-edits-after-revert", this->DeleteEditsAfterRevert);
    AppendConf(&configuration, "skip-to-last-edit", this->LastEdit);
    AppendConf(&configuration, "preferred-provider", this->PreferredProvider);
    AppendConf(&configuration, "remove-oldest-queue-edits", this->RemoveOldQueueEdits);
    AppendConf(&configuration, "truncate-edits", this->TruncateEdits);
    AppendConf(&configuration, "talkpage-freshness", this->TalkPageFreshness);
    AppendConf(&configuration, "remove-after-trusted-edit", this->RemoveAfterTrustedEdit);
    AppendComment(&configuration, "Get original creator of every page so that you can G7 instead of reverting the page");
    AppendConf(&configuration, "retrieve-founder", this->RetrieveFounder);
    AppendConf(&configuration, "display-title", this->DisplayTitle);
    AppendComment(&configuration, "Periodically check if you received new messages and display a notification box if you get them");
    AppendConf(&configuration, "check-tp", this->CheckTP);
    AppendConf(&configuration, "manual-warning", this->ManualWarning);
    AppendConf(&configuration, "summary-mode", this->HighlightSummaryIfExists);
    AppendConf(&configuration, "automatic-reports", this->AutomaticReports);
    AppendComment(&configuration, "HAN");
    AppendConf(&configuration, "han-html", hcfg->UserConfig->HtmlAllowedInIrc);
    AppendConf(&configuration, "han-display-user-talk", this->HAN_DisplayUserTalk);
    AppendConf(&configuration, "han-display-bots", this->HAN_DisplayBots);
    AppendConf(&configuration, "han-display-user", this->HAN_DisplayUser);
    AppendConf(&configuration, "watchlist", WatchListOptionToString(this->Watchlist));
    AppendComment(&configuration, "Whether edits made by same user should be grouped up together in page");
    AppendConf(&configuration, "automatically-group", this->AutomaticallyGroup);
    AppendConf(&configuration, "queue-id", this->QueueID);
    AppendComment(&configuration, "Location of page (wiki page name, for example WP:Huggle) that should be displayed when you hit next and queue is empty. Leave empty for default page.");
    AppendConf(&configuration, "page-empty-queue", this->PageEmptyQueue);
    AppendConf(&configuration, "enable-max-score", this->EnableMaxScore);
    AppendConf(&configuration, "max-score", this->MaxScore);
    AppendConf(&configuration, "enable-min-score", this->EnableMinScore);
    AppendConf(&configuration, "min-score", this->MinScore);
    AppendConf(&configuration, "automatic-refresh", this->AutomaticRefresh);
    AppendConf(&configuration, "automatically-watchlist-warned-users", this->AutomaticallyWatchlistWarnedUsers);
    AppendConf(&configuration, "shortcut-hash", this->ShortcutHash);
    AppendConf(&configuration, "show-warning-if-not-on-last-revision", this->ShowWarningIfNotOnLastRevision);
    AppendConf(&configuration, "number-dropdown-menu-items", this->NumberDropdownMenuItems);
    AppendConf(&configuration, "insert-edits-of-rolled-user-to-queue", this->InsertEditsOfRolledUserToQueue);
    AppendComment(&configuration, "If true you will not warn users who received a warning recently");
    AppendConf(&configuration, "confirm-on-recent-warning", this->ConfirmOnRecentWarning);
    AppendComment(&configuration, "If warning was sent less than N seconds ago it's considered too recent");
    AppendConf(&configuration, "recent-warning-time-span", this->RecentWarningTimeSpan);
    AppendConf(&configuration, "confirm-warning-on-very-old-edits", this->ConfirmWarningOnVeryOldEdits);
    // shortcuts
    QStringList shortcuts = Configuration::HuggleConfiguration->Shortcuts.keys();
    // we need to do this otherwise huggle may sort the items differently every time and spam wiki
    shortcuts.sort();
    int modified_ = 0;
    QString si = "";
    foreach (QString key, shortcuts)
    {
        Shortcut s_ = Configuration::HuggleConfiguration->Shortcuts[key];
        if (s_.Modified)
        {
            si += "    '" + s_.Name + "': '" + s_.QAccel + "'\n";
            modified_++;
        }
    }
    if (si.endsWith(",\n"))
    {
        // remove the extra comma on end
        si = si.mid(0, si.length() - 2);
        si += "\n";
    }
    if (modified_)
        configuration += "shortcut-list:\n" + si + "\n";
    QStringList kl = this->UserOptions.keys();
    kl.sort();
    foreach (QString item, kl)
    {
        HuggleOption *option = this->GetOption(item);
        if (option == nullptr)
        {
            // this must never happen
            throw new Huggle::NullPointerException("HuggleOption *option", BOOST_CURRENT_FUNCTION);
        }
        if (!option->IsDefault())
        {
            if (option->GetVariant().type() != QVariant::StringList)
            {
                // in case we modified this item we store it
                AppendConf(&configuration, item, option->GetVariant().toString());
            } else
            {
                QStringList list = option->GetVariant().toStringList();
                configuration += item + ":\n";
                int x = 0;
                while (x < list.count())
                {
                    if (x+1 < list.count())
                    {
                        configuration += "    - '" + list.at(x) + "'\n";
                    } else
                    {
                        configuration += "    - '" + list.at(x) + "'\n\n";
                    }
                    x++;
                }
            }
        }
    }
    configuration += "queues:\n";
    int c = 0;
    if (!HuggleQueueFilter::Filters.contains(Project->Site))
        throw new Huggle::Exception("There is no such a site in queue filters", BOOST_CURRENT_FUNCTION);
    while (c < HuggleQueueFilter::Filters[Project->Site]->count())
    {
        HuggleQueueFilter *fltr = HuggleQueueFilter::Filters[Project->Site]->at(c);
        c++;
        if (fltr->IsChangeable())
        {
            configuration += "    \"" + SanitizeString(fltr->QueueName) + "\":\n";
            configuration += "        filter-ignored: \"" + Bool2ExcludeRequire(fltr->getIgnoreWL()) + "\"\n";
            configuration += "        filter-bots: \"" + Bool2ExcludeRequire(fltr->getIgnoreBots()) + "\"\n";
            configuration += "        filter-assisted: \"" + Bool2ExcludeRequire(fltr->getIgnoreFriends()) + "\"\n";
            configuration += "        filter-ip: \"" + Bool2ExcludeRequire(fltr->getIgnoreIP()) + "\"\n";
            configuration += "        filter-minor: \"" + Bool2ExcludeRequire(fltr->getIgnoreMinor()) + "\"\n";
            configuration += "        filter-new-pages: \"" + Bool2ExcludeRequire(fltr->getIgnoreNP()) + "\"\n";
            configuration += "        filter-me: \"" + Bool2ExcludeRequire(fltr->getIgnoreSelf()) + "\"\n";
            configuration += "        nsfilter-user: \"" + Bool2ExcludeRequire(fltr->getIgnore_UserSpace()) + "\"\n";
            configuration += "        filter-talk: \"" + Bool2ExcludeRequire(fltr->getIgnoreTalk()) + "\"\n";
            configuration += "        filter-watched: \"" + Bool2ExcludeRequire(fltr->getIgnoreWatched()) + "\"\n";
            configuration += "        filter-reverts: \"" + Bool2ExcludeRequire(fltr->getIgnoreReverts()) + "\"\n";
            configuration += "        ignored-tags: \"" + SanitizeString(fltr->GetIgnoredTags_CommaSeparated()) + "\"\n";
            configuration += "        required-tags: \"" + SanitizeString(fltr->GetRequiredTags_CommaSeparated()) + "\"\n";
            configuration += "        ignored-categories: \"" + SanitizeString(fltr->GetIgnoredCategories_CommaSeparated()) + "\"\n";
            configuration += "        required-categories: \"" + SanitizeString(fltr->GetRequiredCategories_CommaSeparated()) + "\"\n";
            QString ns = "";
            QList<int> filter_keys = fltr->Namespaces.keys();
            qSort(filter_keys);
            foreach (int nsid, filter_keys)
            {
                if (fltr->IgnoresNS(nsid))
                    ns += QString::number(nsid) + ",";
            }
            if (!ns.isEmpty())
                configuration += "        filtered-ns: \"" + SanitizeString(ns) + "\"\n";
            configuration += "\n";
        }
    }
    return configuration;
}

bool UserConfiguration::EnforceSoftwareRollback()
{
    return this->EnforceManualSoftwareRollback || this->EnforceManualSRT;
}

QStringList UserConfiguration::SetUserOptionList(QString key, QString config, QStringList def, bool CS)
{
    if (this->UserOptions.contains(key))
    {
        // we must not add 2 same
        throw new Huggle::Exception("This option is already in a list you can't have multiple same keys in it", BOOST_CURRENT_FUNCTION);
    }
    QStringList value = HuggleParser::ConfigurationParse_QL(key, config, def, CS);
    HuggleOption *h = new HuggleOption(key, value, value == def);
    this->UserOptions.insert(key, h);
    return value;
}

QStringList UserConfiguration::SetUserOptionListYAML(QString key, YAML::Node &config, QStringList def)
{
    if (this->UserOptions.contains(key))
    {
        // we must not add 2 same
        throw new Huggle::Exception("This option is already in a list you can't have multiple same keys in it", BOOST_CURRENT_FUNCTION);
    }
    QStringList value = YAML2QStringList(key, config, def);
    HuggleOption *h = new HuggleOption(key, value, value == def);
    this->UserOptions.insert(key, h);
    return value;
}

bool UserConfiguration::Parse(QString config, ProjectConfiguration *ProjectConfig, bool IsHome)
{
    this->RevertOnMultipleEdits = SafeBool(ConfigurationParse("RevertOnMultipleEdits", config));
    ProjectConfig->EnableAll = SafeBool(ConfigurationParse("enable", config));
    ProjectConfig->Ignores = HuggleParser::ConfigurationParse_QL("ignore", config, ProjectConfig->Ignores);
    // this is a hack so that we can access this value more directly, it can't be changed in huggle
    // so there is no point in using a hash for it
    ProjectConfig->IPScore = this->SetOption(ProjectConfig_IPScore_Key, config, ProjectConfig->IPScore).toLongLong();
    ProjectConfig->ScoreFlag = this->SetOption("score-flag", config, ProjectConfig->ScoreFlag).toLongLong();
    this->EnforceManualSoftwareRollback = SafeBool(ConfigurationParse("software-rollback", config));
    if (ProjectConfig->WarningSummaries.contains(1))
        ProjectConfig->WarningSummaries[1] = this->SetOption("warn-summary", config, ProjectConfig->WarningSummaries[1]).toString();
    if (ProjectConfig->WarningSummaries.contains(2))
        ProjectConfig->WarningSummaries[2] = this->SetOption("warn-summary-2", config, ProjectConfig->WarningSummaries[2]).toString();
    if (ProjectConfig->WarningSummaries.contains(3))
        ProjectConfig->WarningSummaries[3] = this->SetOption("warn-summary-3", config, ProjectConfig->WarningSummaries[3]).toString();
    if (ProjectConfig->WarningSummaries.contains(4))
        ProjectConfig->WarningSummaries[4] = this->SetOption("warn-summary-4", config, ProjectConfig->WarningSummaries[4]).toString();
    this->AutomaticallyResolveConflicts = SafeBool(ConfigurationParse("automatically-resolve-conflicts", config), false);
    this->DefaultSummary = HuggleParser::ConfigurationParse("default-summary", config, ProjectConfig->DefaultSummary);
    this->RollbackSummary = HuggleParser::ConfigurationParse("rollback-summary", config, ProjectConfig->RollbackSummary);
    this->RollbackSummaryUnknownTarget = HuggleParser::ConfigurationParse("rollback-summary-unknown", config, ProjectConfig->RollbackSummaryUnknownTarget);
    ProjectConfig->TemplateAge = this->SetOption("template-age", config, ProjectConfig->TemplateAge).toInt();
    ProjectConfig->RevertSummaries = this->SetUserOptionList("template-summ", config, ProjectConfig->RevertSummaries);
    ProjectConfig->WarningTypes = this->SetUserOptionList("warning-types", config, ProjectConfig->WarningTypes);
    ProjectConfig->ScoreChange = this->SetOption("score-change", config, ProjectConfig->ScoreChange).toLongLong();
    ProjectConfig->ScoreUser = this->SetOption("score-user", config, ProjectConfig->ScoreUser).toLongLong();
    this->HighlightSummaryIfExists = SafeBool(ConfigurationParse("SummaryMode", config), this->HighlightSummaryIfExists);
    ProjectConfig->ScoreTalk = this->SetOption("score-talk", config, ProjectConfig->ScoreTalk).toLongLong();
    ProjectConfig->WarningDefs = this->SetUserOptionList("warning-template-tags", config, ProjectConfig->WarningDefs);
    ProjectConfig->BotScore = this->SetOption("score-bot", config, ProjectConfig->BotScore).toLongLong();
    if (!HuggleQueueFilter::Filters.contains(ProjectConfig->Site))
        throw new Huggle::Exception("There is no such a wiki", BOOST_CURRENT_FUNCTION);
    (*HuggleQueueFilter::Filters[ProjectConfig->Site]) += HuggleParser::ConfigurationParseQueueList(config, false);
    ProjectConfig->ConfirmMultipleEdits = SafeBool(ConfigurationParse("confirm-multiple", config), ProjectConfig->ConfirmMultipleEdits);
    ProjectConfig->ConfirmTalk = SafeBool(ConfigurationParse("confirm-talk", config), ProjectConfig->ConfirmTalk);
    ProjectConfig->ConfirmOnSelfRevs = SafeBool(ConfigurationParse("confirm-self-revert", config), ProjectConfig->ConfirmOnSelfRevs);
    ProjectConfig->ConfirmWL = SafeBool(ConfigurationParse("confirm-whitelist", config), ProjectConfig->ConfirmWL);
    this->DisplayTitle = SafeBool(ConfigurationParse("DisplayTitle", config), this->DisplayTitle);
    this->TruncateEdits = SafeBool(ConfigurationParse("TruncateEdits", config), this->TruncateEdits);
    this->HistoryLoad = SafeBool(ConfigurationParse("HistoryLoad", config), this->HistoryLoad);
    this->LastEdit = SafeBool(ConfigurationParse("SkipToLastEdit", config), this->LastEdit);
    this->PreferredProvider = ConfigurationParse("PreferredProvider", config, QString::number(this->PreferredProvider)).toInt();
    this->CheckTP = SafeBool(ConfigurationParse("CheckTP", config), this->CheckTP);
    this->RetrieveFounder = SafeBool(ConfigurationParse("RetrieveFounder", config, Bool2String(this->RetrieveFounder)));
    this->HAN_DisplayBots = SafeBool(ConfigurationParse("HAN_DisplayBots", config, Bool2String(this->HAN_DisplayBots)));
    this->HAN_DisplayUser = SafeBool(ConfigurationParse("HAN_DisplayUser", config, Bool2String(this->HAN_DisplayUser)));
    this->ManualWarning = SafeBool(ConfigurationParse("ManualWarning", config, Bool2String(this->ManualWarning)));
    this->RemoveAfterTrustedEdit = SafeBool(ConfigurationParse("RemoveAfterTrustedEdit", config), this->RemoveAfterTrustedEdit);
    this->HAN_DisplayUserTalk = SafeBool(ConfigurationParse("HAN_DisplayUserTalk", config), this->HAN_DisplayUserTalk);
    this->HtmlAllowedInIrc = SafeBool(ConfigurationParse("HAN_Html", config), this->HtmlAllowedInIrc);
    this->Watchlist = WatchlistOptionFromString(ConfigurationParse("Watchlist", config));
    this->AutomaticallyGroup = SafeBool(ConfigurationParse("AutomaticallyGroup", config), this->AutomaticallyGroup);
    this->TalkPageFreshness = ConfigurationParse("TalkpageFreshness", config, QString::number(this->TalkPageFreshness)).toUInt();
    this->RemoveOldQueueEdits = SafeBool(ConfigurationParse("RemoveOldestQueueEdits", config), this->RemoveOldQueueEdits);
    this->QueueID = ConfigurationParse("QueueID", config);
    this->GoNext = static_cast<Configuration_OnNext>(ConfigurationParse("OnNext", config, "1").toInt());
    this->DeleteEditsAfterRevert = SafeBool(ConfigurationParse("DeleteEditsAfterRevert", config), this->DeleteEditsAfterRevert);
    this->WelcomeGood = this->SetOption("welcome-good", config, ProjectConfig->WelcomeGood).toBool();
    this->AutomaticReports = SafeBool(ConfigurationParse("AutomaticReports", config), this->AutomaticReports);
    this->PageEmptyQueue = HuggleParser::ConfigurationParse("PageEmptyQueue", config);
    delete this->Previous_Version;
    this->Previous_Version = new Version(ConfigurationParse("version", config, HUGGLE_VERSION));
    this->AutomaticallyWatchlistWarnedUsers = HuggleParser::ConfigurationParseBool("AutomaticallyWatchlistWarnedUsers", config, this->AutomaticallyWatchlistWarnedUsers);
    // Score range
    this->EnableMaxScore = SafeBool(ConfigurationParse("EnableMaxScore", config));
    this->MinScore = ConfigurationParse("MinScore", config, "0").toLongLong();
    this->MaxScore = ConfigurationParse("MaxScore", config, "0").toLongLong();
    this->EnableMinScore = SafeBool(ConfigurationParse("EnableMinScore", config));
    this->AutomaticRefresh = SafeBool(ConfigurationParse("AutomaticRefresh", config), this->AutomaticRefresh);
    this->ShortcutHash = ConfigurationParse("ShortcutHash", config, "null");
    this->ShowWarningIfNotOnLastRevision = SafeBool(ConfigurationParse("ShowWarningIfNotOnLastRevision", config), this->ShowWarningIfNotOnLastRevision);
    // for now we do this only for home wiki but later we need to make it for every wiki
    if (IsHome)
    {
        // Verify a shortcut hash first
        QString types = "";
        QStringList sorted_types = ProjectConfig->WarningTypes;
        sorted_types.sort();
        foreach (QString t, sorted_types)
            types += t + ",";
        QString hash = Generic::MD5(types);
        if (hash != this->ShortcutHash)
        {
            // Suppress this warning in case there is no record yet, this is first run of Huggle by new user
            if (!this->ShortcutHash.isEmpty() && this->ShortcutHash != "null")
                Hooks::ShowWarning(_hl("warning"), _hl("config-reset-menu"));
            hcfg->ResetMenuShortcuts();
            this->ShortcutHash = hash;
            return true;
        }
        QStringList shortcuts = HuggleParser::ConfigurationParse_QL("ShortcutList", config, true);
        foreach (QString line, shortcuts)
        {
            if (!line.contains(";"))
            {
                Syslog::HuggleLogs->WarningLog("Invalid line in user configuration (shortcuts): " + line);
                continue;
            }
            QStringList parts = line.split(';');
            QString id = parts[0];
            if (!hcfg->Shortcuts.contains(id))
            {
                Syslog::HuggleLogs->WarningLog("Invalid shortcut in user configuration (missing id): " + line);
                continue;
            }
            hcfg->Shortcuts[id].Modified = true;
            hcfg->Shortcuts[id].QAccel = QKeySequence(parts[1]).toString();
        }
    }

    // Sanitize
    if (this->DefaultSummary.isEmpty())
        this->DefaultSummary = ProjectConfig->DefaultSummary;
    if (this->RollbackSummary.isEmpty())
        this->RollbackSummary = ProjectConfig->RollbackSummary;
    if (this->RollbackSummaryUnknownTarget.isEmpty())
        this->RollbackSummaryUnknownTarget = ProjectConfig->RollbackSummaryUnknownTarget;

    return true;
}

bool UserConfiguration::ParseYAML(QString config, ProjectConfiguration *ProjectConfig, bool IsHome, QString *reason)
{
    // Fetch the YAML
    std::string config_std = HuggleParser::FetchYAML(config).toStdString();
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

    this->RevertOnMultipleEdits = YAML2Bool("revert-on-multiple-edits", yaml, this->RevertOnMultipleEdits);
    // In case that user has "enable: false" in their config page, huggle is disabled for them, so let's override the Project config here
    if (!YAML2Bool("enable", yaml, true))
        ProjectConfig->EnableAll = false;
    ProjectConfig->Ignores = YAML2QStringList("ignore", yaml, ProjectConfig->Ignores);
    // SetOption functions allow us to preserve information whether the value was changed or is default, this is there because some options
    // which are in project config can be overloaded by user config, but we only want to store them in user config if they were modified
    // by user, so that by default we are always using the project config version (unless user wanted to modify them)
    ProjectConfig->IPScore = this->SetOptionYAML(ProjectConfig_IPScore_Key, yaml, ProjectConfig->IPScore).toLongLong();
    ProjectConfig->ScoreFlag = this->SetOptionYAML("score-flag", yaml, ProjectConfig->ScoreFlag).toLongLong();
    this->EnforceManualSoftwareRollback = YAML2Bool("software-rollback", yaml, this->EnforceManualSoftwareRollback);
    if (ProjectConfig->WarningSummaries.contains(1))
        ProjectConfig->WarningSummaries[1] = this->SetOptionYAML("warn-summary", yaml, ProjectConfig->WarningSummaries[1]).toString();
    if (ProjectConfig->WarningSummaries.contains(2))
        ProjectConfig->WarningSummaries[2] = this->SetOptionYAML("warn-summary-2", yaml, ProjectConfig->WarningSummaries[2]).toString();
    if (ProjectConfig->WarningSummaries.contains(3))
        ProjectConfig->WarningSummaries[3] = this->SetOptionYAML("warn-summary-3", yaml, ProjectConfig->WarningSummaries[3]).toString();
    if (ProjectConfig->WarningSummaries.contains(4))
        ProjectConfig->WarningSummaries[4] = this->SetOptionYAML("warn-summary-4", yaml, ProjectConfig->WarningSummaries[4]).toString();
    this->AutomaticallyResolveConflicts = YAML2Bool("automatically-resolve-conflicts", yaml, this->AutomaticallyResolveConflicts);
    this->DefaultSummary = YAML2String("default-summary", yaml, ProjectConfig->DefaultSummary);
    this->RollbackSummary = YAML2String("rollback-summary", yaml, ProjectConfig->RollbackSummary);
    this->RollbackSummaryUnknownTarget = YAML2String("rollback-summary-unknown", yaml, ProjectConfig->RollbackSummaryUnknownTarget);
    ProjectConfig->TemplateAge = this->SetOptionYAML("template-age", yaml, ProjectConfig->TemplateAge).toInt();
    ProjectConfig->RevertSummaries = this->SetUserOptionListYAML("template-summ", yaml, ProjectConfig->RevertSummaries);
    ProjectConfig->WarningTypes = this->SetUserOptionListYAML("warning-types", yaml, ProjectConfig->WarningTypes);
    ProjectConfig->ScoreChange = this->SetOptionYAML("score-change", yaml, ProjectConfig->ScoreChange).toLongLong();
    ProjectConfig->ScoreUser = this->SetOptionYAML("score-user", yaml, ProjectConfig->ScoreUser).toLongLong();
    this->HighlightSummaryIfExists = YAML2Bool("summary-mode", yaml, this->HighlightSummaryIfExists);
    ProjectConfig->ScoreTalk = this->SetOptionYAML("score-talk", yaml, ProjectConfig->ScoreTalk).toLongLong();
    ProjectConfig->WarningDefs = this->SetUserOptionListYAML("warning-template-tags", yaml, ProjectConfig->WarningDefs);
    ProjectConfig->BotScore = this->SetOptionYAML("score-bot", yaml, ProjectConfig->BotScore).toLongLong();
    if (!HuggleQueueFilter::Filters.contains(ProjectConfig->Site))
        throw new Huggle::Exception("There is no such a wiki in queue filters hash table", BOOST_CURRENT_FUNCTION);
    (*HuggleQueueFilter::Filters[ProjectConfig->Site]) += HuggleParser::ConfigurationParseQueueList_YAML(yaml, false);
    ProjectConfig->ConfirmMultipleEdits = YAML2Bool("confirm-multiple", yaml, ProjectConfig->ConfirmMultipleEdits);
    ProjectConfig->ConfirmTalk = YAML2Bool("confirm-talk", yaml, ProjectConfig->ConfirmTalk);
    ProjectConfig->ConfirmOnSelfRevs = YAML2Bool("confirm-self-revert", yaml, ProjectConfig->ConfirmOnSelfRevs);
    ProjectConfig->ConfirmWL = YAML2Bool("confirm-whitelist", yaml, ProjectConfig->ConfirmWL);
    this->DisplayTitle = YAML2Bool("display-title", yaml, this->DisplayTitle);
    this->TruncateEdits = YAML2Bool("truncate-edits", yaml, this->TruncateEdits);
    this->EnforceMonthsAsHeaders = YAML2Bool("enforce-months-as-headers", yaml, this->EnforceMonthsAsHeaders);
    this->HistoryLoad = YAML2Bool("history-load", yaml, this->HistoryLoad);
    this->LastEdit = YAML2Bool("skip-to-last-edit", yaml, this->LastEdit);
    this->PreferredProvider = YAML2Int("preferred-provider", yaml, this->PreferredProvider);
    this->CheckTP = YAML2Bool("check-tp", yaml, this->CheckTP);
    this->RetrieveFounder = YAML2Bool("retrieve-founder", yaml, this->RetrieveFounder);
    this->HAN_DisplayBots = YAML2Bool("han-display-bots", yaml, this->HAN_DisplayBots);
    this->HAN_DisplayUser = YAML2Bool("han-display-user", yaml, this->HAN_DisplayUser);
    this->HAN_DisplayUserTalk = YAML2Bool("han-display-user-talk", yaml, this->HAN_DisplayUserTalk);
    this->HtmlAllowedInIrc = YAML2Bool("han-html", yaml, this->HtmlAllowedInIrc);
    this->ManualWarning = YAML2Bool("manual-warning", yaml, this->ManualWarning);
    this->RemoveAfterTrustedEdit = YAML2Bool("remove-after-trusted-edit", yaml, this->RemoveAfterTrustedEdit);
    this->Watchlist = WatchlistOptionFromString(YAML2String("watchlist", yaml));
    this->AutomaticallyGroup = YAML2Bool("automatically-group", yaml, this->AutomaticallyGroup);
    this->TalkPageFreshness = static_cast<unsigned int>(YAML2Int("talkpage-freshness", yaml, this->TalkPageFreshness));
    this->RemoveOldQueueEdits = YAML2Bool("remove-oldest-queue-edits", yaml, this->RemoveOldQueueEdits);
    this->QueueID = YAML2String("queue-id", yaml);
    this->GoNext = static_cast<Configuration_OnNext>(YAML2Int("on-next", yaml, 1));
    this->DeleteEditsAfterRevert = YAML2Bool("delete-edits-after-revert", yaml, this->DeleteEditsAfterRevert);
    this->WelcomeGood = this->SetOptionYAML("welcome-good", yaml, ProjectConfig->WelcomeGood).toBool();
    this->AutomaticReports = YAML2Bool("automatic-reports", yaml, this->AutomaticReports);
    this->PageEmptyQueue = YAML2String("page-empty-queue", yaml);
    delete this->Previous_Version;
    this->Previous_Version = new Version(YAML2String("version", yaml, HUGGLE_VERSION));
    this->AutomaticallyWatchlistWarnedUsers = YAML2Bool("automatically-watchlist-warned-users", yaml, this->AutomaticallyWatchlistWarnedUsers);
    // Score range
    this->EnableMaxScore = YAML2Bool("enable-max-score", yaml, this->EnableMaxScore);
    this->MinScore = YAML2LongLong("min-score", yaml, this->MinScore);
    this->MaxScore = YAML2LongLong("max-score", yaml, this->MaxScore);
    this->EnableMinScore = YAML2Bool("enable-min-score", yaml, this->EnableMinScore);
    this->NumberDropdownMenuItems = YAML2Bool("number-dropdown-menu-items", yaml, this->NumberDropdownMenuItems);
    this->AutomaticRefresh = YAML2Bool("automatic-refresh", yaml, this->AutomaticRefresh);
    this->ShortcutHash = YAML2String("shortcut-hash", yaml, "null");
    this->ShowWarningIfNotOnLastRevision = YAML2Bool("show-warning-if-not-on-last-revision", yaml, this->ShowWarningIfNotOnLastRevision);
    this->InsertEditsOfRolledUserToQueue = YAML2Bool("insert-edits-of-rolled-user-to-queue", yaml, this->InsertEditsOfRolledUserToQueue);
    this->ConfirmOnRecentWarning = YAML2Bool("confirm-on-recent-warning", yaml, this->ConfirmOnRecentWarning);
    this->ConfirmWarningOnVeryOldEdits = YAML2Bool("confirm-warning-on-very-old-edits", yaml, this->ConfirmWarningOnVeryOldEdits);
    this->RecentWarningTimeSpan = YAML2Int("recent-warning-time-span", yaml, this->RecentWarningTimeSpan);
    // for now we do this only for home wiki but later we need to make it for every wiki
    if (IsHome)
    {
        // Verify a shortcut hash first
        QString types = "";
        QStringList sorted_types = ProjectConfig->WarningTypes;
        sorted_types.sort();
        foreach (QString t, sorted_types)
            types += t + ",";
        QString hash = Generic::MD5(types);
        if (hash != this->ShortcutHash)
        {
            // Suppress this warning in case there is no record yet, this is first run of Huggle by new user
            if (!this->ShortcutHash.isEmpty() && this->ShortcutHash != "null")
                Hooks::ShowWarning(_hl("warning"), _hl("config-reset-menu"));
            hcfg->ResetMenuShortcuts();
            this->ShortcutHash = hash;
            return true;
        }
        QHash<QString, QString> shortcuts = HuggleParser::YAML2QStringHash("shortcut-list", yaml);
        foreach (QString id, shortcuts.keys())
        {
            if (!hcfg->Shortcuts.contains(id))
            {
                Syslog::HuggleLogs->WarningLog("Invalid shortcut in user configuration (unknown id): " + id);
                continue;
            }
            hcfg->Shortcuts[id].Modified = true;
            hcfg->Shortcuts[id].QAccel = QKeySequence(shortcuts[id]).toString();
        }
    }

    // Sanitize
    if (this->DefaultSummary.isEmpty())
        this->DefaultSummary = ProjectConfig->DefaultSummary;
    if (this->RollbackSummary.isEmpty())
        this->RollbackSummary = ProjectConfig->RollbackSummary;
    if (this->RollbackSummaryUnknownTarget.isEmpty())
        this->RollbackSummaryUnknownTarget = ProjectConfig->RollbackSummaryUnknownTarget;

    return true;
}

int Huggle::UserConfiguration::GetSafeUserInt(QString key, int default_value)
{
    HuggleOption *option = this->GetOption(key);
    if (option != nullptr)
        return option->GetVariant().toInt();
    return default_value;
}

bool Huggle::UserConfiguration::GetSafeUserBool(QString key, bool default_value)
{
    HuggleOption *option = this->GetOption(key);
    if (option != nullptr)
        return option->GetVariant().toBool();
    return default_value;
}

QString Huggle::UserConfiguration::GetSafeUserString(QString key, QString default_value)
{
    HuggleOption *option = this->GetOption(key);
    if (option != nullptr)
        return option->GetVariant().toString();

    return default_value;
}

