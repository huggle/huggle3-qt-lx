//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "apiquery.hpp"

using namespace Huggle;

void ApiQuery::ConstructUrl()
{
    if (this->ActionPart == "")
    {
        throw new Exception("No action provided for api request");
    }

    if (OverrideWiki == "")
    {
        URL = Core::GetProjectScriptURL(Configuration::HuggleConfiguration->Project) + "api.php?action=" + this->ActionPart;
    }
    else
    {
        URL = Configuration::GetURLProtocolPrefix() + OverrideWiki + "api.php?action=" + this->ActionPart;
    }

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

QString ApiQuery::ConstructParameterLessUrl()
{
    QString url;
    if (this->ActionPart == "")
    {
        throw new Exception("No action provided for api request", "void ApiQuery::ConstructParameterLessUrl()");
    }
    if (OverrideWiki == "")
    {
        url = Core::GetProjectScriptURL(Configuration::HuggleConfiguration->Project) + "api.php?action=" + this->ActionPart;
    }
    else
    {
        url = Configuration::GetURLProtocolPrefix() + OverrideWiki + "api.php?action=" + this->ActionPart;
    }

    switch (this->RequestFormat)
    {
        case XML:
            url += "&format=xml";
            break;
        case JSON:
            url += "&format=json";
            break;
        case PlainText:
        case Default:
            break;
    }

    return url;
}

bool ApiQuery::FormatIsCurrentlySupported()
{
    // other formats will be supported later
    return (this->RequestFormat == XML);
}

// TODO: move this function to RevertQuery
void ApiQuery::FinishRollback()
{
    this->CustomStatus = RevertQuery::GetCustomRevertStatus(this->Result->Data);
    if (this->CustomStatus != "Reverted")
    {
        this->Result->Failed = true;
    }
}

ApiQuery::ApiQuery()
{
    this->RequestFormat = XML;
    this->URL = "";
    this->Type = QueryApi;
    this->ActionPart = "";
    this->Result = NULL;
    this->Parameters = "";
    this->UsingPOST = false;
    this->Target = "none";
    this->OverrideWiki = "";
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
        this->Status = StatusDone;
        return;
    }
    if (this->ActionPart == "rollback")
    {
        FinishRollback();
    }
    this->reply->deleteLater();
    this->reply = NULL;
    if (!this->HiddenQuery)
    {
        Huggle::Syslog::HuggleLogs->DebugLog("Finished request " + URL, 6);
    }
    this->Status = StatusDone;
    this->ProcessCallback();
}

void ApiQuery::Process()
{
    if (this->Status == StatusProcessing)
    {
        Huggle::Syslog::HuggleLogs->DebugLog("Cowardly refusing to double process the query");
        return;
    }
    this->StartTime = QDateTime::currentDateTime();
    if (this->URL == "")
    {
        this->ConstructUrl();
    }
    this->Status = StatusProcessing;
    this->Result = new QueryResult();
    QUrl url;
    if (this->UsingPOST)
    {
        url = QUrl::fromEncoded(this->ConstructParameterLessUrl().toUtf8());
    } else
    {
        url = QUrl::fromEncoded(this->URL.toUtf8());
    }
    QNetworkRequest request(url);
    if (this->UsingPOST)
    {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    }
    if (this->UsingPOST)
    {
        this->reply = Query::NetworkManager->post(request, this->Parameters.toUtf8());
    } else
    {
        this->reply = Query::NetworkManager->get(request);
    }
    if (!this->HiddenQuery)
    {
        Huggle::Syslog::HuggleLogs->DebugLog("Processing api request " + this->URL, 6);
    }
    QObject::connect(this->reply, SIGNAL(finished()), this, SLOT(Finished()));
    QObject::connect(this->reply, SIGNAL(readyRead()), this, SLOT(ReadData()));
}

void ApiQuery::ReadData()
{
    this->Result->Data += QString(this->reply->readAll());
}

void ApiQuery::SetAction(const Action action)
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
        case ActionBlock:
            this->ActionPart = "block";
            return;
        case ActionProtect:
            this->ActionPart = "protect";
            return;
        case ActionEdit:
            this->ActionPart = "edit";
            return;
        case ActionPatrol:
            this->ActionPart = "patrol";
            return;
    }
}

void ApiQuery::SetAction(const QString action)
{
    this->ActionPart = action;
}

void ApiQuery::Kill()
{
    if (this->reply != NULL)
    {
        this->reply->abort();
    }
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
