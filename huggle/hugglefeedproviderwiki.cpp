//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "hugglefeedproviderwiki.hpp"

using namespace Huggle;

HuggleFeedProviderWiki::HuggleFeedProviderWiki()
{
    this->Buffer = new QList<WikiEdit*>();
    this->Refreshing = false;
    this->qReload = NULL;
    // we set the latest time to yesterday so that we don't get in troubles with time offset
    this->LatestTime = QDateTime::currentDateTime().addDays(-1);
    this->LastRefresh = QDateTime::currentDateTime().addDays(-1);
}

HuggleFeedProviderWiki::~HuggleFeedProviderWiki()
{
    while (this->Buffer->count() > 0)
    {
        this->Buffer->at(0)->DecRef();
        this->Buffer->removeAt(0);
    }
    delete this->Buffer;
    if (this->qReload != NULL)
    {
        this->qReload->DecRef();
    }
}

bool HuggleFeedProviderWiki::Start()
{
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
    if (this->Buffer->size() == 0)
    {
        if (this->LastRefresh.addSecs(6) < QDateTime::currentDateTime())
        {
            this->Refresh();
            this->LastRefresh = QDateTime::currentDateTime();
        }
        return false;
    }
    return true;
}

void HuggleFeedProviderWiki::Refresh()
{
    if (this->Refreshing)
    {
        // the query is still in progress now
        if (!this->qReload->IsProcessed())
            return;
        if (this->qReload->Result->Failed)
        {
            // failed to obtain the data
            Huggle::Syslog::HuggleLogs->Log(Localizations::HuggleLocalizations->Localize("rc-error",
                                                              this->qReload->Result->ErrorMessage));
            this->qReload->DecRef();
            this->qReload = NULL;
            this->Refreshing = false;
            return;
        }
        this->Process(qReload->Result->Data);
        this->qReload->DecRef();
        this->qReload = NULL;
        this->Refreshing = false;
        return;
    }
    this->Refreshing = true;
    this->qReload = new ApiQuery(ActionQuery);
    this->qReload->Parameters = "list=recentchanges&rcprop=" + QUrl::toPercentEncoding("user|userid|comment|flags|timestamp|title|ids|sizes|loginfo") +
                                "&rcshow=" + QUrl::toPercentEncoding("!bot") + "&rclimit=200";
    this->qReload->Target = "Recent changes refresh";
    this->qReload->IncRef();
    QueryPool::HugglePool->AppendQuery(this->qReload);
    this->qReload->Process();
}

WikiEdit *HuggleFeedProviderWiki::RetrieveEdit()
{
    if (this->Buffer->size() < 1)
    {
        return NULL;
    }
    WikiEdit *edit = this->Buffer->at(0);
    this->Buffer->removeAt(0);
    return edit;
}

QString HuggleFeedProviderWiki::ToString()
{
    return "Wiki";
}

void HuggleFeedProviderWiki::Process(QString data)
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
    QDateTime t = this->LatestTime;
    bool Changed = false;
    while (CurrentNode > 0)
    {
        CurrentNode--;
        // get a time of rc change
        QDomElement item = l.at(CurrentNode).toElement();
        if (item.nodeName() != "rc")
        {
            CurrentNode--;
            continue;
        }
        if (!item.attributes().contains("timestamp"))
        {
            Huggle::Syslog::HuggleLogs->Log("RC Feed: Item was missing timestamp attribute: " + item.toElement().nodeName());
            CurrentNode--;
            continue;
        }
        QDateTime time = MediaWiki::FromMWTimestamp(item.attribute("timestamp"));
        if (time < t)
        {
            // this record is older than latest parsed record, so we don't want to parse it
            CurrentNode--;
            continue;
        } else
        {
            Changed = true;
            t = time;
        }
        if (!item.attributes().contains("type"))
        {
            Huggle::Syslog::HuggleLogs->Log("RC Feed: Item was missing type attribute: " + item.text());
            CurrentNode--;
            continue;
        }
        if (!item.attributes().contains("title"))
        {
            Huggle::Syslog::HuggleLogs->Log("RC Feed: Item was missing title attribute: " + item.text());
            CurrentNode--;
            continue;
        }
        QString type = item.attribute("type");
        if (type == "edit" || type == "new")
        {
            ProcessEdit(item);
        }
        else if (type == "log")
        {
            ProcessLog(item);
        }
        CurrentNode--;
    }
    if (Changed)
    {
        this->LatestTime = t.addSecs(1);
    }
}

void HuggleFeedProviderWiki::ProcessEdit(QDomElement item)
{
    WikiEdit *edit = new WikiEdit();
    edit->Page = new WikiPage(item.attribute("title"));
    QString type = item.attribute("type");
    if (type == "new")
        edit->NewPage = true;
    if (item.attributes().contains("newlen") && item.attributes().contains("oldlen"))
        edit->Size = item.attribute("newlen").toInt() - item.attribute("oldlen").toInt();
    if (item.attributes().contains("user"))
        edit->User = new WikiUser(item.attribute("user"));
    if (item.attributes().contains("comment"))
        edit->Summary = item.attribute("comment");
    if (item.attributes().contains("bot"))
        edit->Bot = true;
    if (item.attributes().contains("anon"))
        edit->User->ForceIP();
    if (item.attributes().contains("revid"))
    {
        edit->RevID = QString(item.attribute("revid")).toInt();
        if (edit->RevID == 0)
        {
            edit->RevID = WIKI_UNKNOWN_REVID;
        }
    }
    if (item.attributes().contains("minor"))
        edit->Minor = true;
    edit->IncRef();
    this->InsertEdit(edit);
}

void HuggleFeedProviderWiki::ProcessLog(QDomElement item)
{
    /*
     * this function doesn't check if every attribute is present (unlike ProcessEdit())
     *
     * needs loginfo in rcprop at apiquery
     */
    QString logtype = item.attribute("logtype");
    QString logaction = item.attribute("logaction");

    if (logtype == "block" && (logaction == "block" || logaction == "reblock") )
    {
        QString admin = item.attribute("user");
        QString blockeduser = item.attribute("title"); // including User-namespaceprefix
        QString reason = item.attribute("comment");
        if (logaction == "block" || logaction == "reblock")
        {
            QDomElement blockinfo = item.elementsByTagName("block").at(0).toElement(); // nested element "block"
            //QString flags = blockinfo.attribute("flags");
            QString duration = blockinfo.attribute("duration");

            Huggle::Syslog::HuggleLogs->DebugLog("RC Feed: ProcessLog: " + blockeduser + " was blocked by " + admin +
                                                 " for the duration \"" + duration + "\": " + reason);
        }
        else if (logaction == "unblock")
        {
            Huggle::Syslog::HuggleLogs->DebugLog("RC Feed: ProcessLog: " + blockeduser + " was unblocked by " + admin + ": " + reason);
        }
        // TODO: process it further to the user so edits get displayed as blob-blocked.png or not any longer
    }
    else if (logtype == "delete")
    {
        QString page = item.attribute("title");
        QString admin = item.attribute("user");
        QString reason = item.attribute("comment");
        Huggle::Syslog::HuggleLogs->DebugLog("RC Feed: ProcessLog: page \"" + page + "\" was deleted by " + admin + ": " + reason);
        // TODO: process page deletes further (e.g. remove page from queue)
    }
}

void HuggleFeedProviderWiki::InsertEdit(WikiEdit *edit)
{
    this->EditCounter++;
    QueryPool::HugglePool->PreProcessEdit(edit);
    if (MainWindow::HuggleMain->Queue1->CurrentFilter->Matches(edit))
    {
        if (this->Buffer->size() > Configuration::HuggleConfiguration->SystemConfig_ProviderCache)
        {
            while (this->Buffer->size() > (Configuration::HuggleConfiguration->SystemConfig_ProviderCache - 10))
            {
                this->Buffer->at(0)->DecRef();
                this->Buffer->removeAt(0);
            }
            Huggle::Syslog::HuggleLogs->Log("WARNING: insufficient space in wiki cache, increase ProviderCache size, otherwise you will be loosing edits");
        }
        this->Buffer->append(edit);
    } else
    {
        edit->DecRef();
    }
}
