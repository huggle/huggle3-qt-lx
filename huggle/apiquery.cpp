//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "apiquery.hpp"
#include <QtXml/QtXml>
#include <QUrl>
#include "syslog.hpp"
#include "exception.hpp"
#include "configuration.hpp"

using namespace Huggle;

void ApiQuery::ConstructUrl()
{
    if (this->ActionPart.isEmpty())
        throw new Huggle::Exception("No action provided for api request");
    if (this->OverrideWiki.isEmpty())
    {
        this->URL = Configuration::GetProjectScriptURL(Configuration::HuggleConfiguration->Project)
                    + "api.php?action=" + this->ActionPart;
    } else
    {
        this->URL = Configuration::GetURLProtocolPrefix() + this->OverrideWiki + "api.php?action=" + this->ActionPart;
    }
    if (this->Parameters.length() > 0)
        this->URL += "&" + this->Parameters;
    switch (this->RequestFormat)
    {
        case XML:
            this->URL += "&format=xml";
            break;
        case JSON:
            this->URL += "&format=json";
            break;
        case PlainText:
        case Default:
            break;
    }
}

QString ApiQuery::ConstructParameterLessUrl()
{
    QString url;
    if (!this->ActionPart.length())
    {
        throw new Huggle::Exception("No action provided for api request", "void ApiQuery::ConstructParameterLessUrl()");
    }
    if (!this->OverrideWiki.size())
        url = Configuration::GetProjectScriptURL(Configuration::HuggleConfiguration->Project)
                + "api.php?action=" + this->ActionPart;
    else
        url = Configuration::GetURLProtocolPrefix() + this->OverrideWiki + "api.php?action=" + this->ActionPart;

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

// TODO: move this function to RevertQuery
void ApiQuery::FinishRollback()
{
    this->CustomStatus = RevertQuery::GetCustomRevertStatus(this->Result->Data);
    if (this->CustomStatus != "Reverted")
        this->Result->Failed = true;
}

ApiQuery::ApiQuery()
{
    this->RequestFormat = XML;
    this->Type = QueryApi;
}

ApiQuery::ApiQuery(Action a)
{
    this->RequestFormat = XML;
    this->Type = QueryApi;
    this->SetAction(a);
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
        this->reply = nullptr;
        this->Status = StatusDone;
        return;
    }
    if (this->ActionPart == "rollback")
    {
        FinishRollback();
    }
    this->reply->deleteLater();
    this->reply = nullptr;
    if (!this->HiddenQuery)
        Huggle::Syslog::HuggleLogs->DebugLog("Finished request " + this->URL, 6);
    this->Status = StatusDone;
    this->ProcessCallback();
}

void ApiQuery::Process()
{
    if (this->Status != Huggle::StatusNull)
    {
        Huggle::Syslog::HuggleLogs->DebugLog("Cowardly refusing to double process the query");
        return;
    }
    this->StartTime = QDateTime::currentDateTime();
    if (!this->URL.size())
        this->ConstructUrl();
    this->Status = StatusProcessing;
    this->Result = new QueryResult();
    QUrl url;
    if (this->UsingPOST)
    {
        this->URL = this->ConstructParameterLessUrl();
        url = QUrl::fromEncoded(this->URL.toUtf8());
    } else
    {
        url = QUrl::fromEncoded(this->URL.toUtf8());
    }
    QNetworkRequest request(url);
    if (this->UsingPOST)
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    if (Configuration::HuggleConfiguration->SystemConfig_DryMode && this->EditingQuery)
    {
        this->Result = new QueryResult();
        this->Result->Data = "DM";
        this->Status = StatusDone;
        Syslog::HuggleLogs->Log("If I wasn't in dry mode I would execute this query (post=" + Configuration::Bool2String(this->UsingPOST) +
                                ") " + this->URL + "\ndata: " + this->Parameters);
        return;
    }
    if (this->UsingPOST)
    {
        this->reply = Query::NetworkManager->post(request, this->Parameters.toUtf8());
    } else
    {
        this->reply = Query::NetworkManager->get(request);
    }
    if (!this->HiddenQuery)
        Huggle::Syslog::HuggleLogs->DebugLog("Processing api request " + this->URL, 6);
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
            this->EditingQuery = true;
            return;
        case ActionDelete:
            this->ActionPart = "delete";
            this->EditingQuery = true;
            return;
        case ActionUndelete:
            this->ActionPart = "undelete";
            this->EditingQuery = true;
            return;
        case ActionBlock:
            this->ActionPart = "block";
            this->EditingQuery = true;
            return;
        case ActionProtect:
            this->ActionPart = "protect";
            this->EditingQuery = true;
            return;
        case ActionEdit:
            this->ActionPart = "edit";
            this->EditingQuery = true;
            return;
        case ActionPatrol:
            this->ActionPart = "patrol";
            this->EditingQuery = true;
            return;
        case ActionReview: // FlaggedRevs
            this->ActionPart = "review";
            this->EditingQuery = true;
            return;
    }
}
