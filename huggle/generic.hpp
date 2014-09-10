//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef GENERIC_HPP
#define GENERIC_HPP

#include "definitions.hpp"

#include <QString>

#define EvaluatePageErrorReason_Missing 0
#define EvaluatePageErrorReason_Unknown 1
#define EvaluatePageErrorReason_NULL    2
#define EvaluatePageErrorReason_NoRevs  3
#define EvaluatePageErrorReason_Running 4

namespace Huggle
{
    class ApiQuery;
    class WikiEdit;
    class WikiPage;
    class WikiSite;

    enum MessageBoxStyle
    {
        MessageBoxStyleNormal,
        MessageBoxStyleQuestion,
        MessageBoxStyleWarning,
        MessageBoxStyleError
    };

    //! Generic requests that are frequently issued to wiki
    namespace Generic
    {
        /*!
         * \brief Bool2String Convert a bool to string
         * \param b bool
         * \return string
         */
        QString Bool2String(bool b);
        /*!
         * \brief CompareVersions
         * \param a version
         * \param b version
         * \return true in case version b is older or same as version a
         */
        bool CompareVersions(QString a, QString b);
        bool SafeBool(QString value, bool defaultvalue = false);
        //! Display a user message before reporting a user based on user preferences
        bool ReportPreFlightCheck();
        /*!
         * \brief MessageBox Display a message box
         * \param title Title of message box
         * \param text What is displayed in a message
         */
        int MessageBox(QString title, QString text, MessageBoxStyle st = MessageBoxStyleNormal, bool enforce_stop = false);
        //! Display a message box telling user that function is not allowed during developer mode
        void DeveloperError();
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
        QString EvaluateWikiPageContents(ApiQuery *query, bool *failed, QString *ts = nullptr, QString *comment = nullptr,
                                         QString *user = nullptr, long *revid = nullptr, int *reason = nullptr,
                                         QString *title = nullptr);
        //! \obsolete RetrieveWikiPageContents(WikiPage *page, bool parse = false);
        ApiQuery *RetrieveWikiPageContents(QString page, WikiSite *site, bool parse = false);
        ApiQuery *RetrieveWikiPageContents(WikiPage *page, bool parse = false);
        QString ShrinkText(QString text, unsigned int size, bool html = true, unsigned int minimum = 2);
    }
}

#endif // GENERIC_HPP
