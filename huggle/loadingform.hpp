//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef LOADINGFORM_HPP
#define LOADINGFORM_HPP

#include "definitions.hpp"

#include <QDialog>

namespace Ui
{
    class LoadingForm;
}

namespace Huggle
{
    enum LoadingForm_Icon
    {
        LoadingForm_Icon_Loading,
        LoadingForm_Icon_Failed,
        LoadingForm_Icon_Waiting,
        LoadingForm_Icon_Success
    };

    //! Working form that is used for tasks that needs to perform multiple simultaneous things in same time
    class LoadingForm : public QDialog
    {
            Q_OBJECT
        public:
            explicit LoadingForm(QWidget *parent = nullptr);
            void Info(QString text);
            void ModifyIcon(int row, LoadingForm_Icon it);
            void Insert(int row, QString text, LoadingForm_Icon icon);
            ~LoadingForm();
        private slots:
            void on_pushButton_clicked();
        private:
            Ui::LoadingForm *ui;
            void reject();
    };
}

#endif // LOADINGFORM_HPP
