//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef WIKIUSER_H
#define WIKIUSER_H

#include <QList>
#include <QString>
#include <QRegExp>

#include "configuration.h"

class WikiUser
{
public:
    static QList<WikiUser*> ProblematicUsers;
    static void UpdateUser(WikiUser *us);
    WikiUser();
    WikiUser(WikiUser *u);
    WikiUser(const WikiUser& u);
    WikiUser(QString user);
    QString GetTalk();
    QString Username;
    long BadnessScore;
    int WarningLevel;
    QString ContentsOfTalkPage;
    bool IsReported;
    bool IP;
private:
    static QRegExp IPv4Regex;
};

#endif // WIKIUSER_H
