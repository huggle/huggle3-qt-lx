//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef WLQUERY_H
#define WLQUERY_H

#include <QString>
#include <QtNetwork/QtNetwork>
#include <QUrl>
#include "configuration.h"
#include "query.h"

//! Whitelist query :o
class WLQuery : public Query
{
    Q_OBJECT
public:
    WLQuery();
    ~WLQuery();
    bool Processed();
    void Process();
    bool Save;
private slots:
    void ReadData();
    void Finished();
private:
    QNetworkReply *r;
};

#endif // WLQUERY_H
