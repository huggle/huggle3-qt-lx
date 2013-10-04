//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef EDITQUERY_H
#define EDITQUERY_H

#include <QString>
#include <QUrl>
//#include "apiquery.h"
#include "core.h"
#include "history.h"

namespace Huggle
{
    class ApiQuery;

    //! Modifications of mediawiki pages can be done using this query
    class EditQuery : public Query
    {
    public:
        EditQuery();
        ~EditQuery();
        void Process();
        bool Processed();
        //! Page that is going to be edited
        QString page;
        //! Text a page will be replaced with
        QString text;
        //! Edit summary
        QString summary;
        ApiQuery *qToken;
        //! Api query to edit page
        ApiQuery *qEdit;
        //! Whether the edit is minor or not
        bool Minor;
        //! Edit token, will be retrieved during request
        QString _Token;
    };
}

#endif // EDITQUERY_H
