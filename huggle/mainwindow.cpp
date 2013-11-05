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
    if (!Configuration::WhiteList.contains(Configuration::UserName))
    {
        Configuration::WhiteList.append(Configuration::UserName);
    }
    this->fScoreWord = NULL;
#if !PRODUCTION_BUILD
    this->fBlockForm = NULL;
    this->fDeleteForm = NULL;
#endif
    this->Shutdown = ShutdownOpRunning;
    this->wlt = NULL;
    this->fWaiting = NULL;
    this->EditablePage = false;
    ShuttingDown = false;
    ui->setupUi(this);
    this->wq = NULL;
    this->Status = new QLabel();
    ui->statusBar->addWidget(this->Status);
    this->showMaximized();
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
    this->addDockWidget(Qt::TopDockWidgetArea, this->wHistory);
    this->addDockWidget(Qt::TopDockWidgetArea, this->wUserInfo);
    this->addDockWidget(Qt::BottomDockWidgetArea, this->VandalDock);
    this->preferencesForm = new Preferences(this);
    this->aboutForm = new AboutForm(this);
    this->fReportForm = NULL;
    this->ui->actionBlock_user->setEnabled(Configuration::Rights.contains("block"));
    this->ui->actionDelete->setEnabled(Configuration::Rights.contains("delete"));
    this->ui->actionProtect->setEnabled(Configuration::Rights.contains("protect"));
    this->addDockWidget(Qt::LeftDockWidgetArea, this->_History);
    this->SystemLog->resize(100, 80);
    QStringList _log = Core::RingLogToQStringList();
    int c=0;
    while (c<_log.count())
    {
        SystemLog->InsertText(_log.at(c));
        c++;
    }
    this->CurrentEdit = NULL;
    this->setWindowTitle("Huggle 3 QT-LX");
    ui->verticalLayout->addWidget(this->Browser);
    this->Ignore = NULL;
    DisplayWelcomeMessage();
    // initialise queues
    if (!Configuration::LocalConfig_UseIrc)
    {
        /// \todo LOCALIZE ME
        Core::Log("Feed: irc is disabled by project config");
    }
    if (Configuration::UsingIRC && Configuration::LocalConfig_UseIrc)
    {
        Core::PrimaryFeedProvider = new HuggleFeedProviderIRC();
        ui->actionIRC->setChecked(true);
        if (!Core::PrimaryFeedProvider->Start())
        {
            /// \todo LOCALIZE ME
            Core::Log("ERROR: primary feed provider has failed, fallback to wiki provider");
            delete Core::PrimaryFeedProvider;
            ui->actionIRC->setChecked(false);
            ui->actionWiki->setChecked(true);
            Core::PrimaryFeedProvider = new HuggleFeedProviderWiki();
            Core::PrimaryFeedProvider->Start();
        }
    } else
    {
        ui->actionIRC->setChecked(false);
        ui->actionWiki->setChecked(true);
        Core::PrimaryFeedProvider = new HuggleFeedProviderWiki();
        Core::PrimaryFeedProvider->Start();
    }
    if (Configuration::LocalConfig_WarningTypes.count() > 0)
    {
        this->RevertSummaries = new QMenu(this);
        this->WarnMenu = new QMenu(this);
        this->RevertWarn = new QMenu(this);
        int r=0;
        while (r<Configuration::LocalConfig_WarningTypes.count())
        {
            QAction *action = new QAction(Core::GetValueFromKey(Configuration::LocalConfig_WarningTypes.at(r)), this);
            QAction *actiona = new QAction(Core::GetValueFromKey(Configuration::LocalConfig_WarningTypes.at(r)), this);
            QAction *actionb = new QAction(Core::GetValueFromKey(Configuration::LocalConfig_WarningTypes.at(r)), this);
            this->RevertWarn->addAction(actiona);
            this->WarnMenu->addAction(actionb);
            this->RevertSummaries->addAction(action);
            r++;
            connect(action, SIGNAL(triggered()), this, SLOT(CustomRevert()));
            connect(actiona, SIGNAL(triggered()), this, SLOT(CustomRevertWarn()));
            connect(actionb, SIGNAL(triggered()), this, SLOT(CustomWarn()));
        }
        ui->actionWarn->setMenu(this->WarnMenu);
        ui->actionRevert->setMenu(this->RevertSummaries);
        ui->actionRevert_and_warn->setMenu(this->RevertWarn);
    }

    this->timer1 = new QTimer(this);
    this->ui->actionTag_2->setVisible(false);
#ifdef PRODUCTION
    this->ui->actionTag->setVisible(false);
#endif
    connect(this->timer1, SIGNAL(timeout()), this, SLOT(on_Tick()));
    this->timer1->start(200);
    this->fRemove = NULL;
    this->eq = NULL;
    QFile *layout = NULL;
    if (QFile().exists(Configuration::GetConfigurationPath() + "mainwindow_state"))
    {
        Core::DebugLog("Loading state");
        layout =new QFile(Configuration::GetConfigurationPath() + "mainwindow_state");
        if (!layout->open(QIODevice::ReadOnly))
        {
            /// \todo LOCALIZE ME
            Core::Log("ERROR: Unable to read state from a config file");
        } else
        {
            if (!this->restoreState(layout->readAll()))
            {
                Core::DebugLog("Failed to restore state");
            }
        }
        layout->close();
        delete layout;
    }
    if (QFile().exists(Configuration::GetConfigurationPath() + "mainwindow_geometry"))
    {
        Core::DebugLog("Loading geometry");
        layout = new QFile(Configuration::GetConfigurationPath() + "mainwindow_geometry");
        if (!layout->open(QIODevice::ReadOnly))
        {
            Core::Log("ERROR: Unable to read geometry from a config file");
        } else
        {
            if (!this->restoreGeometry(layout->readAll()))
            {
                Core::DebugLog("Failed to restore layout");
            }
        }
        layout->close();
        delete layout;
    }
    if (Configuration::Verbosity == 0)
    {
        ui->menuDebug->setVisible(false);
    }
    Core::Log("Main form was loaded in " + QString::number(load.secsTo(QDateTime::currentDateTime())) + " whee");
    this->VandalDock->Connect();
}

MainWindow::~MainWindow()
{
    delete this->fRemove;
    delete this->wq;
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
    delete this->fScoreWord;
    delete this->Ignore;
    delete this->Queue1;
    delete this->SystemLog;
    delete this->Status;
    delete this->Browser;
#if !PRODUCTION_BUILD
    delete this->fBlockForm;
    delete this->fDeleteForm;
#endif
    delete this->fUaaReportForm;
    delete ui;
    delete this->tb;
}

void MainWindow::_ReportUser()
{
    if (!CheckExit())
    {
        return;
    }

    if (Configuration::Restricted)
    {
        Core::DeveloperError();
        return;
    }

    if (this->CurrentEdit->User->IsReported)
    {
        /// \todo LOCALIZE ME
        Core::Log("ERROR: This user is already reported");
        return;
    }

    if (!Configuration::LocalConfig_AIV)
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

void MainWindow::ProcessEdit(WikiEdit *e, bool IgnoreHistory)
{
    if (e == NULL || this->ShuttingDown)
    {
        return;
    }
    // we need to safely delete the edit later
    e->RegisterConsumer("MainForm");
    // if there are actually some totaly old edits in history that we need to delete
    while (this->Historical.count() > Configuration::HistorySize)
    {
        WikiEdit *prev = this->Historical.at(0);
        if (prev == e)
        {
            break;
        }
        this->Historical.removeAt(0);
        Core::DeleteEdit(prev);
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
    this->EditablePage = true;
    this->wUserInfo->ChangeUser(e->User);
    this->wHistory->Update(e);
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
    if (!CheckExit() || !CheckEditableBrowserPage())
    {
        return;
    }
    if (Configuration::Restricted)
    {
        Core::DeveloperError();
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
        Core::Log("ERROR: Unable to revert, edit is null");
        return NULL;
    }

    if (!this->CurrentEdit->IsPostProcessed())
    {
        /// \todo LOCALIZE ME
        Core::Log("ERROR: This edit is still being processed, please wait");
        return NULL;
    }

    if (this->CurrentEdit->RollbackToken == "")
    {
        /// \todo LOCALIZE ME
        Core::Log("WARNING: Rollback token for edit " + this->CurrentEdit->Page->PageName + " could not be retrieved, fallback to manual edit");
        rollback = false;
    }

    if (Core::PreflightCheck(this->CurrentEdit))
    {
        Hooks::OnRevert(this->CurrentEdit);
        RevertQuery *q = Core::RevertEdit(this->CurrentEdit, summary, false, rollback, nd);
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
        Core::DebugLog("NULL");
        return false;
    }

    if (Configuration::Restricted)
    {
        Core::DeveloperError();
        return false;
    }

    // get a template
    this->CurrentEdit->User->WarningLevel++;

    if (this->CurrentEdit->User->WarningLevel > 4)
    {
        if (this->CurrentEdit->User->IsReported)
        {
            return false;
        }
        if (Core::ReportPreFlightCheck())
        {
            this->_ReportUser();
        }
        return false;
    }

    QString __template = WarningType + QString::number(this->CurrentEdit->User->WarningLevel);

    QString warning = Core::RetrieveTemplateToWarn(__template);

    if (warning == "")
    {
        /// \todo LOCALIZE ME
        Core::Log("There is no such warning template " + __template);
        return false;
    }

    warning = warning.replace("$2", this->CurrentEdit->GetFullUrl()).replace("$1", this->CurrentEdit->Page->PageName);

    QString title = "Message re " + this->CurrentEdit->Page->PageName;

    switch (this->CurrentEdit->User->WarningLevel)
    {
        case 1:
            title = Configuration::LocalConfig_WarnSummary;
            break;
        case 2:
            title = Configuration::LocalConfig_WarnSummary2;
            break;
        case 3:
            title = Configuration::LocalConfig_WarnSummary3;
            break;
        case 4:
            title = Configuration::LocalConfig_WarnSummary4;
            break;
    }

    title = title.replace("$1", this->CurrentEdit->Page->PageName);
    Core::MessageUser(this->CurrentEdit->User, warning, "Your edits to " + this->CurrentEdit->Page->PageName,
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
        while(c < Configuration::LocalConfig_WarningTypes.count())
        {
            QString x = Configuration::LocalConfig_WarningTypes.at(c);
            if (x.startsWith(type + ";"))
            {
                x = Configuration::LocalConfig_WarningTypes.at(c);
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
    Exit();
}

void MainWindow::DisplayWelcomeMessage()
{
    WikiPage *welcome = new WikiPage(Configuration::WelcomeMP);
    this->Browser->DisplayPreFormattedPage(welcome);
    this->EditablePage = false;
    this->Render();
}

void MainWindow::on_actionPreferences_triggered()
{
    preferencesForm->show();
}

void MainWindow::on_actionContents_triggered()
{
    QDesktopServices::openUrl(Configuration::GlobalConfig_DocumentationPath);
}

void MainWindow::on_actionAbout_triggered()
{
    aboutForm->show();
}

void MainWindow::on_MainWindow_destroyed()
{
    Core::Shutdown();
}

void MainWindow::on_Tick()
{
    Core::FinalizeMessages();
    bool RetrieveEdit = true;
    GC::DeleteOld();
    // if there is no working feed, let's try to fix it
    if (Core::PrimaryFeedProvider->IsWorking() != true && this->ShuttingDown != true)
    {
        Core::Log("Failure of primary feed provider, trying to recover");
        if (!Core::PrimaryFeedProvider->Restart())
        {
            delete Core::PrimaryFeedProvider;
            Core::PrimaryFeedProvider = new HuggleFeedProviderWiki();
            Core::PrimaryFeedProvider->Start();
        }
    }
    // check if queue isn't full
    if (this->Queue1->Items.count() > Configuration::Cache_InfoSize)
    {
        if (ui->actionStop_feed->isChecked())
        {
            Core::PrimaryFeedProvider->Pause();
            RetrieveEdit = false;
        } else
        {
            this->Queue1->Trim();
        }
    } else
    {
        if (ui->actionStop_feed->isChecked())
        {
            if (Core::PrimaryFeedProvider->IsPaused())
            {
                Core::PrimaryFeedProvider->Resume();
            }
        }
    }
    if (RetrieveEdit)
    {
        if (Core::PrimaryFeedProvider->ContainsEdit())
        {
            // we take the edit and start post processing it
            WikiEdit *edit = Core::PrimaryFeedProvider->RetrieveEdit();
            if (edit != NULL)
            {
                Core::PostProcessEdit(edit);
                PendingEdits.append(edit);
            }
        }
    }
    if (PendingEdits.count() > 0)
    {
        // postprocessed edits can be added to queue
        int c = 0;
        while (c<PendingEdits.count())
        {
            if (PendingEdits.at(c)->IsPostProcessed())
            {
                this->Queue1->AddItem(PendingEdits.at(c));
                PendingEdits.removeAt(c);
            } else
            {
                c++;
            }
        }
    }
    QString t = "Currently processing " + QString::number(Core::ProcessingEdits.count())
            + " edits and " + QString::number(Core::RunningQueries.count()) + " queries"
            + " I have " + QString::number(Configuration::WhiteList.size())
            + " whitelisted users and you have "
            + QString::number(HuggleQueueItemLabel::Count)
            + " edits waiting in queue";
    if (Configuration::Verbosity > 0)
    {
        t += " QGC: " + QString::number(GC::list.count())
                + "U: " + QString::number(WikiUser::ProblematicUsers.count());
    }
    this->Status->setText(t);
    // let's refresh the edits that are being post processed
    if (Core::ProcessingEdits.count() > 0)
    {
        int Edit = 0;
        while (Edit < Core::ProcessingEdits.count())
        {
            if (Core::ProcessingEdits.at(Edit)->FinalizePostProcessing())
            {
                WikiEdit *e = Core::ProcessingEdits.at(Edit);
                Core::ProcessingEdits.removeAt(Edit);
                e->UnregisterConsumer(HUGGLECONSUMER_CORE_POSTPROCESS);
            }
            else
            {
                Edit++;
            }
        }
    }
    Core::CheckQueries();
    this->lUnwrittenLogs.lock();
    if (this->UnwrittenLogs.count() > 0)
    {
        int c = 0;
        while (c < this->UnwrittenLogs.count())
        {
            this->SystemLog->InsertText(this->UnwrittenLogs.at(c));
            c++;
        }
        this->UnwrittenLogs.clear();
    }
    this->lUnwrittenLogs.unlock();
    this->Queries->RemoveExpired();
}

void MainWindow::on_Tick2()
{
    if (this->Shutdown != ShutdownOpUpdatingConf)
    {
        if (this->wq == NULL)
        {
            return;
        }
        if (this->Shutdown == ShutdownOpRetrievingWhitelist)
        {
            if (!this->wq->Processed())
            {
                return;
            }
            QString list = wq->Result->Data;
            list = list.replace("<!-- list -->", "");
            QStringList wl = list.split("|");
            int c=0;
            fWaiting->Status(40, "Merging");
            while (c < wl.count())
            {
                if (wl.at(c) != "")
                {
                    Configuration::WhiteList.append(wl.at(c));
                }
                c++;
            }
            Configuration::WhiteList.removeDuplicates();
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
            if (!this->wq->Processed())
            {
                return;
            }
            // we finished writing the wl
            this->wq->UnregisterConsumer(HUGGLECONSUMER_MAINFORM);
            this->fWaiting->Status(80, "Updating user config");
            this->wq = NULL;
            // we really should delete this page somewhere I guess :o
            this->Shutdown = ShutdownOpUpdatingConf;
            QString page = Configuration::GlobalConfig_UserConf;
            page = page.replace("$1", Configuration::UserName);
            WikiPage *uc = new WikiPage(page);
            this->eq = Core::EditPage(uc, Configuration::MakeLocalUserConfig(), "Writing user config", true);
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
        Core::Log(this->eq->Result->Data);
        this->eq->UnregisterConsumer(HUGGLECONSUMER_MAINFORM);
        this->eq = NULL;
        this->wlt->stop();
        Core::Shutdown();
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
    if (Configuration::Restricted)
    {
        Core::DeveloperError();
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
    if (Configuration::Restricted)
    {
        Core::DeveloperError();
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
    if (Configuration::Restricted)
    {
        Core::DeveloperError();
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
    if (Configuration::Restricted)
    {
        Core::DeveloperError();
        return;
    }

    RevertQuery *result = this->Revert("", true, false);

    if (result != NULL)
    {
        this->Warn("warning", result);
    }

    if (Configuration::NextOnRv)
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
    if (Configuration::Restricted)
    {
        Core::DeveloperError();
        return;
    }

    RevertQuery *result = this->Revert("", true, false);

    if (result != NULL)
    {
        this->Warn("warning", result);
    }

    if (Configuration::NextOnRv)
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
    if (Configuration::Restricted)
    {
        Core::DeveloperError();
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
    if (Configuration::Restricted)
    {
        Core::DeveloperError();
        return;
    }
    QAction *revert = (QAction*) QObject::sender();
    QString k = Core::GetKeyOfWarningTypeFromWarningName(revert->text());
    QString rs = Core::GetSummaryOfWarningTypeFromWarningKey(k);
    this->Revert(rs);
}

void MainWindow::CustomRevertWarn()
{
    if (!CheckExit())
    {
        return;
    }
    if (Configuration::Restricted)
    {
        Core::DeveloperError();
        return;
    }

    QAction *revert = (QAction*) QObject::sender();
    QString k = Core::GetKeyOfWarningTypeFromWarningName(revert->text());
    QString rs = Core::GetSummaryOfWarningTypeFromWarningKey(k);
    RevertQuery *result = this->Revert(rs, true, false);

    if (result != NULL)
    {
        this->Warn(k, result);
    }

    if (Configuration::NextOnRv)
    {
        this->Queue1->Next();
    }
}

void MainWindow::CustomWarn()
{
    if (Configuration::Restricted)
    {
        Core::DeveloperError();
        return;
    }

    QAction *revert = (QAction*) QObject::sender();
    QString k = Core::GetKeyOfWarningTypeFromWarningName(revert->text());

    this->Warn(k, NULL);
}

QString MainWindow::GetSummaryText(QString text)
{
    int id=0;
    while (id<Configuration::LocalConfig_RevertSummaries.count())
    {
        if (text == this->GetSummaryKey(Configuration::LocalConfig_RevertSummaries.at(id)))
        {
            QString data = Configuration::LocalConfig_RevertSummaries.at(id);
            if (data.contains(";"))
            {
                data = data.mid(data.indexOf(";") + 1);
            }
            return data;
        }
        id++;
    }
    return Configuration::LocalConfig_DefaultSummary;
}

void MainWindow::ForceWarn(int level)
{
    if (!CheckExit())
    {
        return;
    }

    if (Configuration::Restricted)
    {
        Core::DeveloperError();
        return;
    }

    if (this->CurrentEdit == NULL)
    {
        return;
    }

    QString __template = "warning" + QString::number(level);

    QString warning = Core::RetrieveTemplateToWarn(__template);

    if (warning == "")
    {
        /// \todo LOCALIZE ME
        Core::Log("There is no such warning template " + __template);
        return;
    }

    warning = warning.replace("$2", this->CurrentEdit->GetFullUrl()).replace("$1", this->CurrentEdit->Page->PageName);

    QString title = "Message re " + Configuration::EditSuffixOfHuggle;

    switch (level)
    {
        case 1:
            title = Configuration::LocalConfig_WarnSummary;
            break;
        case 2:
            title = Configuration::LocalConfig_WarnSummary2;
            break;
        case 3:
            title = Configuration::LocalConfig_WarnSummary3;
            break;
        case 4:
            title = Configuration::LocalConfig_WarnSummary4;
            break;
    }

    title = title.replace("$1", this->CurrentEdit->Page->PageName);
    Core::MessageUser(this->CurrentEdit->User, warning, "Your edits to " + this->CurrentEdit->Page->PageName,
                      title, true);
}

void MainWindow::Exit()
{
    if (ShuttingDown)
    {
        return;
    }
    ShuttingDown = true;
    this->VandalDock->Disconnect();
    QFile *layout = new QFile(Configuration::GetConfigurationPath() + "mainwindow_state");
    if (!layout->open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        /// \todo LOCALIZE ME
        Core::Log("ERROR: Unable to write state to a config file");
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
        Core::Log("ERROR: Unable to write geometry to a config file");
    } else
    {
        layout->write(this->saveGeometry());
    }
    layout->close();
    delete layout;
    this->Shutdown = ShutdownOpRetrievingWhitelist;
    if (Core::PrimaryFeedProvider != NULL)
    {
        Core::PrimaryFeedProvider->Stop();
    }
    if (this->fWaiting != NULL)
    {
        delete this->fWaiting;
    }
    this->fWaiting = new WaitingForm(this);
    this->fWaiting->show();
    /// \todo LOCALIZE ME
    this->fWaiting->Status(10, "Downloading new whitelist");
    this->wq = new WLQuery();
    this->wq->RegisterConsumer(HUGGLECONSUMER_MAINFORM);
    this->wq->Process();
    this->wlt = new QTimer(this);
    connect(this->wlt, SIGNAL(timeout()), this, SLOT(on_Tick2()));
    this->wlt->start(800);
}

void MainWindow::ReconnectIRC()
{
    if (!CheckExit())
    {
        return;
    }
    if (!Configuration::UsingIRC)
    {
        /// \todo LOCALIZE ME
        Core::Log("IRC is disabled by project or huggle configuration, you need to enable it first");
        return;
    }
    Core::Log("Reconnecting to IRC");
    Core::PrimaryFeedProvider->Stop();
    while (!Core::PrimaryFeedProvider->IsStopped())
    {
        /// \todo LOCALIZE ME
        Core::Log("Waiting for primary feed provider to stop");
        Sleeper::usleep(200000);
    }
    delete Core::PrimaryFeedProvider;
    ui->actionIRC->setChecked(true);
    ui->actionWiki->setChecked(false);
    Core::PrimaryFeedProvider = new HuggleFeedProviderIRC();
    if (!Core::PrimaryFeedProvider->Start())
    {
        ui->actionIRC->setChecked(false);
        ui->actionWiki->setChecked(true);
        /// \todo LOCALIZE ME
        Core::Log("ERROR: primary feed provider has failed, fallback to wiki provider");
        delete Core::PrimaryFeedProvider;
        Core::PrimaryFeedProvider = new HuggleFeedProviderWiki();
        Core::PrimaryFeedProvider->Start();
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
        mb.setWindowTitle("Page");
        /// \todo LOCALIZE ME
        mb.setText("Current page can't be edited");
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
        this->CurrentEdit->User->BadnessScore +=1;
        WikiUser::UpdateUser(this->CurrentEdit->User);
    }
    if (Configuration::NextOnRv)
    {
        this->Queue1->Next();
    }
}

bool MainWindow::CheckExit()
{
    if (ShuttingDown)
    {
        QMessageBox mb;
        mb.setWindowTitle("Error");
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
    if (Configuration::Restricted)
    {
        Core::DeveloperError();
        return;
    }
    if (this->CurrentEdit == NULL)
    {
        return;
    }

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
            this->CurrentEdit->User->SetContentsOfTalkPage(Configuration::LocalConfig_WelcomeAnon);
        }
        Core::MessageUser(this->CurrentEdit->User, Configuration::LocalConfig_WelcomeAnon
                          , Configuration::LocalConfig_WelcomeTitle, Configuration::LocalConfig_WelcomeSummary, true);
        return;
    }

    if (Configuration::LocalConfig_WelcomeTypes.count() == 0)
    {
        /// \todo LOCALIZE ME
        Core::Log("There are no welcome messages defined for this project");
        return;
    }

    QString message = Core::GetValueFromKey(Configuration::LocalConfig_WelcomeTypes.at(0));

    if (message == "")
    {
        /// \todo LOCALIZE ME
        Core::Log("ERROR: Invalid welcome template, ignored message");
        return;
    }

    // write something to talk page so that we don't welcome this user twice
    this->CurrentEdit->User->SetContentsOfTalkPage(message);
    Core::MessageUser(this->CurrentEdit->User, message, Configuration::LocalConfig_WelcomeTitle, Configuration::LocalConfig_WelcomeSummary, true);
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
        this->CurrentEdit->User->BadnessScore += 200;
        WikiUser::UpdateUser(this->CurrentEdit->User);
    }
}

void MainWindow::on_actionDecrease_badness_score_by_20_triggered()
{
    if (this->CurrentEdit != NULL)
    {
        this->CurrentEdit->User->BadnessScore -=200;
        WikiUser::UpdateUser(this->CurrentEdit->User);
    }
}

void MainWindow::on_actionGood_edit_triggered()
{
    if (this->CurrentEdit != NULL)
    {
        this->CurrentEdit->User->BadnessScore -=200;
        Hooks::OnGood(this->CurrentEdit);
        WikiUser::UpdateUser(this->CurrentEdit->User);
        if (Configuration::LocalConfig_WelcomeGood && this->CurrentEdit->User->GetContentsOfTalkPage() == "")
        {
            this->Welcome();
        }
    }
    if (Configuration::NextOnRv)
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
        this->CurrentEdit->User->BadnessScore -=200;
        WikiUser::UpdateUser(this->CurrentEdit->User);
        if (Configuration::LocalConfig_WelcomeGood && this->CurrentEdit->User->GetContentsOfTalkPage() == "")
        {
            this->Welcome();
        }
    }
    if (Configuration::NextOnRv)
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
    ui->actionRemove_old_edits->setChecked(false);
    ui->actionStop_feed->setChecked(true);
}

void MainWindow::on_actionRemove_old_edits_triggered()
{
    ui->actionRemove_old_edits->setChecked(true);
    ui->actionStop_feed->setChecked(false);
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

    if (Configuration::Restricted)
    {
        Core::DeveloperError();
        return;
    }

    if (!this->CurrentEdit->User->IsIP())
    {
        /// \todo LOCALIZE ME
        Core::Log("This feature is for ip users only");
        return;
    }

    WikiPage *page = new WikiPage(this->CurrentEdit->User->GetTalk());

    /// \todo LOCALIZE ME
    Core::EditPage(page, Configuration::LocalConfig_ClearTalkPageTemp
                   + "\n" + Configuration::LocalConfig_WelcomeAnon,
                   "Cleaned old templates from talk page " + Configuration::EditSuffixOfHuggle);

    delete page;
}

void MainWindow::on_actionList_all_QGC_items_triggered()
{
    int xx=0;
    while (xx<GC::list.count())
    {
        Collectable *query = GC::list.at(xx);
        Core::Log(query->DebugHgc());
        xx++;
    }
}

void MainWindow::on_actionRevert_currently_displayed_edit_warn_user_and_stay_on_page_triggered()
{
    if (!CheckExit() || !CheckEditableBrowserPage())
    {
        return;
    }
    if (Configuration::Restricted)
    {
        Core::DeveloperError();
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
    if (Configuration::Restricted)
    {
        Core::DeveloperError();
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
    if(this->CurrentEdit == NULL)
    {
        /// \todo LOCALIZE ME
        Core::Log("ERROR: No one to report");
        return;
    }
    this->_ReportUser();
}

void MainWindow::on_actionReport_user_2_triggered()
{
    if(this->CurrentEdit == NULL)
    {
        /// \todo LOCALIZE ME
        Core::Log("ERROR: No one to report");
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
    ReconnectIRC();
}

void MainWindow::on_actionTag_2_triggered()
{

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
		Core::Log("ERROR: No, you cannot delete an NULL page :)");
		return;
	}
    this->fDeleteForm = new DeleteForm(this);
    fDeleteForm->setPage(this->CurrentEdit->Page);
    fDeleteForm->show();
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
        Core::Log("ERROR: No one to block :o");
        return;
    }
    this->fBlockForm = new BlockUser(this);
    fBlockForm->SetWikiUser(this->CurrentEdit->User);
    fBlockForm->show();
#endif
}

void Huggle::MainWindow::on_actionIRC_triggered()
{
    ReconnectIRC();
}

void Huggle::MainWindow::on_actionWiki_triggered()
{
    if (!CheckExit() || !CheckEditableBrowserPage())
    {
        return;
    }
    /// \todo LOCALIZE ME
    Core::Log("Switching to wiki provider");
    Core::PrimaryFeedProvider->Stop();
    ui->actionIRC->setChecked(false);
    ui->actionWiki->setChecked(true);
    while (!Core::PrimaryFeedProvider->IsStopped())
    {
        /// \todo LOCALIZE ME
        Core::Log("Waiting for primary feed provider to stop");
        Sleeper::usleep(200000);
    }
    delete Core::PrimaryFeedProvider;
    Core::PrimaryFeedProvider = new HuggleFeedProviderWiki();
    Core::PrimaryFeedProvider->Start();
}

void Huggle::MainWindow::on_actionShow_talk_triggered()
{
    this->EditablePage = false;
    this->Browser->DisplayPreFormattedPage(Core::GetProjectScriptURL() + "index.php?title=User_talk:" + Configuration::UserName);
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
        Core::Log("ERROR: Cannot protect NULL page");
        return;
    }
    this->fProtectForm = new ProtectPage(this);
    fProtectForm->setPageToProtect(this->CurrentEdit->Page);
    fProtectForm->show();
}

void Huggle::MainWindow::on_actionEdit_info_triggered()
{
    /// \todo LOCALIZE ME
    Core::Log("Current number of edits in memory: " + QString::number(WikiEdit::EditList.count()));
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
    if (Configuration::LocalConfig_UAAavailable)
    {
        QMessageBox dd;
        dd.setIcon(dd.Information);
        /// \todo LOCALIZE ME
        dd.setWindowTitle("UAA not available");
        /// \todo LOCALIZE ME
        dd.setText("The usernames for administrator attention noticeboard is not available on your wiki.");
        dd.exec();
    }
    if (this->CurrentEdit->User->IsIP())
    {
        /// \todo LOCALIZE ME
        Core::Log("ERROR: You can't report an IP to UAA!");
        return;
    }
    this->fUaaReportForm = new UAAReport();
    fUaaReportForm->setUserForUAA(this->CurrentEdit->User);
    fUaaReportForm->show();
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
