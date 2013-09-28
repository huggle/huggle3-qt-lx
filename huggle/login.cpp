//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "login.h"
#include "ui_login.h"

QString Login::Test = "<login result=\"NeedToken\" token=\"";

Login::Login(QWidget *parent) :   QDialog(parent),   ui(new Ui::Login)
{
    ui->setupUi(this);
    this->_Status = Nothing;
    this->LoginQuery = NULL;
    this->timer = new QTimer(this);
    connect(this->timer, SIGNAL(timeout()), this, SLOT(on_Time()));
    this->setWindowTitle("Huggle 3 QT [" + Configuration::HuggleVersion + "]");
    this->Reset();
    ui->checkBox->setChecked(Configuration::UsingSSL);
    // set the language to dummy english
    int l=0;
    while (l<Core::LocalizationData.count())
    {
        ui->Language->addItem(Core::LocalizationData.at(l)->LanguageName);
        l++;
    }
    ui->Language->setCurrentIndex(0);
    int current = 0;
    while (current < Configuration::ProjectList.size())
    {
        ui->Project->addItem(Configuration::ProjectList.at(current)->Name);
        current++;
    }
    ui->Project->setCurrentIndex(0);
    wq = NULL;
    if (!QSslSocket::supportsSsl())
    {
        ui->checkBox->setEnabled(false);
        ui->checkBox->setChecked(false);
    }
    ui->ButtonExit->setText(Core::Localize("[[main-system-exit]]"));
}

Login::~Login()
{
    delete wq;
    delete LoginQuery;
    delete ui;
    delete timer;
}

void Login::Progress(int progress)
{
    ui->progressBar->setValue(progress);
}

void Login::Reset()
{
    ui->label_6->setText("Please enter your wikipedia username and pick a project. The authentication will be processed using OAuth.");
}

void Login::CancelLogin()
{
    this->timer->stop();
    ui->progressBar->setValue(0);
    this->Enable();
    this->_Status = Nothing;
    ui->ButtonOK->setText("Login");
    this->Reset();
}

void Login::Enable()
{
    ui->lineEdit->setEnabled(true);
    ui->Language->setEnabled(true);
    ui->Project->setEnabled(true);
    ui->checkBox->setEnabled(QSslSocket::supportsSsl());
    ui->lineEdit_2->setEnabled(true);
    ui->ButtonExit->setEnabled(true);
    ui->lineEdit_3->setEnabled(true);
    ui->pushButton->setEnabled(true);
}

void Login::Disable()
{
    ui->lineEdit->setDisabled(true);
    ui->Language->setDisabled(true);
    ui->Project->setDisabled(true);
    ui->checkBox->setDisabled(true);
    ui->ButtonExit->setDisabled(true);
    ui->lineEdit_3->setDisabled(true);
    ui->lineEdit_2->setDisabled(true);
    ui->pushButton->setDisabled(true);
}

void Login::PressOK()
{
    if (ui->tab->isVisible())
    {
        QMessageBox mb;
        mb.setWindowTitle("Function not supported");
        mb.setText("This function is not available for wmf wikis in this moment");
        mb.exec();
        //mb.setStyle(QStyle::SP_MessageBoxCritical);
        return;
    }
    Configuration::Project = WikiSite(Configuration::ProjectList.at(ui->Project->currentIndex()));
    Configuration::UsingSSL = ui->checkBox->isChecked();
    if (ui->lineEdit_2->text() == "Developer Mode")
    {
        DeveloperMode();
        return;
    }
    Configuration::UserName = ui->lineEdit_2->text();
    Configuration::Password = ui->lineEdit_3->text();
    this->_Status = LoggingIn;
    this->Disable();
    ui->ButtonOK->setText("Cancel");
    // First of all, we need to login to the site
    this->timer->start(200);
    //this->Thread->start();
}

void Login::PerformLogin()
{
    ui->label_6->setText("Logging in");
    this->Progress(8);
    // we create an api request to login
    this->LoginQuery = new ApiQuery();
    this->LoginQuery->SetAction(ActionLogin);
    this->LoginQuery->Parameters = "lgname=" + QUrl::toPercentEncoding(Configuration::UserName);
    this->LoginQuery->UsingPOST = true;
    this->LoginQuery->Process();
    this->_Status = WaitingForLoginQuery;
}

void Login::FinishLogin()
{
    if (!this->LoginQuery->Processed())
    {
        return;
    }
    if (this->LoginQuery->Result->Failed)
    {
        ui->label_6->setText("Login failed: " + this->LoginQuery->Result->ErrorMessage);
        this->Progress(0);
        this->_Status = LoginFailed;
        delete this->LoginQuery;
        this->LoginQuery = NULL;
        return;
    }
    this->Progress(18);
    Core::DebugLog(this->LoginQuery->Result->Data, 6);
    this->Token = this->LoginQuery->Result->Data;
    this->_Status = WaitingForToken;
    delete this->LoginQuery;
    this->Token = GetToken();
    this->LoginQuery = new ApiQuery();
    this->LoginQuery->SetAction(ActionLogin);
    this->LoginQuery->Parameters = "lgname=" + QUrl::toPercentEncoding(Configuration::UserName)
            + "&lgpassword=" + QUrl::toPercentEncoding(Configuration::Password) + "&lgtoken=" + Token ;
    this->LoginQuery->UsingPOST = true;
    this->LoginQuery->Process();
}

void Login::RetrieveGlobalConfig()
{
    if (this->LoginQuery != NULL)
    {
        if (this->LoginQuery->Processed())
        {
            if (this->LoginQuery->Result->Failed)
            {
                ui->label_6->setText("Login failed unable to retrieve global config: " + this->LoginQuery->Result->ErrorMessage);
                this->Progress(0);
                this->_Status = LoginFailed;
                delete this->LoginQuery;
                this->LoginQuery = NULL;
                return;
            }
            QDomDocument d;
            d.setContent(this->LoginQuery->Result->Data);
            QDomNodeList l = d.elementsByTagName("rev");
            if (l.count() == 0)
            {
                ui->label_6->setText("Login failed unable to retrieve global config, the api query returned no data");
                this->Progress(0);
                this->_Status = LoginFailed;
                delete this->LoginQuery;
                this->LoginQuery = NULL;
                return;
            }
            QDomElement data = l.at(0).toElement();
            if (Core::ParseGlobalConfig(data.text()))
            {
                if (!Configuration::GlobalConfig_EnableAll)
                {
                    ui->label_6->setText("Login failed because huggle is globally disabled");
                    this->Progress(0);
                    this->_Status = LoginFailed;
                    delete this->LoginQuery;
                    this->LoginQuery = NULL;
                    return;
                }
                delete this->LoginQuery;
                this->LoginQuery = NULL;
                this->_Status = RetrievingWhitelist;
                return;
            }
            ui->label_6->setText("Login failed unable to parse the global config, see debug log for more details");
            Core::DebugLog(data.text());
            this->Progress(0);
            this->_Status = LoginFailed;
            delete this->LoginQuery;
            this->LoginQuery = NULL;
            return;
        }
        return;
    }
    this->Progress(40);
    ui->label_6->setText("Retrieving global config");
    this->LoginQuery = new ApiQuery();
    this->LoginQuery->SetAction(ActionQuery);
    this->LoginQuery->OverrideWiki = Configuration::GlobalConfigurationWikiAddress;
    this->LoginQuery->Parameters = "prop=revisions&format=xml&rvprop=content&rvlimit=1&titles=Huggle/Config";
    this->LoginQuery->Process();
}

void Login::FinishToken()
{
    if (!this->LoginQuery->Processed())
    {
        return;
    }
    if (this->LoginQuery->Result->Failed)
    {
        ui->label_6->setText("Login failed: " + this->LoginQuery->Result->ErrorMessage);
        this->Progress(0);
        this->_Status = LoginFailed;
        delete this->LoginQuery;
        this->LoginQuery = NULL;
        return;
    }
    this->Progress(28);

    // this is last step but in fact we should load the config now
    Core::DebugLog(this->LoginQuery->Result->Data, 6);

    // Assume login was successful
    if (this->ProcessOutput())
    {
        this->_Status = RetrievingGlobalConfig;
    }
    // that's all
    delete this->LoginQuery;
    this->LoginQuery = NULL;
}

void Login::RetrieveWhitelist()
{
    if (wq != NULL)
    {
        if (wq->Processed())
        {
            QString list = wq->Result->Data;
            list = list.replace("<!-- list -->", "");
            Configuration::WhiteList = list.split("|");
            int c = 0;
            while (c < Configuration::WhiteList.count())
            {
                if (Configuration::WhiteList.at(c) == "")
                {
                    Configuration::WhiteList.removeAt(c);
                    continue;
                }
                c++;
            }
            delete wq;
            wq = NULL;
            // Now global config
            this->_Status = RetrievingLocalConfig;
            return;
        }
        return;
    }
    this->Progress(52);
    ui->label_6->setText("Retrieving whitelist");
    wq = new WLQuery();
    wq->Process();
    return;
}

void Login::RetrieveLocalConfig()
{
    if (this->LoginQuery != NULL)
    {
        if (this->LoginQuery->Processed())
        {
            if (this->LoginQuery->Result->Failed)
            {
                ui->label_6->setText("Login failed unable to retrieve local config: " + this->LoginQuery->Result->ErrorMessage);
                this->Progress(0);
                this->_Status = LoginFailed;
                delete this->LoginQuery;
                this->LoginQuery = NULL;
                return;
            }
            QDomDocument d;
            d.setContent(this->LoginQuery->Result->Data);
            QDomNodeList l = d.elementsByTagName("rev");
            if (l.count() == 0)
            {
                ui->label_6->setText("Login failed unable to retrieve local config, the api query returned no data");
                this->Progress(0);
                this->_Status = LoginFailed;
                delete this->LoginQuery;
                this->LoginQuery = NULL;
                return;
            }
            QDomElement data = l.at(0).toElement();
            if (Core::ParseLocalConfig(data.text()))
            {
                if (!Configuration::LocalConfig_EnableAll)
                {
                    ui->label_6->setText("Login failed because huggle is disabled");
                    this->Progress(0);
                    this->_Status = LoginFailed;
                    delete this->LoginQuery;
                    this->LoginQuery = NULL;
                    return;
                }
                delete this->LoginQuery;
                this->LoginQuery = NULL;
                this->_Status = RetrievingUserConfig;
                return;
            }
            ui->label_6->setText("Login failed unable to parse the local config, see debug log for more details");
            Core::DebugLog(data.text());
            this->Progress(0);
            this->_Status = LoginFailed;
            delete this->LoginQuery;
            this->LoginQuery = NULL;
            return;
        }
        return;
    }
    this->Progress(68);
    ui->label_6->setText("Retrieving local config");
    this->LoginQuery = new ApiQuery();
    this->LoginQuery->SetAction(ActionQuery);
    this->LoginQuery->Parameters = "prop=revisions&format=xml&rvprop=content&rvlimit=1&titles=Project:Huggle/Config";
    this->LoginQuery->Process();
}

void Login::RetrievePrivateConfig()
{
    if (this->LoginQuery != NULL)
    {
        if (this->LoginQuery->Processed())
        {
            if (this->LoginQuery->Result->Failed)
            {
                ui->label_6->setText("Login failed unable to retrieve user config: " + this->LoginQuery->Result->ErrorMessage);
                this->Progress(0);
                this->_Status = LoginFailed;
                delete this->LoginQuery;
                this->LoginQuery = NULL;
                return;
            }
            QDomDocument d;
            d.setContent(this->LoginQuery->Result->Data);
            QDomNodeList l = d.elementsByTagName("rev");
            if (l.count() == 0)
            {
                Core::DebugLog(this->LoginQuery->Result->Data);
                ui->label_6->setText("Login failed unable to retrieve user config, the api query returned no data");
                this->Progress(0);
                this->_Status = LoginFailed;
                delete this->LoginQuery;
                this->LoginQuery = NULL;
                return;
            }
            QDomElement data = l.at(0).toElement();
            if (Core::ParseUserConfig(data.text()))
            {
                if (!Configuration::LocalConfig_EnableAll)
                {
                    ui->label_6->setText("Login failed because you don't have enable:true in your personal config");
                    this->Progress(0);
                    this->_Status = LoginFailed;
                    delete this->LoginQuery;
                    this->LoginQuery = NULL;
                    return;
                }
                delete this->LoginQuery;
                this->LoginQuery = NULL;
                this->_Status = RetrievingUser;
                return;
            }
            ui->label_6->setText("Login failed unable to parse the user config, see debug log for more details");
            Core::DebugLog(data.text());
            this->Progress(0);
            this->_Status = LoginFailed;
            delete this->LoginQuery;
            this->LoginQuery = NULL;
            return;
        }
        return;
    }
    this->Progress(82);
    ui->label_6->setText("Retrieving user config");
    this->LoginQuery = new ApiQuery();
    QString page = Configuration::GlobalConfig_UserConf;
    page = page.replace("$1", Configuration::UserName);
    this->LoginQuery->SetAction(ActionQuery);
    this->LoginQuery->Parameters = "prop=revisions&rvprop=content&rvlimit=1&titles=" +
            QUrl::toPercentEncoding(page);
    this->LoginQuery->Process();
}

void Login::RetrieveUserInfo()
{
    if (this->LoginQuery != NULL)
    {
        if (this->LoginQuery->Processed())
        {
            if (this->LoginQuery->Result->Failed)
            {
                ui->label_6->setText("Login failed unable to retrieve user info: " + this->LoginQuery->Result->ErrorMessage);
                this->Progress(0);
                this->_Status = LoginFailed;
                delete this->LoginQuery;
                this->LoginQuery = NULL;
                return;
            }
            QDomDocument d;
            d.setContent(this->LoginQuery->Result->Data);
            QDomNodeList l = d.elementsByTagName("r");
            if (l.count() == 0)
            {
                Core::DebugLog(this->LoginQuery->Result->Data);
                ui->label_6->setText("Login failed unable to retrieve user info, the api query returned no data");
                this->Progress(0);
                this->_Status = LoginFailed;
                delete this->LoginQuery;
                this->LoginQuery = NULL;
                return;
            }
            int c=0;
            while(c<l.count())
            {
                Configuration::Rights.append(l.at(c).toElement().text());
                c++;
            }
            if (Configuration::LocalConfig_RequireRollback && !Configuration::Rights.contains("rollback"))
            {
                    ui->label_6->setText("Login failed because you don't have rollback permissions on this project");
                    this->Progress(0);
                    this->_Status = LoginFailed;
                    delete this->LoginQuery;
                    this->LoginQuery = NULL;
                    return;
            }
            delete this->LoginQuery;
            this->LoginQuery = NULL;
            this->_Status = LoginDone;
            Finish();
            return;
        }
        return;
    }
    this->Progress(96);
    ui->label_6->setText("Retrieving user info");
    this->LoginQuery = new ApiQuery();
    this->LoginQuery->SetAction(ActionQuery);
    this->LoginQuery->Parameters = "meta=userinfo&format=xml&uiprop=rights";
    this->LoginQuery->Process();
}

void Login::DeveloperMode()
{
    Configuration::Restricted = true;
    Core::Main = new MainWindow();
    Core::Main->show();
    this->hide();
}

void Login::DisplayError(QString message)
{
    this->_Status = LoginFailed;
    Core::DebugLog(this->LoginQuery->Result->Data);
    ui->label_6->setText(message);
    this->CancelLogin();
}

void Login::Finish()
{
    this->timer->stop();
    Core::Main = new MainWindow();
    this->hide();
    Core::Main->show();
}

bool Login::ProcessOutput()
{
    // Check what the result was
    QString Result = this->LoginQuery->Result->Data;
    if (!Result.contains(("<login result")))
    {
        DisplayError("ERROR: The api.php responded with invalid text (webserver is down?), please check debug log for precise information");
        return false;
    }

    Result = Result.mid(Result.indexOf("result=\"") + 8);
    if (!Result.contains("\""))
    {
        DisplayError("ERROR: The api.php responded with invalid text (webserver is broken), please check debug log for precise information");
        return false;
    }

    Result = Result.mid(0, Result.indexOf("\""));

    if (Result == "Success")
    {
        return true;
    }

    if (Result == "EmptyPass")
    {
        DisplayError("The password you entered was empty");
        return false;
    }

    if (Result == "WrongPass")
    {
        DisplayError("Your password is not correct");
        return false;
    }

    if (Result == "NoName")
    {
        DisplayError("You provided no correct user name for login");
        return false;
    }

    DisplayError("ERROR: The api.php responded with unknown result: " + Result);
    return false;
}

QString Login::GetToken()
{
    QString token = this->Token;
    if (!token.contains(Login::Test))
    {
        // this is invalid token?
        Core::Log("WARNING: the result of api request doesn't contain valid token");
        Core::DebugLog("The token didn't contain the correct string, token was " + token);
        return "<invalid token>";
    }
    token = token.mid(token.indexOf(Login::Test) + Login::Test.length());
    if (!token.contains("\""))
    {
        // this is invalid token?
        Core::Log("WARNING: the result of api request doesn't contain valid token");
        Core::DebugLog("The token didn't contain the closing mark, token was " + token);
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
        return;
    }
}

void Login::on_ButtonExit_clicked()
{
    Core::Shutdown();
}

void Login::on_Login_destroyed()
{
    QApplication::quit();
}

void Login::on_Time()
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
    case RetrievingLocalConfig:
        RetrieveLocalConfig();
        break;
    case RetrievingUserConfig:
        RetrievePrivateConfig();
        break;
    case RetrievingUser:
        RetrieveUserInfo();
        break;
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
        timer->stop();
        ui->ButtonOK->setText("Login");
        this->_Status = Nothing;
    }
}

void Login::on_pushButton_clicked()
{
    this->Disable();
}
