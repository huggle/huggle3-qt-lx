//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "configuration.h"

using namespace Huggle;

unsigned int Configuration::Verbosity = 0;
WikiSite Configuration::Project("enwiki", "en.wikipedia.org/");
bool Configuration::UsingSSL = true;
QString Configuration::UserName = "User";
QString Configuration::Password = "";
QString Configuration::WelcomeMP = "Project:Huggle/Message";
QList<WikiSite *> Configuration::ProjectList;
//! This is a consumer key for "huggle" on wmf wikis
QString Configuration::WmfOAuthConsumerKey = "56a6d6de895e3b859faa57b677f6cd21";
int Configuration::Cache_InfoSize = 200;
#ifdef PYTHONENGINE
bool Configuration::PythonEngine = true;
#else
bool Configuration::PythonEngine = false;
#endif
bool Configuration::Restricted = false;
bool Configuration::UsingIRC = true;
QString Configuration::IRCIdent = "huggle";
QString Configuration::IRCServer = "irc.wikimedia.org";
QString Configuration::IRCNick = "huggle";
quint16 Configuration::IRCPort = 6667;
int Configuration::ProviderCache = 2000;
QString Configuration::HuggleVersion = HUGGLE_VERSION;
int Configuration::RingLogMaxSize = 2000;
QString Configuration::HomePath = QDir::currentPath();
QString Configuration::EditSuffixOfHuggle = "([[WP:HG]])";
QStringList Configuration::EditRegexOfTools;
QString Configuration::WikiDB = Configuration::GetConfigurationPath() + "wikidb.xml";
QString Configuration::DefaultRevertSummary = "Reverted edits by $1 identified as vandalism";
QStringList Configuration::WhiteList;

// Global
QString Configuration::GlobalConfigurationWikiAddress = "meta.wikimedia.org/w/";
bool Configuration::GlobalConfig_EnableAll = true;
QString Configuration::GlobalConfig_MinVersion = HUGGLE_VERSION;
QString Configuration::GlobalConfig_LocalConfigWikiPath = "Project:Huggle/Config";
QString Configuration::GlobalConfig_DocumentationPath = "https://www.mediawiki.org/wiki/Manual:Huggle";
QString Configuration::GlobalConfig_FeedbackPath = "http://en.wikipedia.org/wiki/Wikipedia:Huggle/Feedback";
QString Configuration::GlobalConfig_UserConf = "User:$1/huggle.css";

// Local

QString Configuration::LocalConfig_MinimalVersion = "3.0.0.0";
bool Configuration::LocalConfig_UseIrc = false;
bool Configuration::LocalConfig_RequireRollback = false;
bool Configuration::LocalConfig_RequireAdmin = false;
bool Configuration::LocalConfig_EnableAll = false;
int Configuration::LocalConfig_RequireEdits = 0;

// Reverting

QString Configuration::LocalConfig_ManualRevertSummary = "Reverted edits by [[Special:Contributions/$1|$1]] to last revision by $2";
QString Configuration::LocalConfig_MultipleRevertSummary = "Reverted,edit by,edits by,and,other users,to last revision by,to an older version by";
QStringList Configuration::LocalConfig_RevertSummaries;
QStringList Configuration::LocalConfig_TemplateSummary;
bool Configuration::LocalConfig_RollbackPossible = true;
QString Configuration::LocalConfig_RollbackSummary = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to last revision by $2";
QString Configuration::LocalConfig_DefaultSummary = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to last revision by $2";
QString Configuration::LocalConfig_SingleRevert = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])";
QString Configuration::LocalConfig_UndoSummary = "Undid edit by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])";

// Warnings

QString Configuration::LocalConfig_AgfRevert = "Reverted good faith edits";
QString Configuration::LocalConfig_WarnSummary = "Warning (level 1)";
QString Configuration::LocalConfig_WarnSummary2 = "Warning (level 2)";
QString Configuration::LocalConfig_WarnSummary3 = "Warning (level 3)";
QString Configuration::LocalConfig_WarnSummary4 = "Warning (level 4)";

QStringList Configuration::LocalConfig_Assisted;
QStringList Configuration::LocalConfig_Ignores;
QStringList Configuration::LocalConfig_RevertPatterns;
QStringList Configuration::LocalConfig_IgnorePatterns;
QStringList Configuration::LocalConfig_WarningTypes;
QStringList Configuration::Rights;

QStringList Configuration::LocalConfig_WarningDefs;

int Configuration::QueryListTimeLimit = 2;
int Configuration::HistorySize = 600;

QStringList Configuration::LocalConfig_WarningTemplates;

int Configuration::LocalConfig_IPScore = 800;
int Configuration::LocalConfig_BotScore = -200;
int Configuration::LocalConfig_WarningScore = 2000;
int Configuration::LocalConfig_TalkPageWarningScore = -800;
QList<ScoreWord> Configuration::LocalConfig_ScoreParts;
QList<ScoreWord> Configuration::LocalConfig_ScoreWords;
int Configuration::LocalConfig_ScoreFlag = -20000;
QStringList Configuration::LocalConfig_WelcomeTypes;
bool Configuration::WarnUserSpaceRoll = true;
bool Configuration::NextOnRv = true;
bool Configuration::LocalConfig_WelcomeGood = true;
QString Configuration::LocalConfig_ClearTalkPageTemp = "{{Huggle/Cleared}}";
QString Configuration::LocalConfig_WelcomeAnon = "{{subst:Welcome-anon}} ~~~~";
bool Configuration::GlobalConfigWasLoaded = false;
bool Configuration::LocalConfig_GlobalRequired = true;
bool Configuration::LocalConfig_AIV = false;
bool Configuration::LocalConfig_AIVExtend = true;
QString Configuration::LocalConfig_ReportPath = "";
int Configuration::LocalConfig_ReportSt = 0;
QString Configuration::LocalConfig_RUTemplateReport = "";
QString Configuration::LocalConfig_IPVTemplateReport = "";
QString Configuration::Language = "en";
int Configuration::LocalConfig_WhitelistScore = -800;
double Configuration::RvCounter = 0;
bool Configuration::TrimOldWarnings = true;
double Configuration::EditCounter = 0;
bool Configuration::AskUserBeforeReport = true;
bool Configuration::QueueNewEditsUp = false;
QString Configuration::LocalConfig_WelcomeSummary = "Welcoming user";
QString Configuration::LocalConfig_WelcomeTitle = "Welcome";
bool Configuration::WelcomeEmpty = true;
QString Configuration::LocalConfig_ReportSummary = "Reporting user";
bool Configuration::_SafeMode = false;

QString Configuration::LocalConfig_NSProject = MEDIAWIKI_DEFAULT_NS_PROJECT;
QString Configuration::LocalConfig_NSProjectTalk = MEDIAWIKI_DEFAULT_NS_PROJECTTALK;
QString Configuration::LocalConfig_NSTalk = MEDIAWIKI_DEFAULT_NS_TALK;
QString Configuration::LocalConfig_NSUser = MEDIAWIKI_DEFAULT_NS_USER;
QString Configuration::LocalConfig_NSUserTalk = MEDIAWIKI_DEFAULT_NS_USERTALK;

QStringList Configuration::LocalConfig_DeletionTemplates;
int Configuration::LocalConfig_TemplateAge = -30;
int Configuration::LocalConfig_ScoreChange = 100;

// Blocking users
QStringList Configuration::LocalConfig_BlockExpiryOptions;
QString Configuration::LocalConfig_BlockTime = "indefinite";
QString Configuration::LocalConfig_BlockTimeAnon = "31 hours";
QString Configuration::LocalConfig_BlockMessage;
QString Configuration::LocalConfig_BlockMessageIndef;
QString Configuration::LocalConfig_BlockReason = "[[WP:VAND|Vandalism]]";
QString Configuration::LocalConfig_BlockSummary = "Notification: Blocked";
bool Configuration::AutomaticallyResolveConflicts = false;
QString Configuration::VandalNw_Server = "hub.tm-irc.org";
bool Configuration::VandalNw_Login = true;

QString Configuration::LocalConfig_ProtectReason = "Persistent [[WP:VAND|vandalism]]";

bool Configuration::LocalConfig_ConfirmMultipleEdits = false;
bool Configuration::LocalConfig_ConfirmRange = false;
bool Configuration::LocalConfig_ConfirmPage = false;
bool Configuration::LocalConfig_ConfirmSame = false;
bool Configuration::LocalConfig_ConfirmWarned = false;
int Configuration::FontSize = 10;

QString Configuration::GetURLProtocolPrefix()
{
    if (!Configuration::UsingSSL)
    {
        return "http://";
    }
    return "https://";
}

QString Configuration::GetConfigurationPath()
{
    QDir conf(Configuration::HomePath + QDir::separator() + "Configuration");
    if (!conf.exists())
    {
        conf.mkdir(Configuration::HomePath + QDir::separator() + "Configuration");
    }
    return Configuration::HomePath + QDir::separator() + "Configuration" + QDir::separator();
}

QString Configuration::GetDefaultRevertSummary(QString source)
{
    QString summary = Configuration::DefaultRevertSummary;
    summary = summary.replace("$1", source) + " " + Configuration::EditSuffixOfHuggle;
    return summary;
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
    if (Configuration::LocalConfig_TemplateAge > -1)
    {
        Configuration::LocalConfig_TemplateAge = -30;
    }
    if (Configuration::Cache_InfoSize < 10)
    {
        Configuration::Cache_InfoSize = 10;
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
    QString conf = "<nowiki>\n";
    conf += "enable:true\n";
    conf += "version:" + Configuration::HuggleVersion + "\n\n";
    conf += "admin:true\n";
    conf += "patrol-speedy:true\n";
    conf += "speedy-message-title:Speedy deleted\n";
    conf += "report-summary:" + Configuration::LocalConfig_ReportSummary + "\n";
    conf += "prod-message-summary:Notification: Proposed deletion of [[$1]]\n";
    conf += "warn-summary-4:" + Configuration::LocalConfig_WarnSummary4 + "\n";
    conf += "warn-summary-3:" + Configuration::LocalConfig_WarnSummary3 + "\n";
    conf += "warn-summary-2:" + Configuration::LocalConfig_WarnSummary2 + "\n";
    conf += "warn-summary:" + Configuration::LocalConfig_WarnSummary + "\n";
    conf += "auto-advance:false\n";
    conf += "auto-whitelist:true\n";
    conf += "confirm-multiple:" + Configuration::Bool2String(Configuration::LocalConfig_ConfirmMultipleEdits) + "\n";
    conf += "confirm-range:" + Configuration::Bool2String(Configuration::LocalConfig_ConfirmRange) + "\n";
    conf += "automatically-resolve-conflicts:" + Configuration::Bool2String(Configuration::AutomaticallyResolveConflicts) + "\n";
    conf += "confirm-page:" + Configuration::Bool2String(Configuration::LocalConfig_ConfirmPage) + "\n";
    conf += "template-age:" + QString::number(Configuration::LocalConfig_TemplateAge) + "\n";
    conf += "confirm-same:" + Configuration::Bool2String(Configuration::LocalConfig_ConfirmSame) + "\n";
    conf += "default-summary:" + Configuration::DefaultRevertSummary + "\n";
    conf += "diff-font-size:" + QString::number(Configuration::FontSize) + "\n";
    conf += "</nowiki>";
    return conf;
}

void Configuration::LoadConfig()
{
    QFile file(Configuration::GetConfigurationPath() + "huggle3.xml");
    Core::Log("Home: " + Configuration::GetConfigurationPath());
    if (!QFile().exists(Configuration::GetConfigurationPath() + "huggle3.xml"))
    {
        Core::DebugLog("No config file at " + Configuration::GetConfigurationPath() + "huggle3.xml");
        return;
    }
    if(!file.open(QIODevice::ReadOnly))
    {
        Core::DebugLog("Unable to read config file");
        return;
    }
    QDomDocument conf;
    conf.setContent(file.readAll());
    QDomNodeList l = conf.elementsByTagName("local");
    int item = 0;
    while (item < l.count())
    {
        QDomElement option = l.at(item).toElement();
        QDomNamedNodeMap xx = option.attributes();
        if (!xx.contains("text") || !xx.contains("key"))
        {
            continue;
        }
        QString key = option.attribute("key");
        if (key == "DefaultRevertSummary")
        {
            Configuration::DefaultRevertSummary = option.attribute("text");
            item++;
            continue;
        }
        if (key == "Cache_InfoSize")
        {
            Configuration::Cache_InfoSize = option.attribute("text").toInt();
            item++;
            continue;
        }
        if (key == "GlobalConfigurationWikiAddress")
        {
            Configuration::GlobalConfigurationWikiAddress = option.attribute("text");
            item++;
            continue;
        }
        if (key == "IRCIdent")
        {
            Configuration::IRCIdent = option.attribute("text");
            item++;
            continue;
        }
        if (key == "IRCNick")
        {
            Configuration::IRCNick = option.attribute("text");
            item++;
            continue;
        }
        if (key == "IRCPort")
        {
            Configuration::IRCPort = option.attribute("text").toInt();
            item++;
            continue;
        }
        if (key == "IRCServer")
        {
            Configuration::IRCServer = option.attribute("text");
            item++;
            continue;
        }
        if (key == "Language")
        {
            Configuration::Language = option.attribute("text");
            item++;
            continue;
        }
        if (key == "ProviderCache")
        {
            Configuration::ProviderCache = option.attribute("text").toInt();
            item++;
            continue;
        }
        if (key == "AskUserBeforeReport")
        {
            Configuration::AskUserBeforeReport = Configuration::SafeBool(option.attribute("text"));
            item++;
            continue;
        }
        if (key == "HistorySize")
        {
            Configuration::HistorySize = option.attribute("text").toInt();
            item++;
            continue;
        }
        if (key == "VandalNw_Login")
        {
            Configuration::VandalNw_Login = Configuration::SafeBool(option.attribute("text"));
            item++;
            continue;
        }
        item++;
    }
    Core::DebugLog("Finished conf");
}

void Configuration::SaveConfig()
{
    QFile file(Configuration::GetConfigurationPath() + QDir::separator() + "huggle3.xml");
    if (!file.open(QIODevice::WriteOnly))
    {
        Core::Log("Unable to save configuration because the file can't be open");
        return;
    }
    QXmlStreamWriter *x = new QXmlStreamWriter();
    x->setDevice(&file);
    x->writeStartDocument();
    Configuration::InsertConfig("Cache_InfoSize", QString::number(Configuration::Cache_InfoSize), x);
    Configuration::InsertConfig("DefaultRevertSummary", Configuration::DefaultRevertSummary, x);
    Configuration::InsertConfig("GlobalConfigurationWikiAddress", Configuration::GlobalConfigurationWikiAddress, x);
    Configuration::InsertConfig("IRCIdent", Configuration::IRCIdent, x);
    Configuration::InsertConfig("IRCNick", Configuration::IRCNick, x);
    Configuration::InsertConfig("IRCPort", QString::number(Configuration::IRCPort), x);
    Configuration::InsertConfig("IRCServer", Configuration::IRCServer, x);
    Configuration::InsertConfig("Language", Configuration::Language, x);
    Configuration::InsertConfig("ProviderCache", QString::number(Configuration::ProviderCache), x);
    Configuration::InsertConfig("AskUserBeforeReport", Configuration::Bool2String(Configuration::AskUserBeforeReport), x);
    Configuration::InsertConfig("HistorySize", QString::number(Configuration::HistorySize), x);
    Configuration::InsertConfig("NextOnRv", Configuration::Bool2String(Configuration::NextOnRv), x);
    Configuration::InsertConfig("QueueNewEditsUp", Configuration::Bool2String(Configuration::QueueNewEditsUp), x);
    Configuration::InsertConfig("RingLogMaxSize", QString::number(Configuration::RingLogMaxSize), x);
    Configuration::InsertConfig("TrimOldWarnings", Configuration::Bool2String(Configuration::TrimOldWarnings), x);
    Configuration::InsertConfig("WarnUserSpaceRoll", Configuration::Bool2String(Configuration::WarnUserSpaceRoll), x);
    Configuration::InsertConfig("UserName", Configuration::UserName, x);
    /////////////////////////////
    // Vandal network
    /////////////////////////////
    Configuration::InsertConfig("VandalNw_Login", Configuration::Bool2String(Configuration::VandalNw_Login), x);
    x->writeEndDocument();
    delete x;
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
