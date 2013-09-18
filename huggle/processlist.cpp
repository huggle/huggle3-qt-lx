//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "processlist.h"
#include "ui_processlist.h"

ProcessList::ProcessList(QWidget *parent) : QDockWidget(parent), ui(new Ui::ProcessList)
{
    ui->setupUi(this);
    ui->tableWidget->setColumnCount(4);
    QStringList header;
    header << "ID" << "Type" << "Target" << "Status" << "Result";
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setShowGrid(false);
}

void ProcessList::InsertQuery(Query *q)
{
    if (q == NULL)
    {
        throw new Exception("NULL query");
    }
    int size = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(size);
    ui->tableWidget->setItem(size, 0, new QTableWidgetItem(QString::number(q->ID)));
    ui->tableWidget->setItem(size, 1, new QTableWidgetItem(q->QueryTypeToString()));
    ui->tableWidget->setItem(size, 2, new QTableWidgetItem(q->QueryTargetToString()));
    ui->tableWidget->setItem(size, 3, new QTableWidgetItem(q->QueryStatusToString()));
}

void ProcessList::Clear()
{
    ui->tableWidget->clear();
}

bool ProcessList::ContainsQuery(Query *q)
{
    return GetItem(q) != -1;
}

void ProcessList::RemoveQuery(Query *q)
{

}

void ProcessList::UpdateQuery(Query *q)
{
    int query = GetItem(q);
    if (query == -1)
    {
        this->InsertQuery(q);
        return;
    }
}

int ProcessList::GetItem(Query *q)
{
    int curr = 0;
    int size = ui->tableWidget->rowCount();
    while (curr < size)
    {
        if (ui->tableWidget->item(curr,0)->text() == QString::number(q->ID))
        {
            return curr;
        }
        curr++;
    }
    return -1;
}

ProcessList::~ProcessList()
{
    delete ui;
}
