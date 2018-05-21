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
#include "gc.hpp"
#include "syslog.hpp"

using namespace Huggle;

unsigned long Query::bytesReceived = 0;
unsigned long Query::bytesSent =     0;
QList<Collectable_SmartPtr<Query>> Query::PendingRestart;
unsigned int Query::lastID = 0;
QNetworkAccessManager *Query::NetworkManager = nullptr;

unsigned long Query::GetBytesReceivedSinceStartup()
{
    return Query::bytesReceived;
}

unsigned long Query::GetBytesSentSinceStartup()
{
    return Query::bytesSent;
}

Query::Query()
{
    this->Type = QueryNull;
    this->status = StatusNull;
    this->ID = this->lastID;
    this->lastID++;
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
    if (this->status == StatusNull || this->status == StatusIsSuspended)
        return false;
    if (this->status == StatusDone || this->status == StatusInError || this->status == StatusKilled)
        return true;
    if (QDateTime::currentDateTime() > this->StartTime.addSecs(this->Timeout))
    {
        if (!this->isRepeated && this->RetryOnTimeoutFailure)
        {
            this->Kill();
            delete this->Result;
            this->Result = nullptr;
            this->StartTime = QDateTime::currentDateTime();
            this->isRepeated = true;
            this->Process();
            return false;
        }
        // query is timed out
        if (this->Result == nullptr)
            this->Result = new QueryResult();

        this->Kill();
        this->Result->SetError("Timed out");
        this->status = StatusInError;
        this->processFailure();
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
        case QueryWebServer:
            return "HTTP Query";
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

    switch (this->status)
    {
        case StatusNull:
            return "Waiting";
        case StatusDone:
            return "Done";
        case StatusProcessing:
            return "Processing";
        case StatusKilled:
            return "Killed";
        case StatusIsSuspended:
            return "Suspended";
        case StatusInError:
            if (this->Result != nullptr && this->Result->IsFailed() && !this->Result->ErrorMessage.isEmpty())
                return "In error: " + this->Result->ErrorMessage;

            return "InError";
    }
    return "Unknown";
}

void Query::processCallback()
{
    this->finishedTime = QDateTime::currentDateTime();
    if (this->SuccessCallback != nullptr)
    {
        this->RegisterConsumer(HUGGLECONSUMER_CALLBACK);
        this->SuccessCallback(this);
    }
}

void Query::processFailure()
{
    this->finishedTime = QDateTime::currentDateTime();
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

    if (this->status == Query::StatusInError || this->status == Query::StatusKilled)
        return true;

    return false;
}

QString Query::GetFailureReason()
{
    if (this->status == Query::StatusKilled)
        return "Query was killed";

    if (this->Result != nullptr)
        return this->Result->ErrorMessage;

    if (this->failureReason.isEmpty())
        return "Unknown";

    return this->failureReason;
}

QString Query::DebugURL()
{
    return "null";
}

void Query::ThrowOnValidResult()
{
    if (!this->Result)
        return;

    this->status = StatusInError;
    throw new Huggle::Exception("Result was not NULL memory would leak: 0x" + QString::number((uintptr_t)this->Result, 16), BOOST_CURRENT_FUNCTION);
}

void Query::Restart()
{
    if (this->status == StatusProcessing)
        this->Kill();

    delete this->Result;
    this->Result = nullptr;
    this->status = StatusNull;
    this->isRepeated = false;
    this->failureReason = "";
    this->Process();
}

void Query::Suspend(bool enqueue)
{
    this->status = StatusIsSuspended;
    Collectable_SmartPtr<Query> query = this;
    if (enqueue)
        Query::PendingRestart.append(query);
}

qint64 Query::ExecutionTime()
{
    if (!this->IsProcessed())
        return 0;

    return this->StartTime.msecsTo(this->finishedTime);
}

void Query::SetStatus(Query::Status state)
{
    this->status = state;
}
