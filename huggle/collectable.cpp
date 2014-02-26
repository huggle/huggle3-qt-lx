//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "config.hpp"
#include "collectable.hpp"

using namespace Huggle;

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
    this->_collectableQL = new QMutex(QMutex::Recursive);
}

Collectable::~Collectable()
{
    if (this->IsManaged())
    {
        throw new Exception("Request to delete managed entity");
    }
    this->Unlock();
    delete this->_collectableQL;
}

bool Collectable::SafeDelete()
{
    if (this->Consumers.count() == 0 && this->iConsumers.count() == 0)
    {
        if (GC::gc != NULL)
        {
            GC::gc->Lock->lock();
            if (GC::gc->list.contains(this))
            {
                GC::gc->list.removeAll(this);
            }
            GC::gc->Lock->unlock();
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

void Collectable::RegisterConsumer(const int consumer)
{
    this->Lock();
    if (this->IsManaged() && !this->HasSomeConsumers() && !this->ReclaimingAllowed)
    {
        this->Unlock();
        throw new Huggle::Exception("You can't reclaim this managed resource", "void Collectable::RegisterConsumer(const int consumer)");
    }
    if (!this->iConsumers.contains(consumer))
    {
        this->iConsumers.append(consumer);
    }
    this->SetManaged();
    this->Unlock();
}

void Collectable::UnregisterConsumer(const int consumer)
{
    this->Lock();
    this->iConsumers.removeAll(consumer);
    this->SetManaged();
    this->Unlock();
}

void Collectable::RegisterConsumer(const QString consumer)
{
    this->Lock();
    if (this->IsManaged() && !this->HasSomeConsumers() && !this->ReclaimingAllowed)
    {
        this->Unlock();
        throw new Huggle::Exception("You can't reclaim this managed resource", "void Collectable::RegisterConsumer(const QString consumer)");
    }
    this->Consumers.append(consumer);
    this->Consumers.removeDuplicates();
    this->SetManaged();
    this->Unlock();
}

void Collectable::UnregisterConsumer(const QString consumer)
{
    this->Lock();
    this->Consumers.removeAll(consumer);
    this->SetManaged();
    this->Unlock();
}

unsigned long Collectable::CollectableID()
{
    return this->CID;
}

unsigned long *Collectable::GetLastCIDPtr()
{
    return &Collectable::LastCID;
}

QString Collectable::ConsumerIdToString(const int id)
{
    switch (id)
    {
        case HUGGLECONSUMER_WIKIEDIT:
            return "WikiEdit";
        case HUGGLECONSUMER_PROVIDERIRC:
            return "ProviderIRC";
        case HUGGLECONSUMER_QUEUE:
            return "Queue";
        case HUGGLECONSUMER_CORE_POSTPROCESS:
            return "Core::Postprocess";
        case HUGGLECONSUMER_DELETEFORM:
            return "Delete Form";
        case HUGGLECONSUMER_PROCESSLIST:
            return "ProcessList";
        case HUGGLECONSUMER_HUGGLETOOL:
            return "HuggleTool";
        case HUGGLECONSUMER_EDITQUERY:
            return "EditQuery";
        case HUGGLECONSUMER_REVERTQUERY:
            return "RevertQuery";
        case HUGGLECONSUMER_MAINFORM:
            return "Main Form";
        case HUGGLECONSUMER_LOGINFORM:
            return "Login Form";
    }
    return "Unknown consumer: " + QString::number(id);
}

void Collectable::SetManaged()
{
    this->_collectableManaged = true;
    if (GC::gc == NULL)
    {
        // huggle is probably shutting down
        return;
    }
    if (!GC::gc->list.contains(this))
    {
        GC::gc->list.append(this);
    }
}

bool Collectable::HasSomeConsumers()
{
    return (this->iConsumers.count() > 0 || this->Consumers.count() > 0);
}

QString Collectable::DebugHgc()
{
    QString result = "";
    if (this->HasSomeConsumers())
    {
        result += ("GC: Listing all dependencies for " + QString::number(this->CollectableID())) + "\n";
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

bool Collectable::IsLocked()
{
    return this->_collectableLocked;
}

void Collectable::Lock()
{
    // this is actually pretty lame check but better than nothing
    if (!this->_collectableLocked)
    {
        this->_collectableQL->lock();
        this->_collectableLocked = true;
    }
}

void Collectable::Unlock()
{
    if (this->_collectableLocked)
    {
        this->_collectableQL->unlock();
        this->_collectableLocked = false;
    }
}

bool Collectable::IsManaged()
{
    if (this->_collectableManaged)
    {
        return true;
    }
    return ((this->Consumers.count() > 0) || (this->iConsumers.count() > 0));
}
