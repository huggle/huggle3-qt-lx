//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef WARNINGLIST_HPP
#define WARNINGLIST_HPP

#include "definitions.hpp"
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QDialog>
#include "wikiedit.hpp"

namespace Ui
{
    class WarningList;
}

namespace Huggle
{
    //! Widget that allows user to pick a warning to send to user
    class WarningList : public QDialog
    {
            Q_OBJECT

        public:
            explicit WarningList(WikiEdit *edit, QWidget *parent = 0);
            ~WarningList();

        private slots:
            void on_pushButton_clicked();

        private:
            WikiEdit *wikiEdit;
            Ui::WarningList *ui;
    };
}

#endif // WARNINGLIST_HPP
