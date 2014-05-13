//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "login.hpp"
#include <QMessageBox>
#include <QUrl>
#include <QDesktopServices>
#include <QtXml>
#include "syslog.hpp"
#include "localization.hpp"
#include "core.hpp"
#include "configuration.hpp"
#include "ui_login.h"

#define LOGINFORM_LOGIN 0
#define LOGINFORM_SITEINFO 1
#define LOGINFORM_GLOBALCONFIG 2
#define LOGINFORM_WHITELIST 3
#define LOGINFORM_LOCALCONFIG 4
#define LOGINFORM_USERCONFIG 5
#define LOGINFORM_USERINFO 6

using namespace Huggle;

QString Login::Test = "<login result=\"NeedToken\" token=\"";

Login::Login(QWidget *parent) :   QDialog(parent),   ui(new Ui::Login)
{
    this->Loading = true;
    this->ui->setupUi(this);
    this->_Status = Nothing;
    this->LoadedOldConfig = false;
    this->timer = new QTimer(this);
    connect(this->timer, SIGNAL(timeout()), this, SLOT(OnTimerTick()));
    this->setWindowTitle("Huggle 3 QT [" + Configuration::HuggleConfiguration->HuggleVersion + "]");
    this->Reset();
    this->ui->checkBox->setChecked(Configuration::HuggleConfiguration->SystemConfig_UsingSSL);

    // set the language to dummy english
    int l=0;
    int p=0;
    while (l<Localizations::HuggleLocalizations->LocalizationData.count())
    {
        this->ui->Language->addItem(Localizations::HuggleLocalizations->LocalizationData.at(l)->LanguageID);
        if (Localizations::HuggleLocalizations->LocalizationData.at(l)->LanguageName == Localizations::HuggleLocalizations->PreferredLanguage)
        {
            p = l;
        }
        l++;
    }

    this->ui->Language->setCurrentIndex(p);
    this->Reload();
    if (!QSslSocket::supportsSsl())
    {
        Configuration::HuggleConfiguration->SystemConfig_UsingSSL = false;
        this->ui->checkBox->setEnabled(false);
        this->ui->checkBox->setChecked(false);
    }
    if (Configuration::HuggleConfiguration->SystemConfig_UpdatesEnabled)
    {
        this->Updater = new UpdateForm();
        this->Updater->Check();
    }
    if (Configuration::HuggleConfiguration->SystemConfig_Username != "User")
    {
        this->ui->lineEdit_username->setText(Configuration::HuggleConfiguration->SystemConfig_Username);
        this->ui->lineEdit_password->setFocus();
    }
    this->ui->lineEdit_password->setText(Configuration::HuggleConfiguration->TemporaryConfig_Password);
    this->Loading = false;
    this->Localize();
    if (Configuration::HuggleConfiguration->Login)
    {
        // user wanted to login using a terminal
        this->PressOK();
    }
}

Login::~Login()
{
    delete this->Updater;
    delete this->ui;
    delete this->loadingForm;
    GC_DECREF(this->wq);
    GC_DECREF(this->LoginQuery);
    GC_DECREF(this->qSiteInfo);
    GC_DECREF(this->qCfg);
    delete this->timer;
}

void Login::Localize()
{
    this->ui->ButtonExit->setText(Localizations::HuggleLocalizations->Localize("main-system-exit"));
    this->ui->ButtonOK->setText(Localizations::HuggleLocalizations->Localize("login-start"));
    this->ui->checkBox->setText(Localizations::HuggleLocalizations->Localize("login-ssl"));
    this->ui->labelOauthUsername->setText(Localizations::HuggleLocalizations->Localize("login-username"));
    this->ui->pushButton->setText(Localizations::HuggleLocalizations->Localize("reload"));
    this->ui->labelUsername->setText(Localizations::HuggleLocalizations->Localize("login-username"));
    this->ui->labelProject->setText(Localizations::HuggleLocalizations->Localize("login-project"));
    this->ui->labelLanguage->setText(Localizations::HuggleLocalizations->Localize("login-language"));
    this->ui->labelPassword->setText(Localizations::HuggleLocalizations->Localize("login-password"));
    this->ui->labelIntro->setText(Localizations::HuggleLocalizations->Localize("login-intro"));
    this->ui->labelTranslate->setText(QString("<html><head/><body><p><a href=\"http://meta.wikimedia.org/wiki/Huggle/Localization\"><span style=\" text-decoration: underline; color:#0000ff;\">%1</span></a></p></body></html>")
                               .arg(Localizations::HuggleLocalizations->Localize("login-translate")));
}

void Login::Update(QString ms)
{
     // let's just pass it there
    if (this->loadingForm != nullptr)
        this->loadingForm->Info(ms);

    // update the label
    this->ui->labelIntro->setText(ms);
}

void Login::Kill()
{
    this->_Status = LoginFailed;
    this->timer->stop();
    if (this->loadingForm != nullptr)
    {
        this->loadingForm->close();
        delete this->loadingForm;
        this->loadingForm = nullptr;
    }
}

void Login::Reset()
{
    this->ui->labelIntro->setText(Localizations::HuggleLocalizations->Localize("[[login-intro]]"));
}

void Login::CancelLogin()
{
    this->timer->stop();
    this->Enable();
    if (this->loadingForm != nullptr)
    {
        this->loadingForm->close();
        delete this->loadingForm;
        this->loadingForm = nullptr;
    }
    this->_Status = Nothing;
    this->ui->labelIntro->setText(Localizations::HuggleLocalizations->Localize("login-intro"));
    this->ui->lineEdit_password->setText("");
    this->ui->ButtonOK->setText(Localizations::HuggleLocalizations->Localize("login-start"));
}

void Login::Enable()
{
    this->ui->lineEdit_oauth_username->setEnabled(true);
    this->ui->Language->setEnabled(true);
    this->ui->Project->setEnabled(true);
    this->ui->checkBox->setEnabled(QSslSocket::supportsSsl());
    this->ui->lineEdit_username->setEnabled(true);
    this->ui->ButtonExit->setEnabled(true);
    this->ui->lineEdit_password->setEnabled(true);
    this->ui->pushButton->setEnabled(true);
    this->ui->ButtonOK->setEnabled(true);
}

void Login::Reload()
{
    int current = 0;
    this->ui->Project->clear();
    while (current < Configuration::HuggleConfiguration->ProjectList.size())
    {
        this->ui->Project->addItem(Configuration::HuggleConfiguration->ProjectList.at(current)->Name);
        current++;
    }
    if (Huggle::Configuration::HuggleConfiguration->IndexOfLastWiki < current)
        this->ui->Project->setCurrentIndex(Huggle::Configuration::HuggleConfiguration->IndexOfLastWiki);
    else
        this->ui->Project->setCurrentIndex(0);
}

void Login::DB()
{
    if (this->LoginQuery == nullptr || this->LoginQuery->IsProcessed())
    {
        return;
    }
    Syslog::HuggleLogs->DebugLog(this->LoginQuery->Result->Data, 2);
    QDomDocument d;
    d.setContent(this->LoginQuery->Result->Data);
    QDomNodeList l = d.elementsByTagName("rev");
    if (l.count() > 0)
    {
        if (QFile().exists(Configuration::HuggleConfiguration->WikiDB))
            QFile().remove(Configuration::HuggleConfiguration->WikiDB);
        QFile wiki(Configuration::HuggleConfiguration->WikiDB);
        if (wiki.open(QIODevice::WriteOnly))
        {
            wiki.write(l.at(0).toElement().text().toUtf8());
            wiki.close();
        }
        Core::HuggleCore->LoadDB();
        Reload();
    }
    this->timer->stop();
    this->Enable();
    this->_Status = Nothing;
    this->Localize();
}

void Login::Disable()
{
    this->ui->lineEdit_oauth_username->setDisabled(true);
    this->ui->Language->setDisabled(true);
    this->ui->Project->setDisabled(true);
    this->ui->checkBox->setDisabled(true);
    this->ui->ButtonOK->setEnabled(false);
    this->ui->ButtonExit->setDisabled(true);
    this->ui->lineEdit_username->setDisabled(true);
    this->ui->lineEdit_password->setDisabled(true);
    this->ui->pushButton->setDisabled(true);
}

void Login::PressOK()
{
    if (this->ui->tab->isVisible())
    {
        QMessageBox mb;
        mb.setWindowTitle(Localizations::HuggleLocalizations->Localize("function-miss"));
        mb.setText("This function is not available for wmf wikis in this moment");
        mb.exec();
        return;
    }
    Configuration::HuggleConfiguration->IndexOfLastWiki = this->ui->Project->currentIndex();
    Configuration::HuggleConfiguration->Project = Configuration::HuggleConfiguration->ProjectList.at(this->ui->Project->currentIndex());
    Configuration::HuggleConfiguration->SystemConfig_UsingSSL = ui->checkBox->isChecked();
    if (this->ui->lineEdit_username->text() == "Developer Mode")
    {
        this->DeveloperMode();
        return;
    }
    Configuration::HuggleConfiguration->SystemConfig_Username = ui->lineEdit_username->text();
    Configuration::HuggleConfiguration->TemporaryConfig_Password = ui->lineEdit_password->text();
    if (this->loadingForm != nullptr)
    {
        delete this->loadingForm;
    }
    this->loadingForm = new LoadingForm(this);
    this->_Status = LoggingIn;
    this->Disable();
    this->loadingForm->show();
    // First of all, we need to login to the site
    this->timer->start(200);
    //! \todo Localize string for loadingForm
    this->loadingForm->Insert(LOGINFORM_LOGIN, "Logging in to " + Configuration::HuggleConfiguration->Project->Name, LoadingForm_Icon_Loading);
    this->loadingForm->Insert(LOGINFORM_SITEINFO, "Retrieving information about mediawiki for " + Configuration::HuggleConfiguration->Project->Name, LoadingForm_Icon_Waiting);
    this->loadingForm->Insert(LOGINFORM_GLOBALCONFIG, "Retrieving global configuration", LoadingForm_Icon_Waiting);
    this->loadingForm->Insert(LOGINFORM_WHITELIST, Localizations::HuggleLocalizations->Localize("login-progress-whitelist"), LoadingForm_Icon_Waiting);
    this->loadingForm->Insert(LOGINFORM_LOCALCONFIG, "Retrieving local configuration for " + Configuration::HuggleConfiguration->Project->Name, LoadingForm_Icon_Waiting);
    this->loadingForm->Insert(LOGINFORM_USERCONFIG, "Retrieving user configuration for " + Configuration::HuggleConfiguration->Project->Name, LoadingForm_Icon_Waiting);
    this->loadingForm->Insert(LOGINFORM_USERINFO, "Retrieving the user information", LoadingForm_Icon_Waiting);
}

void Login::PerformLogin()
{
    this->Update(Localizations::HuggleLocalizations->Localize("[[login-progress-start]]"));
    GC_DECREF(this->LoginQuery);
    // we create an api request to login
    this->LoginQuery = new ApiQuery(ActionLogin);
    this->LoginQuery->Parameters = "lgname=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->SystemConfig_Username);
    this->LoginQuery->HiddenQuery = true;
    this->LoginQuery->IncRef();
    this->LoginQuery->UsingPOST = true;
    this->LoginQuery->Process();
    this->_Status = WaitingForLoginQuery;
}

void Login::PerformLoginPart2()
{
    if (!this->LoginQuery->IsProcessed())
    {
        return;
    }
    if (this->LoginQuery->Result->Failed)
    {
        this->Update(Localizations::HuggleLocalizations->Localize("[[login-fail]]") +
                     ": " + this->LoginQuery->Result->ErrorMessage);
        this->Kill();
        this->LoginQuery->DecRef();
        this->LoginQuery = nullptr;
        return;
    }
    this->Token = this->LoginQuery->Result->Data;
    this->_Status = WaitingForToken;
    this->LoginQuery->DecRef();
    this->Token = this->GetToken();
    this->LoginQuery = new ApiQuery(ActionLogin);
    this->LoginQuery->HiddenQuery = true;
    this->LoginQuery->Parameters = "lgname=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->SystemConfig_Username)
            + "&lgpassword=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->TemporaryConfig_Password) + "&lgtoken="
            + Token;
    this->LoginQuery->UsingPOST = true;
    this->LoginQuery->IncRef();
    this->LoginQuery->Process();
}

void Login::RetrieveGlobalConfig()
{
    if (this->LoginQuery != nullptr)
    {
        if (this->LoginQuery->IsProcessed())
        {
            if (this->LoginQuery->Result->Failed)
            {
                this->Update(Localizations::HuggleLocalizations->Localize("[[login-error-global]]") + ": "
                             + this->LoginQuery->Result->ErrorMessage);
                this->Kill();
                this->LoginQuery->DecRef();
                this->LoginQuery = nullptr;
                return;
            }
            QDomDocument d;
            d.setContent(this->LoginQuery->Result->Data);
            QDomNodeList l = d.elementsByTagName("rev");
            this->LoginQuery->DecRef();
            this->LoginQuery = nullptr;
            if (l.count() == 0)
            {
                this->Update("Login failed unable to retrieve global config, the api query returned no data");
                this->Kill();
                return;
            }
            QDomElement data = l.at(0).toElement();
            if (Configuration::HuggleConfiguration->ParseGlobalConfig(data.text()))
            {
                if (!Configuration::HuggleConfiguration->GlobalConfig_EnableAll)
                {
                    this->Update(Localizations::HuggleLocalizations->Localize("login-error-alldisabled"));
                    this->_Status = LoginFailed;
                    return;
                }
                this->loadingForm->ModifyIcon(LOGINFORM_GLOBALCONFIG, LoadingForm_Icon_Success);
                this->_Status = RetrievingProjectConfig;
                this->RetrieveWhitelist();
                return;
            }
            this->Update(Localizations::HuggleLocalizations->Localize("login-error-global"));
            Syslog::HuggleLogs->DebugLog(data.text());
            this->_Status = LoginFailed;
        }
        return;
    }
    this->loadingForm->ModifyIcon(LOGINFORM_LOGIN, LoadingForm_Icon_Success);
    this->loadingForm->ModifyIcon(LOGINFORM_GLOBALCONFIG, LoadingForm_Icon_Loading);
    this->Update(Localizations::HuggleLocalizations->Localize("[[login-progress-global]]"));

    this->LoginQuery = new ApiQuery(ActionQuery);
    this->LoginQuery->IncRef();
    this->LoginQuery->OverrideWiki = Configuration::HuggleConfiguration->GlobalConfigurationWikiAddress;
    this->LoginQuery->Parameters = "prop=revisions&format=xml&rvprop=content&rvlimit=1&titles=Huggle/Config";
    this->LoginQuery->Process();

    this->loadingForm->ModifyIcon(LOGINFORM_SITEINFO, LoadingForm_Icon_Loading);
    this->qSiteInfo = new ApiQuery(ActionQuery);
    this->qSiteInfo->IncRef();
    this->qSiteInfo->Parameters = "meta=siteinfo&siprop=general";
    this->qSiteInfo->Process();

}

void Login::FinishLogin()
{
    if (!this->LoginQuery->IsProcessed())
    {
        return;
    }
    if (this->LoginQuery->Result->Failed)
    {
        this->Update("Login failed: " + this->LoginQuery->Result->ErrorMessage);
        this->_Status = LoginFailed;
        this->LoginQuery->DecRef();
        this->LoginQuery = nullptr;
        return;
    }

    // Assume login was successful
    if (this->ProcessOutput())
    {
        this->_Status = RetrievingGlobalConfig;
    }
    // that's all
    this->LoginQuery->DecRef();
    this->LoginQuery = nullptr;
}

void Login::RetrieveWhitelist()
{
    if (this->wq != nullptr)
    {
        if (this->wq->IsProcessed())
        {
            if (this->wq->Result->Failed)
            {
                Configuration::HuggleConfiguration->SystemConfig_WhitelistDisabled = true;
            } else
            {
                QString list = this->wq->Result->Data;
                list = list.replace("<!-- list -->", "");
                Configuration::HuggleConfiguration->WhiteList = list.split("|");
                Configuration::HuggleConfiguration->WhiteList.removeAll("");
            }
            this->loadingForm->ModifyIcon(LOGINFORM_WHITELIST, LoadingForm_Icon_Success);
            this->wq->DecRef();
            this->wq = nullptr;
        }
        return;
    }
    this->loadingForm->ModifyIcon(LOGINFORM_WHITELIST, LoadingForm_Icon_Loading);
    this->wq = new WLQuery();
    this->wq->RetryOnTimeoutFailure = false;
    this->wq->Process();
    this->wq->IncRef();
}

void Login::RetrieveProjectConfig()
{
    if (this->LoginQuery != nullptr)
    {
        if (this->LoginQuery->IsProcessed())
        {
            if (this->LoginQuery->Result->Failed)
            {
                this->Update(Localizations::HuggleLocalizations->Localize("login-error-config", this->LoginQuery->Result->ErrorMessage));
                this->LoginQuery->DecRef();
                this->Kill();
                this->LoginQuery = nullptr;
                return;
            }
            QDomDocument d;
            d.setContent(this->LoginQuery->Result->Data);
            QDomNodeList l = d.elementsByTagName("rev");
            if (l.count() == 0)
            {
                this->Kill();
                this->Update(Localizations::HuggleLocalizations->Localize("login-error-config", "the api query returned no data"));
                this->LoginQuery->DecRef();
                this->LoginQuery = nullptr;
                return;
            }
            this->LoginQuery->DecRef();
            this->LoginQuery = nullptr;
            QDomElement data = l.at(0).toElement();
            if (Configuration::HuggleConfiguration->ParseProjectConfig(data.text()))
            {
                if (!Configuration::HuggleConfiguration->ProjectConfig_EnableAll)
                {
                    this->Kill();
                    this->Update(Localizations::HuggleLocalizations->Localize("login-error-projdisabled"));
                    return;
                }
                this->loadingForm->ModifyIcon(LOGINFORM_LOCALCONFIG, LoadingForm_Icon_Success);
                this->_Status = RetrievingUserConfig;
                return;
            }
            this->Update(Localizations::HuggleLocalizations->Localize("login-error-config"));
            Syslog::HuggleLogs->DebugLog(data.text());
            this->_Status = LoginFailed;
        }
        return;
    }
    this->loadingForm->ModifyIcon(LOGINFORM_LOCALCONFIG, LoadingForm_Icon_Loading);
    this->Update(Localizations::HuggleLocalizations->Localize("login-progress-config"));
    this->LoginQuery = new ApiQuery(ActionQuery);
    this->LoginQuery->Parameters = "prop=revisions&format=xml&rvprop=content&rvlimit=1&titles=Project:Huggle/Config";
    this->LoginQuery->Process();
    this->LoginQuery->IncRef();
}

void Login::RetrieveUserConfig()
{
    if (this->LoginQuery != nullptr)
    {
        if (this->LoginQuery->IsProcessed())
        {
            if (this->LoginQuery->Result->Failed)
            {
                this->Kill();
                this->Update("Login failed unable to retrieve user config: " + this->LoginQuery->Result->ErrorMessage);
                this->LoginQuery->DecRef();
                this->LoginQuery = nullptr;
                return;
            }
            QDomDocument d;
            d.setContent(this->LoginQuery->Result->Data);
            QDomNodeList revisions = d.elementsByTagName("rev");
            if (revisions.count() == 0) // page is missing
            {
                if(this->LoadedOldConfig == false && Configuration::HuggleConfiguration->GlobalConfig_UserConf_old != "")
                {
                    // try first with old location of config, we don't need to switch the login step here we just
                    // replace the old query with new query that retrieves the old config and call this function
                    // once more, trying to parse the old config
                    this->LoadedOldConfig = true;
                    Syslog::HuggleLogs->DebugLog("couldn't find user config at new location, trying old one");
                    this->LoginQuery->DecRef();
                    /// \todo LOCALIZE ME
                    this->Update("Retrieving user config from old location");
                    this->LoginQuery = new ApiQuery(ActionQuery);
                    QString page = Configuration::HuggleConfiguration->GlobalConfig_UserConf_old;
                    page = page.replace("$1", Configuration::HuggleConfiguration->SystemConfig_Username);
                    this->LoginQuery->Parameters = "prop=revisions&rvprop=content&rvlimit=1&titles=" +
                            QUrl::toPercentEncoding(page);
                    this->LoginQuery->IncRef();
                    this->LoginQuery->Process();
                    return;
                }
                if (!Configuration::HuggleConfiguration->ProjectConfig_RequireConfig)
                {
                    // we don't care if user config is missing or not
                    this->LoginQuery->DecRef();
                    this->LoginQuery = nullptr;
                    this->_Status = RetrievingUser;
                    return;
                }
                Syslog::HuggleLogs->DebugLog(this->LoginQuery->Result->Data);
                /// \todo LOCALIZE ME
                this->Update("Login failed unable to retrieve user config, did you create huggle3.css "\
                             "in your userspace? (Special:MyPage/huggle3.css is missing)");
                this->Kill();
                this->LoginQuery->DecRef();
                this->LoginQuery = nullptr;
                return;
            }
            this->LoginQuery->DecRef();
            this->LoginQuery = nullptr;
            QDomElement data = revisions.at(0).toElement();
            if (Configuration::HuggleConfiguration->ParseUserConfig(data.text()))
            {
                if (this->LoadedOldConfig)
                {
                    // if we loaded the old config we write that to debug log because othewise we hardly check this
                    // piece of code really works
                    Syslog::HuggleLogs->DebugLog("We successfuly loaded and converted the old config (huggle.css) :)");
                }
                if (!Configuration::HuggleConfiguration->ProjectConfig_EnableAll)
                {
                    this->Kill();
                    /// \todo LOCALIZE ME
                    this->Update("Login failed because you don't have enable:true in your personal config");
                    return;
                }
                this->loadingForm->ModifyIcon(LOGINFORM_USERCONFIG, LoadingForm_Icon_Success);
                this->_Status = RetrievingUser;
                return;
            }
            /// \todo LOCALIZE ME
            this->Update("Login failed unable to parse the user config, see debug log for more details");
            Syslog::HuggleLogs->DebugLog(data.text());
            this->Kill();
        }
        return;
    }
    this->loadingForm->ModifyIcon(LOGINFORM_USERCONFIG, LoadingForm_Icon_Loading);
    this->Update("Retrieving user config");
    this->LoginQuery = new ApiQuery(ActionQuery);
    QString page = Configuration::HuggleConfiguration->GlobalConfig_UserConf;
    page = page.replace("$1", Configuration::HuggleConfiguration->SystemConfig_Username);
    this->LoginQuery->IncRef();
    this->LoginQuery->Parameters = "prop=revisions&rvprop=content&rvlimit=1&titles=" +
            QUrl::toPercentEncoding(page);
    this->LoginQuery->Process();
}

void Login::RetrieveUserInfo()
{
    if (this->LoginQuery != nullptr)
    {
        if (this->LoginQuery->IsProcessed())
        {
            if (this->LoginQuery->Result->Failed)
            {
                /// \todo LOCALIZE ME
                this->Update("Login failed unable to retrieve user info: " + this->LoginQuery->Result->ErrorMessage);
                this->Kill();
                this->LoginQuery->DecRef();
                this->LoginQuery = nullptr;
                return;
            }
            QDomDocument dLoginResult;
            dLoginResult.setContent(this->LoginQuery->Result->Data);
            QDomNodeList lRights_ = dLoginResult.elementsByTagName("r");
            if (lRights_.count() == 0)
            {
                Syslog::HuggleLogs->DebugLog(this->LoginQuery->Result->Data);
                this->Kill();
                /// \todo LOCALIZE ME
                this->Update("Login failed unable to retrieve user info, the api query returned no data");
                this->LoginQuery->DecRef();
                this->LoginQuery = nullptr;
                return;
            }
            int c=0;
            while(c<lRights_.count())
            {
                Configuration::HuggleConfiguration->Rights.append(lRights_.at(c).toElement().text());
                c++;
            }
            if (Configuration::HuggleConfiguration->ProjectConfig_RequireRollback &&
                !Configuration::HuggleConfiguration->Rights.contains("rollback"))
            {
                /// \todo LOCALIZE ME
                this->Update("Login failed because you don't have rollback permissions on this project");
                this->Kill();
                this->LoginQuery->DecRef();
                this->LoginQuery = nullptr;
                return;
            }
            if (Configuration::HuggleConfiguration->ProjectConfig_RequireAutoconfirmed &&
                !Configuration::HuggleConfiguration->Rights.contains("autoconfirmed"))
                //sometimes there is something like manually "confirmed", thats currently not included here
            {
                /// \todo LOCALIZE ME
                this->Update("Login failed because you are not autoconfirmed on this project");
                this->Kill();
                this->LoginQuery->DecRef();
                this->LoginQuery = nullptr;
                return;
            }

            QDomNodeList userinfos = dLoginResult.elementsByTagName("userinfo");
            this->Kill();
            this->LoginQuery->DecRef();
            this->LoginQuery = nullptr;
            int editcount = userinfos.at(0).toElement().attribute("editcount", "-1").toInt();
            if (Configuration::HuggleConfiguration->ProjectConfig_RequireEdits > editcount)
            {
                /// \todo LOCALIZE ME
                this->Update("Login failed because you don't have enough edits on this project");
                return;
            }

            /// \todo Implement check for "require-time"
            ///
            this->_Status = LoginDone;
            this->Finish();
        }
        return;
    }
    /// \todo LOCALIZE ME
    this->Update("Retrieving user info");
    this->loadingForm->ModifyIcon(LOGINFORM_USERINFO, LoadingForm_Icon_Loading);
    this->LoginQuery = new ApiQuery(ActionQuery);
    this->LoginQuery->Parameters = "meta=userinfo&format=xml&uiprop=" + QUrl::toPercentEncoding("rights|editcount");
    this->LoginQuery->IncRef();
    this->LoginQuery->Process();
}

void Login::DeveloperMode()
{
    Configuration::HuggleConfiguration->Restricted = true;
    MainWindow::HuggleMain = new MainWindow();
    MainWindow::HuggleMain->show();
    Core::HuggleCore->Main = MainWindow::HuggleMain;
    this->hide();
}

void Login::ProcessSiteInfo()
{
    if (this->qSiteInfo->IsProcessed())
    {
        //! \todo Check that request isnt failed
        QDomDocument d;
        d.setContent(this->qSiteInfo->Result->Data);
        QDomNodeList l = d.elementsByTagName("general");
        if( l.count() < 1 ) {
            //! \todo throw some exception
        }
        QDomElement item = l.at(0).toElement();
        if (item.attributes().contains("rtl")){
            //! \todo set a value and use this to determine project RTL status
        }
        this->qSiteInfo->DecRef();
        this->qSiteInfo = nullptr;
        this->loadingForm->ModifyIcon(LOGINFORM_SITEINFO,LoadingForm_Icon_Success);
    }
}

void Login::DisplayError(QString message)
{
    this->Kill();
    this->CancelLogin();
    this->Update(message);
}

void Login::Finish()
{
    // let's check if all processes are finished
    if (this->wq || this->qSiteInfo || this->_Status != LoginDone)
        return;
    // we generate a random string of same size of current password
    QString pw = "";
    this->_Status = Nothing;
    while (pw.length() < Configuration::HuggleConfiguration->TemporaryConfig_Password.length())
    {
        pw += ".";
    }
    // we no longer need a password since this
    Configuration::HuggleConfiguration->TemporaryConfig_Password = pw;
    this->ui->lineEdit_password->setText(pw);
    this->Update("Loading main huggle window");
    this->timer->stop();
    if (this->loadingForm != nullptr)
    {
        this->loadingForm->close();
        delete this->loadingForm;
        this->loadingForm = nullptr;
    }
    this->hide();
    MainWindow::HuggleMain = new MainWindow();
    MainWindow::HuggleMain->show();
    Core::HuggleCore->Main = MainWindow::HuggleMain;
    Core::HuggleCore->Main->show();
}

void Login::reject()
{
    if (this->_Status != LoginDone)
        QApplication::quit();
}

bool Login::ProcessOutput()
{
    // Check what the result was
    QString Result = this->LoginQuery->Result->Data;
    if (!Result.contains(("<login result")))
    {
        Syslog::HuggleLogs->DebugLog(Result);
        this->DisplayError("ERROR: The api.php responded with invalid text (webserver is down?), please check debug "\
                           "log for precise information");
        return false;
    }

    Result = Result.mid(Result.indexOf("result=\"") + 8);
    if (!Result.contains("\""))
    {
        Syslog::HuggleLogs->DebugLog(Result);
        this->DisplayError("ERROR: The api.php responded with invalid text (webserver is broken), please check debug "\
                           "log for precise information");
        return false;
    }

    Result = Result.mid(0, Result.indexOf("\""));

    if (Result == "Success")
    {
        return true;
    }

    if (Result == "EmptyPass")
    {
        /// \todo LOCALIZE ME
        this->DisplayError("The password you entered was empty");
        return false;
    }

    if (Result == "WrongPass")
    {
        /// \bug This sometimes doesn't work properly
        this->ui->lineEdit_password->setFocus();
        this->DisplayError(Localizations::HuggleLocalizations->Localize("login-error-password"));
        return false;
    }

    if (Result == "NoName")
    {
        /// \todo LOCALIZE ME
        this->DisplayError("You provided no correct user name for login");
        return false;
    }

    /// \todo LOCALIZE ME
    this->DisplayError("ERROR: The api.php responded with unknown result: " + Result);
    return false;
}

QString Login::GetToken()
{
    QString token = this->Token;
    if (!token.contains(Login::Test))
    {
        /// \todo LOCALIZE ME
        Syslog::HuggleLogs->Log("WARNING: the result of api request doesn't contain valid token");
        Syslog::HuggleLogs->DebugLog("The token didn't contain the correct string, token was " + token);
        return "<invalid token>";
    }
    token = token.mid(token.indexOf(Login::Test) + Login::Test.length());
    if (!token.contains("\""))
    {
        /// \todo LOCALIZE ME
        Syslog::HuggleLogs->Log("WARNING: the result of api request doesn't contain valid token");
        Syslog::HuggleLogs->DebugLog("The token didn't contain the closing mark, token was " + token);
        return "<invalid token>";
    }
    token = token.mid(0, token.indexOf("\""));
    return token;
}

void Login::on_ButtonOK_clicked()
{
    if (this->_Status == Nothing)
    {
        this->PressOK();
        return;
    }
    else
    {
        this->CancelLogin();
        this->Reset();
        return;
    }
}

void Login::on_ButtonExit_clicked()
{
    Core::HuggleCore->Shutdown();
}

void Login::OnTimerTick()
{
    if (this->wq != nullptr)
        this->RetrieveWhitelist();
    if (this->qSiteInfo != nullptr)
        this->ProcessSiteInfo();

    switch (this->_Status)
    {
        case LoggingIn:
            PerformLogin();
            break;
        case WaitingForLoginQuery:
            PerformLoginPart2();
            break;
        case WaitingForToken:
            FinishLogin();
            break;
        case RetrievingGlobalConfig:
            RetrieveGlobalConfig();
            break;
        case RetrievingProjectConfig:
            RetrieveProjectConfig();
            break;
        case RetrievingUserConfig:
            RetrieveUserConfig();
            break;
        case RetrievingUser:
            RetrieveUserInfo();
            break;
        case Refreshing:
            DB();
        case LoggedIn:
        case Nothing:
        case Cancelling:
        case LoginFailed:
        case LoginDone:
            break;
    }

    if (this->_Status == LoginFailed)
    {
        this->Enable();
        this->timer->stop();
        this->ui->ButtonOK->setText("Login");
        this->_Status = Nothing;
    } else
    {
        this->Finish();
    }
}

void Login::on_pushButton_clicked()
{
    this->Disable();
    if(this->LoginQuery != nullptr)
    {
        this->LoginQuery->DecRef();
    }
    this->LoginQuery = new ApiQuery(ActionQuery);
    this->LoginQuery->IncRef();
    this->_Status = Refreshing;
    Configuration::HuggleConfiguration->SystemConfig_UsingSSL = this->ui->checkBox->isChecked();
    this->timer->start(200);
    this->LoginQuery->OverrideWiki = Configuration::HuggleConfiguration->GlobalConfigurationWikiAddress;
    this->ui->ButtonOK->setText(Localizations::HuggleLocalizations->Localize("[[cancel]]"));
    this->LoginQuery->Parameters = "prop=revisions&format=xml&rvprop=content&rvlimit=1&titles="
                        + Configuration::HuggleConfiguration->SystemConfig_GlobalConfigWikiList;
    this->LoginQuery->Process();
}

void Login::on_Language_currentIndexChanged(const QString &arg1)
{
    if (this->Loading)
    {
        return;
    }
    QString lang = "en";
    int c = 0;
    while (c<Localizations::HuggleLocalizations->LocalizationData.count())
    {
        if (Localizations::HuggleLocalizations->LocalizationData.at(c)->LanguageID == arg1)
        {
            lang = Localizations::HuggleLocalizations->LocalizationData.at(c)->LanguageName;
            break;
        }
        c++;
    }
    Localizations::HuggleLocalizations->PreferredLanguage = lang;
    this->Localize();
}

void Huggle::Login::on_label_9_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(link);
}

void Huggle::Login::on_lineEdit_username_textChanged(const QString &arg1)
{
    Q_UNUSED( arg1 )
    Login::VerifyLogin();
}

void Huggle::Login::on_lineEdit_password_textChanged(const QString &arg1)
{
    Q_UNUSED( arg1 )
    Login::VerifyLogin();
}

void Login::VerifyLogin()
{
    if( this->ui->lineEdit_username->text() == "Developer Mode" )
    {
        this->ui->ButtonOK->setEnabled( true );
    }
    else if( this->ui->lineEdit_username->text().size() == 0 || this->ui->lineEdit_password->text().size() == 0 )
    {
        this->ui->ButtonOK->setEnabled( false );
    } else {
        this->ui->ButtonOK->setEnabled( true );
    }
}
