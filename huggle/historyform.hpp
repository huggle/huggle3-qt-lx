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

#include <QTimer>
#include <QDockWidget>
#include "core.hpp"
#include "apiquery.hpp"
#include "exception.hpp"
#include "wikiedit.hpp"

namespace Ui
{
    class HistoryForm;
}

namespace Huggle
{
    /// \todo RELEASE BLOCKER: this thing needs to be finished
    //! This is a small gadget that is displayed on top of main window

    //! It can be used to retrieve a history of currently displayed page
    class HistoryForm : public QDockWidget
    {
            Q_OBJECT
        public:
            explicit HistoryForm(QWidget *parent = 0);
            ~HistoryForm();
            void Update(WikiEdit *edit);

        private slots:
            void onTick01();
            void on_pushButton_clicked();

        private:
            Ui::HistoryForm *ui;
            WikiEdit* CurrentEdit;
            ApiQuery *query;
            //! This timer is used to check a query status
            QTimer *t1;
    };
}

#endif // HISTORYFORM_H
