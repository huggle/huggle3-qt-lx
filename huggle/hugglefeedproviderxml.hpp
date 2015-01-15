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

#include <QString>
#include <QMutex>
#include <QThread>
#include <QTcpSocket>
#include "hugglefeed.hpp"

namespace Huggle
{
    class HUGGLE_EX HuggleFeedProviderXml : public QObject, public HuggleFeed
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
            QString GetError();
            WikiEdit *RetrieveEdit();
            QString ToString();
        private slots:
            void OnError(QAbstractSocket::SocketError er);
            void OnReceive();
            void OnConnect();
        protected:
            void Write(QString text);
            void InsertEdit(WikiEdit *edit);
            QString last_error = "No error";
            bool is_connected = false;
            bool is_connecting = false;
            bool is_working = false;
            QList<WikiEdit*> Buffer;
            QTcpSocket *NetworkSocket;
            bool is_paused = false;
    };
}

#endif // HUGGLEFEEDPROVIDERXML_H
