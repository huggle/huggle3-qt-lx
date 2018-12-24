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

#define EvaluatePageErrorReason_Missing 0
#define EvaluatePageErrorReason_Unknown 1
#define EvaluatePageErrorReason_NULL    2
#define EvaluatePageErrorReason_NoRevs  3
#define EvaluatePageErrorReason_Running 4

namespace Huggle
{
    class EditQuery;
    class RevertQuery;
    class Message;
    class WikiPage;
    class WikiEdit;
    class WikiSite;

    //! Provides helper functions that make it easy to do various stuff with mediawiki sites
    namespace WikiUtil
    {
        typedef void* (*RetrieveEditByRevid_Callback) (WikiEdit*, void*, QString);

        /*!
         * \brief APIRequest Perform an API request using ApiQuery class, this is just a helper function that makes it easier to invoke API calls
         * \param action     Refer to Action enum
         * \param site       WikiSite to execute request on
         * \param parameters Parameters of query
         * \param using_post If request should be submitted using POST method of HTTP protocol
         * \param target     Optional target name
         * \return           Pointer to ApiQuery
         */
        HUGGLE_EX_CORE Collectable_SmartPtr<ApiQuery> APIRequest(Action action, WikiSite *site, const QString &parameters, bool using_post = false, const QString &target = "");
        HUGGLE_EX_CORE bool IsRevert(const QString &summary);
        //! Return a localized month for a current wiki
        HUGGLE_EX_CORE QString MonthText(int n, WikiSite *site = nullptr);
        /*!
         * \brief RevertEdit Reverts the edit
         * \param _e Pointer to edit that needs to be reverted
         * \param summary Summary to use if this is empty the default revert summary is used
         * \param minor If revert should be considered as minor edit
         * \param rollback If rollback feature should be used
         * \return Pointer to api query that executes this revert
         */
        HUGGLE_EX_CORE Collectable_SmartPtr<RevertQuery> RevertEdit(WikiEdit* _e, const QString &summary = "", bool minor = false, bool rollback = true);
        /*!
         * \brief MessageUser Message user
         *
         * This function will deliver a message to user using Message class which is returned by this function
         *
         * \param User             Pointer to user
         * \param Text             Text of message
         * \param Title            Title of message
         * \param Summary          Edit summary that will be used while delivering the message
         * \param InsertSection    Whether this message should be created in a new section
         * \param DependencyRevert Rollback that is used as a dependency, if it's not NULL
         *                         the system will wait for it to finish before the message is sent
         * \param NoSuffix         Will not append huggle suffix if this is true, useful if you need to use custom summary
         * \param SectionKeep      If true, huggle will try to use the identical section, if there is not such a section it
         *                         make it
         * \param Autoremove       If true the message class is automatically removed by garbage collector once delivered
         * \param BaseTimestamp    This is used to resolve edit conflicts
         * \param CreateOnly       If true, nothing will be delivered to user if they already have a talk page
         * \param FreshOnly        Will abort the message delivery in case the current talk page in operating memory
         *                         was older than internal "MessageFresh" interval, used as a safeguard while delivering
         *                         warning to user. Typical usage is, that when the talk page is not fresh it needs
         *                         to be retrieved again in order to calculate proper warning level
         *
         * \return NULL on error or instance of Huggle::Message in case it's success
         */
        HUGGLE_EX_CORE Message *MessageUser(WikiUser *User, const QString &Text, const QString &Title, const QString &Summary, bool InsertSection = true,
                                       Query *Dependency = nullptr, bool NoSuffix = false, bool SectionKeep = false,
                                       bool Autoremove = true, const QString &BaseTimestamp = "", bool CreateOnly = false, bool FreshOnly = false);
        /*!
         * \brief SanitizeUser removes all invalid or problematic characters from user name
         * \param username Username that is to be fixed
         * \return
         */
        HUGGLE_EX_CORE QString SanitizeUser(QString username);
        HUGGLE_EX_CORE Collectable_SmartPtr<EditQuery> PrependTextToPage(WikiPage *page, const QString &text, const QString &summary = "Edited using huggle", bool minor = false);
        HUGGLE_EX_CORE Collectable_SmartPtr<EditQuery> PrependTextToPage(const QString &page, const QString &text, QString summary = "Edited using huggle", bool minor = false, WikiSite *site = nullptr);
        HUGGLE_EX_CORE Collectable_SmartPtr<EditQuery> AppendTextToPage(const QString &page, const QString &text, QString summary = "Edited using huggle", bool minor = false, WikiSite *site = nullptr);
        HUGGLE_EX_CORE Collectable_SmartPtr<EditQuery> AppendTextToPage(WikiPage *page, const QString &text, const QString &summary = "Edited using huggle", bool minor = false);
        HUGGLE_EX_CORE void FinalizeMessages();
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
        HUGGLE_EX_CORE Collectable_SmartPtr<EditQuery> EditPage(WikiSite *site, const QString &page, const QString &text, const QString &summary = "Edited using huggle", bool minor = false,
                                                                const QString &BaseTimestamp = "", unsigned int section = 0);
        HUGGLE_EX_CORE Collectable_SmartPtr<EditQuery> EditPage(WikiPage *page, const QString &text, QString summary = "Edited using huggle", bool minor = false,
                                                                const QString &BaseTimestamp = "", unsigned int section = 0);
        /*!
         * \brief EvaluateWikiPageContents evaluates the result of query
         * This function can be only used to check the results of query that was created in order to
         * retrieve contents of a wiki page.
         * \param query
         * \param failed In case the query has failed, this will be set to true
         * \param ts pointer where timestamp string should be stored (optional)
         * \param comment pointer where summary string should be stored (optional)
         * \param user pointer where user should be stored (optional)
         * \param revid id of revision (optional)
         * \param reason if there is a failure this is a number of error that happened
         * \return Text of wiki page or error message
         */
        HUGGLE_EX_CORE QString EvaluateWikiPageContents(ApiQuery *query, bool *failed, QString *ts = nullptr, QString *comment = nullptr,
                                                        QString *user = nullptr, long *revid = nullptr, int *reason = nullptr,
                                                        QString *title = nullptr);
        HUGGLE_EX_CORE void PatrolEdit(WikiEdit *edit);
        //! \obsolete RetrieveWikiPageContents(WikiPage *page, bool parse = false);
        HUGGLE_EX_CORE ApiQuery *RetrieveWikiPageContents(const QString &page, WikiSite *site, bool parse = false);
        HUGGLE_EX_CORE ApiQuery *RetrieveWikiPageContents(WikiPage *page, bool parse = false);
        HUGGLE_EX_CORE Collectable_SmartPtr<ApiQuery> Unwatchlist(WikiPage *page);
        HUGGLE_EX_CORE Collectable_SmartPtr<ApiQuery> Watchlist(WikiPage *page);
        /*!
         * \brief RetrieveEditByRevid Creates a new edit with a given Revid and queries the target site for all information needed
         * \param revid Revision
         * \param site Site from which the revision is fetched
         * \param source Source object that will be passed as a second argument to both callbacks
         * \param callback_success this function is called on successful finish
         * \param callback_er this function is called on error
         */
        HUGGLE_EX_CORE void RetrieveEditByRevid(revid_ht revid, WikiSite *site, void *source, RetrieveEditByRevid_Callback callback_success,
                                                RetrieveEditByRevid_Callback callback_er);
        HUGGLE_EX_CORE void RetrieveTokens(WikiSite *wiki_site);
        HUGGLE_EX_CORE WikiSite *GetSiteByName(QString name);
    }
}

#endif // WIKIUTIL_HPP
