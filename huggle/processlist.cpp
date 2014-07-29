//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "exception.hpp"
#include "core.hpp"
#include "configuration.hpp"
#include "localization.hpp"
#include "processlist.hpp"
#include "ui_processlist.h"

using namespace Huggle;

ProcessList::ProcessList(QWidget *parent) : QDockWidget(parent), ui(new Ui::ProcessList)
{
    this->ui->setupUi(this);
    this->ui->tableWidget->setColumnCount(5);
    QStringList header;
    header << _l("id") << _l("type") << _l("target") << _l("status");
    this->ui->tableWidget->setHorizontalHeaderLabels(header);
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    this->ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    if (Configuration::HuggleConfiguration->SystemConfig_DynamicColsInList)
    {
#if QT_VERSION >= 0x050000
// Qt5 code
        this->ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
// Qt4 code
        this->ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    } else
    {
        this->ui->tableWidget->setColumnWidth(0, 60);
        this->ui->tableWidget->setColumnWidth(1, 200);
        this->ui->tableWidget->setColumnWidth(2, 200);
        this->ui->tableWidget->setColumnWidth(3, 80);
    }
    this->ui->tableWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->ui->tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->ui->tableWidget->setShowGrid(false);
    this->Removed = new QList<ProcessListRemovedItem*> ();
}

void ProcessList::InsertQuery(Collectable_SmartPtr<Query> query)
{
    if (query == nullptr)
    {
        throw new Huggle::Exception("NULL query");
    }
    int size = this->ui->tableWidget->rowCount();
    this->ui->tableWidget->insertRow(size);
    this->ui->tableWidget->setItem(size, 0, new QTableWidgetItem(QString::number(query->QueryID())));
    this->ui->tableWidget->setItem(size, 1, new QTableWidgetItem(query->QueryTypeToString()));
    this->ui->tableWidget->setItem(size, 2, new QTableWidgetItem(query->QueryTargetToString()));
    this->ui->tableWidget->setItem(size, 3, new QTableWidgetItem(query->QueryStatusToString()));
    this->ui->tableWidget->resizeRowToContents(size);
}

void ProcessList::Clear()
{
    delete this->Removed;
    this->ui->tableWidget->clear();
}

bool ProcessList::ContainsQuery(Query *query)
{
    int result = GetItem(query);
    return result != -1;
}

void ProcessList::RemoveQuery(Query *query)
{
    if (!IsExpired(query))
    {
        this->Removed->append(new ProcessListRemovedItem(query->QueryID()));
    }
}

void ProcessList::UpdateQuery(Query *query)
{
    int query_ = GetItem(query);
    if (query_ == -1)
    {
        this->InsertQuery(query);
        return;
    }

    this->ui->tableWidget->setItem(query_, 0, new QTableWidgetItem(QString::number(query->QueryID())));
    this->ui->tableWidget->setItem(query_, 1, new QTableWidgetItem(query->QueryTypeToString()));
    this->ui->tableWidget->setItem(query_, 2, new QTableWidgetItem(query->QueryTargetToString()));
    this->ui->tableWidget->setItem(query_, 3, new QTableWidgetItem(query->QueryStatusToString()));
    this->ui->tableWidget->resizeRowToContents(query_);
}

bool ProcessList::IsExpired(Query *q)
{
    int i = 0;
    while (i < this->Removed->count())
    {
        if ((unsigned int) this->Removed->at(i)->GetID() == q->QueryID())
        {
            return true;
        }
        i++;
    }
    return false;
}

void ProcessList::RemoveExpired()
{
    if (this->Removed->count() == 0)
    {
        return;
    }
    QList<ProcessListRemovedItem*> rm;
    int i = 0;
    while (i < this->Removed->count())
    {
        if (this->Removed->at(i)->Expired())
        {
            rm.append(this->Removed->at(i));
        }
        i++;
    }
    i = 0;
    while (i<rm.count())
    {
        ProcessListRemovedItem *item = rm.at(i);
        this->Removed->removeOne(item);
        int row = this->GetItem(item->GetID());
        if (row != -1)
        {
            this->ui->tableWidget->removeRow(row);
        }
        delete item;
        i++;
    }
}

int ProcessList::GetItem(Query *q)
{
    int curr = 0;
    int size = this->ui->tableWidget->rowCount();
    while (curr < size)
    {
        if (this->ui->tableWidget->item(curr,0)->text() == QString::number(q->QueryID()))
        {
            return curr;
        }
        curr++;
    }
    return -1;
}

int ProcessList::GetItem(int Id)
{
    int curr = 0;
    int size = this->ui->tableWidget->rowCount();
    while (curr < size)
    {
        if (this->ui->tableWidget->item(curr,0)->text() == QString::number(Id))
        {
            return curr;
        }
        curr++;
    }
    return -1;
}

ProcessList::~ProcessList()
{
    delete this->ui;
}

ProcessListRemovedItem::ProcessListRemovedItem(int ID)
{
    this->id = ID;
    this->time = QDateTime::currentDateTime();
}

int ProcessListRemovedItem::GetID()
{
    return this->id;
}

bool ProcessListRemovedItem::Expired()
{
    return this->time < QDateTime::currentDateTime().addSecs(-Configuration::HuggleConfiguration->SystemConfig_QueryListTimeLimit);
}

