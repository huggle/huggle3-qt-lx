// IMPORTANT: this file has a different license than rest of huggle

//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2014

#include "gc.hpp"
#include "collectable.hpp"
#include "gc_thread.hpp"
#include "exception.hpp"

using namespace Huggle;

GC *GC::gc = nullptr;

Huggle::GC::GC()
{
    this->Lock = new QMutex(QMutex::Recursive);
#ifdef HUGGLE_USE_MT_GC
    this->gc_t = new GC_t();
    // this is a background task
    this->gc_t->start(QThread::LowestPriority);
#else
    this->gc_t = nullptr;
#endif
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
            if (q->HasSomeConsumers())
            {
                // there is no need to work with this collectable item
                x++;
                continue;
            }
            q->Lock();
            if (!q->IsManaged())
            {
                copy.removeAt(x);
                // this is very unlikely to happen, so despite this
                // is rather slow, it needs to be done
                this->Lock->lock();
                this->list.removeAll(q);
                this->Lock->unlock();
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
                copy.removeAll(q);
            }
        }
        return;
    }
    while(x < this->list.count())
    {
        Collectable *q = this->list.at(x);
        if (q->HasSomeConsumers())
        {
            // there is no need to work with this collectable item
            x++;
            continue;
        }
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
#ifdef HUGGLE_USE_MT_GC
    if (this->gc_t == nullptr)
        throw new Huggle::Exception("gc_t can't be NULL");

    if (this->gc_t->IsStopped())
    {
        delete this->gc_t;
        this->gc_t = new GC_t();
        this->gc_t->start(QThread::LowestPriority);
    }
#endif
}

void GC::Stop()
{
#ifdef HUGGLE_USE_MT_GC
    if (this->gc_t == nullptr)
        throw new Huggle::Exception("gc_t can't be NULL");

    if (this->gc_t->IsRunning())
    {
        this->gc_t->Stop();
    }
#endif
}

bool GC::IsRunning()
{
#ifdef HUGGLE_USE_MT_GC
    if (this->gc_t == nullptr)
        throw new Huggle::Exception("gc_t can't be NULL");

    return (this->gc_t->IsRunning() || !this->gc_t->IsStopped());
#else
    return false;
#endif
}
