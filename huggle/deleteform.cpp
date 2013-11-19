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
    this->qToken = NULL;
}

DeleteForm::~DeleteForm()
{
    delete this->ui;
    delete this->page;
}

void DeleteForm::setPage(WikiPage *Page)
{
    if (Page == NULL)
    {
        throw new Exception("Page must not be NULL", "void DeleteForm::setPage(WikiPage *Page)");
    }
	this->page = Page;
}

void DeleteForm::getToken()
{
    this->qToken = new ApiQuery();
    this->qToken->SetAction(ActionQuery);
    this->qToken->Parameters = "action=query&prop=info&intoken=delete&titles=" + QUrl::toPercentEncoding(this->page->PageName);
    /// \todo LOCALIZE ME
    this->qToken->Target = "Getting token to delete " + this->page->PageName;
    this->qToken->RegisterConsumer(HUGGLECONSUMER_DELETEFORM);
    Core::HuggleCore->AppendQuery(this->qToken);
    this->qToken->Process();

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
        /// \todo LOCALIZE ME
        this->Failed("ERROR: Retreiving the delete token failed. The reason provided was: " + this->qToken->Result->ErrorMessage);
		return;
	}
	QDomDocument d;
    d.setContent(this->qToken->Result->Data);
    QDomNodeList l = d.elementsByTagName("page");
	if (l.count() == 0)
	{
        Huggle::Syslog::HuggleLogs->DebugLog(this->qToken->Result->Data);
        /// \todo LOCALIZE ME
        this->Failed("no page info was present in query (are you sysop?)");
		return;
	}
	QDomElement element = l.at(0).toElement();
	if (!element.attributes().contains("deletetoken"))
	{
        /// \todo LOCALIZE ME
        this->Failed("No token");
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
        /// \todo LOCALIZE ME
        this->Failed("page can't be deleted: " + this->qDelete->Result->ErrorMessage);
		return;
	}
	// let's assume the page was deleted
    this->ui->pushButton->setText("deleted");
    Huggle::Syslog::HuggleLogs->DebugLog("deletion result: " + this->qDelete->Result->Data, 2);
    this->qDelete->UnregisterConsumer(HUGGLECONSUMER_DELETEFORM);
	this->dt->stop();
}

void DeleteForm::Failed(QString Reason)
{
	QMessageBox *_b = new QMessageBox();
    /// \todo LOCALIZE ME
	_b->setWindowTitle("Unable to delete page");
    /// \todo LOCALIZE ME
    _b->setText("Unable to delete the page because " + Reason);
	_b->exec();
	delete _b;
	this->dt->stop();
	delete this->dt;
    this->dt = NULL;
    this->ui->pushButton->setEnabled(true);
    if (this->qToken != NULL)
	{
        this->qToken->UnregisterConsumer(HUGGLECONSUMER_DELETEFORM);
	}
    if (this->qDelete != NULL)
	{
        this->qDelete->UnregisterConsumer(HUGGLECONSUMER_DELETEFORM);
	}
    this->qDelete = NULL;
    this->qToken = NULL;
}

void DeleteForm::on_pushButton_clicked()
{
	this->getToken();
    this->ui->pushButton->setEnabled(false);
}

void DeleteForm::on_pushButton_2_clicked()
{
	this->hide();
}
