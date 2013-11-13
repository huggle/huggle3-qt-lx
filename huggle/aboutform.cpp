//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "aboutform.hpp"
#include "ui_aboutform.h"

using namespace Huggle;

AboutForm::AboutForm(QWidget *parent) : QDialog(parent), ui(new Ui::AboutForm)
{
    ui->setupUi(this);
    QString python = " without python support";
    if (Configuration::HuggleConfiguration->PythonEngine)
    {
        python = " with python support";
    }
    QString version = " Compiled using QT " + QString(QT_VERSION_STR) + " Running on QT " + QString(qVersion());
    ui->label_7->setText("Version: " + Configuration::HuggleConfiguration->HuggleVersion + python + version);
}

AboutForm::~AboutForm()
{
    delete ui;
}

void AboutForm::on_pushButton_clicked()
{
    this->close();
}
