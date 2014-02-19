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

#include "config.hpp"
// now we need to ensure that python is included first, because it
// simply suck :P
// seriously, Python.h is shitty enough that it requires to be
// included first. Don't believe it? See this:
// http://stackoverflow.com/questions/20300201/why-python-h-of-python-3-2-must-be-included-as-first-together-with-qt4
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QDialog>
#include <QString>
#include <QtXml>
#include <QTimer>
#include "apiquery.hpp"
#include "core.hpp"
#include "configuration.hpp"
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
    class ProtectPage : public QDialog
    {
            Q_OBJECT

        public:
            explicit ProtectPage(QWidget *parent = 0);
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
            void getTokenToProtect();
            void checkTokenToProtect();
            QString ProtectToken;
            //! Pointer to get first token
            ApiQuery *qToken1;
            //! Pointer for second token
            ApiQuery *qToken2;
            //! Pointer for	protection
            ApiQuery *qProtection;
            Ui::ProtectPage *ui;
            //! Page that is about to be protected in this form
            WikiPage *PageToProtect;
            //! Timer that is used to switch between phases
            QTimer *tt;
            int PtQueryPhase;
    };
}

#endif // PROTECTPAGE_H

