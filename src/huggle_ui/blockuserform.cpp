//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "blockuserform.hpp"
#include <QLineEdit>
#include <QTimer>
#include <QUrl>
#include <huggle_core/apiquery.hpp>
#include <huggle_core/apiqueryresult.hpp>
#include <huggle_core/exception.hpp>
#include <huggle_core/generic.hpp>
#include <huggle_core/historyitem.hpp>
#include <huggle_core/wikiuser.hpp>
#include <huggle_core/wikiutil.hpp>
#include <huggle_core/wikisite.hpp>
#include <huggle_core/querypool.hpp>
#include <huggle_core/localization.hpp>
#include <huggle_core/syslog.hpp>
#include <huggle_core/configuration.hpp>
#include "ui_blockuserform.h"
#include "mainwindow.hpp"
#include "history.hpp"
#include "uigeneric.hpp"

using namespace Huggle;

BlockUserForm::BlockUserForm(QWidget *parent) : HW("blockuser", this, parent), ui(new Ui::BlockUser)
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
    this->timer = new QTimer(this);
    connect(this->timer, SIGNAL(timeout()), this, SLOT(onTick()));
    this->RestoreWindow();
}

BlockUserForm::~BlockUserForm()
{
    delete this->user;
    delete this->timer;
    delete this->ui;
}

void BlockUserForm::SetWikiUser(WikiUser *User)
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
        this->ui->comboBox_2->lineEdit()->setText(User->GetSite()->GetProjectConfig()->BlockTimeAnon);
        this->ui->checkBox_5->setChecked(true);
    } else
    {
        this->ui->comboBox_2->lineEdit()->setText(User->GetSite()->GetProjectConfig()->BlockTime);
    }
    this->ui->comboBox->addItem(User->GetSite()->ProjectConfig->BlockReason);
}

void BlockUserForm::on_pushButton_2_clicked()
{
    this->hide();
}

void BlockUserForm::onTick()
{
    switch (this->QueryPhase)
    {
        case 1:
            this->Block();
            return;
        case 2:
            this->recheck();
            return;
    }
    this->timer->stop();
}

void BlockUserForm::Block()
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
        UiGeneric::pMessageBox(this, _l("error"), _l("block-fail", reason), MessageBoxStyleError, true);
        this->ui->pushButton->setText(_l("block-title", this->user->Username));
        this->qUser->Result->SetError(HUGGLE_EUNKNOWN, "Unable to block: " + reason);
        this->qUser = nullptr;
        this->ui->pushButton->setEnabled(true);
        this->timer->stop();
        return;
    }
    // let's assume the user was blocked
    this->ui->pushButton->setText(_l("block-done", this->user->Username));
    HUGGLE_DEBUG("block result: " + this->qUser->Result->Data, 2);
    HistoryItem *history = new HistoryItem(this->user->GetSite());
    history->Result = _l("successful");
    history->Type = HistoryBlock;
    history->IsRevertable = false;
    history->Target = this->user->Username;
    this->qUser.Delete();
    this->timer->stop();
    if (this->ui->cbMessageTarget->isChecked())
        this->sendBlockNotice(nullptr);
    this->close;
}

void BlockUserForm::Failed(QString reason)
{
    UiGeneric::pMessageBox(this, "Unable to block user", _l("block-fail", reason),
                         MessageBoxStyleError, true);
    this->timer->stop();
    this->ui->pushButton->setEnabled(true);
    // remove the pointers
    this->qUser.Delete();
}

void BlockUserForm::on_pushButton_clicked()
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
            + QUrl::toPercentEncoding(this->ui->comboBox->currentText()) + "&expiry="
            + QUrl::toPercentEncoding(this->ui->comboBox_2->currentText())
            + nocreate + anononly + noemail + autoblock + allowusertalk + "&token="
            + QUrl::toPercentEncoding(this->user->GetSite()->GetProjectConfig()->Token_Csrf);
    this->qUser->Target = _l("block-progress", this->user->Username);
    this->qUser->UsingPOST = true;
    QueryPool::HugglePool->AppendQuery(this->qUser);
    this->qUser->Process();
    this->timer->start(HUGGLE_TIMER);
}

void BlockUserForm::sendBlockNotice(ApiQuery *dependency)
{
    QString blocknotice;
    if (this->ui->comboBox_2->currentText() != "indefinite")
    {
        blocknotice = this->user->GetSite()->GetProjectConfig()->BlockMessage;
        blocknotice = blocknotice.replace("$1", this->ui->comboBox_2->currentText());
        blocknotice = blocknotice.replace("$2", this->ui->comboBox->currentText());
    }else
    {
        blocknotice = this->user->GetSite()->GetProjectConfig()->BlockMessageIndef;
        blocknotice = blocknotice.replace("$1", this->ui->comboBox->currentText());
    }
    QString blocksum = this->user->GetSite()->GetProjectConfig()->BlockSummary;
    WikiUtil::MessageUser(user, blocknotice, blocksum, blocksum, true, dependency, false, false);
}

void Huggle::BlockUserForm::on_pushButton_3_clicked()
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
    this->timer->start();
}

void BlockUserForm::recheck()
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
        UiGeneric::MessageBox(_l("result"), text, MessageBoxStyleNormal, true);
        this->qUser = nullptr;
        this->timer->stop();
        this->ui->pushButton_3->setEnabled(true);
        this->ui->pushButton->setEnabled(true);
    }
}

void Huggle::BlockUserForm::on_pushButton_4_clicked()
{
    UiGeneric::DisplayContributionBrowser(this->user, this);
}
