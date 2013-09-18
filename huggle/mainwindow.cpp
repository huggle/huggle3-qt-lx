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
    this->timer1 = new QTimer(this);
    connect(this->timer1, SIGNAL(timeout()), this, SLOT(on_Tick()));
    this->timer1->start(200);
}

MainWindow::~MainWindow()
{
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
        }
        if (!IgnoreHistory)
        {
            this->CurrentEdit->Next = e;
            e->Previous = this->CurrentEdit;
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
        this->tb->SetInfo("Diff of page: " + this->CurrentEdit->Page->PageName);
        return;
    }
    this->tb->SetTitle(this->Browser->CurrentPageName());
}

bool MainWindow::Revert()
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
        Core::RevertEdit(this->CurrentEdit);
        return true;
    }
    return false;
}

bool MainWindow::Warn()
{
    return true;
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
