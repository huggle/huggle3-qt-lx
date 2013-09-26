//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "configuration.h"

unsigned int Configuration::Verbosity = 1;
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
QString Configuration::HuggleVersion = "3.0.0.0";
int Configuration::RingLogMaxSize = 2000;
QString Configuration::HomePath = QDir::currentPath();
QString Configuration::EditSuffixOfHuggle = " ([[WP:HG]]) ";
QStringList Configuration::EditRegexOfTools;
QString Configuration::WikiDB = Configuration::GetConfigurationPath() + "wikidb.xml";
QString Configuration::DefaultRevertSummary = "Reverted edits by $1 identified as vandalism";
QStringList Configuration::WhiteList;

// Global
QString Configuration::GlobalConfigurationWikiAddress = "meta.wikimedia.org/w/";
bool Configuration::GlobalConfig_EnableAll = true;
QString Configuration::GlobalConfig_MinVersion = "3.0.0.0";
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


ScoreWord::ScoreWord(QString Word, int Score)
{
    this->score = Score;
    this->word = Word;
}
