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

#include <QDialog>
#include <QTimer>
#include <QString>
#include "apiquery.hpp"
#include "collectable_smartptr.hpp"

namespace Ui
{
    class ReloginForm;
}

namespace Huggle
{
    class ApiQuery;
    //! Relogin form used to login back to mediawiki when session is removed
    class ReloginForm : public QDialog
    {
            Q_OBJECT
        public:
            explicit ReloginForm(QWidget *parent = nullptr);
            ~ReloginForm();

        private slots:
            void on_pushButton_clicked();
            void on_pushButton_2_clicked();
            void LittleTick();

        private:
            void Fail(QString why);
            void reject();
            //! This is just a timer, it's called little and cute because I was bored when writing this piece of code
            QTimer *little_cute_timer;
            ApiQuery *qReloginTokenReq = nullptr;
            ApiQuery *qReloginPw = nullptr;
            Ui::ReloginForm *ui;
    };
}

#endif // RELOGINFORM_HPP
