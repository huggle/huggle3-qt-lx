//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "hugglefeedproviderwiki.h"

HuggleFeedProviderWiki::HuggleFeedProviderWiki()
{
    Buffer = new QList<WikiEdit>();
    Refreshing = false;
    q = NULL;
    // we set the latest time to yesterday so that we don't get in troubles with time offset
    LatestTime = QDateTime::currentDateTime().addDays(-1);
    LastRefresh = QDateTime::currentDateTime().addDays(-1);
}

HuggleFeedProviderWiki::~HuggleFeedProviderWiki()
{
    delete q;
    delete Buffer;
}

bool HuggleFeedProviderWiki::Start()
{
    Refresh();
    return true;
}

bool HuggleFeedProviderWiki::IsWorking()
{
    return true;
}

void HuggleFeedProviderWiki::Stop()
{

}

bool HuggleFeedProviderWiki::ContainsEdit()
{
    if (this->Buffer->size() == 0)
    {
        if (!(this->LastRefresh.addSecs(6) > QDateTime::currentDateTime()))
        {
            Refresh();
            this->LastRefresh = QDateTime::currentDateTime();
        }
        return false;
    }
    return true;
}

void HuggleFeedProviderWiki::Refresh()
{
    if (Refreshing)
    {
        // the query is still in progress now
        if (!q->Processed())
        {
            return;
        }
        if (q->Result->Failed)
        {
            // failed to obtain the data
            Core::Log("Unable to retrieve data from wiki feed, last error: " + q->Result->ErrorMessage);
            q->DeleteLater = false;
            this->q = NULL;
            Refreshing = false;
            return;
        }
        this->Process(q->Result->Data);
        q->DeleteLater = false;
        this->q = NULL;
        Refreshing = false;
        return;
    }

    Refreshing = true;
    q = new ApiQuery();
    q->SetAction(ActionQuery);
    q->Parameters = "list=recentchanges&rcprop=user|userid|comment|flags|timestamp|title|ids|sizes&rcshow=!bot&rclimit=200";
    q->Target = "Recent changes refresh";
    q->DeleteLater = true;
    Core::RunningQueries.append(q);
    q->Process();
}

WikiEdit *HuggleFeedProviderWiki::RetrieveEdit()
{
    if (this->Buffer->size() < 1)
    {
        return NULL;
    }
    WikiEdit *edit = new WikiEdit(this->Buffer->at(0));
    this->Buffer->removeAt(0);
    Core::PostProcessEdit(edit);
    return edit;
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
        Core::Log("Error, wiki provider returned: " + data);
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
            Core::Log("RC Feed: Item was missing timestamp attribute: " + item.toElement().nodeName());
            CurrentNode--;
            continue;
        }

        QDateTime time = QDateTime::fromString(item.attribute("timestamp"), "yyyy-MM-ddThh:mm:ssZ");

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
            Core::Log("RC Feed: Item was missing type attribute: " + item.text());
            CurrentNode--;
            continue;
        }

        QString type = item.attribute("type");

        if (type != "edit" && type != "new")
        {
            CurrentNode--;
            continue;
        }

        if (!item.attributes().contains("title"))
        {
            Core::Log("RC Feed: Item was missing title attribute: " + item.text());
            CurrentNode--;
            continue;
        }

        WikiEdit edit;
        edit.Page = new WikiPage(item.attribute("title"));

        if (type == "new")
        {
            edit.NewPage = true;
        }

        if (item.attributes().contains("newlen") && item.attributes().contains("oldlen"))
        {
            edit.Size = item.attribute("newlen").toInt() - item.attribute("oldlen").toInt();
        }

        if (item.attributes().contains("user"))
        {
            edit.User = new WikiUser(item.attribute("user"));
        }

        if (item.attributes().contains("comment"))
        {
            edit.Summary = item.attribute("comment");
        }

        if (item.attributes().contains("bot"))
        {
            edit.Bot = true;
        }

        if (item.attributes().contains("anon"))
        {
            edit.User->IP = true;
        }

        if (item.attributes().contains("minor"))
        {
            edit.Minor = true;
        }

        this->InsertEdit(edit);

        CurrentNode--;
    }
    if (Changed)
    {
        this->LatestTime = t.addSecs(1);
    }
}

void HuggleFeedProviderWiki::InsertEdit(WikiEdit edit)
{
    Core::PreProcessEdit(&edit);
    if (Core::Main->Queue1->CurrentFilter->Matches(edit))
    {
        while (this->Buffer->size() > Configuration::ProviderCache)
        {
            this->Buffer->removeAt(0);
        }
        this->Buffer->append(edit);
    }
}
