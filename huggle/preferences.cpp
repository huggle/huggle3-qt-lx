//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "preferences.hpp"
#include <QMessageBox>
#include "core.hpp"
#include "configuration.hpp"
#include "localization.hpp"
#include "syslog.hpp"
#include "ui_preferences.h"

using namespace Huggle;

Preferences::Preferences(QWidget *parent) : QDialog(parent), ui(new Ui::Preferences)
{
    this->ui->setupUi(this);
    // headers
    this->ui->tableWidget->setColumnCount(5);
    this->setWindowTitle(_l("config-title"));
    this->ui->checkBox_12->setText(_l("config-ircmode"));
    QStringList header;
    header << _l("general-name") << _l("author") << _l("description") << _l("status") << _l("version");
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
#ifdef PYTHONENGINE
    c = 0;
    QList<Python::PythonScript*> scripts(Core::HuggleCore->Python->ScriptsList());
    while (c < scripts.count())
    {
        Python::PythonScript *script = scripts.at(c);
        c++;
        this->ui->tableWidget->insertRow(0);
        this->ui->tableWidget->setItem(0, 0, new QTableWidgetItem(script->GetModule()));
        this->ui->tableWidget->setItem(0, 1, new QTableWidgetItem(script->GetAuthor()));
        this->ui->tableWidget->setItem(0, 2, new QTableWidgetItem(script->GetDescription()));
        this->ui->tableWidget->setItem(0, 3, new QTableWidgetItem("Loaded and running"));
        this->ui->tableWidget->setItem(0, 4, new QTableWidgetItem(script->GetVersion()));
    }
#endif
    switch(Configuration::HuggleConfiguration->UserConfig->GoNext)
    {
        case Configuration_OnNext_Stay:
            this->ui->radioButton_5->setChecked(true);
            this->ui->radioButton_4->setChecked(false);
            this->ui->radioButton_3->setChecked(false);
            break;
        case Configuration_OnNext_Revert:
            this->ui->radioButton_5->setChecked(false);
            this->ui->radioButton_4->setChecked(true);
            this->ui->radioButton_3->setChecked(false);
            break;
        case Configuration_OnNext_Next:
            this->ui->radioButton_5->setChecked(false);
            this->ui->radioButton_3->setChecked(true);
            this->ui->radioButton_4->setChecked(false);
            break;
    }
    this->Disable();
    // options
    this->ui->checkBox_26->setChecked(Configuration::HuggleConfiguration->SystemConfig_RequestDelay);
    this->ui->label_2->setEnabled(this->ui->checkBox_26->isChecked());
    this->ui->lineEdit_2->setEnabled(this->ui->checkBox_26->isChecked());
    this->ui->checkBox_15->setChecked(Configuration::HuggleConfiguration->UserConfig->DeleteEditsAfterRevert);
    this->ui->checkBox_5->setChecked(Configuration::HuggleConfiguration->EnforceManualSoftwareRollback);
    this->ui->checkBox_2->setChecked(Configuration::HuggleConfiguration->WarnUserSpaceRoll);
    this->ui->checkBox->setChecked(Configuration::HuggleConfiguration->UserConfig_AutomaticallyResolveConflicts);
    this->ui->checkBox_12->setChecked(Configuration::HuggleConfiguration->UsingIRC);
    this->ui->checkBox_14->setChecked(Configuration::HuggleConfiguration->UserConfig->HistoryLoad);
    this->ui->checkBox_3->setChecked(Configuration::HuggleConfiguration->ProjectConfig->ConfirmOnSelfRevs);
    this->ui->checkBox_4->setChecked(Configuration::HuggleConfiguration->ProjectConfig->ConfirmWL);
    this->ui->checkBox_11->setChecked(Configuration::HuggleConfiguration->ProjectConfig->ConfirmTalk);
    this->ui->checkBox_16->setChecked(Configuration::HuggleConfiguration->UserConfig->EnforceMonthsAsHeaders);
    this->ui->checkBox_19->setChecked(Configuration::HuggleConfiguration->UserConfig->TruncateEdits);
    this->ui->lineEdit_2->setText(QString::number(Configuration::HuggleConfiguration->SystemConfig_DelayVal));
    this->ui->radioButton->setChecked(!Configuration::HuggleConfiguration->RevertOnMultipleEdits);
    this->ui->checkBox_21->setChecked(Configuration::HuggleConfiguration->UserConfig->LastEdit);
    this->ui->checkBox_17->setChecked(Configuration::HuggleConfiguration->UserConfig->SectionKeep);
    this->ui->radioButton_2->setChecked(Configuration::HuggleConfiguration->RevertOnMultipleEdits);
    this->ui->checkBox_20->setEnabled(this->ui->checkBox->isChecked());
    this->ui->radioButton_2->setEnabled(this->ui->checkBox->isChecked());
    this->ui->checkBox_20->setChecked(Configuration::HuggleConfiguration->UserConfig->RevertNewBySame);
    this->ui->radioButton->setEnabled(this->ui->checkBox->isChecked());
    this->ui->lineEdit_3->setText(QString::number(Configuration::HuggleConfiguration->SystemConfig_RevertDelay));
    this->ui->checkBox_24->setChecked(Configuration::HuggleConfiguration->UserConfig->ManualWarning);
    this->ui->checkBox_25->setChecked(Configuration::HuggleConfiguration->UserConfig->CheckTP);
    this->ui->checkBox_27->setChecked(Configuration::HuggleConfiguration->SystemConfig_InstantReverts);
    this->ui->checkBox_22->setChecked(Configuration::HuggleConfiguration->SystemConfig_DynamicColsInList);
    this->ui->checkBox_23->setChecked(Configuration::HuggleConfiguration->UserConfig->DisplayTitle);
    this->ui->checkBox_29->setChecked(Configuration::HuggleConfiguration->SystemConfig_UpdatesEnabled);
    this->on_checkBox_27_clicked();
}

Preferences::~Preferences()
{
    delete this->ui;
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
    this->ui->checkBox_28->setChecked(f->getIgnore_UserSpace());
    this->ui->checkBox_10->setChecked(f->getIgnoreFriends());
    this->ui->checkBox_18->setChecked(f->getIgnoreReverts());
    this->ui->checkBox_6->setChecked(f->getIgnoreSelf());
    this->ui->lineEdit->setText(f->QueueName);
}

void Preferences::Disable()
{
    this->ui->checkBox_6->setEnabled(false);
    this->ui->checkBox_7->setEnabled(false);
    this->ui->checkBox_8->setEnabled(false);
    this->ui->checkBox_9->setEnabled(false);
    this->ui->checkBox_10->setEnabled(false);
    this->ui->pushButton_4->setEnabled(false);
    this->ui->checkBox_18->setEnabled(false);
    this->ui->pushButton_5->setEnabled(false);
    this->ui->checkBox_28->setEnabled(false);
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
    this->ui->checkBox_18->setEnabled(true);
    this->ui->pushButton_5->setEnabled(true);
    this->ui->pushButton_6->setEnabled(true);
    this->ui->checkBox_13->setEnabled(true);
    this->ui->checkBox_28->setEnabled(true);
}

void Preferences::on_pushButton_clicked()
{
    this->hide();
}

void Huggle::Preferences::on_pushButton_2_clicked()
{
    Configuration::HuggleConfiguration->UserConfig_AutomaticallyResolveConflicts = this->ui->checkBox->isChecked();
    Configuration::HuggleConfiguration->WarnUserSpaceRoll = this->ui->checkBox_2->isChecked();
    Configuration::HuggleConfiguration->UsingIRC = this->ui->checkBox_12->isChecked();
    Configuration::HuggleConfiguration->EnforceManualSoftwareRollback = this->ui->checkBox_5->isChecked();
    Configuration::HuggleConfiguration->RevertOnMultipleEdits = this->ui->radioButton_2->isChecked();
    Configuration::HuggleConfiguration->ProjectConfig->ConfirmOnSelfRevs = this->ui->checkBox_3->isChecked();
    Configuration::HuggleConfiguration->ProjectConfig->ConfirmWL = this->ui->checkBox_4->isChecked();
    Configuration::HuggleConfiguration->UserConfig->RevertNewBySame = this->ui->checkBox_20->isChecked();
    Configuration::HuggleConfiguration->UserConfig->HistoryLoad = this->ui->checkBox_14->isChecked();
    Configuration::HuggleConfiguration->UserConfig->EnforceMonthsAsHeaders = this->ui->checkBox_16->isChecked();
    Configuration::HuggleConfiguration->UserConfig->SectionKeep = this->ui->checkBox_17->isChecked();
    Configuration::HuggleConfiguration->ProjectConfig->ConfirmTalk = this->ui->checkBox_11->isChecked();
    Configuration::HuggleConfiguration->UserConfig->LastEdit = this->ui->checkBox_21->isChecked();
    Configuration::HuggleConfiguration->UserConfig->DeleteEditsAfterRevert = this->ui->checkBox_15->isChecked();
    Configuration::HuggleConfiguration->UserConfig->TruncateEdits = this->ui->checkBox_19->isChecked();
    Configuration::HuggleConfiguration->SystemConfig_DynamicColsInList = this->ui->checkBox_22->isChecked();
    Configuration::HuggleConfiguration->UserConfig->DisplayTitle = this->ui->checkBox_23->isChecked();
    Configuration::HuggleConfiguration->UserConfig->ManualWarning = this->ui->checkBox_24->isChecked();
    Configuration::HuggleConfiguration->UserConfig->CheckTP = this->ui->checkBox_25->isChecked();
    Configuration::HuggleConfiguration->SystemConfig_RequestDelay = this->ui->checkBox_26->isChecked();
    Configuration::HuggleConfiguration->SystemConfig_DelayVal = this->ui->lineEdit_2->text().toUInt();
    Configuration::HuggleConfiguration->SystemConfig_RevertDelay = this->ui->lineEdit_3->text().toInt();
    Configuration::HuggleConfiguration->SystemConfig_InstantReverts = this->ui->checkBox_27->isChecked();
    Configuration::HuggleConfiguration->SystemConfig_UpdatesEnabled = this->ui->checkBox_29->isChecked();
    if (this->ui->radioButton_5->isChecked())
    {
        Configuration::HuggleConfiguration->UserConfig->GoNext = Configuration_OnNext_Stay;
    }
    if (this->ui->radioButton_4->isChecked())
    {
        Configuration::HuggleConfiguration->UserConfig->GoNext = Configuration_OnNext_Revert;
    }
    if (this->ui->radioButton_3->isChecked())
    {
        Configuration::HuggleConfiguration->UserConfig->GoNext = Configuration_OnNext_Next;
    }
    Configuration::SaveSystemConfig();
    this->hide();
}

void Huggle::Preferences::on_checkBox_clicked()
{
    this->ui->radioButton_2->setEnabled(this->ui->checkBox->isChecked());
    this->ui->checkBox_20->setEnabled(this->ui->checkBox->isChecked());
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
        mb.setText(_l("config-no-colon"));
        mb.exec();
        return;
    }
    filter->setIgnoreBots(this->ui->checkBox_7->isChecked());
    filter->setIgnoreNP(this->ui->checkBox_8->isChecked());
    filter->setIgnoreWL(this->ui->checkBox_9->isChecked());
    filter->setIgnoreSelf(this->ui->checkBox_6->isChecked());
    filter->setIgnoreReverts(this->ui->checkBox_18->isChecked());
    filter->setIgnoreFriends(this->ui->checkBox_10->isChecked());
    filter->setIgnore_UserSpace(this->ui->checkBox_28->isChecked());
    filter->QueueName = this->ui->lineEdit->text();
    Core::HuggleCore->Main->Queue1->Filters();
    this->Reload();
}

void Huggle::Preferences::on_pushButton_5_clicked()
{
    /// \todo DO SOMETHING WITH ME, FOR FUCK SAKE
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

void Huggle::Preferences::on_checkBox_26_clicked()
{
    this->ui->label_2->setEnabled(this->ui->checkBox_26->isChecked());
    this->ui->lineEdit_2->setEnabled(this->ui->checkBox_26->isChecked());
}

void Huggle::Preferences::on_checkBox_27_clicked()
{
    this->ui->label_3->setEnabled(!this->ui->checkBox_27->isChecked());
    this->ui->lineEdit_3->setEnabled(!this->ui->checkBox_27->isChecked());
}
