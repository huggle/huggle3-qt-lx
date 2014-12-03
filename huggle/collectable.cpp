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

#include "collectable.hpp"
#include "exception.hpp"
#include "syslog.hpp"

using namespace Huggle;

#ifdef HUGGLE_PROFILING
unsigned long Collectable::LockCt = 0;
#endif

unsigned long Collectable::LastCID = 0;
QMutex *Collectable::WideLock = new QMutex(QMutex::Recursive);

Collectable::Collectable()
{
    Collectable::WideLock->lock();
    this->CID = Collectable::LastCID;
    Collectable::LastCID++;
    Collectable::WideLock->unlock();
#if PRODUCTION_BUILD == 1
    this->ReclaimingAllowed = true;
#else
    // don't crash huggle purposefuly unless it's for development
    this->ReclaimingAllowed = false;
#endif
    this->_collectableLocked = false;
    this->_collectableManaged = false;
    this->_collectableRefs = 0;
    this->_collectableQL = new QMutex(QMutex::Recursive);
}

Collectable::~Collectable()
{
    if (this->IsManaged())
    {
        throw new Huggle::Exception("Request to delete managed entity", BOOST_CURRENT_FUNCTION);
    }
    this->Unlock();
    delete this->_collectableQL;
}

bool Collectable::SafeDelete()
{
    if (this->_collectableRefs == 0 && this->Consumers.count() == 0 && this->iConsumers.count() == 0)
    {
        if (GC::gc != nullptr)
        {
            GC::gc->Lock->lock();
            if (GC::gc->list.contains(this))
            {
                GC::gc->list.removeAll(this);
            }
            GC::gc->Lock->unlock();
        } else
        {
            Syslog::HuggleLogs->DebugLog("SafeDelete() called after GC pointer was removed");
        }
        this->_collectableManaged = false;
        delete this;
        return true;
    }

    this->SetManaged();
    return false;
}

void Collectable::SetReclaimable()
{
    this->ReclaimingAllowed = true;
}

void Collectable::DecRef()
{
    if (!this->_collectableRefs)
    {
        throw new Huggle::Exception("Decrementing negative reference", BOOST_CURRENT_FUNCTION);
    }
    this->_collectableRefs--;
}

void Collectable::RegisterConsumer(int consumer)
{
    this->Lock();
    if (this->IsManaged() && !this->HasSomeConsumers() && !this->ReclaimingAllowed)
    {
        this->Unlock();
        throw new Huggle::Exception("You can't reclaim this managed resource", BOOST_CURRENT_FUNCTION);
    }
    if (!this->iConsumers.contains(consumer))
    {
        this->iConsumers.append(consumer);
    }
    this->SetManaged();
    this->Unlock();
}

void Collectable::UnregisterConsumer(int consumer)
{
    this->Lock();
    if (this->IsManaged() && !this->HasSomeConsumers())
    {
        this->Unlock();
        Syslog::HuggleLogs->DebugLog("You are working with class that was already scheduled for collection!");
        //throw new Huggle::Exception("You are working with class that was already scheduled for collection",
        //                            BOOST_CURRENT_FUNCTION);
    }
    this->iConsumers.removeOne(consumer);
    this->SetManaged();
    this->Unlock();
}

void Collectable::RegisterConsumer(const QString consumer)
{
    this->Lock();
    if (this->IsManaged() && !this->HasSomeConsumers() && !this->ReclaimingAllowed)
    {
        this->Unlock();
        throw new Huggle::Exception("You can't reclaim this managed resource", BOOST_CURRENT_FUNCTION);
    }
    this->Consumers.append(consumer);
    this->Consumers.removeDuplicates();
    this->SetManaged();
    this->Unlock();
}

void Collectable::UnregisterConsumer(const QString consumer)
{
    this->Lock();
    if (this->IsManaged() && !this->HasSomeConsumers())
    {
        this->Unlock();
        Huggle::Exception::ThrowSoftException("You are working with class that was already scheduled for collection",
                                              BOOST_CURRENT_FUNCTION);
    }
    this->Consumers.removeOne(consumer);
    this->SetManaged();
    this->Unlock();
}

QString Collectable::ConsumerIdToString(const int id)
{
    switch (id)
    {
        case HUGGLECONSUMER_WIKIEDIT:
            return "WikiEdit";
        case HUGGLECONSUMER_QUEUE:
            return "Queue";
        case HUGGLECONSUMER_CORE_POSTPROCESS:
            return "Core::Postprocess";
        case HUGGLECONSUMER_EDITQUERY:
            return "EditQuery";
        case HUGGLECONSUMER_REVERTQUERY:
            return "RevertQuery";
    }
    return "Unknown consumer: " + QString::number(id);
}

void Collectable::SetManaged()
{
    if (this->_collectableManaged)
    {
        return;
    }
    this->_collectableManaged = true;
    if (GC::gc == nullptr)
    {
        // huggle is probably shutting down
        return;
    }
    if (!GC::gc->list.contains(this))
    {
        GC::gc->list.append(this);
    }
}

QString Collectable::DebugHgc()
{
    QString result = "";
    if (this->HasSomeConsumers())
    {
        result += ("GC: Listing all dependencies for " + QString::number(this->CollectableID())) + "\n";
        if (this->_collectableRefs > 0)
        {
            result += QString::number(this->_collectableRefs) + " unknown references\n";
        }
        int Item=0;
        while (Item < this->Consumers.count())
        {
            result +=("GC: " + QString::number(this->CollectableID()) + " " + this->Consumers.at(Item)) + "\n";
            Item++;
        }
        Item=0;
        while (Item < this->iConsumers.count())
        {
            result +=("GC: " + QString::number(this->CollectableID()) + " " + ConsumerIdToString( this->iConsumers.at(Item)) ) + "\n";
            Item++;
        }
    } else
    {
        result += "No consumers found: " + QString::number(this->CollectableID());
    }
    return result;
}

void Collectable::Lock()
{
#ifdef HUGGLE_PROFILING
    Collectable::LockCt++;
#endif
    // this is actually pretty lame check but better than nothing
    if (!this->_collectableLocked)
    {
        this->_collectableQL->lock();
        this->_collectableLocked = true;
    }
}

void Collectable::Unlock()
{
#ifdef HUGGLE_PROFILING
    Collectable::LockCt++;
#endif
    if (this->_collectableLocked)
    {
        this->_collectableQL->unlock();
        this->_collectableLocked = false;
    }
}
