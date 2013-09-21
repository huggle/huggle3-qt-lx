//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "hugglefeedproviderirc.h"

HuggleFeedProviderIRC::HuggleFeedProviderIRC()
{
    this->Connected = false;
    TcpSocket = NULL;
    thread = NULL;
}

HuggleFeedProviderIRC::~HuggleFeedProviderIRC()
{
    Stop();
    delete thread;
    delete TcpSocket;
}

bool HuggleFeedProviderIRC::Start()
{
    if (Connected)
    {
        Core::DebugLog("Attempted to start connection which was already started");
        return false;
    }
    TcpSocket = new QTcpSocket(Core::Main);
    TcpSocket->connectToHost(Configuration::IRCServer, Configuration::IRCPort);
    if (!TcpSocket->waitForConnected())
    {
        Core::Log("IRC: Connection timeout");
        TcpSocket->close();
        delete TcpSocket;
        TcpSocket = NULL;
        return false;
    }
    Core::Log("IRC: Successfuly connected to irc rc feed");
    this->TcpSocket->write(QString("USER " + Configuration::IRCNick + " 8 * :" + Configuration::IRCIdent + "\n").toUtf8());
    this->TcpSocket->write(QString("NICK " + Configuration::IRCNick + Configuration::UserName + "\n").toUtf8());
    this->TcpSocket->write(QString("JOIN " + Configuration::Project.IRCChannel + "\n").toUtf8());
    this->thread = new HuggleFeedProviderIRC_t(TcpSocket);
    this->thread->p = this;
    this->thread->start();
    Connected = true;
    return true;
}

bool HuggleFeedProviderIRC::IsWorking()
{
    return Connected;
}

void HuggleFeedProviderIRC::Stop()
{
    if (!Connected)
    {
        return;
    }
    if (this->TcpSocket == NULL)
    {
        throw new Exception("The pointer to TcpSocket was NULL during Stop() of irc provider");
    }
    this->TcpSocket->close();
    delete this->TcpSocket;
    this->TcpSocket = NULL;
    if (this->thread == NULL)
    {
        throw new Exception("The pointer to thread was NULL during Stop() of irc provider");
    }
    this->thread->exit();
    delete this->thread;
    this->thread = NULL;
}

void HuggleFeedProviderIRC::InsertEdit(WikiEdit *edit)
{
    Core::PreProcessEdit(edit);
    if (Core::Main->Queue1->CurrentFilter->Matches(edit))
    {
        this->lock.lock();
        while (this->Buffer.size() > Configuration::ProviderCache)
        {
            delete this->Buffer.at(0);
            this->Buffer.removeAt(0);
        }
        this->Buffer.append(edit);
        this->lock.unlock();
    }
}

void HuggleFeedProviderIRC::ParseEdit(QString line)
{
    if (!line.contains(" PRIVMSG "))
    {
        return;
    }

    line = line.mid(line.indexOf(" PRIVMSG ") + 9);

    if (!line.contains(":"))
    {
        Core::DebugLog("Invalid line (no:):" + line);
        return;
    }

    line = line.mid(line.indexOf(":") + 1);

    if (!line.contains(QString(QChar(003)) + "07"))
    {
        Core::DebugLog("Invalid line (no07):" + line);
        return;
    }

    line = line.mid(line.indexOf(QString(QChar(003)) + "07") + 3);

    if (!line.contains(QString(QChar(003)) + "14"))
    {
        Core::DebugLog("Invalid line (no14):" + line);
        return;
    }

    WikiEdit *edit = new WikiEdit();
    edit->Page = new WikiPage(line.mid(0, line.indexOf(QString(QChar(003)) + "14")));

    if (!line.contains(QString(QChar(003)) + "4 "))
    {
        Core::DebugLog("Invalid line (no:x4:" + line);
        delete edit;
        return;
    }

    line = line.mid(line.indexOf(QString(QChar(003)) + "4 ") + 2);
    QString flags = line.mid(0, line.indexOf(QChar(003)));
    edit->Bot = flags.contains("B");
    edit->NewPage = flags.contains("N");
    edit->Minor = flags.contains("M");

    // this below looks like a nasty hack to filter out just what we need
    // but I will later use all of these actions for something too
    if (flags.contains("patrol"))
    {
        delete edit;
        return;
    }

    if (flags.contains("modify"))
    {
        delete edit;
        return;
    }

    if (flags.contains("reviewed"))
    {
        delete edit;
        return;
    }

    if (flags.contains("block"))
    {
        delete edit;
        return;
    }

    if (flags.contains("protect"))
    {
        delete edit;
        return;
    }

    if (flags.contains("reblock"))
    {
        delete edit;
        return;
    }

    if (flags.contains("resolve"))
    {
        delete edit;
        return;
    }

    if (flags.contains("upload"))
    {
        delete edit;
        return;
    }

    if (flags.contains("feature"))
    {
        delete edit;
        return;
    }

    if (flags.contains("noaction"))
    {
        delete edit;
        return;
    }

    if (flags.contains("selfadd"))
    {
        delete edit;
        return;
    }

    if (flags.contains("hit"))
    {
        // abuse filter hit
        delete edit;
        return;
    }

    if (flags.contains("create"))
    {
        delete edit;
        return;
    }

    if (flags.contains("delete"))
    {
        delete edit;
        return;
    }

    if (flags.contains("move"))
    {
        delete edit;
        return;
    }

    if (!edit->NewPage)
    {
        if (!line.contains("?diff="))
        {
            Core::DebugLog("Invalid line (flags: " + flags + ") (no diff):" + line);
            delete edit;
            return;
        }

        line = line.mid(line.indexOf("?diff=") + 6);

        if (!line.contains("&"))
        {
            Core::DebugLog("Invalid line (no &):" + line);
            delete edit;
            return;
        }

        edit->Diff = line.mid(0, line.indexOf("&")).toInt();
    }

    if (!line.contains("oldid="))
    {
        Core::DebugLog("Invalid line (no oldid?):" + line);
        delete edit;
        return;
    }

    line = line.mid(line.indexOf("oldid=") + 6);

    if (!line.contains(QString(QChar(003))))
    {
        Core::DebugLog("Invalid line (no termin):" + line);
        delete edit;
        return;
    }

    edit->OldID = line.mid(0, line.indexOf(QString(QChar(003)))).toInt();

    if (!line.contains(QString(QChar(003)) + "03"))
    {
        Core::DebugLog("Invalid line, no user: " + line);
        delete edit;
        return;
    }

    line = line.mid(line.indexOf(QString(QChar(003)) + "03") + 3);

    if (!line.contains(QString(QChar(3))))
    {
        Core::DebugLog("Invalid line (no termin):" + line);
        delete edit;
        return;
    }

    QString name = line.mid(0, line.indexOf(QString(QChar(3))));

    if (name == "")
    {
        delete edit;
        return;
    }

    edit->User = new WikiUser(name);

    if (line.contains(QString(QChar(3)) + "10"))
    {
        line = line.mid(line.indexOf(QString(QChar(3)) + "10"));
        if (line.contains(QString(QChar(3))))
        {
            edit->Summary = line.mid(0, line.indexOf(QString(QChar(3))));
        }
    }

    this->InsertEdit(edit);
}

bool HuggleFeedProviderIRC::ContainsEdit()
{
    return (this->Buffer.size() != 0);
}

void HuggleFeedProviderIRC_t::run()
{
    if (this->p == NULL)
    {
        throw new Exception("Pointer to parent IRC feed is NULL");
    }
    int ping = 2000;
    while (this->Running && this->s->isOpen())
    {
        if (ping < 0)
        {
            this->s->write(QString("PING :" + Configuration::IRCServer).toUtf8());
            ping = 2000;
        }
        QString text = QString::fromUtf8(this->s->readLine());
        if (text == "")
        {
            QThread::usleep(2000000);
            ping -= 100;
            continue;
        }
        Core::DebugLog("IRC Input: " + text, 6);
        p->ParseEdit(text);
        QThread::usleep(200000);
        ping--;
    }
    Core::Log("IRC: Closed connection to irc feed");
}

HuggleFeedProviderIRC_t::HuggleFeedProviderIRC_t(QTcpSocket *socket)
{
    this->s = socket;
    Running = true;
    this->p = NULL;
}

HuggleFeedProviderIRC_t::~HuggleFeedProviderIRC_t()
{
    // we must not delete the socket here, that's a job of parent object
}

WikiEdit *HuggleFeedProviderIRC::RetrieveEdit()
{
    this->lock.lock();
    if (this->Buffer.size() == 0)
    {
        return NULL;
    }
    WikiEdit *edit = this->Buffer.at(0);
    this->Buffer.removeAt(0);
    this->lock.unlock();
    Core::PostProcessEdit(edit);
    return edit;
}
