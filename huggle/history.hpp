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
// now we need to ensure that python is included first, because it
// simply suck :P
// seriously, Python.h is shitty enough that it requires to be
// included first. Don't believe it? See this:
// http://stackoverflow.com/questions/20300201/why-python-h-of-python-3-2-must-be-included-as-first-together-with-qt4
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QString>
#include <QList>
#include <QDockWidget>
#include "exception.hpp"
#include "localization.hpp"
#include "syslog.hpp"

namespace Ui
{
    class History;
}

namespace Huggle
{
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
            HistoryItem *UndoDependency;
            QString UndoRevBaseTime;
            QString Result;
            QString Target;
            //! Type of item
            HistoryType Type;
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
            void Refresh();
            QList<HistoryItem> Items;
            static int Last;

        private:
            Ui::History *ui;
    };
}

#endif // HISTORY_H
