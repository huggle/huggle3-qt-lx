//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "query.h"

using namespace Huggle;

unsigned int Query::LastID = 0;
QNetworkAccessManager Query::NetworkManager;

Query::Query()
{
    this->Result = NULL;
    this->Type = QueryNull;
    this->Status = StatusNull;
    this->ID = this->LastID;
    this->LastID++;
    this->CustomStatus = "";
    this->Dependency = NULL;
    this->Managed = false;
}

Query::~Query()
{
    if (this->Managed)
    {
        throw new Exception("Request to delete managed query");
    }
    delete Result;
    this->Result = NULL;
}

bool Query::Processed()
{
    if (this->Status == StatusDone)
    {
        return true;
    }
    return false;
}

bool Query::IsManaged()
{
    if (this->Managed)
    {
        return true;
    }
    return (this->Consumers.count() > 0);
}

QString Query::QueryTypeToString()
{
    switch (this->Type)
    {
        case QueryNull:
            return "null";
        case QueryApi:
            return "Api Query";
        case QueryEdit:
            return "Edit Query";
    }
    return "Unknown";
}

QString Query::QueryTargetToString()
{
    return "Invalid target";
}

QString Query::QueryStatusToString()
{
    if (this->CustomStatus != "")
    {
        return CustomStatus;
    }

    switch (this->Status)
    {
    case StatusNull:
        return "NULL";
    case StatusDone:
        return "Done";
    case StatusProcessing:
        return "Processing";
    case StatusInError:
        return "InError";
    }
    return "Unknown";
}

bool Query::SafeDelete(bool forced)
{
    this->Managed = true;

    if (!forced && Consumers.count() == 0)
    {
        if (QueryGC::qgc.contains(this))
        {
            QueryGC::qgc.removeAll(this);
        }
        this->Managed = false;
        delete this;
        return true;
    }

    if (!QueryGC::qgc.contains(this))
    {
        QueryGC::qgc.append(this);
    }
    return false;
}

void Query::RegisterConsumer(QString consumer)
{
    this->Managed = true;
    this->Consumers.append(consumer);
    this->Consumers.removeDuplicates();
}

void Query::UnregisterConsumer(QString consumer)
{
    this->Managed = true;
    this->Consumers.removeAll(consumer);
}

QString Query::DebugQgc()
{
    QString result = "";
    if (this->Consumers.count() > 0)
    {
        result += ("GC: Listing all dependencies for " + QString::number(this->QueryID())) + "\n";
        int Item=0;
        while (Item < this->Consumers.count())
        {
            result +=("GC: " + QString::number(this->QueryID()) + " " + this->Consumers.at(Item)) + "\n";
            Item++;
        }
    } else
    {
        result += "No consumers found: " + QString::number(this->QueryID());
    }
    return result;
}

unsigned int Query::QueryID()
{
    return this->ID;
}
