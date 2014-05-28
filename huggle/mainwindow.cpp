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
#include "configuration.hpp"
#include "reloginform.hpp"
#include "generic.hpp"
#include "gc.hpp"
#include "querypool.hpp"
#include "collectable.hpp"
#include "core.hpp"
#include "wikiutil.hpp"
#include "exception.hpp"
#include "localization.hpp"
#include "syslog.hpp"
#include "ui_mainwindow.h"

using namespace Huggle;
MainWindow *MainWindow::HuggleMain = nullptr;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    this->LastTPRevID = WIKI_UNKNOWN_REVID;
    this->EditLoad = QDateTime::currentDateTime();
    this->Shutdown = ShutdownOpRunning;
    this->EditablePage = false;
    this->ShuttingDown = false;
    this->ui->setupUi(this);
    this->Localize();
    this->Status = new QLabel();
    this->ui->statusBar->addWidget(this->Status);
    this->tb = new HuggleTool();
    this->Queries = new ProcessList(this);
    this->SystemLog = new HuggleLog(this);
    this->Browser = new HuggleWeb(this);
    this->Queue1 = new HuggleQueue(this);
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
    this->preferencesForm = new Preferences(this);
    this->aboutForm = new AboutForm(this);
    this->ui->actionRequest_protection->setEnabled(Configuration::HuggleConfiguration->ProjectConfig_RFPP);
    this->ui->actionDisplay_bot_data->setChecked(Configuration::HuggleConfiguration->UserConfig_HAN_DisplayBots);
    this->ui->actionDisplay_user_data->setChecked(Configuration::HuggleConfiguration->UserConfig_HAN_DisplayUser);
    this->ui->actionDisplay_user_messages->setChecked(Configuration::HuggleConfiguration->UserConfig_HAN_DisplayUserTalk);
    // we store the value in bool so that we don't need to call expensive string function twice
    bool PermissionBlock = Configuration::HuggleConfiguration->Rights.contains("block");
    this->ui->actionBlock_user->setEnabled(PermissionBlock);
    this->ui->actionBlock_user_2->setEnabled(PermissionBlock);
    bool PermissionDelete = Configuration::HuggleConfiguration->Rights.contains("delete");
    this->ui->actionDelete_page->setEnabled(PermissionDelete);
    this->ui->actionDelete->setEnabled(PermissionDelete);
    this->ui->actionProtect->setEnabled(Configuration::HuggleConfiguration->Rights.contains("protect"));
    this->addDockWidget(Qt::LeftDockWidgetArea, this->_History);
    this->SystemLog->resize(100, 80);
    if (!Configuration::HuggleConfiguration->WhiteList.contains(Configuration::HuggleConfiguration->SystemConfig_Username))
    {
        Configuration::HuggleConfiguration->WhiteList.append(Configuration::HuggleConfiguration->SystemConfig_Username);
    }
    QueryPool::HugglePool->Processes = this->Queries;
    this->setWindowTitle("Huggle 3 QT-LX on " + Configuration::HuggleConfiguration->Project->Name);
    this->ui->verticalLayout->addWidget(this->Browser);
    this->DisplayWelcomeMessage();
    if (Configuration::HuggleConfiguration->UserConfig_RemoveOldQueueEdits)
    {
        this->ui->actionRemove_old_edits->setChecked(true);
        this->ui->actionStop_feed->setChecked(false);
    } else
    {
        this->ui->actionRemove_old_edits->setChecked(false);
        this->ui->actionStop_feed->setChecked(true);
    }
    // initialise queues
    if (!Configuration::HuggleConfiguration->ProjectConfig_UseIrc)
    {
        Syslog::HuggleLogs->Log(Localizations::HuggleLocalizations->Localize("irc-not"));
        this->ui->actionReconnect_IRC->setEnabled(false);
        this->ui->actionIRC->setEnabled(false);
    }
    if (Configuration::HuggleConfiguration->UsingIRC && Configuration::HuggleConfiguration->ProjectConfig_UseIrc)
    {
        this->ChangeProvider(new HuggleFeedProviderIRC());
        this->ui->actionIRC->setChecked(true);
        if (!Core::HuggleCore->PrimaryFeedProvider->Start())
        {
            Syslog::HuggleLogs->ErrorLog(Localizations::HuggleLocalizations->Localize("irc-failure"));
            this->ui->actionIRC->setChecked(false);
            this->ui->actionWiki->setChecked(true);
            this->ChangeProvider(new HuggleFeedProviderWiki());
            Core::HuggleCore->PrimaryFeedProvider->Start();
        }
    } else
    {
        this->ui->actionIRC->setChecked(false);
        this->ui->actionWiki->setChecked(true);
        this->ChangeProvider(new HuggleFeedProviderWiki());
        Core::HuggleCore->PrimaryFeedProvider->Start();
    }
    if (Configuration::HuggleConfiguration->ProjectConfig_WarningTypes.count() > 0)
    {
        this->RevertSummaries = new QMenu(this);
        this->WarnMenu = new QMenu(this);
        this->RevertWarn = new QMenu(this);
        int r=0;
        while (r<Configuration::HuggleConfiguration->ProjectConfig_WarningTypes.count())
        {
            QAction *action = new QAction(HuggleParser::GetValueFromKey(Configuration::HuggleConfiguration->ProjectConfig_WarningTypes.at(r)), this);
            QAction *actiona = new QAction(HuggleParser::GetValueFromKey(Configuration::HuggleConfiguration->ProjectConfig_WarningTypes.at(r)), this);
            QAction *actionb = new QAction(HuggleParser::GetValueFromKey(Configuration::HuggleConfiguration->ProjectConfig_WarningTypes.at(r)), this);
            this->RevertWarn->addAction(actiona);
            this->WarnMenu->addAction(actionb);
            this->RevertSummaries->addAction(action);
            r++;
            connect(action, SIGNAL(triggered()), this, SLOT(CustomRevert()));
            connect(actiona, SIGNAL(triggered()), this, SLOT(CustomRevertWarn()));
            connect(actionb, SIGNAL(triggered()), this, SLOT(CustomWarn()));
        }
        this->ui->actionWarn->setMenu(this->WarnMenu);
        this->ui->actionRevert->setMenu(this->RevertSummaries);
        this->ui->actionRevert_and_warn->setMenu(this->RevertWarn);
    }
    this->tabifyDockWidget(this->SystemLog, this->Queries);
    this->GeneralTimer = new QTimer(this);
    this->ui->actionTag_2->setVisible(false);
    connect(this->GeneralTimer, SIGNAL(timeout()), this, SLOT(OnMainTimerTick()));
    this->GeneralTimer->start(200);
    QFile *layout;
    if (QFile().exists(Configuration::GetConfigurationPath() + "mainwindow_state"))
    {
        Syslog::HuggleLogs->DebugLog("Loading state");
        layout = new QFile(Configuration::GetConfigurationPath() + "mainwindow_state");
        if (!layout->open(QIODevice::ReadOnly))
        {
            Syslog::HuggleLogs->ErrorLog("Unable to read state from a config file");
        } else
        {
            if (!this->restoreState(layout->readAll()))
            {
                Syslog::HuggleLogs->DebugLog("Failed to restore state");
            }
        }
        layout->close();
        delete layout;
    }
    if (QFile().exists(Configuration::GetConfigurationPath() + "mainwindow_geometry"))
    {
        Syslog::HuggleLogs->DebugLog("Loading geometry");
        layout = new QFile(Configuration::GetConfigurationPath() + "mainwindow_geometry");
        if (!layout->open(QIODevice::ReadOnly))
        {
            Syslog::HuggleLogs->ErrorLog("Unable to read geometry from a config file");
        } else
        {
            if (!this->restoreGeometry(layout->readAll()))
            {
                Syslog::HuggleLogs->DebugLog("Failed to restore layout");
            }
        }
        layout->close();
        delete layout;
    }
    this->showMaximized();
    // these controls are for debugging only
    if (Configuration::HuggleConfiguration->Verbosity == 0)
    {
        QAction *debugm = this->ui->menuDebug_2->menuAction();
        this->ui->menuHelp->removeAction(debugm);
    }
    Hooks::MainWindowIsLoaded(this);
    this->VandalDock->Connect();
    this->tCheck = new QTimer(this);
    connect(this->tCheck, SIGNAL(timeout()), this, SLOT(TimerCheckTPOnTick()));
    this->tCheck->start(20000);
}

MainWindow::~MainWindow()
{
    delete this->OnNext_EvPage;
    delete this->fSpeedyDelete;
    delete this->wUserInfo;
    delete this->wHistory;
    delete this->wlt;
    delete this->fWaiting;
    delete this->VandalDock;
    delete this->_History;
    delete this->RevertWarn;
    delete this->WarnMenu;
    delete this->fProtectForm;
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
    delete this->Browser;
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
        Syslog::HuggleLogs->ErrorLog(Localizations::HuggleLocalizations->Localize("report-duplicate"));
        return;
    }
    if (!Configuration::HuggleConfiguration->ProjectConfig_AIV)
    {
        QMessageBox mb;
        mb.setText(Localizations::HuggleLocalizations->Localize("missing-aiv"));
        mb.setWindowTitle(Localizations::HuggleLocalizations->Localize("function-miss"));
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
        this->qNext->DecRef();
        this->qNext = nullptr;
    }
    if (e->Page == nullptr || e->User == NULL)
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
                WikiEdit *latest = CurrentEdit;
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
    if (!KeepUser)
    {
        this->wUserInfo->ChangeUser(e->User);
        if (Configuration::HuggleConfiguration->UserConfig_HistoryLoad)
            this->wUserInfo->Read();
    }
    if (!KeepHistory)
    {
        this->wHistory->Update(e);
        if (Configuration::HuggleConfiguration->UserConfig_HistoryLoad)
            this->wHistory->Read();
    }
    this->CurrentEdit = e;
    this->EditLoad = QDateTime::currentDateTime();
    this->Browser->DisplayDiff(e);
    this->Render();
    e->DecRef();
}

void MainWindow::Render()
{
    if (this->CurrentEdit != nullptr)
    {
        if (this->CurrentEdit->Page == nullptr)
            throw new Exception("Page of CurrentEdit can't be nullptr at MainWindow::Render()");

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
        this->tb->SetInfo(Localizations::HuggleLocalizations->Localize("browser-diff", params));
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
    while (x < this->PatroledEdits.count())
    {
        ApiQuery *query = this->PatroledEdits.at(x);
        // check if this query has actually some edit associated to it
        if (query->CallbackResult == nullptr)
        {
            // we really don't want to mess up with this
            query->UnregisterConsumer("patrol");
            // get rid of it
            this->PatroledEdits.removeAt(x);
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
                if (element.attributes().contains("patroltoken"))
                {
                    // we can finish this
                    QString token = element.attribute("patroltoken");
                    edit->PatrolToken = token;
                    this->PatrolThis(edit);
                    // get rid of it
                    this->PatroledEdits.removeAt(x);
                    edit->UnregisterConsumer("patrol");
                    query->CallbackResult = nullptr;
                    query->UnregisterConsumer("patrol");
                    continue;
                }
            }
            // this edit is fucked up
            Syslog::HuggleLogs->DebugLog("Unable to retrieve token for " + edit->Page->PageName);
            // get rid of it
            this->PatroledEdits.removeAt(x);
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
    /// \todo LOCALIZE ME
    QString t = "Processing <b>" + Generic::ShrinkText(QString::number(QueryPool::HugglePool->ProcessingEdits.count()), 3) +
                "</b>edits and <b>" + Generic::ShrinkText(QString::number(QueryPool::HugglePool->RunningQueriesGetCount()), 3) +
                "</b>queries. Whitelisted users: <b>" + QString::number(Configuration::HuggleConfiguration->WhiteList.size()) +
                "</b> Queue size: <b>" + Generic::ShrinkText(QString::number(this->Queue1->Items.count()), 4) +
                "</b> Statistics: ";
    // calculate stats, but not if huggle uptime is lower than 50 seconds
    double Uptime = Core::HuggleCore->PrimaryFeedProvider->GetUptime();
    if (this->ShuttingDown)
    {
        t += " none";
    } else if (Uptime < 50)
    {
        t += " waiting for more edits";
    } else
    {
        double EditsPerMinute = 0;
        double RevertsPerMinute = 0;
        if (Core::HuggleCore->PrimaryFeedProvider->EditCounter > 0)
            EditsPerMinute = Core::HuggleCore->PrimaryFeedProvider->EditCounter / (Uptime / 60);
        if (Core::HuggleCore->PrimaryFeedProvider->RvCounter > 0)
            RevertsPerMinute = Core::HuggleCore->PrimaryFeedProvider->RvCounter / (Uptime / 60);
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
        // make the numbers easier to read
        EditsPerMinute = ((double)qRound(EditsPerMinute * 100)) / 100;
        RevertsPerMinute = ((double)qRound(RevertsPerMinute * 100)) / 100;
        VandalismLevel = ((double)qRound(VandalismLevel * 100)) / 100;
        t += " <font color=" + color + ">" + Generic::ShrinkText(QString::number(EditsPerMinute), 6) +
             " edits per minute " + Generic::ShrinkText(QString::number(RevertsPerMinute), 6) +
             " reverts per minute, level " + Generic::ShrinkText(QString::number(VandalismLevel), 8) + "</font>";
    }
    if (Configuration::HuggleConfiguration->Verbosity > 0)
        t += " QGC: " + QString::number(GC::gc->list.count()) + " U: " + QString::number(WikiUser::ProblematicUsers.count());
    this->Status->setText(t);
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

RevertQuery *MainWindow::Revert(QString summary, bool nd, bool next)
{
    bool rollback = true;
    if (this->CurrentEdit == nullptr)
    {
        Syslog::HuggleLogs->ErrorLog(Localizations::HuggleLocalizations->Localize("main-revert-null"));
        return nullptr;
    }
    if (this->CurrentEdit->NewPage)
    {
        QMessageBox mb;
        mb.setWindowTitle("Can't revert this");
        mb.setText("This is a new page, so it can't be reverted, you can either tag it, or delete it.");
        mb.exec();
        return nullptr;
    }
    if (!this->CurrentEdit->IsPostProcessed())
    {
        // This shouldn't ever happen, there is no need to translate this message
        // becase it's nearly impossible to be ever displayed
        Syslog::HuggleLogs->ErrorLog("This edit is still being processed, please wait");
        return nullptr;
    }
    if (this->CurrentEdit->RollbackToken.size() < 1)
    {
        Syslog::HuggleLogs->WarningLog(Localizations::HuggleLocalizations->Localize("main-revert-manual", this->CurrentEdit->Page->PageName));
        rollback = false;
    }
    if (this->PreflightCheck(this->CurrentEdit))
    {
        this->CurrentEdit->User->Resync();
        this->CurrentEdit->User->SetBadnessScore(this->CurrentEdit->User->GetBadnessScore(false) - 10);
        Hooks::OnRevert(this->CurrentEdit);
        RevertQuery *q = WikiUtil::RevertEdit(this->CurrentEdit, summary, false, rollback, nd);
        if (Configuration::HuggleConfiguration->SystemConfig_InstantReverts)
        {
            q->Process();
        } else
        {
            q->Date = QDateTime::currentDateTime();
            q->IncRef();
            this->RevertStack.append(q);
        }
        if (next)
            this->DisplayNext(q);
        return q;
    }
    return nullptr;
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
    if (Configuration::HuggleConfiguration->WarnUserSpaceRoll && _e->Page->IsUserpage())
    {
        Warn = true;
        type = "in userspace";
    } else if (Configuration::HuggleConfiguration->ProjectConfig_ConfirmOnSelfRevs
               &&(_e->User->Username.toLower() == Configuration::HuggleConfiguration->SystemConfig_Username.toLower()))
    {
        type = "made by you";
        Warn = true;
    } else if (Configuration::HuggleConfiguration->ProjectConfig_ConfirmTalk && _e->Page->IsTalk())
    {
        type = "made on talk page";
        Warn = true;
    } else if (Configuration::HuggleConfiguration->ProjectConfig_ConfirmWL && _e->User->IsWhitelisted())
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
    if (item.contains(";"))
    {
        QString type = item.mid(0, item.indexOf(";"));
        int c=0;
        while(c < Configuration::HuggleConfiguration->ProjectConfig_WarningTypes.count())
        {
            QString x = Configuration::HuggleConfiguration->ProjectConfig_WarningTypes.at(c);
            if (x.startsWith(type + ";"))
            {
                x = Configuration::HuggleConfiguration->ProjectConfig_WarningTypes.at(c);
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
    WikiPage *welcome = new WikiPage(Configuration::HuggleConfiguration->WelcomeMP);
    this->Browser->DisplayPreFormattedPage(welcome);
    this->LockPage();
    delete welcome;
    this->Render();
}

void MainWindow::FinishRestore()
{
    if (this->RestoreEdit == nullptr || this->RestoreQuery == NULL || !this->RestoreQuery->IsProcessed())
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
            this->RestoreQuery->DecRef();
            this->RestoreQuery = nullptr;
            Huggle::Syslog::HuggleLogs->ErrorLog("Unable to restore the revision, because there is no text available for it");
            this->RestoreEdit->DecRef();
            this->RestoreEdit = nullptr;
            return;
        }
    }
    // get last id
    if (page.count() > 0)
    {
        QDomElement e = page.at(0).toElement();
        QString text = e.text();
        if (!text.size())
        {
            this->RestoreQuery->DecRef();
            this->RestoreQuery = nullptr;
            Huggle::Syslog::HuggleLogs->Log("Unable to restore the revision, because there is no text available for it");
            this->RestoreEdit->DecRef();
            this->RestoreEdit = nullptr;
            return;
        }
        QString sm = Configuration::HuggleConfiguration->ProjectConfig_RestoreSummary;
        sm = sm.replace("$1", QString::number(this->RestoreEdit->RevID));
        sm = sm.replace("$2", this->RestoreEdit->User->Username);
        sm = sm.replace("$3", this->RestoreEdit_RevertReason);
        WikiUtil::EditPage(this->RestoreEdit->Page, text, sm)->DecRef();
    } else
    {
        Syslog::HuggleLogs->DebugLog(this->RestoreQuery->Result->Data);
        Syslog::HuggleLogs->ErrorLog("Unable to restore the revision because wiki provided no data for selected version");
    }
    this->RestoreQuery->DecRef();
    this->RestoreQuery = nullptr;
    this->RestoreEdit->DecRef();
    this->RestoreEdit = nullptr;
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
    if (Configuration::HuggleConfiguration->UserConfig_ManualWarning)
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
    QDesktopServices::openUrl(Configuration::HuggleConfiguration->GlobalConfig_DocumentationPath);
}

void MainWindow::on_actionAbout_triggered()
{
    this->aboutForm->show();
}

void MainWindow::OnMainTimerTick()
{
    this->ProcessReverts();
    WikiUtil::FinalizeMessages();
    bool RetrieveEdit = true;
    // if garbage collector is already destroyed there is no point in doing anything in here
    if (GC::gc == nullptr)
        return;
    GC::gc->DeleteOld();
    // if there is no working feed, let's try to fix it
    if (Core::HuggleCore->PrimaryFeedProvider->IsWorking() != true && this->ShuttingDown != true)
    {
        Syslog::HuggleLogs->Log(Localizations::HuggleLocalizations->Localize("provider-failure"));
        if (!Core::HuggleCore->PrimaryFeedProvider->Restart())
        {
            this->ChangeProvider(new HuggleFeedProviderWiki());
            Core::HuggleCore->PrimaryFeedProvider->Start();
        }
    }
    Warnings::ResendWarnings();
    // check if queue isn't full
    if (this->Queue1->Items.count() > Configuration::HuggleConfiguration->SystemConfig_QueueSize)
    {
        if (this->ui->actionStop_feed->isChecked())
        {
            Core::HuggleCore->PrimaryFeedProvider->Pause();
            RetrieveEdit = false;
        } else
        {
            if (Core::HuggleCore->PrimaryFeedProvider->IsPaused())
                Core::HuggleCore->PrimaryFeedProvider->Resume();
            this->Queue1->Trim();
        }
    } else
    {
        if (this->ui->actionStop_feed->isChecked() && Core::HuggleCore->PrimaryFeedProvider->IsPaused())
            Core::HuggleCore->PrimaryFeedProvider->Resume();
    }
    if (RetrieveEdit && Core::HuggleCore->PrimaryFeedProvider->ContainsEdit())
    {
        // we take the edit and start post processing it
        WikiEdit *edit = Core::HuggleCore->PrimaryFeedProvider->RetrieveEdit();
        if (edit != nullptr)
        {
            QueryPool::HugglePool->PostProcessEdit(edit);
            edit->RegisterConsumer(HUGGLECONSUMER_MAINPEND);
            edit->DecRef();
            this->PendingEdits.append(edit);
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
    Syslog::HuggleLogs->lUnwrittenLogs.lock();
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
    Syslog::HuggleLogs->lUnwrittenLogs.unlock();
    this->Queries->RemoveExpired();
    if (this->OnNext_EvPage != nullptr && this->qNext != NULL && this->qNext->IsProcessed())
    {
        this->tb->SetPage(this->OnNext_EvPage);
        this->tb->RenderEdit();
        this->qNext->DecRef();
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
        if (Huggle::Configuration::HuggleConfiguration->UserConfig_DeleteEditsAfterRevert)
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
        if (this->Shutdown == ShutdownOpRetrievingWhitelist)
        {
            this->Shutdown = ShutdownOpUpdatingWhitelist;
            this->fWaiting->Status(60, Localizations::HuggleLocalizations->Localize("updating-wl"));
            this->wq = new WLQuery();
            this->wq->IncRef();
            this->wq->Type = WLQueryType_WriteWL;
            this->wq->Process();
            return;
        }
        if (this->Shutdown == ShutdownOpUpdatingWhitelist)
        {
            if (!this->wq->IsProcessed())
            {
                this->fWaiting->Status(60 + int((this->wq->Progress / 100) * 30));
                return;
            }
            // we finished writing the wl
            this->wq->DecRef();
            this->fWaiting->Status(90, Localizations::HuggleLocalizations->Localize("updating-uc"));
            this->wq = nullptr;
            this->Shutdown = ShutdownOpUpdatingConf;
            QString page = Configuration::HuggleConfiguration->GlobalConfig_UserConf;
            page = page.replace("$1", Configuration::HuggleConfiguration->SystemConfig_Username);
            WikiPage *uc = new WikiPage(page);
            this->eq = WikiUtil::EditPage(uc, Configuration::MakeLocalUserConfig(), "Writing user config", true);
            delete uc;
            return;
        }
    } else
    {
        // we need to check if config was written
        if (!this->eq->IsProcessed())
            return;
        this->eq->DecRef();
        this->eq = nullptr;
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
    if (!this->CheckExit() || !this->CheckEditableBrowserPage())
        return;

    if (Configuration::HuggleConfiguration->Restricted)
    {
        Generic::DeveloperError();
        return;
    }
    this->Revert();
}

void MainWindow::on_actionWarn_the_user_triggered()
{
    this->TriggerWarn();
}

void MainWindow::on_actionRevert_currently_displayed_edit_and_warn_the_user_triggered()
{
    if (!this->CheckExit() || !this->CheckEditableBrowserPage())
        return;
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Generic::DeveloperError();
        return;
    }
    RevertQuery *result = this->Revert("", true, false);
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
    if (!this->CheckExit() || !this->CheckEditableBrowserPage())
        return;

    if (Configuration::HuggleConfiguration->Restricted)
    {
        Generic::DeveloperError();
        return;
    }
    RevertQuery *result = this->Revert("", true, false);
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
    if (!this->CheckExit() || !this->CheckEditableBrowserPage())
        return;
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Generic::DeveloperError();
        return;
    }
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
    if (this->CurrentEdit == nullptr || this->CurrentEdit->Next == NULL)
        return;
    this->ProcessEdit(this->CurrentEdit->Next, true);
}

void MainWindow::on_actionBack_triggered()
{
    if (this->CurrentEdit == nullptr || this->CurrentEdit->Previous == NULL)
        return;
    this->ProcessEdit(this->CurrentEdit->Previous, true);
}

void MainWindow::CustomRevert()
{
    if (!this->CheckExit() || !this->CheckEditableBrowserPage())
        return;
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Generic::DeveloperError();
        return;
    }
    QAction *revert = (QAction*) QObject::sender();
    QString k = HuggleParser::GetKeyOfWarningTypeFromWarningName(revert->text());
    QString rs = HuggleParser::GetSummaryOfWarningTypeFromWarningKey(k);
    rs = Huggle::Configuration::HuggleConfiguration->GenerateSuffix(rs);
    this->Revert(rs);
}

void MainWindow::CustomRevertWarn()
{
    if (!this->CheckExit() || !this->CheckEditableBrowserPage())
        return;
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Generic::DeveloperError();
        return;
    }
    QAction *revert = (QAction*) QObject::sender();
    QString k = HuggleParser::GetKeyOfWarningTypeFromWarningName(revert->text());
    QString rs = HuggleParser::GetSummaryOfWarningTypeFromWarningKey(k);
    rs = Huggle::Configuration::HuggleConfiguration->GenerateSuffix(rs);
    RevertQuery *result = this->Revert(rs, true, false);
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
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Generic::DeveloperError();
        return;
    }
    QAction *revert = (QAction*) QObject::sender();
    QString k = HuggleParser::GetKeyOfWarningTypeFromWarningName(revert->text());
    this->Warn(k, nullptr);
}

QString MainWindow::GetSummaryText(QString text)
{
    int id=0;
    while (id<Configuration::HuggleConfiguration->ProjectConfig_RevertSummaries.count())
    {
        if (text == this->GetSummaryKey(Configuration::HuggleConfiguration->ProjectConfig_RevertSummaries.at(id)))
        {
            QString data = Configuration::HuggleConfiguration->ProjectConfig_RevertSummaries.at(id);
            if (data.contains(";"))
            {
                data = data.mid(data.indexOf(";") + 1);
            }
            return data;
        }
        id++;
    }
    return Configuration::HuggleConfiguration->ProjectConfig_DefaultSummary;
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
        Core::HuggleCore->Shutdown();
        return;
    }
    this->Shutdown = ShutdownOpRetrievingWhitelist;
    if (Core::HuggleCore->PrimaryFeedProvider != nullptr)
        Core::HuggleCore->PrimaryFeedProvider->Stop();
    if (this->fWaiting != nullptr)
        delete this->fWaiting;
    this->fWaiting = new WaitingForm(this);
    this->fWaiting->show();
    this->fWaiting->Status(10, Localizations::HuggleLocalizations->Localize("whitelist-download"));
    this->wlt = new QTimer(this);
    connect(this->wlt, SIGNAL(timeout()), this, SLOT(OnTimerTick0()));
    this->wlt->start(800);
}

void MainWindow::ReconnectIRC()
{
    if (!this->CheckExit())
        return;
    if (!Configuration::HuggleConfiguration->UsingIRC)
    {
        Syslog::HuggleLogs->Log(Localizations::HuggleLocalizations->Localize("irc-not"));
        return;
    }
    Syslog::HuggleLogs->Log(Localizations::HuggleLocalizations->Localize("irc-connecting"));
    Core::HuggleCore->PrimaryFeedProvider->Stop();
    while (!Core::HuggleCore->PrimaryFeedProvider->IsStopped())
    {
        Syslog::HuggleLogs->Log(Localizations::HuggleLocalizations->Localize("irc-ws"));
        Sleeper::usleep(200000);
    }
    this->ui->actionIRC->setChecked(true);
    this->ui->actionWiki->setChecked(false);
    this->ChangeProvider(new HuggleFeedProviderIRC());
    if (!Core::HuggleCore->PrimaryFeedProvider->Start())
    {
        this->ui->actionIRC->setChecked(false);
        this->ui->actionWiki->setChecked(true);
        Syslog::HuggleLogs->ErrorLog(Localizations::HuggleLocalizations->Localize("provider-primary-failure"));
        this->ChangeProvider(new HuggleFeedProviderWiki());
        Core::HuggleCore->PrimaryFeedProvider->Start();
    }
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
        mb.setText(Localizations::HuggleLocalizations->Localize("main-no-page"));
        mb.exec();
        return false;
    }
    if (Configuration::HuggleConfiguration->SystemConfig_RequestDelay)
    {
        qint64 wt = QDateTime::currentDateTime().msecsTo(this->EditLoad.addSecs(Configuration::HuggleConfiguration->SystemConfig_DelayVal));
        if (wt > 0)
        {
            Syslog::HuggleLogs->WarningLog("Ignoring edit request because you are too fast, please wait " +
                                           QString::number(wt)+ " ms");
            return false;
        }
    }
    return true;
}

void MainWindow::SuspiciousEdit()
{
    if (!this->CheckExit() || !CheckEditableBrowserPage())
        return;
    if (this->CurrentEdit != nullptr)
    {
        Hooks::Suspicious(this->CurrentEdit);
        WLQuery *wq_ = new WLQuery();
        wq_->Type = WLQueryType_SuspWL;
        wq_->Parameters = "page=" + QUrl::toPercentEncoding(this->CurrentEdit->Page->PageName) + "&wiki="
                          + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->Project->WhiteList) + "&user="
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
    if (e == nullptr)
        e = this->CurrentEdit;
    if (e == nullptr || !Configuration::HuggleConfiguration->ProjectConfig_Patrolling)
        return;
    ApiQuery *query = nullptr;
    // if this edit doesn't have the patrol token we need to get one
    if (!e->PatrolToken.size())
    {
        // register consumer so that gc doesn't delete this edit meanwhile
        e->RegisterConsumer("patrol");
        query = new ApiQuery();
        query->SetAction(ActionTokens);
        query->Target = "Retrieving patrol token for " + e->Page->PageName;
        query->Parameters = "type=patrol";
        // this uggly piece of code actually rocks
        query->CallbackResult = (void*)e;
        query->RegisterConsumer("patrol");
        QueryPool::HugglePool->AppendQuery(query);
        query->Process();
        this->PatroledEdits.append(query);
        return;
    }
    // we can execute patrol now
    query = new ApiQuery();
    query->SetAction(ActionPatrol);
    query->Target = "Patrolling " + e->Page->PageName;
    query->UsingPOST = true;
    query->Parameters = "revid=" + QString::number(e->RevID) + "&token=" + QUrl::toPercentEncoding(e->PatrolToken);
    QueryPool::HugglePool->AppendQuery(query);
    Syslog::HuggleLogs->DebugLog("Patrolling " + e->Page->PageName);
    query->Process();
}

void MainWindow::Localize()
{
    this->ui->menuPage->setTitle(Localizations::HuggleLocalizations->Localize("main-page"));
    this->ui->menuHelp->setTitle(Localizations::HuggleLocalizations->Localize("main-help"));
    this->ui->menuUser->setTitle(Localizations::HuggleLocalizations->Localize("main-user"));
    this->ui->menuQueue->setTitle(Localizations::HuggleLocalizations->Localize("main-queue"));
    this->ui->menuFile->setTitle(Localizations::HuggleLocalizations->Localize("main-system"));
    this->ui->menuHAN->setTitle(Localizations::HuggleLocalizations->Localize("main-han"));
    this->ui->actionAbout->setText(Localizations::HuggleLocalizations->Localize("main-help-about"));
    this->ui->actionBack->setText(Localizations::HuggleLocalizations->Localize("main-browser-back"));
    this->ui->actionBlock_user->setText(Localizations::HuggleLocalizations->Localize("main-user-block"));
    this->ui->actionClear_talk_page_of_user->setText(Localizations::HuggleLocalizations->Localize("main-user-clear-talk"));
    this->ui->actionDelete->setText(Localizations::HuggleLocalizations->Localize("main-page-delete"));
    this->ui->actionExit->setText(Localizations::HuggleLocalizations->Localize("main-system-exit"));
    this->ui->menuTools->setTitle(Localizations::HuggleLocalizations->Localize("main-tools"));
    this->ui->actionShow_ignore_list_of_current_wiki->setText(Localizations::HuggleLocalizations->Localize("main-tools-il"));
    this->ui->actionDisplay_a_session_data->setText(Localizations::HuggleLocalizations->Localize("main-tools-sess"));
    this->ui->actionClear->setText(Localizations::HuggleLocalizations->Localize("main-queue-clear"));
    this->ui->actionClear_talk_page_of_user->setText(Localizations::HuggleLocalizations->Localize("main-user-clear-tp"));
    this->ui->actionDecrease_badness_score_by_20->setText(Localizations::HuggleLocalizations->Localize("main-user-db"));
    this->ui->actionDelete->setText(Localizations::HuggleLocalizations->Localize("main-page-delete"));
    this->ui->actionDelete_page->setText(Localizations::HuggleLocalizations->Localize("main-page-delete"));
    this->ui->actionDisplay_a_session_data->setText(Localizations::HuggleLocalizations->Localize("main-display-session-data"));
    this->ui->actionDisplay_whitelist->setText(Localizations::HuggleLocalizations->Localize("main-display-whitelist"));
    this->ui->actionDisplay_history_in_browser->setText(Localizations::HuggleLocalizations->Localize("main-page-historypage"));
    this->ui->actionDisplay_this_page_in_browser->setText(Localizations::HuggleLocalizations->Localize("main-browser-open"));
    this->ui->actionFeedback->setText(Localizations::HuggleLocalizations->Localize("main-help-feedback"));
    this->ui->actionReport_user->setText(Localizations::HuggleLocalizations->Localize("main-user-report"));
    this->ui->actionUser_contributions->setText(Localizations::HuggleLocalizations->Localize("main-tip-contribs"));
    this->ui->actionPreferences->setText(Localizations::HuggleLocalizations->Localize("main-system-options"));
    this->ui->actionReconnect_IRC->setText(Localizations::HuggleLocalizations->Localize("main-system-reconnectirc"));
    this->ui->actionShow_talk->setText(Localizations::HuggleLocalizations->Localize("main-goto-mytalk"));
    this->ui->actionAbort_2->setText(Localizations::HuggleLocalizations->Localize("main-system-abort"));
    this->ui->menuChange_provider->setTitle(Localizations::HuggleLocalizations->Localize("main-system-change-provider"));
    this->ui->actionIRC->setText(Localizations::HuggleLocalizations->Localize("main-system-change-provider-irc"));
    this->ui->actionConnect->setText(Localizations::HuggleLocalizations->Localize("main-han-connect"));
    this->ui->actionDisconnect->setText(Localizations::HuggleLocalizations->Localize("main-han-disconnect"));
    this->ui->actionDisplay_user_messages->setText(Localizations::HuggleLocalizations->Localize("main-han-display-user-messages"));
    this->ui->actionDisplay_user_data->setText(Localizations::HuggleLocalizations->Localize("main-han-display-user-data"));
    this->ui->actionDisplay_bot_data->setText(Localizations::HuggleLocalizations->Localize("main-han-display-bot-data"));
    this->ui->actionWiki->setText(Localizations::HuggleLocalizations->Localize("main-system-change-provider-wiki"));
    this->ui->actionNext->setText(Localizations::HuggleLocalizations->Localize("main-queue-next"));
}

void MainWindow::_BlockUser()
{
    if (!this->CheckExit() || !this->CheckEditableBrowserPage())
        return;
    if(this->CurrentEdit == nullptr)
    {
        Syslog::HuggleLogs->ErrorLog(Localizations::HuggleLocalizations->Localize("block-none"));
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
    switch(Configuration::HuggleConfiguration->UserConfig_GoNext)
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
            if (this->qNext != nullptr)
                this->qNext->DecRef();
            this->OnNext_EvPage = new WikiPage(this->CurrentEdit->Page);
            this->qNext = q;
            this->qNext->IncRef();
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

void MainWindow::LockPage()
{
    this->EditablePage = false;
}

bool MainWindow::CheckExit()
{
    if (this->ShuttingDown)
    {
        QMessageBox mb;
        mb.setWindowTitle(Localizations::HuggleLocalizations->Localize("error"));
        /// \todo LOCALIZE ME
        mb.setText("Huggle is shutting down, ignored");
        mb.exec();
        return false;
    }
    return true;
}

void MainWindow::Welcome()
{
    if (!this->CheckExit() || this->CurrentEdit == nullptr)
        return;
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Generic::DeveloperError();
        return;
    }
    this->CurrentEdit->User->Resync();
    bool create_only = true;
    if (this->CurrentEdit->User->TalkPage_GetContents().size() > 0)
    {
        if (QMessageBox::question(this, "Welcome :o", Localizations::HuggleLocalizations->Localize("welcome-tp-empty-fail"),
                                  QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
            return;
        else
            create_only = false;
    } else if (!this->CurrentEdit->User->TalkPage_WasRetrieved())
    {
        if (QMessageBox::question(this, "Welcome :o", Localizations::HuggleLocalizations->Localize("welcome-page-miss-fail"),
                                  QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
            return;
    }
    if (this->CurrentEdit->User->IsIP())
    {
        if (this->CurrentEdit->User->TalkPage_GetContents().size() == 0)
        {
            // write something to talk page so that we don't welcome this user twice
            this->CurrentEdit->User->TalkPage_SetContents(Configuration::HuggleConfiguration->ProjectConfig_WelcomeAnon);
        }
        WikiUtil::MessageUser(this->CurrentEdit->User, Configuration::HuggleConfiguration->ProjectConfig_WelcomeAnon,
                              Configuration::HuggleConfiguration->ProjectConfig_WelcomeTitle,
                              Configuration::HuggleConfiguration->ProjectConfig_WelcomeSummary,
                              false, nullptr, false, false, true, this->CurrentEdit->TPRevBaseTime, create_only);
        return;
    }
    if (Configuration::HuggleConfiguration->ProjectConfig_WelcomeTypes.count() == 0)
    {
        // This error should never happen so we don't need to localize this
        Syslog::HuggleLogs->Log("There are no welcome messages defined for this project");
        return;
    }
    QString message = HuggleParser::GetValueFromKey(Configuration::HuggleConfiguration->ProjectConfig_WelcomeTypes.at(0));
    if (!message.size())
    {
        // This error should never happen so we don't need to localize this
        Syslog::HuggleLogs->ErrorLog("Invalid welcome template, ignored message");
        return;
    }
    // write something to talk page so that we don't welcome this user twice
    this->CurrentEdit->User->TalkPage_SetContents(message);
    WikiUtil::MessageUser(this->CurrentEdit->User, message, Configuration::HuggleConfiguration->ProjectConfig_WelcomeTitle,
                          Configuration::HuggleConfiguration->ProjectConfig_WelcomeSummary, false, nullptr,
                          false, false, true, this->CurrentEdit->TPRevBaseTime, create_only);
}

void MainWindow::ChangeProvider(HuggleFeed *provider)
{
    if (HuggleFeed::PrimaryFeedProvider != nullptr)
        delete HuggleFeed::PrimaryFeedProvider;
    HuggleFeed::PrimaryFeedProvider = provider;
    Core::HuggleCore->PrimaryFeedProvider = provider;
}

void MainWindow::on_actionWelcome_user_triggered()
{
    this->Welcome();
}

void MainWindow::on_actionOpen_in_a_browser_triggered()
{
    if (this->CurrentEdit != nullptr)
    {
        QDesktopServices::openUrl(QString(Configuration::GetProjectWikiURL() + QUrl::toPercentEncoding(this->CurrentEdit->Page->PageName)));
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
    if (this->CurrentEdit != nullptr)
    {
        this->CurrentEdit->User->SetBadnessScore(this->CurrentEdit->User->GetBadnessScore() - 200);
        Hooks::OnGood(this->CurrentEdit);
        this->PatrolThis();
        if (Configuration::HuggleConfiguration->ProjectConfig_WelcomeGood && !this->CurrentEdit->User->TalkPage_GetContents().size())
            this->Welcome();
    }
    this->DisplayNext();
}

void MainWindow::on_actionUser_contributions_triggered()
{
    if (this->CurrentEdit != nullptr)
    {
        QDesktopServices::openUrl(QString(Configuration::GetProjectWikiURL() + "Special:Contributions/"
                                          + QUrl::toPercentEncoding(this->CurrentEdit->User->Username)));
    }
}

void MainWindow::on_actionTalk_page_triggered()
{
    this->DisplayTalk();
}

void MainWindow::on_actionFlag_as_a_good_edit_triggered()
{
    if (!this->CheckExit() || !this->CheckEditableBrowserPage())
        return;
    if (this->CurrentEdit != nullptr)
    {
        Hooks::OnGood(this->CurrentEdit);
        this->PatrolThis();
        this->CurrentEdit->User->SetBadnessScore(this->CurrentEdit->User->GetBadnessScore() - 200);
        if (Configuration::HuggleConfiguration->ProjectConfig_WelcomeGood && this->CurrentEdit->User->TalkPage_GetContents() == "")
            this->Welcome();
    }
    this->DisplayNext();
}

void MainWindow::on_actionDisplay_this_page_in_browser_triggered()
{
    if (this->CurrentEdit != nullptr)
    {
        if (this->CurrentEdit->Diff > 0)
            QDesktopServices::openUrl(QString(Configuration::GetProjectScriptURL() + "index.php?diff=" + QString::number(this->CurrentEdit->Diff)));
        else
            QDesktopServices::openUrl(QString(Configuration::GetProjectWikiURL() + this->CurrentEdit->Page->PageName));
    }
}

void MainWindow::on_actionEdit_page_in_browser_triggered()
{
    if (this->CurrentEdit != nullptr)
        QDesktopServices::openUrl(QString(Configuration::GetProjectWikiURL() + this->CurrentEdit->Page->PageName + "?action=edit"));
}

void MainWindow::on_actionDisplay_history_in_browser_triggered()
{
    if (this->CurrentEdit != nullptr)
        QDesktopServices::openUrl(QString(Configuration::GetProjectWikiURL() + this->CurrentEdit->Page->PageName + "?action=history"));
}

void MainWindow::on_actionStop_feed_triggered()
{
    Configuration::HuggleConfiguration->UserConfig_RemoveOldQueueEdits = false;
    this->ui->actionRemove_old_edits->setChecked(false);
    this->ui->actionStop_feed->setChecked(true);
}

void MainWindow::on_actionRemove_old_edits_triggered()
{
    Configuration::HuggleConfiguration->UserConfig_RemoveOldQueueEdits = true;
    this->ui->actionRemove_old_edits->setChecked(true);
    this->ui->actionStop_feed->setChecked(false);
}

void MainWindow::on_actionClear_talk_page_of_user_triggered()
{
    if (!this->CheckExit() || !this->CheckEditableBrowserPage() || this->CurrentEdit == nullptr)
        return;
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Generic::DeveloperError();
        return;
    }
    if (!this->CurrentEdit->User->IsIP())
    {
        Syslog::HuggleLogs->Log(Localizations::HuggleLocalizations->Localize("feature-nfru"));
        return;
    }
    WikiPage *page = new WikiPage(this->CurrentEdit->User->GetTalk());
    /// \todo LOCALIZE ME
    WikiUtil::EditPage(page, Configuration::HuggleConfiguration->ProjectConfig_ClearTalkPageTemp
                   + "\n" + Configuration::HuggleConfiguration->ProjectConfig_WelcomeAnon,
                   "Cleaned old templates from talk page " + Configuration::HuggleConfiguration->ProjectConfig_EditSuffixOfHuggle)->DecRef();
    delete page;
}

void MainWindow::on_actionList_all_QGC_items_triggered()
{
    int xx=0;
    while (xx<GC::gc->list.count())
    {
        Collectable *query = GC::gc->list.at(xx);
        Syslog::HuggleLogs->Log(query->DebugHgc());
        xx++;
    }
}

void MainWindow::on_actionRevert_currently_displayed_edit_warn_user_and_stay_on_page_triggered()
{
    if (!this->CheckExit() || !this->CheckEditableBrowserPage())
        return;
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Generic::DeveloperError();
        return;
    }
    RevertQuery *result = this->Revert("", false, false);
    if (result != nullptr)
    {
        this->Warn("warning", result);
    }
}

void MainWindow::on_actionRevert_currently_displayed_edit_and_stay_on_page_triggered()
{
    if (!this->CheckExit() || !this->CheckEditableBrowserPage())
        return;
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Generic::DeveloperError();
        return;
    }
    this->Revert("", true, false);
}

void MainWindow::on_actionWelcome_user_2_triggered()
{
    this->Welcome();
}

void MainWindow::on_actionReport_user_triggered()
{
    if (this->CurrentEdit == nullptr)
    {
        Syslog::HuggleLogs->ErrorLog(Localizations::HuggleLocalizations->Localize("report-no-user"));
        return;
    }
    this->DisplayReportUserWindow();
}

void MainWindow::on_actionReport_user_2_triggered()
{
    if(this->CurrentEdit == nullptr)
    {
        Syslog::HuggleLogs->ErrorLog(Localizations::HuggleLocalizations->Localize("report-no-user"));
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
        QDesktopServices::openUrl(QString(Configuration::GetProjectWikiURL() + this->CurrentEdit->User->GetTalk() + "?action=edit"));
}

void MainWindow::on_actionReconnect_IRC_triggered()
{
    this->ReconnectIRC();
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
    this->ReconnectIRC();
}

void Huggle::MainWindow::on_actionWiki_triggered()
{
    if (!this->CheckExit() || !this->CheckEditableBrowserPage())
        return;
    Syslog::HuggleLogs->Log(Localizations::HuggleLocalizations->Localize("irc-switch-rc"));
    Core::HuggleCore->PrimaryFeedProvider->Stop();
    this->ui->actionIRC->setChecked(false);
    this->ui->actionWiki->setChecked(true);
    while (!Core::HuggleCore->PrimaryFeedProvider->IsStopped())
    {
        Syslog::HuggleLogs->Log(Localizations::HuggleLocalizations->Localize("irc-stop"));
        Sleeper::usleep(200000);
    }
    this->ChangeProvider(new HuggleFeedProviderWiki());
    Core::HuggleCore->PrimaryFeedProvider->Start();
}

void Huggle::MainWindow::on_actionShow_talk_triggered()
{
    this->LockPage();
    // we switch this to false so that in case we have received a message,
    // before we display the talk page, it get marked as read
    Configuration::HuggleConfiguration->NewMessage = false;
    this->Browser->DisplayPreFormattedPage(Configuration::GetProjectScriptURL() + "index.php?title=User_talk:" + Configuration::HuggleConfiguration->SystemConfig_Username);
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
    if (!Configuration::HuggleConfiguration->ProjectConfig_UAAavailable)
    {
        QMessageBox dd;
        dd.setIcon(dd.Information);
        dd.setWindowTitle(Localizations::HuggleLocalizations->Localize("uaa-not-supported"));
        dd.setText(Localizations::HuggleLocalizations->Localize("uaa-not-supported-text"));
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
    if (this->CurrentEdit == nullptr || !this->CheckExit() || !this->CheckEditableBrowserPage())
        return;
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Generic::DeveloperError();
        return;
    }
    bool ok;
    QString reason = QInputDialog::getText(this, Localizations::HuggleLocalizations->Localize("reason"),
                                           Localizations::HuggleLocalizations->Localize("main-revert-custom-reson"),
                                           QLineEdit::Normal, "No reason was provided / custom revert", &ok);
    if (!ok)
        return;
    QString summary = Configuration::HuggleConfiguration->ProjectConfig_AgfRevert.replace("$2", this->CurrentEdit->User->Username);
    summary = summary.replace("$1", reason);
    this->Revert(summary);
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
    if (this->RestoreEdit != nullptr || this->RestoreQuery != NULL)
    {
        /// \todo LOCALIZE ME
        Huggle::Syslog::HuggleLogs->Log("I am currently restoring another edit, please wait");
        return;
    }
    bool ok;
    QString reason = QInputDialog::getText(this, Localizations::HuggleLocalizations->Localize("reason"),
                                           Localizations::HuggleLocalizations->Localize("main-revert-custom-reson"),
                                           QLineEdit::Normal, "No reason was provided by user :(", &ok);
    if (!ok)
        return;
    this->RestoreQuery = new ApiQuery();
    this->RestoreQuery->IncRef();
    this->RestoreQuery->Parameters = "prop=revisions&revids=" +
            QString::number(this->CurrentEdit->RevID) + "&rvprop=" +
            QUrl::toPercentEncoding("ids|content");
    this->RestoreQuery->SetAction(ActionQuery);
    this->RestoreQuery->Process();
    this->CurrentEdit->IncRef();
    this->RestoreEdit = this->CurrentEdit;
    this->RestoreEdit_RevertReason = reason;
    Syslog::HuggleLogs->Log(Localizations::HuggleLocalizations->Localize("main-log1", this->CurrentEdit->Page->PageName));
}

void Huggle::MainWindow::on_actionClear_triggered()
{
    this->Queue1->Clear();
}

void Huggle::MainWindow::on_actionDelete_page_triggered()
{
    this->DeletePage();
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
    if (!Configuration::HuggleConfiguration->UserConfig_CheckTP)
        return;
    if (this->qTalkPage == nullptr)
    {
        this->qTalkPage = new ApiQuery();
        this->qTalkPage->SetAction(ActionQuery);
        this->qTalkPage->Parameters = "meta=userinfo&uiprop=hasmsg";
        this->qTalkPage->IncRef();
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
        this->qTalkPage->DecRef();
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
    QDesktopServices::openUrl( QDir().absoluteFilePath( name ) );
}

void Huggle::MainWindow::on_actionEnforce_sysop_rights_triggered()
{
    if (!Configuration::HuggleConfiguration->Rights.contains("delete"))
    {
        Configuration::HuggleConfiguration->Rights.append("delete");
    }
    if (!Configuration::HuggleConfiguration->Rights.contains("protect"))
    {
        Configuration::HuggleConfiguration->Rights.append("protect");
    }
    if (!Configuration::HuggleConfiguration->Rights.contains("block"))
    {
        Configuration::HuggleConfiguration->Rights.append("block");
    }
    this->ui->actionBlock_user->setEnabled(true);
    this->ui->actionBlock_user_2->setEnabled(true);
    this->ui->actionDelete_page->setEnabled(true);
    this->ui->actionDelete->setEnabled(true);
    this->ui->actionProtect->setEnabled(true);
}

void Huggle::MainWindow::on_actionFeedback_triggered()
{
    QDesktopServices::openUrl(Configuration::HuggleConfiguration->GlobalConfig_FeedbackPath);
}

void Huggle::MainWindow::on_actionConnect_triggered()
{
    this->VandalDock->Connect();
}

void Huggle::MainWindow::on_actionDisplay_user_data_triggered()
{
    Configuration::HuggleConfiguration->UserConfig_HAN_DisplayUser = this->ui->actionDisplay_user_data->isChecked();
}

void Huggle::MainWindow::on_actionDisplay_user_messages_triggered()
{
    Configuration::HuggleConfiguration->UserConfig_HAN_DisplayUserTalk = this->ui->actionDisplay_user_messages->isChecked();
}

void Huggle::MainWindow::on_actionDisplay_bot_data_triggered()
{
    Configuration::HuggleConfiguration->UserConfig_HAN_DisplayBots = this->ui->actionDisplay_bot_data->isChecked();
}

void Huggle::MainWindow::on_actionRequest_protection_triggered()
{
    if (!this->CheckExit() || !Configuration::HuggleConfiguration->ProjectConfig_RFPP || this->CurrentEdit == nullptr)
        return;
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Generic::DeveloperError();
        return;
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
    this->Queue1->DeleteByScore(Configuration::HuggleConfiguration->ProjectConfig_WhitelistScore + 1);
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
