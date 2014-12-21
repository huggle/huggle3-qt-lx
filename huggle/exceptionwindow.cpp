//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "syslog.hpp"
#include "exceptionwindow.hpp"
#include "ui_exceptionwindow.h"

using namespace Huggle;

ExceptionWindow::ExceptionWindow(Exception *e) : ui(new Ui::ExceptionWindow)
{
    this->ui->setupUi(this);
    this->ui->textEdit->setText("Unfortunately Huggle has crashed. Please submit the following information "\
                          "together with details of what were you doing to https://phabricator.wikimedia.org/maniphest/task/create/?projects=Huggle"\
                          "\n\nSystem log\n==================\n" + Huggle::Syslog::HuggleLogs->RingLogToText()
                          + "\n\n\n\nException details\n===========================\nError code: "
                          + QString::number(e->ErrorCode) + "\nReason: "
                          + e->Message + "\nSource:" + e->Source + "\nStack trace:\n" + e->StackTrace);
}

ExceptionWindow::~ExceptionWindow()
{
    delete this->ui;
}

void ExceptionWindow::on_pushButton_clicked()
{
    this->close();
}

void ExceptionWindow::on_pushButton_3_clicked()
{
    QApplication::exit(2);
}
