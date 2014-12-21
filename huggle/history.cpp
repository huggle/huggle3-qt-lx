//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "history.hpp"
#include <QMenu>
#include <QMessageBox>
#include <QTimer>
#include "configuration.hpp"
#include "revertquery.hpp"
#include "exception.hpp"
#include "huggleparser.hpp"
#include "localization.hpp"
#include "mainwindow.hpp"
#include "syslog.hpp"
#include "generic.hpp"
#include "querypool.hpp"
#include "ui_history.h"
#include "wikisite.hpp"
#include "wikiuser.hpp"
#include "wikiutil.hpp"

using namespace Huggle;

History::History(QWidget *parent) : QDockWidget(parent), ui(new Ui::History)
{
    this->ui->setupUi(this);
    this->setWindowTitle(_l("userhistory-title"));
    this->timerRetrievePageInformation = new QTimer(this);
    connect(this->timerRetrievePageInformation, SIGNAL(timeout()), this, SLOT(Tick()));
    this->ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this->ui->tableWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ContextMenu(QPoint)));
    this->ui->tableWidget->setColumnCount(5);
    QStringList header;
    header << _l("[[id]]") << _l("[[type]]") << _l("[[target]]") << _l("result") << _l("project");
    this->ui->tableWidget->setHorizontalHeaderLabels(header);
    this->ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    this->ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
#if QT_VERSION >= 0x050000
// Qt5 code
    this->ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
// Qt4 code
    this->ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    this->timerDisplayEditFromHistLs = new QTimer();
    connect(this->timerDisplayEditFromHistLs, SIGNAL(timeout()), this, SLOT(Display()));
    this->ui->tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->ui->tableWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->ui->tableWidget->setShowGrid(false);
}

History::~History()
{
    this->DeleteItems();
    delete this->ui;
    delete this->timerDisplayEditFromHistLs;
    delete this->timerRetrievePageInformation;
}

void History::Undo(HistoryItem *hist)
{
    if (this->RevertingItem != nullptr)
    {
        Syslog::HuggleLogs->ErrorLog("I am already undoing another edit, please wait");
        return;
    }
    if (hist == nullptr)
    {
        // we need to get a currently selected item
        if (this->CurrentItem < 0)
        {
            Syslog::HuggleLogs->ErrorLog(_l("history-not-found"));
            return;
        }
        hist = this->Items.at(this->CurrentItem);
    }
    if (hist->Undone)
    {
        Syslog::HuggleLogs->ErrorLog(_l("history-already-undone"));
        return;
    }
    if (!hist->IsRevertable)
    {
        // there is no way to revert this
        Generic::MessageBox(_l("history-error-message-title"), _l("history-error"), MessageBoxStyleWarning, true);
        return;
    }
    switch (hist->Type)
    {
        case HistoryMessage:
            // we need to revert only the newly created message on talk page, eg. last edit we made on talk page
            if (hist->ReferencedBy)
            {
                int reply;
                // let's ask user if they really want to undo just message because that is weird to do
                reply = Generic::MessageBox(_l("history-message-revert-title"), _l("history-message-revert-body"),
                                              MessageBoxStyleQuestion);
                if (reply == QMessageBox::No)
                    return;

                hist->ReferencedBy->UndoDependency = nullptr;
                hist->ReferencedBy = nullptr;
            }
            this->RevertingItem = hist;
            this->qEdit = Generic::RetrieveWikiPageContents("User_talk:" + hist->Target, hist->GetSite());
            this->qEdit->Site = hist->GetSite();
            this->qEdit->Process();
            QueryPool::HugglePool->AppendQuery(this->qEdit);
            this->timerRetrievePageInformation->start(20);
            break;
        case HistoryRollback:
        case HistoryEdit:
            // we need to revert both warning of user as well as page we rolled back
            this->RevertingItem = hist;
            this->qEdit = Generic::RetrieveWikiPageContents(hist->Target, hist->GetSite());
            this->qEdit->Site = hist->GetSite();
            this->qEdit->Process();
            QueryPool::HugglePool->AppendQuery(this->qEdit);
            this->timerRetrievePageInformation->start(20);
            break;
        case HistoryBlock:
        case HistoryDelete:
        case HistoryProtect:
        case HistoryUndelete:
            Generic::MessageBox(_l("history-error-message-title"), "This feature was not implemented yet");
            return;
        case HistoryUnknown:
            Generic::MessageBox(_l("history-error-message-title"), "Unknown item");
            return;
    }
    Syslog::HuggleLogs->Log(_l("history-work", hist->Target));
}

void History::ContextMenu(const QPoint &position)
{
    QPoint g_ = this->ui->tableWidget->mapToGlobal(position);
    QMenu menu;
    QAction *clean = new QAction(_l("clear"), &menu);
    QAction *show = new QAction(_l("show"), &menu);
    QAction *undo = new QAction(_l("undo"), &menu);
    menu.addAction(show);
    menu.addAction(undo);
    menu.addAction(clean);
    QAction *selection = menu.exec(g_);
    if (selection == undo)
    {
        this->Undo(nullptr);
    } else if (selection == show)
    {
        if (this->DisplayedEdit != nullptr)
        {
            this->timerDisplayEditFromHistLs->stop();
        }
        // now the tricky part is here, we need to get an undo item and display it
        HistoryItem *hi;
        if (this->CurrentItem < 0)
        {
            Syslog::HuggleLogs->ErrorLog("No item is selected in history widget");
            return;
        }
        hi = this->Items.at(this->CurrentItem);
        QString page = hi->Target;
        Collectable_SmartPtr<WikiEdit> edit;
        switch (hi->Type)
        {
            case HistoryMessage:
                page = "User_talk:" + page;
            case HistoryEdit:
            case HistoryRollback:
                // let's see if we know this edit
                edit = WikiEdit::FromCacheByRevID(hi->RevID);
                if (edit == nullptr)
                {
                    // if we don't know it we need to create it
                    edit = new WikiEdit();
                    edit->Page = new WikiPage(page);
                    edit->Page->Site = hi->GetSite();
                    edit->User = new WikiUser(Configuration::HuggleConfiguration->SystemConfig_Username);
                    edit->User->Site = hi->GetSite();
                    edit->RevID = hi->RevID;
                }
                break;
            case HistoryUnknown:
            case HistoryBlock:
            case HistoryUndelete:
            case HistoryDelete:
            case HistoryProtect:
                return;
        }
        if (!edit->IsPostProcessed())
        {
            // now we need to display it
            QueryPool::HugglePool->PreProcessEdit(edit);
            QueryPool::HugglePool->PostProcessEdit(edit);
            this->DisplayedEdit = edit;
            // we need to enable the timer and display this edit as soon as it is post processed
            this->timerDisplayEditFromHistLs->start(200);
        } else
        {
            // let's display it
            MainWindow::HuggleMain->ProcessEdit(edit);
        }
    } else if (selection == clean)
    {
        this->DeleteItems();
    }
}

void History::Tick()
{
    if ((this->qSelf != nullptr && this->qSelf->IsProcessed()) || (this->qTalk != nullptr && this->qTalk->IsProcessed()))
    {
        if (this->qSelf != nullptr && this->qSelf->IsFailed())
        {
            Syslog::HuggleLogs->ErrorLog(_l("history-revert-fail", this->RevertingItem->Target, this->qSelf->Result->ErrorMessage));
            this->Fail();
            return;
        }
        if (this->qTalk != nullptr && this->qTalk->IsFailed())
        {
            Syslog::HuggleLogs->ErrorLog(_l("history-revert-fail", this->RevertingItem->Target, this->qTalk->Result->ErrorMessage));
            this->Fail();
            return;
        }
        // we finished reverting the edit
        this->RevertingItem->Undone = true;
        Syslog::HuggleLogs->Log("Successfully undone edit to " + this->RevertingItem->Target);
        int position = this->ui->tableWidget->rowCount() - this->RevertingItem->ID;
        this->ui->tableWidget->setItem(position, 3, new QTableWidgetItem("Undone"));
        // we need to delete all queries now
        this->qSelf.Delete();
        // if we don't delete this it will stop working
        this->qTalk.Delete();
        // let's see if there is any dep and if so, let's undo it as well
        if (this->RevertingItem->UndoDependency)
        {
            HistoryItem *deps = this->RevertingItem->UndoDependency;
            deps->ReferencedBy = nullptr;
            this->RevertingItem = nullptr;
            this->Undo(deps);
            HUGGLE_DEBUG("Removed a dependency of " + this->RevertingItem->Target, 2);
        } else
        {
            this->RevertingItem = nullptr;
            this->timerRetrievePageInformation->stop();
        }
        return;
    }
    // we check the status of edit
    if (this->qEdit != nullptr && this->qEdit->IsProcessed())
    {
        bool failed = false;
        QString user, title;
        long revid;
        QString result = Generic::EvaluateWikiPageContents(this->qEdit, &failed, nullptr, nullptr, &user, &revid, nullptr, &title);
        this->qEdit.Delete();
        if (failed)
        {
            Syslog::HuggleLogs->ErrorLog("Unable to retrieve content of page we wanted to undo own edit for, error was: " + result);
            this->RevertingItem = nullptr;
            this->timerRetrievePageInformation->stop();
            return;
        }
        Collectable_SmartPtr<WikiEdit> edit = new WikiEdit();
        if (this->RevertingItem->Type == HistoryMessage)
        {
            edit->Page = new WikiPage("User_talk:" + this->RevertingItem->Target);
        } else
        {
            edit->Page = new WikiPage(this->RevertingItem->Target);
        }
        edit->User = new WikiUser(Configuration::HuggleConfiguration->SystemConfig_Username);
        edit->Page->Contents = result;
        edit->RevID = revid;
        if (this->RevertingItem->NewPage && this->RevertingItem->Type == HistoryMessage)
        {
            int message = Generic::MessageBox("Send welcome message instead?", "You created this talk page, so it can't be undone"\
                                              ", do you want to replace it with a welcome template?", MessageBoxStyleQuestion);
            if (message == QMessageBox::Yes)
            {
                if (Configuration::HuggleConfiguration->ProjectConfig->WelcomeTypes.count() == 0)
                {
                    // This error should never happen so we don't need to localize this
                    Syslog::HuggleLogs->Log("There are no welcome messages defined for this project");
                    this->Fail();
                    return;
                }
                QString message = HuggleParser::GetValueFromKey(Configuration::HuggleConfiguration->ProjectConfig->WelcomeTypes.at(0));
                if (!message.size())
                {
                    // This error should never happen so we don't need to localize this
                    Syslog::HuggleLogs->ErrorLog("Invalid welcome template, ignored message");
                    this->Fail();
                    return;
                }
                this->qTalk = WikiUtil::EditPage(edit->Page, message, Configuration::HuggleConfiguration->ProjectConfig->WelcomeSummary, true);
                return;
            } else
            {
                this->RevertingItem = nullptr;
                this->timerRetrievePageInformation->stop();
                return;
            }
        }
        // so now we have likely everything we need, let's revert that page :D
        this->qSelf = WikiUtil::RevertEdit(edit, "Undoing own edit");
        // set it to undo only a last edit
        this->qSelf->SetLast();
        // revert it!!
        this->qSelf->Process();
    }
}

void History::Display()
{
    if (this->DisplayedEdit == nullptr)
        return;

    if (!this->DisplayedEdit->IsPostProcessed())
        return;

    MainWindow::HuggleMain->ProcessEdit(this->DisplayedEdit);
    this->DisplayedEdit.Delete();
    this->timerDisplayEditFromHistLs->stop();
}

void History::Prepend(HistoryItem *item)
{
    // first of all check all items we have in a list
    item->IncRef();
    foreach (HistoryItem* item_, this->Items)
    {
        if (item_->IsRevertable && item_->Target == item->Target)
            item_->IsRevertable = false;
    }
    this->Items.insert(0, item);
    this->ui->tableWidget->insertRow(0);
    this->ui->tableWidget->setItem(0, 0, new QTableWidgetItem(QString::number(item->ID)));
    this->ui->tableWidget->setItem(0, 1, new QTableWidgetItem(HistoryItem::TypeToString(item->Type)));
    this->ui->tableWidget->setItem(0, 2, new QTableWidgetItem(item->Target));
    this->ui->tableWidget->setItem(0, 3, new QTableWidgetItem(item->Result));
    this->ui->tableWidget->setItem(0, 4, new QTableWidgetItem(item->GetSite()->Name));
    this->ui->tableWidget->resizeRowToContents(0);
}

void Huggle::History::on_tableWidget_clicked(const QModelIndex &index)
{
    this->CurrentItem = index.row();
}

void History::DeleteItems()
{
    while (this->Items.count() > 0)
    {
        this->ui->tableWidget->removeRow(0);
        HistoryItem *hi = this->Items.at(0);
        this->Items.removeAt(0);
        hi->DecRef();
    }
}

void History::Fail()
{
    this->qTalk.Delete();
    this->qSelf.Delete();
    this->RevertingItem = nullptr;
    this->timerRetrievePageInformation->stop();
}
