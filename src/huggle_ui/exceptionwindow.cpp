//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include <huggle_core/core.hpp>
#include <huggle_core/syslog.hpp>
#include <huggle_core/localization.hpp>
#include <huggle_core/exception.hpp>
#include "ui_exceptionwindow.h"
#include "exceptionwindow.hpp"

using namespace Huggle;

ExceptionWindow::ExceptionWindow(Exception *e) : ui(new Ui::ExceptionWindow)
{
    this->ui->setupUi(this);
    QString hr;
    if (Huggle::Syslog::HuggleLogs)
        hr = Huggle::Syslog::HuggleLogs->RingLogToText();
    else
        hr = "not available (nullptr)";
    this->ui->textEdit->setText(_l("exception-text-1") +
                          "https://phabricator.wikimedia.org/maniphest/task/create/?projects=Huggle\n\n\n\n" +
                          _l("exception-details") + "\n============================\n" +
                          _l("exception-error-code", QString::number(e->ErrorCode)) + "\n" +
                          _l("exception-reason", e->Message) + "\n\n" +
                          _l("exception-source", e->Source) + "\n" + _l("exception-stack-trace") + "\n" + e->StackTrace +
                          "\n\n\n\n============================\n" + _l("exception-system-log") + "\n=======\n" + hr);
}

ExceptionWindow::~ExceptionWindow()
{
    delete this->ui;
}

void ExceptionWindow::on_recoverButton_clicked()
{
    this->close();
}

void ExceptionWindow::on_killButton_clicked()
{
    QApplication::exit(2);
}

void ExceptionWindow::on_shutdownButton_clicked()
{
    Core::HuggleCore->Shutdown();
}
