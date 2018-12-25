//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HOOKS_H
#define HOOKS_H

#include "definitions.hpp"

#include <QHash>
#include <QString>

namespace Huggle
{
    class ApiQuery;
    class WikiUser;
    class WikiSite;
    class WikiEdit;
    class Shortcut;
    class Exception;
    class Message;
    class Query;
    class HistoryItem;

    //! Hooks that can be used to attach some 3rd code to existing functions
    class HUGGLE_EX_CORE Hooks
    {
        public:
            static bool EditBeforeScore(WikiEdit *edit);
            /*!
             * \brief Event that is called after edit pre process
             * \param Edit that was just pre processed
             */
            static void EditAfterPreProcess(WikiEdit *edit);
            static void EditBeforePostProcess(WikiEdit *edit);
            /*!
             * \brief Event that is called after edit is post processed by internal edit processor
             * \param Edit was just post processed by huggle internal edit processor
             */
            static void EditAfterPostProcess(WikiEdit *edit);
            static bool EditBeforePreProcess(WikiEdit *edit);
            static bool OnEditLoadToQueue(WikiEdit *edit);
            /*!
             * \brief Event that happens when edit is marked as good
             * \param Edit
             */
            static void OnGood(WikiEdit *edit);
            /*!
             * \brief Event that happens when edit is queued for revert
             * \param Edit
             */
            static void OnRevert(WikiEdit *edit);
            /*!
             * \brief Event that checks if edit can be considered processed
             *
             * In case there are some extensions that add extra stuff to edit processing (such as extra queries) and need to wait
             * for them to finish, they can return false here in case they are still waiting for some query to finish
             * so that this edit is hold in a queue instead of being distributed to interface. This is useful in case you make
             * extension that needs to execute asynchronous jobs during the processing of each edit.
             *
             * Until this function returns true edit is hold in a queue
             * \param Edit
             * \return
             */
            static bool EditCheckIfReady(WikiEdit *edit);
            //! This exists for JS to allow scoring of edit by user, called after post processing from main thread
            static void WikiEdit_ScoreJS(WikiEdit *edit);
            static bool RevertPreflight(WikiEdit *edit);
            /*!
             * \brief Event that happens when user attempt to send a warning to editor of page
             * \param User
             */
            static void OnWarning(WikiUser *user);
            /*!
             * \brief Event that happens when edit is flagged as suspicious modification
             * \param Edit
             */
            static void Suspicious(WikiEdit *edit);
            /*!
             * \brief When the score of user is changed
             * \param User pointer to user whom score is changed
             * \param Score New score of user
             */
            static void BadnessScore(WikiUser *user, int score);
            static Message *MessageUser(WikiUser *user, QString text, QString title, QString summary, bool insert_section = true,
                                    Query *dependency = nullptr, bool no_suffix = false, bool section_keep = false,
                                    bool autoremove = true, QString base_timestamp = "", bool create_only = false, bool fresh_only = false);
            static void WikiUser_Updated(WikiUser *user);
            static void WarningFinished(WikiEdit *edit);
            static void WikiEdit_OnNewHistoryItem(HistoryItem *history_item);
            static void QueryPool_Remove(Query *q);
            static void QueryPool_Update(Query *q);
            static void OnLocalConfigWrite();
            static void OnLocalConfigRead();
            static void ReportUser(WikiUser *u);
            static void SilentReport(WikiUser *u);
            static void ShowMessage(QString title, QString message);
            static void ShowError(QString title, QString message);
            static void ShowWarning(QString title, QString message);
            static bool ShowYesNoQuestion(QString title, QString message, bool default_answer);
            static void Shutdown();
            //! Called on initialization of feed providers
            static void FeedProvidersOnInit(WikiSite *site);
            static bool HAN_Suspicious(WikiEdit *edit, QString nick, QString ident, QString host);
            static bool HAN_Revert(WikiEdit *edit, QString nick, QString ident, QString host);
            static bool HAN_Good(WikiEdit *edit, QString nick, QString ident, QString host);
            static bool HAN_Rescore(WikiEdit *edit, long score,  QString nick, QString ident, QString host);
            static bool HAN_Message(WikiSite *site, QString message, QString nick, QString ident, QString host);
    };
}

#endif // HOOKS_H
