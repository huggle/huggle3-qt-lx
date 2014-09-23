//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "editbaritem.hpp"
#include "exception.hpp"
#include "historyform.hpp"
#include "mainwindow.hpp"
#include "localization.hpp"
#include "ui_editbaritem.h"
#include <QMouseEvent>

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
        if (this->RowId < 0)
            MainWindow::HuggleMain->wHistory->GetEdit(this->RevID.toInt(), QString("prev"), this->Username, _l("wait"));
        else
            MainWindow::HuggleMain->wHistory->GetEdit(this->RevID.toInt(), QString("prev"), this->RowId, _l("wait"));
    }
}

void EditBarItem::SetText(QString text)
{
    this->ui->label->setToolTip(text);
}

void EditBarItem::SetPixmap(QString path)
{
    this->ui->label->setPixmap(QPixmap(path));
}
