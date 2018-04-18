//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "editbar.hpp"
#include "configuration.hpp"
#include "editbaritem.hpp"
#include "exception.hpp"
#include "historyform.hpp"
#include "localization.hpp"
#include "mainwindow.hpp"
#include "userinfoform.hpp"
#include "syslog.hpp"
#include "wikipage.hpp"
#include "wikiuser.hpp"
#include "wikiedit.hpp"
#include "huggleprofiler.hpp"
#include "ui_editbar.h"
#include <QModelIndex>
#include <QScrollBar>

using namespace Huggle;

EditBar::EditBar(QWidget *parent) : QDockWidget(parent), ui(new Ui::EditBar)
{
    this->ui->setupUi(this);
    connect(&this->timer, SIGNAL(timeout()), this, SLOT(OnReload()));
    this->ui->label_2->setText(_l("user"));
    this->ui->label->setText(_l("page"));
    this->PageSX = 0;
    this->UserSX = 0;
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

void EditBar::InsertEdit(WikiPageHistoryItem *page, int RowId)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    EditBarItem *item = new EditBarItem(this);
    this->Items.append(item);
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

void EditBar::InsertUser(UserInfoFormHistoryItem *user)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    EditBarItem *item = new EditBarItem(this);
    this->Items.append(item);
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
    this->ClearUser();
    this->ClearPage();
    while (this->Items.count())
    {
        delete this->Items.at(0);
        this->Items.removeAt(0);
    }
}

void EditBar::RefreshPage()
{
    if (!this->isVisible())
    {
        this->needsRefresh = true;
        return;
    }
    this->ClearPage();
    // we need to fetch all data from history form
    HistoryForm *history = MainWindow::HuggleMain->wHistory;
    // now we need to insert the items upside down
    int RowId = 0;
    foreach (WikiPageHistoryItem *c, history->Items)
    {
        this->InsertEdit(c, RowId);
        RowId++;
    }
    // we need to scroll to the edge of list
    this->PageSX = (history->Items.count() * 20);
    this->timer.start(HUGGLE_TIMER);
}

void EditBar::RefreshUser()
{
    if (!this->isVisible())
    {
        this->needsRefresh = true;
        return;
    }
    this->ClearUser();
    UserinfoForm *userinfo = MainWindow::HuggleMain->wUserInfo;
    // now we need to insert the items upside down
    foreach (UserInfoFormHistoryItem i, userinfo->Items)
        this->InsertUser(&i);
    // we need to scroll to the edge of list
    this->MoveUser(20 * userinfo->Items.count());
}

void EditBar::MovePage(int size)
{
    this->ui->scrollArea->horizontalScrollBar()->setValue(this->ui->scrollArea->horizontalScrollBar()->value() + size);
}

void EditBar::ClearUser()
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    while (this->ui->horizontalLayout_user->count() > 1)
    {
        QLayoutItem *i = this->ui->horizontalLayout_user->itemAt(1);
        this->ui->horizontalLayout_user->removeItem(i);
        delete i;
    }
}

void EditBar::ClearPage()
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    while (this->ui->horizontalLayout_page->count() > 1)
    {
        QLayoutItem *i = this->ui->horizontalLayout_page->itemAt(1);
        this->ui->horizontalLayout_page->removeItem(i);
        delete i;
    }
}

void EditBar::MoveUser(int size)
{
    this->ui->scrollArea_2->horizontalScrollBar()->setValue(this->ui->scrollArea_2->horizontalScrollBar()->value() + size);
}

void Huggle::EditBar::on_pushButton_2_clicked()
{
    this->MovePage(-20);
}

void Huggle::EditBar::on_pushButton_clicked()
{
    this->MovePage(20);
}

void Huggle::EditBar::on_pushButton_4_clicked()
{
    this->MoveUser(-20);
}

void Huggle::EditBar::on_pushButton_3_clicked()
{
    this->MoveUser(20);
}

void EditBar::OnReload()
{
    if (this->UserSX != 0)
        this->MoveUser(this->UserSX);
    if (this->PageSX != 0)
        this->MovePage(this->PageSX);

    this->UserSX = 0;
    this->PageSX = 0;
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
