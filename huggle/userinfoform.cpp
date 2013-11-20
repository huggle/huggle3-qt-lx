//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "userinfoform.hpp"
#include "ui_userinfoform.h"

using namespace Huggle;

UserinfoForm::UserinfoForm(QWidget *parent) : QDockWidget(parent), ui(new Ui::UserinfoForm)
{
    this->ui->setupUi(this);
    this->ui->pushButton->setEnabled(false);
}

UserinfoForm::~UserinfoForm()
{
    delete this->ui;
}

void UserinfoForm::ChangeUser(WikiUser *user)
{
    if (user == NULL)
    {
        throw new Exception("WikiUser *user can't be NULL in this fc", "void UserinfoForm::ChangeUser(WikiUser *user)");
    }
    this->User = user;
    this->ui->pushButton->setEnabled(true);
    this->ui->pushButton->setText("Retrieve info");
    this->ui->label->setText("Flags: " + user->Flags() + " Score: " + QString::number(user->getBadnessScore()) + " level: " + QString::number(user->WarningLevel));
}
