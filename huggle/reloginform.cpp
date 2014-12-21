//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "reloginform.hpp"
#include "apiquery.hpp"
#include "apiqueryresult.hpp"
#include "core.hpp"
#include "configuration.hpp"
#include "generic.hpp"
#include "localization.hpp"
#include "syslog.hpp"
#include "ui_reloginform.h"
#include "wikiutil.hpp"

using namespace Huggle;
ReloginForm::ReloginForm(WikiSite *site, QWidget *parent) : QDialog(parent), ui(new Ui::ReloginForm)
{
    this->ui->setupUi(this);
    this->Site = site;
    this->little_cute_timer = new QTimer();
    connect(this->little_cute_timer, SIGNAL(timeout()), this, SLOT(LittleTick()));
}

ReloginForm::~ReloginForm()
{
    delete this->little_cute_timer;
    delete this->ui;
}

void Huggle::ReloginForm::on_pushButton_clicked()
{
    Core::HuggleCore->Shutdown();
}

void Huggle::ReloginForm::on_pushButton_2_clicked()
{
    this->ui->pushButton_2->setEnabled(false);
    this->ui->lineEdit->setEnabled(false);
    this->qReloginTokenReq = new ApiQuery(ActionLogin, this->Site);
    this->little_cute_timer->start(HUGGLE_TIMER);
    this->qReloginTokenReq->Parameters = "lgname=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->SystemConfig_Username);
    this->qReloginTokenReq->HiddenQuery = true;
    this->qReloginTokenReq->UsingPOST = true;
    this->qReloginTokenReq->Process();
}

void ReloginForm::LittleTick()
{
    if (this->qReloginPw != nullptr)
    {
        if (!this->qReloginPw->IsProcessed())
            return;

        if (this->qReloginPw->IsFailed())
        {
            this->Fail(this->qReloginPw->Result->ErrorMessage);
            return;
        }
        ApiQueryResultNode *login_ = this->qReloginPw->GetApiQueryResult()->GetNode("login");
        if (login_ == nullptr)
        {
            this->Fail("No data returned by login query");
            HUGGLE_DEBUG1(this->qReloginPw->Result->Data);
            return;
        }
        if (!login_->Attributes.contains("result"))
        {
            this->Fail("No result was provided by login query");
            HUGGLE_DEBUG1(this->qReloginPw->Result->Data);
            return;
        }
        QString Result = login_->GetAttribute("result");
        if (Result == "Success")
        {
            // we are logged back in
            Configuration::HuggleConfiguration->ProjectConfig->IsLoggedIn = true;
            Configuration::HuggleConfiguration->ProjectConfig->RequestingLogin = false;
            this->close();
            this->little_cute_timer->stop();
            WikiUtil::RetrieveTokens(this->Site);
            this->qReloginPw.Delete();
            return;
        }
        if (Result == "EmptyPass")
        {
            this->Fail(_l("login-password-empty"));
            return;
        }
        if (Result == "WrongPass")
        {
            this->Fail(_l("login-error-password"));
            return;
        }
        if (Result == "NoName")
        {
            this->Fail(_l("login-fail-wrong-name"));
            return;
        }
        this->Fail(_l("login-api", Result));
        return;
    }
    if (this->qReloginTokenReq != nullptr && this->qReloginTokenReq->IsProcessed())
    {
        if (this->qReloginTokenReq->IsFailed())
        {
            this->Fail(this->qReloginTokenReq->Result->ErrorMessage);
            return;
        }
        ApiQueryResultNode *login_ = this->qReloginTokenReq->GetApiQueryResult()->GetNode("login");
        if (login_ == nullptr)
        {
            this->Fail("No data returned by login query");
            HUGGLE_DEBUG1(this->qReloginTokenReq->Result->Data);
            return;
        }
        if (!login_->Attributes.contains("result"))
        {
            this->Fail("No result was provided by login query");
            HUGGLE_DEBUG1(this->qReloginTokenReq->Result->Data);
            return;
        }
        QString reslt_ = login_->GetAttribute("result");
        if (reslt_ != "NeedToken")
        {
            this->Fail("Result returned " + reslt_ + " NeedToken expected");
            HUGGLE_DEBUG1(this->qReloginTokenReq->Result->Data);
            return;
        }
        if (!login_->Attributes.contains("token"))
        {
            this->Fail(_l("no-token"));
            HUGGLE_DEBUG1(this->qReloginTokenReq->Result->Data);
            return;
        }
        QString t = QUrl::toPercentEncoding(login_->GetAttribute("token"));
        this->qReloginPw = new ApiQuery(ActionLogin);
        this->qReloginPw->HiddenQuery = true;
        this->qReloginPw->Parameters = "lgname=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->SystemConfig_Username)
                + "&lgpassword=" + QUrl::toPercentEncoding(this->ui->lineEdit->text()) + "&lgtoken=" + t;
        this->qReloginPw->UsingPOST = true;
        this->qReloginPw->Process();
        return;
    }
}

void ReloginForm::Fail(QString why)
{
    this->little_cute_timer->stop();
    this->qReloginTokenReq.Delete();
    this->ui->lineEdit->setEnabled(true);
    this->qReloginPw.Delete();
    Generic::MessageBox("Fail", "Unable to login to wiki: " + why,
                        MessageBoxStyleWarning, true);
    this->ui->pushButton_2->setEnabled(true);
}

void ReloginForm::reject()
{
    if (!this->Site->GetProjectConfig()->IsLoggedIn)
        Core::HuggleCore->Shutdown();
    else
        QDialog::reject();
}
