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
#include <QtNetwork>
#include <QUrl>
#include "apiqueryresult.hpp"
#include "configuration.hpp"
#include "syslog.hpp"
#include "revertquery.hpp"
#include "exception.hpp"
#include "localization.hpp"
#include "generic.hpp"
#include "wikisite.hpp"

using namespace Huggle;

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

QString ApiQuery::GetFailureReason()
{
    if (this->FailureReason.isEmpty() && this->Result && this->Result->Data.isEmpty())
        return _l("query-result-nodata");
    return Query::GetFailureReason();
}

void ApiQuery::ConstructUrl()
{
    if (this->ActionPart.isEmpty())
        throw new Huggle::Exception(_l("query-request-noaction"), BOOST_CURRENT_FUNCTION);
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
        throw new Huggle::Exception(_l("query-request-noaction"), BOOST_CURRENT_FUNCTION);
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
    bool Rollback_Failed, Require_Suspend;
    this->CustomStatus = RevertQuery::GetCustomRevertStatus(this->Result, this->GetSite(), &Rollback_Failed, &Require_Suspend);
    if (Require_Suspend)
    {
        this->Suspend();
        return;
    }
    if (Rollback_Failed)
    {
        this->Result->SetError();
        this->ProcessFailure();
    }
}

ApiQueryResult *ApiQuery::GetApiQueryResult()
{
    return (ApiQueryResult*)this->Result;
}

void ApiQuery::SetCustomActionPart(QString action, bool editing, bool enforce_login, bool is_continuous)
{
    this->SetAction(ActionCustom);
    this->ActionPart = action;
    this->EnforceLogin = enforce_login;
    this->IsContinuous = is_continuous;
    this->EditingQuery = editing;
}

void ApiQuery::SetToken(Token token, QString name, QString value)
{
    if (value.isEmpty())
    {
        switch (token)
        {
            case TokenLogin:
                throw new Exception(_l("query-login-undefined"), BOOST_CURRENT_FUNCTION);
            case TokenWatch:
                value = this->GetSite()->GetProjectConfig()->Token_Watch;
                break;
            case TokenCsrf:
                value = this->GetSite()->GetProjectConfig()->Token_Csrf;
                break;
            case TokenPatrol:
                value = this->GetSite()->GetProjectConfig()->Token_Patrol;
                break;
            case TokenRollback:
                value = this->GetSite()->GetProjectConfig()->Token_Rollback;
                break;
        }
    }

    if (name.isEmpty())
    {
        switch (token)
        {
            case TokenLogin:
                name = "ltoken";
                break;
            case TokenRollback:
                name = "rltoken";
                break;
            case TokenWatch:
            case TokenCsrf:
                name = "token";
                break;
            case TokenPatrol:
                name = "ptoken";
                break;
        }
    }

    this->SetParam(name, value);
}

void ApiQuery::SetParam(QString name, QString value)
{
    if (this->params.contains(name))
        this->params[name] = value;
    else
        this->params.insert(name, value);
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

static void WriteIn(ApiQuery *q, QNetworkReply *reply)
{
    if (hcfg->QueryDebugging)
    {
        QString header_list;
        QList<QByteArray> headerList = reply->rawHeaderList();
        foreach(QByteArray head, headerList)
            header_list += head + ": " + reply->rawHeader(head) + "\n";
        WriteFile("======================================\n" + QString::number(q->QueryID()) + " IN " + QDateTime::currentDateTime().toString() + "\n======================================\nHEADERS:\n" +
            header_list + "\n\nDATA:\n" + q->Result->Data);
    }
}

static void WriteOut(ApiQuery *q, QNetworkRequest *request)
{
    if (!hcfg->QueryDebugging)
        return;
    QString id = "======================================\n" + QString::number(q->QueryID()) + " OUT " + QDateTime::currentDateTime().toString() + "\n======================================\n";
    QString header_list;
    QList<QByteArray> headerList = request->rawHeaderList();
    foreach(QByteArray head, headerList)
        header_list += head + ": " + request->rawHeader(head) + "\n";
    if (q->HiddenQuery)
        WriteFile(id + "(secret url) HEADERS:\n" + header_list + "\n\nDATA: (secret data)");
    else if (q->UsingPOST)
        WriteFile(id + q->URL + " POST/PARAMETERS: " + q->Parameters + "\nHEADERS:\n" + header_list);
    else
        WriteFile(id + q->URL + "\nHEADERS:\n" + header_list);
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
    this->temp += this->reply->readAll();
    result->Data = QString(this->temp);
    // remove the temporary data so that we save the ram
    this->temp.clear();
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
    //! \todo This bellow needs to be fixed, rollback handling doesn't belong here
    // BEGINING OF SHIT
    if (this->ActionPart == "rollback")
        FinishRollback();
    if (this->Status == StatusIsSuspended)
        return;
    // END OF SHIT
    if (!this->HiddenQuery)
        HUGGLE_DEBUG("Finished request " + this->URL, 6);
    WriteIn(this, this->reply);
    this->reply->deleteLater();
    this->reply = nullptr;
    if (result->Data.isEmpty() || result->IsFailed())
    {
        this->Status = StatusInError;
        this->ProcessFailure();
        return;
    }
    if (this->RequestFormat == XML)
        result->Process();
    this->Status = StatusDone;
    this->ProcessCallback();
}

void ApiQuery::Process()
{
    if (this->Status != Huggle::StatusNull)
    {
        HUGGLE_DEBUG1("Cowardly refusing to double process the query");
        return;
    }
    this->StartTime = QDateTime::currentDateTime();
    this->ThrowOnValidResult();
    this->Result = new ApiQueryResult();

    // Cancel if target project is read only and this API query edits wiki somehow
    if (this->EditingQuery && this->GetSite()->GetProjectConfig()->ReadOnly)
    {
        this->FailureReason = "Unable to edit read-only wiki";
        this->Result->SetError(HUGGLE_EREADONLY, this->FailureReason);
        this->Status = StatusInError;
        this->ProcessFailure();
        return;
    }

    this->temp.clear();
    foreach(QString value, this->params.values())
        this->Parameters += "&" + value + "=" + QUrl::toPercentEncoding(this->params[value]);
    if (this->Parameters.startsWith("&"))
    {
        // remove the trailing symbol
        this->Parameters = this->Parameters.mid(1);
    }
    if (!this->URL.size() && !this->UsingPOST)
        this->ConstructUrl();
    this->Status = StatusProcessing;
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
    WriteOut(this, &request);
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

void ApiQuery::Kill()
{
    if (this->reply != nullptr)
    {
        QObject::disconnect(this->reply, SIGNAL(finished()), this, SLOT(Finished()));
        QObject::disconnect(this->reply, SIGNAL(readyRead()), this, SLOT(ReadData()));
        if (this->Status == StatusProcessing)
        {
            this->Status = StatusKilled;
            this->disconnect(this->reply);
            this->reply->abort();
            this->reply->disconnect(this);
            this->reply->deleteLater();
            this->reply = nullptr;
        }
    }
}

QString ApiQuery::GetURL()
{
    this->ConstructUrl();
    return this->URL;
}

void ApiQuery::ReadData()
{
    // don't even try to do anything if query was killed
    if (this->Status == StatusKilled)
        return;
    if (this->reply == nullptr)
        throw new Huggle::NullPointerException("loc ApiQuery::reply", BOOST_CURRENT_FUNCTION);
    this->temp += this->reply->readAll();
}

void ApiQuery::SetAction(const Action action)
{
    this->_action = action;
    switch (action)
    {
        case ActionClearHasMsg:
            this->ActionPart = "clearhasmsg";
            this->UsingPOST = true;
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
        //case ActionTokens:
        //    this->ActionPart = "tokens";
        //    return;
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
        case ActionCustom:
            return;
    }
}

QString ApiQuery::DebugURL()
{
    if (this->HiddenQuery)
        return _l("query-protected-link");
    if (this->UsingPOST)
        return this->URL + " POST: " + this->Parameters;
    return this->URL;
}
