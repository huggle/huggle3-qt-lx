//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <QMessageBox>
#include <QThread>
#include <QUrl>
#include <QtXml>
#include <QTimer>
#include "core.h"
#include "oauthloginquery.h"
#include "wlquery.h"
#include "apiquery.h"
#include "configuration.h"

namespace Ui {
class Login;
}

enum Status
{
    RetrievingGlobalConfig,
    RetrievingUserConfig,
    RetrievingLocalConfig,
    LoggingIn,
    WaitingForLoginQuery,
    Refreshing,
    WaitingForToken,
    LoggedIn,
    Nothing,
    Cancelling,
    LoginFailed,
    RetrievingUser,
    LoginDone,
    RetrievingWhitelist
};

class WLQuery;
class ApiQuery;

//! Window that is displayed as first when huggle is started
class Login : public QDialog
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = 0);
    ~Login();
    Status _Status;
    void Progress(int progress);
    void Localize();

private slots:
    void on_ButtonOK_clicked();
    void on_ButtonExit_clicked();
    void on_Login_destroyed();
    void on_Time();
    void on_pushButton_clicked();
    void on_Language_currentIndexChanged(const QString &arg1);

private:
    Ui::Login *ui;
    QTimer *timer;
    WLQuery *wq;
    ApiQuery *LoginQuery;
    //! The token obtained from login
    QString Token;
    static QString Test;
    //! Reset the interface to default
    void Reset();
    //! Enable parts of interface
    void Enable();
    void Reload();
    void DB();
    //! Cancel currently running login attempt
    void CancelLogin();
    void Disable();
    void PressOK();
    void PerformLogin();
    void FinishLogin();
    void FinishToken();
    void RetrieveWhitelist();
    void RetrieveLocalConfig();
    void RetrieveGlobalConfig();
    void RetrievePrivateConfig();
    void RetrieveUserInfo();
    void DeveloperMode();
    void DisplayError(QString message);
    void Finish();
    //! This function make sure that login result is done
    bool ProcessOutput();
    QString GetToken();
};

#endif // LOGIN_H
