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

#include <QString>

class QueryResult
{
public:
    QueryResult();
    QString Data;
    QString ErrorMessage;
    bool Failed;
};

#endif // QUERYRESULT_H
