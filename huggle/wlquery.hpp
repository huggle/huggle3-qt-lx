//This program is free software: you can redistribute it and/or modify it under
//the terms of the GNU General Public License as published by the Free Software
//Foundation, either version 3 of the License, or (at your option) any later
//version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef WLQUERY_H
#define WLQUERY_H

#include <QString>
#include <QtNetwork/QtNetwork>
#include <QUrl>
#include "configuration.hpp"
#include "query.hpp"

namespace Huggle
{
    //! Whitelist query :o
    class WLQuery : public QObject, public Query
    {
            Q_OBJECT
        public:
            WLQuery();
            ~WLQuery();
            void Process();
            bool Save;
            double Progress;
        private slots:
            void ReadData();
            void Finished();
            void WriteProgress(qint64 n, qint64 m);
        private:
            QNetworkReply *r;
    };
}

#endif // WLQUERY_H
