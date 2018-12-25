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

#include <QList>
#include <QDomElement>
#include <QStringList>
#include <QString>
#include <QDateTime>
#include "collectable_smartptr.hpp"
#include "apiquery.hpp"
#include "wikiedit.hpp"
#include "hugglefeed.hpp"

namespace Huggle
{
    class ApiQuery;
    class WikiEdit;

    //! This is a very simple provider of changes that basically refresh recent changes every 6 seconds
    class HUGGLE_EX_CORE HuggleFeedProviderWiki : public HuggleFeed
    {
        public:
            HuggleFeedProviderWiki(WikiSite *site);
            ~HuggleFeedProviderWiki() override;
            bool Start() override;
            bool IsPaused() override;
            void Resume() override;
            void Pause() override;
            bool IsWorking() override;
            void Stop() override;
            bool Restart() override { this->Stop(); return this->Start(); }
            int GetID() override { return HUGGLE_FEED_PROVIDER_WIKI; }
            int FeedPriority() override { return 1; }
            bool ContainsEdit() override;
            void Refresh();
            unsigned long long GetBytesReceived() override;
            unsigned long long GetBytesSent() override;
            WikiEdit *RetrieveEdit() override;
            QString ToString() override;
        private:
            void processData(QString data);
            void processEdit(QDomElement item);
            void processLog(const QDomElement& item);
            void insertEdit(WikiEdit *edit);
            bool isPaused = false;
            bool isRefreshing;
            QList<WikiEdit*> *editBuffer;
            Collectable_SmartPtr<ApiQuery> qReload;
            QDateTime lastRefresh;
            QDateTime latestTime;
    };

    inline bool HuggleFeedProviderWiki::IsPaused()
    {
        return this->isPaused;
    }

    inline void HuggleFeedProviderWiki::Resume()
    {
        this->isPaused = false;
    }

    inline void HuggleFeedProviderWiki::Pause()
    {
        this->isPaused = true;
    }
}

#endif // HUGGLEFEEDPROVIDERWIKI_H
