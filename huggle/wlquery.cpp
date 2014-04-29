//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "wlquery.hpp"
#include "configuration.hpp"
using namespace Huggle;

WLQuery::WLQuery()
{
    this->Result = NULL;
    this->Progress = 0;
    Save = false;
}

WLQuery::~WLQuery()
{
    delete Result;
    this->Result = NULL;
}

void WLQuery::Process()
{
    this->StartTime = QDateTime::currentDateTime();
    this->Status = StatusProcessing;
    this->Result = new QueryResult();
    QUrl url("http://huggle.wmflabs.org/data/wl.php?action=read&wp=" + Configuration::HuggleConfiguration->Project->WhiteList);
    QString params = "";
    QByteArray data;
    if (Save)
    {
        url = QUrl("http://huggle.wmflabs.org/data/wl.php?action=save&wp=" + Configuration::HuggleConfiguration->Project->WhiteList);
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
    if (!Save)
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
    Syslog::HuggleLogs->DebugLog(this->Result->Data, 2);
    // now we need to check if request was successful or not
    if (this->r->error())
    {
        this->Result->ErrorMessage = r->errorString();
        this->Result->Failed = true;
        this->r->deleteLater();
        this->r = NULL;
        return;
    }
    this->r->deleteLater();
    this->r = NULL;
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
