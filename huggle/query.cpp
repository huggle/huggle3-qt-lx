//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "query.hpp"

using namespace Huggle;

unsigned int Query::LastID = 0;
QNetworkAccessManager *Query::NetworkManager = NULL;

Query::Query()
{
    this->Result = NULL;
    this->Type = QueryNull;
    this->Status = StatusNull;
    this->ID = this->LastID;
    this->LastID++;
    this->CustomStatus = "";
    this->callback = NULL;
    this->HiddenQuery = false;
    this->Dependency = NULL;
    this->Timeout = 60;
    this->CallbackResult = NULL;
    this->StartTime = QDateTime::currentDateTime();
    this->RetryOnTimeoutFailure = true;
}

Query::~Query()
{
    delete this->Result;
    if (this->CallbackResult != NULL)
    {
        throw new Exception("Memory leak: Query::CallbackResult was not deleted before destructor was called");
    }
    this->Result = NULL;
}

bool Query::Processed()
{
    if (this->Status == StatusDone || this->Status == StatusInError)
    {
        return true;
    }
    if (QDateTime::currentDateTime() > this->StartTime.addSecs(this->Timeout))
    {
        if (!this->Repeated && this->RetryOnTimeoutFailure)
        {
            this->Kill();
            this->StartTime = QDateTime::currentDateTime();
            this->Repeated = true;
            this->Process();
            return false;
        }
        // query is timed out
        if (this->Result == NULL)
        {
            this->Result = new QueryResult();
        }
        this->Kill();
        this->Result->Failed = true;
        this->Result->ErrorMessage = "Timed out";
        this->Status = StatusInError;
        return true;
    }
    return false;
}

QString Query::QueryTypeToString()
{
    switch (this->Type)
    {
        case QueryNull:
            return "null";
        case QueryWl:
            return "Wl Query";
        case QueryApi:
            return "Api Query";
        case QueryRevert:
            return "Revert Query";
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
            if (this->Result != NULL)
            {
                if (this->Result->Failed && this->Result->ErrorMessage != "")
                {
                    return "In error: " + this->Result->ErrorMessage;
                }
            }
            return "InError";
    }
    return "Unknown";
}

void Query::ProcessCallback()
{
    if (this->callback != NULL)
    {
        this->RegisterConsumer("delegate");
        this->CallbackResult = this->callback(this);
    }
}

unsigned int Query::QueryID()
{
    return this->ID;
}

bool Query::Failed()
{
    if (this->Result != NULL)
    {
        if (this->Result->Failed)
        {
            return true;
        }
    }
    if (this->Status == Huggle::StatusInError)
    {
        return true;
    }
    return false;
}
