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
#include "ui_editbar.h"
#include <QScrollBar>

using namespace Huggle;

EditBar::EditBar(QWidget *parent) : QDockWidget(parent), ui(new Ui::EditBar)
{
    this->ui->setupUi(this);
    connect(&this->timer, SIGNAL(timeout()), this, SLOT(OnReload()));
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

void EditBar::InsertEdit(WikiPageHistoryItem *page)
{
    EditBarItem *item = new EditBarItem();
    this->Items.append(item);
    this->ui->horizontalLayout_page->insertWidget(1, item);
    item->RevID = page->RevID;
    item->SetPixmap(WikiEdit::GetPixmapFromEditType(page->Type));
    item->SetText("User: " + page->User + "\n" +
                  "Size: " + page->Size + "\n" +
                  "Date: " + page->Date + "\n" +
                  "Summary: " + page->Summary);
}

void EditBar::InsertUser(UserInfoFormHistoryItem *user)
{
    EditBarItem *item = new EditBarItem();
    this->Items.append(item);
    this->ui->horizontalLayout_user->insertWidget(1, item);
    item->RevID = user->RevID;
    item->SetPixmap(WikiEdit::GetPixmapFromEditType(user->Type));
    QString top;
    if (user->Top)
        top = "\nThis edit is a top revision";
    item->SetText("Page: " + user->Page + "\n" +
                  "Date: " + user->Date + "\n" +
                  "Summary: " + user->Summary + top);
}

void EditBar::RemoveAll()
{
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
    this->ClearPage();
    // we need to fetch all data from history form
    HistoryForm *history = MainWindow::HuggleMain->wHistory;
    // now we need to insert the items upside down
    foreach (WikiPageHistoryItem c, history->Items)
        this->InsertEdit(&c);
    // we need to scroll to the edge of list
    this->PageSX = (history->Items.count() * 20);
    this->timer.start(1);
}

void EditBar::RefreshUser()
{
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
    while (this->ui->horizontalLayout_user->count() > 1)
    {
        QLayoutItem *i = this->ui->horizontalLayout_user->itemAt(1);
        this->ui->horizontalLayout_user->removeItem(i);
    }
}

void EditBar::ClearPage()
{
    while (this->ui->horizontalLayout_page->count() > 1)
    {
        QLayoutItem *i = this->ui->horizontalLayout_page->itemAt(1);
        this->ui->horizontalLayout_page->removeItem(i);
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
