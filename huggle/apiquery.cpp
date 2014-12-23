//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "apiquery.hpp"
#include <QFile>
#include <QUrl>
#include "apiqueryresult.hpp"
#include "configuration.hpp"
#include "syslog.hpp"
#include "revertquery.hpp"
#include "exception.hpp"
#include "generic.hpp"
#include "wikisite.hpp"

using namespace Huggle;

void ApiQuery::ConstructUrl()
{
    if (this->ActionPart.isEmpty())
        throw new Huggle::Exception("No action provided for api request", BOOST_CURRENT_FUNCTION);
    if (this->OverrideWiki.isEmpty())
    {
        this->URL = Configuration::GetProjectScriptURL(this->GetSite()) + "api.php?action=" + this->ActionPart;
    } else
    {
        this->URL = Configuration::GetURLProtocolPrefix(this->GetSite()) + this->OverrideWiki + "api.php?action=" + this->ActionPart;
    }
    if (this->Parameters.length() > 0)
        this->URL += "&" + this->Parameters;
    if (this->IsContinuous)
        this->URL += "&rawcontinue=1";
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
    this->URL += this->GetAssertPartSuffix();
}

QString ApiQuery::ConstructParameterLessUrl()
{
    QString url;
    if (this->ActionPart.isEmpty())
    {
        throw new Huggle::Exception("No action provided for api request", BOOST_CURRENT_FUNCTION);
    }
    if (!this->OverrideWiki.size())
        url = Configuration::GetProjectScriptURL(this->GetSite()) + "api.php?action=" + this->ActionPart;
    else
        url = Configuration::GetURLProtocolPrefix(this->GetSite()) + this->OverrideWiki + "api.php?action=" + this->ActionPart;
    if (this->IsContinuous)
        url += "&rawcontinue=1";
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
    return url + this->GetAssertPartSuffix();
}

QString ApiQuery::GetAssertPartSuffix()
{
    if (this->EnforceLogin && !Configuration::HuggleConfiguration->Restricted)
    {
        // we need to use this so that mediawiki will fail if we aren't logged in
        return "&assert=user";
    }
    return "";
}

// TODO: move this function to RevertQuery
void ApiQuery::FinishRollback()
{
    bool Rollback_Failed;
    this->CustomStatus = RevertQuery::GetCustomRevertStatus(this->Result, this->GetSite(), &Rollback_Failed);
    if (Rollback_Failed)
    {
        this->Result->SetError();
        this->ProcessFailure();
    }
}

ApiQuery::ApiQuery()
{
    this->RequestFormat = XML;
    this->Type = QueryApi;
}

ApiQuery::ApiQuery(Action action)
{
    this->RequestFormat = XML;
    this->Type = QueryApi;
    this->SetAction(action);
}

ApiQuery::ApiQuery(Action action, WikiSite *site)
{
    this->RequestFormat = XML;
    this->Site = site;
    this->Type = QueryApi;
    this->SetAction(action);
}

ApiQuery::~ApiQuery()
{
    this->Kill();
}

Action ApiQuery::GetAction()
{
    return this->_action;
}

ApiQueryResult *ApiQuery::GetApiQueryResult()
{
    return (ApiQueryResult*)this->Result;
}

static void WriteFile(QString text)
{
    QFile *file = new QFile(hcfg->QueryDebugPath);
    if (file->open(QIODevice::Append))
    {
        file->write(QString(text + "\n").toUtf8());
        file->close();
    }
    delete file;
}

static void WriteIn(ApiQuery *q)
{
    if (hcfg->QueryDebugging)
        WriteFile(QString::number(q->QueryID()) + " IN " + q->Result->Data);
}

static void WriteOut(ApiQuery *q)
{
    if (!hcfg->QueryDebugging)
        return;
    if (q->HiddenQuery)
        WriteFile(QString::number(q->QueryID()) + " OUT secret");
    else if (q->UsingPOST)
        WriteFile(QString::number(q->QueryID()) + " OUT " + q->URL + " " + q->Parameters);
    else
        WriteFile(QString::number(q->QueryID()) + " OUT " + q->URL);
}

void ApiQuery::Finished()
{
    // don't even try to do anything if query was killed
    if (this->Status == StatusKilled)
        return;
    if (this->Result == nullptr)
        throw new Huggle::NullPointerException("loc ApiQuery::Result", BOOST_CURRENT_FUNCTION);
    if (this->reply == nullptr)
        throw new Huggle::NullPointerException("loc ApiQuery::reply", BOOST_CURRENT_FUNCTION);
    ApiQueryResult *result = (ApiQueryResult*)this->Result;
    result->Data += QString(this->reply->readAll());
    // now we need to check if request was successful or not
    if (this->reply->error())
    {
        this->Result->SetError(HUGGLE_EUNKNOWN, this->reply->errorString());
        this->reply->deleteLater();
        this->reply = nullptr;
        this->Status = StatusDone;
        this->ProcessFailure();
        return;
    }
    if (this->ActionPart == "rollback")
        FinishRollback();
    this->reply->deleteLater();
    this->reply = nullptr;
    if (!this->HiddenQuery)
        HUGGLE_DEBUG("Finished request " + this->URL, 6);
    if (!result->IsFailed() && this->RequestFormat == XML)
        result->Process();
    this->Status = StatusDone;
    this->ProcessCallback();
    WriteIn(this);
}

void ApiQuery::Process()
{
    if (this->Status != Huggle::StatusNull)
    {
        HUGGLE_DEBUG1("Cowardly refusing to double process the query");
        return;
    }
    this->StartTime = QDateTime::currentDateTime();
    if (!this->URL.size())
        this->ConstructUrl();
    this->Status = StatusProcessing;
    this->Result = new ApiQueryResult();
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
    request.setRawHeader("User-Agent", Configuration::HuggleConfiguration->WebqueryAgent);
    if (this->UsingPOST)
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    if (Configuration::HuggleConfiguration->SystemConfig_DryMode && this->EditingQuery)
    {
        this->Result->Data = "DM (didn't run a query)";
        this->Status = StatusDone;
        this->ProcessCallback();
        Syslog::HuggleLogs->Log("If I wasn't in dry mode I would execute this query (post=" + Generic::Bool2String(this->UsingPOST) +
                                ") " + this->URL + "\ndata: " + QUrl::fromPercentEncoding(this->Parameters.toUtf8()));
        return;
    }
    WriteOut(this);
    if (this->UsingPOST)
    {
        this->reply = Query::NetworkManager->post(request, this->Parameters.toUtf8());
    } else
    {
        this->reply = Query::NetworkManager->get(request);
    }
    if (!this->HiddenQuery)
        HUGGLE_DEBUG("Processing api request " + this->URL, 6);
    QObject::connect(this->reply, SIGNAL(finished()), this, SLOT(Finished()));
    QObject::connect(this->reply, SIGNAL(readyRead()), this, SLOT(ReadData()));
}

void ApiQuery::ReadData()
{
    // don't even try to do anything if query was killed
    if (this->Status == StatusKilled)
        return;
    if (this->Result == nullptr)
        throw new Huggle::NullPointerException("loc ApiQuery::Result", BOOST_CURRENT_FUNCTION);
    if (this->reply == nullptr)
        throw new Huggle::NullPointerException("loc ApiQuery::reply", BOOST_CURRENT_FUNCTION);
    this->Result->Data += QString(this->reply->readAll());
}

void ApiQuery::SetAction(const Action action)
{
    this->_action = action;
    switch (action)
    {
        case ActionClearHasMsg:
            this->ActionPart = "clearhasmsg";
            return;
        case ActionQuery:
            this->ActionPart = "query";
            this->IsContinuous = true;
            this->EnforceLogin = false;
            return;
        case ActionLogin:
            this->ActionPart = "login";
            this->EnforceLogin = false;
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
        case ActionUnwatch:
        case ActionWatch:
            this->ActionPart = "watch";
            this->EditingQuery = true;
            return;
    }
}

QString ApiQuery::DebugURL()
{
    if (this->HiddenQuery)
        return "Protected link";
    if (this->UsingPOST)
        return this->URL + " POST: " + this->Parameters;
    return this->URL;
}
