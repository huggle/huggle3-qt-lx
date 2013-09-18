//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "wlquery.h"

WLQuery::WLQuery()
{

}

WLQuery::~WLQuery()
{

}

bool WLQuery::Processed()
{
    return this->Status == Done;
}

void WLQuery::Process()
{
    this->Status = Processing;
    this->Result = new QueryResult();
    QUrl url("http://huggle.wmflabs.org/data/wl.php?action=read&wp=" + Configuration::Project.WhiteList);
    QNetworkRequest request(url);
    this->r = Query::NetworkManager.get(request);
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
    this->Status = Done;
}
