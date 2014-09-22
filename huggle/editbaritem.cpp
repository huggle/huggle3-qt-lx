//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "editbaritem.hpp"
#include "ui_editbaritem.h"

using namespace Huggle;

EditBarItem::EditBarItem(QWidget *parent) : QFrame(parent), ui(new Ui::EditBarItem)
{
    this->ui->setupUi(this);
}

EditBarItem::~EditBarItem()
{
    delete this->ui;
}

void EditBarItem::SetText(QString text)
{
    this->ui->label->setToolTip(text);
}

void EditBarItem::SetPixmap(QString path)
{
    this->ui->label->setPixmap(QPixmap(path));
}
