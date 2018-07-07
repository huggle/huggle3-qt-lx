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
            ~HuggleFeedProviderXml();
            bool Start();
            bool IsPaused();
            int GetID() { return HUGGLE_FEED_PROVIDER_XMLRPC; }
            void Resume();
            void Pause();
            bool IsWorking();
            void Stop();
            bool Restart() { this->Stop(); return this->Start(); }
            bool ContainsEdit();
            int FeedPriority() { return 100; }
            QString GetError();
            unsigned long long GetBytesReceived();
            unsigned long long GetBytesSent();
            WikiEdit *RetrieveEdit();
            QString ToString();
        private slots:
            void OnError(QAbstractSocket::SocketError er);
            void OnReceive();
            void OnConnect();
            void OnPing();
        protected:
            void write(QString text);
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
