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
    this->ui->setupUi(this);
    // we should initialise every variable
    this->BlockToken = "";
    this->user = NULL;
    this->b = NULL;
    this->Dependency = NULL;
    this->t0 = NULL;
    this->tb = NULL;
    this->ui->comboBox->addItem(Configuration::HuggleConfiguration->LocalConfig_BlockReason);
}

BlockUser::~BlockUser()
{
    delete this->t0;
    delete this->ui;
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
        this->ui->checkBox_5->setEnabled(true);
    }
}

void BlockUser::GetToken()
{
    // Let's get a token before anything
    this->b = new ApiQuery();
    this->b->SetAction(ActionQuery);
    this->b->Parameters = "prop=info&intoken=block&titles=User:" +
            QUrl::toPercentEncoding(this->user->Username);
    /// \todo LOCALIZE ME
    this->b->Target = "Getting token to block" + this->user->Username;
    this->b->RegisterConsumer("BlockUser::GetToken");
    Core::HuggleCore->AppendQuery(this->b);
    this->b->Process();
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
        this->Failed("token can't be retrieved: " + this->b->Result->ErrorMessage);
        return;
    }
    QDomDocument d;
    d.setContent(this->b->Result->Data);
    QDomNodeList l = d.elementsByTagName("user");
    if (l.count() == 0)
    {
        Huggle::Syslog::HuggleLogs->DebugLog(this->b->Result->Data);
        /// \todo LOCALIZE ME
        this->Failed("no user info was present in query (are you sysop?)");
        return;
    }
    QDomElement element = l.at(0).toElement();
    if (!element.attributes().contains("blocktoken"))
    {
        /// \todo LOCALIZE ME
        this->Failed("No token");
        return;
    }
    this->BlockToken = element.attribute("blocktoken");
    this->QueryPhase++;
    this->b->UnregisterConsumer("BlockUser::GetToken");
    this->b = NULL;
    Huggle::Syslog::HuggleLogs->DebugLog("Block token for " + this->user->Username + ": " + this->BlockToken);

    // let's block them
    this->tb = new ApiQuery();
    this->tb->SetAction(ActionQuery);
    if (user->IsIP())
    {
        tb->Parameters = "action=block&user=" +  QUrl::toPercentEncoding(this->user->Username) + "reason="
                + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->LocalConfig_BlockReason) + "expiry="
                + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->LocalConfig_BlockTimeAnon) + "token="
                + QUrl::toPercentEncoding(BlockToken);

    }else
    {
        tb->Parameters = "action=block&user=" + QUrl::toPercentEncoding(this->user->Username) + "reason="
                + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->LocalConfig_BlockReason) + "token="
                + QUrl::toPercentEncoding(BlockToken);
    }
    /// \todo LOCALIZE ME
    this->tb->Target = "Blocking " + this->user->Username;
    this->tb->UsingPOST = true;
    this->tb->RegisterConsumer("BlockUser::on_pushButton_clicked()");
    Core::HuggleCore->AppendQuery(this->tb);
    this->tb->Process();
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
        this->Failed("user can't be blocked: " + this->tb->Result->ErrorMessage);
        return;
    }
    // let's assume the user was blocked
    this->ui->pushButton->setText("Blocked");
    Huggle::Syslog::HuggleLogs->DebugLog("block result: " + this->tb->Result->Data, 2);
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
    this->ui->pushButton->setEnabled(true);
    if (this->b != NULL)
    {
        this->b->UnregisterConsumer("BlockUser::GetToken");
    }
    if (this->tb != NULL)
    {
        this->tb->UnregisterConsumer("BlockUser::on_pushButton_clicked()");
    }
    this->tb = NULL;
    this->b = NULL;
}

void BlockUser::on_pushButton_clicked()
{
    this->GetToken();
    this->sendBlockNotice(this->Dependency);
    // disable the button so that user can't click it multiple times
    this->ui->pushButton->setEnabled(false);
}

void BlockUser::sendBlockNotice(ApiQuery *dependency)
{
    QString blocknotice;
    if (user->IsIP())
    {
        blocknotice = Configuration::HuggleConfiguration->LocalConfig_BlockMessage;
    }else
    {
        blocknotice = Configuration::HuggleConfiguration->LocalConfig_BlockMessageIndef;
    }
    QString blocksum = Configuration::HuggleConfiguration->LocalConfig_BlockSummary;
    Core::HuggleCore->MessageUser(user, blocknotice, "Blocked", blocksum, true, dependency);
}

#endif

