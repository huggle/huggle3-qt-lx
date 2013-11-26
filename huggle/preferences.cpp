//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "preferences.hpp"
#include "ui_preferences.h"

using namespace Huggle;

Preferences::Preferences(QWidget *parent) : QDialog(parent), ui(new Ui::Preferences)
{
    this->ui->setupUi(this);
    // headers
    this->ui->tableWidget->setColumnCount(4);
    this->setWindowTitle(Localizations::HuggleLocalizations->Localize("preferences"));
    this->ui->checkBox_12->setText(Localizations::HuggleLocalizations->Localize("config-ircmode"));
    QStringList header;
    /// \todo LOCALIZE ME
    header << "Name" << "Author" << "Description" << "Status" << "Version";
    this->ui->tableWidget->setHorizontalHeaderLabels(header);
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    this->ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
#if QT_VERSION >= 0x050000
// Qt5 code
    this->ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
// Qt4 code
    this->ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    this->ui->tableWidget->setShowGrid(false);
    int c = 0;
    this->Reload();
    while (c < Huggle::Core::HuggleCore->Extensions.count())
    {
        Huggle::Syslog::HuggleLogs->DebugLog("Loading extension info");
        iExtension *extension = Huggle::Core::HuggleCore->Extensions.at(c);
        this->ui->tableWidget->insertRow(0);
        this->ui->tableWidget->setItem(0, 0, new QTableWidgetItem(extension->GetExtensionName()));
        this->ui->tableWidget->setItem(0, 1, new QTableWidgetItem(extension->GetExtensionAuthor()));
        this->ui->tableWidget->setItem(0, 2, new QTableWidgetItem(extension->GetExtensionDescription()));
        this->ui->tableWidget->setItem(0, 3, new QTableWidgetItem("Loaded and running"));
        this->ui->tableWidget->setItem(0, 4, new QTableWidgetItem(extension->GetExtensionVersion()));
        c++;
    }
    c = 0;
    this->Disable();
    // options
    this->ui->checkBox_5->setChecked(Configuration::HuggleConfiguration->EnforceManualSoftwareRollback);
    this->ui->checkBox_2->setChecked(Configuration::HuggleConfiguration->WarnUserSpaceRoll);
    this->ui->checkBox->setChecked(Configuration::HuggleConfiguration->AutomaticallyResolveConflicts);
    this->ui->checkBox_12->setChecked(Configuration::HuggleConfiguration->UsingIRC);
    this->ui->checkBox_3->setChecked(Configuration::HuggleConfiguration->LocalConfig_ConfirmOnSelfRevs);
    this->ui->checkBox_4->setChecked(Configuration::HuggleConfiguration->LocalConfig_ConfirmWL);
    this->ui->checkBox_11->setChecked(Configuration::HuggleConfiguration->LocalConfig_ConfirmTalk);
    this->ui->radioButton->setChecked(!Configuration::HuggleConfiguration->RevertOnMultipleEdits);
}

void Huggle::Preferences::on_listWidget_itemSelectionChanged()
{
    int id = this->ui->listWidget->currentRow();
    if (id < 0 || id >= HuggleQueueFilter::Filters.count())
    {
        return;
    }
    if (!HuggleQueueFilter::Filters.at(id)->IsChangeable())
    {
        this->Disable();
    } else
    {
        this->EnableQueues();
    }
    HuggleQueueFilter *f = HuggleQueueFilter::Filters.at(ui->listWidget->currentRow());
    this->ui->checkBox_7->setChecked(f->getIgnoreBots());
    this->ui->checkBox_8->setChecked(f->getIgnoreNP());
    this->ui->checkBox_9->setChecked(f->getIgnoreWL());
    this->ui->checkBox_10->setChecked(f->getIgnoreFriends());
    this->ui->checkBox_6->setChecked(f->getIgnoreSelf());
    this->ui->lineEdit->setText(f->QueueName);
}

Preferences::~Preferences()
{
    delete this->ui;
}

void Preferences::Disable()
{
    this->ui->checkBox_6->setEnabled(false);
    this->ui->checkBox_7->setEnabled(false);
    this->ui->checkBox_8->setEnabled(false);
    this->ui->checkBox_9->setEnabled(false);
    this->ui->checkBox_10->setEnabled(false);
    this->ui->pushButton_4->setEnabled(false);
    this->ui->checkBox_12->setEnabled(false);
    this->ui->pushButton_5->setEnabled(false);
    this->ui->pushButton_6->setEnabled(false);
    this->ui->checkBox_13->setEnabled(false);
    this->ui->lineEdit->setEnabled(false);
}

void Preferences::EnableQueues()
{
    this->ui->lineEdit->setEnabled(true);
    this->ui->checkBox_6->setEnabled(true);
    this->ui->checkBox_7->setEnabled(true);
    this->ui->checkBox_8->setEnabled(true);
    this->ui->checkBox_9->setEnabled(true);
    this->ui->checkBox_10->setEnabled(true);
    this->ui->pushButton_4->setEnabled(true);
    this->ui->pushButton_5->setEnabled(true);
    this->ui->pushButton_6->setEnabled(true);
    this->ui->checkBox_12->setEnabled(true);
    this->ui->checkBox_13->setEnabled(true);
}

void Preferences::on_pushButton_clicked()
{
    this->hide();
}

void Huggle::Preferences::on_pushButton_2_clicked()
{
    Configuration::HuggleConfiguration->AutomaticallyResolveConflicts = ui->checkBox->isChecked();
    Configuration::HuggleConfiguration->WarnUserSpaceRoll = ui->checkBox_2->isChecked();
    Configuration::HuggleConfiguration->UsingIRC = ui->checkBox_12->isChecked();
    Configuration::HuggleConfiguration->EnforceManualSoftwareRollback = ui->checkBox_5->isChecked();
    Configuration::HuggleConfiguration->RevertOnMultipleEdits = ui->radioButton_2->isChecked();
    Configuration::HuggleConfiguration->LocalConfig_ConfirmOnSelfRevs = ui->checkBox_3->isChecked();
    Configuration::HuggleConfiguration->LocalConfig_ConfirmWL = ui->checkBox_4->isChecked();
    Configuration::HuggleConfiguration->LocalConfig_ConfirmTalk = ui->checkBox_11->isChecked();
    Configuration::SaveConfig();
    this->hide();
}

void Huggle::Preferences::on_checkBox_clicked()
{
    this->ui->radioButton_2->setEnabled(this->ui->checkBox->isChecked());
    this->ui->radioButton->setEnabled(this->ui->checkBox->isChecked());
}

void Huggle::Preferences::on_pushButton_6_clicked()
{
    int id = this->ui->listWidget->currentRow();
    if (id < 0 || id >= HuggleQueueFilter::Filters.count())
    {
        return;
    }
    HuggleQueueFilter *filter = HuggleQueueFilter::Filters.at(id);
    if (!filter->IsChangeable())
    {
        // don't touch a default filter
        return;
    }
    if (this->ui->lineEdit->text().contains(":"))
    {
        QMessageBox mb;
        /// \todo LOCALIZE ME
        mb.setText("You can't use : in name of queue");
        mb.exec();
        return;
    }
    filter->setIgnoreBots(this->ui->checkBox_7->isChecked());
    filter->setIgnoreNP(this->ui->checkBox_8->isChecked());
    filter->setIgnoreWL(this->ui->checkBox_9->isChecked());
    filter->setIgnoreSelf(this->ui->checkBox_6->isChecked());
    filter->setIgnoreFriends(this->ui->checkBox_10->isChecked());
    filter->QueueName = this->ui->lineEdit->text();
    Core::HuggleCore->Main->Queue1->Filters();
    this->Reload();
}

void Huggle::Preferences::on_pushButton_5_clicked()
{

}

void Huggle::Preferences::on_pushButton_4_clicked()
{
    int id = this->ui->listWidget->currentRow();
    if (id < 0 || id >= HuggleQueueFilter::Filters.count())
    {
        return;
    }
    HuggleQueueFilter *filter = HuggleQueueFilter::Filters.at(id);
    if (!filter->IsChangeable())
    {
        // don't touch a default filter
        return;
    }
    if (Core::HuggleCore->Main->Queue1->CurrentFilter == filter)
    {
        QMessageBox mb;
        mb.setText("You can't delete a filter that is currently being used");
        mb.exec();
        return;
    }
    HuggleQueueFilter::Filters.removeAll(filter);
    delete filter;
    this->Disable();
    Core::HuggleCore->Main->Queue1->Filters();
    this->Reload();
}

void Huggle::Preferences::on_pushButton_3_clicked()
{
    HuggleQueueFilter *filter = new HuggleQueueFilter();
    filter->QueueName = "User defined queue #" + QString::number(HuggleQueueFilter::Filters.count());
    HuggleQueueFilter::Filters.append(filter);
    Core::HuggleCore->Main->Queue1->Filters();
    this->Reload();
}

void Preferences::Reload()
{
    int c = 0;
    this->ui->listWidget->clear();
    while (c < HuggleQueueFilter::Filters.count())
    {
        this->ui->listWidget->addItem(HuggleQueueFilter::Filters.at(c)->QueueName);
        c++;
    }
}
