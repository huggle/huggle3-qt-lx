//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "wikisite.h"

WikiSite::WikiSite(QString name, QString url)
{
    this->LongPath = "wiki/";
    this->Name = name;
    this->URL = url;
    this->ScriptPath = "w/";
    this->OAuthURL = url + "w/index.php?title=Special:MWOAuth";
    this->SupportHttps = true;
    this->SupportOAuth = true;
    this->IRCChannel = "#en.wikipedia";
    this->WhiteList = "en.wikipedia";
}

WikiSite::WikiSite(QString name, QString url, QString path, QString script, bool https, bool oauth, QString channel, QString wl)
{
    this->IRCChannel = ic;
    this->LongPath = path;
    this->Name = name;
    this->SupportHttps = https;
    this->OAuthURL = url + "w/index.php?title=Special:MWOAuth";
    this->ScriptPath = script;
    this->URL = url;
    this->SupportOAuth = oauth;
    this->WhiteList = wl;
}
