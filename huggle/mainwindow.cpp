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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->Status = new QLabel();
    ui->statusBar->addWidget(this->Status);
    this->showMaximized();
    this->tb = new HuggleTool();
    this->Queries = new ProcessList(this);
    this->SystemLog = new HuggleLog(this);
    this->Browser = new HuggleWeb(this);
    this->Queue1 = new HuggleQueue(this);
    this->addDockWidget(Qt::LeftDockWidgetArea, this->Queue1);
    this->addDockWidget(Qt::BottomDockWidgetArea, this->SystemLog);
    this->addDockWidget(Qt::TopDockWidgetArea, this->tb);
    this->addDockWidget(Qt::BottomDockWidgetArea, this->Queries);
    this->preferencesForm = new Preferences(this);
    this->aboutForm = new AboutForm(this);
    this->SystemLog->resize(100, 80);
    SystemLog->InsertText(Core::RingLogToText());
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
        int r=0;
        while (r<Configuration::LocalConfig_WarningTypes.count())
        {
            QAction *action = new QAction(Core::GetValueFromKey(Configuration::LocalConfig_WarningTypes.at(r)), this);
            this->RevertSummaries->addAction(action);
            connect(action, SIGNAL(triggered()), this, SLOT(CustomRevert()));
            r++;
        }
        ui->actionRevert->setMenu(this->RevertSummaries);
    }

    this->timer1 = new QTimer(this);
    connect(this->timer1, SIGNAL(timeout()), this, SLOT(on_Tick()));
    this->timer1->start(200);
}

MainWindow::~MainWindow()
{
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

void MainWindow::ProcessEdit(WikiEdit *e, bool IgnoreHistory)
{
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

bool MainWindow::Revert(QString summary)
{
    bool rollback = true;
    if (this->CurrentEdit == NULL)
    {
        Core::Log("ERROR: Unable to revert, edit is null");
        return false;
    }

    if (!this->CurrentEdit->IsPostProcessed())
    {
        Core::Log("ERROR: This edit is still being processed, please wait");
        return false;
    }

    if (this->CurrentEdit->RollbackToken == "")
    {
        Core::Log("WARNING: Rollback token for edit " + this->CurrentEdit->Page->PageName + " could not be retrieved, fallback to manual edit");
        rollback = false;
    }

    if (Core::PreflightCheck(this->CurrentEdit))
    {
        Core::RevertEdit(this->CurrentEdit, summary);
        return true;
    }
    return false;
}

bool MainWindow::Warn()
{
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
    Core::Shutdown();
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
    if (Core::PrimaryFeedProvider->ContainsEdit())
    {
        // we take the edit and start post processing it
        WikiEdit *edit = Core::PrimaryFeedProvider->RetrieveEdit();
        Core::PostProcessEdit(edit);
        PendingEdits.append(edit);
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
        t += " QGC: " + QString::number(QueryGC::qgc.count());
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
    if (Configuration::Restricted)
    {
        Core::DeveloperError();
        return;
    }
}

void MainWindow::on_actionRevert_currently_displayed_edit_triggered()
{
    if (Configuration::Restricted)
    {
        Core::DeveloperError();
        return;
    }

    this->Revert();
}

void MainWindow::on_actionWarn_the_user_triggered()
{
    if (Configuration::Restricted)
    {
        Core::DeveloperError();
        return;
    }
}

void MainWindow::on_actionRevert_currently_displayed_edit_and_warn_the_user_triggered()
{
    if (Configuration::Restricted)
    {
        Core::DeveloperError();
        return;
    }

    if (this->Revert())
    {

    }
}

void MainWindow::on_actionRevert_and_warn_triggered()
{
    if (Configuration::Restricted)
    {
        Core::DeveloperError();
        return;
    }
    if (this->Revert())
    {

    }
}

void MainWindow::on_actionRevert_triggered()
{
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
    QAction *revert = (QAction*) QObject::sender();
    QString rs = Core::GetSummaryOfWarningTypeFromWarningKey(Core::GetKeyOfWarningTypeFromWarningName(revert->text()));
    this->Revert(rs);
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

void MainWindow::on_actionWelcome_user_triggered()
{

}

void MainWindow::on_actionOpen_in_a_browser_triggered()
{
    if (this->CurrentEdit != NULL)
    {
        QDesktopServices::openUrl(Core::GetProjectWikiURL() + QUrl::toPercentEncoding( this->CurrentEdit->Page->PageName ));
    }
}

void MainWindow::on_actionOpen_page_history_in_browser_triggered()
{
    if (this->CurrentEdit != NULL)
    {
        QDesktopServices::openUrl(Core::GetProjectWikiURL() + QUrl::toPercentEncoding( this->CurrentEdit->Page->PageName )
                                  + "?action=history");
    }
}

void MainWindow::on_actionIncrease_badness_score_by_20_triggered()
{
    if (this->CurrentEdit != NULL)
    {
        this->CurrentEdit->User->BadnessScore += 20;
        WikiUser::UpdateUser(this->CurrentEdit->User);
    }
}

void MainWindow::on_actionDecrease_badness_score_by_20_triggered()
{
    if (this->CurrentEdit != NULL)
    {
        this->CurrentEdit->User->BadnessScore -=20;
        WikiUser::UpdateUser(this->CurrentEdit->User);
    }
}

void MainWindow::on_actionGood_edit_triggered()
{
    if (this->CurrentEdit != NULL)
    {
        this->CurrentEdit->User->BadnessScore -=20;
        WikiUser::UpdateUser(this->CurrentEdit->User);
    } this->Queue1->Next();
}
