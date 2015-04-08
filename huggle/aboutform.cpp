//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "aboutform.hpp"
#include <QDesktopServices>
#include <QUrl>
#include "configuration.hpp"
#include "generic.hpp"
#include "mainwindow.hpp"
#include "localization.hpp"
#include "ui_aboutform.h"

using namespace Huggle;

int developer = 0;
int last_dev = -1;

AboutForm::AboutForm(QWidget *parent) : HW("aboutform", this, parent), ui(new Ui::AboutForm)
{
    this->ui->setupUi(this);
    QString python = _l("about-python-without");
    if (Configuration::HuggleConfiguration->PythonEngine)
    {
        python = _l("about-python-with");
    }
    QString version = _l("about-qt", QString(QT_VERSION_STR), QString(qVersion()));
    this->ui->label_7->setText(_l("version") + ": " + Configuration::HuggleConfiguration->HuggleVersion + python + version);
    this->RestoreWindow();
}

AboutForm::~AboutForm()
{
    delete this->ui;
}

void AboutForm::on_pushButton_clicked()
{
    this->close();
}

void AboutForm::on_label_8_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(link);
}

void AboutForm::on_label_5_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(link);
}

void AboutForm::on_label_3_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(link);
}

void Huggle::AboutForm::on_label_4_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(link);
}

void Huggle::AboutForm::on_label_10_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(link);
}

void Huggle::AboutForm::on_label_9_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(link);
}

void Huggle::AboutForm::on_label_11_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(link);
}

void Huggle::AboutForm::on_label_12_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(link);
}

void Huggle::AboutForm::on_label_13_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(link);
}

static void check_dev()
{
    if (developer > 20)
    {
        Generic::MessageBox("Whee", "You entered a developer mode! Addshore stinks!");
        hcfg->Verbosity = 1;
        MainWindow::HuggleMain->EnableDev();
        developer = -200;
    }
}

void Huggle::AboutForm::on_label_3_linkHovered(const QString &link)
{
    // ignore
    Q_UNUSED(link);
    if (last_dev == 2)
        developer++;
    last_dev = 1;
    check_dev();
}

void Huggle::AboutForm::on_label_4_linkHovered(const QString &link)
{
    // ignore
    Q_UNUSED(link);
    if (last_dev == 1)
        developer++;
    last_dev = 2;
    check_dev();
}
