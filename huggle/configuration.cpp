//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "configuration.hpp"
#include <QDir>
#include <QtXml>
#include <QKeySequence>
#include <QDesktopServices>
#include "syslog.hpp"
#include "exception.hpp"
#include "generic.hpp"
#include "hugglequeuefilter.hpp"
#include "huggleparser.hpp"
#include "localization.hpp"
#include "wikipage.hpp"
#include "wikisite.hpp"

using namespace Huggle::HuggleParser;
using namespace Huggle;
using namespace Huggle::Generic;
Configuration * Configuration::HuggleConfiguration = nullptr;

Configuration::Configuration()
{
#ifdef HUGGLE_PYTHON
    this->PythonEngine = true;
#else
    this->PythonEngine = false;
#endif
    //! This is a consumer key for "huggle" on wmf wikis
    this->WmfOAuthConsumerKey = "56a6d6de895e3b859faa57b677f6cd21";
    this->HuggleVersion = HUGGLE_VERSION;
#if QT_VERSION >= 0x050000
    this->HomePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
    this->HomePath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif
    this->Platform = HUGGLE_UPDATER_PLATFORM_TYPE;
    this->ProjectConfig = new ProjectConfiguration();
    this->UserConfig = new UserConfiguration();

    this->MakeShortcut("main-revert-and-warn", "shortcut-raw", "Q");
    this->MakeShortcut("main-exit", "shortcut-exit");
    this->MakeShortcut("main-next", "shortcut-next", "Space");
    this->MakeShortcut("main-suspicious-edit", "shortcut-suspicious", "S");
    this->MakeShortcut("main-back", "shortcut-back", "[");
    this->MakeShortcut("main-forward", "shortcut-forward", "]");
    this->MakeShortcut("main-warn", "shortcut-warn", "W");
    this->MakeShortcut("main-revert", "shortcut-revert", "R");
    this->MakeShortcut("main-revert-and-warn-0", "shortcut-x-raw");
    this->MakeShortcut("main-revert-and-warn-1", "shortcut-x-raw");
    this->MakeShortcut("main-revert-and-warn-2", "shortcut-x-raw");
    this->MakeShortcut("main-revert-and-warn-3", "shortcut-x-raw");
    this->MakeShortcut("main-revert-and-warn-4", "shortcut-x-raw");
    this->MakeShortcut("main-revert-and-warn-5", "shortcut-x-raw");
    this->MakeShortcut("main-revert-and-warn-6", "shortcut-x-raw");
    this->MakeShortcut("main-revert-and-warn-7", "shortcut-x-raw");
    this->MakeShortcut("main-revert-and-warn-8", "shortcut-x-raw");
    this->MakeShortcut("main-revert-and-warn-9", "shortcut-x-raw");
    this->MakeShortcut("main-warn-0", "shortcut-x-warn");
    this->MakeShortcut("main-warn-1", "shortcut-x-warn");
    this->MakeShortcut("main-warn-2", "shortcut-x-warn");
    this->MakeShortcut("main-warn-3", "shortcut-x-warn");
    this->MakeShortcut("main-warn-4", "shortcut-x-warn");
    this->MakeShortcut("main-warn-5", "shortcut-x-warn");
    this->MakeShortcut("main-warn-6", "shortcut-x-warn");
    this->MakeShortcut("main-warn-7", "shortcut-x-warn");
    this->MakeShortcut("main-warn-8", "shortcut-x-warn");
    this->MakeShortcut("main-warn-9", "shortcut-x-warn");
    this->MakeShortcut("main-revert-0", "shortcut-x-revert");
    this->MakeShortcut("main-revert-1", "shortcut-x-revert");
    this->MakeShortcut("main-revert-2", "shortcut-x-revert");
    this->MakeShortcut("main-revert-3", "shortcut-x-revert");
    this->MakeShortcut("main-revert-4", "shortcut-x-revert");
    this->MakeShortcut("main-revert-5", "shortcut-x-revert");
    this->MakeShortcut("main-revert-6", "shortcut-x-revert");
    this->MakeShortcut("main-revert-7", "shortcut-x-revert");
    this->MakeShortcut("main-revert-8", "shortcut-x-revert");
    this->MakeShortcut("main-revert-9", "shortcut-x-revert");
    this->MakeShortcut("main-talk", "shortcut-talk", "T");
    this->MakeShortcut("main-mytalk", "shortcut-my-talk", "Alt+M");
    this->MakeShortcut("main-open-in-browser", "shortcut-open", "O");
    this->MakeShortcut("main-good", "shortcut-good", "G");
}

Configuration::~Configuration()
{
    QStringList ol = this->ExtensionData.keys();
    while (ol.count())
    {
        ExtensionConfig *option = this->ExtensionData[ol[0]];
        this->ExtensionData.remove(ol[0]);
        delete option;
        ol.removeAt(0);
    }
    delete this->Project;
    delete this->ProjectConfig;
    delete this->UserConfig;
}

HuggleOption *Configuration::GetOption(QString key)
{
    if (this->UserConfig->UserOptions.contains(key))
    {
        return this->UserConfig->UserOptions[key];
    }
    return nullptr;
}

QVariant Configuration::SetOption(QString key_, QString config_, QVariant default_)
{
    if (this->UserConfig->UserOptions.contains(key_))
    {
        // we must not add 2 same
        throw new Huggle::Exception("This option is already in a list you can't have multiple same keys in it",
                                    "void Configuration::SetOption(QString key, QVariant data)");
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
            h = new HuggleOption(key_, SafeBool(value), value == d_);
            break;
        default:
            h = new HuggleOption(key_, value, value == d_);
            break;
    }
    this->UserConfig->UserOptions.insert(key_, h);
    return h->GetVariant();
}

QStringList Configuration::SetUserOptionList(QString key_, QString config_, QStringList default_, bool CS)
{
    if (this->UserConfig->UserOptions.contains(key_))
    {
        // we must not add 2 same
        throw new Huggle::Exception("This option is already in a list you can't have multiple same keys in it",
                                    "void Configuration::SetUserOptionList(QString key, QVariant data)");
    }
    QStringList value = HuggleParser::ConfigurationParse_QL(key_, config_, default_, CS);
    HuggleOption *h = new HuggleOption(key_, value, value == default_);
    this->UserConfig->UserOptions.insert(key_, h);
    return value;
}

int Configuration::GetSafeUserInt(QString key_, int default_value)
{
    HuggleOption *option = this->GetOption(key_);
    if (option != nullptr)
        return option->GetVariant().toInt();
    return default_value;
}

bool Configuration::GetSafeUserBool(QString key_, bool default_value)
{
    HuggleOption *option = this->GetOption(key_);
    if (option != nullptr)
        return option->GetVariant().toBool();
    return default_value;
}

QString Configuration::GetSafeUserString(QString key_, QString default_value)
{
    HuggleOption *option = this->GetOption(key_);
    if (option != nullptr)
        return option->GetVariant().toString();

    return default_value;
}

QString Configuration::GenerateSuffix(QString text, ProjectConfiguration *conf)
{
    if (!text.endsWith(conf->EditSuffixOfHuggle))
    {
        text = text + " " + conf->EditSuffixOfHuggle;
    }
    return text;
}

QString Configuration::GetExtensionsRootPath()
{
    QString path_ = Configuration::HuggleConfiguration->HomePath + QDir::separator() + EXTENSION_PATH;
    QDir conf(path_);
    if (!conf.exists() && !conf.mkpath(path_))
    {
            Syslog::HuggleLogs->WarningLog("Unable to create " + path_);
    }
    return path_ + QDir::separator();
}

QString Configuration::GetLocalizationDataPath()
{
    QDir conf(Configuration::HuggleConfiguration->HomePath + QDir::separator() + "Localization");
    if (!conf.exists())
    {
        conf.mkpath(Configuration::HuggleConfiguration->HomePath + QDir::separator() + "Localization");
    }
    return Configuration::HuggleConfiguration->HomePath + QDir::separator() + "Localization" + QDir::separator();
}

QString Configuration::GetURLProtocolPrefix(WikiSite *s)
{
    if (s && !s->SupportHttps)
        return "http://";
    if (!Configuration::HuggleConfiguration->SystemConfig_UsingSSL)
        return "http://";

    return "https://";
}

QString Configuration::GetConfigurationPath()
{
    QDir conf(Configuration::HuggleConfiguration->HomePath + QDir::separator() + "Configuration");
    if (!conf.exists())
    {
        conf.mkpath(Configuration::HuggleConfiguration->HomePath + QDir::separator() + "Configuration");
    }
    return Configuration::HuggleConfiguration->HomePath + QDir::separator() + "Configuration" + QDir::separator();
}

QString Configuration::ReplaceSpecialUserPage(QString PageName)
{
    QString result = PageName;
    PageName = PageName.toLower();
    if (PageName.startsWith("special:mypage"))
    {
        result = result.mid(14);
        result = "User:$1" + result;
    } else if (PageName.startsWith("special:mytalk"))
    {
        result = result.mid(14);
        result = "User_talk:$1" + result;
    }
    return result;
}

QString Configuration::Bool2ExcludeRequire(bool b)
{
    if (b)
    {
        return "exclude";
    }
    return "require";
}

void Configuration::MakeShortcut(QString name, QString description, QString default_accel)
{
    Shortcut shortcut = Shortcut(name, description);
    shortcut.QAccel = default_accel;
    this->Shortcuts.insert(name, shortcut);
}

void Configuration::NormalizeConf()
{
    if (this->ProjectConfig->TemplateAge > -1)
        this->ProjectConfig->TemplateAge = -30;
    if (this->SystemConfig_QueueSize < 10)
        this->SystemConfig_QueueSize = 10;
    if (this->SystemConfig_HistorySize < 2)
        this->SystemConfig_HistorySize = 2;

    if (this->ProjectConfig->MessageHeadings == HeadingsPageName || this->ProjectConfig->MessageHeadings == HeadingsNone)
    {
        this->UserConfig->EnforceMonthsAsHeaders = false;
    }
}

QString Configuration::GenerateSuffix(QString text)
{
    return this->GenerateSuffix(text, this->ProjectConfig);
}

QString Configuration::MakeLocalUserConfig()
{
    /// \todo rewrite to save all SharedConfig only if different from project
    QString configuration_ = "<nowiki>\n";
    configuration_ += "enable:true\n";
    configuration_ += "version:" + HuggleConfiguration->HuggleVersion + "\n\n";
    configuration_ += "speedy-message-title:Speedy deleted\n";
    configuration_ += "report-summary:" + HuggleConfiguration->ProjectConfig->ReportSummary + "\n";
    configuration_ += "prod-message-summary:Notification: Proposed deletion of [[$1]]\n";
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // huggle 2 options
    configuration_ += "auto-advance:false\n";
    configuration_ += "auto-whitelist:true\n";
    configuration_ += "username-listed:true\n";
    configuration_ += "admin:true\n";
    configuration_ += "patrol-speedy:true\n";
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    configuration_ += "confirm-multiple:" + Bool2String(HuggleConfiguration->ProjectConfig->ConfirmMultipleEdits) + "\n";
    configuration_ += "confirm-talk:" + Bool2String(HuggleConfiguration->ProjectConfig->ConfirmTalk) + "\n";
    // configuration_ += "confirm-page:" + Bool2String(HuggleConfiguration->ProjectConfig->ConfirmPage) + "\n";
    // configuration_ += "confirm-same:" + Bool2String(HuggleConfiguration->ProjectConfig->ConfirmSame) + "\n";
    configuration_ += "confirm-self-revert:" + Bool2String(HuggleConfiguration->ProjectConfig->ConfirmOnSelfRevs) + "\n";
    configuration_ += "confirm-whitelist:" + Bool2String(HuggleConfiguration->ProjectConfig->ConfirmWL) + "\n";
    //configuration_ += "confirm-warned:" + Bool2String(HuggleConfiguration->ProjectConfig->ConfirmWarned) + "\n";
    // configuration_ += "confirm-range:" + Bool2String(HuggleConfiguration->ProjectConfig->ConfirmRange) + "\n";
    configuration_ += "default-summary:" + HuggleConfiguration->ProjectConfig->DefaultSummary + "\n";
    configuration_ += "// this option will change the behaviour of automatic resolution, be carefull\n";
    configuration_ += "revert-auto-multiple-edits:" + Bool2String(HuggleConfiguration->RevertOnMultipleEdits) + "\n";
    configuration_ += "automatically-resolve-conflicts:" + Bool2String(HuggleConfiguration->UserConfig->AutomaticallyResolveConflicts) + "\n";
    configuration_ += "software-rollback:" + Bool2String(HuggleConfiguration->EnforceManualSoftwareRollback) + "\n";
    configuration_ += "diff-font-size:" + QString::number(HuggleConfiguration->SystemConfig_FontSize) + "\n";
    configuration_ += "RevertOnMultipleEdits:" + Bool2String(HuggleConfiguration->RevertOnMultipleEdits) + "\n";
    configuration_ += "HistoryLoad:" + Bool2String(HuggleConfiguration->UserConfig->HistoryLoad) + "\n";
    configuration_ += "OnNext:" + QString::number(static_cast<int>(HuggleConfiguration->UserConfig->GoNext)) + "\n";
    configuration_ += "DeleteEditsAfterRevert:" + Bool2String(HuggleConfiguration->UserConfig->DeleteEditsAfterRevert) + "\n";
    configuration_ += "SkipToLastEdit:" + Bool2String(HuggleConfiguration->UserConfig->LastEdit) + "\n";
    configuration_ += "RemoveOldestQueueEdits:" + Bool2String(HuggleConfiguration->UserConfig->RemoveOldQueueEdits) + "\n";
    configuration_ += "TruncateEdits:" + Bool2String(HuggleConfiguration->UserConfig->TruncateEdits) + "\n";
    configuration_ += "TalkpageFreshness:" + QString::number(HuggleConfiguration->UserConfig->TalkPageFreshness) + "\n";
    configuration_ += "DisplayTitle:" + Bool2String(HuggleConfiguration->UserConfig->DisplayTitle) + "\n";
    configuration_ += "// Periodically check if you received new messages and display a notification box if you get them\n";
    configuration_ += "CheckTP:" + Bool2String(HuggleConfiguration->UserConfig->CheckTP) + "\n";
    configuration_ += "ManualWarning:" + Bool2String(HuggleConfiguration->UserConfig->ManualWarning) + "\n";
    configuration_ += "// HAN\n";
    configuration_ += "HAN_Html:" + Bool2String(HuggleConfiguration->HtmlAllowedInIrc) + "\n";
    configuration_ += "HAN_DisplayUserTalk:" + Bool2String(HuggleConfiguration->UserConfig->HAN_DisplayUserTalk) + "\n";
    configuration_ += "HAN_DisplayBots:" + Bool2String(HuggleConfiguration->UserConfig->HAN_DisplayBots) + "\n";
    configuration_ += "HAN_DisplayUser:" + Bool2String(HuggleConfiguration->UserConfig->HAN_DisplayUser) + "\n";
    configuration_ += "QueueID:" + HuggleConfiguration->UserConfig->QueueID + "\n";
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
    QStringList kl = HuggleConfiguration->UserConfig->UserOptions.keys();
    foreach (QString item, kl)
    {
        HuggleOption *option = HuggleConfiguration->GetOption(item);
        if (option == nullptr)
        {
            // this must never happen
            throw new Huggle::Exception("Option key was nullptr during store", "QString Configuration::MakeLocalUserConfig()");
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
            configuration_ += "        filter-ignored:" + Configuration::Bool2ExcludeRequire(fltr->getIgnoreWL()) + "\n";
            configuration_ += "        filter-bots:" + Configuration::Bool2ExcludeRequire(fltr->getIgnoreBots()) + "\n";
            configuration_ += "        filter-assisted:" + Configuration::Bool2ExcludeRequire(fltr->getIgnoreFriends()) + "\n";
            configuration_ += "        filter-ip:" + Configuration::Bool2ExcludeRequire(fltr->getIgnoreIP()) + "\n";
            configuration_ += "        filter-minor:" + Configuration::Bool2ExcludeRequire(fltr->getIgnoreMinor()) + "\n";
            configuration_ += "        filter-new-pages:" + Configuration::Bool2ExcludeRequire(fltr->getIgnoreNP()) + "\n";
            configuration_ += "        filter-me:" + Configuration::Bool2ExcludeRequire(fltr->getIgnoreSelf()) + "\n";
            configuration_ += "        filter-users:" + Configuration::Bool2ExcludeRequire(fltr->getIgnoreUsers()) + "\n";
            configuration_ += "        nsfilter-user:" + Configuration::Bool2ExcludeRequire(fltr->getIgnore_UserSpace()) + "\n";
            configuration_ += "        filter-talk:" + Configuration::Bool2ExcludeRequire(fltr->getIgnoreTalk()) + "\n";
            configuration_ += "\n";
        }
    }
    configuration_ += "</nowiki>";
    return configuration_;
}

void Configuration::LoadSystemConfig(QString fn)
{
    QFile file(fn);
    if (!QFile().exists(fn))
    {
        HUGGLE_DEBUG1("No config file at " + fn);
        return;
    }
    if(!file.open(QIODevice::ReadOnly))
    {
        HUGGLE_DEBUG1("Unable to read config file");
        return;
    }
    QDomDocument conf;
    conf.setContent(file.readAll());
    file.close();
    QDomNodeList l = conf.elementsByTagName("local");
    QDomNodeList e = conf.elementsByTagName("extern");
    int item = 0;
    while (item < l.count())
    {
        QDomElement option = l.at(item).toElement();
        item++;
        QDomNamedNodeMap xx = option.attributes();
        if (!xx.contains("text") || !xx.contains("key"))
        {
            continue;
        }
        QString key = option.attribute("key");
        if (key == "Cache_InfoSize")
        {
            Configuration::HuggleConfiguration->SystemConfig_QueueSize = option.attribute("text").toInt();
            continue;
        }
        if (key == "GlobalConfigurationWikiAddress")
        {
            Configuration::HuggleConfiguration->GlobalConfigurationWikiAddress = option.attribute("text");
            continue;
        }
        if (key == "IRCIdent")
        {
            Configuration::HuggleConfiguration->IRCIdent = option.attribute("text");
            continue;
        }
        if (key == "IRCNick")
        {
            Configuration::HuggleConfiguration->IRCNick = option.attribute("text");
            continue;
        }
        if (key == "IRCPort")
        {
            Configuration::HuggleConfiguration->IRCPort = option.attribute("text").toInt();
            continue;
        }
        if (key == "IRCServer")
        {
            Configuration::HuggleConfiguration->IRCServer = option.attribute("text");
            continue;
        }
        if (key == "Language")
        {
            Localizations::HuggleLocalizations->PreferredLanguage = option.attribute("text");
            continue;
        }
        if (key == "ProviderCache")
        {
            Configuration::HuggleConfiguration->SystemConfig_ProviderCache = option.attribute("text").toInt();
            continue;
        }
        if (key == "AskUserBeforeReport")
        {
            Configuration::HuggleConfiguration->AskUserBeforeReport = SafeBool(option.attribute("text"));
            continue;
        }
        if (key == "HistorySize")
        {
            Configuration::HuggleConfiguration->SystemConfig_HistorySize = option.attribute("text").toInt();
            continue;
        }
        if (key == "VandalNw_Login")
        {
            Configuration::HuggleConfiguration->VandalNw_Login = SafeBool(option.attribute("text"));
            continue;
        }
        if (key == "UserName" && Configuration::HuggleConfiguration->SystemConfig_Username.isEmpty())
        {
            Configuration::HuggleConfiguration->SystemConfig_Username = option.attribute("text");
            continue;
        }
        if (key == "RingLogMaxSize")
        {
            Configuration::HuggleConfiguration->SystemConfig_RingLogMaxSize = option.attribute("text").toInt();
            continue;
        }
        if (key == "DynamicColsInList")
        {
            Configuration::HuggleConfiguration->SystemConfig_DynamicColsInList = SafeBool(option.attribute("text"));
            continue;
        }
        if (key == "WarnUserSpaceRoll")
        {
            Configuration::HuggleConfiguration->WarnUserSpaceRoll = SafeBool(option.attribute("text"));
            continue;
        }
        if (key == "EnableUpdates")
        {
            Configuration::HuggleConfiguration->SystemConfig_UpdatesEnabled = SafeBool(option.attribute("text"));
            continue;
        }
        if (key == "NotifyBeta")
        {
            Configuration::HuggleConfiguration->SystemConfig_NotifyBeta = SafeBool(option.attribute("text"));
            continue;
        }
        if (key == "QueueNewEditsUp")
        {
            Configuration::HuggleConfiguration->SystemConfig_QueueNewEditsUp = SafeBool(option.attribute("text"));
            continue;
        }
        if (key == "IndexOfLastWiki")
        {
            Configuration::HuggleConfiguration->IndexOfLastWiki = option.attribute("text").toInt();
            continue;
        }
        if (key == "UsingSSL")
        {
            Configuration::HuggleConfiguration->SystemConfig_UsingSSL = SafeBool(option.attribute("text"));
            continue;
        }
        if (key == "GlobalConfigWikiList")
        {
            Configuration::HuggleConfiguration->SystemConfig_GlobalConfigWikiList = option.attribute("text");
            continue;
        }
        if (key == "DelayVal")
        {
            Configuration::HuggleConfiguration->SystemConfig_DelayVal = option.attribute("text").toUInt();
            continue;
        }
        if (key == "WikiRC")
        {
            Configuration::HuggleConfiguration->SystemConfig_WikiRC = option.attribute("text").toUInt();
            continue;
        }
        if (key == "RequestDelay")
        {
            Configuration::HuggleConfiguration->SystemConfig_RequestDelay = SafeBool(option.attribute("text"));
            continue;
        }
        if (key == "RevertDelay")
        {
            Configuration::HuggleConfiguration->SystemConfig_RevertDelay = option.attribute("text").toUInt();
            continue;
        }
        if (key == "InstantReverts")
        {
            Configuration::HuggleConfiguration->SystemConfig_InstantReverts = SafeBool(option.attribute("text"));
            continue;
        }
    }
    item = 0;
    while (item < e.count())
    {
        QDomElement option = l.at(item).toElement();
        item++;
        if (!option.attributes().contains("extension") || !option.attributes().contains("name"))
            continue;
        QString ExtensionName = option.attribute("extension");
        QString KeyName = option.attribute("name");
        if (!Configuration::HuggleConfiguration->ExtensionData.contains(ExtensionName))
        {
            // we need to insert a new option list because this extension is not yet known
            Configuration::HuggleConfiguration->ExtensionData.insert(ExtensionName, new ExtensionConfig());
        }
        Configuration::HuggleConfiguration->ExtensionData[ExtensionName]->SetOption(KeyName, option.attribute("value"));
    }
    Huggle::Syslog::HuggleLogs->DebugLog("Finished conf");
}

void Configuration::SaveSystemConfig()
{
    QFile file(Configuration::GetConfigurationPath() + QDir::separator() + "huggle3.xml");
    if (!file.open(QIODevice::WriteOnly))
    {
        Huggle::Syslog::HuggleLogs->Log("Unable to save configuration because the file can't be open");
        return;
    }
    QXmlStreamWriter *writer = new QXmlStreamWriter();
    writer->setDevice(&file);
    writer->setAutoFormatting(true);
    writer->writeStartDocument();
    writer->writeStartElement("huggle");
    InsertConfig("DelayVal", QString::number(Configuration::HuggleConfiguration->SystemConfig_DelayVal), writer);
    InsertConfig("RequestDelay", Bool2String(Configuration::HuggleConfiguration->SystemConfig_RequestDelay), writer);
    InsertConfig("RevertDelay", QString::number(Configuration::HuggleConfiguration->SystemConfig_RevertDelay), writer);
    InsertConfig("InstantReverts", Bool2String(Configuration::HuggleConfiguration->SystemConfig_InstantReverts), writer);
    InsertConfig("UsingSSL", Bool2String(Configuration::HuggleConfiguration->SystemConfig_UsingSSL), writer);
    InsertConfig("Cache_InfoSize", QString::number(Configuration::HuggleConfiguration->SystemConfig_QueueSize), writer);
    InsertConfig("GlobalConfigurationWikiAddress", Configuration::HuggleConfiguration->GlobalConfigurationWikiAddress, writer);
    InsertConfig("IRCIdent", Configuration::HuggleConfiguration->IRCIdent, writer);
    InsertConfig("IRCNick", Configuration::HuggleConfiguration->IRCNick, writer);
    InsertConfig("IRCPort", QString::number(Configuration::HuggleConfiguration->IRCPort), writer);
    InsertConfig("IRCServer", Configuration::HuggleConfiguration->IRCServer, writer);
    InsertConfig("Language", Localizations::HuggleLocalizations->PreferredLanguage, writer);
    InsertConfig("ProviderCache", QString::number(Configuration::HuggleConfiguration->SystemConfig_ProviderCache), writer);
    InsertConfig("AskUserBeforeReport", Bool2String(Configuration::HuggleConfiguration->AskUserBeforeReport), writer);
    InsertConfig("HistorySize", QString::number(Configuration::HuggleConfiguration->SystemConfig_HistorySize), writer);
    InsertConfig("QueueNewEditsUp", Bool2String(Configuration::HuggleConfiguration->SystemConfig_QueueNewEditsUp), writer);
    InsertConfig("RingLogMaxSize", QString::number(Configuration::HuggleConfiguration->SystemConfig_RingLogMaxSize), writer);
    InsertConfig("TrimOldWarnings", Bool2String(Configuration::HuggleConfiguration->TrimOldWarnings), writer);
    InsertConfig("EnableUpdates", Bool2String(Configuration::HuggleConfiguration->SystemConfig_UpdatesEnabled), writer);
    InsertConfig("NotifyBeta", Bool2String(Configuration::HuggleConfiguration->SystemConfig_NotifyBeta), writer);
    InsertConfig("WarnUserSpaceRoll", Bool2String(Configuration::HuggleConfiguration->WarnUserSpaceRoll), writer);
    InsertConfig("WikiRC", QString::number(Configuration::HuggleConfiguration->SystemConfig_WikiRC), writer);
    InsertConfig("UserName", Configuration::HuggleConfiguration->SystemConfig_Username, writer);
    InsertConfig("IndexOfLastWiki", QString::number(Configuration::HuggleConfiguration->IndexOfLastWiki), writer);
    InsertConfig("DynamicColsInList", Bool2String(Configuration::HuggleConfiguration->SystemConfig_DynamicColsInList), writer);
    /////////////////////////////
    // Vandal network
    /////////////////////////////
    InsertConfig("VandalNw_Login", Bool2String(Configuration::HuggleConfiguration->VandalNw_Login), writer);
    InsertConfig("GlobalConfigWikiList", Configuration::HuggleConfiguration->SystemConfig_GlobalConfigWikiList, writer);
    QStringList extensions = Configuration::HuggleConfiguration->ExtensionData.keys();
    foreach (QString ex, extensions)
    {
        ExtensionConfig *ed = Configuration::HuggleConfiguration->ExtensionData[ex];
        QStringList options = ed->Options.keys();
        foreach (QString option, options)
        {
            writer->writeStartElement("extern");
            writer->writeAttribute("extension", ex);
            writer->writeAttribute("name", option);
            writer->writeAttribute("value", ed->Options[option]);
            writer->writeEndElement();
        }
    }
    writer->writeEndElement();
    writer->writeEndDocument();
    delete writer;
}

bool Configuration::ParseGlobalConfig(QString config)
{
    this->GlobalConfig_EnableAll = SafeBool(ConfigurationParse("enable-all", config));
    this->GlobalConfig_DocumentationPath = ConfigurationParse("documentation", config, this->GlobalConfig_DocumentationPath);
    this->GlobalConfig_FeedbackPath = ConfigurationParse("feedback", config, this->GlobalConfig_FeedbackPath);
    // Sanitize page titles (huggle2 done sth. similiar at Page.SanitizeTitle before requesting them)
    this->GlobalConfig_UserConf = ReplaceSpecialUserPage(ConfigurationParse("user-config-hg3", config));
    this->GlobalConfig_UserConf_old = ReplaceSpecialUserPage(ConfigurationParse("user-config", config));
    this->HANMask = ConfigurationParse("han-mask", config, this->HANMask);
    this->GlobalConfig_Whitelist = ConfigurationParse("whitelist-server", config);
    QString Webquery_ = ConfigurationParse("user-agent", config, "Huggle/$1 http://en.wikipedia.org/wiki/Wikipedia:Huggle");
    Webquery_.replace("$1", this->SystemConfig_Username);
    this->WebqueryAgent = Webquery_.toUtf8();
    this->GlobalConfigWasLoaded = true;
    return true;
}

bool Configuration::ParseUserConfig(QString config)
{
    this->RevertOnMultipleEdits = SafeBool(ConfigurationParse("RevertOnMultipleEdits", config));
    this->ProjectConfig->EnableAll = SafeBool(ConfigurationParse("enable", config));
    this->ProjectConfig->Ignores = HuggleParser::ConfigurationParse_QL("ignore", config, this->ProjectConfig->Ignores);
    // this is a hack so that we can access this value more directly, it can't be changed in huggle
    // so there is no point in using a hash for it
    this->ProjectConfig->IPScore = this->SetOption(ProjectConfig_IPScore_Key, config, this->ProjectConfig->IPScore).toInt();
    this->ProjectConfig->ScoreFlag = this->SetOption("score-flag", config, this->ProjectConfig->ScoreFlag).toInt();
    this->ProjectConfig->WarnSummary = this->SetOption("warn-summary", config, this->ProjectConfig->WarnSummary).toString();
    this->EnforceManualSoftwareRollback = SafeBool(ConfigurationParse("software-rollback", config));
    this->ProjectConfig->WarnSummary2 = this->SetOption("warn-summary-2", config, this->ProjectConfig->WarnSummary2).toString();
    this->ProjectConfig->WarnSummary3 = this->SetOption("warn-summary-3", config, this->ProjectConfig->WarnSummary3).toString();
    this->ProjectConfig->WarnSummary4 = this->SetOption("warn-summary-4", config, this->ProjectConfig->WarnSummary4).toString();
    this->UserConfig->AutomaticallyResolveConflicts = SafeBool(ConfigurationParse("automatically-resolve-conflicts", config), false);
    this->ProjectConfig->TemplateAge = this->SetOption("template-age", config, this->ProjectConfig->TemplateAge).toInt();
    this->ProjectConfig->RevertSummaries = this->SetUserOptionList("template-summ", config, this->ProjectConfig->RevertSummaries);
    this->ProjectConfig->WarningTypes = this->SetUserOptionList("warning-types", config, this->ProjectConfig->WarningTypes);
    this->ProjectConfig->ScoreChange = this->SetOption("score-change", config, this->ProjectConfig->ScoreChange).toInt();
    this->ProjectConfig->ScoreUser = this->SetOption("score-user", config, this->ProjectConfig->ScoreUser).toInt();
    this->ProjectConfig->ScoreTalk = this->SetOption("score-talk", config, this->ProjectConfig->ScoreTalk).toInt();
    this->ProjectConfig->WarningDefs = this->SetUserOptionList("warning-template-tags", config, this->ProjectConfig->WarningDefs);
    this->ProjectConfig->BotScore = this->SetOption("score-bot", config, this->ProjectConfig->BotScore).toInt();
    HuggleQueueFilter::Filters += HuggleParser::ConfigurationParseQueueList(config, false);
    this->ProjectConfig->ConfirmMultipleEdits = SafeBool(ConfigurationParse("confirm-multiple", config), this->ProjectConfig->ConfirmMultipleEdits);
    this->ProjectConfig->ConfirmTalk = SafeBool(ConfigurationParse("confirm-talk", config), this->ProjectConfig->ConfirmTalk);
    this->ProjectConfig->ConfirmOnSelfRevs = SafeBool(ConfigurationParse("confirm-self-revert", config), this->ProjectConfig->ConfirmOnSelfRevs);
    this->ProjectConfig->ConfirmWL = SafeBool(ConfigurationParse("confirm-whitelist", config), this->ProjectConfig->ConfirmWL);
    this->UserConfig->DisplayTitle = SafeBool(ConfigurationParse("DisplayTitle", config, "false"));
    this->UserConfig->TruncateEdits = SafeBool(ConfigurationParse("TruncateEdits", config, "false"));
    this->UserConfig->HistoryLoad = SafeBool(ConfigurationParse("HistoryLoad", config, "true"));
    this->UserConfig->LastEdit = SafeBool(ConfigurationParse("SkipToLastEdit", config, "false"));
    this->UserConfig->CheckTP = SafeBool(ConfigurationParse("CheckTP", config, "true"));
    this->UserConfig->HAN_DisplayBots = SafeBool(ConfigurationParse("HAN_DisplayBots", config, "true"));
    this->UserConfig->HAN_DisplayUser = SafeBool(ConfigurationParse("HAN_DisplayUser", config, "true"));
    this->UserConfig->ManualWarning = SafeBool(ConfigurationParse("ManualWarning", config, "true"));
    this->UserConfig->HAN_DisplayUserTalk = SafeBool(ConfigurationParse("HAN_DisplayUserTalk", config, "true"));
    this->HtmlAllowedInIrc = SafeBool(ConfigurationParse("HAN_Html", config, "false"));
    this->UserConfig->TalkPageFreshness = ConfigurationParse("TalkpageFreshness", config, QString::number(this->UserConfig->TalkPageFreshness)).toInt();
    this->UserConfig->RemoveOldQueueEdits = SafeBool(ConfigurationParse("RemoveOldestQueueEdits", config, "false"));
    this->UserConfig->QueueID = ConfigurationParse("QueueID", config);
    this->UserConfig->GoNext = static_cast<Configuration_OnNext>(ConfigurationParse("OnNext", config, "1").toInt());
    this->UserConfig->DeleteEditsAfterRevert = SafeBool(ConfigurationParse("DeleteEditsAfterRevert", config, "true"));
    this->UserConfig->WelcomeGood = this->SetOption("welcome-good", config, this->ProjectConfig->WelcomeGood).toBool();
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
        if (!this->Shortcuts.contains(id))
        {
            Syslog::HuggleLogs->WarningLog("Invalid shortcut in user configuration (missing id): " + line);
            continue;
        }
        this->Shortcuts[id].Modified = true;
        this->Shortcuts[id].QAccel = QKeySequence(parts[1]).toString();
    }
    this->NormalizeConf();
    /// \todo Lot of configuration options are missing
    return true;
}

QString Configuration::GetExtensionConfig(QString extension, QString name, QString ms)
{
    if (!this->ExtensionData.contains(extension))
        return ms;
    return this->ExtensionData[extension]->GetOption(name, ms);
}

QDateTime Configuration::ServerTime()
{
    return QDateTime::currentDateTime().addSecs(this->ServerOffset);
}

QString Configuration::GetProjectURL(WikiSite *Project)
{
    return Configuration::HuggleConfiguration->GetURLProtocolPrefix() + Project->URL;
}

QString Configuration::GetProjectWikiURL(WikiSite *Project)
{
    return Configuration::GetProjectURL(Project) + Project->LongPath;
}

QString Configuration::GetProjectScriptURL(WikiSite *Project)
{
    return Configuration::GetProjectURL(Project) + Project->ScriptPath;
}

QString Configuration::GetProjectURL()
{
    return Configuration::GetURLProtocolPrefix() + Configuration::HuggleConfiguration->Project->URL;
}

QString Configuration::GetProjectWikiURL()
{
    return Configuration::GetProjectURL(Configuration::HuggleConfiguration->Project) + Configuration::HuggleConfiguration->Project->LongPath;
}

QString Configuration::GetProjectScriptURL()
{
    return Configuration::GetProjectURL(Configuration::HuggleConfiguration->Project) + Configuration::HuggleConfiguration->Project->ScriptPath;
}

void Configuration::InsertConfig(QString key, QString value, QXmlStreamWriter *s)
{
    s->writeStartElement("local");
    s->writeAttribute("key", key);
    s->writeAttribute("text", value);
    s->writeEndElement();
}

Shortcut::Shortcut()
{
    this->ID = -1;
    this->Description = "";
    this->Name = "";
    this->QAccel = "";
}

Shortcut::Shortcut(QString name, QString description)
{
    this->Name = name;
    this->Description = description;
    this->QAccel = "";
    this->ID = -1;
    // resolve ID from name
    if (name == "main-exit")
        this->ID = HUGGLE_ACCEL_MAIN_EXIT;
    else if (name == "main-revert-and-warn")
        this->ID = HUGGLE_ACCEL_MAIN_REVERT_AND_WARN;
    else if (name == "main-revert")
        this->ID = HUGGLE_ACCEL_MAIN_REVERT;
    else if (name == "main-warn")
        this->ID = HUGGLE_ACCEL_MAIN_WARN;
    else if (name == "main-suspicious-edit")
        this->ID = HUGGLE_ACCEL_SUSPICIOUS_EDIT;
    else if (name == "main-forward")
        this->ID = HUGGLE_ACCEL_MAIN_FORWARD;
    else if (name == "main-next")
        this->ID = HUGGLE_ACCEL_NEXT;
    else if (name == "main-back")
        this->ID = HUGGLE_ACCEL_MAIN_BACK;
    else if (name == "main-good")
        this->ID = HUGGLE_ACCEL_MAIN_GOOD;
    else if (name == "main-open-in-browser")
        this->ID = HUGGLE_ACCEL_MAIN_OPEN_IN_BROWSER;
    else if (name == "main-mytalk")
        this->ID = HUGGLE_ACCEL_MAIN_MYTALK_PAGE;
    else if (name == "main-talk")
        this->ID = HUGGLE_ACCEL_MAIN_TALK;
    else if (name.startsWith("main-revert-and-warn-"))
    {
        if (name == "main-revert-and-warn-0")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_AND_WARN0;
        else if (name == "main-revert-and-warn-1")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_AND_WARN1;
        else if (name == "main-revert-and-warn-2")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_AND_WARN2;
        else if (name == "main-revert-and-warn-3")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_AND_WARN3;
        else if (name == "main-revert-and-warn-4")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_AND_WARN4;
        else if (name == "main-revert-and-warn-5")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_AND_WARN5;
        else if (name == "main-revert-and-warn-6")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_AND_WARN6;
        else if (name == "main-revert-and-warn-7")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_AND_WARN7;
        else if (name == "main-revert-and-warn-8")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_AND_WARN8;
        else if (name == "main-revert-and-warn-9")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_AND_WARN9;
    } else if (name.startsWith("main-revert-"))
    {
        if (name == "main-revert-0")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_0;
        else if (name == "main-revert-1")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_1;
        else if (name == "main-revert-2")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_2;
        else if (name == "main-revert-3")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_3;
        else if (name == "main-revert-4")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_4;
        else if (name == "main-revert-5")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_5;
        else if (name == "main-revert-6")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_6;
        else if (name == "main-revert-7")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_7;
        else if (name == "main-revert-8")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_8;
        else if (name == "main-revert-9")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_9;
    } else if (name.startsWith("main-warn-"))
    {
        if (name == "main-warn-0")
            this->ID = HUGGLE_ACCEL_MAIN_WARN0;
        else if (name == "main-warn-1")
            this->ID = HUGGLE_ACCEL_MAIN_WARN1;
        else if (name == "main-warn-2")
            this->ID = HUGGLE_ACCEL_MAIN_WARN2;
        else if (name == "main-warn-3")
            this->ID = HUGGLE_ACCEL_MAIN_WARN3;
        else if (name == "main-warn-4")
            this->ID = HUGGLE_ACCEL_MAIN_WARN4;
        else if (name == "main-warn-5")
            this->ID = HUGGLE_ACCEL_MAIN_WARN5;
        else if (name == "main-warn-6")
            this->ID = HUGGLE_ACCEL_MAIN_WARN6;
        else if (name == "main-warn-7")
            this->ID = HUGGLE_ACCEL_MAIN_WARN7;
        else if (name == "main-warn-8")
            this->ID = HUGGLE_ACCEL_MAIN_WARN8;
        else if (name == "main-warn-9")
            this->ID = HUGGLE_ACCEL_MAIN_WARN9;
    }
}

Shortcut::Shortcut(const Shortcut &copy)
{
    this->Name = copy.Name;
    this->Modified = copy.Modified;
    this->QAccel = copy.QAccel;
    this->ID = copy.ID;
    this->Description = copy.Description;
}


void ExtensionConfig::SetOption(QString name, QString value)
{
    if (this->Options.contains(name))
    {
        this->Options[name] = value;
        return;
    }
    this->Options.insert(name, value);
}

QString ExtensionConfig::GetOption(QString name, QString md)
{
    // only return the value if we have it
    if (!this->Options.contains(name))
        return md;

    return this->Options[name];
}
