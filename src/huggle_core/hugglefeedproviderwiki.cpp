//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "hugglefeedproviderwiki.hpp"
#include <QtXml>
#include "configuration.hpp"
#include "exception.hpp"
#include "hugglequeuefilter.hpp"
#include "localization.hpp"
#include "mediawiki.hpp"
#include "hooks.hpp"
#include "querypool.hpp"
#include "syslog.hpp"
#include "wikipage.hpp"
#include "wikisite.hpp"
#include "wikiuser.hpp"

using namespace Huggle;
HuggleFeedProviderWiki::HuggleFeedProviderWiki(WikiSite *site) : HuggleFeed(site)
{
    this->editBuffer = new QList<WikiEdit*>();
    this->Site = site;
    this->isRefreshing = false;
    // we set the latest time to yesterday so that we don't get in troubles with time offset
    this->latestTime = QDateTime::currentDateTime().addDays(-1);
    this->lastRefresh = QDateTime::currentDateTime().addDays(-1);
}

HuggleFeedProviderWiki::~HuggleFeedProviderWiki()
{
    while (this->editBuffer->count() > 0)
    {
        this->editBuffer->at(0)->DecRef();
        this->editBuffer->removeAt(0);
    }
    delete this->editBuffer;
}

bool HuggleFeedProviderWiki::Start()
{
    if (this->IsPaused())
        this->startupTime = QDateTime::currentDateTime();
    this->Resume();
    this->Refresh();
    return true;
}

bool HuggleFeedProviderWiki::IsWorking()
{
    return true;
}

void HuggleFeedProviderWiki::Stop()
{
    this->Pause();
}

bool HuggleFeedProviderWiki::ContainsEdit()
{
    if (this->editBuffer->isEmpty())
    {
        if (this->lastRefresh.addSecs(6) < QDateTime::currentDateTime())
        {
            this->Refresh();
            this->lastRefresh = QDateTime::currentDateTime();
        }
        return false;
    }
    return true;
}

void HuggleFeedProviderWiki::Refresh()
{
    if (this->isPaused)
        return;
    if (this->isRefreshing)
    {
        // the query is still in progress now
        if (!this->qReload->IsProcessed())
            return;
        if (this->qReload->IsFailed())
        {
            // failed to obtain the data
            Huggle::Syslog::HuggleLogs->Log(_l("rc-error", this->qReload->GetFailureReason()));
            this->qReload = nullptr;
            this->isRefreshing = false;
            return;
        }
        this->processData(qReload->Result->Data);
        this->qReload = nullptr;
        this->isRefreshing = false;
        return;
    }
    this->isRefreshing = true;
    this->qReload = new ApiQuery(ActionQuery, this->GetSite());
    this->qReload->Parameters = "list=recentchanges&rcprop=" + QUrl::toPercentEncoding("user|userid|comment|flags|timestamp|title|ids|sizes|loginfo") +
                                "&rclimit=" + QString::number(Configuration::HuggleConfiguration->SystemConfig_WikiRC);
    this->qReload->Target = "Recent changes refresh";
    QueryPool::HugglePool->AppendQuery(this->qReload);
    this->qReload->Process();
}

unsigned long long HuggleFeedProviderWiki::GetBytesReceived()
{
    return 0;
}

unsigned long long HuggleFeedProviderWiki::GetBytesSent()
{
    return 0;
}

WikiEdit *HuggleFeedProviderWiki::RetrieveEdit()
{
    if (this->editBuffer->size() < 1)
    {
        return nullptr;
    }
    WikiEdit *edit = this->editBuffer->at(0);
    this->editBuffer->removeAt(0);
    return edit;
}

QString HuggleFeedProviderWiki::ToString()
{
    return "Wiki";
}

void HuggleFeedProviderWiki::processData(QString data)
{
    //QStringList lines = data.split("\n");
    QDomDocument d;
    d.setContent(data);
    QDomNodeList l = d.elementsByTagName("rc");
    int CurrentNode = l.count();
    if (l.count() == 0)
    {
        Huggle::Syslog::HuggleLogs->Log("Error, wiki provider returned: " + data);
        return;
    }
    // recursively scan all RC changes
    QDateTime t = this->latestTime;
    bool Changed = false;
    while (CurrentNode > 0)
    {
        CurrentNode--;
        // get a time of rc change
        QDomElement item = l.at(CurrentNode).toElement();
        if (!item.attributes().contains("timestamp"))
        {
            Huggle::Syslog::HuggleLogs->Log(_l("rc-timestamp-missing", item.toElement().nodeName()));
            continue;
        }
        QDateTime time = MediaWiki::FromMWTimestamp(item.attribute("timestamp"));
        if (time < t)
        {
            // this record is older than latest parsed record, so we don't want to parse it
            continue;
        } else
        {
            Changed = true;
            t = time;
        }
        if (!item.attributes().contains("type"))
        {
            Huggle::Syslog::HuggleLogs->Log(_l("rc-type-missing", item.text()));
            continue;
        }
        if (!item.attributes().contains("title"))
        {
            Huggle::Syslog::HuggleLogs->Log(_l("rc-title-missing", item.text()));
            continue;
        }
        QString type = item.attribute("type");
        if (type == "edit" || type == "new")
        {
            processEdit(item);
        }
        else if (type == "log")
        {
            processLog(item);
        }
    }
    if (Changed)
    {
        this->latestTime = t.addSecs(1);
    }
}

void HuggleFeedProviderWiki::processEdit(QDomElement item)
{
    WikiEdit *edit = new WikiEdit();
    edit->Page = new WikiPage(item.attribute("title"), this->GetSite());
    QString type = item.attribute("type");
    if (type == "new")
        edit->NewPage = true;
    if (item.attributes().contains("newlen") && item.attributes().contains("oldlen"))
        edit->SetSize(item.attribute("newlen").toLong() - item.attribute("oldlen").toLong());
    if (item.attributes().contains("user"))
    {
        edit->User = new WikiUser(item.attribute("user"), this->GetSite());
        if (item.attributes().contains("anon"))
            edit->User->ForceIP();
    }
    if (item.attributes().contains("comment"))
        edit->Summary = item.attribute("comment");
    if (item.attributes().contains("bot"))
        edit->Bot = true;
    if (item.attributes().contains("revid"))
    {
        edit->RevID = QString(item.attribute("revid")).toInt();
        if (!edit->RevID)
            edit->RevID = WIKI_UNKNOWN_REVID;

    }
    if (item.attributes().contains("minor"))
        edit->IsMinor = true;
    edit->IncRef();
    this->insertEdit(edit);
}

void HuggleFeedProviderWiki::processLog(const QDomElement& item)
{
    /*
     * this function doesn't check if every attribute is present (unlike ProcessEdit())
     *
     * needs loginfo in rcprop at apiquery
     */
    QString logtype = item.attribute("logtype");
    QString logaction = item.attribute("logaction");

    if (logtype == "block" && (logaction == "block" || logaction == "reblock"))
    {
        QString admin = item.attribute("user");
        QString blockeduser = item.attribute("title"); // including User-namespaceprefix
        QString reason = item.attribute("comment");
        if (logaction == "block" || logaction == "reblock")
        {
            QDomElement blockinfo = item.elementsByTagName("block").at(0).toElement(); // nested element "block"
            //QString flags = blockinfo.attribute("flags");
            QString duration = blockinfo.attribute("duration");
            HUGGLE_DEBUG("RC Feed: ProcessLog: " + blockeduser + " was blocked by " + admin +
                          " for the duration \"" + duration + "\": " + reason, 1);
        }
        else if (logaction == "unblock")
        {
            HUGGLE_DEBUG("RC Feed: ProcessLog: " + blockeduser + " was unblocked by " + admin + ": " + reason, 1);
        }
        // TODO: process it further to the user so edits get displayed as blob-blocked.png or not any longer
    }
    else if (logtype == "delete")
    {
        QString page = item.attribute("title");
        QString admin = item.attribute("user");
        QString reason = item.attribute("comment");
        HUGGLE_DEBUG("RC Feed: ProcessLog: page \"" + page + "\" was deleted by " + admin + ": " + reason, 1);
        // TODO: process page deletes further (e.g. remove page from queue)
    }
}

void HuggleFeedProviderWiki::insertEdit(WikiEdit *edit)
{
    this->IncrementEdits();
    QueryPool::HugglePool->PreProcessEdit(edit);
    if (edit->GetSite()->CurrentFilter->Matches(edit) && Hooks::EditBeforePreProcess(edit))
    {
        if (this->editBuffer->size() > Configuration::HuggleConfiguration->SystemConfig_ProviderCache)
        {
            while (this->editBuffer->size() > (Configuration::HuggleConfiguration->SystemConfig_ProviderCache - 10))
            {
                this->editBuffer->at(0)->DecRef();
                this->editBuffer->removeAt(0);
            }
            Huggle::Syslog::HuggleLogs->WarningLog("insufficient space in wiki cache, increase ProviderCache size, otherwise you will be losing edits");
        }
        this->editBuffer->append(edit);
    } else
    {
        edit->DecRef();
    }
}
