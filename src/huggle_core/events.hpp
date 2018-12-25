//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef EVENTS_HPP
#define EVENTS_HPP

#include "definitions.hpp"
#include <QObject>
#include <QString>

namespace Huggle
{
    class Query;
    class WikiEdit;
    class WikiUser;
    class WikiSite;
    class Hooks;
    class HistoryItem;
    class HUGGLE_EX_CORE Events : public QObject
    {
            Q_OBJECT
        public:
            static Events *Global;
            Events()=default;
             ~Events() override =default;

        signals:
            void WikiUser_Updated(WikiUser *wiki_user);
            void WikiEdit_OnNewHistoryItem(HistoryItem *hi);
            //! Triggered when "good edit" button is pressed by user
            void WikiEdit_OnGood(WikiEdit *wiki_edit);
            //! Triggered when "revert" is done by user
            void WikiEdit_OnRevert(WikiEdit *wiki_edit);
            //! When warning is sent to vandal user
            void WikiEdit_OnWarning(WikiUser *wiki_user, byte_ht warning_level);
            //! When suspicious edit is flagged by user
            void WikiEdit_OnSuspicious(WikiEdit *wiki_edit);
            void QueryPool_Remove(Query *q);
            void QueryPool_Update(Query *q);
            void QueryPool_FinishPreprocess(WikiEdit *wiki_edit);
            void QueryPool_FinishPostprocess(WikiEdit *wiki_edit);
            void Reporting_SilentReport(WikiUser *wiki_user);
            void Reporting_Report(WikiUser *wiki_user);
            void System_Message(QString title, QString text);
            void System_WarningMessage(QString title, QString text);
            void System_ErrorMessage(QString title, QString text);
            void System_YesNoQuestion(QString title, QString text, bool *yes);

        private:
            friend class Hooks;

            void on_WERevert(WikiEdit *e);
            void on_WEGood(WikiEdit *e);
            void on_WENewHistoryItem(HistoryItem *hi);
            void on_WEWarningSent(WikiUser *u, byte_ht wl);
            void on_WESuspicious(WikiEdit *e);
            void on_QueryPoolFinishWEPreprocess(WikiEdit *e);
            void on_QueryPoolFinishWEPostprocess(WikiEdit *e);
            void on_QueryPoolRemove(Query *q);
            void on_QueryPoolUpdate(Query *q);
            void on_Report(WikiUser *u);
            void on_SReport(WikiUser *u);
            void on_UpdateUser(WikiUser *wiki_user);
            void on_SMessage(const QString &title, const QString &text);
            void on_SWarning(const QString &title, const QString &text);
            void on_SError(const QString &title, const QString &text);
            bool on_SYesNoQs(const QString &title, const QString &text, bool d);
    };
}

#endif // EVENTS_HPP
