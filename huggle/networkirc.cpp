//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "networkirc.hpp"

using namespace Huggle::IRC;

NetworkIrc::NetworkIrc(QString server, QString nick)
{
    this->MessagesLock = new QMutex(QMutex::Recursive);
    this->Ident = "huggle";
    this->Nick = nick;
    this->Port = 6667;
    this->Server = server;
    this->UserName = "Huggle client";
    this->Timer = new QTimer(this);
    this->NetworkSocket = NULL;
    this->NetworkThread = NULL;
}

NetworkIrc::~NetworkIrc()
{
    delete this->MessagesLock;
    delete this->NetworkSocket;
    delete this->Timer;
    delete this->NetworkThread;
}

bool NetworkIrc::Connect()
{
    if (this->NetworkThread != NULL)
    {
        if (this->NetworkThread->__IsConnecting)
        {
            throw new Huggle::Exception("You attempted to connect NetworkIrc which is already connecting", "bool NetworkIrc::Connect()");
        }
        if (this->NetworkThread->__Connected)
        {
            throw new Huggle::Exception("You attempted to connect NetworkIrc which is already connected");
        }
    }
    if (this->NetworkThread != NULL)
    {
        delete this->NetworkThread;
    }
    this->NetworkThread = new NetworkIrc_th();
    this->NetworkThread->root = this;
    if (this->NetworkSocket != NULL)
    {
        delete this->NetworkSocket;
    }
    this->NetworkSocket = new QTcpSocket();
    connect(this->NetworkSocket, SIGNAL(readyRead()), this, SLOT(OnReceive()));
    this->NetworkThread->__IsConnecting = true;
    this->NetworkSocket->connectToHost(this->Server, this->Port);
    if (!this->NetworkSocket->waitForConnected())
    {
        this->NetworkThread->__IsConnecting = false;
        return false;
    }
    this->Data("USER " + this->Ident + " 8 * :" + this->UserName);
    QString nick = this->Nick;
    nick = nick.replace(" ", "");
    this->Data("NICK " + nick);
    this->NetworkThread->start();
    connect(this->Timer, SIGNAL(timeout()), this, SLOT(OnTime()));
    this->Timer->start(100);
    return true;
}

bool NetworkIrc::IsConnected()
{
    if (this->NetworkThread == NULL)
    {
        return false;
    }
    return this->NetworkThread->__Connected;
}

bool NetworkIrc::IsConnecting()
{
    if (this->NetworkThread == NULL)
    {
        return false;
    }
    return this->NetworkThread->__IsConnecting;
}

void NetworkIrc::Disconnect()
{
    if (!this->IsConnected())
    {
        return;
    }
    this->Data("QUIT :Huggle, the anti vandalism software. See #huggle on irc://chat.freenode.net");
    this->NetworkSocket->disconnect();
    this->Timer->stop();
    this->NetworkThread->__IsConnecting = false;
    this->NetworkThread->__Connected = false;
    if (this->NetworkThread != NULL)
    {
        // we have to request the network thread to stop
        this->NetworkThread->Running = false;
    }
}

void NetworkIrc::Join(QString name)
{
    this->Data("JOIN " + name);
}

void NetworkIrc::Part(QString name)
{
    this->Data("PART " + name);
}

void NetworkIrc::Data(QString text)
{
    if (this->NetworkThread == NULL || this->NetworkSocket == NULL)
    {
        throw new Exception("You can't send data to network which you never connected to", "void NetworkIrc::Data(QString text)");
    }
    this->NetworkSocket->write((text + "\n").toUtf8());
}

void NetworkIrc::Send(QString name, QString text)
{
    this->Data("PRIVMSG " + name + " :" + text);
}

Message* NetworkIrc::GetMessage()
{
    Message *message;
    this->MessagesLock->lock();
    if (this->Messages.count() == 0)
    {
        this->MessagesLock->unlock();
        return NULL;
    } else
    {
        message = new Message(Messages.at(0));
        this->Messages.removeAt(0);
    }
    this->MessagesLock->unlock();
    return message;
}

void NetworkIrc::OnReceive()
{
    QString data(this->NetworkSocket->readLine());
    this->NetworkThread->lIOBuffers->lock();
    while (data != "")
    {
        this->NetworkThread->IncomingBuffer.append(data);
        data = QString(this->NetworkSocket->readLine());
    }
    this->NetworkThread->lIOBuffers->unlock();
}

void NetworkIrc::OnTime()
{
    this->NetworkThread->lIOBuffers->lock();
    while (this->NetworkThread->OutgoingBuffer.count() > 0)
    {
        this->Data(this->NetworkThread->OutgoingBuffer.at(0));
        this->NetworkThread->OutgoingBuffer.removeAt(0);
    }
    this->NetworkThread->lIOBuffers->unlock();
}

Message::Message(QString text, User us)
{
    this->Text = text;
    this->Channel = "";
    this->user = us;
}

Message::Message(const Message &ms)
{
    this->Text = ms.Text;
    this->Channel = ms.Channel;
    this->user = ms.user;
}

Message::Message(QString chan, QString text, User us)
{
    this->Channel = chan;
    this->Text = text;
    this->user = us;
}

Message::Message()
{
    this->Channel = "";
    this->Text = "";
}

Message::Message(Message *ms)
{
    this->Channel = ms->Channel;
    this->Text = ms->Text;
    this->user = ms->user;
}

User::User(QString nick, QString ident, QString host)
{
    this->Nick = nick;
    this->Ident = ident;
    this->Host = host;
}

User::User()
{
    this->Ident = "";
    this->Nick = "";
    this->Host = "";
}

User::User(User *user)
{
    this->Host = user->Host;
    this->Ident = user->Ident;
    this->Nick = user->Nick;
}

User::User(const User &user)
{
    this->Host = user.Host;
    this->Ident = user.Ident;
    this->Nick = user.Nick;
}

NetworkIrc_th::NetworkIrc_th()
{
    this->s = NULL;
    this->__Connected = false;
    this->__IsConnecting = false;
    this->lIOBuffers = new QMutex(QMutex::Recursive);
    this->root = NULL;
    this->Running = true;
}

NetworkIrc_th::~NetworkIrc_th()
{
    delete this->s;
    delete this->lIOBuffers;
}

void NetworkIrc_th::Data(QString text)
{
    this->lIOBuffers->lock();
    this->OutgoingBuffer.append(text);
    this->lIOBuffers->unlock();
}

void NetworkIrc_th::Line(QString line)
{
    QString Command = "";
    QString Source = "";
    if (!line.startsWith(":") || !line.contains(" "))
    {
        return;
    }

    QString xx = line;
    xx = xx.mid(1);
    Source = xx.mid(0, xx.indexOf(" "));
    xx= xx.mid(xx.indexOf(" ") + 1);
    if (!xx.contains(" "))
    {
        Command = xx;
    } else
    {
        Command = xx.mid(0, xx.indexOf(" "));
        xx = xx.mid(xx.indexOf(" ") + 1);
    }

    if (Command == "002")
    {
        this->__IsConnecting = false;
        this->__Connected = true;
        return;
    }
    /// \todo implement PART
    /// \todo implement KICK
    /// \todo implement QUIT
    /// \todo implement TOPIC
    /// \todo implement CTCP
    /// \todo implement NOTICES
    if (Command == "PRIVMSG")
    {
        ProcessPrivmsg(Source, xx);
        return;
    }
}

void NetworkIrc_th::ProcessPrivmsg(QString source, QString xx)
{
    User user;
    user.Nick = source.mid(0, source.indexOf("!"));
    Message message;
    if (!xx.contains("#") || !xx.contains(" :"))
    {
        return;
    }
    message.Channel = xx.mid(xx.indexOf("#"), xx.indexOf(" :"));
    message.user = user;
    xx = xx.replace("\r\n", "");
    message.Text = xx.mid(xx.indexOf(" :") + 2);
    this->root->MessagesLock->lock();
    this->root->Messages.append(message);
    this->root->MessagesLock->unlock();
}

void NetworkIrc_th::run()
{
    int ping = 0;
    while (this->Running)
    {
        this->lIOBuffers->lock();
        QStringList buffer;
        if (this->IncomingBuffer.count() > 0)
        {
            buffer += this->IncomingBuffer;
            this->IncomingBuffer.clear();
        }
        this->lIOBuffers->unlock();
        while (buffer.count() > 0)
        {
            QString data = buffer.at(0);
            buffer.removeAt(0);
            this->Line(data);
        }
        ping++;
        if (ping > 200)
        {
            this->Data("PING :" + this->root->Server);
            ping = 0;
        }
        this->usleep(10000);
    }
    this->__Connected = false;
    return;
}
