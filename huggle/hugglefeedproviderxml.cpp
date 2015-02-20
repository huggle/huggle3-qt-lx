//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "configuration.hpp"
#include "hugglefeedproviderxml.hpp"
#include "apiquery.hpp"
#include "querypool.hpp"
#include "exception.hpp"
#include "hugglequeue.hpp"
#include "generic.hpp"
#include "mainwindow.hpp"
#include "wikipage.hpp"
#include "syslog.hpp"
#include "wikiedit.hpp"
#include "wikiuser.hpp"
#include "wikisite.hpp"
#include <QtXml>

using namespace Huggle;

HuggleFeedProviderXml::HuggleFeedProviderXml(WikiSite *site) : HuggleFeed(site)
{
    this->pinger = new QTimer();
    connect(this->pinger, SIGNAL(timeout()), this, SLOT(OnPing()));
    this->NetworkSocket = nullptr;
}

HuggleFeedProviderXml::~HuggleFeedProviderXml()
{
    this->Stop();
    delete this->pinger;
    while (this->Buffer.count() > 0)
    {
        this->Buffer.at(0)->DecRef();
        this->Buffer.removeAt(0);
    }
    if (this->NetworkSocket && this->NetworkSocket->isOpen())
        this->NetworkSocket->close();
    delete this->NetworkSocket;
}

bool HuggleFeedProviderXml::Start()
{
    if (this->IsWorking())
    {
        HUGGLE_DEBUG1("Refusing to start working Xml feed");
        return false;
    }
    // we add some seconds here just to make sure it will not timeout before we finish
    // connecting to it
    this->LastPong = QDateTime::currentDateTime().addSecs(22);
    if (this->GetSite()->XmlRcsName.isEmpty())
    {
        Syslog::HuggleLogs->ErrorLog("There is no XmlRcs provider for " + this->GetSite()->Name);
        return false;
    }
    if (this->NetworkSocket != nullptr)
        delete this->NetworkSocket;
    this->NetworkSocket = new QTcpSocket();
    connect(this->NetworkSocket, SIGNAL(readyRead()), this, SLOT(OnReceive()));
    connect(this->NetworkSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(OnError(QAbstractSocket::SocketError)));
    this->is_connecting = true;
    this->is_working = true;
    this->NetworkSocket->connectToHost(hcfg->GlobalConfig_Xmlrcs, hcfg->GlobalConfig_XmlrcsPort);
    // we need to handle the connection later
    connect(this->NetworkSocket, SIGNAL(connected()), this, SLOT(OnConnect()));
    return true;
}

bool HuggleFeedProviderXml::IsPaused()
{
    return this->is_paused;
}

void HuggleFeedProviderXml::Resume()
{
    this->is_paused = false;
}

void HuggleFeedProviderXml::Pause()
{
    this->is_paused = true;
}

void HuggleFeedProviderXml::OnPing()
{
    if (this->is_connected)
    {
        if (QDateTime::currentDateTime().addSecs(-20) > this->LastPong)
        {
            Syslog::HuggleLogs->ErrorLog("XmlRcs feed has timed out, reconnecting to it");
            this->Restart();
        }
        else if (QDateTime::currentDateTime().addSecs(-8) > this->LastPong)
        {
            this->Write("ping");
        }
    }
}

bool HuggleFeedProviderXml::IsWorking()
{
    return this->is_working;
}

void HuggleFeedProviderXml::Stop()
{
    if (this->NetworkSocket)
        this->NetworkSocket->disconnect();
    this->is_connected = false;
    this->is_connecting = false;
    this->pinger->stop();
    this->is_working = false;
    this->is_paused = false;
}

bool HuggleFeedProviderXml::ContainsEdit()
{
    return this->Buffer.count() > 0;
}

QString HuggleFeedProviderXml::GetError()
{
    return this->last_error;
}

WikiEdit *HuggleFeedProviderXml::RetrieveEdit()
{
    if (this->Buffer.size() == 0)
        return nullptr;

    WikiEdit *edit = this->Buffer.at(0);
    this->Buffer.removeAt(0);
    return edit;
}

QString HuggleFeedProviderXml::ToString()
{
    return "XMLRCS";
}

void HuggleFeedProviderXml::OnError(QAbstractSocket::SocketError er)
{
    this->last_error = Generic::SocketError2Str(er);
    this->Stop();
}

void HuggleFeedProviderXml::OnReceive()
{
    // THIS IS IMPORTANT
    // readLine() has some bugs in Qt so don't use it, this function needs to always use readAll from socket otherwise we get in troubles
    if (!this->NetworkSocket)
        throw new Huggle::NullPointerException("this->NetworkSocket", BOOST_CURRENT_FUNCTION);
    QString data(this->NetworkSocket->readAll());
    // when there is no data we can quit this
    if (data.isEmpty())
        return;

    if (!this->BufferedPart.isEmpty())
        data = this->BufferedPart + data;

    if (data.contains("\n"))
    {
        this->BufferedPart.clear();
        if (data.endsWith("\n"))
        {
            // we received whole block of lines
            this->BufferedLines = data.split("\n");
        } else
        {
            QStringList lines = data.split("\n");
            // last line is has not yet finished
            this->BufferedPart = lines.last();
            lines.removeLast();
            this->BufferedLines = lines;
        }
        this->ProcessBufs();
        return;
    }

    this->BufferedPart = data;
}

void HuggleFeedProviderXml::OnConnect()
{
    this->is_connected = true;
    this->pinger->start(1000);
    this->is_connecting = false;
    // subscribe
    this->Write("S " + this->GetSite()->XmlRcsName);
}

void HuggleFeedProviderXml::Write(QString text)
{
    // check if network socket isn't nullptr
    if (!this->NetworkSocket)
        throw new Huggle::NullPointerException("this->NetworkSocket", BOOST_CURRENT_FUNCTION);

    this->NetworkSocket->write(QString(text + "\n").toUtf8());
}

void HuggleFeedProviderXml::InsertEdit(WikiEdit *edit)
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
        if (this->Buffer.size() > hcfg->SystemConfig_ProviderCache)
        {
            // If the buffer is full we need to remove 10 oldest edits
            // we also show a warning to user
            while (this->Buffer.size() > (hcfg->SystemConfig_ProviderCache - 10))
            {
                this->Buffer.at(0)->DecRef();
                this->Buffer.removeAt(0);
            }
            Huggle::Syslog::HuggleLogs->WarningLog("insufficient space in xml cache, increase ProviderCache size, otherwise you will be losing edits");
        }
        this->Buffer.append(edit);
    } else
    {
        edit->DecRef();
    }
}

void HuggleFeedProviderXml::ProcessBufs()
{
    while (!this->BufferedLines.isEmpty())
    {
        QString data = this->BufferedLines.at(0);
        this->BufferedLines.removeAt(0);
        if (data.isEmpty())
            continue;
        // this should be an XML string, let's do some quick test
        if (!data.startsWith("<"))
        {
            Syslog::HuggleLogs->WarningLog("Invalid input from XmlRcs server: " + data);
            continue;
        }

        // every message will update last time
        this->LastPong = QDateTime::currentDateTime();
        QDomDocument input;
        input.setContent(data);
        QDomElement element = input.firstChild().toElement();
        QDateTime ts;
        QString name = element.nodeName();
        if (name == "error")
        {
            Syslog::HuggleLogs->ErrorLog("XmlRcs returned error: " + element.text());
            continue;
        }
        if (name == "ping")
        {
            this->Write("pong");
            continue;
        }

        if (name == "fatal")
        {
            Syslog::HuggleLogs->ErrorLog("XmlRcs failed: " + element.text());
            this->Stop();
            continue;
        }

        if (name == "ok" || name == "pong")
            continue;
        if (name != "edit")
        {
            HUGGLE_DEBUG1("Weird result from xml provider: " + data);
            continue;
        }

        WikiEdit *edit;
        QString type;

        if (!element.attributes().contains("type"))
            goto invalid;

        type = element.attribute("type");
        if (type != "edit" && type != "new")
        {
            // we are not interested in this
            continue;
        }

        // let's verify if all necessary elements are present
        if (!element.attributes().contains("server_name") ||
            !element.attributes().contains("revid") ||
            !element.attributes().contains("type") ||
            !element.attributes().contains("title") ||
            !element.attributes().contains("user"))
        {
            goto invalid;
        }

        // if server name doesn't match we drop edit
        if (this->GetSite()->XmlRcsName != element.attribute("server_name"))
        {
            HUGGLE_DEBUG1("Invalid server: " + this->GetSite()->XmlRcsName + " isn't " + element.attribute("server_name"));
            continue;
        }

        // now we can create an edit
        edit = new WikiEdit();
        edit->Page = new WikiPage(element.attribute("title"));
        edit->Page->Site = this->GetSite();
        edit->IncRef();
        edit->Bot = Generic::SafeBool(element.attribute("bot"));
        edit->NewPage = (element.attribute("type") == "new");
        edit->Minor = Generic::SafeBool(element.attribute("minor"));
        edit->RevID = element.attribute("revid").toLong();
        edit->User = new WikiUser(element.attribute("user"));
        edit->User->Site = this->GetSite();
        edit->Summary = element.attribute("summary");
        if (element.attributes().contains("length_new")
               && element.attributes().contains("length_old"))
        {
            long size = element.attribute("length_new").toLong() - element.attribute("length_old").toLong();
            edit->SetSize(size);
        }
        edit->OldID = element.attribute("oldid").toInt();
        ts.setTime_t(element.attribute("timestamp").toUInt());
        this->InsertEdit(edit);
        continue;

        invalid:
            Syslog::HuggleLogs->WarningLog("Invalid Xml from RC feed: " + data);
    }
}
