//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "exceptionwindow.hpp"
#include "ui_exceptionwindow.h"

using namespace Huggle;

ExceptionWindow::ExceptionWindow(Exception *e) : ui(new Ui::ExceptionWindow)
{
    ui->setupUi(this);
    //ui->textEdit->setText("We are sorry, but huggle just crashed! Please submit the following information together with details of what were you doing to "\
    //                      "http://bugzilla.wikimedia.org/\n\n");
}

ExceptionWindow::~ExceptionWindow()
{
    delete ui;
}

void ExceptionWindow::on_pushButton_clicked()
{

}

void ExceptionWindow::on_pushButton_3_clicked()
{

}
