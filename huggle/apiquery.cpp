//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "apiquery.h"

void ApiQuery::ConstructUrl()
{
    if (this->ActionPart == "")
    {
        throw new Exception("No action provided for api request");
    }
    URL = Core::GetProjectScriptURL(Configuration::Project) + "api.php?action=" + this->ActionPart;
    if (this->Parameters != "")
    {
        URL = URL + "&" + this->Parameters;
    }

    switch (this->RequestFormat)
    {
    case XML:
        URL += "&format=xml";
        break;
    case JSON:
        URL += "&format=json";
        break;
    case PlainText:
    case Default:
        break;
    }
}

bool ApiQuery::FormatIsCurrentlySupported()
{
    // other formats will be supported later
    return (this->RequestFormat == XML);
}

ApiQuery::ApiQuery()
{
    this->RequestFormat = XML;
    this->URL = "";
    this->Type = QueryApi;
    this->ActionPart = "";
    this->Parameters = "";
    this->UsingPOST = false;
    this->Target = "none";
}

void ApiQuery::Finished()
{
    this->Result->Data += QString(this->reply->readAll());
    // now we need to check if request was successful or not
    if (this->reply->error())
    {
        this->Result->ErrorMessage = reply->errorString();
        this->Result->Failed = true;
        this->reply->deleteLater();
        this->reply = NULL;
        return;
    }
    this->reply->deleteLater();
    this->reply = NULL;
    Core::DebugLog("Finished request " + URL, 2);
    this->Status = Done;
}

void ApiQuery::Process()
{
    if (this->URL == "")
    {
        this->ConstructUrl();
    }
    this->Status = Processing;
    this->Result = new QueryResult();
    QUrl url(this->URL);
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
    Core::DebugLog("Processing api request " + this->URL, 2);
}

void ApiQuery::ReadData()
{
    this->Result->Data += QString(this->reply->readAll());
}

void ApiQuery::SetAction(Action action)
{
    switch (action)
    {
    case ActionQuery:
        this->ActionPart = "query";
        return;
    case ActionLogin:
        this->ActionPart = "login";
        return;
    case ActionLogout:
        this->ActionPart = "logout";
        return;
    case ActionTokens:
        this->ActionPart = "tokens";
        return;
    case ActionPurge:
        this->ActionPart = "purge";
        return;
    case ActionRollback:
        this->ActionPart = "rollback";
        return;
    case ActionDelete:
        this->ActionPart = "delete";
        return;
    case ActionUndelete:
        this->ActionPart = "undelete";
        return;
    case ActionEdit:
        this->ActionPart = "edit";
    }
}

void ApiQuery::SetAction(QString action)
{
    this->ActionPart = action;
}

void ApiQuery::Kill()
{
    reply->abort();
}

QString ApiQuery::QueryTargetToString()
{
    return this->Target;
}

QString ApiQuery::QueryTypeToString()
{
    return "ApiQuery (" +
            this->ActionPart + ")";
}
