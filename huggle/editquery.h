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

class ApiQuery;

class EditQuery : public Query
{
public:
    EditQuery();
    ~EditQuery();
    void Process();
    bool Processed();
    QString page;
    QString text;
    QString summary;
    ApiQuery *qToken;
    ApiQuery *qEdit;
    bool Minor;
    QString _Token;
};

#endif // EDITQUERY_H
