//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include <QtXml>
#include <QLineEdit>
#include "deleteform.hpp"
#include "ui_deleteform.h"

using namespace Huggle;

DeleteForm::DeleteForm(QWidget *parent) : QDialog(parent), ui(new Ui::DeleteForm)
{
    this->ui->setupUi(this);
    int xx = 0;
    while (xx < Configuration::HuggleConfiguration->ProjectConfig_DeletionSummaries.count())
    {
        this->ui->comboBox->addItem(Configuration::HuggleConfiguration->ProjectConfig_DeletionSummaries.at(xx));
        xx++;
    }
    this->page = NULL;
    this->tDelete = NULL;
    this->DeleteToken = "";
    this->qDelete = NULL;
    this->TalkPage = NULL;
    this->ui->comboBox->setCurrentIndex(0);
    this->qTokenOfTalkPage = NULL;
    this->PageUser = NULL;
    this->qTalk = NULL;
    this->qToken = NULL;
}

DeleteForm::~DeleteForm()
{
    delete this->ui;
    delete this->page;
    delete this->TalkPage;
}

void DeleteForm::SetPage(WikiPage *Page, WikiUser *User)
{
    if (Page == NULL)
    {
        throw new Exception("Page must not be NULL", "void DeleteForm::setPage(WikiPage *Page)");
    }
    this->page = new WikiPage(Page);
    if (this->page->IsTalk())
    {
        this->ui->checkBox_2->setChecked(false);
        this->ui->checkBox_2->setEnabled(false);
    }
    this->setWindowTitle(Localizations::HuggleLocalizations->Localize("delete-title", Page->PageName));
    this->PageUser = User;
}

void DeleteForm::GetToken()
{
    this->qToken = new ApiQuery();
    this->qToken->SetAction(ActionQuery);
    this->qToken->Parameters = "action=query&prop=info&intoken=delete&titles=" + QUrl::toPercentEncoding(this->page->PageName);
    this->qToken->Target = Localizations::HuggleLocalizations->Localize("delete-token01", this->page->PageName);
    this->qToken->RegisterConsumer(HUGGLECONSUMER_DELETEFORM);
    QueryPool::HugglePool->AppendQuery(this->qToken);
    this->qToken->Process();
    if (this->TalkPage != NULL)
    {
        this->qTokenOfTalkPage = new ApiQuery();
        this->qTokenOfTalkPage->SetAction(ActionQuery);
        this->qTokenOfTalkPage->Parameters = "action=query&prop=info&intoken=delete&titles=" + QUrl::toPercentEncoding(this->TalkPage->PageName);
        this->qTokenOfTalkPage->Target = Localizations::HuggleLocalizations->Localize("delete-token01", this->TalkPage->PageName);
        this->qTokenOfTalkPage->RegisterConsumer(HUGGLECONSUMER_DELETEFORM);
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
    if (this->qToken == NULL)
    {
        return;
    }
    if (!this->qToken->IsProcessed())
    {
        return;
    }
    if (this->qToken->Result->Failed)
    {
        this->Failed(Localizations::HuggleLocalizations->Localize("delete-error-token", this->qToken->Result->ErrorMessage));
        return;
    }
    QDomDocument d;
    QDomNodeList l;
    if (this->TalkPage != NULL)
    {
        if (this->qTokenOfTalkPage == NULL)
        {
            return;
        }
        if (!this->qTokenOfTalkPage->IsProcessed())
        {
            return;
        }
        if (this->qTokenOfTalkPage->Result->Failed)
        {
            this->Failed(Localizations::HuggleLocalizations->Localize("delete-error-token", this->qToken->Result->ErrorMessage));
            return;
        }
        d.setContent(this->qTokenOfTalkPage->Result->Data);
        l = d.elementsByTagName("page");
        if (l.count() == 0)
        {
            Huggle::Syslog::HuggleLogs->DebugLog(this->qTokenOfTalkPage->Result->Data);
            this->Failed(Localizations::HuggleLocalizations->Localize("delete-failed-no-info"));
            return;
        }
        QDomElement element = l.at(0).toElement();
        if (!element.attributes().contains("deletetoken"))
        {
            this->Failed(Localizations::HuggleLocalizations->Localize("delete-token02"));
            return;
        }
        this->DeleteToken2 = element.attribute("deletetoken");
        this->qTokenOfTalkPage->UnregisterConsumer(HUGGLECONSUMER_DELETEFORM);
        this->qTokenOfTalkPage = NULL;
        Huggle::Syslog::HuggleLogs->DebugLog("Delete token for " + this->TalkPage->PageName + ": " + this->DeleteToken2);

        // let's delete the page
        this->qTalk = new ApiQuery();
        this->qTalk->SetAction(ActionDelete);
        this->qTalk->Parameters = "title=" + QUrl::toPercentEncoding(this->TalkPage->PageName)
                + "&reason=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->ProjectConfig_AssociatedDelete);
                + "&token=" + QUrl::toPercentEncoding(this->DeleteToken2);
        this->qTalk->Target = "Deleting "  + this->TalkPage->PageName;
        this->qTalk->UsingPOST = true;
        this->qTalk->RegisterConsumer(HUGGLECONSUMER_DELETEFORM);
        QueryPool::HugglePool->AppendQuery(this->qTalk);
        this->qTalk->Process();
    }
    d.setContent(this->qToken->Result->Data);
    l = d.elementsByTagName("page");
    if (l.count() == 0)
    {
        Huggle::Syslog::HuggleLogs->DebugLog(this->qToken->Result->Data);
        this->Failed(Huggle::Localizations::HuggleLocalizations->Localize("delete-failed-no-info"));
        return;
    }
    QDomElement element = l.at(0).toElement();
    if (!element.attributes().contains("deletetoken"))
    {
        this->Failed(Localizations::HuggleLocalizations->Localize("delete-token02"));
        return;
    }
    this->DeleteToken = element.attribute("deletetoken");
    this->delQueryPhase++;
    this->qToken->UnregisterConsumer(HUGGLECONSUMER_DELETEFORM);
    this->qToken = NULL;
    Huggle::Syslog::HuggleLogs->DebugLog("Delete token for " + this->page->PageName + ": " + this->DeleteToken);

    // let's delete the page
    this->qDelete = new ApiQuery();
    this->qDelete->SetAction(ActionDelete);
    this->qDelete->Parameters = "title=" + QUrl::toPercentEncoding(this->page->PageName)
            + "&reason=" + QUrl::toPercentEncoding(this->ui->comboBox->lineEdit()->text())
            + "&token=" + QUrl::toPercentEncoding(this->DeleteToken);
    this->qDelete->Target = "Deleting "  + this->page->PageName;
    this->qDelete->UsingPOST = true;
    this->qDelete->RegisterConsumer(HUGGLECONSUMER_DELETEFORM);
    QueryPool::HugglePool->AppendQuery(qDelete);
    this->qDelete->Process();
}

void DeleteForm::Delete()
{
    if (this->qDelete == NULL)
    {
        return;
    }

    if (!this->qDelete->IsProcessed())
    {
        return;
    }

    if (this->qDelete->Result->Failed)
    {
        this->Failed(Huggle::Localizations::HuggleLocalizations->Localize("delete-e1", this->qDelete->Result->ErrorMessage));
        return;
    }
    // let's assume the page was deleted
    this->ui->pushButton->setText(Huggle::Localizations::HuggleLocalizations->Localize("deleted"));
    Huggle::Syslog::HuggleLogs->DebugLog("Deletion result: " + this->qDelete->Result->Data, 2);
    this->qDelete->UnregisterConsumer(HUGGLECONSUMER_DELETEFORM);
    this->tDelete->stop();
}

void DeleteForm::Failed(QString Reason)
{
    QMessageBox *_b = new QMessageBox();
    _b->setWindowTitle(Huggle::Localizations::HuggleLocalizations->Localize("delete-e2"));
    _b->setText(Huggle::Localizations::HuggleLocalizations->Localize("delete-edsc", Reason));
    _b->exec();
    delete _b;
    this->tDelete->stop();
    delete this->tDelete;
    this->tDelete = NULL;
    this->ui->pushButton->setEnabled(true);
    if (this->qToken != NULL)
    {
        this->qToken->UnregisterConsumer(HUGGLECONSUMER_DELETEFORM);
        this->qToken = NULL;
    }
    if (this->qTokenOfTalkPage != NULL)
    {
        this->qTokenOfTalkPage->UnregisterConsumer(HUGGLECONSUMER_DELETEFORM);
        this->qTokenOfTalkPage = NULL;
    }
    if (this->qDelete != NULL)
    {
        this->qDelete->UnregisterConsumer(HUGGLECONSUMER_DELETEFORM);
    }
    this->qDelete = NULL;
    this->qTalk = NULL;
}

void DeleteForm::on_pushButton_clicked()
{
    if (this->ui->checkBox_2->isChecked())
    {
        this->TalkPage = this->page->RetrieveTalk();
        if (this->TalkPage == NULL)
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
