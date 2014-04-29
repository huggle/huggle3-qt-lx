//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "editquery.hpp"
#include "querypool.hpp"

using namespace Huggle;

EditQuery::EditQuery()
{
    this->Summary = "";
    this->Result = NULL;
    this->qEdit = NULL;
    this->Minor = false;
    this->Page = "";
    this->qToken = NULL;
    this->Section = 0;
    this->BaseTimestamp = "";
    this->StartTimestamp = "";
    this->text = "";
    this->Type = QueryEdit;
}

EditQuery::~EditQuery()
{
    if (this->qToken != NULL)
    {
        this->qToken->UnregisterConsumer(HUGGLECONSUMER_EDITQUERY);
    }
}

void EditQuery::Process()
{
    this->Status = StatusProcessing;
    this->StartTime = QDateTime::currentDateTime();
    if (Configuration::HuggleConfiguration->TemporaryConfig_EditToken == "")
    {
        this->qToken = new ApiQuery(ActionQuery);
        this->qToken->Parameters = "prop=info&intoken=edit&titles=" + QUrl::toPercentEncoding(Page);
        this->qToken->Target = Localizations::HuggleLocalizations->Localize("editquery-token", Page);
        this->qToken->RegisterConsumer(HUGGLECONSUMER_EDITQUERY);
        QueryPool::HugglePool->AppendQuery(qToken);
        this->qToken->Process();
    } else
    {
        this->EditPage();
    }
}

bool EditQuery::IsProcessed()
{
    if (this->Result != NULL)
    {
        return true;
    }
    if (this->qToken != NULL)
    {
        if (!this->qToken->IsProcessed())
        {
            return false;
        }
        if (this->qToken->Result->Failed)
        {
            this->Result = new QueryResult();
            this->Result->Failed = true;
            this->Result->ErrorMessage = Localizations::HuggleLocalizations->Localize("editquery-token-error") + ": " +
                                         this->qToken->Result->ErrorMessage;
            this->qToken->UnregisterConsumer(HUGGLECONSUMER_EDITQUERY);
            this->qToken = NULL;
            return true;
        }
        QDomDocument dToken_;
        dToken_.setContent(this->qToken->Result->Data);
        QDomNodeList l = dToken_.elementsByTagName("page");
        if (l.count() == 0)
        {
            this->Result = new QueryResult();
            this->Result->Failed = true;
            this->Result->ErrorMessage = Localizations::HuggleLocalizations->Localize("editquery-token-error");
            Huggle::Syslog::HuggleLogs->DebugLog("Debug message for edit: " + qToken->Result->Data);
            this->qToken->UnregisterConsumer(HUGGLECONSUMER_EDITQUERY);
            this->qToken = NULL;
            return true;
        }
        QDomElement element = l.at(0).toElement();
        if (!element.attributes().contains("edittoken"))
        {
            this->Result = new QueryResult();
            this->Result->Failed = true;
            this->Result->ErrorMessage = Localizations::HuggleLocalizations->Localize("editquery-token-error");
            Huggle::Syslog::HuggleLogs->DebugLog("Debug message for edit: " + this->qToken->Result->Data);
            this->qToken->UnregisterConsumer(HUGGLECONSUMER_EDITQUERY);
            this->qToken = NULL;
            return true;
        }
        Configuration::HuggleConfiguration->TemporaryConfig_EditToken = element.attribute("edittoken");
        this->qToken->UnregisterConsumer(HUGGLECONSUMER_EDITQUERY);
        this->qToken = NULL;
        this->EditPage();
        return false;
    }
    if (this->qEdit != NULL)
    {
        if (!this->qEdit->IsProcessed())
        {
            return false;
        }
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
                    if (MainWindow::HuggleMain != NULL)
                    {
                        HistoryItem item;
                        item.Result = Localizations::HuggleLocalizations->Localize("successful");
                        item.Type = HistoryEdit;
                        item.Target = this->Page;
                        MainWindow::HuggleMain->_History->Prepend(item);
                    }
                    Huggle::Syslog::HuggleLogs->Log(Localizations::HuggleLocalizations->Localize("editquery-success", Page));
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
        this->qEdit = NULL;
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
    if (this->Section > 0)
    {
        section = "&section=" + QString::number(this->Section);
    }
    if (this->BaseTimestamp.length())
        base = "&basetimestamp=" + QUrl::toPercentEncoding(this->BaseTimestamp);
    if (this->StartTimestamp.length())
        start_ = "&starttimestamp=" + QUrl::toPercentEncoding(this->StartTimestamp);
    this->qEdit->Parameters = "title=" + QUrl::toPercentEncoding(Page) + "&text=" + QUrl::toPercentEncoding(this->text) + section +
                              "&summary=" + QUrl::toPercentEncoding(this->Summary) + base + start_ + "&token=" +
                              QUrl::toPercentEncoding(Configuration::HuggleConfiguration->TemporaryConfig_EditToken);
    QueryPool::HugglePool->AppendQuery(qEdit);
    this->qEdit->Process();
}
