//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "networkirc.h"

using namespace Huggle::IRC;

NetworkIrc::NetworkIrc(QString server, QString nick)
{
    this->Ident = "huggle";
    this->Nick = nick;
    this->Port = 6667;
    this->Server = server;
    this->UserName = "Huggle client";
    this->s = new QTcpSocket();
    this->NetworkThread = NULL;
    this->__Connected = false;
}

NetworkIrc::~NetworkIrc()
{
    delete this->NetworkThread;
    delete this->s;
}

void NetworkIrc::Connect()
{
    this->s->connectToHost(this->Server, this->Port);
    if (!this->s->waitForConnected())
    {
        this->Exit();
        return;
    }
    this->NetworkThread = new NetworkIrc_th(this->s);
    this->NetworkThread->root = this;
    this->__Connected = true;
    this->NetworkThread->start();
    return;
}

bool NetworkIrc::IsConnected()
{
    return this->__Connected;
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
    this->writer_lock.lock();
    this->s->write((text + QString("\n")).toUtf8());
    this->writer_lock.unlock();
}

void NetworkIrc::Send(QString name, QString text)
{
    this->Data("PRIVMSG " + name + " :" + text);
}

void NetworkIrc::Exit()
{
    this->__Connected = false;
}

Message* NetworkIrc::GetMessage()
{
    Message *message;
    this->messages_lock.lock();
    if (this->Messages.count() == 0)
    {
        this->messages_lock.unlock();
        return NULL;
    } else
    {
        message = new Message(Messages.at(0));
        this->Messages.removeAt(0);
    }
    this->messages_lock.unlock();
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
}

User::User()
{
    this->Ident = "";
    this->Nick = "";
}

User::User(User *user)
{
    this->Ident = user->Ident;
    this->Nick = user->Nick;
}

User::User(const User &user)
{
    this->Ident = user.Ident;
    this->Nick = user.Nick;
}


NetworkIrc_th::NetworkIrc_th(QTcpSocket *socket)
{
    this->s = socket;
    this->Stopped = false;
    this->root = NULL;
    Running = true;
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
    this->root->messages_lock.lock();
    this->root->Messages.append(message);
    this->root->messages_lock.unlock();
}

bool NetworkIrc_th::IsFinished()
{

}

void NetworkIrc_th::run()
{
    this->root->Data("USER " + this->root->Ident + " 8 * :" + this->root->UserName);
    this->root->Data("NICK " + this->root->Nick + QString::number(qrand()));
    int ping = 0;
    while (this->root->IsConnected() && this->s->isOpen())
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
    return;
}
