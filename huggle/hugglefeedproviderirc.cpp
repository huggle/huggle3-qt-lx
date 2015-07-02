//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "hugglefeedproviderirc.hpp"
#include "configuration.hpp"
#include "mainwindow.hpp"
#include "exception.hpp"
#include "hugglequeue.hpp"
#include "localization.hpp"
#include "networkirc.hpp"
#include "querypool.hpp"
#include "syslog.hpp"
#include "wikiedit.hpp"
#include "wikipage.hpp"
#include "wikisite.hpp"
#include "wikiuser.hpp"

using namespace Huggle;

HuggleFeedProviderIRC::HuggleFeedProviderIRC(WikiSite *site) : HuggleFeed(site)
{
    this->Paused = false;
    this->Connected = false;
    this->timer = new QTimer();
    connect(this->timer, SIGNAL(timeout()), this, SLOT(OnTick()));
    this->Network = nullptr;
}

HuggleFeedProviderIRC::~HuggleFeedProviderIRC()
{
    this->Stop();
    delete this->timer;
    while (this->Buffer.count() > 0)
    {
        this->Buffer.at(0)->DecRef();
        this->Buffer.removeAt(0);
    }
    delete this->Network;
}

bool HuggleFeedProviderIRC::Start()
{
    if (this->Connected)
    {
        HUGGLE_DEBUG1("Attempted to start connection which was already started");
        return false;
    }
    if (this->Network != nullptr)
        delete this->Network;

    QString nick = "huggle";
    qsrand(QTime::currentTime().msec());
    nick += QString::number(qrand());
    this->Network = new Huggle::IRC::NetworkIrc(Configuration::HuggleConfiguration->IRCServer, nick);
    this->Network->Port = Configuration::HuggleConfiguration->IRCPort;
    this->Network->UserName = Configuration::HuggleConfiguration->HuggleVersion;
    if (!this->Network->Connect())
    {
        Huggle::Syslog::HuggleLogs->Log(_l("irc-error", Configuration::HuggleConfiguration->IRCServer, this->Network->ErrorMs));
        delete this->Network;
        this->Network = nullptr;
        return false;
    }
    this->Network->Join(this->GetSite()->IRCChannel);
    Huggle::Syslog::HuggleLogs->Log(_l("irc-connected", this->Site->Name));
    this->timer->start(HUGGLE_TIMER);
    this->Connected = true;
    this->UptimeDate = QDateTime::currentDateTime();
    return true;
}

bool HuggleFeedProviderIRC::IsWorking()
{
    if (this->Network != nullptr)
    {
        return this->Connected && (this->Network->IsConnected() || this->Network->IsConnecting());
    }
    return false;
}

void HuggleFeedProviderIRC::Stop()
{
    if (!this->Connected || this->Network == nullptr)
    {
        return;
    }
    this->timer->stop();
    this->Network->Disconnect();
    this->Connected = false;
}

void HuggleFeedProviderIRC::InsertEdit(WikiEdit *edit)
{
    if (edit == nullptr)
        throw new Huggle::NullPointerException("WikiEdit *edit", BOOST_CURRENT_FUNCTION);

    // Increase the number of edits that were made since provider is up, this is used for statistics
    this->IncrementEdits();
    // We need to pre process edit so that we have all its properties ready for queue filter
    QueryPool::HugglePool->PreProcessEdit(edit);
    // We only insert it to buffer in case that current filter matches the edit, this is probably not needed
    // but it might be a performance improvement at some point
    if (edit->GetSite()->CurrentFilter->Matches(edit))
    {
        if (this->Buffer.size() > Configuration::HuggleConfiguration->SystemConfig_ProviderCache)
        {
            // If the buffer is full we need to remove 10 oldest edits
            // we also show a warning to user
            while (this->Buffer.size() > (Configuration::HuggleConfiguration->SystemConfig_ProviderCache - 10))
            {
                this->Buffer.at(0)->DecRef();
                this->Buffer.removeAt(0);
            }
            // This warning isn't useful if the provider is not running
            if (!this->IsPaused())
                Huggle::Syslog::HuggleLogs->WarningLog("insufficient space in irc cache, increase ProviderCache size, otherwise you will be losing edits");
        }
        this->Buffer.append(edit);
    } else
    {
        edit->DecRef();
    }
}

void HuggleFeedProviderIRC::ParseEdit(QString line)
{
    // skip edits if provider is disabled
    if (this->Paused)
    {
        return;
    }
    if (!line.contains(QString(QChar(003)) + "07"))
    {
        HUGGLE_DEBUG("Invalid line (no07):" + line, 1);
        return;
    }
    line = line.mid(line.indexOf(QString(QChar(003)) + "07") + 3);
    if (!line.contains(QString(QChar(003)) + "14"))
    {
        HUGGLE_DEBUG("Invalid line (no14):" + line, 1);
        return;
    }
    WikiEdit *edit = new WikiEdit();
    edit->Page = new WikiPage(line.mid(0, line.indexOf(QString(QChar(003)) + "14")));
    edit->Page->Site = this->GetSite();
    edit->IncRef();
    if (!line.contains(QString(QChar(003)) + "4 "))
    {
        HUGGLE_DEBUG("Invalid line (no:x4:" + line, 1);
        edit->DecRef();
        return;
    }
    line = line.mid(line.indexOf(QString(QChar(003)) + "4 ") + 2);
    QString flags = line.mid(0, line.indexOf(QChar(003)));
    edit->Bot = flags.contains("B");
    edit->NewPage = flags.contains("N");
    edit->Minor = flags.contains("M");
    // this below looks like a nasty hack to filter out just what we need
    // but I will later use all of these actions for something too
    if (flags.contains("thank")    || flags.contains("modify") ||
        flags.contains("rights")   || flags.contains("review") ||
        flags.contains("block")    || flags.contains("protect") ||
        flags.contains("reblock")  || flags.contains("unhelpful") ||
        flags.contains("helpful")  || flags.contains("approve") ||
        flags.contains("resolve")  || flags.contains("upload") ||
        flags.contains("feature")  || flags.contains("noaction") ||
        flags.contains("byemail")  || flags.contains("overwrite") ||
        flags.contains("create")   || flags.contains("delete") ||
        flags.contains("restore")  || flags.contains("move") ||
        flags.contains("tag")      || /* abuse filter */flags.contains("hit") ||
        flags.contains("patrol")   || flags.contains("revision") ||
        flags.contains("add")      || flags.contains("selfadd"))
    {
        edit->DecRef();
        return;
    }
    if (!edit->NewPage)
    {
        if (!line.contains("?diff="))
        {
            HUGGLE_DEBUG("Invalid line (flags: " + flags + ") (no diff):" + line, 1);
            edit->DecRef();
            return;
        }

        line = line.mid(line.indexOf("?diff=") + 6);

        if (!line.contains("&"))
        {
            HUGGLE_DEBUG("Invalid line (no &):" + line, 1);
            edit->DecRef();
            return;
        }
        edit->Diff = line.mid(0, line.indexOf("&")).toLongLong();
        edit->RevID = line.mid(0, line.indexOf("&")).toLongLong();
    }
    if (!line.contains("oldid="))
    {
        HUGGLE_DEBUG("Invalid line (no oldid?):" + line, 1);
        edit->DecRef();
        return;
    }
    line = line.mid(line.indexOf("oldid=") + 6);
    if (!line.contains(QString(QChar(003))))
    {
        HUGGLE_DEBUG("Invalid line (no termin):" + line, 1);
        edit->DecRef();
        return;
    }
    edit->OldID = line.mid(0, line.indexOf(QString(QChar(003)))).toInt();
    if (!line.contains(QString(QChar(003)) + "03"))
    {
        HUGGLE_DEBUG("Invalid line, no user: " + line, 1);
        edit->DecRef();
        return;
    }
    line = line.mid(line.indexOf(QString(QChar(003)) + "03") + 3);
    if (!line.contains(QString(QChar(3))))
    {
        HUGGLE_DEBUG("Invalid line (no termin):" + line, 1);
        edit->DecRef();
        return;
    }
    QString name = line.mid(0, line.indexOf(QString(QChar(3))));
    if (name.length() <= 0)
    {
        edit->DecRef();
        return;
    }
    edit->User = new WikiUser(name);
    edit->User->Site = this->GetSite();
    if (line.contains(QString(QChar(3)) + " ("))
    {
        line = line.mid(line.indexOf(QString(QChar(3)) + " (") + 3);
        if (line.contains(")"))
        {
            QString xx = line.mid(0, line.indexOf(")"));
            xx = xx.replace("\002", "");
            long size = 0;
            if (xx.startsWith("+"))
            {
                xx = xx.mid(1);
                size = xx.toLong();
                edit->SetSize(size);
            } else if (xx.startsWith("-"))
            {
                xx = xx.mid(1);
                size = xx.toLong() * -1;
                edit->SetSize(size);
            } else
            {
                HUGGLE_DEBUG("No size information for " + edit->Page->PageName, 1);
            }
        }else
        {
            HUGGLE_DEBUG("No size information for " + edit->Page->PageName, 1);
        }
    } else
    {
        HUGGLE_DEBUG("No size information for " + edit->Page->PageName, 1);
    }
    if (line.contains(QString(QChar(3)) + "10"))
    {
        line = line.mid(line.indexOf(QString(QChar(3)) + "10") + 3);
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
    return true;
}

bool HuggleFeedProviderIRC::ContainsEdit()
{
    return (this->Buffer.size() != 0);
}

WikiEdit *HuggleFeedProviderIRC::RetrieveEdit()
{
    if (this->Buffer.size() == 0)
    {
        return nullptr;
    }
    WikiEdit *edit = this->Buffer.at(0);
    this->Buffer.removeAt(0);
    return edit;
}

bool HuggleFeedProviderIRC::IsConnected()
{
    return this->Connected;
}

QString HuggleFeedProviderIRC::ToString()
{
    return "IRC";
}

void HuggleFeedProviderIRC::OnTick()
{
    // wait until we finish connecting to a network
    if (!this->IsWorking() || !this->Network->IsConnected())
        return;

    Huggle::IRC::Message *message = this->Network->GetMessage();
    if (message != nullptr)
    {
        QString text = message->Text;
        this->ParseEdit(text);
    }
}
