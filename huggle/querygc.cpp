//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "querygc.h"

QList<Query*> QueryGC::qgc;

void QueryGC::DeleteOld()
{
    int curr=0;
    QList<Query*> list(QueryGC::qgc);
    while(curr<list.count())
    {
        Query *q = list.at(curr);
        if (q->Consumers.count() == 0)
        {
            QueryGC::qgc.removeOne(list.at(curr));
            delete list.at(curr);
        }
        curr++;
    }
}
