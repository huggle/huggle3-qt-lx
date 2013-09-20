//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "wikiuser.h"

QRegExp WikiUser::IPv4Regex("^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}$");
QList<WikiUser*> WikiUser::ProblematicUsers;

void WikiUser::UpdateUser(WikiUser *us)
{
    int c=0;
    while (c<ProblematicUsers.count())
    {
        if (ProblematicUsers.at(c)->Username == us->Username)
        {
            ProblematicUsers.at(c)->BadnessScore = us->BadnessScore;
            ProblematicUsers.at(c)->WarningLevel = us->WarningLevel;
            return;
        }
        c++;
    }
    ProblematicUsers.append(new WikiUser(us));
}

WikiUser::WikiUser()
{
    this->Username = "";
    this->IP = true;
    this->BadnessScore = 0;
    this->WarningLevel = 0;
}

WikiUser::WikiUser(WikiUser *u)
{
    this->IP = u->IP;
    this->Username = u->Username;
    this->WarningLevel = u->WarningLevel;
    this->BadnessScore = u->BadnessScore;
}

WikiUser::WikiUser(const WikiUser &u)
{
    this->WarningLevel = u.WarningLevel;
    this->IP = u.IP;
    this->Username = u.Username;
    this->BadnessScore = u.BadnessScore;
}

WikiUser::WikiUser(QString user)
{
    this->IP = WikiUser::IPv4Regex.exactMatch(user);
    this->Username = user;
    int c=0;
    while (c<ProblematicUsers.count())
    {
        if (ProblematicUsers.at(c)->Username == this->Username)
        {
            this->BadnessScore = ProblematicUsers.at(c)->BadnessScore;
            this->WarningLevel = ProblematicUsers.at(c)->WarningLevel;
            return;
        }
        c++;
    }
    this->BadnessScore = 0;
    this->WarningLevel = 0;
}
