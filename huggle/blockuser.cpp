//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "blockuser.hpp"
#include <QtXml>
#include <QLineEdit>
#include <QMessageBox>
#include "exception.hpp"
#include "wikiutil.hpp"
#include "querypool.hpp"
#include "localization.hpp"
#include "syslog.hpp"
#include "configuration.hpp"
#include "ui_blockuser.h"

using namespace Huggle;

BlockUser::BlockUser(QWidget *parent) : QDialog(parent), ui(new Ui::BlockUser)
{
    this->ui->setupUi(this);
    // we should initialise every variable
    this->BlockToken = "";
    this->user = nullptr;
    this->qTokenApi = nullptr;
    this->t0 = new QTimer(this);
    connect(this->t0, SIGNAL(timeout()), this, SLOT(onTick()));
    this->qUser = nullptr;
    this->ui->comboBox->addItem(Configuration::HuggleConfiguration->ProjectConfig->BlockReason);
    int x = 0;
    while (Configuration::HuggleConfiguration->ProjectConfig->BlockExpiryOptions.count() > x)
    {
        this->ui->comboBox_2->addItem(Configuration::HuggleConfiguration->ProjectConfig->BlockExpiryOptions.at(x));
        x++;
    }
}

BlockUser::~BlockUser()
{
    GC_DECREF(this->qUser);
    GC_DECREF(this->qTokenApi);
    delete this->t0;
    delete this->ui;
}

void BlockUser::SetWikiUser(WikiUser *User)
{
    if (User == nullptr)
    {
        throw new Huggle::Exception("WikiUser *User can't be NULL", "void BlockUser::SetWikiUser(WikiUser *User)");
    }
    this->user = User;
    if (this->user->IsIP())
    {
        this->ui->checkBox_5->setEnabled(true);
        this->ui->comboBox_2->lineEdit()->setText(Huggle::Configuration::HuggleConfiguration->ProjectConfig->BlockTimeAnon);
        this->ui->checkBox_5->setChecked(true);
    } else
    {
        this->ui->comboBox_2->lineEdit()->setText(Huggle::Configuration::HuggleConfiguration->ProjectConfig->BlockTime);
    }
}

void BlockUser::GetToken()
{
    // Let's get a token before anything
    this->qTokenApi = new ApiQuery();
    this->qTokenApi->SetAction(ActionQuery);
    this->qTokenApi->Parameters = "prop=info&intoken=block&titles=User:" +
            QUrl::toPercentEncoding(this->user->Username);
    this->qTokenApi->Target = _l("block-token-1", this->user->Username);
    this->qTokenApi->IncRef();
    QueryPool::HugglePool->AppendQuery(this->qTokenApi);
    this->qTokenApi->Process();
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
        case 2:
            this->Recheck();
            return;
    }
    this->t0->stop();
}

void BlockUser::CheckToken()
{
    if (this->qTokenApi == nullptr || !this->qTokenApi->IsProcessed())
        return;
    if (this->qTokenApi->Result->Failed)
    {
        this->Failed(_l("block-token-e1", this->qTokenApi->Result->ErrorMessage));
        return;
    }
    QDomDocument d;
    d.setContent(this->qTokenApi->Result->Data);
    QDomNodeList l = d.elementsByTagName("page");
    if (l.count() == 0)
    {
        Huggle::Syslog::HuggleLogs->DebugLog(this->qTokenApi->Result->Data);
        this->Failed(_l("block-error-no-info"));
        return;
    }
    QDomElement element = l.at(0).toElement();
    if (!element.attributes().contains("blocktoken"))
    {
        this->Failed(_l("no-token"));
        return;
    }
    this->BlockToken = element.attribute("blocktoken");
    this->QueryPhase++;
    this->qTokenApi->DecRef();
    this->qTokenApi = nullptr;
    Huggle::Syslog::HuggleLogs->DebugLog("Block token for " + this->user->Username + ": " + this->BlockToken);

    // let's block them
    this->qUser = new ApiQuery();
    QString nocreate = "";
    if (this->ui->checkBox_4->isChecked())
        nocreate = "&nocreate=";
    QString anononly = "";
    if (this->ui->checkBox_5->isChecked())
        anononly = "&anononly=";
    QString noemail = "";
    if (this->ui->checkBox_2->isChecked())
        noemail = "&noemail=";
    QString autoblock = "";
    if (!this->ui->checkBox_3->isChecked())
        autoblock = "&autoblock=";
    QString allowusertalk = "";
    if (!this->ui->checkBox->isChecked())
        allowusertalk = "&allowusertalk=";
    this->qUser->SetAction(ActionQuery);
    this->qUser->Parameters = "action=block&user=" +  QUrl::toPercentEncoding(this->user->Username) + "&reason="
            + QUrl::toPercentEncoding(this->ui->comboBox->currentText()) + "&expiry="
            + QUrl::toPercentEncoding(this->ui->comboBox_2->currentText()) + nocreate + anononly
            + noemail + autoblock + allowusertalk + "&token=" + QUrl::toPercentEncoding(BlockToken);
    this->qUser->Target = _l("blocking", this->user->Username);
    this->qUser->UsingPOST = true;
    this->qUser->IncRef();
    QueryPool::HugglePool->AppendQuery(this->qUser);
    this->qUser->Process();
}

void BlockUser::Block()
{
    if (this->qUser == nullptr || !this->qUser->IsProcessed())
        return;
    if (this->qUser->Result->Failed)
    {
        this->Failed(_l("block-fail", this->qUser->Result->ErrorMessage));
        return;
    }
    QDomDocument d;
    d.setContent(this->qUser->Result->Data);
    QDomNodeList l = d.elementsByTagName("error");
    if (l.count() > 0)
    {
        QDomElement node = l.at(0).toElement();
        QString reason = this->qUser->Result->Data;
        if (node.attributes().contains("info"))
        {
            reason = node.attribute("info");
        }
        QMessageBox mb;
        mb.setWindowTitle(_l("error"));
        mb.setText(_l("block-fail", reason));
        mb.exec();
        this->ui->pushButton->setText("Block");
        this->qUser->Result->Failed = true;
        this->qUser->Result->ErrorMessage = "Unable to block: " + reason;
        this->qUser->DecRef();
        this->qUser = nullptr;
        this->ui->pushButton->setEnabled(true);
        this->t0->stop();
        return;
    }
    // let's assume the user was blocked
    Huggle::Syslog::HuggleLogs->DebugLog(this->qUser->Result->Data);
    this->ui->pushButton->setText("Blocked");
    Huggle::Syslog::HuggleLogs->DebugLog("block result: " + this->qUser->Result->Data, 2);
    this->qUser->DecRef();
    this->qUser = nullptr;
    this->t0->stop();
    this->sendBlockNotice(nullptr);
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
    this->t0 = nullptr;
    this->ui->pushButton->setEnabled(true);
    GC_DECREF(this->qTokenApi);
    GC_DECREF(this->qUser);
}

void BlockUser::on_pushButton_clicked()
{
    this->GetToken();
    // disable the button so that user can't click it multiple times
    this->ui->pushButton->setEnabled(false);
}

void BlockUser::sendBlockNotice(ApiQuery *dependency)
{
    QString blocknotice;
    if (this->ui->comboBox_2->currentText() != "indefinite")
    {
        blocknotice = Configuration::HuggleConfiguration->ProjectConfig->BlockMessage;
        blocknotice = blocknotice.replace("$1", this->ui->comboBox_2->currentText());
        blocknotice = blocknotice.replace("$2", this->ui->comboBox->currentText());
    }else
    {
        blocknotice = Configuration::HuggleConfiguration->ProjectConfig->BlockMessageIndef;
        blocknotice = blocknotice.replace("$1", this->ui->comboBox->currentText());
    }
    QString blocksum = Configuration::HuggleConfiguration->ProjectConfig->BlockSummary;
    WikiUtil::MessageUser(user, blocknotice, "Blocked", blocksum, true, dependency, false, false, true);
}


void Huggle::BlockUser::on_pushButton_3_clicked()
{
    if (this->qUser != nullptr)
        return;
    this->ui->pushButton_3->setEnabled(false);
    this->ui->pushButton->setEnabled(false);
    this->qUser = new ApiQuery(ActionQuery);
    this->qUser->Target = "user";
    this->qUser->Parameters = "list=blocks&";
    if (!this->user->IsIP())
    {
        this->qUser->Parameters += "bkusers=" + QUrl::toPercentEncoding(this->user->Username);
    } else
    {
        this->qUser->Parameters += "bkip=" + QUrl::toPercentEncoding(this->user->Username);
    }
    this->qUser->IncRef();
    this->qUser->Process();
    this->QueryPhase = 2;
    this->t0->start();
}

void BlockUser::Recheck()
{
    if (this->qUser == nullptr)
        throw new Huggle::Exception("user must not be NULLPTR",  "void BlockUser::Recheck()");
    if (this->qUser->IsProcessed())
    {
        QDomDocument d;
        d.setContent(this->qUser->Result->Data);
        QMessageBox mb;
        mb.setWindowTitle("Result");
        QDomNodeList l = d.elementsByTagName("block");
        if (l.count() > 0)
        {
            mb.setText("User is already blocked");
            this->user->IsBanned = true;
            this->user->Update();
        } else
        {
            mb.setText("User is not blocked");
        }
        mb.exec();
        this->qUser->DecRef();
        this->qUser = nullptr;
        this->t0->stop();
        this->ui->pushButton_3->setEnabled(true);
        this->ui->pushButton->setEnabled(true);
    }
}
