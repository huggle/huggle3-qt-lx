//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HISTORYFORM_H
#define HISTORYFORM_H

#include "definitions.hpp"
// now we need to ensure that python is included first, because it
// simply suck :P
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QTimer>
#include <QToolTip>
#include <QDockWidget>
#include "apiquery.hpp"
#include "resources.hpp"
#include "exception.hpp"
#include "wikiedit.hpp"
#include "mainwindow.hpp"
#include "localization.hpp"

namespace Ui
{
    class HistoryForm;
}

namespace Huggle
{
    //! This is a small gadget that is displayed on top of main window

    //! It can be used to retrieve a history of currently displayed page
    class HistoryForm : public QDockWidget
    {
            Q_OBJECT
        public:
            explicit HistoryForm(QWidget *parent = 0);
            ~HistoryForm();
            void Read();
            void Update(WikiEdit *edit);

        private slots:
            void onTick01();
            void on_pushButton_clicked();
            void on_tableWidget_clicked(const QModelIndex &index);

        private:
            void Clear();
            void Display(int row, QString html, bool turtlemode = false);
            //! Make the selected row bold
            void MakeSelectedRowBold();
            bool RetrievingEdit;
            Ui::HistoryForm *ui;
            WikiEdit* CurrentEdit;
            ApiQuery *query;
            int PreviouslySelectedRow;
            int SelectedRow;
            WikiEdit* RetrievedEdit;
            //! This timer is used to check a query status
            QTimer *t1;
    };
}

#endif // HISTORYFORM_H

