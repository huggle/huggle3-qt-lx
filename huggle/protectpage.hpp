//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef PROTECTPAGE_H
#define PROTECTPAGE_H

#include "definitions.hpp"

#include "hw.hpp"
#include <QString>
#include <QtXml>
#include "collectable_smartptr.hpp"
#include <QTimer>
#include "apiquery.hpp"
#include "wikipage.hpp"

namespace Ui
{
    class ProtectPage;
}

namespace Huggle
{
    class ApiQuery;
    class WikiPage;

    /*!
     * \brief The ProtectPage class display a window where user can protect a page given they have the permissions to do that
     */
    class HUGGLE_EX ProtectPage : public HW
    {
            Q_OBJECT
        public:
            explicit ProtectPage(QWidget *parent = nullptr);
            ~ProtectPage();
            /*!
             * \brief set a page that is supposed to be protected, this needs to be called by owner who created this form
             * \param Page that will be protected by user
             */
            void setPageToProtect(WikiPage *Page);
        private slots:
            void on_pushButton_clicked();
            void on_pushButton_2_clicked();
            void onTick();
        private:
            void Failed(QString reason);
            void Protect();
            //! Pointer for	protection
            Collectable_SmartPtr<ApiQuery> qProtection;
            Ui::ProtectPage *ui;
            //! Page that is about to be protected in this form
            WikiPage *PageToProtect;
            //! Timer that is used to switch between phases
            QTimer *tt;
    };
}

#endif // PROTECTPAGE_H

