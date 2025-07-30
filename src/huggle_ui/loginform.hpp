//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

// This file contains source for login form and most of login operations

#ifndef LOGINFORM_H
#define LOGINFORM_H

#include <huggle_core/definitions.hpp>

#include <huggle_core/collectable_smartptr.hpp>
#include <QThread>
#include <QHash>
#include <QTimer>
#include "hw.hpp"

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

    //! Window that is displayed as first when huggle is started, letting user login to one or more wikis
    class HUGGLE_EX_UI LoginForm : public HW
    {
            Q_OBJECT
        public:
            explicit LoginForm(QWidget *parent = nullptr);
            ~LoginForm();
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
            void on_buttonReloadWikis_clicked();
            void on_labelTranslate_linkActivated(const QString &link);
            void on_lineEdit_username_textChanged(const QString &arg1);
            void on_lineEdit_password_textChanged(const QString &arg1);
            void on_buttonToggleProjects_clicked();
            void on_label_linkActivated(const QString &link);
            void on_lineEditBotUser_textChanged(const QString &arg1);
            void on_lineEditBotP_textChanged(const QString &arg1);
            void on_labelBotPasswordHelp_linkActivated(const QString &link);
            void on_tabWidget_currentChanged(int index);
            void on_Language_currentIndexChanged(int index);

        private:
            //! Reset the interface to default
            void resetForm();
            void removeQueries();
            //! Enable parts of interface
            void enableForm(bool value = true);
            /*!
             * \brief ReadonlyFallback try to fallback into read-only mode
             * \param site to switch to read-only mode
             * \return true on success
             */
            void closeEvent(QCloseEvent *event);
            bool readonlyFallback(WikiSite *site, QString why);
            void reloadForm();
            void reloadWikiDB();
            void disableForm();
            void pressOK();
            void performLogin(WikiSite *site);
            void performLoginPart2(WikiSite *site);
            void finishLogin(WikiSite *site);
            void retrieveWhitelist(WikiSite *site);
            void retrieveProjectYamlConfig(WikiSite *site);
            void fallbackToLegacyConfig(WikiSite *site);
            void retrieveProjectConfig(WikiSite *site);
            bool retrieveGlobalConfig();
            void retrieveUserConfig(WikiSite *site);
            void retrieveUserInfo(WikiSite *site);
            void developerMode();
            bool isDeveloperMode();
            void processSiteInfo(WikiSite *site);
            void displayError(QString message);
            void finishLogin();
            void verifyLogin();
            int registerLoadingFormRow(WikiSite *site, int row);
            void clearLoadingFormRows();
            void reject();
            //! This function make sure that login result is done
            bool processOutput(WikiSite *site);
            UpdateForm *updateForm = nullptr;
            bool globalConfigIsLoaded = false;
            Ui::Login *ui;
            bool loginFinished = false;
            bool loginInProgress = false;
            //! ID of row in loading form which belongs to global config progress
            int loadingFormGlobalConfigRow = 0;
            QList <QCheckBox*> project_CheckBoxens;
            QTimer *timer;
            QHash<WikiSite*, bool> processedWL;
            bool Refreshing = false;
            QHash<WikiSite*, bool> processedSiteinfos;
            QHash<WikiSite*, bool> processedLogin;
            QHash<WikiSite*, ApiQuery*> qApproval;
            QHash<WikiSite*, WLQuery*> wlQueries;
            QHash<WikiSite*, ApiQuery*> qSiteInfo;
            QHash<WikiSite*, ApiQuery*> qTokenInfo;
            QHash<WikiSite*, ApiQuery*> LoginQueries;
            Collectable_SmartPtr<ApiQuery> qCurrentLoginRequest;
            LoadingForm *loadingForm = nullptr;
            //! True until the login form is fully loaded
            bool isLoading;
            Collectable_SmartPtr<ApiQuery> qDatabase;
            Collectable_SmartPtr<ApiQuery> qConfig;
            //! This is a list that identify every single row in loading form so that we know which row is which
            //! every site has its own hash of rows, where every hash correspond to real position
            QHash<WikiSite*,QHash<int,int>> loadingFormRows;
            //! Last row we used in loading form
            int loadingForm_LastRow = 0;
            //! The tokens obtained from login
            QHash<WikiSite*, QString> loginTokens;
            //! for RetrievePrivateConfig, if we should try to load from old config pages
            QHash <WikiSite*,bool> usingOldUserConfig;
    };
}

#endif // LOGIN_H
