//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "huggletool.h"
#include "ui_huggletool.h"

HuggleTool::HuggleTool(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::HuggleTool)
{
    ui->setupUi(this);
}

HuggleTool::~HuggleTool()
{
    delete ui;
}

void HuggleTool::SetTitle(QString title)
{
    ui->lineEdit->setText(title);
    ui->comboBox_2->lineEdit()->setText(title);
}

void HuggleTool::SetInfo(QString info)
{
    ui->lineEdit->setText(info);
}

void HuggleTool::SetUser(QString user)
{
    ui->comboBox->lineEdit()->setText(user);
}
