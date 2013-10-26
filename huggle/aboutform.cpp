//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "aboutform.h"
#include "ui_aboutform.h"

using namespace Huggle;

AboutForm::AboutForm(QWidget *parent) : QDialog(parent), ui(new Ui::AboutForm)
{
    ui->setupUi(this);
    QString python = " without python support";
    if (Configuration::PythonEngine)
    {
        python = " with python support";
    }
    ui->label_7->setText("Version: " + Configuration::HuggleVersion + python);
}

AboutForm::~AboutForm()
{
    delete ui;
}

void AboutForm::on_pushButton_clicked()
{
    this->close();
}
