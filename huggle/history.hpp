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

#include <QString>
#include <QList>
#include <QDockWidget>
#include "exception.hpp"

namespace Ui
{
    class History;
}

namespace Huggle
{
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
            QString Result;
            QString Target;
            //! Type of item
            HistoryType Type;
        private:

    };

    /// \todo It should be possible to go back in history to review what you have you done
    /// currently nothing happens when you click on history items
    /// \todo Function to revert your own changes

    //! History of actions done by user
    class History : public QDockWidget
    {
            Q_OBJECT

        public:
            explicit History(QWidget *parent = 0);
            ~History();
            //! Insert a new item to top of list
            void Prepend(HistoryItem item);
            void Refresh();
            void Remove(HistoryItem item);
            QList<HistoryItem> Items;
            static int Last;

        private:
            Ui::History *ui;
    };
}

#endif // HISTORY_H
