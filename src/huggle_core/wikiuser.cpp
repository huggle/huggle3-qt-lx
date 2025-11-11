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
#include "projectconfiguration.hpp"
#include "exception.hpp"
#include "huggleparser.hpp"
#include "localization.hpp"
#include "hooks.hpp"
#include "huggleprofiler.hpp"
#include "syslog.hpp"
#include "wikipage.hpp"
#include "wikisite.hpp"
using namespace Huggle;

//QRegExp WikiUser::IPv4Regex("^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}$");
HREGEX_TYPE WikiUser::IPv4Regex(R"(\b((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\.|$)){4}\b)");
HREGEX_TYPE WikiUser::IPv6Regex("(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]"\
                            "{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|("\
                            "[0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-"\
                            "fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,"\
                            "4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0"\
                            ",4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0"\
                            "-9]){0,1}[0-9]).){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}"\
                            ":){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]).){3,3}(25[0-5]|(2[0-4]|1{0,1}["\
                            "0-9]){0,1}[0-9]))");
HREGEX_TYPE WikiUser::TempAccountRegex(R"(^~\d{4}-\d{1,5}-\d{1,5}$)");
QList<WikiUser*> WikiUser::ProblematicUsers;

#ifdef QT6_BUILD
HMUTEX_TYPE WikiUser::ProblematicUserListLock;
#else
HMUTEX_TYPE WikiUser::ProblematicUserListLock(QMutex::Recursive);
#endif
QDateTime WikiUser::InvalidTime = QDateTime::fromMSecsSinceEpoch(2);

WikiUser *WikiUser::RetrieveUser(WikiUser *user)
{
    return WikiUser::RetrieveUser(user->Username, user->GetSite());
}

WikiUser *WikiUser::RetrieveUser(const QString &user, WikiSite *site)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    WikiUser::ProblematicUserListLock.lock();
    foreach (WikiUser *user_, WikiUser::ProblematicUsers)
    {
        if (site == user_->Site && user == user_->Username)
        {
            WikiUser::ProblematicUserListLock.unlock();
            return user_;
        }
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
        if (user->GetBadnessScore(false) == 0 && user->warningLevel == 0)
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
    foreach (WikiUser *user, WikiUser::ProblematicUsers)
    {
        if (user->Username == us->Username)
        {
            user->BadnessScore = us->BadnessScore;
            if (user->warningLevel != us->warningLevel)
            {
                user->warningLevel = us->warningLevel;
                Hooks::WikiUser_Updated(us);
            }
            user->whitelistInfo = us->whitelistInfo;
            if (us->IsReported)
            {
                user->IsReported = true;
            }
            user->talkPageWasRetrieved = us->talkPageWasRetrieved;
            user->dateOfTalkPage = us->dateOfTalkPage;
            user->contentsOfTalkPage = us->contentsOfTalkPage;
            user->LastMessageTime = us->LastMessageTime;
            user->LastMessageTimeKnown = us->LastMessageTimeKnown;
            if (!us->IsAnon() && user->EditCount < 0)
            {
                user->EditCount = us->EditCount;
            }
            WikiUser::ProblematicUserListLock.unlock();
            return;
        }
    }
    ProblematicUsers.append(new WikiUser(us));
    WikiUser::ProblematicUserListLock.unlock();

    if (us->GetWarningLevel() > 0)
    {
        // this user has higher warning level than 0 so we need to update interface in case it was already somewhere
        Hooks::WikiUser_Updated(us);
    }
}

bool WikiUser::CompareUsernames(QString a, QString b)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    a = a.replace("_", " ").toLower();
    b = b.replace("_", " ").toLower();
    return (a == b);
}

bool WikiUser::IsIPv4(const QString &user)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
#ifdef QT6_BUILD
    QRegularExpressionMatch match = WikiUser::IPv4Regex.match(user);
    return match.hasMatch() && (match.captured(0) == user);
#else
    return WikiUser::IPv4Regex.exactMatch(user);
#endif
}

bool WikiUser::IsIPv6(const QString &user)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
#ifdef QT6_BUILD
    QRegularExpressionMatch match = WikiUser::IPv6Regex.match(user);
    return match.hasMatch() && (match.captured(0) == user);
#else
    return WikiUser::IPv6Regex.exactMatch(user);
#endif
}

bool WikiUser::IsTemporary(const QString &user)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
#ifdef QT6_BUILD
    QRegularExpressionMatch match = WikiUser::TempAccountRegex.match(user);
    if (match.hasMatch())
        return true;
#else
    if (WikiUser::TempAccountRegex.exactMatch(user))
        return true;
#endif

    return false;
}

void WikiUser::UpdateWl(WikiUser *us, long score)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    if (!us->IsAnon() && score <= us->GetSite()->GetProjectConfig()->WhitelistScore && !us->IsWhitelisted())
    {
        if (us->GetSite()->GetProjectConfig()->WhiteList.contains(us->Username))
        {
            us->whitelistInfo = HUGGLE_WL_TRUE;
            us->Update();
            return;
        }
        QStringList pm = QStringList() << us->Username << QString::number(score) << us->GetSite()->Name;
        Syslog::HuggleLogs->Log(_l("whitelisted", pm));
        us->GetSite()->GetProjectConfig()->NewWhitelist.append(us->Username);
        us->GetSite()->GetProjectConfig()->WhiteList.append(us->Username);
        us->whitelistInfo = HUGGLE_WL_TRUE;
        us->Update();
    }
}

WikiUser::WikiUser(WikiSite *site) : MediaWikiObject(site)
{
#ifdef QT6_BUILD
    this->userMutex = new QRecursiveMutex();
#else
    this->userMutex = new QMutex(QMutex::Recursive);
#endif
    this->Username = "";
    this->isAnon = true;
    this->BadnessScore = 0;
    this->warningLevel = 0;
    this->IsBlocked = false;
    this->contentsOfTalkPage = "";
    this->dateOfTalkPage = InvalidTime;
    this->IsReported = false;
    this->talkPageWasRetrieved = false;
    this->whitelistInfo = HUGGLE_WL_UNKNOWN;
    this->EditCount = -1;
    this->RegistrationDate = "";
    this->isBot = false;
}

WikiUser::WikiUser(WikiUser *u) : MediaWikiObject(u)
{
#ifdef QT6_BUILD
    this->userMutex = new QRecursiveMutex();
#else
    this->userMutex = new QMutex(QMutex::Recursive);
#endif
    this->isAnon = u->isAnon;
    this->Username = u->Username;
    this->warningLevel = u->warningLevel;
    this->BadnessScore = u->BadnessScore;
    this->dateOfTalkPage = u->dateOfTalkPage;
    this->IsBlocked = u->IsBlocked;
    this->contentsOfTalkPage = u->contentsOfTalkPage;
    this->IsReported = u->IsReported;
    this->talkPageWasRetrieved = u->talkPageWasRetrieved;
    this->whitelistInfo = HUGGLE_WL_UNKNOWN;
    this->isBot = u->isBot;
    this->EditCount = u->EditCount;
    this->RegistrationDate = u->RegistrationDate;
    this->LastMessageTimeKnown = u->LastMessageTimeKnown;
    this->LastMessageTime = u->LastMessageTime;
}

WikiUser::WikiUser(const WikiUser &u) : MediaWikiObject(u)
{
#ifdef QT6_BUILD
    this->userMutex = new QRecursiveMutex();
#else
    this->userMutex = new QMutex(QMutex::Recursive);
#endif
    this->warningLevel = u.warningLevel;
    this->IsReported = u.IsReported;
    this->isAnon = u.isAnon;
    this->Username = u.Username;
    this->BadnessScore = u.BadnessScore;
    this->IsBlocked = u.IsBlocked;
    this->dateOfTalkPage = u.dateOfTalkPage;
    this->contentsOfTalkPage = u.contentsOfTalkPage;
    this->talkPageWasRetrieved = u.talkPageWasRetrieved;
    this->whitelistInfo = HUGGLE_WL_UNKNOWN;
    this->isBot = u.isBot;
    this->EditCount = u.EditCount;
    this->RegistrationDate = u.RegistrationDate;
    this->LastMessageTime = u.LastMessageTime;
    this->LastMessageTimeKnown = u.LastMessageTimeKnown;
}

WikiUser::WikiUser(const QString &user, WikiSite *site) : MediaWikiObject(site)
{
#ifdef QT6_BUILD
    this->userMutex = new QRecursiveMutex();
#else
    this->userMutex = new QMutex(QMutex::Recursive);
#endif
    this->isAnon = false;

    if (!user.isEmpty() && (WikiUser::IsIPv4(user) || WikiUser::IsIPv6(user) || WikiUser::IsTemporary(user)))
        this->isAnon = true;

    this->Username = user;
    this->Sanitize();
    this->IsBlocked = false;
    this->talkPageWasRetrieved = false;
    this->dateOfTalkPage = InvalidTime;
    this->contentsOfTalkPage = "";
    this->EditCount = -1;
    this->isBot = false;
    this->RegistrationDate = "";
    this->whitelistInfo = HUGGLE_WL_UNKNOWN;
    this->BadnessScore = 0;
    this->warningLevel = 0;
    this->IsReported = false;
    this->Resync();
}

WikiUser::~WikiUser()
{
    delete this->wpTalkPage;
    delete this->userMutex;
}

bool WikiUser::Resync()
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    WikiUser *user = WikiUser::RetrieveUser(this);
    if (user && user != this)
    {
        this->BadnessScore = user->BadnessScore;
        this->contentsOfTalkPage = user->TalkPage_GetContents();
        this->talkPageWasRetrieved = user->talkPageWasRetrieved;
        this->dateOfTalkPage = user->dateOfTalkPage;
        if (user->warningLevel > this->warningLevel)
            this->warningLevel = user->warningLevel;
        if (this->EditCount < 0)
            this->EditCount = user->EditCount;
        this->IsReported = user->IsReported;
        this->IsBlocked = user->IsBlocked;
        this->LastMessageTime = user->LastMessageTime;
        this->LastMessageTimeKnown = user->LastMessageTimeKnown;
        return true;
    }
    return false;
}

QString WikiUser::TalkPage_GetContents()
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    // first we need to lock this object because it might be accessed from another thread in same moment
    this->userMutex->lock();
    // check if there isn't some global talk page
    WikiUser *user = WikiUser::RetrieveUser(this);
    // we need to copy the value to local variable so that if someone change it from different
    // thread we are still working with same data
    QString contents = "";
    if (user != nullptr && user->TalkPage_WasRetrieved())
    {
        // we return a value of user from global db instead of local
        contents = user->contentsOfTalkPage;
        this->userMutex->unlock();
        return contents;
    }
    contents = this->contentsOfTalkPage;
    this->userMutex->unlock();
    return contents;
}

void WikiUser::TalkPage_SetContents(const QString &text)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    this->userMutex->lock();
    this->talkPageWasRetrieved = true;
    this->contentsOfTalkPage = text;
    this->dateOfTalkPage = QDateTime::currentDateTime();
    this->Update();
    this->userMutex->unlock();
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
    {
        this->warningLevel = HuggleParser::GetLevel(tp, bt, this->GetSite());
    }
}

QString WikiUser::UnderscorelessUsername()
{
    QString name = this->Username;
    return name.replace("_", " ");
}

bool WikiUser::EqualTo(WikiUser *user)
{
    return this->Site == user->Site && this->Username == user->Username;
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

WikiPage *WikiUser::GetTalkPage()
{
    if (this->wpTalkPage)
    {
        // In case that a pointer to wiki site was changed in past, ensure it's consistent in every case
        if (this->Site != this->wpTalkPage->Site)
            this->wpTalkPage->Site = this->Site;
        return this->wpTalkPage;
    }

    WikiPage *page = new WikiPage(this->GetTalk(), this->Site);
    this->wpTalkPage = page;
    return page;
}

bool WikiUser::TalkPage_ContainsSharedIPTemplate()
{
    if (this->GetSite()->GetProjectConfig()->SharedIPTemplateTags.length() < 1)
        return false;
    if (this->TalkPage_WasRetrieved())
    {
        return this->TalkPage_GetContents().contains(this->Site->ProjectConfig->SharedIPTemplateTags);
    }
    return false;
}

bool WikiUser::IsWhitelisted()
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    if (this->whitelistInfo == HUGGLE_WL_TRUE)
        return true;
    if (this->whitelistInfo == HUGGLE_WL_FALSE)
        return false;
    QString spaced = this->Username;
    spaced.replace("_", " ");
    if (this->GetSite()->GetProjectConfig()->NewWhitelist.contains(this->Username) ||
            this->GetSite()->GetProjectConfig()->NewWhitelist.contains(spaced) ||
            this->GetSite()->GetProjectConfig()->WhiteList.contains(this->Username) ||
            this->GetSite()->GetProjectConfig()->WhiteList.contains(spaced))
    {
        this->whitelistInfo = HUGGLE_WL_TRUE;
        return true;
    } else
    {
        this->whitelistInfo = HUGGLE_WL_FALSE;
        return false;
    }
}

QString WikiUser::GetUserPage()
{
    // get a userspace prefix for this site
    WikiPageNS *ns = this->GetSite()->RetrieveNSByCanonicalName("User");
    QString prefix = ns->GetName();
    if (!prefix.size())
        prefix = "User";
    prefix += ":";
    return prefix + this->Username;
}

QString WikiUser::Flags()
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    QString pflags = "";
    QString nflags = "";
    if (this->TalkPage_GetContents().length() == 0 && this->TalkPage_WasRetrieved())
    {
        nflags += "T";
    } else
    {
        pflags += "T";
    }
    if (this->warningLevel > 0)
    {
        pflags += "w";
    }
    if (this->IsWhitelisted())
    {
        pflags += "E";
    }
    if (this->IsAnon())
    {
        nflags += "R";
    }
    if (this->IsReported)
    {
        pflags += "r";
    }
    if (this->isBot)
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

bool WikiUser::IsBot() const
{
    return this->isBot;
}

void WikiUser::SetBot(bool value)
{
    this->isBot = value;
}

void WikiUser::DecrementWarningLevel()
{
    this->warningLevel--;
    if (this->warningLevel < 0)
        this->warningLevel = 0;
}

void WikiUser::IncrementWarningLevel()
{
    this->warningLevel++;
    if (this->warningLevel > this->GetSite()->GetProjectConfig()->WarningLevel)
        this->warningLevel = this->GetSite()->GetProjectConfig()->WarningLevel;
}

void WikiUser::SetWarningLevel(byte_ht level)
{
    this->warningLevel = level;
}

void WikiUser::SetLastMessageTime(const QDateTime &date_time)
{
    if (this->LastMessageTimeKnown && this->LastMessageTime > date_time)
    {
        HUGGLE_DEBUG1(this->GetSite()->Name + ": user " + this->Username + " had LastWarningTime changed to past by " + QString::number(date_time.secsTo(this->LastMessageTime)) + " seconds");
    }

    this->LastMessageTimeKnown = true;
    this->LastMessageTime = date_time;
}

byte_ht WikiUser::GetWarningLevel() const
{
    return this->warningLevel;
}
