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
#include <QDateTime>
#include "hugglefeed.hpp"
#include "apiquery.hpp"
#include "wikiedit.hpp"

namespace Huggle
{
    class WikiEdit;
    class ApiQuery;

    //! This is a very simple provider of changes that basically refresh recent changes every 6 seconds
    class HuggleFeedProviderWiki : public HuggleFeed
    {
        public:
            HuggleFeedProviderWiki();
            ~HuggleFeedProviderWiki();
            bool Start();
            bool IsPaused();
            void Resume();
            void Pause();
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
            bool Paused = false;
            bool Refreshing;
            QList<WikiEdit*> *Buffer;
            ApiQuery *qReload;
            QDateTime LastRefresh;
            QDateTime LatestTime;
    };

    inline bool HuggleFeedProviderWiki::IsPaused()
    {
        return this->Paused;
    }

    inline void HuggleFeedProviderWiki::Resume()
    {
        this->Paused = false;
    }

    inline void HuggleFeedProviderWiki::Pause()
    {
        this->Paused = true;
    }
}

#endif // HUGGLEFEEDPROVIDERWIKI_H
