//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "collectable.hpp"

using namespace Huggle;

unsigned long Collectable::LastCID = 0;
QMutex Collectable::WideLock(QMutex::Recursive);

Collectable::Collectable()
{
    WideLock.lock();
    this->CID = Collectable::LastCID;
    Collectable::LastCID++;
    WideLock.unlock();
    this->Locked = false;
    this->Managed = false;
    this->QL = new QMutex(QMutex::Recursive);
}

Collectable::~Collectable()
{
    if (this->IsManaged())
    {
        throw new Exception("Request to delete managed entity");
    }
    delete this->QL;
}

bool Collectable::SafeDelete()
{
    if (this->Consumers.count() == 0 && this->iConsumers.count() == 0)
    {
        GC::Lock.lock();
        if (GC::list.contains(this))
        {
            GC::list.removeAll(this);
        }
        GC::Lock.unlock();
        this->Managed = false;
        delete this;
        return true;
    }

    this->SetManaged();
    return false;
}

void Collectable::RegisterConsumer(const int consumer)
{
    this->Lock();
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

QString Collectable::ConsumerIdToString(const int id)
{
    switch (id)
    {
    case HUGGLECONSUMER_WIKIEDIT:
        return "WikiEdit";
    case HUGGLECONSUMER_PROVIDERIRC:
        return "ProviderIRC";
    }
    return "Unknown consumer: " + QString::number(id);
}

void Collectable::SetManaged()
{
    this->Managed = true;
    if (!GC::list.contains(this))
    {
        GC::list.append(this);
    }
}

QString Collectable::DebugHgc()
{
    QString result = "";
    if (this->iConsumers.count() > 0 || this->Consumers.count() > 0)
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
    return Locked;
}

void Collectable::Lock()
{
    this->QL->lock();
    Locked = true;
}

void Collectable::Unlock()
{
    this->QL->unlock();
    Locked = false;
}

bool Collectable::IsManaged()
{
    if (this->Managed)
    {
        return true;
    }
    return ((this->Consumers.count() > 0) || (this->iConsumers.count() > 0));
}
