//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QMutex>
#include <QSplitter>
#include <QDockWidget>
#include "configuration.h"
#include "core.h"
#include "aboutform.h"
#include "preferences.h"
#include "hugglelog.h"
#include "hugglequeue.h"
#include "huggletool.h"
#include "huggleweb.h"
#include "wikipage.h"
#include "processlist.h"
#include "wikiuser.h"
#include "ignorelist.h"
#include "exception.h"
#include "hugglefeedproviderwiki.h"
#include "hugglefeedproviderirc.h"
#include "querygc.h"

namespace Ui {
class MainWindow;
}

class HuggleQueue;
class HuggleWeb;
class WikiEdit;
class WikiPage;
class WikiUser;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QList<WikiEdit*> PendingEdits;
    HuggleLog *SystemLog;
    HuggleQueue *Queue1;
    HuggleWeb *Browser;
    HuggleTool *tb;
    Preferences *preferencesForm;
    IgnoreList *Ignore;
    AboutForm *aboutForm;
    WikiEdit *CurrentEdit;
    ProcessList *Queries;
    QStringList UnwrittenLogs;
    QMutex lUnwrittenLogs;
    QMenu *RevertSummaries;
    //! Recreate interface, should be called everytime you do anything with main form
    void ProcessEdit(WikiEdit *e, bool IgnoreHistory = false);
    void Render();
    bool Revert(QString summary = "");
    bool Warn();
    QString GetSummaryKey(QString item);
    QString GetSummaryText(QString text);


private slots:
    void on_actionExit_triggered();
    void on_actionPreferences_triggered();
    void on_actionContents_triggered();
    void on_actionAbout_triggered();
    void on_MainWindow_destroyed();
    void on_Tick();
    void on_actionNext_triggered();
    void on_actionNext_2_triggered();
    void on_actionWarn_triggered();
    void on_actionRevert_currently_displayed_edit_triggered();
    void on_actionWarn_the_user_triggered();
    void on_actionRevert_currently_displayed_edit_and_warn_the_user_triggered();
    void on_actionRevert_and_warn_triggered();
    void on_actionRevert_triggered();
    void on_actionShow_ignore_list_of_current_wiki_triggered();
    void on_actionForward_triggered();
    void on_actionBack_triggered();

    void CustomRevert();

private:
    Ui::MainWindow *ui;
    QTimer *timer1;
    QLabel *Status;
    void DisplayWelcomeMessage();
};

#endif // MAINWINDOW_H

