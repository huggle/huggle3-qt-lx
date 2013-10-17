//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef QUERYGC_H
#define QUERYGC_H

#include <QList>
#include "query.h"

namespace Huggle
{
    class Query;

    //! Garbage collector

    //! Garbage collector for old queries that should be deleted,
    //! but can't be right now
    class QueryGC
    {
    public:
        //! List of all managed queries that qgc keeps track of
        static QList<Query*> qgc;
        //! Function that walks through the list and delete these that can be deleted
        static void DeleteOld();
    };
}

#endif // QUERYGC_H
