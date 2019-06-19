//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HUGGLEFEEDPROVIDERXML_H
#define HUGGLEFEEDPROVIDERXML_H

#include "definitions.hpp"

#include <QStringList>
#include <QString>
#include <QMutex>
#include <QTimer>
#include <QThread>
#include <QDateTime>
#include <QTcpSocket>
#include "hugglefeed.hpp"

namespace Huggle
{
    class HUGGLE_EX_CORE HuggleFeedProviderXml : public QObject, public HuggleFeed
    {
            Q_OBJECT
        public:
            HuggleFeedProviderXml(WikiSite *site);
            ~HuggleFeedProviderXml() override;
            bool Start() override;
            bool IsPaused() override;
            int GetID() override { return HUGGLE_FEED_PROVIDER_XMLRPC; }
            void Resume() override;
            void Pause() override;
            bool IsWorking() override;
            void Stop() override;
            bool Restart() override { this->Stop(); return this->Start(); }
            bool ContainsEdit() override;
            int FeedPriority() override { return 100; }
            QString GetError() override;
            unsigned long long GetBytesReceived() override;
            unsigned long long GetBytesSent() override;
            WikiEdit *RetrieveEdit() override;
            QString ToString() override;
        private slots:
            void OnError(QAbstractSocket::SocketError er);
            void OnReceive();
            void OnConnect();
            void OnPing();
        protected:
            void write(const QString& text);
            void insertEdit(WikiEdit *edit);
            void processBufs();
            QStringList bufferedLines;
            QString bufferedPart;
            QDateTime lastPong;
            QString lastError = "No error";
            bool isConnected = false;
            bool isConnecting = false;
            bool isWorking = false;
            unsigned long long bytesSent = 0;
            unsigned long long bytesRcvd = 0;
            QList<WikiEdit*> buffer;
            QTcpSocket *networkSocket;
        private:
            QTimer *pinger;
            bool isPaused = false;
    };
}

#endif // HUGGLEFEEDPROVIDERXML_H
