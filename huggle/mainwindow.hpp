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

#include "definitions.hpp"
// now we need to ensure that python is included first
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QMainWindow>
#include <QTimer>
#include <QInputDialog>
#include <QLabel>
#include <QMutex>
#include <QThread>
#include <QSplitter>
#include <QDockWidget>
#include "aboutform.hpp"
#include "blockuser.hpp"
#include "deleteform.hpp"
#include "editquery.hpp"
#include "history.hpp"
#include "hugglefeedproviderwiki.hpp"
#include "hugglefeedproviderirc.hpp"
#include "hugglelog.hpp"
#include "huggleparser.hpp"
#include "hugglequeue.hpp"
#include "huggletool.hpp"
#include "huggleweb.hpp"
#include "wikipage.hpp"
#include "preferences.hpp"
#include "processlist.hpp"
#include "protectpage.hpp"
#include "wikiuser.hpp"
#include "ignorelist.hpp"
#include "speedyform.hpp"
#include "userinfoform.hpp"
#include "vandalnw.hpp"
#include "reportuser.hpp"
#include "revertquery.hpp"
#include "requestprotect.hpp"
#include "whitelistform.hpp"
#include "sessionform.hpp"
#include "historyform.hpp"
#include "scorewordsdbform.hpp"
#include "warnings.hpp"
#include "warninglist.hpp"
#include "waitingform.hpp"
#include "wlquery.hpp"
#include "uaareport.hpp"

namespace Ui
{
    class MainWindow;
}

namespace Huggle
{
    class HuggleLog;
    class History;
    class HistoryForm;
    class UserinfoForm;
    class HuggleQueue;
    class HuggleTool;
    class AboutForm;
    class HuggleWeb;
    class SpeedyForm;
    class WikiEdit;
    class RevertQuery;
    class WlQuery;
    class WikiPage;
    class EditQuery;
    class ProcessList;
    class WhitelistForm;
    class Message;
    class PendingWarning;
    class Preferences;
    class SessionForm;
    class IgnoreList;
    class WaitingForm;
    class VandalNw;
    class Syslog;
    class WikiUser;
    class ReportUser;
    class RequestProtect;
    class DeleteForm;
    class BlockUser;
    class ProtectPage;
    class WarningList;
    class WLQuery;
    class UAAReport;
    class ScoreWordsDbForm;

    /*!
     * \brief The ShutdownOp enum contains a various parts of shutdown so that we can keep the track of what is going on
     */
    enum ShutdownOp
    {
        //! Huggle is not shutting down
        ShutdownOpRunning,
        //! Huggle is downloading a whitelist in order to update it
        ShutdownOpRetrievingWhitelist,
        //! Huggle is updating the whitelist
        ShutdownOpUpdatingWhitelist,
        //! Huggle is updating a config of user
        ShutdownOpUpdatingConf
    };

    //! Primary huggle window
    class MainWindow : public QMainWindow
    {
            Q_OBJECT
        public:
            static MainWindow *HuggleMain;

            explicit MainWindow(QWidget *parent = 0);
            ~MainWindow();
            void DisplayReportUserWindow(WikiUser *User = nullptr);
            /*!
             * \brief ProcessEdit Will display an edit in huggle window
             * \param e Edit
             * \param IgnoreHistory If true the history of huggle will not be updated
             * \param KeepHistory
             * \param KeepUser
             * \param ForceJump
             */
            void ProcessEdit(WikiEdit *e, bool IgnoreHistory = false, bool KeepHistory = false, bool KeepUser = false, bool ForceJump = false);
            RevertQuery *Revert(QString summary = "", bool nd = false, bool next = true);
            //! Warn a current user
            bool Warn(QString WarningType, RevertQuery *dependency);
            QString GetSummaryKey(QString item);
            QString GetSummaryText(QString text);
            //! Send a template to user no matter if they can be messaged or not
            void ForceWarn(int level);
            void Exit();
            void ReconnectIRC();
            //! Returns true if current page can be edited
            bool BrowserPageIsEditable();
            /*!
             * \brief CheckEditableBrowserPage will check if current page is editable and if it's not it display a message box
             * \return true on success or false in case it's not
             */
            bool CheckEditableBrowserPage();
            void SuspiciousEdit();
            void PatrolThis(WikiEdit *e = nullptr);
            void Localize();
            void _BlockUser();
            void DisplayNext(Query *q = nullptr);
            void DeletePage();
            void DisplayTalk();
            //! Make currently displayed page unchangeable (useful when you render non-diff pages where rollback wouldn't work)
            void LockPage();
            //! List of edits that are being saved
            QList<WikiEdit*> PendingEdits;
            //! Pointer to syslog
            HuggleLog *SystemLog;
            //! Pointer to queue
            HuggleQueue *Queue1;
            QList<ApiQuery*> PatrolledEdits;
            //! Pointer to browser
            HuggleWeb *Browser;
            HistoryForm *wHistory;
            UserinfoForm *wUserInfo;
            //! Pointer to toolbar
            HuggleTool *tb;
            //! Pointer to options
            Preferences *preferencesForm = nullptr;
            //! Pointer to ignore list (see ignorelist.h)
            IgnoreList *Ignore = nullptr;
            //! Pointer to about dialog (see aboutform.h)
            AboutForm *aboutForm = nullptr;
            //! Pointer to current edit, if it's NULL there is no edit being displayed
            WikiEdit *CurrentEdit = nullptr;
            SpeedyForm* fSpeedyDelete = nullptr;
            //! Pointer to processes
            ProcessList *Queries;
            //! Pointer to history
            History *_History;
            //! Pointer to menu of revert warn button
            QMenu *RevertWarn = nullptr;
            //! Pointer to vandal network
            VandalNw *VandalDock;
            SessionForm *fSessionData = nullptr;
            //! Pointer to query that is used to store user config on exit of huggle
            EditQuery *eq = nullptr;
            //! This query is used to refresh white list
            WLQuery *wq = nullptr;
            //! Warning menu
            QMenu *WarnMenu = nullptr;
            //! Revert menu
            QMenu *RevertSummaries = nullptr;
            ScoreWordsDbForm *fScoreWord = nullptr;
            Ui::MainWindow *ui;
            bool ShuttingDown;
            //! If system is shutting down this is displaying which part of shutdown is currently being executed
            ShutdownOp Shutdown;
            ReportUser *fReportForm = nullptr;
            //! Pointer to a form to block user
            BlockUser *fBlockForm = nullptr;
            //! Pointer to a form to delete a page
            DeleteForm *fDeleteForm = nullptr;
            //! Pointer to a form to protect a page
            ProtectPage *fProtectForm = nullptr;
            //! Pointer to UAA dialog
            UAAReport *fUaaReportForm = nullptr;
            WhitelistForm *fWhitelist = nullptr;
            int LastTPRevID;
            //! This is a query for rollback of current edit which we need to keep in case
            //! that user wants to display their own revert instead of next page
            Query *qNext = nullptr;
            //! Timer that is used to check if there are new messages on talk page
            QTimer *tCheck;
            //! Query that is used to check if talk page contains new messages
            ApiQuery *qTalkPage = nullptr;
        private slots:
            void on_actionExit_triggered();
            void on_actionPreferences_triggered();
            void on_actionContents_triggered();
            void on_actionAbout_triggered();
            void OnMainTimerTick();
            void OnTimerTick0();
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
            void on_actionRequest_speedy_deletion_triggered();
            void on_actionDelete_triggered();
            void on_actionBlock_user_triggered();
            void on_actionIRC_triggered();
            void on_actionWiki_triggered();
            void on_actionProtect_triggered();
            void on_actionShow_talk_triggered();
            void on_actionEdit_info_triggered();
            void on_actionFlag_as_suspicious_edit_triggered();
            void on_actionDisconnect_triggered();
            void on_actionReport_username_triggered();
            void on_actionShow_list_of_score_words_triggered();
            void on_actionRevert_AGF_triggered();
            void on_actionDisplay_a_session_data_triggered();
            void on_actionDisplay_whitelist_triggered();
            void on_actionResort_queue_triggered();
            void on_actionRestore_this_revision_triggered();
            void on_actionClear_triggered();
            void on_actionDelete_page_triggered();
            void on_actionBlock_user_2_triggered();
            void on_actionDisplay_talk_triggered();
            void TimerCheckTPOnTick();
            void on_actionSimulate_message_triggered();
            void on_actionHtml_dump_triggered();
            void on_actionEnforce_sysop_rights_triggered();
            void on_actionFeedback_triggered();
            void on_actionConnect_triggered();
            void on_actionDisplay_user_data_triggered();
            void on_actionDisplay_user_messages_triggered();
            void on_actionDisplay_bot_data_triggered();
            void on_actionRequest_protection_triggered();
            void on_actionRemove_edits_made_by_whitelisted_users_triggered();
            void on_actionDelete_all_edits_with_score_lower_than_200_triggered();
            void on_actionRelog_triggered();
            void on_actionAbort_2_triggered();
            void on_actionUser_contributions_triggered();
            void on_actionDisplay_this_page_triggered();
            void on_actionResume_provider_triggered();
            void on_actionStop_provider_triggered();

        private:
            //! Check if huggle is shutting down or not, in case it is, message box is shown as well
            //! this function should be called before every action user can trigger
            bool CheckExit();
            void DisplayWelcomeMessage();
            void FinishRestore();
            //! When any button to warn current user is pressed it call this function
            void TriggerWarn();
            //! Check if we can revert this edit
            bool PreflightCheck(WikiEdit *_e);
            //! Welcome user
            void Welcome();
            void ChangeProvider(HuggleFeed *provider);
            //! Recreate interface, should be called everytime you do anything with main form
            void Render();
            //! Request a page deletion csd or afd and so on
            void RequestPD();
            //! This function is called by main thread and is used to remove edits that were already reverted
            void TruncateReverts();
            void closeEvent(QCloseEvent *event);
            void FinishPatrols();
            void UpdateStatusBarData();
            void DecreaseBS();
            void IncreaseBS();
            void ProcessReverts();
            QString WikiScriptURL();
            QString ProjectURL();
            //! This timer periodically executes various jobs that needs to be executed in main thread loop
            QTimer *GeneralTimer;
            QDateTime EditLoad;
            QString RestoreEdit_RevertReason;
            QTimer *wlt = nullptr;
            //! Status bar
            QLabel *Status;
            bool EditablePage;
            WarningList *fWarningList = nullptr;
            WaitingForm *fWaiting = nullptr;
            RequestProtect *fRFProtection = nullptr;
            //! List of all edits that are kept in history, so that we can track them and delete them
            QList<WikiEdit*> Historical;
            ApiQuery *RestoreQuery = nullptr;
            WikiEdit *RestoreEdit = nullptr;
            QList<RevertQuery*> RevertStack;
            //! This is a page that is going to be displayed if users request their latest action to be
            //! reviewed when it's done (for example when they rollback an edit and they want to
            //! display it, instead of next one)
            WikiPage *OnNext_EvPage = nullptr;
    };
}
#endif // MAINWINDOW_H
