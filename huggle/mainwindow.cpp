//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "mainwindow.hpp"
#include "ui_mainwindow.h"

using namespace Huggle;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    QDateTime load = QDateTime::currentDateTime();
    if (!Configuration::HuggleConfiguration->WhiteList.contains(Configuration::HuggleConfiguration->UserName))
    {
        Configuration::HuggleConfiguration->WhiteList.append(Configuration::HuggleConfiguration->UserName);
    }
    this->fScoreWord = NULL;
    this->fSessionData = NULL;
    this->fReportForm = NULL;
    this->fBlockForm = NULL;
#if !PRODUCTION_BUILD
    this->fDeleteForm = NULL;
#endif
    this->Shutdown = ShutdownOpRunning;
    this->wlt = NULL;
    this->fWaiting = NULL;
    this->fWhitelist = NULL;
    this->EditablePage = false;
    this->ShuttingDown = false;
    this->ui->setupUi(this);
    this->Localize();
    this->wq = NULL;
    this->Status = new QLabel();
    this->ui->statusBar->addWidget(this->Status);
    this->showMaximized();
    this->tb = new HuggleTool();
    this->Queries = new ProcessList(this);
    this->SystemLog = new HuggleLog(this);
    this->Browser = new HuggleWeb(this);
    this->Queue1 = new HuggleQueue(this);
    this->_History = new History(this);
    this->wHistory = new HistoryForm(this);
    this->fUaaReportForm = NULL;
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
    this->ui->actionBlock_user->setEnabled(Configuration::HuggleConfiguration->Rights.contains("block"));
    this->ui->actionDelete->setEnabled(Configuration::HuggleConfiguration->Rights.contains("delete"));
    this->ui->actionProtect->setEnabled(Configuration::HuggleConfiguration->Rights.contains("protect"));
    this->addDockWidget(Qt::LeftDockWidgetArea, this->_History);
    this->SystemLog->resize(100, 80);
    QStringList _log = Syslog::HuggleLogs->RingLogToQStringList();
    int c=0;
    while (c<_log.count())
    {
        this->SystemLog->InsertText(_log.at(c));
        c++;
    }
    this->CurrentEdit = NULL;
    this->setWindowTitle("Huggle 3 QT-LX on " + Configuration::HuggleConfiguration->Project->Name);
    this->ui->verticalLayout->addWidget(this->Browser);
    this->Ignore = NULL;
    this->DisplayWelcomeMessage();
    // initialise queues
    if (!Configuration::HuggleConfiguration->LocalConfig_UseIrc)
    {
        /// \todo LOCALIZE ME
        Syslog::HuggleLogs->Log("Feed: irc is disabled by project config");
    }
    if (Configuration::HuggleConfiguration->UsingIRC && Configuration::HuggleConfiguration->LocalConfig_UseIrc)
    {
        Core::HuggleCore->PrimaryFeedProvider = new HuggleFeedProviderIRC();
        this->ui->actionIRC->setChecked(true);
        if (!Core::HuggleCore->PrimaryFeedProvider->Start())
        {
            /// \todo LOCALIZE ME
            Syslog::HuggleLogs->Log("ERROR: primary feed provider has failed, fallback to wiki provider");
            delete Core::HuggleCore->PrimaryFeedProvider;
            this->ui->actionIRC->setChecked(false);
            this->ui->actionWiki->setChecked(true);
            Core::HuggleCore->PrimaryFeedProvider = new HuggleFeedProviderWiki();
            Core::HuggleCore->PrimaryFeedProvider->Start();
        }
    } else
    {
        this->ui->actionIRC->setChecked(false);
        this->ui->actionWiki->setChecked(true);
        Core::HuggleCore->PrimaryFeedProvider = new HuggleFeedProviderWiki();
        Core::HuggleCore->PrimaryFeedProvider->Start();
    }
    if (Configuration::HuggleConfiguration->LocalConfig_WarningTypes.count() > 0)
    {
        this->RevertSummaries = new QMenu(this);
        this->WarnMenu = new QMenu(this);
        this->RevertWarn = new QMenu(this);
        int r=0;
        while (r<Configuration::HuggleConfiguration->LocalConfig_WarningTypes.count())
        {
            QAction *action = new QAction(HuggleParser::GetValueFromKey(Configuration::HuggleConfiguration->LocalConfig_WarningTypes.at(r)), this);
            QAction *actiona = new QAction(HuggleParser::GetValueFromKey(Configuration::HuggleConfiguration->LocalConfig_WarningTypes.at(r)), this);
            QAction *actionb = new QAction(HuggleParser::GetValueFromKey(Configuration::HuggleConfiguration->LocalConfig_WarningTypes.at(r)), this);
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

    this->timer1 = new QTimer(this);
    this->ui->actionTag_2->setVisible(false);
    connect(this->timer1, SIGNAL(timeout()), this, SLOT(OnTimerTick1()));
    this->timer1->start(200);
    this->fRemove = NULL;
    this->eq = NULL;
    QFile *layout = NULL;
    if (QFile().exists(Configuration::GetConfigurationPath() + "mainwindow_state"))
    {
        Syslog::HuggleLogs->DebugLog("Loading state");
        layout =new QFile(Configuration::GetConfigurationPath() + "mainwindow_state");
        if (!layout->open(QIODevice::ReadOnly))
        {
            /// \todo LOCALIZE ME
            Syslog::HuggleLogs->Log("ERROR: Unable to read state from a config file");
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
            Syslog::HuggleLogs->Log("ERROR: Unable to read geometry from a config file");
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
    // these controls are for debugging only
    if (Configuration::HuggleConfiguration->Verbosity == 0)
    {
        this->ui->actionList_all_QGC_items->setVisible(false);
        this->ui->actionEdit_info->setVisible(false);
        /// \bug This doesn't work
        this->ui->menuDebug->hide();
    }
    Hooks::MainWindowIsLoad(this);
    this->VandalDock->Connect();
}

MainWindow::~MainWindow()
{
    delete this->fRemove;
    delete this->wUserInfo;
    delete this->wHistory;
    delete this->wlt;
    delete this->fWaiting;
    delete this->VandalDock;
    delete this->_History;
    delete this->RevertWarn;
    delete this->WarnMenu;
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
#if !PRODUCTION_BUILD
    delete this->fDeleteForm;
#endif
    delete this->fUaaReportForm;
    delete this->ui;
    delete this->tb;
}

void MainWindow::_ReportUser()
{
    if (!this->CheckExit())
    {
        return;
    }

    if (Configuration::HuggleConfiguration->Restricted)
    {
        Core::HuggleCore->DeveloperError();
        return;
    }

    if (this->CurrentEdit->User->IsReported)
    {
        /// \todo LOCALIZE ME
        Syslog::HuggleLogs->Log("ERROR: This user is already reported");
        return;
    }

    if (!Configuration::HuggleConfiguration->LocalConfig_AIV)
    {
        QMessageBox mb;
        /// \todo LOCALIZE ME
        mb.setText("This project doesn't support AIV system");
        /// \todo LOCALIZE ME
        mb.setWindowTitle("Function not available");
        mb.setIcon(QMessageBox::Information);
        mb.exec();
        return;
    }

    if (this->CurrentEdit == NULL)
    {
        return;
    }

    if (this->fReportForm != NULL)
    {
        delete this->fReportForm;
        this->fReportForm = NULL;
    }

    this->fReportForm = new ReportUser(this);
    this->fReportForm->show();
    this->fReportForm->SetUser(this->CurrentEdit->User);
}

void MainWindow::ProcessEdit(WikiEdit *e, bool IgnoreHistory, bool KeepHistory)
{
    if (e == NULL || this->ShuttingDown)
    {
        return;
    }
    // we need to safely delete the edit later
    e->RegisterConsumer(HUGGLECONSUMER_MAINFORM);
    // if there are actually some totaly old edits in history that we need to delete
    while (this->Historical.count() > Configuration::HuggleConfiguration->HistorySize)
    {
        WikiEdit *prev = this->Historical.at(0);
        if (prev == e)
        {
            break;
        }
        this->Historical.removeAt(0);
        Core::HuggleCore->DeleteEdit(prev);
    }
    if (this->Historical.contains(e) == false)
    {
        this->Historical.append(e);
    }
    if (this->CurrentEdit != NULL)
    {
        if (!IgnoreHistory)
        {
            if (this->CurrentEdit->Next != NULL)
            {
                // now we need to get to last edit in chain
                WikiEdit *latest = CurrentEdit;
                while (latest->Next != NULL)
                {
                    latest = latest->Next;
                }
                latest->Next = e;
                e->Previous = latest;
            } else
            {
                this->CurrentEdit->Next = e;
                e->Previous = this->CurrentEdit;
            }
        }
    }
    e->User->Resync();
    this->EditablePage = true;
    this->wUserInfo->ChangeUser(e->User);
    if (!KeepHistory)
    {
        this->wHistory->Update(e);
    }
    this->CurrentEdit = e;
    this->Browser->DisplayDiff(e);
    this->Render();
}

void MainWindow::Render()
{
    if (this->CurrentEdit != NULL)
    {
        if (this->CurrentEdit->Page == NULL)
        {
            throw new Exception("Page of CurrentEdit can't be NULL at MainWindow::Render()");
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
            {
                word = word.mid(0, word.length() - 2);
            }
        }

        /// \todo LOCALIZE ME
        this->tb->SetInfo("Diff of page: " + this->CurrentEdit->Page->PageName
                          + " (score: " + QString::number(this->CurrentEdit->Score)
                          + word + ")");
        return;
    }
    this->tb->SetTitle(this->Browser->CurrentPageName());
}

void MainWindow::RequestPD()
{
    if (!this->CheckExit() || !this->CheckEditableBrowserPage())
    {
        return;
    }
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Core::HuggleCore->DeveloperError();
        return;
    }
    if (this->CurrentEdit == NULL)
    {
        return;
    }

    if (this->fRemove != NULL)
    {
        delete this->fRemove;
    }

    this->fRemove = new SpeedyForm();
    this->fRemove->Init(this->CurrentEdit->User, this->CurrentEdit->Page);
    this->fRemove->show();
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

RevertQuery *MainWindow::Revert(QString summary, bool nd, bool next)
{
    bool rollback = true;
    if (this->CurrentEdit == NULL)
    {
        /// \todo LOCALIZE ME
        Syslog::HuggleLogs->Log("ERROR: Unable to revert, edit is null");
        return NULL;
    }

    if (!this->CurrentEdit->IsPostProcessed())
    {
        /// \todo LOCALIZE ME
        Syslog::HuggleLogs->Log("ERROR: This edit is still being processed, please wait");
        return NULL;
    }

    if (this->CurrentEdit->RollbackToken == "")
    {
        /// \todo LOCALIZE ME
        Syslog::HuggleLogs->Log("WARNING: Rollback token for edit " + this->CurrentEdit->Page->PageName + " could not be retrieved, fallback to manual edit");
        rollback = false;
    }

    if (Core::HuggleCore->PreflightCheck(this->CurrentEdit))
    {
        this->CurrentEdit->User->Resync();
        this->CurrentEdit->User->setBadnessScore(this->CurrentEdit->User->getBadnessScore(false) - 10);
        Hooks::OnRevert(this->CurrentEdit);
        RevertQuery *q = Core::HuggleCore->RevertEdit(this->CurrentEdit, summary, false, rollback, nd);
        if (next)
        {
            this->Queue1->Next();
        }
        return q;
    }
    return NULL;
}

bool MainWindow::Warn(QString WarningType, RevertQuery *dependency)
{
    if (this->CurrentEdit == NULL)
    {
        Syslog::HuggleLogs->DebugLog("NULL");
        return false;
    }

    if (Configuration::HuggleConfiguration->Restricted)
    {
        Core::HuggleCore->DeveloperError();
        return false;
    }

    // check if user wasn't changed and if was, let's update the info
    this->CurrentEdit->User->Resync();

    // get a template
    this->CurrentEdit->User->WarningLevel++;

    if (this->CurrentEdit->User->WarningLevel > 4)
    {
        if (this->CurrentEdit->User->IsReported)
        {
            return false;
        }
        if (Core::HuggleCore->ReportPreFlightCheck())
        {
            this->_ReportUser();
        }
        return false;
    }

    QString __template = WarningType + QString::number(this->CurrentEdit->User->WarningLevel);

    QString warning = Core::HuggleCore->RetrieveTemplateToWarn(__template);

    if (warning == "")
    {
        /// \todo LOCALIZE ME
        Syslog::HuggleLogs->Log("There is no such warning template " + __template);
        return false;
    }

    warning = warning.replace("$2", this->CurrentEdit->GetFullUrl()).replace("$1", this->CurrentEdit->Page->PageName);

    QString title = "Message re " + this->CurrentEdit->Page->PageName;

    switch (this->CurrentEdit->User->WarningLevel)
    {
        case 1:
            title = Configuration::HuggleConfiguration->LocalConfig_WarnSummary;
            break;
        case 2:
            title = Configuration::HuggleConfiguration->LocalConfig_WarnSummary2;
            break;
        case 3:
            title = Configuration::HuggleConfiguration->LocalConfig_WarnSummary3;
            break;
        case 4:
            title = Configuration::HuggleConfiguration->LocalConfig_WarnSummary4;
            break;
    }

    title = title.replace("$1", this->CurrentEdit->Page->PageName);
    /// \todo Properly implement system to ensure that section head is not posted after level 1 warning
    Core::HuggleCore->MessageUser(this->CurrentEdit->User, warning, "Your edits to " + this->CurrentEdit->Page->PageName,
                      title, true, dependency);
    Hooks::OnWarning(this->CurrentEdit->User);

    return true;
}

QString MainWindow::GetSummaryKey(QString item)
{
    if (item.contains(";"))
    {
        QString type = item.mid(0, item.indexOf(";"));
        int c=0;
        while(c < Configuration::HuggleConfiguration->LocalConfig_WarningTypes.count())
        {
            QString x = Configuration::HuggleConfiguration->LocalConfig_WarningTypes.at(c);
            if (x.startsWith(type + ";"))
            {
                x = Configuration::HuggleConfiguration->LocalConfig_WarningTypes.at(c);
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
    this->EditablePage = false;
    delete welcome;
    this->Render();
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

void MainWindow::OnTimerTick1()
{
    Core::HuggleCore->FinalizeMessages();
    bool RetrieveEdit = true;
    GC::gc->DeleteOld();
    // if there is no working feed, let's try to fix it
    if (Core::HuggleCore->PrimaryFeedProvider->IsWorking() != true && this->ShuttingDown != true)
    {
        Syslog::HuggleLogs->Log("Failure of primary feed provider, trying to recover");
        if (!Core::HuggleCore->PrimaryFeedProvider->Restart())
        {
            delete Core::HuggleCore->PrimaryFeedProvider;
            Core::HuggleCore->PrimaryFeedProvider = new HuggleFeedProviderWiki();
            Core::HuggleCore->PrimaryFeedProvider->Start();
        }
    }
    // check if queue isn't full
    if (this->Queue1->Items.count() > Configuration::HuggleConfiguration->Cache_InfoSize)
    {
        if (this->ui->actionStop_feed->isChecked())
        {
            Core::HuggleCore->PrimaryFeedProvider->Pause();
            RetrieveEdit = false;
        } else
        {
            if (Core::HuggleCore->PrimaryFeedProvider->IsPaused())
            {
                Core::HuggleCore->PrimaryFeedProvider->Resume();
            }
            this->Queue1->Trim();
        }
    } else
    {
        if (this->ui->actionStop_feed->isChecked())
        {
            if (Core::HuggleCore->PrimaryFeedProvider->IsPaused())
            {
                Core::HuggleCore->PrimaryFeedProvider->Resume();
            }
        }
    }
    if (RetrieveEdit)
    {
        if (Core::HuggleCore->PrimaryFeedProvider->ContainsEdit())
        {
            // we take the edit and start post processing it
            WikiEdit *edit = Core::HuggleCore->PrimaryFeedProvider->RetrieveEdit();
            if (edit != NULL)
            {
                Core::HuggleCore->PostProcessEdit(edit);
                this->PendingEdits.append(edit);
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
                this->Queue1->AddItem(this->PendingEdits.at(c));
                this->PendingEdits.removeAt(c);
            } else
            {
                c++;
            }
        }
    }
    /// \todo LOCALIZE ME
    QString t = "All systems go! - currently processing " + QString::number(Core::HuggleCore->ProcessingEdits.count())
            + " edits and " + QString::number(Core::HuggleCore->RunningQueriesGetCount()) + " queries."
            + " I have " + QString::number(Configuration::HuggleConfiguration->WhiteList.size())
            + " whitelisted users and you have " + QString::number(HuggleQueueItemLabel::Count)
            + " edits waiting in queue.";
    if (Configuration::HuggleConfiguration->Verbosity > 0)
    {
        t += " QGC: " + QString::number(GC::gc->list.count()) + "U: " + QString::number(WikiUser::ProblematicUsers.count());
    }
    this->Status->setText(t);
    // let's refresh the edits that are being post processed
    if (Core::HuggleCore->ProcessingEdits.count() > 0)
    {
        int Edit = 0;
        while (Edit < Core::HuggleCore->ProcessingEdits.count())
        {
            if (Core::HuggleCore->ProcessingEdits.at(Edit)->FinalizePostProcessing())
            {
                WikiEdit *e = Core::HuggleCore->ProcessingEdits.at(Edit);
                Core::HuggleCore->ProcessingEdits.removeAt(Edit);
                e->UnregisterConsumer(HUGGLECONSUMER_CORE_POSTPROCESS);
            }
            else
            {
                Edit++;
            }
        }
    }
    Core::HuggleCore->CheckQueries();
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
}

void MainWindow::OnTimerTick0()
{
    if (this->Shutdown != ShutdownOpUpdatingConf)
    {
        if (this->wq == NULL)
        {
            return;
        }
        if (this->Shutdown == ShutdownOpRetrievingWhitelist)
        {
            if (Configuration::HuggleConfiguration->WhitelistDisabled)
            {
                this->Shutdown = ShutdownOpUpdatingWhitelist;
                return;
            }
            if (!this->wq->Processed())
            {
                return;
            }
            QString list = wq->Result->Data;
            list = list.replace("<!-- list -->", "");
            QStringList wl = list.split("|");
            int c=0;
            /// \todo LOCALIZE ME
            this->fWaiting->Status(40, "Merging");
            while (c < wl.count())
            {
                if (wl.at(c) != "")
                {
                    Configuration::HuggleConfiguration->WhiteList.append(wl.at(c));
                }
                c++;
            }
            Configuration::HuggleConfiguration->WhiteList.removeDuplicates();
            /// \todo LOCALIZE ME
            this->fWaiting->Status(60, "Updating whitelist");
            this->Shutdown = ShutdownOpUpdatingWhitelist;
            this->wq->UnregisterConsumer(HUGGLECONSUMER_MAINFORM);
            this->wq = new WLQuery();
            this->wq->RegisterConsumer(HUGGLECONSUMER_MAINFORM);
            this->wq->Save = true;
            this->wq->Process();
            return;
        }
        if (this->Shutdown == ShutdownOpUpdatingWhitelist)
        {
            if (!Configuration::HuggleConfiguration->WhitelistDisabled && !this->wq->Processed())
            {
                return;
            }
            // we finished writing the wl
            this->wq->UnregisterConsumer(HUGGLECONSUMER_MAINFORM);
            /// \todo LOCALIZE ME
            this->fWaiting->Status(80, "Updating user config");
            this->wq = NULL;
            this->Shutdown = ShutdownOpUpdatingConf;
            QString page = Configuration::HuggleConfiguration->GlobalConfig_UserConf;
            page = page.replace("$1", Configuration::HuggleConfiguration->UserName);
            WikiPage *uc = new WikiPage(page);
            this->eq = Core::HuggleCore->EditPage(uc, Configuration::MakeLocalUserConfig(), "Writing user config", true);
            this->eq->RegisterConsumer(HUGGLECONSUMER_MAINFORM);
            delete uc;
            return;
        }
    } else
    {
        // we need to check if config was written
        if (!this->eq->Processed())
        {
            return;
        }
        Syslog::HuggleLogs->Log(this->eq->Result->Data);
        this->eq->UnregisterConsumer(HUGGLECONSUMER_MAINFORM);
        this->eq = NULL;
        this->wlt->stop();
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
    if (!CheckExit() || !CheckEditableBrowserPage())
    {
        return;
    }
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Core::HuggleCore->DeveloperError();
        return;
    }
    this->Warn("warning", NULL);
}

void MainWindow::on_actionRevert_currently_displayed_edit_triggered()
{
    if (!CheckExit())
    {
        return;
    }
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Core::HuggleCore->DeveloperError();
        return;
    }
    this->Revert();
}

void MainWindow::on_actionWarn_the_user_triggered()
{
    if (!CheckExit())
    {
        return;
    }
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Core::HuggleCore->DeveloperError();
        return;
    }
    this->Warn("warning", NULL);
}

void MainWindow::on_actionRevert_currently_displayed_edit_and_warn_the_user_triggered()
{
    if (!CheckExit())
    {
        return;
    }
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Core::HuggleCore->DeveloperError();
        return;
    }

    RevertQuery *result = this->Revert("", true, false);

    if (result != NULL)
    {
        this->Warn("warning", result);
    }

    if (Configuration::HuggleConfiguration->NextOnRv)
    {
        this->Queue1->Next();
    }
}

void MainWindow::on_actionRevert_and_warn_triggered()
{
    if (!CheckExit())
    {
        return;
    }
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Core::HuggleCore->DeveloperError();
        return;
    }

    RevertQuery *result = this->Revert("", true, false);

    if (result != NULL)
    {
        this->Warn("warning", result);
    }

    if (Configuration::HuggleConfiguration->NextOnRv)
    {
        this->Queue1->Next();
    }
}

void MainWindow::on_actionRevert_triggered()
{
    if (!CheckExit())
    {
        return;
    }
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Core::HuggleCore->DeveloperError();
        return;
    }
    this->Revert();
}

void MainWindow::on_actionShow_ignore_list_of_current_wiki_triggered()
{
    if (this->Ignore != NULL)
    {
        delete this->Ignore;
    }
    this->Ignore = new IgnoreList(this);
    this->Ignore->show();
}

void MainWindow::on_actionForward_triggered()
{
    if (this->CurrentEdit == NULL)
    {
        return;
    }
    if (this->CurrentEdit->Next == NULL)
    {
        return;
    }
    this->ProcessEdit(this->CurrentEdit->Next, true);
}

void MainWindow::on_actionBack_triggered()
{
    if (this->CurrentEdit == NULL)
    {
        return;
    }
    if (this->CurrentEdit->Previous == NULL)
    {
        return;
    }
    this->ProcessEdit(this->CurrentEdit->Previous, true);
}

void MainWindow::CustomRevert()
{
    if (!CheckExit())
    {
        return;
    }
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Core::HuggleCore->DeveloperError();
        return;
    }
    QAction *revert = (QAction*) QObject::sender();
    QString k = HuggleParser::GetKeyOfWarningTypeFromWarningName(revert->text());
    QString rs = HuggleParser::GetSummaryOfWarningTypeFromWarningKey(k);
    this->Revert(rs);
}

void MainWindow::CustomRevertWarn()
{
    if (!CheckExit())
    {
        return;
    }
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Core::HuggleCore->DeveloperError();
        return;
    }

    QAction *revert = (QAction*) QObject::sender();
    QString k = HuggleParser::GetKeyOfWarningTypeFromWarningName(revert->text());
    QString rs = HuggleParser::GetSummaryOfWarningTypeFromWarningKey(k);
    RevertQuery *result = this->Revert(rs, true, false);

    if (result != NULL)
    {
        this->Warn(k, result);
    }

    if (Configuration::HuggleConfiguration->NextOnRv)
    {
        this->Queue1->Next();
    }
}

void MainWindow::CustomWarn()
{
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Core::HuggleCore->DeveloperError();
        return;
    }

    QAction *revert = (QAction*) QObject::sender();
    QString k = HuggleParser::GetKeyOfWarningTypeFromWarningName(revert->text());

    this->Warn(k, NULL);
}

QString MainWindow::GetSummaryText(QString text)
{
    int id=0;
    while (id<Configuration::HuggleConfiguration->LocalConfig_RevertSummaries.count())
    {
        if (text == this->GetSummaryKey(Configuration::HuggleConfiguration->LocalConfig_RevertSummaries.at(id)))
        {
            QString data = Configuration::HuggleConfiguration->LocalConfig_RevertSummaries.at(id);
            if (data.contains(";"))
            {
                data = data.mid(data.indexOf(";") + 1);
            }
            return data;
        }
        id++;
    }
    return Configuration::HuggleConfiguration->LocalConfig_DefaultSummary;
}

void MainWindow::ForceWarn(int level)
{
    if (!CheckExit())
    {
        return;
    }

    if (Configuration::HuggleConfiguration->Restricted)
    {
        Core::HuggleCore->DeveloperError();
        return;
    }

    if (this->CurrentEdit == NULL)
    {
        return;
    }

    QString __template = "warning" + QString::number(level);

    QString warning = Core::HuggleCore->RetrieveTemplateToWarn(__template);

    if (warning == "")
    {
        /// \todo LOCALIZE ME
        Syslog::HuggleLogs->Log("There is no such warning template " + __template);
        return;
    }

    warning = warning.replace("$2", this->CurrentEdit->GetFullUrl()).replace("$1", this->CurrentEdit->Page->PageName);

    QString title = "Message re " + Configuration::HuggleConfiguration->EditSuffixOfHuggle;

    switch (level)
    {
        case 1:
            title = Configuration::HuggleConfiguration->LocalConfig_WarnSummary;
            break;
        case 2:
            title = Configuration::HuggleConfiguration->LocalConfig_WarnSummary2;
            break;
        case 3:
            title = Configuration::HuggleConfiguration->LocalConfig_WarnSummary3;
            break;
        case 4:
            title = Configuration::HuggleConfiguration->LocalConfig_WarnSummary4;
            break;
    }

    title = title.replace("$1", this->CurrentEdit->Page->PageName);
    Core::HuggleCore->MessageUser(this->CurrentEdit->User, warning, "Your edits to " + this->CurrentEdit->Page->PageName,
                      title, true);
}

void MainWindow::Exit()
{
    if (ShuttingDown)
    {
        return;
    }
    this->ShuttingDown = true;
    this->VandalDock->Disconnect();
    QFile *layout = new QFile(Configuration::GetConfigurationPath() + "mainwindow_state");
    if (!layout->open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        /// \todo LOCALIZE ME
        Syslog::HuggleLogs->Log("ERROR: Unable to write state to a config file");
    } else
    {
        layout->write(this->saveState());
    }
    layout->close();
    delete layout;
    layout = new QFile(Configuration::GetConfigurationPath() + "mainwindow_geometry");
    if (!layout->open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        /// \todo LOCALIZE ME
        Syslog::HuggleLogs->Log("ERROR: Unable to write geometry to a config file");
    } else
    {
        layout->write(this->saveGeometry());
    }
    layout->close();
    delete layout;
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Core::HuggleCore->Shutdown();
        return;
    }
    this->Shutdown = ShutdownOpRetrievingWhitelist;
    if (Core::HuggleCore->PrimaryFeedProvider != NULL)
    {
        Core::HuggleCore->PrimaryFeedProvider->Stop();
    }
    if (this->fWaiting != NULL)
    {
        delete this->fWaiting;
    }
    this->fWaiting = new WaitingForm(this);
    this->fWaiting->show();
    this->fWaiting->Status(10, Localizations::HuggleLocalizations->Localize("whitelist-download"));
    this->wq = new WLQuery();
    this->wq->RegisterConsumer(HUGGLECONSUMER_MAINFORM);
    this->wq->Process();
    this->wlt = new QTimer(this);
    connect(this->wlt, SIGNAL(timeout()), this, SLOT(OnTimerTick0()));
    this->wlt->start(800);
}

void MainWindow::ReconnectIRC()
{
    if (!CheckExit())
    {
        return;
    }
    if (!Configuration::HuggleConfiguration->UsingIRC)
    {
        /// \todo LOCALIZE ME
        Syslog::HuggleLogs->Log("IRC is disabled by project or huggle configuration, you need to enable it first");
        return;
    }
    Syslog::HuggleLogs->Log("Reconnecting to IRC");
    Core::HuggleCore->PrimaryFeedProvider->Stop();
    while (!Core::HuggleCore->PrimaryFeedProvider->IsStopped())
    {
        /// \todo LOCALIZE ME
        Syslog::HuggleLogs->Log("Waiting for primary feed provider to stop");
        Sleeper::usleep(200000);
    }
    delete Core::HuggleCore->PrimaryFeedProvider;
    this->ui->actionIRC->setChecked(true);
    this->ui->actionWiki->setChecked(false);
    Core::HuggleCore->PrimaryFeedProvider = new HuggleFeedProviderIRC();
    if (!Core::HuggleCore->PrimaryFeedProvider->Start())
    {
        this->ui->actionIRC->setChecked(false);
        this->ui->actionWiki->setChecked(true);
        Syslog::HuggleLogs->Log("ERROR: " + Localizations::HuggleLocalizations->Localize("provider-primary-failure"));
        delete Core::HuggleCore->PrimaryFeedProvider;
        Core::HuggleCore->PrimaryFeedProvider = new HuggleFeedProviderWiki();
        Core::HuggleCore->PrimaryFeedProvider->Start();
    }
}

bool MainWindow::BrowserPageIsEditable()
{
    return this->EditablePage;
}

bool MainWindow::CheckEditableBrowserPage()
{
    if (!this->EditablePage)
    {
        QMessageBox mb;
        mb.setWindowTitle("Cannot perform action");
        mb.setText(Localizations::HuggleLocalizations->Localize("main-no-page"));
        mb.exec();
        return false;
    }
    return true;
}

void MainWindow::SuspiciousEdit()
{
    if (!CheckExit() || !CheckEditableBrowserPage())
    {
        return;
    }
    if (this->CurrentEdit != NULL)
    {
        Hooks::Suspicious(this->CurrentEdit);
        this->CurrentEdit->User->setBadnessScore(this->CurrentEdit->User->getBadnessScore() + 1);
    }
    if (Configuration::HuggleConfiguration->NextOnRv)
    {
        this->Queue1->Next();
    }
}

void MainWindow::Localize()
{
    this->ui->menuPage->setTitle(Localizations::HuggleLocalizations->Localize("main-page"));
    this->ui->menuHelp->setTitle(Localizations::HuggleLocalizations->Localize("main-help"));
    this->ui->menuUser->setTitle(Localizations::HuggleLocalizations->Localize("main-user"));
    this->ui->menuQueue->setTitle(Localizations::HuggleLocalizations->Localize("main-queue"));
    this->ui->menuFile->setTitle(Localizations::HuggleLocalizations->Localize("main-system"));
    this->ui->actionAbout->setText(Localizations::HuggleLocalizations->Localize("main-help-about"));
    this->ui->actionBack->setText(Localizations::HuggleLocalizations->Localize("main-browser-back"));
    this->ui->actionBlock_user->setText(Localizations::HuggleLocalizations->Localize("main-user-block"));
    this->ui->actionClear_talk_page_of_user->setText(Localizations::HuggleLocalizations->Localize("main-user-clear-talk"));
    this->ui->actionDelete->setText(Localizations::HuggleLocalizations->Localize("main-page-delete"));
    this->ui->actionExit->setText(Localizations::HuggleLocalizations->Localize("main-system-exit"));
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
    if (!CheckExit())
    {
        return;
    }
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Core::HuggleCore->DeveloperError();
        return;
    }
    if (this->CurrentEdit == NULL)
    {
        return;
    }

    this->CurrentEdit->User->Resync();

    if (this->CurrentEdit->User->GetContentsOfTalkPage() != "")
    {
        /// \todo LOCALIZE ME
        if (QMessageBox::question(this, "Welcome :o", "This user doesn't have empty talk page, are you sure you want to send a message to him?", QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
        {
            return;
        }
    }

    if (this->CurrentEdit->User->IsIP())
    {
        if (this->CurrentEdit->User->GetContentsOfTalkPage() == "")
        {
            // write something to talk page so that we don't welcome this user twice
            this->CurrentEdit->User->SetContentsOfTalkPage(Configuration::HuggleConfiguration->LocalConfig_WelcomeAnon);
        }
        Core::HuggleCore->MessageUser(this->CurrentEdit->User, Configuration::HuggleConfiguration->LocalConfig_WelcomeAnon
              , Configuration::HuggleConfiguration->LocalConfig_WelcomeTitle, Configuration::HuggleConfiguration->LocalConfig_WelcomeSummary, false);
        return;
    }

    if (Configuration::HuggleConfiguration->LocalConfig_WelcomeTypes.count() == 0)
    {
        /// \todo LOCALIZE ME
        Syslog::HuggleLogs->Log("There are no welcome messages defined for this project");
        return;
    }

    QString message = HuggleParser::GetValueFromKey(Configuration::HuggleConfiguration->LocalConfig_WelcomeTypes.at(0));

    if (message == "")
    {
        /// \todo LOCALIZE ME
        Syslog::HuggleLogs->Log("ERROR: Invalid welcome template, ignored message");
        return;
    }

    // write something to talk page so that we don't welcome this user twice
    this->CurrentEdit->User->SetContentsOfTalkPage(message);
    Core::HuggleCore->MessageUser(this->CurrentEdit->User, message, Configuration::HuggleConfiguration->LocalConfig_WelcomeTitle,
                      Configuration::HuggleConfiguration->LocalConfig_WelcomeSummary, false);
}

void MainWindow::on_actionWelcome_user_triggered()
{
    this->Welcome();
}

void MainWindow::on_actionOpen_in_a_browser_triggered()
{
    if (this->CurrentEdit != NULL)
    {
        QDesktopServices::openUrl(Core::GetProjectWikiURL() + QUrl::toPercentEncoding( this->CurrentEdit->Page->PageName ));
    }
}

void MainWindow::on_actionIncrease_badness_score_by_20_triggered()
{
    if (this->CurrentEdit != NULL)
    {
        this->CurrentEdit->User->setBadnessScore(this->CurrentEdit->User->getBadnessScore() + 200);
    }
}

void MainWindow::on_actionDecrease_badness_score_by_20_triggered()
{
    if (this->CurrentEdit != NULL)
    {
        this->CurrentEdit->User->setBadnessScore(this->CurrentEdit->User->getBadnessScore() - 200);
    }
}

void MainWindow::on_actionGood_edit_triggered()
{
    if (this->CurrentEdit != NULL)
    {
        this->CurrentEdit->User->setBadnessScore(this->CurrentEdit->User->getBadnessScore() - 200);
        Hooks::OnGood(this->CurrentEdit);
        if (Configuration::HuggleConfiguration->LocalConfig_WelcomeGood && this->CurrentEdit->User->GetContentsOfTalkPage() == "")
        {
            this->Welcome();
        }
    }
    if (Configuration::HuggleConfiguration->NextOnRv)
    {
        this->Queue1->Next();
    }
}

void MainWindow::on_actionTalk_page_triggered()
{
    if (this->CurrentEdit == NULL)
    {
        return;
    }
    WikiPage *page = new WikiPage(this->CurrentEdit->User->GetTalk());
    this->Browser->DisplayPreFormattedPage(page);
    delete page;
}

void MainWindow::on_actionFlag_as_a_good_edit_triggered()
{
    if (!CheckExit() || !CheckEditableBrowserPage())
    {
        return;
    }
    if (this->CurrentEdit != NULL)
    {
        Hooks::OnGood(this->CurrentEdit);
        this->CurrentEdit->User->setBadnessScore(this->CurrentEdit->User->getBadnessScore() - 200);
        WikiUser::UpdateUser(this->CurrentEdit->User);
        if (Configuration::HuggleConfiguration->LocalConfig_WelcomeGood && this->CurrentEdit->User->GetContentsOfTalkPage() == "")
        {
            this->Welcome();
        }
    }
    if (Configuration::HuggleConfiguration->NextOnRv)
    {
        this->Queue1->Next();
    }
}

void MainWindow::on_actionDisplay_this_page_in_browser_triggered()
{
    if (this->CurrentEdit != NULL)
    {
        if (this->CurrentEdit->Diff > 0)
        {
            QDesktopServices::openUrl(Core::GetProjectScriptURL() + "index.php?diff=" + QString::number(this->CurrentEdit->Diff));
        } else
        {
            QDesktopServices::openUrl(Core::GetProjectWikiURL() + this->CurrentEdit->Page->PageName);
        }
    }
}

void MainWindow::on_actionEdit_page_in_browser_triggered()
{
    if (this->CurrentEdit != NULL)
    {
        QDesktopServices::openUrl(Core::GetProjectWikiURL() + this->CurrentEdit->Page->PageName
                                  + "?action=edit");
    }
}

void MainWindow::on_actionDisplay_history_in_browser_triggered()
{
    if (this->CurrentEdit != NULL)
    {
        QDesktopServices::openUrl(Core::GetProjectWikiURL() + this->CurrentEdit->Page->PageName
                                  + "?action=history");
    }
}

void MainWindow::on_actionStop_feed_triggered()
{
    this->ui->actionRemove_old_edits->setChecked(false);
    this->ui->actionStop_feed->setChecked(true);
}

void MainWindow::on_actionRemove_old_edits_triggered()
{
    this->ui->actionRemove_old_edits->setChecked(true);
    this->ui->actionStop_feed->setChecked(false);
}

void MainWindow::on_actionClear_talk_page_of_user_triggered()
{
    if (!CheckExit() || !CheckEditableBrowserPage())
    {
        return;
    }

    if (this->CurrentEdit == NULL)
    {
        return;
    }

    if (Configuration::HuggleConfiguration->Restricted)
    {
        Core::HuggleCore->DeveloperError();
        return;
    }

    if (!this->CurrentEdit->User->IsIP())
    {
        /// \todo LOCALIZE ME
        Syslog::HuggleLogs->Log("This feature is for ip users only");
        return;
    }

    WikiPage *page = new WikiPage(this->CurrentEdit->User->GetTalk());

    /// \todo LOCALIZE ME
    Core::HuggleCore->EditPage(page, Configuration::HuggleConfiguration->LocalConfig_ClearTalkPageTemp
                   + "\n" + Configuration::HuggleConfiguration->LocalConfig_WelcomeAnon,
                   "Cleaned old templates from talk page " + Configuration::HuggleConfiguration->EditSuffixOfHuggle);

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
    if (!CheckExit() || !CheckEditableBrowserPage())
    {
        return;
    }
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Core::HuggleCore->DeveloperError();
        return;
    }
    this->Revert("", false, false);
}

void MainWindow::on_actionRevert_currently_displayed_edit_and_stay_on_page_triggered()
{
    if (!CheckExit() || !CheckEditableBrowserPage())
    {
        return;
    }
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Core::HuggleCore->DeveloperError();
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
    if (this->CurrentEdit == NULL)
    {
        /// \todo LOCALIZE ME
        Syslog::HuggleLogs->Log("ERROR: No one to report");
        return;
    }
    this->_ReportUser();
}

void MainWindow::on_actionReport_user_2_triggered()
{
    if(this->CurrentEdit == NULL)
    {
        /// \todo LOCALIZE ME
        Syslog::HuggleLogs->Log("ERROR: No one to report");
        return;
    }
    this->_ReportUser();
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
    if (this->CurrentEdit != NULL)
    {
        QDesktopServices::openUrl(Core::GetProjectWikiURL() + this->CurrentEdit->User->GetTalk()
                                  + "?action=edit");
    }
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
#if !PRODUCTION_BUILD
    if (!CheckExit() || !CheckEditableBrowserPage())
    {
        return;
    }

	if (this->CurrentEdit == NULL)
	{
        /// \todo LOCALIZE ME
        Syslog::HuggleLogs->Log("ERROR: No, you cannot delete an NULL page :)");
		return;
	}

    if (this->fDeleteForm != NULL)
    {
        delete this->fDeleteForm;
    }
    this->fDeleteForm = new DeleteForm(this);
    this->fDeleteForm->setPage(this->CurrentEdit->Page);
    this->fDeleteForm->show();
#endif
}

void Huggle::MainWindow::on_actionBlock_user_triggered()
{
#if !PRODUCTION_BUILD
    if (!CheckExit() || !CheckEditableBrowserPage())
    {
        return;
    }

    if(this->CurrentEdit == NULL)
    {
        /// \todo LOCALIZE ME
        Syslog::HuggleLogs->Log("ERROR: No one to block :o");
        return;
    }

    if (this->fBlockForm != NULL)
    {
        delete this->fBlockForm;
    }

    this->fBlockForm = new BlockUser(this);
    this->CurrentEdit->User->Resync();
    this->fBlockForm->SetWikiUser(this->CurrentEdit->User);
    this->fBlockForm->show();
#endif
}

void Huggle::MainWindow::on_actionIRC_triggered()
{
    this->ReconnectIRC();
}

void Huggle::MainWindow::on_actionWiki_triggered()
{
    if (!CheckExit() || !CheckEditableBrowserPage())
    {
        return;
    }
    /// \todo LOCALIZE ME
    Syslog::HuggleLogs->Log("Switching to wiki provider");
    Core::HuggleCore->PrimaryFeedProvider->Stop();
    this->ui->actionIRC->setChecked(false);
    this->ui->actionWiki->setChecked(true);
    while (!Core::HuggleCore->PrimaryFeedProvider->IsStopped())
    {
        /// \todo LOCALIZE ME
        Syslog::HuggleLogs->Log("Waiting for primary feed provider to stop");
        Sleeper::usleep(200000);
    }
    delete Core::HuggleCore->PrimaryFeedProvider;
    Core::HuggleCore->PrimaryFeedProvider = new HuggleFeedProviderWiki();
    Core::HuggleCore->PrimaryFeedProvider->Start();
}

void Huggle::MainWindow::on_actionShow_talk_triggered()
{
    this->EditablePage = false;
    this->Browser->DisplayPreFormattedPage(Core::GetProjectScriptURL() + "index.php?title=User_talk:" + Configuration::HuggleConfiguration->UserName);
}

void MainWindow::on_actionProtect_triggered()
{
    if (!CheckExit() || !CheckEditableBrowserPage())
    {
        return;
    }
    if (this->CurrentEdit == NULL)
    {
        /// \todo LOCALIZE ME
        Syslog::HuggleLogs->Log("ERROR: Cannot protect NULL page");
        return;
    }
    if (this->fProtectForm != NULL)
    {
        delete this->fProtectForm;
    }
    this->fProtectForm = new ProtectPage(this);
    this->fProtectForm->setPageToProtect(this->CurrentEdit->Page);
    this->fProtectForm->show();
}

void Huggle::MainWindow::on_actionEdit_info_triggered()
{
    /// \todo LOCALIZE ME
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
    if (!CheckExit() || !CheckEditableBrowserPage())
    {
        return;
    }
    if (this->CurrentEdit == NULL)
    {
        return;
    }
    if (Configuration::HuggleConfiguration->LocalConfig_UAAavailable)
    {
        QMessageBox dd;
        dd.setIcon(dd.Information);
        dd.setWindowTitle(Localizations::HuggleLocalizations->Localize("uaa-not-supported"));
        dd.setText(Localizations::HuggleLocalizations->Localize("uaa-not-supported-text"));
        dd.exec();
    }
    if (this->CurrentEdit->User->IsIP())
    {
        this->ui->actionReport_username->setDisabled(true);
        return;
    }
    if (this->fUaaReportForm != NULL)
    {
        delete this->fUaaReportForm;
    }
    this->fUaaReportForm = new UAAReport();
    this->fUaaReportForm->setUserForUAA(this->CurrentEdit->User);
    this->fUaaReportForm->show();
}

void Huggle::MainWindow::on_actionShow_list_of_score_words_triggered()
{
    if (this->fScoreWord != NULL)
    {
        delete this->fScoreWord;
    }
    this->fScoreWord = new ScoreWordsDbForm(this);
    this->fScoreWord->show();
}

void Huggle::MainWindow::on_actionRevert_AGF_triggered()
{
    if (!CheckExit())
    {
        return;
    }
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Core::HuggleCore->DeveloperError();
        return;
    }
    this->Revert(Configuration::HuggleConfiguration->LocalConfig_AgfRevert);
}

void Huggle::MainWindow::on_actionDisplay_a_session_data_triggered()
{
    if (this->fSessionData != NULL)
    {
        delete this->fSessionData;
    }
    this->fSessionData = new SessionForm(this);
    this->fSessionData->show();
}

void Huggle::MainWindow::on_actionDisplay_whitelist_triggered()
{
    if (this->fWhitelist != NULL)
    {
        delete this->fWhitelist;
    }
    this->fWhitelist = new WhitelistForm(this);
    this->fWhitelist->show();
}

void Huggle::MainWindow::on_actionResort_queue_triggered()
{
    this->Queue1->Sort();
}
