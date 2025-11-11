//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "mainwindow.hpp"
#include "blockuserform.hpp"
#include "custommessage.hpp"
#include "deleteform.hpp"
#include "editbar.hpp"
#include "editform.hpp"
#include "history.hpp"
#include "hugglelog.hpp"
#include "huggletool.hpp"
#include "hugglequeue.hpp"
#include "hugglequeueitemlabel.hpp"
#include "ignorelist.hpp"
#include "speedyform.hpp"
#include "userinfoform.hpp"
#include "vandalnw.hpp"
#include "whitelistform.hpp"
#include "sessionform.hpp"
#include "scripting/scriptingmanager.hpp"
#include "historyform.hpp"
#include "scorewordsdbform.hpp"
#include "overlaybox.hpp"
#include "preferences.hpp"
#include "processlist.hpp"
#include "protectpage.hpp"
#include "reloginform.hpp"
#include "reportuser.hpp"
#include "welcomeinfo.hpp"
#include "warninglist.hpp"
#include "waitingform.hpp"
#include "wikipagetagsform.hpp"
#include "uaareport.hpp"
#include "ui_mainwindow.h"
#include "uihooks.hpp"
#include "uigeneric.hpp"
#include "requestprotect.hpp"
#include "queuehelp.hpp"
#include <huggle_core/apiquery.hpp>
#include <huggle_core/apiqueryresult.hpp>
#include <huggle_core/events.hpp>
#include <huggle_core/configuration.hpp>
#include <huggle_core/generic.hpp>
#include <huggle_core/gc.hpp>
#include <huggle_core/querypool.hpp>
#include <huggle_core/hooks.hpp>
#include <huggle_core/hugglefeedproviderwiki.hpp>
#include <huggle_core/hugglefeedproviderirc.hpp>
#include <huggle_core/hugglefeedproviderxml.hpp>
#include <huggle_core/huggleparser.hpp>
#include <huggle_core/huggleprofiler.hpp>

#ifdef HUGGLE_WEBEN
    #include "web_engine/huggleweb.hpp"
#else
    #include "webkit/huggleweb.hpp"
#endif

#include <huggle_core/wikipage.hpp>
#include <huggle_core/resources.hpp>
#include <huggle_core/collectable.hpp>
#include <huggle_core/core.hpp>
#include <huggle_core/wikiutil.hpp>
#include <huggle_core/exception.hpp>
#include <huggle_core/localization.hpp>
#include <huggle_core/syslog.hpp>
#include <huggle_core/sleeper.hpp>
#include <huggle_core/wikiuser.hpp>
#include <huggle_core/wikisite.hpp>
#include <huggle_core/warnings.hpp>
#include <QMessageBox>
#include <QDesktopServices>
#include <QClipboard>
#include <QToolButton>
#include <QInputDialog>
#include <QMutex>
#include <QMenu>
#include <QLabel>
#include <QToolTip>
#include <QTimer>
#include <QCloseEvent>
#include <QThread>
#include <QVBoxLayout>
#include <QSplitter>
#include <QDockWidget>
#include "aboutform.hpp"
#ifdef DeleteForm
    #undef DeleteForm
#endif

using namespace Huggle;
MainWindow *MainWindow::HuggleMain = nullptr;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    if (MainWindow::HuggleMain)
        throw new Exception("Main window already exist", BOOST_CURRENT_FUNCTION);
    MainWindow::HuggleMain = this;
    HUGGLE_PROFILER_RESET;
    this->lastKeyStrokeCheck = QDateTime::currentDateTime();
    this->LastTPRevID = WIKI_UNKNOWN_REVID;
    this->editLoadDateTime = QDateTime::currentDateTime();
    this->Shutdown = ShutdownOpRunning;
    this->ShuttingDown = false;
    this->ui->setupUi(this);
#ifndef HUGGLE_PROFILING
    this->ui->actionProfiler_info->setVisible(false);
#endif
    if (Configuration::HuggleConfiguration->SystemConfig_Multiple)
    {
        this->ui->menuChange_provider->setVisible(false);
        this->ui->actionStop_provider->setVisible(false);
        this->ui->actionIRC->setVisible(false);
        this->ui->actionWiki->setVisible(false);
        this->ui->actionXmlRcs->setVisible(false);
    }
    this->Status = new QLabel();
    this->Status->setTextInteractionFlags(Qt::TextSelectableByMouse);
    this->Status->setWordWrap(true);
    this->Status->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    this->ui->statusBar->addWidget(this->Status, 1);
    this->tb = new HuggleTool();
    this->Queries = new ProcessList(this);
    this->SystemLog = new HuggleLog(this);
    this->createBrowserTab(_l("main-tab-welcome-title"), 0);
    this->TrayIcon.setIcon(this->windowIcon());
    this->TrayIcon.show();
    this->TrayIcon.setToolTip("Huggle");
    this->Queue1 = new HuggleQueue(this);
    this->wEditBar = new EditBar(this);
    this->_History = new History(this);
    this->wHistory = new HistoryForm(this);
    this->wUserInfo = new UserinfoForm(this);
    this->VandalDock = new VandalNw(this);
    this->addDockWidget(Qt::LeftDockWidgetArea, this->Queue1);
    this->addDockWidget(Qt::BottomDockWidgetArea, this->SystemLog);
    this->addDockWidget(Qt::TopDockWidgetArea, this->tb);
    this->addDockWidget(Qt::BottomDockWidgetArea, this->Queries);
    this->addDockWidget(Qt::RightDockWidgetArea, this->wHistory);
    this->addDockWidget(Qt::RightDockWidgetArea, this->wUserInfo);
    this->addDockWidget(Qt::BottomDockWidgetArea, this->VandalDock);
    this->addDockWidget(Qt::TopDockWidgetArea, this->wEditBar);
    this->wEditBar->hide();
    this->aboutForm = nullptr;
    this->ui->actionDisplay_bot_data->setChecked(hcfg->UserConfig->HAN_DisplayBots);
    this->ui->actionDisplay_user_data->setChecked(hcfg->UserConfig->HAN_DisplayUser);
    this->ui->actionDisplay_user_messages->setChecked(hcfg->UserConfig->HAN_DisplayUserTalk);
    // we store the value in bool so that we don't need to call expensive string function twice
    bool PermissionBlock = hcfg->ProjectConfig->Rights.contains("block");
    this->ui->actionBlock_user->setEnabled(PermissionBlock);
    this->ui->actionBlock_user_2->setEnabled(PermissionBlock);
    bool PermissionDelete = hcfg->ProjectConfig->Rights.contains("delete");
    this->ui->actionDelete->setEnabled(PermissionDelete);
    this->ui->actionProtect->setEnabled(hcfg->ProjectConfig->Rights.contains("protect"));
    this->addDockWidget(Qt::LeftDockWidgetArea, this->_History);
    this->SystemLog->resize(100, 80);
    foreach (WikiSite *site, hcfg->Projects)
    {
        if (!site->GetProjectConfig()->WhiteList.contains(hcfg->SystemConfig_UserName))
            site->GetProjectConfig()->WhiteList.append(hcfg->SystemConfig_UserName);
    }
    QString projects;
    if (hcfg->SystemConfig_Multiple)
    {
        foreach (WikiSite *site, hcfg->Projects)
            projects += site->Name + ", ";
        projects = projects.mid(0, projects.length() - 2);
    } else
    {
        // Set the name of current site
        projects = hcfg->Project->Name;
    }
    if (!hcfg->SystemConfig_Multiple)
        this->setWindowTitle("Huggle 3 " + _l("title-on", projects));
    else
        this->setWindowTitle("Huggle 3 " + _l("title-on", _l("title-multiple-projects", projects)));
    HUGGLE_PROFILER_PRINT_TIME("MainWindow::MainWindow(QWidget *parent)@layout");
    this->DisplayWelcomeMessage();
    HUGGLE_PROFILER_PRINT_TIME("MainWindow::MainWindow(QWidget *parent)@welcome");
    if (Configuration::HuggleConfiguration->UserConfig->RemoveOldQueueEdits)
    {
        this->ui->actionRemove_old_edits->setChecked(true);
        this->ui->actionStop_feed->setChecked(false);
    }
    else
    {
        this->ui->actionRemove_old_edits->setChecked(false);
        this->ui->actionStop_feed->setChecked(true);
    }
    // configure RC feed provider
    foreach (WikiSite *site, Configuration::HuggleConfiguration->Projects)
    {
        if (Configuration::HuggleConfiguration->SystemConfig_Multiple)
        {
            QMenu *menu = new QMenu(site->Name, this);
            this->ui->menuChange_provider->addMenu(menu);
            QAction *provider_irc = new QAction("IRC", menu);
            QAction *provider_wiki = new QAction("Wiki", menu);
            QAction *provider_xml = new QAction("XmlRcs", menu);
            this->lXml.insert(site, provider_xml);
            this->lWikis.insert(site, provider_wiki);
            this->lIRC.insert(site, provider_irc);
            provider_xml->setCheckable(true);
            provider_wiki->setCheckable(true);
            provider_irc->setCheckable(true);
            connect(provider_xml, SIGNAL(triggered()), this, SLOT(SetProviderXml()));
            connect(provider_wiki, SIGNAL(triggered()), this, SLOT(SetProviderWiki()));
            connect(provider_irc, SIGNAL(triggered()), this, SLOT(SetProviderIRC()));
            menu->addAction(provider_xml);
            menu->addAction(provider_irc);
            menu->addAction(provider_wiki);
            this->ActionSites.insert(provider_xml, site);
            this->ActionSites.insert(provider_irc, site);
            this->ActionSites.insert(provider_wiki, site);
        }
        // Create basic providers
        this->huggleFeeds.append(new HuggleFeedProviderWiki(site));
        this->huggleFeeds.append(new HuggleFeedProviderIRC(site));
        this->huggleFeeds.append(new HuggleFeedProviderXml(site));
        Hooks::FeedProvidersOnInit(site);
        this->ChangeProvider(site, hcfg->UserConfig->PreferredProvider);
    }
    if (hcfg->DeveloperMode)
    {
        this->ui->actionResume_provider->setVisible(true);
        this->ui->actionStop_provider->setVisible(false);
        this->PauseQueue();
    }
    HUGGLE_PROFILER_PRINT_TIME("MainWindow::MainWindow(QWidget *parent)@providers");
    this->ReloadInterface();
    this->tabifyDockWidget(this->SystemLog, this->Queries);
    this->generalTimer = new QTimer(this);
    //this->ui->actionTag_2->setVisible(false);
    connect(this->generalTimer, SIGNAL(timeout()), this, SLOT(OnMainTimerTick()));
    this->generalTimer->start(HUGGLE_TIMER);
    QFile *layout;
    if (QFile::exists(Configuration::GetConfigurationPath() + "mainwindow_state"))
    {
        HUGGLE_DEBUG1("Loading state");
        layout = new QFile(Configuration::GetConfigurationPath() + "mainwindow_state");
        if (!layout->open(QIODevice::ReadOnly))
            Syslog::HuggleLogs->ErrorLog(_l("main-config-state-fail"));
        else if (!this->restoreState(layout->readAll()))
            HUGGLE_DEBUG1("Failed to restore state");
        layout->close();
        delete layout;
    }
    if (QFile::exists(Configuration::GetConfigurationPath() + "mainwindow_geometry"))
    {
        HUGGLE_DEBUG1("Loading geometry");
        layout = new QFile(Configuration::GetConfigurationPath() + "mainwindow_geometry");
        if (!layout->open(QIODevice::ReadOnly))
            Syslog::HuggleLogs->ErrorLog(_l("main-config-geom-fail"));
        else if (!this->restoreGeometry(layout->readAll()))
            HUGGLE_DEBUG1("Failed to restore layout");
        layout->close();
        delete layout;
    }
    this->showMaximized();
    HUGGLE_PROFILER_PRINT_TIME("MainWindow::MainWindow(QWidget *parent)@layout");
    // these controls are for debugging only
    if (Configuration::HuggleConfiguration->Verbosity == 0)
    {
        QAction *debugm = this->ui->menuDebug_2->menuAction();
        this->ui->menuHelp->removeAction(debugm);
    }
    if (hcfg->ProjectConfig->Goto.count() > 0)
    {
        this->ui->menuGo_to->addSeparator();
        QList<QAction*> list;
        foreach (QString item, hcfg->ProjectConfig->Goto)
        {
            if (!item.contains(";"))
            {
                Syslog::HuggleLogs->WarningLog("Invalid item for go menu: " + item);
                continue;
            }
            QString url = item.mid(0, item.indexOf(";"));
            QString name = item.mid(item.indexOf(";") + 1);
            if (name.endsWith(","))
                name = name.mid(0, name.length() - 1);
            QAction *action = new QAction(name, this);
            connect(action, SIGNAL(triggered()), this, SLOT(Go()));
            action->setToolTip(url);
            list.append(action);
        }
        this->ui->menuGo_to->addActions(list);
    }
    HuggleQueueFilter::SetFilters();
    this->Localize();
    this->VandalDock->Connect();
    HUGGLE_PROFILER_PRINT_TIME("MainWindow::MainWindow(QWidget *parent)@irc");
    this->tCheck = new QTimer(this);
    HUGGLE_PROFILER_PRINT_TIME("MainWindow::MainWindow(QWidget *parent)@hooks");
    connect(this->tCheck, SIGNAL(timeout()), this, SLOT(TimerCheckTPOnTick()));
    this->tStatusBarRefreshTimer = new QTimer(this);
    connect(this->tStatusBarRefreshTimer, SIGNAL(timeout()), this, SLOT(OnStatusBarRefreshTimerTick()));
    this->tCheck->start(20000);
    this->tStatusBarRefreshTimer->start(500);
    if (!Events::Global)
        throw new Exception("Global events aren't instantiated now", BOOST_CURRENT_FUNCTION);
    connect(Events::Global, SIGNAL(QueryPool_FinishPreprocess(WikiEdit*)), this, SLOT(OnFinishPreProcess(WikiEdit*)));
    connect(Events::Global, SIGNAL(WikiEdit_OnNewHistoryItem(HistoryItem*)), this, SLOT(OnWikiEditHist(HistoryItem*)));
    connect(Events::Global, SIGNAL(WikiUser_Updated(WikiUser*)), this, SLOT(OnWikiUserUpdate(WikiUser*)));
    connect(Events::Global, SIGNAL(System_ErrorMessage(QString,QString)), this, SLOT(OnError(QString,QString)));
    connect(Events::Global, SIGNAL(System_Message(QString,QString)), this, SLOT(OnMessage(QString,QString)));
    connect(Events::Global, SIGNAL(System_WarningMessage(QString,QString)), this, SLOT(OnWarning(QString,QString)));
    connect(Events::Global, SIGNAL(System_YesNoQuestion(QString,QString,bool*)), this, SLOT(OnQuestion(QString,QString,bool*)));
    this->EnableEditing(false);
    UiHooks::MainWindow_OnLoad(this);
}

MainWindow::~MainWindow()
{
    while (this->Historical.count())
    {
        this->Historical.at(0)->UnregisterConsumer(HUGGLECONSUMER_MAINFORM_HISTORICAL);
        this->Historical.removeAt(0);
    }
    while (this->RevertStack.count())
    {
        this->RevertStack.at(0)->DecRef();
        this->RevertStack.removeAt(0);
    }
    while (this->Browsers.count())
    {
        delete this->Browsers.at(0);
        this->Browsers.removeAt(0);
    }
    this->tStatusBarRefreshTimer->stop();
    delete this->fWikiPageTags;
    delete this->OnNext_EvPage;
    delete this->fSpeedyDelete;
    delete this->wUserInfo;
    delete this->wHistory;
    delete this->wlt;
    delete this->fWaiting;
    delete this->VandalDock;
    delete this->_History;
    delete this->RevertWarn;
    delete this->tCheck;
    delete this->fRelogin;
    delete this->WarnMenu;
    delete this->fProtectForm;
    delete this->wEditBar;
    delete this->tStatusBarRefreshTimer;
    delete this->RevertSummaries;
    delete this->Queries;
    delete this->aboutForm;
    delete this->fSessionData;
    delete this->fScoreWord;
    delete this->fWhitelist;
    delete this->Ignore;
    delete this->Queue1;
    delete this->SystemLog;
    delete this->Status;
    delete this->fBlockForm;
    delete this->fWarningList;
    delete this->fDeleteForm;
    delete this->fUaaReportForm;
    delete this->fScripting;
    delete this->ui;
    delete this->tb;
    if (hcfg)
    {
        foreach (WikiSite *site, hcfg->Projects)
            site->Provider = nullptr;
    }
    qDeleteAll(this->huggleFeeds);
}

void MainWindow::DisplayReportUserWindow(WikiUser *User)
{
    if (!this->CheckExit() || this->CurrentEdit == nullptr)
        return;

    if (Configuration::HuggleConfiguration->DeveloperMode)
    {
        Generic::DeveloperError();
        return;
    }
    if (User == nullptr)
        User = this->CurrentEdit->User;
    if (User == nullptr)
        throw new Huggle::NullPointerException("local WikiUser *User", BOOST_CURRENT_FUNCTION);
    if (User->IsReported)
    {
        Syslog::HuggleLogs->ErrorLog(_l("report-duplicate"));
        return;
    }
    ProjectConfiguration *conf = this->GetCurrentWikiSite()->GetProjectConfig();
    // only use this if current projects support it
    if (!conf->AIV)
    {
        UiGeneric::pMessageBox(this, _l("missing-aiv"), _l("function-miss"));
        return;
    }

    ReportUser *rf = new ReportUser(this);
    rf->SetUser(User);
    rf->setAttribute(Qt::WA_DeleteOnClose);
    rf->show();
}

void MainWindow::EnableEditing(bool enabled)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    // In case we are in dry mode allow everything, for debugging ;)
    if (hcfg->SystemConfig_DryMode)
        enabled = true;

    // This is just so that we can track it later
    this->EditablePage = enabled;

    this->ui->actionWarn->setEnabled(enabled);
    this->ui->actionRevert->setEnabled(enabled);
    this->ui->actionRevert_AGF->setEnabled(enabled);
    this->ui->actionRevert_and_warn->setEnabled(enabled);
    this->ui->actionRevert_currently_displayed_edit->setEnabled(enabled);
    this->ui->actionRevert_currently_displayed_edit_and_warn_the_user->setEnabled(enabled);
    this->ui->actionWelcome_user->setEnabled(enabled);
    this->ui->actionRevert_currently_displayed_edit_and_stay_on_page->setEnabled(enabled);
    this->ui->actionRevert_currently_displayed_edit_warn_user_and_stay_on_page->setEnabled(enabled);
    this->ui->actionRevert_only_this_revision->setEnabled(enabled);
    this->ui->actionRevert_only_this_revision_assuming_good_faith->setEnabled(enabled);
    this->ui->actionTag_2->setEnabled(enabled);
    this->ui->actionWarn_the_user->setEnabled(enabled);
    this->ui->actionPatrol->setEnabled(enabled);
    this->ui->actionDelete_page->setEnabled(enabled);
    this->ui->actionRevert_edit_using_custom_reason->setEnabled(enabled);
    this->ui->actionReport_user_2->setEnabled(enabled);
    this->ui->actionRequest_speedy_deletion->setEnabled(enabled);
    this->ui->actionReport_user->setEnabled(enabled);

    if (!enabled)
    {
        this->ui->actionDelete->setEnabled(false);
    } else if (this->CurrentEdit != nullptr)
    {
        this->ui->actionDelete->setEnabled(this->GetCurrentWikiSite()->GetProjectConfig()->Rights.contains("delete"));
    }
}

WikiEdit *MainWindow::GetCurrentWikiEdit()
{
    return this->CurrentEdit.GetPtr();
}

QMenu *MainWindow::GetMenu(int menu_id)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    switch (menu_id)
    {
        case HUGGLE_MW_MENU_SYSTEM:
            return this->ui->menuFile;
        case HUGGLE_MW_MENU_USER:
            return this->ui->menuUser;
        case HUGGLE_MW_MENU_PAGE:
            return this->ui->menuPage;
        case HUGGLE_MW_MENU_GOTO:
            return this->ui->menuGo_to;
        case HUGGLE_MW_MENU_QUEUE:
            return this->ui->menuQueue;
        case HUGGLE_MW_MENU_TOOLS:
            return this->ui->menuTools;
        case HUGGLE_MW_MENU_HAN:
            return this->ui->menuHAN;
        case HUGGLE_MW_MENU_SCRIPTING:
            return this->ui->menuScripting;
        case HUGGLE_MW_MENU_DEBUG:
            return this->ui->menuDebug_2;
        case HUGGLE_MW_MENU_HELP:
            return this->ui->menuHelp;
        default:
            return nullptr;
    }
}

QAction *MainWindow::GetMenuItem(int menu_item)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    switch (menu_item)
    {
        case HUGGLE_MW_MENUITEM_EXIT:
            return this->ui->actionExit;
        default:
            return nullptr;
    }
}

void MainWindow::DisplayEdit(WikiEdit *edit, bool ignore_history, bool keep_history, bool keep_user, bool forced_jump)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    if (edit == nullptr || this->ShuttingDown)
    {
        // Huggle is either shutting down or edit is nullptr so we can't do anything here
        return;
    }
    if (this->qNext != nullptr)
    {
        // we need to delete this because it's related to an old edit
        this->qNext.Delete();
    }
    if (edit->Page == nullptr || edit->User == nullptr)
    {
        throw new Huggle::NullPointerException("WikiEdit *e->Page || WikiEdit *e->User", BOOST_CURRENT_FUNCTION);
    }
    if (this->OnNext_EvPage != nullptr)
    {
        delete this->OnNext_EvPage;
        this->OnNext_EvPage = nullptr;
    }
    // we need to safely delete the edit later
    edit->IncRef();
    // if there are actually some totally old edits in history that we need to delete
    while (this->Historical.count() > Configuration::HuggleConfiguration->SystemConfig_HistorySize)
    {
        WikiEdit *prev = this->Historical.at(0);
        if (prev == edit)
            break;

        this->Historical.removeAt(0);
        prev->RemoveFromHistoryChain();
        prev->UnregisterConsumer(HUGGLECONSUMER_MAINFORM_HISTORICAL);
    }
    if (!this->Historical.contains(edit))
    {
        edit->RegisterConsumer(HUGGLECONSUMER_MAINFORM_HISTORICAL);
        this->Historical.append(edit);
        if (this->CurrentEdit != nullptr)
        {
            if (!ignore_history)
            {
                edit->RemoveFromHistoryChain();
                // now we need to get to last edit in chain
                WikiEdit *latest = this->CurrentEdit;
                while (latest->Next != nullptr)
                    latest = latest->Next;
                latest->Next = edit;
                edit->Previous = latest;
            }
        }
    }
    this->Queue1->ChangeSite(edit->GetSite());
    edit->User->Resync();
    Configuration::HuggleConfiguration->ForceNoEditJump = forced_jump;
    this->CurrentEdit = edit;
    this->editLoadDateTime = QDateTime::currentDateTime();
    this->Browser->DisplayDiff(edit);
    this->Render(keep_history, keep_user);
    edit->DecRef();
}

void MainWindow::Render(bool keep_history, bool keep_user)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    if (this->CurrentEdit != nullptr)
    {
        if (this->CurrentEdit->Page == nullptr)
            throw new Huggle::NullPointerException("local CurrentEdit->Page", BOOST_CURRENT_FUNCTION);

        this->ui->actionFinal->setVisible(this->GetCurrentWikiSite()->GetProjectConfig()->InstantWarnings);
        this->wEditBar->RemoveAll();
        if (!keep_user)
        {
            this->wUserInfo->ChangeUser(this->CurrentEdit->User);
            if (Configuration::HuggleConfiguration->UserConfig->HistoryLoad)
                this->wUserInfo->Read();
        }
        if (!keep_history)
        {
            this->wHistory->Update(this->CurrentEdit);
            if (Configuration::HuggleConfiguration->UserConfig->HistoryLoad)
                this->wHistory->Read();
        }
        this->changeCurrentBrowserTabTitle(this->CurrentEdit->Page->PageName);
        if (this->previousSite != this->GetCurrentWikiSite())
        {
            this->ReloadInterface();
            this->previousSite = this->GetCurrentWikiSite();
        }
        this->tb->SetTitle(this->CurrentEdit->Page->PageName);
        this->tb->SetPage(this->CurrentEdit->Page);
        this->tb->SetUser(this->CurrentEdit->User->Username);
        QString word_list = "";
        if (this->CurrentEdit->ScoreWords.count() != 0)
        {
            word_list = " words: ";
            foreach (QString word, this->CurrentEdit->ScoreWords)
            {
                word_list += word + ", ";
            }
            if (word_list.endsWith(", "))
                word_list = word_list.mid(0, word_list.length() - 2);
        }
        QStringList params;
        params << this->CurrentEdit->Page->PageName << QString::number(this->CurrentEdit->Score) + word_list;
        this->tb->SetInfo(_l("browser-diff", params));
        this->EnableEditing(!this->CurrentEdit->GetSite()->GetProjectConfig()->ReadOnly);
        UiHooks::MainWindow_OnRender();
        return;
    }
    this->EnableEditing(false);
    this->tb->SetTitle(this->Browser->CurrentPageName());
    UiHooks::MainWindow_OnRender();
}

void MainWindow::RequestPD(WikiEdit *edit)
{
    if (!this->CheckExit() || !this->CheckEditableBrowserPage() || this->CurrentEdit == nullptr)
        return;
    if (Configuration::HuggleConfiguration->DeveloperMode)
    {
        Generic::DeveloperError();
        return;
    }
    if (edit == nullptr)
        edit = this->CurrentEdit;
    delete this->fSpeedyDelete;
    this->fSpeedyDelete = new SpeedyForm(this);
    this->fSpeedyDelete->Init(edit);
    this->fSpeedyDelete->show();
}

void MainWindow::TrayMessage(const QString& title, const QString& text)
{
    this->TrayIcon.showMessage(title, text);
}

void MainWindow::RevertAgf(bool only)
{
    if (this->CurrentEdit == nullptr || !this->CheckExit() || !this->CheckEditableBrowserPage() || !this->CheckRevertable())
        return;
    if (Configuration::HuggleConfiguration->DeveloperMode)
    {
        Generic::DeveloperError();
        return;
    }
    bool ok;
    QString reason = QInputDialog::getText(this, _l("reason"), _l("main-revert-custom-reson"), QLineEdit::Normal,
                                           _l("main-revert-custom-reason-text"), &ok);
    if (!ok)
        return;
    QString summary = this->GetCurrentWikiSite()->GetProjectConfig()->AgfRevert;
    summary.replace("$2", this->CurrentEdit->User->Username);
    summary.replace("$1", reason);
    this->Revert(summary, true, only);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (this->ShuttingDown)
    {
        event->ignore();
        return;
    }
    this->Exit();
    event->ignore();
}

void MainWindow::UpdateStatusBarData()
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    QStringList params;
    params << Generic::ShrinkText(QString::number(QueryPool::HugglePool->ProcessingEdits.count()), 3)
           << Generic::ShrinkText(QString::number(QueryPool::HugglePool->RunningQueriesGetCount()), 3)
           << QString::number(this->GetCurrentWikiSite()->GetProjectConfig()->WhiteList.size())
           << Generic::ShrinkText(QString::number(this->Queue1->Items.count()), 4);
    QString statistics_;
    // calculate stats, but not if huggle uptime is lower than 50 seconds
    qint64 Uptime = this->GetCurrentWikiSite()->Provider->GetUptime();
    if (this->ShuttingDown)
    {
        statistics_ = _l("main-statistics-none");
    } else if (Uptime < 50)
    {
        statistics_ = _l("main-statistics-waiting");
    } else if (this->GetCurrentWikiSite()->Provider->IsPaused())
    {
        statistics_ = _l("main-statistics-paused");
    } else
    {
        double EditsPerMinute = this->GetCurrentWikiSite()->Provider->GetEditsPerMinute();
        double RevertsPerMinute = this->GetCurrentWikiSite()->Provider->GetRevertsPerMinute();
        double VandalismLevel = 0;
        if (EditsPerMinute > 0 && RevertsPerMinute > 0)
            VandalismLevel = (RevertsPerMinute / (EditsPerMinute / 2)) * 10;
        QString color = "green";
        if (VandalismLevel > 0.8)
            color = "blue";
        if (VandalismLevel > 1.2)
            color = "black";
        if (VandalismLevel > 1.8)
            color = "red";
        // make the numbers easier to read
        EditsPerMinute = static_cast<double>(qRound(EditsPerMinute * 100)) / 100;
        RevertsPerMinute = static_cast<double>(qRound(RevertsPerMinute * 100)) / 100;
        VandalismLevel = static_cast<double>(qRound(VandalismLevel * 100)) / 100;
        QStringList counter_params;
        counter_params << Generic::ShrinkText(QString::number(EditsPerMinute), 6)
               << Generic::ShrinkText(QString::number(RevertsPerMinute), 6)
               << Generic::ShrinkText(QString::number(VandalismLevel), 8);
        statistics_ = " <font color=" + color + ">" + _l("main-stat", counter_params) + "</font>";
    }
    if (hcfg->Verbosity > 0)
        statistics_ += " QGC: " + QString::number(GC::gc->list.count()) + " U: " + QString::number(WikiUser::ProblematicUsers.count());
    params << statistics_ << this->GetCurrentWikiSite()->Name;
    QString status_text = _l("main-status-bar", params);
#ifdef HUGGLE_METRICS
    qint64 response_time = QueryPool::HugglePool->GetAverageExecutionTime();
    if (response_time >= 0)
        status_text += " | " + _l("main-metric-bar", QString::number(QueryPool::HugglePool->GetAverageExecutionTime()));
#endif
    status_text = UiHooks::MainStatusBarUpdate(status_text);
    this->Status->setText(status_text);
}

bool MainWindow::EditingChecks()
{
    if (!this->CheckExit() || !this->CheckEditableBrowserPage())
        return false;

    if (Configuration::HuggleConfiguration->DeveloperMode)
    {
        Generic::DeveloperError();
        return false;
    }
    return true;
}

void MainWindow::DecreaseBS()
{
    if (this->CurrentEdit != nullptr)
        this->CurrentEdit->User->SetBadnessScore(this->CurrentEdit->User->GetBadnessScore() - 200);
}

void MainWindow::IncreaseBS()
{
    if (this->CurrentEdit != nullptr)
        this->CurrentEdit->User->SetBadnessScore(this->CurrentEdit->User->GetBadnessScore() + 200);
}

OverlayBox *MainWindow::ShowOverlay(const QString &text, int x, int y, int timeout, int width, int height, bool is_dismissable)
{
    QPoint global;
    if (x < 0)
    {
        global = QPoint(this->pos().x() + (this->width() / 2) - (OverlayBox::GetDefaultOverlayWidth() / 2), this->pos().y() + 100);
    } else
    {
        global = this->mapToGlobal(QPoint(x, y));
    }
    return OverlayBox::ShowOverlay(this, text, global.x(), global.y(), timeout, width, height, is_dismissable);
}

void MainWindow::GoForward()
{
    if (this->CurrentEdit == nullptr || this->CurrentEdit->Next == nullptr)
        return;
    this->DisplayEdit(this->CurrentEdit->Next, true);
}

void MainWindow::GoBackward()
{
    if (this->CurrentEdit == nullptr || this->CurrentEdit->Previous == nullptr)
        return;
    this->DisplayEdit(this->CurrentEdit->Previous, true);
}

void MainWindow::ShowToolTip(const QString& text)
{
    QPoint pntr(this->pos().x() + (this->width() / 2), this->pos().y() + 100);
    QToolTip::showText(pntr, text, this);
}

void MainWindow::ShutdownForm()
{
    if (this->wlt)
        this->wlt->stop();
    this->tStatusBarRefreshTimer->stop();
    this->generalTimer->stop();
    this->tCheck->stop();
    this->deleteLater();
    this->close();
}

QLabel *MainWindow::CreateStatusBarLabel(const QString& text)
{
    QLabel *lb = new QLabel(this->statusBar());
    this->statusBar()->addWidget(lb);
    lb->setText(text);
    return lb;
}

void MainWindow::RemoveStatusBarItem(QWidget *widget)
{
    this->statusBar()->removeWidget(widget);
}

void MainWindow::ReloadSc()
{
    Configuration::HuggleConfiguration->ReloadOfMainformNeeded = false;
    QStringList shorts = Configuration::HuggleConfiguration->Shortcuts.keys();
    foreach (QString sh, shorts)
    {
        this->ReloadShort(sh);
    }
}

static inline void ReloadIndexedMenuShortcut(const QList<QAction *>& list, int item, const Shortcut &s)
{
    if (list.count() <= item)
        return;

    list.at(item)->setShortcut(s.QAccel);
}

void MainWindow::ReloadShort(const QString& id)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    if (!Configuration::HuggleConfiguration->Shortcuts.contains(id))
        throw new Huggle::Exception("Invalid shortcut name", BOOST_CURRENT_FUNCTION);
    Shortcut s = Configuration::HuggleConfiguration->Shortcuts[id];
    if (!UiHooks::MainWindow_ReloadShortcut(&s))
        return;
    QAction *q = nullptr;
    QAction *tip = nullptr;
    // now this horrid switch
    switch (s.ID)
    {
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN:
            tip = this->ui->actionRevert_and_warn;
            q = this->ui->actionRevert_currently_displayed_edit_and_warn_the_user;
            break;
        case HUGGLE_ACCEL_MAIN_REVERT:
            q = this->ui->actionRevert_currently_displayed_edit;
            tip = this->ui->actionRevert;
            break;
        case HUGGLE_ACCEL_MAIN_USER_CUSTOM_MSG:
            q = this->ui->actionPost_a_custom_message;
            break;
        case HUGGLE_ACCEL_MAIN_USER_CONTRIBUTIONS:
            q = this->ui->actionUser_contributions;
            break;
        case HUGGLE_ACCEL_MAIN_CONTRIB_BROWSER:
            q = this->ui->actionContribution_browser;
            break;
        case HUGGLE_ACCEL_CREATE_NEW_TAB:
            q = this->ui->actionOpen_new_tab;
            break;
        case HUGGLE_ACCEL_REVERT_STAY:
            q = this->ui->actionRevert_currently_displayed_edit_and_stay_on_page;
            break;
        case HUGGLE_ACCEL_REVW_STAY:
            q = this->ui->actionRevert_currently_displayed_edit_warn_user_and_stay_on_page;
            break;
        case HUGGLE_ACCEL_MAIN_WATCH:
            q = this->ui->actionInsert_page_to_a_watchlist;
            break;
        case HUGGLE_ACCEL_CLOSE_TAB:
            q = this->ui->actionClose_current_tab;
            break;
        case HUGGLE_ACCEL_MAIN_UNWATCH:
            q = this->ui->actionRemove_page_from_a_watchlist;
            break;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN0:
            ReloadIndexedMenuShortcut(this->revertAndWarnItems, 0, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN1:
            ReloadIndexedMenuShortcut(this->revertAndWarnItems, 1, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN2:
            ReloadIndexedMenuShortcut(this->revertAndWarnItems, 2, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN3:
            ReloadIndexedMenuShortcut(this->revertAndWarnItems, 3, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN4:
            ReloadIndexedMenuShortcut(this->revertAndWarnItems, 4, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN5:
            ReloadIndexedMenuShortcut(this->revertAndWarnItems, 5, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN6:
            ReloadIndexedMenuShortcut(this->revertAndWarnItems, 6, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN7:
            ReloadIndexedMenuShortcut(this->revertAndWarnItems, 7, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN8:
            ReloadIndexedMenuShortcut(this->revertAndWarnItems, 8, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN9:
            ReloadIndexedMenuShortcut(this->revertAndWarnItems, 9, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN10:
            ReloadIndexedMenuShortcut(this->revertAndWarnItems, 10, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN11:
            ReloadIndexedMenuShortcut(this->revertAndWarnItems, 11, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN12:
            ReloadIndexedMenuShortcut(this->revertAndWarnItems, 12, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN13:
            ReloadIndexedMenuShortcut(this->revertAndWarnItems, 13, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN14:
            ReloadIndexedMenuShortcut(this->revertAndWarnItems, 14, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN15:
            ReloadIndexedMenuShortcut(this->revertAndWarnItems, 15, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN16:
            ReloadIndexedMenuShortcut(this->revertAndWarnItems, 16, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN17:
            ReloadIndexedMenuShortcut(this->revertAndWarnItems, 17, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN18:
            ReloadIndexedMenuShortcut(this->revertAndWarnItems, 18, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN19:
            ReloadIndexedMenuShortcut(this->revertAndWarnItems, 19, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN20:
            ReloadIndexedMenuShortcut(this->revertAndWarnItems, 20, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN0:
            ReloadIndexedMenuShortcut(this->warnItems, 0, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN1:
            ReloadIndexedMenuShortcut(this->warnItems, 1, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN2:
            ReloadIndexedMenuShortcut(this->warnItems, 2, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN3:
            ReloadIndexedMenuShortcut(this->warnItems, 3, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN4:
            ReloadIndexedMenuShortcut(this->warnItems, 4, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN5:
            ReloadIndexedMenuShortcut(this->warnItems, 5, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN6:
            ReloadIndexedMenuShortcut(this->warnItems, 6, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN7:
            ReloadIndexedMenuShortcut(this->warnItems, 7, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN8:
            ReloadIndexedMenuShortcut(this->warnItems, 8, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN9:
            ReloadIndexedMenuShortcut(this->warnItems, 9, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN10:
            ReloadIndexedMenuShortcut(this->warnItems, 10, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN11:
            ReloadIndexedMenuShortcut(this->warnItems, 11, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN12:
            ReloadIndexedMenuShortcut(this->warnItems, 12, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN13:
            ReloadIndexedMenuShortcut(this->warnItems, 13, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN14:
            ReloadIndexedMenuShortcut(this->warnItems, 14, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN15:
            ReloadIndexedMenuShortcut(this->warnItems, 15, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN16:
            ReloadIndexedMenuShortcut(this->warnItems, 16, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN17:
            ReloadIndexedMenuShortcut(this->warnItems, 17, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN18:
            ReloadIndexedMenuShortcut(this->warnItems, 18, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN19:
            ReloadIndexedMenuShortcut(this->warnItems, 19, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN20:
            ReloadIndexedMenuShortcut(this->warnItems, 20, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_0:
            ReloadIndexedMenuShortcut(this->revertItems, 0, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_1:
            ReloadIndexedMenuShortcut(this->revertItems, 1, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_2:
            ReloadIndexedMenuShortcut(this->revertItems, 2, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_3:
            ReloadIndexedMenuShortcut(this->revertItems, 3, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_4:
            ReloadIndexedMenuShortcut(this->revertItems, 4, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_5:
            ReloadIndexedMenuShortcut(this->revertItems, 5, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_6:
            ReloadIndexedMenuShortcut(this->revertItems, 6, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_7:
            ReloadIndexedMenuShortcut(this->revertItems, 7, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_8:
            ReloadIndexedMenuShortcut(this->revertItems, 8, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_9:
            ReloadIndexedMenuShortcut(this->revertItems, 9, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_10:
            ReloadIndexedMenuShortcut(this->revertItems, 10, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_11:
            ReloadIndexedMenuShortcut(this->revertItems, 11, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_12:
            ReloadIndexedMenuShortcut(this->revertItems, 12, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_13:
            ReloadIndexedMenuShortcut(this->revertItems, 13, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_14:
            ReloadIndexedMenuShortcut(this->revertItems, 14, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_15:
            ReloadIndexedMenuShortcut(this->revertItems, 15, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_16:
            ReloadIndexedMenuShortcut(this->revertItems, 16, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_17:
            ReloadIndexedMenuShortcut(this->revertItems, 17, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_18:
            ReloadIndexedMenuShortcut(this->revertItems, 18, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_19:
            ReloadIndexedMenuShortcut(this->revertItems, 19, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_20:
            ReloadIndexedMenuShortcut(this->revertItems, 20, s);
            return;
        case HUGGLE_ACCEL_SUSPICIOUS_EDIT:
            q = this->ui->actionFlag_as_suspicious_edit;
            break;
        case HUGGLE_ACCEL_MAIN_MYTALK_PAGE:
            q = this->ui->actionShow_talk;
            break;
        case HUGGLE_ACCEL_MAIN_WARN:
            q = this->ui->actionWarn_the_user;
            tip = this->ui->actionWarn;
            break;
        case HUGGLE_ACCEL_MAIN_REVERT_AGF_ONE_REV:
            q = this->ui->actionRevert_only_this_revision_assuming_good_faith;
            break;
        case HUGGLE_ACCEL_MAIN_GOOD:
            q = this->ui->actionFlag_as_a_good_edit;
            break;
        case HUGGLE_ACCEL_MAIN_OPEN_IN_BROWSER:
            q = this->ui->actionOpen_page_in_browser;
            break;
        case HUGGLE_ACCEL_MAIN_TALK:
            q = this->ui->actionTalk_page;
            break;
        case HUGGLE_ACCEL_MAIN_OPEN:
            q = this->ui->actionDisplay_this_page;
            break;
        case HUGGLE_ACCEL_MAIN_FORWARD:
            q = this->ui->actionForward;
            break;
        case HUGGLE_ACCEL_MAIN_BACK:
            q = this->ui->actionBack;
            break;
        case HUGGLE_ACCEL_NEXT:
            q = this->ui->actionNext_2;
            tip = this->ui->actionNext;
            break;
        case HUGGLE_ACCEL_MAIN_EXIT:
            q = this->ui->actionExit;
            break;
        case HUGGLE_ACCEL_USER_REPORT_USER_NAME:
            q = this->ui->actionReport_username;
            break;
        case HUGGLE_ACCEL_USER_REPORT:
            q = this->ui->actionReport_user_2;
            break;
        case HUGGLE_ACCEL_MAIN_PATROL:
            q = this->ui->actionPatrol;
            break;
        case HUGGLE_ACCEL_MAIN_C_REVERT:
            q = this->ui->actionRevert_edit_using_custom_reason;
            break;
        case HUGGLE_ACCEL_MAIN_REVERT_AGF:
            q = this->ui->actionRevert_AGF;
            break;
        case HUGGLE_ACCEL_MAIN_REFRESH:
            q = this->ui->actionRefresh;
            break;
        case HUGGLE_ACCEL_MAIN_USER_CLEAR_TALK:
            q = this->ui->actionClear_talk_page_of_user;
            break;
        case HUGGLE_ACCEL_MAIN_CLEAR_QUEUE:
            q = this->ui->actionClear;
            break;
        case HUGGLE_ACCEL_MAIN_EDIT:
            q = this->ui->actionEdit_page;
            break;
        case HUGGLE_ACCEL_MAIN_EDIT_IN_BROWSER:
            q = this->ui->actionEdit_page_in_browser;
            break;
    }

    if (q != nullptr)
    {
        q->setShortcut(QKeySequence(s.QAccel));
        q->setToolTip(_l(s.Description));
    }
    if (tip != nullptr)
        tip->setToolTip(_l(s.Description) + " [" + s.QAccel + "]");
}

void MainWindow::ProcessReverts()
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    if (this->RevertStack.count())
    {
        QDateTime now = QDateTime::currentDateTime().addSecs(-1 * Configuration::HuggleConfiguration->SystemConfig_RevertDelay);
        int x = 0;
        while (x < this->RevertStack.count())
        {
            RevertQuery *query_ = this->RevertStack.at(x);
            if (now > query_->Date)
            {
                // we can finally revert it
                query_->Process();
                query_->DecRef();
                this->RevertStack.removeAt(x);
            } else
            {
                x++;
            }
        }
    }
}

void MainWindow::insertRelatedEditsToQueue()
{
    QList<revid_ht> revs = this->wUserInfo->GetTopRevisions();
    foreach (revid_ht r, revs)
    {
        if (r == this->CurrentEdit->RevID)
            continue;
        HUGGLE_DEBUG("Inserting top revision of reverted user to Queue: " + QString::number(r), 2);
        this->Queue1->AddUnprocessedEditFromRevID(r, this->GetCurrentWikiSite());
    }
}

QString MainWindow::WikiScriptURL()
{
    if (this->CurrentEdit == nullptr || this->CurrentEdit->Page == nullptr)
    {
        return Configuration::GetProjectScriptURL();
    } else
    {
        return Configuration::GetProjectScriptURL(this->CurrentEdit->GetSite());
    }
}

Collectable_SmartPtr<RevertQuery> MainWindow::Revert(const QString& summary, bool next, bool single_rv)
{
    bool rollback = true;
    Collectable_SmartPtr<RevertQuery> revert_query;
    if (this->CurrentEdit == nullptr)
    {
        Syslog::HuggleLogs->ErrorLog(_l("main-revert-null"));
        return revert_query;
    }
    if (!this->CheckRevertable())
        return revert_query;
    if (this->CurrentEdit->NewPage)
    {
        UiGeneric::pMessageBox(this, _l("main-revert-newpage-title"), _l("main-revert-newpage"), MessageBoxStyleNormal, true);
        return revert_query;
    }
    if (!this->CurrentEdit->IsPostProcessed())
    {
        // This shouldn't ever happen, however there is still a rare bug, when you get error messaages like
        // WARNING: unable to retrieve diff for edit [edit name] fallback to web rendering
        // which is exactly what gets us in here

        // We need to keep this check until that bug is fixed, translation of this is probably not necessary
        Syslog::HuggleLogs->ErrorLog("This edit is still being processed, please wait");
        return revert_query;
    }
    if (this->CurrentEdit->GetSite()->GetProjectConfig()->Token_Rollback.isEmpty())
    {
        Syslog::HuggleLogs->WarningLog(_l("main-revert-manual", this->CurrentEdit->Page->PageName));
        rollback = false;
    }
    if (this->preflightCheck(this->CurrentEdit))
    {
        this->CurrentEdit->User->Resync();
        this->CurrentEdit->User->SetBadnessScore(this->CurrentEdit->User->GetBadnessScore(false) - 10);
        Hooks::OnRevert(this->CurrentEdit);
        if (hcfg->UserConfig->InsertEditsOfRolledUserToQueue)
            this->insertRelatedEditsToQueue();
        revert_query = WikiUtil::RevertEdit(this->CurrentEdit, summary, false, rollback);
        if (single_rv) { revert_query->SetLast(); }
        if (Configuration::HuggleConfiguration->SystemConfig_InstantReverts)
        {
            revert_query->Process();
        } else
        {
            revert_query->Date = QDateTime::currentDateTime();
            revert_query->IncRef();
            this->RevertStack.append(revert_query);
        }
        if (next)
            this->DisplayNext(revert_query);
    }
    return revert_query;
}

bool MainWindow::preflightCheck(WikiEdit *edit)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    if (!Hooks::RevertPreflight(edit))
    {
        HUGGLE_DEBUG("Hook prevented revert of " + edit->Page->PageName, 2);
        return false;
    }
    if (this->qNext != nullptr)
    {
        UiGeneric::pMessageBox(this, _l("main-revert-already-pending-title"), _l("main-revert-already-pending-text"),
                            MessageBoxStyleNormal, true);
        return false;
    }
    if (edit == nullptr)
        throw new Huggle::NullPointerException("WikiEdit *_e", BOOST_CURRENT_FUNCTION);
    bool warn = false;
    QString type = _l("main-revert-type-unknown");
    if (hcfg->SystemConfig_WarnUserSpaceRoll && edit->Page->IsUserpage())
    {
        type = _l("main-revert-type-in-userspace");
        warn = true;
    } else if (hcfg->ProjectConfig->ConfirmOnSelfRevs && (edit->User->Username.toLower() == hcfg->SystemConfig_UserName.toLower()))
    {
        type = _l("main-revert-type-made-by-you");
        warn = true;
    } else if (hcfg->ProjectConfig->ConfirmTalk && edit->Page->IsTalk())
    {
        type = _l("main-revert-type-made-on-talk-page");
        warn = true;
    } else if (hcfg->ProjectConfig->ConfirmWL && edit->User->IsWhitelisted())
    {
        type = _l("main-revert-type-made-white-list");
        warn = true;
    }
    if (warn)
    {
        int q = UiGeneric::pMessageBox(this, _l("shortcut-revert"), _l("main-revert-warn", type), MessageBoxStyleQuestion);
        if (q == QMessageBox::No)
            return false;
    }
    return true;
}

bool MainWindow::Warn(const QString& warning_type, RevertQuery *dependency, WikiEdit *related_edit)
{
    if (related_edit == nullptr)
        return false;
    bool report_user = false;
    // Call to WarnUser will change report_user to true in case that user already reached max warning level
    PendingWarning *ptr_warning = Warnings::WarnUser(warning_type, dependency, related_edit, &report_user);
    if (report_user)
    {
        // User already reached maximum level, so we need to report them instead - check if current project supports reporting,
        // whether we use silent reports, or if strict manual reports are enforced by project config
        if ((hcfg->UserConfig->AutomaticReports && related_edit->GetSite()->GetProjectConfig()->ReportMode != ReportType_StrictManual) ||
                related_edit->GetSite()->GetProjectConfig()->ReportMode == ReportType_StrictAuto)
        {
            // Silent reports are enabled, so just report user without asking Huggle user for input
            ReportUser::SilentReport(related_edit->User);
        } else
        {
            this->DisplayReportUserWindow(related_edit->User);
        }
    }
    if (ptr_warning != nullptr)
    {
        // Warning is processed on background, so we just store it for later check of results
        PendingWarning::PendingWarnings.append(ptr_warning);
        return true;
    }
    return false;
}

void MainWindow::on_actionExit_triggered()
{
    this->Exit();
}

void MainWindow::DisplayWelcomeMessage()
{
    if (hcfg->DeveloperMode)
    {
        this->Browser->RenderHtml(Resources::GetResource("/huggle/resources/Resources/html/dev.html"));
        this->LockPage();
        return;
    }
    WikiPage *welcome = new WikiPage(hcfg->ProjectConfig->WelcomeMP, hcfg->Project);
    this->Browser->DisplayPreFormattedPage(welcome);
    this->LockPage();
    delete welcome;
    this->Render();
}

void MainWindow::finishRestore()
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    if (this->RestoreEdit == nullptr || this->RestoreQuery == nullptr || !this->RestoreQuery->IsProcessed())
        return;
    QDomDocument d;
    d.setContent(this->RestoreQuery->Result->Data);
    QDomNodeList page = d.elementsByTagName("rev");
    QDomNodeList code = d.elementsByTagName("page");
    if (code.count() > 0)
    {
        QDomElement e = code.at(0).toElement();
        if (e.attributes().contains("missing"))
        {
            this->RestoreQuery.Delete();
            this->RestoreEdit.Delete();
            Huggle::Syslog::HuggleLogs->ErrorLog(_l("main-restore-text-fail"));
            return;
        }
    }
    // get last id
    if (page.count() > 0)
    {
        QDomElement e = page.at(0).toElement();
        QString text = e.text();
        if (text.isEmpty())
        {
            this->RestoreQuery.Delete();
            Huggle::Syslog::HuggleLogs->Log(_l("main-restore-text-fail"));
            this->RestoreEdit.Delete();
            return;
        }
        QString sm = this->RestoreEdit->GetSite()->GetProjectConfig()->RestoreSummary;
        sm = sm.replace("$1", QString::number(this->RestoreEdit->RevID));
        sm = sm.replace("$2", this->RestoreEdit->User->Username);
        sm = sm.replace("$3", this->RestoreEdit_RevertReason);
        WikiUtil::EditPage(this->RestoreEdit->Page, text, sm);
    } else
    {
        HUGGLE_DEBUG1(this->RestoreQuery->Result->Data);
        Syslog::HuggleLogs->ErrorLog(_l("main-restore-data-fail"));
    }
    this->RestoreEdit.Delete();
    this->RestoreQuery.Delete();
}

void MainWindow::createBrowserTab(const QString& name, int index)
{
    QWidget *tab = new QWidget(this);
    HuggleWeb *web = new HuggleWeb();
    this->Browsers.append(web);
    QVBoxLayout *lay = new QVBoxLayout(tab);
    lay->setSizeConstraint(QLayout::SetNoConstraint);
    tab->setLayout(lay);
    lay->setSpacing(0);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(web);
    this->ui->tabWidget->insertTab(index, tab, name);
    this->ui->tabWidget->setCurrentIndex(index);
    this->Browser = web;
    if (this->Browsers.count() > 1)
        this->ui->tabWidget->setTabsClosable(true);
    this->Browser->RenderHtml(Resources::GetNewTabHTML());
}

void MainWindow::changeCurrentBrowserTabTitle(const QString& name)
{
    this->ui->tabWidget->setTabText(this->ui->tabWidget->currentIndex(), Generic::ShrinkText(name, 20, false, 3));
}

void MainWindow::triggerWelcome()
{
    ProjectConfiguration *conf = this->GetCurrentWikiSite()->GetProjectConfig();
    QString message;
    if (this->CurrentEdit->User->IsIP() && !conf->WelcomeAnon.isEmpty())
    {
        message = conf->WelcomeAnon + " ~~~~";
    }
    if (message.isEmpty())
    {
        if (conf->Welcome.isEmpty() && conf->WelcomeTypes.isEmpty())
        {
            // This error should never happen so we don't need to localize this
            Syslog::HuggleLogs->ErrorLog("There are no welcome messages defined for this project");
            return;
        }
        if (!conf->Welcome.isEmpty())
            message = conf->Welcome;
        else
           message = HuggleParser::GetValueFromSSItem(conf->WelcomeTypes.at(0));
        message += " ~~~~";
    }
    this->welcomeCurrentUser(message);
}

void MainWindow::triggerWarn()
{
    if (!this->CheckExit() || !this->CheckEditableBrowserPage())
        return;
    if (Configuration::HuggleConfiguration->DeveloperMode)
    {
        Generic::DeveloperError();
        return;
    }
    if (Configuration::HuggleConfiguration->UserConfig->ManualWarning)
    {
        delete this->fWarningList;
        this->fWarningList = new Huggle::WarningList(this->CurrentEdit, this);
        this->fWarningList->show();
        return;
    }
    this->Warn(this->GetCurrentWikiSite()->GetProjectConfig()->DefaultTemplate, nullptr, this->CurrentEdit);
}

void MainWindow::on_actionPreferences_triggered()
{
    Preferences preferencesForm;
    preferencesForm.exec();
}

void MainWindow::on_actionContents_triggered()
{
    QDesktopServices::openUrl(QUrl(Configuration::HuggleConfiguration->GlobalConfig_DocumentationPath));
}

void MainWindow::on_actionAbout_triggered()
{
    if (!this->aboutForm)
        this->aboutForm = new AboutForm(this);

    this->aboutForm->show();
}

void MainWindow::OnMainTimerTick()
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    foreach (WikiSite *site, hcfg->Projects)
    {
        ProjectConfiguration *cfg = site->GetProjectConfig();
        if (!cfg->IsLoggedIn && !cfg->RequestingLogin && !hcfg->DeveloperMode)
        {
            delete this->fRelogin;
            // we need to flag it here so that we don't reload the form next tick
            cfg->RequestingLogin = true;
            this->fRelogin = new ReloginForm(site, this);
            // exec to freeze
            this->fRelogin->exec();
            return;
        }
    }
    if (Configuration::HuggleConfiguration->ReloadOfMainformNeeded)
        this->ReloadSc();
    this->ProcessReverts();
    WikiUtil::FinalizeMessages();
    bool RetrieveEdit = true;
#ifndef HUGGLE_USE_MT_GC
    if (Core::HuggleCore->gc)
    {
        Core::HuggleCore->gc->DeleteOld();
    }
#endif
    // if there is no working feed, let's try to fix it
    WikiSite *site = this->GetCurrentWikiSite();
    if (!site->Provider->IsWorking() && !this->ShuttingDown)
    {
        Syslog::HuggleLogs->Log(_l("provider-failure", site->Provider->ToString(), this->GetCurrentWikiSite()->Name));
        this->SwitchAlternativeFeedProvider(site);
    }
    Warnings::ResendWarnings();
    // check if queue isn't full
    if (this->Queue1->Items.count() > Configuration::HuggleConfiguration->SystemConfig_QueueSize)
    {
        if (this->ui->actionStop_feed->isChecked())
        {
            this->PauseQueue();
            RetrieveEdit = false;
        } else
        {
            if (this->ui->actionStop_provider->isVisible())
                this->ResumeQueue();
            this->Queue1->Trim();
        }
    } else if (this->ui->actionStop_feed->isChecked() && this->ui->actionStop_provider->isVisible())
    {
        // Automaticall resume the queue in case it was paused due to auto-stop feature
        if (this->QueueIsNowPaused)
            this->ResumeQueue();
    }
    if (RetrieveEdit)
    {
        bool full = true;
        while (full)
        {
            full = false;
            foreach (WikiSite *wiki, Configuration::HuggleConfiguration->Projects)
            {
                if (!wiki->Provider->ContainsEdit())
                    continue;

                // we take the edit and start post processing it
                WikiEdit *edit = wiki->Provider->RetrieveEdit();
                if (edit != nullptr)
                {
                    QueryPool::HugglePool->PostProcessEdit(edit);
                    edit->RegisterConsumer(HUGGLECONSUMER_MAINPEND);
                    edit->DecRef();
                    this->PendingEdits.append(edit);
                }

                if (!full && wiki->Provider->ContainsEdit())
                    full = true;
            }
        }
    }
    if (this->PendingEdits.count() > 0)
    {
        // postprocessed edits can be added to queue
        int edit_id = 0;
        while (edit_id < this->PendingEdits.count())
        {
            if (this->PendingEdits.at(edit_id)->IsReady() && this->PendingEdits.at(edit_id)->IsPostProcessed())
            {
                WikiEdit *edit = this->PendingEdits.at(edit_id);
                Hooks::WikiEdit_ScoreJS(edit);
                // We need to check the edit against filter once more, because some of the checks work
                // only on post processed edits
                if (edit->GetSite()->CurrentFilter->Matches(edit))
                    this->Queue1->AddItem(edit);
                this->PendingEdits.removeAt(edit_id);
                edit->UnregisterConsumer(HUGGLECONSUMER_MAINPEND);
            } else
            {
                edit_id++;
            }
        }
    }
    // let's refresh the edits that are being post processed
    if (QueryPool::HugglePool->ProcessingEdits.count() > 0)
    {
        int edit_id = 0;
        while (edit_id < QueryPool::HugglePool->ProcessingEdits.count())
        {
            WikiEdit *edit = QueryPool::HugglePool->ProcessingEdits.at(edit_id);
            if (edit->finalizePostProcessing())
            {
                QueryPool::HugglePool->ProcessingEdits.removeAt(edit_id);
                edit->UnregisterConsumer(HUGGLECONSUMER_CORE_POSTPROCESS);
            } else
            {
                edit_id++;
            }
        }
    }
    QueryPool::HugglePool->CheckQueries();
    if (this->SystemLog->isVisible())
    {
        // We need to copy the list of unwritten logs so that we don't hold the lock for so long
        QList<HuggleLog_Line> logs;
        Syslog::HuggleLogs->lUnwrittenLogs->lock();
        logs.append(Syslog::HuggleLogs->UnwrittenLogs);
        Syslog::HuggleLogs->UnwrittenLogs.clear();
        Syslog::HuggleLogs->lUnwrittenLogs->unlock();
        while (logs.count())
        {
            this->SystemLog->InsertText(logs.at(0));
            logs.removeFirst();
        }
    }
    this->Queries->RemoveExpired();
    if (this->OnNext_EvPage != nullptr && this->qNext != nullptr && this->qNext->IsProcessed())
    {
        this->tb->SetPage(this->OnNext_EvPage);
        this->tb->DownloadEdit();
        delete this->OnNext_EvPage;
        this->OnNext_EvPage = nullptr;
        this->qNext = nullptr;
    }
    this->finishRestore();
    this->TruncateReverts();
    this->SystemLog->Render();
}

void MainWindow::TruncateReverts()
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    while (QueryPool::HugglePool->UncheckedReverts.count() > 0)
    {
        WikiEdit *edit = QueryPool::HugglePool->UncheckedReverts.at(0);
        if (Huggle::Configuration::HuggleConfiguration->UserConfig->DeleteEditsAfterRevert)
        {
            // we need to delete older edits that we know and that may be somewhere in queue
            if (this->Queue1 != nullptr)
                this->Queue1->DeleteOlder(edit);
        }
        // we swap the edit from one pool to another, so we need to switch the consumers as well
        edit->RegisterConsumer(HUGGLECONSUMER_QP_REVERTBUFFER);
        edit->UnregisterConsumer(HUGGLECONSUMER_QP_UNCHECKED);
        QueryPool::HugglePool->UncheckedReverts.removeAt(0);
        QueryPool::HugglePool->RevertBuffer.append(edit);
    }
    while (QueryPool::HugglePool->RevertBuffer.count() > 10)
    {
        WikiEdit *edit = QueryPool::HugglePool->RevertBuffer.at(0);
        QueryPool::HugglePool->RevertBuffer.removeAt(0);
        edit->UnregisterConsumer(HUGGLECONSUMER_QP_REVERTBUFFER);
    }
}

void MainWindow::OnTimerTick0()
{
    if (this->Shutdown != ShutdownOpUpdatingConf)
    {
        // we can clear the queue meanwhile
        this->Queue1->Clear();
        if (this->Shutdown == ShutdownOpRetrievingWhitelist)
        {
            if (hcfg->SystemConfig_WhitelistDisabled)
            {
                // we finished writing the wl
                this->fWaiting->Status(60, _l("saveuserconfig-progress"));
                this->Shutdown = ShutdownOpGracetimeQueries;
                return;
            }
            this->Shutdown = ShutdownOpUpdatingWhitelist;
            this->fWaiting->Status(20, _l("updating-wl"));
            foreach (WikiSite*site, Configuration::HuggleConfiguration->Projects)
            {
                if (this->WhitelistQueries.contains(site))
                {
                    this->WhitelistQueries[site]->DecRef();
                    this->WhitelistQueries.remove(site);
                }
                if (!site->GetProjectConfig()->NewWhitelist.count())
                    continue;
                this->WhitelistQueries.insert(site, new WLQuery(site));
                this->WhitelistQueries[site]->WL_Type = WLQueryType_WriteWL;
                this->WhitelistQueries[site]->IncRef();
                this->WhitelistQueries[site]->Process();
            }
            return;
        }
        if (this->Shutdown == ShutdownOpUpdatingWhitelist)
        {
            bool finished = true;
            foreach (WikiSite *site, Configuration::HuggleConfiguration->Projects)
            {
                if (this->WhitelistQueries.contains(site))
                {
                    if(!this->WhitelistQueries[site]->IsProcessed())
                    {
                        finished = false;
                        break;
                    } else
                    {
                        this->WhitelistQueries[site]->DecRef();
                        this->WhitelistQueries.remove(site);
                    }
                }
            }
            if (!finished)
            {
                this->fWaiting->Status(20);
                return;
            }
            // we finished writing the wl
            this->fWaiting->Status(60, _l("gracetime"));
            this->Shutdown = ShutdownOpGracetimeQueries;
            // now we need to give a gracetime to queries
            this->Gracetime = QDateTime::currentDateTime();
            return;
        }
        if (this->Shutdown == ShutdownOpGracetimeQueries)
        {
            // check if there are still some queries that need to finish
            if (QueryPool::HugglePool->GetRunningEditingQueries() > 0 && this->Gracetime.addSecs(6) > QDateTime::currentDateTime())
                return;
            // we finished waiting for the editing queries
            this->fWaiting->Status(80, _l("saveuserconfig-progress"));
            this->Shutdown = ShutdownOpUpdatingConf;
            QString page = Configuration::HuggleConfiguration->GlobalConfig_UserConf;
            page = page.replace("$1", Configuration::HuggleConfiguration->SystemConfig_UserName);
            Version huggle_version(HUGGLE_VERSION);
            foreach (WikiSite *site, Configuration::HuggleConfiguration->Projects)
            {
                if (*site->UserConfig->Previous_Version > huggle_version)
                {
                    if (UiGeneric::pMessageBox(this, _l("main-config-version-mismatch-title"),
                                               _l("main-config-version-mismatch-text", QString(HUGGLE_VERSION), 
                                                  site->UserConfig->Previous_Version->ToString()),
                                               MessageBoxStyleQuestion, true) == QMessageBox::No)
                        continue;
                }
                Collectable_SmartPtr<EditQuery> edit_query = WikiUtil::EditPage(site, page, site->GetUserConfig()->MakeLocalUserConfig(site->GetProjectConfig()),
                                                                          _l("saveuserconfig-progress"), true);
                edit_query->IncRef();
                this->storageQueries.insert(site, edit_query.GetPtr());
            }
            return;
        }
    } else
    {
        // we need to check if config was written
        foreach (WikiSite *site, Configuration::HuggleConfiguration->Projects)
        {
            if (this->storageQueries.contains(site))
            {
                if (this->storageQueries[site]->IsProcessed())
                {
                    if (this->storageQueries[site]->IsFailed())
                    {
                        HUGGLE_ERROR("Unable to save personal config on " + site->Name + ": " + this->storageQueries[site]->GetFailureReason());
                    }
                    this->storageQueries[site]->DecRef();
                    this->storageQueries.remove(site);
                } else
                {
                    return;
                }
            }
        }
        this->ShutdownForm();
        this->wlt->stop();
        Core::HuggleCore->Shutdown();
    }
}

void MainWindow::on_actionNext_triggered()
{
    if (!this->Queue1->Next())
        this->ShowEmptyQueuePage();
}

void MainWindow::on_actionNext_2_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_NEXT))
        return;
    if (!this->Queue1->Next())
        this->ShowEmptyQueuePage();
}

void MainWindow::on_actionWarn_triggered()
{
    this->triggerWarn();
}

void MainWindow::on_actionRevert_currently_displayed_edit_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_REVERT))
        return;
    if (this->EditingChecks() && this->CheckRevertable())
        this->Revert();
}

void MainWindow::on_actionWarn_the_user_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_WARN))
        return;
    this->triggerWarn();
}

void MainWindow::on_actionRevert_currently_displayed_edit_and_warn_the_user_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_REVERT_AND_WARN))
        return;
    if (!this->EditingChecks() || !this->CheckRevertable())
        return;
    Collectable_SmartPtr<RevertQuery> result = this->Revert("");
    if (result != nullptr)
    {
        this->Warn(result->GetEdit()->GetSite()->GetProjectConfig()->DefaultTemplate, result, result->GetEdit());
    }
}

void MainWindow::on_actionRevert_and_warn_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_REVERT_AND_WARN))
        return;
    if (!this->EditingChecks() || !this->CheckRevertable())
        return;
    Collectable_SmartPtr<RevertQuery> result = this->Revert("");
    if (result != nullptr)
    {
        this->Warn(result->GetEdit()->GetSite()->GetProjectConfig()->DefaultTemplate, result, result->GetEdit());
    }
}

void MainWindow::on_actionRevert_triggered()
{
    if (!this->EditingChecks() || !this->CheckRevertable())
        return;

    this->Revert();
}

void MainWindow::on_actionShow_ignore_list_of_current_wiki_triggered()
{
    delete this->Ignore;
    this->Ignore = new IgnoreList(this);
    this->Ignore->show();
}

void MainWindow::on_actionForward_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_FORWARD))
        return;
    // Navigate to next edit if there is some
    this->GoForward();
}

void MainWindow::on_actionBack_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_BACK))
        return;
    // Navigate back in chain
    this->GoBackward();
}

void MainWindow::CustomWelcome()
{
    if (!this->EditingChecks())
        return;
    QAction *welcome = reinterpret_cast<QAction*>(QObject::sender());
    this->welcomeCurrentUser(HuggleParser::GetValueFromSSItem(welcome->data().toString()));
}

void MainWindow::CustomRevert()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_REVERT))
        return;
    if (!this->EditingChecks() || !this->CheckRevertable())
        return;
    QAction *revert = reinterpret_cast<QAction*>(QObject::sender());
    ProjectConfiguration *conf = this->GetCurrentWikiSite()->GetProjectConfig();
    UserConfiguration *ucf = this->GetCurrentWikiSite()->GetUserConfig();
    if (!this->actionKeys.contains(revert))
        throw new Huggle::Exception("QAction was not found in this->actionKeys", BOOST_CURRENT_FUNCTION);
    QString key = this->actionKeys[revert];
    QString summary = HuggleParser::GetSummaryOfWarningTypeFromWarningKey(key, conf, ucf);
    if (summary.isEmpty())
    {
        HUGGLE_ERROR(this->GetCurrentWikiSite()->Name + ": Edit summary for revert of " + this->GetCurrentWikiEdit()->Page->PageName + " is empty! Revert aborted.");
        return;
    }
    summary = Huggle::Configuration::GenerateSuffix(summary, conf);
    this->Revert(summary);
}

void MainWindow::CustomRevertWarn()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_REVERT_AND_WARN))
        return;
    if (!this->EditingChecks() || !this->CheckRevertable())
        return;
    QAction *revert = reinterpret_cast<QAction*>(QObject::sender());
    ProjectConfiguration *conf = this->GetCurrentWikiSite()->GetProjectConfig();
    UserConfiguration *uconf = this->GetCurrentWikiSite()->GetUserConfig();
    if (!this->actionKeys.contains(revert))
        throw new Huggle::Exception("QAction was not found in this->actionKeys", BOOST_CURRENT_FUNCTION);
    QString key = this->actionKeys[revert];
    QString summary = HuggleParser::GetSummaryOfWarningTypeFromWarningKey(key, conf, uconf);
    if (summary.isEmpty())
    {
        HUGGLE_ERROR(this->GetCurrentWikiSite()->Name + ": Edit summary for revert of " + this->GetCurrentWikiEdit()->Page->PageName + " is empty! Revert aborted.");
        return;
    }
    summary = Huggle::Configuration::GenerateSuffix(summary, conf);
    Collectable_SmartPtr<RevertQuery> result = this->Revert(summary, false);
    if (result != nullptr)
    {
        this->Warn(key, result, result->GetEdit());
        this->DisplayNext(result);
    } else
    {
        this->DisplayNext(result);
    }
}

void MainWindow::CustomWarn()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_WARN))
        return;
    if (!this->EditingChecks())
        return;
    QAction *revert = reinterpret_cast<QAction*>(QObject::sender());
    if (!this->actionKeys.contains(revert))
        throw new Huggle::Exception("QAction was not found in this->actionKeys", BOOST_CURRENT_FUNCTION);
    QString key = this->actionKeys[revert];
    this->Warn(key, nullptr, this->CurrentEdit);
}

void MainWindow::EnableDev()
{
    QAction *debugm = this->ui->menuDebug_2->menuAction();
    this->ui->menuHelp->addAction(debugm);
}

void MainWindow::ForceWarn(int level)
{
    if (!this->CheckExit())
        return;
    Warnings::ForceWarn(level, this->CurrentEdit);
}

void MainWindow::Exit()
{
    if (this->ShuttingDown)
        return;
    this->ShuttingDown = true;
    this->VandalDock->Disconnect();
    if (hcfg->SystemConfig_SaveLayout)
    {
        QFile *layout = new QFile(Configuration::GetConfigurationPath() + "mainwindow_state");
        if (!layout->open(QIODevice::ReadWrite | QIODevice::Truncate))
            Syslog::HuggleLogs->ErrorLog("Unable to write state to a config file");
        else
            layout->write(this->saveState());
        layout->close();
        delete layout;
        layout = new QFile(Configuration::GetConfigurationPath() + "mainwindow_geometry");
        if (!layout->open(QIODevice::ReadWrite | QIODevice::Truncate))
            Syslog::HuggleLogs->ErrorLog("Unable to write geometry to a config file");
        else
            layout->write(this->saveGeometry());
        layout->close();
        delete layout;
    }
    if (Configuration::HuggleConfiguration->DeveloperMode)
    {
        this->ShutdownForm();
        MainWindow::HuggleMain = nullptr;
        Core::HuggleCore->Shutdown();
        return;
    }
    this->Shutdown = ShutdownOpRetrievingWhitelist;
    foreach (WikiSite *site, Configuration::HuggleConfiguration->Projects)
    {
        if (site->Provider != nullptr)
            site->Provider->Stop();
    }
    delete this->fWaiting;
    this->fWaiting = new WaitingForm(this);
    this->fWaiting->show();
    this->fWaiting->Status(10, _l("whitelist-download"));
    this->wlt = new QTimer(this);
    connect(this->wlt, SIGNAL(timeout()), this, SLOT(OnTimerTick0()));
    this->wlt->start(800);
    Configuration::SaveSystemConfig();
}

bool MainWindow::BrowserPageIsEditable()
{
    return this->EditablePage;
}

bool MainWindow::CheckEditableBrowserPage()
{
    if (!this->EditablePage || this->CurrentEdit == nullptr)
    {
        UiGeneric::pMessageBox(this, _l("main-action-unavailable-title"), _l("main-no-page"), MessageBoxStyleNormal, true);
        return false;
    }
    if (hcfg->SystemConfig_RequestDelay)
    {
        qint64 wt = QDateTime::currentDateTime().msecsTo(this->editLoadDateTime.addSecs(hcfg->SystemConfig_DelayVal));
        if (wt > 0)
        {
            Syslog::HuggleLogs->WarningLog("Ignoring edit request because you are too fast, please wait " +
                                           QString::number(wt)+ " ms");
            return false;
        }
    }
    if (this->CurrentEdit->Page == nullptr)
        throw Huggle::NullPointerException("local CurrentEdit->Page", BOOST_CURRENT_FUNCTION);

    return true;
}

void MainWindow::SuspiciousEdit()
{
    if (!this->CheckExit() || !CheckEditableBrowserPage())
        return;
    if (this->CurrentEdit != nullptr)
    {
        Hooks::Suspicious(this->CurrentEdit);
        WLQuery *wq_ = new WLQuery(this->GetCurrentWikiSite());
        wq_->WL_Type = WLQueryType_SuspWL;
        wq_->Parameters = "page=" + QUrl::toPercentEncoding(this->CurrentEdit->Page->PageName) + "&wiki="
                          + QUrl::toPercentEncoding(this->GetCurrentWikiSite()->WhiteList) + "&user="
                          + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->SystemConfig_UserName) + "&score="
                          + QString::number(this->CurrentEdit->Score) + "&revid="
                          + QString::number(this->CurrentEdit->RevID) + "&summary="
                          + QUrl::toPercentEncoding(this->CurrentEdit->Summary);
        wq_->Process();
        HUGGLE_QP_APPEND(wq_);
        this->CurrentEdit->User->SetBadnessScore(this->CurrentEdit->User->GetBadnessScore() + 1);
    }
    this->DisplayNext();
}

void MainWindow::PatrolEdit(WikiEdit *e)
{
    if (e == nullptr)
        e = this->CurrentEdit;
    if (e == nullptr)
        return;

    WikiUtil::PatrolEdit(e);
}

void MainWindow::Localize()
{
    this->ui->menuPage->setTitle(_l("main-page"));
    this->ui->menuHelp->setTitle(_l("main-help"));
    this->ui->menuUser->setTitle(_l("main-user"));
    this->ui->menuQueue->setTitle(_l("main-queue"));
    this->ui->menuFile->setTitle(_l("main-system"));
    this->ui->menuHAN->setTitle(_l("main-han"));
    this->ui->actionAbout->setText(_l("main-help-about"));
    this->ui->actionBack->setText(_l("main-browser-back"));
    this->ui->menuGo_to->setTitle(_l("main-goto"));
    this->ui->actionMy_talk_page->setText(_l("main-goto-mytalk"));
    this->ui->actionMy_Contributions->setText(_l("main-goto-mycontribs"));
    this->ui->actionBlock_user->setText(_l("main-user-block"));
    this->ui->actionClear_talk_page_of_user->setText(_l("main-user-clear-talk"));
    this->ui->actionDelete->setText(_l("main-page-delete"));
    this->ui->actionExit->setText(_l("main-system-exit"));
    this->ui->menuTools->setTitle(_l("main-tools"));
    this->ui->actionShow_ignore_list_of_current_wiki->setText(_l("main-tools-il"));
    this->ui->actionDisplay_a_session_data->setText(_l("main-tools-sess"));
    this->ui->menuWhen_queue_is_full->setTitle(_l("main-queue-when-full"));
    this->ui->actionStop_feed->setText(_l("main-queue-when-full-stop"));
    this->ui->actionRemove_old_edits->setText(_l("main-queue-when-full-remove"));
    this->ui->actionRemove_edits_made_by_whitelisted_users->setText(_l("main-queue-remove-whitelisted"));
    this->ui->actionDelete_all_edits_with_score_lower_than_200->setText(_l("main-queue-remove-200"));
    this->ui->actionClear->setText(_l("main-queue-clear"));
    this->ui->actionClear_talk_page_of_user->setText(_l("main-user-clear-tp"));
    this->ui->actionDecrease_badness_score_by_20->setText(_l("main-user-db"));
    this->ui->actionNext_2->setText(_l("main-page-next"));
    this->ui->actionIncrease_badness_score_by_20->setText(_l("main-user-ib"));
    this->ui->actionEdit_page_in_browser->setText(_l("main-page-edit"));
    this->ui->actionFlag_as_suspicious_edit->setText(_l("main-page-flag-suspicious-edit"));
    this->ui->actionFlag_as_a_good_edit->setText(_l("main-page-flag-good-edit"));
    this->ui->actionTag_2->setText(_l("main-page-tag"));
    this->ui->actionRequest_speedy_deletion->setText(_l("main-page-reqdeletion"));
    this->ui->actionDelete->setText(_l("main-page-delete"));
    this->ui->actionRequest_protection->setText(_l("main-page-reqprotection"));
    this->ui->actionProtect->setText(_l("main-page-protect"));
    this->ui->actionRestore_this_revision->setText(_l("main-page-restore"));
    this->ui->actionInsert_page_to_a_watchlist->setText(_l("main-page-watchlist-add"));
    this->ui->actionRemove_page_from_a_watchlist->setText(_l("main-page-watchlist-remove"));
    this->ui->actionClose_current_tab->setText(_l("main-page-tab-close"));
    this->ui->actionOpen_new_tab->setText(_l("main-page-tab-open"));
    this->ui->actionDisplay_a_session_data->setText(_l("main-display-session-data"));
    this->ui->actionDisplay_whitelist->setText(_l("main-display-whitelist"));
    this->ui->actionDisplay_history_in_browser->setText(_l("main-page-historypage"));
    this->ui->actionDisplay_this_page_in_browser->setText(_l("main-browser-open"));
    this->ui->actionFeedback->setText(_l("main-help-feedback"));
    this->ui->actionReport_user->setText(_l("main-user-report"));
    this->ui->actionUser_contributions->setText(_l("main-user-contribs"));
    this->ui->actionPatrol->setText(_l("main-page-patrol"));
    this->ui->actionPreferences->setText(_l("main-system-options"));
    this->ui->actionShow_talk->setText(_l("main-goto-mytalk"));
    this->ui->actionAbort_2->setText(_l("main-system-abort"));
    this->ui->menuChange_provider->setTitle(_l("main-system-change-provider"));
    this->ui->actionIRC->setText(_l("main-system-change-provider-irc"));
    this->ui->actionConnect->setText(_l("main-han-connect"));
    this->ui->actionDisconnect->setText(_l("main-han-disconnect"));
    this->ui->actionDisplay_user_messages->setText(_l("main-han-display-user-messages"));
    this->ui->actionDisplay_user_data->setText(_l("main-han-display-user-data"));
    this->ui->actionDisplay_bot_data->setText(_l("main-han-display-bot-data"));
    this->ui->actionWiki->setText(_l("main-system-change-provider-wiki"));
    this->ui->actionNext->setText(_l("main-queue-next"));
    this->ui->actionStop_provider->setText(_l("main-menu-provider-stop"));
    this->ui->actionResume_provider->setText(_l("main-menu-provider-resume"));
    this->ui->actionShow_list_of_score_words->setText(_l("main-tools-scoreword-list"));
    this->ui->actionRevert_currently_displayed_edit->setText(_l("main-revision-revert"));
    this->ui->actionRevert_currently_displayed_edit_and_stay_on_page->setText(_l("main-revision-rv-stay"));
    this->ui->actionRevert_currently_displayed_edit_and_warn_the_user->setText(_l("main-revision-revert-warn"));
    this->ui->actionRevert_currently_displayed_edit_warn_user_and_stay_on_page->setText(_l("main-revision-rws"));
    this->ui->actionRevert_AGF->setText(_l("main-revision-faith"));
    this->ui->actionWarn_the_user->setText(_l("main-user-warn"));
    this->ui->actionOpen_in_a_browser->setText(_l("main-browser-open"));
    this->ui->actionDisplay_this_page->setText(_l("main-page-display"));
    this->ui->actionWelcome_user_2->setText(_l("main-welcome-user"));
    this->ui->actionReport_username->setText(_l("main-report-username"));
    this->ui->actionReport_user_2->setText(_l("main-report-user"));
    this->ui->actionEdit_user_talk->setText(_l("main-edit-user-talk"));
    this->ui->actionRevert_edit_using_custom_reason->setText(_l("main-custom-reason-title"));
    this->ui->actionIntroduction->setText(_l("main-help-introduction"));
    this->ui->actionContents->setText(_l("main-help-contents"));
    this->ui->actionQueue_legend->setText(_l("main-help-queuelegend"));
    this->ui->actionTalk_page->setText(_l("main-user-talk"));
    this->ui->actionUser_page->setText(_l("main-user-page"));
    this->ui->menuManual_template->setTitle(_l("main-user-manualtemplate"));
    this->ui->actionPost_a_custom_message->setText(_l("custommessage-menu"));
    this->ui->actionRefresh->setText(_l("main-page-refresh"));
    this->ui->actionScripts_manager->setText(_l("main-scripting-script-manager"));
    this->ui->menuScripting->setTitle(_l("main-scripting"));
    this->ui->actionWelcome_page->setText(_l("main-help-welcome-page"));
    this->ui->actionContribution_browser->setText(_l("main-user-contribution-browser"));
    this->ui->actionRevert_only_this_revision_assuming_good_faith->setText(_l("main-revision-revert-agf"));
    this->ui->actionRevert_only_this_revision->setText(_l("main-revision-revert-only-this"));
    this->ui->actionCopy_system_log_to_clipboard->setText(_l("main-help-copy-syslog-to-clip"));

    // arrows icons should be mirrored for RTL languages
    if (Localizations::HuggleLocalizations->IsRTL())
    {
        this->ui->actionForward->setIcon(QIcon(":/huggle/pictures/Resources/browser-prev.png"));
        this->ui->actionBack->setIcon(QIcon(":/huggle/pictures/Resources/browser-next.png"));
    }
}

void MainWindow::BlockUser()
{
    if (!this->CheckExit() || !this->CheckEditableBrowserPage())
        return;
    if(this->CurrentEdit == nullptr)
    {
        Syslog::HuggleLogs->ErrorLog(_l("block-none"));
        return;
    }
    delete this->fBlockForm;
    this->fBlockForm = new BlockUserForm(this);
    this->CurrentEdit->User->Resync();
    this->fBlockForm->SetWikiUser(this->CurrentEdit->User);
    this->fBlockForm->show();
}

void MainWindow::DisplayNext(Query *q)
{
    HUGGLE_DEBUG("Showing next edit", 2);
    switch(Configuration::HuggleConfiguration->UserConfig->GoNext)
    {
        case Configuration_OnNext_Stay:
            return;
        case Configuration_OnNext_Next:
            if (!this->Queue1->Next())
                this->ShowEmptyQueuePage();
            return;
        case Configuration_OnNext_Revert:
            //! \bug This doesn't seem to work
            if (this->CurrentEdit == nullptr)
                return;
            if (q == nullptr)
            {
                if (!this->Queue1->Next())
                    this->ShowEmptyQueuePage();
                return;
            }
            delete this->OnNext_EvPage;
            this->OnNext_EvPage = new WikiPage(this->CurrentEdit->Page);
            this->qNext = q;
            return;
    }
}

void MainWindow::ShowEmptyQueuePage()
{
    if(!hcfg->UserConfig->PageEmptyQueue.isEmpty())
    {
        WikiPage empty(hcfg->UserConfig->PageEmptyQueue, hcfg->Project);
        this->Browser->DisplayPreFormattedPage(&empty);
        //this->Render();
    }
    else
    {
        this->Browser->RenderHtml(Resources::GetEmptyQueueHTML());
    }
    this->LockPage();
}

void MainWindow::RenderHtml(const QString& html)
{
    this->Browser->RenderHtml(html);
}

void MainWindow::DeletePage()
{
    if (!this->CheckExit() || !this->CheckEditableBrowserPage())
        return;
    if (this->CurrentEdit == nullptr)
    {
        Syslog::HuggleLogs->ErrorLog("unable to delete: nullptr page");
        return;
    }
    delete this->fDeleteForm;
    this->fDeleteForm = new DeleteForm(this);
    // We always want to notify creator of the page, not the user who made this edit
    WikiUser *page_founder = nullptr;
    //! \todo We are not currently sending any notification to founder of page which was delete by Huggle
    //! so this code was commented out, we might want to do that though
    /*
    if (this->CurrentEdit->Page->FounderKnown())
    {
        WikiUser *user_from_cache = WikiUser::RetrieveUser(this->CurrentEdit->Page->GetFounder(), this->CurrentEdit->GetSite());
        if (user_from_cache != nullptr)
        {
            page_founder = new WikiUser(user_from_cache);
        } else
        {
            page_founder = new WikiUser(this->CurrentEdit->Page->GetFounder(), this->CurrentEdit->GetSite());
        }
    }
    */
    this->fDeleteForm->SetPage(this->CurrentEdit->Page, page_founder);
    this->fDeleteForm->show();
}

void MainWindow::DisplayTalk()
{
    if (this->CurrentEdit == nullptr)
        return;
    // display a talk page
    WikiPage *page = new WikiPage(this->CurrentEdit->User->GetTalk(), this->CurrentEdit->GetSite());
    this->Browser->DisplayPreFormattedPage(page);
    this->LockPage();
    delete page;
}

void MainWindow::DisplayURL(const QString& uri)
{
    this->Browser->DisplayPage(uri);
}

static void DisplayRevid_Finish(WikiEdit *edit, void *source, QString er)
{
    Q_UNUSED(source);
    Q_UNUSED(er);
    MainWindow *window = MainWindow::HuggleMain;
    // this is true hack as it's async call, but we don't really need to have the edit post processed for it
    // to be rendered, let's just call it to be safe, as having unprocessed edits in buffer is a bad thing
    QueryPool::HugglePool->PostProcessEdit(edit);
    window->DisplayEdit(edit);
}

static void DisplayRevid_Error(WikiEdit *edit, void *source, const QString& error)
{
    Q_UNUSED(source);
    Syslog::HuggleLogs->ErrorLog("Unable to retrieve edit for revision " + QString::number(edit->RevID) +
                                 " reason: " + error);
    edit->SafeDelete();
}

void MainWindow::DisplayRevid(revid_ht revid, WikiSite *site)
{
    // kill currently displayed edit
    this->LockPage();
    Collectable_SmartPtr<WikiEdit> edit = WikiEdit::FromCacheByRevID(revid);
    if (edit != nullptr)
    {
        this->DisplayEdit(edit);
        this->wEditBar->RefreshPage();
        return;
    }
    // there is no such edit, let's get it
    WikiUtil::RetrieveEditByRevid(revid, site, this,
                                  reinterpret_cast<WikiUtil::RetrieveEditByRevid_Callback>(DisplayRevid_Finish),
                                  reinterpret_cast<WikiUtil::RetrieveEditByRevid_Callback>(DisplayRevid_Error));
    //this->Browser->RenderHtml(html);
}

void MainWindow::PauseQueue()
{
    if (this->QueueIsNowPaused)
        return;

    this->QueueIsNowPaused = true;
    foreach (WikiSite *site, Configuration::HuggleConfiguration->Projects)
        site->Provider->Pause();
}

void MainWindow::ResumeQueue()
{
    if (!this->QueueIsNowPaused)
        return;

    this->QueueIsNowPaused = false;
    foreach (WikiSite *site, Configuration::HuggleConfiguration->Projects)
        site->Provider->Resume();
}

void MainWindow::FlagGood()
{
    if (this->CurrentEdit == nullptr || !this->CheckExit() || !this->CheckEditableBrowserPage())
        return;
    Hooks::OnGood(this->CurrentEdit);
    this->PatrolEdit();
    this->CurrentEdit->User->SetBadnessScore(this->CurrentEdit->User->GetBadnessScore() - 200);
    if (Configuration::HuggleConfiguration->UserConfig->WelcomeGood && this->CurrentEdit->User->TalkPage_GetContents().isEmpty())
        this->triggerWelcome();
    this->DisplayNext();
}

void MainWindow::SwitchAlternativeFeedProvider(WikiSite *site)
{
    if (site->Provider == nullptr)
    {
        this->ChangeProvider(site, HUGGLE_FEED_PROVIDER_XMLRPC);
        return;
    }

    HuggleFeed *provider = HuggleFeed::GetAlternativeFeedProvider(site->Provider);

    if (!provider)
    {
        Syslog::HuggleLogs->ErrorLog("No more solutions to find alternative feed provider for site " + site->Name);
        return;
    }

    this->ChangeProvider(site, provider->GetID());
}

void MainWindow::RenderPage(const QString& Page)
{
    WikiPage *page = new WikiPage(Page, this->GetCurrentWikiSite());
    this->tb->SetPage(page);
    delete page;
    this->tb->DownloadEdit();
}

WikiSite *MainWindow::GetCurrentWikiSite()
{
    if (this->CurrentEdit == nullptr || this->CurrentEdit->Page == nullptr)
        return Configuration::HuggleConfiguration->Project;

    return this->CurrentEdit->GetSite();
}

void MainWindow::RefreshPage()
{
    if (!this->CheckExit() || this->CurrentEdit == nullptr)
        return;

    this->tb->SetPage(this->CurrentEdit->Page);
    this->tb->DownloadEdit();
}

void MainWindow::LockPage()
{
    this->EnableEditing(false);
}

bool MainWindow::IsLocked()
{
    return !this->EditablePage;
}

QString MainWindow::ProjectURL()
{
    if (this->CurrentEdit == nullptr || this->CurrentEdit->Page == nullptr)
        return Configuration::GetProjectWikiURL();
    return Configuration::GetProjectWikiURL(this->CurrentEdit->GetSite());
}

bool MainWindow::keystrokeCheck(int id)
{
    if (this->ShuttingDown)
        return true;
    if (!hcfg->SystemConfig_KeystrokeMultiPressFix)
        return true;
    if (!this->lKeyPressTime.contains(id))
    {
        this->lKeyPressTime.insert(id, QDateTime::currentDateTime());
        return true;
    }
    qint64 global_span = this->lastKeyStrokeCheck.msecsTo(QDateTime::currentDateTime());
    qint64 span = this->lKeyPressTime[id].msecsTo(QDateTime::currentDateTime());
    qint64 rate = static_cast<qint64>(hcfg->SystemConfig_KeystrokeMultiPressRate);
    if (span < rate || global_span < rate)
    {
        HUGGLE_DEBUG1("Keystroke bug on " + QString::number(id) + " span " + QString::number(span));
        return false;
    }
    this->lKeyPressTime[id] = QDateTime::currentDateTime();
    return true;
}

bool MainWindow::CheckExit()
{
    if (this->ShuttingDown)
    {
        UiGeneric::pMessageBox(this, _l("error"), _l("main-shutting-down"), MessageBoxStyleNormal, true);
        return false;
    }
    return true;
}

bool MainWindow::CheckRevertable()
{
    if (!this->GetCurrentWikiSite()->GetProjectConfig()->RevertingEnabled)
    {
        UiGeneric::pMessageBox(this, _l("error"), "This site doesn't support reverting through huggle, you can only use it to browse edits");
        return false;
    }
    return true;
}

void MainWindow::welcomeCurrentUser(QString message)
{
    if (!this->EditingChecks())
        return;
    ProjectConfiguration *conf = this->GetCurrentWikiSite()->GetProjectConfig();
    this->CurrentEdit->User->Resync();
    bool create_only = true;
    if (!this->CurrentEdit->User->TalkPage_GetContents().isEmpty())
    {
        if (UiGeneric::pMessageBox(this, "Welcome :o", _l("welcome-tp-empty-fail"), MessageBoxStyleQuestion) == QMessageBox::No)
            return;
        else
            create_only = false;
    } else if (!this->CurrentEdit->User->TalkPage_WasRetrieved())
    {
        if (UiGeneric::pMessageBox(this, "Welcome :o", _l("welcome-page-miss-fail"), MessageBoxStyleQuestion) == QMessageBox::No)
            return;
    }
    if (message.isEmpty())
    {
        // This error should never happen so we don't need to localize this
        Syslog::HuggleLogs->ErrorLog("Invalid welcome template, ignored message");
        return;
    }
    message.replace("$1", this->CurrentEdit->User->Username);
    message.replace("$page", this->CurrentEdit->Page->PageName);
    WikiUtil::MessageUser(this->CurrentEdit->User, message, conf->WelcomeTitle, conf->WelcomeSummary, false, nullptr,
                          false, false, true, this->CurrentEdit->TPRevBaseTime, create_only);
}

void MainWindow::ChangeProvider(WikiSite *site, int id)
{
    if (!this->CheckExit())
        return;

    HuggleFeed *provider = HuggleFeed::GetProviderByID(site, id);
    if (!provider)
    {
        HUGGLE_ERROR("Provider not found: id " + QString::number(id));
        return;
    }

    if (site->Provider != nullptr)
    {
        if (site->Provider->IsWorking())
            site->Provider->Stop();
    }

    site->Provider = provider;
    Syslog::HuggleLogs->Log(_l("provider-up", provider->ToString(), site->Name));
    if (!hcfg->SystemConfig_Multiple)
    {
        // uncheck all menus for provider
        this->ui->actionXmlRcs->setChecked(false);
        this->ui->actionIRC->setChecked(false);
        this->ui->actionWiki->setChecked(false);
        // check the proper one based on type
        switch (site->Provider->GetID())
        {
            case HUGGLE_FEED_PROVIDER_IRC:
                if (!site->GetProjectConfig()->UseIrc)
                {
                    Syslog::HuggleLogs->Log(_l("irc-not"));
                    this->ui->actionIRC->setEnabled(false);
                    this->SwitchAlternativeFeedProvider(site);
                    return;
                }
                this->ui->actionIRC->setChecked(true);
                break;
            case HUGGLE_FEED_PROVIDER_WIKI:
                this->ui->actionWiki->setChecked(true);
                break;
            case HUGGLE_FEED_PROVIDER_XMLRPC:
                this->ui->actionXmlRcs->setChecked(true);
                break;
        }
    } else
    {
        if (!this->lIRC.contains(site) || !this->lWikis.contains(site) || !this->lXml.contains(site))
            throw new Huggle::Exception("This site is not in provider lists", BOOST_CURRENT_FUNCTION);
        // uncheck all menus for provider
        this->lWikis[site]->setChecked(false);
        this->lXml[site]->setChecked(false);
        this->lIRC[site]->setChecked(false);
        // check the proper one based on type
        switch (site->Provider->GetID())
        {
            case HUGGLE_FEED_PROVIDER_IRC:
                if (!site->GetProjectConfig()->UseIrc)
                {   
                    Syslog::HuggleLogs->Log(_l("irc-not"));
                    this->lIRC[site]->setEnabled(false);
                    this->SwitchAlternativeFeedProvider(site);
                    return;
                }
                this->lIRC[site]->setChecked(true);
                break;
            case HUGGLE_FEED_PROVIDER_WIKI:
                this->lWikis[site]->setChecked(true);
                break;
            case HUGGLE_FEED_PROVIDER_XMLRPC:
                this->lXml[site]->setChecked(true);
                break;
        }
    }
    // try to launch the provider now
    if (!site->Provider->Start())
    {
        Syslog::HuggleLogs->ErrorLog(_l("provider-failure", provider->ToString(), site->Name));
        // provider didn't start so we need to find alternative
        this->SwitchAlternativeFeedProvider(site);
    } else if (this->QueueIsNowPaused)
    {
        site->Provider->Pause();
    }
}

void MainWindow::ReloadInterface()
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    ProjectConfiguration *conf = this->GetCurrentWikiSite()->GetProjectConfig();
    this->warnItems.clear();
    this->revertAndWarnItems.clear();
    this->revertItems.clear();
    this->actionKeys.clear();
    // Direct delete seems to cause crash for some reasons
    if (this->RevertSummaries)
        this->RevertSummaries->deleteLater();
    if (this->WarnMenu)
        this->WarnMenu->deleteLater();
    if (this->RevertWarn)
        this->RevertWarn->deleteLater();
    this->ui->actionRequest_protection->setEnabled(conf->RFPP);
    this->RevertSummaries = new QMenu(this);
    this->WarnMenu = new QMenu(this);
    this->RevertWarn = new QMenu(this);
    if (conf->WarningTypes.count() > 0)
    {
        int r=0;
        while (r< conf->WarningTypes.count())
        {
            QString prefix = QString::number(r) + ": ";
            if (!hcfg->UserConfig->NumberDropdownMenuItems)
                prefix = "";
            QString name = HuggleParser::GetValueFromSSItem(conf->WarningTypes.at(r));
            QString key = HuggleParser::GetKeyFromSSItem(conf->WarningTypes.at(r));
            QAction *action = new QAction(prefix + name, this->RevertSummaries);
            QAction *actiona = new QAction(prefix + name, this->RevertWarn);
            QAction *actionb = new QAction(prefix + name, this->WarnMenu);
            this->actionKeys.insert(action, key);
            this->actionKeys.insert(actiona, key);
            this->actionKeys.insert(actionb, key);
            this->revertAndWarnItems.append(actiona);
            this->warnItems.append(actionb);
            this->revertItems.append(action);
            this->RevertWarn->addAction(actiona);
            this->WarnMenu->addAction(actionb);
            this->RevertSummaries->addAction(action);
            r++;
            connect(action, SIGNAL(triggered()), this, SLOT(CustomRevert()));
            connect(actiona, SIGNAL(triggered()), this, SLOT(CustomRevertWarn()));
            connect(actionb, SIGNAL(triggered()), this, SLOT(CustomWarn()));
        }
    }

    delete this->WelcomeMenu;
    this->WelcomeMenu = new QMenu(this);

    if (conf->WelcomeTypes.count() > 0)
    {
        int r = 0;
        while (r < conf->WelcomeTypes.count())
        {
            QAction *action = new QAction(HuggleParser::GetKeyFromSSItem(conf->WelcomeTypes.at(r)), this->WelcomeMenu);
            QVariant qv(conf->WelcomeTypes.at(r));
            action->setData(qv);
            this->WelcomeMenu->addAction(action);
            r++;
            connect(action, SIGNAL(triggered()), this, SLOT(CustomWelcome()));
        }
    }

    this->ui->actionWelcome_user->setMenu(this->WelcomeMenu);
    this->ui->actionWarn->setMenu(this->WarnMenu);
    this->ui->actionRevert->setMenu(this->RevertSummaries);
    this->ui->actionRevert_and_warn->setMenu(this->RevertWarn);
    bool fr = (this->warnToolButtonMenu == nullptr || this->rtToolButtonMenu == nullptr || this->welcomeToolButtonMenu == nullptr);
    // replace abstract QAction with QToolButton to be able to set PopupMode for nicer menu opening
    if (this->warnToolButtonMenu == nullptr)
        this->warnToolButtonMenu = new QToolButton(this);
    if (this->rtToolButtonMenu == nullptr)
        this->rtToolButtonMenu = new QToolButton(this);
    if (this->rwToolButtonMenu == nullptr)
        this->rwToolButtonMenu = new QToolButton(this);
    if (this->welcomeToolButtonMenu == nullptr)
        this->welcomeToolButtonMenu = new QToolButton(this);
    this->warnToolButtonMenu->setDefaultAction(this->ui->actionWarn);
    this->rtToolButtonMenu->setDefaultAction(this->ui->actionRevert);
    this->rwToolButtonMenu->setDefaultAction(this->ui->actionRevert_and_warn);
    this->welcomeToolButtonMenu->setDefaultAction(this->ui->actionWelcome_user);
    this->warnToolButtonMenu->setPopupMode(QToolButton::MenuButtonPopup);
    this->rtToolButtonMenu->setPopupMode(QToolButton::MenuButtonPopup);
    this->rwToolButtonMenu->setPopupMode(QToolButton::MenuButtonPopup);
    this->welcomeToolButtonMenu->setPopupMode(QToolButton::MenuButtonPopup);
    if (fr)
    {
        // insert them before their counterparts and then delete the counterpart
        this->ui->mainToolBar->insertWidget(this->ui->actionRevert_and_warn, rwToolButtonMenu);
        this->ui->mainToolBar->removeAction(this->ui->actionRevert_and_warn);
        this->ui->mainToolBar->insertWidget(this->ui->actionRevert, rtToolButtonMenu);
        this->ui->mainToolBar->removeAction(this->ui->actionRevert);
        this->ui->mainToolBar->insertWidget(this->ui->actionWarn, warnToolButtonMenu);
        this->ui->mainToolBar->removeAction(this->ui->actionWarn);
        this->ui->mainToolBar->insertWidget(this->ui->actionWelcome_user, welcomeToolButtonMenu);
        this->ui->mainToolBar->removeAction(this->ui->actionWelcome_user);
    }
    // button action depends on adminrights
    if (conf->Rights.contains("delete"))
        this->ui->actionDelete_page->setText(_l("main-page-delete"));
    else
        this->ui->actionDelete_page->setText(_l("main-page-reqdeletion"));
    this->ReloadSc();
}

void MainWindow::ResetKeyStrokeCheck()
{
    if (!hcfg->SystemConfig_KeystrokeMultiPressFix)
        return;

    this->lastKeyStrokeCheck = QDateTime::currentDateTime();
}

void MainWindow::on_actionWelcome_user_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_WELCOME))
        return;
    this->triggerWelcome();
}

void MainWindow::on_actionOpen_in_a_browser_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_OPEN_IN_BROWSER))
        return;
    if (this->CurrentEdit != nullptr)
    {
        if (this->CurrentEdit->NewPage)
            QDesktopServices::openUrl(QUrl::fromEncoded(QString(this->ProjectURL() + this->CurrentEdit->Page->EncodedName()).toUtf8()));
        else if (this->CurrentEdit->Diff > 0)
            QDesktopServices::openUrl(QUrl::fromEncoded(QString(Configuration::GetProjectScriptURL(this->GetCurrentWikiSite()) +
                                                                  "index.php?diff=" +
                                                                  QString::number(this->CurrentEdit->Diff)).toUtf8()));
        else
            QDesktopServices::openUrl(QUrl::fromEncoded(QString(Configuration::GetProjectScriptURL(this->GetCurrentWikiSite()) + "index.php?diff=" +
                                                                QString::number(this->CurrentEdit->RevID)).toUtf8()));
    }
}

void MainWindow::on_actionIncrease_badness_score_by_20_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_INCREASE_BS))
        return;
    this->IncreaseBS();
}

void MainWindow::on_actionDecrease_badness_score_by_20_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_DECREASE_BS))
        return;
    this->DecreaseBS();
}

void MainWindow::on_actionGood_edit_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_GOOD))
        return;
    this->FlagGood();
}

void MainWindow::on_actionUser_contributions_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_USER_CONTRIBUTIONS))
        return;
    if (this->CurrentEdit != nullptr)
    {
        QDesktopServices::openUrl(QUrl::fromEncoded(QString(this->ProjectURL() + "Special:Contributions/" +
                                  QUrl::toPercentEncoding(this->CurrentEdit->User->Username)).toUtf8()));
    }
}

void MainWindow::on_actionTalk_page_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_TALK))
        return;
    this->DisplayTalk();
}

void MainWindow::on_actionFlag_as_a_good_edit_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_GOOD))
        return;
    this->FlagGood();
}

void MainWindow::on_actionDisplay_this_page_in_browser_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_OPEN_IN_BROWSER))
        return;
    if (this->CurrentEdit != nullptr)
    {
        if (this->CurrentEdit->Diff > 0)
            QDesktopServices::openUrl(QUrl::fromEncoded(QString(this->WikiScriptURL() +
              "index.php?diff=" + QString::number(this->CurrentEdit->Diff)).toUtf8()));
        else if (this->CurrentEdit->RevID > 0)
            QDesktopServices::openUrl(QUrl::fromEncoded(QString(this->WikiScriptURL() +
              "index.php?diff=" + QString::number(this->CurrentEdit->RevID)).toUtf8()));
        else
            QDesktopServices::openUrl(QUrl::fromEncoded(QString(this->ProjectURL() +
                              this->CurrentEdit->Page->EncodedName()).toUtf8()));
    }
}

void MainWindow::on_actionEdit_page_in_browser_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_EDIT_IN_BROWSER))
        return;
    if (this->CurrentEdit != nullptr)
        QDesktopServices::openUrl(QUrl::fromEncoded(QString(this->ProjectURL() + this->CurrentEdit->Page->EncodedName() + "?action=edit").toUtf8()));
}

void MainWindow::on_actionDisplay_history_in_browser_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_OPEN_IN_BROWSER))
        return;
    if (this->CurrentEdit != nullptr)
        QDesktopServices::openUrl(QUrl::fromEncoded(QString(this->ProjectURL() + this->CurrentEdit->Page->EncodedName() + "?action=history").toUtf8()));
}

void MainWindow::on_actionStop_feed_triggered()
{
    Configuration::HuggleConfiguration->UserConfig->RemoveOldQueueEdits = false;
    this->ui->actionRemove_old_edits->setChecked(false);
    this->ui->actionStop_feed->setChecked(true);
}

void MainWindow::on_actionRemove_old_edits_triggered()
{
    Configuration::HuggleConfiguration->UserConfig->RemoveOldQueueEdits = true;
    this->ui->actionRemove_old_edits->setChecked(true);
    this->ui->actionStop_feed->setChecked(false);
}

void MainWindow::on_actionClear_talk_page_of_user_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_USER_CLEAR_TALK))
        return;
    if (!this->EditingChecks())
        return;
    if (!this->CurrentEdit->User->IsIP())
    {
        Syslog::HuggleLogs->Log(_l("feature-nfru"));
        return;
    }
    WikiPage *page = new WikiPage(this->CurrentEdit->User->GetTalk(), this->CurrentEdit->GetSite());
    /// \todo LOCALIZE ME
    WikiUtil::EditPage(page, this->GetCurrentWikiSite()->ProjectConfig->ClearTalkPageTemp
                       + "\n" + this->GetCurrentWikiSite()->ProjectConfig->WelcomeAnon + " ~~~~",
                       Configuration::GenerateSuffix("Cleaned old templates from talk page", this->GetCurrentWikiSite()->ProjectConfig));
    delete page;
}

void MainWindow::on_actionList_all_QGC_items_triggered()
{
    int xx=0;
    GC::gc->Lock->lock();
    while (xx<GC::gc->list.count())
    {
        Collectable *query = GC::gc->list.at(xx);
        Syslog::HuggleLogs->Log(query->DebugHgc());
        xx++;
    }
    GC::gc->Lock->unlock();
}

void MainWindow::on_actionRevert_currently_displayed_edit_warn_user_and_stay_on_page_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_REVERT))
        return;
    if (!this->EditingChecks() || !this->CheckRevertable())
        return;
    Collectable_SmartPtr<RevertQuery> result = this->Revert("", false);
    if (result != nullptr)
        this->Warn(result->GetEdit()->GetSite()->GetProjectConfig()->DefaultTemplate, result, result->GetEdit());
}

void MainWindow::on_actionRevert_currently_displayed_edit_and_stay_on_page_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_REVERT))
        return;
    if (!this->EditingChecks() || !this->CheckRevertable())
        return;
    this->Revert("", false);
}

void MainWindow::on_actionWelcome_user_2_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_WELCOME))
        return;
    this->triggerWelcome();
}

void MainWindow::on_actionReport_user_triggered()
{
    if (this->CurrentEdit == nullptr)
    {
        Syslog::HuggleLogs->ErrorLog(_l("report-no-user"));
        return;
    }
    this->DisplayReportUserWindow();
}

void MainWindow::on_actionReport_user_2_triggered()
{
    if(this->CurrentEdit == nullptr)
    {
        Syslog::HuggleLogs->ErrorLog(_l("report-no-user"));
        return;
    }
    this->DisplayReportUserWindow();
}

void MainWindow::on_actionWarning_1_triggered()
{
    this->ForceWarn(1);
}

void MainWindow::on_actionWarning_2_triggered()
{
    this->ForceWarn(2);
}

void MainWindow::on_actionWarning_3_triggered()
{
    this->ForceWarn(3);
}

void MainWindow::on_actionWarning_4_triggered()
{
    this->ForceWarn(4);
}

void MainWindow::on_actionEdit_user_talk_triggered()
{
    if (this->CurrentEdit != nullptr)
        QDesktopServices::openUrl(QUrl::fromEncoded(QString(this->ProjectURL() + QUrl::toPercentEncoding(this->CurrentEdit->User->GetTalk()) +
                                                            "?action=edit").toUtf8()));
}

void MainWindow::on_actionRequest_speedy_deletion_triggered()
{
    this->RequestPD();
}

void MainWindow::on_actionDelete_triggered()
{
    this->DeletePage();
}

void MainWindow::on_actionBlock_user_triggered()
{
    this->BlockUser();
}

void MainWindow::on_actionIRC_triggered()
{
    WikiSite *site = this->GetCurrentWikiSite();
    this->ChangeProvider(site, HUGGLE_FEED_PROVIDER_IRC);
}

void MainWindow::on_actionWiki_triggered()
{
    if (!this->CheckExit())
        return;
    Syslog::HuggleLogs->Log(_l("irc-switch-rc"));
    this->GetCurrentWikiSite()->Provider->Stop();
    this->ui->actionIRC->setChecked(false);
    this->ui->actionWiki->setChecked(true);
    while (!this->GetCurrentWikiSite()->Provider->IsStopped())
    {
        Syslog::HuggleLogs->Log(_l("irc-stop", this->GetCurrentWikiSite()->Name));
        Sleeper::usleep(200000);
    }
    this->ChangeProvider(this->GetCurrentWikiSite(), HUGGLE_FEED_PROVIDER_WIKI);
    this->GetCurrentWikiSite()->Provider->Start();
}

void MainWindow::on_actionShow_talk_triggered()
{
    this->LockPage();
    // we switch this to false so that in case we have received a message,
    // before we display the talk page, it get marked as read
    if (Configuration::HuggleConfiguration->NewMessage)
    {
        Configuration::HuggleConfiguration->NewMessage = false;
        ApiQuery *query = new ApiQuery(ActionClearHasMsg, this->GetCurrentWikiSite());
        query->IncRef();
        query->Target = "Flagging new messages as read";
        HUGGLE_QP_APPEND(query);
        query->Process();
        query->DecRef();
    }
    this->RenderPage("User_talk:" + Configuration::HuggleConfiguration->SystemConfig_UserName);
}

void MainWindow::on_actionProtect_triggered()
{
    if (!this->CheckExit() || !this->CheckEditableBrowserPage())
        return;
    if (this->CurrentEdit == nullptr)
    {
        // This doesn't need to be localized
        Syslog::HuggleLogs->ErrorLog("Cannot protect nullptr page");
        return;
    }
    delete this->fProtectForm;
    this->fProtectForm = new ProtectPage(this);
    this->fProtectForm->setPageToProtect(this->CurrentEdit->Page);
    this->fProtectForm->show();
}

void MainWindow::on_actionEdit_info_triggered()
{
    // don't localize this please
    Syslog::HuggleLogs->Log("Current number of edits in memory: " + QString::number(WikiEdit::EditList.count()));
}

void MainWindow::on_actionFlag_as_suspicious_edit_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_SUSPICIOUS_EDIT))
        return;
    this->SuspiciousEdit();
}

void MainWindow::on_actionDisconnect_triggered()
{
    this->VandalDock->Disconnect();
}

void MainWindow::on_actionReport_username_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_USER_REPORT_USER_NAME))
        return;
    if (this->CurrentEdit == nullptr || !this->CheckExit() || !this->CheckEditableBrowserPage())
        return;
    if (!this->GetCurrentWikiSite()->ProjectConfig->UAAavailable)
    {
        UiGeneric::pMessageBox(this, _l("uaa-not-supported"), _l("uaa-not-supported-text"), MessageBoxStyleWarning, true);
        return;
    }
    if (this->CurrentEdit->User->IsIP())
    {
        Syslog::HuggleLogs->ErrorLog(_l("main-report-ip-not-supported"));
        return;
    }
    delete this->fUaaReportForm;
    this->fUaaReportForm = new UAAReport(this);
    this->fUaaReportForm->SetUserForUAA(this->CurrentEdit->User);
    this->fUaaReportForm->show();
}

void MainWindow::on_actionShow_list_of_score_words_triggered()
{
    delete this->fScoreWord;
    this->fScoreWord = new ScoreWordsDbForm(this);
    this->fScoreWord->show();
}

void MainWindow::on_actionRevert_AGF_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_REVERT_AGF_ONE_REV))
        return;

    this->RevertAgf(false);
}

void MainWindow::on_actionDisplay_a_session_data_triggered()
{
    delete this->fSessionData;
    this->fSessionData = new SessionForm(this);
    this->fSessionData->show();
}

void MainWindow::on_actionDisplay_whitelist_triggered()
{
    delete this->fWhitelist;
    this->fWhitelist = new WhitelistForm(this);
    this->fWhitelist->show();
}

void MainWindow::on_actionResort_queue_triggered()
{
    this->Queue1->Sort();
}

void MainWindow::on_actionRestore_this_revision_triggered()
{
    if (!this->EditingChecks())
        return;
    if (this->RestoreEdit != nullptr || this->RestoreQuery != nullptr)
    {
        Huggle::Syslog::HuggleLogs->Log(_l("main-restoring-slap"));
        return;
    }
    bool ok;
    QString reason = QInputDialog::getText(this, _l("reason"), _l("main-revert-custom-reson"), QLineEdit::Normal,
                                                 _l("main-no-reason"), &ok);
    if (!ok)
        return;
    this->RestoreQuery = new ApiQuery(ActionQuery, this->GetCurrentWikiSite());
    this->RestoreQuery->Parameters = "prop=revisions&revids=" +
            QString::number(this->CurrentEdit->RevID) + "&rvprop=" +
            QUrl::toPercentEncoding("ids|content");
    this->RestoreQuery->Process();
    this->RestoreEdit = this->CurrentEdit;
    this->RestoreEdit_RevertReason = reason;
    Syslog::HuggleLogs->Log(_l("main-log1", this->CurrentEdit->Page->PageName));
}

void MainWindow::on_actionClear_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_CLEAR_QUEUE))
        return;
    this->Queue1->Clear();
}

void MainWindow::on_actionDelete_page_triggered()
{
    if (this->GetCurrentWikiSite()->GetProjectConfig()->Rights.contains("delete"))
    {
        this->DeletePage();
    } else
    {
        this->RequestPD();
    }
}

void MainWindow::on_actionBlock_user_2_triggered()
{
    this->BlockUser();
}

void MainWindow::on_actionDisplay_talk_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_TALK))
        return;
    this->DisplayTalk();
}

void MainWindow::TimerCheckTPOnTick()
{
    if (Configuration::HuggleConfiguration->DeveloperMode || this->ShuttingDown)
    {
        this->tCheck->stop();
        return;
    }
    if (!Configuration::HuggleConfiguration->UserConfig->CheckTP)
        return;
    if (this->qTalkPage == nullptr)
    {
        //! \todo Check this for every site we are logged to
        this->qTalkPage = new ApiQuery(ActionQuery, this->GetCurrentWikiSite());
        this->qTalkPage->Parameters = "meta=userinfo&uiprop=hasmsg";
        this->qTalkPage->Process();
        return;
    } else
    {
        if (!this->qTalkPage->IsProcessed())
        {
            // we are still waiting for api query to finish
            return;
        }
        QDomDocument d;
        d.setContent(this->qTalkPage->Result->Data);
        QDomNodeList page = d.elementsByTagName("userinfo");
        if (page.count() > 0)
        {
            QDomElement _e = page.at(0).toElement();
            if (_e.attributes().contains("messages"))
            {
                Configuration::HuggleConfiguration->NewMessage = true;
            } else
            {
                Configuration::HuggleConfiguration->NewMessage = false;
            }
        }
        this->qTalkPage = nullptr;
    }
}

void MainWindow::on_actionSimulate_message_triggered()
{
    Configuration::HuggleConfiguration->NewMessage = true;
}

void MainWindow::on_actionHtml_dump_triggered()
{
    QString name = "huggleDump.html";
    QFile *f = new QFile(name);
    if (!f->open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        Syslog::HuggleLogs->ErrorLog("Unable to write to " + name);
        delete f;
        return;
    }
    f->write(this->Browser->RetrieveHtml().toUtf8());
    f->close();
    Syslog::HuggleLogs->Log("Current page dumped as " + QFileInfo(f->fileName()).absoluteFilePath());
    delete f;
    QDesktopServices::openUrl(QDir().absoluteFilePath(name));
}

void MainWindow::on_actionEnforce_sysop_rights_triggered()
{
    ProjectConfiguration *conf = this->GetCurrentWikiSite()->GetProjectConfig();
    if (!conf->Rights.contains("delete"))
        conf->Rights.append("delete");
    if (!conf->Rights.contains("protect"))
        conf->Rights.append("protect");
    if (!conf->Rights.contains("block"))
        conf->Rights.append("block");
    this->ui->actionBlock_user->setEnabled(true);
    this->ui->actionBlock_user_2->setEnabled(true);
    this->ui->actionDelete_page->setEnabled(true);
    this->ui->actionDelete->setEnabled(true);
    this->ui->actionProtect->setEnabled(true);
    this->Localize();
}

void MainWindow::on_actionFeedback_triggered()
{
    QString feedback = Configuration::HuggleConfiguration->ProjectConfig->Feedback;
    if (feedback.isEmpty())
        feedback = Configuration::HuggleConfiguration->GlobalConfig_FeedbackPath;
    QDesktopServices::openUrl(feedback);
}

void MainWindow::on_actionConnect_triggered()
{
    this->VandalDock->Connect();
}

void MainWindow::on_actionDisplay_user_data_triggered()
{
    Configuration::HuggleConfiguration->UserConfig->HAN_DisplayUser = this->ui->actionDisplay_user_data->isChecked();
}

void MainWindow::on_actionDisplay_user_messages_triggered()
{
    Configuration::HuggleConfiguration->UserConfig->HAN_DisplayUserTalk = this->ui->actionDisplay_user_messages->isChecked();
}

void MainWindow::on_actionDisplay_bot_data_triggered()
{
    Configuration::HuggleConfiguration->UserConfig->HAN_DisplayBots = this->ui->actionDisplay_bot_data->isChecked();
}

void MainWindow::on_actionRequest_protection_triggered()
{
    if (!this->EditingChecks())
        return;
    if (!this->GetCurrentWikiSite()->GetProjectConfig()->RFPP)
    {
        Syslog::HuggleLogs->ErrorLog(_l("protect-request-proj-fail"));
    }
    delete this->fRFProtection;
    this->fRFProtection = new RequestProtect(this->CurrentEdit->Page, this);
    this->fRFProtection->show();
}

void MainWindow::on_actionRemove_edits_made_by_whitelisted_users_triggered()
{
    // the number must be higher than the real score so that we match even the edits
    // which have the same score (-800 + 1) > (-800)
    this->Queue1->DeleteByScore(this->GetCurrentWikiSite()->GetProjectConfig()->WhitelistScore + 1);
}

void MainWindow::on_actionDelete_all_edits_with_score_lower_than_200_triggered()
{
    this->Queue1->DeleteByScore(-200);
}

void MainWindow::on_actionRelog_triggered()
{
    ReloginForm *form = new ReloginForm(this->GetCurrentWikiSite(), this);
    form->exec();
    delete form;
}

void MainWindow::on_actionAbort_2_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_ESC))
        return;
    if (!this->RevertStack.count())
    {
        Syslog::HuggleLogs->ErrorLog(_l("main-abort-nothing-to-stop"));
        return;
    }
    if (Configuration::HuggleConfiguration->SystemConfig_InstantReverts)
    {
        Syslog::HuggleLogs->ErrorLog(_l("main-abort-instant-reverts-enabled"));
        return;
    }
    // we cancel the latest revert query that is waiting in a stack
    RevertQuery *revert_query = this->RevertStack.last();
    revert_query->Kill();
    this->RevertStack.removeLast();
    revert_query->DecRef();
}

void MainWindow::on_actionDryMode_triggered()
{
    Configuration::HuggleConfiguration->SystemConfig_DryMode = this->ui->actionDryMode->isChecked();
}

void MainWindow::on_actionDisplay_this_page_triggered()
{
    if (!this->CheckExit() || this->CurrentEdit == nullptr)
        return;
    this->Browser->DisplayPreFormattedPage(this->CurrentEdit->Page);
    this->LockPage();
}

void MainWindow::on_actionResume_provider_triggered()
{
    this->ui->actionResume_provider->setVisible(false);
    this->ui->actionStop_provider->setVisible(true);
    this->ResumeQueue();
}

void MainWindow::on_actionStop_provider_triggered()
{
    this->PauseQueue();
    this->ui->actionResume_provider->setVisible(true);
    this->ui->actionStop_provider->setVisible(false);
}

void MainWindow::on_actionRevert_only_this_revision_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_REVERT_THIS))
        return;
    if (this->EditingChecks())
        this->Revert("", true, true);
}

void MainWindow::on_actionTag_2_triggered()
{
    if (!this->CheckEditableBrowserPage())
        return;

    delete this->fWikiPageTags;
    this->fWikiPageTags = new WikiPageTagsForm(this, this->CurrentEdit->Page);
    this->fWikiPageTags->show();
}

void MainWindow::on_actionReload_menus_triggered()
{
    this->ReloadInterface();
}

void MainWindow::SetProviderIRC()
{
    QAction *action = reinterpret_cast<QAction*>(QObject::sender());
    if (!this->ActionSites.contains(action))
        throw new Huggle::Exception("There is no such a site in hash table", BOOST_CURRENT_FUNCTION);
    WikiSite *wiki = this->ActionSites[action];
    this->ChangeProvider(wiki, HUGGLE_FEED_PROVIDER_IRC);
}

void MainWindow::SetProviderWiki()
{
    QAction *action = reinterpret_cast<QAction*>(QObject::sender());
    if (!this->ActionSites.contains(action))
        throw new Huggle::Exception("There is no such a site in hash table", BOOST_CURRENT_FUNCTION);
    WikiSite *wiki = this->ActionSites[action];
    this->ChangeProvider(wiki, HUGGLE_FEED_PROVIDER_WIKI);
}

void MainWindow::SetProviderXml()
{
    QAction *action = reinterpret_cast<QAction*>(QObject::sender());
    if (!this->ActionSites.contains(action))
        throw new Huggle::Exception("There is no such a site in hash table", BOOST_CURRENT_FUNCTION);
    WikiSite *wiki = this->ActionSites[action];
    this->ChangeProvider(wiki, HUGGLE_FEED_PROVIDER_XMLRPC);
}

void MainWindow::on_actionInsert_page_to_a_watchlist_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_WATCH))
        return;

    if (!this->CheckEditableBrowserPage())
        return;

    if (this->CurrentEdit->Page == nullptr)
        throw new Huggle::NullPointerException("local CurrentEdit->Page", BOOST_CURRENT_FUNCTION);
    WikiUtil::Watchlist(this->CurrentEdit->Page);
}

void MainWindow::on_actionRemove_page_from_a_watchlist_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_UNWATCH))
        return;

    if (!this->CheckEditableBrowserPage())
        return;

    if (this->CurrentEdit->Page == nullptr)
        throw new Huggle::NullPointerException("local CurrentEdit->Page", BOOST_CURRENT_FUNCTION);
    WikiUtil::Unwatchlist(this->CurrentEdit->Page);
}

void MainWindow::on_actionMy_talk_page_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_TALK))
        return;
    if (Configuration::HuggleConfiguration->DeveloperMode)
        return;
    this->LockPage();
    this->Browser->DisplayPage(Configuration::GetProjectWikiURL(this->GetCurrentWikiSite()) + "User_talk:" +
                                           QUrl::toPercentEncoding(hcfg->SystemConfig_UserName));
}

void MainWindow::on_actionMy_Contributions_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_CONTRIB_BROWSER))
        return;
    if (Configuration::HuggleConfiguration->DeveloperMode)
        return;
    this->LockPage();
    this->Browser->DisplayPage(Configuration::GetProjectWikiURL(this->GetCurrentWikiSite()) + "Special:Contributions/" +
                                    QUrl::toPercentEncoding(hcfg->SystemConfig_UserName));
}

void MainWindow::Go()
{
    QAction *action = reinterpret_cast<QAction*>(QObject::sender());
    QDesktopServices::openUrl(QString(Configuration::GetProjectWikiURL() + action->toolTip()));
}

void MainWindow::on_actionRevert_only_this_revision_assuming_good_faith_triggered()
{
    this->RevertAgf(true);
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    int in = this->ui->tabWidget->count() - 1;
    if (index == in)
    {
        // we need to create a new browser window
        this->createBrowserTab("New tab", in);
        this->CurrentEdit = nullptr;
        this->LockPage();
    } else
    {
        this->Browser = reinterpret_cast<HuggleWeb*>(this->ui->tabWidget->widget(index)->layout()->itemAt(0)->widget());
        if (!this->Browser)
            throw new Huggle::Exception("Invalid browser pointer", BOOST_CURRENT_FUNCTION);

        // we need to change edit to what we have in that tab including all other stuff
        this->CurrentEdit = this->Browser->CurrentEdit;
        this->Render();
    }
}

void MainWindow::on_actionClose_current_tab_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_CLOSE_TAB))
        return;
    this->closeTab(this->ui->tabWidget->currentIndex());
}

void MainWindow::on_actionOpen_new_tab_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_CREATE_NEW_TAB))
        return;
    this->createBrowserTab("New tab", this->ui->tabWidget->count() - 1);
    this->CurrentEdit = nullptr;
    this->LockPage();
}

void MainWindow::on_actionVerbosity_2_triggered()
{
    hcfg->Verbosity++;
}

void MainWindow::on_actionVerbosity_triggered()
{
    if (hcfg->Verbosity > 0)
        hcfg->Verbosity--;
}

static void FinishLogout(Query *query)
{
    Configuration::Logout(reinterpret_cast<WikiSite*>(query->CallbackOwner));
    query->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
}

void MainWindow::on_actionLog_out_triggered()
{
    ApiQuery *qx = new ApiQuery(ActionLogout, this->GetCurrentWikiSite());
    qx->CallbackOwner = this->GetCurrentWikiSite();
    qx->SuccessCallback = reinterpret_cast<Callback>(FinishLogout);
    HUGGLE_QP_APPEND(qx);
    qx->Process();
}

void MainWindow::on_actionReload_tokens_triggered()
{
    WikiUtil::RetrieveTokens(this->GetCurrentWikiSite());
}

void MainWindow::on_actionXmlRcs_triggered()
{
    WikiSite *site = this->GetCurrentWikiSite();
    this->ChangeProvider(site, HUGGLE_FEED_PROVIDER_XMLRPC);
}

void MainWindow::OnStatusBarRefreshTimerTick()
{
    this->UpdateStatusBarData();
}

void MainWindow::on_actionQueue_legend_triggered()
{
    QueueHelp *w = new QueueHelp(this);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
}

void MainWindow::on_actionPatrol_triggered()
{
    if (!this->EditingChecks())
        return;

    if (!this->CurrentEdit->GetSite()->GetProjectConfig()->Patrolling)
    {
        Syslog::HuggleLogs->ErrorLog(_l("main-patrol-not-enabled", this->CurrentEdit->GetSite()->Name));
        return;
    }

    this->PatrolEdit();
}

void MainWindow::on_actionFinal_triggered()
{
    this->ForceWarn(0);
}

void MainWindow::on_actionPrint_API_for_diff_triggered()
{
    if (this->CurrentEdit == nullptr)
        return;
    if (!this->CurrentEdit->PropertyBag.contains("debug_api_url_diff"))
        return;
    HUGGLE_DEBUG1(this->CurrentEdit->PropertyBag["debug_api_url_diff"].toString());
}

void MainWindow::on_actionContribution_browser_triggered()
{
    if (this->CurrentEdit == nullptr)
        return;

    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_USER_CONTRIBUTIONS))
        return;

    UiGeneric::DisplayContributionBrowser(this->CurrentEdit->User, this);
}

void MainWindow::on_actionCheck_for_dups_triggered()
{
    QHash<QString, int> occurences;
    foreach (HuggleQueueItemLabel *e, this->Queue1->Items)
    {
        QString page = e->Edit->Page->PageName.toLower();
        if (!occurences.contains(page))
        {
            occurences.insert(page, 1);
        } else
        {
            occurences[page]++;
        }
    }
    bool found = false;
    foreach (QString page, occurences.keys())
    {
        if (occurences[page] > 1)
        {
            HUGGLE_WARNING("Multiple occurences found for " + page + ": " + QString::number(occurences[page]));
            found = true;
        }
    }
    if (!found)
    {
        HUGGLE_LOG("No duplicates found");
    }
}

void MainWindow::on_actionIntroduction_triggered()
{
    WelcomeInfo *w = new WelcomeInfo(this);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->DisableFirst();
    w->show();
}

void MainWindow::on_tabWidget_customContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos);
    // finish me
}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    this->closeTab(index);
}

void MainWindow::closeTab(int tab)
{
    if (tab == (this->ui->tabWidget->count() - 1))
    {
        // Someone wants to close + tab
        // ignore that
        return;
    }
    if (this->Browsers.count() < 2)
    {
        Syslog::HuggleLogs->ErrorLog(_l("main-browser-lasttab"));
        return;
    }

    // Get the pointer to web browser
    HuggleWeb *browser_to_close = reinterpret_cast<HuggleWeb*>(this->ui->tabWidget->widget(tab)->layout()->itemAt(0)->widget());

    if (browser_to_close == this->Browser)
    {
        // We want to close the browser that is currently open, which means, we need to switch to another one, most easy, although a bit ugly
        // is to switch to tab with id 0, because that guarantees it's not the last tab, opening last tab would be a problem because that one
        // opens a new tab
        this->ui->tabWidget->setCurrentIndex(0);
    }

    this->ui->tabWidget->removeTab(tab);

    // Now we need to find and remove the browser object, we got the pointer but we don't know the actual position in list, which doesn't need
    // to be same as position of tab
    int index = 0;
    while (index < this->Browsers.count())
    {
        if (this->Browsers.at(index) == browser_to_close)
        {
            this->Browsers.removeAt(index);
            break;
        }
        index++;
    }
    delete browser_to_close;
    if (this->Browsers.count() < 2)
    {
        // Since we closed last closable tab, we can disable the X buttons
        this->ui->tabWidget->setTabsClosable(false);
    }
}

void MainWindow::on_actionRevert_edit_using_custom_reason_triggered()
{
    if (!this->EditingChecks())
        return;
    bool ok;
    QString reason = QInputDialog::getText(this, _l("main-custom-reason-title"), _l("main-custom-reason-text"), QLineEdit::Normal,
                                           this->GetCurrentWikiSite()->GetProjectConfig()->DefaultSummary, &ok);
    if (!ok)
        return;
    if (reason.isEmpty())
    {
        Syslog::HuggleLogs->ErrorLog(_l("main-custom-reason-fail"));
        return;
    }
    this->Revert(reason);
}

void MainWindow::on_actionRefresh_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_REFRESH))
        return;
    this->RefreshPage();
}

void MainWindow::on_actionUser_page_triggered()
{
    if (this->CurrentEdit == nullptr)
        return;

    WikiPage *page = new WikiPage(this->CurrentEdit->User->GetUserPage(), this->CurrentEdit->GetSite());
    this->Browser->DisplayPreFormattedPage(page);
    this->LockPage();
    delete page;
}

void MainWindow::on_actionShow_score_debug_triggered()
{
    if (this->CurrentEdit == nullptr)
        return;

    QString debug_info = this->CurrentEdit->Page->PageName + ": " + QString::number(this->CurrentEdit->Score) + " ";

    foreach (QString key, this->CurrentEdit->PropertyBag.keys())
    {
        if (key.startsWith("score_") && this->CurrentEdit->PropertyBag[key].toLongLong() != 0)
            debug_info += QString(key).replace("score_", "") + ": " + QString::number(this->CurrentEdit->PropertyBag[key].toLongLong()) + ", ";
    }

    if (debug_info.endsWith(", "))
        debug_info = debug_info.mid(0, debug_info.size() - 2);

    HUGGLE_DEBUG1(debug_info);
}

void MainWindow::on_actionPost_a_custom_message_triggered()
{
    if (!this->CheckExit() || !this->CheckEditableBrowserPage())
        return;

    if (this->CurrentEdit == nullptr)
    {
        Syslog::HuggleLogs->ErrorLog(_l("custommessage-none"));
        return;
    }

    CustomMessage *cm = new CustomMessage(this->CurrentEdit->User, this);
    cm->show();
}

void MainWindow::on_actionWrite_text_to_HAN_triggered()
{
    this->VandalDock->WriteTest(this->CurrentEdit);
}

void MainWindow::OnWikiUserUpdate(WikiUser *user)
{
    this->Queue1->UpdateUser(user);
}

void MainWindow::OnWikiEditHist(HistoryItem *item)
{
    this->_History->Prepend(item);
}

void MainWindow::OnReport(WikiUser *user)
{
    this->DisplayReportUserWindow(user);
}

void MainWindow::OnSReport(WikiUser *user)
{
    ReportUser::SilentReport(user);
}

void MainWindow::OnMessage(QString title, QString text)
{
    UiGeneric::MessageBox(title, text);
}

void MainWindow::OnWarning(QString title, QString text)
{
    UiGeneric::MessageBox(title, text, MessageBoxStyleWarning);
}

void MainWindow::OnQuestion(QString title, QString text, bool *y)
{
    int result = UiGeneric::MessageBox(title, text, MessageBoxStyleQuestion);
    if (result == QMessageBox::Yes)
        *y = true;
    else
        *y = false;
}

void MainWindow::OnError(QString title, QString text)
{
    UiGeneric::MessageBox(title, text, MessageBoxStyleError);
}

void MainWindow::on_actionThrow_triggered()
{
    throw new Huggle::Exception("Test exception (from menu)", BOOST_CURRENT_FUNCTION);
}

void MainWindow::on_actionWelcome_page_triggered()
{
    this->DisplayWelcomeMessage();
}

void MainWindow::on_actionScripts_manager_triggered()
{
    delete this->fScripting;
    this->fScripting = new ScriptingManager(this);
    this->fScripting->exec();
}

void MainWindow::OnFinishPreProcess(WikiEdit *ed)
{
    // In case we are currently looking at this page in main window, let's refresh
    if (hcfg->UserConfig->AutomaticRefresh && this->CurrentEdit != nullptr && ed->Page->SanitizedName() == this->CurrentEdit->Page->SanitizedName())
        this->RefreshPage();
}

void MainWindow::on_actionFind_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_FIND))
        return;
    this->Browser->ToggleSearchWidget();
}

void MainWindow::on_actionEdit_page_triggered()
{
    if (!this->keystrokeCheck(HUGGLE_ACCEL_MAIN_EDIT))
        return;
    if (!this->CheckExit() || this->CurrentEdit == nullptr)
        return;

    EditForm *ef = new EditForm(this->CurrentEdit->Page, this);
    ef->setAttribute(Qt::WA_DeleteOnClose);
    ef->show();
}

void MainWindow::on_actionProfiler_info_triggered()
{
    Core::HuggleCore->WriteProfilerDataIntoSyslog();
}

void Huggle::MainWindow::on_actionCopy_system_log_to_clipboard_triggered()
{
    QGuiApplication::clipboard()->setText(Huggle::Syslog::HuggleLogs->RingLogToText());
}

void Huggle::MainWindow::on_actionDisplay_revid_triggered()
{
    bool ok;
    int revid = QInputDialog::getInt(this, "Display revid", "Revision ID:", 947919407, 1, 2147483647, 1, &ok);
    if (!ok)
        return;
    this->DisplayRevid(revid, this->GetCurrentWikiSite());
}

void Huggle::MainWindow::on_actionPlay_sound_triggered()
{
    Resources::PlayEmbeddedSoundFile("not1.wav");
}
