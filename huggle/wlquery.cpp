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

WLQuery::WLQuery()
{
    this->Type = WLQueryType_ReadWL;
    this->Result = NULL;
    this->Parameters = "";
    this->Progress = 0;
}

WLQuery::~WLQuery()
{
    delete Result;
    this->Result = NULL;
}

QString WLQuery::QueryTypeToString()
{
    return "Whitelist";
}

QString WLQuery::QueryTargetToString()
{
    if (this->Type != WLQueryType_SuspWL)
        return "Writing users to WhiteList";
    else
        return "Reporting suspicious edit";
}

void WLQuery::Process()
{
    if (!Configuration::HuggleConfiguration->GlobalConfig_Whitelist.size())
    {
        // there is no whitelist in config for this wiki
        Syslog::HuggleLogs->ErrorLog("Unable to process WL request, there is no whitelist server defined");
        this->Result = new QueryResult();
        this->Result->SetError("Invalid URL");
        this->Status = Huggle::StatusInError;
        return;
    }
    this->StartTime = QDateTime::currentDateTime();
    this->Status = StatusProcessing;
    this->Result = new QueryResult();
    QUrl url(Configuration::HuggleConfiguration->GlobalConfig_Whitelist
             + "?action=read&wp="
             + Configuration::HuggleConfiguration->Project->WhiteList);
    switch (this->Type)
    {
        case WLQueryType_ReadWL:
            break;
        case WLQueryType_SuspWL:
            url = QUrl(Configuration::HuggleConfiguration->GlobalConfig_Whitelist +
                       "susp.php?action=insert&" + this->Parameters);
            break;
        case WLQueryType_WriteWL:
            url = QUrl(Configuration::HuggleConfiguration->GlobalConfig_Whitelist + "?action=save&user=" +
                      QUrl::toPercentEncoding("huggle_" + Configuration::HuggleConfiguration->SystemConfig_Username) +
                      "&wp=" + Configuration::HuggleConfiguration->Project->WhiteList);
            break;
    }
    QString params = "";
    QByteArray data;
    if (this->Type == WLQueryType_WriteWL)
    {
        QString whitelist = "";
        int p = 0;
        while (p < Configuration::HuggleConfiguration->NewWhitelist.count())
        {
            if (Configuration::HuggleConfiguration->NewWhitelist.at(p) != "")
            {
                whitelist += Configuration::HuggleConfiguration->NewWhitelist.at(p) + "|";
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
        long size = (long)data.size();
        Syslog::HuggleLogs->DebugLog("Sending whitelist data of size: " + QString::number(size) + " byte");
    }
    QNetworkRequest request(url);
    if (this->Type == WLQueryType_ReadWL)
    {
        this->r = Query::NetworkManager->get(request);
    } else
    {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        this->r = Query::NetworkManager->post(request, data);
    }
    QObject::connect(this->r, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(WriteProgress(qint64,qint64)));
    QObject::connect(this->r, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(WriteProgress(qint64,qint64)));
    QObject::connect(this->r, SIGNAL(finished()), this, SLOT(Finished()));
    QObject::connect(this->r, SIGNAL(readyRead()), this, SLOT(ReadData()));
}

void WLQuery::ReadData()
{
    this->Result->Data += QString(this->r->readAll());
}

void WLQuery::Finished()
{
    this->Result->Data += QString(this->r->readAll());
    if (this->Type == WLQueryType_WriteWL)
    {
        Syslog::HuggleLogs->DebugLog(this->Result->Data, 2);
        if (!this->Result->Data.contains("written"))
            Syslog::HuggleLogs->ErrorLog("Failed to store data to white list: " + this->Result->Data);
    }
    // now we need to check if request was successful or not
    if (this->r->error())
    {
        this->Result->SetError(r->errorString());
    }
    this->r->deleteLater();
    this->r = nullptr;
    this->Status = StatusDone;
}

void WLQuery::WriteProgress(qint64 n, qint64 m)
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
