//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef IGNORELIST_H
#define IGNORELIST_H

#include "definitions.hpp"
// now we need to ensure that python is included first
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QDialog>
#include <QStandardItemModel>

namespace Ui
{
    class IgnoreList;
}

namespace Huggle
{
    //! A window that contains ignore list
    class IgnoreList : public QDialog
    {
            Q_OBJECT

        public:
            explicit IgnoreList(QWidget *parent = nullptr);
            ~IgnoreList();

        private slots:
            void on_pushButton_clicked();

        private:
            QStandardItemModel *model;
            Ui::IgnoreList *ui;
    };
}

#endif // IGNORELIST_H
