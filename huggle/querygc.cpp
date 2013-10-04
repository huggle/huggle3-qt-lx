//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "querygc.h"

using namespace Huggle;

QList<Query*> QueryGC::qgc;

void QueryGC::DeleteOld()
{
    int curr=0;
    while(curr<QueryGC::qgc.count())
    {
        Query *q = QueryGC::qgc.at(curr);
        if (!q->IsManaged())
        {
            QueryGC::qgc.removeAt(curr);
            delete q;
            continue;
        }
        if (!q->SafeDelete())
        {
            curr++;
        }
    }
}
