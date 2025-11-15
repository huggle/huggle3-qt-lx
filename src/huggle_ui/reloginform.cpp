//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "reloginform.hpp"
#include "mainwindow.hpp"
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <huggle_core/apiquery.hpp>
#include <huggle_core/apiqueryresult.hpp>
#include <huggle_core/core.hpp>
#include <huggle_core/configuration.hpp>
#include <huggle_core/generic.hpp>
#include <huggle_core/localization.hpp>
#include <huggle_core/syslog.hpp>
#include <huggle_core/wikiutil.hpp>
#include <QUrl>
#include "uigeneric.hpp"
#include "ui_reloginform.h"

using namespace Huggle;
ReloginForm::ReloginForm(WikiSite *site, QWidget *parent) : QDialog(parent), ui(new Ui::ReloginForm)
{
    this->ui->setupUi(this);
    this->loginSite = site;
    this->reloginTimer = new QTimer();
    if (hcfg->SystemConfig_StorePassword)
        this->ui->lineEdit->setText(hcfg->SystemConfig_RememberedPassword);
    this->ui->checkBox->setChecked(hcfg->SystemConfig_Autorelog);
    this->ui->checkBox_RemeberPw->setChecked(hcfg->SystemConfig_StorePassword);
    if (!hcfg->SystemConfig_StorePassword)
        this->ui->checkBox->setEnabled(false);
    this->Localize();
    connect(this->reloginTimer, &QTimer::timeout, this, &ReloginForm::ReloginTick);
    if (hcfg->SystemConfig_Autorelog && hcfg->SystemConfig_StorePassword)
        this->ui->pushButton_Relog->click();
}

ReloginForm::~ReloginForm()
{
    delete this->reloginTimer;
    delete this->ui;
}

void Huggle::ReloginForm::on_pushButton_ForceExit_clicked()
{
    MainWindow::HuggleMain->ShutdownForm();
    Core::HuggleCore->Shutdown();
}

void Huggle::ReloginForm::on_pushButton_Relog_clicked()
{
    // Reinitialize network manager in order to rid of all cookies, session data or cached info
    Core::HuggleCore->ResetNetworkManager();

    hcfg->SystemConfig_StorePassword = this->ui->checkBox_RemeberPw->isChecked();
    this->ui->pushButton_Relog->setEnabled(false);
    this->ui->lineEdit->setEnabled(false);
    this->qReloginTokenReq = new ApiQuery(ActionLogin, this->loginSite);
    this->reloginTimer->start(HUGGLE_TIMER);
    QString username = Configuration::GetLoginName();
    HUGGLE_DEBUG1("Trying to login again using username " + username);
    this->qReloginTokenReq->Parameters = "lgname=" + QUrl::toPercentEncoding(username);
    this->qReloginTokenReq->HiddenQuery = true;
    this->qReloginTokenReq->UsingPOST = true;
    this->qReloginTokenReq->Process();
}

void ReloginForm::ReloginTick()
{
    if (this->qReloginPw != nullptr)
    {
        if (!this->qReloginPw->IsProcessed())
            return;

        if (this->qReloginPw->IsFailed())
        {
            this->Fail(this->qReloginPw->GetFailureReason());
            return;
        }
        ApiQueryResultNode *login_result_node = this->qReloginPw->GetApiQueryResult()->GetNode("login");
        if (login_result_node == nullptr)
        {
            this->Fail("No data returned by login query");
            HUGGLE_DEBUG1(this->qReloginPw->Result->Data);
            return;
        }
        if (!login_result_node->Attributes.contains("result"))
        {
            this->Fail("No result was provided by login query");
            HUGGLE_DEBUG1(this->qReloginPw->Result->Data);
            return;
        }
        QString result_code = login_result_node->GetAttribute("result");
        if (result_code == "Success")
        {
            // we are logged back in
            this->loginSite->ProjectConfig->IsLoggedIn = true;
            this->loginSite->ProjectConfig->RequestingLogin = false;
            this->close();
            this->reloginTimer->stop();
            WikiUtil::RetrieveTokens(this->loginSite);
            this->qReloginPw.Delete();
            return;
        }
        if (result_code == "EmptyPass")
        {
            this->Fail(_l("login-password-empty"));
            return;
        }
        if (result_code == "WrongPass")
        {
            this->Fail(_l("login-error-password"));
            return;
        }
        if (result_code == "NoName")
        {
            this->Fail(_l("login-fail-wrong-name"));
            return;
        }
        HUGGLE_DEBUG1(this->qReloginPw->Result->Data);
        this->Fail(_l("login-api", result_code));
    } else if (this->qReloginTokenReq != nullptr && this->qReloginTokenReq->IsProcessed())
    {
        if (this->qReloginTokenReq->IsFailed())
        {
            this->Fail(this->qReloginTokenReq->GetFailureReason());
            return;
        }
        ApiQueryResultNode *query_result = this->qReloginTokenReq->GetApiQueryResult()->GetNode("login");
        if (query_result == nullptr)
        {
            this->Fail("No data returned by login query");
            HUGGLE_DEBUG1(this->qReloginTokenReq->Result->Data);
            return;
        }
        if (!query_result->Attributes.contains("result"))
        {
            this->Fail("No result was provided by login query");
            HUGGLE_DEBUG1(this->qReloginTokenReq->Result->Data);
            return;
        }
        QString result_code = query_result->GetAttribute("result");
        if (result_code != "NeedToken")
        {
            this->Fail("Result returned " + result_code + " NeedToken expected");
            HUGGLE_DEBUG1(this->qReloginTokenReq->Result->Data);
            return;
        }
        if (!query_result->Attributes.contains("token"))
        {
            this->Fail(_l("no-token"));
            HUGGLE_DEBUG1(this->qReloginTokenReq->Result->Data);
            return;
        }
        QString token = query_result->GetAttribute("token");
        this->qReloginPw = new ApiQuery(ActionLogin, this->loginSite);
        this->qReloginPw->HiddenQuery = true;
        this->qReloginPw->Parameters = "lgname=" + QUrl::toPercentEncoding(Configuration::GetLoginName()) + "&lgpassword=" + QUrl::toPercentEncoding(this->ui->lineEdit->text()) + "&lgtoken=" + QUrl::toPercentEncoding(token);
        this->qReloginPw->UsingPOST = true;
        this->qReloginPw->Process();
    }
}

void ReloginForm::Fail(QString why)
{
    this->reloginTimer->stop();
    this->qReloginTokenReq.Delete();
    this->ui->lineEdit->setEnabled(true);
    this->qReloginPw.Delete();
    UiGeneric::MessageBox(_l("fail"), _l("relogin-fail", why), MessageBoxStyleWarning, true);
    this->ui->pushButton_Relog->setEnabled(true);
}

void ReloginForm::Localize()
{
    this->ui->checkBox_RemeberPw->setText(_l("login-remember-password"));
    this->ui->checkBox->setText(_l("autorelog"));
}

void ReloginForm::reject()
{
    if (!Core::IsRunning())
    {
        // At this point touching any configuration or core structures is unsafe as we performed emergency shutdown previous step
        QDialog::reject();
        return;
    }

    // We store it when we destroy the window because sometimes user wants to cancel the login and also prevent it
    // from auto-relogging you (especially if stored pw is wrong). So this way it's possible.
    hcfg->SystemConfig_Autorelog = this->ui->checkBox->isChecked();
    if (!this->loginSite->GetProjectConfig()->IsLoggedIn)
    {
        MainWindow::HuggleMain->ShutdownForm();
        Core::HuggleCore->Shutdown();
    }
    else
    {
        QDialog::reject();
    }
}

void Huggle::ReloginForm::on_checkBox_RemeberPw_toggled(bool checked)
{
    this->ui->checkBox->setEnabled(checked);
}
