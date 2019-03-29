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
#include <yaml-cpp/yaml.h>
#include "syslog.hpp"
#include "exception.hpp"
#include "generic.hpp"
#include "hooks.hpp"
#include "huggleoption.hpp"
#include "hugglequeuefilter.hpp"
#include "huggleparser.hpp"
#include "version.hpp"
#include "localization.hpp"
#include "wikipage.hpp"
#include "wikisite.hpp"

using namespace Huggle::HuggleParser;
using namespace Huggle;
using namespace Huggle::Generic;
Configuration * hcfg = nullptr;

Configuration::Configuration()
{
    //! This is a consumer key for "huggle" on wmf wikis
    this->WmfOAuthConsumerKey = "56a6d6de895e3b859faa57b677f6cd21";
    this->HuggleVersion = HUGGLE_VERSION;
#if QT_VERSION >= 0x050000
    this->HomePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
    this->HomePath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif
    this->Platform = HUGGLE_UPDATER_PLATFORM_TYPE;
    this->ProjectConfig = nullptr;
    this->UserConfig = new UserConfiguration();

    this->MakeShortcut("main-revert-and-warn", "shortcut-raw", "Q");
    this->MakeShortcut("main-exit", "shortcut-exit");
    this->MakeShortcut("main-next", "shortcut-next", "Space");
    this->MakeShortcut("main-suspicious-edit", "shortcut-suspicious", "S");
    this->MakeShortcut("main-back", "shortcut-back", "[");
    this->MakeShortcut("main-forward", "shortcut-forward", "]");
    this->MakeShortcut("main-warn", "shortcut-warn", "W");
    this->MakeShortcut("main-revert", "shortcut-revert", "R");
    this->MakeShortcut("main-custom-reason", "shortcut-custom-reason");
    this->MakeShortcut("main-talk", "shortcut-talk", "T");
    this->MakeShortcut("main-mytalk", "shortcut-my-talk", "Alt+M");
    this->MakeShortcut("main-open-in-browser", "shortcut-open", "O");
    this->MakeShortcut("main-good", "shortcut-good", "G");
    this->MakeShortcut("main-watch", "shortcut-watch", "Alt+X");
    this->MakeShortcut("main-unwatch", "shortcut-unwatch", "Alt+C");
    this->MakeShortcut("main-open", "shortcut-open-in-huggle", "Alt+O");
    this->MakeShortcut("main-revert-agf-one-only", "shortcut-revert-agf-1");
    this->MakeShortcut("main-revert-agf", "shortcut-revert-agf", "Y");
    this->MakeShortcut("main-revert-and-stay", "shortcut-revert-and-stay", "Shift+R");
    this->MakeShortcut("main-revert-warn-and-stay", "shortcut-rw-stay", "Shift+Q");
    this->MakeShortcut("main-tab", "shortcut-tab", "Ctrl+T");
    this->MakeShortcut("main-browser-close-tab", "shortcut-browser-close-tab", "Ctrl+W");
    this->MakeShortcut("main-user-report-name", "shortcut-report-username", "Alt+R");
    this->MakeShortcut("main-user-report-user", "shortcut-report-user", "Ctrl+R");
    this->MakeShortcut("main-user-display-contribs", "shortcut-contrib", "C");
    this->MakeShortcut("main-user-contribs-browser", "shortcut-user-contribs-browser", "Shift+C");
    this->MakeShortcut("main-page-patrol-edit", "shortcut-page-patrol-edit");
    this->MakeShortcut("main-page-refresh", "shortcut-refresh", "F5");
    this->MakeShortcut("main-user-clear-tp", "shortcut-user-clear-talk");
    this->MakeShortcut("main-user-custom-msg", "shortcut-custom-msg", "M");
    this->MakeShortcut("main-clear-queue", "shortcut-main-clear-queue", "Ctrl+Space");
    this->MakeShortcut("main-edit", "shortcut-edit", "Shift+E");
    this->MakeShortcut("main-edit-in-browser", "shortcut-edit-in-browser", "E");
    this->ResetMenuShortcuts();
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
    while (this->ProjectList.count())
    {
        delete this->ProjectList.at(0);
        this->ProjectList.removeAt(0);
    }
    delete this->GlobalWiki;
}

QString Configuration::GenerateSuffix(QString text, ProjectConfiguration *conf)
{
    if (hcfg->UserConfig && !hcfg->UserConfig->HuggleSuffix)
        return text;
    if (!text.endsWith(conf->EditSuffixOfHuggle))
    {
        text = text + " " + conf->EditSuffixOfHuggle;
    }
    return text;
}

void Configuration::Logout(WikiSite *site)
{
    site->GetProjectConfig()->RequestLogin();
}

QString Configuration::GetExtensionsRootPath()
{
    QString path_ = Generic::SanitizePath(hcfg->HomePath + QDir::separator() + EXTENSION_PATH + QDir::separator());
    QDir conf(path_);
    if (!conf.exists() && !conf.mkpath(path_))
    {
        Syslog::HuggleLogs->WarningLog("Unable to create " + path_);
    }
    return path_;
}

QString Configuration::GetLocalizationDataPath()
{
    QString path_ = Generic::SanitizePath(hcfg->HomePath + QDir::separator() + "Localization" + QDir::separator());
    QDir conf(path_);
    if (!conf.exists())
    {
        conf.mkpath(path_);
    }
    return path_;
}

QString Configuration::GetURLProtocolPrefix(WikiSite *s)
{
    if (s && !s->SupportHttps)
        return "http://";
    if (!hcfg->SystemConfig_UsingSSL)
        return "http://";

    return "https://";
}

QString Configuration::GetConfigurationPath()
{
    QString path = Generic::SanitizePath(hcfg->HomePath + QDir::separator() + "Configuration" + QDir::separator());
    QDir conf(path);
    if (!conf.exists())
    {
        conf.mkpath(path);
    }
    return path;
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

void Configuration::MakeShortcut(const QString &name, const QString &description, const QString &default_accel)
{
    Shortcut shortcut = Shortcut(name, description);
    shortcut.QAccel = default_accel;
    this->Shortcuts.insert(name, shortcut);
}

void Configuration::NormalizeConf(WikiSite *site)
{
    if (site->ProjectConfig->TemplateAge > -1)
        site->ProjectConfig->TemplateAge = -30;
    if (this->SystemConfig_QueueSize < 10)
        this->SystemConfig_QueueSize = 10;
    if (this->SystemConfig_HistorySize < 2)
        this->SystemConfig_HistorySize = 2;

    if (site->ProjectConfig->MessageHeadings == HeadingsPageName || site->ProjectConfig->MessageHeadings == HeadingsNone)
        site->UserConfig->EnforceMonthsAsHeaders = false;
}

// These macros save us some typing
// For historical purposes and because it's cleaner, we store variables as real variables, rather than hashes indexed by strings,
// this however makes some stuff more complicated, such as dynamic reading and writing of configuration.
// Following function basically consist of huge stack of if {} blocks that check if key name matches the known name.

// These will make the writing of code easier, you basically just type RC(variable_name) for every string where variable_name
// gets substituted to SystemConfig_variable_name, or other version for int and bools

// If you use some exotic variable name or just can't use any of these data types, just write the if condition yourself.

// RC(Test) gets expanded to:
/*
  if (key == "Test")
  {
      hcfg->SystemConfig_Test = option.attribute("text");
      continue;
  }
*/
#define RC(n)   if (key == #n) { hcfg->SystemConfig_##n = option.attribute("text"); continue; }
#define RCU(n)  if (key == #n) { hcfg->SystemConfig_##n = option.attribute("text").toUInt(); continue; }
#define RCB(n)  if (key == #n) { hcfg->SystemConfig_##n = SafeBool(option.attribute("text")); continue; }
#define RCN(n)  if (key == #n) { hcfg->SystemConfig_##n = option.attribute("text").toInt(); continue; }

void Configuration::LoadSystemConfig(const QString& fn)
{
    QFile file(fn);
    if (!QFile::exists(fn))
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
    QDomNodeList ignores = conf.elementsByTagName("ignore");
    int item = 0;
    while (item < ignores.count())
    {
        hcfg->IgnoredExtensions.append(ignores.at(item).toElement().text());
        item++;
    }
    item = 0;
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
        // Load properly named config using macros
        RCB(CatScansAndWatched);
        RCB(Multiple);
        RC(Font);
        RCN(FontSize);
        RCN(QueueSize);
        RC(GlobalConfigurationWikiAddress);
        RC(IRCIdent);
        RC(IRCNick);
        RCU(IRCPort);
        RC(IRCServer);
        RCN(ProviderCache);
        RCB(AskUserBeforeReport);
        RCN(HistorySize);
        RCN(RingLogMaxSize);
        RC(GlobalConfigYAML);
        RCB(DynamicColsInList);
        RCB(UnsafeExts);
        RCB(WarnUserSpaceRoll);
        RCB(EnforceBlackAndWhiteCss);
        RCB(EnableUpdates);
        RCB(NotifyBeta);
        RCB(ParallelLogin);
        RCB(QueueNewEditsUp);
        RCN(IndexOfLastWiki);
        RCB(UsingSSL);
        RC(GlobalConfigWikiList);
        RCU(DelayVal);
        RCU(WikiRC);
        RCB(RequestDelay);
        RCB(BotPassword);
        RCN(RevertDelay);
        RCB(FirstRun);
        RCB(ScoreDebug);
        RCB(ShowStartupInfo);
        RCB(InstantReverts);
        RCB(SuppressWarnings);
        RC(RememberedPassword);
        RCB(StorePassword);
        RCB(UseProxy);
        RC(ProxyUser);
        RCN(ProxyType);
        RC(ProxyHost);
        RCN(KeystrokeMultiPressRate);
        RCB(KeystrokeMultiPressFix);
        RCU(ProxyPort);
        RC(ProxyPass);
        RCB(VandalNw_Login)
        RCB(PlaySoundOnQueue);
        RCB(PlaySoundOnIRCUserMsg);
        RCN(PlaySoundQueueScore);
        RCB(Autorelog);
        if (key == "Projects")
        {
            hcfg->ProjectString = option.attribute("text").split(",");
            continue;
        }
        if (key == "Language")
        {
            Localizations::HuggleLocalizations->PreferredLanguage = option.attribute("text");
            continue;
        }
        if (key == "UserName" && hcfg->SystemConfig_UserName.isEmpty())
        {
            hcfg->SystemConfig_UserName = option.attribute("text");
            continue;
        }
        if (key == "BotLogin" && hcfg->SystemConfig_BotLogin.isEmpty())
        {
            hcfg->SystemConfig_BotLogin = option.attribute("text");
            continue;
        }
    }
    item = 0;
    while (item < e.count())
    {
        QDomElement option = e.at(item).toElement();
        item++;
        if (!option.attributes().contains("extension") || !option.attributes().contains("name"))
            continue;
        QString ExtensionName = option.attribute("extension");
        QString KeyName = option.attribute("name");
        if (!hcfg->ExtensionData.contains(ExtensionName))
        {
            // we need to insert a new option list because this extension is not yet known
            hcfg->ExtensionData.insert(ExtensionName, new ExtensionConfig());
        }
        hcfg->ExtensionData[ExtensionName]->SetOption(KeyName, option.attribute("value"));
    }
    Hooks::OnLocalConfigRead();
    Huggle::Syslog::HuggleLogs->DebugLog("Finished conf");
}

// These macros make our life easier, but work only if you name the variables properly
#define INSERT_CONFIG(name)     InsertConfig(#name, Huggle::Configuration::HuggleConfiguration->SystemConfig_##name, writer)
#define INSERT_CONFIG_B(name)   InsertConfig(#name, Bool2String(Huggle::Configuration::HuggleConfiguration->SystemConfig_##name), writer)
#define INSERT_CONFIG_N(name)   InsertConfig(#name, QString::number(Huggle::Configuration::HuggleConfiguration->SystemConfig_##name), writer)

void Configuration::SaveSystemConfig()
{
    Hooks::OnLocalConfigWrite();
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
    INSERT_CONFIG_B(FirstRun);
    INSERT_CONFIG_B(ShowStartupInfo);
    INSERT_CONFIG_B(Autorelog);
    INSERT_CONFIG_N(DelayVal);
    INSERT_CONFIG_B(RequestDelay);
    INSERT_CONFIG_N(RevertDelay);
    INSERT_CONFIG_B(InstantReverts);
    INSERT_CONFIG_B(UsingSSL);
    INSERT_CONFIG_B(ScoreDebug);
    INSERT_CONFIG_N(QueueSize);
    INSERT_CONFIG_B(UnsafeExts);
    INSERT_CONFIG(GlobalConfigYAML);
    INSERT_CONFIG(GlobalConfigurationWikiAddress);
    INSERT_CONFIG(IRCIdent);
    INSERT_CONFIG(IRCNick);
    INSERT_CONFIG_N(IRCPort);
    INSERT_CONFIG(IRCServer);
    InsertConfig("Language", Localizations::HuggleLocalizations->PreferredLanguage, writer);
    INSERT_CONFIG_N(ProviderCache);
    INSERT_CONFIG_B(AskUserBeforeReport);
    INSERT_CONFIG_N(HistorySize);
    INSERT_CONFIG_B(QueueNewEditsUp);
    INSERT_CONFIG_B(BotPassword);
    INSERT_CONFIG_N(RingLogMaxSize);
    INSERT_CONFIG_B(TrimOldWarnings);
    INSERT_CONFIG_B(EnableUpdates);
    INSERT_CONFIG_B(NotifyBeta);
    INSERT_CONFIG_B(WarnUserSpaceRoll);
    INSERT_CONFIG_B(EnforceBlackAndWhiteCss);
    INSERT_CONFIG_N(WikiRC);
    INSERT_CONFIG_B(CatScansAndWatched);
    INSERT_CONFIG_N(PlaySoundQueueScore);
    INSERT_CONFIG_B(PlaySoundOnQueue);
    INSERT_CONFIG_B(ParallelLogin);
    INSERT_CONFIG_B(PlaySoundOnIRCUserMsg);
    INSERT_CONFIG_N(KeystrokeMultiPressRate);
    INSERT_CONFIG_B(KeystrokeMultiPressFix);
    INSERT_CONFIG(BotLogin);
    INSERT_CONFIG(UserName);
    InsertConfig("IndexOfLastWiki", QString::number(hcfg->SystemConfig_IndexOfLastWiki), writer);
    InsertConfig("DynamicColsInList", Bool2String(hcfg->SystemConfig_DynamicColsInList), writer);
    INSERT_CONFIG_B(Multiple);
    INSERT_CONFIG(Font);
    InsertConfig("FontSize", QString::number(hcfg->SystemConfig_FontSize), writer);
    InsertConfig("SuppressWarnings", Bool2String(hcfg->SystemConfig_SuppressWarnings), writer);
    InsertConfig("StorePassword", Bool2String(hcfg->SystemConfig_StorePassword), writer);
    InsertConfig("UseProxy", Bool2String(hcfg->SystemConfig_UseProxy), writer);
    // Only store password if user wants it
    if (hcfg->SystemConfig_UseProxy)
        InsertConfig("ProxyPass", hcfg->SystemConfig_ProxyPass, writer);
    InsertConfig("ProxyHost", hcfg->SystemConfig_ProxyHost, writer);
    InsertConfig("ProxyUser", hcfg->SystemConfig_ProxyUser, writer);
    InsertConfig("ProxyPort", QString::number(hcfg->SystemConfig_ProxyPort), writer);
    InsertConfig("ProxyType", QString::number(hcfg->SystemConfig_ProxyType), writer);
    if (hcfg->SystemConfig_StorePassword)
        InsertConfig("RememberedPassword", hcfg->SystemConfig_RememberedPassword, writer);
    QString projects;
    foreach (QString wiki, hcfg->ProjectString)
    {
        projects += wiki + ",";
    }
    InsertConfig("Projects", projects, writer);
    /////////////////////////////
    // Vandal network
    /////////////////////////////
    InsertConfig("VandalNw_Login", Bool2String(hcfg->SystemConfig_VandalNw_Login), writer);
    InsertConfig("GlobalConfigWikiList", hcfg->SystemConfig_GlobalConfigWikiList, writer);
    QStringList extensions = hcfg->ExtensionData.keys();
    foreach (QString ex, extensions)
    {
        ExtensionConfig *ed = hcfg->ExtensionData[ex];
        QStringList options = ed->options.keys();
        foreach (QString option, options)
        {
            writer->writeStartElement("extern");
            writer->writeAttribute("extension", ex);
            writer->writeAttribute("name", option);
            writer->writeAttribute("value", ed->options[option]);
            writer->writeEndElement();
        }
    }
    foreach (QString extension, hcfg->IgnoredExtensions)
    {
        writer->writeStartElement("ignore");
        writer->writeCharacters(extension);
        writer->writeEndElement();
    }
    writer->writeEndElement();
    writer->writeEndDocument();
    // close the file
    file.close();
    delete writer;
}

bool Configuration::ParseGlobalConfig(const QString &config)
{
    std::string config_std = FetchYAML(config).toStdString();
    YAML::Node yaml;
    try
    {
        yaml = YAML::Load(config_std);
    } catch (YAML::Exception exception)
    {
        HUGGLE_ERROR("YAML: " + QString(exception.what()));
        return false;
    }
    if (yaml.IsNull())
    {
        HUGGLE_ERROR("Unable to read global configuration: yml node is NULL");
        return false;
    }

    this->GlobalConfig_EnableAll = HuggleParser::YAML2Bool("enable-all", yaml, false);
    this->GlobalConfig_DocumentationPath = HuggleParser::YAML2String("documentation", yaml, this->GlobalConfig_DocumentationPath);
    this->GlobalConfig_FeedbackPath = HuggleParser::YAML2String("feedback", yaml, this->GlobalConfig_FeedbackPath);
    this->GlobalConfig_LocalConfigYAMLPath = HuggleParser::YAML2String("config-yml", yaml, this->GlobalConfig_LocalConfigYAMLPath);
    this->GlobalConfig_LocalConfigWikiPath = HuggleParser::YAML2String("config", yaml, this->GlobalConfig_LocalConfigWikiPath);
    // Sanitize page titles (huggle2 done sth. similiar at Page.SanitizeTitle before requesting them)
    this->GlobalConfig_UserConf = ReplaceSpecialUserPage(HuggleParser::YAML2String("user-config-yaml", yaml, this->GlobalConfig_UserConf));
    this->GlobalConfig_UserConf_old = ReplaceSpecialUserPage(HuggleParser::YAML2String("user-config-hg3", yaml, this->GlobalConfig_UserConf_old));
    this->GlobalConfig_Xmlrcs = HuggleParser::YAML2String("xmlrcs", yaml, "huggle-rc.wmflabs.org");
    this->GlobalConfig_XmlrcsPort = static_cast<quint16>(HuggleParser::YAML2Int("xmlrcs-port", yaml, 8822));
    this->HANMask = HuggleParser::YAML2String("han-mask", yaml, this->HANMask);
    this->GlobalConfig_Whitelist = HuggleParser::YAML2String("whitelist-server", yaml);
    this->VandalNw_Server = HuggleParser::YAML2String("han-server", yaml, this->VandalNw_Server);
    this->SystemConfig_IRCServer = HuggleParser::YAML2String("irc-server", yaml, this->SystemConfig_IRCServer);
    this->GlobalConfigWasLoaded = true;
    return true;
}

QString Configuration::GetExtensionConfig(const QString &extension, const QString &name, const QString &default_value)
{
    if (!this->ExtensionData.contains(extension))
        return default_value;
    return this->ExtensionData[extension]->GetOption(name, default_value);
}

void Configuration::ResetMenuShortcuts()
{
    int i = 0;
    while (i <= HUGGLE_MAX_DROPDOWN_SHORTCUTS)
    {
        QString key = "main-revert-and-warn-" + QString::number(i++);
        if (this->Shortcuts.contains(key))
            this->Shortcuts.remove(key);
        this->MakeShortcut(key, "shortcut-x-raw");
    }

    i = 0;
    while (i <= HUGGLE_MAX_DROPDOWN_SHORTCUTS)
    {
        QString key = "main-warn-" + QString::number(i++);
        if (this->Shortcuts.contains(key))
            this->Shortcuts.remove(key);
        this->MakeShortcut(key, "shortcut-x-warn");
    }

    i = 0;
    while (i <= HUGGLE_MAX_DROPDOWN_SHORTCUTS)
    {
        QString key = "main-revert-" + QString::number(i++);
        if (this->Shortcuts.contains(key))
            this->Shortcuts.remove(key);
        this->MakeShortcut(key, "shortcut-x-revert");
    }
}

void Configuration::SetExtensionConfig(const QString &extension, const QString &name, const QString &value)
{
    if (!this->ExtensionData.contains(extension))
        this->ExtensionData.insert(extension, new ExtensionConfig());
    this->ExtensionData[extension]->SetOption(name, value);
}

bool Configuration::ExtensionConfigContainsKey(const QString &extension, const QString &name)
{
    if (!this->ExtensionData.contains(extension))
        return false;
    return this->ExtensionData[extension]->Contains(name);
}

QString Configuration::GetProjectURL(WikiSite *Project)
{
    return hcfg->GetURLProtocolPrefix(Project) + Project->URL;
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
    return Configuration::GetURLProtocolPrefix() + hcfg->Project->URL;
}

QString Configuration::GetProjectWikiURL()
{
    return Configuration::GetProjectURL(hcfg->Project) + Configuration::HuggleConfiguration->Project->LongPath;
}

QString Configuration::GetProjectScriptURL()
{
    return Configuration::GetProjectURL(hcfg->Project) + Configuration::HuggleConfiguration->Project->ScriptPath;
}

void Configuration::InsertConfig(const QString &key, const QString &value, QXmlStreamWriter *s)
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

Shortcut::Shortcut(const QString &name, const QString &description)
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
    else if (name == "main-watch")
        this->ID = HUGGLE_ACCEL_MAIN_WATCH;
    else if (name == "main-unwatch")
        this->ID = HUGGLE_ACCEL_MAIN_UNWATCH;
    else if (name == "main-tab")
        this->ID = HUGGLE_ACCEL_CREATE_NEW_TAB;
    else if (name == "main-open")
        this->ID = HUGGLE_ACCEL_MAIN_OPEN;
    else if (name == "main-mytalk")
        this->ID = HUGGLE_ACCEL_MAIN_MYTALK_PAGE;
    else if (name == "main-browser-close-tab")
        this->ID = HUGGLE_ACCEL_CLOSE_TAB;
    else if (name == "main-revert-agf-one-only")
        this->ID = HUGGLE_ACCEL_MAIN_REVERT_AGF_ONE_REV;
    else if (name == "main-revert-warn-and-stay")
        this->ID = HUGGLE_ACCEL_REVW_STAY;
    else if (name == "main-revert-and-stay")
        this->ID = HUGGLE_ACCEL_REVERT_STAY;
    else if (name == "main-revert-agf")
        this->ID = HUGGLE_ACCEL_MAIN_REVERT_AGF;
    else if (name == "main-talk")
        this->ID = HUGGLE_ACCEL_MAIN_TALK;
    else if (name == "main-user-report-name")
        this->ID = HUGGLE_ACCEL_USER_REPORT_USER_NAME;
    else if (name == "main-user-report-user")
        this->ID = HUGGLE_ACCEL_USER_REPORT;
    else if (name == "main-user-display-contribs")
        this->ID = HUGGLE_ACCEL_MAIN_USER_CONTRIBUTIONS;
    else if (name == "main-user-contribs-browser")
        this->ID = HUGGLE_ACCEL_MAIN_CONTRIB_BROWSER;
    else if (name == "main-custom-reason")
        this->ID = HUGGLE_ACCEL_MAIN_C_REVERT;
    else if (name == "main-page-patrol-edit")
        this->ID = HUGGLE_ACCEL_MAIN_PATROL;
    else if (name == "main-clear-queue")
        this->ID = HUGGLE_ACCEL_MAIN_CLEAR_QUEUE;
    else if (name == "main-edit")
        this->ID = HUGGLE_ACCEL_MAIN_EDIT;
    else if (name == "main-edit-in-browser")
        this->ID = HUGGLE_ACCEL_MAIN_EDIT_IN_BROWSER;
    else if (name == "main-page-refresh")
        this->ID = HUGGLE_ACCEL_MAIN_REFRESH;
    else if (name == "main-user-clear-tp")
        this->ID = HUGGLE_ACCEL_MAIN_USER_CLEAR_TALK;
    else if (name == "main-user-custom-msg")
        this->ID = HUGGLE_ACCEL_MAIN_USER_CUSTOM_MSG;
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
        else if (name == "main-revert-and-warn-10")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_AND_WARN10;
        else if (name == "main-revert-and-warn-11")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_AND_WARN11;
        else if (name == "main-revert-and-warn-12")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_AND_WARN12;
        else if (name == "main-revert-and-warn-13")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_AND_WARN13;
        else if (name == "main-revert-and-warn-14")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_AND_WARN14;
        else if (name == "main-revert-and-warn-15")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_AND_WARN15;
        else if (name == "main-revert-and-warn-16")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_AND_WARN16;
        else if (name == "main-revert-and-warn-17")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_AND_WARN17;
        else if (name == "main-revert-and-warn-18")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_AND_WARN18;
        else if (name == "main-revert-and-warn-19")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_AND_WARN19;
        else if (name == "main-revert-and-warn-20")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_AND_WARN20;
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
        else if (name == "main-revert-10")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_10;
        else if (name == "main-revert-11")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_11;
        else if (name == "main-revert-12")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_12;
        else if (name == "main-revert-13")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_13;
        else if (name == "main-revert-14")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_14;
        else if (name == "main-revert-15")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_15;
        else if (name == "main-revert-16")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_16;
        else if (name == "main-revert-17")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_17;
        else if (name == "main-revert-18")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_18;
        else if (name == "main-revert-19")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_19;
        else if (name == "main-revert-20")
            this->ID = HUGGLE_ACCEL_MAIN_REVERT_20;
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
        else if (name == "main-warn-10")
            this->ID = HUGGLE_ACCEL_MAIN_WARN10;
        else if (name == "main-warn-11")
            this->ID = HUGGLE_ACCEL_MAIN_WARN11;
        else if (name == "main-warn-12")
            this->ID = HUGGLE_ACCEL_MAIN_WARN12;
        else if (name == "main-warn-13")
            this->ID = HUGGLE_ACCEL_MAIN_WARN13;
        else if (name == "main-warn-14")
            this->ID = HUGGLE_ACCEL_MAIN_WARN14;
        else if (name == "main-warn-15")
            this->ID = HUGGLE_ACCEL_MAIN_WARN15;
        else if (name == "main-warn-16")
            this->ID = HUGGLE_ACCEL_MAIN_WARN16;
        else if (name == "main-warn-17")
            this->ID = HUGGLE_ACCEL_MAIN_WARN17;
        else if (name == "main-warn-18")
            this->ID = HUGGLE_ACCEL_MAIN_WARN18;
        else if (name == "main-warn-19")
            this->ID = HUGGLE_ACCEL_MAIN_WARN19;
        else if (name == "main-warn-20")
            this->ID = HUGGLE_ACCEL_MAIN_WARN20;
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

void ExtensionConfig::SetOption(const QString &name, const QString &value)
{
    if (this->options.contains(name))
    {
        this->options[name] = value;
        return;
    }
    this->options.insert(name, value);
}

QString ExtensionConfig::GetOption(const QString &name, const QString &md)
{
    // only return the value if we have it
    if (!this->options.contains(name))
        return md;

    return this->options[name];
}

bool ExtensionConfig::Contains(const QString &name)
{
    return this->options.contains(name);
}
