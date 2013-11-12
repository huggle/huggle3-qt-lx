//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "sessionform.hpp"
#include "ui_sessionform.h"

using namespace Huggle;

SessionForm::SessionForm(QWidget *parent) : QDialog(parent), ui(new Ui::SessionForm)
{
    ui->setupUi(this);
    ui->label_2->setText("You are logged in as " + Configuration::UserName + "\n" +
                         "SSL: " + Configuration::Bool2String(Configuration::UsingSSL));
    int xx=0;
    while (xx < Configuration::Rights.count())
    {
        ui->listWidget->addItem(Configuration::Rights.at(xx));
        xx++;
    }
}

SessionForm::~SessionForm()
{
    delete ui;
}

void Huggle::SessionForm::on_pushButton_clicked()
{
    this->close();
}
