//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "hugglefeedproviderirc.h"

using namespace Huggle;

HuggleFeedProviderIRC::HuggleFeedProviderIRC()
{
    this->Paused = false;
    this->Connected = false;
    this->TcpSocket = NULL;
    this->thread = NULL;
}

HuggleFeedProviderIRC::~HuggleFeedProviderIRC()
{
    while (Buffer.count() > 0)
    {
        Buffer.at(0)->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
        Buffer.removeAt(0);
    }
    this->Stop();
    delete this->thread;
    delete TcpSocket;
}

bool HuggleFeedProviderIRC::Start()
{
    if (this->Connected)
    {
        Core::DebugLog("Attempted to start connection which was already started");
        return false;
    }
    TcpSocket = new QTcpSocket(Core::Main);
    qsrand(QTime::currentTime().msec());
    TcpSocket->connectToHost(Configuration::IRCServer, Configuration::IRCPort);
    if (!TcpSocket->waitForConnected())
    {
        /// \todo LOCALIZE ME
        Core::Log("IRC: Connection timeout");
        TcpSocket->close();
        delete TcpSocket;
        TcpSocket = NULL;
        return false;
    }
    Core::Log("IRC: Successfuly connected to irc rc feed");
    this->TcpSocket->write(QString("USER " + Configuration::IRCNick
                       + QString::number(qrand()) + " 8 * :"
                       + Configuration::IRCIdent + "\n").toUtf8());
    this->TcpSocket->write(QString("NICK " + Configuration::IRCNick
                       + QString::number(qrand()) + Configuration::UserName.replace(" ", "")
                       + "\n").toUtf8());
    this->TcpSocket->write(QString("JOIN " + Configuration::Project.IRCChannel + "\n").toUtf8());
    if (this->thread != NULL)
    {
        delete this->thread;
    }
    this->thread = new HuggleFeedProviderIRC_t(TcpSocket);
    this->thread->p = this;
    this->thread->start();
    this->Connected = true;
    return true;
}

bool HuggleFeedProviderIRC::IsWorking()
{
    return this->Connected;
}

void HuggleFeedProviderIRC::Stop()
{
    if (!this->Connected)
    {
        return;
    }
    if (this->TcpSocket == NULL)
    {
        throw new Exception("The pointer to TcpSocket was NULL during Stop() of irc provider");
    }
    this->thread->Running = false;
    this->TcpSocket->close();
    delete this->TcpSocket;
    this->TcpSocket = NULL;
    if (this->thread == NULL)
    {
        throw new Exception("The pointer to thread was NULL during Stop() of irc provider");
    }
    this->thread->Running = false;
    this->Connected = false;
    while (!IsStopped())
    {
        Core::Log("Waiting for irc feed provider to stop");
        Sleeper::usleep(200000);
    }
    this->Connected = false;
}

void HuggleFeedProviderIRC::InsertEdit(WikiEdit *edit)
{
    Configuration::EditCounter++;
    Core::PreProcessEdit(edit);
    if (Core::Main->Queue1->CurrentFilter->Matches(edit))
    {
        this->lock.lock();
        if (this->Buffer.size() > Configuration::ProviderCache)
        {
            while (this->Buffer.size() > (Configuration::ProviderCache - 10))
            {
                this->Buffer.at(0)->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
                this->Buffer.removeAt(0);
            }
            Core::Log("WARNING: insufficient space in irc cache, increase ProviderCache size, otherwise you will be loosing edits");
        }
        this->Buffer.append(edit);
        this->lock.unlock();
    } else
    {
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
    }
}

void HuggleFeedProviderIRC::ParseEdit(QString line)
{
    // skip edits if provider is disabled
    if (Paused)
    {
        return;
    }

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
    edit->RegisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
    edit->UnregisterConsumer(HUGGLECONSUMER_WIKIEDIT);

    if (!line.contains(QString(QChar(003)) + "4 "))
    {
        Core::DebugLog("Invalid line (no:x4:" + line);
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
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
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
        return;
    }

    if (flags.contains("modify"))
    {
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
        return;
    }

    if (flags.contains("reviewed"))
    {
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
        return;
    }

    if (flags.contains("block"))
    {
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
        return;
    }

    if (flags.contains("protect"))
    {
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
        return;
    }

    if (flags.contains("reblock"))
    {
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
        return;
    }

    if (flags.contains("unhelpful"))
    {
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
        return;
    }

    if (flags.contains("helpful"))
    {
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
        return;
    }

    if (flags.contains("approve"))
    {
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
        return;
    }

    if (flags.contains("resolve"))
    {
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
        return;
    }

    if (flags.contains("upload"))
    {
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
        return;
    }

    if (flags.contains("feature"))
    {
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
        return;
    }

    if (flags.contains("noaction"))
    {
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
        return;
    }

    if (flags.contains("selfadd"))
    {
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
        return;
    }

    if (flags.contains("overwrite"))
    {
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
        return;
    }

    if (flags.contains("hit"))
    {
        // abuse filter hit
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
        return;
    }

    if (flags.contains("create"))
    {
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
        return;
    }

    if (flags.contains("delete"))
    {
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
        return;
    }

    if (flags.contains("move"))
    {
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
        return;
    }

    if (!edit->NewPage)
    {
        if (!line.contains("?diff="))
        {
            Core::DebugLog("Invalid line (flags: " + flags + ") (no diff):" + line);
            edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
            return;
        }

        line = line.mid(line.indexOf("?diff=") + 6);

        if (!line.contains("&"))
        {
            Core::DebugLog("Invalid line (no &):" + line);
            edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
            return;
        }

        edit->Diff = line.mid(0, line.indexOf("&")).toInt();
        edit->RevID = line.mid(0, line.indexOf("&")).toInt();
    }

    if (!line.contains("oldid="))
    {
        Core::DebugLog("Invalid line (no oldid?):" + line);
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
        return;
    }

    line = line.mid(line.indexOf("oldid=") + 6);

    if (!line.contains(QString(QChar(003))))
    {
        Core::DebugLog("Invalid line (no termin):" + line);
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
        return;
    }

    edit->OldID = line.mid(0, line.indexOf(QString(QChar(003)))).toInt();

    if (!line.contains(QString(QChar(003)) + "03"))
    {
        Core::DebugLog("Invalid line, no user: " + line);
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
        return;
    }

    line = line.mid(line.indexOf(QString(QChar(003)) + "03") + 3);

    if (!line.contains(QString(QChar(3))))
    {
        Core::DebugLog("Invalid line (no termin):" + line);
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
        return;
    }

    QString name = line.mid(0, line.indexOf(QString(QChar(3))));

    if (name == "")
    {
        edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
        return;
    }

    edit->User = new WikiUser(name);

    if (line.contains(QString(QChar(3)) + " ("))
    {
        line = line.mid(line.indexOf(QString(QChar(3)) + " (") + 3);
        if (line.contains(")"))
        {
            QString xx = line.mid(0, line.indexOf(")"));
            int size = 0;
            if (xx.startsWith("+"))
            {
                xx = xx.mid(1);
                size = xx.toInt();
                edit->Size = size;
            } else if (xx.startsWith("-"))
            {
                xx = xx.mid(1);
                size = xx.toInt() * -1;
                edit->Size = size;
            }
        }
    }

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

bool HuggleFeedProviderIRC::IsStopped()
{
    if (this->IsWorking())
    {
        return false;
    }
    if (this->thread != NULL)
    {
        if (this->thread->Running || !this->thread->IsFinished())
        {
            return false;
        }
    }
    return true;
}

bool HuggleFeedProviderIRC::ContainsEdit()
{
    return (this->Buffer.size() != 0);
}

void HuggleFeedProviderIRC_t::run()
{
    if (this->p == NULL)
    {
        this->Stopped = true;
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
            QThread::currentThread()->usleep(2000000);
            ping -= 100;
            continue;
        }
        Core::DebugLog("IRC Input: " + text, 6);
        p->ParseEdit(text);
        QThread::usleep(200000);
        ping--;
    }
    Core::Log("IRC: Closed connection to irc feed");
    if (this->Running)
    {
        p->Connected = false;
    }
    this->Stopped = true;
}

HuggleFeedProviderIRC_t::HuggleFeedProviderIRC_t(QTcpSocket *socket)
{
    this->s = socket;
    this->Stopped = false;
    Running = true;
    this->p = NULL;
}

HuggleFeedProviderIRC_t::~HuggleFeedProviderIRC_t()
{
    // we must not delete the socket here, that's a job of parent object
}

bool HuggleFeedProviderIRC_t::IsFinished()
{
    return Stopped;
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
    edit->UnregisterConsumer(HUGGLECONSUMER_PROVIDERIRC);
    return edit;
}

bool HuggleFeedProviderIRC::IsConnected()
{
    return Connected;
}
