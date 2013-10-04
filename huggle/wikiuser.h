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

namespace Huggle
{
    //! User
    class WikiUser
    {
    public:
        static QList<WikiUser*> ProblematicUsers;
        QString Username;
        long BadnessScore;
        int WarningLevel;
        QString ContentsOfTalkPage;
        bool IsReported;
        bool IP;
        static void UpdateUser(WikiUser *us);
        WikiUser();
        WikiUser(WikiUser *u);
        WikiUser(const WikiUser& u);
        WikiUser(QString user);
        QString GetTalk();
    private:
        //! Matches only IPv4
        static QRegExp IPv4Regex;
        //! Matches all IP
        static QRegExp IPv6Regex;
    };
}

#endif // WIKIUSER_H
