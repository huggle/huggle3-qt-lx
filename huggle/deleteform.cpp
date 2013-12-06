//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "deleteform.hpp"
#include "ui_deleteform.h"

using namespace Huggle;

DeleteForm::DeleteForm(QWidget *parent) : QDialog(parent), ui(new Ui::DeleteForm)
{
    this->ui->setupUi(this);
	this->page = NULL;
	this->dt = NULL;
    this->DeleteToken = "";
    this->qDelete = NULL;
    this->TP = NULL;
    this->qTokenOfTalkPage = NULL;
    this->user = NULL;
    this->qTalk = NULL;
    this->qToken = NULL;
}

DeleteForm::~DeleteForm()
{
    delete this->ui;
    delete this->page;
    delete this->TP;
}

void DeleteForm::setPage(WikiPage *Page, WikiUser *User)
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
    this->user = User;
}

void DeleteForm::getToken()
{
    this->qToken = new ApiQuery();
    this->qToken->SetAction(ActionQuery);
    this->qToken->Parameters = "action=query&prop=info&intoken=delete&titles=" + QUrl::toPercentEncoding(this->page->PageName);
    this->qToken->Target = Localizations::HuggleLocalizations->Localize("delete-token01", this->page->PageName);
    this->qToken->RegisterConsumer(HUGGLECONSUMER_DELETEFORM);
    Core::HuggleCore->AppendQuery(this->qToken);
    this->qToken->Process();
    if (this->TP != NULL)
    {
        this->qTokenOfTalkPage = new ApiQuery();
        this->qTokenOfTalkPage->SetAction(ActionQuery);
        this->qTokenOfTalkPage->Parameters = "action=query&prop=info&intoken=delete&titles=" + QUrl::toPercentEncoding(this->TP->PageName);
        this->qTokenOfTalkPage->Target = Localizations::HuggleLocalizations->Localize("delete-token01", this->TP->PageName);
        this->qTokenOfTalkPage->RegisterConsumer(HUGGLECONSUMER_DELETEFORM);
        Core::HuggleCore->AppendQuery(this->qTokenOfTalkPage);
        this->qTokenOfTalkPage->Process();
    }
    this->dt = new QTimer(this);
    connect(this->dt, SIGNAL(timeout()), this, SLOT(onTick()));
    this->delQueryPhase = 0;
    this->dt->start(200);
}

void DeleteForm::onTick()
{
	switch (this->delQueryPhase)
	{
		case 0:
			this->checkDelToken();
			return;
		case 1:
			this->Delete();
			return;
	}
	this->dt->stop();
}

void DeleteForm::checkDelToken()
{
    if (this->qToken == NULL)
	{
		return;
	}
    if (!this->qToken->Processed())
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
    if (this->TP != NULL)
    {
        if (this->qTokenOfTalkPage == NULL)
        {
            return;
        }
        if (!this->qTokenOfTalkPage->Processed())
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
        Huggle::Syslog::HuggleLogs->DebugLog("Delete token for " + this->TP->PageName + ": " + this->DeleteToken2);

        // let's delete the page
        this->qTalk = new ApiQuery();
        this->qTalk->SetAction(ActionDelete);
        this->qTalk->Parameters = "title=" + QUrl::toPercentEncoding(this->TP->PageName)
                + "&reason=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->LocalConfig_AssociatedDelete);
                + "&token=" + QUrl::toPercentEncoding(this->DeleteToken2);
        this->qTalk->Target = "Deleting "  + this->TP->PageName;
        this->qTalk->UsingPOST = true;
        this->qTalk->RegisterConsumer(HUGGLECONSUMER_DELETEFORM);
        Core::HuggleCore->AppendQuery(this->qTalk);
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
    Core::HuggleCore->AppendQuery(qDelete);
    this->qDelete->Process();
}

void DeleteForm::Delete()
{
    if (this->qDelete == NULL)
	{
		return;
	}

    if (!this->qDelete->Processed())
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
	this->dt->stop();
}

void DeleteForm::Failed(QString Reason)
{
	QMessageBox *_b = new QMessageBox();
    _b->setWindowTitle(Huggle::Localizations::HuggleLocalizations->Localize("delete-e2"));
    _b->setText(Huggle::Localizations::HuggleLocalizations->Localize("delete-edsc", Reason));
	_b->exec();
	delete _b;
	this->dt->stop();
	delete this->dt;
    this->dt = NULL;
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
        this->TP = this->page->RetrieveTalk();
        if (this->TP == NULL)
        {
            this->ui->checkBox_2->setChecked(false);
        }
    }
    this->ui->checkBox_2->setEnabled(false);
    this->ui->comboBox->setEnabled(false);
    this->ui->pushButton->setEnabled(false);
    this->getToken();
}

void DeleteForm::on_pushButton_2_clicked()
{
    this->close();
}
