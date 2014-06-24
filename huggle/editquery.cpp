//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "editquery.hpp"
#include <QUrl>
#include <QtXml>
#include "configuration.hpp"
#include "historyitem.hpp"
#include "localization.hpp"
#include "history.hpp"
#include "mainwindow.hpp"
#include "generic.hpp"
#include "syslog.hpp"
#include "querypool.hpp"

using namespace Huggle;

EditQuery::EditQuery()
{
    this->Summary = "";
    this->Minor = false;
    this->Page = "";
    this->Section = 0;
    this->BaseTimestamp = "";
    this->StartTimestamp = "";
    this->text = "";
    this->Type = QueryEdit;
}

EditQuery::~EditQuery()
{
    this->HI.Delete();
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
        QueryPool::HugglePool->AppendQuery(this->qToken.GetPtr());
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

    if (this->qRetrieve != nullptr && this->qRetrieve->IsProcessed())
    {
        if (this->qRetrieve->IsFailed())
        {
            this->Result = new QueryResult(true);
            this->Result->SetError("Unable to retrieve the previous content of page: " + this->qRetrieve->Result->ErrorMessage);
            this->qRetrieve.Delete();
            this->ProcessFailure();
            return true;
        }
        bool failed = false;
        QString ts;
        this->OriginalText = Generic::EvaluateWikiPageContents(this->qRetrieve, &failed, &ts);
        this->qRetrieve.Delete();
        if (failed)
        {
            this->Result = new QueryResult(true);
            this->Result->SetError("Unable to retrieve the previous content of page: " + this->OriginalText);
            this->ProcessFailure();
            return true;
        }
        this->HasPreviousPageText = true;
        this->BaseTimestamp = ts;
        this->EditPage();
        return false;
    }
    if (this->qToken != nullptr)
    {
        if (!this->qToken->IsProcessed())
            return false;

        if (this->qToken->Result->IsFailed())
        {
            this->Result = new QueryResult();
            this->Result->SetError(_l("editquery-token-error") + ": " + this->qToken->Result->ErrorMessage);
            this->qToken.Delete();
            this->ProcessFailure();
            return true;
        }
        QDomDocument dToken_;
        dToken_.setContent(this->qToken->Result->Data);
        QDomNodeList l = dToken_.elementsByTagName("page");
        if (l.count() == 0)
        {
            this->Result = new QueryResult();
            this->Result->SetError(_l("editquery-token-error"));
            HUGGLE_DEBUG1("Debug message for edit: " + this->qToken->Result->Data);
            this->qToken.Delete();
            this->ProcessFailure();
            return true;
        }
        QDomElement element = l.at(0).toElement();
        if (!element.attributes().contains("edittoken"))
        {
            this->Result = new QueryResult();
            this->Result->SetError(_l("editquery-token-error"));
            HUGGLE_DEBUG1("Debug message for edit: " + this->qToken->Result->Data);
            this->qToken.Delete();
            this->ProcessFailure();
            return true;
        }
        Configuration::HuggleConfiguration->TemporaryConfig_EditToken = element.attribute("edittoken");
        this->qToken.Delete();
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
        QDomNodeList error_ = dEdit_.elementsByTagName("error");
        bool failed = true;
        if (error_.count() > 0)
        {
            QDomElement element = edits_.at(0).toElement();
            if (element.attributes().contains("code"))
            {
                QString ec = element.attribute("code");
                int hec = HUGGLE_EUNKNOWN;
                QString reason = ec;
                if (ec == "assertuserfailed")
                {
                    reason = "Not logged in";
                    hec = HUGGLE_ENOTLOGGEDIN;
                    // this is some fine hacking here :)
                    // we use this later in main form
                    HUGGLE_DEBUG("Session expired requesting a new login", 3);
                    Configuration::HuggleConfiguration->ProjectConfig->RequestLogin();
                }
                this->Result = new QueryResult(true);
                this->Result->SetError(hec, reason);
                this->qEdit = nullptr;
                this->ProcessFailure();
                return true;
            }
        }
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
            this->ProcessFailure();
        }
        this->qEdit = nullptr;
    }
    return false;
}

void EditQuery::EditPage()
{
    if (this->Append && !this->HasPreviousPageText)
    {
        // we first need to get a text of current page
        this->qRetrieve = Generic::RetrieveWikiPageContents(this->Page);
        QueryPool::HugglePool->AppendQuery(this->qRetrieve);
        this->qRetrieve->Process();
        return;
    }
    this->qEdit = new ApiQuery(ActionEdit);
    this->qEdit->Target = "Writing " + this->Page;
    this->qEdit->UsingPOST = true;
    QString t;
    if (this->Append)
    {
        // we append new text now
        this->Section = 0;
        t = this->OriginalText + this->text;
    } else
    {
        t = this->text;
    }
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
    if (!this->BaseTimestamp.isEmpty())
        base = "&basetimestamp=" + QUrl::toPercentEncoding(this->BaseTimestamp);
    if (!this->StartTimestamp.isEmpty())
        start_ = "&starttimestamp=" + QUrl::toPercentEncoding(this->StartTimestamp);
    this->qEdit->Parameters = "title=" + QUrl::toPercentEncoding(this->Page) + "&text=" + QUrl::toPercentEncoding(this->text) + section +
                              wl + "&summary=" + QUrl::toPercentEncoding(this->Summary) + base + start_ + "&token=" +
                              QUrl::toPercentEncoding(Configuration::HuggleConfiguration->TemporaryConfig_EditToken);
    QueryPool::HugglePool->AppendQuery(this->qEdit);
    this->qEdit->Process();
}
