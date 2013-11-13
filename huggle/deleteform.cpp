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

#if !PRODUCTION_BUILD
using namespace Huggle;

DeleteForm::DeleteForm(QWidget *parent) : QDialog(parent), ui(new Ui::DeleteForm)
{
    ui->setupUi(this);
	this->page = NULL;
	this->dt = NULL;
	this->deletetoken = "";
	this->delquery = NULL;
	this->tokenquery = NULL;
}

DeleteForm::~DeleteForm()
{
    delete ui;
	delete page;
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
	this->tokenquery = new ApiQuery();
	tokenquery->SetAction(ActionQuery);
	tokenquery->Parameters = "action=query&prop=info&intoken=delete&titles=" + QUrl::toPercentEncoding(this->page->PageName);
    /// \todo LOCALIZE ME
	tokenquery->Target = "Getting token to delete " + this->page->PageName;
    tokenquery->RegisterConsumer(HUGGLECONSUMER_DELETEFORM);
    Core::HuggleCore->AppendQuery(tokenquery);
	tokenquery->Process();

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
	if (tokenquery == NULL)
	{
		return;
	}
	if (!tokenquery->Processed())
	{
		return;
	}
	if (this->tokenquery->Result->Failed)
	{
        /// \todo LOCALIZE ME
		Failed("ERROR: Retreiving the delete token failed. The reason provided was: " + this->tokenquery->Result->ErrorMessage);
		return;
	}
	QDomDocument d;
	d.setContent(this->tokenquery->Result->Data);
    QDomNodeList l = d.elementsByTagName("page");
	if (l.count() == 0)
	{
        Core::HuggleCore->DebugLog(this->tokenquery->Result->Data);
        /// \todo LOCALIZE ME
		Failed("no page info was present in query (are you sysop?)");
		return;
	}
	QDomElement element = l.at(0).toElement();
	if (!element.attributes().contains("deletetoken"))
	{
        /// \todo LOCALIZE ME
		Failed("No token");
		return;
	}
	this->deletetoken = element.attribute("deletetoken");
	this->delQueryPhase++;
    this->tokenquery->UnregisterConsumer(HUGGLECONSUMER_DELETEFORM);
	this->tokenquery = NULL;
    Core::HuggleCore->DebugLog("Delete token for " + this->page->PageName + ": " + this->deletetoken);

	// let's delete the page
	this->delquery = new ApiQuery();
    this->delquery->SetAction(ActionDelete);
    delquery->Parameters = "title=" + QUrl::toPercentEncoding(this->page->PageName)
            + "&reason=" + QUrl::toPercentEncoding(ui->comboBox->lineEdit()->text())
            + "&token=" + QUrl::toPercentEncoding(deletetoken);
	delquery->Target = "Deleting "  + this->page->PageName;
	delquery->UsingPOST = true;
    delquery->RegisterConsumer(HUGGLECONSUMER_DELETEFORM);
    Core::HuggleCore->AppendQuery(delquery);
	delquery->Process();
}

void DeleteForm::Delete()
{
	if (this->delquery == NULL)
	{
		return;
	}
	if (!this->delquery->Processed())
	{
		return;
	}

	if (this->delquery->Result->Failed)
	{
        /// \todo LOCALIZE ME
		Failed("page can't be deleted: " + this->delquery->Result->ErrorMessage);
		return;
	}
	// let's assume the page was deleted
	ui->pushButton->setText("deleted");
    Core::HuggleCore->DebugLog("deletion result: " + this->delquery->Result->Data, 2);
    this->delquery->UnregisterConsumer(HUGGLECONSUMER_DELETEFORM);
	this->dt->stop();
}

void DeleteForm::Failed(QString reason)
{
	QMessageBox *_b = new QMessageBox();
    /// \todo LOCALIZE ME
	_b->setWindowTitle("Unable to delete page");
    /// \todo LOCALIZE ME
    _b->setText("Unable to delete the page because " + reason);
	_b->exec();
	delete _b;
	this->dt->stop();
	delete this->dt;
	this->dt = NULL;
	ui->pushButton->setEnabled(true);
	if (this->tokenquery != NULL)
	{
        tokenquery->UnregisterConsumer(HUGGLECONSUMER_DELETEFORM);
	}
	if (this->delquery != NULL)
	{
        delquery->UnregisterConsumer(HUGGLECONSUMER_DELETEFORM);
	}
	this->delquery = NULL;
	this->tokenquery = NULL;
}


void DeleteForm::on_pushButton_clicked()
{
	this->getToken();
	ui->pushButton->setEnabled(false);
}

void DeleteForm::on_pushButton_2_clicked()
{
	this->hide();
}

#endif
