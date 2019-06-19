//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HUGGLEFEEDPROVIDERIRC_H
#define HUGGLEFEEDPROVIDERIRC_H

#include "definitions.hpp"

#include <QString>
#include <QThread>
#include <QList>
#include <QTimer>
#include <QMutex>
#include <QTcpSocket>
#include "hugglefeed.hpp"

namespace libircclient
{
    class Parser;
    class Network;
    class Channel;
    class User;
}

namespace Huggle
{
    class WikiEdit;
    namespace IRC
    {
        class NetworkIrc;
    }

    //! Provider that uses a wikimedia irc recent changes feed to retrieve information about edits
    class HUGGLE_EX_CORE HuggleFeedProviderIRC : public QObject, public HuggleFeed
    {
            Q_OBJECT
        public:
            HuggleFeedProviderIRC(WikiSite *site);
            ~HuggleFeedProviderIRC() override;
            bool Start() override;
            bool IsWorking() override;
            void Stop() override;
            bool Restart() override { this->Stop(); return this->Start(); }
            void InsertEdit(WikiEdit *edit);
            void ParseEdit(QString line);
            bool IsStopped() override;
            int FeedPriority() override { return 10; }
            int GetID() override { return HUGGLE_FEED_PROVIDER_IRC; }
            bool ContainsEdit() override;
            WikiEdit *RetrieveEdit() override;
            bool IsPaused() override { return this->isPaused; }
            void Pause() override { this->isPaused = true; }
            void Resume() override { this->isPaused = false; }
            unsigned long long GetBytesReceived() override;
            unsigned long long GetBytesSent() override;
            bool IsConnected();
            QString ToString() override;
        private slots:
            void OnIRCChannelMessage(libircclient::Parser *px);
            void OnConnected();
            void OnFailure(QString reason, int code);
            void OnDisconnected();
        protected:
            bool isConnected;
            QList<WikiEdit*> editBuffer;
            bool isPaused;
            libircclient::Network *network;
    };
}

#endif // HUGGLEFEEDPROVIDERIRC_H
