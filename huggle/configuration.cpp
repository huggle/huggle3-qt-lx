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
QList<WikiSite> Configuration::ProjectList;
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
unsigned int Configuration::ProviderCache = 2000;
QString Configuration::HuggleVersion = "3.0.0.0";
unsigned int Configuration::RingLogMaxSize = 2000;
QString Configuration::HomePath = QDir::currentPath();
QString Configuration::EditSuffixOfHuggle = "([[WP:HG]])";
QStringList Configuration::EditRegexOfTools;
QString Configuration::WikiDB = Configuration::GetConfigurationPath() + "wikidb.xml";
QString Configuration::DefaultRevertSummary = "Reverted edits by $1 identified as vandalism";
QStringList Configuration::WhiteList;


QString Configuration::GlobalConfigurationWikiAddress = "meta.wikimedia.org/w/";

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
    return Configuration::HomePath + QDir::separator() + "Configuration" + QDir::separator();
}

Configuration::Configuration()
{
}

QString Configuration::GetDefaultRevertSummary(QString source)
{
    return Configuration::DefaultRevertSummary.replace("$1", source) + " " + Configuration::EditSuffixOfHuggle;
}
