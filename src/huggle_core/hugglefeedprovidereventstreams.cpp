//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "hugglefeedprovidereventstreams.hpp"
#include "configuration.hpp"
#include "exception.hpp"
#include "generic.hpp"
#include "hooks.hpp"
#include "hugglequeuefilter.hpp"
#include "querypool.hpp"
#include "syslog.hpp"
#include "wikiedit.hpp"
#include "wikipage.hpp"
#include "wikisite.hpp"
#include "wikiuser.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QVariant>

using namespace Huggle;

static const char *EVENTSTREAMS_RECENTCHANGE_URL = "https://stream.wikimedia.org/v2/stream/recentchange";

HuggleFeedProviderEventStreams::HuggleFeedProviderEventStreams(WikiSite *site) : HuggleFeed(site)
{
    this->manager = new QNetworkAccessManager(this);
    this->watchdog = new QTimer(this);
    connect(this->watchdog, &QTimer::timeout, this, &HuggleFeedProviderEventStreams::onWatchdog);
}

HuggleFeedProviderEventStreams::~HuggleFeedProviderEventStreams()
{
    this->Stop();
    while (this->buffer.count() > 0)
    {
        this->buffer.at(0)->DecRef();
        this->buffer.removeAt(0);
    }
}

bool HuggleFeedProviderEventStreams::Start()
{
    if (this->IsWorking())
    {
        HUGGLE_DEBUG1("Refusing to start working EventStreams feed");
        return false;
    }
    if (this->GetSite()->XmlRcsName.isEmpty())
    {
        Syslog::HuggleLogs->ErrorLog("There is no EventStreams server_name for " + this->GetSite()->Name);
        return false;
    }

    this->pendingData.clear();
    this->lastProgress = QDateTime::currentDateTime();
    QNetworkRequest request;
    request.setUrl(QUrl(EVENTSTREAMS_RECENTCHANGE_URL));
    request.setRawHeader("Accept", "text/event-stream");
    request.setRawHeader("Cache-Control", "no-cache");
    request.setRawHeader("User-Agent", QString("Huggle/%1 (native EventStreams provider)").arg(Configuration::HuggleConfiguration->HuggleVersion).toUtf8());
    if (!this->lastEventID.isEmpty())
        request.setRawHeader("Last-Event-ID", this->lastEventID.toUtf8());

    this->reply = this->manager->get(request);
    connect(this->reply, &QNetworkReply::readyRead, this, &HuggleFeedProviderEventStreams::onReceive);
    connect(this->reply, &QNetworkReply::finished, this, &HuggleFeedProviderEventStreams::onFinished);
#ifdef QT6_BUILD
    connect(this->reply, &QNetworkReply::errorOccurred, this, &HuggleFeedProviderEventStreams::onError);
#else
    connect(this->reply,
            static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
            this,
            &HuggleFeedProviderEventStreams::onError);
#endif
    this->isWorking = true;
    this->watchdog->start(10000);
    return true;
}

bool HuggleFeedProviderEventStreams::IsPaused()
{
    return this->isPaused;
}

void HuggleFeedProviderEventStreams::Resume()
{
    this->isPaused = false;
}

void HuggleFeedProviderEventStreams::Pause()
{
    this->isPaused = true;
}

bool HuggleFeedProviderEventStreams::IsWorking()
{
    return this->isWorking && this->reply && !this->reply->isFinished();
}

void HuggleFeedProviderEventStreams::Stop()
{
    this->reconnecting = false;
    this->watchdog->stop();
    this->isWorking = false;
    this->isPaused = false;
    if (this->reply)
    {
        QObject::disconnect(this->reply, nullptr, this, nullptr);
        this->reply->abort();
        this->reply->deleteLater();
        this->reply = nullptr;
    }
}

bool HuggleFeedProviderEventStreams::ContainsEdit()
{
    return this->buffer.count() > 0;
}

QString HuggleFeedProviderEventStreams::GetError()
{
    return this->lastError;
}

unsigned long long HuggleFeedProviderEventStreams::GetBytesReceived()
{
    return this->bytesRcvd;
}

unsigned long long HuggleFeedProviderEventStreams::GetBytesSent()
{
    return this->bytesSent;
}

WikiEdit *HuggleFeedProviderEventStreams::RetrieveEdit()
{
    if (this->buffer.empty())
        return nullptr;

    WikiEdit *edit = this->buffer.at(0);
    this->buffer.removeAt(0);
    return edit;
}

QString HuggleFeedProviderEventStreams::ToString()
{
    return "EventStreams";
}

void HuggleFeedProviderEventStreams::onReceive()
{
    if (!this->reply)
        throw new Huggle::NullPointerException("this->reply", BOOST_CURRENT_FUNCTION);

    QByteArray data = this->reply->readAll();
    if (data.isEmpty())
        return;

    this->bytesRcvd += static_cast<unsigned long long>(data.size());
    this->lastProgress = QDateTime::currentDateTime();
    this->pendingData.append(data);
    this->processBufferedData();
}

void HuggleFeedProviderEventStreams::onFinished()
{
    bool shouldReconnect = this->isWorking || this->reconnecting;
    if (this->reply)
    {
        if (this->reply->error() != QNetworkReply::NoError)
            this->lastError = this->reply->errorString();
        QObject::disconnect(this->reply, nullptr, this, nullptr);
        this->reply->deleteLater();
        this->reply = nullptr;
    }
    this->watchdog->stop();
    this->isWorking = false;
    this->reconnecting = false;
    if (shouldReconnect)
        this->Start();
}

void HuggleFeedProviderEventStreams::onError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);
    if (this->reply)
        this->lastError = this->reply->errorString();
}

void HuggleFeedProviderEventStreams::onWatchdog()
{
    if (!this->isWorking)
        return;

    if (this->lastProgress.secsTo(QDateTime::currentDateTime()) > 180)
    {
        Syslog::HuggleLogs->ErrorLog("EventStreams feed has timed out, reconnecting to it");
        this->reconnecting = true;
        this->Restart();
    }
}

void HuggleFeedProviderEventStreams::processBufferedData()
{
    int delimiterLength = 0;
    int delimiter = this->nextEventBlockDelimiter(&delimiterLength);
    while (delimiter >= 0)
    {
        QByteArray block = this->pendingData.left(delimiter);
        this->pendingData.remove(0, delimiter + delimiterLength);
        this->processEventBlock(block);
        delimiter = this->nextEventBlockDelimiter(&delimiterLength);
    }
}

void HuggleFeedProviderEventStreams::processEventBlock(const QByteArray &block)
{
    QList<QByteArray> lines = block.split('\n');
    QByteArray eventName = "message";
    QByteArray data;
    foreach (QByteArray line, lines)
    {
        if (line.endsWith('\r'))
            line.chop(1);
        if (line.startsWith(':'))
            continue;
        if (line.startsWith("event:"))
            eventName = line.mid(6).trimmed();
        else if (line.startsWith("id:"))
            this->lastEventID = QString::fromUtf8(line.mid(3).trimmed());
        else if (line.startsWith("data:"))
        {
            if (!data.isEmpty())
                data.append('\n');
            data.append(line.mid(5).trimmed());
        }
    }

    if (eventName == "message" && !data.isEmpty())
        this->processMessage(data);
    else if (eventName == "error")
        this->lastError = QString::fromUtf8(data);
}

void HuggleFeedProviderEventStreams::processMessage(const QByteArray &data)
{
    if (this->IsPaused())
        return;

    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject())
    {
        Syslog::HuggleLogs->WarningLog("Invalid JSON from EventStreams: " + parseError.errorString());
        return;
    }

    QJsonObject change = document.object();
    QJsonObject meta = change.value("meta").toObject();
    if (meta.value("domain").toString() == "canary")
        return;

    if (change.value("server_name").toString() != this->GetSite()->XmlRcsName)
        return;

    QString type = change.value("type").toString();
    if (type != "edit" && type != "new")
        return;

    QString title = change.value("title").toString();
    QString user = change.value("user").toString();
    if (title.isEmpty() || user.isEmpty())
    {
        Syslog::HuggleLogs->WarningLog("Invalid EventStreams recentchange event: missing title or user");
        return;
    }

    WikiEdit *edit = new WikiEdit();
    edit->Page = new WikiPage(title, this->GetSite());
    edit->IncRef();
    edit->User = new WikiUser(user, this->GetSite());
    edit->Bot = change.value("bot").toBool();
    edit->NewPage = (type == "new");
    edit->IsMinor = change.value("minor").toBool();
    edit->Summary = change.value("comment").toString();

    QJsonObject revision = change.value("revision").toObject();
    edit->RevID = this->jsonRevisionId(revision.value("new"));
    edit->Diff = edit->RevID;
    edit->OldID = this->jsonRevisionId(revision.value("old"));

    QJsonObject length = change.value("length").toObject();
    if (length.contains("new") && length.contains("old"))
        edit->SetSize(static_cast<long>(length.value("new").toInt() - length.value("old").toInt()));

    this->insertEdit(edit);
}

void HuggleFeedProviderEventStreams::insertEdit(WikiEdit *edit)
{
    if (edit == nullptr)
        throw new Huggle::NullPointerException("WikiEdit *edit", BOOST_CURRENT_FUNCTION);

    this->IncrementEdits();
    QueryPool::HugglePool->PreProcessEdit(edit);
    if (edit->GetSite()->CurrentFilter->Matches(edit) && Hooks::EditBeforePreProcess(edit))
    {
        if (this->buffer.size() > hcfg->SystemConfig_ProviderCache)
        {
            while (this->buffer.size() > (hcfg->SystemConfig_ProviderCache - 10))
            {
                this->buffer.at(0)->DecRef();
                this->buffer.removeAt(0);
            }
            if (!this->IsPaused())
                Huggle::Syslog::HuggleLogs->WarningLog("insufficient space in eventstreams cache, increase ProviderCache size, otherwise you will be losing edits");
        }
        this->buffer.append(edit);
    } else
    {
        edit->DecRef();
    }
}

int HuggleFeedProviderEventStreams::nextEventBlockDelimiter(int *delimiterLength) const
{
    int lf = this->pendingData.indexOf("\n\n");
    int crlf = this->pendingData.indexOf("\r\n\r\n");
    int cr = this->pendingData.indexOf("\r\r");
    int best = -1;
    int length = 0;

    if (lf >= 0)
    {
        best = lf;
        length = 2;
    }
    if (crlf >= 0 && (best < 0 || crlf < best))
    {
        best = crlf;
        length = 4;
    }
    if (cr >= 0 && (best < 0 || cr < best))
    {
        best = cr;
        length = 2;
    }
    if (delimiterLength)
        *delimiterLength = length;
    return best;
}

revid_ht HuggleFeedProviderEventStreams::jsonRevisionId(const QJsonValue &value) const
{
    if (value.isUndefined() || value.isNull())
        return 0;
    if (value.isString())
        return value.toString().toLongLong();
    return value.toVariant().toLongLong();
}
