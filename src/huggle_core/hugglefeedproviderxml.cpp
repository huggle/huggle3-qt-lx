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
#include "hugglequeuefilter.hpp"
#include "generic.hpp"
#include "wikipage.hpp"
#include "hooks.hpp"
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
    this->networkSocket = nullptr;
}

HuggleFeedProviderXml::~HuggleFeedProviderXml()
{
    this->Stop();
    delete this->pinger;
    while (this->buffer.count() > 0)
    {
        this->buffer.at(0)->DecRef();
        this->buffer.removeAt(0);
    }
    if (this->networkSocket && this->networkSocket->isOpen())
        this->networkSocket->close();
    delete this->networkSocket;
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
    this->lastPong = QDateTime::currentDateTime().addSecs(22);
    if (this->GetSite()->XmlRcsName.isEmpty())
    {
        Syslog::HuggleLogs->ErrorLog("There is no XmlRcs provider for " + this->GetSite()->Name);
        return false;
    }
    delete this->networkSocket;
    this->networkSocket = new QTcpSocket();
    connect(this->networkSocket, SIGNAL(readyRead()), this, SLOT(OnReceive()));
    connect(this->networkSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(OnError(QAbstractSocket::SocketError)));
    this->isConnecting = true;
    this->isWorking = true;
    this->networkSocket->connectToHost(hcfg->GlobalConfig_Xmlrcs, hcfg->GlobalConfig_XmlrcsPort);
    // we need to handle the connection later
    connect(this->networkSocket, SIGNAL(connected()), this, SLOT(OnConnect()));
    return true;
}

bool HuggleFeedProviderXml::IsPaused()
{
    return this->isPaused;
}

void HuggleFeedProviderXml::Resume()
{
    this->isPaused = false;
}

void HuggleFeedProviderXml::Pause()
{
    this->isPaused = true;
}

void HuggleFeedProviderXml::OnPing()
{
    if (this->isConnected)
    {
        if (QDateTime::currentDateTime().addSecs(-20) > this->lastPong)
        {
            Syslog::HuggleLogs->ErrorLog("XmlRcs feed has timed out, reconnecting to it");
            this->Restart();
        }
        else if (QDateTime::currentDateTime().addSecs(-8) > this->lastPong)
        {
            this->write("ping");
        }
    }
}

bool HuggleFeedProviderXml::IsWorking()
{
    return this->isWorking;
}

void HuggleFeedProviderXml::Stop()
{
    if (this->networkSocket)
        this->networkSocket->disconnect();
    this->isConnected = false;
    this->isConnecting = false;
    this->pinger->stop();
    this->isWorking = false;
    this->isPaused = false;
}

bool HuggleFeedProviderXml::ContainsEdit()
{
    return this->buffer.count() > 0;
}

QString HuggleFeedProviderXml::GetError()
{
    return this->lastError;
}

unsigned long long HuggleFeedProviderXml::GetBytesReceived()
{
    return this->bytesRcvd;
}

unsigned long long HuggleFeedProviderXml::GetBytesSent()
{
    return this->bytesSent;
}

WikiEdit *HuggleFeedProviderXml::RetrieveEdit()
{
    if (this->buffer.empty())
        return nullptr;

    WikiEdit *edit = this->buffer.at(0);
    this->buffer.removeAt(0);
    return edit;
}

QString HuggleFeedProviderXml::ToString()
{
    return "XMLRCS";
}

void HuggleFeedProviderXml::OnError(QAbstractSocket::SocketError er)
{
    this->lastError = Generic::SocketError2Str(er);
    this->Stop();
}

void HuggleFeedProviderXml::OnReceive()
{
    // THIS IS IMPORTANT
    // readLine() has some bugs in Qt so don't use it, this function needs to always use readAll from socket otherwise we get in troubles
    if (!this->networkSocket)
        throw new Huggle::NullPointerException("this->NetworkSocket", BOOST_CURRENT_FUNCTION);
    QByteArray incoming_data = this->networkSocket->readAll();
    QString data(incoming_data);
    // when there is no data we can quit this
    if (data.isEmpty())
        return;

    this->bytesRcvd += static_cast<unsigned long long>(incoming_data.length());

    if (!this->bufferedPart.isEmpty())
        data = this->bufferedPart + data;

    if (data.contains("\n"))
    {
        this->bufferedPart.clear();
        if (data.endsWith("\n"))
        {
            // we received whole block of lines
            this->bufferedLines = data.split("\n");
        } else
        {
            QStringList lines = data.split("\n");
            // last line is has not yet finished
            this->bufferedPart = lines.last();
            lines.removeLast();
            this->bufferedLines = lines;
        }
        this->processBufs();
        return;
    }

    this->bufferedPart = data;
}

void HuggleFeedProviderXml::OnConnect()
{
    this->isConnected = true;
    this->pinger->start(1000);
    this->isConnecting = false;
    // subscribe
    this->write("S " + this->GetSite()->XmlRcsName);
}

void HuggleFeedProviderXml::write(const QString& text)
{
    // check if network socket isn't nullptr
    if (!this->networkSocket)
        throw new Huggle::NullPointerException("this->NetworkSocket", BOOST_CURRENT_FUNCTION);

    QByteArray outgoing_data = QString(text + "\n").toUtf8();
    this->bytesSent += static_cast<unsigned long long>(outgoing_data.size());
    this->networkSocket->write(outgoing_data);
}

void HuggleFeedProviderXml::insertEdit(WikiEdit *edit)
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
        if (this->buffer.size() > hcfg->SystemConfig_ProviderCache)
        {
            // If the buffer is full we need to remove 10 oldest edits
            // we also show a warning to user
            while (this->buffer.size() > (hcfg->SystemConfig_ProviderCache - 10))
            {
                this->buffer.at(0)->DecRef();
                this->buffer.removeAt(0);
            }
            if (!this->IsPaused())
                Huggle::Syslog::HuggleLogs->WarningLog("insufficient space in xml cache, increase ProviderCache size, otherwise you will be losing edits");
        }
        this->buffer.append(edit);
    } else
    {
        edit->DecRef();
    }
}

void HuggleFeedProviderXml::processBufs()
{
    while (!this->bufferedLines.isEmpty())
    {
        QString data = this->bufferedLines.at(0);
        this->bufferedLines.removeAt(0);
        if (data.isEmpty())
            continue;
        // this should be an XML string, let's do some quick test
        if (!data.startsWith("<"))
        {
            Syslog::HuggleLogs->WarningLog("Invalid input from XmlRcs server: " + data);
            continue;
        }

        // every message will update last time
        this->lastPong = QDateTime::currentDateTime();
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
            this->write("pong");
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

        if (this->IsPaused())
            continue;

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
        edit->Page = new WikiPage(element.attribute("title"), this->GetSite());
        edit->IncRef();
        edit->Bot = Generic::SafeBool(element.attribute("bot"));
        edit->NewPage = (element.attribute("type") == "new");
        edit->IsMinor = Generic::SafeBool(element.attribute("minor"));
        edit->RevID = element.attribute("revid").toLong();
        edit->User = new WikiUser(element.attribute("user"), this->GetSite());
        edit->Summary = element.attribute("summary");
        if (element.attributes().contains("length_new")
               && element.attributes().contains("length_old"))
        {
            long size = element.attribute("length_new").toLong() - element.attribute("length_old").toLong();
            edit->SetSize(size);
        }
        edit->OldID = element.attribute("oldid").toInt();
        ts.setTime_t(element.attribute("timestamp").toUInt());
        this->insertEdit(edit);
        continue;

        invalid:
            Syslog::HuggleLogs->WarningLog("Invalid Xml from RC feed: " + data);
    }
}
