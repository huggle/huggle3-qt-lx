//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef WIKISITE_H
#define WIKISITE_H

#include <QString>

class WikiSite
{
public:
    //! Name of wiki, used by huggle only
    QString Name;
    //! URL of wiki, no http prefix must be present
    QString URL;
    //! long article path (wiki/ for example on english wp)
    QString LongPath;
    QString ScriptPath;
    QString OAuthURL;
    QString IRCChannel;
    WikiSite(WikiSite *w);
    WikiSite(const WikiSite &w);
    QString WhiteList;
    bool SupportHttps;
    bool SupportOAuth;
    WikiSite(QString name, QString url);
    //! This will create a new instance of wikisite with most of configuration
    //! param name is a name of wiki for internal purposes
    //! param url is relative url to wiki (no http prefix) which must be terminated with slash
    //! param path is long path for articles, like wiki/
    //! param script is short path w/
    //! param https set this true if your wiki support https
    //! param oauth set this true if your wiki support oauth
    //! param channel irc
    //! param wl whitelist
    WikiSite(QString name, QString url, QString path, QString script, bool https, bool oauth, QString channel, QString wl);
};

#endif // WIKISITE_H
