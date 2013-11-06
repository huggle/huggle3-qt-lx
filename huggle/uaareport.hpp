//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef UAAREPORT_H
#define UAAREPORT_H

#include <QDialog>
#include <QString>
#include <QtXml>
#include <QTimer>
#include "core.hpp"
#include "configuration.hpp"
#include "wikiuser.hpp"
#include "wikipage.hpp"
#include "apiquery.hpp"

namespace Ui
{
    class UAAReport;
}

namespace Huggle
{
    class WikiUser;
    class WikiPage;
    class ApiQuery;
    //! Form to report users to UAA
    /// \todo This form wasn't tested by anyone so far, it must be tested before it's included in production build
    class UAAReport : public QDialog
    {
            Q_OBJECT
        public:
            explicit UAAReport(QWidget *parent = 0);
            ~UAAReport();
            void setUserForUAA(WikiUser *user);
        private slots:
            void on_pushButton_clicked();
            void on_pushButton_2_clicked();
            void onTick();
        private:
            Ui::UAAReport *ui;
            /// \todo DOCUMENT ME - it's not really clear what this is for because the name is too weird
            QTimer *uaat;
            WikiUser *User;
            QString contentsOfUAA;
            /// \todo DOCUMENT ME - it's not really clear what this is for because the name is too weird
            QString dr;
            QString optionalreason;
            /// \todo DOCUMENT ME - it's not really clear what this is for because the name is too weird
            QString ta;
            QString uaaReportReason;
            WikiPage *page;
            /// \todo DOCUMENT ME - it's not really clear what this is for because the name is too weird
            ApiQuery *qUAApage;
            /// \todo DOCUMENT ME - it's not really clear what this is for because the name is too weird
            QTimer *uT;
            void getPageContents();
            void whatToReport();
            void insertUsername();
            void failed(QString reason);
    };
}

#endif // UAAREPORT_H
