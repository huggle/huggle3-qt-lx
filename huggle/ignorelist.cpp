//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "ignorelist.hpp"
#include "configuration.hpp"
#include "ui_ignorelist.h"

using namespace Huggle;

IgnoreList::IgnoreList(QWidget *parent) : HW("ignorelist", this, parent), ui(new Ui::IgnoreList)
{
    this->ui->setupUi(this);
    this->model = new QStandardItemModel(Configuration::HuggleConfiguration->ProjectConfig->Ignores.count(), 1, this);
    this->ui->listView->setModel(model);
    int it = 0;
    while (it < Configuration::HuggleConfiguration->ProjectConfig->Ignores.count())
    {
        this->model->setItem(it, new QStandardItem(Configuration::HuggleConfiguration->ProjectConfig->Ignores.at(it)));
        it++;
    }
    it = 0;
    while (it < Configuration::HuggleConfiguration->ProjectConfig->IgnorePatterns.count())
    {
        this->model->setItem(it, new QStandardItem("^.*" + Configuration::HuggleConfiguration->ProjectConfig->IgnorePatterns.at(it) + ".*$"));
        it++;
    }
    this->RestoreWindow();
}

IgnoreList::~IgnoreList()
{
    delete this->model;
    delete this->ui;
}

void IgnoreList::on_pushButton_clicked()
{
    this->close();
}
