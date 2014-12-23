//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "query.hpp"
#include <QNetworkAccessManager>
#include "exception.hpp"
#include "syslog.hpp"
#include "gc.hpp"

using namespace Huggle;

unsigned int Query::LastID = 0;
QNetworkAccessManager *Query::NetworkManager = nullptr;

Query::Query()
{
    this->Type = QueryNull;
    this->Status = StatusNull;
    this->ID = this->LastID;
    this->LastID++;
    this->CustomStatus = "";
    this->HiddenQuery = false;
    this->Timeout = 60;
    this->StartTime = QDateTime::currentDateTime();
    this->RetryOnTimeoutFailure = true;
}

Query::~Query()
{
    delete this->Result;
    this->Result = nullptr;
}

bool Query::IsProcessed()
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
        if (this->Result == nullptr)
            this->Result = new QueryResult();

        this->Kill();
        this->Result->SetError("Timed out");
        this->Status = StatusInError;
        this->ProcessFailure();
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

QString Query::QueryStatusToString()
{
    if (this->CustomStatus.size())
        return CustomStatus;

    switch (this->Status)
    {
        case StatusNull:
            return "Waiting";
        case StatusDone:
            return "Done";
        case StatusProcessing:
            return "Processing";
        case StatusInError:
            if (this->Result != nullptr && this->Result->IsFailed() && !this->Result->ErrorMessage.isEmpty())
                return "In error: " + this->Result->ErrorMessage;

            return "InError";
    }
    return "Unknown";
}

void Query::ProcessCallback()
{
    if (this->callback != nullptr)
    {
        this->RegisterConsumer(HUGGLECONSUMER_CALLBACK);
        this->callback(this);
    }
}

void Query::ProcessFailure()
{
    if (this->FailureCallback != nullptr)
    {
        this->RegisterConsumer(HUGGLECONSUMER_CALLBACK);
        this->FailureCallback(this);
    }
}

bool Query::IsFailed()
{
    if (this->Result != nullptr && this->Result->IsFailed())
        return true;

    if (this->Status == Huggle::StatusInError)
        return true;

    return false;
}

QString Query::GetFailureReason()
{
    if (this->Result != nullptr)
        return this->Result->ErrorMessage;

    return this->FailureReason;
}

QString Query::DebugURL()
{
    return "null";
}
