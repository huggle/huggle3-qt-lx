//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "editquery.hpp"
#include "configuration.hpp"
#include "historyitem.hpp"
#include "localization.hpp"
#include "mainwindow.hpp"
#include "syslog.hpp"
#include "querypool.hpp"

using namespace Huggle;

EditQuery::EditQuery()
{
    this->Summary = "";
    this->qEdit = nullptr;
    this->Minor = false;
    this->Page = "";
    this->qToken = nullptr;
    this->Section = 0;
    this->BaseTimestamp = "";
    this->StartTimestamp = "";
    this->text = "";
    this->Type = QueryEdit;
}

EditQuery::~EditQuery()
{
    if (this->qToken != nullptr)
        this->qToken->UnregisterConsumer(HUGGLECONSUMER_EDITQUERY);

    GC_DECREF(this->HI);
}

void EditQuery::Process()
{
    this->Status = StatusProcessing;
    this->StartTime = QDateTime::currentDateTime();
    if (Configuration::HuggleConfiguration->TemporaryConfig_EditToken.isEmpty())
    {
        this->qToken = new ApiQuery(ActionQuery);
        this->qToken->Parameters = "prop=info&intoken=edit&titles=" + QUrl::toPercentEncoding(this->Page);
        this->qToken->Target = _l("editquery-token", this->Page);
        this->qToken->RegisterConsumer(HUGGLECONSUMER_EDITQUERY);
        QueryPool::HugglePool->AppendQuery(this->qToken);
        this->qToken->Process();
    } else
    {
        this->EditPage();
    }
}

bool EditQuery::IsProcessed()
{
    if (this->Result != nullptr)
        return true;

    if (this->qToken != nullptr)
    {
        if (!this->qToken->IsProcessed())
            return false;

        if (this->qToken->Result->Failed)
        {
            this->Result = new QueryResult();
            this->Result->Failed = true;
            this->Result->ErrorMessage = _l("editquery-token-error") + ": " + this->qToken->Result->ErrorMessage;
            this->qToken->UnregisterConsumer(HUGGLECONSUMER_EDITQUERY);
            this->qToken = nullptr;
            return true;
        }
        QDomDocument dToken_;
        dToken_.setContent(this->qToken->Result->Data);
        QDomNodeList l = dToken_.elementsByTagName("page");
        if (l.count() == 0)
        {
            this->Result = new QueryResult();
            this->Result->Failed = true;
            this->Result->ErrorMessage = _l("editquery-token-error");
            Huggle::Syslog::HuggleLogs->DebugLog("Debug message for edit: " + this->qToken->Result->Data);
            this->qToken->UnregisterConsumer(HUGGLECONSUMER_EDITQUERY);
            this->qToken = nullptr;
            return true;
        }
        QDomElement element = l.at(0).toElement();
        if (!element.attributes().contains("edittoken"))
        {
            this->Result = new QueryResult();
            this->Result->Failed = true;
            this->Result->ErrorMessage = _l("editquery-token-error");
            Huggle::Syslog::HuggleLogs->DebugLog("Debug message for edit: " + this->qToken->Result->Data);
            this->qToken->UnregisterConsumer(HUGGLECONSUMER_EDITQUERY);
            this->qToken = nullptr;
            return true;
        }
        Configuration::HuggleConfiguration->TemporaryConfig_EditToken = element.attribute("edittoken");
        this->qToken->UnregisterConsumer(HUGGLECONSUMER_EDITQUERY);
        this->qToken = nullptr;
        this->EditPage();
        return false;
    }
    if (this->qEdit != nullptr)
    {
        if (!this->qEdit->IsProcessed())
            return false;

        QDomDocument dEdit_;
        dEdit_.setContent(this->qEdit->Result->Data);
        QDomNodeList edits_ = dEdit_.elementsByTagName("edit");
        bool failed = true;
        if (edits_.count() > 0)
        {
            QDomElement element = edits_.at(0).toElement();
            if (element.attributes().contains("result"))
            {
                if (element.attribute("result") == "Success")
                {
                    failed = false;
                    if (MainWindow::HuggleMain != nullptr)
                    {
                        HistoryItem *item = new HistoryItem();
                        item->Result = _l("successful");
                        item->Type = HistoryEdit;
                        item->Target = this->Page;
                        item->IncRef();
                        this->HI = item;
                        MainWindow::HuggleMain->_History->Prepend(item);
                    }
                    this->ProcessCallback();
                    Huggle::Syslog::HuggleLogs->Log(_l("editquery-success", this->Page));
                }
            }
        }
        this->Result = new QueryResult();
        if (failed)
        {
            this->Result->Failed = true;
            this->Result->ErrorMessage = this->qEdit->Result->Data;
        }
        this->qEdit->UnregisterConsumer(HUGGLECONSUMER_EDITQUERY);
        this->qEdit = nullptr;
    }
    return true;
}

void EditQuery::EditPage()
{
    this->qEdit = new ApiQuery();
    this->qEdit->Target = "Writing " + this->Page;
    this->qEdit->UsingPOST = true;
    this->qEdit->RegisterConsumer(HUGGLECONSUMER_EDITQUERY);
    this->qEdit->SetAction(ActionEdit);
    QString base = "";
    QString start_ = "";
    QString section = "";
    QString wl = "&watchlist=nochange";
    if (this->InsertTargetToWatchlist)
        wl = "";
    if (this->Section > 0)
    {
        section = "&section=" + QString::number(this->Section);
    }
    if (this->BaseTimestamp.length())
        base = "&basetimestamp=" + QUrl::toPercentEncoding(this->BaseTimestamp);
    if (this->StartTimestamp.length())
        start_ = "&starttimestamp=" + QUrl::toPercentEncoding(this->StartTimestamp);
    this->qEdit->Parameters = "title=" + QUrl::toPercentEncoding(Page) + "&text=" + QUrl::toPercentEncoding(this->text) + section +
                              wl + "&summary=" + QUrl::toPercentEncoding(this->Summary) + base + start_ + "&token=" +
                              QUrl::toPercentEncoding(Configuration::HuggleConfiguration->TemporaryConfig_EditToken);
    QueryPool::HugglePool->AppendQuery(this->qEdit);
    this->qEdit->Process();
}
