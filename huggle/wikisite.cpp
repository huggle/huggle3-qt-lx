//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "wikisite.hpp"
using namespace Huggle;

WikiSite::WikiSite(const WikiSite &w)
{
    this->LongPath = w.LongPath;
    this->IRCChannel = w.IRCChannel;
    this->Name = w.Name;
    this->OAuthURL = w.OAuthURL;
    this->ScriptPath = w.ScriptPath;
    this->SupportHttps = w.SupportHttps;
    this->SupportOAuth = w.SupportOAuth;
    this->URL = w.URL;
    this->WhiteList = w.WhiteList;
}

WikiSite::WikiSite(WikiSite *w)
{
    this->LongPath = w->LongPath;
    this->IRCChannel = w->IRCChannel;
    this->Name = w->Name;
    this->OAuthURL = w->OAuthURL;
    this->WhiteList = w->WhiteList;
    this->URL = w->URL;
    this->SupportOAuth = w->SupportOAuth;
    this->SupportHttps = w->SupportHttps;
    this->ScriptPath = w->ScriptPath;
}

WikiSite::WikiSite(QString name, QString url)
{
    this->LongPath = "wiki/";
    this->Name = name;
    this->URL = url;
    this->ScriptPath = "w/";
    this->OAuthURL = url + "w/index.php?title=Special:MWOAuth";
    this->SupportHttps = true;
    this->SupportOAuth = true;
    this->IRCChannel = "#test.wikipedia";
    this->WhiteList = "test.wikipedia";
}

WikiSite::WikiSite(QString name, QString url, QString path, QString script, bool https, bool oauth, QString channel, QString wl)
{
    this->IRCChannel = channel;
    this->LongPath = path;
    this->Name = name;
    this->SupportHttps = https;
    this->OAuthURL = url + "w/index.php?title=Special:MWOAuth";
    this->ScriptPath = script;
    this->URL = url;
    this->SupportOAuth = oauth;
    this->WhiteList = wl;
}
