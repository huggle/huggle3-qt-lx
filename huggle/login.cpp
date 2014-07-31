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
#include <QCheckBox>
#include <QUrl>
#include <QDesktopServices>
#include <QtXml>
#include "core.hpp"
#include "configuration.hpp"
#include "exception.hpp"
#include "syslog.hpp"
#include "mainwindow.hpp"
#include "mediawiki.hpp"
#include "localization.hpp"
#include "loadingform.hpp"
#include "huggleprofiler.hpp"
#include "wikisite.hpp"
#include "wikiutil.hpp"
#include "ui_login.h"
#include "updateform.hpp"

#define LOGINFORM_LOGIN 0
#define LOGINFORM_SITEINFO 1
#define LOGINFORM_WHITELIST 3
#define LOGINFORM_LOCALCONFIG 4
#define LOGINFORM_USERCONFIG 5
#define LOGINFORM_USERINFO 6

using namespace Huggle;

QString Login::Test = "<login result=\"NeedToken\" token=\"";

Login::Login(QWidget *parent) :   QDialog(parent), ui(new Ui::Login)
{
    HUGGLE_PROFILER_RESET;
    this->Loading = true;
    this->ui->setupUi(this);
    this->ui->tableWidget->setVisible(false);
    if (Configuration::HuggleConfiguration->Multiple)
        this->on_pushButton_2_clicked();
    this->ui->tableWidget->setColumnCount(2);
    this->ui->tableWidget->horizontalHeader()->setVisible(false);
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    this->ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->ui->tableWidget->setShowGrid(false);
    this->ui->tableWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->ui->tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->timer = new QTimer(this);
    connect(this->timer, SIGNAL(timeout()), this, SLOT(OnTimerTick()));
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
    QString title = "Huggle 3 QT-LX";
    if (Configuration::HuggleConfiguration->Verbosity > 0)
    {
        // add debug lang "qqx" last
        this->ui->Language->addItem(Localizations::LANG_QQX);
        if(Localizations::HuggleLocalizations->PreferredLanguage == Localizations::LANG_QQX)
            p = l;
        title += " [" + Configuration::HuggleConfiguration->HuggleVersion + "]";
        l++;
    }
    this->setWindowTitle(title);
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
    if (!Configuration::HuggleConfiguration->SystemConfig_Username.isEmpty())
    {
        this->ui->lineEdit_username->setText(Configuration::HuggleConfiguration->SystemConfig_Username);
        this->ui->lineEdit_password->setFocus();
    }
    this->ui->lineEdit_password->setText(Configuration::HuggleConfiguration->TemporaryConfig_Password);
    this->Loading = false;
    this->Localize();
    HUGGLE_PROFILER_PRINT_TIME("Login::Login(QWidget *parent)");
    if (Configuration::HuggleConfiguration->Login)
    {
        // user wanted to login using a terminal
        this->Processing = true;
        this->PressOK();
    }
}

Login::~Login()
{
    this->RemoveQueries();
    delete this->Updater;
    delete this->ui;
    delete this->loadingForm;
    delete this->timer;
}

void Login::Localize()
{
    this->ui->ButtonExit->setText(_l("main-system-exit"));
    this->ui->ButtonOK->setText(_l("login-start"));
    this->ui->checkBox->setText(_l("login-ssl"));
    this->ui->labelOauthUsername->setText(_l("login-username"));
    this->ui->pushButton->setToolTip(_l("login-reload-tool-tip"));
    this->ui->pushButton->setText(_l("reload"));
    this->ui->tabWidget->setTabText(0, _l("login-tab-oauth"));
    this->ui->tabWidget->setTabText(1, _l("login-tab-login"));
    this->ui->labelOauthNotSupported->setText(_l("login-oauth-notsupported"));
    this->ui->labelUsername->setText(_l("login-username"));
    this->ui->labelProject->setText(_l("login-project"));
    this->ui->labelLanguage->setText(_l("login-language"));
    this->ui->labelPassword->setText(_l("login-password"));
    this->ui->labelIntro->setText(_l("login-intro"));
    this->ui->labelTranslate->setText(QString("<html><head/><body><p><a href=\"http://meta.wikimedia.org/wiki/Huggle/Localization\"><span style=\""\
                                              " text-decoration: underline; color:#0000ff;\">%1</span></a></p></body></html>")
                                              .arg(_l("login-translate")));
    // Change the layout based on preference
    if (Localizations::HuggleLocalizations->IsRTL())
        QApplication::setLayoutDirection(Qt::RightToLeft);
    else
        QApplication::setLayoutDirection(Qt::LeftToRight);
}

void Login::Update(QString ms)
{
     // let's just pass it there
    if (this->loadingForm != nullptr)
        this->loadingForm->Info(ms);

    // update the label
    this->ui->labelIntro->setText(ms);
}

void Login::Reset()
{
    this->ui->labelIntro->setText(_l("[[login-intro]]"));
}

void Login::RemoveQueries()
{
    QList<WikiSite*> Sites = this->LoginQueries.keys();
    foreach (WikiSite* st, Sites)
        this->LoginQueries[st]->DecRef();
    Sites = this->WhitelistQueries.keys();
    foreach (WikiSite* st, Sites)
        this->WhitelistQueries[st]->DecRef();
    Sites = this->qSiteInfo.keys();
    foreach (WikiSite* st, Sites)
        this->qSiteInfo[st]->DecRef();
    this->qSiteInfo.clear();
    this->WhitelistQueries.clear();
    this->LoginQueries.clear();
}

void Login::CancelLogin()
{
    this->Processing = false;
    this->timer->stop();
    this->Enable();
    if (this->loadingForm != nullptr)
    {
        this->loadingForm->close();
        delete this->loadingForm;
        this->loadingForm = nullptr;
    }
    this->ui->labelIntro->setText(_l("login-intro"));
    this->ui->lineEdit_password->setText("");
    this->ui->ButtonOK->setText(_l("login-start"));
    this->RemoveQueries();
}

int Login::GetRowIDForSite(WikiSite *site, int row)
{
    if (!this->LoadingFormRows.contains(site))
    {
        throw new Huggle::Exception("There is no such a site in DB of rows", "int Login::GetRowIDForSite(WikiSite *site, int row)");
    }
    if (!this->LoadingFormRows[site].contains(row))
    {
        throw new Huggle::Exception("There is no such a row in DB of rows", "int Login::GetRowIDForSite(WikiSite *site, int row)");
    }
    return this->LoadingFormRows[site][row];
}

void Login::Enable()
{
    this->ui->lineEdit_oauth_username->setEnabled(true);
    this->ui->Language->setEnabled(true);
    this->ui->Project->setEnabled(true);
    this->ui->checkBox->setEnabled(QSslSocket::supportsSsl());
    this->ui->lineEdit_username->setEnabled(true);
    this->ui->ButtonExit->setEnabled(true);
    this->ui->pushButton_2->setEnabled(true);
    this->ui->tableWidget->setEnabled(true);
    this->ui->lineEdit_password->setEnabled(true);
    this->ui->pushButton->setEnabled(true);
    this->ui->ButtonOK->setEnabled(true);
}

void Login::Disable()
{
    this->ui->lineEdit_oauth_username->setDisabled(true);
    this->ui->Language->setDisabled(true);
    this->ui->Project->setDisabled(true);
    this->ui->checkBox->setDisabled(true);
    this->ui->ButtonOK->setEnabled(false);
    this->ui->ButtonExit->setDisabled(true);
    this->ui->pushButton_2->setDisabled(true);
    this->ui->tableWidget->setDisabled(true);
    this->ui->lineEdit_username->setDisabled(true);
    this->ui->lineEdit_password->setDisabled(true);
    this->ui->pushButton->setDisabled(true);
}

void Login::Reload()
{
    int current = 0;
    this->ui->Project->clear();
    while (this->ui->tableWidget->rowCount() > 0)
        this->ui->tableWidget->removeRow(0);
    this->Project_CheckBoxens.clear();
    while (current < Configuration::HuggleConfiguration->ProjectList.size())
    {
        QString project_ = Configuration::HuggleConfiguration->ProjectList.at(current)->Name;
        this->ui->Project->addItem(project_);
        this->ui->tableWidget->insertRow(current);
        this->ui->tableWidget->setItem(current, 0, new QTableWidgetItem(project_));
        QCheckBox *Item = new QCheckBox();
        if (Configuration::HuggleConfiguration->ProjectString.contains(project_))
        {
            Item->setChecked(true);
        }
        this->Project_CheckBoxens.append(Item);
        this->ui->tableWidget->setCellWidget(current, 1, Item);
        current++;
    }
    this->ui->tableWidget->resizeColumnsToContents();
    this->ui->tableWidget->resizeRowsToContents();
    if (Huggle::Configuration::HuggleConfiguration->IndexOfLastWiki < current)
        this->ui->Project->setCurrentIndex(Huggle::Configuration::HuggleConfiguration->IndexOfLastWiki);
    else
        this->ui->Project->setCurrentIndex(0);
}

void Login::DB()
{
    if (this->qDatabase == nullptr || !this->qDatabase->IsProcessed())
    {
        return;
    }
    Syslog::HuggleLogs->DebugLog(this->qDatabase->Result->Data, 2);
    QDomDocument d;
    d.setContent(this->qDatabase->Result->Data);
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
        this->Reload();
    }
    this->timer->stop();
    this->Enable();
    this->Localize();
    this->Refreshing = false;
}

void Login::PressOK()
{
    this->GlobalConfig = false;
    if (this->ui->tab_oauth->isVisible())
    {
        QMessageBox mb;
        mb.setWindowTitle(_l("function-miss"));
        mb.setText("This function is not available for wmf wikis in this moment");
        mb.exec();
        return;
    }
    if (this->ui->Project->count() == 0)
    {
        // there are no projects in login form
        QMessageBox mb;
        mb.setWindowTitle(_l("error"));
        mb.setText("There are no projects defined in a list you need to set up some on global wiki");
        mb.exec();
        return;
    }
    Configuration::HuggleConfiguration->IndexOfLastWiki = this->ui->Project->currentIndex();
    Configuration::HuggleConfiguration->Project = Configuration::HuggleConfiguration->ProjectList.at(this->ui->Project->currentIndex());
    // we need to clear a list of projects we are logged to and insert at least this one
    Configuration::HuggleConfiguration->Projects.clear();
    Configuration::HuggleConfiguration->SystemConfig_UsingSSL = this->ui->checkBox->isChecked();
    Configuration::HuggleConfiguration->Projects << Configuration::HuggleConfiguration->Project;
    if (Configuration::HuggleConfiguration->Multiple)
    {
        int project_id = 0;
        foreach (QCheckBox* cb, this->Project_CheckBoxens)
        {
            if (project_id >= Configuration::HuggleConfiguration->ProjectList.count())
                throw new Huggle::Exception("Inconsistent number of projects and check boxes in memory");
            WikiSite *project = Configuration::HuggleConfiguration->ProjectList.at(project_id);
            if (cb->isChecked() && !Configuration::HuggleConfiguration->Projects.contains(project))
                Configuration::HuggleConfiguration->Projects << project;
            project_id++;
        }
    }
    Configuration::HuggleConfiguration->Multiple = Configuration::HuggleConfiguration->Projects.count() > 1;
    Configuration::HuggleConfiguration->SystemConfig_Username = WikiUtil::SanitizeUser(ui->lineEdit_username->text());
    Configuration::HuggleConfiguration->TemporaryConfig_Password = ui->lineEdit_password->text();
    if (this->loadingForm != nullptr)
        delete this->loadingForm;

    this->loadingForm = new LoadingForm(this);
    // set new status for all projects
    this->LoadedOldConfigs.clear();
    this->Statuses.clear();
    Configuration::HuggleConfiguration->ProjectString.clear();
    this->processedLogin.clear();
    this->processedSiteinfos.clear();
    this->processedWL.clear();
    foreach (WikiSite *wiki, Configuration::HuggleConfiguration->Projects)
    {
        delete wiki->UserConfig;
        Configuration::HuggleConfiguration->ProjectString.append(wiki->Name);
        wiki->UserConfig = new UserConfiguration();
        delete wiki->ProjectConfig;
        wiki->ProjectConfig = new ProjectConfiguration(wiki->Name);
        this->LoadedOldConfigs.insert(wiki, false);
        this->Statuses.insert(wiki, LoggingIn);
        this->processedLogin.insert(wiki, false);
        this->processedSiteinfos.insert(wiki, false);
        this->processedWL.insert(wiki, false);
    }
    Configuration::HuggleConfiguration->UserConfig = Configuration::HuggleConfiguration->Project->GetUserConfig();
    Configuration::HuggleConfiguration->ProjectConfig = Configuration::HuggleConfiguration->Project->GetProjectConfig();

    if (this->ui->lineEdit_username->text() == "Developer Mode")
    {
        this->DeveloperMode();
        return;
    }
    this->Disable();
    this->loadingForm->show();
    // this is pretty tricky here
    // we need to register every single login operation but we also need to remember which row it was on
    // for that we will use some super hash table, but first we need to make it empty...
    this->ClearLoadingFormRows();
    foreach (WikiSite *wiki, Configuration::HuggleConfiguration->Projects)
    {
        // we need to add this wiki to our super table
        this->LoadingFormRows.insert(wiki, QHash<int,int>());
        this->loadingForm->Insert(this->RegisterLoadingFormRow(wiki, LOGINFORM_LOGIN), _l("login-progress-start", wiki->Name), LoadingForm_Icon_Loading);
        this->loadingForm->Insert(this->RegisterLoadingFormRow(wiki, LOGINFORM_SITEINFO), _l("login-progress-retrieve-mw", wiki->Name), LoadingForm_Icon_Waiting);
        this->loadingForm->Insert(this->RegisterLoadingFormRow(wiki, LOGINFORM_WHITELIST), _l("login-progress-whitelist", wiki->Name), LoadingForm_Icon_Waiting);
        this->loadingForm->Insert(this->RegisterLoadingFormRow(wiki, LOGINFORM_LOCALCONFIG), _l("login-progress-local", wiki->Name), LoadingForm_Icon_Waiting);
        this->loadingForm->Insert(this->RegisterLoadingFormRow(wiki, LOGINFORM_USERCONFIG), _l("login-progress-user", wiki->Name), LoadingForm_Icon_Waiting);
        this->loadingForm->Insert(this->RegisterLoadingFormRow(wiki, LOGINFORM_USERINFO), _l("login-progress-user-info", wiki->Name), LoadingForm_Icon_Waiting);
    }
    this->GlobalRow = this->LastRow;
    this->loadingForm->Insert(this->LastRow, _l("login-progress-global"), LoadingForm_Icon_Waiting);
    this->LastRow++;
    // First of all, we need to login to the site
    this->timer->start(HUGGLE_TIMER);
}

void Login::PerformLogin(WikiSite *site)
{
    this->Update(_l("[[login-progress-start]]", site->Name));
    // we create an api request to login
    this->LoginQueries.insert(site, new ApiQuery(ActionLogin, site));
    ApiQuery *qr = this->LoginQueries[site];
    qr->Parameters = "lgname=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->SystemConfig_Username);
    qr->HiddenQuery = true;
    qr->UsingPOST = true;
    qr->IncRef();
    qr->Process();
    this->Statuses[site] = WaitingForLoginQuery;
}

void Login::PerformLoginPart2(WikiSite *site)
{
    // verify if query is already finished
    if (!this->LoginQueries.contains(site) || !this->LoginQueries[site]->IsProcessed())
        return;

    ApiQuery *query = this->LoginQueries[site];
    if (query->Result->IsFailed())
    {
        this->CancelLogin();
        this->Update(_l("login-fail", site->Name) + ": " + query->Result->ErrorMessage);
        return;
    }
    QString token = this->GetToken(query->Result->Data);
    this->Tokens.insert(site, token);
    this->Statuses[site] = WaitingForToken;
    this->LoginQueries.remove(site);
    query->DecRef();
    query = new ApiQuery(ActionLogin, site);
    this->LoginQueries.insert(site, query);
    query->HiddenQuery = true;
    query->IncRef();
    query->Parameters = "lgname=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->SystemConfig_Username)
            + "&lgpassword=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->TemporaryConfig_Password)
            + "&lgtoken=" + token;
    query->UsingPOST = true;
    query->Process();
}

void Login::RetrieveGlobalConfig()
{
    if (this->qConfig != nullptr)
    {
        if (this->qConfig->IsProcessed())
        {
            if (this->qConfig->Result->IsFailed())
            {
                this->DisplayError(_l("[[login-error-global]]") + ": " + this->qConfig->Result->ErrorMessage);
                return;
            }
            QDomDocument d;
            d.setContent(this->qConfig->Result->Data);
            QDomNodeList l = d.elementsByTagName("rev");
            this->qConfig.Delete();
            if (l.count() == 0)
            {
                this->DisplayError("Login failed unable to retrieve global config, the api query returned no data");
                return;
            }
            QDomElement data = l.at(0).toElement();
            if (Configuration::HuggleConfiguration->ParseGlobalConfig(data.text()))
            {
                if (!Configuration::HuggleConfiguration->GlobalConfig_EnableAll)
                {
                    this->DisplayError(_l("login-error-alldisabled"));
                    return;
                }
                this->GlobalConfig = true;
                this->loadingForm->ModifyIcon(this->GlobalRow, LoadingForm_Icon_Success);
                return;
            }
            Syslog::HuggleLogs->DebugLog(data.text());
            this->DisplayError(_l("login-error-global"));
        }
        return;
    }
    //this->loadingForm->ModifyIcon(LOGINFORM_LOGIN, LoadingForm_Icon_Success);
    this->loadingForm->ModifyIcon(this->GlobalRow, LoadingForm_Icon_Loading);
    this->Update(_l("[[login-progress-global]]"));
    this->qConfig = new ApiQuery(ActionQuery);
    this->qConfig->OverrideWiki = Configuration::HuggleConfiguration->GlobalConfigurationWikiAddress;
    this->qConfig->Parameters = "prop=revisions&format=xml&rvprop=content&rvlimit=1&titles=Huggle/Config";
    this->qConfig->Process();
}

void Login::FinishLogin(WikiSite *site)
{
    if (!this->LoginQueries.contains(site) || !this->LoginQueries[site]->IsProcessed())
        return;

    ApiQuery *query = this->LoginQueries[site];
    if (query->Result->IsFailed())
    {
        this->DisplayError("Login failed (on " + site->Name + "): " + query->Result->ErrorMessage);
        this->Statuses[site] = LoginFailed;
        return;
    }

    // Assume login was successful
    if (this->ProcessOutput(site))
    {
        this->LoginQueries.remove(site);
        query->DecRef();
        ApiQuery *qr = new ApiQuery(ActionQuery, site);
        this->loadingForm->ModifyIcon(this->GetRowIDForSite(site, LOGINFORM_SITEINFO), LoadingForm_Icon_Loading);
        this->qSiteInfo.insert(site, qr);
        qr->IncRef();
        this->loadingForm->ModifyIcon(this->GetRowIDForSite(site, LOGINFORM_LOGIN), LoadingForm_Icon_Success);
        qr->Parameters = "meta=siteinfo&siprop=" + QUrl::toPercentEncoding("namespaces|general");
        qr->Process();
        this->Statuses[site] = RetrievingProjectConfig;
    }
}

void Login::RetrieveWhitelist(WikiSite *site)
{
    if (this->WhitelistQueries.contains(site))
    {
        WLQuery *query = this->WhitelistQueries[site];
        if (query->IsProcessed())
        {
            this->WhitelistQueries.remove(site);
            if (query->Result->IsFailed())
            {
                //! \todo This needs to be handled per project, there is no point in disabling WL on all projects
                Configuration::HuggleConfiguration->SystemConfig_WhitelistDisabled = true;
            } else
            {
                QString list = query->Result->Data;
                list = list.replace("<!-- list -->", "");
                site->GetProjectConfig()->WhiteList = list.split("|");
                site->GetProjectConfig()->WhiteList.removeAll("");
            }
            this->processedWL[site] = true;
            this->loadingForm->ModifyIcon(this->GetRowIDForSite(site, LOGINFORM_WHITELIST), LoadingForm_Icon_Success);
            query->DecRef();
        }
        return;
    }
    this->loadingForm->ModifyIcon(this->GetRowIDForSite(site, LOGINFORM_WHITELIST), LoadingForm_Icon_Loading);
    WLQuery *query = new WLQuery(site);
    query->IncRef();
    this->WhitelistQueries.insert(site, query);
    query->RetryOnTimeoutFailure = false;
    query->Process();
}

void Login::RetrieveProjectConfig(WikiSite *site)
{
    if (this->LoginQueries.contains(site))
    {
        ApiQuery *query = this->LoginQueries[site];
        if (query->IsProcessed())
        {
            if (query->Result->IsFailed())
            {
                this->DisplayError(_l("login-error-config", site->Name, query->Result->ErrorMessage));
                return;
            }
            QDomDocument d;
            d.setContent(query->Result->Data);
            QDomNodeList l = d.elementsByTagName("rev");
            if (l.count() == 0)
            {
                this->DisplayError(_l("login-error-config", site->Name, "the api query returned no data"));
                return;
            }
            this->LoginQueries.remove(site);
            query->DecRef();
            QDomElement data = l.at(0).toElement();
            if (site->ProjectConfig == nullptr)
                throw new Huggle::NullPointerException("site->Project", "void Login::RetrieveProjectConfig(WikiSite *site)");
            QString reason;
            if (site->ProjectConfig->Parse(data.text(), &reason))
            {
                if (!site->ProjectConfig->EnableAll)
                {
                    this->DisplayError(_l("login-error-projdisabled", site->Name));
                    return;
                }
                this->loadingForm->ModifyIcon(this->GetRowIDForSite(site, LOGINFORM_LOCALCONFIG), LoadingForm_Icon_Success);
                this->Statuses[site] = RetrievingUserConfig;
                return;
            } else
            {
                this->DisplayError(_l("login-error-config", site->Name, reason));
                return;
            }
        }
        return;
    }
    this->loadingForm->ModifyIcon(this->GetRowIDForSite(site, LOGINFORM_LOCALCONFIG), LoadingForm_Icon_Loading);
    ApiQuery *query = new ApiQuery(ActionQuery, site);
    query->IncRef();
    query->Parameters = "prop=revisions&format=xml&rvprop=content&rvlimit=1&titles=Project:Huggle/Config";
    query->Process();
    this->LoginQueries.insert(site, query);
}

void Login::RetrieveUserConfig(WikiSite *site)
{
    if (this->LoginQueries.contains(site))
    {
        ApiQuery *q = this->LoginQueries[site];
        if (q->IsProcessed())
        {
            if (q->Result->IsFailed())
            {
                this->DisplayError("Login failed unable to retrieve user config: " + q->Result->ErrorMessage);
                return;
            }
            QDomDocument d;
            d.setContent(q->Result->Data);
            QDomNodeList revisions = d.elementsByTagName("rev");
            if (revisions.count() == 0) // page is missing
            {
                if (this->LoadedOldConfigs[site] == false && !Configuration::HuggleConfiguration->GlobalConfig_UserConf_old.isEmpty())
                {
                    // try first with old location of config, we don't need to switch the login step here we just
                    // replace the old query with new query that retrieves the old config and call this function
                    // once more, trying to parse the old config
                    this->LoadedOldConfigs[site] = true;
                    Syslog::HuggleLogs->DebugLog("couldn't find user config for " + site->Name + " at new location, trying old one");
                    q->DecRef();
                    // let's get an old configuration instead
                    q = new ApiQuery(ActionQuery, site);
                    this->LoginQueries[site] = q;
                    q->IncRef();
                    QString page = Configuration::HuggleConfiguration->GlobalConfig_UserConf_old;
                    page = page.replace("$1", Configuration::HuggleConfiguration->SystemConfig_Username);
                    q->Parameters = "prop=revisions&rvprop=content&rvlimit=1&titles=" + QUrl::toPercentEncoding(page);
                    q->Process();
                    return;
                }
                if (!site->GetProjectConfig()->RequireConfig)
                {
                    // we don't care if user config is missing or not
                    q->DecRef();
                    this->LoginQueries.remove(site);
                    this->Statuses[site] = RetrievingUser;
                    return;
                }
                Syslog::HuggleLogs->DebugLog(q->Result->Data);
                this->DisplayError(_l("login-fail-css", site->Name));
                return;
            }

            this->LoginQueries.remove(site);
            q->DecRef();

            QDomElement data = revisions.at(0).toElement();
            if (Configuration::HuggleConfiguration->ParseUserConfig(site, data.text()))
            {
                if (this->LoadedOldConfigs[site])
                {
                    // if we loaded the old config we write that to debug log because othewise we hardly check this
                    // piece of code really works
                    Syslog::HuggleLogs->DebugLog("We successfuly loaded and converted the old config for " + site->Name + " (huggle.css) :)");
                }
                if (!Configuration::HuggleConfiguration->ProjectConfig->EnableAll)
                {
                    this->DisplayError(_l("login-fail-enable-true", site->Name));
                    return;
                }
                this->loadingForm->ModifyIcon(this->GetRowIDForSite(site, LOGINFORM_USERCONFIG), LoadingForm_Icon_Success);
                this->Statuses[site] = RetrievingUser;
                return;
            }
            // failed unable to parse the user config
            Syslog::HuggleLogs->DebugLog(data.text());
            this->DisplayError(_l("login-fail-parse-config", site->Name));
        }
        return;
    }
    this->loadingForm->ModifyIcon(this->GetRowIDForSite(site, LOGINFORM_USERCONFIG), LoadingForm_Icon_Loading);
    ApiQuery *query = new ApiQuery(ActionQuery, site);
    query->IncRef();
    this->LoginQueries.insert(site, query);
    QString page = Configuration::HuggleConfiguration->GlobalConfig_UserConf;
    page = page.replace("$1", Configuration::HuggleConfiguration->SystemConfig_Username);
    query->Parameters = "prop=revisions&rvprop=content&rvlimit=1&titles=" + QUrl::toPercentEncoding(page);
    query->Process();
}

void Login::RetrieveUserInfo(WikiSite *site)
{
    if (this->LoginQueries.contains(site))
    {
        ApiQuery *query = this->LoginQueries[site];
        if (query->IsProcessed())
        {
            if (query->Result->IsFailed())
            {
                this->DisplayError(_l("login-fail-no-info", site->Name, query->Result->ErrorMessage));
                return;
            }
            QDomDocument dLoginResult;
            dLoginResult.setContent(query->Result->Data);
            QDomNodeList lRights_ = dLoginResult.elementsByTagName("r");
            if (lRights_.count() == 0)
            {
                Syslog::HuggleLogs->DebugLog(query->Result->Data);
                // Login failed unable to retrieve user info since the api query returned no data
                this->DisplayError(_l("login-fail-user-data", site->Name));
                return;
            }
            int c=0;
            while(c<lRights_.count())
            {
                site->GetProjectConfig()->Rights.append(lRights_.at(c).toElement().text());
                c++;
            }
            if (site->GetProjectConfig()->RequireRollback && !site->GetProjectConfig()->Rights.contains("rollback"))
            {
                this->DisplayError(_l("login-fail-rollback-rights", site->Name));
                return;
            }
            if (site->GetProjectConfig()->RequireAutoconfirmed && !site->GetProjectConfig()->Rights.contains("autoconfirmed"))
                //sometimes there is something like manually "confirmed", thats currently not included here
            {
                this->DisplayError(_l("login-failed-autoconfirm-rights", site->Name));
                return;
            }
            // remove the query
            this->LoginQueries.remove(site);
            QDomNodeList userinfos = dLoginResult.elementsByTagName("userinfo");
            query->DecRef();
            int editcount = userinfos.at(0).toElement().attribute("editcount", "-1").toInt();
            if (site->GetProjectConfig()->RequireEdits > editcount)
            {
                this->DisplayError(_l("login-failed-edit", site->Name));
                return;
            }

            /// \todo Implement check for "require-time"
            this->loadingForm->ModifyIcon(this->GetRowIDForSite(site, LOGINFORM_USERINFO), LoadingForm_Icon_Success);
            this->processedLogin[site] = true;
            this->Statuses[site] = LoginDone;
        }
        return;
    }
    this->loadingForm->ModifyIcon(this->GetRowIDForSite(site, LOGINFORM_USERINFO), LoadingForm_Icon_Loading);
    ApiQuery *temp = new ApiQuery(ActionQuery, site);
    temp->IncRef();
    // now we can retrieve some information about user for this project
    temp->Parameters = "meta=userinfo&format=xml&uiprop=" + QUrl::toPercentEncoding("rights|editcount");
    temp->Process();
    this->LoginQueries.insert(site, temp);
}

void Login::DeveloperMode()
{
    Configuration::HuggleConfiguration->Restricted = true;
    MainWindow::HuggleMain = new MainWindow();
    MainWindow::HuggleMain->show();
    Core::HuggleCore->Main = MainWindow::HuggleMain;
    this->hide();
}

void Login::ProcessSiteInfo(WikiSite *site)
{
    if (this->qSiteInfo.contains(site) && this->qSiteInfo[site]->IsProcessed())
    {
        //! \todo Check that request isnt failed
        QDomDocument d;
        d.setContent(this->qSiteInfo[site]->Result->Data);
        QDomNodeList l = d.elementsByTagName("general");
        if( l.count() < 1 )
        {
            this->DisplayError("No site info was returned for this wiki");
            return;
        }
        QDomElement item = l.at(0).toElement();
        if (item.attributes().contains("rtl"))
        {
            site->IsRightToLeft = true;
        }
        if (item.attributes().contains("time"))
        {
            QDateTime server_time = MediaWiki::FromMWTimestamp(item.attribute("time"));
            site->GetProjectConfig()->ServerOffset = QDateTime::currentDateTime().secsTo(server_time);
        }
        l = d.elementsByTagName("ns");
        if (l.count() < 1)
        {
            Syslog::HuggleLogs->WarningLog(QString("Mediawiki of ") + site->Name + " provided no information about namespaces");
        } else
        {
            // let's prepare a NS list
            site->ClearNS();
            register int index = 0;
            while (index < l.count())
            {
                QDomElement e = l.at(index).toElement();
                index++;
                if (!e.attributes().contains("id") || !e.attributes().contains("canonical"))
                    continue;
                site->InsertNS(new WikiPageNS(e.attribute("id").toInt(), e.text(), e.attribute("canonical")));
            }
        }
        this->processedSiteinfos[site] = true;
        this->qSiteInfo[site]->DecRef();
        this->qSiteInfo.remove(site);
        this->loadingForm->ModifyIcon(this->GetRowIDForSite(site, LOGINFORM_SITEINFO), LoadingForm_Icon_Success);
    }
}

void Login::DisplayError(QString message)
{
    this->CancelLogin();
    Syslog::HuggleLogs->ErrorLog(message);
    this->Update(message);
}

void Login::Finish()
{
    // let's check if all processes are finished
    foreach (WikiSite *xx, Configuration::HuggleConfiguration->Projects)
    {
        if (!this->processedWL[xx]) return;
        if (!this->processedLogin[xx]) return;
        if (this->Statuses[xx] != LoginDone) return;
        if (!this->processedSiteinfos[xx]) return;
    }
    if (!this->GlobalConfig)
        return;
    QString pw = "";
    this->Finished = true;
    foreach (WikiSite *site, Configuration::HuggleConfiguration->Projects)
    {
        site->GetProjectConfig()->IsLoggedIn = true;
        this->Statuses[site] = Nothing;
    }
    // we generate a random string of same size of current password
    while (pw.length() < Configuration::HuggleConfiguration->TemporaryConfig_Password.length())
    {
        pw += "x";
    }
    // we no longer need a password since this
    Configuration::HuggleConfiguration->TemporaryConfig_Password = pw;
    this->ui->lineEdit_password->setText(pw);
    this->Update("Loading main huggle window");
    this->timer->stop();
    this->hide();
    MainWindow::HuggleMain = new MainWindow();
    Core::HuggleCore->Main = MainWindow::HuggleMain;
    Core::HuggleCore->Main->show();
    if (this->loadingForm != nullptr)
    {
        this->loadingForm->close();
        delete this->loadingForm;
        this->loadingForm = nullptr;
    }
}

void Login::reject()
{
    if (this->Finished == false)
    {
        Core::HuggleCore->Shutdown();
    }
    else
    {
        QDialog::reject();
    }
}

bool Login::ProcessOutput(WikiSite *site)
{
    ApiQuery *query = this->LoginQueries[site];
    // Check what the result was
    QString Result = query->Result->Data;
    if (!Result.contains(("<login result")))
    {
        Syslog::HuggleLogs->DebugLog(Result);
        this->DisplayError("The api.php responded with invalid text (webserver is down?), please check debug "\
                           "log for precise information");
        return false;
    }

    Result = Result.mid(Result.indexOf("result=\"") + 8);
    if (!Result.contains("\""))
    {
        Syslog::HuggleLogs->DebugLog(Result);
        this->DisplayError("The api.php responded with invalid text (webserver is broken), please check debug "\
                           "log for precise information");
        return false;
    }
    Result = Result.mid(0, Result.indexOf("\""));
    if (Result == "Success")
        return true;
    if (Result == "EmptyPass")
    {
        this->DisplayError(_l("login-password-empty"));
        return false;
    }
    if (Result == "WrongPass")
    {
        /// \bug This sometimes doesn't work properly
        this->ui->lineEdit_password->setFocus();
        this->DisplayError(_l("login-error-password"));
        return false;
    }
    if (Result == "NoName")
    {
        this->DisplayError(_l("login-fail-wrong-name"));
        return false;
    }
    this->DisplayError(_l("login-api", Result));
    return false;
}

QString Login::GetToken(QString source_code)
{
    QString token = source_code;
    if (!token.contains(Login::Test))
    {
        Syslog::HuggleLogs->WarningLog("the result of api request doesn't contain valid token");
        Syslog::HuggleLogs->DebugLog("The token didn't contain the correct string, token was " + token);
        return "<invalid token>";
    }
    token = token.mid(token.indexOf(Login::Test) + Login::Test.length());
    if (!token.contains("\""))
    {
        Syslog::HuggleLogs->WarningLog("the result of api request doesn't contain valid token");
        Syslog::HuggleLogs->DebugLog("The token didn't contain the closing mark, token was " + token);
        return "<invalid token>";
    }
    token = token.mid(0, token.indexOf("\""));
    return token;
}

void Login::on_ButtonOK_clicked()
{
    if (!this->Processing)
    {
        this->Processing = true;
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
    if (this->Refreshing)
    {
        this->DB();
        return;
    }
    if (!this->Processing)
        return;
    if (!this->GlobalConfig)
        this->RetrieveGlobalConfig();
    // let's check status for every single project
    foreach (WikiSite *site, Configuration::HuggleConfiguration->Projects)
    {
        if (!this->Statuses.contains(site))
            throw new Huggle::Exception("There is no such a wiki in statuses list");
        if (!this->GlobalConfig)
        {
            // we need to skip these unless it's login
            switch(this->Statuses[site])
            {
                case LoggedIn:
                case Nothing:
                case Cancelling:
                case LoginFailed:
                case LoginDone:
                case LoggingIn:
                case WaitingForLoginQuery:
                case WaitingForToken:
                    break;
                case RetrievingProjectConfig:
                case RetrievingUserConfig:
                case RetrievingUser:
                    continue;
            }
        }
        switch(this->Statuses[site])
        {
            case LoggingIn:
                this->PerformLogin(site);
                break;
            case WaitingForLoginQuery:
                PerformLoginPart2(site);
                break;
            case WaitingForToken:
                FinishLogin(site);
                break;
            case RetrievingProjectConfig:
                RetrieveProjectConfig(site);
                break;
            case RetrievingUserConfig:
                RetrieveUserConfig(site);
                break;
            case RetrievingUser:
                RetrieveUserInfo(site);
                break;
            case LoggedIn:
            case Nothing:
            case Cancelling:
            case LoginFailed:
            case LoginDone:
                break;
        }
        if (this->GlobalConfig)
        {
            if (!this->processedWL[site])
                this->RetrieveWhitelist(site);
            if (this->qSiteInfo.contains(site))
                this->ProcessSiteInfo(site);
        }
        if (this->Statuses[site] == LoginFailed)
        {
            this->Enable();
            this->timer->stop();
            this->ui->ButtonOK->setText("Login");
        }
    }

    this->Finish();
}

void Login::on_pushButton_clicked()
{
    this->Disable();
    this->qDatabase = new ApiQuery(ActionQuery);
    this->Refreshing = true;
    Configuration::HuggleConfiguration->SystemConfig_UsingSSL = this->ui->checkBox->isChecked();
    this->timer->start(HUGGLE_TIMER);
    this->qDatabase->OverrideWiki = Configuration::HuggleConfiguration->GlobalConfigurationWikiAddress;
    this->ui->ButtonOK->setText(_l("[[cancel]]"));
    this->qDatabase->Parameters = "prop=revisions&format=xml&rvprop=content&rvlimit=1&titles="
                        + Configuration::HuggleConfiguration->SystemConfig_GlobalConfigWikiList;
    this->qDatabase->Process();
}

void Login::on_Language_currentIndexChanged(const QString &arg1)
{
    if (this->Loading)  return;
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
    if (Localizations::LANG_QQX == arg1)
        lang = Localizations::LANG_QQX;
    Localizations::HuggleLocalizations->PreferredLanguage = lang;
    this->Localize();
}

void Huggle::Login::on_labelTranslate_linkActivated(const QString &link)
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
    if((this->ui->lineEdit_username->text().size() == 0 || this->ui->lineEdit_password->text().size() == 0) &&
            (this->ui->lineEdit_username->text() != "Developer Mode"))
        this->ui->ButtonOK->setEnabled( false );
    else
        this->ui->ButtonOK->setEnabled( true );
}

int Login::RegisterLoadingFormRow(WikiSite *site, int row)
{
    this->LoadingFormRows[site].insert(row, this->LastRow);
    this->LastRow++;
    return this->LastRow - 1;
}

void Login::ClearLoadingFormRows()
{
    QList<WikiSite*> Sites = this->LoadingFormRows.keys();
    this->LastRow = 0;
    foreach (WikiSite *Site, Sites)
        this->LoadingFormRows[Site].clear();
    this->LoadingFormRows.clear();
}

void Huggle::Login::on_pushButton_2_clicked()
{
    if (this->ui->tableWidget->isVisible())
    {
        this->ui->pushButton_2->setText("Projects >>");
        this->ui->tableWidget->setVisible(false);
        Configuration::HuggleConfiguration->Multiple = false;
    } else
    {
        Configuration::HuggleConfiguration->Multiple = true;
        this->ui->pushButton_2->setText("Projects <<");
        this->ui->tableWidget->setVisible(true);
        if (this->height() < 460)
            this->resize(this->width(), 480);
    }
}
