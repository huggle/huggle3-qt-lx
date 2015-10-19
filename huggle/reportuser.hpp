//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef REPORTUSER_H
#define REPORTUSER_H

#include "definitions.hpp"

#include "hw.hpp"
#include <QTimer>
#include <QCheckBox>
#include <QList>
#include "editquery.hpp"
#include "apiquery.hpp"
#include "collectable_smartptr.hpp"
class QModelIndex;

namespace Ui
{
    class ReportUser;
}

namespace Huggle
{
    class WikiUser;
    class ApiQuery;
    class EditQuery;
    class BlockUser;

    //! Report user

    //! Huggle 3.1.19 implemented this window also as contribution list window.
    //! In standard mode this window is used to report users to AIV, but it's
    //! possible to switch it into "contrib browser" mode in which it serves
    //! only in order to display contributions of a given user
    class HUGGLE_EX ReportUser : public HW
    {
            Q_OBJECT
        public:
            static void SilentReport(WikiUser *user);

            /*!
             * \brief ReportUser
             * \param parent
             * \param browser If true the report form will turn into a contribution browser
             */
            explicit ReportUser(QWidget *parent = nullptr, bool browser = false);
            //! Set a user
            bool SetUser(WikiUser *user);
            void SilentReport();
            ~ReportUser();
        private slots:
            void Tick();
            void On_DiffTick();
            void Test();
            void on_pushButton_clicked();
            void on_pushButton_2_clicked();
            void on_tableWidget_clicked(const QModelIndex &index);
            void on_pushButton_3_clicked();
            void on_pushButton_4_clicked();
            void on_pushButton_5_clicked();
            void on_pushButton_6_clicked();
            void on_pushButton_7_clicked();
        private:
            bool CheckUser();
            void InsertUser();
            void Report();
            //! Stop all operations
            void Kill();
            void failCheck(QString reason);
            bool isBrowser = false;
            bool flagSilent = false;
            Ui::ReportUser *ui;
            //! Reported user
            WikiUser *ReportedUser;
            //! This query is used to retrieve a history of user
            Collectable_SmartPtr<ApiQuery> qHistory;
            Collectable_SmartPtr<ApiQuery> qCheckIfBlocked;
            //! Timer is used to report the user
            QTimer *tReportUser;
            //! Timer to check the report page
            QTimer *tReportPageCheck;
            //! Used to retrieve a diff of page
            QTimer *tPageDiff;
            Collectable_SmartPtr<EditQuery> qEdit;
            QList <QCheckBox*> CheckBoxes;
            //! Text of report to send to AIV page
            QString ReportText;
            //! Content of report
            QString ReportContent;
            QString ReportTs;
            bool Loading;
            bool Messaging;
            BlockUser *BlockForm;
            //! This query is used to get a block history
            Collectable_SmartPtr<ApiQuery> qBlockHistory;
            //! This is used to retrieve current report page and write to it
            Collectable_SmartPtr<ApiQuery> qReport;
            Collectable_SmartPtr<ApiQuery> qDiff;
    };
}

#endif // REPORTUSER_H
