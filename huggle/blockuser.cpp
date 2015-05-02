//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "blockuser.hpp"
#include <QLineEdit>
#include <QTimer>
#include <QUrl>
#include "apiquery.hpp"
#include "apiqueryresult.hpp"
#include "exception.hpp"
#include "generic.hpp"
#include "history.hpp"
#include "historyitem.hpp"
#include "mainwindow.hpp"
#include "wikiuser.hpp"
#include "wikiutil.hpp"
#include "wikisite.hpp"
#include "querypool.hpp"
#include "localization.hpp"
#include "syslog.hpp"
#include "configuration.hpp"
#include "ui_blockuser.h"

using namespace Huggle;

BlockUser::BlockUser(QWidget *parent) : HW("blockuser", this, parent), ui(new Ui::BlockUser)
{
    this->ui->setupUi(this);
    // we should initialise every variable
    this->user = nullptr;
    this->ui->checkBox_5->setText(_l("block-anononly"));
    this->ui->checkBox_3->setText(_l("block-autoblock"));
    this->ui->checkBox_4->setText(_l("block-creation"));
    this->ui->checkBox_2->setText(_l("block-email"));
    this->ui->cbMessageTarget->setText(_l("block-message-user"));
    this->ui->label_2->setText(_l("block-duration"));
    this->t0 = new QTimer(this);
    connect(this->t0, SIGNAL(timeout()), this, SLOT(onTick()));
    this->ui->comboBox->addItem(Configuration::HuggleConfiguration->ProjectConfig->BlockReason);
    this->RestoreWindow();
}

BlockUser::~BlockUser()
{
    delete this->user;
    delete this->t0;
    delete this->ui;
}

void BlockUser::SetWikiUser(WikiUser *User)
{
    if (User == nullptr)
    {
        throw new Huggle::NullPointerException("WikiUser *User", BOOST_CURRENT_FUNCTION);
    }
    this->user = new WikiUser(User);
    this->ui->comboBox_2->clear();
    foreach (QString op, User->GetSite()->GetProjectConfig()->BlockExpiryOptions)
        this->ui->comboBox_2->addItem(op);
    this->setWindowTitle(_l("block-title", this->user->Username));
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

void BlockUser::on_pushButton_2_clicked()
{
    this->hide();
}

void BlockUser::onTick()
{
    switch (this->QueryPhase)
    {
        case 1:
            this->Block();
            return;
        case 2:
            this->Recheck();
            return;
    }
    this->t0->stop();
}

void BlockUser::Block()
{
    if (this->qUser == nullptr || !this->qUser->IsProcessed())
        return;
    if (this->qUser->IsFailed())
    {
        this->Failed(_l("block-fail", this->qUser->GetFailureReason()));
        return;
    }
    ApiQueryResultNode *error = this->qUser->GetApiQueryResult()->GetNode("error");
    if (error != nullptr)
    {
        QString reason = this->qUser->Result->Data;
        if (error->Attributes.contains("info"))
        {
            reason = error->GetAttribute("info");
        }
        Generic::pMessageBox(this, _l("error"), _l("block-fail", reason), MessageBoxStyleError, true);
        this->ui->pushButton->setText(_l("block-title", this->user->Username));
        this->qUser->Result->SetError(HUGGLE_EUNKNOWN, "Unable to block: " + reason);
        this->qUser = nullptr;
        this->ui->pushButton->setEnabled(true);
        this->t0->stop();
        return;
    }
    // let's assume the user was blocked
    this->ui->pushButton->setText(_l("block-done", this->user->Username));
    HUGGLE_DEBUG("block result: " + this->qUser->Result->Data, 2);
    HistoryItem *history = new HistoryItem();
    history->Result = _l("successful");
    history->Type = HistoryBlock;
    history->IsRevertable = false;
    history->Target = this->user->Username;
    this->qUser.Delete();
    this->t0->stop();
    if (this->ui->cbMessageTarget->isChecked())
        this->sendBlockNotice(nullptr);
}

void BlockUser::Failed(QString reason)
{
    Generic::pMessageBox(this, "Unable to block user", _l("block-fail", reason),
                         MessageBoxStyleError, true);
    this->t0->stop();
    this->ui->pushButton->setEnabled(true);
    // remove the pointers
    this->qUser.Delete();
}

void BlockUser::on_pushButton_clicked()
{
    // disable the button so that user can't click it multiple times
    this->ui->pushButton->setEnabled(false);
    this->qUser = new ApiQuery(ActionQuery, this->user->GetSite());
    this->QueryPhase = 1;
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
    if (this->ui->checkBox_3->isChecked())
        autoblock = "&autoblock=";
    QString allowusertalk = "";
    if (!this->ui->checkBox->isChecked())
        allowusertalk = "&allowusertalk=";
    this->qUser->Parameters = "action=block&user=" +  QUrl::toPercentEncoding(this->user->Username) + "&reason="
            + QUrl::toPercentEncoding(Configuration::GenerateSuffix(this->ui->comboBox->currentText(),
                                                                    this->user->GetSite()->GetProjectConfig()))
            + "&expiry=" + QUrl::toPercentEncoding(this->ui->comboBox_2->currentText()) + nocreate + anononly
            + noemail + autoblock + allowusertalk + "&token=" + QUrl::toPercentEncoding(this->user->GetSite()->GetProjectConfig()->Token_Csrf);
    this->qUser->Target = _l("block-progress", this->user->Username);
    this->qUser->UsingPOST = true;
    QueryPool::HugglePool->AppendQuery(this->qUser);
    this->qUser->Process();
    this->t0->start(HUGGLE_TIMER);
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
    WikiUtil::MessageUser(user, blocknotice, blocksum, blocksum, true, dependency, false, false);
}

void Huggle::BlockUser::on_pushButton_3_clicked()
{
    if (this->qUser != nullptr)
        return;
    this->ui->pushButton_3->setEnabled(false);
    this->ui->pushButton->setEnabled(false);
    this->qUser = new ApiQuery(ActionQuery, this->user->GetSite());
    this->qUser->Target = "user";
    this->qUser->Parameters = "list=blocks&";
    if (!this->user->IsIP())
    {
        this->qUser->Parameters += "bkusers=" + QUrl::toPercentEncoding(this->user->Username);
    } else
    {
        this->qUser->Parameters += "bkip=" + QUrl::toPercentEncoding(this->user->Username);
    }
    this->qUser->Process();
    this->QueryPhase = 2;
    this->t0->start();
}

void BlockUser::Recheck()
{
    if (this->qUser == nullptr)
        throw new Huggle::NullPointerException("local ApiQuery qUser",  BOOST_CURRENT_FUNCTION);
    if (this->qUser->IsProcessed())
    {
        ApiQueryResultNode *result = this->qUser->GetApiQueryResult()->GetNode("block");
        QString text;
        if (result != nullptr)
        {
            text = _l("warn-alreadyblocked");
            this->user->IsBlocked = true;
            this->user->Update();
        } else
        {
            text = _l("block-not");
        }
        Generic::MessageBox(_l("result"), text, MessageBoxStyleNormal, true);
        this->qUser = nullptr;
        this->t0->stop();
        this->ui->pushButton_3->setEnabled(true);
        this->ui->pushButton->setEnabled(true);
    }
}
