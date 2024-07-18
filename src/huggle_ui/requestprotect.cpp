//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "requestprotect.hpp"
#include <QMessageBox>
#include <QtXml>
#include <huggle_core/querypool.hpp>
#include <huggle_core/generic.hpp>
#include <huggle_core/core.hpp>
#include <huggle_core/syslog.hpp>
#include <huggle_core/wikisite.hpp>
#include <huggle_core/wikiutil.hpp>
#include <huggle_core/localization.hpp>
#include <huggle_core/configuration.hpp>
#include "ui_requestprotect.h"

using namespace Huggle;

RequestProtect::RequestProtect(WikiPage *wikiPage, QWidget *parent) : HW("requestprotect", this, parent), ui(new Ui::RequestProtect)
{
    this->page = new Huggle::WikiPage(wikiPage);
    this->ui->setupUi(this);
    this->setWindowTitle(_l("protect-request-title", this->page->PageName));
    this->tm = new QTimer(this);
    connect(this->tm, SIGNAL(timeout()), this, SLOT(Tick()));
    this->ui->lineEdit->setText(wikiPage->GetSite()->GetProjectConfig()->RFPP_Reason);
    this->ui->LabelDuration->setText(_l("duration"));
    this->RestoreWindow();
}

RequestProtect::~RequestProtect()
{
    delete this->tm;
    delete this->page;
    delete this->ui;
}

void RequestProtect::Tick()
{
    if (this->qRFPPage != nullptr && this->qRFPPage->IsProcessed())
    {
        // we are reading the request page let's see if we got it
        if (this->qRFPPage->IsFailed())
        {
            this->Fail(_l("protect-request-fail", this->qRFPPage->GetFailureReason()));
            return;
        }
        QDomDocument d;
        d.setContent(this->qRFPPage->Result->Data);
        QDomNodeList results = d.elementsByTagName("rev");
        if (results.count() == 0)
        {
            this->Fail(_l("protect-request-fail-notext"));
            return;
        }
        QDomElement e = results.at(0).toElement();
        if (!e.attributes().contains("timestamp"))
        {
            this->Fail(_l("protect-request-fail-notime"));
            return;
        }
        this->Timestamp = e.attribute("timestamp");
        QString PageText = e.text();
        // make a regex out of the pattern string
        QString regex_str = this->page->GetSite()->GetProjectConfig()->RFPP_Regex;
        regex_str.replace("$title", this->page->PageName);
        if (Generic::RegexExactMatch(regex_str, PageText))
        {
            this->Fail(_l("reqprotection-duplicate"));
            return;
        }
        QString report = this->page->GetSite()->GetProjectConfig()->RFPP_Template;
        if ((this->page->IsUserpage() || this->page->GetNS()->GetCanonicalName() == "User talk") &&
            this->page->GetSite()->GetProjectConfig()->RFPP_TemplateUser.size() > 0)
        {
            report = this->page->GetSite()->GetProjectConfig()->RFPP_TemplateUser;
        }
        report.replace("$title", this->page->PageName);
        report.replace("\\n", "\n");
        report.replace("$reason", this->ui->lineEdit->text());
        report.replace("$protection", this->ProtectionType());
        if (!this->page->GetSite()->GetProjectConfig()->RFPP_PlaceTop)
        {
            PageText += "\n\n" + report;
        } else
        {
            if (this->page->GetSite()->GetProjectConfig()->RFPP_Mark.isEmpty())
            {
                PageText = report + "\n\n" + PageText;
            } else
            {
                // let's find end index
                QString mk = this->page->GetSite()->GetProjectConfig()->RFPP_Mark;
                if (!PageText.contains(mk))
                {
                    // there is no mark, abort this
                    this->Fail("There is no RFPP:Mark on protection request page, unable to request page protection");
                    return;
                }
                int index = PageText.indexOf(mk);
                index += mk.length();
                PageText.insert(index, "\n\n" + report + "\n\n");
            }
        }
        QString summary_ = this->page->GetSite()->GetProjectConfig()->RFPP_Summary;
        summary_.replace("$1", this->ProtectionType());
        summary_.replace("$2", this->page->PageName);
        // we no longer need the query we used
        this->qRFPPage = nullptr;
        this->ui->pushButton_RequestProtection->setText(_l("requesting"));
        // let's edit the page now
        if (this->page->GetSite()->GetProjectConfig()->RFPP_Section == 0)
        {
            this->qEditRFP = WikiUtil::EditPage(this->page->GetSite(), this->page->GetSite()->GetProjectConfig()->RFPP_Page, PageText,
                                                summary_, false, this->Timestamp);
        } else
        {
            this->qEditRFP = WikiUtil::EditPage(this->page->GetSite(), this->page->GetSite()->GetProjectConfig()->RFPP_Page, PageText,
                                                summary_, false, this->Timestamp,
                                                this->page->GetSite()->GetProjectConfig()->RFPP_Section);
        }
        return;
    }

    if (this->qEditRFP != nullptr && this->qEditRFP->IsProcessed())
    {
        if (this->qEditRFP->IsFailed())
        {
            this->Fail("Unable to process: " + this->qEditRFP->GetFailureReason());
            return;
        }
        this->ui->pushButton_RequestProtection->setText(_l("requested"));
        this->tm->stop();
    }
}

QString RequestProtect::ProtectionType()
{
    if (this->ui->radioButton_Indefinite->isChecked())
    {
        return this->qRFPPage->GetSite()->GetProjectConfig()->RFPP_Permanent;
    }
    return this->qRFPPage->GetSite()->GetProjectConfig()->RFPP_Temporary;
}

void RequestProtect::Fail(const QString &message)
{
    QMessageBox mb;
    mb.setWindowTitle(_l("error"));
    mb.setText(message);
    mb.exec();
    // delete the queries and stop
    this->qEditRFP.Delete();
    this->qRFPPage.Delete();
    this->tm->stop();
    this->ui->pushButton_RequestProtection->setEnabled(true);
    this->ui->pushButton_RequestProtection->setText(_l("request"));
}

void RequestProtect::on_pushButton_RequestProtection_clicked()
{
    this->qRFPPage = new ApiQuery(ActionQuery, this->page->GetSite());
    // if this wiki has the requests in separate section, get it, if not, we get a whole page
    if (this->page->GetSite()->GetProjectConfig()->RFPP_Section == 0)
    {
        this->qRFPPage->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("timestamp|user|comment|content") +
                         "&titles=" + QUrl::toPercentEncoding(this->page->GetSite()->GetProjectConfig()->RFPP_Page);
    } else
    {
        this->qRFPPage->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("timestamp|user|comment|content") +
                         "&titles=" + QUrl::toPercentEncoding(this->page->GetSite()->GetProjectConfig()->RFPP_Page) +
                         "&rvsection=" + QString::number(this->page->GetSite()->GetProjectConfig()->RFPP_Section);
    }
    QueryPool::HugglePool->AppendQuery(this->qRFPPage);
    this->qRFPPage->Process();
    this->tm->start(HUGGLE_TIMER);
    this->ui->pushButton_RequestProtection->setText(_l("retrieving"));
    this->ui->pushButton_RequestProtection->setEnabled(false);
}

void RequestProtect::on_pushButton_Cancel_clicked()
{
    this->close();
}
