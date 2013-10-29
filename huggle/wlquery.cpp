//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "wlquery.h"
using namespace Huggle;

WLQuery::WLQuery()
{
    this->Result = NULL;
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
    QUrl url("http://huggle.wmflabs.org/data/wl.php?action=read&wp=" + Configuration::Project.WhiteList);
    QString params = "";
    if (Save)
    {
        url = QUrl("http://huggle.wmflabs.org/data/wl.php?action=save&wp=" + Configuration::Project.WhiteList);
        QString whitelist = "";
        int p = 0;
        Configuration::WhiteList.sort();
        while (p < Configuration::WhiteList.count())
        {
            if (Configuration::WhiteList.at(p) != "")
            {
                whitelist += Configuration::WhiteList.at(p) + "|";
            }
            p++;
        }
        if (whitelist.endsWith("|"))
        {
            whitelist = whitelist.mid(0, whitelist.length() - 1);
        }
        whitelist += "||EOW||";
        params = "wl=" + QUrl::toPercentEncoding(whitelist);
    }
    QNetworkRequest request(url);
    if (!Save)
    {
        this->r = Query::NetworkManager.get(request);
    } else
    {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        this->r = Query::NetworkManager.post(request, params.toUtf8());
    }
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
