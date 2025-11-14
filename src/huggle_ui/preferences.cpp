//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "preferences.hpp"
#include "waitingform.hpp"
#include "mainwindow.hpp"
#include "hugglequeue.hpp"
#include "uigeneric.hpp"
#include "ui_preferences.h"
#include <QMessageBox>
#include <QLabel>
#include <QSpinBox>
#include <QFile>
#include <QDir>
#include <QMenu>
#include <QCloseEvent>
#include <huggle_core/core.hpp>
#include <huggle_core/configuration.hpp>
#include <huggle_core/exception.hpp>
#include <huggle_core/generic.hpp>
#include <huggle_core/huggleoption.hpp>
#include <huggle_core/localization.hpp>
#include <huggle_core/wikisite.hpp>
#include <huggle_core/syslog.hpp>
#include <huggle_core/resources.hpp>

using namespace Huggle;

static void SetDefaults(QComboBox *item)
{
    item->addItem(_l("config-queue-filter-ignore"));
    item->addItem(_l("config-queue-filter-exclude"));
    item->addItem(_l("config-queue-filter-require"));
    item->setCurrentIndex(0);
}

Preferences::Preferences(QWidget *parent) : HW("preferences", this, parent), ui(new Ui::Preferences)
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
    SetDefaults(this->ui->cbqWatched);
    SetDefaults(this->ui->cbqIP);
    this->ui->cbProviders->addItem("Wiki");
    this->ui->cbProviders->addItem("IRC");
    this->ui->cbProviders->addItem("XmlRcs");
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
    // Set up context menu for the queue filter list
    this->ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
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
    this->ui->comboBox_WatchlistPreference->addItem(_l("preferences-watchlist-watch"));
    this->ui->comboBox_WatchlistPreference->addItem(_l("preferences-watchlist-unwatch"));
    this->ui->comboBox_WatchlistPreference->addItem(_l("preferences-watchlist-preferences"));
    this->ui->comboBox_WatchlistPreference->addItem(_l("preferences-watchlist-nochange"));
    this->ui->comboBox_WatchlistPreference->setCurrentIndex(static_cast<int>(hcfg->UserConfig->Watchlist));
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
        this->ui->tableWidget->insertRow(c);
        this->ui->tableWidget->setItem(c, 0, new QTableWidgetItem(extension->GetExtensionName()));
        this->ui->tableWidget->setItem(c, 1, new QTableWidgetItem(extension->GetExtensionAuthor()));
        this->ui->tableWidget->setItem(c, 2, new QTableWidgetItem(extension->GetExtensionDescription()));
        this->ui->tableWidget->setItem(c, 3, new QTableWidgetItem(status));
        this->ui->tableWidget->setItem(c, 4, new QTableWidgetItem(extension->GetExtensionVersion()));
        c++;
    }

    this->EnableQueues(false);
    this->ui->checkBox_AutoResolveConflicts->setText(_l("config-conflicts-revert"));
    this->ui->checkBox_ConfirmUserSpaceEditRevert->setText(_l("config-confirm-user"));
    this->ui->checkBox_ConfirmOwnEditRevert->setText(_l("config-confirmselfrevert"));
    this->ui->checkBox_ConfirmWhitelistedRevert->setText(_l("config-confirm-wl"));
    this->ui->checkBox_ConfirmTalkRevert->setText(_l("config-confirm-talk"));
    this->ui->checkBox_UseRollback->setText(_l("config-use-rollback"));
    this->ui->checkBox_WelcomeEmptyPage->setText(_l("config-welcome-empty-page"));
    this->ui->checkBox_InstantReverts->setText(_l("config-instant-reverts"));
    this->ui->groupBox->setTitle(_l("config-reverts-multiple"));
    this->ui->radioButton->setText(_l("config-skip"));
    this->ui->label_RevertWait->setText(_l("config-revert-wait"));
    this->ui->radioButton_Revert->setText(_l("config-revert"));
    this->ui->checkBox_RevertNewerEdits->setText(_l("config-revert-diff"));
    this->ui->pbHuggle2->setText(_l("config-change-all"));
    this->ui->checkBox_MergeMessages->setText(_l("config-merge-messages"));
    this->ui->checkBox_MonthHeaders->setText(_l("config-months-name"));
    this->ui->checkBox_AutoWarning->setText(_l("config-automatic-warning"));
    this->ui->checkBox_EnableIrc->setText(_l("config-enable-irc"));
    this->ui->pushButton_ResetConfig->setText(_l("preferences-factory"));
    this->ui->checkBox_RemoveRevertedEdits->setText(_l("config-remove-reverted"));
    this->ui->checkBox_RemoveOldEdits->setText(_l("config-remove-old"));
    this->ui->checkBox_EnableIrc->setText(_l("config-ircmode"));
    this->ui->checkBox_notifyBeta->setText(_l("config-notify-beta"));
    this->ui->checkBox_notifyUpdate->setText(_l("config-notify-update"));
    this->ui->checkBox_AutoLoadHistory->setText(_l("config-auto-load-history"));
    this->ui->checkBox_LastRevision->setText(_l("config-last-revision"));
    this->ui->checkBox_RequireDelay->setText(_l("config-require-delay"));
    this->ui->label_WaitEdit->setText(_l("config-wait-edit"));
    this->ui->radioButton_DisplayNext->setText(_l("config-display-next"));
    this->ui->radioButton_RetrieveEdit->setText(_l("config-retrieve-edit"));
    this->ui->radioButton_DoNothing->setText(_l("config-nothing"));
    this->ui->label_botEdits->setText(_l("config-bot-edits"));
    this->ui->label_ownEdits->setText(_l("config-own-edits"));
    this->ui->label_reverts->setText(_l("config-reverts"));
    this->ui->label_8->setText(_l("config-new-page"));
    this->ui->label_minor->setText(_l("config-minor"));
    this->ui->label_9->setText(_l("config-whitelisted"));
    this->ui->label_10->setText(_l("config-friend"));
    this->ui->label_11->setText(_l("config-userspace"));
    this->ui->label_talk->setText(_l("config-talk"));
    this->ui->checkBox_DynamicColumns->setText(_l("config-columns-dynamic"));
    this->ui->checkBox_MessageNotification->setText(_l("config-message-notification"));
    this->ui->checkBox_WarningApi->setText(_l("config-warning-api"));
    this->ui->checkBox_TitleDiff->setText(_l("config-title-diff"));
    this->ui->checkBox_notifyUpdate->setText(_l("config-updates"));
    this->ui->checkBox_notifyBeta->setText(_l("config-beta"));
    this->ui->checkBox_HtmlMessages->setText(_l("config-html-messages"));
    this->ui->label_Unregistered->setText(_l("config-ip"));
    this->ui->checkBox_SummaryPresent->setText(_l("config-summary-present"));
    this->ui->pushButton_OK->setText(_l("ok"));
    this->ui->cbPlayOnNewItem->setText(_l("preferences-sounds-enable-queue"));
    this->ui->cbPlayOnIRCMsg->setText(_l("preferences-sounds-enable-irc"));
    this->ui->pushButton_CloseWin->setText(_l("config-close-without"));
    this->ui->label_minimal_score->setText(_l("preferences-sounds-minimal-score"));
    this->ui->cbCatScansAndWatched->setText(_l("preferences-performance-catscansandwatched"));
    this->ui->cbMaxScore->setText(_l("preferences-max-score"));
    this->ui->cbMinScore->setText(_l("preferences-min-score"));
    this->ui->cb_AutoRefresh->setText(_l("config-auto-refresh"));
    this->ui->cb_WatchWarn->setText(_l("preferences-auto-watch-talk"));
    this->ui->cbKeystrokeFix->setText(_l("preferences-keystroke-rate-limit"));
    this->ui->checkBox_EnforceBAWC->setText(_l("preferences-enforce-baw"));
    this->ui->l_QueueSize->setText(_l("preferences-queue-size"));
    this->ui->l_EmptyQueuePage->setText(_l("preferences-empty-queue-page"));
    this->ui->cbShowWarningIfNotOnLastRevision->setText(_l("preferences-show-warning-if-not-last-revision"));
    this->ui->groupBox_DiffColors->setTitle(_l("preferences-color-scheme-diff"));
    this->ui->radioButton_DarkMode->setText(_l("preferences-color-scheme-diff-dark-mode"));
    this->ui->radioButton_DefaultColors->setText(_l("preferences-color-scheme-diff-default"));
    this->ui->gb_TemplateSpam->setTitle(_l("preferences-template-spam-title"));
    this->ui->label_TemplateSpamPrevInfo->setText(_l("preferences-template-spam-info"));

#ifndef HUGGLE_NOAUDIO
    this->ui->label_NoAudio->setVisible(false);
#endif

    // options
    this->ResetItems();
    this->on_checkBox_RequireDelay_clicked();
    this->on_checkBox_InstantReverts_clicked();

    this->ui->label_pt->setText("<b>" + _l("protip") + ":</b> " + Resources::GetRandomProTip());
    this->RestoreWindow();
    this->queueModified = false;
}

Preferences::~Preferences()
{
    foreach (QCheckBox *i, this->NamespaceBoxes.keys())
        delete i;
    delete this->ui;
}

void Preferences::closeEvent(QCloseEvent *event)
{
    if (this->forceClose)
    {
        event->accept();
        return;
    }

    int result = UiGeneric::pMessageBox(this, _l("preferences-unsaved-title"),
                                       _l("preferences-unsaved-text"),
                                       MessageBoxStyleQuestion);
    if (result == QMessageBox::No)
    {
        event->ignore();
        return;
    }
    event->accept();
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
    if (this->ui->listWidget->currentRow() == this->queueID)
        return;

    if (this->queueModified)
    {
        if (UiGeneric::MessageBox(_l("config-queue-modified-title"), _l("config-queue-modified-text"), MessageBoxStyleQuestion) != QMessageBox::Yes)
        {
            this->ui->listWidget->setCurrentRow(this->queueID);
            return;
        }
    }
    if (!HuggleQueueFilter::Filters.contains(this->Site))
        throw new Huggle::Exception("There is no such a wiki site", BOOST_CURRENT_FUNCTION);
    int id = this->ui->listWidget->currentRow();
    if (id < 0 || id >= HuggleQueueFilter::Filters[this->Site]->count())
    {
        return;
    }
    if (!HuggleQueueFilter::Filters[this->Site]->at(id)->IsChangeable())
    {
        this->EnableQueues(false);
    } else
    {
        this->EnableQueues(true);
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
    SetValue(f->getIgnoreWatched(), this->ui->cbqWatched);
    SetValue(f->getIgnoreIP(), this->ui->cbqIP);
    SetValue(f->getIgnoreMinor(), this->ui->cbqMinor);
    this->ui->leIgnoredTags->setText(f->GetIgnoredTags_CommaSeparated());
    this->ui->leRequiredTags->setText(f->GetRequiredTags_CommaSeparated());
    this->ui->leIgnoredCategories->setText(f->GetIgnoredCategories_CommaSeparated());
    this->ui->leRequiredCategories->setText(f->GetRequiredCategories_CommaSeparated());
    foreach (QCheckBox *cb, this->NamespaceBoxes.keys())
        cb->setChecked(f->IgnoresNS(this->NamespaceBoxes[cb]));
    this->ui->lineEdit->setText(f->QueueName);
    this->queueID = id;
    this->queueModified = false;
}

void Preferences::EnableQueues(bool enabled)
{
    this->ui->lineEdit->setEnabled(enabled);
    this->ui->cbqBots->setEnabled(enabled);
    this->ui->cbqFrd->setEnabled(enabled);
    this->ui->cbqMinor->setEnabled(enabled);
    this->ui->cbqNew->setEnabled(enabled);
    this->ui->cbqOwn->setEnabled(enabled);
    this->ui->cbqIP->setEnabled(enabled);
    this->ui->cbqWatched->setEnabled(enabled);
    this->ui->cbqRevert->setEnabled(enabled);
    this->ui->pushButton_QueueSave->setEnabled(enabled);
    this->ui->cbqTp->setEnabled(enabled);
    this->ui->tableWidget_3->setEnabled(enabled);
    this->ui->pushButton_QueueReset->setEnabled(enabled);
    this->ui->pushButton_QueueDelete->setEnabled(enabled);
    this->ui->leIgnoredTags->setEnabled(enabled);
    this->ui->leRequiredTags->setEnabled(enabled);
    this->ui->leIgnoredCategories->setEnabled(enabled);
    this->ui->leRequiredCategories->setEnabled(enabled);
    this->ui->cbqUserspace->setEnabled(enabled);
    this->ui->cbqWl->setEnabled(enabled);
}

void Huggle::Preferences::on_checkBox_clicked()
{
    this->ui->radioButton_Revert->setEnabled(this->ui->checkBox_AutoResolveConflicts->isChecked());
    this->ui->checkBox_RevertNewerEdits->setEnabled(this->ui->checkBox_AutoResolveConflicts->isChecked());
    this->ui->radioButton->setEnabled(this->ui->checkBox_AutoResolveConflicts->isChecked());
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
        // Connect checkbox signal to our slot
        connect(Item, SIGNAL(toggled(bool)), this, SLOT(onNamespaceBoxToggled(bool)));
        this->NamespaceBoxes.insert(Item, ms->GetID());
        this->ui->tableWidget_3->setCellWidget(rw, 1, Item);
        rw++;
    }
    this->ui->tableWidget_3->resizeColumnsToContents();
    this->ui->tableWidget_3->resizeRowsToContents();
    int c = 0;
    int d = 0;
    this->isNowReloadingFilters = true;
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
    this->isNowReloadingFilters = false;
    this->queueModified = false;
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

void Huggle::Preferences::on_checkBox_RequireDelay_clicked()
{
    this->ui->label_WaitEdit->setEnabled(this->ui->checkBox_RequireDelay->isChecked());
    this->ui->lineEdit_DelayValue->setEnabled(this->ui->checkBox_RequireDelay->isChecked());
}

void Huggle::Preferences::on_checkBox_InstantReverts_clicked()
{
    this->ui->label_RevertWait->setEnabled(!this->ui->checkBox_InstantReverts->isChecked());
    this->ui->lineEdit_RevertDelay->setEnabled(!this->ui->checkBox_InstantReverts->isChecked());
}

void Preferences::RecordKeys(int row, int column)
{
    if (this->shortcutsRewriting)
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

    this->shortcutsModified = true;
    this->shortcutsRewriting = true;
    this->IgnoreConflicts = false;
    hcfg->Shortcuts[id].Modified = true;
    hcfg->Shortcuts[id].QAccel = key;
    this->ui->tableWidget_2->setItem(row, column, new QTableWidgetItem(key));
    this->shortcutsRewriting = false;
    return;

    revert:
        this->IgnoreConflicts = true;
        this->ui->tableWidget_2->setItem(row, column, new QTableWidgetItem(hcfg->Shortcuts[id].QAccel));
        return;
}

void Huggle::Preferences::on_pbHuggle2_clicked()
{
    this->ui->checkBox_WelcomeEmptyPage->setChecked(false);
    this->ui->checkBox_RemoveRevertedEdits->setChecked(true);
    this->ui->checkBox_MonthHeaders->setChecked(true);
    this->ui->checkBox_MergeMessages->setChecked(true);
    this->ui->checkBox_LastRevision->setChecked(true);
    this->ui->checkBox_RemoveOldEdits->setChecked(true);
    this->ui->checkBox_TitleDiff->setChecked(true);
    this->ui->radioButton_RetrieveEdit->setChecked(true);
    this->ui->checkBox_AutoLoadHistory->setChecked(true);
    
    UiGeneric::MessageBox(_l("done"), _l("preferences-huggle2-config-success"));
}

void Huggle::Preferences::on_cbSites_currentIndexChanged(int index)
{
    this->Site = hcfg->Projects.at(index);
    this->Reload();
}

void Huggle::Preferences::on_cbDefault_currentIndexChanged(int index)
{
    if (this->isNowReloadingFilters)
        return;

    // update the filter
    this->Site->UserConfig->QueueID = this->ui->cbDefault->itemText(index);
    this->Site->CurrentFilter = HuggleQueueFilter::GetFilter(this->ui->cbDefault->itemText(index), this->Site);
    MainWindow::HuggleMain->Queue1->Filters();
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
    int lastrow = -1;
    if (selection == disable)
    {
        foreach (QTableWidgetItem *extension, this->ui->tableWidget->selectedItems())
        {
            if (extension->row() == lastrow)
                continue;
            if (extension->row() >= Core::HuggleCore->Extensions.count())
                throw new Huggle::Exception("ERROR: Invalid ext", BOOST_CURRENT_FUNCTION);

            iExtension *ex = Core::HuggleCore->Extensions.at(extension->row());
            lastrow = extension->row();
            if (hcfg->IgnoredExtensions.contains(ex->GetExtensionFullPath()))
            {
                UiGeneric::MessageBox(_l("error"), _l("preferences-extension-disabled"));
            }
            else
            {
                hcfg->IgnoredExtensions.append(ex->GetExtensionFullPath());
                UiGeneric::MessageBox(_l("done"), _l("preferences-extension-disabled-restart"));
            }
        }
    }
    if (selection == enable)
    {
        foreach (QTableWidgetItem *extension, this->ui->tableWidget->selectedItems())
        {
            if (extension->row() == lastrow)
                continue;
            lastrow = extension->row();
            if (extension->row() >= Core::HuggleCore->Extensions.count())
                throw new Huggle::Exception("ERROR: Invalid exception id", BOOST_CURRENT_FUNCTION);

            iExtension *ex = Core::HuggleCore->Extensions.at(extension->row());
            if (!hcfg->IgnoredExtensions.contains(ex->GetExtensionFullPath()))
            {
                UiGeneric::MessageBox(_l("error"), _l("preferences-extension-enabled"));
            }
            else
            {
                hcfg->IgnoredExtensions.removeAll(ex->GetExtensionFullPath());
                UiGeneric::MessageBox(_l("done"), _l("preferences-extension-enabled-restart"));
            }
        }
    }
}

void Preferences::ResetItems()
{
    int provider = 1;
    if (hcfg->UserConfig->PreferredProvider > -1 && hcfg->UserConfig->PreferredProvider < 3)
        provider = hcfg->UserConfig->PreferredProvider;
    this->ui->cbProviders->setCurrentIndex(provider);
    switch(hcfg->UserConfig->GoNext)
    {
        case Configuration_OnNext_Stay:
            this->ui->radioButton_DoNothing->setChecked(true);
            this->ui->radioButton_RetrieveEdit->setChecked(false);
            this->ui->radioButton_DisplayNext->setChecked(false);
            break;
        case Configuration_OnNext_Revert:
            this->ui->radioButton_DoNothing->setChecked(false);
            this->ui->radioButton_RetrieveEdit->setChecked(true);
            this->ui->radioButton_DisplayNext->setChecked(false);
            break;
        case Configuration_OnNext_Next:
            this->ui->radioButton_DoNothing->setChecked(false);
            this->ui->radioButton_DisplayNext->setChecked(true);
            this->ui->radioButton_RetrieveEdit->setChecked(false);
            break;
    }
    this->ui->checkBox_AutomaticallyGroup->setChecked(hcfg->UserConfig->AutomaticallyGroup);
    this->ui->checkBox_RequireDelay->setChecked(hcfg->SystemConfig_RequestDelay);
    this->ui->checkBox_RemoveRevertedEdits->setChecked(hcfg->UserConfig->DeleteEditsAfterRevert);
    this->ui->checkBox_UseRollback->setChecked(hcfg->UserConfig->EnforceSoftwareRollback());
    this->ui->checkBox_WarningApi->setChecked(!hcfg->SystemConfig_SuppressWarnings);
    this->ui->checkBox_ConfirmUserSpaceEditRevert->setChecked(hcfg->SystemConfig_WarnUserSpaceRoll);
    this->ui->checkBox_AutoResolveConflicts->setChecked(hcfg->UserConfig->AutomaticallyResolveConflicts);
    this->ui->checkBox_EnableIrc->setChecked(hcfg->UsingIRC);
    this->ui->checkBox_AutoLoadHistory->setChecked(hcfg->UserConfig->HistoryLoad);
    this->ui->checkBox_ConfirmOwnEditRevert->setChecked(hcfg->ProjectConfig->ConfirmOnSelfRevs);
    this->ui->checkBox_ConfirmWhitelistedRevert->setChecked(hcfg->ProjectConfig->ConfirmWL);
    this->ui->checkBox_ConfirmTalkRevert->setChecked(hcfg->ProjectConfig->ConfirmTalk);
    this->ui->checkBox_MonthHeaders->setChecked(hcfg->UserConfig->EnforceMonthsAsHeaders);
    this->ui->checkBox_RemoveOldEdits->setChecked(hcfg->UserConfig->TruncateEdits);
    this->ui->lineEdit_DelayValue->setText(QString::number(hcfg->SystemConfig_DelayVal));
    this->ui->radioButton->setChecked(!hcfg->UserConfig->RevertOnMultipleEdits);
    this->ui->checkBox_LastRevision->setChecked(hcfg->UserConfig->LastEdit);
    this->ui->checkBox_MergeMessages->setChecked(hcfg->UserConfig->SectionKeep);
    this->ui->ck_RemoveTrusted->setChecked(hcfg->UserConfig->RemoveAfterTrustedEdit);
    this->ui->radioButton_Revert->setChecked(hcfg->UserConfig->RevertOnMultipleEdits);
    this->ui->checkBox_RevertNewerEdits->setEnabled(this->ui->checkBox_AutoResolveConflicts->isChecked());
    this->ui->radioButton_Revert->setEnabled(this->ui->checkBox_AutoResolveConflicts->isChecked());
    this->ui->checkBox_RevertNewerEdits->setChecked(hcfg->UserConfig->RevertNewBySame);
    this->ui->radioButton->setEnabled(this->ui->checkBox_AutoResolveConflicts->isChecked());
    this->ui->lineEdit_RevertDelay->setText(QString::number(hcfg->SystemConfig_RevertDelay));
    this->ui->checkBox_AutoWarning->setChecked(!hcfg->UserConfig->ManualWarning);
    this->ui->checkBox_MessageNotification->setChecked(hcfg->UserConfig->CheckTP);
    this->ui->checkBox_InstantReverts->setChecked(hcfg->SystemConfig_InstantReverts);
    this->ui->checkBox_DynamicColumns->setChecked(hcfg->SystemConfig_DynamicColsInList);
    this->ui->checkBox_TitleDiff->setChecked(hcfg->UserConfig->DisplayTitle);
    this->ui->checkBox_WelcomeEmptyPage->setChecked(hcfg->UserConfig->WelcomeGood);
    this->ui->checkBox_AutoReport->setChecked(hcfg->UserConfig->AutomaticReports);
    this->ui->checkBox_HtmlMessages->setChecked(hcfg->UserConfig->HtmlAllowedInIrc);
    this->ui->lineEdit_Font->setText(hcfg->SystemConfig_Font);
    this->ui->sxFontSize->setValue(hcfg->SystemConfig_FontSize);
    this->ui->checkBox_RetrieveFounder->setChecked(hcfg->UserConfig->RetrieveFounder);
    this->ui->cbMaxScore->setChecked(hcfg->UserConfig->EnableMaxScore);
    this->ui->cbMinScore->setChecked(hcfg->UserConfig->EnableMinScore);
    this->ui->le_QueueSize->setText(QString::number(hcfg->SystemConfig_QueueSize));
    this->ui->le_EmptyQueuePage->setText(hcfg->UserConfig->PageEmptyQueue);
    this->ui->leMaxScore->setText(QString::number(hcfg->UserConfig->MaxScore));
    this->ui->leMinScore->setText(QString::number(hcfg->UserConfig->MinScore));
    this->ui->checkBox_notifyUpdate->setChecked(hcfg->SystemConfig_EnableUpdates);
    this->ui->checkBox_notifyBeta->setChecked(hcfg->SystemConfig_NotifyBeta);
    this->ui->ln_QueueSoundMinScore->setText(QString::number(hcfg->SystemConfig_PlaySoundQueueScore));
    this->ui->cbPlayOnNewItem->setChecked(hcfg->SystemConfig_PlaySoundOnQueue);
    this->ui->cbPlayOnIRCMsg->setChecked(hcfg->SystemConfig_PlaySoundOnIRCUserMsg);
    this->ui->cbCatScansAndWatched->setChecked(hcfg->SystemConfig_CatScansAndWatched);
    this->ui->cbShowWarningIfNotOnLastRevision->setChecked(hcfg->UserConfig->ShowWarningIfNotOnLastRevision);
    this->ui->cb_AutoRefresh->setChecked(hcfg->UserConfig->AutomaticRefresh);
    this->ui->cb_WatchWarn->setChecked(hcfg->UserConfig->AutomaticallyWatchlistWarnedUsers);
    this->ui->le_KeystrokeRate->setText(QString::number(hcfg->SystemConfig_KeystrokeMultiPressRate));
    this->ui->checkBox_SummaryPresent->setChecked(hcfg->UserConfig->HighlightSummaryIfExists);
    this->ui->cbNumberMenus->setChecked(hcfg->UserConfig->NumberDropdownMenuItems);
    this->ui->checkBox_ReviewEditsMadeByVandal->setChecked(hcfg->UserConfig->InsertEditsOfRolledUserToQueue);
    this->ui->checkBox_unsafe->setChecked(hcfg->SystemConfig_UnsafeExts);
    this->ui->cbKeystrokeFix->setChecked(hcfg->SystemConfig_KeystrokeMultiPressFix);
    this->ui->checkBox_OldEdits->setChecked(hcfg->UserConfig->ConfirmWarningOnVeryOldEdits);
    this->ui->checkBox_SkipConfirm->setChecked(hcfg->UserConfig->SkipWarningOnConfirm);
    this->ui->checkBox_RecentMsgs->setChecked(hcfg->UserConfig->ConfirmOnRecentWarning);
    this->ui->checkBox_EnforceBAWC->setChecked(hcfg->SystemConfig_EnforceBlackAndWhiteCss);
    this->ui->radioButton_DarkMode->setChecked(hcfg->SystemConfig_ColorScheme == 1);
    this->ui->radioButton_DefaultColors->setChecked(hcfg->SystemConfig_ColorScheme == 0);

#ifdef HUGGLE_NOAUDIO
    this->ui->ln_QueueSoundMinScore->setEnabled(false);
    this->ui->cbPlayOnNewItem->setEnabled(false);
    this->ui->cbPlayOnIRCMsg->setEnabled(false);
#endif
}

void Huggle::Preferences::on_pushButton_rs_clicked()
{
    if (UiGeneric::pMessageBox(this, _l("preferences-reset-gui"), _l("preferences-restore-factory-layout"), MessageBoxStyleQuestion) == QMessageBox::No)
        return;
    Configuration::HuggleConfiguration->SystemConfig_SaveLayout = false;
    // remove all layout files
    QDir config(Configuration::HuggleConfiguration->GetConfigurationPath());
    config.setNameFilters(QStringList() << "*_state" << "*_geometry");
    config.setFilter(QDir::Files);
    foreach(QString file, config.entryList())
    {
        if (!config.remove(file))
            throw new Huggle::Exception("Unable to delete " + file, BOOST_CURRENT_FUNCTION);
    }
    MainWindow::HuggleMain->Exit();
}

void Huggle::Preferences::on_cbqBots_currentIndexChanged(int index)
{
    (void)index;
    this->queueModified = true;
}

void Huggle::Preferences::on_cbqIP_currentIndexChanged(int index)
{
    (void)index;
    this->queueModified = true;
}

void Huggle::Preferences::on_cbqOwn_currentIndexChanged(int index)
{
    (void)index;
    this->queueModified = true;
}

void Huggle::Preferences::on_cbqRevert_currentIndexChanged(int index)
{
    (void)index;
    this->queueModified = true;
}

void Huggle::Preferences::on_cbqNew_currentIndexChanged(int index)
{
    (void)index;
    this->queueModified = true;
}

void Huggle::Preferences::on_cbqMinor_currentIndexChanged(int index)
{
    (void)index;
    this->queueModified = true;
}

void Huggle::Preferences::on_cbqWl_currentIndexChanged(int index)
{
    (void)index;
    this->queueModified = true;
}

void Huggle::Preferences::on_cbqFrd_currentIndexChanged(int index)
{
    (void)index;
    this->queueModified = true;
}

void Huggle::Preferences::on_cbqUserspace_currentIndexChanged(int index)
{
    (void)index;
    this->queueModified = true; 
}

void Huggle::Preferences::on_cbqTp_currentIndexChanged(int index)
{
    (void)index;
    this->queueModified = true;
}

void Huggle::Preferences::on_cbqWatched_currentIndexChanged(int index)
{
    (void)index;
    this->queueModified = true;
}

void Huggle::Preferences::on_leIgnoredTags_textEdited(const QString &arg1)
{
    (void)arg1;
    this->queueModified = true;
}

void Huggle::Preferences::on_leIgnoredCategories_textEdited(const QString &arg1)
{
    (void)arg1;
    this->queueModified = true;
}

void Huggle::Preferences::on_leRequiredTags_textEdited(const QString &arg1)
{
    (void)arg1;
    this->queueModified = true;
}

void Huggle::Preferences::on_leRequiredCategories_textEdited(const QString &arg1)
{
    (void)arg1;
    this->queueModified = true;
}

void Huggle::Preferences::on_pushButton_OK_clicked()
{
    if (this->queueModified)
    {
        if (UiGeneric::MessageBox(_l("config-queue-modified-title"), _l("config-queue-modified-text"), MessageBoxStyleQuestion) != QMessageBox::Yes)
            return;
    }
    hcfg->UserConfig->AutomaticallyResolveConflicts = this->ui->checkBox_AutoResolveConflicts->isChecked();
    hcfg->SystemConfig_WarnUserSpaceRoll = this->ui->checkBox_ConfirmUserSpaceEditRevert->isChecked();
    hcfg->SystemConfig_SuppressWarnings = !this->ui->checkBox_WarningApi->isChecked();
    hcfg->UsingIRC = this->ui->checkBox_EnableIrc->isChecked();
    hcfg->UserConfig->EnforceManualSoftwareRollback = this->ui->checkBox_UseRollback->isChecked();
    hcfg->UserConfig->AutomaticallyGroup = this->ui->checkBox_AutomaticallyGroup->isChecked();
    hcfg->UserConfig->EnforceManualSRT = this->ui->checkBox_UseRollback->isChecked();
    hcfg->UserConfig->RevertOnMultipleEdits = this->ui->radioButton_Revert->isChecked();
    hcfg->ProjectConfig->ConfirmOnSelfRevs = this->ui->checkBox_ConfirmOwnEditRevert->isChecked();
    hcfg->ProjectConfig->ConfirmWL = this->ui->checkBox_ConfirmWhitelistedRevert->isChecked();
    hcfg->UserConfig->RevertNewBySame = this->ui->checkBox_RevertNewerEdits->isChecked();
    hcfg->UserConfig->HistoryLoad = this->ui->checkBox_AutoLoadHistory->isChecked();
    hcfg->UserConfig->EnforceMonthsAsHeaders = this->ui->checkBox_MonthHeaders->isChecked();
    hcfg->UserConfig->SectionKeep = this->ui->checkBox_MergeMessages->isChecked();
    hcfg->ProjectConfig->ConfirmTalk = this->ui->checkBox_ConfirmTalkRevert->isChecked();
    hcfg->UserConfig->LastEdit = this->ui->checkBox_LastRevision->isChecked();
    hcfg->UserConfig->DeleteEditsAfterRevert = this->ui->checkBox_RemoveRevertedEdits->isChecked();
    hcfg->UserConfig->TruncateEdits = this->ui->checkBox_RemoveOldEdits->isChecked();
    hcfg->SystemConfig_DynamicColsInList = this->ui->checkBox_DynamicColumns->isChecked();
    hcfg->UserConfig->DisplayTitle = this->ui->checkBox_TitleDiff->isChecked();
    hcfg->UserConfig->PreferredProvider = this->ui->cbProviders->currentIndex();
    hcfg->UserConfig->ManualWarning = !this->ui->checkBox_AutoWarning->isChecked();
    hcfg->UserConfig->RetrieveFounder = this->ui->checkBox_RetrieveFounder->isChecked();
    hcfg->UserConfig->CheckTP = this->ui->checkBox_MessageNotification->isChecked();
    hcfg->SystemConfig_RequestDelay = this->ui->checkBox_RequireDelay->isChecked();
    hcfg->UserConfig->RemoveAfterTrustedEdit = this->ui->ck_RemoveTrusted->isChecked();
    hcfg->SystemConfig_DelayVal = this->ui->lineEdit_DelayValue->text().toUInt();
    hcfg->SystemConfig_RevertDelay = this->ui->lineEdit_RevertDelay->text().toInt();
    hcfg->SystemConfig_InstantReverts = this->ui->checkBox_InstantReverts->isChecked();
    hcfg->UserConfig->AutomaticReports = this->ui->checkBox_AutoReport->isChecked();
    hcfg->SystemConfig_EnableUpdates = this->ui->checkBox_notifyUpdate->isChecked();
    hcfg->SystemConfig_NotifyBeta = this->ui->checkBox_notifyBeta->isChecked();
    hcfg->UserConfig->HtmlAllowedInIrc = this->ui->checkBox_HtmlMessages->isChecked();
    hcfg->UserConfig->EnableMinScore = this->ui->cbMinScore->isChecked();
    hcfg->UserConfig->MinScore = this->ui->leMinScore->text().toLongLong();
    hcfg->UserConfig->MaxScore = this->ui->leMaxScore->text().toLongLong();
    hcfg->UserConfig->NumberDropdownMenuItems = this->ui->cbNumberMenus->isChecked();
    hcfg->UserConfig->EnableMaxScore = this->ui->cbMaxScore->isChecked();
    hcfg->SystemConfig_QueueSize = this->ui->le_QueueSize->text().toInt();
    hcfg->UserConfig->AutomaticallyWatchlistWarnedUsers = this->ui->cb_WatchWarn->isChecked();
    hcfg->UserConfig->PageEmptyQueue = this->ui->le_EmptyQueuePage->text();
    hcfg->UserConfig->AutomaticRefresh = this->ui->cb_AutoRefresh->isChecked();
    if (hcfg->SystemConfig_QueueSize < HUGGLE_MIN_QUEUE_SIZE)
        hcfg->SystemConfig_QueueSize = HUGGLE_MIN_QUEUE_SIZE;
    hcfg->SystemConfig_PlaySoundOnIRCUserMsg = this->ui->cbPlayOnIRCMsg->isChecked();
    hcfg->SystemConfig_PlaySoundQueueScore = this->ui->ln_QueueSoundMinScore->text().toLong();
    hcfg->SystemConfig_PlaySoundOnQueue = this->ui->cbPlayOnNewItem->isChecked();
    hcfg->SystemConfig_CatScansAndWatched = this->ui->cbCatScansAndWatched->isChecked();
    hcfg->UserConfig->ShowWarningIfNotOnLastRevision = this->ui->cbShowWarningIfNotOnLastRevision->isChecked();
    hcfg->UserConfig->HighlightSummaryIfExists = this->ui->checkBox_SummaryPresent->isChecked();
    hcfg->UserConfig->InsertEditsOfRolledUserToQueue = this->ui->checkBox_ReviewEditsMadeByVandal->isChecked();
    hcfg->SystemConfig_FontSize = this->ui->sxFontSize->value();
    hcfg->SystemConfig_EnforceBlackAndWhiteCss = this->ui->checkBox_EnforceBAWC->isChecked();
    hcfg->UserConfig->ConfirmOnRecentWarning = this->ui->checkBox_RecentMsgs->isChecked();
    hcfg->UserConfig->SkipWarningOnConfirm = this->ui->checkBox_SkipConfirm->isChecked();
    hcfg->UserConfig->ConfirmWarningOnVeryOldEdits = this->ui->checkBox_OldEdits->isChecked();

    if (hcfg->SystemConfig_FontSize < 1)
        hcfg->SystemConfig_FontSize = 10;

    hcfg->UserConfig->Watchlist = static_cast<WatchlistOption>(this->ui->comboBox_WatchlistPreference->currentIndex());
    hcfg->SystemConfig_Font = this->ui->lineEdit_Font->text();
    hcfg->SystemConfig_UnsafeExts = this->ui->checkBox_unsafe->isChecked();

    hcfg->SystemConfig_KeystrokeMultiPressRate = this->ui->le_KeystrokeRate->text().toInt();
    hcfg->SystemConfig_KeystrokeMultiPressFix = this->ui->cbKeystrokeFix->isChecked();
    if (this->ui->radioButton_DarkMode->isChecked())
        hcfg->SystemConfig_ColorScheme = 1;
    else
        hcfg->SystemConfig_ColorScheme = 0;

    if (hcfg->UserConfig->WelcomeGood != this->ui->checkBox_WelcomeEmptyPage->isChecked())
    {
        hcfg->UserConfig->WelcomeGood = this->ui->checkBox_WelcomeEmptyPage->isChecked();
        // now we need to update the option as well just to ensure that user config will be updated as well
        // this option needs to be written only if it was explicitly changed by user to a value that
        // is different from a project config file
        HuggleOption *o_ = hcfg->UserConfig->GetOption("welcome-good");
        if (o_)
            o_->SetVariant(hcfg->UserConfig->WelcomeGood);
    }
    if (this->ui->radioButton_DoNothing->isChecked())
    {
        hcfg->UserConfig->GoNext = Configuration_OnNext_Stay;
    }
    if (this->ui->radioButton_RetrieveEdit->isChecked())
    {
        hcfg->UserConfig->GoNext = Configuration_OnNext_Revert;
    }
    if (this->ui->radioButton_DisplayNext->isChecked())
    {
        hcfg->UserConfig->GoNext = Configuration_OnNext_Next;
    }
    if (this->shortcutsModified)
    {
        // we need to reload the shortcuts in main form
        hcfg->ReloadOfMainformNeeded = true;
    }
    Configuration::SaveSystemConfig();
    MainWindow::HuggleMain->ReloadInterface();

    this->hide();
}

void Huggle::Preferences::on_pushButton_CloseWin_clicked()
{
    this->forceClose = true;
    this->close();
    this->forceClose = false;
}

void Huggle::Preferences::on_pushButton_QueueSave_clicked()
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
    filter->SetIgnoredTags_CommaSeparated(this->ui->leIgnoredTags->text());
    filter->SetRequiredTags_CommaSeparated(this->ui->leRequiredTags->text());
    filter->SetIgnoredCategories_CommaSeparated(this->ui->leIgnoredCategories->text());
    filter->SetRequiredCategories_CommaSeparated(this->ui->leRequiredCategories->text());
    filter->setIgnoreBots(Match(this->ui->cbqBots));
    filter->setIgnoreNP(Match(this->ui->cbqNew));
    filter->setIgnoreIP(Match(this->ui->cbqIP));
    filter->setIgnoreMinor(Match(this->ui->cbqMinor));
    filter->setIgnoreWL(Match(this->ui->cbqWl));
    filter->setIgnoreSelf(Match(this->ui->cbqOwn));
    filter->setIgnoreReverts(Match(this->ui->cbqRevert));
    filter->setIgnoreTalk(Match(this->ui->cbqTp));
    filter->setIgnoreFriends(Match(this->ui->cbqFrd));
    filter->setIgnore_UserSpace(Match(this->ui->cbqUserspace));
    filter->setIgnoreWatched(Match(this->ui->cbqWatched));
    int ns = 0;
    while (ns < this->ui->tableWidget_3->rowCount())
    {
        QCheckBox *selected_box = dynamic_cast<QCheckBox*>(this->ui->tableWidget_3->cellWidget(ns, 1));
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
    MainWindow::HuggleMain->Queue1->Filters();
    this->Reload();
    this->queueModified = false;
}

void Huggle::Preferences::on_pushButton_QueueDelete_clicked()
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
    if (this->Site->CurrentFilter == filter)
    {
        UiGeneric::MessageBox(_l("error"), _l("preferences-delete-using-filter"), MessageBoxStyleWarning);
        return;
    }
    HuggleQueueFilter::Filters[this->Site]->removeAll(filter);
    delete filter;
    this->EnableQueues(false);
    MainWindow::HuggleMain->Queue1->Filters();
    this->Reload();
}

void Huggle::Preferences::on_pushButton_QueueInsert_clicked()
{
    if (!HuggleQueueFilter::Filters.contains(this->Site))
        throw new Huggle::Exception("There is no such a wiki site", BOOST_CURRENT_FUNCTION);
    HuggleQueueFilter *filter = new HuggleQueueFilter();
    filter->QueueName = "User defined queue #" + QString::number(HuggleQueueFilter::Filters[this->Site]->count());
    HuggleQueueFilter::Filters[this->Site]->append(filter);
    MainWindow::HuggleMain->Queue1->Filters();
    this->Reload();
}

void Huggle::Preferences::on_pushButton_QueueReset_clicked()
{

}

void Huggle::Preferences::on_listWidget_customContextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = this->ui->listWidget->mapToGlobal(pos);
    QMenu menu;
    QAction *addItem = new QAction(_l("queue-filter-add"), &menu);
    QAction *deleteItem = new QAction(_l("queue-filter-delete"), &menu);
    
    // Only enable delete if an item is selected and it's changeable
    bool canDelete = false;
    if (this->ui->listWidget->currentRow() >= 0)
    {
        int id = this->ui->listWidget->currentRow();
        if (id < HuggleQueueFilter::Filters[this->Site]->count())
        {
            HuggleQueueFilter *filter = HuggleQueueFilter::Filters[this->Site]->at(id);
            canDelete = filter->IsChangeable() && (this->Site->CurrentFilter != filter);
        }
    }
    
    deleteItem->setEnabled(canDelete);
    
    menu.addAction(addItem);
    menu.addAction(deleteItem);
    
    QAction *selectedItem = menu.exec(globalPos);
    
    if (selectedItem == addItem)
    {
        this->on_pushButton_QueueInsert_clicked();
    }
    else if (selectedItem == deleteItem)
    {
        this->on_pushButton_QueueDelete_clicked();
    }
}

void Huggle::Preferences::onNamespaceBoxToggled(bool checked)
{
    Q_UNUSED(checked);
    
    // Mark queue as modified when any namespace checkbox is toggled
    this->queueModified = true;
}

void Huggle::Preferences::on_pushButton_ResetConfig_clicked()
{
    // Display a confirmation dialog before resetting
    if (UiGeneric::pMessageBox(this, _l("preferences-reset-config-title"), _l("preferences-reset-config-question"), MessageBoxStyleQuestion) == QMessageBox::No)
        return;
    
    // For now we only delete the configuration files, but in future we need to also clear the user config page

    /*
    // Create waiting form
    UiGeneric::WaitingForm *wf = new UiGeneric::WaitingForm(_l("preferences-reset-config-waiting"));
    this->setEnabled(false);
    wf->show();
    
    // Create edit request to clear the user config page
    QString page = HCFG->GlobalConfig_UserConf;
    page = page.replace("$1", HCFG->SystemConfig_UserName);
    Collectable_SmartPtr<EditQuery> temp = WikiUtil::EditPage(HCFG->Project, page, "", _l("saveuserconfig-progress"), true);

    */

    // Delete all layout and configuration files
    Configuration::HuggleConfiguration->SystemConfig_SaveLayout = false;
    // remove all layout files
    QDir config(Configuration::HuggleConfiguration->GetConfigurationPath());
    config.setNameFilters(QStringList() << "*_state" << "*_geometry" << "huggle3.xml");
    config.setFilter(QDir::Files);
    foreach(QString file, config.entryList())
    {
        if (!config.remove(file))
            throw new Huggle::Exception("Unable to delete " + file, BOOST_CURRENT_FUNCTION);
    }
    UiGeneric::MessageBox(_l("preferences-reset-config-done"), _l("preferences-reset-config-restart"));
    MainWindow::HuggleMain->Exit();
}
