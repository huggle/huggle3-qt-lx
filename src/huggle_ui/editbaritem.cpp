//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "editbaritem.hpp"
#include <huggle_core/exception.hpp>
#include <huggle_core/localization.hpp>
#include "ui_editbaritem.h"
#include "userinfoform.hpp"
#include <QMouseEvent>
#include "historyform.hpp"
#include "mainwindow.hpp"

using namespace Huggle;

EditBarItem::EditBarItem(QWidget *parent) : QFrame(parent), ui(new Ui::EditBarItem)
{
    this->ui->setupUi(this);
}

EditBarItem::~EditBarItem()
{
    delete this->ui;
}

void EditBarItem::SetLineWidth(int width)
{
    this->setLineWidth(width);
}

void EditBarItem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (this->IsUser)
        {
            Huggle::MainWindow::HuggleMain->wUserInfo->JumpToSpecificContrib(this->RevID.toLong(), this->Page);
        } else
        {
            if (this->RowId < 0)
                MainWindow::HuggleMain->wHistory->GetEdit(this->RevID.toLong(), QString("prev"), this->Username, _l("wait"));
            else
               MainWindow::HuggleMain->wHistory->GetEdit(this->RevID.toLong(), QString("prev"), this->RowId, _l("wait"));
        }
    }
}

void EditBarItem::SetText(const QString& text)
{
    this->ui->label->setToolTip(text);
}

void EditBarItem::SetFrame(Qt::GlobalColor colour)
{
    //this->setStyleSheet("QFrame { border-color: " + color + " }");
    QPalette px;
#ifdef QT6_BUILD
    px.setColor(QPalette::WindowText, colour);
#else
    px.setColor(QPalette::Foreground, colour);
#endif
    this->setPalette(px);
}

void EditBarItem::SetPixmap(const QString& path)
{
    this->ui->label->setPixmap(QPixmap(path));
}
