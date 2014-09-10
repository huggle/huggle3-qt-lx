//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "proxy.hpp"
#include "localization.hpp"
#include <QNetworkProxy>
#include "ui_proxy.h"

using namespace Huggle;

Proxy::Proxy(QWidget *parent) : QDialog(parent), ui(new Ui::Proxy)
{
    this->ui->setupUi(this);
    this->ui->label->setText(_l("login-proxyaddress"));
    this->ui->label_3->setText(_l("login-proxyport"));
    this->ui->comboBox->addItem("None");
    this->ui->comboBox->addItem("Socks 5");
    this->ui->comboBox->setCurrentIndex(0);
}

Proxy::~Proxy()
{
    delete this->ui;
}

void Proxy::on_buttonBox_accepted()
{
    QNetworkProxy proxy;
    switch (this->ui->comboBox->currentIndex())
    {
        case 0:
            QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
            return;
        case 1:
            proxy.setType(QNetworkProxy::Socks5Proxy);
            break;
    }

    proxy.setHostName(this->ui->lineEdit->text());
    proxy.setPort(this->ui->lineEdit_2->text().toUInt());
    proxy.setUser(this->ui->lineEdit_3->text());
    proxy.setPassword(this->ui->lineEdit_4->text());
    QNetworkProxy::setApplicationProxy(proxy);
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
