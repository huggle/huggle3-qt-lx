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
#include <QMenu>
#include "core.hpp"
#include "configuration.hpp"
#include "exception.hpp"
#include "generic.hpp"
#include "huggleoption.hpp"
#include "hugglequeue.hpp"
#include "localization.hpp"
#include "wikisite.hpp"
#include "syslog.hpp"
#include "mainwindow.hpp"
#include "ui_preferences.h"

using namespace Huggle;

static void SetDefaults(QComboBox *item)
{
    item->addItem(_l("config-queue-filter-ignore"));
    item->addItem(_l("config-queue-filter-exclude"));
    item->addItem(_l("config-queue-filter-require"));
    item->setCurrentIndex(0);
}

Preferences::Preferences(QWidget *parent) : QDialog(parent), ui(new Ui::Preferences)
{
    this->ui->setupUi(this);
    this->ui->tableWidget_2->setColumnCount(3);
    QStringList headers;
    SetDefaults(this->ui->cbqBots);
    SetDefaults(this->ui->cbqFrd);
    SetDefaults(this->ui->cbqMinor);
    SetDefaults(this->ui->cbqNew);
    SetDefaults(this->ui->cbqOwn);
    SetDefaults(this->ui->cbqRevert);
    SetDefaults(this->ui->cbqTp);
    SetDefaults(this->ui->cbqUserspace);
    SetDefaults(this->ui->cbqWl);
    headers << _l("config-function") << _l("config-description") << _l("config-shortcut");
    this->ui->tableWidget_2->setHorizontalHeaderLabels(headers);
    this->ui->tableWidget_2->verticalHeader()->setVisible(false);
    this->ui->tableWidget_2->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->Reload2();
    this->ui->tableWidget_2->setShowGrid(false);
    this->ui->tableWidget_2->resizeRowsToContents();
    // headers
    this->ui->tableWidget->setColumnCount(5);
    this->setWindowTitle(_l("config-title"));
    QStringList header;
    header << _l("general-name") << _l("author") << _l("description") << _l("status") << _l("version");
    this->ui->tableWidget->setHorizontalHeaderLabels(header);
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    this->ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->ui->tableWidget_3->setColumnCount(2);
    header.clear();
    header << _l("namespace") << _l("main-user-ignore");
    this->ui->tableWidget_3->setHorizontalHeaderLabels(header);
    this->ui->tableWidget_3->verticalHeader()->setVisible(false);
    this->ui->tableWidget_3->setShowGrid(false);
    this->ui->tableWidget_3->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->ui->tableWidget_3->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(this->ui->tableWidget_2, SIGNAL(cellChanged(int,int)), this, SLOT(RecordKeys(int,int)));
#if QT_VERSION >= 0x050000
// Qt5 code
    this->ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
// Qt4 code
    this->ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    this->ui->tableWidget->setShowGrid(false);
    int c = 0;
    this->Site = hcfg->Projects.at(0);
    foreach (WikiSite *site, hcfg->Projects)
        this->ui->cbSites->addItem(site->Name);
    this->Reload();
    this->ui->cbSites->setCurrentIndex(0);
    while (c < Huggle::Core::HuggleCore->Extensions.count())
    {
        Huggle::Syslog::HuggleLogs->DebugLog("Loading extension info");
        iExtension *extension = Huggle::Core::HuggleCore->Extensions.at(c);
        QString status;
        if (extension->IsWorking())
            status = _l("extension-ok");
        else
            status = _l("extension-kl");
        this->ui->tableWidget->insertRow(0);
        this->ui->tableWidget->setItem(0, 0, new QTableWidgetItem(extension->GetExtensionName()));
        this->ui->tableWidget->setItem(0, 1, new QTableWidgetItem(extension->GetExtensionAuthor()));
        this->ui->tableWidget->setItem(0, 2, new QTableWidgetItem(extension->GetExtensionDescription()));
        this->ui->tableWidget->setItem(0, 3, new QTableWidgetItem(status));
        this->ui->tableWidget->setItem(0, 4, new QTableWidgetItem(extension->GetExtensionVersion()));
        c++;
    }
#ifdef HUGGLE_PYTHON
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
        if (script->IsEnabled())
        {
            this->ui->tableWidget->setItem(0, 3, new QTableWidgetItem(_l("extension-ok")));
        } else
        {
            this->ui->tableWidget->setItem(0, 3, new QTableWidgetItem(_l("extension-kl")));
        }
        this->ui->tableWidget->setItem(0, 4, new QTableWidgetItem(script->GetVersion()));
    }
#endif
    this->Disable();
    this->ui->checkBox->setText(_l("config-conflicts-revert"));
    this->ui->checkBox_2->setText(_l("config-confirm-user"));
    this->ui->checkBox_3->setText(_l("config-confirmselfrevert"));
    this->ui->checkBox_4->setText(_l("config-confirm-wl"));
    this->ui->checkBox_11->setText(_l("config-confirm-talk"));
    this->ui->checkBox_5->setText(_l("config-use-rollback"));
    this->ui->checkBox_30->setText(_l("config-welcome-empty-page"));
    this->ui->checkBox_27->setText(_l("config-instant-reverts"));
    this->ui->label_3->setText(_l("config-revert-wait"));
    this->ui->radioButton_2->setText(_l("config-revert"));
    this->ui->checkBox_20->setText(_l("config-revert-diff"));
    this->ui->pushButton_7->setText(_l("config-change-all"));
    this->ui->checkBox_17->setText(_l("config-merge-messages"));
    this->ui->checkBox_16->setText(_l("config-months-name"));
    this->ui->checkBox_24->setText(_l("config-automatic-warning"));
    this->ui->checkBox_12->setText(_l("config-enable-irc"));
    this->ui->checkBox_15->setText(_l("config-remove-reverted"));
    this->ui->checkBox_19->setText(_l("config-remove-old"));
    this->ui->checkBox_12->setText(_l("config-ircmode"));
    this->ui->checkBox_notifyBeta->setText(_l("config-notify-beta"));
    this->ui->checkBox_notifyUpdate->setText(_l("config-notify-update"));
    this->ui->checkBox_14->setText(_l("config-auto-load-history"));
    this->ui->checkBox_21->setText(_l("config-last-revision"));
    this->ui->checkBox_26->setText(_l("config-require-delay"));
    this->ui->label_2->setText(_l("config-wait-edit"));
    this->ui->radioButton_3->setText(_l("config-display-text"));
    this->ui->radioButton_4->setText(_l("config-retrieve-edit"));
    this->ui->radioButton_5->setText(_l("config-nothing"));
    this->ui->label_4->setText(_l("config-bot-edits"));
    this->ui->label_6->setText(_l("config-own-edits"));
    this->ui->label_5->setText(_l("config-reverts"));
    this->ui->label_8->setText(_l("config-new-page"));
    this->ui->label_7->setText(_l("config-minor"));
    this->ui->label_9->setText(_l("config-whitelisted"));
    this->ui->label_10->setText(_l("config-friend"));
    this->ui->label_11->setText(_l("config-userspace"));
    this->ui->label_12->setText(_l("config-talk"));
    this->ui->checkBox_22->setText(_l("config-columns-dynamic"));
    this->ui->checkBox_25->setText(_l("config-message-notification"));
    this->ui->checkBox_6->setText(_l("config-warning-api"));
    this->ui->checkBox_23->setText(_l("config-title-diff"));
    this->ui->checkBox_notifyUpdate->setText(_l("config-updates"));
    this->ui->checkBox_notifyBeta->setText(_l("config-beta"));
    this->ui->checkBox_31->setText(_l("config-html-messages"));
    this->ui->checkBox_7->setText(_l("config-summary-present"));
    this->ui->pushButton_2->setText(_l("ok"));
    this->ui->pushButton->setText(_l("config-close-without"));

    // options
    this->ResetItems();
    this->on_checkBox_26_clicked();
    this->on_checkBox_27_clicked();
}

Preferences::~Preferences()
{
    foreach (QCheckBox *i, this->NamespaceBoxes.keys())
        delete i;
    delete this->ui;
}

static void SetValue(HuggleQueueFilterMatch matching, QComboBox *item)
{
    switch (matching)
    {
        case HuggleQueueFilterMatchIgnore:
            item->setCurrentIndex(0);
            return;
        case HuggleQueueFilterMatchRequire:
            item->setCurrentIndex(2);
            return;
        case HuggleQueueFilterMatchExclude:
            item->setCurrentIndex(1);
            return;
    }
}

void Huggle::Preferences::on_listWidget_itemSelectionChanged()
{
    if (!HuggleQueueFilter::Filters.contains(this->Site))
        throw new Huggle::Exception("There is no such a wiki site", BOOST_CURRENT_FUNCTION);
    int id = this->ui->listWidget->currentRow();
    if (id < 0 || id >= HuggleQueueFilter::Filters[this->Site]->count())
    {
        return;
    }
    if (!HuggleQueueFilter::Filters[this->Site]->at(id)->IsChangeable())
    {
        this->Disable();
    } else
    {
        this->EnableQueues();
    }
    HuggleQueueFilter *f = HuggleQueueFilter::Filters[this->Site]->at(ui->listWidget->currentRow());
    SetValue(f->getIgnoreBots(), this->ui->cbqBots);
    SetValue(f->getIgnoreNP(), this->ui->cbqNew);
    SetValue(f->getIgnoreWL(), this->ui->cbqWl);
    SetValue(f->getIgnore_UserSpace(), this->ui->cbqUserspace);
    SetValue(f->getIgnoreTalk(), this->ui->cbqTp);
    SetValue(f->getIgnoreFriends(), this->ui->cbqFrd);
    SetValue(f->getIgnoreReverts(), this->ui->cbqRevert);
    SetValue(f->getIgnoreSelf(), this->ui->cbqOwn);
    foreach (QCheckBox *cb, this->NamespaceBoxes.keys())
        cb->setChecked(f->IgnoresNS(this->NamespaceBoxes[cb]));
    this->ui->lineEdit->setText(f->QueueName);
}

void Preferences::Disable()
{
    this->ui->cbqBots->setEnabled(false);
    this->ui->cbqFrd->setEnabled(false);
    this->ui->cbqMinor->setEnabled(false);
    this->ui->cbqNew->setEnabled(false);
    this->ui->cbqOwn->setEnabled(false);
    this->ui->pushButton_4->setEnabled(false);
    this->ui->cbqRevert->setEnabled(false);
    this->ui->pushButton_5->setEnabled(false);
    this->ui->cbqTp->setEnabled(false);
    this->ui->pushButton_6->setEnabled(false);
    this->ui->tableWidget_3->setEnabled(false);
    this->ui->cbqUserspace->setEnabled(false);
    this->ui->lineEdit->setEnabled(false);
    this->ui->cbqWl->setEnabled(false);
}

void Preferences::EnableQueues()
{
    this->ui->lineEdit->setEnabled(true);
    this->ui->cbqBots->setEnabled(true);
    this->ui->cbqFrd->setEnabled(true);
    this->ui->cbqMinor->setEnabled(true);
    this->ui->cbqNew->setEnabled(true);
    this->ui->cbqOwn->setEnabled(true);
    this->ui->cbqRevert->setEnabled(true);
    this->ui->pushButton_4->setEnabled(true);
    this->ui->cbqTp->setEnabled(true);
    this->ui->tableWidget_3->setEnabled(true);
    this->ui->pushButton_5->setEnabled(true);
    this->ui->pushButton_6->setEnabled(true);
    this->ui->cbqUserspace->setEnabled(true);
    this->ui->cbqWl->setEnabled(true);
}

void Preferences::on_pushButton_clicked()
{
    this->ResetItems();
    this->hide();
}

void Huggle::Preferences::on_pushButton_2_clicked()
{
    hcfg->UserConfig->AutomaticallyResolveConflicts = this->ui->checkBox->isChecked();
    hcfg->WarnUserSpaceRoll = this->ui->checkBox_2->isChecked();
    hcfg->SystemConfig_SuppressWarnings = !this->ui->checkBox_6->isChecked();
    hcfg->UsingIRC = this->ui->checkBox_12->isChecked();
    hcfg->UserConfig->EnforceManualSoftwareRollback = this->ui->checkBox_5->isChecked();
    hcfg->UserConfig->EnforceManualSRT = this->ui->checkBox_5->isChecked();
    hcfg->UserConfig->RevertOnMultipleEdits = this->ui->radioButton_2->isChecked();
    hcfg->ProjectConfig->ConfirmOnSelfRevs = this->ui->checkBox_3->isChecked();
    hcfg->ProjectConfig->ConfirmWL = this->ui->checkBox_4->isChecked();
    hcfg->UserConfig->RevertNewBySame = this->ui->checkBox_20->isChecked();
    hcfg->UserConfig->HistoryLoad = this->ui->checkBox_14->isChecked();
    hcfg->UserConfig->EnforceMonthsAsHeaders = this->ui->checkBox_16->isChecked();
    hcfg->UserConfig->SectionKeep = this->ui->checkBox_17->isChecked();
    hcfg->ProjectConfig->ConfirmTalk = this->ui->checkBox_11->isChecked();
    hcfg->UserConfig->LastEdit = this->ui->checkBox_21->isChecked();
    hcfg->UserConfig->DeleteEditsAfterRevert = this->ui->checkBox_15->isChecked();
    hcfg->UserConfig->TruncateEdits = this->ui->checkBox_19->isChecked();
    hcfg->SystemConfig_DynamicColsInList = this->ui->checkBox_22->isChecked();
    hcfg->UserConfig->DisplayTitle = this->ui->checkBox_23->isChecked();
    hcfg->UserConfig->ManualWarning = this->ui->checkBox_24->isChecked();
    hcfg->UserConfig->CheckTP = this->ui->checkBox_25->isChecked();
    hcfg->SystemConfig_RequestDelay = this->ui->checkBox_26->isChecked();
    hcfg->SystemConfig_DelayVal = this->ui->lineEdit_2->text().toUInt();
    hcfg->SystemConfig_RevertDelay = this->ui->lineEdit_3->text().toInt();
    hcfg->SystemConfig_InstantReverts = this->ui->checkBox_27->isChecked();
    hcfg->SystemConfig_UpdatesEnabled = this->ui->checkBox_notifyUpdate->isChecked();
    hcfg->SystemConfig_NotifyBeta = this->ui->checkBox_notifyBeta->isChecked();
    hcfg->UserConfig->HtmlAllowedInIrc = this->ui->checkBox_31->isChecked();
    if (this->ui->checkBox_7->isChecked())
        hcfg->UserConfig->SummaryMode = 1;
    else
        hcfg->UserConfig->SummaryMode = 0;
    hcfg->UserConfig->FontSize = this->ui->lineEdit_4->text().toInt();

    if (hcfg->UserConfig->FontSize < 1)
        hcfg->UserConfig->FontSize = 10;

    hcfg->UserConfig->Font = this->ui->lineEdit_5->text();

    if (hcfg->UserConfig->WelcomeGood != this->ui->checkBox_30->isChecked())
    {
        hcfg->UserConfig->WelcomeGood = this->ui->checkBox_30->isChecked();
        // now we need to update the option as well just to ensure that user config will be updated as well
        // this option needs to be written only if it was explicitly changed by user to a value that
        // is different from a project config file
        HuggleOption *o_ = hcfg->UserConfig->GetOption("welcome-good");
        if (o_)
            o_->SetVariant(hcfg->UserConfig->WelcomeGood);
    }
    if (this->ui->radioButton_5->isChecked())
    {
        hcfg->UserConfig->GoNext = Configuration_OnNext_Stay;
    }
    if (this->ui->radioButton_4->isChecked())
    {
        hcfg->UserConfig->GoNext = Configuration_OnNext_Revert;
    }
    if (this->ui->radioButton_3->isChecked())
    {
        hcfg->UserConfig->GoNext = Configuration_OnNext_Next;
    }
    if (this->ModifiedForm)
    {
        // we need to reload the shortcuts in main form
        hcfg->ReloadOfMainformNeeded = true;
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

static HuggleQueueFilterMatch Match(QComboBox *item)
{
    switch (item->currentIndex())
    {
        case 1:
            return HuggleQueueFilterMatchExclude;
        case 2:
            return HuggleQueueFilterMatchRequire;
    }
    return HuggleQueueFilterMatchIgnore;
}

void Huggle::Preferences::on_pushButton_6_clicked()
{
    if (!HuggleQueueFilter::Filters.contains(this->Site))
        throw new Huggle::Exception("There is no such a wiki site", BOOST_CURRENT_FUNCTION);
    int id = this->ui->listWidget->currentRow();
    if (id < 0 || id >= HuggleQueueFilter::Filters[this->Site]->count())
    {
        return;
    }
    HuggleQueueFilter *filter = HuggleQueueFilter::Filters[this->Site]->at(id);
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
    if (this->ui->tableWidget_3->rowCount() != this->Site->NamespaceList.count())
        throw new Huggle::Exception("Number of ns in config file differs", BOOST_CURRENT_FUNCTION);
    filter->setIgnoreBots(Match(this->ui->cbqBots));
    filter->setIgnoreNP(Match(this->ui->cbqNew));
    filter->setIgnoreWL(Match(this->ui->cbqWl));
    filter->setIgnoreSelf(Match(this->ui->cbqOwn));
    filter->setIgnoreReverts(Match(this->ui->cbqRevert));
    filter->setIgnoreTalk(Match(this->ui->cbqTp));
    filter->setIgnoreFriends(Match(this->ui->cbqFrd));
    filter->setIgnore_UserSpace(Match(this->ui->cbqUserspace));
    int ns = 0;
    while (ns < this->ui->tableWidget_3->rowCount())
    {
        QCheckBox *selected_box = (QCheckBox*)this->ui->tableWidget_3->cellWidget(ns, 1);
        if (!this->NamespaceBoxes.contains(selected_box))
            throw new Huggle::Exception("There is no such a box in the ram", BOOST_CURRENT_FUNCTION);
        if (!this->Site->NamespaceList.contains(this->NamespaceBoxes[selected_box]))
            throw new Huggle::Exception("There is no such space in site", BOOST_CURRENT_FUNCTION);
        int nsid = this->NamespaceBoxes[selected_box];
        if (!filter->Namespaces.contains(nsid))
        {
            filter->Namespaces.insert(nsid, selected_box->isChecked());
        } else
        {
            filter->Namespaces[nsid] = selected_box->isChecked();
        }
        ns++;
    }
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
    if (!HuggleQueueFilter::Filters.contains(this->Site))
        throw new Huggle::Exception("There is no such a wiki site", BOOST_CURRENT_FUNCTION);
    int id = this->ui->listWidget->currentRow();
    if (id < 0 || id >= HuggleQueueFilter::Filters[this->Site]->count())
    {
        return;
    }
    HuggleQueueFilter *filter = HuggleQueueFilter::Filters[this->Site]->at(id);
    if (!filter->IsChangeable())
    {
        // don't touch a default filter
        return;
    }
    if (Core::HuggleCore->Main->Queue1->CurrentFilter == filter)
    {
        QMessageBox mb;
        mb.setText(_l("preferences-delete-using-filter"));
        mb.exec();
        return;
    }
    HuggleQueueFilter::Filters[this->Site]->removeAll(filter);
    delete filter;
    this->Disable();
    Core::HuggleCore->Main->Queue1->Filters();
    this->Reload();
}

void Huggle::Preferences::on_pushButton_3_clicked()
{
    if (!HuggleQueueFilter::Filters.contains(this->Site))
        throw new Huggle::Exception("There is no such a wiki site", BOOST_CURRENT_FUNCTION);
    HuggleQueueFilter *filter = new HuggleQueueFilter();
    filter->QueueName = "User defined queue #" + QString::number(HuggleQueueFilter::Filters[this->Site]->count());
    HuggleQueueFilter::Filters[this->Site]->append(filter);
    Core::HuggleCore->Main->Queue1->Filters();
    this->Reload();
}

void Preferences::Reload()
{
    if (!HuggleQueueFilter::Filters.contains(this->Site))
        throw new Huggle::Exception("There is no such a wiki site", BOOST_CURRENT_FUNCTION);
    while (this->ui->tableWidget_3->rowCount() > 0)
        this->ui->tableWidget_3->removeRow(0);
    foreach (QCheckBox *qb, this->NamespaceBoxes.keys())
        delete qb;
    this->NamespaceBoxes.clear();
    int rw = 0;
    foreach (WikiPageNS *ms, this->Site->NamespaceList.values())
    {
        this->ui->tableWidget_3->insertRow(rw);
        this->ui->tableWidget_3->setItem(rw, 0, new QTableWidgetItem(ms->GetName()));
        QCheckBox *Item = new QCheckBox();
        this->NamespaceBoxes.insert(Item, ms->GetID());
        this->ui->tableWidget_3->setCellWidget(rw, 1, Item);
        rw++;
    }
    this->ui->tableWidget_3->resizeColumnsToContents();
    this->ui->tableWidget_3->resizeRowsToContents();
    int c = 0;
    int d = 0;
    this->ui->cbDefault->clear();
    this->ui->listWidget->clear();
    while (c < HuggleQueueFilter::Filters[this->Site]->count())
    {
        QString name = HuggleQueueFilter::Filters[this->Site]->at(c)->QueueName;

        if (name == this->Site->UserConfig->QueueID)
            d = c;
        this->ui->listWidget->addItem(name);
        this->ui->cbDefault->addItem(name);
        c++;
    }
    this->ui->cbDefault->setCurrentIndex(d);
}

void Preferences::Reload2()
{
    QStringList list = hcfg->Shortcuts.keys();
    this->ui->tableWidget_2->clearContents();
    list.sort();
    int row = 0;
    foreach (QString key, list)
    {
        this->ui->tableWidget_2->insertRow(row);
        Shortcut shortcut = Shortcut(hcfg->Shortcuts[key]);
        QTableWidgetItem *w = new QTableWidgetItem(shortcut.Name);
        w->setFlags(w->flags() ^Qt::ItemIsEditable);
        this->ui->tableWidget_2->setItem(row, 0, w);
        w = new QTableWidgetItem(_l(hcfg->Shortcuts[key].Description));
        w->setFlags(w->flags() ^Qt::ItemIsEditable);
        this->ui->tableWidget_2->setItem(row, 1, w);
        this->ui->tableWidget_2->setItem(row, 2, new QTableWidgetItem(hcfg->Shortcuts[key].QAccel));
        row++;
    }
    this->ui->tableWidget_2->resizeColumnsToContents();
    this->ui->tableWidget_2->resizeRowsToContents();
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

void Preferences::RecordKeys(int row, int column)
{
    if (this->RewritingForm)
        return;
    if (column != 2)
    {
        Exception::ThrowSoftException("Invalid column", BOOST_CURRENT_FUNCTION);
        return;
    }

    // let's get the shortcut id
    QString id = this->ui->tableWidget_2->item(row, 0)->text();
    QString key = "";

    if (!this->ui->tableWidget_2->item(row, column)->text().isEmpty())
    {
        key = QKeySequence(this->ui->tableWidget_2->item(row, column)->text()).toString();
        if (key.isEmpty())
        {
            // let's revert this
            Syslog::HuggleLogs->ErrorLog("Invalid shortcut: " + this->ui->tableWidget_2->item(row, column)->text());
            goto revert;
        }
        if (!this->IgnoreConflicts)
        {
            // check if there isn't another shortcut which uses this
            QStringList keys = hcfg->Shortcuts.keys();
            foreach (QString s, keys)
            {
                if (hcfg->Shortcuts[s].QAccel == key && s != id)
                {
                    QMessageBox m;
                    m.setWindowTitle(_l("fail"));
                    m.setText(_l("config-already-in-use", hcfg->Shortcuts[s].Name));
                    m.exec();
                    goto revert;
                }
            }
        }
    }

    this->ModifiedForm = true;
    this->RewritingForm = true;
    this->IgnoreConflicts = false;
    hcfg->Shortcuts[id].Modified = true;
    hcfg->Shortcuts[id].QAccel = key;
    this->ui->tableWidget_2->setItem(row, column, new QTableWidgetItem(key));
    this->RewritingForm = false;
    return;

    revert:
        this->IgnoreConflicts = true;
        this->ui->tableWidget_2->setItem(row, column, new QTableWidgetItem(hcfg->Shortcuts[id].QAccel));
        return;
}

void Huggle::Preferences::on_pushButton_7_clicked()
{
    this->ui->checkBox_30->setChecked(false);
    this->ui->checkBox_15->setChecked(true);
    this->ui->checkBox_16->setChecked(true);
    this->ui->checkBox_17->setChecked(true);
    this->ui->checkBox_21->setChecked(true);
    this->ui->checkBox_19->setChecked(true);
    this->ui->checkBox_23->setChecked(true);
    this->ui->radioButton_4->setChecked(true);
    this->ui->checkBox_14->setChecked(true);
}

void Huggle::Preferences::on_cbSites_currentIndexChanged(int index)
{
    this->Site = hcfg->Projects.at(index);
    this->Reload();
}

void Huggle::Preferences::on_cbDefault_currentIndexChanged(int index)
{
    this->Site->UserConfig->QueueID = this->ui->cbDefault->itemText(index);
    Core::HuggleCore->Main->Queue1->Filters();
}

void Huggle::Preferences::on_tableWidget_customContextMenuRequested(const QPoint &pos)
{
    QPoint g_ = this->ui->tableWidget->mapToGlobal(pos);
    QMenu menu;
    QAction *disable = new QAction(_l("disable"), &menu);
    QAction *enable = new QAction(_l("enable"), &menu);
    menu.addAction(disable);
    menu.addAction(enable);
    QAction *selection = menu.exec(g_);
    if (selection == disable)
    {
        foreach (QTableWidgetItem *extension, this->ui->tableWidget->selectedItems())
        {
            if (extension->row() >= Core::HuggleCore->Extensions.count())
                throw new Huggle::Exception("ERROR: Invalid exception id", BOOST_CURRENT_FUNCTION);

            iExtension *ex = Core::HuggleCore->Extensions.at(extension->row());
            if (hcfg->IgnoredExtensions.contains(ex->GetExtensionFullPath()))
            {
                Generic::MessageBox("Error", "This extension is already scheduled for disabling next startup of huggle");
            }
            else
            {
                hcfg->IgnoredExtensions.append(ex->GetExtensionFullPath());
                Generic::MessageBox("Done", "Extension was disabled, however you need to restart huggle for it to happen");
            }
        }
    }
    if (selection == enable)
    {
        foreach (QTableWidgetItem *extension, this->ui->tableWidget->selectedItems())
        {
            if (extension->row() >= Core::HuggleCore->Extensions.count())
                throw new Huggle::Exception("ERROR: Invalid exception id", BOOST_CURRENT_FUNCTION);

            iExtension *ex = Core::HuggleCore->Extensions.at(extension->row());
            if (!hcfg->IgnoredExtensions.contains(ex->GetExtensionFullPath()))
            {
                Generic::MessageBox("Error", "This extension is not scheduled for disabling next startup of huggle");
            }
            else
            {
                hcfg->IgnoredExtensions.removeAll(ex->GetExtensionFullPath());
                Generic::MessageBox("Done", "Extension was enabled, however you need to restart huggle for it to happen");
            }
        }
    }
}

void Preferences::ResetItems()
{
    switch(hcfg->UserConfig->GoNext)
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
    this->ui->checkBox_26->setChecked(hcfg->SystemConfig_RequestDelay);
    this->ui->checkBox_15->setChecked(hcfg->UserConfig->DeleteEditsAfterRevert);
    this->ui->checkBox_5->setChecked(hcfg->UserConfig->EnforceSoftwareRollback());
    this->ui->checkBox_6->setChecked(!hcfg->SystemConfig_SuppressWarnings);
    this->ui->checkBox_2->setChecked(hcfg->WarnUserSpaceRoll);
    this->ui->checkBox->setChecked(hcfg->UserConfig->AutomaticallyResolveConflicts);
    this->ui->checkBox_12->setChecked(hcfg->UsingIRC);
    this->ui->checkBox_14->setChecked(hcfg->UserConfig->HistoryLoad);
    this->ui->checkBox_3->setChecked(hcfg->ProjectConfig->ConfirmOnSelfRevs);
    this->ui->checkBox_4->setChecked(hcfg->ProjectConfig->ConfirmWL);
    this->ui->checkBox_11->setChecked(hcfg->ProjectConfig->ConfirmTalk);
    this->ui->checkBox_16->setChecked(hcfg->UserConfig->EnforceMonthsAsHeaders);
    this->ui->checkBox_19->setChecked(hcfg->UserConfig->TruncateEdits);
    this->ui->lineEdit_2->setText(QString::number(hcfg->SystemConfig_DelayVal));
    this->ui->radioButton->setChecked(!hcfg->UserConfig->RevertOnMultipleEdits);
    this->ui->checkBox_21->setChecked(hcfg->UserConfig->LastEdit);
    this->ui->checkBox_17->setChecked(hcfg->UserConfig->SectionKeep);
    this->ui->radioButton_2->setChecked(hcfg->UserConfig->RevertOnMultipleEdits);
    this->ui->checkBox_20->setEnabled(this->ui->checkBox->isChecked());
    this->ui->radioButton_2->setEnabled(this->ui->checkBox->isChecked());
    this->ui->checkBox_20->setChecked(hcfg->UserConfig->RevertNewBySame);
    this->ui->radioButton->setEnabled(this->ui->checkBox->isChecked());
    this->ui->lineEdit_3->setText(QString::number(hcfg->SystemConfig_RevertDelay));
    this->ui->checkBox_24->setChecked(hcfg->UserConfig->ManualWarning);
    this->ui->checkBox_25->setChecked(hcfg->UserConfig->CheckTP);
    this->ui->checkBox_27->setChecked(hcfg->SystemConfig_InstantReverts);
    this->ui->checkBox_22->setChecked(hcfg->SystemConfig_DynamicColsInList);
    this->ui->checkBox_23->setChecked(hcfg->UserConfig->DisplayTitle);
    this->ui->checkBox_30->setChecked(hcfg->UserConfig->WelcomeGood);
    this->ui->checkBox_31->setChecked(hcfg->UserConfig->HtmlAllowedInIrc);
    this->ui->lineEdit_5->setText(hcfg->UserConfig->Font);
    this->ui->lineEdit_4->setText(QString::number(hcfg->UserConfig->FontSize));
    this->ui->checkBox_notifyUpdate->setChecked(hcfg->SystemConfig_UpdatesEnabled);
    this->ui->checkBox_notifyBeta->setChecked(hcfg->SystemConfig_NotifyBeta);
}
