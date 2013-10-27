//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef GC_H
#define GC_H

#include <QList>
#include <QMutex>
#include "collectable.h"

#define HUGGLECONSUMER_WIKIEDIT 0
#define HUGGLECONSUMER_PROVIDERIRC 1

namespace Huggle
{
    class Collectable;
    class GC
    {
    public:
        //! List of all managed queries that qgc keeps track of
        static QList<Collectable*> list;
        static QMutex Lock;
        //! Function that walks through the list and delete these that can be deleted
        static void DeleteOld();
    };
}

#endif // GC_H
