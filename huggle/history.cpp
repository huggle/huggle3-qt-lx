//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "history.h"
#include "ui_history.h"

History::History(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::History)
{
    ui->setupUi(this);
    ui->tableWidget->setColumnCount(4);
    QStringList header;
    header << "ID" << "Type" << "Target" << "Result";
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
#if QT_VERSION >= 0x050000
// Qt5 code
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
// Qt4 code
    ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    //ui->tableWidget->horizontalHeaderItem(0)->setSizeHint(QSize(20,-1));
    ui->tableWidget->setShowGrid(false);
}

void History::Prepend(HistoryItem item)
{
    this->Items.insert(0, item);
    ui->tableWidget->insertRow(0);
    ui->tableWidget->setItem(0, 0, new QTableWidgetItem(QString::number(item.ID)));
    ui->tableWidget->setItem(0, 1, new QTableWidgetItem(HistoryItem::TypeToString(item.Type)));
    ui->tableWidget->setItem(0, 2, new QTableWidgetItem(item.Target));
    ui->tableWidget->setItem(0, 3, new QTableWidgetItem(item.Result));
}

void History::Refresh()
{

}

void History::Remove(HistoryItem item)
{

}

History::~History()
{
    delete ui;
}

QString HistoryItem::TypeToString(HistoryType type)
{
    switch (type)
    {
    case HistoryUnknown:
        return "Unknown";
    case HistoryMessage:
        return "Message";
    case HistoryEdit:
        return "Edit";
    case HistoryRollback:
        return "Rollback";
    }
    return "Unknown";
}


int History::Last = 0;

HistoryItem::HistoryItem()
{
    History::Last++;
    this->Target = "Unknown target";
    this->Type = HistoryUnknown;
    this->ID = History::Last;
    this->Result = "Unknown??";
}

HistoryItem::HistoryItem(const HistoryItem &item)
{
    this->ID = item.ID;
    this->Target = item.Target;
    this->Type = item.Type;
    this->Result = item.Result;
}

HistoryItem::HistoryItem(HistoryItem *item)
{
    this->ID = item->ID;
    this->Type = item->Type;
    this->Target = item->Target;
    this->Result = item->Result;
}
