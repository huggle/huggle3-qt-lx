//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HUGGLEFEEDPROVIDERWIKI_H
#define HUGGLEFEEDPROVIDERWIKI_H

#include "definitions.hpp"
// now we need to ensure that python is included first, because it
// simply suck :P
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QList>
#include <QStringList>
#include <QString>
#include <QtXml/QtXml>
#include <QDateTime>
#include "hugglefeed.hpp"
#include "exception.hpp"
#include "querypool.hpp"
#include "mainwindow.hpp"
#include "mediawiki.hpp"
#include "apiquery.hpp"
#include "configuration.hpp"
#include "wikiedit.hpp"
#include "localization.hpp"

namespace Huggle
{
    class ApiQuery;

    //! This is a very simple provider of changes that basically refresh recent changes every 6 seconds
    class HuggleFeedProviderWiki : public HuggleFeed
    {
        public:
            HuggleFeedProviderWiki();
            ~HuggleFeedProviderWiki();
            bool Start();
            bool IsWorking();
            void Stop();
            bool Restart() { this->Stop(); return this->Start(); }
            bool ContainsEdit();
            void Refresh();
            WikiEdit *RetrieveEdit();
            QString ToString();
        private:
            void Process(QString data);
            void ProcessEdit(QDomElement item);
            void ProcessLog(QDomElement item);
            void InsertEdit(WikiEdit *edit);
            bool Refreshing;
            QList<WikiEdit*> *Buffer;
            ApiQuery *qReload;
            QDateTime LastRefresh;
            QDateTime LatestTime;
    };
}

#endif // HUGGLEFEEDPROVIDERWIKI_H
