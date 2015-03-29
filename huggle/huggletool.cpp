//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "huggletool.hpp"
#include "apiqueryresult.hpp"
#include "core.hpp"
#include "exception.hpp"
#include "generic.hpp"
#include "configuration.hpp"
#include "localization.hpp"
#include "querypool.hpp"
#include "wikipage.hpp"
#include "mainwindow.hpp"
#include "wikisite.hpp"
#include "wikiuser.hpp"
#include "syslog.hpp"
#include "ui_huggletool.h"
#include <QUrl>

using namespace Huggle;

HuggleTool::HuggleTool(QWidget *parent) : QDockWidget(parent), ui(new Ui::HuggleTool)
{
    this->ui->setupUi(this);
    if (Configuration::HuggleConfiguration->Multiple)
    {
        this->ui->label_4->setText(_l("project"));
    } else
    {
        this->ui->comboBox->setVisible(false);
        this->ui->label_4->setVisible(false);
    }
    this->ui->pushButton->setText(_l("main-page-load"));
    this->ui->label_3->setText(_l("main-page-curr-disp"));
    this->tick = new QTimer(this);
    this->ui->label->setText(_l("user"));
    this->page = nullptr;
    this->ui->label_2->setText(_l("page"));
    foreach (WikiSite *site, Configuration::HuggleConfiguration->Projects)
        this->ui->comboBox->addItem(site->Name);
    this->ui->comboBox->setCurrentIndex(0);
    connect(this->tick, SIGNAL(timeout()), this, SLOT(onTick()));
}

HuggleTool::~HuggleTool()
{
    delete this->page;
    delete this->tick;
    delete this->ui;
}

void HuggleTool::SetTitle(QString title)
{
    this->ui->lineEdit->setText(title);
    this->ui->lineEdit_3->setText(title);
}

void HuggleTool::SetInfo(QString info)
{
    this->ui->lineEdit->setText(info);
}

void HuggleTool::SetUser(QString user)
{
    this->ui->lineEdit_2->setText(user);
}

void HuggleTool::SetPage(WikiPage *page)
{
    if (page == nullptr)
        throw new Huggle::NullPointerException("WikiPage *page", BOOST_CURRENT_FUNCTION);

    this->ui->lineEdit_3->setText(page->PageName);
    if (Configuration::HuggleConfiguration->Projects.contains(page->GetSite()) &&
            this->ui->comboBox->count() <= Configuration::HuggleConfiguration->Projects.count())
    {
        this->ui->comboBox->setCurrentIndex(Configuration::HuggleConfiguration->Projects.indexOf(page->GetSite()));
    } else
    {
        Syslog::HuggleLogs->WarningLog("Project not in a list: " + page->GetSite()->Name);
    }
    if (this->page != nullptr)
    {
        delete this->page;
    }
    this->page = new WikiPage(page);
    this->tick->stop();
    this->ui->pushButton->setEnabled(true);
    // change color to default
    this->query.Delete();
    this->ui->lineEdit_2->setStyleSheet("color: black;");
    this->ui->lineEdit_3->setStyleSheet("color: black;");
}

WikiSite *HuggleTool::GetSite()
{
    if (Configuration::HuggleConfiguration->Projects.count() != this->ui->comboBox->count())
    {
        // this should not happen ever
        throw new Huggle::Exception("Wrong number of projects (Configuration::HuggleConfiguration->Projects.count() isn't same as comboBox->count())",
                                    BOOST_CURRENT_FUNCTION);
    }
    return Configuration::HuggleConfiguration->Projects.at(this->ui->comboBox->currentIndex());
}

void HuggleTool::RenderEdit()
{
    if (!this->ui->pushButton->isEnabled() || !this->ui->lineEdit_3->text().length())
        return;
    if (this->ui->lineEdit_3->text().endsWith("_") || this->ui->lineEdit_3->text().endsWith(" "))
    {
        Generic::MessageBox(_l("error"), _l("main-space"), MessageBoxStyleError);
        return;
    }
    this->ui->pushButton->setEnabled(false);
    this->ui->lineEdit_3->setStyleSheet("color: green;");
    // retrieve information about the page
    this->QueryPhase = 1;
    this->query = Generic::RetrieveWikiPageContents(this->ui->lineEdit_3->text(), this->GetSite());
    this->query->Process();
    this->tick->start(HUGGLE_TIMER);
}

void Huggle::HuggleTool::on_pushButton_clicked()
{
    this->RenderEdit();
}

void HuggleTool::onTick()
{
    switch (this->QueryPhase)
    {
        case 0:
            this->tick->stop();
            return;
        case 1:
        case 3:
            this->FinishPage();
            return;
        case 2:
        case 4:
            this->FinishEdit();
            return;
    }
}

void HuggleTool::FinishPage()
{
    if (this->query == nullptr || !this->query->IsProcessed())
        return;
    if (this->QueryPhase == 3)
    {
        ApiQueryResultNode *item = this->query->GetApiQueryResult()->GetNode("item");
        if (!item)
        {
            this->ui->lineEdit_2->setStyleSheet("color: red;");
            this->tick->stop();
            return;
        }
        if (!item->Attributes.contains("title"))
        {
            this->ui->lineEdit_2->setStyleSheet("color: red;");
            this->tick->stop();
            return;
        }
        this->edit = new WikiEdit();
        this->edit->Page = new WikiPage(item->GetAttribute("title"));
        this->edit->Page->Site = this->query->GetSite();
        this->edit->User = new WikiUser(item->GetAttribute("user"));
        this->edit->User->Site = this->query->GetSite();
        this->edit->RevID = item->GetAttribute("revid").toInt();
        QueryPool::HugglePool->PreProcessEdit(this->edit);
        QueryPool::HugglePool->PostProcessEdit(this->edit);
        this->QueryPhase = 4;
    } else
    {
        this->edit = new WikiEdit();
        this->edit->Page = new WikiPage(this->ui->lineEdit_3->text());
        this->edit->Page->Site = this->query->GetSite();
        ApiQueryResultNode *rev = this->query->GetApiQueryResult()->GetNode("rev");
        if (rev)
        {
            if (rev->Attributes.contains("missing"))
            {
                // there is no such a page
                this->ui->lineEdit_3->setStyleSheet("color: red;");
                Huggle::Syslog::HuggleLogs->WarningLog(_l("missing-page", ui->lineEdit_3->text()));
                this->tick->stop();
                this->edit.Delete();
                this->query.Delete();
                return;
            }
            if (rev->Attributes.contains("user"))
            {
                this->edit->User = new WikiUser(rev->GetAttribute("user"));
                this->edit->User->Site = this->GetSite();
            }
            if (rev->Attributes.contains("revid"))
                this->edit->RevID = rev->GetAttribute("revid").toInt();
        }
        if (this->edit->User == nullptr)
        {
            this->edit->User = new WikiUser();
            this->edit->User->Site = this->query->GetSite();
        }
        QueryPool::HugglePool->PreProcessEdit(this->edit);
        QueryPool::HugglePool->PostProcessEdit(this->edit);
        this->QueryPhase = 2;
    }
}

void HuggleTool::FinishEdit()
{
    if (this->edit == nullptr || !this->edit->IsPostProcessed())
        return;
    this->tick->stop();
    this->ui->pushButton->setEnabled(true);
    MainWindow::HuggleMain->ProcessEdit(this->edit, false, false, false, true);
}

void Huggle::HuggleTool::on_lineEdit_3_returnPressed()
{
    this->RenderEdit();
}

void Huggle::HuggleTool::on_lineEdit_2_returnPressed()
{
    if (!this->ui->pushButton->isEnabled() || !this->ui->lineEdit_2->text().length())
    {
        return;
    }
    this->ui->pushButton->setEnabled(false);
    this->ui->lineEdit_2->setStyleSheet("color: green;");
    // retrieve information about the user
    this->query = new ApiQuery(ActionQuery, this->GetSite());
    this->QueryPhase = 3;
    this->query->Parameters = "list=usercontribs&ucuser=" + QUrl::toPercentEncoding(this->ui->lineEdit_2->text()) +
                              "&ucprop=flags%7Ccomment%7Ctimestamp%7Ctitle%7Cids%7Csize&uclimit=20";
    this->query->Process();
    this->tick->start(HUGGLE_TIMER);
}
