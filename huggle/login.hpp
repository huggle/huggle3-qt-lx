//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

// This file contains source for login form and most of login operations

#ifndef LOGIN_H
#define LOGIN_H

#include "definitions.hpp"
// now we need to ensure that python is included first
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QDialog>
#include <QThread>
#include <QTimer>
#include "collectable_smartptr.hpp"
#include "oauthloginquery.hpp"
#include "wlquery.hpp"
#include "updateform.hpp"
#include "loadingform.hpp"
#include "apiquery.hpp"

namespace Ui
{
    class Login;
}

namespace Huggle
{
    enum Status
    {
        RetrievingGlobalConfig,
        RetrievingUserConfig,
        RetrievingProjectConfig,
        LoggingIn,
        WaitingForLoginQuery,
        Refreshing,
        WaitingForToken,
        LoggedIn,
        Nothing,
        Cancelling,
        LoginFailed,
        RetrievingUser,
        LoginDone
    };

    class WLQuery;
    class UpdateForm;
    class ApiQuery;

    //! Window that is displayed as first when huggle is started
    class Login : public QDialog
    {
            Q_OBJECT
        public:
            explicit Login(QWidget *parent = 0);
            ~Login();
            //! This function will reload all localizations for login form, called when user change a language
            void Localize();
            //! Updates the info message down on login form as well as on LoadingForm
            void Update(QString ms);
            void Kill();
            //! Cancel currently running login jobs
            void CancelLogin();
            //! Status we are in (loggin it, waiting for this and that etc)
            Status _Status;

        private slots:
            void on_ButtonOK_clicked();
            void on_ButtonExit_clicked();
            void OnTimerTick();
            void on_pushButton_clicked();
            void on_Language_currentIndexChanged(const QString &arg1);
            void on_labelTranslate_linkActivated(const QString &link);
            void on_lineEdit_username_textChanged(const QString &arg1);
            void on_lineEdit_password_textChanged(const QString &arg1);

        private:
            //! String that is used to test against the login failed text
            static QString Test;

            //! Reset the interface to default
            void Reset();
            //! Enable parts of interface
            void Enable();
            void Reload();
            void DB();
            void Disable();
            void PressOK();
            void PerformLogin();
            void PerformLoginPart2();
            void FinishLogin();
            void RetrieveWhitelist();
            void RetrieveProjectConfig();
            void RetrieveGlobalConfig();
            void RetrieveUserConfig();
            void RetrieveUserInfo();
            void DeveloperMode();
            void ProcessSiteInfo();
            void DisplayError(QString message);
            void Finish();
            void VerifyLogin();
            void reject();
            //! This function make sure that login result is done
            bool ProcessOutput();
            QString GetToken();
            UpdateForm *Updater = nullptr;
            Ui::Login *ui;
            QTimer *timer;
            bool processedWlQuery;
            bool processedSiteinfo;
            bool processedLogin;
            //! This query is used to get a wl
            Collectable_SmartPtr<WLQuery> wq;
            Collectable_SmartPtr<ApiQuery> LoginQuery;
            LoadingForm *loadingForm = nullptr;
            bool Loading;
            Collectable_SmartPtr<ApiQuery> qSiteInfo;
            Collectable_SmartPtr<ApiQuery> qCfg;
            //! The token obtained from login
            QString Token;
            //! for RetrievePrivateConfig, if we should try to load from
            bool LoadedOldConfig;
    };
}

#endif // LOGIN_H
