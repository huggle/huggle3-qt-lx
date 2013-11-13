//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "webserverquery.hpp"

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
        this->Result = new QueryResult();
        this->Result->Failed = true;
        this->Result->ErrorMessage = "You provided invalid url";
        this->Status = StatusInError;
        return;
    }
    this->StartTime = QDateTime::currentDateTime();
    this->Status = StatusProcessing;
    this->Result = new QueryResult();

    QUrl url = QUrl::fromEncoded(this->URL.toUtf8());
    QNetworkRequest request(url);
    if (UsingPOST)
    {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    }
    if (UsingPOST)
    {
        //this->reply = Query::NetworkManager.post(request, url.encodedQuery());
        this->reply = Query::NetworkManager.post(request, this->Parameters.toUtf8());
    } else
    {
        this->reply = Query::NetworkManager.get(request);
    }
    QObject::connect(this->reply, SIGNAL(finished()), this, SLOT(Finished()));
    QObject::connect(this->reply, SIGNAL(readyRead()), this, SLOT(ReadData()));
    Core::HuggleCore->DebugLog("Processing webserver request " + this->URL, 2);
}

void WebserverQuery::Kill()
{

}

void WebserverQuery::ReadData()
{

}

void WebserverQuery::Finished()
{

}
