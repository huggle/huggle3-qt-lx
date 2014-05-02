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
#include <QDesktopServices>
#include "syslog.hpp"
#include "huggleparser.hpp"
#include "localization.hpp"

using namespace Huggle;

Configuration * Configuration::HuggleConfiguration = NULL;

Configuration::Configuration()
{
    this->AIVP = NULL;
    this->UAAP = NULL;
    this->Verbosity = 0;
    //this->Project = new WikiSite("enwiki", "en.wikipedia.org/", "wiki/", "w/", true, true, "#en.wikipedia", "en");
    this->Project = NULL;
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
    this->ProjectConfig_EditSuffixOfHuggle = "([[WP:HG|HG 3]])";
    this->WikiDB = "";
    this->UserConfig_HistoryMax = 50;
    this->Platform = HUGGLE_UPDATER_PLATFORM_TYPE;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Global
    //////////////////////////////////////////////////////////////////////////////////////////
    this->GlobalConfig_Whitelist = "http://huggle.wmflabs.org/data/";
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
    // Local (project wide)
    //////////////////////////////////////////////////////////////////////////////////////////
    this->ProjectConfig_MinimalVersion = "3.0.0.0";
    this->ProjectConfig_RevertSummaries.append("Test edits;Reverted edits by [[Special:Contributions/$1|$1]] "\
                                               "identified as test edits");
    this->ProjectConfig_UseIrc = false;
    this->ProjectConfig_RequireAdmin = false;
    this->ProjectConfig_RequireAutoconfirmed = false;
    this->ProjectConfig_RequireConfig = false;
    this->ProjectConfig_RequireEdits = 0;
    this->ProjectConfig_RequireRollback = false;
    this->ProjectConfig_ConfirmOnSelfRevs = true;
    this->ProjectConfig_ConfirmWL = true;
    this->ProjectConfig_ConfirmTalk = true;
    this->ProjectConfig_SharedIPTemplateTags = "";
    this->ProjectConfig_SharedIPTemplate = "";
    this->ProjectConfig_EnableAll = false;
    this->ProjectConfig_WarningLevel = 4;
    this->ProjectConfig_ScoreTalk = -800;
    this->ProjectConfig_AssociatedDelete = "G8. Page dependent on a non-existent or deleted page.";
    this->ProjectConfig_DeletionSummaries << "Deleted page using Huggle";

    // Reverting
    this->ProjectConfig_MultipleRevertSummary = "Reverted,edit by,edits by,and,other users,to last revision by,to an older version by";
    this->ProjectConfig_RollbackSummary = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to last revision by $2";
    this->ProjectConfig_DefaultSummary = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to last revision by $2";
    this->ProjectConfig_SingleRevert = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])";
    this->ProjectConfig_UndoSummary = "Undid edit by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])";
    this->ProjectConfig_SoftwareRevertDefaultSummary = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to"\
            " last revision by $2 using huggle software rollback (reverted by $3 revisions to revision $4)";
    this->ProjectConfig_RollbackSummaryUnknownTarget = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])";

    // Warnings
    this->ProjectConfig_AgfRevert = "Reverted good faith edits";
    this->ProjectConfig_WarnSummary = "Warning (level 1)";
    this->ProjectConfig_WarnSummary2 = "Warning (level 2)";
    this->ProjectConfig_WarnSummary3 = "Warning (level 3)";
    this->ProjectConfig_WarnSummary4 = "Warning (level 4)";

    this->ProjectConfig_IPScore = 800;
    this->ProjectConfig_ForeignUser = 800;
    this->ProjectConfig_BotScore = -200;
    this->ProjectConfig_ScoreUser = -600;
    this->ProjectConfig_WarningScore = 2000;
    this->ProjectConfig_TalkPageWarningScore = -800;
    this->ProjectConfig_ScoreFlag = -60;
    this->WarnUserSpaceRoll = true;
    this->ProjectConfig_WelcomeGood = true;
    this->ProjectConfig_ClearTalkPageTemp = "{{Huggle/Cleared}}";
    this->ProjectConfig_WelcomeAnon = "{{subst:Welcome-anon}} ~~~~";
    this->ProjectConfig_GlobalRequired = true;
    this->ProjectConfig_AIV = false;
    this->ProjectConfig_AIVExtend = true;
    this->ProjectConfig_ReportAIV = "";
    this->ProjectConfig_SpeedyWarningSummary = "Sending user a notification regarding deletion of their page";
    this->ProjectConfig_ReportSt = 0;
    this->ProjectConfig_SpeedyEditSummary = "Tagging page for deletion";
    this->ProjectConfig_ReportDefaultReason = "vandalism";
    this->ProjectConfig_RUTemplateReport = "User $1: $2$3 ~~~~";
    this->ProjectConfig_IPVTemplateReport = "User $1: $2$3 ~~~~";
    this->ProjectConfig_WhitelistScore = -800;
    this->TrimOldWarnings = true;
    this->AskUserBeforeReport = true;
    this->WelcomeEmpty = true;
    this->ProjectConfig_ReportSummary = "Reporting user";

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

    this->ProjectConfig_NSProject = MEDIAWIKI_DEFAULT_NS_PROJECT;
    this->ProjectConfig_NSProjectTalk = MEDIAWIKI_DEFAULT_NS_PROJECTTALK;
    this->ProjectConfig_NSTalk = MEDIAWIKI_DEFAULT_NS_TALK;
    this->ProjectConfig_NSUser = MEDIAWIKI_DEFAULT_NS_USER;
    this->ProjectConfig_NSUserTalk = MEDIAWIKI_DEFAULT_NS_USERTALK;
    this->ProjectConfig_NSFile = MEDIAWIKI_DEFAULT_NS_FILE;
    this->ProjectConfig_NSFileTalk = MEDIAWIKI_DEFAULT_NS_FILETALK;
    this->ProjectConfig_NSCategory = MEDIAWIKI_DEFAULT_NS_CATEGORY;
    this->ProjectConfig_NSCategoryTalk = MEDIAWIKI_DEFAULT_NS_CATEGORYTALK;
    this->ProjectConfig_NSMediaWiki = MEDIAWIKI_DEFAULT_NS_MEDIAWIKI;
    this->ProjectConfig_NSMediaWikiTalk = MEDIAWIKI_DEFAULT_NS_MEDIAWIKITALK;
    this->ProjectConfig_NSHelp = MEDIAWIKI_DEFAULT_NS_HELP;
    this->ProjectConfig_NSHelpTalk = MEDIAWIKI_DEFAULT_NS_HELPTALK;
    this->ProjectConfig_NSPortal = MEDIAWIKI_DEFAULT_NS_PORTAL;
    this->ProjectConfig_NSPortalTalk = MEDIAWIKI_DEFAULT_NS_PORTALTALK;
    this->ProjectConfig_Patrolling = false;
    this->ProjectConfig_TemplateAge = -30;
    this->ProjectConfig_ScoreChange = 100;
    this->ProjectConfig_UAAavailable = false;
    this->ProjectConfig_UAAPath = "Project:Usernames for administrator attention";
    this->ProjectConfig_UAATemplate = "* {{user-uaa|1=$1}} $2 ~~~~";
    this->ProjectConfig_WelcomeSummary = "Welcoming user";
    this->ProjectConfig_WelcomeTitle = "Welcome";

    // Blocking users
    this->ProjectConfig_BlockTime = "indefinite";
    this->ProjectConfig_BlockTimeAnon = "31 hours";
    this->ProjectConfig_BlockMessage = "{{subst:huggle/block|1=$1|2=$2}}";
    this->ProjectConfig_BlockMessageIndef = "{{subst:huggle/block-indef|1=$1}}";
    this->ProjectConfig_BlockReason = "[[WP:VAND|Vandalism]]";
    this->ProjectConfig_BlockSummary = "Notification: Blocked";
    this->ProjectConfig_RestoreSummary = "Restored revision $1 made by $2";
    this->ProjectConfig_ProtectReason = "Persistent [[WP:VAND|vandalism]]";
    this->ProjectConfig_BlockExpiryOptions.append("indefinite");

    // RFPP
    this->ProjectConfig_RFPP_Page = "";
    this->ProjectConfig_RFPP_PlaceTop = false;
    this->ProjectConfig_RFPP_Summary = "Sending request to protect a page";
    this->ProjectConfig_RFPP = false;
    this->ProjectConfig_RFPP_Template = "";
    this->ProjectConfig_RFPP_TemplateUser = "";
    this->ProjectConfig_RFPP_Regex = "";
    this->ProjectConfig_ConfirmMultipleEdits = false;
    this->ProjectConfig_ConfirmRange = false;
    this->ProjectConfig_ConfirmPage = false;
    this->ProjectConfig_ConfirmSame = false;
    this->ProjectConfig_ConfirmWarned = false;
    this->ForcedNoEditJump = false;

    //////////////////////////////////////////////////////////////////////////////////////////
    // User
    //////////////////////////////////////////////////////////////////////////////////////////
    this->UserConfig_ManualWarning = false;
    this->UserConfig_HAN_DisplayBots = true;
    this->UserConfig_HAN_DisplayUserTalk = true;
    this->UserConfig_HAN_DisplayUser = true;
    this->UserConfig_LastEdit = false;
    this->UserConfig_AutomaticallyResolveConflicts = false;
    this->UserConfig_RevertNewBySame = true;
    this->UserConfig_RemoveOldQueueEdits = false;
    this->UserConfig_HistoryLoad = true;
    this->UserConfig_TruncateEdits = false;
    // we need to maintain some compatibility with older huggle
    this->UserConfig_EnforceMonthsAsHeaders = true;
    this->UserConfig_DeleteEditsAfterRevert = true;
    this->UserConfig_SectionKeep = true;
    this->UserConfig_TalkPageFreshness = 20;
    this->UserConfig_DisplayTitle = false;
    this->UserConfig_GoNext = Configuration_OnNext_Next;
    this->UserConfig_CheckTP = false;

    //////////////////////////////////////////////////////////////////////////////////////////
    // System (pc wide)
    //////////////////////////////////////////////////////////////////////////////////////////

    this->SystemConfig_DelayVal = 0;
    this->SystemConfig_RequestDelay = false;
    this->SystemConfig_WhitelistDisabled = false;
    this->SystemConfig_RevertDelay = 0;
    this->SystemConfig_InstantReverts = true;
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
    this->SystemConfig_GlobalConfigWikiList = "Project:Huggle/List";
    this->SystemConfig_Username = "User";
    this->SystemConfig_DynamicColsInList = false;
    this->SystemConfig_RingLogMaxSize = 2000;
    this->SystemConfig_IRCConnectionTimeOut = 2;
    this->SystemConfig_LanguageSanity = false;
    this->SystemConfig_Dot = false;
    this->SystemConfig_QueueNewEditsUp = false;

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
    if (!text.endsWith(this->ProjectConfig_EditSuffixOfHuggle))
    {
        text = text + " " + this->ProjectConfig_EditSuffixOfHuggle;
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
    QString summary = Configuration::HuggleConfiguration->ProjectConfig_DefaultSummary;
    summary = summary.replace("$1", source) + " " + Configuration::HuggleConfiguration->ProjectConfig_EditSuffixOfHuggle;
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
    if (this->ProjectConfig_TemplateAge > -1)
    {
        this->ProjectConfig_TemplateAge = -30;
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
    configuration_ += "report-summary:" + Configuration::HuggleConfiguration->ProjectConfig_ReportSummary + "\n";
    configuration_ += "prod-message-summary:Notification: Proposed deletion of [[$1]]\n";
    configuration_ += "warn-summary-4:" + Configuration::HuggleConfiguration->ProjectConfig_WarnSummary4 + "\n";
    configuration_ += "warn-summary-3:" + Configuration::HuggleConfiguration->ProjectConfig_WarnSummary3 + "\n";
    configuration_ += "warn-summary-2:" + Configuration::HuggleConfiguration->ProjectConfig_WarnSummary2 + "\n";
    configuration_ += "warn-summary:" + Configuration::HuggleConfiguration->ProjectConfig_WarnSummary + "\n";
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // huggle 2 options
    configuration_ += "auto-advance:false\n";
    configuration_ += "auto-whitelist:true\n";
    configuration_ += "username-listed:true\n";
    configuration_ += "admin:true\n";
    configuration_ += "patrol-speedy:true\n";
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    configuration_ += "confirm-multiple:" + Configuration::Bool2String(Configuration::HuggleConfiguration->ProjectConfig_ConfirmMultipleEdits) + "\n";
    configuration_ += "confirm-page:" + Configuration::Bool2String(Configuration::HuggleConfiguration->ProjectConfig_ConfirmPage) + "\n";
    configuration_ += "confirm-same:" + Configuration::Bool2String(Configuration::HuggleConfiguration->ProjectConfig_ConfirmSame) + "\n";
    configuration_ += "confirm-self-revert:" + Configuration::Bool2String(Configuration::HuggleConfiguration->ProjectConfig_ConfirmOnSelfRevs) + "\n";
    configuration_ += "confirm-warned:" + Configuration::Bool2String(Configuration::HuggleConfiguration->ProjectConfig_ConfirmWarned) + "\n";
    configuration_ += "confirm-range:" + Configuration::Bool2String(Configuration::HuggleConfiguration->ProjectConfig_ConfirmRange) + "\n";
    configuration_ += "default-summary:" + Configuration::HuggleConfiguration->ProjectConfig_DefaultSummary + "\n";
    configuration_ += "// this option will change the behaviour of automatic resolution, be carefull\n";
    configuration_ += "revert-auto-multiple-edits:" + Configuration::Bool2String(Configuration::HuggleConfiguration->RevertOnMultipleEdits) + "\n";
    configuration_ += "automatically-resolve-conflicts:" +
            Configuration::Bool2String(Configuration::HuggleConfiguration->UserConfig_AutomaticallyResolveConflicts) + "\n";
    configuration_ += "confirm-page:" + Configuration::Bool2String(Configuration::HuggleConfiguration->ProjectConfig_ConfirmPage) + "\n";
    configuration_ += "template-age:" + QString::number(Configuration::HuggleConfiguration->ProjectConfig_TemplateAge) + "\n";
    configuration_ += "confirm-same:" + Configuration::Bool2String(Configuration::HuggleConfiguration->ProjectConfig_ConfirmSame) + "\n";
    configuration_ += "software-rollback:" + Configuration::Bool2String(Configuration::HuggleConfiguration->EnforceManualSoftwareRollback) + "\n";
    configuration_ += "diff-font-size:" + QString::number(Configuration::HuggleConfiguration->SystemConfig_FontSize) + "\n";
    configuration_ += "RevertOnMultipleEdits:" + Configuration::Bool2String(Configuration::HuggleConfiguration->RevertOnMultipleEdits) + "\n";
    configuration_ += "HistoryLoad:" + Configuration::Bool2String(Configuration::HuggleConfiguration->UserConfig_HistoryLoad) + "\n";
    configuration_ += "OnNext:" + QString::number(static_cast<int>(Configuration::HuggleConfiguration->UserConfig_GoNext)) + "\n";
    configuration_ += "DeleteEditsAfterRevert:" + Configuration::Bool2String(Configuration::HuggleConfiguration->UserConfig_DeleteEditsAfterRevert) + "\n";
    configuration_ += "SkipToLastEdit:" + Configuration::Bool2String(Configuration::HuggleConfiguration->UserConfig_LastEdit) + "\n";
    configuration_ += "RemoveOldestQueueEdits:" + Configuration::Bool2String(Configuration::HuggleConfiguration->UserConfig_RemoveOldQueueEdits) + "\n";
    configuration_ += "TruncateEdits:" + Configuration::Bool2String(Configuration::HuggleConfiguration->UserConfig_TruncateEdits) + "\n";
    configuration_ += "TalkpageFreshness:" + QString::number(Configuration::HuggleConfiguration->UserConfig_TalkPageFreshness) + "\n";
    configuration_ += "DisplayTitle:" + Configuration::Bool2String(Configuration::HuggleConfiguration->UserConfig_DisplayTitle) + "\n";
    configuration_ += "// Periodically check if you received new messages and display a notification box if you get them\n";
    configuration_ += "CheckTP:" + Configuration::Bool2String(Configuration::HuggleConfiguration->UserConfig_CheckTP) + "\n";
    configuration_ += "ManualWarning:" + Configuration::Bool2String(Configuration::HuggleConfiguration->UserConfig_ManualWarning) + "\n";
    configuration_ += "// HAN\n";
    configuration_ += "HAN_DisplayUserTalk:" + Configuration::Bool2String(Configuration::HuggleConfiguration->UserConfig_HAN_DisplayUserTalk) + "\n";
    configuration_ += "HAN_DisplayBots:" + Configuration::Bool2String(Configuration::HuggleConfiguration->UserConfig_HAN_DisplayBots) + "\n";
    configuration_ += "HAN_DisplayUser:" + Configuration::Bool2String(Configuration::HuggleConfiguration->UserConfig_HAN_DisplayUser) + "\n";
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

void Configuration::LoadSystemConfig(QString fn)
{
    QFile file(fn);
    if (!QFile().exists(fn))
    {
        Huggle::Syslog::HuggleLogs->DebugLog("No config file at " + fn);
        return;
    }
    if(!file.open(QIODevice::ReadOnly))
    {
        Huggle::Syslog::HuggleLogs->DebugLog("Unable to read config file");
        return;
    }
    QDomDocument conf;
    conf.setContent(file.readAll());
    file.close();
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
            Configuration::HuggleConfiguration->SystemConfig_QueueNewEditsUp = Configuration::SafeBool(option.attribute("text"));
            continue;
        }
        if (key == "IndexOfLastWiki")
        {
            Configuration::HuggleConfiguration->IndexOfLastWiki = option.attribute("text").toInt();
            continue;
        }
        if (key == "UsingSSL")
        {
            Configuration::HuggleConfiguration->SystemConfig_UsingSSL = Configuration::SafeBool(option.attribute("text"));
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
        if (key == "RequestDelay")
        {
            Configuration::HuggleConfiguration->SystemConfig_RequestDelay = Configuration::SafeBool(option.attribute("text"));
            continue;
        }
        if (key == "RevertDelay")
        {
            Configuration::HuggleConfiguration->SystemConfig_RevertDelay = option.attribute("text").toUInt();
            continue;
        }
        if (key == "InstantReverts")
        {
            Configuration::HuggleConfiguration->SystemConfig_InstantReverts = Configuration::SafeBool(option.attribute("text"));
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
    QXmlStreamWriter *writer = new QXmlStreamWriter();
    writer->setDevice(&file);
    writer->setAutoFormatting(true);
    writer->writeStartDocument();
    writer->writeStartElement("huggle");
    InsertConfig("DelayVal", QString::number(Configuration::HuggleConfiguration->SystemConfig_DelayVal), writer);
    InsertConfig("RequestDelay", Configuration::Bool2String(Configuration::HuggleConfiguration->SystemConfig_RequestDelay), writer);
    InsertConfig("RevertDelay", QString::number(Configuration::HuggleConfiguration->SystemConfig_RevertDelay), writer);
    InsertConfig("InstantReverts", Configuration::Bool2String(Configuration::HuggleConfiguration->SystemConfig_InstantReverts), writer);
    InsertConfig("UsingSSL", Configuration::Bool2String(Configuration::HuggleConfiguration->SystemConfig_UsingSSL), writer);
    InsertConfig("Cache_InfoSize", QString::number(Configuration::HuggleConfiguration->SystemConfig_QueueSize), writer);
    InsertConfig("GlobalConfigurationWikiAddress", Configuration::HuggleConfiguration->GlobalConfigurationWikiAddress, writer);
    InsertConfig("IRCIdent", Configuration::HuggleConfiguration->IRCIdent, writer);
    InsertConfig("IRCNick", Configuration::HuggleConfiguration->IRCNick, writer);
    InsertConfig("IRCPort", QString::number(Configuration::HuggleConfiguration->IRCPort), writer);
    InsertConfig("IRCServer", Configuration::HuggleConfiguration->IRCServer, writer);
    InsertConfig("Language", Localizations::HuggleLocalizations->PreferredLanguage, writer);
    InsertConfig("ProviderCache", QString::number(Configuration::HuggleConfiguration->SystemConfig_ProviderCache), writer);
    InsertConfig("AskUserBeforeReport", Configuration::Bool2String(Configuration::HuggleConfiguration->AskUserBeforeReport), writer);
    InsertConfig("HistorySize", QString::number(Configuration::HuggleConfiguration->SystemConfig_HistorySize), writer);
    InsertConfig("QueueNewEditsUp", Configuration::Bool2String(Configuration::HuggleConfiguration->SystemConfig_QueueNewEditsUp), writer);
    InsertConfig("RingLogMaxSize", QString::number(Configuration::HuggleConfiguration->SystemConfig_RingLogMaxSize), writer);
    InsertConfig("TrimOldWarnings", Configuration::Bool2String(Configuration::HuggleConfiguration->TrimOldWarnings), writer);
    InsertConfig("EnableUpdates", Configuration::Bool2String(Configuration::HuggleConfiguration->SystemConfig_UpdatesEnabled), writer);
    InsertConfig("WarnUserSpaceRoll", Configuration::Bool2String(Configuration::HuggleConfiguration->WarnUserSpaceRoll), writer);
    InsertConfig("UserName", Configuration::HuggleConfiguration->SystemConfig_Username, writer);
    InsertConfig("IndexOfLastWiki", QString::number(Configuration::HuggleConfiguration->IndexOfLastWiki), writer);
    InsertConfig("DynamicColsInList", Configuration::Bool2String(Configuration::HuggleConfiguration->SystemConfig_DynamicColsInList), writer);
    /////////////////////////////
    // Vandal network
    /////////////////////////////
    InsertConfig("VandalNw_Login", Configuration::Bool2String(Configuration::HuggleConfiguration->VandalNw_Login), writer);
    InsertConfig("GlobalConfigWikiList", Configuration::HuggleConfiguration->SystemConfig_GlobalConfigWikiList, writer);
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
    this->GlobalConfigWasLoaded = true;
    return true;
}

bool Configuration::ParseProjectConfig(QString config)
{
    //AIV
    this->ProjectConfig_AIV = SafeBool(ConfigurationParse("aiv-reports", config));
    this->ProjectConfig_AIVExtend = SafeBool(ConfigurationParse("aiv-extend", config));
    this->ProjectConfig_ReportAIV = ConfigurationParse("aiv", config);
    this->ProjectConfig_ReportSt = ConfigurationParse("aiv-section", config).toInt();
    this->ProjectConfig_IPVTemplateReport = ConfigurationParse("aiv-ip", config, "User $1: $2$3 ~~~~");
    this->ProjectConfig_RUTemplateReport = ConfigurationParse("aiv-user", config, "User $1: $2$3 ~~~~");
    this->ProjectConfig_ReportDefaultReason = ConfigurationParse("vandal-report-reason", config, "Persistent vandalism and/or "\
                                                                 "unconstructive edits found with [[WP:HG|Huggle 3]].");
    // Restrictions
    this->ProjectConfig_EnableAll = SafeBool(ConfigurationParse("enable-all", config));
    this->ProjectConfig_RequireAdmin = SafeBool(ConfigurationParse("require-admin", config));
    this->ProjectConfig_RequireAutoconfirmed = SafeBool(ConfigurationParse("require-autoconfirmed", config, "false"));
    this->ProjectConfig_RequireConfig = SafeBool(ConfigurationParse("require-config", config, "false"));
    this->ProjectConfig_RequireEdits = ConfigurationParse("require-edits", config, "0").toInt();
    this->ProjectConfig_RequireRollback = SafeBool(ConfigurationParse("require-rollback", config));
    // IRC
    this->ProjectConfig_UseIrc = SafeBool(ConfigurationParse("irc", config));
    // Ignoring
    this->ProjectConfig_Ignores = HuggleParser::ConfigurationParse_QL("ignore", config, true);
    this->ProjectConfig_IgnorePatterns = HuggleParser::ConfigurationParse_QL("ignore-patterns", config, true);
    // Scoring
    this->ProjectConfig_IPScore = ConfigurationParse("score-ip", config, "800").toInt();
    this->ProjectConfig_ScoreFlag = ConfigurationParse("score-flag", config).toInt();
    this->ProjectConfig_ForeignUser = ConfigurationParse("score-foreign-user", config, "200").toInt();
    this->ProjectConfig_BotScore = ConfigurationParse("score-bot", config, "-200000").toInt();
    this->ProjectConfig_ScoreUser = ConfigurationParse("score-user", config, "-200").toInt();
    this->ProjectConfig_ScoreTalk = ConfigurationParse("score-talk", config, "-800").toInt();
    // Summaries
    this->ProjectConfig_WarnSummary = ConfigurationParse("warn-summary", config);
    this->ProjectConfig_WarnSummary2 = ConfigurationParse("warn-summary-2", config);
    this->ProjectConfig_DefaultSummary = ConfigurationParse("default-summary", config,
              "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to last revision by $2");
    this->ProjectConfig_AgfRevert = ConfigurationParse("agf", config, "Reverted good faith edits by [[Special:Contributions/$2|$2]]"\
                                                       " [[User talk:$2|talk]]");
    this->ProjectConfig_WarnSummary3 = ConfigurationParse("warn-summary-3", config);
    this->ProjectConfig_WarnSummary4 = ConfigurationParse("warn-summary-4", config);
    this->ProjectConfig_RevertSummaries = HuggleParser::ConfigurationParse_QL("template-summ", config);
    this->ProjectConfig_RollbackSummary = ConfigurationParse("rollback-summary", config,
              "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to last revision by $2");
    this->ProjectConfig_SingleRevert = ConfigurationParse("single-revert-summary", config,
              "Undid edit by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])");
    this->ProjectConfig_UndoSummary = ConfigurationParse("undo-summary", config);
    this->ProjectConfig_SoftwareRevertDefaultSummary = ConfigurationParse("manual-revert-summary", config,
              "Reverted edits by [[Special:Contributions/$1|$1]] to last revision by $2");
    this->ProjectConfig_MultipleRevertSummary = ConfigurationParse("multiple-revert-summary-parts", config,
              "Reverted,edit by,edits by,and,other users,to last revision by,to an older version by");
    this->ProjectConfig_RollbackSummaryUnknownTarget = ConfigurationParse("rollback-summary-unknown",
              config, "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])");
    // Warning types
    this->ProjectConfig_WarningTypes = HuggleParser::ConfigurationParse_QL("warning-types", config);
    this->ProjectConfig_WarningLevel = (byte_ht)ConfigurationParse("warning-mode", config, "4").toInt();
    this->ProjectConfig_WarningDefs = HuggleParser::ConfigurationParse_QL("warning-template-tags", config);
    // Reverting
    this->ProjectConfig_ConfirmMultipleEdits = SafeBool(ConfigurationParse("confirm-multiple", config));
    this->ProjectConfig_ConfirmRange = SafeBool(ConfigurationParse("confirm-range", config));
    this->ProjectConfig_ConfirmSame = SafeBool(ConfigurationParse("confirm-same", config));
    this->ProjectConfig_ConfirmWarned = SafeBool(ConfigurationParse("confirm-warned", config));
    this->UserConfig_AutomaticallyResolveConflicts = SafeBool(ConfigurationParse("automatically-resolve-conflicts", config), false);
    // Welcoming
    this->WelcomeMP = ConfigurationParse("startup-message-location", config, "Project:Huggle/Message");
    this->ProjectConfig_WelcomeGood = SafeBool(ConfigurationParse("welcome-on-good-edit", config, "true"));
    this->ProjectConfig_WelcomeTypes = HuggleParser::ConfigurationParse_QL("welcome-messages", config);
    // Reporting
    this->ProjectConfig_SpeedyEditSummary = ConfigurationParse("speedy-message-summary", config, "Notification: Speedy deletion of [[$1]]");
    this->ProjectConfig_SpeedyWarningSummary = ConfigurationParse("speedy-message-summary", config, "Notification: [[$1]] has been listed for deletion");
    this->ProjectConfig_Patrolling = SafeBool(ConfigurationParse("patrolling-enabled", config));
    this->ProjectConfig_ReportSummary = ConfigurationParse("report-summary", config);
    this->ProjectConfig_SpeedyTemplates = HuggleParser::ConfigurationParse_QL("speedy-options", config);
    // Parsing
    this->ProjectConfig_TemplateAge = ConfigurationParse("template-age", config, QString::number(this->ProjectConfig_TemplateAge)).toInt();
    // UAA
    this->ProjectConfig_UAAPath = ConfigurationParse("uaa", config);
    // Blocking
    this->ProjectConfig_BlockMessage = ConfigurationParse("block-message", config);
    this->ProjectConfig_BlockReason = ConfigurationParse("block-reason", config);
    this->ProjectConfig_BlockExpiryOptions.clear();
    // Templates
    this->ProjectConfig_Headings = HeadingsStandard;
    QString headings = ConfigurationParse("headings", config, "standard");
    if (headings == "page")
    {
        this->ProjectConfig_Headings = HeadingsPageName;
        this->UserConfig_EnforceMonthsAsHeaders = false;
    } else if(headings == "none")
    {
        this->ProjectConfig_Headings = HeadingsNone;
        this->UserConfig_EnforceMonthsAsHeaders = false;
    }
    QString Options = ConfigurationParse("block-expiry-options", config);
    QStringList list = Options.split(",");
    while (list.count() > 0)
    {
        QString item = list.at(0);
        item = item.trimmed();
        this->ProjectConfig_BlockExpiryOptions.append(item);
        list.removeAt(0);
    }
    this->ProjectConfig_DeletionSummaries = HuggleParser::ConfigurationParseTrimmed_QL("deletion-reasons", config, false);
    this->ProjectConfig_BlockSummary = ConfigurationParse("block-summary", config, "Notification: Blocked");
    this->ProjectConfig_BlockTime = ConfigurationParse("blocktime", config, "indef");
    this->ProjectConfig_ClearTalkPageTemp = ConfigurationParse("template-clear-talk-page", config, "{{Huggle/Cleared}}");
    this->ProjectConfig_Assisted = HuggleParser::ConfigurationParse_QL("assisted-summaries", config, true);
    this->ProjectConfig_SharedIPTemplateTags = ConfigurationParse("shared-ip-template-tag", config, "");
    this->ProjectConfig_SharedIPTemplate = ConfigurationParse("shared-ip-template", config, "");
    this->ProjectConfig_ProtectReason =  ConfigurationParse("protection-reason", config, "Excessive [[Wikipedia:Vandalism|vandalism]]");
    this->ProjectConfig_RevertPatterns = HuggleParser::ConfigurationParse_QL("revert-patterns", config, true);
    this->ProjectConfig_RFPP_PlaceTop = SafeBool(ConfigurationParse("protection-request-top", config));
    this->ProjectConfig_RFPP_Regex = ConfigurationParse("rfpp-verify", config);
    this->ProjectConfig_RFPP_Section = (unsigned int)ConfigurationParse("rfpp-section", config, "0").toInt();
    this->ProjectConfig_RFPP_Page = ConfigurationParse("protection-request-page", config);
    this->ProjectConfig_RFPP_Template = ConfigurationParse("rfpp-template", config);
    this->ProjectConfig_RFPP_Summary = ConfigurationParse("protection-request-summary", config, "Request to protect page");
    this->ProjectConfig_RFPP = (this->ProjectConfig_RFPP_Template.length() && this->ProjectConfig_RFPP_Regex.length());
    this->ProjectConfig_RFPP_TemplateUser = ConfigurationParse("rfpp-template-user", config);
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
    while (xx < this->ProjectConfig_RevertPatterns.count())
    {
        this->RevertPatterns.append(QRegExp(this->ProjectConfig_RevertPatterns.at(xx)));
        xx++;
    }
    HuggleQueueFilter::Filters += HuggleParser::ConfigurationParseQueueList(config, true);

    if (this->AIVP != NULL)
    {
        delete this->AIVP;
    }

    this->AIVP = new WikiPage(this->ProjectConfig_ReportAIV);
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
                this->ProjectConfig_NSTalk = name;
                break;
            case MEDIAWIKI_NSID_CATEGORY:
                this->ProjectConfig_NSCategory = name;
                break;
            case MEDIAWIKI_NSID_CATEGORYTALK:
                this->ProjectConfig_NSCategoryTalk = name;
                break;
            case MEDIAWIKI_NSID_FILE:
                this->ProjectConfig_NSFile = name;
                break;
            case MEDIAWIKI_NSID_FILETALK:
                this->ProjectConfig_NSFileTalk = name;
                break;
            case MEDIAWIKI_NSID_HELP:
                this->ProjectConfig_NSHelp = name;
                break;
            case MEDIAWIKI_NSID_HELPTALK:
                this->ProjectConfig_NSHelpTalk = name;
                break;
            case MEDIAWIKI_NSID_MEDIAWIKI:
                this->ProjectConfig_NSMediaWiki = name;
                break;
            case MEDIAWIKI_NSID_MEDIAWIKITALK:
                this->ProjectConfig_NSMediaWikiTalk = name;
                break;
            case MEDIAWIKI_NSID_PORTAL:
                this->ProjectConfig_NSPortal = name;
                break;
            case MEDIAWIKI_NSID_PORTALTALK:
                this->ProjectConfig_NSPortalTalk = name;
                break;
            case MEDIAWIKI_NSID_PROJECT:
                this->ProjectConfig_NSProject = name;
                break;
            case MEDIAWIKI_NSID_PROJECTTALK:
                this->ProjectConfig_NSProjectTalk = name;
                break;
            case MEDIAWIKI_NSID_USER:
                this->ProjectConfig_NSUser = name;
                break;
            case MEDIAWIKI_NSID_USERTALK:
                this->ProjectConfig_NSUserTalk = name;
                break;
            case MEDIAWIKI_NSID_TEMPLATE:
                this->ProjectConfig_NSTemplate = name;
                break;
            case MEDIAWIKI_NSID_TEMPLATETALK:
                this->ProjectConfig_NSTemplateTalk = name;
                break;
        }
    }

    if (this->UAAP != NULL)
    {
        delete this->UAAP;
    }

    this->UAAP = new WikiPage(this->ProjectConfig_UAAPath);

    // templates
    int CurrentTemplate=0;
    while (CurrentTemplate<this->ProjectConfig_WarningTypes.count())
    {
        QString type = HuggleParser::GetKeyFromValue(this->ProjectConfig_WarningTypes.at(CurrentTemplate));
        int CurrentWarning = 1;
        while (CurrentWarning <= 4)
        {
            QString xx = ConfigurationParse(type + QString::number(CurrentWarning), config);
            if (xx != "")
            {
                this->ProjectConfig_WarningTemplates.append(type + QString::number(CurrentWarning) + ";" + xx);
            }
            CurrentWarning++;
        }
        CurrentTemplate++;
    }
    // sanitize
    if (this->ProjectConfig_ReportAIV == "")
        this->ProjectConfig_AIV = false;
    // Do the same for UAA as well
    this->ProjectConfig_UAAavailable = this->ProjectConfig_UAAPath != "";
    return true;
}

bool Configuration::ParseUserConfig(QString config)
{
    this->RevertOnMultipleEdits = SafeBool(ConfigurationParse("RevertOnMultipleEdits", config));
    this->ProjectConfig_EnableAll = SafeBool(ConfigurationParse("enable", config));
    this->ProjectConfig_Ignores = HuggleParser::ConfigurationParse_QL("ignore", config, this->ProjectConfig_Ignores);
    this->ProjectConfig_IPScore = ConfigurationParse("score-ip", config, QString::number(this->ProjectConfig_IPScore)).toInt();
    this->ProjectConfig_ScoreFlag = ConfigurationParse("score-flag", config, QString::number(this->ProjectConfig_ScoreFlag)).toInt();
    this->ProjectConfig_WarnSummary = ConfigurationParse("warn-summary", config, this->ProjectConfig_WarnSummary);
    this->EnforceManualSoftwareRollback = SafeBool(ConfigurationParse("software-rollback", config));
    this->ProjectConfig_WarnSummary2 = ConfigurationParse("warn-summary-2", config, this->ProjectConfig_WarnSummary2);
    this->ProjectConfig_WarnSummary3 = ConfigurationParse("warn-summary-3", config, this->ProjectConfig_WarnSummary3);
    this->ProjectConfig_WarnSummary4 = ConfigurationParse("warn-summary-4", config, this->ProjectConfig_WarnSummary4);
    this->UserConfig_AutomaticallyResolveConflicts = SafeBool(ConfigurationParse("automatically-resolve-conflicts", config), false);
    this->ProjectConfig_TemplateAge = ConfigurationParse("template-age", config, QString::number(this->ProjectConfig_TemplateAge)).toInt();
    this->ProjectConfig_RevertSummaries = HuggleParser::ConfigurationParse_QL("template-summ", config, this->ProjectConfig_RevertSummaries);
    this->ProjectConfig_WarningTypes = HuggleParser::ConfigurationParse_QL("warning-types", config, this->ProjectConfig_WarningTypes);
    this->ProjectConfig_ScoreChange = ConfigurationParse("score-change", config, QString::number(this->ProjectConfig_ScoreChange)).toInt();
    this->ProjectConfig_ScoreFlag = ConfigurationParse("score-flag", config, QString::number(this->ProjectConfig_ScoreFlag)).toInt();
    this->ProjectConfig_ScoreUser = ConfigurationParse("score-user", config, QString::number(this->ProjectConfig_ScoreUser)).toInt();
    this->ProjectConfig_ScoreTalk = ConfigurationParse("score-talk", config, QString::number(this->ProjectConfig_ScoreTalk)).toInt();
    this->ProjectConfig_WarningDefs = HuggleParser::ConfigurationParse_QL("warning-template-tags", config, this->ProjectConfig_WarningDefs);
    this->ProjectConfig_BotScore = ConfigurationParse("score-bot", config, QString::number(this->ProjectConfig_BotScore)).toInt();
    HuggleQueueFilter::Filters += HuggleParser::ConfigurationParseQueueList(config, false);
    this->UserConfig_TruncateEdits = SafeBool(ConfigurationParse("TruncateEdits", config, "false"));
    this->UserConfig_HistoryLoad = SafeBool(ConfigurationParse("HistoryLoad", config, "true"));
    this->UserConfig_LastEdit = SafeBool(ConfigurationParse("SkipToLastEdit", config, "false"));
    this->UserConfig_CheckTP = SafeBool(ConfigurationParse("CheckTP", config, "true"));
    this->UserConfig_HAN_DisplayBots = SafeBool(ConfigurationParse("HAN_DisplayBots", config, "true"));
    this->UserConfig_HAN_DisplayUser = SafeBool(ConfigurationParse("HAN_DisplayUser", config, "true"));
    this->UserConfig_ManualWarning = SafeBool(ConfigurationParse("ManualWarning", config, "true"));
    this->UserConfig_HAN_DisplayUserTalk = SafeBool(ConfigurationParse("HAN_DisplayUserTalk", config, "true"));
    this->UserConfig_TalkPageFreshness = ConfigurationParse("TalkpageFreshness", config, QString::number(this->UserConfig_TalkPageFreshness)).toInt();
    this->UserConfig_RemoveOldQueueEdits = SafeBool(ConfigurationParse("RemoveOldestQueueEdits", config, "false"));
    this->UserConfig_GoNext = static_cast<Configuration_OnNext>(ConfigurationParse("OnNext", config, "1").toInt());
    this->UserConfig_DeleteEditsAfterRevert = SafeBool(ConfigurationParse("DeleteEditsAfterRevert", config, "true"));
    this->NormalizeConf();
    /// \todo Lot of configuration options are missing
    return true;
}

QString Configuration::GetProjectURL(WikiSite Project)
{
    return Configuration::HuggleConfiguration->GetURLProtocolPrefix() + Project.URL;
}

QString Configuration::GetProjectWikiURL(WikiSite Project)
{
    return Configuration::GetProjectURL(Project) + Project.LongPath;
}

QString Configuration::GetProjectScriptURL(WikiSite Project)
{
    return Configuration::GetProjectURL(Project) + Project.ScriptPath;
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
