//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HISTORY_H
#define HISTORY_H

#include "definitions.hpp"
// now we need to ensure that python is included first
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QString>
#include <QTimer>
#include <QList>
#include <QDockWidget>
#include "apiquery.hpp"
#include "editquery.hpp"
#include "revertquery.hpp"

namespace Ui
{
    class History;
}

namespace Huggle
{
    class ApiQuery;
    class EditQuery;
    class RevertQuery;
    //! Types of history items
    enum HistoryType
    {
        HistoryUnknown,
        HistoryEdit,
        HistoryRollback,
        HistoryMessage
    };

    //! History consist of these items
    class HistoryItem
    {
        public:
            static QString TypeToString(HistoryType type);

            HistoryItem();
            HistoryItem(const HistoryItem &item);
            HistoryItem(HistoryItem * item);
            //! Unique ID of this item
            int ID;
            HistoryItem *UndoDependency = nullptr;
            QString UndoRevBaseTime;
            QString Result;
            QString Target;
            //! Change this to false in case that item can't be reverted
            bool IsRevertable = true;
            bool NewPage = false;
            //! Type of item
            HistoryType Type;
            bool Undone = false;
            HistoryItem *ReferencedBy = nullptr;
    };

    /// \todo It should be possible to go back in history to review what you have you done
    /// currently nothing happens when you click on history items
    /// \todo Function to revert your own changes
    /// \todo Option to remove the items / trim them etc so that operating memory is not cluttered by these

    //! History of actions done by user

    //! This is a widget that displays the user history
    class History : public QDockWidget
    {
            Q_OBJECT
        public:
            explicit History(QWidget *parent = 0);
            ~History();
            void Undo(HistoryItem *hist);
            //! Insert a new item to top of list
            void Prepend(HistoryItem item);
            QList<HistoryItem*> Items;
            static int Last;

        private slots:
            void ContextMenu(const QPoint& position);
            void Tick();
            void on_tableWidget_clicked(const QModelIndex &index);

        private:
            QTimer *timerRetrievePageInformation;
            HistoryItem *RevertingItem = nullptr;
            //! This is a query we need to use to retrieve our own edit before we undo it
            ApiQuery *qEdit = nullptr;
            //! This is for welcome message that is used to replace a talk page
            EditQuery *qTalk = nullptr;
            //! Used to revert edits we made
            RevertQuery *qSelf = nullptr;
            int CurrentItem = -200;
            Ui::History *ui;
    };
}

#endif // HISTORY_H
