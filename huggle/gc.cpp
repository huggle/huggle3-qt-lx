//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "gc.h"

QList<Huggle::Collectable*> Huggle::GC::list;
QMutex Huggle::GC::Lock(QMutex::Recursive);

void Huggle::GC::DeleteOld()
{
    Huggle::GC::Lock.lock();
    int x=0;
    while(x < GC::list.count())
    {
        Collectable *q = GC::list.at(x);
        q->Lock();
        if (!q->IsManaged())
        {
            GC::list.removeAt(x);
            delete q;
            continue;
        }
        if (!q->SafeDelete())
        {
            q->Unlock();
            x++;
        }
    }
    Huggle::GC::Lock.unlock();
}
