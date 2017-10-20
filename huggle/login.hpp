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

#include "apiquery.hpp"
#include "collectable_smartptr.hpp"
#include "hw.hpp"
#include "oauthloginquery.hpp"
#include "wlquery.hpp"
#include <QThread>
#include <QHash>
#include <QTimer>
class QCheckBox;

namespace Ui
{
    class Login;
}

namespace Huggle
{
    enum Status
    {
        RetrievingUserConfig,
        RetrievingProjectYAMLConfig,
        RetrievingProjectOldConfig,
        LoggingIn,
        WaitingForLoginQuery,
        WaitingForToken,
        LoggedIn,
        Nothing,
        Cancelling,
        LoginFailed,
        RetrievingUser,
        LoginDone
    };

    class ApiQuery;
    class UpdateForm;
    class WLQuery;
    class LoadingForm;
    class WikiSite;

    //! Window that is displayed as first when huggle is started
    class HUGGLE_EX Login : public HW
    {
            Q_OBJECT
        public:
            explicit Login(QWidget *parent = 0);
            ~Login();
            //! This function will reload all localizations for login form, called when user change a language
            void Localize();
            //! Updates the info message down on login form as well as on LoadingForm
            void Update(QString ms);
            //! Cancel currently running login jobs
            void CancelLogin();
            int GetRowIDForSite(WikiSite *site, int row);
            //! Status we are in (loggin it, waiting for this and that etc)
            QHash<WikiSite*,Status> Statuses;

        private slots:
            void on_ButtonOK_clicked();
            void on_ButtonExit_clicked();
            void OnTimerTick();
            void on_pushButton_clicked();
            void on_Language_currentIndexChanged(const QString &arg1);
            void on_labelTranslate_linkActivated(const QString &link);
            void on_lineEdit_username_textChanged(const QString &arg1);
            void on_lineEdit_password_textChanged(const QString &arg1);
            void on_pushButton_2_clicked();
            void on_label_linkActivated(const QString &link);
            void on_lineEditBotUser_textChanged(const QString &arg1);
            void on_lineEditBotP_textChanged(const QString &arg1);
            void on_label_2_linkActivated(const QString &link);
            void on_tabWidget_currentChanged(int index);

        private:
            //! Reset the interface to default
            void Reset();
            void RemoveQueries();
            //! Enable parts of interface
            void EnableForm(bool value = true);
            /*!
             * \brief ReadonlyFallback try to fallback into read-only mode
             * \param site to switch to read-only mode
             * \return true on success
             */
            bool ReadonlyFallback(WikiSite *site, QString why);
            void Reload();
            void DB();
            void Disable();
            void PressOK();
            void PerformLogin(WikiSite *site);
            void PerformLoginPart2(WikiSite *site);
            void FinishLogin(WikiSite *site);
            void RetrieveWhitelist(WikiSite *site);
            void RetrieveProjectYamlConfig(WikiSite *site);
            void FallbackToLegacyConfig(WikiSite *site);
            void RetrieveProjectConfig(WikiSite *site);
            bool RetrieveGlobalConfig();
            void RetrieveUserConfig(WikiSite *site);
            void RetrieveUserInfo(WikiSite *site);
            void DeveloperMode();
            bool IsDeveloperMode();
            void ProcessSiteInfo(WikiSite *site);
            void DisplayError(QString message);
            void Finish();
            void VerifyLogin();
            int RegisterLoadingFormRow(WikiSite *site, int row);
            void ClearLoadingFormRows();
            void reject();
            //! This function make sure that login result is done
            bool ProcessOutput(WikiSite *site);
            UpdateForm *Updater = nullptr;
            bool GlobalConfig = false;
            Ui::Login *ui;
            bool Finished = false;
            bool Processing = false;
            int GlobalRow = 0;
            QList <QCheckBox*> Project_CheckBoxens;
            QTimer *timer;
            QHash<WikiSite*, bool> processedWL;
            bool Refreshing = false;
            QHash<WikiSite*, bool> processedSiteinfos;
            QHash<WikiSite*, bool> processedLogin;
            QHash<WikiSite*, ApiQuery*> qApproval;
            QHash<WikiSite*, WLQuery*> WhitelistQueries;
            QHash<WikiSite*, ApiQuery*> qSiteInfo;
            QHash<WikiSite*, ApiQuery*> qTokenInfo;
            QHash<WikiSite*, ApiQuery*> LoginQueries;
            LoadingForm *loadingForm = nullptr;
            bool Loading;
            Collectable_SmartPtr<ApiQuery> qDatabase;
            Collectable_SmartPtr<ApiQuery> qConfig;
            //! This is a list that identify every single row in loading form so that we know which row is which
            //! every site has its own hash of rows, where every hash correspond to real position
            QHash<WikiSite*,QHash<int,int>> LoadingFormRows;
            //! Last row we used in loading form
            int LastRow = 0;
            //! The token obtained from login
            QHash<WikiSite*, QString> Tokens;
            //! for RetrievePrivateConfig, if we should try to load from
            QHash <WikiSite*,bool> LoadedOldConfigs;
    };
}

#endif // LOGIN_H
