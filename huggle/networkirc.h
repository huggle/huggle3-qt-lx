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

#include <QString>
#include <QtNetwork>
#include <QThread>
#include <QMutex>

namespace Huggle
{
namespace IRC
{
    class User
    {
    public:
        User(QString nick, QString ident, QString host);
        User();
        User(User *user);
        User(const User &user);
        QString Nick;
        QString Ident;
    };

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

    class NetworkIrc;

    class NetworkIrc_th : public QThread
    {
        Q_OBJECT
    public:
        NetworkIrc_th(QTcpSocket *socket);
        ~NetworkIrc_th();
        bool Running;
        NetworkIrc *root;
        void Line(QString line);
        void ProcessPrivmsg(QString source, QString xx);
        bool IsFinished();
    protected:
        void run();
    private:
        QTcpSocket *s;
        bool Stopped;
    };

    class NetworkIrc
    {
    public:
        NetworkIrc(QString server, QString nick);
        ~NetworkIrc();
        void Connect();
        bool IsConnected();
        void Join(QString name);
        void Part(QString name);
        void Data(QString text);
        void Send(QString name, QString text);
        void Exit();
        QString Server;
        QString Nick;
        QString Ident;
        QMutex messages_lock;
        QStringList Channels;
        int Port;
        QList<Message> Messages;
    private:
        bool __Connected;
        QMutex writer_lock;
        NetworkIrc_th *NetworkThread;
        QTcpSocket *s;
    };
}
}

#endif // NETWORKIRC_H
