//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "wlquery.hpp"
#include <QtNetwork>
#include <QUrl>
#include "configuration.hpp"
#include "syslog.hpp"
#include "wikisite.hpp"
using namespace Huggle;

WLQuery::WLQuery(WikiSite *site) : MediaWikiObject(site)
{
    this->Type = QueryType::QueryWl;
    this->WL_Type = WLQueryType_ReadWL;
    this->Result = nullptr;
    this->Parameters = "";
    this->networkReply = nullptr;
    this->Progress = 0;
}

WLQuery::~WLQuery()
{
    if (this->networkReply != nullptr)
    {
        QObject::disconnect(this->networkReply, SIGNAL(finished()), this, SLOT(finished()));
        QObject::disconnect(this->networkReply, SIGNAL(readyRead()), this, SLOT(readData()));
        this->networkReply->abort();
        this->networkReply->disconnect(this);
        this->networkReply->deleteLater();
        this->networkReply = nullptr;
    }
}

QString WLQuery::QueryTypeToString()
{
    return "Whitelist";
}

QString WLQuery::QueryTargetToString()
{
    if (this->WL_Type != WLQueryType_SuspWL)
        return "Writing users to WhiteList";
    else
        return "Reporting suspicious edit";
}

void WLQuery::Kill()
{
    if (this->networkReply != nullptr)
    {
        QObject::disconnect(this->networkReply, SIGNAL(finished()), this, SLOT(finished()));
        QObject::disconnect(this->networkReply, SIGNAL(readyRead()), this, SLOT(readData()));
        if (this->status == StatusProcessing)
        {
            this->status = StatusKilled;
            this->disconnect(this->networkReply);
            this->networkReply->abort();
            this->networkReply->disconnect(this);
            this->networkReply->deleteLater();
            this->networkReply = nullptr;
        }
    }
}

void WLQuery::Process()
{
    this->StartTime = QDateTime::currentDateTime();
    this->ThrowOnValidResult();
    if (Configuration::HuggleConfiguration->GlobalConfig_Whitelist.isEmpty())
    {
        // there is no whitelist in config for this wiki
        Syslog::HuggleLogs->ErrorLog("Unable to process WL request, there is no whitelist server defined");
        this->Result = new QueryResult();
        this->Result->SetError("Invalid URL");
        this->status = Query::StatusInError;
        return;
    }
    this->status = StatusProcessing;
    this->Result = new QueryResult();
    QUrl url(Configuration::HuggleConfiguration->GlobalConfig_Whitelist
             + "?action=read&wp=" + this->GetSite()->WhiteList);
    switch (this->WL_Type)
    {
        case WLQueryType_ReadWL:
            break;
        case WLQueryType_SuspWL:
            url = QUrl(Configuration::HuggleConfiguration->GlobalConfig_Whitelist +
                       "susp.php?action=insert&" + this->Parameters);
            break;
        case WLQueryType_WriteWL:
            url = QUrl(Configuration::HuggleConfiguration->GlobalConfig_Whitelist + "?action=save&user=" +
                      QUrl::toPercentEncoding("huggle_" + Configuration::HuggleConfiguration->SystemConfig_UserName) +
                      "&wp=" + this->GetSite()->WhiteList);
            break;
    }
    QString params = "";
    QByteArray data;
    if (this->WL_Type == WLQueryType_WriteWL)
    {
        QString whitelist = "";
        int p = 0;
        while (p < this->GetSite()->GetProjectConfig()->NewWhitelist.count())
        {
            if (!this->GetSite()->GetProjectConfig()->NewWhitelist.at(p).isEmpty())
            {
                whitelist += this->GetSite()->GetProjectConfig()->NewWhitelist.at(p) + "|";
            }
            p++;
        }
        if (whitelist.endsWith("|"))
        {
            whitelist = whitelist.mid(0, whitelist.length() - 1);
        }
        whitelist += "||EOW||";
        params = "wl=" + QUrl::toPercentEncoding(whitelist);
        data = params.toUtf8();
        long size = static_cast<long>(data.size());
        Syslog::HuggleLogs->DebugLog("Sending whitelist data of size: " + QString::number(size) + " byte to " + this->GetSite()->Name);
    }
    QNetworkRequest request(url);
    if (this->WL_Type == WLQueryType_ReadWL)
    {
        this->networkReply = Query::NetworkManager->get(request);
    } else
    {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        this->networkReply = Query::NetworkManager->post(request, data);
    }
    request.setRawHeader("User-Agent", Configuration::HuggleConfiguration->WebRequest_UserAgent);
    QObject::connect(this->networkReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(writeProgress(qint64,qint64)));
    QObject::connect(this->networkReply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(writeProgress(qint64,qint64)));
    QObject::connect(this->networkReply, SIGNAL(finished()), this, SLOT(finished()));
    QObject::connect(this->networkReply, SIGNAL(readyRead()), this, SLOT(readData()));
}

void WLQuery::readData()
{
    this->Result->Data += QString(this->networkReply->readAll());
}

void WLQuery::finished()
{
    this->Result->Data += QString(this->networkReply->readAll());
    if (this->WL_Type == WLQueryType_WriteWL)
    {
        Syslog::HuggleLogs->DebugLog(this->Result->Data, 2);
        if (!this->Result->Data.contains("written"))
            Syslog::HuggleLogs->ErrorLog("Failed to store data to white list: " + this->Result->Data);
    }
    if (this->WL_Type == WLQueryType_SuspWL)
        HUGGLE_DEBUG("Result of susp.php: " + this->Result->Data, 2);
    // now we need to check if request was successful or not
    if (this->networkReply->error())
    {
        this->Result->SetError(networkReply->errorString());
    }
    this->networkReply->deleteLater();
    this->networkReply = nullptr;
    this->status = StatusDone;
}

void WLQuery::writeProgress(qint64 n, qint64 m)
{
    if (m < 0 || n < 0)
    {
        // we don't know the target size
        return;
    }
    if (n == 0 || m == 0)
    {
        return;
    }
    this->Progress = (n / m) * 100;
}
