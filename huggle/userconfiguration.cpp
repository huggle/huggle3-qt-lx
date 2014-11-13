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
#include "huggleoption.hpp"
#include "huggleparser.hpp"
#include "syslog.hpp"
#include "projectconfiguration.hpp"
#include "version.hpp"
#include <QKeySequence>

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

Huggle::UserConfiguration::UserConfiguration()
{
    this->Previous_Version = new Version(HUGGLE_VERSION);
}

Huggle::UserConfiguration::~UserConfiguration()
{
    delete this->Previous_Version;
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

QVariant UserConfiguration::SetOption(QString key_, QString config_, QVariant default_)
{
    if (this->UserOptions.contains(key_))
    {
        // we must not add 2 same
        throw new Huggle::Exception("This option is already in a list you can't have multiple same keys in it", BOOST_CURRENT_FUNCTION);
    }
    QString d_ = default_.toString();
    QString value = ConfigurationParse(key_, config_, d_);
    HuggleOption *h;
    switch (default_.type())
    {
        case QVariant::Int:
            h = new HuggleOption(key_, value.toInt(), value == d_);
            break;
        case QVariant::Bool:
            h = new HuggleOption(key_, Generic::SafeBool(value), value == d_);
            break;
        default:
            h = new HuggleOption(key_, value, value == d_);
            break;
    }
    this->UserOptions.insert(key_, h);
    return h->GetVariant();
}

QString UserConfiguration::MakeLocalUserConfig(ProjectConfiguration *Project)
{
    QString configuration_ = "<nowiki>\n";
    configuration_ += "// This is a configuration of huggle, do not change it unless you know what you do.\n";
    configuration_ += "enable:true\n";
    configuration_ += "// Last version of huggle that wrote into this configuration file (sanity check)\n";
    configuration_ += "version:" + hcfg->HuggleVersion + "\n\n";
    configuration_ += "speedy-message-title:Speedy deleted\n";
    configuration_ += "report-summary:" + Project->ReportSummary + "\n";
    configuration_ += "prod-message-summary:Notification: Proposed deletion of [[$1]]\n";
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // huggle 2 options
    configuration_ += "auto-advance:false\n";
    configuration_ += "auto-whitelist:true\n";
    configuration_ += "username-listed:true\n";
    configuration_ += "admin:true\n";
    configuration_ += "patrol-speedy:true\n";
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    configuration_ += "confirm-multiple:" + Bool2String(Project->ConfirmMultipleEdits) + "\n";
    configuration_ += "confirm-talk:" + Bool2String(Project->ConfirmTalk) + "\n";
    // configuration_ += "confirm-page:" + Bool2String(HuggleConfiguration->ProjectConfig->ConfirmPage) + "\n";
    // configuration_ += "confirm-same:" + Bool2String(HuggleConfiguration->ProjectConfig->ConfirmSame) + "\n";
    configuration_ += "confirm-self-revert:" + Bool2String(Project->ConfirmOnSelfRevs) + "\n";
    configuration_ += "confirm-whitelist:" + Bool2String(Project->ConfirmWL) + "\n";
    //configuration_ += "confirm-warned:" + Bool2String(HuggleConfiguration->ProjectConfig->ConfirmWarned) + "\n";
    // configuration_ += "confirm-range:" + Bool2String(HuggleConfiguration->ProjectConfig->ConfirmRange) + "\n";
    configuration_ += "default-summary:" + Project->DefaultSummary + "\n";
    configuration_ += "// this option will change the behaviour of automatic resolution, be carefull\n";
    configuration_ += "revert-auto-multiple-edits:" + Bool2String(this->RevertOnMultipleEdits) + "\n";
    configuration_ += "automatically-resolve-conflicts:" + Bool2String(this->AutomaticallyResolveConflicts) + "\n";
    configuration_ += "font:" + this->Font + "\n";
    configuration_ += "software-rollback:" + Bool2String(this->EnforceManualSoftwareRollback) + "\n";
    configuration_ += "diff-font-size:" + QString::number(this->FontSize) + "\n";
    configuration_ += "HistoryLoad:" + Bool2String(this->HistoryLoad) + "\n";
    configuration_ += "OnNext:" + QString::number(static_cast<int>(this->GoNext)) + "\n";
    configuration_ += "DeleteEditsAfterRevert:" + Bool2String(this->DeleteEditsAfterRevert) + "\n";
    configuration_ += "SkipToLastEdit:" + Bool2String(this->LastEdit) + "\n";
    configuration_ += "RemoveOldestQueueEdits:" + Bool2String(this->RemoveOldQueueEdits) + "\n";
    configuration_ += "TruncateEdits:" + Bool2String(this->TruncateEdits) + "\n";
    configuration_ += "TalkpageFreshness:" + QString::number(this->TalkPageFreshness) + "\n";
    configuration_ += "DisplayTitle:" + Bool2String(this->DisplayTitle) + "\n";
    configuration_ += "// Periodically check if you received new messages and display a notification box if you get them\n";
    configuration_ += "CheckTP:" + Bool2String(this->CheckTP) + "\n";
    configuration_ += "ManualWarning:" + Bool2String(this->ManualWarning) + "\n";
    configuration_ += "SummaryMode:" + QString::number(this->SummaryMode) + "\n";
    configuration_ += "// HAN\n";
    configuration_ += "HAN_Html:" + Bool2String(hcfg->UserConfig->HtmlAllowedInIrc) + "\n";
    configuration_ += "HAN_DisplayUserTalk:" + Bool2String(this->HAN_DisplayUserTalk) + "\n";
    configuration_ += "HAN_DisplayBots:" + Bool2String(this->HAN_DisplayBots) + "\n";
    configuration_ += "HAN_DisplayUser:" + Bool2String(this->HAN_DisplayUser) + "\n";
    configuration_ += "QueueID:" + this->QueueID + "\n";
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
            si += "  " + s_.Name + ";" + s_.QAccel + ",\n";
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
        configuration_ += "ShortcutList:\n" + si + "\n";
    QStringList kl = this->UserOptions.keys();
    foreach (QString item, kl)
    {
        HuggleOption *option = this->GetOption(item);
        if (option == nullptr)
        {
            // this must never happen
            throw new Huggle::Exception("Option key was nullptr during store", BOOST_CURRENT_FUNCTION);
        }
        if (!option->IsDefault())
        {
            if (option->GetVariant().type() != QVariant::StringList)
            {
                // in case we modified this item we store it
                configuration_ += item + ":" + option->GetVariant().toString() + "\n";
            } else
            {
                QStringList list = option->GetVariant().toStringList();
                configuration_ += item + ":\n";
                int x = 0;
                while (x < list.count())
                {
                    if (x+1 < list.count())
                    {
                        configuration_ += "    " + list.at(x) + ",\n";
                    } else
                    {
                        configuration_ += "    " + list.at(x) + "\n\n";
                    }
                    x++;
                }
            }
        }
    }
    configuration_ += "// queues\nqueues:\n";
    int c = 0;
    while (c < HuggleQueueFilter::Filters.count())
    {
        HuggleQueueFilter *fltr = HuggleQueueFilter::Filters.at(c);
        c++;
        if (fltr->IsChangeable())
        {
            configuration_ += "    " + fltr->QueueName + ":\n";
            configuration_ += "        filter-ignored:" + Bool2ExcludeRequire(fltr->getIgnoreWL()) + "\n";
            configuration_ += "        filter-bots:" + Bool2ExcludeRequire(fltr->getIgnoreBots()) + "\n";
            configuration_ += "        filter-assisted:" + Bool2ExcludeRequire(fltr->getIgnoreFriends()) + "\n";
            configuration_ += "        filter-ip:" + Bool2ExcludeRequire(fltr->getIgnoreIP()) + "\n";
            configuration_ += "        filter-minor:" + Bool2ExcludeRequire(fltr->getIgnoreMinor()) + "\n";
            configuration_ += "        filter-new-pages:" + Bool2ExcludeRequire(fltr->getIgnoreNP()) + "\n";
            configuration_ += "        filter-me:" + Bool2ExcludeRequire(fltr->getIgnoreSelf()) + "\n";
            configuration_ += "        filter-users:" + Bool2ExcludeRequire(fltr->getIgnoreUsers()) + "\n";
            configuration_ += "        nsfilter-user:" + Bool2ExcludeRequire(fltr->getIgnore_UserSpace()) + "\n";
            configuration_ += "        filter-talk:" + Bool2ExcludeRequire(fltr->getIgnoreTalk()) + "\n";
            configuration_ += "\n";
        }
    }
    configuration_ += "</nowiki>";
    return configuration_;
}

QStringList UserConfiguration::SetUserOptionList(QString key_, QString config_, QStringList default_, bool CS)
{
    if (this->UserOptions.contains(key_))
    {
        // we must not add 2 same
        throw new Huggle::Exception("This option is already in a list you can't have multiple same keys in it", BOOST_CURRENT_FUNCTION);
    }
    QStringList value = HuggleParser::ConfigurationParse_QL(key_, config_, default_, CS);
    HuggleOption *h = new HuggleOption(key_, value, value == default_);
    this->UserOptions.insert(key_, h);
    return value;
}

bool UserConfiguration::ParseUserConfig(QString config, ProjectConfiguration *ProjectConfig, bool IsHome)
{
    this->RevertOnMultipleEdits = SafeBool(ConfigurationParse("RevertOnMultipleEdits", config));
    ProjectConfig->EnableAll = SafeBool(ConfigurationParse("enable", config));
    ProjectConfig->Ignores = HuggleParser::ConfigurationParse_QL("ignore", config, ProjectConfig->Ignores);
    // this is a hack so that we can access this value more directly, it can't be changed in huggle
    // so there is no point in using a hash for it
    ProjectConfig->IPScore = this->SetOption(ProjectConfig_IPScore_Key, config, ProjectConfig->IPScore).toInt();
    ProjectConfig->ScoreFlag = this->SetOption("score-flag", config, ProjectConfig->ScoreFlag).toInt();
    ProjectConfig->WarnSummary = this->SetOption("warn-summary", config, ProjectConfig->WarnSummary).toString();
    this->EnforceManualSoftwareRollback = SafeBool(ConfigurationParse("software-rollback", config));
    this->FontSize = ConfigurationParse("diff-font-size", config, "16").toInt();
    this->Font = ConfigurationParse("font", config, this->Font);
    ProjectConfig->WarnSummary2 = this->SetOption("warn-summary-2", config, ProjectConfig->WarnSummary2).toString();
    ProjectConfig->WarnSummary3 = this->SetOption("warn-summary-3", config, ProjectConfig->WarnSummary3).toString();
    ProjectConfig->WarnSummary4 = this->SetOption("warn-summary-4", config, ProjectConfig->WarnSummary4).toString();
    this->AutomaticallyResolveConflicts = SafeBool(ConfigurationParse("automatically-resolve-conflicts", config), false);
    ProjectConfig->TemplateAge = this->SetOption("template-age", config, ProjectConfig->TemplateAge).toInt();
    ProjectConfig->RevertSummaries = this->SetUserOptionList("template-summ", config, ProjectConfig->RevertSummaries);
    ProjectConfig->WarningTypes = this->SetUserOptionList("warning-types", config, ProjectConfig->WarningTypes);
    ProjectConfig->ScoreChange = this->SetOption("score-change", config, ProjectConfig->ScoreChange).toInt();
    ProjectConfig->ScoreUser = this->SetOption("score-user", config, ProjectConfig->ScoreUser).toInt();
    this->SummaryMode = ConfigurationParse("SummaryMode", config, "0").toInt();
    ProjectConfig->ScoreTalk = this->SetOption("score-talk", config, ProjectConfig->ScoreTalk).toInt();
    ProjectConfig->WarningDefs = this->SetUserOptionList("warning-template-tags", config, ProjectConfig->WarningDefs);
    ProjectConfig->BotScore = this->SetOption("score-bot", config, ProjectConfig->BotScore).toInt();
    if (IsHome)
    {
        // we do this only for home wiki
        HuggleQueueFilter::Filters += HuggleParser::ConfigurationParseQueueList(config, false);
    }
    ProjectConfig->ConfirmMultipleEdits = SafeBool(ConfigurationParse("confirm-multiple", config), ProjectConfig->ConfirmMultipleEdits);
    ProjectConfig->ConfirmTalk = SafeBool(ConfigurationParse("confirm-talk", config), ProjectConfig->ConfirmTalk);
    ProjectConfig->ConfirmOnSelfRevs = SafeBool(ConfigurationParse("confirm-self-revert", config), ProjectConfig->ConfirmOnSelfRevs);
    ProjectConfig->ConfirmWL = SafeBool(ConfigurationParse("confirm-whitelist", config), ProjectConfig->ConfirmWL);
    this->DisplayTitle = SafeBool(ConfigurationParse("DisplayTitle", config, "false"));
    this->TruncateEdits = SafeBool(ConfigurationParse("TruncateEdits", config, "false"));
    this->HistoryLoad = SafeBool(ConfigurationParse("HistoryLoad", config, "true"));
    this->LastEdit = SafeBool(ConfigurationParse("SkipToLastEdit", config, "false"));
    this->CheckTP = SafeBool(ConfigurationParse("CheckTP", config, "true"));
    this->HAN_DisplayBots = SafeBool(ConfigurationParse("HAN_DisplayBots", config, "true"));
    this->HAN_DisplayUser = SafeBool(ConfigurationParse("HAN_DisplayUser", config, "true"));
    this->ManualWarning = SafeBool(ConfigurationParse("ManualWarning", config, "true"));
    this->HAN_DisplayUserTalk = SafeBool(ConfigurationParse("HAN_DisplayUserTalk", config, "true"));
    this->HtmlAllowedInIrc = SafeBool(ConfigurationParse("HAN_Html", config, "false"));
    this->TalkPageFreshness = ConfigurationParse("TalkpageFreshness", config, QString::number(this->TalkPageFreshness)).toInt();
    this->RemoveOldQueueEdits = SafeBool(ConfigurationParse("RemoveOldestQueueEdits", config, "false"));
    this->QueueID = ConfigurationParse("QueueID", config);
    this->GoNext = static_cast<Configuration_OnNext>(ConfigurationParse("OnNext", config, "1").toInt());
    this->DeleteEditsAfterRevert = SafeBool(ConfigurationParse("DeleteEditsAfterRevert", config, "true"));
    this->WelcomeGood = this->SetOption("welcome-good", config, ProjectConfig->WelcomeGood).toBool();
    delete this->Previous_Version;
    this->Previous_Version = new Version(ConfigurationParse("version", config, HUGGLE_VERSION));
    // for now we do this only for home wiki but later we need to make it for every wiki
    if (IsHome)
    {
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
    return true;
}

int Huggle::UserConfiguration::GetSafeUserInt(QString key_, int default_value)
{
    HuggleOption *option = this->GetOption(key_);
    if (option != nullptr)
        return option->GetVariant().toInt();
    return default_value;
}

bool Huggle::UserConfiguration::GetSafeUserBool(QString key_, bool default_value)
{
    HuggleOption *option = this->GetOption(key_);
    if (option != nullptr)
        return option->GetVariant().toBool();
    return default_value;
}

QString Huggle::UserConfiguration::GetSafeUserString(QString key_, QString default_value)
{
    HuggleOption *option = this->GetOption(key_);
    if (option != nullptr)
        return option->GetVariant().toString();

    return default_value;
}

