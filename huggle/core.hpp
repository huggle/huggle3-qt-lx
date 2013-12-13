//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef CORE_H
#define CORE_H

#include <QApplication>
#include <QNetworkAccessManager>
#include <QList>
#include <QString>
#include <QPluginLoader>
#include <QFile>
#include <QMap>
#include <QtXml>
#include <QMessageBox>
#include "syslog.hpp"
#include "query.hpp"
#include "login.hpp"
#include "configuration.hpp"
#include "wikiedit.hpp"
#include "mainwindow.hpp"
#include "exceptionwindow.hpp"
#include "message.hpp"
#include "iextension.hpp"
#include "hugglequeuefilter.hpp"
#include "editquery.hpp"
#include "localization.hpp"
#include "history.hpp"
#include "apiquery.hpp"
#include "sleeper.hpp"
#include "revertquery.hpp"
#include "huggleparser.hpp"

#ifdef PYTHONENGINE
#include "pythonengine.hpp"
#endif

namespace Huggle
{
    // Predeclaring some types
#ifdef PYTHONENGINE
    class PythonEngine;
#endif
    class Sleeper;
    class Login;
    class Query;
    class ApiQuery;
    class MainWindow;
    class HuggleFeed;
    class EditQuery;
    class ProcessorThread;
    class Collectable;
    class HuggleQueueFilter;
    class WikiSite;
    class WikiPage;
    class OAuthLoginQuery;
    class WikiUser;
    class WikiEdit;
    class RevertQuery;
    class Message;
    class Syslog;
    class iExtension;
    class Configuration;
    class Localizations;

    //! Overwrite of qapplication so that we can reimplement notify
    class HgApplication : public QApplication
    {
        public:
            HgApplication(int& argc, char** argv) : QApplication(argc, argv) {}
            bool notify(QObject* receiver, QEvent* event);
    };

    /*!
     * \brief Miscelanceous system functions
     *
     * Making any instance of this class is nonsense don't do it :D
     */
    class Core
    {
        public:
            static QString GetProjectURL(WikiSite Project);
            static void ExceptionHandler(Exception *exception);
            //! Return a full url like http://en.wikipedia.org/wiki/
            static QString GetProjectWikiURL(WikiSite Project);
            //! Return a script url like http://en.wikipedia.org/w/
            static QString GetProjectScriptURL(WikiSite Project);
            //! Return a base url of current project
            static QString GetProjectURL();
            //! Return a full url like http://en.wikipedia.org/wiki/
            static QString GetProjectWikiURL();
            //! Return a script url like http://en.wikipedia.org/w/
            static QString GetProjectScriptURL();
            static QString ParameterizedTitle(QString title, QString parameter);
            //! Pointer to core, there should be only 1 core for whole application and this is that one
            //! if you are running extension you need to update this pointer with that one you receive
            //! using iExtension::HuggleCore
            static Core *HuggleCore;

            Core();
            ~Core();
            //! Function which is called as one of first when huggle is loaded
            void Init();
            //! Load extensions (libraries as well as python)
            void ExtensionLoad();
            /*!
             * \brief VersionRead - read the version from embedded git file
             *
             * This function may be called also from terminal parser
             */
            void VersionRead();
            //! Helper function that will return URL of project in question
            /*!
             * \param Project Site
             * \return String with url
             */
            void ProcessEdit(WikiEdit *e);
            //! Terminate the process, call this after you release all resources and finish all queries
            void Shutdown();
            //! Display a message box telling user that function is not allowed during developer mode
            void DeveloperError();
            //! Check the edit summary and similar in order to
            //! determine several edit attributes etc
            void PreProcessEdit(WikiEdit *_e);
            //! Perform more expensive tasks to finalize
            //! edit processing
            void PostProcessEdit(WikiEdit *_e);
            //! Check if all running queries are finished and if so it removes them from list
            void CheckQueries();
            //! Check if we can revert this edit
            bool PreflightCheck(WikiEdit *_e);
            /*!
             * \brief RevertEdit Reverts the edit
             * \param _e Pointer to edit that needs to be reverted
             * \param summary Summary to use if this is empty the default revert summary is used
             * \param minor If revert should be considered as minor edit
             * \param rollback If rollback feature should be used
             * \param keep Whether the query produced by this function should not be automatically deleted
             * \return Pointer to api query that executes this revert
             */
            RevertQuery *RevertEdit(WikiEdit* _e, QString summary = "", bool minor = false, bool rollback = true, bool keep = false);
            void LoadDB();
            //! Remove edit in proper manner
            void DeleteEdit(WikiEdit *edit);
            //! Load a definitions of problematic users, see WikiUser::ProblematicUsers for details
            void LoadDefs();
            //! Store a definitions of problematic users, see WikiUser::ProblematicUsers for details
            void SaveDefs();
            QString MonthText(int n);
            /*!
             * \brief MessageUser Message user
             * \param user Pointer to user
             * \param message Text of message
             * \param title Title
             * \param summary Summary
             * \param section Whether this message should be created in a new section
             * \param dependency Query that is used as a dependency, if it's not NULL
             * the system will wait for it to finish before the message is sent
             * \return
             */
            Message *MessageUser(WikiUser *user, QString message, QString title, QString summary, bool section = true,
                                 Query *dependency = NULL, bool nosuffix = false, bool keep = false);
            void FinalizeMessages();
            QString RetrieveTemplateToWarn(QString type);
            EditQuery *EditPage(WikiPage *page, QString text, QString summary = "Edited using huggle", bool minor = false, QString token = "");
            /*!
             * \brief Insert a query to internal list of running queries, so that they can be watched
             * This will insert it to a process list in main form
             * \param item Query that is about to be inserted to list of running queries
             */
            void AppendQuery(Query* item);
            void LoadLocalizations();
            bool ReportPreFlightCheck();
            int RunningQueriesGetCount();
            //! This function is called by main thread and is used to remove edits that were already reverted
            void TruncateReverts();
            // Global variables
            QDateTime StartupTime;
            //! Pointer to main
            MainWindow *Main;
            //! Login form
            Login *f_Login;
            //! This string contains a html header
            QString HtmlHeader;
            //! This string contains a html footer
            QString HtmlFooter;
            //! Pointer to primary feed provider
            HuggleFeed *PrimaryFeedProvider;
            //! Pointer to secondary feed provider
            HuggleFeed *SecondaryFeedProvider;
            //! This is a list of all edits that are being processed by some way
            //! whole list needs to be checked and probed everytime once a while
            QList<WikiEdit*> ProcessingEdits;
            //! Pending changes
            QList<EditQuery*> PendingMods;
            //! List of extensions loaded in huggle
            QList<iExtension*> Extensions;
            QList<HuggleQueueFilter *> FilterDB;
            //! Change this to false when you want to terminate all threads properly (you will need to wait few ms)
            bool Running;
            double GetUptimeInSeconds();
            //! Garbage collector
            GC *gc;
#ifdef PYTHONENGINE
            PythonEngine *Python;
#endif
        private:
            //! List of all running queries
            QList<Query*> RunningQueries;
            //! We need to store some recent reverts for wiki provider so that we can backward decide if edit
            //! was reverted before we parse it
            QList<WikiEdit*> RevertBuffer;
            QList<WikiEdit*> UncheckedReverts;
            //! This is a post-processor for edits
            ProcessorThread * Processor;
            //! List of all messages that are being sent
            QList<Message*> Messages;
    };
}

#endif // CORE_H
