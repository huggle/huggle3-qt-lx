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
#include "exception.hpp"
#include "generic.hpp"
#include "syslog.hpp"
#include "wikiedit.hpp"
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
}

bool HuggleFeedProviderXml::Start()
{
    if (this->IsWorking())
    {
        HUGGLE_DEBUG1("Refusing to start working Xml feed");
        return false;
    }
    if (this->GetSite()->XmlRpcName.isEmpty())
    {
        HUGGLE_DEBUG1("There is no XmlRpc provider for " + this->GetSite()->Name);
        return false;
    }
    if (this->NetworkSocket != nullptr)
        delete this->NetworkSocket;
    this->NetworkSocket = new QTcpSocket();
    connect(this->NetworkSocket, SIGNAL(readyRead()), this, SLOT(OnReceive()));
    this->local_thread->__IsConnecting = true;
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

}

void HuggleFeedProviderXml::Pause()
{

}

bool HuggleFeedProviderXml::IsWorking()
{
    return this->is_working;
}

void HuggleFeedProviderXml::Stop()
{

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
    return nullptr;
}

QString HuggleFeedProviderXml::ToString()
{
    return "XMLRCS";
}

void HuggleFeedProviderXml::OnError(QAbstractSocket::SocketError er)
{
    this->last_error = Generic::SocketError2Str(er);
}

void HuggleFeedProviderXml::OnReceive()
{

}

void HuggleFeedProviderXml::OnTime()
{

}

void HuggleFeedProviderXml::OnConnect()
{
    this->is_connected = true;
    // subscribe
    this->Write("S " + this->GetSite()->XmlRpcName);
}

void HuggleFeedProviderXml::Write(QString text)
{
    this->NetworkSocket->write(QString(text + "\n").toUtf8());
}

HuggleFeedProviderXml_thread::HuggleFeedProviderXml_thread()
{

}

HuggleFeedProviderXml_thread::~HuggleFeedProviderXml_thread()
{

}

bool HuggleFeedProviderXml_thread::IsFinished()
{
    return this->isFinished();
}

void HuggleFeedProviderXml_thread::run()
{

}
