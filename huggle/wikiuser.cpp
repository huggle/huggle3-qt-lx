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
QRegExp WikiUser::IPv6Regex("(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]"\
                            "{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|("\
                            "[0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-"\
                            "fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,"\
                            "4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0"\
                            ",4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0"\
                            "-9]){0,1}[0-9]).){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}"\
                            ":){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]).){3,3}(25[0-5]|(2[0-4]|1{0,1}["\
                            "0-9]){0,1}[0-9]))");
QList<WikiUser*> WikiUser::ProblematicUsers;
QMutex WikiUser::ProblematicUserListLock(QMutex::Recursive);
QDateTime WikiUser::InvalidTime = QDateTime::fromMSecsSinceEpoch(2);

WikiUser *WikiUser::RetrieveUser(WikiUser *user)
{
    return WikiUser::RetrieveUser(user->Username);
}

WikiUser *WikiUser::RetrieveUser(QString user)
{
    WikiUser::ProblematicUserListLock.lock();
    int User = 0;
    while (User < WikiUser::ProblematicUsers.count())
    {
        if (user == WikiUser::ProblematicUsers.at(User)->Username)
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
        if (user->GetBadnessScore() == 0 && user->WarningLevel == 0)
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
    WikiUser::UpdateWl(us, us->GetBadnessScore());
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
            ProblematicUsers.at(c)->_talkPageWasRetrieved = us->_talkPageWasRetrieved;
            ProblematicUsers.at(c)->DateOfTalkPage = us->DateOfTalkPage;
            ProblematicUsers.at(c)->ContentsOfTalkPage = us->ContentsOfTalkPage;
            if (!us->IsIP() && ProblematicUsers.at(c)->EditCount < 0)
            {
                ProblematicUsers.at(c)->EditCount = us->EditCount;
            }
            WikiUser::ProblematicUserListLock.unlock();
            return;
        }
        c++;
    }
    ProblematicUsers.append(new WikiUser(us));
    WikiUser::ProblematicUserListLock.unlock();
}

bool WikiUser::IsIPv4(QString user)
{
    return WikiUser::IPv4Regex.exactMatch(user);
}

bool WikiUser::IsIPv6(QString user)
{
    return WikiUser::IPv6Regex.exactMatch(user);
}

void WikiUser::UpdateWl(WikiUser *us, long score)
{
    if (!us->IsIP() &&
        score <= Configuration::HuggleConfiguration->ProjectConfig_WhitelistScore &&
        !us->IsWhitelisted())
    {
        Configuration::HuggleConfiguration->WhiteList.append(us->Username);
    }
}

WikiUser::WikiUser()
{
    this->UserLock = new QMutex(QMutex::Recursive);
    this->Username = "";
    this->IP = true;
    this->BadnessScore = 0;
    this->WarningLevel = 0;
    this->IsBanned = false;
    this->ContentsOfTalkPage = "";
    this->DateOfTalkPage = InvalidTime;
    this->IsReported = false;
    this->_talkPageWasRetrieved = false;
    this->WhitelistInfo = 0;
    this->EditCount = -1;
    this->RegistrationDate = "";
    this->Bot = false;
}

WikiUser::WikiUser(WikiUser *u)
{
    this->UserLock = new QMutex(QMutex::Recursive);
    this->IP = u->IP;
    this->Username = u->Username;
    this->WarningLevel = u->WarningLevel;
    this->BadnessScore = u->BadnessScore;
    this->DateOfTalkPage = u->DateOfTalkPage;
    this->IsBanned = u->IsBanned;
    this->ContentsOfTalkPage = u->ContentsOfTalkPage;
    this->IsReported = u->IsReported;
    this->_talkPageWasRetrieved = u->_talkPageWasRetrieved;
    this->WhitelistInfo = u->WhitelistInfo;
    this->Bot = u->Bot;
    this->EditCount = u->EditCount;
    this->RegistrationDate = u->RegistrationDate;
}

WikiUser::WikiUser(const WikiUser &u)
{
    this->UserLock = new QMutex(QMutex::Recursive);
    this->WarningLevel = u.WarningLevel;
    this->IsReported = u.IsReported;
    this->IP = u.IP;
    this->Username = u.Username;
    this->BadnessScore = u.BadnessScore;
    this->IsBanned = u.IsBanned;
    this->DateOfTalkPage = u.DateOfTalkPage;
    this->ContentsOfTalkPage = u.ContentsOfTalkPage;
    this->_talkPageWasRetrieved = u._talkPageWasRetrieved;
    this->WhitelistInfo = u.WhitelistInfo;
    this->Bot = u.Bot;
    this->EditCount = u.EditCount;
    this->RegistrationDate = u.RegistrationDate;
}

WikiUser::WikiUser(QString user)
{
    this->UserLock = new QMutex(QMutex::Recursive);
    this->IP = false;
    if (user.length() != 0)
    {
        if (WikiUser::IPv6Regex.exactMatch(user))
        {
            this->IP = true;
        } else if (WikiUser::IPv4Regex.exactMatch(user))
        {
            this->IP = true;
        }
    }
    this->Username = user;
    this->Sanitize();
    this->IsBanned = false;
    this->_talkPageWasRetrieved = false;
    this->DateOfTalkPage = InvalidTime;
    int c=0;
    this->ContentsOfTalkPage = "";
    WikiUser::ProblematicUserListLock.lock();
    while (c<ProblematicUsers.count())
    {
        if (ProblematicUsers.at(c)->Username == this->Username)
        {
            this->BadnessScore = ProblematicUsers.at(c)->BadnessScore;
            this->WarningLevel = ProblematicUsers.at(c)->WarningLevel;
            this->IsReported = ProblematicUsers.at(c)->IsReported;
            WikiUser::ProblematicUserListLock.unlock();
            return;
        }
        c++;
    }
    WikiUser::ProblematicUserListLock.unlock();
    this->BadnessScore = 0;
    this->WarningLevel = 0;
    this->Bot = false;
    this->IsReported = false;
    this->EditCount = -1;
    this->RegistrationDate = "";
}

WikiUser::~WikiUser()
{
    delete this->UserLock;
}

void WikiUser::Resync()
{
    WikiUser::ProblematicUserListLock.lock();
    WikiUser *user = WikiUser::RetrieveUser(this);
    if (user != NULL)
    {
        this->BadnessScore = user->BadnessScore;
        this->ContentsOfTalkPage = user->TalkPage_GetContents();
        this->_talkPageWasRetrieved = user->_talkPageWasRetrieved;
        this->DateOfTalkPage = user->DateOfTalkPage;
        if (user->WarningLevel > this->WarningLevel)
            this->WarningLevel = user->WarningLevel;
        if (this->EditCount < 0)
            this->EditCount = user->EditCount;
    }
    // we can finally unlock it
    WikiUser::ProblematicUserListLock.unlock();
}

QString WikiUser::TalkPage_GetContents()
{
    // first we need to lock this object because it might be accessed from another thread in same moment
    this->UserLock->lock();
    // check if there isn't some global talk page
    WikiUser *user = WikiUser::RetrieveUser(this);
    // we need to copy the value to local variable so that if someone change it from different
    // thread we are still working with same data
    QString contents = "";
    if (user != NULL && user->TalkPage_WasRetrieved())
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

void WikiUser::TalkPage_SetContents(QString text)
{
    this->UserLock->lock();
    this->_talkPageWasRetrieved = true;
    this->ContentsOfTalkPage = text;
    this->DateOfTalkPage = QDateTime::currentDateTime();
    this->Update();
    this->UserLock->unlock();
}

void WikiUser::Update(bool MatchingOnly)
{
    WikiUser::ProblematicUserListLock.lock();
    if (MatchingOnly)
    {
        // here we want to update the user only if it already is in database so we
        // need to check if it is there and if yes, we continue
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

void WikiUser::ParseTP(QDate bt)
{
    QString tp = this->TalkPage_GetContents();
    if (tp.length() > 0)
    {
        this->WarningLevel = HuggleParser::GetLevel(tp, bt);
    }
}

QString WikiUser::GetTalk()
{
    return Configuration::HuggleConfiguration->ProjectConfig_NSUserTalk + this->Username;
}

bool WikiUser::TalkPage_WasRetrieved()
{
    return this->_talkPageWasRetrieved;
}

bool WikiUser::TalkPage_ContainsSharedIPTemplate()
{
    if (Configuration::HuggleConfiguration->ProjectConfig_SharedIPTemplateTags.length() < 1)
    {
        return false;
    }
    if (this->TalkPage_WasRetrieved())
    {
        return this->TalkPage_GetContents().contains(Configuration::HuggleConfiguration->ProjectConfig_SharedIPTemplateTags);
    }
    return false;
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
    QString spaced = this->Username;
    spaced.replace("_", " ");
    if (Configuration::HuggleConfiguration->WhiteList.contains(this->Username) ||
            Configuration::HuggleConfiguration->WhiteList.contains(spaced))
    {
        this->WhitelistInfo = 1;
        return true;
    } else
    {
        this->WhitelistInfo = 2;
        return false;
    }
}

QDateTime WikiUser::TalkPage_RetrievalTime()
{
    return this->DateOfTalkPage;
}

long WikiUser::GetBadnessScore(bool _resync)
{
    if (_resync)
    {
        this->Resync();
    }
    return BadnessScore;
}

void WikiUser::SetBadnessScore(long value)
{
    this->Resync();
    BadnessScore = value;
    this->Update(true);
}

QString WikiUser::Flags()
{
    QString pflags = "";
    QString nflags = "";
    if (this->TalkPage_GetContents().length() == 0 && this->TalkPage_WasRetrieved())
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
    if (nflags.length() >= 1)
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
