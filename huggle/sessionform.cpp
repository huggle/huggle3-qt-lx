//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "sessionform.hpp"
#include "configuration.hpp"
#include "core.hpp"
#include "hugglefeed.hpp"
#include "ui_sessionform.h"

using namespace Huggle;

SessionForm::SessionForm(QWidget *parent) : QDialog(parent), ui(new Ui::SessionForm)
{
    this->ui->setupUi(this);
    /// \todo TRANSLATE ME
    this->ui->label_2->setText("You are logged in as " + Configuration::HuggleConfiguration->SystemConfig_Username + "\n" +
                               "SSL: " + Configuration::Bool2String(Configuration::HuggleConfiguration->SystemConfig_UsingSSL) + "\n" +
                               "RC feed: " + Core::HuggleCore->PrimaryFeedProvider->ToString());
    int xx=0;
    while (xx < Configuration::HuggleConfiguration->Rights.count())
    {
        this->ui->listWidget->addItem(Configuration::HuggleConfiguration->Rights.at(xx));
        xx++;
    }
}

SessionForm::~SessionForm()
{
    delete this->ui;
}

void Huggle::SessionForm::on_pushButton_clicked()
{
    this->close();
}
