//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace Huggle;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    QDateTime load = QDateTime::currentDateTime();
    this->Shutdown = ShutdownOpRunning;
    this->wlt = NULL;
    this->fWaiting = NULL;
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
    this->addDockWidget(Qt::LeftDockWidgetArea, this->Queue1);
    this->addDockWidget(Qt::BottomDockWidgetArea, this->SystemLog);
    this->addDockWidget(Qt::TopDockWidgetArea, this->tb);
    this->addDockWidget(Qt::BottomDockWidgetArea, this->Queries);
    this->preferencesForm = new Preferences(this);
    this->aboutForm = new AboutForm(this);
    this->report = NULL;
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
        Core::Log("Feed: irc is disabled by project config");
    }
    if (Configuration::UsingIRC && Configuration::LocalConfig_UseIrc)
    {
        Core::PrimaryFeedProvider = new HuggleFeedProviderIRC();
        if (!Core::PrimaryFeedProvider->Start())
        {
            Core::Log("ERROR: primary feed provider has failed, fallback to wiki provider");
            delete Core::PrimaryFeedProvider;
            Core::PrimaryFeedProvider = new HuggleFeedProviderWiki();
            Core::PrimaryFeedProvider->Start();
        }
    } else
    {
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
    this->eq = NULL;
    Core::Log("Main form was loaded in " + QString::number(load.secsTo(QDateTime::currentDateTime())) + " whee");
}

MainWindow::~MainWindow()
{
    delete this->wq;
    delete this->wlt;
    delete this->fWaiting;
    delete this->_History;
    delete this->RevertWarn;
    delete this->WarnMenu;
    delete this->RevertSummaries;
    delete this->Queries;
    delete this->preferencesForm;
    delete this->aboutForm;
    delete this->Ignore;
    delete this->Queue1;
    delete this->SystemLog;
    delete this->Status;
    delete this->Browser;
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
        Core::Log("ERROR: This user is already reported");
        return;
    }

    if (!Configuration::LocalConfig_AIV)
    {
        QMessageBox mb;
        mb.setText("This project doesn't support AIV system");
        mb.setWindowTitle("Function not available");
        mb.setIcon(QMessageBox::Information);
        mb.exec();
        return;
    }

    if (this->CurrentEdit == NULL)
    {
        return;
    }

    if (this->report != NULL)
    {
        delete this->report;
        this->report = NULL;
    }

    this->report = new ReportUser(this);
    this->report->show();
    this->report->SetUser(this->CurrentEdit->User);
}

void MainWindow::ProcessEdit(WikiEdit *e, bool IgnoreHistory)
{
    if (e == NULL)
    {
        return;
    }
    // we need to safely delete the edit later
    if (this->CurrentEdit != NULL)
    {
        // we need to track all edits so that we prevent
        // any possible leak
        if (!Core::ProcessedEdits.contains(this->CurrentEdit))
        {
            Core::ProcessedEdits.append(this->CurrentEdit);
            while (Core::ProcessedEdits.count() > Configuration::HistorySize)
            {
                Core::DeleteEdit(Core::ProcessedEdits.at(0));
                Core::ProcessedEdits.removeAt(0);
            }
        }
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

        this->tb->SetInfo("Diff of page: " + this->CurrentEdit->Page->PageName
                          + " (score: " + QString::number(this->CurrentEdit->Score)
                          + word + ")");
        return;
    }
    this->tb->SetTitle(this->Browser->CurrentPageName());
}

ApiQuery *MainWindow::Revert(QString summary, bool nd, bool next)
{
    bool rollback = true;
    if (this->CurrentEdit == NULL)
    {
        Core::Log("ERROR: Unable to revert, edit is null");
        return NULL;
    }

    if (!this->CurrentEdit->IsPostProcessed())
    {
        Core::Log("ERROR: This edit is still being processed, please wait");
        return NULL;
    }

    if (this->CurrentEdit->RollbackToken == "")
    {
        Core::Log("WARNING: Rollback token for edit " + this->CurrentEdit->Page->PageName + " could not be retrieved, fallback to manual edit");
        rollback = false;
    }

    if (Core::PreflightCheck(this->CurrentEdit))
    {
        ApiQuery *q = Core::RevertEdit(this->CurrentEdit, summary, false, true, nd);
        if (next)
        {
            this->Queue1->Next();
        }
        return q;
    }
    return NULL;
}

bool MainWindow::Warn(QString WarningType, ApiQuery *dependency)
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
    QueryGC::DeleteOld();
    // if there is no working feed, let's try to fix it
    if (Core::PrimaryFeedProvider->IsWorking() != true)
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
            Core::PostProcessEdit(edit);
            PendingEdits.append(edit);
        }
    }
    if (PendingEdits.count() > 0)
    {
        // postprocessed edits can be added to queue
        QList<WikiEdit*> Processed;
        int c = 0;
        while (c<PendingEdits.count())
        {
            if (PendingEdits.at(c)->IsPostProcessed())
            {
                Processed.append(PendingEdits.at(c));
            }
            c++;
        }
        c = 0;
        while (c< Processed.count())
        {
            // insert it to queue
            this->Queue1->AddItem(Processed.at(c));
            PendingEdits.removeOne(Processed.at(c));
            c++;
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
        t += " QGC: " + QString::number(QueryGC::qgc.count())
                + "U: " + QString::number(WikiUser::ProblematicUsers.count());
    }
    this->Status->setText(t);
    // let's refresh the edits that are being post processed
    if (Core::ProcessingEdits.count() > 0)
    {
        QList<WikiEdit*> rm;
        int Edit = 0;
        while (Edit < Core::ProcessingEdits.count())
        {
            if (Core::ProcessingEdits.at(Edit)->FinalizePostProcessing())
            {
                rm.append(Core::ProcessingEdits.at(Edit));
            }
            Edit++;
        }
        // remove the edits that were already processed from queue now :o
        Edit = 0;
        while (Edit < rm.count())
        {
            Core::ProcessingEdits.removeOne(rm.at(Edit));
            Edit++;
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
            delete this->wq;
            //this->fWaiting
            this->wq = new WLQuery();
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
            this->fWaiting->Status(80, "Updating user config");
            delete this->wq;
            // we really should delete this page somewhere I guess :o
            this->Shutdown = ShutdownOpUpdatingConf;
            QString page = Configuration::GlobalConfig_UserConf;
            page = page.replace("$1", Configuration::UserName);
            WikiPage *uc = new WikiPage(page);
            this->eq = Core::EditPage(uc, Core::MakeLocalUserConfig(), "Writing user config", true);
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
        delete this->eq;
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

    ApiQuery *result = this->Revert("", true, false);

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

    ApiQuery *result = this->Revert("", true, false);

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
    ApiQuery *result = this->Revert(rs, true, false);

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
    this->fWaiting->Status(10, "Downloading new whitelist");
    this->wq = new WLQuery();
    this->wq->Process();
    this->wlt = new QTimer(this);
    connect(this->wlt, SIGNAL(timeout()), this, SLOT(on_Tick2()));
    this->wlt->start(800);
}

bool MainWindow::CheckExit()
{
    if (ShuttingDown)
    {
        QMessageBox mb;
        mb.setWindowTitle("Error");
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
    if (this->CurrentEdit->User->IP)
    {
        Core::MessageUser(this->CurrentEdit->User, Configuration::LocalConfig_WelcomeAnon
                          , Configuration::LocalConfig_WelcomeTitle, Configuration::LocalConfig_WelcomeSummary, true);
        return;
    }

    if (this->CurrentEdit->User->ContentsOfTalkPage != "")
    {
        if (QMessageBox::question(this, "Welcome :o", "This user doesn't have empty talk page, are you sure you want to send a message to him?", QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
        {
            return;
        }
    }

    if (Configuration::LocalConfig_WelcomeTypes.count() == 0)
    {
        Core::Log("There are no welcome messages defined for this project");
        return;
    }

    QString message = Core::GetValueFromKey(Configuration::LocalConfig_WelcomeTypes.at(0));

    if (message == "")
    {
        Core::Log("ERROR: Invalid welcome template, ignored message");
        return;
    }

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
        WikiUser::UpdateUser(this->CurrentEdit->User);
        if (this->CurrentEdit->User->ContentsOfTalkPage == "")
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
    if (!CheckExit())
    {
        return;
    }
    if (this->CurrentEdit != NULL)
    {
        this->CurrentEdit->User->BadnessScore -=200;
        WikiUser::UpdateUser(this->CurrentEdit->User);
    } this->Queue1->Next();
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

void MainWindow::on_actionQueue_triggered()
{
    this->Queue1->setVisible(ui->actionQueue->isChecked());
}

void MainWindow::on_actionHistory_triggered()
{
    this->_History->setVisible(ui->actionHistory->isChecked());
}

void MainWindow::on_actionProcesses_triggered()
{
    this->Queries->setVisible(ui->actionProcesses->isChecked());
}

void MainWindow::on_actionSystem_log_triggered()
{
    this->SystemLog->setVisible(ui->actionSystem_log->isChecked());
}

void MainWindow::on_actionTools_dock_triggered()
{
    this->tb->setVisible(ui->actionTools_dock->isChecked());
}

void MainWindow::on_actionClear_talk_page_of_user_triggered()
{
    if (!CheckExit())
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

    if (!this->CurrentEdit->User->IP)
    {
        Core::Log("This feature is for ip users only");
        return;
    }

    WikiPage *page = new WikiPage(this->CurrentEdit->User->GetTalk());

    Core::EditPage(page, Configuration::LocalConfig_ClearTalkPageTemp
                   + "\n" + Configuration::LocalConfig_WelcomeAnon,
                   "Cleaned old templates from talk page " + Configuration::EditSuffixOfHuggle);

    delete page;
}

void MainWindow::on_actionList_all_QGC_items_triggered()
{
    int xx=0;
    while (xx<QueryGC::qgc.count())
    {
        Query *query = QueryGC::qgc.at(xx);
        Core::Log(query->DebugQgc());
        xx++;
    }
}

void MainWindow::on_actionRevert_currently_displayed_edit_warn_user_and_stay_on_page_triggered()
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
    this->Revert("", false, false);
}

void MainWindow::on_actionRevert_currently_displayed_edit_and_stay_on_page_triggered()
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
		Core::Log("ERROR: No one to report");
		return;
	}
    this->_ReportUser();
    
}

void MainWindow::on_actionReport_user_2_triggered()
{
	if(this->CurrentEdit == NULL)
    {
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
    if (!CheckExit())
    {
        return;
    }
    if (!Configuration::UsingIRC)
    {
        Core::Log("IRC is disabled by project or huggle configuration, you need to enable it first");
        return;
    }
    Core::Log("Reconnecting to IRC");
    Core::PrimaryFeedProvider->Stop();
    while (!Core::PrimaryFeedProvider->IsStopped())
    {
        Core::Log("Waiting for primary feed provider to stop");
        Sleeper::usleep(200000);
    }
    delete Core::PrimaryFeedProvider;
    Core::PrimaryFeedProvider = new HuggleFeedProviderIRC();
    if (!Core::PrimaryFeedProvider->Start())
    {
        Core::Log("ERROR: primary feed provider has failed, fallback to wiki provider");
        delete Core::PrimaryFeedProvider;
        Core::PrimaryFeedProvider = new HuggleFeedProviderWiki();
        Core::PrimaryFeedProvider->Start();
    }
}

void MainWindow::on_actionTag_2_triggered()
{

}

void MainWindow::on_actionRequest_speedy_deletion_triggered()
{

}

