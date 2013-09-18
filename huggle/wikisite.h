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
    QString Name;
    QString URL;
    QString LongPath;
    QString ScriptPath;
    QString OAuthURL;
    QString IRCChannel;
    QString WhiteList;
    bool SupportHttps;
    bool SupportOAuth;
    WikiSite(QString name, QString url);
    WikiSite(QString name, QString url, QString path, QString script, bool https, bool oauth, QString ic, QString wl);
};

#endif // WIKISITE_H
