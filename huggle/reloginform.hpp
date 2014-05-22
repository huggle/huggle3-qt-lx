//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef RELOGINFORM_HPP
#define RELOGINFORM_HPP

#include "definitions.hpp"
#ifdef PYTHONENGINE
#include <Python.h>
#endif
#include <QDialog>

namespace Ui
{
    class ReloginForm;
}

namespace Huggle
{
    //! Relogin form used to login back to mediawiki when session is removed
    class ReloginForm : public QDialog
    {
            Q_OBJECT
        public:
            explicit ReloginForm(QWidget *parent = 0);
            ~ReloginForm();

        private slots:
            void on_pushButton_clicked();

            void on_pushButton_2_clicked();

        private:
            Ui::ReloginForm *ui;
    };
}

#endif // RELOGINFORM_HPP
