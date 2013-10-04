//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "wikiuser.h"
using namespace Huggle;

QRegExp WikiUser::IPv4Regex("^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}$");
QRegExp WikiUser::IPv6Regex("^(?>(?>([a-f0-9]{1,4})(?>:(?1)){7}|(?!(?:.*[a-f0-9](?>:|$)){8,})((?1)(?>:(?1)){0,6})?:"\
                            ":(?2)?)|(?>(?>(?1)(?>:(?1)){5}:|(?!(?:.*[a-f0-9]:){6,})(?3)?::(?>((?1)(?>:(?1)){0,4}):)?)"\
                            "?(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])(?>\\.(?4)){3}))$");
QList<WikiUser*> WikiUser::ProblematicUsers;

void WikiUser::UpdateUser(WikiUser *us)
{
    if (!us->IP && us->BadnessScore <= Configuration::LocalConfig_WhitelistScore)
    {
        if (!Configuration::WhiteList.contains(us->Username))
        {
            Configuration::WhiteList.append(us->Username);
        }
    }
    int c=0;
    while (c<ProblematicUsers.count())
    {
        if (ProblematicUsers.at(c)->Username == us->Username)
        {
            if (us->BadnessScore > ProblematicUsers.at(c)->BadnessScore)
            {
                ProblematicUsers.at(c)->BadnessScore = us->BadnessScore;
            }
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
    this->ContentsOfTalkPage = "";
    this->IsReported = false;
}

WikiUser::WikiUser(WikiUser *u)
{
    this->IP = u->IP;
    this->Username = u->Username;
    this->WarningLevel = u->WarningLevel;
    this->BadnessScore = u->BadnessScore;
    this->ContentsOfTalkPage = u->ContentsOfTalkPage;
    this->IsReported = u->IsReported;
}

WikiUser::WikiUser(const WikiUser &u)
{
    this->WarningLevel = u.WarningLevel;
    this->IsReported = u.IsReported;
    this->IP = u.IP;
    this->Username = u.Username;
    this->BadnessScore = u.BadnessScore;
    this->ContentsOfTalkPage = u.ContentsOfTalkPage;
}

WikiUser::WikiUser(QString user)
{
    this->IP = false;
    if (user != "")
    {
        this->IP = WikiUser::IPv6Regex.exactMatch(user);
    }
    this->Username = user;
    int c=0;
    this->ContentsOfTalkPage = "";
    while (c<ProblematicUsers.count())
    {
        if (ProblematicUsers.at(c)->Username == this->Username)
        {
            this->BadnessScore = ProblematicUsers.at(c)->BadnessScore;
            this->WarningLevel = ProblematicUsers.at(c)->WarningLevel;
            this->IsReported = ProblematicUsers.at(c)->IsReported;
            return;
        }
        c++;
    }
    this->BadnessScore = 0;
    this->WarningLevel = 0;
    this->IsReported = false;
}

QString WikiUser::GetTalk()
{
    return "User_talk:" + this->Username;
}
