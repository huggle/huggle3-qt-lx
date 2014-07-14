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
    class HuggleFeedProviderIRC;

    //! Thread which process the IRC feed
    class HuggleFeedProviderIRC_t : public QThread
    {
            Q_OBJECT
        public:
            HuggleFeedProviderIRC_t();
            ~HuggleFeedProviderIRC_t();
            bool IsFinished();
            bool Running;
            HuggleFeedProviderIRC *p;
        protected:
            void run();
        private:
            bool Stopped;
    };

    //! Provider that uses a wikimedia irc recent changes feed to retrieve information about edits
    class HuggleFeedProviderIRC : public HuggleFeed
    {
        public:
            HuggleFeedProviderIRC();
            ~HuggleFeedProviderIRC();
            Huggle::IRC::NetworkIrc *Network;
            bool Start();
            bool IsWorking();
            void Stop();
            bool Restart() { this->Stop(); return this->Start(); }
            void InsertEdit(WikiEdit *edit);
            void ParseEdit(QString line);
            bool IsStopped();
            bool ContainsEdit();
            WikiEdit *RetrieveEdit();
            bool IsPaused() { return Paused; }
            void Pause() { Paused = true; }
            void Resume() { Paused = false; }
            bool IsConnected();
            QString ToString();
            bool Connected;
        private:
            QMutex lock;
            QList<WikiEdit*> Buffer;
            HuggleFeedProviderIRC_t *thread;
            bool Paused;
    };
}

#endif // HUGGLEFEEDPROVIDERIRC_H
