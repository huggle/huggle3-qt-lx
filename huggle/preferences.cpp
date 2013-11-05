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
    /// \todo LOCALIZE ME
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
    int c = 0;
    while (c < HuggleQueueFilter::Filters.count())
    {
        ui->listWidget->addItem(HuggleQueueFilter::Filters.at(c)->QueueName);
        c++;
    }
    this->Disable();
    // options
    ui->checkBox_5->setChecked(Configuration::EnforceManualSoftwareRollback);
    ui->checkBox_2->setChecked(Configuration::WarnUserSpaceRoll);
    ui->checkBox->setChecked(Configuration::AutomaticallyResolveConflicts);
    ui->checkBox_12->setChecked(Configuration::UsingIRC);
}

void Huggle::Preferences::on_listWidget_itemSelectionChanged()
{
    if (ui->listWidget->currentRow() == 0)
    {
        this->Disable();
    } else
    {
        this->EnableQueues();
    }
    HuggleQueueFilter *f = HuggleQueueFilter::Filters.at(ui->listWidget->currentRow());
    ui->checkBox_7->setChecked(f->getIgnoreBots());
    ui->lineEdit->setText(f->QueueName);
}

Preferences::~Preferences()
{
    delete ui;
}

void Preferences::Disable()
{
    ui->checkBox_6->setEnabled(false);
    ui->checkBox_7->setEnabled(false);
    ui->checkBox_8->setEnabled(false);
    ui->checkBox_9->setEnabled(false);
    ui->checkBox_10->setEnabled(false);
    ui->pushButton_4->setEnabled(false);
    ui->pushButton_5->setEnabled(false);
    ui->pushButton_6->setEnabled(false);
    ui->lineEdit->setEnabled(false);
}

void Preferences::EnableQueues()
{
    ui->lineEdit->setEnabled(true);
    ui->checkBox_6->setEnabled(true);
    ui->checkBox_7->setEnabled(true);
    ui->checkBox_8->setEnabled(true);
    ui->checkBox_9->setEnabled(true);
    ui->checkBox_10->setEnabled(true);
    ui->pushButton_4->setEnabled(true);
    ui->pushButton_5->setEnabled(true);
    ui->pushButton_6->setEnabled(true);
}

void Preferences::on_pushButton_clicked()
{
    this->hide();
}

void Huggle::Preferences::on_pushButton_2_clicked()
{
    Configuration::AutomaticallyResolveConflicts = ui->checkBox->isChecked();
    Configuration::WarnUserSpaceRoll = ui->checkBox_2->isChecked();
    Configuration::UsingIRC = ui->checkBox_12->isChecked();
    Configuration::EnforceManualSoftwareRollback = ui->checkBox_5->isChecked();
    Configuration::SaveConfig();
    this->hide();
}
