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

#include "definitions.hpp"

#include <QDialog>
#include <QString>
#include <QTimer>
#include "apiquery.hpp"
#include "collectable_smartptr.hpp"

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
    class HUGGLE_EX UAAReport : public QDialog
    {
            Q_OBJECT
        public:
            explicit UAAReport(QWidget *parent = nullptr);
            ~UAAReport();
            //! I don't have much of a clue why this is here lol, but I know the dialog can't be initialised from mainwindow without this
            // Who are you? It's there because we must somehow set user to this class I guess --Petrb
            void setUserForUAA(WikiUser *user);
        private slots:
            void on_pushButton_clicked();
            void on_pushButton_2_clicked();
            void on_pushButton_3_clicked();
            void onTick();
            void onStartOfSearch();
        private:
            //! Function to decide what we are reporting
            void whatToReport();
            //! Check if user is reported
            bool checkIfReported();
            //! Get page contents (reason for this is above)
            void getPageContents();
            //! Function to that allows us to properly insert what we need to insert
            void insertUsername();
            //! Message box, if anything fails
            void failed(QString reason);
            Ui::UAAReport *ui;
            WikiUser *User;
            QString ContentsOfUAA;
            //! Whole contents of UAA page
            QString dr;
            //! String that represents what is in the line edit
            QString OptionalReason;
            //! UAA template
            QString ta;
            //! Reason for report
            QString UAAReportReason;
            //! Pointer to WikiUser
            WikiPage *page;
            //! Pointer to get UAA contents (we don't want replace the page with our content, do we?)
            Collectable_SmartPtr<ApiQuery> qUAApage;
            //! Timer pointer that allows us to do magical things
            QTimer *Timer;
            //! Timer that does other magical things
            QTimer *TimerCheck;
            //! Pointer that also gets UAA contents; this time it is used for checking if a user is reported or not
            Collectable_SmartPtr<ApiQuery> qCheckUAAUser;
    };
}

#endif // UAAREPORT_H
