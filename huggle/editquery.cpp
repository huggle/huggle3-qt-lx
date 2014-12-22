//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "editquery.hpp"
#include "apiqueryresult.hpp"
#include "configuration.hpp"
#include "exception.hpp"
#include "history.hpp"
#include "historyitem.hpp"
#include "localization.hpp"
#include "mainwindow.hpp"
#include "generic.hpp"
#include "syslog.hpp"
#include "querypool.hpp"
#include "wikipage.hpp"
#include "wikisite.hpp"
#include <QUrl>

using namespace Huggle;

EditQuery::EditQuery()
{
    this->Summary = "";
    this->Minor = false;
    this->Section = 0;
    this->BaseTimestamp = "";
    this->StartTimestamp = "";
    this->text = "";
    this->Type = QueryEdit;
}

EditQuery::~EditQuery()
{
    delete this->Page;
    this->HI.Delete();
}

void EditQuery::Process()
{
    if (this->Page == nullptr)
    {
        throw new Huggle::NullPointerException("local Page", BOOST_CURRENT_FUNCTION);
    }

    this->Status = StatusProcessing;
    this->StartTime = QDateTime::currentDateTime();
    if (this->Page->GetSite()->GetProjectConfig()->Token_Csrf.isEmpty())
    {
        this->SetError("No CSRF token");
        return;
    }
    this->EditPage();
}

bool EditQuery::IsProcessed()
{
    if (this->Result != nullptr)
        return true;

    if (this->qRetrieve != nullptr && this->qRetrieve->IsProcessed())
    {
        if (this->qRetrieve->IsFailed())
        {
            this->SetError("Unable to retrieve the previous content of page: " + this->qRetrieve->Result->ErrorMessage);
            this->qRetrieve.Delete();
            return true;
        }
        bool failed = false;
        QString ts;
        this->OriginalText = Generic::EvaluateWikiPageContents(this->qRetrieve, &failed, &ts);
        this->qRetrieve.Delete();
        if (failed)
        {
            this->SetError("Unable to retrieve the previous content of page: " + this->OriginalText);
            return true;
        }
        this->HasPreviousPageText = true;
        this->BaseTimestamp = ts;
        this->EditPage();
        return false;
    }

    if (this->qEdit != nullptr)
    {
        if (!this->qEdit->IsProcessed())
            return false;

        ApiQueryResultNode *err = this->qEdit->GetApiQueryResult()->GetNode("error");
        ApiQueryResultNode *edit = this->qEdit->GetApiQueryResult()->GetNode("edit");
        bool failed = true;
        if (err != nullptr)
        {
            if (err->Attributes.contains("code"))
            {
                QString ec = err->GetAttribute("code");
                int hec = HUGGLE_EUNKNOWN;
                QString reason = ec;
                if (ec == "assertuserfailed")
                {
                    reason = "Not logged in";
                    hec = HUGGLE_ENOTLOGGEDIN;
                    // this is some fine hacking here :)
                    // we use this later in main form
                    HUGGLE_DEBUG1("Session expired requesting a new login");
                    Configuration::Logout(this->Page->GetSite());
                }
                if (ec == "badtoken")
                {
                    reason = "Bad token";
                    hec = HUGGLE_ETOKEN;
                    // we log off the site
                    Configuration::Logout(this->Page->GetSite());
                    Syslog::HuggleLogs->ErrorLog("Unable to edit " + this->Page->PageName + " because token I had in cache is no longer valid, please try to edit that page once more");
                }
                this->Result = new QueryResult(true);
                this->Result->SetError(hec, reason);
                this->qEdit = nullptr;
                this->ProcessFailure();
                return true;
            }
        }
        if (edit != nullptr)
        {
            if (edit->Attributes.contains("result"))
            {
                if (edit->Attributes["result"] == "Success")
                {
                    failed = false;
                    if (MainWindow::HuggleMain != nullptr)
                    {
                        HistoryItem *item = new HistoryItem();
                        item->Result = _l("successful");
                        item->Type = HistoryEdit;
                        item->Target = this->Page->PageName;
                        this->HI = item;
                        MainWindow::HuggleMain->_History->Prepend(item);
                    }
                    this->ProcessCallback();
                    Huggle::Syslog::HuggleLogs->Log(_l("editquery-success", this->Page->PageName, this->Page->GetSite()->Name));
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
    if (this->Append && this->Prepend)
    {
        throw Huggle::Exception("You can't use both Append and Prepend for edit of page", BOOST_CURRENT_FUNCTION);
    }
    if ((this->Append || this->Prepend) && !this->HasPreviousPageText)
    {
        // we first need to get a text of current page
        this->qRetrieve = Generic::RetrieveWikiPageContents(this->Page);
        QueryPool::HugglePool->AppendQuery(this->qRetrieve);
        this->qRetrieve->Process();
        return;
    }
    this->qEdit = new ApiQuery(ActionEdit, this->Page->Site);
    this->qEdit->Target = _l("report-write") + " " + this->Page->PageName;
    this->qEdit->UsingPOST = true;
    if (this->Append)
    {
        // we append new text now
        this->Section = 0;
        this->text = this->OriginalText + this->text;
    } else if (this->Prepend)
    {
        this->Section = 0;
        this->text = this->text + this->OriginalText;
    }
    {
        this->text = this->text;
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
    this->qEdit->Parameters = "title=" + QUrl::toPercentEncoding(this->Page->PageName) + "&text=" + QUrl::toPercentEncoding(this->text) + section +
                              wl + "&summary=" + QUrl::toPercentEncoding(this->Summary) + base + start_ + "&token=" +
                              QUrl::toPercentEncoding(this->Page->GetSite()->GetProjectConfig()->Token_Csrf);
    QueryPool::HugglePool->AppendQuery(this->qEdit);
    this->qEdit->Process();
}

void EditQuery::SetError(QString reason)
{
    this->Result = new QueryResult(true);
    this->Result->SetError(reason);
    this->FailureReason = reason;
    this->Status = StatusInError;
}
