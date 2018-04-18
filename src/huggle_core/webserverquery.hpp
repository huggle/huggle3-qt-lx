//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef WEBSERVERQUERY_H
#define WEBSERVERQUERY_H

#include "definitions.hpp"

#include <QList>
#include <QString>
#include <QObject>
#include "query.hpp"
class QNetworkReply;

namespace Huggle
{
    //! This is a query that can be used to perform simple webserver requests
    class HUGGLE_EX WebserverQuery : public QObject, public Query
    {
            Q_OBJECT
        public:
            WebserverQuery();
            ~WebserverQuery();
            QString QueryTargetToString() { return this->URL; }
            //! Whether the query will submit parameters using POST data
            bool UsingPOST;
            //! This is an url of api request, you probably don't want to change it unless
            //! you want to construct whole api request yourself
            QString URL;
            //! Parameters for action, for example page title
            QString Parameters;
            //! Run
            void Process();
            //! Terminate the query
            void Kill();
        private:
            QNetworkReply *reply = nullptr;
            QByteArray temp;
        private slots:
            void ReadData();
            void Finished();
    };
}

#endif // WEBSERVERQUERY_H
