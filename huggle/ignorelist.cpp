//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "ignorelist.h"
#include "ui_ignorelist.h"

IgnoreList::IgnoreList(QWidget *parent) : QDialog(parent),
    ui(new Ui::IgnoreList)
{
    ui->setupUi(this);
    model = new QStandardItemModel(Configuration::LocalConfig_Ignores.count(), 1, this);
    ui->listView->setModel(model);
    int it = 0;
    while (it < Configuration::LocalConfig_Ignores.count())
    {
        model->setItem(it, new QStandardItem(Configuration::LocalConfig_Ignores.at(it)));
        it++;
    }
}

IgnoreList::~IgnoreList()
{
    delete model;
    delete ui;
}

void IgnoreList::on_pushButton_clicked()
{
    this->close();
}
