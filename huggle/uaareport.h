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
#include "core.h"
#include "configuration.h"
#include "wikiuser.h"
#include "wikipage.h"
#include "apiquery.h"

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
            QTimer *uaat;
            WikiUser *User;
            QString contentsOfUAA;
            QString dr;
            QString optionalreason;
            QString ta;
            QString uaaReportReason;
            WikiPage *page;
            ApiQuery *qUAApage;
            QTimer *uT;
            void getPageContents();
            void whatToReport();
            void insertUsername();
            void failed(QString reason);
    };
}

#endif // UAAREPORT_H
