//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "requestprotect.hpp"
#include "ui_requestprotect.h"

using namespace Huggle;

RequestProtect::RequestProtect(WikiPage *wikiPage, QWidget *parent) : QDialog(parent), ui(new Ui::RequestProtect)
{
    this->qEditRFP = NULL;
    this->qRFPPage = NULL;
    this->page = new Huggle::WikiPage(wikiPage);
    this->ui->setupUi(this);
    this->setWindowTitle(Localizations::HuggleLocalizations->Localize("reqprotection-title", this->page->PageName));
    this->tm = new QTimer(this);
    connect(this->tm, SIGNAL(timeout()), this, SLOT(Tick()));
}

RequestProtect::~RequestProtect()
{
    delete this->tm;
    delete this->page;
    this->DelRef();
    delete this->ui;
}

void RequestProtect::Tick()
{
    if (this->qRFPPage != NULL)
    {
        // we are reading the request page let's see if we got it
        if (!this->qRFPPage->IsProcessed())
        {
            return;
        }
        // we finished let's parse the page now
        if (this->qRFPPage->IsFailed())
        {
            this->Fail("Unable to retrieve the current report page: " + this->qRFPPage->Result->ErrorMessage);
            return;
        }
        QDomDocument d;
        d.setContent(this->qRFPPage->Result->Data);
        QDomNodeList results = d.elementsByTagName("rev");
        if (results.count() == 0)
        {
            this->Fail("Unable to retrieve the current report page because the query didn't return any text for this page");
            return;
        }
        QDomElement e = results.at(0).toElement();
        if (!e.attributes().contains("timestamp"))
        {
            this->Fail("The query didn't return any timestamp (mediawiki bug?) aborting the query");
            return;
        }
        this->Timestamp = e.attribute("timestamp");
        QString PageText = e.text();
        // make a regex out of the pattern string
        QRegExp *rx = new QRegExp(Huggle::Configuration::HuggleConfiguration->ProjectConfig_RFPP_Regex);
        if (rx->exactMatch(PageText))
        {
            this->Fail(Localizations::HuggleLocalizations->Localize("reqprotection-duplicate"));
            delete rx;
            return;
        }
        delete rx;
        QString report = Configuration::HuggleConfiguration->ProjectConfig_RFPP_Template;
        report.replace("$title", this->page->PageName);
        report.replace("\\n", "\n");
        report.replace("$reason", this->ui->lineEdit->text());
        report.replace("$protection", this->ProtectionType());
        if (!Configuration::HuggleConfiguration->ProjectConfig_RFPP_PlaceTop)
            PageText += "\n\n" + report;
        else
            PageText = report + "\n\n" + PageText;
        // we no longer need the query we used
        this->qRFPPage->DecRef();
        this->qRFPPage = NULL;
        QString summary_ = Configuration::HuggleConfiguration->ProjectConfig_RFPP_Summary;
        summary_.replace("$1", this->ProtectionType());
        summary_.replace("$2", this->page->PageName);
        this->ui->pushButton->setText("Requesting");
        // let's edit the page now
        if (Configuration::HuggleConfiguration->ProjectConfig_RFPP_Section == 0)
        {
            this->qEditRFP = WikiUtil::EditPage(Configuration::HuggleConfiguration->ProjectConfig_RFPP_Page, PageText,
                                                summary_, false, this->Timestamp);
        } else
        {
            this->qEditRFP = WikiUtil::EditPage(Configuration::HuggleConfiguration->ProjectConfig_RFPP_Page, PageText,
                                                summary_, false, this->Timestamp,
                                                Configuration::HuggleConfiguration->ProjectConfig_RFPP_Section);
        }
        return;
    }

    if (this->qEditRFP != NULL)
    {
        if (!this->qEditRFP->IsProcessed())
        {
            return;
        }
        if (this->qEditRFP->IsFailed())
        {
            this->Fail("Unable to process: " + this->qEditRFP->Result->ErrorMessage);
            return;
        }
        this->ui->pushButton->setText("Requested");
        this->tm->stop();
    }
}

void Huggle::RequestProtect::on_pushButton_clicked()
{
    this->DelRef();
    this->qRFPPage = new ApiQuery(ActionQuery);
    this->qRFPPage->IncRef();
    // if this wiki has the requests in separate section, get it, if not, we get a whole page
    if (Configuration::HuggleConfiguration->ProjectConfig_RFPP_Section == 0)
    {
        this->qRFPPage->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("timestamp|user|comment|content") +
                         "&titles=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->ProjectConfig_RFPP_Page);
    } else
    {
        this->qRFPPage->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("timestamp|user|comment|content") +
                         "&titles=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->ProjectConfig_RFPP_Page) +
                         "&rvsection=" + QString::number(Configuration::HuggleConfiguration->ProjectConfig_RFPP_Section);
    }
    QueryPool::HugglePool->AppendQuery(this->qRFPPage);
    this->qRFPPage->Process();
    this->tm->start(800);
    this->ui->pushButton->setText("Retrieving");
    this->ui->pushButton->setEnabled(false);
}

QString RequestProtect::ProtectionType()
{
    if (this->ui->radioButton_2->isChecked())
    {
        return "Permanent protection";
    }
    return "Temporary protection";
}

void RequestProtect::Fail(QString message)
{
    QMessageBox mb;
    mb.setWindowTitle(":(");
    mb.setText(message);
    mb.exec();
    // delete the queries and stop
    this->DelRef();
    this->tm->stop();
    this->ui->pushButton->setEnabled(true);
    this->ui->pushButton->setText("Request");
}

void RequestProtect::DelRef()
{
    if (this->qEditRFP)
    {
        this->qEditRFP->DecRef();
        this->qEditRFP = NULL;
    }
    if (this->qRFPPage)
    {
        this->qRFPPage->DecRef();
        this->qRFPPage = NULL;
    }
}

void Huggle::RequestProtect::on_pushButton_2_clicked()
{
    this->close();
}
