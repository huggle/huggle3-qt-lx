//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "blockuser.h"
#include "ui_blockuser.h"

using namespace Huggle;

BlockUser::BlockUser(QWidget *parent) : QDialog(parent), ui(new Ui::BlockUser)
{
    ui->setupUi(this);
    this->user = NULL;
    this->b = NULL;
    this->tb = NULL;
    ui->comboBox->addItem(Configuration::LocalConfig_BlockReason);
    if (user->IP)
    {
        ui->checkBox_5->setEnabled(true);
    }
}

BlockUser::~BlockUser()
{
    delete ui;
    delete b;
	delete tb;

}
void BlockUser::GetToken()
{
	// Let's get a token before anything
	if (b == NULL)
	{
		delete b;
		return;
	}
	b = new ApiQuery();
	b->SetAction(ActionQuery);
	b->Parameters = "prop=info&intoken=block&titles=User:" + this->user->Username;
	b->Target = "Getting token to block" + this->user->Username;
	b->RegisterConsumer("BlockUser::GetToken");
	Core::AppendQuery(b);
	b->Process();

	if (blocktoken == "none")
	{
		if (!tb->Processed())
		{
			return;
		}
		if (tb->Result->Failed)
		{
			return;
		}
		QDomDocument d;
		d.setContent(tb->Result->Data);
		QDomNodeList l = d.elementsByTagName("user");
		if (l.count() == 0)
		{
			Core::DebugLog("No page");
			return;
		}
		QDomElement element = l.at(0).toElement();
		if (!element.attributes().contains("blocktoken"))
		{
			Core::DebugLog("No token");
			return;
		}
	}
}
void BlockUser::on_pushButton_2_clicked()
{
	this->hide();
}
void BlockUser::on_pushButton_clicked()
{
	this->GetToken();
	QDomDocument d;
	d.setContent(tb->Result->Data);
	QDomNodeList l = d.elementsByTagName("user");
	QDomElement element = l.at(0).toElement();
	blocktoken = element.attribute("blocktoken");
	tb->SafeDelete(true);
	tb->UnregisterConsumer("BlockUser::on_pushButton_clicked()");
	tb = new ApiQuery();
	tb->SetAction(ActionQuery);
	if (user->IP)
	{
		tb->Parameters = "action=block&user=" + this->user->Username + "reason=" + Configuration::LocalConfig_BlockReason + "expiry=" +
				QUrl::toPercentEncoding(Configuration::LocalConfig_BlockTimeAnon) + "token=" + blocktoken;

	}else
	{
		tb->Parameters = "action=block&user=" + this->user->Username + "reason=" + Configuration::LocalConfig_BlockReason + "token=" + blocktoken;
	}
	tb->Target = "Blocking" + this->user->Username;
	tb->UsingPOST = true;
	tb->RegisterConsumer("BlockUser::on_pushButton_clicked()");
	Core::AppendQuery(tb);
	tb->Process();
}


