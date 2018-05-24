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
#include "apiqueryresult.hpp"
#include "configuration.hpp"
#include "exception.hpp"
#include "localization.hpp"
#include "historyitem.hpp"
#include "hooks.hpp"
#include "syslog.hpp"
#include "querypool.hpp"
#include "wikipage.hpp"
#include "wikiutil.hpp"
#include "wikisite.hpp"

using namespace Huggle;

EditQuery::EditQuery()
{
    this->Summary = "";
    this->Minor = false;
    this->Section = 0;
    this->BaseTimestamp = "";
    this->StartTimestamp = "";
    this->Text = "";
    this->Type = QueryEdit;
}

EditQuery::~EditQuery()
{
    delete this->Page;
    this->HI.Delete();
}

void EditQuery::Kill()
{
    if (this->qEdit != nullptr)
        this->qEdit->Kill();
    if (this->qRetrieve != nullptr)
        this->qRetrieve->Kill();

    this->qEdit.Delete();
    this->qRetrieve.Delete();
    this->hasPreviousPageText = false;
    this->originalText = "";
    this->status = StatusKilled;
}

void EditQuery::Process()
{
    if (this->status == StatusIsSuspended)
    {
        HUGGLE_DEBUG1("Ignoring request to process suspended query " + QString::number(this->QueryID()) + " fix me");
        return;
    }

    if (this->status == StatusProcessing)
    {
        HUGGLE_DEBUG1("Ignoring request to process query that is already running " + QString::number(this->QueryID()) + " fix me");
        return;
    }

    if (this->Page == nullptr)
    {
        throw new Huggle::NullPointerException("local Page", BOOST_CURRENT_FUNCTION);
    }

    this->status = StatusProcessing;
    this->StartTime = QDateTime::currentDateTime();
    if (this->Page->GetSite()->GetProjectConfig()->Token_Csrf.isEmpty())
    {
        this->setError(_l("editquery-nocsrf"));
        return;
    }
    this->editPage();
}

bool EditQuery::IsProcessed()
{
    if (this->status == StatusIsSuspended)
        return false;

    if (this->Result != nullptr)
        return true;

    if (this->qRetrieve != nullptr && this->qRetrieve->IsProcessed())
    {
        if (this->qRetrieve->IsFailed())
        {
            this->setError(_l("editquery-error-retrieve-prev", this->qRetrieve->GetFailureReason()));
            this->qRetrieve.Delete();
            return true;
        }
        bool failed = false;
        QString ts;
        this->originalText = WikiUtil::EvaluateWikiPageContents(this->qRetrieve, &failed, &ts);
        this->qRetrieve.Delete();
        if (failed)
        {
            HUGGLE_DEBUG1("Unable to retrieve text of page " + this->Page->PageName + ": " + this->originalText);
            this->setError(_l("editquery-error-retrieve-prev", this->originalText));
            return true;
        }
        this->hasPreviousPageText = true;
        this->BaseTimestamp = ts;
        this->editPage();
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
                    this->Suspend();
                    Configuration::Logout(this->Page->GetSite());
                } else if (ec == "badtoken")
                {
                    reason = _l("editquery-error-badtoken");
                    hec = HUGGLE_ETOKEN;
                    // we log off the site
                    this->Suspend();
                    Configuration::Logout(this->Page->GetSite());
                    Syslog::HuggleLogs->ErrorLog(_l("editquery-invalid-token", this->Page->PageName));
                    this->qEdit.Delete();
                    return false;
                } else
                {
                    // We don't want to process failure in case the query was suspended
                    this->Result = new QueryResult(true);
                    this->Result->SetError(hec, reason);
                    this->processFailure();
                }
                this->qEdit = nullptr;
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
                    HistoryItem *item = new HistoryItem();
                    item->Result = _l("successful");
                    item->Type = HistoryEdit;
                    item->Target = this->Page->PageName;
                    this->HI = item;
                    Hooks::WikiEdit_OnNewHistoryItem(item);
                    this->processCallback();
                    Huggle::Syslog::HuggleLogs->Log(_l("editquery-success", this->Page->PageName, this->Page->GetSite()->Name));
                }
            }
        }
        this->Result = new QueryResult();
        if (failed)
        {
            this->Result->SetError(this->qEdit->Result->Data);
            this->processFailure();
        }
        this->qEdit = nullptr;
    }
    return false;
}

void EditQuery::Restart()
{
    Query::Restart();
}

void EditQuery::editPage()
{
    if (this->Append && this->Prepend)
    {
        throw Huggle::Exception(_l("editquery-error-append"), BOOST_CURRENT_FUNCTION);
    }
    if ((this->Append || this->Prepend) && !this->hasPreviousPageText)
    {
        // we first need to get a text of current page
        this->qRetrieve = WikiUtil::RetrieveWikiPageContents(this->Page);
        HUGGLE_QP_APPEND(this->qRetrieve);
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
        this->Text = this->originalText + this->Text;
    } else if (this->Prepend)
    {
        this->Section = 0;
        this->Text = this->Text + this->originalText;
    }
    {
        this->Text = this->Text;
    }
    QString base = "";
    QString start_ = "";
    QString section = "";
    QString tag = "";
    if (Huggle::Version("1.25.2") <= this->Page->GetSite()->MediawikiVersion && !this->Page->GetSite()->GetProjectConfig()->Tag.isEmpty())
        tag = "&tags=" + QUrl::toPercentEncoding(this->Page->GetSite()->GetProjectConfig()->Tag);
    QString wl = "&watchlist=" + UserConfiguration::WatchListOptionToString(hcfg->UserConfig->Watchlist);
    if (this->InsertTargetToWatchlist)
        wl = "&watchlist=" + UserConfiguration::WatchListOptionToString(WatchlistOption_Watch);
    if (this->Section > 0)
    {
        section = "&section=" + QString::number(this->Section);
    }
    if (!this->BaseTimestamp.isEmpty())
        base = "&basetimestamp=" + QUrl::toPercentEncoding(this->BaseTimestamp);
    if (!this->StartTimestamp.isEmpty())
        start_ = "&starttimestamp=" + QUrl::toPercentEncoding(this->StartTimestamp);
    this->qEdit->Parameters = "title=" + QUrl::toPercentEncoding(this->Page->PageName) + "&text=" + QUrl::toPercentEncoding(this->Text) + section +
                              wl + "&summary=" + QUrl::toPercentEncoding(this->Summary) + tag + base + start_ + "&token=" +
                              QUrl::toPercentEncoding(this->Page->GetSite()->GetProjectConfig()->Token_Csrf);
    HUGGLE_QP_APPEND(this->qEdit);
    this->qEdit->Process();
}

void EditQuery::setError(QString reason)
{
    this->Result = new QueryResult(true);
    this->Result->SetError(reason);
    this->failureReason = reason;
    this->status = StatusInError;
}
