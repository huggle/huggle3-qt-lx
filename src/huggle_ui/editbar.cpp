//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "editbar.hpp"
#include <huggle_core/configuration.hpp>
#include <huggle_core/exception.hpp>
#include <huggle_core/localization.hpp>
#include <huggle_core/syslog.hpp>
#include <huggle_core/wikipage.hpp>
#include <huggle_core/wikiuser.hpp>
#include <huggle_core/wikiedit.hpp>
#include <huggle_core/huggleprofiler.hpp>
#include "ui_editbar.h"
#include <QModelIndex>
#include <QScrollBar>
#include "editbaritem.hpp"
#include "historyform.hpp"
#include "mainwindow.hpp"
#include "userinfoform.hpp"

using namespace Huggle;

EditBar::EditBar(QWidget *parent) : QDockWidget(parent), ui(new Ui::EditBar)
{
    this->ui->setupUi(this);
    connect(&this->timer, SIGNAL(timeout()), this, SLOT(OnReload()));
    this->ui->label_2->setText(_l("user"));
    this->ui->label->setText(_l("page"));
    this->pageSX = 0;
    this->userSX = 0;
}

EditBar::~EditBar()
{
    this->RemoveAll();
    delete this->ui;
}

void EditBar::Refresh()
{
    if (!this->isVisible())
        return;

    this->RemoveAll();
    this->RefreshPage();
    this->RefreshUser();
}

void EditBar::insertEdit(WikiPageHistoryItem *page, int RowId)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    EditBarItem *item = new EditBarItem(this);
    this->items.append(item);
    item->IsUser = false;
    this->ui->horizontalLayout_page->insertWidget(1, item);
    item->RevID = page->RevID;
    if (page->IsCurrent)
        item->SetLineWidth(2);
    item->Username = page->User;
    item->RowId = RowId;
    item->SetPixmap(WikiEdit::GetPixmapFromEditType(page->Type));
    item->SetText(_l("user") + ": " + page->User + "\n" +
                  _l("size") + ": " + page->Size + "\n" +
                  _l("date") + ": " + page->Date + "\n" +
                  _l("summary") + ": " + page->Summary);
}

void EditBar::insertUser(UserInfoFormHistoryItem *user)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    EditBarItem *item = new EditBarItem(this);
    this->items.append(item);
    this->ui->horizontalLayout_user->insertWidget(1, item);
    item->RevID = user->RevID;
    item->Username = user->Name;
    item->IsUser = true;
    item->Page = user->Page;
    item->SetPixmap(WikiEdit::GetPixmapFromEditType(user->Type));
    QString top;
    if (user->Top)
    {
        item->SetFrame(Qt::magenta);
        top = "\n" + _l("edit-bar-top");
    }
    item->SetText(_l("page") + ": " + user->Page + "\n" +
                  _l("date") + ": " + user->Date + "\n" +
                  _l("summary") + ": " + user->Summary + top);
}

void EditBar::RemoveAll()
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    this->clearUser();
    this->clearPage();
    while (this->items.count())
    {
        delete this->items.at(0);
        this->items.removeAt(0);
    }
}

void EditBar::RefreshPage()
{
    if (!this->isVisible())
    {
        this->needsRefresh = true;
        return;
    }
    this->clearPage();
    // we need to fetch all data from history form
    HistoryForm *history = MainWindow::HuggleMain->wHistory;
    // now we need to insert the items upside down
    int RowId = 0;
    foreach (WikiPageHistoryItem *c, history->Items)
    {
        this->insertEdit(c, RowId);
        RowId++;
    }
    // we need to scroll to the edge of list
    this->pageSX = (history->Items.count() * 20);
    this->timer.start(HUGGLE_TIMER);
}

void EditBar::RefreshUser()
{
    if (!this->isVisible())
    {
        this->needsRefresh = true;
        return;
    }
    this->clearUser();
    UserinfoForm *userinfo = MainWindow::HuggleMain->wUserInfo;
    // now we need to insert the items upside down
    foreach (UserInfoFormHistoryItem i, userinfo->Items)
        this->insertUser(&i);
    // we need to scroll to the edge of list
    this->moveUser(20 * userinfo->Items.count());
}

void EditBar::movePage(int size)
{
    this->ui->scrollArea->horizontalScrollBar()->setValue(this->ui->scrollArea->horizontalScrollBar()->value() + size);
}

void EditBar::clearUser()
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    while (this->ui->horizontalLayout_user->count() > 1)
    {
        QLayoutItem *i = this->ui->horizontalLayout_user->itemAt(1);
        this->ui->horizontalLayout_user->removeItem(i);
        delete i;
    }
}

void EditBar::clearPage()
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    while (this->ui->horizontalLayout_page->count() > 1)
    {
        QLayoutItem *i = this->ui->horizontalLayout_page->itemAt(1);
        this->ui->horizontalLayout_page->removeItem(i);
        delete i;
    }
}

void EditBar::moveUser(int size)
{
    this->ui->scrollArea_2->horizontalScrollBar()->setValue(this->ui->scrollArea_2->horizontalScrollBar()->value() + size);
}

void Huggle::EditBar::on_pushButton_2_clicked()
{
    this->movePage(-20);
}

void Huggle::EditBar::on_pushButton_clicked()
{
    this->movePage(20);
}

void Huggle::EditBar::on_pushButton_4_clicked()
{
    this->moveUser(-20);
}

void Huggle::EditBar::on_pushButton_3_clicked()
{
    this->moveUser(20);
}

void EditBar::OnReload()
{
    if (this->userSX != 0)
        this->moveUser(this->userSX);
    if (this->pageSX != 0)
        this->movePage(this->pageSX);

    this->userSX = 0;
    this->pageSX = 0;
    this->timer.stop();
}

void Huggle::EditBar::on_EditBar_visibilityChanged(bool visible)
{
    if (!this->needsRefresh)
        return;

    if (!visible)
        return;

    this->needsRefresh = false;
    this->RefreshPage();
    this->RefreshUser();
}
