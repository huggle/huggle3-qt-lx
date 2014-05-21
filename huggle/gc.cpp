//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "gc.hpp"

using namespace Huggle;

GC *GC::gc = nullptr;

Huggle::GC::GC()
{
    this->Lock = new QMutex(QMutex::Recursive);
}

Huggle::GC::~GC()
{
    delete this->Lock;
}

void Huggle::GC::DeleteOld()
{
    this->Lock->lock();
    int x=0;
    while(x < this->list.count())
    {
        Collectable *q = this->list.at(x);
        q->Lock();
        if (!q->IsManaged())
        {
            this->list.removeAt(x);
            delete q;
            continue;
        }
        if (!q->SafeDelete())
        {
            q->Unlock();
            x++;
        }
    }
    this->Lock->unlock();
}
