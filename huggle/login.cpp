//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "login.hpp"
#include "ui_login.h"

using namespace Huggle;

QString Login::Test = "<login result=\"NeedToken\" token=\"";

Login::Login(QWidget *parent) :   QDialog(parent),   ui(new Ui::Login)
{
    this->Loading = true;
    this->ui->setupUi(this);
    this->_Status = Nothing;
    this->LoadedOldConfig = false;
    this->LoginQuery = NULL;
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
    this->wq = NULL;
    if (!QSslSocket::supportsSsl())
    {
        Configuration::HuggleConfiguration->SystemConfig_UsingSSL = false;
        this->ui->checkBox->setEnabled(false);
        this->ui->checkBox->setChecked(false);
    }
    this->ui->ButtonExit->setText(Localizations::HuggleLocalizations->Localize("[[main-system-exit]]"));
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
    this->Loading = false;
    this->Localize();
}

Login::~Login()
{
    if (this->LoginQuery != NULL)
    {
        this->LoginQuery->SafeDelete();
    }
    if (this->wq != NULL)
    {
        this->wq->SafeDelete();
    }
    delete this->Updater;
    delete this->ui;
    delete this->timer;
}

void Login::Progress(const int progress)
{
    this->ui->progressBar->setValue(progress);
}

void Login::Localize()
{
    this->ui->ButtonExit->setText(Localizations::HuggleLocalizations->Localize("[[main-system-exit]]"));
    this->ui->ButtonOK->setText(Localizations::HuggleLocalizations->Localize("[[login-start]]"));
    this->ui->checkBox->setText(Localizations::HuggleLocalizations->Localize("[[login-ssl]]"));
    this->ui->label_2->setText(Localizations::HuggleLocalizations->Localize("[[login-username]]"));
    this->ui->pushButton->setText(Localizations::HuggleLocalizations->Localize("[[reload]]"));
    this->ui->label_3->setText(Localizations::HuggleLocalizations->Localize("[[login-username]]"));
    this->ui->label_4->setText(Localizations::HuggleLocalizations->Localize("[[login-project]]"));
    this->ui->label_5->setText(Localizations::HuggleLocalizations->Localize("[[login-language]]"));
    this->ui->label_7->setText(Localizations::HuggleLocalizations->Localize("[[login-password"));
    this->ui->label_6->setText(Localizations::HuggleLocalizations->Localize("login-intro"));
}

void Login::Reset()
{
    this->ui->label_6->setText(Localizations::HuggleLocalizations->Localize("[[login-intro]]"));
}

void Login::CancelLogin()
{
    this->timer->stop();
    this->ui->progressBar->setValue(0);
    this->Enable();
    this->_Status = Nothing;
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
    {
        this->ui->Project->setCurrentIndex(Huggle::Configuration::HuggleConfiguration->IndexOfLastWiki);
    } else
    {
        this->ui->Project->setCurrentIndex(0);
    }
}

void Login::DB()
{
    if (this->LoginQuery == NULL)
    {
        return;
    }

    if (this->LoginQuery->IsProcessed())
    {
        Syslog::HuggleLogs->DebugLog(LoginQuery->Result->Data, 2);
        QDomDocument d;
        d.setContent(this->LoginQuery->Result->Data);
        QDomNodeList l = d.elementsByTagName("rev");
        if (l.count() > 0)
        {
            if (QFile().exists(Configuration::HuggleConfiguration->WikiDB))
            {
                QFile().remove(Configuration::HuggleConfiguration->WikiDB);
            }
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
}

void Login::Disable()
{
    this->ui->lineEdit_oauth_username->setDisabled(true);
    this->ui->Language->setDisabled(true);
    this->ui->Project->setDisabled(true);
    this->ui->checkBox->setDisabled(true);
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
        DeveloperMode();
        return;
    }
    Configuration::HuggleConfiguration->SystemConfig_Username = ui->lineEdit_username->text();
    Configuration::HuggleConfiguration->TemporaryConfig_Password = ui->lineEdit_password->text();
    this->_Status = LoggingIn;
    this->Disable();
    this->ui->ButtonOK->setText(Localizations::HuggleLocalizations->Localize("[[cancel]]"));
    // First of all, we need to login to the site
    this->timer->start(200);
}

void Login::PerformLogin()
{
    this->ui->label_6->setText(Localizations::HuggleLocalizations->Localize("[[login-progress-start]]"));
    this->Progress(8);
    // we create an api request to login
    this->LoginQuery = new ApiQuery();
    this->LoginQuery->SetAction(ActionLogin);
    this->LoginQuery->Parameters = "lgname=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->SystemConfig_Username);
    this->LoginQuery->HiddenQuery = true;
    this->LoginQuery->UsingPOST = true;
    this->LoginQuery->Process();
    this->_Status = WaitingForLoginQuery;
}

void Login::FinishLogin()
{
    if (!this->LoginQuery->IsProcessed())
    {
        return;
    }
    if (this->LoginQuery->Result->Failed)
    {
        this->ui->label_6->setText(Localizations::HuggleLocalizations->Localize("[[login-fail]]") + ": "
                                   + this->LoginQuery->Result->ErrorMessage);
        this->Progress(0);
        this->_Status = LoginFailed;
        this->LoginQuery->SafeDelete();
        this->LoginQuery = NULL;
        return;
    }
    this->Progress(18);
    this->Token = this->LoginQuery->Result->Data;
    this->_Status = WaitingForToken;
    this->LoginQuery->SafeDelete();
    this->Token = GetToken();
    this->LoginQuery = new ApiQuery();
    this->LoginQuery->SetAction(ActionLogin);
    this->LoginQuery->HiddenQuery = true;
    this->LoginQuery->Parameters = "lgname=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->SystemConfig_Username)
            + "&lgpassword=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->TemporaryConfig_Password) + "&lgtoken=" + Token ;
    this->LoginQuery->UsingPOST = true;
    this->LoginQuery->Process();
}

void Login::RetrieveGlobalConfig()
{
    if (this->LoginQuery != NULL)
    {
        if (this->LoginQuery->IsProcessed())
        {
            if (this->LoginQuery->Result->Failed)
            {
                this->ui->label_6->setText(Localizations::HuggleLocalizations->Localize("[[login-error-global]]") + ": "
                                           + this->LoginQuery->Result->ErrorMessage);
                this->Progress(0);
                this->_Status = LoginFailed;
                this->LoginQuery->SafeDelete();
                this->LoginQuery = NULL;
                return;
            }
            QDomDocument d;
            d.setContent(this->LoginQuery->Result->Data);
            QDomNodeList l = d.elementsByTagName("rev");
            if (l.count() == 0)
            {
                this->ui->label_6->setText("Login failed unable to retrieve global config, the api query returned no data");
                this->Progress(0);
                this->_Status = LoginFailed;
                this->LoginQuery->SafeDelete();
                this->LoginQuery = NULL;
                return;
            }
            QDomElement data = l.at(0).toElement();
            if (Configuration::HuggleConfiguration->ParseGlobalConfig(data.text()))
            {
                if (!Configuration::HuggleConfiguration->GlobalConfig_EnableAll)
                {
                    this->ui->label_6->setText(Localizations::HuggleLocalizations->Localize("login-error-alldisabled"));
                    this->Progress(0);
                    this->_Status = LoginFailed;
                    this->LoginQuery->SafeDelete();
                    this->LoginQuery = NULL;
                    return;
                }
                this->LoginQuery->SafeDelete();
                this->LoginQuery = NULL;
                this->_Status = RetrievingWhitelist;
                return;
            }
            this->ui->label_6->setText(Localizations::HuggleLocalizations->Localize("login-error-global"));
            Syslog::HuggleLogs->DebugLog(data.text());
            this->Progress(0);
            this->_Status = LoginFailed;
            this->LoginQuery->SafeDelete();
            this->LoginQuery = NULL;
            return;
        }
        return;
    }
    this->Progress(40);
    this->ui->label_6->setText(Localizations::HuggleLocalizations->Localize("[[login-progress-global]]"));
    this->LoginQuery = new ApiQuery();
    this->LoginQuery->SetAction(ActionQuery);
    this->LoginQuery->OverrideWiki = Configuration::HuggleConfiguration->GlobalConfigurationWikiAddress;
    this->LoginQuery->Parameters = "prop=revisions&format=xml&rvprop=content&rvlimit=1&titles=Huggle/Config";
    this->LoginQuery->Process();
}

void Login::FinishToken()
{
    if (!this->LoginQuery->IsProcessed())
    {
        return;
    }
    if (this->LoginQuery->Result->Failed)
    {
        this->ui->label_6->setText("Login failed: " + this->LoginQuery->Result->ErrorMessage);
        this->Progress(0);
        this->_Status = LoginFailed;
        this->LoginQuery->SafeDelete();
        this->LoginQuery = NULL;
        return;
    }
    this->Progress(28);

    // Assume login was successful
    if (this->ProcessOutput())
    {
        this->_Status = RetrievingGlobalConfig;
    }
    // that's all
    this->LoginQuery->SafeDelete();
    this->LoginQuery = NULL;
}

void Login::RetrieveWhitelist()
{
    if (this->wq != NULL)
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
                int c = 0;
                while (c < Configuration::HuggleConfiguration->WhiteList.count())
                {
                    if (Configuration::HuggleConfiguration->WhiteList.at(c) == "")
                    {
                        Configuration::HuggleConfiguration->WhiteList.removeAt(c);
                        continue;
                    }
                    c++;
                }
            }
            this->wq->SafeDelete();
            this->wq = NULL;
            // Now local config
            this->_Status = RetrievingProjectConfig;
            return;
        }
        return;
    }
    this->Progress(52);
    this->ui->label_6->setText(Localizations::HuggleLocalizations->Localize("login-progress-whitelist"));
    this->wq = new WLQuery();
    this->wq->RetryOnTimeoutFailure = false;
    this->wq->Process();
    return;
}

void Login::RetrieveProjectConfig()
{
    if (this->LoginQuery != NULL)
    {
        if (this->LoginQuery->IsProcessed())
        {
            if (this->LoginQuery->Result->Failed)
            {
                this->ui->label_6->setText(Localizations::HuggleLocalizations->Localize("login-error-config",
                                                                   this->LoginQuery->Result->ErrorMessage));
                this->Progress(0);
                this->_Status = LoginFailed;
                this->LoginQuery->SafeDelete();
                this->LoginQuery = NULL;
                return;
            }
            QDomDocument d;
            d.setContent(this->LoginQuery->Result->Data);
            QDomNodeList l = d.elementsByTagName("rev");
            if (l.count() == 0)
            {
                this->ui->label_6->setText(Localizations::HuggleLocalizations->Localize("login-error-config",
                                                                        "the api query returned no data"));
                this->Progress(0);
                this->_Status = LoginFailed;
                this->LoginQuery->SafeDelete();
                this->LoginQuery = NULL;
                return;
            }
            QDomElement data = l.at(0).toElement();
            if (Configuration::HuggleConfiguration->ParseProjectConfig(data.text()))
            {
                if (!Configuration::HuggleConfiguration->ProjectConfig_EnableAll)
                {
                    this->ui->label_6->setText(Localizations::HuggleLocalizations->Localize("login-error-projdisabled"));
                    this->Progress(0);
                    this->_Status = LoginFailed;
                    this->LoginQuery->SafeDelete();
                    this->LoginQuery = NULL;
                    return;
                }
                this->LoginQuery->SafeDelete();
                this->LoginQuery = NULL;
                this->_Status = RetrievingUserConfig;
                return;
            }
            this->ui->label_6->setText(Localizations::HuggleLocalizations->Localize("login-error-config"));
            Syslog::HuggleLogs->DebugLog(data.text());
            this->Progress(0);
            this->_Status = LoginFailed;
            this->LoginQuery->SafeDelete();
            this->LoginQuery = NULL;
            return;
        }
        return;
    }
    this->Progress(68);
    this->ui->label_6->setText(Localizations::HuggleLocalizations->Localize("login-progress-config"));
    this->LoginQuery = new ApiQuery();
    this->LoginQuery->SetAction(ActionQuery);
    this->LoginQuery->Parameters = "prop=revisions&format=xml&rvprop=content&rvlimit=1&titles=Project:Huggle/Config";
    this->LoginQuery->Process();
}

void Login::RetrieveUserConfig()
{
    if (this->LoginQuery != NULL)
    {
        if (this->LoginQuery->IsProcessed())
        {
            if (this->LoginQuery->Result->Failed)
            {
                /// \todo LOCALIZE ME
                this->ui->label_6->setText("Login failed unable to retrieve user config: " +
                                           this->LoginQuery->Result->ErrorMessage);
                this->Progress(0);
                this->_Status = LoginFailed;
                this->LoginQuery->SafeDelete();
                this->LoginQuery = NULL;
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
                    this->LoginQuery->SafeDelete();
                    this->LoginQuery = NULL;
                    /// \todo LOCALIZE ME
                    this->ui->label_6->setText("Retrieving user config from old location");
                    this->LoginQuery = new ApiQuery();
                    QString page = Configuration::HuggleConfiguration->GlobalConfig_UserConf_old;
                    page = page.replace("$1", Configuration::HuggleConfiguration->SystemConfig_Username);
                    this->LoginQuery->SetAction(ActionQuery);
                    this->LoginQuery->Parameters = "prop=revisions&rvprop=content&rvlimit=1&titles=" +
                            QUrl::toPercentEncoding(page);
                    this->LoginQuery->Process();
                    return;
                }

                if (!Configuration::HuggleConfiguration->ProjectConfig_RequireConfig)
                {
                    // we don't care if user config is missing or not
                    this->LoginQuery->SafeDelete();
                    this->LoginQuery = NULL;
                    this->_Status = RetrievingUser;
                    return;
                }
                Syslog::HuggleLogs->DebugLog(this->LoginQuery->Result->Data);
                /// \todo LOCALIZE ME
                this->ui->label_6->setText("Login failed unable to retrieve user config, did you create huggle3.css "\
                                           "in your userspace? (Special:MyPage/huggle3.css is missing)");
                this->Progress(0);
                this->_Status = LoginFailed;
                this->LoginQuery->SafeDelete();
                this->LoginQuery = NULL;
                return;
            }
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
                    /// \todo LOCALIZE ME
                    this->ui->label_6->setText("Login failed because you don't have enable:true in your personal config");
                    this->Progress(0);
                    this->_Status = LoginFailed;
                    this->LoginQuery->SafeDelete();
                    this->LoginQuery = NULL;
                    return;
                }
                this->LoginQuery->SafeDelete();
                this->LoginQuery = NULL;
                this->_Status = RetrievingUser;
                return;
            }
            /// \todo LOCALIZE ME
            this->ui->label_6->setText("Login failed unable to parse the user config, see debug log for more details");
            Syslog::HuggleLogs->DebugLog(data.text());
            this->Progress(0);
            this->_Status = LoginFailed;
            this->LoginQuery->SafeDelete();
            this->LoginQuery = NULL;
            return;
        }
        return;
    }
    this->Progress(82);
    this->ui->label_6->setText("Retrieving user config");
    this->LoginQuery = new ApiQuery();
    QString page = Configuration::HuggleConfiguration->GlobalConfig_UserConf;
    page = page.replace("$1", Configuration::HuggleConfiguration->SystemConfig_Username);
    this->LoginQuery->SetAction(ActionQuery);
    this->LoginQuery->Parameters = "prop=revisions&rvprop=content&rvlimit=1&titles=" +
            QUrl::toPercentEncoding(page);
    this->LoginQuery->Process();
}

void Login::RetrieveUserInfo()
{
    if (this->LoginQuery != NULL)
    {
        if (this->LoginQuery->IsProcessed())
        {
            if (this->LoginQuery->Result->Failed)
            {
                /// \todo LOCALIZE ME
                this->ui->label_6->setText("Login failed unable to retrieve user info: "
                                           + this->LoginQuery->Result->ErrorMessage);
                this->Progress(0);
                this->_Status = LoginFailed;
                this->LoginQuery->SafeDelete();
                this->LoginQuery = NULL;
                return;
            }
            QDomDocument dLoginResult;
            dLoginResult.setContent(this->LoginQuery->Result->Data);
            QDomNodeList lRights_ = dLoginResult.elementsByTagName("r");
            if (lRights_.count() == 0)
            {
                Syslog::HuggleLogs->DebugLog(this->LoginQuery->Result->Data);
                /// \todo LOCALIZE ME
                this->ui->label_6->setText("Login failed unable to retrieve user info, the api query returned no data");
                this->Progress(0);
                this->_Status = LoginFailed;
                this->LoginQuery->SafeDelete();
                this->LoginQuery = NULL;
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
                this->ui->label_6->setText("Login failed because you don't have rollback permissions on this project");
                this->Progress(0);
                this->_Status = LoginFailed;
                this->LoginQuery->SafeDelete();
                this->LoginQuery = NULL;
                return;
            }
            if (Configuration::HuggleConfiguration->ProjectConfig_RequireAutoconfirmed &&
                !Configuration::HuggleConfiguration->Rights.contains("autoconfirmed"))
                //sometimes there is something like manually "confirmed", thats currently not included here
            {
                /// \todo LOCALIZE ME
                this->ui->label_6->setText("Login failed because you are not autoconfirmed on this project");
                this->Progress(0);
                this->_Status = LoginFailed;
                this->LoginQuery->SafeDelete();
                this->LoginQuery = NULL;
                return;
            }

            QDomNodeList userinfos = dLoginResult.elementsByTagName("userinfo");
            int editcount = userinfos.at(0).toElement().attribute("editcount", "-1").toInt();
            if (Configuration::HuggleConfiguration->ProjectConfig_RequireEdits > editcount)
            {
                /// \todo LOCALIZE ME
                this->ui->label_6->setText("Login failed because you don't have enough edits on this project");
                this->Progress(0);
                this->_Status = LoginFailed;
                this->LoginQuery->SafeDelete();
                this->LoginQuery = NULL;
                return;
            }

            /// \todo Implement check for "require-time"

            this->LoginQuery->SafeDelete();
            this->LoginQuery = NULL;
            this->_Status = LoginDone;
            this->Finish();
            return;
        }
        return;
    }
    this->Progress(96);
    /// \todo LOCALIZE ME
    this->ui->label_6->setText("Retrieving user info");
    this->LoginQuery = new ApiQuery();
    this->LoginQuery->SetAction(ActionQuery);
    this->LoginQuery->Parameters = "meta=userinfo&format=xml&uiprop=" + QUrl::toPercentEncoding("rights|editcount");
    this->LoginQuery->Process();
}

void Login::DeveloperMode()
{
    Configuration::HuggleConfiguration->Restricted = true;
    Core::HuggleCore->Main = new MainWindow();
    Core::HuggleCore->Main->show();
    this->hide();
}

void Login::DisplayError(QString message)
{
    this->_Status = LoginFailed;
    Syslog::HuggleLogs->DebugLog(this->LoginQuery->Result->Data);
    this->ui->label_6->setText(message);
    this->CancelLogin();
}

void Login::Finish()
{
    // we generate a random string of same size of current password
    QString pw = "";
    while (pw.length() < Configuration::HuggleConfiguration->TemporaryConfig_Password.length())
    {
        pw += ".";
    }
    // we no longer need a password since this
    Configuration::HuggleConfiguration->TemporaryConfig_Password = pw;
    this->ui->lineEdit_password->setText(pw);
    this->Progress(100);
    this->ui->label_6->setText("Loading main huggle window");
    this->timer->stop();
    Core::HuggleCore->Main = new MainWindow();
    this->hide();
    Core::HuggleCore->Main->show();
}

void Login::reject()
{
    if (this->_Status != LoginDone)
    {
        QApplication::quit();
    }
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
    switch (this->_Status)
    {
        case LoggingIn:
            PerformLogin();
            break;
        case WaitingForLoginQuery:
            FinishLogin();
            break;
        case RetrievingWhitelist:
            RetrieveWhitelist();
            break;
        case WaitingForToken:
            FinishToken();
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
    }
}

void Login::on_pushButton_clicked()
{
    this->Disable();
    if(this->LoginQuery != NULL)
    {
        this->LoginQuery->SafeDelete();
    }
    this->LoginQuery = new ApiQuery();
    this->_Status = Refreshing;
    Configuration::HuggleConfiguration->SystemConfig_UsingSSL = ui->checkBox->isChecked();
    this->LoginQuery->SetAction(ActionQuery);
    this->timer->start(200);
    this->LoginQuery->OverrideWiki = Configuration::HuggleConfiguration->GlobalConfigurationWikiAddress;
    this->ui->ButtonOK->setText(Localizations::HuggleLocalizations->Localize("[[cancel]]"));
    this->LoginQuery->Parameters = "prop=revisions&format=xml&rvprop=content&rvlimit=1&titles="
                        + Configuration::HuggleConfiguration->SystemConfig_GlobalConfigWikiList;
    this->LoginQuery->Process();
}

void Login::on_Language_currentIndexChanged(const QString &arg1)
{
    if (Loading)
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
