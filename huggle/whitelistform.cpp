//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "whitelistform.hpp"
#include "configuration.hpp"
#include "ui_whitelistform.h"

using namespace Huggle;

WhitelistForm::WhitelistForm(QWidget *parent) : QDialog(parent), ui(new Ui::WhitelistForm)
{
    this->ui->setupUi(this);
    Configuration::HuggleConfiguration->WhiteList.sort();
    this->Whitelist += Configuration::HuggleConfiguration->WhiteList;
    this->timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(OnTick()));
    this->timer->start(HUGGLE_TIMER);
}

WhitelistForm::~WhitelistForm()
{
    delete this->timer;
    delete this->ui;
}

void WhitelistForm::OnTick()
{
    if (this->Whitelist.count() == 0)
    {
        this->timer->stop();
    }
    int i = 0;
    while (this->Whitelist.count() > 0 && i < 200)
    {
        QString xx = this->Whitelist.at(0);
        xx = xx.replace(QRegExp("[^\\w\\s]"), "");
        this->ui->listWidget->addItem(xx);
        this->Whitelist.removeAt(0);
        i++;
    }
}

void WhitelistForm::on_pushButton_clicked()
{
    this->close();
}
