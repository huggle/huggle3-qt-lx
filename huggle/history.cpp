//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "history.hpp"
#include <QMenu>
#include "localization.hpp"
#include "syslog.hpp"
#include "exception.hpp"
#include "ui_history.h"

using namespace Huggle;

int History::Last = 0;
History::History(QWidget *parent) : QDockWidget(parent), ui(new Ui::History)
{
    this->ui->setupUi(this);
    this->setWindowTitle(Localizations::HuggleLocalizations->Localize("userhistory-title"));
    this->ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this->ui->tableWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ContextMenu(QPoint)));
    this->ui->tableWidget->setColumnCount(4);
    QStringList header;
    header << Localizations::HuggleLocalizations->Localize("[[id]]") <<
              Localizations::HuggleLocalizations->Localize("[[type]]") <<
              Localizations::HuggleLocalizations->Localize("[[target]]") <<
              Localizations::HuggleLocalizations->Localize("result");
    this->ui->tableWidget->setHorizontalHeaderLabels(header);
    this->ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    this->ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
#if QT_VERSION >= 0x050000
// Qt5 code
    this->ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
// Qt4 code
    this->ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    this->ui->tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->ui->tableWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->ui->tableWidget->setShowGrid(false);
}

void History::Prepend(HistoryItem item)
{
    this->Items.insert(0, item);
    this->ui->tableWidget->insertRow(0);
    this->ui->tableWidget->setItem(0, 0, new QTableWidgetItem(QString::number(item.ID)));
    this->ui->tableWidget->setItem(0, 1, new QTableWidgetItem(HistoryItem::TypeToString(item.Type)));
    this->ui->tableWidget->setItem(0, 2, new QTableWidgetItem(item.Target));
    this->ui->tableWidget->setItem(0, 3, new QTableWidgetItem(item.Result));
    this->ui->tableWidget->resizeRowToContents(0);
}

History::~History()
{
    delete this->ui;
}

void History::Undo(HistoryItem *hist)
{
    if (hist == nullptr)
    {
        // we need to get a currently selected item
        if (this->CurrentItem < 0)
            return;

        hist = new HistoryItem(this->Items.at(this->CurrentItem));
    }

    delete hist;
}

void History::ContextMenu(const QPoint &position)
{
    QPoint g_ = this->ui->tableWidget->mapToGlobal(position);
    QMenu menu;
    menu.addAction("Undo");
    QAction *selection = menu.exec(g_);
    if (selection)
    {
        this->Undo(nullptr);
    }
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

HistoryItem::HistoryItem()
{
    History::Last++;
    this->Target = "Unknown target";
    this->UndoRevBaseTime = "";
    this->Type = HistoryUnknown;
    this->ID = History::Last;
    this->Result = "Unknown??";
}

HistoryItem::HistoryItem(const HistoryItem &item)
{
    this->ID = item.ID;
    this->Target = item.Target;
    this->UndoRevBaseTime = item.UndoRevBaseTime;
    this->Type = item.Type;
    this->UndoDependency = item.UndoDependency;
    this->Result = item.Result;
}

HistoryItem::HistoryItem(HistoryItem *item)
{
    if (item == nullptr)
    {
        throw new Exception("HistoryItem item must not be NULL", "HistoryItem::HistoryItem(HistoryItem *item)");
    }
    this->ID = item->ID;
    this->Type = item->Type;
    this->UndoRevBaseTime = item->UndoRevBaseTime;
    this->Target = item->Target;
    this->UndoDependency = item->UndoDependency;
    this->Result = item->Result;
}

void Huggle::History::on_tableWidget_clicked(const QModelIndex &index)
{
    this->CurrentItem = index.row();
}
