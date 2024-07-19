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

#include <huggle_core/definitions.hpp>

#include <QDialog>
#include <QTimer>
#include <QString>
#include <huggle_core/apiquery.hpp>
#include <huggle_core/collectable_smartptr.hpp>
#include <huggle_core/wikisite.hpp>

namespace Ui
{
    class ReloginForm;
}

namespace Huggle
{
    class ApiQuery;
    //! Relogin form used to login back into mediawiki when session is no longer valid
    class HUGGLE_EX_UI ReloginForm : public QDialog
    {
            Q_OBJECT
        public:
            explicit ReloginForm(WikiSite *site, QWidget *parent = nullptr);
            ~ReloginForm();

        private slots:
            void on_pushButton_ForceExit_clicked();
            void on_pushButton_Relog_clicked();
            void ReloginTick();

            void on_checkBox_RemeberPw_toggled(bool checked);

    private:
            void Fail(QString why);
            void Localize();
            void reject();
            WikiSite *loginSite;
            QTimer *reloginTimer;
            Collectable_SmartPtr<ApiQuery> qReloginTokenReq;
            Collectable_SmartPtr<ApiQuery> qReloginPw;
            Ui::ReloginForm *ui;
    };
}

#endif // RELOGINFORM_HPP
