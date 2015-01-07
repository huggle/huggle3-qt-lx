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

#include "definitions.hpp"

#include <QHash>
#include <QString>
#include <QStringList>
#include <QAbstractSocket>
#include <QThread>
#include "sleeper.hpp"
#include "exception.hpp"

class QTcpSocket;
class QTimer;
class QMutex;

namespace Huggle
{
    //! Namespace that contains IRC objects
    namespace IRC
    {
        class Channel;
        class User;
        class NetworkIrc;

        //! Represent a channel on IRC network
        class HUGGLE_EX Channel
        {
            public:
                Channel(QString name);
                ~Channel();
                void InsertUser(User user);
                void RemoveUser(QString user);
                //! Returns true in case that user list has changed and reset the value to false
                //! useful for one time check if user list needs to be redrawn
                bool UsersChanged();
                QString Name;
                QHash<QString, User> Users;
                QMutex *UsersLock;
            private:
                bool UsersChange_;
        };

        /*!
         * \brief The User class represent a user of irc network
         */
        class HUGGLE_EX User
        {
            public:
                User(QString nick);
                User(QString nick, QString ident, QString host);
                User();
                User(User *user);
                User(const User &user);
                void SanitizeNick();
                QString Ident;
                QString Nick;
                QString Host;
        };

        //! Represent a message on irc network sent either to a channel or to a user
        class HUGGLE_EX Message
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
        class HUGGLE_EX NetworkIrc_th : public QThread
        {
                Q_OBJECT
            public:
                NetworkIrc_th();
                ~NetworkIrc_th();
                void Data(QString text);
                void Line(QString line);
                void ProcessPrivmsg(QString source, QString parameters, QString message);
                void ProcessJoin(QString source, QString channel, QString message);
                void ProcessChannel(QString channel, QString data);
                void ProcessKick(QString source, QString parameters, QString message);
                void ProcessQuit(QString source, QString message);
                void ProcessPart(QString source, QString channel, QString message);
                bool Running;
                NetworkIrc *root;
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
        class HUGGLE_EX NetworkIrc : public QObject
        {
                Q_OBJECT
            public:
                NetworkIrc(QString server, QString nick, bool is_async = false);
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
                bool IsAsync() { return this->async; }
                /*!
                 * \brief GetMessage provides a last message from any channel and remove it from buffer
                 * \return NULL in case there is no message in buffer remaining or message
                 */
                Message* GetMessage();
                //! Hostname of IRC server to use
                QString Server;
                QString Nick;
                QString UserName;
                QString Ident;
                QString ErrorMs;
                QMutex *MessagesLock;
                QHash<QString, Channel*> Channels;
                QMutex *ChannelsLock;
                int Port;
                QList<Message> Messages;
            private slots:
                void OnError(QAbstractSocket::SocketError er);
                void OnReceive();
                void OnTime();
                void OnConnect();
            private:
                void ClearList();
                void Stop();
                NetworkIrc_th *NetworkThread;
                bool async;
                QTcpSocket *NetworkSocket;
                QTimer *Timer;
        };
    }
}

#endif // NETWORKIRC_H
