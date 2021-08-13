//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef QUERYRESULT_H
#define QUERYRESULT_H

#include "definitions.hpp"

#include <QHash>
#include <QString>

constexpr int HUGGLE_EUNKNOWN = 1;
constexpr int HUGGLE_ENOTLOGGEDIN = 2;
constexpr int HUGGLE_EKILLED = 3;
constexpr int HUGGLE_ETOKEN = 4;
constexpr int HUGGLE_EREADONLY = 5;

namespace Huggle
{
    //! Result of query

    //! This is abstract result of every web query, it can be used for API queries as well and their result should be always parsed
    //! using this, instead of native XML parsers so that we can change the API output format while keeping the code unchanged.
    class HUGGLE_EX_CORE QueryResult
    {
        public:
            //! Creates a new instance of query result
            QueryResult();
            QueryResult(bool failed);
            virtual ~QueryResult() = default;
            //! Data retrieved by query, this contains the JSON / XML for api requests
            QString Data;
            void SetError();
            void SetError(const QString &error);
            void SetError(int error, const QString &details = "");
            //! Check whether query has failed
            virtual bool IsFailed();
            //! If query is in error the reason for error is stored here, otherwise it's null string
            QString ErrorMessage;
            int ErrorCode = 0;
        protected:
            bool Failed;
    };
}

#endif // QUERYRESULT_H
