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
#include "huggleprofiler.hpp"
#include "processlist.hpp"
#include "ui_processlist.h"
#include <QMenu>
#include <QClipboard>

using namespace Huggle;

ProcessList::ProcessList(QWidget *parent) : QDockWidget(parent), ui(new Ui::ProcessList)
{
    this->ui->setupUi(this);
    QStringList header;
    this->IsDebuged = hcfg->Verbosity > 0;
    header << _l("id") << _l("type") << _l("target") << _l("status");
    if (this->IsDebuged)
    {
        this->ui->tableWidget->setColumnCount(6);
        header << "Url" << "Debug info";
    } else
    {
        this->ui->tableWidget->setColumnCount(4);
    }
    this->ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    this->ui->tableWidget->setHorizontalHeaderLabels(header);
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    this->ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(this->ui->tableWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ContextMenu(QPoint)));
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
        if (this->IsDebuged)
        {
            this->ui->tableWidget->setColumnWidth(4, 800);
        }
    }
    this->ui->tableWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->ui->tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->ui->tableWidget->setShowGrid(false);
    this->Removed = new QList<ProcessListRemovedItem*>();
}

ProcessList::~ProcessList()
{
    delete this->ui;
}

void ProcessList::InsertQuery(Collectable_SmartPtr<Query> query)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    if (query == nullptr)
    {
        throw new Huggle::NullPointerException("Collectable_SmartPtr<Query> query", BOOST_CURRENT_FUNCTION);
    }
    int size = this->ui->tableWidget->rowCount();
    this->ui->tableWidget->insertRow(size);
    this->ui->tableWidget->setItem(size, 0, new QTableWidgetItem(QString::number(query->QueryID())));
    this->ui->tableWidget->setItem(size, 1, new QTableWidgetItem(query->QueryTypeToString()));
    this->ui->tableWidget->setItem(size, 2, new QTableWidgetItem(query->QueryTargetToString()));
    this->ui->tableWidget->setItem(size, 3, new QTableWidgetItem(query->QueryStatusToString()));
    if (this->IsDebuged)
    {
        this->ui->tableWidget->setItem(size, 4, new QTableWidgetItem(query->DebugURL()));
        this->ui->tableWidget->setItem(size, 5, new QTableWidgetItem(query->GetFailureReason()));
    }
    this->ui->tableWidget->resizeRowToContents(size);
}

void ProcessList::Clear()
{
    delete this->Removed;
    this->ui->tableWidget->clear();
}

bool ProcessList::ContainsQuery(Query *query)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    int result = GetItem(query);
    return result != -1;
}

void ProcessList::RemoveQuery(Query *query)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    if (!IsExpired(query))
    {
        this->Removed->append(new ProcessListRemovedItem(query->QueryID()));
    }
}

void ProcessList::UpdateQuery(Query *query)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
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
    if (this->IsDebuged)
    {
        this->ui->tableWidget->setItem(query_, 4, new QTableWidgetItem(query->DebugURL()));
        this->ui->tableWidget->setItem(query_, 5, new QTableWidgetItem(query->GetFailureReason()));
    }
    this->ui->tableWidget->resizeRowToContents(query_);
}

bool ProcessList::IsExpired(Query *q)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    foreach(ProcessListRemovedItem *item, *this->Removed)
        if ((unsigned int) item->GetID() == q->QueryID())
            return true;

    return false;
}

void ProcessList::RemoveExpired()
{
    if (this->Removed->count() == 0)
        return;

    QList<ProcessListRemovedItem*> rm;
    foreach (ProcessListRemovedItem *item, *this->Removed)
        if (item->Expired(this->IsDebuged))
            rm.append(item);

    foreach (ProcessListRemovedItem *item, rm)
    {
        this->Removed->removeOne(item);
        int row = this->GetItem(item->GetID());
        if (row != -1)
            this->ui->tableWidget->removeRow(row);

        delete item;
    }
}

int ProcessList::GetItem(Query *q)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
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
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
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

void ProcessList::ContextMenu(const QPoint &position)
{
    QPoint g_ = this->ui->tableWidget->mapToGlobal(position);
    QMenu menu;
    QAction *copy = new QAction(_l("copy"), &menu);
    menu.addAction(copy);
    QAction *selection = menu.exec(g_);
    if (selection == copy)
    {
        QString t = "";
        foreach (QTableWidgetItem *text, this->ui->tableWidget->selectedItems())
            t += text->text() + "\n";
        if (!t.isEmpty())
            QApplication::clipboard()->setText(t);
    }
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

bool ProcessListRemovedItem::Expired(bool Debug)
{
    if (Debug)
        return this->time < QDateTime::currentDateTime().addSecs(-120);
    return this->time < QDateTime::currentDateTime().addSecs(-hcfg->SystemConfig_QueryListTimeLimit);
}

