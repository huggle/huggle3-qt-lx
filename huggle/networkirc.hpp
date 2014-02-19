//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef NETWORKIRC_H
#define NETWORKIRC_H

#include "config.hpp"
// now we need to ensure that python is included first, because it
// simply suck :P
// seriously, Python.h is shitty enough that it requires to be
// included first. Don't believe it? See this:
// http://stackoverflow.com/questions/20300201/why-python-h-of-python-3-2-must-be-included-as-first-together-with-qt4
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QString>
#include <QtNetwork>
#include <QThread>
#include <QMutex>
#include "configuration.hpp"
#include "localization.hpp"
#include "syslog.hpp"
#include "sleeper.hpp"
#include "exception.hpp"

namespace Huggle
{
    //! Namespace that contains IRC objects
    namespace IRC
    {
        class Channel;
        class User;
        class NetworkIrc;

        //! Represent a channel on IRC network
        class Channel
        {
            public:
                Channel();
                QString Name;
                QList<User> Users;
        };

        /*!
         * \brief The User class represent a user of irc network
         */
        class User
        {
            public:
                User(QString nick, QString ident, QString host);
                User();
                User(User *user);
                User(const User &user);
                QString Ident;
                QString Nick;
                QString Host;
        };

        //! Represent a message on irc network sent either to a channel or to a user
        class Message
        {
            public:
                Message(QString text, User us);
                Message(QString chan, QString text, User us);
                Message();
                Message(Message *ms);
                Message(const Message &ms);
                QString Channel;
                QString Text;
                User user;
        };

        /*!
         * \brief The NetworkIrc_th class is a network thread for Network Irc
         */
        class NetworkIrc_th : public QThread
        {
                Q_OBJECT
            public:
                NetworkIrc_th();
                ~NetworkIrc_th();
                void Data(QString text);
                bool Running;
                NetworkIrc *root;
                void Line(QString line);
                void ProcessPrivmsg(QString source, QString xx);
                bool __Connected;
                bool __IsConnecting;
                QStringList IncomingBuffer;
                QStringList OutgoingBuffer;
                //! This lock is used to lock the data structures when they are accessed by anyone
                QMutex *lIOBuffers;
            protected:
                void run();
            private:
                /*!
                 * \brief Pointer to a QT Socket that is handling the connection to irc server
                 * this object is managed by parent so don't delete it
                 */
                QTcpSocket *s;
        };

        /*!
         * \brief The NetworkIrc provides connection to IRC servers
         */
        class NetworkIrc : public QObject
        {
                Q_OBJECT
            public:
                NetworkIrc(QString server, QString nick);
                ~NetworkIrc();
                //! Connect to server
                bool Connect();
                /*!
                 * \brief IsConnected checks for connection
                 * \return When you are connected returns true
                 */
                bool IsConnected();
                bool IsConnecting();
                void Disconnect();
                void Join(QString name);
                void Part(QString name);
                void Data(QString text);
                void Send(QString name, QString text);
                QString Server;
                QString Nick;
                QString UserName;
                QString Ident;
                QMutex *MessagesLock;
                Message* GetMessage();
                QStringList Channels;
                int Port;
                QList<Message> Messages;
            private slots:
                void OnReceive();
                void OnTime();
            private:
                NetworkIrc_th *NetworkThread;
                QTcpSocket *NetworkSocket;
                QTimer *Timer;
        };
    }
}

#endif // NETWORKIRC_H
