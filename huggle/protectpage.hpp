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
        QString protecttoken;
        //! Pointer to get first token
        ApiQuery *ptkq;
        //! Pointer for second token
        ApiQuery *ptkk;
        //! Pointer for	protection
        ApiQuery *ptpt;
        Ui::ProtectPage *ui;
        //! DOCUMENT ME
        WikiPage *ptpge;
        //! DOCUMENT ME
        QTimer *tt;
        int PtQueryPhase;
    };
}

#endif // PROTECTPAGE_H

