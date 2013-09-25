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
#include <QDesktopServices>
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
#include "history.h"
#include "hugglefeedproviderwiki.h"
#include "hugglefeedproviderirc.h"
#include "querygc.h"
#include "reportuser.h"

namespace Ui {
class MainWindow;
}

class HuggleQueue;
class HuggleWeb;
class WikiEdit;
class WikiPage;
class WikiUser;
class ReportUser;

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
    History * _History;
    QMutex lUnwrittenLogs;
    QMenu *RevertWarn;
    QMenu *WarnMenu;
    QMenu *RevertSummaries;
    ReportUser *report;
    void _ReportUser();
    //! Recreate interface, should be called everytime you do anything with main form
    void ProcessEdit(WikiEdit *e, bool IgnoreHistory = false);
    ApiQuery *Revert(QString summary = "", bool nd = false, bool next = true);
    bool Warn(QString WarningType, ApiQuery *dependency);
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
    void CustomRevertWarn();
    void CustomWarn();
    void on_actionWelcome_user_triggered();
    void on_actionOpen_in_a_browser_triggered();
    void on_actionIncrease_badness_score_by_20_triggered();
    void on_actionDecrease_badness_score_by_20_triggered();
    void on_actionGood_edit_triggered();
    void on_actionTalk_page_triggered();
    void on_actionFlag_as_a_good_edit_triggered();
    void on_actionDisplay_this_page_in_browser_triggered();
    void on_actionEdit_page_in_browser_triggered();
    void on_actionDisplay_history_in_browser_triggered();
    void on_actionStop_feed_triggered();
    void on_actionRemove_old_edits_triggered();
    void on_actionQueue_triggered();
    void on_actionHistory_triggered();
    void on_actionProcesses_triggered();
    void on_actionSystem_log_triggered();
    void on_actionTools_dock_triggered();
    void on_actionClear_talk_page_of_user_triggered();
    void on_actionList_all_QGC_items_triggered();
    void on_actionRevert_currently_displayed_edit_warn_user_and_stay_on_page_triggered();
    void on_actionRevert_currently_displayed_edit_and_stay_on_page_triggered();
    void on_actionWelcome_user_2_triggered();
    void on_actionReport_user_triggered();

    void on_actionReport_user_2_triggered();

private:
    Ui::MainWindow *ui;
    QTimer *timer1;
    QLabel *Status;
    void DisplayWelcomeMessage();
    void Welcome();
    void Render();
};

#endif // MAINWINDOW_H


