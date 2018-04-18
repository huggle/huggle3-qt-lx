//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef EXCEPTIONWINDOW_H
#define EXCEPTIONWINDOW_H

#include "definitions.hpp"

#if _MSC_VER
#define TRACING 0
#else
#define TRACING 1
#endif

#include <QDialog>
#include "exception.hpp"

namespace Ui
{
    class ExceptionWindow;
}

namespace Huggle
{
    //! Recovery window
    class ExceptionWindow : public QDialog
    {
            Q_OBJECT

        public:
            explicit ExceptionWindow(Exception *e);
            ~ExceptionWindow();

        private slots:
            void on_pushButton_clicked();
            void on_pushButton_3_clicked();
            void on_pushButton_2_clicked();

        private:
            Ui::ExceptionWindow *ui;
    };
}

#endif // EXCEPTIONWINDOW_H
