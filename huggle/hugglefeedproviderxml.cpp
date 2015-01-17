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
    this->NetworkSocket = nullptr;
}

HuggleFeedProviderXml::~HuggleFeedProviderXml()
{
    this->Stop();
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
    if (this->GetSite()->XmlRcsName.isEmpty())
    {
        HUGGLE_DEBUG1("There is no XmlRpc provider for " + this->GetSite()->Name);
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
    if (!this->NetworkSocket)
        throw new Huggle::NullPointerException("this->NetworkSocket", BOOST_CURRENT_FUNCTION);
    QString data(this->NetworkSocket->readLine());
    // when there is no data we can quit this
    if (data.isEmpty())
        return;

    // this should be an XML string, let's do some quick test
    if (!data.startsWith("<"))
    {
        Syslog::HuggleLogs->WarningLog("Invalid input from XmlRcs server: " + data);
        return;
    }

    QDomDocument input;
    input.setContent(data);
    QDomElement element = input.firstChild().toElement();
    QString name = element.nodeName();
    if (name == "error")
    {
        Syslog::HuggleLogs->ErrorLog("XmlRcs returned error: " + element.text());
        return;
    }
    if (name == "ping")
    {
        this->Write("pong");
        return;
    }

    if (name == "fatal")
    {
        Syslog::HuggleLogs->ErrorLog("XmlRcs failed: " + element.text());
        this->Stop();
        return;
    }

    if (name == "ok")
        return;
    if (name != "edit")
    {
        HUGGLE_DEBUG1("Weird result from xml provider: " + data);
        return;
    }

    WikiEdit *edit;
    QString type;

    if (!element.attributes().contains("type"))
        goto invalid;

    type = element.attribute("type");
    if (type != "edit" && type != "new")
    {
        // we are not interested in this
        return;
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
        HUGGLE_DEBUG1("Invalid server: " + this->GetSite()->XmlRcsName + " isn't' " + element.attribute("server_name"));
        return;
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
    this->InsertEdit(edit);
    return;

    invalid:
        Syslog::HuggleLogs->WarningLog("Invalid Xml from RC feed: " + data);
}

void HuggleFeedProviderXml::OnConnect()
{
    this->is_connected = true;
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
    if (MainWindow::HuggleMain->Queue1->CurrentFilter->Matches(edit))
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
