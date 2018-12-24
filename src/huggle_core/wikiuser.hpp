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

#include "definitions.hpp"

#include <QList>
#include <QStringList>
#include <QDateTime>
#include <QString>
#include <QRegExp>
#include "mediawikiobject.hpp"

class QMutex;

namespace Huggle
{
    class WikiSite;
    class WikiPage;

    //! User
    class HUGGLE_EX_CORE WikiUser : public MediaWikiObject
    {
        public:
            //! Delete all users that have badness score 0 these users aren't necessary to be stored in a list
            static void TrimProblematicUsersList();
            static bool CompareUsernames(QString a, QString b);
            //! Update a list of problematic users
            static void UpdateUser(WikiUser *us);
            static bool IsIPv4(const QString &user);
            static bool IsIPv6(const QString &user);
            static void UpdateWl(WikiUser *us, long score);
            /*!
             * \brief Function that return static version of this user
             *
             * In case the user in question is already in list of problematic users, this function
             * will return its instance. It compares the username against the usernames that
             * are in this list.
             * \param user
             * \return static user from list of problematic users
             */
            static WikiUser *RetrieveUser(const QString &user, WikiSite *site);
            static WikiUser *RetrieveUser(WikiUser *user);
            /*!
             * \brief List of users that are scored in this instance of huggle
             *
             * Either vandals or even good users, this list is preserved on shutdown and startup
             */
            static QList<WikiUser*> ProblematicUsers;
            static QMutex ProblematicUserListLock;
            static QDateTime InvalidTime;

            WikiUser(WikiSite *site);
            WikiUser(WikiUser *u);
            WikiUser(const WikiUser& u);
            WikiUser(const QString &user, WikiSite *site);
            ~WikiUser() override;
            /*!
             * \brief GetContentsOfTalkPage returns a precached content of this user's talk page
             * If there is a global instance of this user, the talk page is retrieved from it
             * so that in case there are multiple instances of this user, they all share same
             * cached talk page.
             *
             * Because this function needs to obtain the user from global cache it may be slow,
             * in case you need to use its value multiple times, cache it as QString instead
             * of calling this function repeatedly
             * \return a precached content of this users talk page
             */
            QString TalkPage_GetContents();
            /*!
             * \brief SetContentsOfTalkPage Change a cache for talk page in local and global cache
             * \param text New content of talk page
             */
            void TalkPage_SetContents(const QString &text);
            //! Call UpdateUser on current user
            void Update(bool MatchingOnly = false);
            QString UnderscorelessUsername();
            void Sanitize();
            /*!
             * \brief Change the IP property to true forcefully even if user isn't IP
             */
            void ForceIP();
            //! Returns true in case the current user is IP user
            bool IsIP() const;
            bool EqualTo(WikiUser *user);
            //! This function will reparse whole talk page of user in order to figure out which level they have

            //! This function needs to be called after the content of talk page has changed
            void ParseTP(QDate bt);
            //! Update the information of this user based on global user list

            //! This is useful when you created user in past and since then a global user has changed
            //! so that you just call this to refresh all the scores and information or stuff
            bool Resync();
            //! Return a name of talk page of this user (like User talk:Jimbo)
            QString GetTalk();
            //! Returns a WikiPage object for talk page of this user, unlike "GetTalk()" which only returns a name of talk page
            WikiPage *GetTalkPage();
            bool TalkPage_WasRetrieved();
            bool TalkPage_ContainsSharedIPTemplate();
            //! Returns true if this user is wl
            bool IsWhitelisted();
            QDateTime TalkPage_RetrievalTime();
            QString GetUserPage();
            /*!
             * \brief Retrieve a badness score for current user, see WikiUser::BadnessScore for more
             * \param _resync If true the user will be resynced before the score is returned
             * \return badness score
             */
            long GetBadnessScore(bool _resync = true);
            void SetBadnessScore(long value, bool resync = true, bool update = true);
            //! Flags

            //! w - is warned
            //! r - is reported
            //! T- has talkpage
            //! R - is registered
            //! E - exception
            //! b - bot
            QString Flags();
            bool IsBot() const;
            void SetBot(bool value);
            void DecrementWarningLevel();
            void IncrementWarningLevel();
            void SetWarningLevel(byte_ht level);
            void SetLastMessageTime(const QDateTime &date_time);
            byte_ht GetWarningLevel() const;
            //! Username
            QString Username;
            bool IsBlocked;
            //! Local cache that holds information if user is reported or not. This information
            //! may be wrong, don't relly on it
            bool IsReported;
            //! Number of contributions, if it's not known, it contains negative value
            long EditCount;
            //! This is a mediawiki string containing a time when user was registered
            QString RegistrationDate;
            //! Groups that this user is in, by default it's empty
            QStringList Groups;
            bool LastMessageTimeKnown = false;
            //! Time when received last warning
            QDateTime LastMessageTime;

    protected:
            //! Matches only IPv4
            static QRegExp IPv4Regex;
            //! Matches all IP
            static QRegExp IPv6Regex;

            /*!
             * \brief Badness score of current user
             *
             * This score change the badness score of edit, score can be positive (bad) as well as negative
             * in case you want to change the score, don't forget to call WikiUser::UpdateUser(WikiUser *user)
             */
            long BadnessScore;
            //! Current warning level of user
            byte_ht warningLevel;
            //! Status of whitelist 0 means user is not whitelisted, 1 that it is and different value means we don't know
            byte_ht whitelistInfo;
            //! In case that we retrieved the talk page during parse of warning level, this string contains it
            QString contentsOfTalkPage;
            bool talkPageWasRetrieved;
            //! This is a date when we retrieved this talk page
            QDateTime dateOfTalkPage;
            QMutex *userMutex;
            WikiPage *wpTalkPage = nullptr;
            bool isBot;
            bool IP;
    };

    inline void WikiUser::Sanitize()
    {
        this->Username = this->Username.replace(" ", "_");
    }

    inline void WikiUser::ForceIP()
    {
        this->IP = true;
    }

    inline bool WikiUser::TalkPage_WasRetrieved()
    {
        return this->talkPageWasRetrieved;
    }

    inline bool WikiUser::IsIP() const
    {
        return this->IP;
    }

    inline QDateTime WikiUser::TalkPage_RetrievalTime()
    {
        return this->dateOfTalkPage;
    }

    inline long WikiUser::GetBadnessScore(bool _resync)
    {
        if (_resync)
        {
            this->Resync();
        }
        return this->BadnessScore;
    }

    inline void WikiUser::SetBadnessScore(long value, bool resync, bool update)
    {
        if (resync)
            this->Resync();
        this->BadnessScore = value;
        if (update)
            this->Update(true);
    }
}

#endif // WIKIUSER_H
