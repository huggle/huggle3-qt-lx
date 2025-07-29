//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "proxy.hpp"
#include <huggle_core/configuration.hpp>
#include <huggle_core/localization.hpp>
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
    proxy.setPort(static_cast<quint16>(port));
    proxy.setUser(name);
    proxy.setPassword(pass);
    QNetworkProxy::setApplicationProxy(proxy);
}

Proxy::Proxy(QWidget *parent) : HW("proxy", this, parent), ui(new Ui::Proxy)
{
    this->ui->setupUi(this);
    this->ui->labelHostname->setText(_l("login-proxyaddress"));
    this->ui->labelPort->setText(_l("login-proxyport"));
    this->ui->checkBoxRememberSettings->setText(_l("login-proxy-remember-this"));
    this->ui->comboBoxProxyType->addItem(_l("protect-none"));
    this->ui->comboBoxProxyType->addItem("Socks 5");
    this->ui->comboBoxProxyType->addItem("Http");
    this->ui->comboBoxProxyType->addItem("Http (caching proxy)");
    this->ui->comboBoxProxyType->addItem("Ftp");
    this->ui->labelRestartWarning->setText(_l("proxy-restart"));
    this->ui->comboBoxProxyType->setCurrentIndex(hcfg->SystemConfig_ProxyType);
    // Right now it seems proxy needs restart so this option doesn't really make much sense on any other value than true
    this->ui->checkBoxRememberSettings->setChecked(true);
    if (hcfg->SystemConfig_UseProxy)
    {
        this->ui->lineEditHostname->setText(hcfg->SystemConfig_ProxyHost);
        this->ui->lineEditPort->setText(QString::number(hcfg->SystemConfig_ProxyPort));
        this->ui->lineEditUsername->setText(hcfg->SystemConfig_ProxyUser);
        this->ui->lineEditPassword->setText(hcfg->SystemConfig_ProxyPass);
    }
    this->RestoreWindow();
}

Proxy::~Proxy()
{
    delete this->ui;
}

void Proxy::on_buttonBox_accepted()
{
    SetProxy(this->ui->comboBoxProxyType->currentIndex(), this->ui->lineEditHostname->text(), this->ui->lineEditPort->text().toUInt(),
               this->ui->lineEditUsername->text(), this->ui->lineEditPassword->text());
    if (this->ui->checkBoxRememberSettings->isChecked())
    {
        hcfg->SystemConfig_UseProxy = this->ui->comboBoxProxyType->currentIndex() != 0;
        hcfg->SystemConfig_ProxyHost = this->ui->lineEditHostname->text();
        hcfg->SystemConfig_ProxyType = this->ui->comboBoxProxyType->currentIndex();
        hcfg->SystemConfig_ProxyPort = this->ui->lineEditPort->text().toUInt();
        hcfg->SystemConfig_ProxyUser = this->ui->lineEditUsername->text();
        hcfg->SystemConfig_ProxyPass = this->ui->lineEditPassword->text();
        hcfg->SaveSystemConfig();
    }
}

void Proxy::on_buttonBox_rejected()
{
    this->close();
}

void Huggle::Proxy::on_comboBoxProxyType_currentIndexChanged(int index)
{
    bool visible = index != 0;
    this->ui->labelUsername->setEnabled(visible);
    this->ui->labelHostname->setEnabled(visible);
    this->ui->labelPassword->setEnabled(visible);
    this->ui->labelPort->setEnabled(visible);
    this->ui->lineEditHostname->setEnabled(visible);
    this->ui->lineEditPort->setEnabled(visible);
    this->ui->lineEditUsername->setEnabled(visible);
    this->ui->lineEditPassword->setEnabled(visible);
}
