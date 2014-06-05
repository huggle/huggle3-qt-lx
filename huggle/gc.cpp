//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "gc.hpp"
#include "exception.hpp"

using namespace Huggle;

GC *GC::gc = nullptr;

Huggle::GC::GC()
{
    this->Lock = new QMutex(QMutex::Recursive);
    this->gc_t = new GC_t();
    // this is a background task
    this->gc_t->start(QThread::LowestPriority);
}

Huggle::GC::~GC()
{
    delete this->Lock;
    delete this->gc_t;
}

void Huggle::GC::DeleteOld()
{
    this->Lock->lock();
    int x=0;
    if (this->list.count() > GC_LIMIT)
    {
        QList<Collectable*> copy(this->list);
        this->Lock->unlock();
        while(x < copy.count())
        {
            Collectable *q = copy.at(x);
            q->Lock();
            if (!q->IsManaged())
            {
                copy.removeAt(x);
                delete q;
                continue;
            }
            if (!q->SafeDelete())
            {
                q->Unlock();
                x++;
            } else
            {
                // we can remove it now because it's deleted
                copy.removeOne(q);
            }
        }
        return;
    }
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

void GC::Start()
{
    if (this->gc_t == nullptr)
        throw new Huggle::Exception("gc_t can't be NULL");

    if (this->gc_t->IsStopped())
    {
        delete this->gc_t;
        this->gc_t = new GC_t();
        this->gc_t->start(QThread::LowestPriority);
    }
}

void GC::Stop()
{
    if (this->gc_t == nullptr)
        throw new Huggle::Exception("gc_t can't be NULL");

    if (this->gc_t->IsRunning())
    {
        this->gc_t->Stop();
    }
}

bool GC::IsRunning()
{
    if (this->gc_t == nullptr)
        throw new Huggle::Exception("gc_t can't be NULL");

    return (this->gc_t->IsRunning() || !this->gc_t->IsStopped());
}
