//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "configuration.hpp"

using namespace Huggle;

Configuration * Configuration::HuggleConfiguration = NULL;

Configuration::Configuration()
{
    this->AIVP = NULL;
    this->UAAP = NULL;
    this->Verbosity = 0;
    this->Project = new WikiSite("enwiki", "en.wikipedia.org/", "wiki/", "w/", true, true, "#en.wikipedia", "en");
    this->IndexOfLastWiki = 0;
    this->NewMessage = false;
    this->WelcomeMP = "Project:Huggle/Message";
#ifdef PYTHONENGINE
    this->PythonEngine = true;
#else
    this->PythonEngine = false;
#endif
    //! This is a consumer key for "huggle" on wmf wikis
    this->WmfOAuthConsumerKey = "56a6d6de895e3b859faa57b677f6cd21";
    this->SystemConfig_QueueSize = 200;
    this->Restricted = false;
    this->SystemConfig_ProviderCache = 200;
    this->HuggleVersion = HUGGLE_VERSION;
    this->UsingIRC = true;
    this->IRCIdent = "huggle";
    this->IRCServer = "irc.wikimedia.org";
    this->IRCNick = "huggle";
    this->IRCPort = 6667;
#if QT_VERSION >= 0x050000
    this->HomePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
    this->HomePath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif
    this->SystemConfig_UpdatesEnabled = true;
    this->LocalConfig_EditSuffixOfHuggle = "([[WP:HG|HG 3]])";
    this->WikiDB = "";
    this->UserConfig_HistoryMax = 50;
    this->Platform = HUGGLE_UPDATER_PLATFORM_TYPE;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Global
    //////////////////////////////////////////////////////////////////////////////////////////
    this->GlobalConfigurationWikiAddress = "meta.wikimedia.org/w/";
    this->GlobalConfig_EnableAll = true;
    this->GlobalConfig_MinVersion = HUGGLE_VERSION;
    this->GlobalConfig_LocalConfigWikiPath = "Project:Huggle/Config";
    this->GlobalConfig_DocumentationPath = "https://www.mediawiki.org/wiki/Manual:Huggle";
    this->GlobalConfig_FeedbackPath = "http://en.wikipedia.org/wiki/Wikipedia:Huggle/Feedback";
    this->GlobalConfig_UserConf = "User:$1/huggle3.css";        // user-config-hg3
    this->GlobalConfig_UserConf_old = "User:$1/huggle.css";    // user-config
    this->GlobalConfigWasLoaded = false;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Local
    //////////////////////////////////////////////////////////////////////////////////////////
    this->ProjectConfig_MinimalVersion = "3.0.0.0";
    this->LocalConfig_RevertSummaries.append("Test edits;Reverted edits by [[Special:Contributions/$1|$1]] "\
                                               "identified as test edits");
    this->LocalConfig_UseIrc = false;
    this->ProjectConfig_RequireAdmin = false;
    this->ProjectConfig_RequireAutoconfirmed = false;
    this->ProjectConfig_RequireConfig = false;
    this->ProjectConfig_RequireEdits = 0;
    this->ProjectConfig_RequireRollback = false;
    this->LocalConfig_ConfirmOnSelfRevs = true;
    this->LocalConfig_ConfirmWL = true;
    this->LocalConfig_ConfirmTalk = true;
    this->LocalConfig_SharedIPTemplateTags = "";
    this->LocalConfig_SharedIPTemplate = "";
    this->ProjectConfig_EnableAll = false;
    this->LocalConfig_ScoreTalk = -800;
    this->LocalConfig_AssociatedDelete = "G8. Page dependent on a non-existent or deleted page.";
    this->LocalConfig_DeletionSummaries << "Deleted page using Huggle";

    // Reverting
    this->LocalConfig_MultipleRevertSummary = "Reverted,edit by,edits by,and,other users,to last revision by,to an older version by";
    this->LocalConfig_RollbackSummary = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to last revision by $2";
    this->LocalConfig_DefaultSummary = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to last revision by $2";
    this->LocalConfig_SingleRevert = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])";
    this->LocalConfig_UndoSummary = "Undid edit by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])";
    this->LocalConfig_SoftwareRevertDefaultSummary = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to"\
            " last revision by $2 using huggle software rollback (reverted by $3 revisions to revision $4)";
    this->LocalConfig_RollbackSummaryUnknownTarget = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])";

    // Warnings
    this->LocalConfig_AgfRevert = "Reverted good faith edits";
    this->LocalConfig_WarnSummary = "Warning (level 1)";
    this->LocalConfig_WarnSummary2 = "Warning (level 2)";
    this->LocalConfig_WarnSummary3 = "Warning (level 3)";
    this->LocalConfig_WarnSummary4 = "Warning (level 4)";

    this->LocalConfig_IPScore = 800;
    this->LocalConfig_ForeignUser = 800;
    this->LocalConfig_BotScore = -200;
    this->LocalConfig_ScoreUser = -600;
    this->LocalConfig_WarningScore = 2000;
    this->LocalConfig_TalkPageWarningScore = -800;
    this->LocalConfig_ScoreFlag = -20000;
    this->WarnUserSpaceRoll = true;
    this->LocalConfig_WelcomeGood = true;
    this->LocalConfig_ClearTalkPageTemp = "{{Huggle/Cleared}}";
    this->LocalConfig_WelcomeAnon = "{{subst:Welcome-anon}} ~~~~";
    this->LocalConfig_GlobalRequired = true;
    this->LocalConfig_AIV = false;
    this->LocalConfig_AIVExtend = true;
    this->LocalConfig_ReportAIV = "";
    this->LocalConfig_ReportSt = 0;
    this->LocalConfig_ReportDefaultReason = "vandalism";
    this->LocalConfig_RUTemplateReport = "User $1: $2$3 ~~~~";
    this->LocalConfig_IPVTemplateReport = "User $1: $2$3 ~~~~";
    this->LocalConfig_WhitelistScore = -800;
    this->UserConfig_GoNext = Configuration_OnNext_Next;
    this->TrimOldWarnings = true;
    this->AskUserBeforeReport = true;
    this->QueueNewEditsUp = false;
    this->WelcomeEmpty = true;
    this->LocalConfig_ReportSummary = "Reporting user";

    // these headers are parsed by project config so don't change them
    // no matter if there is a nice function to retrieve them

    // THIS MUST BE HARDCODED SO KEEP IT
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

    this->LocalConfig_NSProject = MEDIAWIKI_DEFAULT_NS_PROJECT;
    this->LocalConfig_NSProjectTalk = MEDIAWIKI_DEFAULT_NS_PROJECTTALK;
    this->LocalConfig_NSTalk = MEDIAWIKI_DEFAULT_NS_TALK;
    this->LocalConfig_NSUser = MEDIAWIKI_DEFAULT_NS_USER;
    this->LocalConfig_NSUserTalk = MEDIAWIKI_DEFAULT_NS_USERTALK;
    this->LocalConfig_NSFile = MEDIAWIKI_DEFAULT_NS_FILE;
    this->LocalConfig_NSFileTalk = MEDIAWIKI_DEFAULT_NS_FILETALK;
    this->LocalConfig_NSCategory = MEDIAWIKI_DEFAULT_NS_CATEGORY;
    this->LocalConfig_NSCategoryTalk = MEDIAWIKI_DEFAULT_NS_CATEGORYTALK;
    this->LocalConfig_NSMediaWiki = MEDIAWIKI_DEFAULT_NS_MEDIAWIKI;
    this->LocalConfig_NSMediaWikiTalk = MEDIAWIKI_DEFAULT_NS_MEDIAWIKITALK;
    this->LocalConfig_NSHelp = MEDIAWIKI_DEFAULT_NS_HELP;
    this->LocalConfig_NSHelpTalk = MEDIAWIKI_DEFAULT_NS_HELPTALK;
    this->LocalConfig_NSPortal = MEDIAWIKI_DEFAULT_NS_PORTAL;
    this->LocalConfig_NSPortalTalk = MEDIAWIKI_DEFAULT_NS_PORTALTALK;
    this->LocalConfig_Patrolling = false;
    this->LocalConfig_TemplateAge = -30;
    this->LocalConfig_ScoreChange = 100;
    this->LocalConfig_UAAavailable = false;
    this->LocalConfig_UAAPath = "Project:Usernames for administrator attention";
    this->LocalConfig_UAATemplate = "* {{user-uaa|1=$1}} $2 ~~~~";
    this->LocalConfig_WelcomeSummary = "Welcoming user";
    this->LocalConfig_WelcomeTitle = "Welcome";

    // Blocking users
    this->LocalConfig_BlockTime = "indefinite";
    this->LocalConfig_BlockTimeAnon = "31 hours";
    this->LocalConfig_BlockMessage = "{{subst:huggle/block|1=$1|2=$2}}";
    this->LocalConfig_BlockMessageIndef = "{{subst:huggle/block-indef|1=$1}}";
    this->LocalConfig_BlockReason = "[[WP:VAND|Vandalism]]";
    this->LocalConfig_BlockSummary = "Notification: Blocked";
    this->LocalConfig_RestoreSummary = "Restored revision $1 made by $2";
    this->LocalConfig_ProtectReason = "Persistent [[WP:VAND|vandalism]]";
    this->LocalConfig_BlockExpiryOptions.append("indefinite");
    this->LocalConfig_ConfirmMultipleEdits = false;
    this->LocalConfig_ConfirmRange = false;
    this->LocalConfig_ConfirmPage = false;
    this->LocalConfig_ConfirmSame = false;
    this->LocalConfig_ConfirmWarned = false;


    //////////////////////////////////////////////////////////////////////////////////////////
    // User
    //////////////////////////////////////////////////////////////////////////////////////////
    this->UserConfig_LastEdit = false;
    this->UserConfig_AutomaticallyResolveConflicts = false;
    this->UserConfig_RevertNewBySame = true;
    this->UserConfig_HistoryLoad = true;
    this->UserConfig_TruncateEdits = false;
    // we need to maintain some compatibility with older huggle
    this->UserConfig_EnforceMonthsAsHeaders = true;
    this->UserConfig_DeleteEditsAfterRevert = true;
    this->UserConfig_SectionKeep = true;
    this->UserConfig_TalkPageFreshness = 20;

    //////////////////////////////////////////////////////////////////////////////////////////
    //System
    //////////////////////////////////////////////////////////////////////////////////////////

    this->SystemConfig_WhitelistDisabled = false;
    this->SystemConfig_QueryListTimeLimit = 2;
    this->SystemConfig_HistorySize = 20;
    this->SystemConfig_SyslogPath = "huggle.log";
    this->SystemConfig_Log2File = false;
    this->SystemConfig_FontSize = 10;
    this->SystemConfig_WriteTimeout = 200;
    this->SystemConfig_ReadTimeout = 60;
    this->SystemConfig_SafeMode = false;
    this->SystemConfig_CacheHAN = 100;
    this->SystemConfig_UsingSSL = true;
    this->SystemConfig_Username = "User";
    this->SystemConfig_DynamicColsInList = false;
    this->SystemConfig_RingLogMaxSize = 2000;
    this->SystemConfig_IRCConnectionTimeOut = 2;
    this->SystemConfig_LanguageSanity = false;
    this->SystemConfig_Dot = false;

    // Temporary only
    this->TemporaryConfig_EditToken = "";
    this->TemporaryConfig_Password = "";

    // Unknown parts
    this->RevertOnMultipleEdits = false;
    this->EnforceManualSoftwareRollback = false;
    this->VandalNw_Server = "irc.tm-irc.org";
    this->VandalNw_Login = true;
}

Configuration::~Configuration()
{
    delete this->AIVP;
    delete this->Project;
    delete this->UAAP;
}

Option *Configuration::GetOption(QString key)
{
    if (this->Options.contains(key))
    {
        return new Option(this->Options[key]);
    }
    return NULL;
}

QString Configuration::GenerateSuffix(QString text)
{
    if (!text.endsWith(this->LocalConfig_EditSuffixOfHuggle))
    {
        text = text + " " + this->LocalConfig_EditSuffixOfHuggle;
    }

    return text;
}

QString Configuration::GetExtensionsRootPath()
{
    QString path_ = Configuration::HuggleConfiguration->HomePath + QDir::separator() + EXTENSION_PATH;
    QDir conf(path_);
    if (!conf.exists())
    {
        if(!conf.mkpath(path_))
        {
            Syslog::HuggleLogs->WarningLog("Unable to create " + path_);
        }
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

QString Configuration::GetURLProtocolPrefix()
{
    if (!Configuration::HuggleConfiguration->SystemConfig_UsingSSL)
    {
        return "http://";
    }
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

QString Configuration::GetDefaultRevertSummary(QString source)
{
    QString summary = Configuration::HuggleConfiguration->LocalConfig_DefaultSummary;
    summary = summary.replace("$1", source) + " " + Configuration::HuggleConfiguration->LocalConfig_EditSuffixOfHuggle;
    return summary;
}

QString Configuration::Bool2ExcludeRequire(bool b)
{
    if (b)
    {
        return "exclude";
    }
    return "require";
}

QString Configuration::Bool2String(bool b)
{
    if (b)
    {
        return "true";
    }
    return "false";
}

void Configuration::NormalizeConf()
{
    if (this->LocalConfig_TemplateAge > -1)
    {
        this->LocalConfig_TemplateAge = -30;
    }
    if (this->SystemConfig_QueueSize < 10)
    {
        this->SystemConfig_QueueSize = 10;
    }
    if (this->SystemConfig_HistorySize < 2)
    {
        this->SystemConfig_HistorySize = 2;
    }
}

bool Configuration::SafeBool(QString value, bool defaultvalue)
{
    if (value.toLower() == "true")
    {
        return true;
    }
    return defaultvalue;
}

QString Configuration::MakeLocalUserConfig()
{
    /// \todo rewrite to save all SharedConfig only if different from project
    QString configuration_ = "<nowiki>\n";
    configuration_ += "enable:true\n";
    configuration_ += "version:" + Configuration::HuggleConfiguration->HuggleVersion + "\n\n";
    configuration_ += "speedy-message-title:Speedy deleted\n";
    configuration_ += "report-summary:" + Configuration::HuggleConfiguration->LocalConfig_ReportSummary + "\n";
    configuration_ += "prod-message-summary:Notification: Proposed deletion of [[$1]]\n";
    configuration_ += "warn-summary-4:" + Configuration::HuggleConfiguration->LocalConfig_WarnSummary4 + "\n";
    configuration_ += "warn-summary-3:" + Configuration::HuggleConfiguration->LocalConfig_WarnSummary3 + "\n";
    configuration_ += "warn-summary-2:" + Configuration::HuggleConfiguration->LocalConfig_WarnSummary2 + "\n";
    configuration_ += "warn-summary:" + Configuration::HuggleConfiguration->LocalConfig_WarnSummary + "\n";
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // huggle 2 options
    configuration_ += "auto-advance:false\n";
    configuration_ += "auto-whitelist:true\n";
    configuration_ += "username-listed:true\n";
    configuration_ += "admin:true\n";
    configuration_ += "patrol-speedy:true\n";
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    configuration_ += "confirm-multiple:" + Configuration::Bool2String(Configuration::HuggleConfiguration->LocalConfig_ConfirmMultipleEdits) + "\n";
    configuration_ += "confirm-page:" + Configuration::Bool2String(Configuration::HuggleConfiguration->LocalConfig_ConfirmPage) + "\n";
    configuration_ += "confirm-same:" + Configuration::Bool2String(Configuration::HuggleConfiguration->LocalConfig_ConfirmSame) + "\n";
    configuration_ += "confirm-self-revert:" + Configuration::Bool2String(Configuration::HuggleConfiguration->LocalConfig_ConfirmOnSelfRevs) + "\n";
    configuration_ += "confirm-warned:" + Configuration::Bool2String(Configuration::HuggleConfiguration->LocalConfig_ConfirmWarned) + "\n";
    configuration_ += "confirm-range:" + Configuration::Bool2String(Configuration::HuggleConfiguration->LocalConfig_ConfirmRange) + "\n";
    configuration_ += "default-summary:" + Configuration::HuggleConfiguration->LocalConfig_DefaultSummary + "\n";
    configuration_ += "// this option will change the behaviour of automatic resolution, be carefull\n";
    configuration_ += "revert-auto-multiple-edits:" + Configuration::Bool2String(Configuration::HuggleConfiguration->RevertOnMultipleEdits) + "\n";
    configuration_ += "automatically-resolve-conflicts:" +
            Configuration::Bool2String(Configuration::HuggleConfiguration->UserConfig_AutomaticallyResolveConflicts) + "\n";
    configuration_ += "confirm-page:" + Configuration::Bool2String(Configuration::HuggleConfiguration->LocalConfig_ConfirmPage) + "\n";
    configuration_ += "template-age:" + QString::number(Configuration::HuggleConfiguration->LocalConfig_TemplateAge) + "\n";
    configuration_ += "confirm-same:" + Configuration::Bool2String(Configuration::HuggleConfiguration->LocalConfig_ConfirmSame) + "\n";
    configuration_ += "software-rollback:" + Configuration::Bool2String(Configuration::HuggleConfiguration->EnforceManualSoftwareRollback) + "\n";
    configuration_ += "diff-font-size:" + QString::number(Configuration::HuggleConfiguration->SystemConfig_FontSize) + "\n";
    configuration_ += "RevertOnMultipleEdits:" + Configuration::Bool2String(Configuration::HuggleConfiguration->RevertOnMultipleEdits) + "\n";
    configuration_ += "HistoryLoad:" + Configuration::Bool2String(Configuration::HuggleConfiguration->UserConfig_HistoryLoad) + "\n";
    configuration_ += "OnNext:" + QString::number(static_cast<int>(Configuration::HuggleConfiguration->UserConfig_GoNext)) + "\n";
    configuration_ += "DeleteEditsAfterRevert:" + Configuration::Bool2String(Configuration::HuggleConfiguration->UserConfig_DeleteEditsAfterRevert) + "\n";
    configuration_ += "SkipToLastEdit:" + Configuration::Bool2String(Configuration::HuggleConfiguration->UserConfig_LastEdit) + "\n";
    configuration_ += "TruncateEdits:" + Configuration::Bool2String(Configuration::HuggleConfiguration->UserConfig_TruncateEdits) + "\n";
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
            configuration_ += "\n";
        }
    }
    /// \todo Missing options
    configuration_ += "</nowiki>";
    return configuration_;
}

void Configuration::LoadSystemConfig()
{
    QFile file(Configuration::GetConfigurationPath() + "huggle3.xml");
    Huggle::Syslog::HuggleLogs->Log("Home: " + Configuration::GetConfigurationPath());
    if (!QFile().exists(Configuration::GetConfigurationPath() + "huggle3.xml"))
    {
        Huggle::Syslog::HuggleLogs->DebugLog("No config file at " + Configuration::GetConfigurationPath() + "huggle3.xml");
        return;
    }
    if(!file.open(QIODevice::ReadOnly))
    {
        Huggle::Syslog::HuggleLogs->DebugLog("Unable to read config file");
        return;
    }
    QDomDocument conf;
    conf.setContent(file.readAll());
    QDomNodeList l = conf.elementsByTagName("local");
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
            Configuration::HuggleConfiguration->AskUserBeforeReport = Configuration::SafeBool(option.attribute("text"));
            continue;
        }
        if (key == "HistorySize")
        {
            Configuration::HuggleConfiguration->SystemConfig_HistorySize = option.attribute("text").toInt();
            continue;
        }
        if (key == "VandalNw_Login")
        {
            Configuration::HuggleConfiguration->VandalNw_Login = Configuration::SafeBool(option.attribute("text"));
            continue;
        }
        if (key == "UserName")
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
            Configuration::HuggleConfiguration->SystemConfig_DynamicColsInList = Configuration::SafeBool(option.attribute("text"));
            continue;
        }
        if (key == "WarnUserSpaceRoll")
        {
            Configuration::HuggleConfiguration->WarnUserSpaceRoll = Configuration::SafeBool(option.attribute("text"));
            continue;
        }
        if (key == "EnableUpdates")
        {
            Configuration::HuggleConfiguration->SystemConfig_UpdatesEnabled = Configuration::SafeBool(option.attribute("text"));
            continue;
        }
        if (key == "QueueNewEditsUp")
        {
            Configuration::HuggleConfiguration->QueueNewEditsUp = Configuration::SafeBool(option.attribute("text"));
            continue;
        }
        if (key == "IndexOfLastWiki")
        {
            Configuration::HuggleConfiguration->IndexOfLastWiki = option.attribute("text").toInt();
            continue;
        }
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
    QXmlStreamWriter *x = new QXmlStreamWriter();
    x->setDevice(&file);
    x->writeStartDocument();
    x->writeStartElement("huggle");
    InsertConfig("Cache_InfoSize", QString::number(Configuration::HuggleConfiguration->SystemConfig_QueueSize), x);
    InsertConfig("GlobalConfigurationWikiAddress", Configuration::HuggleConfiguration->GlobalConfigurationWikiAddress, x);
    InsertConfig("IRCIdent", Configuration::HuggleConfiguration->IRCIdent, x);
    InsertConfig("IRCNick", Configuration::HuggleConfiguration->IRCNick, x);
    InsertConfig("IRCPort", QString::number(Configuration::HuggleConfiguration->IRCPort), x);
    InsertConfig("IRCServer", Configuration::HuggleConfiguration->IRCServer, x);
    InsertConfig("Language", Localizations::HuggleLocalizations->PreferredLanguage, x);
    InsertConfig("ProviderCache", QString::number(Configuration::HuggleConfiguration->SystemConfig_ProviderCache), x);
    InsertConfig("AskUserBeforeReport", Configuration::Bool2String(Configuration::HuggleConfiguration->AskUserBeforeReport), x);
    InsertConfig("HistorySize", QString::number(Configuration::HuggleConfiguration->SystemConfig_HistorySize), x);
    InsertConfig("QueueNewEditsUp", Configuration::Bool2String(Configuration::HuggleConfiguration->QueueNewEditsUp), x);
    InsertConfig("RingLogMaxSize", QString::number(Configuration::HuggleConfiguration->SystemConfig_RingLogMaxSize), x);
    InsertConfig("TrimOldWarnings", Configuration::Bool2String(Configuration::HuggleConfiguration->TrimOldWarnings), x);
    InsertConfig("EnableUpdates", Configuration::Bool2String(Configuration::HuggleConfiguration->SystemConfig_UpdatesEnabled), x);
    InsertConfig("WarnUserSpaceRoll", Configuration::Bool2String(Configuration::HuggleConfiguration->WarnUserSpaceRoll), x);
    InsertConfig("UserName", Configuration::HuggleConfiguration->SystemConfig_Username, x);
    InsertConfig("IndexOfLastWiki", QString::number(Configuration::HuggleConfiguration->IndexOfLastWiki), x);
    InsertConfig("DynamicColsInList", Configuration::Bool2String(Configuration::HuggleConfiguration->SystemConfig_DynamicColsInList), x);
    /////////////////////////////
    // Vandal network
    /////////////////////////////
    InsertConfig("VandalNw_Login", Configuration::Bool2String(Configuration::HuggleConfiguration->VandalNw_Login), x);
    x->writeEndElement();
    x->writeEndDocument();
    delete x;
}

bool Configuration::ParseGlobalConfig(QString config)
{
    this->GlobalConfig_EnableAll = SafeBool(ConfigurationParse("enable-all", config));
    this->GlobalConfig_DocumentationPath = ConfigurationParse("documentation", config, this->GlobalConfig_DocumentationPath);
    this->GlobalConfig_FeedbackPath = ConfigurationParse("feedback", config, this->GlobalConfig_FeedbackPath);
    // Sanitize page titles (huggle2 done sth. similiar at Page.SanitizeTitle before requesting them)
    this->GlobalConfig_UserConf = ReplaceSpecialUserPage(ConfigurationParse("user-config-hg3", config));
    this->GlobalConfig_UserConf_old = ReplaceSpecialUserPage(ConfigurationParse("user-config", config));
    this->GlobalConfigWasLoaded = true;
    return true;
}

bool Configuration::ParseProjectConfig(QString config)
{
    //AIV
    this->LocalConfig_AIV = SafeBool(ConfigurationParse("aiv-reports", config));
    this->LocalConfig_AIVExtend = SafeBool(ConfigurationParse("aiv-extend", config));
    this->LocalConfig_ReportAIV = ConfigurationParse("aiv", config);
    this->LocalConfig_ReportSt = ConfigurationParse("aiv-section", config).toInt();
    this->LocalConfig_IPVTemplateReport = ConfigurationParse("aiv-ip", config, "User $1: $2$3 ~~~~");
    this->LocalConfig_RUTemplateReport = ConfigurationParse("aiv-user", config, "User $1: $2$3 ~~~~");
    this->LocalConfig_ReportDefaultReason = ConfigurationParse("vandal-report-reason", config, "Persistent vandalism and/or "\
                                                               "unconstructive edits found with [[WP:HG|Huggle 3]].");
    // Restrictions
    this->ProjectConfig_EnableAll = SafeBool(ConfigurationParse("enable-all", config));
    this->ProjectConfig_RequireAdmin = SafeBool(ConfigurationParse("require-admin", config));
    this->ProjectConfig_RequireAutoconfirmed = SafeBool(ConfigurationParse("require-autoconfirmed", config, "false"));
    this->ProjectConfig_RequireConfig = SafeBool(ConfigurationParse("require-config", config, "false"));
    this->ProjectConfig_RequireEdits = ConfigurationParse("require-edits", config, "0").toInt();
    this->ProjectConfig_RequireRollback = SafeBool(ConfigurationParse("require-rollback", config));
    // IRC
    this->LocalConfig_UseIrc = SafeBool(ConfigurationParse("irc", config));
    // Ignoring
    this->LocalConfig_Ignores = HuggleParser::ConfigurationParse_QL("ignore", config, true);
    this->LocalConfig_IgnorePatterns = HuggleParser::ConfigurationParse_QL("ignore-patterns", config, true);
    // Scoring
    this->LocalConfig_IPScore = ConfigurationParse("score-ip", config, "800").toInt();
    this->LocalConfig_ScoreFlag = ConfigurationParse("score-flag", config).toInt();
    this->LocalConfig_ForeignUser = ConfigurationParse("score-foreign-user", config, "200").toInt();
    this->LocalConfig_BotScore = ConfigurationParse("score-bot", config, "-200000").toInt();
    this->LocalConfig_ScoreUser = ConfigurationParse("score-user", config, "-200").toInt();
    this->LocalConfig_ScoreTalk = ConfigurationParse("score-talk", config, "-800").toInt();
    // Summaries
    this->LocalConfig_WarnSummary = ConfigurationParse("warn-summary", config);
    this->LocalConfig_WarnSummary2 = ConfigurationParse("warn-summary-2", config);
    this->LocalConfig_DefaultSummary = ConfigurationParse("default-summary", config,
              "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to last revision by $2");
    this->LocalConfig_AgfRevert = ConfigurationParse("agf", config,
              "Reverted good faith edits by [[Special:Contributions/$2|$2]] [[User talk:$2|talk]]");
    this->LocalConfig_WarnSummary3 = ConfigurationParse("warn-summary-3", config);
    this->LocalConfig_WarnSummary4 = ConfigurationParse("warn-summary-4", config);
    this->LocalConfig_RevertSummaries = HuggleParser::ConfigurationParse_QL("template-summ", config);
    this->LocalConfig_RollbackSummary = ConfigurationParse("rollback-summary", config,
              "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to last revision by $2");
    this->LocalConfig_SingleRevert = ConfigurationParse("single-revert-summary", config,
              "Undid edit by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])");
    this->LocalConfig_UndoSummary = ConfigurationParse("undo-summary", config);
    this->LocalConfig_SoftwareRevertDefaultSummary = ConfigurationParse("manual-revert-summary", config,
              "Reverted edits by [[Special:Contributions/$1|$1]] to last revision by $2");
    this->LocalConfig_MultipleRevertSummary = ConfigurationParse("multiple-revert-summary-parts", config,
              "Reverted,edit by,edits by,and,other users,to last revision by,to an older version by");
    this->LocalConfig_RollbackSummaryUnknownTarget = ConfigurationParse("rollback-summary-unknown",
              config, "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])");
    // Warning types
    this->LocalConfig_WarningTypes = HuggleParser::ConfigurationParse_QL("warning-types", config);
    this->LocalConfig_WarningDefs = HuggleParser::ConfigurationParse_QL("warning-template-tags", config);
    // Reverting
    this->LocalConfig_ConfirmMultipleEdits = SafeBool(ConfigurationParse("confirm-multiple", config));
    this->LocalConfig_ConfirmRange = SafeBool(ConfigurationParse("confirm-range", config));
    this->LocalConfig_ConfirmSame = SafeBool(ConfigurationParse("confirm-same", config));
    this->LocalConfig_ConfirmWarned = SafeBool(ConfigurationParse("confirm-warned", config));
    this->UserConfig_AutomaticallyResolveConflicts = SafeBool(ConfigurationParse("automatically-resolve-conflicts", config), false);
    // Welcoming
    this->WelcomeMP = ConfigurationParse("startup-message-location", config, "Project:Huggle/Message");
    this->LocalConfig_WelcomeGood = SafeBool(ConfigurationParse("welcome-on-good-edit", config, "true"));
    this->LocalConfig_WelcomeTypes = HuggleParser::ConfigurationParse_QL("welcome-messages", config);
    // Reporting
    this->LocalConfig_Patrolling = SafeBool(ConfigurationParse("patrolling-enabled", config));
    this->LocalConfig_ReportSummary = ConfigurationParse("report-summary", config);
    this->LocalConfig_SpeedyTemplates = HuggleParser::ConfigurationParse_QL("speedy-options", config);
    // Parsing
    this->LocalConfig_TemplateAge = ConfigurationParse("template-age", config, QString::number(this->LocalConfig_TemplateAge)).toInt();
    // UAA
    this->LocalConfig_UAAPath = ConfigurationParse("uaa", config);
    // Blocking
    this->LocalConfig_BlockMessage = ConfigurationParse("block-message", config);
    this->LocalConfig_BlockReason = ConfigurationParse("block-reason", config);
    this->LocalConfig_BlockExpiryOptions.clear();
    // Templates
    this->LocalConfig_Headings = HeadingsStandard;
    QString headings = ConfigurationParse("headings", config, "standard");
    if (headings == "page")
    {
        this->LocalConfig_Headings = HeadingsPageName;
        this->UserConfig_EnforceMonthsAsHeaders = false;
    } else if(headings == "none")
    {
        this->LocalConfig_Headings = HeadingsNone;
        this->UserConfig_EnforceMonthsAsHeaders = false;
    }
    QString Options = ConfigurationParse("block-expiry-options", config);
    QStringList list = Options.split(",");
    while (list.count() > 0)
    {
        QString item = list.at(0);
        item = item.trimmed();
        this->LocalConfig_BlockExpiryOptions.append(item);
        list.removeAt(0);
    }
    this->LocalConfig_DeletionSummaries = HuggleParser::ConfigurationParseTrimmed_QL("deletion-reasons", config, false);
    this->LocalConfig_BlockSummary = ConfigurationParse("block-summary", config, "Notification: Blocked");
    this->LocalConfig_BlockTime = ConfigurationParse("blocktime", config, "indef");
    this->LocalConfig_ClearTalkPageTemp = ConfigurationParse("template-clear-talk-page", config, "{{Huggle/Cleared}}");
    this->LocalConfig_Assisted = HuggleParser::ConfigurationParse_QL("assisted-summaries", config, true);
    this->LocalConfig_SharedIPTemplateTags = ConfigurationParse("shared-ip-template-tag", config, "");
    this->LocalConfig_SharedIPTemplate = ConfigurationParse("shared-ip-template", config, "");
    this->LocalConfig_ProtectReason =  ConfigurationParse("protection-reason", config, "Excessive [[Wikipedia:Vandalism|vandalism]]");
    this->LocalConfig_RevertPatterns = HuggleParser::ConfigurationParse_QL("revert-patterns", config, true);
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
    this->RevertPatterns.clear();
    int xx = 0;
    while (xx < this->LocalConfig_RevertPatterns.count())
    {
        this->RevertPatterns.append(QRegExp(this->LocalConfig_RevertPatterns.at(xx)));
        xx++;
    }
    HuggleQueueFilter::Filters += HuggleParser::ConfigurationParseQueueList(config, true);

    if (this->AIVP != NULL)
    {
        delete this->AIVP;
    }

    this->AIVP = new WikiPage(this->LocalConfig_ReportAIV);
    HuggleParser::ParsePats(config);
    HuggleParser::ParseWords(config);
    QStringList namespaces = HuggleParser::ConfigurationParse_QL("namespace-names", config, true);
    int NS=0;
    while (namespaces.count() > NS)
    {
        QString line = namespaces.at(NS);
        NS++;

        if (!line.contains(";"))
        {
            continue;
        }

        int ns = line.mid(0, line.indexOf(";")).toInt();
        QString name = line.mid(line.indexOf(";") + 1) + ":";

        switch (ns)
        {
            /// \todo Some NS are missing here
            case MEDIAWIKI_NSID_TALK:
                this->LocalConfig_NSTalk = name;
                break;
            case MEDIAWIKI_NSID_CATEGORY:
                this->LocalConfig_NSCategory = name;
                break;
            case MEDIAWIKI_NSID_CATEGORYTALK:
                this->LocalConfig_NSCategoryTalk = name;
                break;
            case MEDIAWIKI_NSID_FILE:
                this->LocalConfig_NSFile = name;
                break;
            case MEDIAWIKI_NSID_FILETALK:
                this->LocalConfig_NSFileTalk = name;
                break;
            case MEDIAWIKI_NSID_HELP:
                this->LocalConfig_NSHelp = name;
                break;
            case MEDIAWIKI_NSID_HELPTALK:
                this->LocalConfig_NSHelpTalk = name;
                break;
            case MEDIAWIKI_NSID_MEDIAWIKI:
                this->LocalConfig_NSMediaWiki = name;
                break;
            case MEDIAWIKI_NSID_MEDIAWIKITALK:
                this->LocalConfig_NSMediaWikiTalk = name;
                break;
            case MEDIAWIKI_NSID_PORTAL:
                this->LocalConfig_NSPortal = name;
                break;
            case MEDIAWIKI_NSID_PORTALTALK:
                this->LocalConfig_NSPortalTalk = name;
                break;
            case MEDIAWIKI_NSID_PROJECT:
                this->LocalConfig_NSProject = name;
                break;
            case MEDIAWIKI_NSID_PROJECTTALK:
                this->LocalConfig_NSProjectTalk = name;
                break;
            case MEDIAWIKI_NSID_USER:
                this->LocalConfig_NSUser = name;
                break;
            case MEDIAWIKI_NSID_USERTALK:
                this->LocalConfig_NSUserTalk = name;
                break;
            case MEDIAWIKI_NSID_TEMPLATE:
                this->LocalConfig_NSTemplate = name;
                break;
            case MEDIAWIKI_NSID_TEMPLATETALK:
                this->LocalConfig_NSTemplateTalk = name;
                break;
        }
    }

    if (this->UAAP != NULL)
    {
        delete this->UAAP;
    }

    this->UAAP = new WikiPage(this->LocalConfig_UAAPath);

    // templates
    int CurrentTemplate=0;
    while (CurrentTemplate<this->LocalConfig_WarningTypes.count())
    {
        QString type = HuggleParser::GetKeyFromValue(this->LocalConfig_WarningTypes.at(CurrentTemplate));
        int CurrentWarning = 1;
        while (CurrentWarning <= 4)
        {
            QString xx = ConfigurationParse(type + QString::number(CurrentWarning), config);
            if (xx != "")
            {
                this->LocalConfig_WarningTemplates.append(type + QString::number(CurrentWarning) + ";" + xx);
            }
            CurrentWarning++;
        }
        CurrentTemplate++;
    }
    // sanitize
    if (this->LocalConfig_ReportAIV == "")
    {
        this->LocalConfig_AIV = false;
    }
    // Do the same for UAA as well
    if (this->LocalConfig_UAAPath == "")
    {
        this->LocalConfig_UAAavailable = false;
    }
    return true;
}

bool Configuration::ParseUserConfig(QString config)
{
    this->RevertOnMultipleEdits = SafeBool(ConfigurationParse("RevertOnMultipleEdits", config));
    this->ProjectConfig_EnableAll = SafeBool(ConfigurationParse("enable", config));
    this->LocalConfig_Ignores = HuggleParser::ConfigurationParse_QL("ignore", config, this->LocalConfig_Ignores);
    this->LocalConfig_IPScore = ConfigurationParse("score-ip", config, QString::number(this->LocalConfig_IPScore)).toInt();
    this->LocalConfig_ScoreFlag = ConfigurationParse("score-flag", config, QString::number(this->LocalConfig_ScoreFlag)).toInt();
    this->LocalConfig_WarnSummary = ConfigurationParse("warn-summary", config, this->LocalConfig_WarnSummary);
    this->EnforceManualSoftwareRollback = SafeBool(ConfigurationParse("software-rollback", config));
    this->LocalConfig_WarnSummary2 = ConfigurationParse("warn-summary-2", config, this->LocalConfig_WarnSummary2);
    this->LocalConfig_WarnSummary3 = ConfigurationParse("warn-summary-3", config, this->LocalConfig_WarnSummary3);
    this->LocalConfig_WarnSummary4 = ConfigurationParse("warn-summary-4", config, this->LocalConfig_WarnSummary4);
    this->UserConfig_AutomaticallyResolveConflicts = SafeBool(ConfigurationParse("automatically-resolve-conflicts", config), false);
    this->LocalConfig_TemplateAge = ConfigurationParse("template-age", config, QString::number(this->LocalConfig_TemplateAge)).toInt();
    this->LocalConfig_RevertSummaries = HuggleParser::ConfigurationParse_QL("template-summ", config, this->LocalConfig_RevertSummaries);
    this->LocalConfig_WarningTypes = HuggleParser::ConfigurationParse_QL("warning-types", config, this->LocalConfig_WarningTypes);
    this->LocalConfig_ScoreChange = ConfigurationParse("score-change", config, QString::number(this->LocalConfig_ScoreChange)).toInt();
    this->LocalConfig_ScoreFlag = ConfigurationParse("score-flag", config, QString::number(this->LocalConfig_ScoreFlag)).toInt();
    this->LocalConfig_ScoreUser = ConfigurationParse("score-user", config, QString::number(this->LocalConfig_ScoreUser)).toInt();
    this->LocalConfig_ScoreTalk = ConfigurationParse("score-talk", config, QString::number(this->LocalConfig_ScoreTalk)).toInt();
    this->LocalConfig_WarningDefs = HuggleParser::ConfigurationParse_QL("warning-template-tags", config, this->LocalConfig_WarningDefs);
    this->LocalConfig_BotScore = ConfigurationParse("score-bot", config, QString::number(this->LocalConfig_BotScore)).toInt();
    HuggleQueueFilter::Filters += HuggleParser::ConfigurationParseQueueList(config, false);
    this->UserConfig_TruncateEdits = SafeBool(ConfigurationParse("TruncateEdits", config, "false"));
    this->UserConfig_HistoryLoad = SafeBool(ConfigurationParse("HistoryLoad", config, "true"));
    this->UserConfig_LastEdit = SafeBool(ConfigurationParse("SkipToLastEdit", config, "false"));
    this->UserConfig_GoNext = static_cast<Configuration_OnNext>(ConfigurationParse("OnNext", config, "1").toInt());
    this->UserConfig_DeleteEditsAfterRevert = SafeBool(ConfigurationParse("DeleteEditsAfterRevert", config, "true"));
    this->NormalizeConf();
    /// \todo Lot of configuration options are missing
    return true;
}

QString Configuration::ConfigurationParse(QString key, QString content, QString missing)
{
    /// \todo this parses the config a lot different than HG2 (here only one line, mising replaces...)
    /// \todo maybe move it to Huggle::HuggleParser like ConfigurationParse_QL
    // if first line in config
    if (content.startsWith(key + ":"))
    {
        QString value = content.mid(key.length() + 1);
        if (value.contains("\n"))
        {
            value = value.mid(0, value.indexOf("\n"));
        }
        return value;
    }

    // make sure it's not inside of some string
    if (content.contains("\n" + key + ":"))
    {
        QString value = content.mid(content.indexOf("\n" + key + ":") + key.length() + 2);
        if (value.contains("\n"))
        {
            value = value.mid(0, value.indexOf("\n"));
        }
        return value;
    }
    return missing;
}

void Configuration::InsertConfig(QString key, QString value, QXmlStreamWriter *s)
{
    s->writeStartElement("local");
    s->writeAttribute("key", key);
    s->writeAttribute("text", value);
    s->writeEndElement();
}

ScoreWord::ScoreWord(QString Word, int Score)
{
    this->score = Score;
    this->word = Word;
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

Option::Option()
{
    this->Name = "";
    this->Type = OptionType_String;
    this->ContainerString = "";
}

Option::Option(QString name, OptionType type)
{
    this->Name = name;
    this->Type = type;
}

Option::Option(Option *option)
{
    this->Name = option->Name;
    this->Type = option->Type;
}

Option::Option(const Option &option)
{
    this->Name = option.Name;
    this->Type = option.Type;
}
