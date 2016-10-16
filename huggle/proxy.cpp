//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "proxy.hpp"
#include "configuration.hpp"
#include "localization.hpp"
#include <QNetworkProxy>
#include "ui_proxy.h"

using namespace Huggle;

void Proxy::SetProxy(int type, QString host, unsigned int port, QString name, QString pass)
{
    QNetworkProxy proxy;
    switch (type)
    {
        case 0:
            QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
            return;
        case 1:
            proxy.setType(QNetworkProxy::Socks5Proxy);
            break;
        case 2:
            proxy.setType(QNetworkProxy::HttpProxy);
            break;
        case 3:
            proxy.setType(QNetworkProxy::HttpCachingProxy);
            break;
        case 4:
            proxy.setType(QNetworkProxy::FtpCachingProxy);
            break;
        default:
            return;
    }

    proxy.setHostName(host);
    proxy.setPort(port);
    proxy.setUser(name);
    proxy.setPassword(pass);
    QNetworkProxy::setApplicationProxy(proxy);
}

Proxy::Proxy(QWidget *parent) : HW("proxy", this, parent), ui(new Ui::Proxy)
{
    this->ui->setupUi(this);
    this->ui->label->setText(_l("login-proxyaddress"));
    this->ui->label_3->setText(_l("login-proxyport"));
    this->ui->checkBox->setText(_l("login-proxy-remember-this"));
    this->ui->comboBox->addItem(_l("protect-none"));
    this->ui->comboBox->addItem("Socks 5");
    this->ui->comboBox->addItem("Http");
    this->ui->comboBox->addItem("Http (caching proxy)");
    this->ui->comboBox->addItem("Ftp");
    this->ui->comboBox->setCurrentIndex(hcfg->SystemConfig_ProxyType);
    if (hcfg->SystemConfig_UseProxy)
    {
        this->ui->checkBox->setChecked(true);
        this->ui->lineEdit->setText(hcfg->SystemConfig_ProxyHost);
        this->ui->lineEdit_2->setText(QString::number(hcfg->SystemConfig_ProxyPort));
        this->ui->lineEdit_3->setText(hcfg->SystemConfig_ProxyUser);
        this->ui->lineEdit_4->setText(hcfg->SystemConfig_ProxyPass);
    }
    this->RestoreWindow();
}

Proxy::~Proxy()
{
    delete this->ui;
}

void Proxy::on_buttonBox_accepted()
{
    SetProxy(this->ui->comboBox->currentIndex(), this->ui->lineEdit->text(), this->ui->lineEdit_2->text().toUInt(),
               this->ui->lineEdit_3->text(), this->ui->lineEdit_4->text());
    if (this->ui->checkBox->isChecked())
    {
        hcfg->SystemConfig_UseProxy = this->ui->comboBox->currentIndex() != 0;
        hcfg->SystemConfig_ProxyHost = this->ui->lineEdit->text();
        hcfg->SystemConfig_ProxyType = this->ui->comboBox->currentIndex();
        hcfg->SystemConfig_ProxyPort = this->ui->lineEdit_2->text().toUInt();
        hcfg->SystemConfig_ProxyUser = this->ui->lineEdit_3->text();
        hcfg->SystemConfig_ProxyPass = this->ui->lineEdit_4->text();
        hcfg->SaveSystemConfig();
    }
}

void Proxy::on_buttonBox_rejected()
{
    this->close();
}

void Huggle::Proxy::on_comboBox_currentIndexChanged(int index)
{
    bool visible = index != 0;
    this->ui->label_5->setEnabled(visible);
    this->ui->label->setEnabled(visible);
    this->ui->label_4->setEnabled(visible);
    this->ui->label_3->setEnabled(visible);
    this->ui->lineEdit->setEnabled(visible);
    this->ui->lineEdit_2->setEnabled(visible);
    this->ui->lineEdit_3->setEnabled(visible);
    this->ui->lineEdit_4->setEnabled(visible);
}
