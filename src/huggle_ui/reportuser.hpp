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

#include <huggle_core/definitions.hpp>

#include "hw.hpp"
#include <QTimer>
#include <QCheckBox>
#include <QList>
#include <huggle_core/editquery.hpp>
#include <huggle_core/apiquery.hpp>
#include <huggle_core/collectable_smartptr.hpp>
class QModelIndex;

namespace Ui
{
    class ReportUser;
}

namespace Huggle
{
    class WikiUser;
    class HuggleWeb;
    class ApiQuery;
    class EditQuery;
    class BlockUserForm;

    //! Report user

    //! Huggle 3.1.19 implemented this window also as contribution list window.
    //! In standard mode this window is used to report users to AIV, but it's
    //! possible to switch it into "contrib browser" mode in which it serves
    //! only in order to display contributions of a given user
    class HUGGLE_EX_UI ReportUser : public HW
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
            void OnReportUserTimer();
            void OnPageDiffTimer();
            void OnReportCheckTimer();
            void on_buttonReport_clicked();
            void on_buttonTalkPage_clicked();
            void on_tableWidgetEdits_clicked(const QModelIndex &index);
            void on_buttonSelectAll_clicked();
            void on_buttonRemoveSelection_clicked();
            void on_buttonBlock_clicked();
            void on_buttonCheckBlocked_clicked();
            void on_buttonCheckReported_clicked();

    private:
            bool checkUserIsReported();
            void insertUser();
            void reportUser();
            //! Stop all operations
            void kill();
            void errorMessage(QString reason);
            HuggleWeb        *webView = nullptr;
            bool isBrowser = false;
            bool flagSilent = false;
            Ui::ReportUser *ui;
            //! Reported user
            WikiUser *reportedUser;
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
            QList <QCheckBox*> checkBoxes;
            //! Text of report to send to AIV page
            QString reportText;
            //! Content of report
            QString reportContent;
            QString reportTs;
            bool loading;
            bool isSendingMessageNow;
            BlockUserForm *blockUser;
            //! This query is used to get a block history
            Collectable_SmartPtr<ApiQuery> qBlockHistory;
            //! This is used to retrieve current report page and write to it
            Collectable_SmartPtr<ApiQuery> qReport;
            Collectable_SmartPtr<ApiQuery> qDiff;
    };
}

#endif // REPORTUSER_H
