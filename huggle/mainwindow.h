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
#include <QThread>
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
#include "editquery.h"
#include "processlist.h"
#include "wikiuser.h"
#include "ignorelist.h"
#include "speedyform.h"
#include "exception.h"
#include "history.h"
#include "hugglefeedproviderwiki.h"
#include "hugglefeedproviderirc.h"
#include "userinfoform.h"
#include "querygc.h"
#include "reportuser.h"
#include "waitingform.h"
#include "wlquery.h"
#include "historyform.h"

namespace Ui
{
    class MainWindow;
}

namespace Huggle
{
    class HistoryForm;
    class UserinfoForm;
    class HuggleQueue;
    class HuggleWeb;
    class SpeedyForm;
    class WikiEdit;
    class WikiPage;
    class EditQuery;
    class ProcessList;
    class WaitingForm;
    class WikiUser;
    class ReportUser;

    enum ShutdownOp
    {
        ShutdownOpRunning,
        ShutdownOpRetrievingWhitelist,
        ShutdownOpUpdatingWhitelist,
        ShutdownOpUpdatingConf
    };

    //! Primary huggle window
    class MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        //! List of edits that are being saved
        QList<WikiEdit*> PendingEdits;
        //! Pointer to syslog
        HuggleLog *SystemLog;
        //! Pointer to queue
        HuggleQueue *Queue1;
        //! Pointer to browser
        HuggleWeb *Browser;
        HistoryForm *wHistory;
        UserinfoForm *wUserInfo;
        //! Pointer to toolbar
        HuggleTool *tb;
        //! Pointer to options
        Preferences *preferencesForm;
        //! Pointer to ignore list (see ignorelist.h)
        IgnoreList *Ignore;
        //! Pointer to about dialog (see aboutform.h)
        AboutForm *aboutForm;
        //! Pointer to current edit, if it's NULL there is no edit being displayed
        WikiEdit *CurrentEdit;
        SpeedyForm* fRemove;
        //! Pointer to processes
        ProcessList *Queries;
        //! This is a list of logs that needs to be written, it exist so that logs can be written from
        //! other threads as well, writing to syslog from other thread would crash huggle
        QStringList UnwrittenLogs;
        //! Pointer to history
        History * _History;
        //! Mutex we lock unwritten logs with so that only 1 thread can write to it
        QMutex lUnwrittenLogs;
        //! Pointer to menu of revert warn button
        QMenu *RevertWarn;
        //! Pointer to query that is used to store user config on exit of huggle
        EditQuery *eq;
        //! This query is used to refresh white list
        WLQuery *wq;
        //! Warning menu
        QMenu *WarnMenu;
        //! Revert menu
        QMenu *RevertSummaries;
        Ui::MainWindow *ui;
        bool ShuttingDown;
        //! If system is shutting down this is displaying which part of shutdown is currently being executed
        ShutdownOp Shutdown;
        ReportUser *report;
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();
        void _ReportUser();
        //! Recreate interface, should be called everytime you do anything with main form
        void ProcessEdit(WikiEdit *e, bool IgnoreHistory = false);
        ApiQuery *Revert(QString summary = "", bool nd = false, bool next = true);
        bool Warn(QString WarningType, ApiQuery *dependency);
        QString GetSummaryKey(QString item);
        QString GetSummaryText(QString text);
        //! Send a template to user no matter if they can be messaged or not
        void ForceWarn(int level);
        void Exit();


    private slots:
        void on_actionExit_triggered();
        void on_actionPreferences_triggered();
        void on_actionContents_triggered();
        void on_actionAbout_triggered();
        void on_MainWindow_destroyed();
        void on_Tick();
        void on_Tick2();
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
        void on_actionWarning_1_triggered();
        void on_actionWarning_2_triggered();
        void on_actionWarning_3_triggered();
        void on_actionWarning_4_triggered();
        void on_actionEdit_user_talk_triggered();
        void on_actionReconnect_IRC_triggered();
        void on_actionTag_2_triggered();
        void on_actionRequest_speedy_deletion_triggered();
        void on_actionDelete_triggered();
        void on_actionBlock_triggered();

    private:
        //! Check if huggle is shutting down or not, in case it is message box is shown as well
        //! this function should be called before every action user can trigger
        bool CheckExit();
        QTimer *timer1;
        // Whitelist
        QTimer *wlt;
        QLabel *Status;
        WaitingForm *fWaiting;
        void DisplayWelcomeMessage();
        void Welcome();
        void Render();
        //! Request a page deletion csd or afd and so on
        void RequestPD();
    };
}
#endif // MAINWINDOW_H
