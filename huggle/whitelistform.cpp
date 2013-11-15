//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "whitelistform.hpp"
#include "ui_whitelistform.h"

using namespace Huggle;

WhitelistForm::WhitelistForm(QWidget *parent) : QDialog(parent), ui(new Ui::WhitelistForm)
{
    ui->setupUi(this);
    int i = 0;
    Configuration::HuggleConfiguration->WhiteList.sort();
    while (i < Configuration::HuggleConfiguration->WhiteList.count())
    {
        ui->listWidget->addItem(Configuration::HuggleConfiguration->WhiteList.at(i));
        i++;
    }
}

WhitelistForm::~WhitelistForm()
{
    delete ui;
}
