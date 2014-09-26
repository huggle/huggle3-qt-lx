//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "mainwindow.hpp"
#include <QDesktopServices>
#include <QMessageBox>
#include <QToolButton>
#include <QInputDialog>
#include <QMutex>
#include <QMenu>
#include <QLabel>
#include <QTimer>
#include <QThread>
#include <QVBoxLayout>
#include <QSplitter>
#include <QDockWidget>
#include "aboutform.hpp"
#include "configuration.hpp"
#include "editbar.hpp"
#include "reloginform.hpp"
#include "generic.hpp"
#include "gc.hpp"
#include "querypool.hpp"
#include "hooks.hpp"
#include "history.hpp"
#include "hugglefeedproviderwiki.hpp"
#include "hugglefeedproviderirc.hpp"
#include "hugglelog.hpp"
#include "huggleparser.hpp"
#include "huggleprofiler.hpp"
#include "hugglequeue.hpp"
#include "huggletool.hpp"
#include "huggleweb.hpp"
#include "blockuser.hpp"
#include "deleteform.hpp"
#include "wikipage.hpp"
#include "preferences.hpp"
#include "processlist.hpp"
#include "protectpage.hpp"
#include "reloginform.hpp"
#include "reportuser.hpp"
#include "collectable.hpp"
#include "core.hpp"
#include "wikiutil.hpp"
#include "exception.hpp"
#include "localization.hpp"
#include "syslog.hpp"
#include "sleeper.hpp"
#include "wikiuser.hpp"
#include "wikisite.hpp"
#include "ignorelist.hpp"
#include "speedyform.hpp"
#include "userinfoform.hpp"
#include "vandalnw.hpp"
#include "whitelistform.hpp"
#include "sessionform.hpp"
#include "historyform.hpp"
#include "scorewordsdbform.hpp"
#include "warnings.hpp"
#include "warninglist.hpp"
#include "waitingform.hpp"
#include "wikipagetagsform.hpp"
#include "uaareport.hpp"
#include "ui_mainwindow.h"
#include "requestprotect.hpp"

using namespace Huggle;
MainWindow *MainWindow::HuggleMain = nullptr;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    HUGGLE_PROFILER_RESET;
    this->LastTPRevID = WIKI_UNKNOWN_REVID;
    this->EditLoad = QDateTime::currentDateTime();
    this->Shutdown = ShutdownOpRunning;
    this->EditablePage = false;
    this->ShuttingDown = false;
    this->ui->setupUi(this);
    if (Configuration::HuggleConfiguration->Multiple)
    {
        this->ui->menuChange_provider->setVisible(false);
        this->ui->actionStop_provider->setVisible(false);
        this->ui->actionReconnect_IRC->setVisible(false);
        this->ui->actionIRC->setVisible(false);
        this->ui->actionWiki->setVisible(false);
    }
    this->Status = new QLabel();
    this->Status->setWordWrap(true);
    this->Status->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    this->ui->statusBar->addWidget(this->Status, 1);
    this->tb = new HuggleTool();
    this->Queries = new ProcessList(this);
    this->SystemLog = new HuggleLog(this);
    this->CreateBrowserTab("Welcome page", 0);
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
    this->preferencesForm = new Preferences(this);
    this->aboutForm = new AboutForm(this);
    this->ui->actionDisplay_bot_data->setChecked(Configuration::HuggleConfiguration->UserConfig->HAN_DisplayBots);
    this->ui->actionDisplay_user_data->setChecked(Configuration::HuggleConfiguration->UserConfig->HAN_DisplayUser);
    this->ui->actionDisplay_user_messages->setChecked(Configuration::HuggleConfiguration->UserConfig->HAN_DisplayUserTalk);
    // we store the value in bool so that we don't need to call expensive string function twice
    bool PermissionBlock = Configuration::HuggleConfiguration->ProjectConfig->Rights.contains("block");
    this->ui->actionBlock_user->setEnabled(PermissionBlock);
    this->ui->actionBlock_user_2->setEnabled(PermissionBlock);
    bool PermissionDelete = Configuration::HuggleConfiguration->ProjectConfig->Rights.contains("delete");
    this->ui->actionDelete->setEnabled(PermissionDelete);
    this->ui->actionProtect->setEnabled(Configuration::HuggleConfiguration->ProjectConfig->Rights.contains("protect"));
    this->addDockWidget(Qt::LeftDockWidgetArea, this->_History);
    this->SystemLog->resize(100, 80);
    foreach (WikiSite *site, Configuration::HuggleConfiguration->Projects)
    {
        if (!site->GetProjectConfig()->WhiteList.contains(Configuration::HuggleConfiguration->SystemConfig_Username))
            site->GetProjectConfig()->WhiteList.append(Configuration::HuggleConfiguration->SystemConfig_Username);
    }
    QueryPool::HugglePool->Processes = this->Queries;
    QString projects = Configuration::HuggleConfiguration->Project->Name;
    if (Configuration::HuggleConfiguration->Multiple)
    {
        projects = "multiple projects (";
        foreach (WikiSite *site, Configuration::HuggleConfiguration->Projects)
            projects += site->Name + ", ";
        projects = projects.mid(0, projects.length() - 2);
        projects += ")";
    }
    this->setWindowTitle("Huggle 3 QT-LX on " + projects);
    HUGGLE_PROFILER_PRINT_TIME("MainWindow::MainWindow(QWidget *parent)@layout");
    this->DisplayWelcomeMessage();
    HUGGLE_PROFILER_PRINT_TIME("MainWindow::MainWindow(QWidget *parent)@welcome");
    if (Configuration::HuggleConfiguration->UserConfig->RemoveOldQueueEdits)
    {
        this->ui->actionRemove_old_edits->setChecked(true);
        this->ui->actionStop_feed->setChecked(false);
    } else
    {
        this->ui->actionRemove_old_edits->setChecked(false);
        this->ui->actionStop_feed->setChecked(true);
    }
    // initialise queues
    if (!Configuration::HuggleConfiguration->ProjectConfig->UseIrc)
    {
        Syslog::HuggleLogs->Log(_l("irc-not"));
        this->ui->actionReconnect_IRC->setEnabled(false);
        this->ui->actionIRC->setEnabled(false);
    }
    foreach (WikiSite *site, Configuration::HuggleConfiguration->Projects)
    {
        bool irc = false;
        if (Configuration::HuggleConfiguration->UsingIRC && site->ProjectConfig->UseIrc)
        {
            this->ChangeProvider(site, new HuggleFeedProviderIRC(site));
            this->ui->actionIRC->setChecked(true);
            irc = true;
            if (!site->Provider->Start())
            {
                Syslog::HuggleLogs->ErrorLog(_l("irc-failure", site->Name));
                this->ui->actionIRC->setChecked(false);
                this->ui->actionWiki->setChecked(true);
                this->ChangeProvider(site, new HuggleFeedProviderWiki(site));
                site->Provider->Start();
                irc = false;
            }
        } else
        {
            this->ui->actionIRC->setChecked(false);
            this->ui->actionWiki->setChecked(true);
            this->ChangeProvider(site, new HuggleFeedProviderWiki(site));
            site->Provider->Start();
        }
        if (Configuration::HuggleConfiguration->Multiple)
        {
            QMenu *menu = new QMenu(site->Name, this);
            this->ui->menuChange_provider->addMenu(menu);
            QAction *irc_a = new QAction("IRC", menu);
            QAction *wik_a = new QAction("Wiki", menu);
            this->lWikis.insert(site, wik_a);
            this->lIRC.insert(site, irc_a);
            wik_a->setCheckable(true);
            irc_a->setCheckable(true);
            connect(wik_a, SIGNAL(triggered()), this, SLOT(SetProviderWiki()));
            connect(irc_a, SIGNAL(triggered()), this, SLOT(SetProviderIRC()));
            menu->addAction(irc_a);
            menu->addAction(wik_a);
            if (irc)
                irc_a->setChecked(true);
            else
                wik_a->setChecked(true);
        }
    }
    HUGGLE_PROFILER_PRINT_TIME("MainWindow::MainWindow(QWidget *parent)@providers");
    this->ReloadInterface();
    this->tabifyDockWidget(this->SystemLog, this->Queries);
    this->GeneralTimer = new QTimer(this);
    //this->ui->actionTag_2->setVisible(false);
    connect(this->GeneralTimer, SIGNAL(timeout()), this, SLOT(OnMainTimerTick()));
    this->GeneralTimer->start(HUGGLE_TIMER);
    QFile *layout;
    if (QFile().exists(Configuration::GetConfigurationPath() + "mainwindow_state"))
    {
        HUGGLE_DEBUG1("Loading state");
        layout = new QFile(Configuration::GetConfigurationPath() + "mainwindow_state");
        if (!layout->open(QIODevice::ReadOnly))
            Syslog::HuggleLogs->ErrorLog("Unable to read state from a config file");
        else if (!this->restoreState(layout->readAll()))
            HUGGLE_DEBUG1("Failed to restore state");

        layout->close();
        delete layout;
    }
    if (QFile().exists(Configuration::GetConfigurationPath() + "mainwindow_geometry"))
    {
        HUGGLE_DEBUG1("Loading geometry");
        layout = new QFile(Configuration::GetConfigurationPath() + "mainwindow_geometry");
        if (!layout->open(QIODevice::ReadOnly))
            Syslog::HuggleLogs->ErrorLog("Unable to read geometry from a config file");
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
    this->Localize();
    this->VandalDock->Connect();
    HUGGLE_PROFILER_PRINT_TIME("MainWindow::MainWindow(QWidget *parent)@irc");
    this->tCheck = new QTimer(this);
    Hooks::MainWindowIsLoaded(this);
    HUGGLE_PROFILER_PRINT_TIME("MainWindow::MainWindow(QWidget *parent)@hooks");
    connect(this->tCheck, SIGNAL(timeout()), this, SLOT(TimerCheckTPOnTick()));
    this->tCheck->start(20000);
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
    while (this->PatrolledEdits.count())
    {
        this->PatrolledEdits.at(0)->UnregisterConsumer("patrol");
        this->PatrolledEdits.removeAt(0);
    }
    while (this->Browsers.count())
    {
        delete this->Browsers.at(0);
        this->Browsers.removeAt(0);
    }
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
    delete this->RevertSummaries;
    delete this->Queries;
    delete this->preferencesForm;
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
    delete this->ui;
    delete this->tb;
}

void MainWindow::DisplayReportUserWindow(WikiUser *User)
{
    if (!this->CheckExit() || this->CurrentEdit == nullptr)
        return;

    if (Configuration::HuggleConfiguration->Restricted)
    {
        Generic::DeveloperError();
        return;
    }
    if (User == nullptr)
        User = this->CurrentEdit->User;
    if (User == nullptr)
        throw new Huggle::Exception("WikiUser must not be nullptr", "void MainWindow::DisplayReportUserWindow(WikiUser *User)");
    if (User->IsReported)
    {
        Syslog::HuggleLogs->ErrorLog(_l("report-duplicate"));
        return;
    }
    ProjectConfiguration *conf = this->GetCurrentWikiSite()->GetProjectConfig();
    // only use this if current projects support it
    if (!conf->AIV)
    {
        QMessageBox mb;
        mb.setText(_l("missing-aiv"));
        mb.setWindowTitle(_l("function-miss"));
        mb.setIcon(QMessageBox::Information);
        mb.exec();
        return;
    }
    if (this->fReportForm != nullptr)
    {
        delete this->fReportForm;
        this->fReportForm = nullptr;
    }
    this->fReportForm = new ReportUser(this);
    this->fReportForm->show();
    this->fReportForm->SetUser(User);
}

void MainWindow::ProcessEdit(WikiEdit *e, bool IgnoreHistory, bool KeepHistory, bool KeepUser, bool ForcedJump)
{
    if (e == nullptr || this->ShuttingDown)
    {
        // Huggle is either shutting down or edit is nullptr so we can't do anything here
        return;
    }
    if (this->qNext != nullptr)
    {
        // we need to delete this because it's related to an old edit
        this->qNext.Delete();
    }
    if (e->Page == nullptr || e->User == nullptr)
    {
        throw new Huggle::Exception("Page and User must not be nullptr in edit that is supposed to be displayed on form",
                  "void MainWindow::ProcessEdit(WikiEdit *e, bool IgnoreHistory, bool KeepHistory, bool KeepUser)");
    }
    if (this->OnNext_EvPage != nullptr)
    {
        delete this->OnNext_EvPage;
        this->OnNext_EvPage = nullptr;
    }
    // we need to safely delete the edit later
    e->IncRef();
    // if there are actually some totaly old edits in history that we need to delete
    while (this->Historical.count() > Configuration::HuggleConfiguration->SystemConfig_HistorySize)
    {
        WikiEdit *prev = this->Historical.at(0);
        if (prev == e)
            break;

        this->Historical.removeAt(0);
        prev->RemoveFromHistoryChain();
        prev->UnregisterConsumer(HUGGLECONSUMER_MAINFORM_HISTORICAL);
    }
    if (this->Historical.contains(e) == false)
    {
        e->RegisterConsumer(HUGGLECONSUMER_MAINFORM_HISTORICAL);
        this->Historical.append(e);
        if (this->CurrentEdit != nullptr)
        {
            if (!IgnoreHistory)
            {
                e->RemoveFromHistoryChain();
                // now we need to get to last edit in chain
                WikiEdit *latest = this->CurrentEdit;
                while (latest->Next != nullptr)
                    latest = latest->Next;
                latest->Next = e;
                e->Previous = latest;
            }
        }
    }
    e->User->Resync();
    this->EditablePage = true;
    Configuration::HuggleConfiguration->ForcedNoEditJump = ForcedJump;
    this->CurrentEdit = e;
    this->EditLoad = QDateTime::currentDateTime();
    this->Browser->DisplayDiff(e);
    this->Render(KeepHistory, KeepUser);
    e->DecRef();
}

void MainWindow::Render(bool KeepHistory, bool KeepUser)
{
    if (this->CurrentEdit != nullptr)
    {
        if (this->CurrentEdit->Page == nullptr)
            throw new Huggle::Exception("Page of CurrentEdit can't be nullptr at MainWindow::Render()");

        this->wEditBar->RemoveAll();
        if (!KeepUser)
        {
            this->wUserInfo->ChangeUser(this->CurrentEdit->User);
            if (Configuration::HuggleConfiguration->UserConfig->HistoryLoad)
                this->wUserInfo->Read();
        }
        if (!KeepHistory)
        {
            this->wHistory->Update(this->CurrentEdit);
            if (Configuration::HuggleConfiguration->UserConfig->HistoryLoad)
                this->wHistory->Read();
        }

        this->Title(this->CurrentEdit->Page->PageName);
        if (this->PreviousSite != this->GetCurrentWikiSite())
        {
            this->ReloadInterface();
            this->PreviousSite = this->GetCurrentWikiSite();
        }
        this->tb->SetTitle(this->CurrentEdit->Page->PageName);
        this->tb->SetPage(this->CurrentEdit->Page);
        this->tb->SetUser(this->CurrentEdit->User->Username);
        QString word = "";
        if (this->CurrentEdit->ScoreWords.count() != 0)
        {
            word = " words: ";
            int x = 0;
            while (x < this->CurrentEdit->ScoreWords.count())
            {
                word += this->CurrentEdit->ScoreWords.at(x) + ", ";
                x++;
            }
            if (word.endsWith(", "))
                word = word.mid(0, word.length() - 2);
        }
        QStringList params;
        params << this->CurrentEdit->Page->PageName << QString::number(this->CurrentEdit->Score) + word;
        this->tb->SetInfo(_l("browser-diff", params));
        return;
    }
    this->tb->SetTitle(this->Browser->CurrentPageName());
}

void MainWindow::RequestPD()
{
    if (!this->CheckExit() || !this->CheckEditableBrowserPage() || this->CurrentEdit == nullptr)
        return;
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Generic::DeveloperError();
        return;
    }
    if (this->fSpeedyDelete)
        delete this->fSpeedyDelete;
    this->fSpeedyDelete = new SpeedyForm();
    this->fSpeedyDelete->Init(this->CurrentEdit);
    this->fSpeedyDelete->show();
}

void MainWindow::RevertAgf(bool only)
{
    if (this->CurrentEdit == nullptr || !this->CheckExit() || !this->CheckEditableBrowserPage())
        return;
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Generic::DeveloperError();
        return;
    }
    bool ok;
    QString reason = QInputDialog::getText(this, _l("reason"), _l("main-revert-custom-reson"), QLineEdit::Normal,
                                           "No reason was provided / custom revert", &ok);
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

void MainWindow::FinishPatrols()
{
    int x = 0;
    bool flaggedrevs = hcfg->ProjectConfig->PatrollingFlaggedRevs;
    while (x < this->PatrolledEdits.count())
    {
        Collectable_SmartPtr<ApiQuery> query = this->PatrolledEdits.at(x);
        // check if this query has actually some edit associated to it
        if (query->CallbackResult == nullptr)
        {
            // we really don't want to mess up with this
            query->UnregisterConsumer("patrol");
            // get rid of it
            this->PatrolledEdits.removeAt(x);
            continue;
        }
        // check if it's done
        if (query->IsProcessed())
        {
            // wheeee now we have a token
            WikiEdit *edit = (WikiEdit*) query->CallbackResult;
            QDomDocument d;
            d.setContent(query->Result->Data);
            QDomNodeList l = d.elementsByTagName("tokens");
            if (l.count() > 0)
            {
                QDomElement element = l.at(0).toElement();
                const char *tokenname = flaggedrevs ? "edittoken" : "patroltoken";
                if (element.attributes().contains(tokenname))
                {
                    // we can finish this
                    QString token = element.attribute(tokenname);
                    edit->PatrolToken = token;
                    this->PatrolThis(edit);
                    // get rid of it
                    this->PatrolledEdits.removeAt(x);
                    edit->UnregisterConsumer("patrol");
                    query->CallbackResult = nullptr;
                    query->UnregisterConsumer("patrol");
                    continue;
                }
            }
            // this edit is fucked up
            HUGGLE_DEBUG1("Unable to retrieve token for " + edit->Page->PageName);
            // get rid of it
            this->PatrolledEdits.removeAt(x);
            edit->UnregisterConsumer("patrol");
            query->CallbackResult = nullptr;
            query->UnregisterConsumer("patrol");
            continue;
        } else
        {
            x++;
        }
    }
}

void MainWindow::UpdateStatusBarData()
{
    QStringList params;
    params << Generic::ShrinkText(QString::number(QueryPool::HugglePool->ProcessingEdits.count()), 3)
           << Generic::ShrinkText(QString::number(QueryPool::HugglePool->RunningQueriesGetCount()), 3)
           << QString::number(this->GetCurrentWikiSite()->GetProjectConfig()->WhiteList.size())
           << Generic::ShrinkText(QString::number(this->Queue1->Items.count()), 4);
    QString statistics_;
    // calculate stats, but not if huggle uptime is lower than 50 seconds
    double Uptime = this->GetCurrentWikiSite()->Provider->GetUptime();
    if (this->ShuttingDown)
    {
        statistics_ = "none";
    } else if (Uptime < 50)
    {
        statistics_ = "waiting for more edits";
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
            color = "orange";
        //! \todo LOCALIZE ME
        // make the numbers easier to read
        EditsPerMinute = ((double)qRound(EditsPerMinute * 100)) / 100;
        RevertsPerMinute = ((double)qRound(RevertsPerMinute * 100)) / 100;
        VandalismLevel = ((double)qRound(VandalismLevel * 100)) / 100;
        statistics_ = " <font color=" + color + ">" + Generic::ShrinkText(QString::number(EditsPerMinute), 6) +
             " edits per minute " + Generic::ShrinkText(QString::number(RevertsPerMinute), 6) +
             " reverts per minute, level " + Generic::ShrinkText(QString::number(VandalismLevel), 8) + "</font>";
    }
    if (Configuration::HuggleConfiguration->Verbosity > 0)
        statistics_ += " QGC: " + QString::number(GC::gc->list.count()) + " U: " + QString::number(WikiUser::ProblematicUsers.count());
    params << statistics_ << this->GetCurrentWikiSite()->Name;
    this->Status->setText(_l("main-status-bar", params));
}

bool MainWindow::EditingChecks()
{
    if (!this->CheckExit() || !this->CheckEditableBrowserPage())
        return false;

    if (Configuration::HuggleConfiguration->Restricted)
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

void MainWindow::ReloadSc()
{
    Configuration::HuggleConfiguration->ReloadOfMainformNeeded = false;
    QStringList shorts = Configuration::HuggleConfiguration->Shortcuts.keys();
    foreach (QString sh, shorts)
    {
        this->ReloadShort(sh);
    }
}

static inline void ReloadIndexedMenuShortcut(QList<QAction *> list, int item, Shortcut s)
{
    if (list.count() <= item)
        return;

    list.at(item)->setShortcut(s.QAccel);
}

void MainWindow::ReloadShort(QString id)
{
    if (!Configuration::HuggleConfiguration->Shortcuts.contains(id))
        throw new Huggle::Exception("Invalid shortcut name");
    Shortcut s = Configuration::HuggleConfiguration->Shortcuts[id];
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
            ReloadIndexedMenuShortcut(this->RevertAndWarnItems, 0, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN1:
            ReloadIndexedMenuShortcut(this->RevertAndWarnItems, 1, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN2:
            ReloadIndexedMenuShortcut(this->RevertAndWarnItems, 2, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN3:
            ReloadIndexedMenuShortcut(this->RevertAndWarnItems, 3, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN4:
            ReloadIndexedMenuShortcut(this->RevertAndWarnItems, 4, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN5:
            ReloadIndexedMenuShortcut(this->RevertAndWarnItems, 5, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN6:
            ReloadIndexedMenuShortcut(this->RevertAndWarnItems, 6, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN7:
            ReloadIndexedMenuShortcut(this->RevertAndWarnItems, 7, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN8:
            ReloadIndexedMenuShortcut(this->RevertAndWarnItems, 8, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_AND_WARN9:
            ReloadIndexedMenuShortcut(this->RevertAndWarnItems, 9, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN0:
            ReloadIndexedMenuShortcut(this->WarnItems, 0, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN1:
            ReloadIndexedMenuShortcut(this->WarnItems, 1, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN2:
            ReloadIndexedMenuShortcut(this->WarnItems, 2, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN3:
            ReloadIndexedMenuShortcut(this->WarnItems, 3, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN4:
            ReloadIndexedMenuShortcut(this->WarnItems, 4, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN5:
            ReloadIndexedMenuShortcut(this->WarnItems, 5, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN6:
            ReloadIndexedMenuShortcut(this->WarnItems, 6, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN7:
            ReloadIndexedMenuShortcut(this->WarnItems, 7, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN8:
            ReloadIndexedMenuShortcut(this->WarnItems, 8, s);
            return;
        case HUGGLE_ACCEL_MAIN_WARN9:
            ReloadIndexedMenuShortcut(this->WarnItems, 9, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_0:
            ReloadIndexedMenuShortcut(this->RevertItems, 0, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_1:
            ReloadIndexedMenuShortcut(this->RevertItems, 1, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_2:
            ReloadIndexedMenuShortcut(this->RevertItems, 2, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_3:
            ReloadIndexedMenuShortcut(this->RevertItems, 3, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_4:
            ReloadIndexedMenuShortcut(this->RevertItems, 4, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_5:
            ReloadIndexedMenuShortcut(this->RevertItems, 5, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_6:
            ReloadIndexedMenuShortcut(this->RevertItems, 6, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_7:
            ReloadIndexedMenuShortcut(this->RevertItems, 7, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_8:
            ReloadIndexedMenuShortcut(this->RevertItems, 8, s);
            return;
        case HUGGLE_ACCEL_MAIN_REVERT_9:
            ReloadIndexedMenuShortcut(this->RevertItems, 9, s);
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

Collectable_SmartPtr<RevertQuery> MainWindow::Revert(QString summary, bool next, bool single_rv)
{
    bool rollback = true;
    Collectable_SmartPtr<RevertQuery> ptr_;
    if (this->CurrentEdit == nullptr)
    {
        Syslog::HuggleLogs->ErrorLog(_l("main-revert-null"));
        return ptr_;
    }
    if (this->CurrentEdit->NewPage)
    {
        QMessageBox mb;
        mb.setWindowTitle("Can't revert this");
        mb.setText("This is a new page, so it can't be reverted, you can either tag it, or delete it.");
        mb.exec();
        return ptr_;
    }
    if (!this->CurrentEdit->IsPostProcessed())
    {
        // This shouldn't ever happen, there is no need to translate this message
        // becase it's nearly impossible to be ever displayed
        Syslog::HuggleLogs->ErrorLog("This edit is still being processed, please wait");
        return ptr_;
    }
    if (this->CurrentEdit->RollbackToken.isEmpty())
    {
        Syslog::HuggleLogs->WarningLog(_l("main-revert-manual", this->CurrentEdit->Page->PageName));
        rollback = false;
    }
    if (this->PreflightCheck(this->CurrentEdit))
    {
        this->CurrentEdit->User->Resync();
        this->CurrentEdit->User->SetBadnessScore(this->CurrentEdit->User->GetBadnessScore(false) - 10);
        Hooks::OnRevert(this->CurrentEdit);
        ptr_ = WikiUtil::RevertEdit(this->CurrentEdit, summary, false, rollback);
        if (single_rv) { ptr_->SetLast(); }
        if (Configuration::HuggleConfiguration->SystemConfig_InstantReverts)
        {
            ptr_->Process();
        } else
        {
            ptr_->Date = QDateTime::currentDateTime();
            ptr_->IncRef();
            this->RevertStack.append(ptr_);
        }
        if (next)
            this->DisplayNext(ptr_);
    }
    return ptr_;
}

bool MainWindow::PreflightCheck(WikiEdit *_e)
{
    if (this->qNext != nullptr)
    {
        QMessageBox *mb = new QMessageBox();
        mb->setWindowTitle("This edit is already being reverted");
        mb->setText("You can't revert this edit, because it's already being reverted. Please wait!");
        mb->exec();
        return false;
    }
    if (_e == nullptr)
        throw new Huggle::Exception("nullptr edit in PreflightCheck(WikiEdit *_e) is not a valid edit");
    bool Warn = false;
    QString type = "unknown";
    if (hcfg->WarnUserSpaceRoll && _e->Page->IsUserpage())
    {
        Warn = true;
        type = "in userspace";
    } else if (hcfg->ProjectConfig->ConfirmOnSelfRevs && (_e->User->Username.toLower() == hcfg->SystemConfig_Username.toLower()))
    {
        type = "made by you";
        Warn = true;
    } else if (hcfg->ProjectConfig->ConfirmTalk && _e->Page->IsTalk())
    {
        type = "made on talk page";
        Warn = true;
    } else if (hcfg->ProjectConfig->ConfirmWL && _e->User->IsWhitelisted())
    {
        type = "made by a user who is on white list";
        Warn = true;
    }
    if (Warn)
    {
        QMessageBox::StandardButton q = QMessageBox::question(nullptr, "Revert edit"
                      , "This edit is " + type + ", so even if it looks like it is a vandalism,"\
                      " it may not be, are you sure you want to revert it?"
                      , QMessageBox::Yes|QMessageBox::No);
        if (q == QMessageBox::No)
            return false;
    }
    return true;
}

bool MainWindow::Warn(QString WarningType, RevertQuery *dependency)
{
    if (this->CurrentEdit == nullptr)
        return false;
    bool Report_ = false;
    PendingWarning *ptr_Warning_ = Warnings::WarnUser(WarningType, dependency, this->CurrentEdit, &Report_);
    if (Report_)
        this->DisplayReportUserWindow(this->CurrentEdit->User);
    if (ptr_Warning_ != nullptr)
    {
        PendingWarning::PendingWarnings.append(ptr_Warning_);
        return true;
    }
    return false;
}

QString MainWindow::GetSummaryKey(QString item)
{
    // first get the configuration for the project we are on
    ProjectConfiguration *pr = this->GetCurrentWikiSite()->GetProjectConfig();
    if (item.contains(";"))
    {
        QString type = item.mid(0, item.indexOf(";"));
        int c=0;
        while(c < pr->WarningTypes.count())
        {
            QString x = pr->WarningTypes.at(c);
            if (x.startsWith(type + ";"))
            {
                x = pr->WarningTypes.at(c);
                x = x.mid(x.indexOf(";") + 1);
                if (x.endsWith(","))
                {
                    x = x.mid(0, x.length() - 1);
                }
                return x;
            }
            c++;
        }
    }
    return item;
}

void MainWindow::on_actionExit_triggered()
{
    this->Exit();
}

void MainWindow::DisplayWelcomeMessage()
{
    WikiPage *welcome = new WikiPage(hcfg->ProjectConfig->WelcomeMP);
    this->Browser->DisplayPreFormattedPage(welcome);
    this->LockPage();
    delete welcome;
    this->Render();
}

void MainWindow::FinishRestore()
{
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
            Huggle::Syslog::HuggleLogs->ErrorLog("Unable to restore the revision, because there is no text available for it");
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
            Huggle::Syslog::HuggleLogs->Log("Unable to restore the revision, because there is no text available for it");
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
        Syslog::HuggleLogs->ErrorLog("Unable to restore the revision because wiki provided no data for selected version");
    }
    this->RestoreEdit.Delete();
    this->RestoreQuery.Delete();
}

void MainWindow::CreateBrowserTab(QString name, int index)
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
}

void MainWindow::Title(QString name)
{
    this->ui->tabWidget->setTabText(this->ui->tabWidget->currentIndex(), Generic::ShrinkText(name, 20, false, 3));
}

void MainWindow::TriggerWarn()
{
    if (!this->CheckExit() || !this->CheckEditableBrowserPage())
        return;
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Generic::DeveloperError();
        return;
    }
    if (Configuration::HuggleConfiguration->UserConfig->ManualWarning)
    {
        if (this->fWarningList != nullptr)
            delete this->fWarningList;
        this->fWarningList = new Huggle::WarningList(this->CurrentEdit);
        this->fWarningList->show();
        return;
    }
    this->Warn("warning", nullptr);
}

void MainWindow::on_actionPreferences_triggered()
{
    this->preferencesForm->show();
}

void MainWindow::on_actionContents_triggered()
{
    QDesktopServices::openUrl(QUrl(Configuration::HuggleConfiguration->GlobalConfig_DocumentationPath));
}

void MainWindow::on_actionAbout_triggered()
{
    this->aboutForm->show();
}

void MainWindow::OnMainTimerTick()
{
    ProjectConfiguration *cfg = this->GetCurrentWikiSite()->GetProjectConfig();
    if (!cfg->IsLoggedIn && !cfg->RequestingLogin && !hcfg->Restricted)
    {
        delete this->fRelogin;
        // we need to flag it here so that we don't reload the form next tick
        cfg->RequestingLogin = true;
        this->fRelogin = new ReloginForm(this);
        // exec to freeze
        this->fRelogin->show();
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
    if (site->Provider->IsWorking() != true && this->ShuttingDown != true)
    {
        Syslog::HuggleLogs->Log(_l("provider-failure", this->GetCurrentWikiSite()->Name));
        if (!site->Provider->Restart())
        {
            this->ChangeProvider(site, new HuggleFeedProviderWiki(site));
            site->Provider->Start();
        }
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
            this->ResumeQueue();
            this->Queue1->Trim();
        }
    } else
    {
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
        int c = 0;
        while (c < this->PendingEdits.count())
        {
            if (this->PendingEdits.at(c)->IsPostProcessed())
            {
                WikiEdit *edit = this->PendingEdits.at(c);
                this->Queue1->AddItem(edit);
                this->PendingEdits.removeAt(c);
                edit->UnregisterConsumer(HUGGLECONSUMER_MAINPEND);
            } else
            {
                c++;
            }
        }
    }
    this->UpdateStatusBarData();
    // let's refresh the edits that are being post processed
    if (QueryPool::HugglePool->ProcessingEdits.count() > 0)
    {
        int Edit = 0;
        while (Edit < QueryPool::HugglePool->ProcessingEdits.count())
        {
            WikiEdit *e = QueryPool::HugglePool->ProcessingEdits.at(Edit);
            if (e->FinalizePostProcessing())
            {
                QueryPool::HugglePool->ProcessingEdits.removeAt(Edit);
                e->UnregisterConsumer(HUGGLECONSUMER_CORE_POSTPROCESS);
            }
            else
            {
                Edit++;
            }
        }
    }
    QueryPool::HugglePool->CheckQueries();
    this->FinishPatrols();
    Syslog::HuggleLogs->lUnwrittenLogs->lock();
    if (Syslog::HuggleLogs->UnwrittenLogs.count() > 0)
    {
        int c = 0;
        while (c < Syslog::HuggleLogs->UnwrittenLogs.count())
        {
            this->SystemLog->InsertText(Syslog::HuggleLogs->UnwrittenLogs.at(c));
            c++;
        }
        Syslog::HuggleLogs->UnwrittenLogs.clear();
    }
    Syslog::HuggleLogs->lUnwrittenLogs->unlock();
    this->Queries->RemoveExpired();
    if (this->OnNext_EvPage != nullptr && this->qNext != nullptr && this->qNext->IsProcessed())
    {
        this->tb->SetPage(this->OnNext_EvPage);
        this->tb->RenderEdit();
        delete this->OnNext_EvPage;
        this->OnNext_EvPage = nullptr;
        this->qNext = nullptr;
    }
    this->FinishRestore();
    this->TruncateReverts();
    this->SystemLog->Render();
}

void MainWindow::TruncateReverts()
{
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
        WikiEdit *we = QueryPool::HugglePool->RevertBuffer.at(0);
        QueryPool::HugglePool->RevertBuffer.removeAt(0);
        we->UnregisterConsumer(HUGGLECONSUMER_QP_REVERTBUFFER);
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
            this->Shutdown = ShutdownOpUpdatingWhitelist;
            this->fWaiting->Status(60, _l("updating-wl"));
            foreach (WikiSite*site, Configuration::HuggleConfiguration->Projects)
            {
                if (this->WhitelistQueries.contains(site))
                {
                    this->WhitelistQueries[site]->DecRef();
                    this->WhitelistQueries.remove(site);
                }
                this->WhitelistQueries.insert(site, new WLQuery(site));
                this->WhitelistQueries[site]->Type = WLQueryType_WriteWL;
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
                this->fWaiting->Status(60);
                return;
            }
            // we finished writing the wl
            this->fWaiting->Status(90, _l("saveuserconfig-progress"));
            this->Shutdown = ShutdownOpUpdatingConf;
            QString page = Configuration::HuggleConfiguration->GlobalConfig_UserConf;
            page = page.replace("$1", Configuration::HuggleConfiguration->SystemConfig_Username);
            Version huggle_version(HUGGLE_VERSION);
            foreach (WikiSite*site, Configuration::HuggleConfiguration->Projects)
            {
                if (*site->UserConfig->Previous_Version > huggle_version)
                {
                    if (Generic::MessageBox("Do you really want to store the configs",
                                            "This version of huggle (" + QString(HUGGLE_VERSION) + ") is older that version of huggle that you used last (" +
                                            site->UserConfig->Previous_Version->ToString() + ") if you continue, some of the settings you stored "\
                                            "with the newer version may be lost. Do you really want to do that? (clicking no will skip it)",
                                            MessageBoxStyleQuestion, true) == QMessageBox::No)
                        continue;
                }
                WikiPage *uc = new WikiPage(page);
                uc->Site = site;
                Collectable_SmartPtr<EditQuery> temp = WikiUtil::EditPage(uc, Configuration::MakeLocalUserConfig(site),
                                                                          _l("saveuserconfig-progress"), true);
                temp->IncRef();
                this->StorageQueries.insert(site, temp.GetPtr());
                delete uc;
            }
            return;
        }
    } else
    {
        // we need to check if config was written
        foreach (WikiSite *site, Configuration::HuggleConfiguration->Projects)
        {
            if (this->StorageQueries.contains(site))
            {
                if (this->StorageQueries[site]->IsProcessed())
                {
                    this->StorageQueries[site]->DecRef();
                    this->StorageQueries.remove(site);
                } else
                {
                    return;
                }
            }
        }
        this->wlt->stop();
        this->GeneralTimer->stop();
        Core::HuggleCore->Shutdown();
    }
}

void MainWindow::on_actionNext_triggered()
{
    this->Queue1->Next();
}

void MainWindow::on_actionNext_2_triggered()
{
    this->Queue1->Next();
}

void MainWindow::on_actionWarn_triggered()
{
    this->TriggerWarn();
}

void MainWindow::on_actionRevert_currently_displayed_edit_triggered()
{
    if (this->EditingChecks())
        this->Revert();
}

void MainWindow::on_actionWarn_the_user_triggered()
{
    this->TriggerWarn();
}

void MainWindow::on_actionRevert_currently_displayed_edit_and_warn_the_user_triggered()
{
    if (!this->EditingChecks())
        return;
    Collectable_SmartPtr<RevertQuery> result = this->Revert("", false);
    if (result != nullptr)
    {
        this->Warn("warning", result);
        this->DisplayNext(result);
    } else
    {
        this->DisplayNext(result);
    }
}

void MainWindow::on_actionRevert_and_warn_triggered()
{
    if (!this->EditingChecks())
        return;
    Collectable_SmartPtr<RevertQuery> result = this->Revert("", false);
    if (result != nullptr)
    {
        this->Warn("warning", result);
        this->DisplayNext(result);
    } else
    {
        this->DisplayNext(result);
    }
}

void MainWindow::on_actionRevert_triggered()
{
    if (!this->EditingChecks())
        return;

    this->Revert();
}

void MainWindow::on_actionShow_ignore_list_of_current_wiki_triggered()
{
    if (this->Ignore != nullptr)
        delete this->Ignore;
    this->Ignore = new IgnoreList(this);
    this->Ignore->show();
}

void MainWindow::on_actionForward_triggered()
{
    if (this->CurrentEdit == nullptr || this->CurrentEdit->Next == nullptr)
        return;
    this->ProcessEdit(this->CurrentEdit->Next, true);
}

void MainWindow::on_actionBack_triggered()
{
    if (this->CurrentEdit == nullptr || this->CurrentEdit->Previous == nullptr)
        return;
    this->ProcessEdit(this->CurrentEdit->Previous, true);
}

void MainWindow::CustomRevert()
{
    if (!this->EditingChecks())
        return;
    QAction *revert = (QAction*) QObject::sender();
    ProjectConfiguration *conf = this->GetCurrentWikiSite()->GetProjectConfig();
    QString k = HuggleParser::GetKeyOfWarningTypeFromWarningName(revert->text(), conf);
    QString rs = HuggleParser::GetSummaryOfWarningTypeFromWarningKey(k, conf);
    rs = Huggle::Configuration::HuggleConfiguration->GenerateSuffix(rs, conf);
    this->Revert(rs);
}

void MainWindow::CustomRevertWarn()
{
    if (!this->EditingChecks())
        return;
    QAction *revert = (QAction*) QObject::sender();
    ProjectConfiguration *conf = this->GetCurrentWikiSite()->GetProjectConfig();
    QString k = HuggleParser::GetKeyOfWarningTypeFromWarningName(revert->text(), conf);
    QString rs = HuggleParser::GetSummaryOfWarningTypeFromWarningKey(k, conf);
    rs = Huggle::Configuration::HuggleConfiguration->GenerateSuffix(rs, conf);
    Collectable_SmartPtr<RevertQuery> result = this->Revert(rs, false);
    if (result != nullptr)
    {
        this->Warn(k, result);
        this->DisplayNext(result);
    } else
    {
        this->DisplayNext(result);
    }
}

void MainWindow::CustomWarn()
{
    if (!this->EditingChecks())
        return;
    ProjectConfiguration *conf = this->GetCurrentWikiSite()->GetProjectConfig();
    QAction *revert = (QAction*) QObject::sender();
    QString k = HuggleParser::GetKeyOfWarningTypeFromWarningName(revert->text(), conf);
    this->Warn(k, nullptr);
}

QString MainWindow::GetSummaryText(QString text)
{
    int id=0;
    ProjectConfiguration *conf = this->GetCurrentWikiSite()->GetProjectConfig();
    while (id <conf->RevertSummaries.count())
    {
        if (text == this->GetSummaryKey(conf->RevertSummaries.at(id)))
        {
            QString data = conf->RevertSummaries.at(id);
            if (data.contains(";"))
            {
                data = data.mid(data.indexOf(";") + 1);
            }
            return data;
        }
        id++;
    }
    return conf->DefaultSummary;
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
    if (Configuration::HuggleConfiguration->Restricted)
    {
        this->tCheck->stop();
        this->GeneralTimer->stop();
        Core::HuggleCore->Main = nullptr;
        this->deleteLater();
        this->close();
        Core::HuggleCore->Shutdown();
        return;
    }
    this->Shutdown = ShutdownOpRetrievingWhitelist;
    foreach (WikiSite *site, Configuration::HuggleConfiguration->Projects)
    {
        if (site->Provider != nullptr)
            site->Provider->Stop();
    }
    if (this->fWaiting != nullptr)
        delete this->fWaiting;
    this->fWaiting = new WaitingForm(this);
    this->fWaiting->show();
    this->fWaiting->Status(10, _l("whitelist-download"));
    this->wlt = new QTimer(this);
    connect(this->wlt, SIGNAL(timeout()), this, SLOT(OnTimerTick0()));
    this->wlt->start(800);
}

bool MainWindow::ReconnectIRC(WikiSite *site)
{
    if (!this->CheckExit())
        return false;
    if (!Configuration::HuggleConfiguration->UsingIRC)
    {
        Syslog::HuggleLogs->Log(_l("irc-not"));
        return false;
    }
    Syslog::HuggleLogs->Log(_l("irc-connecting", site->Name));
    if (!site->Provider)
    {
        // this is problem
        throw new Huggle::NullPointerException("site->Provider", "void MainWindow::ReconnectIRC(WikiSite *site)");
    }
    site->Provider->Stop();
    while (!site->Provider->IsStopped())
    {
        Syslog::HuggleLogs->Log(_l("irc-stop", site->Name));
        Sleeper::usleep(200000);
    }
    this->ui->actionIRC->setChecked(true);
    this->ui->actionWiki->setChecked(false);
    this->ChangeProvider(site, new HuggleFeedProviderIRC(site));
    if (!site->Provider->Start())
    {
        this->ui->actionIRC->setChecked(false);
        this->ui->actionWiki->setChecked(true);
        Syslog::HuggleLogs->ErrorLog(_l("provider-primary-failure", site->Name));
        this->ChangeProvider(site, new HuggleFeedProviderWiki(site));
        site->Provider->Start();
        return false;
    }
    return true;
}

bool MainWindow::BrowserPageIsEditable()
{
    return this->EditablePage;
}

bool MainWindow::CheckEditableBrowserPage()
{
    if (!this->EditablePage || this->CurrentEdit == nullptr)
    {
        QMessageBox mb;
        mb.setWindowTitle("Cannot perform action");
        mb.setText(_l("main-no-page"));
        mb.exec();
        return false;
    }
    if (Configuration::HuggleConfiguration->SystemConfig_RequestDelay)
    {
        qint64 wt = QDateTime::currentDateTime().msecsTo(this->EditLoad.addSecs(hcfg->SystemConfig_DelayVal));
        if (wt > 0)
        {
            Syslog::HuggleLogs->WarningLog("Ignoring edit request because you are too fast, please wait " +
                                           QString::number(wt)+ " ms");
            return false;
        }
    }
    if (this->CurrentEdit->Page == nullptr)
        throw Huggle::Exception("this->CurrentEdit->Page == nullptr");

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
        wq_->Type = WLQueryType_SuspWL;
        wq_->Parameters = "page=" + QUrl::toPercentEncoding(this->CurrentEdit->Page->PageName) + "&wiki="
                          + QUrl::toPercentEncoding(this->GetCurrentWikiSite()->WhiteList) + "&user="
                          + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->SystemConfig_Username) + "&score="
                          + QString::number(this->CurrentEdit->Score) + "&revid="
                          + QString::number(this->CurrentEdit->RevID) + "&summary="
                          + QUrl::toPercentEncoding(this->CurrentEdit->Summary);
        wq_->Process();
        QueryPool::HugglePool->AppendQuery(wq_);
        this->CurrentEdit->User->SetBadnessScore(this->CurrentEdit->User->GetBadnessScore() + 1);
    }
    this->DisplayNext();
}

void MainWindow::PatrolThis(WikiEdit *e)
{
    ProjectConfiguration *conf = this->GetCurrentWikiSite()->GetProjectConfig();
    if (e == nullptr)
        e = this->CurrentEdit;
    if (e == nullptr || !conf->Patrolling)
        return;
    ApiQuery *query = nullptr;
    bool flaggedrevs = conf->PatrollingFlaggedRevs;

    // if this edit doesn't have the patrol token we need to get one
    // if we're using flaggedrevs this will actually be an edit token, but we pretend it's a patrol one
    if (e->PatrolToken.isEmpty())
    {
        // register consumer so that gc doesn't delete this edit meanwhile
        e->RegisterConsumer("patrol");
        query = new ApiQuery(ActionTokens, this->GetCurrentWikiSite());
        if (flaggedrevs)
        {
            query->Target = "Retrieving patrol token (FlaggedRevs) for " + e->Page->PageName;
            query->Parameters = "type=edit";
        } else
        {
            query->Target = "Retrieving patrol token for " + e->Page->PageName;
            query->Parameters = "type=patrol";
        }
        // this uggly piece of code actually rocks
        query->CallbackResult = (void*)e;
        query->RegisterConsumer("patrol");
        QueryPool::HugglePool->AppendQuery(query);
        query->Process();
        this->PatrolledEdits.append(query);
        return;
    }

    // we can execute patrol now
    query = new ApiQuery();
    query->Site = this->GetCurrentWikiSite();
    query->UsingPOST = true;
    if (flaggedrevs)
    {
        query->SetAction(ActionReview);
        query->Target = "Patrolling (FlaggedRevs) " + e->Page->PageName;
    } else
    {
        query->SetAction(ActionPatrol);
        query->Target = "Patrolling " + e->Page->PageName;
    }
    query->Parameters = "revid=" + QString::number(e->RevID) + "&token=" + QUrl::toPercentEncoding(e->PatrolToken);
    if (flaggedrevs)
        query->Parameters += "&flag_accuracy=1";

    QueryPool::HugglePool->AppendQuery(query);
    HUGGLE_DEBUG1("Patrolling " + e->Page->PageName);
    query->Process();
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
    this->ui->actionClear->setText(_l("main-queue-clear"));
    this->ui->actionClear_talk_page_of_user->setText(_l("main-user-clear-tp"));
    this->ui->actionDecrease_badness_score_by_20->setText(_l("main-user-db"));
    this->ui->actionNext_2->setText(_l("main-page-next"));
    this->ui->actionIncrease_badness_score_by_20->setText(_l("main-user-ib"));
    this->ui->actionEdit_page_in_browser->setText(_l("main-page-edit"));
    this->ui->actionFlag_as_suspicious_edit->setText(_l("main-page-flag-suspicious-edit"));
    this->ui->actionFlag_as_a_good_edit->setText(_l("main-page-flag-good-edit"));
    this->ui->actionRequest_speedy_deletion->setText(_l("main-page-reqdeletion"));
    this->ui->actionDelete->setText(_l("main-page-delete"));
    this->ui->actionRequest_protection->setText(_l("main-page-reqprotection"));
    this->ui->actionProtect->setText(_l("main-page-protect"));
    this->ui->actionRestore_this_revision->setText(_l("main-page-restore"));
    this->ui->actionDisplay_a_session_data->setText(_l("main-display-session-data"));
    this->ui->actionDisplay_whitelist->setText(_l("main-display-whitelist"));
    this->ui->actionDisplay_history_in_browser->setText(_l("main-page-historypage"));
    this->ui->actionDisplay_this_page_in_browser->setText(_l("main-browser-open"));
    this->ui->actionFeedback->setText(_l("main-help-feedback"));
    this->ui->actionReport_user->setText(_l("main-user-report"));
    this->ui->actionUser_contributions->setText(_l("main-user-contribs"));
    this->ui->actionPreferences->setText(_l("main-system-options"));
    this->ui->actionReconnect_IRC->setText(_l("main-system-reconnectirc"));
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

    // arrows icons should be mirrored for RTL languages
    if (Localizations::HuggleLocalizations->IsRTL())
    {
        this->ui->actionForward->setIcon(QIcon(":/huggle/pictures/Resources/browser-prev.png"));
        this->ui->actionBack->setIcon(QIcon(":/huggle/pictures/Resources/browser-next.png"));
    }
}

void MainWindow::_BlockUser()
{
    if (!this->CheckExit() || !this->CheckEditableBrowserPage())
        return;
    if(this->CurrentEdit == nullptr)
    {
        Syslog::HuggleLogs->ErrorLog(_l("block-none"));
        return;
    }
    if (this->fBlockForm != nullptr)
        delete this->fBlockForm;

    this->fBlockForm = new BlockUser(this);
    this->CurrentEdit->User->Resync();
    this->fBlockForm->SetWikiUser(this->CurrentEdit->User);
    this->fBlockForm->show();
}

void MainWindow::DisplayNext(Query *q)
{
    switch(Configuration::HuggleConfiguration->UserConfig->GoNext)
    {
        case Configuration_OnNext_Stay:
            return;
        case Configuration_OnNext_Next:
            this->Queue1->Next();
            return;
        case Configuration_OnNext_Revert:
            //! \bug This doesn't seem to work
            if (this->CurrentEdit == nullptr)
                return;
            if (q == nullptr)
            {
                this->Queue1->Next();
                return;
            }
            if (this->OnNext_EvPage != nullptr)
                delete this->OnNext_EvPage;
            this->OnNext_EvPage = new WikiPage(this->CurrentEdit->Page);
            this->qNext = q;
            return;
    }
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
    if (this->fDeleteForm != nullptr)
        delete this->fDeleteForm;
    this->fDeleteForm = new DeleteForm(this);
    this->fDeleteForm->SetPage(this->CurrentEdit->Page, this->CurrentEdit->User);
    this->fDeleteForm->show();
}

void MainWindow::DisplayTalk()
{
    if (this->CurrentEdit == nullptr)
        return;
    // display a talk page
    WikiPage *page = new WikiPage(this->CurrentEdit->User->GetTalk());
    this->Browser->DisplayPreFormattedPage(page);
    delete page;
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

void MainWindow::WelcomeGood()
{
    if (this->CurrentEdit == nullptr || !this->CheckExit() || !this->CheckEditableBrowserPage())
        return;
    Hooks::OnGood(this->CurrentEdit);
    this->PatrolThis();
    this->CurrentEdit->User->SetBadnessScore(this->CurrentEdit->User->GetBadnessScore() - 200);
    if (Configuration::HuggleConfiguration->UserConfig->WelcomeGood &&
            this->CurrentEdit->User->TalkPage_GetContents().isEmpty())
        this->Welcome();
    this->DisplayNext();
}

void MainWindow::RenderPage(QString Page)
{
    WikiPage *page = new WikiPage(Page);
    page->Site = this->GetCurrentWikiSite();
    this->tb->SetPage(page);
    delete page;
    this->tb->RenderEdit();
}

WikiSite *MainWindow::GetCurrentWikiSite()
{
    if (this->CurrentEdit == nullptr || this->CurrentEdit->Page == nullptr)
    {
        return Configuration::HuggleConfiguration->Project;
    }

    return this->CurrentEdit->GetSite();
}

void MainWindow::LockPage()
{
    this->EditablePage = false;
}

QString MainWindow::ProjectURL()
{
    if (this->CurrentEdit == nullptr || this->CurrentEdit->Page == nullptr)
    {
        return Configuration::GetProjectWikiURL();
    }
    return Configuration::GetProjectWikiURL(this->CurrentEdit->GetSite());
}

bool MainWindow::CheckExit()
{
    if (this->ShuttingDown)
    {
        QMessageBox mb;
        mb.setWindowTitle(_l("error"));
        mb.setText(_l("main-shutting-down"));
        mb.exec();
        return false;
    }
    return true;
}

void MainWindow::Welcome()
{
    if (!this->EditingChecks())
        return;
    ProjectConfiguration *conf = this->GetCurrentWikiSite()->GetProjectConfig();
    this->CurrentEdit->User->Resync();
    bool create_only = true;
    if (!this->CurrentEdit->User->TalkPage_GetContents().isEmpty())
    {
        if (QMessageBox::question(this, "Welcome :o", _l("welcome-tp-empty-fail"),
                         QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
            return;
        else
            create_only = false;
    } else if (!this->CurrentEdit->User->TalkPage_WasRetrieved())
    {
        if (QMessageBox::question(this, "Welcome :o", _l("welcome-page-miss-fail"), QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
            return;
    }
    if (this->CurrentEdit->User->IsIP())
    {
        if (this->CurrentEdit->User->TalkPage_GetContents().isEmpty())
        {
            // write something to talk page so that we don't welcome this user twice
            this->CurrentEdit->User->TalkPage_SetContents(conf->WelcomeAnon);
        }
        WikiUtil::MessageUser(this->CurrentEdit->User, conf->WelcomeAnon + " ~~~~", conf->WelcomeTitle, conf->WelcomeSummary,
                              false, nullptr, false, false, true, this->CurrentEdit->TPRevBaseTime, create_only);
        return;
    }
    if (conf->WelcomeTypes.isEmpty())
    {
        // This error should never happen so we don't need to localize this
        Syslog::HuggleLogs->ErrorLog("There are no welcome messages defined for this project");
        return;
    }
    QString message = HuggleParser::GetValueFromKey(conf->WelcomeTypes.at(0));
    if (message.isEmpty())
    {
        // This error should never happen so we don't need to localize this
        Syslog::HuggleLogs->ErrorLog("Invalid welcome template, ignored message");
        return;
    }
    // write something to talk page so that we don't welcome this user twice
    this->CurrentEdit->User->TalkPage_SetContents(message);
    WikiUtil::MessageUser(this->CurrentEdit->User, message, conf->WelcomeTitle, conf->WelcomeSummary, false, nullptr,
                          false, false, true, this->CurrentEdit->TPRevBaseTime, create_only);
}

void MainWindow::ChangeProvider(WikiSite *site, HuggleFeed *provider)
{
    if (site->Provider != nullptr)
        delete site->Provider;

    site->Provider = provider;
}

void MainWindow::ReloadInterface()
{
    ProjectConfiguration *conf = this->GetCurrentWikiSite()->GetProjectConfig();
    this->WarnItems.clear();
    this->RevertAndWarnItems.clear();
    this->RevertItems.clear();
    delete this->RevertSummaries;
    delete this->WarnMenu;
    delete this->RevertWarn;
    this->ui->actionRequest_protection->setEnabled(conf->RFPP);
    this->RevertSummaries = new QMenu(this);
    this->WarnMenu = new QMenu(this);
    this->RevertWarn = new QMenu(this);
    if (conf->WarningTypes.count() > 0)
    {
        int r=0;
        while (r< conf->WarningTypes.count())
        {
            QAction *action = new QAction(HuggleParser::GetValueFromKey(conf->WarningTypes.at(r)), this->RevertSummaries);
            QAction *actiona = new QAction(HuggleParser::GetValueFromKey(conf->WarningTypes.at(r)), this->RevertWarn);
            QAction *actionb = new QAction(HuggleParser::GetValueFromKey(conf->WarningTypes.at(r)), this->WarnMenu);
            this->RevertAndWarnItems.append(actiona);
            this->WarnItems.append(actionb);
            this->RevertItems.append(action);
            this->RevertWarn->addAction(actiona);
            this->WarnMenu->addAction(actionb);
            this->RevertSummaries->addAction(action);
            r++;
            connect(action, SIGNAL(triggered()), this, SLOT(CustomRevert()));
            connect(actiona, SIGNAL(triggered()), this, SLOT(CustomRevertWarn()));
            connect(actionb, SIGNAL(triggered()), this, SLOT(CustomWarn()));
        }
    }
    this->ui->actionWarn->setMenu(this->WarnMenu);
    this->ui->actionRevert->setMenu(this->RevertSummaries);
    this->ui->actionRevert_and_warn->setMenu(this->RevertWarn);
    bool fr = (this->warnToolButtonMenu == nullptr || this->rtToolButtonMenu == nullptr);
    // replace abstract QAction with QToolButton to be able to set PopupMode for nicer menu opening
    if (this->warnToolButtonMenu == nullptr)
        this->warnToolButtonMenu = new QToolButton(this);
    if (this->rtToolButtonMenu == nullptr)
        this->rtToolButtonMenu = new QToolButton(this);
    if (this->rwToolButtonMenu == nullptr)
        this->rwToolButtonMenu = new QToolButton(this);
    this->warnToolButtonMenu->setDefaultAction(this->ui->actionWarn);
    this->rtToolButtonMenu->setDefaultAction(this->ui->actionRevert);
    this->rwToolButtonMenu->setDefaultAction(this->ui->actionRevert_and_warn);
    this->warnToolButtonMenu->setPopupMode(QToolButton::MenuButtonPopup);
    this->rtToolButtonMenu->setPopupMode(QToolButton::MenuButtonPopup);
    this->rwToolButtonMenu->setPopupMode(QToolButton::MenuButtonPopup);
    if (fr)
    {
        // insert them before their counterparts and then delete the counterpart
        this->ui->mainToolBar->insertWidget(this->ui->actionRevert_and_warn, rwToolButtonMenu);
        this->ui->mainToolBar->removeAction(this->ui->actionRevert_and_warn);
        this->ui->mainToolBar->insertWidget(this->ui->actionRevert, rtToolButtonMenu);
        this->ui->mainToolBar->removeAction(this->ui->actionRevert);
        this->ui->mainToolBar->insertWidget(this->ui->actionWarn, warnToolButtonMenu);
        this->ui->mainToolBar->removeAction(this->ui->actionWarn);
    }
    // button action depends on adminrights
    if (conf->Rights.contains("delete"))
        this->ui->actionDelete_page->setText(_l("main-page-delete"));
    else
        this->ui->actionDelete_page->setText(_l("main-page-reqdeletion"));
    this->ReloadSc();
}

void MainWindow::on_actionWelcome_user_triggered()
{
    this->Welcome();
}

void MainWindow::on_actionOpen_in_a_browser_triggered()
{
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
    this->IncreaseBS();
}

void MainWindow::on_actionDecrease_badness_score_by_20_triggered()
{
    this->DecreaseBS();
}

void MainWindow::on_actionGood_edit_triggered()
{
    this->WelcomeGood();
}

void MainWindow::on_actionUser_contributions_triggered()
{
    if (this->CurrentEdit != nullptr)
    {
        QDesktopServices::openUrl(QUrl::fromEncoded(QString(this->ProjectURL() + "Special:Contributions/" +
                                  QUrl::toPercentEncoding(this->CurrentEdit->User->Username)).toUtf8()));
    }
}

void MainWindow::on_actionTalk_page_triggered()
{
    this->DisplayTalk();
}

void MainWindow::on_actionFlag_as_a_good_edit_triggered()
{
    this->WelcomeGood();
}

void MainWindow::on_actionDisplay_this_page_in_browser_triggered()
{
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
    if (this->CurrentEdit != nullptr)
        QDesktopServices::openUrl(QUrl::fromEncoded(QString(this->ProjectURL() + this->CurrentEdit->Page->EncodedName() + "?action=edit").toUtf8()));
}

void MainWindow::on_actionDisplay_history_in_browser_triggered()
{
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
    if (!this->EditingChecks())
        return;
    if (!this->CurrentEdit->User->IsIP())
    {
        Syslog::HuggleLogs->Log(_l("feature-nfru"));
        return;
    }
    WikiPage *page = new WikiPage(this->CurrentEdit->User->GetTalk());
    /// \todo LOCALIZE ME
    WikiUtil::EditPage(page, this->GetCurrentWikiSite()->ProjectConfig->ClearTalkPageTemp
                       + "\n" + this->GetCurrentWikiSite()->ProjectConfig->WelcomeAnon + " ~~~~",
                       "Cleaned old templates from talk page " + this->GetCurrentWikiSite()->ProjectConfig->EditSuffixOfHuggle);
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
    if (!this->EditingChecks())
        return;
    Collectable_SmartPtr<RevertQuery> result = this->Revert("", false);
    if (result != nullptr)
        this->Warn("warning", result);
}

void MainWindow::on_actionRevert_currently_displayed_edit_and_stay_on_page_triggered()
{
    if (!this->EditingChecks())
        return;
    this->Revert("", false);
}

void MainWindow::on_actionWelcome_user_2_triggered()
{
    this->Welcome();
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

void MainWindow::on_actionReconnect_IRC_triggered()
{
    this->ReconnectIRC(this->GetCurrentWikiSite());
}

void MainWindow::on_actionRequest_speedy_deletion_triggered()
{
    this->RequestPD();
}

void MainWindow::on_actionDelete_triggered()
{
    this->DeletePage();
}

void Huggle::MainWindow::on_actionBlock_user_triggered()
{
    this->_BlockUser();
}

void Huggle::MainWindow::on_actionIRC_triggered()
{
    this->ReconnectIRC(this->GetCurrentWikiSite());
}

void Huggle::MainWindow::on_actionWiki_triggered()
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
    this->ChangeProvider(this->GetCurrentWikiSite(), new HuggleFeedProviderWiki(this->GetCurrentWikiSite()));
    this->GetCurrentWikiSite()->Provider->Start();
}

void Huggle::MainWindow::on_actionShow_talk_triggered()
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
        QueryPool::HugglePool->AppendQuery(query);
        query->Process();
        query->DecRef();
    }
    this->RenderPage("User_talk:" + Configuration::HuggleConfiguration->SystemConfig_Username);
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
    if (this->fProtectForm != nullptr)
        delete this->fProtectForm;
    this->fProtectForm = new ProtectPage(this);
    this->fProtectForm->setPageToProtect(this->CurrentEdit->Page);
    this->fProtectForm->show();
}

void Huggle::MainWindow::on_actionEdit_info_triggered()
{
    // don't localize this please
    Syslog::HuggleLogs->Log("Current number of edits in memory: " + QString::number(WikiEdit::EditList.count()));
}

void Huggle::MainWindow::on_actionFlag_as_suspicious_edit_triggered()
{
    this->SuspiciousEdit();
}

void Huggle::MainWindow::on_actionDisconnect_triggered()
{
    this->VandalDock->Disconnect();
}

void MainWindow::on_actionReport_username_triggered()
{
    if (this->CurrentEdit == nullptr || !this->CheckExit() || !this->CheckEditableBrowserPage())
    {
        return;
    }
    if (!this->GetCurrentWikiSite()->ProjectConfig->UAAavailable)
    {
        QMessageBox dd;
        dd.setIcon(dd.Information);
        dd.setWindowTitle(_l("uaa-not-supported"));
        dd.setText(_l("uaa-not-supported-text"));
        dd.exec();
    }
    if (this->CurrentEdit->User->IsIP())
    {
        Syslog::HuggleLogs->ErrorLog("You can't report IP address using this feature");
        return;
    }
    if (this->fUaaReportForm != nullptr)
        delete this->fUaaReportForm;
    this->fUaaReportForm = new UAAReport();
    this->fUaaReportForm->setUserForUAA(this->CurrentEdit->User);
    this->fUaaReportForm->show();
}

void Huggle::MainWindow::on_actionShow_list_of_score_words_triggered()
{
    if (this->fScoreWord != nullptr)
        delete this->fScoreWord;
    this->fScoreWord = new ScoreWordsDbForm(this);
    this->fScoreWord->show();
}

void Huggle::MainWindow::on_actionRevert_AGF_triggered()
{
    this->RevertAgf(false);
}

void Huggle::MainWindow::on_actionDisplay_a_session_data_triggered()
{
    if (this->fSessionData != nullptr)
        delete this->fSessionData;

    this->fSessionData = new SessionForm(this);
    this->fSessionData->show();
}

void Huggle::MainWindow::on_actionDisplay_whitelist_triggered()
{
    if (this->fWhitelist != nullptr)
        delete this->fWhitelist;
    this->fWhitelist = new WhitelistForm(this);
    this->fWhitelist->show();
}

void Huggle::MainWindow::on_actionResort_queue_triggered()
{
    this->Queue1->Sort();
}

void Huggle::MainWindow::on_actionRestore_this_revision_triggered()
{
    if (!this->CheckExit() || this->CurrentEdit == nullptr)
        return;

    if (Configuration::HuggleConfiguration->Restricted)
    {
        Generic::DeveloperError();
        return;
    }
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

void Huggle::MainWindow::on_actionClear_triggered()
{
    this->Queue1->Clear();
}

void Huggle::MainWindow::on_actionDelete_page_triggered()
{
    if (this->GetCurrentWikiSite()->GetProjectConfig()->Rights.contains("delete"))
    {
        this->DeletePage();
    } else
    {
        this->RequestPD();
    }
}

void Huggle::MainWindow::on_actionBlock_user_2_triggered()
{
    this->_BlockUser();
}

void Huggle::MainWindow::on_actionDisplay_talk_triggered()
{
    this->DisplayTalk();
}

void MainWindow::TimerCheckTPOnTick()
{
    if (Configuration::HuggleConfiguration->Restricted || this->ShuttingDown)
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

void Huggle::MainWindow::on_actionSimulate_message_triggered()
{
    Configuration::HuggleConfiguration->NewMessage = true;
}

void Huggle::MainWindow::on_actionHtml_dump_triggered()
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
    delete f;
    QDesktopServices::openUrl(QDir().absoluteFilePath(name));
}

void Huggle::MainWindow::on_actionEnforce_sysop_rights_triggered()
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

void Huggle::MainWindow::on_actionFeedback_triggered()
{
    QString feedback = Configuration::HuggleConfiguration->ProjectConfig->Feedback;
    if (feedback.isEmpty())
        feedback = Configuration::HuggleConfiguration->GlobalConfig_FeedbackPath;
    QDesktopServices::openUrl(feedback);
}

void Huggle::MainWindow::on_actionConnect_triggered()
{
    this->VandalDock->Connect();
}

void Huggle::MainWindow::on_actionDisplay_user_data_triggered()
{
    Configuration::HuggleConfiguration->UserConfig->HAN_DisplayUser = this->ui->actionDisplay_user_data->isChecked();
}

void Huggle::MainWindow::on_actionDisplay_user_messages_triggered()
{
    Configuration::HuggleConfiguration->UserConfig->HAN_DisplayUserTalk = this->ui->actionDisplay_user_messages->isChecked();
}

void Huggle::MainWindow::on_actionDisplay_bot_data_triggered()
{
    Configuration::HuggleConfiguration->UserConfig->HAN_DisplayBots = this->ui->actionDisplay_bot_data->isChecked();
}

void Huggle::MainWindow::on_actionRequest_protection_triggered()
{
    if (!this->EditingChecks())
        return;
    if (!this->GetCurrentWikiSite()->GetProjectConfig()->RFPP)
    {
        //! \todo Localize
        Syslog::HuggleLogs->ErrorLog("This project doesn't support requests for protection");
    }
    if (this->fRFProtection != nullptr)
        delete this->fRFProtection;
    this->fRFProtection = new RequestProtect(this->CurrentEdit->Page);
    this->fRFProtection->show();
}

void Huggle::MainWindow::on_actionRemove_edits_made_by_whitelisted_users_triggered()
{
    // the number must be higher that the real score so that we match even the edits
    // which have the same score (-800 + 1) > (-800)
    this->Queue1->DeleteByScore(this->GetCurrentWikiSite()->GetProjectConfig()->WhitelistScore + 1);
}

void Huggle::MainWindow::on_actionDelete_all_edits_with_score_lower_than_200_triggered()
{
    this->Queue1->DeleteByScore(-200);
}

void Huggle::MainWindow::on_actionRelog_triggered()
{
    ReloginForm *form = new ReloginForm(this);
    form->exec();
    delete form;
}

void Huggle::MainWindow::on_actionAbort_2_triggered()
{
    if (!this->RevertStack.count())
    {
        Syslog::HuggleLogs->ErrorLog("Nothing to stop");
        return;
    }
    if (Configuration::HuggleConfiguration->SystemConfig_InstantReverts)
    {
        Syslog::HuggleLogs->ErrorLog("Unable to cancel the current operation, you need to disable Instant reverts in preferences for this feature to work");
        return;
    }
    // we cancel the latest revert query that is waiting in a stack
    RevertQuery *revert_query = this->RevertStack.last();
    revert_query->Kill();
    this->RevertStack.removeLast();
    revert_query->DecRef();
}

void Huggle::MainWindow::on_actionDryMode_triggered()
{
    Configuration::HuggleConfiguration->SystemConfig_DryMode = this->ui->actionDryMode->isChecked();
}

void Huggle::MainWindow::on_actionDisplay_this_page_triggered()
{
    if (!this->CheckExit() || this->CurrentEdit == nullptr)
        return;
    this->Browser->DisplayPreFormattedPage(this->CurrentEdit->Page);
    this->LockPage();
}

void Huggle::MainWindow::on_actionResume_provider_triggered()
{
    this->ui->actionResume_provider->setVisible(false);
    this->ui->actionStop_provider->setVisible(true);
    this->ResumeQueue();
}

void Huggle::MainWindow::on_actionStop_provider_triggered()
{
    this->PauseQueue();
    this->ui->actionResume_provider->setVisible(true);
    this->ui->actionStop_provider->setVisible(false);
}

void Huggle::MainWindow::on_actionRevert_only_this_revision_triggered()
{
    if (this->EditingChecks())
        this->Revert("", true, true);
}

void Huggle::MainWindow::on_actionTag_2_triggered()
{
    if (!this->CheckEditableBrowserPage())
        return;

    if (this->fWikiPageTags)
        delete this->fWikiPageTags;
    this->fWikiPageTags = new WikiPageTagsForm(this);
    this->fWikiPageTags->show();
    this->fWikiPageTags->ChangePage(this->CurrentEdit->Page);
}

void Huggle::MainWindow::on_actionReload_menus_triggered()
{
    this->ReloadInterface();
}

void MainWindow::SetProviderIRC()
{
    if (!this->CheckExit())
        return;
    QAction *action = (QAction*)QObject::sender();
    if (!this->ActionSites.contains(action))
        throw new Huggle::Exception("There is no such a site in hash table",
                                    "void MainWindow::SetProviderIRC()");
    WikiSite *wiki = this->ActionSites[action];
    if (this->ReconnectIRC(wiki))
    {
        action->setChecked(true);
        if (this->lWikis.contains(wiki))
            this->lWikis[wiki]->setChecked(false);
    } else
    {
        action->setChecked(false);
        if (this->lWikis.contains(wiki))
            this->lWikis[wiki]->setChecked(true);
    }
}

void MainWindow::SetProviderWiki()
{
    if (!this->CheckExit())
        return;
    QAction *action = (QAction*)QObject::sender();
    if (!this->ActionSites.contains(action))
        throw new Huggle::Exception("There is no such a site in hash table",
                                    "void MainWindow::SetProviderIRC()");
    WikiSite *wiki = this->ActionSites[action];
    Syslog::HuggleLogs->Log(_l("irc-switch-rc"));
    wiki->Provider->Stop();
    this->ui->actionIRC->setChecked(false);
    action->setChecked(true);
    while (!wiki->Provider->IsStopped())
    {
        Syslog::HuggleLogs->Log(_l("irc-stop", wiki->Name));
        Sleeper::usleep(200000);
    }
    this->ChangeProvider(wiki, new HuggleFeedProviderWiki(wiki));
    wiki->Provider->Start();
    if (this->lIRC.contains(wiki))
        this->lIRC[wiki]->setChecked(false);
}

void Huggle::MainWindow::on_actionInsert_page_to_a_watchlist_triggered()
{
    if (!this->CheckEditableBrowserPage())
        return;

    if (this->CurrentEdit->Page == nullptr)
        throw new Huggle::NullPointerException("this->CurrentEdit->Page",
                                               "void Huggle::MainWindow::on_actionInsert_page_to_a_watchlist_triggered()");
    WikiUtil::Watchlist(this->CurrentEdit->Page);
}

void Huggle::MainWindow::on_actionRemove_page_from_a_watchlist_triggered()
{
    if (!this->CheckEditableBrowserPage())
        return;

    if (this->CurrentEdit->Page == nullptr)
        throw new Huggle::NullPointerException("this->CurrentEdit->Page", "void Huggle::MainWindow::on_actionRemove_page_from_a_watchlist_triggered()");
    WikiUtil::Unwatchlist(this->CurrentEdit->Page);
}

void Huggle::MainWindow::on_actionMy_talk_page_triggered()
{
    if (Configuration::HuggleConfiguration->Restricted)
        return;
    this->LockPage();
    this->Browser->DisplayPage(Configuration::GetProjectWikiURL(this->GetCurrentWikiSite()) +
                                           "User_talk:" +
                                           QUrl::toPercentEncoding(hcfg->SystemConfig_Username));
}

void Huggle::MainWindow::on_actionMy_Contributions_triggered()
{
    if (Configuration::HuggleConfiguration->Restricted)
        return;
    this->LockPage();
    this->Browser->DisplayPage(Configuration::GetProjectWikiURL(this->GetCurrentWikiSite()) +
                                           "Special:Contributions/" +
                               QUrl::toPercentEncoding(hcfg->SystemConfig_Username));
}

void MainWindow::Go()
{
    QAction *action = (QAction*)QObject::sender();
    QDesktopServices::openUrl(QString(Configuration::GetProjectWikiURL() + action->toolTip()));
}

void Huggle::MainWindow::on_actionRevert_only_this_revision_assuming_good_faith_triggered()
{
    this->RevertAgf(true);
}

void Huggle::MainWindow::on_tabWidget_currentChanged(int index)
{
    int in = this->ui->tabWidget->count() - 1;
    if (index == in)
    {
        // we need to create a new browser window
        this->CreateBrowserTab("New tab", in);
        this->CurrentEdit = nullptr;
        this->LockPage();
    } else
    {
        this->Browser = (HuggleWeb*)this->ui->tabWidget->widget(index)->layout()->itemAt(0)->widget();
        if (!this->Browser)
            throw new Huggle::Exception("Invalid browser pointer");

        // we need to change edit to what we have in that tab including all other stuff
        this->CurrentEdit = this->Browser->CurrentEdit;
        this->Render();
    }
}

void Huggle::MainWindow::on_actionClose_current_tab_triggered()
{
    if (this->Browsers.count() < 2)
    {
        Syslog::HuggleLogs->ErrorLog("I can't close this tab because it's last one, you must have at least 1 tab open");
        return;
    }

    // first kill the tab
    HuggleWeb *br = this->Browser;
    // we need to store the id of current tab because we must switch to index 0 before
    // deleting it, otherwise it could jump on + tab which would immediatelly open
    // new tab
    int cu = this->ui->tabWidget->currentIndex();
    this->ui->tabWidget->setCurrentIndex(0);
    this->ui->tabWidget->removeTab(cu);
    int index = 0;
    while (index < this->Browsers.count())
    {
        if (this->Browsers.at(index) == br)
        {
            this->Browsers.removeAt(index);
            break;
        }
        index++;
    }
    delete br;
}

void Huggle::MainWindow::on_actionOpen_new_tab_triggered()
{
    this->CreateBrowserTab("New tab", this->ui->tabWidget->count() - 1);
    this->CurrentEdit = nullptr;
    this->LockPage();
}

void Huggle::MainWindow::on_actionVerbosity_2_triggered()
{
    hcfg->Verbosity += 1;
}

void Huggle::MainWindow::on_actionVerbosity_triggered()
{
    hcfg->Verbosity -= 1;
}
