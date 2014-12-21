//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef WIKIUTIL_HPP
#define WIKIUTIL_HPP

#include "definitions.hpp"

#include <QString>
#include "apiquery.hpp"
#include "editquery.hpp"
#include "message.hpp"
#include "revertquery.hpp"
#include "wikipage.hpp"

namespace Huggle
{
    class EditQuery;
    class RevertQuery;
    class Message;
    class WikiPage;

    namespace WikiUtil
    {
        HUGGLE_EX bool IsRevert(QString Summary);
        //! Return a localized month for a current wiki
        HUGGLE_EX QString MonthText(int n);
        /*!
         * \brief RevertEdit Reverts the edit
         * \param _e Pointer to edit that needs to be reverted
         * \param summary Summary to use if this is empty the default revert summary is used
         * \param minor If revert should be considered as minor edit
         * \param rollback If rollback feature should be used
         * \return Pointer to api query that executes this revert
         */
        HUGGLE_EX Collectable_SmartPtr<RevertQuery> RevertEdit(WikiEdit* _e, QString summary = "", bool minor = false, bool rollback = true);
        /*!
         * \brief MessageUser Message user
         *
         * This function will deliver a message to user using Message class which is returned by this function
         *
         * \param User Pointer to user
         * \param Text Text of message
         * \param Title Title of message
         * \param Summary Summary
         * \param InsertSection Whether this message should be created in a new section
         * \param DependencyRevert Rollback that is used as a dependency, if it's not NULL
         * the system will wait for it to finish before the message is sent
         * \param NoSuffix will not append huggle suffix if this is true, useful if you need to use custom summary
         * \param SectionKeep will try to use the identical section, if there is not such a section it
         *        make it
         *
         * \return NULL on error or instance of Huggle::Message in case it's success
         */
        HUGGLE_EX Message *MessageUser(WikiUser *User, QString Text, QString Title, QString Summary, bool InsertSection = true,
                                       Query *Dependency = nullptr, bool NoSuffix = false, bool SectionKeep = false,
                                       bool autoremove = true, QString BaseTimestamp = "", bool CreateOnly_ = false, bool FreshOnly_ = false);
        /*!
         * \brief SanitizeUser removes all invalid or problematic characters from user name
         * \param username Username that is to be fixed
         * \return
         */
        HUGGLE_EX QString SanitizeUser(QString username);
        HUGGLE_EX Collectable_SmartPtr<EditQuery> PrependTextToPage(WikiPage *page, QString text, QString summary = "Edited using huggle", bool minor = false);
        HUGGLE_EX Collectable_SmartPtr<EditQuery> PrependTextToPage(QString page, QString text, QString summary = "Edited using huggle", bool minor = false, WikiSite *site = nullptr);
        HUGGLE_EX Collectable_SmartPtr<EditQuery> AppendTextToPage(QString page, QString text, QString summary = "Edited using huggle", bool minor = false, WikiSite *site = nullptr);
        HUGGLE_EX Collectable_SmartPtr<EditQuery> AppendTextToPage(WikiPage *page, QString text, QString summary = "Edited using huggle", bool minor = false);
        HUGGLE_EX void FinalizeMessages();
        /*!
         * \brief EditPage Run a new EditQuery that will edit the page
         *
         * NOTE: References are incremented during query creation so that you can work with it later, in case you don't want to work
         *       with returned value you have to decrement its reference count, otherwise it will never be collected out
         * \param page
         * \param text
         * \param summary
         * \param minor
         * \param BaseTimestamp
         * \param section
         * \return New instance of edit query
         */
        HUGGLE_EX Collectable_SmartPtr<EditQuery> EditPage(QString page, QString text, QString summary = "Edited using huggle", bool minor = false,
                                                           QString BaseTimestamp = "", unsigned int section = 0);
        HUGGLE_EX Collectable_SmartPtr<EditQuery> EditPage(WikiPage *page, QString text, QString summary = "Edited using huggle", bool minor = false,
                                                           QString BaseTimestamp = "", unsigned int section = 0);
        HUGGLE_EX Collectable_SmartPtr<ApiQuery> Unwatchlist(WikiPage *page);
        HUGGLE_EX Collectable_SmartPtr<ApiQuery> Watchlist(WikiPage *page);
        void RetrieveTokens(WikiSite *wiki_site);
    }
}

#endif // WIKIUTIL_HPP
