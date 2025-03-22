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

#include <huggle_core/definitions.hpp>

#include <QString>
#include <QList>
#include <QModelIndex>
#include <QDockWidget>
#include <huggle_core/collectable_smartptr.hpp>
#include <huggle_core/apiquery.hpp>
#include <huggle_core/historyitem.hpp>
#include <huggle_core/editquery.hpp>
#include <huggle_core/revertquery.hpp>
#include <huggle_core/wikiedit.hpp>

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
    class HUGGLE_EX_UI History : public QDockWidget
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
            void on_tableWidget_itemSelectionChanged();
            void on_tableWidget_doubleClicked(const QModelIndex &index);

        private:
            void DeleteItems();
            void Fail();
            void ShowEdit();
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
