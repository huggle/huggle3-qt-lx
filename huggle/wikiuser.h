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
        /*!
         * \brief List of users that are scored in this instance of huggle
         *
         * Either vandals or even good users, this list is preserved on shutdown and startup
         */
        static QList<WikiUser*> ProblematicUsers;
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
        //! In case that we retrieved the talk page during parse of warning level, this string contains it
        QString ContentsOfTalkPage;
        bool IsReported;
        /*!
         * \brief Change the IP property to true forcefully even if user isn't IP
         */
        void ForceIP();
        //! Returns true in case the current user is IP user
        bool IsIP();
        //! Update a list of problematic users
        static void UpdateUser(WikiUser *us);
        WikiUser();
        WikiUser(WikiUser *u);
        WikiUser(const WikiUser& u);
        WikiUser(QString user);
        //! Return a link to talk page of this user (like User talk:Jimbo)
        QString GetTalk();
        bool IsWhitelisted();
    private:
        //! Matches only IPv4
        static QRegExp IPv4Regex;
        //! Matches all IP
        static QRegExp IPv6Regex;
        int WhitelistInfo;
        bool IP;
    };
}

#endif // WIKIUSER_H
