//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "deleteform.h"
#include "ui_deleteform.h"

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
	this->page = Page;
}

void DeleteForm::getToken()
{
	this->tokenquery = new ApiQuery();
	tokenquery->SetAction(ActionQuery);
	tokenquery->Parameters = "action=query&prop=info&intoken=delete&titles=" + QUrl::toPercentEncoding(this->page->PageName);
	tokenquery->Target = "Getting token to delete " + this->page->PageName;
	tokenquery->RegisterConsumer("DeleteForm::getToken()");
	Core::AppendQuery(tokenquery);
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
		Failed("ERROR: Retreiving the delete token failed. The reason provided was: " + this->tokenquery->Result->ErrorMessage);
		return;
	}
	QDomDocument d;
	d.setContent(this->tokenquery->Result->Data);
	QDomNodeList l = d.elementsByTagName("titles");
	if (l.count() == 0)
	{
		Core::DebugLog(this->tokenquery->Result->Data);
		Failed("no page info was present in query (are you sysop?)");
		return;
	}
	QDomElement element = l.at(0).toElement();
	if (!element.attributes().contains("deletetoken"))
	{
		Failed("No token");
		return;
	}
	this->deletetoken = element.attribute("deletetoken");
	this->delQueryPhase++;
	this->tokenquery->UnregisterConsumer("DeleteForm::getToken");
	this->tokenquery = NULL;
	Core::DebugLog("Delete token for " + this->page->PageName + ": " + this->deletetoken);

	// let's delete the page
	this->delquery = new ApiQuery();
	this->delquery->SetAction(ActionQuery);
	delquery->Parameters = "action=delete&titles=" + QUrl::toPercentEncoding(this->page->PageName) + "reason=&" + "token=" + QUrl::toPercentEncoding(deletetoken);
	delquery->Target = "Deleting "  + this->page->PageName;
	delquery->UsingPOST = true;
	delquery->RegisterConsumer("DeleteForm::on_pushButton_clicked()");
	Core::AppendQuery(delquery);
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
		Failed("page can't be deleted: " + this->delquery->Result->ErrorMessage);
		return;
	}
	// let's assume the page was deleted
	ui->pushButton->setText("deleted");
	Core::DebugLog("deletion result: " + this->delquery->Result->Data, 2);
	this->delquery->UnregisterConsumer("DeleteForm::on_pushButton_clicked()");
	this->dt->stop();
}

void DeleteForm::Failed(QString reason)
{
	QMessageBox *_b = new QMessageBox();
	_b->setWindowTitle("Unable to delete page");
	_b->setText("Unable to block the user because " + reason);
	_b->exec();
	delete _b;
	this->dt->stop();
	delete this->dt;
	this->dt = NULL;
	ui->pushButton->setEnabled(true);
	if (this->tokenquery != NULL)
	{
		tokenquery->UnregisterConsumer("DeleteForm::GetToken");
	}
	if (this->delquery != NULL)
	{
		delquery->UnregisterConsumer("DeleteForm::on_pushButton_clicked()");
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
