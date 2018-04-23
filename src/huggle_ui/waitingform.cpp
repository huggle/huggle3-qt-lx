//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "waitingform.hpp"
#include "ui_waitingform.h"
#include <huggle_core/localization.hpp>
using namespace Huggle;

WaitingForm::WaitingForm(QWidget *parent) : QDialog(parent), ui(new Ui::WaitingForm)
{
    this->ui->setupUi(this);
    this->ui->progressBar->setValue(0);
    this->setWindowTitle(_l("waiting"));
}

WaitingForm::~WaitingForm()
{
    delete this->ui;
}

void WaitingForm::Status(int progress)
{
    this->ui->progressBar->setValue(progress);
}

void WaitingForm::Status(int progress, QString text)
{
    this->ui->label->setText(text);
    this->ui->progressBar->setValue(progress);
}

void WaitingForm::reject()
{
    // this function replaces the original that hides the window so that
    // it's not possible to reject this window
}
