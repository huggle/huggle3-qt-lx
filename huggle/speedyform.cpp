//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "speedyform.h"
#include "ui_speedyform.h"

using namespace Huggle;

SpeedyForm::SpeedyForm(QWidget *parent) : QDialog(parent), ui(new Ui::SpeedyForm)
{
    this->Page = NULL;
    this->User = NULL;
    ui->setupUi(this);
}

SpeedyForm::~SpeedyForm()
{
    delete ui;
}

void SpeedyForm::Init(WikiUser *user, WikiPage *page)
{
    this->User = user;
    this->Page = page;
}
