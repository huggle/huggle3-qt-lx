//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "wikiuser.hpp"
#include <QMutex>
#include "configuration.hpp"
#include "exception.hpp"
#include "huggleparser.hpp"
#include "localization.hpp"
#include "mainwindow.hpp"
#include "hugglequeue.hpp"
#include "huggleprofiler.hpp"
#include "syslog.hpp"
#include "wikisite.hpp"
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
    return WikiUser::RetrieveUser(user->Username, user->GetSite());
}

WikiUser *WikiUser::RetrieveUser(QString user, WikiSite *site)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    WikiUser::ProblematicUserListLock.lock();
    int User = 0;
    while (User < WikiUser::ProblematicUsers.count())
    {
        if (site == WikiUser::ProblematicUsers.at(User)->Site && user == WikiUser::ProblematicUsers.at(User)->Username)
        {
            WikiUser *u = WikiUser::ProblematicUsers.at(User);
            WikiUser::ProblematicUserListLock.unlock();
            return u;
        }
        ++User;
    }
    WikiUser::ProblematicUserListLock.unlock();
    return nullptr;
}

void WikiUser::TrimProblematicUsersList()
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    WikiUser::ProblematicUserListLock.lock();
    int i = 0;
    while (i < WikiUser::ProblematicUsers.count())
    {
        WikiUser *user = WikiUser::ProblematicUsers.at(i);
        if (!user)
            throw new Huggle::NullPointerException("WikiUser user", BOOST_CURRENT_FUNCTION);
        if (user->GetBadnessScore(false) == 0 && user->WarningLevel == 0)
        {
            // there is no point to hold information for them
            WikiUser::ProblematicUsers.removeAt(i);
            delete user;
            continue;
        }
        ++i;
    }
    WikiUser::ProblematicUserListLock.unlock();
}

void WikiUser::UpdateUser(WikiUser *us)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    WikiUser::ProblematicUserListLock.lock();
    WikiUser::UpdateWl(us, us->GetBadnessScore(false));
    int c=0;
    while (c<ProblematicUsers.count())
    {
        if (ProblematicUsers.at(c)->Username == us->Username)
        {
            ProblematicUsers.at(c)->BadnessScore = us->BadnessScore;
            if (ProblematicUsers.at(c)->WarningLevel != us->WarningLevel)
            {
                // houston, we have this user with a different warning level, what should we do now?? pls tell us
                // You need to update the interface of huggle so that it display all latest information about it
                ProblematicUsers.at(c)->WarningLevel = us->WarningLevel;
                // let's update the queue first
                MainWindow::HuggleMain->Queue1->UpdateUser(us);
            }
            ProblematicUsers.at(c)->WhitelistInfo = us->WhitelistInfo;
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
        ++c;
    }
    ProblematicUsers.append(new WikiUser(us));
    WikiUser::ProblematicUserListLock.unlock();
    if (us->GetWarningLevel() > 0)
    {
        // this user has higher warning level than 0 so we need to update interface in case it was already somewhere
        MainWindow::HuggleMain->Queue1->UpdateUser(us);
    }
}

bool WikiUser::CompareUsernames(QString a, QString b)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    a = a.replace("_", " ").toLower();
    b = b.replace("_", " ").toLower();
    return (a == b);
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
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    if (!us->IsIP() && score <= us->GetSite()->GetProjectConfig()->WhitelistScore && !us->IsWhitelisted())
    {
        if (us->GetSite()->GetProjectConfig()->WhiteList.contains(us->Username))
        {
            us->WhitelistInfo = HUGGLE_WL_TRUE;
            us->Update();
            return;
        }
        QStringList pm = QStringList() << us->Username << QString::number(score) << us->GetSite()->Name;
        Syslog::HuggleLogs->Log(_l("whitelisted", pm));
        us->GetSite()->GetProjectConfig()->NewWhitelist.append(us->Username);
        us->GetSite()->GetProjectConfig()->WhiteList.append(us->Username);
        us->WhitelistInfo = HUGGLE_WL_TRUE;
        us->Update();
    }
}

WikiUser::WikiUser()
{
    this->UserLock = new QMutex(QMutex::Recursive);
    this->Username = "";
    this->IP = true;
    this->BadnessScore = 0;
    this->WarningLevel = 0;
    this->IsBlocked = false;
    this->ContentsOfTalkPage = "";
    this->DateOfTalkPage = InvalidTime;
    this->IsReported = false;
    this->_talkPageWasRetrieved = false;
    this->WhitelistInfo = HUGGLE_WL_UNKNOWN;
    this->EditCount = -1;
    this->Site = Configuration::HuggleConfiguration->Project;
    this->RegistrationDate = "";
    this->Bot = false;
}

WikiUser::WikiUser(WikiUser *u) : MediaWikiObject(u)
{
    this->UserLock = new QMutex(QMutex::Recursive);
    this->IP = u->IP;
    this->Username = u->Username;
    this->WarningLevel = u->WarningLevel;
    this->BadnessScore = u->BadnessScore;
    this->DateOfTalkPage = u->DateOfTalkPage;
    this->IsBlocked = u->IsBlocked;
    this->ContentsOfTalkPage = u->ContentsOfTalkPage;
    this->IsReported = u->IsReported;
    this->_talkPageWasRetrieved = u->_talkPageWasRetrieved;
    this->WhitelistInfo = HUGGLE_WL_UNKNOWN;
    this->Bot = u->Bot;
    this->EditCount = u->EditCount;
    this->RegistrationDate = u->RegistrationDate;
}

WikiUser::WikiUser(const WikiUser &u) : MediaWikiObject(u)
{
    this->UserLock = new QMutex(QMutex::Recursive);
    this->WarningLevel = u.WarningLevel;
    this->IsReported = u.IsReported;
    this->IP = u.IP;
    this->Username = u.Username;
    this->BadnessScore = u.BadnessScore;
    this->IsBlocked = u.IsBlocked;
    this->DateOfTalkPage = u.DateOfTalkPage;
    this->ContentsOfTalkPage = u.ContentsOfTalkPage;
    this->_talkPageWasRetrieved = u._talkPageWasRetrieved;
    this->WhitelistInfo = HUGGLE_WL_UNKNOWN;
    this->Bot = u.Bot;
    this->EditCount = u.EditCount;
    this->RegistrationDate = u.RegistrationDate;
}

WikiUser::WikiUser(QString user)
{
    this->UserLock = new QMutex(QMutex::Recursive);
    this->IP = false;
    if (!user.isEmpty())
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
    this->IsBlocked = false;
    this->_talkPageWasRetrieved = false;
    this->DateOfTalkPage = InvalidTime;
    int c=0;
    this->ContentsOfTalkPage = "";
    this->Site = Configuration::HuggleConfiguration->Project;
    this->EditCount = -1;
    this->Bot = false;
    this->RegistrationDate = "";
    this->WhitelistInfo = HUGGLE_WL_UNKNOWN;
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
        ++c;
    }
    WikiUser::ProblematicUserListLock.unlock();
    this->BadnessScore = 0;
    this->WarningLevel = 0;
    this->IsReported = false;
}

WikiUser::~WikiUser()
{
    delete this->UserLock;
}

void WikiUser::Resync()
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    WikiUser::ProblematicUserListLock.lock();
    WikiUser *user = WikiUser::RetrieveUser(this);
    if (user && user != this)
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
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    // first we need to lock this object because it might be accessed from another thread in same moment
    this->UserLock->lock();
    // check if there isn't some global talk page
    WikiUser *user = WikiUser::RetrieveUser(this);
    // we need to copy the value to local variable so that if someone change it from different
    // thread we are still working with same data
    QString contents = "";
    if (user != nullptr && user->TalkPage_WasRetrieved())
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
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    this->UserLock->lock();
    this->_talkPageWasRetrieved = true;
    this->ContentsOfTalkPage = text;
    this->DateOfTalkPage = QDateTime::currentDateTime();
    this->Update();
    this->UserLock->unlock();
}

void WikiUser::Update(bool MatchingOnly)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    WikiUser::ProblematicUserListLock.lock();
    if (MatchingOnly)
    {
        // here we want to update the user only if it already is in database so we
        // need to check if it is there and if yes, we continue
        if (WikiUser::RetrieveUser(this) == nullptr)
        {
            WikiUser::ProblematicUserListLock.unlock();
            return;
        }
    }
    WikiUser::UpdateUser(this);
    WikiUser::ProblematicUserListLock.unlock();
}

void WikiUser::ParseTP(QDate bt)
{
    QString tp = this->TalkPage_GetContents();
    if (tp.length() > 0)
        this->WarningLevel = HuggleParser::GetLevel(tp, bt, this->GetSite());
}

QString WikiUser::UnderscorelessUsername()
{
    QString name = this->Username;
    return name.replace("_", " ");
}

QString WikiUser::GetTalk()
{
    // get a usertalk prefix for this site
    WikiPageNS *ns = this->GetSite()->RetrieveNSByCanonicalName("User talk");
    QString prefix = ns->GetName();
    if (!prefix.size())
        prefix = "User talk";
    prefix += ":";
    return prefix + this->Username;
}

bool WikiUser::TalkPage_ContainsSharedIPTemplate()
{
    if (Configuration::HuggleConfiguration->ProjectConfig->SharedIPTemplateTags.length() < 1)
        return false;
    if (this->TalkPage_WasRetrieved())
    {
        return this->TalkPage_GetContents().contains(Configuration::HuggleConfiguration->ProjectConfig->SharedIPTemplateTags);
    }
    return false;
}

bool WikiUser::IsWhitelisted()
{
    if (this->WhitelistInfo == HUGGLE_WL_TRUE)
        return true;
    if (this->WhitelistInfo == HUGGLE_WL_FALSE)
        return false;
    QString spaced = this->Username;
    spaced.replace("_", " ");
    if (this->GetSite()->GetProjectConfig()->NewWhitelist.contains(this->Username) ||
            this->GetSite()->GetProjectConfig()->NewWhitelist.contains(spaced) ||
            this->GetSite()->GetProjectConfig()->WhiteList.contains(this->Username) ||
            this->GetSite()->GetProjectConfig()->WhiteList.contains(spaced))
    {
        this->WhitelistInfo = HUGGLE_WL_TRUE;
        return true;
    } else
    {
        this->WhitelistInfo = HUGGLE_WL_FALSE;
        return false;
    }
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
    return this->Bot;
}

void WikiUser::SetBot(bool value)
{
    this->Bot = value;
}

void WikiUser::DecrementWarningLevel()
{
    this->WarningLevel--;
    if (this->WarningLevel < 0)
        this->WarningLevel = 0;
}

void WikiUser::IncrementWarningLevel()
{
    this->WarningLevel++;
    if (this->WarningLevel > this->GetSite()->GetProjectConfig()->WarningLevel)
        this->WarningLevel = this->GetSite()->GetProjectConfig()->WarningLevel;
}

void WikiUser::SetWarningLevel(byte_ht level)
{
    this->WarningLevel = level;
}

byte_ht WikiUser::GetWarningLevel() const
{
    return this->WarningLevel;
}
