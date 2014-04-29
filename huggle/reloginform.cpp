//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "reloginform.hpp"
#include "core.hpp"
#include "ui_reloginform.h"

using namespace Huggle;
ReloginForm::ReloginForm(QWidget *parent) : QDialog(parent), ui(new Ui::ReloginForm)
{
    ui->setupUi(this);
}

ReloginForm::~ReloginForm()
{
    delete ui;
}

void Huggle::ReloginForm::on_pushButton_clicked()
{
    Core::HuggleCore->Shutdown();
}

void Huggle::ReloginForm::on_pushButton_2_clicked()
{

}
