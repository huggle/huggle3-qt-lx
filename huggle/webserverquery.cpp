//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "webserverquery.hpp"
#include <QtNetwork>
#include <QUrl>
#include <QtXml>
#include "syslog.hpp"

using namespace Huggle;

WebserverQuery::WebserverQuery()
{
    this->URL = "";
    this->reply = NULL;
    this->Parameters = "";
    this->UsingPOST = false;
}

void WebserverQuery::Process()
{
    if (this->URL == "")
    {
        this->Result = new QueryResult(true);
        this->Result->SetError("You provided invalid url");
        this->Status = StatusInError;
        return;
    }
    this->StartTime = QDateTime::currentDateTime();
    this->Status = StatusProcessing;
    this->Result = new QueryResult();

    QUrl url = QUrl::fromEncoded(this->URL.toUtf8());
    QNetworkRequest request(url);
    if (this->UsingPOST)
    {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        this->reply = Query::NetworkManager->post(request, this->Parameters.toUtf8());
    } else
    {
        this->reply = Query::NetworkManager->get(request);
    }
    QObject::connect(this->reply, SIGNAL(finished()), this, SLOT(Finished()));
    QObject::connect(this->reply, SIGNAL(readyRead()), this, SLOT(ReadData()));
    Huggle::Syslog::HuggleLogs->DebugLog("Processing webserver request " + this->URL, 2);
}

void WebserverQuery::Kill()
{
    if (this->reply != NULL)
    {
        this->reply->abort();
    }
}

void WebserverQuery::ReadData()
{
    this->Result->Data += QString(this->reply->readAll());
}

void WebserverQuery::Finished()
{
    this->Result->Data += QString(this->reply->readAll());
    // now we need to check if request was successful or not
    if (this->reply->error())
    {
        this->Result->SetError(reply->errorString());
        this->reply->deleteLater();
        this->reply = NULL;
        this->Status = StatusDone;
        return;
    }
    this->reply->deleteLater();
    this->reply = NULL;
    if (!this->HiddenQuery)
    {
        Huggle::Syslog::HuggleLogs->DebugLog("Finished request " + URL, 2);
    }
    this->Status = StatusDone;
}
