//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "processlist.hpp"
#include "ui_processlist.h"

using namespace Huggle;

ProcessList::ProcessList(QWidget *parent) : QDockWidget(parent), ui(new Ui::ProcessList)
{
    this->ui->setupUi(this);
    this->ui->tableWidget->setColumnCount(4);
    QStringList header;
    header << Huggle::Localizations::HuggleLocalizations->Localize("id")
           << Huggle::Localizations::HuggleLocalizations->Localize("type")
           << Huggle::Localizations::HuggleLocalizations->Localize("target")
           << Huggle::Localizations::HuggleLocalizations->Localize("status")
           << Huggle::Localizations::HuggleLocalizations->Localize("result");
    this->ui->tableWidget->setHorizontalHeaderLabels(header);
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    this->ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
#if QT_VERSION >= 0x050000
// Qt5 code
    this->ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
// Qt4 code
    this->ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    //ui->tableWidget->horizontalHeaderItem(0)->setSizeHint(QSize(20,-1));
    this->ui->tableWidget->setShowGrid(false);
    this->Removed = new QList<ProcessListRemovedItem*> ();
}

void ProcessList::InsertQuery(Query *q)
{
    if (q == NULL)
    {
        throw new Exception("NULL query");
    }
    q->RegisterConsumer(HUGGLECONSUMER_PROCESSLIST);
    int size = this->ui->tableWidget->rowCount();
    this->ui->tableWidget->insertRow(size);
    this->ui->tableWidget->setItem(size, 0, new QTableWidgetItem(QString::number(q->QueryID())));
    this->ui->tableWidget->setItem(size, 1, new QTableWidgetItem(q->QueryTypeToString()));
    this->ui->tableWidget->setItem(size, 2, new QTableWidgetItem(q->QueryTargetToString()));
    this->ui->tableWidget->setItem(size, 3, new QTableWidgetItem(q->QueryStatusToString()));
    this->ui->tableWidget->resizeRowToContents(size);
    q->UnregisterConsumer(HUGGLECONSUMER_PROCESSLIST);
}

void ProcessList::Clear()
{
    delete this->Removed;
    this->ui->tableWidget->clear();
}

bool ProcessList::ContainsQuery(Query *q)
{
    q->RegisterConsumer(HUGGLECONSUMER_PROCESSLIST);
    int result = GetItem(q);
    q->UnregisterConsumer(HUGGLECONSUMER_PROCESSLIST);
    return result != -1;
}

void ProcessList::RemoveQuery(Query *q)
{
    q->RegisterConsumer(HUGGLECONSUMER_PROCESSLIST);
    if (!IsExpired(q))
    {
        this->Removed->append(new ProcessListRemovedItem(q->QueryID()));
    }
    q->UnregisterConsumer(HUGGLECONSUMER_PROCESSLIST);
}

void ProcessList::UpdateQuery(Query *q)
{
    q->RegisterConsumer(HUGGLECONSUMER_PROCESSLIST);
    int query = GetItem(q);
    if (query == -1)
    {
        this->InsertQuery(q);
        return;
    }

    this->ui->tableWidget->setItem(query, 0, new QTableWidgetItem(QString::number(q->QueryID())));
    this->ui->tableWidget->setItem(query, 1, new QTableWidgetItem(q->QueryTypeToString()));
    this->ui->tableWidget->setItem(query, 2, new QTableWidgetItem(q->QueryTargetToString()));
    this->ui->tableWidget->setItem(query, 3, new QTableWidgetItem(q->QueryStatusToString()));
    this->ui->tableWidget->resizeRowToContents(query);
    q->UnregisterConsumer(HUGGLECONSUMER_PROCESSLIST);
}

bool ProcessList::IsExpired(Query *q)
{
    q->RegisterConsumer(HUGGLECONSUMER_PROCESSLIST);
    int i = 0;
    while (i < this->Removed->count())
    {
        if ((unsigned int) this->Removed->at(i)->GetID() == q->QueryID())
        {
            q->UnregisterConsumer(HUGGLECONSUMER_PROCESSLIST);
            return true;
        }
        i++;
    }
    q->UnregisterConsumer(HUGGLECONSUMER_PROCESSLIST);
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
    q->RegisterConsumer(HUGGLECONSUMER_PROCESSLIST);
    int curr = 0;
    int size = this->ui->tableWidget->rowCount();
    while (curr < size)
    {
        if (this->ui->tableWidget->item(curr,0)->text() == QString::number(q->QueryID()))
        {
            q->UnregisterConsumer(HUGGLECONSUMER_PROCESSLIST);
            return curr;
        }
        curr++;
    }
    q->UnregisterConsumer(HUGGLECONSUMER_PROCESSLIST);
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
    return this->time < QDateTime::currentDateTime().addSecs(-Configuration::HuggleConfiguration->QueryListTimeLimit);
}

