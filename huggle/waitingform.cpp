//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "waitingform.h"
#include "ui_waitingform.h"

WaitingForm::WaitingForm(QWidget *parent) : QDialog(parent),
    ui(new Ui::WaitingForm)
{
    ui->setupUi(this);
    ui->progressBar->setValue(0);
}

WaitingForm::~WaitingForm()
{
    delete ui;
}

void WaitingForm::Status(int progress, QString text)
{
    ui->label->setText(text);
    ui->progressBar->setValue(progress);
}
