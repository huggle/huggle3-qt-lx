//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "wikiuser.hpp"
using namespace Huggle;

//QRegExp WikiUser::IPv4Regex("^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}$");
QRegExp WikiUser::IPv4Regex("\\b((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\\.|$)){4}\\b");
QRegExp WikiUser::IPv6Regex("^(?>(?>([a-f0-9]{1,4})(?>:(?1)){7}|(?!(?:.*[a-f0-9](?>:|$)){8,})((?1)(?>:(?1)){0,6})?:"\
                            ":(?2)?)|(?>(?>(?1)(?>:(?1)){5}:|(?!(?:.*[a-f0-9]:){6,})(?3)?::(?>((?1)(?>:(?1)){0,4}):)?)"\
                            "?(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])(?>\\.(?4)){3}))$");
QList<WikiUser*> WikiUser::ProblematicUsers;
QMutex WikiUser::ProblematicUserListLock(QMutex::Recursive);

WikiUser *WikiUser::RetrieveUser(WikiUser *user)
{
    WikiUser::ProblematicUserListLock.lock();
    int User = 0;
    while (User < WikiUser::ProblematicUsers.count())
    {
        if (user->Username == WikiUser::ProblematicUsers.at(User)->Username)
        {
            WikiUser *u = WikiUser::ProblematicUsers.at(User);
            WikiUser::ProblematicUserListLock.unlock();
            return u;
        }
        User++;
    }
    WikiUser::ProblematicUserListLock.unlock();
    return NULL;
}

void WikiUser::TrimProblematicUsersList()
{
    WikiUser::ProblematicUserListLock.lock();
    int i = 0;
    while (i < WikiUser::ProblematicUsers.count())
    {
        WikiUser *user = WikiUser::ProblematicUsers.at(i);
        if (user->getBadnessScore() == 0 && user->WarningLevel == 0)
        {
            // there is no point to hold information for them
            WikiUser::ProblematicUsers.removeAt(i);
            delete user;
            continue;
        }
        i++;
    }
    WikiUser::ProblematicUserListLock.unlock();
}

void WikiUser::UpdateUser(WikiUser *us)
{
    WikiUser::ProblematicUserListLock.lock();
    if (!us->IP && us->getBadnessScore() <= Configuration::HuggleConfiguration->LocalConfig_WhitelistScore)
    {
        if (!Configuration::HuggleConfiguration->WhiteList.contains(us->Username))
        {
            Configuration::HuggleConfiguration->WhiteList.append(us->Username);
        }
    }
    int c=0;
    while (c<ProblematicUsers.count())
    {
        if (ProblematicUsers.at(c)->Username == us->Username)
        {
            ProblematicUsers.at(c)->BadnessScore = us->BadnessScore;
            ProblematicUsers.at(c)->WarningLevel = us->WarningLevel;
            if (us->IsReported)
            {
                ProblematicUsers.at(c)->IsReported = true;
            }
            ProblematicUsers.at(c)->ContentsOfTalkPage = us->ContentsOfTalkPage;
            WikiUser::ProblematicUserListLock.unlock();
            return;
        }
        c++;
    }
    ProblematicUsers.append(new WikiUser(us));
    WikiUser::ProblematicUserListLock.unlock();
}

WikiUser::WikiUser()
{
    this->UserLock = new QMutex(QMutex::Recursive);
    this->Username = "";
    this->IP = true;
    this->BadnessScore = 0;
    this->WarningLevel = 0;
    this->ContentsOfTalkPage = "";
    this->IsReported = false;
    this->WhitelistInfo = 0;
    this->Bot = false;
}

WikiUser::WikiUser(WikiUser *u)
{
    this->UserLock = new QMutex(QMutex::Recursive);
    this->IP = u->IP;
    this->Username = u->Username;
    this->WarningLevel = u->WarningLevel;
    this->BadnessScore = u->BadnessScore;
    this->ContentsOfTalkPage = u->ContentsOfTalkPage;
    this->IsReported = u->IsReported;
    this->WhitelistInfo = u->WhitelistInfo;
    this->Bot = u->Bot;
}

WikiUser::WikiUser(const WikiUser &u)
{
    this->UserLock = new QMutex(QMutex::Recursive);
    this->WarningLevel = u.WarningLevel;
    this->IsReported = u.IsReported;
    this->IP = u.IP;
    this->Username = u.Username;
    this->BadnessScore = u.BadnessScore;
    this->ContentsOfTalkPage = u.ContentsOfTalkPage;
    this->WhitelistInfo = u.WhitelistInfo;
    this->Bot = u.Bot;
}

WikiUser::WikiUser(QString user)
{
    this->UserLock = new QMutex(QMutex::Recursive);
    this->IP = false;
    if (user != "")
    {
        this->IP = WikiUser::IPv6Regex.exactMatch(user);
        if (!this->IP)
        {
            if (WikiUser::IPv4Regex.exactMatch(user))
            {
                this->IP = true;
            }
        }
    }
    this->Username = user;
    this->Sanitize();
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
    this->Bot = false;
    this->IsReported = false;
}

WikiUser::~WikiUser()
{
    delete UserLock;
    while (this->Contributions.count() > 0)
    {
        delete this->Contributions.at(0);
        this->Contributions.removeAt(0);
    }
}

void WikiUser::Resync()
{
    WikiUser::ProblematicUserListLock.lock();
    WikiUser *user = WikiUser::RetrieveUser(this);
    if (user != NULL)
    {
        this->BadnessScore = user->BadnessScore;
        this->ContentsOfTalkPage = user->GetContentsOfTalkPage();
        if (user->WarningLevel > this->WarningLevel)
        {
            this->WarningLevel = user->WarningLevel;
        }
    }
    // we can finally unlock it
    WikiUser::ProblematicUserListLock.unlock();
}

QString WikiUser::GetContentsOfTalkPage()
{
    // first we need to lock this object because it might be accessed from another thread in same moment
    this->UserLock->lock();
    // check if there isn't some global talk page
    WikiUser *user = WikiUser::RetrieveUser(this);
    // we need to copy the value to local variable so that if someone change it from different
    // thread we are still working with same data
    QString contents = "";
    if (user != NULL)
    {
        // we return a value of user from global db instead of local
        contents = user->ContentsOfTalkPage;
        this->UserLock->unlock();
        return contents;
    }
    contents = this->ContentsOfTalkPage;
    this->UserLock->unlock();
    return contents;
}

void WikiUser::SetContentsOfTalkPage(QString text)
{
    this->UserLock->lock();
    this->ContentsOfTalkPage = text;
    this->Update();
    this->UserLock->unlock();
}

void WikiUser::Update(bool MatchingOnly)
{
    WikiUser::ProblematicUserListLock.lock();
    if (MatchingOnly)
    {
        if (WikiUser::RetrieveUser(this) == NULL)
        {
            WikiUser::ProblematicUserListLock.unlock();
            return;
        }
    }
    WikiUser::UpdateUser(this);
    WikiUser::ProblematicUserListLock.unlock();
}

void WikiUser::Sanitize()
{
    this->Username = this->Username.replace(" ", "_");
}

void WikiUser::ForceIP()
{
    this->IP = true;
}

bool WikiUser::IsIP() const
{
    return IP;
}

QString WikiUser::GetTalk()
{
    return Configuration::HuggleConfiguration->LocalConfig_NSUserTalk + this->Username;
}

bool WikiUser::IsWhitelisted()
{
    if (this->WhitelistInfo == 1)
    {
        return true;
    }
    if (this->WhitelistInfo == 2)
    {
        return false;
    }
    if (Configuration::HuggleConfiguration->WhiteList.contains(this->Username))
    {
        this->WhitelistInfo = 1;
        return true;
    } else
    {
        this->WhitelistInfo = 2;
        return false;
    }
}

long WikiUser::getBadnessScore(bool _resync)
{
    if (_resync)
    {
        this->Resync();
    }
    return BadnessScore;
}

void WikiUser::setBadnessScore(long value)
{
    BadnessScore = value;
    this->Update(true);
}

QString WikiUser::Flags()
{
    QString pflags = "";
    QString nflags = "";
    if (this->GetContentsOfTalkPage() == "")
    {
        nflags += "T";
    } else
    {
        pflags += "T";
    }
    if (this->WarningLevel > 0)
    {
        pflags += "w";
    }
    if (this->IsWhitelisted())
    {
        pflags += "E";
    }
    if (this->IsIP())
    {
        nflags += "R";
    }
    if (this->IsReported)
    {
        pflags += "r";
    }
    if (this->Bot)
    {
        pflags += "b";
    }
    QString flags = "";
    if (nflags != "")
    {
        flags += "-" + nflags;
    }
    if (pflags != "")
    {
        flags += "+" + pflags;
    }
    return flags;
}
bool WikiUser::GetBot() const
{
    return Bot;
}

void WikiUser::SetBot(bool value)
{
    Bot = value;
}


