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
// now we need to ensure that python is included first
#ifdef PYTHONENGINE
#include <Python.h>
#endif

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

    //! Generic requests that are frequently issued to wiki
    namespace Generic
    {
        //! Display a user message before reporting a user based on user preferences
        bool ReportPreFlightCheck();
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
                                         QString *user = nullptr, int *revid = nullptr, int *reason = nullptr,
                                         QString *title = nullptr);
        ApiQuery *RetrieveWikiPageContents(QString page, bool parse = false);
        QString ShrinkText(QString text, int size, bool html = true);
    }
}

#endif // GENERIC_HPP
