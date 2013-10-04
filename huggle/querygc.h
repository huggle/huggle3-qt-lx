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
    class QueryGC
    {
    public:
        //! Garbage collector for old queries that should be deleted,
        //! but can't be right now
        static QList<Query*> qgc;
        static void DeleteOld();
    };
}

#endif // QUERYGC_H
