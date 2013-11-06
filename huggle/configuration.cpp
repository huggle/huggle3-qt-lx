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
int Configuration::ProviderCache = 200;
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
QString Configuration::LocalConfig_MultipleRevertSummary = "Reverted,edit by,edits by,and,other users,to last revision by,to an older version by";
QStringList Configuration::LocalConfig_RevertSummaries;
QString Configuration::LocalConfig_RollbackSummary = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to last revision by $2";
QString Configuration::LocalConfig_DefaultSummary = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to last revision by $2";
QString Configuration::LocalConfig_SingleRevert = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])";
QString Configuration::LocalConfig_UndoSummary = "Undid edit by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])";
QString Configuration::LocalConfig_SoftwareRevertDefaultSummary = "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to last revision by $2 using huggle software rollback (reverted by $3 revisions to revision $4)";

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
int Configuration::HistorySize = 20;

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
QString Configuration::LocalConfig_NSFile = MEDIAWIKI_DEFAULT_NS_FILE;
QString Configuration::LocalConfig_NSFileTalk = MEDIAWIKI_DEFAULT_NS_FILETALK;
QString Configuration::LocalConfig_NSCategory = MEDIAWIKI_DEFAULT_NS_CATEGORY;
QString Configuration::LocalConfig_NSCategoryTalk = MEDIAWIKI_DEFAULT_NS_CATEGORYTALK;
QString Configuration::LocalConfig_NSMediaWiki = MEDIAWIKI_DEFAULT_NS_MEDIAWIKI;
QString Configuration::LocalConfig_NSMediaWikiTalk = MEDIAWIKI_DEFAULT_NS_MEDIAWIKITALK;
QString Configuration::LocalConfig_NSHelp = MEDIAWIKI_DEFAULT_NS_HELP;
QString Configuration::LocalConfig_NSHelpTalk = MEDIAWIKI_DEFAULT_NS_HELPTALK;

QStringList Configuration::LocalConfig_DeletionTemplates;
int Configuration::LocalConfig_TemplateAge = -30;
int Configuration::LocalConfig_ScoreChange = 100;

// Blocking users
QStringList Configuration::LocalConfig_BlockExpiryOptions;
QString Configuration::LocalConfig_BlockTime = "indefinite";
QString Configuration::LocalConfig_BlockTimeAnon = "31 hours";
QString Configuration::LocalConfig_BlockMessage = "{{subst:huggle/block|1=$1|2=$2}}";
QString Configuration::LocalConfig_BlockMessageIndef = "{{subst:huggle/block-indef|1=$1}}";
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
int Configuration::WriteTimeout = 200;
int Configuration::ReadTimeout = 60;
bool Configuration::EnforceManualSoftwareRollback = false;


bool Configuration::LocalConfig_UAAavailable = false;
QString Configuration::LocalConfig_UAAPath = "Project:Usernames for administrator attention";
QString Configuration::LocalConfig_UAATemplate = "{{user-uaa|1=$1}}";

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
    if (Configuration::HistorySize < 2)
    {
        Configuration::HistorySize = 2;
    }
    Configuration::EditCounter = 0;
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
    conf += "software-rollback:" + Configuration::Bool2String(Configuration::EnforceManualSoftwareRollback) + "\n";
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

QStringList Configuration::ConfigurationParse_QL(QString key, QString content, bool CS)
{
    QStringList list;
    if (content.startsWith(key + ":"))
    {
        QString value = content.mid(key.length() + 1);
        QStringList lines = value.split("\n");
        int curr = 1;
        while (curr < lines.count())
        {
            QString _line = Core::Trim(lines.at(curr));
            if (_line.endsWith(","))
            {
                list.append(_line);
            } else
            {
                if (_line != "")
                {
                    list.append(_line);
                    break;
                }
            }
            curr++;
        }
        if (CS)
        {
            // now we need to split values by comma as well
            QStringList f;
            int c = 0;
            while (c<list.count())
            {
                QStringList xx = list.at(c).split(",");
                int i2 = 0;
                while (i2<xx.count())
                {
                    if (Core::Trim(xx.at(i2)) != "")
                    {
                        f.append(Core::Trim(xx.at(i2)));
                    }
                    i2++;
                }
                c++;
            }
            list = f;
        }
        return list;
    } else if (content.contains("\n" + key + ":"))
    {
        QString value = content.mid(content.indexOf("\n" + key + ":") + key.length() + 2);
        QStringList lines = value.split("\n");
        int curr = 1;
        while (curr < lines.count())
        {
            QString _line = Core::Trim(lines.at(curr));
            if (_line.endsWith(","))
            {
                list.append(_line);
            } else
            {
                if (_line != "")
                {
                    list.append(_line);
                    break;
                }
            }
            curr++;
        }
        if (CS)
        {
            // now we need to split values by comma as well
            QStringList f;
            int c = 0;
            while (c<list.count())
            {
                QStringList xx = list.at(c).split(",");
                int i2 = 0;
                while (i2<xx.count())
                {
                    if (Core::Trim(xx.at(i2)) != "")
                    {
                        f.append(Core::Trim(xx.at(i2)));
                    }
                    i2++;
                }
                c++;
            }
            list = f;
        }
        return list;
    }
    return list;
}

QStringList Configuration::ConfigurationParse_QL(QString key, QString content, QStringList list, bool CS)
{
    QStringList result = Configuration::ConfigurationParse_QL(key, content, CS);
    if (result.count() == 0)
    {
        return list;
    }
    return result;
}

bool Configuration::ParseGlobalConfig(QString config)
{
    Configuration::GlobalConfig_EnableAll = Configuration::SafeBool(Configuration::ConfigurationParse("enable-all", config));
    QString temp = Configuration::ConfigurationParse("documentation", config);
    if (temp != "")
    {
        Configuration::GlobalConfig_DocumentationPath = temp;
    }
    temp = Configuration::ConfigurationParse("feedback", config);
    if (temp != "")
    {
        Configuration::GlobalConfig_FeedbackPath = temp;
    }
    Configuration::GlobalConfigWasLoaded = true;
    return true;
}

bool Configuration::ParseLocalConfig(QString config)
{
    //AIV
    Configuration::LocalConfig_AIV = Configuration::SafeBool(Configuration::ConfigurationParse("aiv-reports", config));
    Configuration::LocalConfig_AIVExtend = Configuration::SafeBool(Configuration::ConfigurationParse("aiv-extend", config));
    // Restrictions
    Configuration::LocalConfig_EnableAll = Configuration::SafeBool(Configuration::ConfigurationParse("enable-all", config));
    Configuration::LocalConfig_RequireAdmin = Configuration::SafeBool(Configuration::ConfigurationParse("require-admin", config));
    Configuration::LocalConfig_RequireRollback = Configuration::SafeBool(Configuration::ConfigurationParse("require-rollback", config));
    Configuration::LocalConfig_RequireEdits = Configuration::ConfigurationParse("require-edits", config, "0").toInt();
    Configuration::LocalConfig_UseIrc = Configuration::SafeBool(Configuration::ConfigurationParse("irc", config));
    Configuration::LocalConfig_Ignores = Configuration::ConfigurationParse_QL("ignore", config, true);
    Configuration::LocalConfig_IPScore = Configuration::ConfigurationParse("score-ip", config, "800").toInt();
    Configuration::LocalConfig_ScoreFlag = Configuration::ConfigurationParse("score-flag", config).toInt();
    Configuration::LocalConfig_WarnSummary = Configuration::ConfigurationParse("warn-summary", config);
    Configuration::LocalConfig_WarnSummary2 = Configuration::ConfigurationParse("warn-summary-2", config);
    Configuration::LocalConfig_WarnSummary3 = Configuration::ConfigurationParse("warn-summary-3", config);
    Configuration::LocalConfig_WarnSummary4 = Configuration::ConfigurationParse("warn-summary-4", config);
    Configuration::LocalConfig_RevertSummaries = Configuration::ConfigurationParse_QL("template-summ", config);
    Configuration::LocalConfig_WarningTypes = Configuration::ConfigurationParse_QL("warning-types", config);
    Configuration::LocalConfig_WarningDefs = Configuration::ConfigurationParse_QL("warning-template-tags", config);
    Configuration::LocalConfig_BotScore = Configuration::ConfigurationParse("score-bot", config, "-200000").toInt();
    Configuration::LocalConfig_ReportPath = Configuration::ConfigurationParse("aiv", config);
    Configuration::LocalConfig_ReportSt = Configuration::ConfigurationParse("aiv-section", config).toInt();
    Configuration::LocalConfig_IPVTemplateReport = Configuration::ConfigurationParse("aiv-ip", config);
    Configuration::LocalConfig_RUTemplateReport = Configuration::ConfigurationParse("aiv-user", config);
    Configuration::AutomaticallyResolveConflicts = Configuration::SafeBool(Configuration::ConfigurationParse("automatically-resolve-conflicts", config), false);
    Configuration::LocalConfig_WelcomeTypes = Configuration::ConfigurationParse_QL("welcome-messages", config);
    Configuration::LocalConfig_ReportSummary = Configuration::ConfigurationParse("report-summary", config);
    Configuration::LocalConfig_DeletionTemplates = Configuration::ConfigurationParse_QL("speedy-options", config);
    Configuration::LocalConfig_TemplateAge = Configuration::ConfigurationParse("template-age", config, QString::number(Configuration::LocalConfig_TemplateAge)).toInt();
    Configuration::LocalConfig_WelcomeGood = Configuration::SafeBool(Configuration::ConfigurationParse("welcome-on-good-edit", config, "true"));
    Configuration::LocalConfig_UAAPath = Configuration::ConfigurationParse("uaa", config);
    Configuration::LocalConfig_ConfirmMultipleEdits = Configuration::SafeBool(Configuration::ConfigurationParse("confirm-multiple", config));
    Configuration::LocalConfig_ConfirmRange = Configuration::SafeBool(Configuration::ConfigurationParse("confirm-range", config));
    Configuration::LocalConfig_ConfirmSame = Configuration::SafeBool(Configuration::ConfigurationParse("confirm-same", config));
    Configuration::LocalConfig_ConfirmWarned = Configuration::SafeBool(Configuration::ConfigurationParse("confirm-warned", config));
    Configuration::LocalConfig_DefaultSummary = Configuration::ConfigurationParse("default-summary", config,
              "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to last revision by $2");
    Configuration::LocalConfig_AgfRevert = Configuration::ConfigurationParse("agf", config,
              "Reverted good faith edits by [[Special:Contributions/$2|$2]] [[User talk:$2|talk]]");
    Configuration::LocalConfig_BlockMessage = Configuration::ConfigurationParse("block-message", config);
    Configuration::LocalConfig_BlockReason = Configuration::ConfigurationParse("block-reason", config);
    Configuration::LocalConfig_BlockSummary = Configuration::ConfigurationParse("block-summary", config, "Notification: Blocked");
    Configuration::LocalConfig_BlockTime = Configuration::ConfigurationParse("blocktime", config, "indef");
    Configuration::LocalConfig_ClearTalkPageTemp = Configuration::ConfigurationParse("template-clear-talk-page", config, "{{Huggle/Cleared}}");
    Configuration::LocalConfig_IgnorePatterns = Configuration::ConfigurationParse_QL("ignore-patterns", config);
    Configuration::LocalConfig_SoftwareRevertDefaultSummary = Configuration::ConfigurationParse("manual-revert-summary", config,
              "Reverted edits by [[Special:Contributions/$1|$1]] to last revision by $2");
    Configuration::LocalConfig_MultipleRevertSummary = Configuration::ConfigurationParse("multiple-revert-summary-parts", config,
              "Reverted,edit by,edits by,and,other users,to last revision by,to an older version by");
    Configuration::LocalConfig_ProtectReason = Configuration::ConfigurationParse("protection-reason", config, "Excessive [[Wikipedia:Vandalism|vandalism]]");
    Configuration::LocalConfig_RevertPatterns = Configuration::ConfigurationParse_QL("revert-patterns", config);
    Configuration::LocalConfig_RollbackSummary = Configuration::ConfigurationParse("rollback-summary", config,
              "Reverted edits by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]]) to last revision by $2");
    Configuration::LocalConfig_SingleRevert = Configuration::ConfigurationParse("single-revert-summary", config,
              "Undid edit by [[Special:Contributions/$1|$1]] ([[User talk:$1|talk]])");
    Configuration::LocalConfig_UndoSummary = Configuration::ConfigurationParse("undo-summary", config);

    if (Core::AIVP != NULL)
    {
        delete Core::AIVP;
    }

    Core::AIVP = new WikiPage(Configuration::LocalConfig_ReportPath);
    Core::ParsePats(config);
    Core::ParseWords(config);
    QStringList namespaces = Configuration::ConfigurationParse_QL("namespace-names", config, true);
    int NS=0;
    while (namespaces.count() > NS)
    {
        QString line = namespaces.at(NS);
        NS++;

        if (!line.contains(",") || !line.contains(";"))
        {
            continue;
        }

        int ns = line.mid(0, line.indexOf(";")).toInt();
        QString name = line.mid(line.indexOf(";"));

        if (!name.contains(","))
        {
            continue;
        }

        name = name.mid(0, name.indexOf(","));
        QString talk = line.mid(line.indexOf(",") + 1);

        if (talk.endsWith(","))
        {
            talk = talk.mid(0, talk.length() - 1);
        }

        if (talk.contains(";"))
        {
            talk = talk.mid(talk.indexOf(";") + 1);
        }

        switch (ns)
        {
        /// \todo Some NS are missing here
        case MEDIAWIKI_NSID_MAIN:
            Configuration::LocalConfig_NSTalk = talk;
            break;
        case MEDIAWIKI_NSID_CATEGORY:
            Configuration::LocalConfig_NSCategory = name;
            Configuration::LocalConfig_NSCategoryTalk = talk;
            break;
        case MEDIAWIKI_NSID_FILE:
            Configuration::LocalConfig_NSFile = name;
            Configuration::LocalConfig_NSFileTalk = talk;
            break;
        case MEDIAWIKI_NSID_HELP:
            Configuration::LocalConfig_NSHelp = name;
            Configuration::LocalConfig_NSHelpTalk = talk;
            break;
        case MEDIAWIKI_NSID_MEDIAWIKI:
            Configuration::LocalConfig_NSMediaWiki = name;
            Configuration::LocalConfig_NSMediaWikiTalk = talk;
            break;
        }
    }

    Core::UAAP = new WikiPage(Configuration::LocalConfig_UAAPath);

    // templates
    int CurrentTemplate=0;
    while (CurrentTemplate<Configuration::LocalConfig_WarningTypes.count())
    {
        QString type = Core::GetKeyFromValue(Configuration::LocalConfig_WarningTypes.at(CurrentTemplate));
        int CurrentWarning = 1;
        while (CurrentWarning <= 4)
        {
            QString xx = Configuration::ConfigurationParse(type + QString::number(CurrentWarning), config);
            if (xx != "")
            {
                Configuration::LocalConfig_WarningTemplates.append(type + QString::number(CurrentWarning) + ";" + xx);
            }
            CurrentWarning++;
        }
        CurrentTemplate++;
    }
    // sanitize
    if (Configuration::LocalConfig_ReportPath == "")
    {
        Configuration::LocalConfig_AIV = false;
    }
    // Do the same for UAA as well
    if (Configuration::LocalConfig_UAAPath == "")
    {
        Configuration::LocalConfig_UAAavailable = false;
    }
    return true;
}

bool Configuration::ParseUserConfig(QString config)
{
    Configuration::LocalConfig_EnableAll = Configuration::SafeBool(Configuration::ConfigurationParse("enable", config));
    Configuration::LocalConfig_Ignores = Configuration::ConfigurationParse_QL("ignore", config, Configuration::LocalConfig_Ignores);
    Configuration::LocalConfig_IPScore = Configuration::ConfigurationParse("score-ip", config, QString::number(Configuration::LocalConfig_IPScore)).toInt();
    Configuration::LocalConfig_ScoreFlag = Configuration::ConfigurationParse("score-flag", config, QString::number(Configuration::LocalConfig_ScoreFlag)).toInt();
    Configuration::LocalConfig_WarnSummary = Configuration::ConfigurationParse("warn-summary", config, Configuration::LocalConfig_WarnSummary);
    Configuration::EnforceManualSoftwareRollback = Configuration::SafeBool(Configuration::ConfigurationParse("software-rollback", config));
    Configuration::LocalConfig_WarnSummary2 = Configuration::ConfigurationParse("warn-summary-2", config, Configuration::LocalConfig_WarnSummary2);
    Configuration::LocalConfig_WarnSummary3 = Configuration::ConfigurationParse("warn-summary-3", config, Configuration::LocalConfig_WarnSummary3);
    Configuration::LocalConfig_WarnSummary4 = Configuration::ConfigurationParse("warn-summary-4", config, Configuration::LocalConfig_WarnSummary4);
    Configuration::AutomaticallyResolveConflicts = Configuration::SafeBool(Configuration::ConfigurationParse("automatically-resolve-conflicts", config), false);
    Configuration::LocalConfig_TemplateAge = Configuration::ConfigurationParse("template-age", config, QString::number(Configuration::LocalConfig_TemplateAge)).toInt();
    Configuration::LocalConfig_RevertSummaries = Configuration::ConfigurationParse_QL("template-summ", config, Configuration::LocalConfig_RevertSummaries);
    Configuration::LocalConfig_WarningTypes = Configuration::ConfigurationParse_QL("warning-types", config, Configuration::LocalConfig_WarningTypes);
    Configuration::LocalConfig_WarningDefs = Configuration::ConfigurationParse_QL("warning-template-tags", config, Configuration::LocalConfig_WarningDefs);
    Configuration::LocalConfig_BotScore = Configuration::ConfigurationParse("score-bot", config, QString(Configuration::LocalConfig_BotScore)).toInt();
    Configuration::NormalizeConf();
    /// \todo Lot of configuration options are missing
    return true;
}

QString Configuration::ConfigurationParse(QString key, QString content, QString missing)
{
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
