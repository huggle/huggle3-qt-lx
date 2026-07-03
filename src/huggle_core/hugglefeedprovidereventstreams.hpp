//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HUGGLEFEEDPROVIDEREVENTSTREAMS_H
#define HUGGLEFEEDPROVIDEREVENTSTREAMS_H

#include "definitions.hpp"

#include <QByteArray>
#include <QDateTime>
#include <QList>
#include <QNetworkReply>
#include <QObject>
#include <QTimer>
#include <QString>
#include "hugglefeed.hpp"

class QNetworkAccessManager;
class QJsonObject;
class QJsonValue;

namespace Huggle
{
    class WikiEdit;

    class HUGGLE_EX_CORE HuggleFeedProviderEventStreams : public QObject, public HuggleFeed
    {
            Q_OBJECT
        public:
            HuggleFeedProviderEventStreams(WikiSite *site);
            ~HuggleFeedProviderEventStreams() override;
            bool Start() override;
            bool IsPaused() override;
            int GetID() override { return HUGGLE_FEED_PROVIDER_EVENTSTREAMS; }
            void Resume() override;
            void Pause() override;
            bool IsWorking() override;
            void Stop() override;
            bool Restart() override { this->Stop(); return this->Start(); }
            bool ContainsEdit() override;
            int FeedPriority() override { return 110; }
            QString GetError() override;
            unsigned long long GetBytesReceived() override;
            unsigned long long GetBytesSent() override;
            WikiEdit *RetrieveEdit() override;
            QString ToString() override;
        private slots:
            void onReceive();
            void onFinished();
            void onWatchdog();
#ifdef QT6_BUILD
            void onError(QNetworkReply::NetworkError error);
#else
            void onError(QNetworkReply::NetworkError error);
#endif
        private:
            void processBufferedData();
            void processEventBlock(const QByteArray &block);
            void processMessage(const QByteArray &data);
            void insertEdit(WikiEdit *edit);
            int nextEventBlockDelimiter(int *delimiterLength) const;
            revid_ht jsonRevisionId(const QJsonValue &value) const;
            QNetworkAccessManager *manager = nullptr;
            QNetworkReply *reply = nullptr;
            QTimer *watchdog = nullptr;
            QByteArray pendingData;
            QList<WikiEdit*> buffer;
            QString lastError = "No error";
            QString lastEventID;
            QDateTime lastProgress;
            bool isPaused = false;
            bool isWorking = false;
            bool reconnecting = false;
            unsigned long long bytesRcvd = 0;
            unsigned long long bytesSent = 0;
    };
}

#endif // HUGGLEFEEDPROVIDEREVENTSTREAMS_H
