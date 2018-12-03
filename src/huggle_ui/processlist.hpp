//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef PROCESSLIST_H
#define PROCESSLIST_H

#include <huggle_core/definitions.hpp>

#include <QList>
#include <QTableWidgetItem>
#include <QDateTime>
#include <QHeaderView>
#include <QDockWidget>
#include <huggle_core/collectable_smartptr.hpp>
#include <huggle_core/query.hpp>

namespace Ui
{
    class ProcessList;
}

namespace Huggle
{
    //! Removed item that was in the process list

    //! When you remove an item it should stay in list for some time so that user can notice it finished and that's
    //! why we store it to separate object and for that we have this class ;)
    class ProcessListRemovedItem
    {
        private:
            QDateTime time;
            unsigned int id;
        public:
            ProcessListRemovedItem(unsigned int ID);
            unsigned int GetID();
            bool Expired(bool Debug);
    };

    //! List of processes in a main window

    //! List of active processes, when some process finish it's collected by garbage collector
    //! this is only a dialog that you see in huggle form, it doesn't contain process list
    class ProcessList : public QDockWidget
    {
            Q_OBJECT
        public:
            explicit ProcessList(QWidget *parent = nullptr);
            //! Insert a query to process list, the query is automatically removed once it's done
            void InsertQuery(Collectable_SmartPtr<Query> query);
            //! Remove all entries in process list
            void Clear();
            //! Return true if there is already this in a list
            bool ContainsQuery(Query *query);
            //! Remove a query from list no matter if it finished or not
            void RemoveQuery(Query *query);
            //! Update information about query in list
            void UpdateQuery(Query *query);
            void RemoveExpired();
            ~ProcessList();
        private slots:
            void ContextMenu(const QPoint& position);
            void OnQueryPoolRemove(Query *q);
            void OnQueryPoolUpdate(Query *q);
        private:
            int GetItem(Query *q);
            int GetItem(unsigned int id);
            bool IsExpired(Query *q);
            bool IsDebuged;
            QList<ProcessListRemovedItem*> *Removed;
            Ui::ProcessList *ui;
    };
}

#endif // PROCESSLIST_H
