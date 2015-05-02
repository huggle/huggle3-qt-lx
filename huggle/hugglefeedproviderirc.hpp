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

namespace Huggle
{
    class WikiEdit;
    namespace IRC
    {
        class NetworkIrc;
    }

    //! Provider that uses a wikimedia irc recent changes feed to retrieve information about edits
    class HUGGLE_EX HuggleFeedProviderIRC : public QObject, public HuggleFeed
    {
            Q_OBJECT
        public:
            HuggleFeedProviderIRC(WikiSite *site);
            ~HuggleFeedProviderIRC();
            Huggle::IRC::NetworkIrc *Network;
            bool Start();
            bool IsWorking();
            void Stop();
            bool Restart() { this->Stop(); return this->Start(); }
            void InsertEdit(WikiEdit *edit);
            void ParseEdit(QString line);
            bool IsStopped();
            int GetID() { return HUGGLE_FEED_PROVIDER_IRC; }
            bool ContainsEdit();
            WikiEdit *RetrieveEdit();
            bool IsPaused() { return Paused; }
            void Pause() { Paused = true; }
            void Resume() { Paused = false; }
            bool IsConnected();
            QString ToString();
            bool Connected;
        private slots:
            void OnTick();
        private:
            QTimer *timer;
            QList<WikiEdit*> Buffer;
            bool Paused;
    };
}

#endif // HUGGLEFEEDPROVIDERIRC_H
