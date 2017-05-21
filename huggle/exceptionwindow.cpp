//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "core.hpp"
#include "syslog.hpp"
#include "exceptionwindow.hpp"
#include "ui_exceptionwindow.h"

using namespace Huggle;

ExceptionWindow::ExceptionWindow(Exception *e) : ui(new Ui::ExceptionWindow)
{
    this->ui->setupUi(this);
    QString hr;
    if (Huggle::Syslog::HuggleLogs)
        hr = Huggle::Syslog::HuggleLogs->RingLogToText();
    else
        hr = "not available (nullptr)";
    this->ui->textEdit->setText("Unfortunately Huggle has crashed. Please submit the following information "\
                          "together with details of what were you doing to https://phabricator.wikimedia.org/maniphest/task/create/?projects=Huggle"\
                          "\n\n\n\nException details\n============================\nError code: "
                          + QString::number(e->ErrorCode) + "\nReason: "
                          + e->Message + "\n\nSource: " + e->Source + "\nStack trace:\n" + e->StackTrace
                          + "\n\n\n\n============================\nSystem log\n=======\n" + hr);
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

void Huggle::ExceptionWindow::on_pushButton_2_clicked()
{
    Core::HuggleCore->Shutdown();
}
