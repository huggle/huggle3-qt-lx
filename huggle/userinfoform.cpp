//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "userinfoform.h"
#include "ui_userinfoform.h"

using namespace Huggle;

UserinfoForm::UserinfoForm(QWidget *parent) : QDockWidget(parent), ui(new Ui::UserinfoForm)
{
    ui->setupUi(this);
    ui->pushButton->setEnabled(false);
}

UserinfoForm::~UserinfoForm()
{
    delete ui;
}

void UserinfoForm::ChangeUser(WikiUser *user)
{
    this->User = user;
    ui->pushButton->setEnabled(true);
    ui->pushButton->setText("Retrieve info");
}
