//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include <huggle_core/apiquery.hpp>
#include <huggle_core/apiqueryresult.hpp>
#include <huggle_core/configuration.hpp>
#include <huggle_core/core.hpp>
#include <huggle_core/exception.hpp>
#include <huggle_core/generic.hpp>
#include <huggle_core/huggleprofiler.hpp>
#include <huggle_core/hugglequeuefilter.hpp>
#include <huggle_core/localization.hpp>
#include <huggle_core/mediawiki.hpp>
#include <huggle_core/syslog.hpp>
#include <huggle_core/wikisite.hpp>
#include <huggle_core/wikiutil.hpp>
#include <QCheckBox>
#include <QFile>
#include <QUrl>
#include <QSslSocket>
#include <QDesktopServices>
#include <QMessageBox>
#include "loginform.hpp"
#include "mainwindow.hpp"
#include "loadingform.hpp"
#include "proxy.hpp"
#include "welcomeinfo.hpp"
#include "uigeneric.hpp"
#include "uihooks.hpp"
#include "updateform.hpp"
#include "ui_loginform.h"

#define LOGINFORM_LOGIN 0
#define LOGINFORM_SITEINFO 1
#define LOGINFORM_WHITELIST 3
#define LOGINFORM_LOCALCONFIG 4
#define LOGINFORM_USERCONFIG 5
#define LOGINFORM_USERINFO 6
#define LOGINFORM_YAMLCONFIG 7

using namespace Huggle;

LoginForm::LoginForm(QWidget *parent) : HW("login", this, parent), ui(new Ui::Login)
{
    HUGGLE_PROFILER_RESET;
    this->isLoading = true;
    if (!hcfg->Login && (hcfg->SystemConfig_FirstRun || hcfg->SystemConfig_ShowStartupInfo))
    {
        WelcomeInfo info;
        info.exec();
    }
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
    this->resetForm();
    this->ui->checkBox->setChecked(hcfg->SystemConfig_UsingSSL);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    int localization_ix=0, preferred=0;
    while (localization_ix<Localizations::HuggleLocalizations->LocalizationData.count())
    {
        this->ui->Language->addItem(Localizations::HuggleLocalizations->LocalizationData.at(localization_ix)->LanguageID);
        if (Localizations::HuggleLocalizations->LocalizationData.at(localization_ix)->LanguageName == Localizations::HuggleLocalizations->PreferredLanguage)
            preferred = localization_ix;

        localization_ix++;
    }
    QString title = "Huggle 3";
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
    this->reloadForm();
    if (hcfg->TemporaryConfig_LoginFile)
        hcfg->SystemConfig_BotPassword = true;
    if (!QSslSocket::supportsSsl())
    {
        hcfg->SystemConfig_UsingSSL = false;
        this->ui->checkBox->setEnabled(false);
        this->ui->checkBox->setChecked(false);
#if QT_VERSION >= 0x050400
        UiGeneric::pMessageBox(this, "Unable to find SSL libraries", "Huggle was unable to locate SSL libraries. This build requires:\nSSL version: " + QSslSocket::sslLibraryBuildVersionString() +
                                                                     "\nQt (built on): " + QString(QT_VERSION_STR) + "\nQt (running): " + QString(qVersion()), MessageBoxStyleError);
#endif
    }
    if (hcfg->SystemConfig_EnableUpdates)
    {
        this->updateForm = new UpdateForm(this);
        this->updateForm->Check();
    }
    if (hcfg->SystemConfig_BotPassword)
    {
        this->ui->tab_oauth->show();
        if (!hcfg->SystemConfig_UserName.isEmpty())
        {
            this->ui->lineEditBotUser->setText(hcfg->SystemConfig_BotLogin);
            this->ui->lineEditBotP->setFocus();
        }
    } else
    {
        this->ui->tab_login->show();
        if (!hcfg->SystemConfig_UserName.isEmpty())
        {
            this->ui->lineEdit_username->setText(hcfg->SystemConfig_UserName);
            this->ui->lineEdit_password->setFocus();
        }
    }
    this->isLoading = false;
    this->Localize();
    // Load the proxy
    if (hcfg->SystemConfig_UseProxy)
    {
        Proxy::SetProxy(hcfg->SystemConfig_ProxyType, hcfg->SystemConfig_ProxyHost, hcfg->SystemConfig_ProxyPort,
            hcfg->SystemConfig_ProxyUser, hcfg->SystemConfig_ProxyPass);
    }
    if (hcfg->SystemConfig_BotPassword)
        this->ui->tabWidget->setCurrentIndex(0);
    else
        this->ui->tabWidget->setCurrentIndex(1);
    HUGGLE_PROFILER_PRINT_TIME(BOOST_CURRENT_FUNCTION);
    // Finished loading the login form
#if QT_VERSION >= 0x050400
    HUGGLE_DEBUG1("SSL library: " + QSslSocket::sslLibraryBuildVersionString());
#endif
    if (!hcfg->GlobalConfig_OverrideConfigYAMLPath.isEmpty())
        UiGeneric::MessageBox(_l("warning"), _l("override-warn"), MessageBoxStyleWarning, false, this);

    // Load the stored credentials, if we have them
    if (hcfg->SystemConfig_StorePassword && !hcfg->TemporaryConfig_LoginFile)
    {
        if (hcfg->SystemConfig_BotPassword)
            this->ui->lineEditBotP->setText(hcfg->SystemConfig_RememberedPassword);
        else
            this->ui->lineEdit_password->setText(hcfg->SystemConfig_RememberedPassword);
        this->ui->checkBox_2->setChecked(true);
    }

    // If we are using login file, override the stored password (and ignore password updates)
    if (hcfg->TemporaryConfig_LoginFile)
    {
        this->ui->lineEditBotP->setText(hcfg->TemporaryConfig_Password);
        this->ui->lineEditBotUser->setText(hcfg->SystemConfig_BotLogin);
        this->ui->checkBox_2->setChecked(false);
    }

    // Check if we wanted to login using --login option
    if (hcfg->Login)
    {
        // user wanted to login using a terminal
        this->loginInProgress = true;
        this->pressOK();
    }
    this->RestoreWindow();
    UiHooks::LoginForm_OnLoad(this);
}

LoginForm::~LoginForm()
{
    this->removeQueries();
    delete this->updateForm;
    delete this->ui;
    delete this->loadingForm;
    LoadingForm::IsKilled = true;
    delete this->timer;
}

void LoginForm::Localize()
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
    this->ui->labelUsername->setText(_l("login-username"));
    this->ui->labelProject->setText(_l("login-project"));
    this->ui->labelLanguage->setText(_l("login-language"));
    this->ui->label_2->setText("<a href=\"https://www.mediawiki.org/wiki/Manual:Huggle/Bot_passwords\">" + _l("login-bot") + "</a>");
    this->ui->labelPassword->setText(_l("login-password"));
    this->ui->checkBox_2->setText(_l("login-remember-password"));
    this->ui->checkBox_2->setToolTip(_l("login-remember-password-tooltip"));
    this->ui->labelIntro->setText(_l("login-intro"));
    if (!this->ui->tableWidget->isVisible())
        this->ui->pushButton_2->setText(_l("projects") + " >>");
    else
        this->ui->pushButton_2->setText(_l("projects") + " <<");
    this->ui->label->setText("<a href='dummy'>" + _l("login-proxy") + "</a>");
    this->ui->labelTranslate->setText(QString("<html><head/><body><p><a href=\"https://meta.wikimedia.org/wiki/Huggle/Localization\"><span style=\""\
                                              " text-decoration: underline; color:#0000ff;\">%1</span></a></p></body></html>").arg(_l("login-translate")));
    // Change the layout based on preference
    if (Localizations::HuggleLocalizations->IsRTL())
        QApplication::setLayoutDirection(Qt::RightToLeft);
    else
        QApplication::setLayoutDirection(Qt::LeftToRight);
}

void LoginForm::Update(QString ms)
{
     // let's just pass it there
    if (this->loadingForm != nullptr)
        this->loadingForm->Info(ms);

    // update the label
    this->ui->labelIntro->setText(ms);
}

void LoginForm::resetForm()
{
    this->ui->labelIntro->setText(_l("[[login-intro]]"));
}

void LoginForm::removeQueries()
{
    QList<WikiSite*> Sites = this->LoginQueries.keys();
    foreach (WikiSite* st, Sites)
        this->LoginQueries[st]->DecRef();
    Sites = this->wlQueries.keys();
    foreach (WikiSite* st, Sites)
        this->wlQueries[st]->DecRef();
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
    this->wlQueries.clear();
    this->LoginQueries.clear();
    this->qCurrentLoginRequest.Delete();
}

void LoginForm::CancelLogin()
{
    // No longer a login file managed stuff
    hcfg->TemporaryConfig_LoginFile = false;
    this->loginInProgress = false;
    LoadingForm::IsKilled = true;
    this->timer->stop();
    this->enableForm();
    if (this->loadingForm != nullptr)
    {
        this->loadingForm->close();
        delete this->loadingForm;
        this->loadingForm = nullptr;
    }
    this->ui->labelIntro->setText(_l("login-intro"));
    if (hcfg->SystemConfig_StorePassword)
        this->ui->lineEdit_password->setText(hcfg->SystemConfig_RememberedPassword);
    else
        this->ui->lineEdit_password->setText("");
    this->ui->ButtonOK->setText(_l("login-start"));
    this->removeQueries();
}

int LoginForm::GetRowIDForSite(WikiSite *site, int row)
{
    if (!this->loadingFormRows.contains(site))
        throw new Huggle::Exception("There is no such a site in DB of rows", BOOST_CURRENT_FUNCTION);

    if (!this->loadingFormRows[site].contains(row))
        throw new Huggle::Exception("There is no such a row in DB of rows", BOOST_CURRENT_FUNCTION);

    return this->loadingFormRows[site][row];
}

void LoginForm::enableForm(bool value)
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

void LoginForm::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    this->reject();
}

bool LoginForm::readonlyFallback(WikiSite *site, QString why)
{
    if (site->GetProjectConfig()->ReadOnly)
        return true;

    if (UiGeneric::MessageBox(_l("login-ro-title"), _l("login-ro-question", site->Name, why), MessageBoxStyleQuestion, true, this) == QMessageBox::No)
        return false;

    Syslog::HuggleLogs->WarningLog(_l("login-ro-info", site->Name));
    site->GetProjectConfig()->ReadOnly = true;
    return true;
}

void LoginForm::disableForm()
{
    this->enableForm(false);
}

void LoginForm::reloadForm()
{
    int current = 0;
    this->ui->Project->clear();
    while (this->ui->tableWidget->rowCount() > 0)
        this->ui->tableWidget->removeRow(0);
    this->project_CheckBoxens.clear();
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
        this->project_CheckBoxens.append(Item);
        this->ui->tableWidget->setCellWidget(current, 1, Item);
        current++;
    }
    this->ui->tableWidget->resizeColumnsToContents();
    this->ui->tableWidget->resizeRowsToContents();
    if (hcfg->SystemConfig_IndexOfLastWiki < current)
        this->ui->Project->setCurrentIndex(hcfg->SystemConfig_IndexOfLastWiki);
    else
        this->ui->Project->setCurrentIndex(0);
}

void LoginForm::reloadWikiDB()
{
    if (this->qDatabase == nullptr || !this->qDatabase->IsProcessed())
        return;

    if (this->qDatabase->IsFailed())
    {
        this->displayError(_l("wikis-db-download-fail", this->qDatabase->GetFailureReason()));
        this->timer->stop();
        this->enableForm();
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
        this->reloadForm();
    }
    this->timer->stop();
    this->enableForm();
    this->Localize();
    this->Refreshing = false;
}

void LoginForm::pressOK()
{
    this->globalConfigIsLoaded = false;
    hcfg->TemporaryConfig_UserNameWasChanged = false;
    hcfg->SystemConfig_BotPassword = this->ui->tab_oauth->isVisible();
    // If we use login file, it's always a bot password
    if (hcfg->TemporaryConfig_LoginFile)
        hcfg->SystemConfig_BotPassword = true;
    // Simple hack - some users are aware of bot passwords but not that you need to explicitly specify if you use them in Huggle
    if (!hcfg->SystemConfig_BotPassword && this->ui->lineEdit_username->text().contains("@"))
    {
        // toggle to bot passwords
        this->ui->tabWidget->setCurrentIndex(0);
        this->ui->lineEditBotUser->setText(this->ui->lineEdit_username->text());
        this->ui->lineEditBotP->setText(this->ui->lineEdit_password->text());
        hcfg->SystemConfig_BotPassword = true;
    }
    if (this->ui->Project->count() == 0)
    {
        // there are no projects in login form
        UiGeneric::pMessageBox(this, _l("error"), _l("no-projects-defined-in-list"));
        return;
    }
    hcfg->SystemConfig_StorePassword = this->ui->checkBox_2->isChecked();
    if (hcfg->SystemConfig_BotPassword)
    {
        hcfg->SystemConfig_BotLogin = this->ui->lineEditBotUser->text();
        if (!this->ui->lineEditBotUser->text().contains("@"))
        {
            this->displayError(_l("invalid-bot-user-name"));
            return;
        }
        QString name = hcfg->SystemConfig_BotLogin;
        hcfg->SystemConfig_UserName = WikiUtil::SanitizeUser(name.mid(0, name.indexOf("@")));
        if (hcfg->SystemConfig_StorePassword)
            hcfg->SystemConfig_RememberedPassword = this->ui->lineEditBotP->text();
        hcfg->TemporaryConfig_Password = ui->lineEditBotP->text();
    } else
    {
        if (hcfg->SystemConfig_StorePassword)
            hcfg->SystemConfig_RememberedPassword = this->ui->lineEdit_password->text();
        hcfg->SystemConfig_UserName = WikiUtil::SanitizeUser(ui->lineEdit_username->text());
        hcfg->TemporaryConfig_Password = ui->lineEdit_password->text();
    }
    hcfg->SystemConfig_IndexOfLastWiki = this->ui->Project->currentIndex();
    hcfg->Project = hcfg->ProjectList.at(this->ui->Project->currentIndex());
    // we need to clear a list of projects we are logged to and insert at least this one
    hcfg->Projects.clear();
    hcfg->SystemConfig_UsingSSL = this->ui->checkBox->isChecked();
    hcfg->Projects << hcfg->Project;
    if (hcfg->SystemConfig_Multiple)
    {
        int project_id = 0;
        foreach (QCheckBox* cb, this->project_CheckBoxens)
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
                this->displayError(_l("ssl-is-not-supported", wiki->Name));
                return;
            }
        }
    } else
    {
        foreach(WikiSite *wiki, hcfg->Projects)
        {
            if (wiki->ForceSSL)
            {
                this->displayError(_l("ssl-required", wiki->Name));
                return;
            }
        }
    }
    if (this->loadingForm != nullptr)
        delete this->loadingForm;

    this->loadingForm = new LoadingForm(this);
    LoadingForm::IsKilled = false;
    // set new status for all projects
    this->usingOldUserConfig.clear();
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
        this->usingOldUserConfig.insert(wiki, false);
        this->Statuses.insert(wiki, LoggingIn);
        this->processedLogin.insert(wiki, false);
        this->processedSiteinfos.insert(wiki, false);
        this->processedWL.insert(wiki, false);
    }
    hcfg->UserConfig = hcfg->Project->GetUserConfig();
    hcfg->ProjectConfig = hcfg->Project->GetProjectConfig();

    if (this->isDeveloperMode())
    {
        this->developerMode();
        return;
    }
    this->disableForm();
    this->loadingForm->show();
    this->loadingForm->setWindowTitle(_l("login-progress-formtitle"));
    // we need to delete the filters here so that if previous login didn't finish we don't reuse the previous ones
    HuggleQueueFilter::Delete();
    // this is pretty tricky here
    // we need to register every single login operation but we also need to remember which row it was on
    // for that we will use some super hash table, but first we need to make it empty...
    this->clearLoadingFormRows();
    foreach (WikiSite *wiki, hcfg->Projects)
    {
        // we need to add this wiki to our super table
        this->loadingFormRows.insert(wiki, QHash<int,int>());
        this->loadingForm->Insert(this->registerLoadingFormRow(wiki, LOGINFORM_LOGIN), _l("login-progress-start", wiki->Name), LoadingForm_Icon_Loading);
        this->loadingForm->Insert(this->registerLoadingFormRow(wiki, LOGINFORM_SITEINFO), _l("login-progress-retrieve-mw", wiki->Name), LoadingForm_Icon_Waiting);
        this->loadingForm->Insert(this->registerLoadingFormRow(wiki, LOGINFORM_WHITELIST), _l("login-progress-whitelist", wiki->Name), LoadingForm_Icon_Waiting);
        this->loadingForm->Insert(this->registerLoadingFormRow(wiki, LOGINFORM_YAMLCONFIG), _l("login-progress-yaml", wiki->Name), LoadingForm_Icon_Waiting);
        this->loadingForm->Insert(this->registerLoadingFormRow(wiki, LOGINFORM_LOCALCONFIG), _l("login-progress-local", wiki->Name), LoadingForm_Icon_Waiting);
        this->loadingForm->Insert(this->registerLoadingFormRow(wiki, LOGINFORM_USERCONFIG), _l("login-progress-user", wiki->Name), LoadingForm_Icon_Waiting);
        this->loadingForm->Insert(this->registerLoadingFormRow(wiki, LOGINFORM_USERINFO), _l("login-progress-user-info", wiki->Name), LoadingForm_Icon_Waiting);
    }
    this->loadingFormGlobalConfigRow = this->loadingForm_LastRow;
    this->loadingForm->Insert(this->loadingForm_LastRow, _l("login-progress-global"), LoadingForm_Icon_Waiting);
    this->loadingForm_LastRow++;
    // First of all, we need to login to the site
    this->timer->start(HUGGLE_TIMER);
}

void LoginForm::performLogin(WikiSite *site)
{
    this->Update(_l("[[login-progress-start]]", site->Name));
    // we create an api request to login
    this->LoginQueries.insert(site, new ApiQuery(ActionQuery, site));
    ApiQuery *qr = this->LoginQueries[site];
    this->qCurrentLoginRequest = qr;
    qr->IncRef();
    qr->Parameters = "meta=tokens&type=login";
    qr->Process();
    this->Statuses[site] = WaitingForLoginQuery;
}

void LoginForm::performLoginPart2(WikiSite *site)
{
    // verify if query is already finished
    if (!this->LoginQueries.contains(site) || !this->LoginQueries[site]->IsProcessed())
        return;

    ApiQuery *query = this->LoginQueries[site];
    if (query->IsFailed())
    {
        this->displayError(_l("login-fail", site->Name) + ": " + query->GetFailureReason());
        return;
    }
    ApiQueryResultNode *token_info = query->GetApiQueryResult()->GetNode("tokens");
    if (!token_info || !token_info->Attributes.contains("logintoken"))
    {
        this->displayError(_l("login-fail-with-reason", site->Name, _l("login-error-no-valid-token")));
        return;
    }
    QString token = token_info->GetAttribute("logintoken");
    this->loginTokens.insert(site, token);
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
        query->Parameters = "lgname=" + QUrl::toPercentEncoding(hcfg->SystemConfig_UserName)
                + "&lgpassword=" + QUrl::toPercentEncoding(hcfg->TemporaryConfig_Password)
                + "&lgtoken=" + QUrl::toPercentEncoding(token);
    }
    query->UsingPOST = true;
    query->Process();
}

bool LoginForm::retrieveGlobalConfig()
{
    if (this->qConfig != nullptr)
    {
        if (this->qConfig->IsProcessed())
        {
            if (this->qConfig->Result->IsFailed())
            {
                this->displayError(_l("[[login-error-global]]") + ": " + this->qConfig->GetFailureReason());
                return false;
            }
            ApiQueryResultNode *data = this->qConfig->GetApiQueryResult()->GetNode("rev");
            if (data == nullptr)
            {
                this->displayError(_l("login-error-config-query-no-data"));
                return false;
            }
            if (hcfg->ParseGlobalConfig(data->Value))
            {
                if (!hcfg->GlobalConfig_EnableAll)
                {
                    this->displayError(_l("login-error-alldisabled"));
                    return false;
                }
                this->globalConfigIsLoaded = true;
                this->loadingForm->ModifyIcon(this->loadingFormGlobalConfigRow, LoadingForm_Icon_Success);
                return true;
            }
            Syslog::HuggleLogs->DebugLog(data->Value);
            this->qConfig.Delete();
            this->displayError(_l("login-error-global"));
            return false;
        }
        return false;
    }
    this->loadingForm->ModifyIcon(this->loadingFormGlobalConfigRow, LoadingForm_Icon_Loading);
    this->Update(_l("[[login-progress-global]]"));
    this->qConfig = new ApiQuery(ActionQuery, hcfg->GlobalWiki);
    this->qConfig->OverrideWiki = hcfg->SystemConfig_GlobalConfigurationWikiAddress;
    this->qConfig->Parameters = "prop=revisions&rvprop=content&rvlimit=1&titles=" + hcfg->SystemConfig_GlobalConfigYAML;
    this->qConfig->Process();
    return false;
}

void LoginForm::finishLogin(WikiSite *site)
{
    if (!this->LoginQueries.contains(site) || !this->LoginQueries[site]->IsProcessed())
        return;

    ApiQuery *query = this->LoginQueries[site];
    if (query->Result->IsFailed())
    {
        this->displayError(_l("login-fail-with-reason", site->Name, query->GetFailureReason()));
        this->Statuses[site] = LoginFailed;
        return;
    }

    ApiQueryResultNode *login_result = query->GetApiQueryResult()->GetNode("login");
    if (!login_result)
    {
        // Probably not necessary to localize this, as it's extremely rare
        this->displayError(_l("login-fail-with-reason", site->Name, "login result was missing in API response"));
        this->Statuses[site] = LoginFailed;
        return;
    }

    if (login_result->GetAttribute("result") != "Success")
    {
        QString reason = login_result->GetAttribute("reason");
        if (reason.isEmpty())
        {
            // ERROR: api.php responded with unknown result: $1
            this->displayError(_l("login-fail-with-reason", site->Name, _l("login-api", login_result->GetAttribute("result"))));
        } else
        {
            // Show why the login failed
            this->displayError(_l("login-fail-with-reason", site->Name, reason));
        }
        this->Statuses[site] = LoginFailed;
        return;
    }

    if (!login_result->Attributes.contains("lgusername"))
    {
        this->displayError(_l("login-fail-with-reason", site->Name, "API result did not contain actual login name"));
        this->Statuses[site] = LoginFailed;
        return;
    }

    QString actual_user_name = login_result->GetAttribute("lgusername");
    if (actual_user_name != hcfg->SystemConfig_UserName)
    {
        if (hcfg->SystemConfig_Multiple && hcfg->TemporaryConfig_UserNameWasChanged)
        {
            this->displayError(_l("login-fail-with-reason", site->Name, "multiple sites changed your username after login to different names. That is not supported. You can't use this username in multi-wiki mode"));
            this->Statuses[site] = LoginFailed;
            return;
        }
        // Remember that we already changed the username once, in case some other wiki changes it again (in case we are logging in to multiple sites) we would have a problem
        hcfg->TemporaryConfig_UserNameWasChanged = true;

        // Show a warning
        HUGGLE_WARNING("Actual username was changed by MediaWiki (on " + site->Name + ") from '" + hcfg->SystemConfig_UserName + "' to '" + actual_user_name + "', fixing up");
        hcfg->SystemConfig_UserName = actual_user_name;
    }

    // Assume login was successful
    if (this->processOutput(site))
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
        this->Statuses[site] = RetrievingProjectYAMLConfig;
    }
    // Let other huggle continue login process for other projects if there are some
    this->qCurrentLoginRequest = nullptr;
}

void LoginForm::retrieveWhitelist(WikiSite *site)
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
    if (this->wlQueries.contains(site))
    {
        WLQuery *query = this->wlQueries[site];
        if (query->IsProcessed())
        {
            this->wlQueries.remove(site);
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
    this->wlQueries.insert(site, query);
    query->RetryOnTimeoutFailure = false;
    query->Process();
}

void LoginForm::retrieveProjectYamlConfig(WikiSite *site)
{
    if (this->LoginQueries.contains(site))
    {
        ApiQuery *query = this->LoginQueries[site];
        if (query->IsProcessed())
        {
            if (query->IsFailed())
            {
                HUGGLE_DEBUG1("YAML for " + site->Name + " failed to load: " + query->GetFailureReason());
                this->fallbackToLegacyConfig(site);
                return;
            }
            ApiQueryResultNode *data = query->GetApiQueryResult()->GetNode("rev");
            if (data == nullptr)
            {
                HUGGLE_DEBUG1("YAML for " + site->Name + " is missing");
                this->fallbackToLegacyConfig(site);
                return;
            }
            QString value = data->Value;
            this->LoginQueries.remove(site);
            query->DecRef();
            if (site->ProjectConfig == nullptr)
                throw new Huggle::NullPointerException("site->ProjectConfig", BOOST_CURRENT_FUNCTION);
            QString reason;
            if (site->GetProjectConfig()->ParseYAML(value, &reason, site))
            {
                if (!site->GetProjectConfig()->EnableAll)
                {
                    this->displayError(_l("login-error-projdisabled", site->Name));
                    return;
                }
                this->loadingForm->ModifyIcon(this->GetRowIDForSite(site, LOGINFORM_YAMLCONFIG), LoadingForm_Icon_Success);
                this->loadingForm->ModifyIcon(this->GetRowIDForSite(site, LOGINFORM_LOCALCONFIG), LoadingForm_Icon_Success);
                this->Statuses[site] = RetrievingUserConfig;
                return;
            } else
            {
                this->displayError(_l("login-error-config", site->Name, reason));
                return;
            }
        }
        return;
    }
    this->loadingForm->ModifyIcon(this->GetRowIDForSite(site, LOGINFORM_YAMLCONFIG), LoadingForm_Icon_Loading);
    ApiQuery *query = new ApiQuery(ActionQuery, site);
    query->IncRef();
    if (!hcfg->GlobalConfig_OverrideConfigYAMLPath.isEmpty())
        hcfg->GlobalConfig_LocalConfigYAMLPath = hcfg->GlobalConfig_OverrideConfigYAMLPath;
    query->Parameters = "prop=revisions&rvprop=content&rvlimit=1&titles=" + hcfg->GlobalConfig_LocalConfigYAMLPath;
    query->Process();
    this->LoginQueries.insert(site, query);
}

void LoginForm::fallbackToLegacyConfig(WikiSite *site)
{
    ApiQuery *query = this->LoginQueries[site];
    query->DecRef();
    Syslog::HuggleLogs->WarningLog(site->Name + " - YAML configuration not found, falling back to deprecated config page");
    this->LoginQueries.remove(site);
    this->Statuses[site] = RetrievingProjectOldConfig;
    this->loadingForm->ModifyIcon(this->GetRowIDForSite(site, LOGINFORM_YAMLCONFIG), LoadingForm_Icon_Failed);
    this->retrieveProjectConfig(site);
}

void LoginForm::retrieveProjectConfig(WikiSite *site)
{
    if (this->LoginQueries.contains(site))
    {
        ApiQuery *query = this->LoginQueries[site];
        if (query->IsProcessed())
        {
            if (query->IsFailed())
            {
                this->displayError(_l("login-error-config", site->Name, query->GetFailureReason()));
                return;
            }
            ApiQueryResultNode *data = query->GetApiQueryResult()->GetNode("rev");
            if (data == nullptr)
            {
                this->displayError(_l("login-error-config", site->Name, _l("api-query-no-data")));
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
                    this->displayError(_l("login-error-projdisabled", site->Name));
                    return;
                }
                this->loadingForm->ModifyIcon(this->GetRowIDForSite(site, LOGINFORM_LOCALCONFIG), LoadingForm_Icon_Success);
                this->Statuses[site] = RetrievingUserConfig;
                return;
            } else
            {
                this->displayError(_l("login-error-config", site->Name, reason));
                return;
            }
        }
        return;
    }
    this->loadingForm->ModifyIcon(this->GetRowIDForSite(site, LOGINFORM_LOCALCONFIG), LoadingForm_Icon_Loading);
    ApiQuery *query = new ApiQuery(ActionQuery, site);
    query->IncRef();
    query->Parameters = "prop=revisions&rvprop=content&rvlimit=1&titles=" + hcfg->GlobalConfig_LocalConfigWikiPath;
    query->Process();
    this->LoginQueries.insert(site, query);
}

void LoginForm::retrieveUserConfig(WikiSite *site)
{
    if (this->LoginQueries.contains(site))
    {
        ApiQuery *q = this->LoginQueries[site];
        if (q->IsProcessed())
        {
            if (q->IsFailed())
            {
                this->displayError(_l("login-error-config-retrieve", q->GetFailureReason()));
                return;
            }
            ApiQueryResultNode *data = q->GetApiQueryResult()->GetNode("rev");
            if (data == nullptr) // page is missing
            {
                HUGGLE_DEBUG("User config is missing on " + site->Name, 2);
                if (this->usingOldUserConfig[site] == false && !hcfg->GlobalConfig_UserConf_old.isEmpty())
                {
                    // There is no user config page, let's try to fallback to old user config page
                    // try first with old location of config, we don't need to switch the login step here we just
                    // replace the old query with new query that retrieves the old config and call this function
                    // once more, trying to parse the old config
                    this->usingOldUserConfig[site] = true;
                    HUGGLE_DEBUG1("couldn't find user config for " + site->Name + " at new location, trying old one");
                    q->DecRef();
                    // let's get an old configuration instead
                    q = new ApiQuery(ActionQuery, site);
                    this->LoginQueries[site] = q;
                    q->IncRef();
                    QString page = hcfg->GlobalConfig_UserConf_old;
                    page = page.replace("$1", hcfg->SystemConfig_UserName);
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
                this->displayError(_l("login-fail-css", site->Name));
                return;
            }
            QString val_ = data->Value;
            this->LoginQueries.remove(site);
            q->DecRef();
            if (this->usingOldUserConfig[site])
            {
                if (!site->GetUserConfig()->Parse(val_, site->GetProjectConfig(), site == hcfg->Project))
                {
                    Syslog::HuggleLogs->DebugLog(val_);
                    this->displayError(_l("login-fail-parse-config", site->Name));
                    return;
                }
                Syslog::HuggleLogs->DebugLog("We successfully loaded and converted the old config for " + site->Name + " (huggle.css) :)");
            } else
            {
                QString error;
                if (!site->GetUserConfig()->ParseYAML(val_, site->GetProjectConfig(), site == hcfg->Project, &error))
                {
                    Syslog::HuggleLogs->DebugLog(val_);
                    this->displayError(_l("login-fail-parse-config-yaml", site->Name, error));
                    return;
                }
            }
            if (!site->ProjectConfig->EnableAll)
            {
                this->displayError(_l("login-fail-enable-true", site->Name));
                return;
            }
            hcfg->NormalizeConf(site);
            this->loadingForm->ModifyIcon(this->GetRowIDForSite(site, LOGINFORM_USERCONFIG), LoadingForm_Icon_Success);
            this->Statuses[site] = RetrievingUser;
            return;
        }
        return;
    }
    this->loadingForm->ModifyIcon(this->GetRowIDForSite(site, LOGINFORM_USERCONFIG), LoadingForm_Icon_Loading);
    ApiQuery *query = new ApiQuery(ActionQuery, site);
    query->IncRef();
    this->LoginQueries.insert(site, query);
    QString page = hcfg->GlobalConfig_UserConf;
    page = page.replace("$1", hcfg->SystemConfig_UserName);
    query->Parameters = "prop=revisions&rvprop=content&rvlimit=1&titles=" + QUrl::toPercentEncoding(page);
    query->Process();
}

void LoginForm::retrieveUserInfo(WikiSite *site)
{
    // check approval page
    if (this->qApproval.contains(site))
    {
        ApiQuery *query = this->qApproval[site];
        if (query->IsProcessed())
        {
            QString result;
            bool failed = false;
            result = WikiUtil::EvaluateWikiPageContents(query, &failed);
            if (failed)
            {
                this->displayError(_l("unable-to-retrieve-user-list", result));
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
            QString sanitized_name = hcfg->SystemConfig_UserName;
            sanitized_name = sanitized_name.toLower();
            sanitized_name = sanitized_name.replace("_", " ");
            if (!sanitized.contains("* [[special:contributions/" + sanitized_name + "|" + sanitized_name + "]]"))
            {
                if (site->GetProjectConfig()->Approval)
                {
                    this->displayError(_l("login-error-approval", site->Name));
                    return;
                }
                if (site->GetProjectConfig()->UserlistSync)
                {
                    // we need to insert this user into the list
                    QString un = WikiUtil::SanitizeUser(hcfg->SystemConfig_UserName);
                    QString line = "* [[Special:Contributions/" + un + "|" + un + "]]";
                    result += "\n" + line;
                    QString summary = site->GetProjectConfig()->UserlistUpdateSummary;
                    summary.replace("$1", hcfg->SystemConfig_UserName);
                    WikiUtil::EditPage(site, site->GetProjectConfig()->ApprovalPage, result, summary);
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
                this->displayError(_l("login-fail-no-info", site->Name, query->GetFailureReason()));
                return;
            }
            QList<ApiQueryResultNode*> nodes_r = query->GetApiQueryResult()->GetNodes("r");
            if (nodes_r.count() == 0)
            {
                HUGGLE_DEBUG1(query->Result->Data);
                // Login failed unable to retrieve user info since the api query returned no data
                this->displayError(_l("login-fail-user-data", site->Name));
                return;
            }
            foreach (ApiQueryResultNode *node, nodes_r)
            {
                site->GetProjectConfig()->Rights.append(node->Value);
            }
            if (site->GetProjectConfig()->RequireRollback && !site->GetProjectConfig()->Rights.contains("rollback"))
            {
                QString reason = _l("login-fail-rollback-rights", site->Name);
                if (!this->readonlyFallback(site, reason))
                {
                    this->displayError(reason);
                    return;
                }
            }
            if (site->GetProjectConfig()->RequireAutoconfirmed && (!site->GetProjectConfig()->Rights.contains("autoconfirmed") &&
                                                                   !site->GetProjectConfig()->Rights.contains("confirmed")))
            {
                QString reason = _l("login-failed-autoconfirm-rights", site->Name);
                if (!this->readonlyFallback(site, reason))
                {
                    this->displayError(reason);
                    return;
                }
            }
            // remove the query
            this->LoginQueries.remove(site);
            ApiQueryResultNode* ui = query->GetApiQueryResult()->GetNode("userinfo");
            if (!ui)
            {
                this->displayError(_l("login-no-userinfo"));
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
                    if (!this->readonlyFallback(site, reason))
                    {
                        this->displayError(reason);
                        return;
                    }
                }
                QDateTime registration_date = MediaWiki::FromMWTimestamp(registration_info);
                if (registration_date.addDays(site->GetProjectConfig()->RequireTime) > QDateTime::currentDateTime())
                {
                    QString reason = _l("login-error-age", QString::number(site->GetProjectConfig()->RequireTime));
                    if (!this->readonlyFallback(site, reason))
                    {
                        this->displayError(reason);
                        return;
                    }
                }
            }
            if (site->GetProjectConfig()->RequireEdits > editcount)
            {
                QString reason = _l("login-failed-edit", site->Name);
                if (!this->readonlyFallback(site, reason))
                {
                    this->displayError(reason);
                    return;
                }
            }

            // So now we passed all checks if we can use huggle, so let's update the user list in case we want that
            if (!site->GetProjectConfig()->ReadOnly && (site->GetProjectConfig()->UserlistSync || site->GetProjectConfig()->Approval))
            {
                this->qApproval.insert(site, WikiUtil::RetrieveWikiPageContents(site->GetProjectConfig()->ApprovalPage, site));
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

void LoginForm::developerMode()
{
    // load dummy project config files
    QFile *vf;
    vf = new QFile(":/huggle/resources/Resources/DefaultConfig.txt");
    vf->open(QIODevice::ReadOnly);
    QString pref = QString(vf->readAll());
    vf->close();
    delete vf;
    foreach (WikiSite *site, hcfg->Projects)
        site->GetProjectConfig()->ParseYAML(pref, nullptr, site);
    hcfg->DeveloperMode = true;
    MainWindow *main = new MainWindow();
    main->show();
    this->hide();
}

void LoginForm::processSiteInfo(WikiSite *site)
{
    if (this->qTokenInfo.contains(site) && this->qSiteInfo.contains(site)
            && this->qTokenInfo[site]->IsProcessed() && this->qSiteInfo[site]->IsProcessed())
    {
        if (this->qSiteInfo[site]->IsFailed())
        {
            this->displayError(_l("login-site-info-query-failed", site->Name, this->qSiteInfo[site]->GetFailureReason()));
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
            this->displayError(_l("login-no-site-info-returned"));
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

void LoginForm::displayError(QString message)
{
    this->CancelLogin();
    Syslog::HuggleLogs->ErrorLog(message);
    this->Update(QString("<span style=\"color: red;\">%1</span>").arg(message));
    QMessageBox::critical(this->window(), _l("error"), message);
}

void LoginForm::finishLogin()
{
    // let's check if all processes are finished
    foreach (WikiSite *xx, hcfg->Projects)
    {
        if (!this->processedWL[xx]) return;
        if (!this->processedLogin[xx]) return;
        if (this->Statuses[xx] != LoginDone) return;
        if (!this->processedSiteinfos[xx]) return;
    }
    if (!this->globalConfigIsLoaded)
        return;
    QString pw = "";
    this->loginFinished = true;
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
    MainWindow *main = new MainWindow();
    main->show();
    if (this->updateForm)
    {
        this->updateForm->close();
        delete this->updateForm;
        this->updateForm = nullptr;
    }
    if (this->loadingForm != nullptr)
    {
        this->loadingForm->close();
        delete this->loadingForm;
        this->loadingForm = nullptr;
        LoadingForm::IsKilled = true;
    }
}

void LoginForm::reject()
{
    if (this->loginFinished == false)
    {
        // Destroy the updater form so that it doesn't crash the application in case we close login form
        delete this->updateForm;
        this->updateForm = nullptr;
        Core::HuggleCore->Shutdown();
    }
    else
    {
        QDialog::reject();
    }
}

bool LoginForm::processOutput(WikiSite *site)
{
    ApiQuery *query = this->LoginQueries[site];
    // Check what the result was
    ApiQueryResult *result = query->GetApiQueryResult();
    ApiQueryResultNode *ln = result->GetNode("login");
    QString result_code = ln->GetAttribute("result");
    QString reason = ln->GetAttribute("reason");
    if (result_code.isEmpty())
    {
        this->displayError(_l("api.php-invalid-response"));
        return false;
    }
    if (result_code == "Success")
        return true;
    if (result_code == "EmptyPass")
    {
        this->displayError(_l("login-password-empty"));
        return false;
    }
    if (result_code == "WrongPass")
    {
        /// \bug This sometimes doesn't work properly
        this->ui->lineEdit_password->setFocus();
        this->displayError(_l("login-error-password"));
        return false;
    }
    if (result_code == "NoName")
    {
        this->displayError(_l("login-fail-wrong-name"));
        return false;
    }
    if (result_code == "NotExists")
    {
        this->displayError(_l("login-username-doesnt-exist"));
        return false;
    }
    if (reason.isEmpty())
        reason = result_code;
    this->displayError(_l("login-api", reason));
    return false;
}

void LoginForm::on_ButtonOK_clicked()
{
    if (!this->loginInProgress)
    {
        this->loginInProgress = true;
        this->pressOK();
        return;
    }
    else
    {
        this->CancelLogin();
        this->resetForm();
        return;
    }
}

void LoginForm::on_ButtonExit_clicked()
{
    Core::HuggleCore->Shutdown();
}

void LoginForm::OnTimerTick()
{
    if (this->Refreshing)
    {
        this->reloadWikiDB();
        return;
    }
    if (!this->loginInProgress)
        return;
    if (!this->globalConfigIsLoaded && !this->retrieveGlobalConfig())
        return;
    // let's check status for every single project
    foreach (WikiSite *site, hcfg->Projects)
    {
        if (!this->Statuses.contains(site))
            throw new Huggle::Exception("There is no such a wiki in statuses list", BOOST_CURRENT_FUNCTION);
        if (!this->globalConfigIsLoaded)
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
                case RetrievingProjectYAMLConfig:
                case RetrievingProjectOldConfig:
                case RetrievingUserConfig:
                case RetrievingUser:
                    continue;
            }
        }
        // Check if there isn't any ongoing request to login to other project, per https://phabricator.wikimedia.org/T195109
        // in case there is some we need to wait because MW doesn't support simultaneous logins to multiple projects
        if (!hcfg->SystemConfig_ParallelLogin && this->qCurrentLoginRequest != nullptr && this->qCurrentLoginRequest->GetSite() != site)
                continue;

        switch(this->Statuses[site])
        {
            case LoggingIn:
                this->performLogin(site);
                break;
            case WaitingForLoginQuery:
                this->performLoginPart2(site);
                break;
            case WaitingForToken:
                this->finishLogin(site);
                break;
            case RetrievingProjectOldConfig:
                this->retrieveProjectConfig(site);
                break;
            case RetrievingProjectYAMLConfig:
                this->retrieveProjectYamlConfig(site);
                break;
            case RetrievingUserConfig:
                this->retrieveUserConfig(site);
                break;
            case RetrievingUser:
                retrieveUserInfo(site);
                break;
            case LoggedIn:
            case Nothing:
            case Cancelling:
            case LoginFailed:
            case LoginDone:
                break;
        }
        if (this->globalConfigIsLoaded)
        {
            if (!this->processedWL[site])
                this->retrieveWhitelist(site);
            if (this->qSiteInfo.contains(site))
                this->processSiteInfo(site);
        }
        if (this->Statuses[site] == LoginFailed)
        {
            this->enableForm();
            this->timer->stop();
            this->ui->ButtonOK->setText(_l("login-start"));
        }
    }
    this->finishLogin();
}

void LoginForm::on_pushButton_clicked()
{
    this->disableForm();
    this->qDatabase = new ApiQuery(ActionQuery, hcfg->GlobalWiki);
    this->Refreshing = true;
    hcfg->SystemConfig_UsingSSL = this->ui->checkBox->isChecked();
    this->timer->start(HUGGLE_TIMER);
    this->qDatabase->OverrideWiki = hcfg->SystemConfig_GlobalConfigurationWikiAddress;
    this->ui->ButtonOK->setText(_l("[[cancel]]"));
    this->qDatabase->Parameters = "prop=revisions&rvprop=content&rvlimit=1&titles=" + hcfg->SystemConfig_GlobalConfigWikiList;
    this->qDatabase->Process();
}

void LoginForm::on_Language_currentIndexChanged(const QString &arg1)
{
    if (this->isLoading)  return;
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

void Huggle::LoginForm::on_labelTranslate_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(link);
}

void Huggle::LoginForm::on_lineEdit_username_textChanged(const QString &arg1)
{
    Q_UNUSED( arg1 )
    LoginForm::verifyLogin();
}

void Huggle::LoginForm::on_lineEdit_password_textChanged(const QString &arg1)
{
    Q_UNUSED( arg1 )
    LoginForm::verifyLogin();
}

void LoginForm::verifyLogin()
{
    if((((this->ui->lineEditBotUser->text().size() == 0 && this->ui->lineEdit_username->text().size() == 0) ||
        (this->ui->lineEdit_password->text().size() == 0 && this->ui->lineEditBotP->text().size() == 0))) &&
        !this->isDeveloperMode())
        this->ui->ButtonOK->setEnabled(false);
    else
        this->ui->ButtonOK->setEnabled(true);
}

int LoginForm::registerLoadingFormRow(WikiSite *site, int row)
{
    this->loadingFormRows[site].insert(row, this->loadingForm_LastRow);
    return this->loadingForm_LastRow++;
}

void LoginForm::clearLoadingFormRows()
{
    QList<WikiSite*> Sites = this->loadingFormRows.keys();
    this->loadingForm_LastRow = 0;
    foreach (WikiSite *Site, Sites)
        this->loadingFormRows[Site].clear();
    this->loadingFormRows.clear();
}

void Huggle::LoginForm::on_pushButton_2_clicked()
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
        if (this->height() <= 480)
            this->resize(this->width(), 520);
    }
}

void Huggle::LoginForm::on_label_linkActivated(const QString &link)
{
    Q_UNUSED(link);
    Proxy pr;
    pr.exec();
}

void Huggle::LoginForm::on_lineEditBotUser_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    LoginForm::verifyLogin();
}

void Huggle::LoginForm::on_lineEditBotP_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    LoginForm::verifyLogin();
}

void Huggle::LoginForm::on_label_2_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(link);
}

void Huggle::LoginForm::on_tabWidget_currentChanged(int index)
{
    if (index == 0)
    {
        if (this->ui->lineEditBotUser->text().isEmpty() && !this->ui->lineEdit_username->text().isEmpty())
            this->ui->lineEditBotUser->setText(this->ui->lineEdit_username->text() + "@huggle");
    }
}

bool LoginForm::isDeveloperMode()
{
    return this->ui->lineEdit_username->text().replace('_', ' ').toLower() == "developer mode";
}
