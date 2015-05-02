//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "deleteform.hpp"
#include <QLineEdit>
#include <QUrl>
#include "apiqueryresult.hpp"
#include "exception.hpp"
#include "generic.hpp"
#include "history.hpp"
#include "localization.hpp"
#include "historyitem.hpp"
#include "mainwindow.hpp"
#include "querypool.hpp"
#include "syslog.hpp"
#include "gc.hpp"
#include "configuration.hpp"
#include "wikipage.hpp"
#include "wikisite.hpp"
#include "wikiuser.hpp"
#include "ui_deleteform.h"

using namespace Huggle;

DeleteForm::DeleteForm(QWidget *parent) : HW("deleteform", this, parent), ui(new Ui::DeleteForm)
{
    this->ui->setupUi(this);
    this->page = nullptr;
    this->tDelete = nullptr;
    this->TalkPage = nullptr;
    this->ui->comboBox->setCurrentIndex(0);
    this->PageUser = nullptr;
    this->RestoreWindow();
}

DeleteForm::~DeleteForm()
{
    delete this->tDelete;
    delete this->ui;
    delete this->page;
    delete this->TalkPage;
}

void DeleteForm::SetPage(WikiPage *Page, WikiUser *User)
{
    if (Page == nullptr)
    {
        throw new Huggle::NullPointerException("WikiPage *Page", BOOST_CURRENT_FUNCTION);
    }
    this->page = new WikiPage(Page);
    foreach(QString summary, Page->GetSite()->GetProjectConfig()->DeletionSummaries)
    {
        this->ui->comboBox->addItem(summary);
    }
    if (this->page->IsTalk())
    {
        this->ui->checkBox_2->setChecked(false);
        this->ui->checkBox_2->setEnabled(false);
    }
    this->setWindowTitle(_l("delete-title", Page->PageName));
    this->PageUser = User;
}

void DeleteForm::OnTick()
{
    this->Delete();
}

void DeleteForm::Delete()
{
    if (this->qDelete == nullptr || !this->qDelete->IsProcessed())
        return;
    if (this->qDelete->Result->IsFailed())
    {
        this->Failed(_l("delete-e1", this->qDelete->GetFailureReason()));
        return;
    }
    // let's assume the page was deleted
    this->ui->pushButton->setText(_l("deleted"));
    this->tDelete->stop();
    HUGGLE_DEBUG("Deletion result: " + this->qDelete->Result->Data, 2);
    HistoryItem *hi = new HistoryItem();
    hi->IsRevertable = false;
    hi->Result = _l("successful");
    hi->Target = this->page->PageName;
    hi->Type = HistoryDelete;
    MainWindow::HuggleMain->_History->Prepend(hi);
    this->close();
}

void DeleteForm::Failed(QString Reason)
{
    Generic::MessageBox(_l("delete-e2"), _l("delete-edsc", Reason), MessageBoxStyleError, true);
    this->tDelete->stop();
    delete this->tDelete;
    this->tDelete = nullptr;
    this->qDelete.Delete();
    this->qTalk.Delete();
    this->ui->pushButton->setEnabled(true);
}

void DeleteForm::on_pushButton_clicked()
{
    if (this->ui->checkBox_2->isChecked())
    {
        this->TalkPage = this->page->RetrieveTalk();
        if (this->TalkPage == nullptr)
        {
            this->ui->checkBox_2->setChecked(false);
        }
    }
    this->ui->checkBox_2->setEnabled(false);
    this->ui->comboBox->setEnabled(false);
    this->ui->pushButton->setEnabled(false);
    // let's delete the page
    this->qDelete = new ApiQuery(ActionDelete, this->page->GetSite());
    this->qDelete->Parameters = "title=" + QUrl::toPercentEncoding(this->page->PageName)
            + "&reason=" + QUrl::toPercentEncoding(Configuration::GenerateSuffix(this->ui->comboBox->lineEdit()->text(),
                                                                                 this->page->GetSite()->GetProjectConfig()))
            + "&token=" + QUrl::toPercentEncoding(this->page->GetSite()->GetProjectConfig()->Token_Csrf);
    this->qDelete->Target = "Deleting "  + this->page->PageName;
    this->qDelete->UsingPOST = true;
    QueryPool::HugglePool->AppendQuery(this->qDelete);
    this->qDelete->Process();

    if (this->TalkPage != nullptr)
    {
        // let's delete the talk page
        this->qTalk = new ApiQuery(ActionDelete, this->page->GetSite());
        this->qTalk->Parameters = "title=" + QUrl::toPercentEncoding(this->TalkPage->PageName)
                + "&reason=" + QUrl::toPercentEncoding(Configuration::GenerateSuffix(Configuration::HuggleConfiguration->ProjectConfig->AssociatedDelete,
                                                                                     this->page->GetSite()->GetProjectConfig()))
                + "&token=" + QUrl::toPercentEncoding(this->page->GetSite()->GetProjectConfig()->Token_Csrf);
        this->qTalk->Target = "Deleting "  + this->TalkPage->PageName;
        this->qTalk->UsingPOST = true;
        QueryPool::HugglePool->AppendQuery(this->qTalk);
        this->qTalk->Process();
    }
    this->tDelete = new QTimer();
    connect(this->tDelete, SIGNAL(timeout()), this, SLOT(OnTick()));
    this->tDelete->start(HUGGLE_TIMER);
}

void DeleteForm::on_pushButton_2_clicked()
{
    this->close();
}
