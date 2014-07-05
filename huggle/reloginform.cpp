//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "reloginform.hpp"
#include <QMessageBox>
#include <QtXml>
#include "core.hpp"
#include "configuration.hpp"
#include "syslog.hpp"
#include "localization.hpp"
#include "ui_reloginform.h"

using namespace Huggle;
ReloginForm::ReloginForm(QWidget *parent) : QDialog(parent), ui(new Ui::ReloginForm)
{
    this->ui->setupUi(this);
    this->little_cute_timer = new QTimer();
    connect(this->little_cute_timer, SIGNAL(timeout()), this, SLOT(LittleTick()));
}

ReloginForm::~ReloginForm()
{
    delete this->little_cute_timer;
    delete this->ui;
    GC_DECREF(this->qReloginTokenReq);
    GC_DECREF(this->qReloginPw);
}

void Huggle::ReloginForm::on_pushButton_clicked()
{
    Core::HuggleCore->Shutdown();
}

void Huggle::ReloginForm::on_pushButton_2_clicked()
{
    this->ui->pushButton_2->setEnabled(false);
    this->qReloginTokenReq = new ApiQuery(ActionLogin);
    this->little_cute_timer->start(HUGGLE_TIMER);
    this->qReloginTokenReq->Parameters = "lgname=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->SystemConfig_Username);
    this->qReloginTokenReq->HiddenQuery = true;
    this->qReloginTokenReq->IncRef();
    this->qReloginTokenReq->UsingPOST = true;
    this->qReloginTokenReq->Process();
}

void ReloginForm::LittleTick()
{
    if (this->qReloginPw)
    {
        if (!this->qReloginPw->IsProcessed())
        {
            return;
        }

        if (this->qReloginPw->IsFailed())
        {
            this->Fail(this->qReloginPw->Result->ErrorMessage);
            return;
        }
        QDomDocument result;
        result.setContent(this->qReloginPw->Result->Data);
        QDomNodeList login = result.elementsByTagName("login");
        if (login.count() < 1)
        {
            this->Fail("No data returned by login query");
            HUGGLE_DEBUG1(this->qReloginPw->Result->Data);
            return;
        }
        QDomElement element = login.at(0).toElement();
        if (!element.attributes().contains("result"))
        {
            this->Fail("No result was provided by login query");
            HUGGLE_DEBUG1(this->qReloginPw->Result->Data);
            return;
        }
        QString Result = element.attribute("result");
        if (Result == "Success")
        {
            // we are logged back in
            Configuration::HuggleConfiguration->ProjectConfig->IsLoggedIn = true;
            Configuration::HuggleConfiguration->ProjectConfig->RequestingLogin = false;
            this->close();
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
    if (this->qReloginTokenReq && this->qReloginTokenReq->IsProcessed())
    {
        if (this->qReloginTokenReq->IsFailed())
        {
            this->Fail(this->qReloginTokenReq->Result->ErrorMessage);
            return;
        }
        QDomDocument result;
        result.setContent(this->qReloginTokenReq->Result->Data);
        QDomNodeList login = result.elementsByTagName("login");
        if (login.count() < 1)
        {
            this->Fail("No data returned by login query");
            HUGGLE_DEBUG1(this->qReloginTokenReq->Result->Data);
            return;
        }
        QDomElement element = login.at(0).toElement();
        if (!element.attributes().contains("result"))
        {
            this->Fail("No result was provided by login query");
            HUGGLE_DEBUG1(this->qReloginTokenReq->Result->Data);
            return;
        }
        QString reslt_ = element.attribute("result");
        if (reslt_ != "NeedToken")
        {
            this->Fail("Result returned " + reslt_ + " NeedToken expected");
            HUGGLE_DEBUG1(this->qReloginTokenReq->Result->Data);
            return;
        }
        if (!element.attributes().contains("token"))
        {
            this->Fail("No token");
            HUGGLE_DEBUG1(this->qReloginTokenReq->Result->Data);
            return;
        }
        QString t = QUrl::toPercentEncoding(element.attribute("token"));
        GC_DECREF(this->qReloginTokenReq);
        this->qReloginPw = new ApiQuery(ActionLogin);
        this->qReloginPw->HiddenQuery = true;
        this->qReloginPw->Parameters = "lgname=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->SystemConfig_Username)
                + "&lgpassword=" + QUrl::toPercentEncoding(this->ui->lineEdit->text()) + "&lgtoken=" + t;
        this->qReloginPw->UsingPOST = true;
        this->qReloginPw->IncRef();
        this->qReloginPw->Process();
        return;
    }
}

void ReloginForm::Fail(QString why)
{
    this->little_cute_timer->stop();
    GC_DECREF(this->qReloginTokenReq);
    GC_DECREF(this->qReloginPw);
    QMessageBox mb;
    mb.setWindowTitle("Fail");
    mb.setText("Unable to login to wiki: " + why);
    mb.exec();
    this->ui->pushButton_2->setEnabled(true);
}

void ReloginForm::reject()
{
    Core::HuggleCore->Shutdown();
}
