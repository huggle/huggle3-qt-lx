//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "deleteform.hpp"
#include <QtXml>
#include <QMessageBox>
#include <QLineEdit>
#include <QUrl>
#include "exception.hpp"
#include "localization.hpp"
#include "querypool.hpp"
#include "syslog.hpp"
#include "gc.hpp"
#include "configuration.hpp"
#include "wikipage.hpp"
#include "wikiuser.hpp"
#include "ui_deleteform.h"

using namespace Huggle;

DeleteForm::DeleteForm(QWidget *parent) : QDialog(parent), ui(new Ui::DeleteForm)
{
    this->ui->setupUi(this);
    int xx = 0;
    while (xx < Configuration::HuggleConfiguration->ProjectConfig->DeletionSummaries.count())
    {
        this->ui->comboBox->addItem(Configuration::HuggleConfiguration->ProjectConfig->DeletionSummaries.at(xx));
        xx++;
    }
    this->page = nullptr;
    this->tDelete = nullptr;
    this->DeleteToken = "";
    this->TalkPage = nullptr;
    this->ui->comboBox->setCurrentIndex(0);
    this->PageUser = nullptr;
}

DeleteForm::~DeleteForm()
{
    delete this->ui;
    delete this->page;
    delete this->TalkPage;
}

void DeleteForm::SetPage(WikiPage *Page, WikiUser *User)
{
    if (Page == nullptr)
    {
        throw new Huggle::Exception("Page must not be NULL", "void DeleteForm::setPage(WikiPage *Page)");
    }
    this->page = new WikiPage(Page);
    if (this->page->IsTalk())
    {
        this->ui->checkBox_2->setChecked(false);
        this->ui->checkBox_2->setEnabled(false);
    }
    this->setWindowTitle(_l("delete-title", Page->PageName));
    this->PageUser = User;
}

void DeleteForm::GetToken()
{
    this->qToken = new ApiQuery(ActionQuery);
    this->qToken->Parameters = "action=query&prop=info&intoken=delete&titles=" + QUrl::toPercentEncoding(this->page->PageName);
    this->qToken->Target = _l("delete-token01", this->page->PageName);
    QueryPool::HugglePool->AppendQuery(this->qToken);
    this->qToken->Process();
    if (this->TalkPage != nullptr)
    {
        this->qTokenOfTalkPage = new ApiQuery(ActionQuery);
        this->qTokenOfTalkPage->Parameters = "action=query&prop=info&intoken=delete&titles=" + QUrl::toPercentEncoding(this->TalkPage->PageName);
        this->qTokenOfTalkPage->Target = _l("delete-token01", this->TalkPage->PageName);
        QueryPool::HugglePool->AppendQuery(this->qTokenOfTalkPage);
        this->qTokenOfTalkPage->Process();
    }
    this->tDelete = new QTimer(this);
    connect(this->tDelete, SIGNAL(timeout()), this, SLOT(OnTick()));
    this->delQueryPhase = 0;
    this->tDelete->start(200);
}

void DeleteForm::OnTick()
{
    switch (this->delQueryPhase)
    {
        case 0:
            this->CheckDeleteToken();
            return;
        case 1:
            this->Delete();
            return;
    }
    this->tDelete->stop();
}

void DeleteForm::CheckDeleteToken()
{
    if (this->qToken == nullptr || !this->qToken->IsProcessed())
    {
        return;
    }
    if (this->qToken->Result->IsFailed())
    {
        this->Failed(_l("delete-error-token", this->qToken->Result->ErrorMessage));
        return;
    }
    QDomDocument d;
    QDomNodeList l;
    if (this->TalkPage != nullptr)
    {
        if (this->qTokenOfTalkPage == nullptr || !this->qTokenOfTalkPage->IsProcessed())
            return;

        if (this->qTokenOfTalkPage->Result->IsFailed())
        {
            this->Failed(_l("delete-error-token", this->qToken->Result->ErrorMessage));
            return;
        }
        d.setContent(this->qTokenOfTalkPage->Result->Data);
        l = d.elementsByTagName("page");
        if (l.count() == 0)
        {
            HUGGLE_DEBUG(this->qTokenOfTalkPage->Result->Data, 1);
            this->Failed(_l("delete-failed-no-info"));
            return;
        }
        QDomElement element = l.at(0).toElement();
        if (!element.attributes().contains("deletetoken"))
        {
            this->Failed(_l("delete-token02"));
            return;
        }
        this->DeleteToken2 = element.attribute("deletetoken");
        this->qTokenOfTalkPage = nullptr;
        HUGGLE_DEBUG("Delete token for " + this->TalkPage->PageName + ": " + this->DeleteToken2, 1);

        // let's delete the page
        this->qTalk = new ApiQuery(ActionDelete);
        this->qTalk->Parameters = "title=" + QUrl::toPercentEncoding(this->TalkPage->PageName)
                + "&reason=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->ProjectConfig->AssociatedDelete);
                + "&token=" + QUrl::toPercentEncoding(this->DeleteToken2);
        this->qTalk->Target = "Deleting "  + this->TalkPage->PageName;
        this->qTalk->UsingPOST = true;
        QueryPool::HugglePool->AppendQuery(this->qTalk);
        this->qTalk->Process();
    }
    d.setContent(this->qToken->Result->Data);
    l = d.elementsByTagName("page");
    if (l.count() == 0)
    {
        HUGGLE_DEBUG(this->qToken->Result->Data, 1);
        this->Failed(_l("delete-failed-no-info"));
        return;
    }
    QDomElement element = l.at(0).toElement();
    if (!element.attributes().contains("deletetoken"))
    {
        this->Failed(_l("delete-token02"));
        return;
    }
    this->DeleteToken = element.attribute("deletetoken");
    this->delQueryPhase++;
    this->qToken = nullptr;
    HUGGLE_DEBUG("Delete token for " + this->page->PageName + ": " + this->DeleteToken, 1);

    // let's delete the page
    this->qDelete = new ApiQuery(ActionDelete);
    this->qDelete->Parameters = "title=" + QUrl::toPercentEncoding(this->page->PageName)
            + "&reason=" + QUrl::toPercentEncoding(this->ui->comboBox->lineEdit()->text())
            + "&token=" + QUrl::toPercentEncoding(this->DeleteToken);
    this->qDelete->Target = "Deleting "  + this->page->PageName;
    this->qDelete->UsingPOST = true;
    QueryPool::HugglePool->AppendQuery(qDelete);
    this->qDelete->Process();
}

void DeleteForm::Delete()
{
    if (this->qDelete == nullptr || !this->qDelete->IsProcessed())
        return;
    if (this->qDelete->Result->IsFailed())
    {
        this->Failed(_l("delete-e1", this->qDelete->Result->ErrorMessage));
        return;
    }
    // let's assume the page was deleted
    this->ui->pushButton->setText(_l("deleted"));
    HUGGLE_DEBUG("Deletion result: " + this->qDelete->Result->Data, 2);
    this->tDelete->stop();
}

void DeleteForm::Failed(QString Reason)
{
    QMessageBox *_b = new QMessageBox();
    _b->setWindowTitle(_l("delete-e2"));
    _b->setText(_l("delete-edsc", Reason));
    _b->exec();
    delete _b;
    this->tDelete->stop();
    delete this->tDelete;
    this->tDelete = nullptr;
    this->qDelete.Delete();
    this->qTalk.Delete();
    this->qToken.Delete();
    this->qTokenOfTalkPage.Delete();
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
    this->GetToken();
}

void DeleteForm::on_pushButton_2_clicked()
{
    this->close();
}
