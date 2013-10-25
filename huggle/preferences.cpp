//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "preferences.h"
#include "ui_preferences.h"

using namespace Huggle;

Preferences::Preferences(QWidget *parent) : QDialog(parent), ui(new Ui::Preferences)
{
    ui->setupUi(this);
    // headers
    ui->tableWidget->setColumnCount(4);
    QStringList header;
    header << "Name" << "Author" << "Description" << "Status" << "Version";
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
#if QT_VERSION >= 0x050000
// Qt5 code
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
// Qt4 code
    ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    ui->tableWidget->setShowGrid(false);
    // options
    ui->checkBox_2->setChecked(Configuration::WarnUserSpaceRoll);
    ui->checkBox->setChecked(Configuration::AutomaticallyResolveConflicts);
    ui->checkBox_12->setChecked(Configuration::UsingIRC);
}

Preferences::~Preferences()
{
    delete ui;
}

void Preferences::on_pushButton_clicked()
{
    Configuration::AutomaticallyResolveConflicts = ui->checkBox->isChecked();
    Configuration::WarnUserSpaceRoll = ui->checkBox_2->isChecked();
    Configuration::UsingIRC = ui->checkBox_12->isChecked();
    Configuration::SaveConfig();
    this->hide();
}

void Huggle::Preferences::on_pushButton_2_clicked()
{
    this->hide();
}
