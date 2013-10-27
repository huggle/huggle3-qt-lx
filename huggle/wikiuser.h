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
#include <QMutex>
#include <QString>
#include <QRegExp>

#include "configuration.h"
#include "wikiedit.h"

namespace Huggle
{
    class WikiEdit;

    //! User
    class WikiUser
    {
    public:
        /*!
         * \brief List of users that are scored in this instance of huggle
         *
         * Either vandals or even good users, this list is preserved on shutdown and startup
         */
        static QList<WikiUser*> ProblematicUsers;
        //! Update a list of problematic users
        static void UpdateUser(WikiUser *us);
        static WikiUser *RetrieveUser(WikiUser *user);
        static QMutex ProblematicUserListLock;
        static void TrimProblematicUsersList();
        //! Username
        QString Username;
        /*!
         * \brief Badness score of current user
         *
         * This score change the badness score of edit, score can be positive (bad) as well as negative
         * in case you want to change the score, don't forget to call WikiUser::UpdateUser(WikiUser *user)
         */
        long BadnessScore;
        //! Current warning level of user
        int WarningLevel;
        /*!
         * \brief GetContentsOfTalkPage returns a precached content of this users talk page
         * If there is a global instance of this user, the talk page is retrieved from it
         * so that in case there are multiple instances of this user, they all share same
         * cached talk page.
         *
         * Because this function needs to obtain the user from global cache it may be slow,
         * in case you need to use its value multiple times, cache it as QString instead
         * of calling this function repeatedly
         * \return a precached content of this users talk page
         */
        QString GetContentsOfTalkPage();
        /*!
         * \brief SetContentsOfTalkPage Change a cache for talk page in local and global cache
         * \param text New content of talk page
         */
        void SetContentsOfTalkPage(QString text);
        //! Local cache that holds information if user is reported or not. This information
        //! may be wrong, don't relly on it
        bool IsReported;
        //! Call UpdateUser on current user
        void Update();
        //! Cache of contributions made by this user
        QList<WikiEdit*> Contributions;
        /*!
         * \brief Change the IP property to true forcefully even if user isn't IP
         */
        void ForceIP();
        //! Returns true in case the current user is IP user
        bool IsIP();
        WikiUser();
        WikiUser(WikiUser *u);
        WikiUser(const WikiUser& u);
        WikiUser(QString user);
        ~WikiUser();
        //! Return a link to talk page of this user (like User talk:Jimbo)
        QString GetTalk();
        //! Returns true if this user is wl
        bool IsWhitelisted();
    private:
        //! Matches only IPv4
        static QRegExp IPv4Regex;
        //! Matches all IP
        static QRegExp IPv6Regex;
        int WhitelistInfo;
        //! In case that we retrieved the talk page during parse of warning level, this string contains it
        QString ContentsOfTalkPage;
        QMutex *UserLock;
        bool IP;
    };
}

#endif // WIKIUSER_H
