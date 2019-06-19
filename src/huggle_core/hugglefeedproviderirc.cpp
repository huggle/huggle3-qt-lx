//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "definitions.hpp"
#include "hugglefeedproviderirc.hpp"
#include "configuration.hpp"
#include "exception.hpp"
#include "generic.hpp"
#include "localization.hpp"
#include "hooks.hpp"
#include "hugglequeuefilter.hpp"
#include "querypool.hpp"
#include "syslog.hpp"
#include "wikiedit.hpp"
#include "wikipage.hpp"
#include "wikisite.hpp"
#include "wikiuser.hpp"
#include <libirc/libirc/serveraddress.h>
#include <libirc/libircclient/parser.h>
#include <libirc/libircclient/network.h>
#include <libirc/libircclient/channel.h>

using namespace Huggle;

HuggleFeedProviderIRC::HuggleFeedProviderIRC(WikiSite *site) : HuggleFeed(site)
{
    this->isPaused = false;
    this->isConnected = false;
    this->network = nullptr;
}

HuggleFeedProviderIRC::~HuggleFeedProviderIRC()
{
    this->Stop();
    while (this->editBuffer.count() > 0)
    {
        this->editBuffer.at(0)->DecRef();
        this->editBuffer.removeAt(0);
    }
    delete this->network;
}

bool HuggleFeedProviderIRC::Start()
{
    if (this->isConnected)
    {
        HUGGLE_DEBUG1("Attempted to start connection which was already started");
        return false;
    }
    delete this->network;

    QString nick = "huggle";
    qsrand(static_cast<unsigned int>(QTime::currentTime().msec()));
    nick += QString::number(qrand());
    libirc::ServerAddress server(hcfg->SystemConfig_IRCServer, false, hcfg->SystemConfig_IRCPort, nick);
    server.SetSuffix(this->GetSite()->IRCChannel);
    this->network = new libircclient::Network(server, "Wikimedia IRC");
    this->network->SetDefaultUsername(Configuration::HuggleConfiguration->HuggleVersion);
    this->network->SetDefaultIdent("huggle");
    connect(this->network, SIGNAL(Event_Connected()), this, SLOT(OnConnected()));
    //connect(this->Network, SIGNAL(Event_SelfJoin(libircclient::Channel*)), this, SLOT(OnIRCSelfJoin(libircclient::Channel*)));
    //connect(this->Network, SIGNAL(Event_SelfPart(libircclient::Parser*,libircclient::Channel*)), this, SLOT(OnIRCSelfPart(libircclient::Parser*,libircclient::Channel*)));
    connect(this->network, SIGNAL(Event_PRIVMSG(libircclient::Parser*)), this, SLOT(OnIRCChannelMessage(libircclient::Parser*)));
    //connect(this->Network, SIGNAL(Event_PerChannelQuit(libircclient::Parser*,libircclient::Channel*)), this, SLOT(OnIRCChannelQuit(libircclient::Parser*,libircclient::Channel*)));
    connect(this->network, SIGNAL(Event_Disconnected()), this, SLOT(OnDisconnected()));
    //connect(this->Network, SIGNAL(Event_Join(libircclient::Parser*,libircclient::User*,libircclient::Channel*)), this, SLOT(OnIRCUserJoin(libircclient::Parser*,libircclient::User*,libircclient::Channel*)));
    //connect(this->Network, SIGNAL(Event_Part(libircclient::Parser*,libircclient::Channel*)), this, SLOT(OnIRCUserPart(libircclient::Parser*,libircclient::Channel*)));
    connect(this->network, SIGNAL(Event_NetworkFailure(QString,int)), this, SLOT(OnFailure(QString,int)));
    this->isConnected = true;
    this->network->Connect();
    return true;
}

bool HuggleFeedProviderIRC::IsWorking()
{
    if (this->network != nullptr)
    {
        return this->isConnected && (this->network->IsConnected());
    }
    return false;
}

void HuggleFeedProviderIRC::Stop()
{
    if (!this->isConnected || this->network == nullptr)
    {
        return;
    }
    this->network->Disconnect(Generic::IRCQuitDefaultMessage());
    this->isConnected = false;
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
    if (edit->GetSite()->CurrentFilter->Matches(edit) && Hooks::EditBeforePreProcess(edit))
    {
        if (this->editBuffer.size() > Configuration::HuggleConfiguration->SystemConfig_ProviderCache)
        {
            // If the buffer is full we need to remove 10 oldest edits
            // we also show a warning to user
            while (this->editBuffer.size() > (Configuration::HuggleConfiguration->SystemConfig_ProviderCache - 10))
            {
                this->editBuffer.at(0)->DecRef();
                this->editBuffer.removeAt(0);
            }
            // This warning isn't useful if the provider is not running
            if (!this->IsPaused())
                Huggle::Syslog::HuggleLogs->WarningLog("insufficient space in irc cache, increase ProviderCache size, otherwise you will be losing edits");
        }
        this->editBuffer.append(edit);
    } else
    {
        edit->DecRef();
    }
}

void HuggleFeedProviderIRC::ParseEdit(QString line)
{
    // skip edits if provider is disabled
    if (this->isPaused)
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
    edit->Page = new WikiPage(line.mid(0, line.indexOf(QString(QChar(003)) + "14")), this->GetSite());
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
    edit->IsMinor = flags.contains("M");
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
    edit->OldID = line.midRef(0, line.indexOf(QString(QChar(003)))).toInt();
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
    edit->User = new WikiUser(name, this->GetSite());
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
    return !this->IsWorking();
}

bool HuggleFeedProviderIRC::ContainsEdit()
{
    return (!this->editBuffer.empty());
}

WikiEdit *HuggleFeedProviderIRC::RetrieveEdit()
{
    if (this->editBuffer.empty())
        return nullptr;

    WikiEdit *edit = this->editBuffer.at(0);
    this->editBuffer.removeAt(0);
    return edit;
}

unsigned long long HuggleFeedProviderIRC::GetBytesReceived()
{
    if (!this->network)
        return 0;
    return static_cast<unsigned long long>(this->network->GetBytesReceived());
}

unsigned long long HuggleFeedProviderIRC::GetBytesSent()
{
    if (!this->network)
        return 0;
    return static_cast<unsigned long long>(this->network->GetBytesSent());
}

bool HuggleFeedProviderIRC::IsConnected()
{
    return this->isConnected;
}

QString HuggleFeedProviderIRC::ToString()
{
    return "IRC";
}

void HuggleFeedProviderIRC::OnIRCChannelMessage(libircclient::Parser *px)
{
    this->ParseEdit(px->GetText());
}

void HuggleFeedProviderIRC::OnConnected()
{
    Huggle::Syslog::HuggleLogs->Log(_l("irc-connected", this->Site->Name));
    this->isConnected = true;
    this->startupTime = QDateTime::currentDateTime();
}

void HuggleFeedProviderIRC::OnFailure(QString reason, int code)
{
    // We don't need to echo this as error, it's actually OK
    if (code == EDISCONNECTED)
        return;

    Syslog::HuggleLogs->ErrorLog("IRC provider: ec (" + QString::number(code) + ") " + reason);
}

void HuggleFeedProviderIRC::OnDisconnected()
{
    this->isConnected = false;
}
