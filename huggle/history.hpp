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

#include <QString>
#include <QList>
#include <QModelIndex>
#include <QDockWidget>
#include "collectable_smartptr.hpp"
#include "apiquery.hpp"
#include "historyitem.hpp"
#include "editquery.hpp"
#include "revertquery.hpp"
#include "wikiedit.hpp"

namespace Ui
{
    class History;
}

namespace Huggle
{
    class ApiQuery;
    class EditQuery;
    class HistoryItem;
    class RevertQuery;

    //! History of actions done by user

    //! This is a widget that displays the user history
    class HUGGLE_EX History : public QDockWidget
    {
            Q_OBJECT
        public:
            explicit History(QWidget *parent = 0);
            ~History();
            void Undo(HistoryItem *hist);
            //! Insert a new item to top of list
            void Prepend(HistoryItem *item);
            QList<HistoryItem*> Items;

        private slots:
            void ContextMenu(const QPoint& position);
            void Tick();
            void Display();
            void on_tableWidget_clicked(const QModelIndex &index);

        private:
            void DeleteItems();
            void Fail();
            QTimer *timerRetrievePageInformation;
            Collectable_SmartPtr<HistoryItem> RevertingItem;
            QTimer *timerDisplayEditFromHistLs;
            //! This is a query we need to use to retrieve our own edit before we undo it
            Collectable_SmartPtr<ApiQuery> qEdit;
            //! This is for welcome message that is used to replace a talk page
            Collectable_SmartPtr<EditQuery> qTalk;
            //! Used to revert edits we made
            Collectable_SmartPtr<RevertQuery> qSelf;
            Collectable_SmartPtr<WikiEdit> DisplayedEdit;
            int CurrentItem = -200;
            Ui::History *ui;
    };
}

#endif // HISTORY_H
