//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "blockuser.hpp"
#include "ui_blockuser.h"

#if !PRODUCTION_BUILD
using namespace Huggle;

BlockUser::BlockUser(QWidget *parent) : QDialog(parent), ui(new Ui::BlockUser)
{
    ui->setupUi(this);
    // we should initialise every variable
    this->blocktoken = "";
    this->user = NULL;
    this->b = NULL;
    this->t0 = NULL;
    this->tb = NULL;
    ui->comboBox->addItem(Configuration::LocalConfig_BlockReason);
}

BlockUser::~BlockUser()
{
    delete t0;
    delete ui;
}

void BlockUser::SetWikiUser(WikiUser *User)
{
    if (User == NULL)
    {
        throw new Exception("WikiUser *User can't be NULL", "void BlockUser::SetWikiUser(WikiUser *User)");
    }
    this->user = User;
    if (this->user->IsIP())
    {
        ui->checkBox_5->setEnabled(true);
    }
}

void BlockUser::GetToken()
{
    // Let's get a token before anything
    b = new ApiQuery();
    b->SetAction(ActionQuery);
    b->Parameters = "prop=info&intoken=block&titles=User:" +
            QUrl::toPercentEncoding(this->user->Username);
    /// \todo LOCALIZE ME
    b->Target = "Getting token to block" + this->user->Username;
    b->RegisterConsumer("BlockUser::GetToken");
    Core::AppendQuery(b);
    b->Process();

    this->t0 = new QTimer(this);
    connect(this->t0, SIGNAL(timeout()), this, SLOT(onTick()));
    this->QueryPhase = 0;
    this->t0->start(200);
}

void BlockUser::on_pushButton_2_clicked()
{
    this->hide();
}

void BlockUser::onTick()
{
    switch (this->QueryPhase)
    {
        case 0:
            this->CheckToken();
            return;
        case 1:
            this->Block();
            return;
    }
    this->t0->stop();
}

void BlockUser::CheckToken()
{
    if (this->b == NULL)
    {
        return;
    }
    if (!this->b->Processed())
    {
        return;
    }
    if (this->b->Result->Failed)
    {
        /// \todo LOCALIZE ME
        Failed("token can't be retrieved: " + this->b->Result->ErrorMessage);
        return;
    }
    QDomDocument d;
    d.setContent(this->b->Result->Data);
    QDomNodeList l = d.elementsByTagName("user");
    if (l.count() == 0)
    {
        Core::DebugLog(this->b->Result->Data);
        /// \todo LOCALIZE ME
        Failed("no user info was present in query (are you sysop?)");
        return;
    }
    QDomElement element = l.at(0).toElement();
    if (!element.attributes().contains("blocktoken"))
    {
        /// \todo LOCALIZE ME
        Failed("No token");
        return;
    }
    this->blocktoken = element.attribute("blocktoken");
    this->QueryPhase++;
    this->b->UnregisterConsumer("BlockUser::GetToken");
    this->b = NULL;
    Core::DebugLog("Block token for " + this->user->Username + ": " + this->blocktoken);

    // let's block them
    this->tb = new ApiQuery();
    this->tb->SetAction(ActionQuery);
    if (user->IsIP())
    {
        tb->Parameters = "action=block&user=" +  QUrl::toPercentEncoding(this->user->Username) + "reason="
                + QUrl::toPercentEncoding(Configuration::LocalConfig_BlockReason) + "expiry="
                + QUrl::toPercentEncoding(Configuration::LocalConfig_BlockTimeAnon) + "token="
                + QUrl::toPercentEncoding(blocktoken);

    }else
    {
        tb->Parameters = "action=block&user=" + QUrl::toPercentEncoding(this->user->Username) + "reason="
                + QUrl::toPercentEncoding(Configuration::LocalConfig_BlockReason) + "token="
                + QUrl::toPercentEncoding(blocktoken);
    }
    /// \todo LOCALIZE ME
    tb->Target = "Blocking" + this->user->Username;
    tb->UsingPOST = true;
    tb->RegisterConsumer("BlockUser::on_pushButton_clicked()");
    Core::AppendQuery(tb);
    tb->Process();
}

void BlockUser::Block()
{
    if (this->tb == NULL)
    {
        return;
    }
    if (!this->tb->Processed())
    {
        return;
    }

    if (this->tb->Result->Failed)
    {
        /// \todo LOCALIZE ME
        Failed("user can't be blocked: " + this->tb->Result->ErrorMessage);
        return;
    }
    // let's assume the user was blocked
    ui->pushButton->setText("Blocked");
    Core::DebugLog("block result: " + this->tb->Result->Data, 2);
    this->tb->UnregisterConsumer("BlockUser::on_pushButton_clicked()");
    this->t0->stop();
}

void BlockUser::Failed(QString reason)
{
    QMessageBox *_b = new QMessageBox();
    _b->setWindowTitle("Unable to block user");
    _b->setText("Unable to block the user because " + reason);
    _b->exec();
    delete _b;
    this->t0->stop();
    delete this->t0;
    this->t0 = NULL;
    ui->pushButton->setEnabled(true);
    if (this->b != NULL)
    {
        b->UnregisterConsumer("BlockUser::GetToken");
    }
    if (this->tb != NULL)
    {
        tb->UnregisterConsumer("BlockUser::on_pushButton_clicked()");
    }
    this->tb = NULL;
    this->b = NULL;
}

void BlockUser::on_pushButton_clicked()
{
    this->GetToken();
    this->sendBlockNotice(dependency);
    // disable the button so that user can't click it multiple times
    ui->pushButton->setEnabled(false);
}

void BlockUser::sendBlockNotice(ApiQuery *dependency)
{
    if (dependency == NULL)
    {
        throw new Exception("ApiQuery *dependency", "void BlockUser::sendBlockNotice(ApiQuery *dependency)");
    }
    QString blocknotice;
    if (user->IsIP())
    {
        blocknotice = Configuration::LocalConfig_BlockMessage;
    }else
    {
        blocknotice = Configuration::LocalConfig_BlockMessageIndef;
    }
    QString blocksum = Configuration::LocalConfig_BlockSummary;
    Core::MessageUser(user, blocknotice, "Blocked", blocksum, true, dependency);
}

#endif

