//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "apiqueryresult.hpp"
#include "configuration.hpp"
#include "core.hpp"
#include "exception.hpp"
#include "generic.hpp"
#include "huggleprofiler.hpp"
#include "hugglequeuefilter.hpp"
#include "loadingform.hpp"
#include "localization.hpp"
#include "login.hpp"
#include "mainwindow.hpp"
#include "mediawiki.hpp"
#include "proxy.hpp"
#include "welcomeinfo.hpp"
#include "syslog.hpp"
#include "ui_login.h"
#include "updateform.hpp"
#include "wikisite.hpp"
#include "wikiutil.hpp"
#include <QCheckBox>
#include <QFile>
#include <QUrl>
#include <QSslSocket>
#include <QDesktopServices>
#include <QMessageBox>

#define LOGINFORM_LOGIN 0
#define LOGINFORM_SITEINFO 1
#define LOGINFORM_WHITELIST 3
#define LOGINFORM_LOCALCONFIG 4
#define LOGINFORM_USERCONFIG 5
#define LOGINFORM_USERINFO 6

using namespace Huggle;

Login::Login(QWidget *parent) : HW("login", this, parent), ui(new Ui::Login)
{
    HUGGLE_PROFILER_RESET;
    this->Loading = true;
    this->ui->setupUi(this);
    this->ui->tableWidget->setVisible(false);
    if (hcfg->SystemConfig_Multiple)
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
    this->ui->checkBox->setChecked(hcfg->SystemConfig_UsingSSL);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    int localization_ix=0, preferred=0;
    while (localization_ix<Localizations::HuggleLocalizations->LocalizationData.count())
    {
        this->ui->Language->addItem(Localizations::HuggleLocalizations->LocalizationData.at(localization_ix)->LanguageID);
        if (Localizations::HuggleLocalizations->LocalizationData.at(localization_ix)->LanguageName == Localizations::HuggleLocalizations->PreferredLanguage)
        {
            preferred = localization_ix;
        }
        localization_ix++;
    }
    QString title = "Huggle 3 QT-LX";
    if (hcfg->Verbosity > 0)
    {
        // add debug lang "qqx" last
        this->ui->Language->addItem(Localizations::LANG_QQX);
        if(Localizations::HuggleLocalizations->PreferredLanguage == Localizations::LANG_QQX)
            preferred = localization_ix;
        title += " [" + hcfg->HuggleVersion + "]";
        localization_ix++;
    }
    this->setWindowTitle(title);
    this->ui->Language->setCurrentIndex(preferred);
    this->Reload();
    if (!QSslSocket::supportsSsl())
    {
        hcfg->SystemConfig_UsingSSL = false;
        this->ui->checkBox->setEnabled(false);
        this->ui->checkBox->setChecked(false);
    }
    if (hcfg->SystemConfig_EnableUpdates)
    {
        this->Updater = new UpdateForm();
        this->Updater->Check();
    }
    if (hcfg->SystemConfig_BotPassword)
    {
        this->ui->tab_oauth->show();
        if (!hcfg->SystemConfig_Username.isEmpty())
        {
            this->ui->lineEditBotUser->setText(hcfg->SystemConfig_BotLogin);
            this->ui->lineEditBotP->setFocus();
        }
    } else
    {
        this->ui->tab_login->show();
        if (!hcfg->SystemConfig_Username.isEmpty())
        {
            this->ui->lineEdit_username->setText(hcfg->SystemConfig_Username);
            this->ui->lineEdit_password->setFocus();
        }
    }
    this->Loading = false;
    this->Localize();
    // Load the proxy
    if (hcfg->SystemConfig_UseProxy)
    {
        Proxy::SetProxy(hcfg->SystemConfig_ProxyType, hcfg->SystemConfig_ProxyHost, hcfg->SystemConfig_ProxyPort,
            hcfg->SystemConfig_ProxyUser, hcfg->SystemConfig_ProxyPass);
    }
    HUGGLE_PROFILER_PRINT_TIME(BOOST_CURRENT_FUNCTION);
    if (!hcfg->Login && (hcfg->SystemConfig_FirstRun ||
        hcfg->SystemConfig_ShowStartupInfo))
    {
        WelcomeInfo info;
        info.exec();
    }
    if (hcfg->Login)
    {
        // user wanted to login using a terminal
        this->Processing = true;
        this->PressOK();
    } else if (hcfg->SystemConfig_StorePassword)
    {
        if (hcfg->SystemConfig_BotPassword)
            this->ui->lineEditBotP->setText(hcfg->SystemConfig_RememberedPassword);
        else
            this->ui->lineEdit_password->setText(hcfg->SystemConfig_RememberedPassword);
        this->ui->checkBox_2->setChecked(true);
    }
    this->RestoreWindow();
    if (hcfg->SystemConfig_BotPassword)
        this->ui->tabWidget->setCurrentIndex(0);
    else
        this->ui->tabWidget->setCurrentIndex(1);
}

Login::~Login()
{
    this->RemoveQueries();
    delete this->Updater;
    delete this->ui;
    delete this->loadingForm;
    LoadingForm::IsKilled = true;
    delete this->timer;
}

void Login::Localize()
{
    this->ui->ButtonExit->setText(_l("main-system-exit"));
    this->ui->ButtonOK->setText(_l("login-start"));
    this->ui->checkBox->setText(_l("login-ssl"));
    this->ui->labelBotUserName->setText(_l("login-username"));
    this->ui->pushButton->setToolTip(_l("login-reload-tool-tip"));
    this->ui->pushButton->setText(_l("reload"));
    this->ui->labelBotPassword->setText(_l("login-password"));
    this->ui->tabWidget->setTabText(0, _l("login-tab-botp"));
    this->ui->tabWidget->setTabText(1, _l("login-tab-login"));
    //this->ui->labelOauthNotSupported->setText(_l("login-oauth-notsupported"));
    this->ui->labelUsername->setText(_l("login-username"));
    this->ui->labelProject->setText(_l("login-project"));
    this->ui->labelLanguage->setText(_l("login-language"));
    this->ui->label_2->setText("<a href=\"https://www.mediawiki.org/wiki/Manual:Huggle/Bot_passwords\">" + _l("login-bot") + "</a>");
    this->ui->labelPassword->setText(_l("login-password"));
    this->ui->checkBox_2->setText(_l("login-remember-password"));
    this->ui->checkBox_2->setToolTip(_l("login-remember-password-tooltip"));
    this->ui->labelIntro->setText(_l("login-intro"));
    this->ui->pushButton_2->setText(_l("projects"));
    this->ui->labelTranslate->setText(QString("<html><head/><body><p><a href=\"http://meta.wikimedia.org/wiki/Huggle/Localization\"><span style=\""\
                                              " text-decoration: underline; color:#0000ff;\">%1</span></a></p></body></html>").arg(_l("login-translate")));
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
    Sites = this->qTokenInfo.keys();
    foreach (WikiSite* st, Sites)
        this->qTokenInfo[st]->DecRef();
    Sites = this->qApproval.keys();
    foreach (WikiSite* st, Sites)
        this->qApproval[st]->DecRef();
    Sites = this->qSiteInfo.keys();
    foreach (WikiSite* st, Sites)
        this->qSiteInfo[st]->DecRef();
    this->qSiteInfo.clear();
    this->qTokenInfo.clear();
    this->qApproval.clear();
    this->WhitelistQueries.clear();
    this->LoginQueries.clear();
}

void Login::CancelLogin()
{
    this->Processing = false;
    LoadingForm::IsKilled = true;
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
        throw new Huggle::Exception("There is no such a site in DB of rows", BOOST_CURRENT_FUNCTION);

    if (!this->LoadingFormRows[site].contains(row))
        throw new Huggle::Exception("There is no such a row in DB of rows", BOOST_CURRENT_FUNCTION);

    return this->LoadingFormRows[site][row];
}

void Login::Enable(bool value)
{
    this->ui->checkBox_2->setEnabled(value);
    this->ui->lineEditBotP->setEnabled(value);
    this->ui->lineEditBotUser->setEnabled(value);
    this->ui->Language->setEnabled(value);
    this->ui->label->setEnabled(value);
    this->ui->Project->setEnabled(value);
    if (value)
        this->ui->checkBox->setEnabled(QSslSocket::supportsSsl());
    else
        this->ui->checkBox->setEnabled(false);
    this->ui->lineEdit_username->setEnabled(value);
    this->ui->ButtonExit->setEnabled(value);
    this->ui->pushButton_2->setEnabled(value);
    this->ui->tableWidget->setEnabled(value);
    this->ui->lineEdit_password->setEnabled(value);
    this->ui->pushButton->setEnabled(value);
    this->ui->ButtonOK->setEnabled(value);
}

bool Login::ReadonlyFallback(WikiSite *site, QString why)
{
    if (site->GetProjectConfig()->ReadOnly)
        return true;

    if (Generic::MessageBox(_l("login-ro-title"), _l("login-ro-question", site->Name, why), MessageBoxStyleQuestion, true, this) == QMessageBox::No)
    {
        return false;
    }

    Syslog::HuggleLogs->WarningLog(_l("login-ro-info", site->Name));
    site->GetProjectConfig()->ReadOnly = true;
    return true;
}

void Login::Disable()
{
    this->Enable(false);
}

void Login::Reload()
{
    int current = 0;
    this->ui->Project->clear();
    while (this->ui->tableWidget->rowCount() > 0)
        this->ui->tableWidget->removeRow(0);
    this->Project_CheckBoxens.clear();
    while (current < hcfg->ProjectList.size())
    {
        QString project_ = hcfg->ProjectList.at(current)->Name;
        this->ui->Project->addItem(project_);
        this->ui->tableWidget->insertRow(current);
        this->ui->tableWidget->setItem(current, 0, new QTableWidgetItem(project_));
        QCheckBox *Item = new QCheckBox();
        if (hcfg->ProjectString.contains(project_))
        {
            Item->setChecked(true);
        }
        this->Project_CheckBoxens.append(Item);
        this->ui->tableWidget->setCellWidget(current, 1, Item);
        current++;
    }
    this->ui->tableWidget->resizeColumnsToContents();
    this->ui->tableWidget->resizeRowsToContents();
    if (hcfg->IndexOfLastWiki < current)
        this->ui->Project->setCurrentIndex(hcfg->IndexOfLastWiki);
    else
        this->ui->Project->setCurrentIndex(0);
}

void Login::DB()
{
    if (this->qDatabase == nullptr || !this->qDatabase->IsProcessed())
    {
        return;
    }
    if (this->qDatabase->IsFailed())
    {
        this->DisplayError("Unable to download a db of wikis: " + this->qDatabase->GetFailureReason());
        this->timer->stop();
        this->Enable();
        this->Refreshing = false;
        return;
    }
    Syslog::HuggleLogs->DebugLog(this->qDatabase->Result->Data, 2);
    ApiQueryResultNode *result = this->qDatabase->GetApiQueryResult()->GetNode("rev");
    if (result != nullptr)
    {
        if (QFile().exists(hcfg->WikiDB))
            QFile().remove(hcfg->WikiDB);
        QFile wiki(hcfg->WikiDB);
        if (wiki.open(QIODevice::WriteOnly))
        {
            wiki.write(result->Value.toUtf8());
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
    hcfg->SystemConfig_BotPassword = this->ui->tab_oauth->isVisible();
    if (this->ui->Project->count() == 0)
    {
        // there are no projects in login form
        Generic::pMessageBox(this, _l("error"), "There are no projects defined in a list you need to set up some on global wiki");
        return;
    }
    hcfg->SystemConfig_StorePassword = this->ui->checkBox_2->isChecked();
    if (hcfg->SystemConfig_BotPassword)
    {
        hcfg->SystemConfig_BotLogin = this->ui->lineEditBotUser->text();
        if (!this->ui->lineEditBotUser->text().contains("@"))
        {
            this->DisplayError(_l("invalid-bot-user-name"));
            return;
        }
        QString name = hcfg->SystemConfig_BotLogin;
        hcfg->SystemConfig_Username = WikiUtil::SanitizeUser(name.mid(0, name.indexOf("@")));
        if (hcfg->SystemConfig_StorePassword)
            hcfg->SystemConfig_RememberedPassword = this->ui->lineEditBotP->text();
        hcfg->TemporaryConfig_Password = ui->lineEditBotP->text();
    } else
    {
        if (hcfg->SystemConfig_StorePassword)
            hcfg->SystemConfig_RememberedPassword = this->ui->lineEdit_password->text();
        hcfg->SystemConfig_Username = WikiUtil::SanitizeUser(ui->lineEdit_username->text());
        hcfg->TemporaryConfig_Password = ui->lineEdit_password->text();
    }
    hcfg->IndexOfLastWiki = this->ui->Project->currentIndex();
    hcfg->Project = hcfg->ProjectList.at(this->ui->Project->currentIndex());
    // we need to clear a list of projects we are logged to and insert at least this one
    hcfg->Projects.clear();
    hcfg->SystemConfig_UsingSSL = this->ui->checkBox->isChecked();
    hcfg->Projects << hcfg->Project;
    if (hcfg->SystemConfig_Multiple)
    {
        int project_id = 0;
        foreach (QCheckBox* cb, this->Project_CheckBoxens)
        {
            if (project_id >= hcfg->ProjectList.count())
                throw new Huggle::Exception("Inconsistent number of projects and check boxes in memory", BOOST_CURRENT_FUNCTION);
            WikiSite *project = hcfg->ProjectList.at(project_id);
            if (cb->isChecked() && !hcfg->Projects.contains(project))
                hcfg->Projects << project;
            project_id++;
        }
    }
    hcfg->SystemConfig_Multiple = hcfg->Projects.count() > 1;
    if (hcfg->SystemConfig_UsingSSL)
    {
        foreach(WikiSite *wiki, hcfg->Projects)
        {
            if (!wiki->SupportHttps)
            {
                this->DisplayError(_l("ssl-is-not-supported", wiki->Name));
                return;
            }
        }
    }
    else
    {
        foreach(WikiSite *wiki, hcfg->Projects)
        {
            if (wiki->ForceSSL)
            {
                this->DisplayError(_l("ssl-required", wiki->Name));
                return;
            }
        }
    }
    if (this->loadingForm != nullptr)
        delete this->loadingForm;

    this->loadingForm = new LoadingForm(this);
    LoadingForm::IsKilled = false;
    // set new status for all projects
    this->LoadedOldConfigs.clear();
    this->Statuses.clear();
    hcfg->ProjectString.clear();
    this->processedLogin.clear();
    this->processedSiteinfos.clear();
    this->processedWL.clear();
    foreach (WikiSite *wiki, hcfg->Projects)
    {
        delete wiki->UserConfig;
        hcfg->ProjectString.append(wiki->Name);
        wiki->UserConfig = new UserConfiguration();
        delete wiki->ProjectConfig;
        wiki->ProjectConfig = new ProjectConfiguration(wiki->Name);
        this->LoadedOldConfigs.insert(wiki, false);
        this->Statuses.insert(wiki, LoggingIn);
        this->processedLogin.insert(wiki, false);
        this->processedSiteinfos.insert(wiki, false);
        this->processedWL.insert(wiki, false);
    }
    hcfg->UserConfig = hcfg->Project->GetUserConfig();
    hcfg->ProjectConfig = hcfg->Project->GetProjectConfig();

    if (this->ui->lineEdit_username->text() == "Developer Mode" ||
            this->ui->lineEdit_username->text() == "Developer_Mode")
    {
        this->DeveloperMode();
        return;
    }
    this->Disable();
    this->loadingForm->show();
    this->loadingForm->setWindowTitle(_l("login-progress-formtitle"));
    // we need to delete the filters here so that if previous login didn't finish we don't reuse the previous ones
    HuggleQueueFilter::Delete();
    // this is pretty tricky here
    // we need to register every single login operation but we also need to remember which row it was on
    // for that we will use some super hash table, but first we need to make it empty...
    this->ClearLoadingFormRows();
    foreach (WikiSite *wiki, hcfg->Projects)
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
    this->LoginQueries.insert(site, new ApiQuery(ActionQuery, site));
    ApiQuery *qr = this->LoginQueries[site];
    qr->IncRef();
    qr->Parameters = "meta=tokens&type=login";
    qr->Process();
    this->Statuses[site] = WaitingForLoginQuery;
}

void Login::PerformLoginPart2(WikiSite *site)
{
    // verify if query is already finished
    if (!this->LoginQueries.contains(site) || !this->LoginQueries[site]->IsProcessed())
        return;

    ApiQuery *query = this->LoginQueries[site];
    if (query->IsFailed())
    {
        this->DisplayError(_l("login-fail", site->Name) + ": " + query->GetFailureReason());
        return;
    }
    ApiQueryResultNode *token_info = query->GetApiQueryResult()->GetNode("tokens");
    if (!token_info || !token_info->Attributes.contains("logintoken"))
    {
        this->DisplayError(_l("login-fail", site->Name) + ": No valid login token returned by the site");
        return;
    }
    QString token = token_info->GetAttribute("logintoken");
    this->Tokens.insert(site, token);
    this->Statuses[site] = WaitingForToken;
    this->LoginQueries.remove(site);
    query->DecRef();
    query = new ApiQuery(ActionLogin, site);
    this->LoginQueries.insert(site, query);
    query->HiddenQuery = true;
    query->IncRef();
    if (hcfg->SystemConfig_BotPassword)
    {
        query->Parameters = "lgname=" + QUrl::toPercentEncoding(hcfg->SystemConfig_BotLogin)
                + "&lgpassword=" + QUrl::toPercentEncoding(hcfg->TemporaryConfig_Password)
                + "&lgtoken=" + QUrl::toPercentEncoding(token);
    } else
    {
        query->Parameters = "lgname=" + QUrl::toPercentEncoding(hcfg->SystemConfig_Username)
                + "&lgpassword=" + QUrl::toPercentEncoding(hcfg->TemporaryConfig_Password)
                + "&lgtoken=" + QUrl::toPercentEncoding(token);
    }
    query->UsingPOST = true;
    query->Process();
}

bool Login::RetrieveGlobalConfig()
{
    if (this->qConfig != nullptr)
    {
        if (this->qConfig->IsProcessed())
        {
            if (this->qConfig->Result->IsFailed())
            {
                this->DisplayError(_l("[[login-error-global]]") + ": " + this->qConfig->Result->ErrorMessage);
                return false;
            }
            ApiQueryResultNode *data = this->qConfig->GetApiQueryResult()->GetNode("rev");
            if (data == nullptr)
            {
                this->DisplayError("Login failed unable to retrieve global config, the api query returned no data");
                return false;
            }
            if (hcfg->ParseGlobalConfig(data->Value))
            {
                if (!hcfg->GlobalConfig_EnableAll)
                {
                    this->DisplayError(_l("login-error-alldisabled"));
                    return false;
                }
                this->GlobalConfig = true;
                this->loadingForm->ModifyIcon(this->GlobalRow, LoadingForm_Icon_Success);
                return true;
            }
            Syslog::HuggleLogs->DebugLog(data->Value);
            this->qConfig.Delete();
            this->DisplayError(_l("login-error-global"));
            return false;
        }
        return false;
    }
    this->loadingForm->ModifyIcon(this->GlobalRow, LoadingForm_Icon_Loading);
    this->Update(_l("[[login-progress-global]]"));
    this->qConfig = new ApiQuery(ActionQuery);
    this->qConfig->OverrideWiki = hcfg->GlobalConfigurationWikiAddress;
    this->qConfig->Parameters = "prop=revisions&rvprop=content&rvlimit=1&titles=" + hcfg->SystemConfig_GlobalConfig;
    this->qConfig->Process();
    return false;
}

void Login::FinishLogin(WikiSite *site)
{
    if (!this->LoginQueries.contains(site) || !this->LoginQueries[site]->IsProcessed())
        return;

    ApiQuery *query = this->LoginQueries[site];
    if (query->Result->IsFailed())
    {
        this->DisplayError("Login failed (on " + site->Name + "): " + query->GetFailureReason());
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
        qr->Parameters = "meta=siteinfo&siprop=" + QUrl::toPercentEncoding("namespaces|general|extensions|restrictions|usergroups");
        qr->Process();
        qr = new ApiQuery(ActionQuery, site);
        this->qTokenInfo.insert(site, qr);
        qr->IncRef();
        qr->Parameters = "meta=tokens&type=" + QUrl::toPercentEncoding("csrf|patrol|rollback|watch");
        qr->Process();
        this->Statuses[site] = RetrievingProjectConfig;
    }
}

void Login::RetrieveWhitelist(WikiSite *site)
{
    // if whitelist is not defined in config we don't need to do this
    if (!hcfg->SystemConfig_WhitelistDisabled && Configuration::HuggleConfiguration->GlobalConfig_Whitelist.isEmpty())
    {
        Syslog::HuggleLogs->WarningLog("There is no whitelist defined in global config, disabling whitelist globally");
        hcfg->SystemConfig_WhitelistDisabled = true;
    }
    if (hcfg->SystemConfig_WhitelistDisabled)
    {
        this->loadingForm->ModifyIcon(this->GetRowIDForSite(site, LOGINFORM_WHITELIST), LoadingForm_Icon_Failed);
        this->processedWL[site] = true;
        return;
    }
    if (this->WhitelistQueries.contains(site))
    {
        WLQuery *query = this->WhitelistQueries[site];
        if (query->IsProcessed())
        {
            this->WhitelistQueries.remove(site);
            if (query->IsFailed())
            {
                //! \todo This needs to be handled per project, there is no point in disabling WL on all projects
                hcfg->SystemConfig_WhitelistDisabled = true;
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
            if (query->IsFailed())
            {
                this->DisplayError(_l("login-error-config", site->Name, query->Result->ErrorMessage));
                return;
            }
            ApiQueryResultNode *data = query->GetApiQueryResult()->GetNode("rev");
            if (data == nullptr)
            {
                this->DisplayError(_l("login-error-config", site->Name, "the api query returned no data"));
                return;
            }
            QString value = data->Value;
            this->LoginQueries.remove(site);
            query->DecRef();
            // since now data may be deleted
            if (site->ProjectConfig == nullptr)
                throw new Huggle::NullPointerException("site->ProjectConfig", BOOST_CURRENT_FUNCTION);
            QString reason;
            if (site->GetProjectConfig()->Parse(value, &reason, site))
            {
                if (!site->GetProjectConfig()->EnableAll)
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
    query->Parameters = "prop=revisions&rvprop=content&rvlimit=1&titles=Project:Huggle/Config";
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
            if (q->IsFailed())
            {
                this->DisplayError("Login failed unable to retrieve user config: " + q->Result->ErrorMessage);
                return;
            }
            ApiQueryResultNode *data = q->GetApiQueryResult()->GetNode("rev");
            if (data == nullptr) // page is missing
            {
                HUGGLE_DEBUG("User config is missing on " + site->Name, 2);
                if (this->LoadedOldConfigs[site] == false && !hcfg->GlobalConfig_UserConf_old.isEmpty())
                {
                    // try first with old location of config, we don't need to switch the login step here we just
                    // replace the old query with new query that retrieves the old config and call this function
                    // once more, trying to parse the old config
                    this->LoadedOldConfigs[site] = true;
                    HUGGLE_DEBUG1("couldn't find user config for " + site->Name + " at new location, trying old one");
                    q->DecRef();
                    // let's get an old configuration instead
                    q = new ApiQuery(ActionQuery, site);
                    this->LoginQueries[site] = q;
                    q->IncRef();
                    QString page = hcfg->GlobalConfig_UserConf_old;
                    page = page.replace("$1", hcfg->SystemConfig_Username);
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
                HUGGLE_DEBUG1(q->Result->Data);
                this->DisplayError(_l("login-fail-css", site->Name));
                return;
            }
            QString val_ = data->Value;
            this->LoginQueries.remove(site);
            q->DecRef();
            if (site->GetUserConfig()->ParseUserConfig(val_, site->GetProjectConfig(), site == hcfg->Project))
            {
                if (this->LoadedOldConfigs[site])
                {
                    // if we loaded the old config we write that to debug log because othewise we hardly check this
                    // piece of code really works
                    Syslog::HuggleLogs->DebugLog("We successfully loaded and converted the old config for " + site->Name + " (huggle.css) :)");
                }
                if (!site->ProjectConfig->EnableAll)
                {
                    this->DisplayError(_l("login-fail-enable-true", site->Name));
                    return;
                }
                hcfg->NormalizeConf(site);
                this->loadingForm->ModifyIcon(this->GetRowIDForSite(site, LOGINFORM_USERCONFIG), LoadingForm_Icon_Success);
                this->Statuses[site] = RetrievingUser;
                return;
            }
            // failed unable to parse the user config
            Syslog::HuggleLogs->DebugLog(val_);
            this->DisplayError(_l("login-fail-parse-config", site->Name));
        }
        return;
    }
    this->loadingForm->ModifyIcon(this->GetRowIDForSite(site, LOGINFORM_USERCONFIG), LoadingForm_Icon_Loading);
    ApiQuery *query = new ApiQuery(ActionQuery, site);
    query->IncRef();
    this->LoginQueries.insert(site, query);
    QString page = hcfg->GlobalConfig_UserConf;
    page = page.replace("$1", hcfg->SystemConfig_Username);
    query->Parameters = "prop=revisions&rvprop=content&rvlimit=1&titles=" + QUrl::toPercentEncoding(page);
    query->Process();
}

void Login::RetrieveUserInfo(WikiSite *site)
{
    // check approval page
    if (this->qApproval.contains(site))
    {
        ApiQuery *query = this->qApproval[site];
        if (query->IsProcessed())
        {
            QString result;
            bool failed = false;
            result = Generic::EvaluateWikiPageContents(query, &failed);
            if (failed)
            {
                this->DisplayError(_l("unable-to-retrieve-user-list", result));
                return;
            }
            QStringList users = result.toLower().split("\n");
            QStringList sanitized;
            // sanitize user list
            foreach(QString user, users)
            {
                user = user.replace("_", " ");
                user = user.trimmed();
                sanitized.append(user);
            }
            QString sanitized_name = hcfg->SystemConfig_Username;
            sanitized_name = sanitized_name.toLower();
            sanitized_name = sanitized_name.replace("_", " ");
            if (!sanitized.contains("* [[special:contributions/" + sanitized_name + "|" + sanitized_name + "]]"))
            {
                if (site->GetProjectConfig()->Approval)
                {
                    this->DisplayError(_l("login-error-approval", site->Name));
                    return;
                }
                if (site->GetProjectConfig()->UserlistSync)
                {
                    // we need to insert this user into the list
                    QString un = WikiUtil::SanitizeUser(hcfg->SystemConfig_Username);
                    QString line = "* [[Special:Contributions/" + un + "|" + un + "]]";
                    result += "\n" + line;
                    QString summary = site->GetProjectConfig()->UserlistUpdateSummary;
                    summary.replace("$1", hcfg->SystemConfig_Username);
                    WikiUtil::EditPage(site->GetProjectConfig()->ApprovalPage, result, summary, false, "", 0, site);
                    //WikiUtil::AppendTextToPage(site->GetProjectConfig()->ApprovalPage, line, site->GetProjectConfig()->UserlistUpdateSummary, true, site);
                }
            }
            this->loadingForm->ModifyIcon(this->GetRowIDForSite(site, LOGINFORM_USERINFO), LoadingForm_Icon_Success);
            this->processedLogin[site] = true;
            this->Statuses[site] = LoginDone;
        }
        return;
    }

    if (this->LoginQueries.contains(site))
    {
        ApiQuery *query = this->LoginQueries[site];
        if (query->IsProcessed())
        {
            if (query->IsFailed())
            {
                this->DisplayError(_l("login-fail-no-info", site->Name, query->GetFailureReason()));
                return;
            }
            QList<ApiQueryResultNode*> node = query->GetApiQueryResult()->GetNodes("r");
            if (node.count() == 0)
            {
                HUGGLE_DEBUG1(query->Result->Data);
                // Login failed unable to retrieve user info since the api query returned no data
                this->DisplayError(_l("login-fail-user-data", site->Name));
                return;
            }
            int c=0;
            while(c<node.count())
            {
                site->GetProjectConfig()->Rights.append(node.at(c)->Value);
                c++;
            }
            if (site->GetProjectConfig()->RequireRollback && !site->GetProjectConfig()->Rights.contains("rollback"))
            {
                QString reason = _l("login-fail-rollback-rights", site->Name);
                if (!this->ReadonlyFallback(site, reason))
                {
                    this->DisplayError(reason);
                    return;
                }
            }
            if (site->GetProjectConfig()->RequireAutoconfirmed && (!site->GetProjectConfig()->Rights.contains("autoconfirmed") &&
                                                                   !site->GetProjectConfig()->Rights.contains("confirmed")))
            {
                QString reason = _l("login-failed-autoconfirm-rights", site->Name);
                if (!this->ReadonlyFallback(site, reason))
                {
                    this->DisplayError(reason);
                    return;
                }
            }
            // remove the query
            this->LoginQueries.remove(site);
            ApiQueryResultNode* ui = query->GetApiQueryResult()->GetNode("userinfo");
            if (!ui)
            {
                this->DisplayError(_l("login-no-userinfo"));
                return;
            }
            int editcount = ui->GetAttribute("editcount", "-1").toInt();
            query->DecRef();
            if (site->GetProjectConfig()->RequireTime)
            {
                QString registration_info = ui->GetAttribute("registrationdate");
                if (registration_info.isEmpty())
                {
                    QString reason = _l("login-invalid-register-date");
                    if (!this->ReadonlyFallback(site, reason))
                    {
                        this->DisplayError(reason);
                        return;
                    }
                }
                QDateTime registration_date = MediaWiki::FromMWTimestamp(registration_info);
                if (registration_date.addDays(site->GetProjectConfig()->RequireTime) > QDateTime::currentDateTime())
                {
                    QString reason = _l("login-error-age", QString::number(site->GetProjectConfig()->RequireTime));
                    if (!this->ReadonlyFallback(site, reason))
                    {
                        this->DisplayError(reason);
                        return;
                    }
                }
            }
            if (site->GetProjectConfig()->RequireEdits > editcount)
            {
                QString reason = _l("login-failed-edit", site->Name);
                if (!this->ReadonlyFallback(site, reason))
                {
                    this->DisplayError(reason);
                    return;
                }
            }

            // So now we passed all checks if we can use huggle, so let's update the user list in case we want that
            if (!site->GetProjectConfig()->ReadOnly && (site->GetProjectConfig()->UserlistSync || site->GetProjectConfig()->Approval))
            {
                this->qApproval.insert(site, Generic::RetrieveWikiPageContents(site->GetProjectConfig()->ApprovalPage, site));
                this->qApproval[site]->IncRef();
                this->qApproval[site]->Process();
                return;
            }
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
    temp->Parameters = "meta=userinfo&uiprop=" + QUrl::toPercentEncoding("rights|registrationdate|editcount");
    temp->Process();
    this->LoginQueries.insert(site, temp);
}

void Login::DeveloperMode()
{
    // load dummy project config files
    QFile *vf;
    vf = new QFile(":/huggle/resources/Resources/DefaultConfig.txt");
    vf->open(QIODevice::ReadOnly);
    QString pref = QString(vf->readAll());
    vf->close();
    delete vf;
    foreach (WikiSite *site, hcfg->Projects)
        site->GetProjectConfig()->Parse(pref, nullptr, site);
    hcfg->Restricted = true;
    MainWindow::HuggleMain = new MainWindow();
    MainWindow::HuggleMain->show();
    this->hide();
}

void Login::ProcessSiteInfo(WikiSite *site)
{
    if (this->qTokenInfo.contains(site) && this->qSiteInfo.contains(site)
            && this->qTokenInfo[site]->IsProcessed() && this->qSiteInfo[site]->IsProcessed())
    {
        if (this->qSiteInfo[site]->IsFailed())
        {
            this->DisplayError(_l("login-site-info-query-failed", site->Name, this->qSiteInfo[site]->GetFailureReason()));
            return;
        }
        if (!this->qTokenInfo[site]->IsFailed())
        {
            // if this query failed user is probably on older mediawiki, that means some features will not work
            // but there is no reason why we should abort whole login operation just because of that
            ApiQueryResultNode *tokens = this->qTokenInfo[site]->GetApiQueryResult()->GetNode("tokens");
            if (tokens != nullptr)
            {
                if (tokens->Attributes.contains("rollbacktoken"))
                {
                    site->GetProjectConfig()->Token_Rollback = tokens->GetAttribute("rollbacktoken");
                    HUGGLE_DEBUG("Token for " + site->Name + " rollback " + site->GetProjectConfig()->Token_Rollback, 2);
                } else
                {
                    HUGGLE_DEBUG1("No rollback for " + site->Name + " result: " + this->qTokenInfo[site]->Result->Data);
                }
                if (tokens->Attributes.contains("csrftoken"))
                {
                    site->GetProjectConfig()->Token_Csrf = tokens->GetAttribute("csrftoken");
                    HUGGLE_DEBUG("Token for " + site->Name + " csrf " + site->GetProjectConfig()->Token_Csrf, 2);
                } else
                {
                    HUGGLE_DEBUG1("No csrf for " + site->Name + " result: " + this->qTokenInfo[site]->Result->Data);
                }
                if (tokens->Attributes.contains("watchtoken"))
                {
                    site->GetProjectConfig()->Token_Watch = tokens->GetAttribute("watchtoken");
                    HUGGLE_DEBUG("Token for " + site->Name + " watch " + site->GetProjectConfig()->Token_Watch, 2);
                } else
                {
                    HUGGLE_DEBUG1("No watch for " + site->Name + " result: " + this->qTokenInfo[site]->Result->Data);
                }
                if (tokens->Attributes.contains("patroltoken"))
                {
                    site->GetProjectConfig()->Token_Patrol = tokens->GetAttribute("patroltoken");
                    HUGGLE_DEBUG("Token for " + site->Name + " patrol " + site->GetProjectConfig()->Token_Patrol, 2);
                } else
                {
                    HUGGLE_DEBUG1("No patrol for " + site->Name + " result: " + this->qTokenInfo[site]->Result->Data);
                }
            }
        } else
        {
            Syslog::HuggleLogs->WarningLog("Tokens query for " + site->Name + " has failed: " + this->qTokenInfo[site]->GetFailureReason());
        }
        // we can remove the query no matter if it was finished or not
        this->qTokenInfo[site]->DecRef();
        this->qTokenInfo.remove(site);
        ApiQueryResultNode *g_ = this->qSiteInfo[site]->GetApiQueryResult()->GetNode("general");
        if( g_ == nullptr )
        {
            this->DisplayError(_l("login-no-site-info-returned"));
            return;
        }
        if (g_->Attributes.contains("rtl"))
            site->IsRightToLeft = true;

        if (g_->Attributes.contains("generator"))
        {
            QString vr = g_->GetAttribute("generator");
            if (!vr.contains(" "))
            {
                Syslog::HuggleLogs->WarningLog("Mediawiki of " + site->Name + " has some invalid version: " + vr);
            } else
            {
                vr = vr.mid(vr.indexOf(" ") + 1);
                site->MediawikiVersion = Version(vr);
                HUGGLE_DEBUG1(site->Name + " mediawiki " + site->MediawikiVersion.ToString());
                if (site->MediawikiVersion < Version::SupportedMediawiki)
                    Syslog::HuggleLogs->WarningLog("Mediawiki of " + site->Name + " is using version " + site->MediawikiVersion.ToString() +
                                                   " which isn't supported by huggle");
            }
        } else
        {
            Syslog::HuggleLogs->WarningLog("MediaWiki of " + site->Name + " provides no version");
        }
        if (g_->Attributes.contains("time"))
        {
            QDateTime server_time = MediaWiki::FromMWTimestamp(g_->GetAttribute("time"));
            site->GetProjectConfig()->ServerOffset = QDateTime::currentDateTime().secsTo(server_time);
        }
        QList<ApiQueryResultNode*> ns = this->qSiteInfo[site]->GetApiQueryResult()->GetNodes("ns");
        if (ns.count() < 1)
        {
            Syslog::HuggleLogs->WarningLog(QString("Mediawiki of ") + site->Name + " provided no information about namespaces");
        } else
        {
            // let's prepare a NS list
            site->ClearNS();
            int index = 0;
            while (index < ns.count())
            {
                ApiQueryResultNode *node = ns.at(index);
                index++;
                if (!node->Attributes.contains("id") || !node->Attributes.contains("canonical"))
                    continue;
                site->InsertNS(new WikiPageNS(node->GetAttribute("id").toInt(), node->Value, node->GetAttribute("canonical")));
            }
        }
        // extensions
        site->Extensions.clear();
        QList<ApiQueryResultNode*> extensionlist = this->qSiteInfo[site]->GetApiQueryResult()->GetNodes("ext");
        foreach(ApiQueryResultNode *ext, extensionlist)
        {
            site->Extensions.append(WikiSite_Ext(ext->GetAttribute("name", "unknown"), ext->GetAttribute("type", "unknown"),
                      ext->GetAttribute("descriptionmsg", "unknown"), ext->GetAttribute("author", "unknown"),
                      ext->GetAttribute("url", "unknown"), ext->GetAttribute("version", "0")));
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
    this->Update(QString("<span style=\"color: red;\">%1</span>").arg(message));
    QMessageBox::critical(this->window(), _l("error"), message);
}

void Login::Finish()
{
    // let's check if all processes are finished
    foreach (WikiSite *xx, hcfg->Projects)
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
    foreach (WikiSite *site, hcfg->Projects)
    {
        site->GetProjectConfig()->IsLoggedIn = true;
        this->Statuses[site] = Nothing;
    }
    // we generate a random string of same size of current password
    while (pw.length() < hcfg->TemporaryConfig_Password.length())
    {
        pw += "x";
    }
    // we no longer need a password since this
    hcfg->TemporaryConfig_Password = pw;
    this->ui->lineEdit_password->setText(pw);
    this->Update(_l("loading-main-window"));
    this->timer->stop();
    this->hide();
    MainWindow::HuggleMain = new MainWindow();
    MainWindow::HuggleMain->show();
    if (this->loadingForm != nullptr)
    {
        this->loadingForm->close();
        delete this->loadingForm;
        this->loadingForm = nullptr;
        LoadingForm::IsKilled = true;
    }
    // now we can safely delete this form
    Core::HuggleCore->fLogin = nullptr;
    delete this;
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
    ApiQueryResult *result = query->GetApiQueryResult();
    ApiQueryResultNode *ln = result->GetNode("login");
    QString result_code = ln->GetAttribute("result");
    QString reason = ln->GetAttribute("reason");
    if (result_code.isEmpty())
    {
        this->DisplayError(_l("api.php-invalid-response"));
        return false;
    }
    if (result_code == "Success")
        return true;
    if (result_code == "EmptyPass")
    {
        this->DisplayError(_l("login-password-empty"));
        return false;
    }
    if (result_code == "WrongPass")
    {
        /// \bug This sometimes doesn't work properly
        this->ui->lineEdit_password->setFocus();
        this->DisplayError(_l("login-error-password"));
        return false;
    }
    if (result_code == "NoName")
    {
        this->DisplayError(_l("login-fail-wrong-name"));
        return false;
    }
    if (result_code == "NotExists")
    {
        this->DisplayError(_l("login-username-doesnt-exist"));
        return false;
    }
    if (reason.isEmpty())
        reason = result_code;
    this->DisplayError(_l("login-api", reason));
    return false;
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
    if (!this->GlobalConfig && !this->RetrieveGlobalConfig())
        return;
    // let's check status for every single project
    foreach (WikiSite *site, hcfg->Projects)
    {
        if (!this->Statuses.contains(site))
            throw new Huggle::Exception("There is no such a wiki in statuses list", BOOST_CURRENT_FUNCTION);
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
                this->PerformLoginPart2(site);
                break;
            case WaitingForToken:
                this->FinishLogin(site);
                break;
            case RetrievingProjectConfig:
                this->RetrieveProjectConfig(site);
                break;
            case RetrievingUserConfig:
                this->RetrieveUserConfig(site);
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
    hcfg->SystemConfig_UsingSSL = this->ui->checkBox->isChecked();
    this->timer->start(HUGGLE_TIMER);
    this->qDatabase->OverrideWiki = hcfg->GlobalConfigurationWikiAddress;
    this->ui->ButtonOK->setText(_l("[[cancel]]"));
    this->qDatabase->Parameters = "prop=revisions&rvprop=content&rvlimit=1&titles=" + hcfg->SystemConfig_GlobalConfigWikiList;
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
    if(((this->ui->lineEditBotUser->text().size() == 0 && this->ui->lineEdit_username->text().size() == 0) ||
        (this->ui->lineEdit_password->text().size() == 0) && this->ui->lineEditBotP->text().size() == 0 ) &&
       (this->ui->lineEdit_username->text() != "Developer Mode" &&
        this->ui->lineEdit_username->text() != "Developer_Mode"))
        this->ui->ButtonOK->setEnabled( false );
    else
        this->ui->ButtonOK->setEnabled( true );
}

int Login::RegisterLoadingFormRow(WikiSite *site, int row)
{
    this->LoadingFormRows[site].insert(row, this->LastRow);
    return this->LastRow++;
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
        this->ui->pushButton_2->setText(_l("projects") + " >>");
        this->ui->tableWidget->setVisible(false);
        hcfg->SystemConfig_Multiple = false;
    } else
    {
        hcfg->SystemConfig_Multiple = true;
        this->ui->pushButton_2->setText(_l("projects") + " <<");
        this->ui->tableWidget->setVisible(true);
        if (this->height() < 460)
            this->resize(this->width(), 480);
    }
}

void Huggle::Login::on_label_linkActivated(const QString &link)
{
    Q_UNUSED(link);
    Proxy pr;
    pr.exec();
}

void Huggle::Login::on_lineEditBotUser_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    Login::VerifyLogin();
}

void Huggle::Login::on_lineEditBotP_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    Login::VerifyLogin();
}

void Huggle::Login::on_label_2_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(link);
}

void Huggle::Login::on_tabWidget_currentChanged(int index)
{
    if (index == 0)
    {
        if (this->ui->lineEditBotUser->text().isEmpty() && !this->ui->lineEdit_username->text().isEmpty())
            this->ui->lineEditBotUser->setText(this->ui->lineEdit_username->text() + "@huggle");
    }
}
