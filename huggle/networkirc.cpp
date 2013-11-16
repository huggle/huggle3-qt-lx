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
    this->messages_lock = new QMutex(QMutex::Recursive);
    this->writer_lock = new QMutex(QMutex::Recursive);
    this->__IsConnecting = false;
    this->Ident = "huggle";
    this->Nick = nick;
    this->Port = 6667;
    this->Server = server;
    this->UserName = "Huggle client";
    this->s = NULL;
    this->NetworkThread = NULL;
    this->__Connected = false;
}

NetworkIrc::~NetworkIrc()
{
    delete this->messages_lock;
    delete this->NetworkThread;
    delete this->writer_lock;
    delete this->s;
}

bool NetworkIrc::Connect()
{
    if (this->__IsConnecting)
    {
        throw new Huggle::Exception("You attempted to connect NetworkIrc which is already connecting", "bool NetworkIrc::Connect()");
    }
    if (this->IsConnected())
    {
        throw new Huggle::Exception("You attempted to connect NetworkIrc which is already connected");
    }
    if (this->s != NULL)
    {
        delete this->s;
    }
    this->s = new QTcpSocket();
    this->s->connectToHost(this->Server, this->Port);
    this->__IsConnecting = true;
    if (!this->s->waitForConnected())
    {
        this->Exit();
        this->__IsConnecting = false;
        return false;
    }
    this->NetworkThread = new NetworkIrc_th(this->s);
    this->NetworkThread->root = this;
    this->__Connected = true;
    this->NetworkThread->start();
    return true;
}

bool NetworkIrc::IsConnected()
{
    if (this->NetworkThread != NULL)
    {
        if (!this->NetworkThread->Connected)
        {
            return false;
        }
    }
    return this->__Connected;
}

bool NetworkIrc::IsConnecting()
{
    if (this->__IsConnecting)
    {
        if (this->NetworkThread != NULL)
        {
            if (this->NetworkThread->Connected)
            {
                this->__IsConnecting = false;
                this->__Connected = true;
            } else if (this->NetworkThread->isFinished())
            {
                this->__IsConnecting = false;
                this->__Connected = false;
            }
        }
    }
    return this->__IsConnecting;
}

void NetworkIrc::Disconnect()
{
    if (!this->IsConnected())
    {
        return;
    }
    this->Data("QUIT :Huggle, the anti vandalism software. See #huggle on irc://chat.freenode.net");
    this->Exit();
    if (this->NetworkThread != NULL)
    {
        // we have to request the network thread to stop
        this->NetworkThread->Running = false;
    }
    if (s != NULL)
    {
        this->s->disconnect();
        delete this->s;
        s = NULL;
    }
    this->__IsConnecting = false;
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
    if (!__Connected)
    {
        return;
    }
    this->writer_lock->lock();
    this->s->write((text + QString("\n")).toUtf8());
    this->writer_lock->unlock();
}

void NetworkIrc::Send(QString name, QString text)
{
    this->Data("PRIVMSG " + name + " :" + text);
}

void NetworkIrc::Exit()
{
    this->__Connected = false;
    this->__IsConnecting = false;
}

Message* NetworkIrc::GetMessage()
{
    Message *message;
    this->messages_lock->lock();
    if (this->Messages.count() == 0)
    {
        this->messages_lock->unlock();
        return NULL;
    } else
    {
        message = new Message(Messages.at(0));
        this->Messages.removeAt(0);
    }
    this->messages_lock->unlock();
    return message;
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

NetworkIrc_th::NetworkIrc_th(QTcpSocket *socket)
{
    this->s = socket;
    this->Stopped = false;
    this->Connected = false;
    this->root = NULL;
    this->Running = true;
}

NetworkIrc_th::~NetworkIrc_th()
{

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
        this->Connected = true;
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
    this->root->messages_lock->lock();
    this->root->Messages.append(message);
    this->root->messages_lock->unlock();
}

void NetworkIrc_th::run()
{
    this->root->Data("USER " + this->root->Ident + " 8 * :" + this->root->UserName);
    QString nick = this->root->Nick;
    nick = nick.replace(" ", "");
    this->root->Data("NICK " + nick);
    int ping = 0;
    while (this->Running && this->s->isOpen())
    {
        ping++;
        if (ping > 200)
        {
            this->root->Data("PING :" + this->root->Server);
            ping = 0;
        }
        QString data(s->readLine());
        if (data != "")
        {
            this->Line(data);
        }
        this->usleep(100000);
    }
    this->Connected = false;
    return;
}
