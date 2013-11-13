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

#include <iostream>
#include <QApplication>
#include <QNetworkAccessManager>
#include <QList>
#include <QString>
#include <QPluginLoader>
#include <QFile>
#include <QMap>
#include <QtXml>
#include <QMessageBox>
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
#include "history.hpp"
#include "apiquery.hpp"
#include "revertquery.hpp"

#ifdef PYTHONENGINE
#include "pythonengine.hpp"
#endif

namespace Huggle
{
    // Predeclaring some types
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
    class iExtension;
    class Configuration;

    /*!
     * \brief This is a workaround that allow us to use sleep
     */
    class Sleeper : public QThread
    {
        public:
            static void usleep(unsigned long usecs){QThread::usleep(usecs);}
            static void msleep(unsigned long msecs){QThread::msleep(msecs);}
            static void sleep(unsigned long secs){QThread::sleep(secs);}
    };

    //! Overwrite of qapplication so that we can reimplement notify
    class HgApplication : public QApplication
    {
        public:
            HgApplication(int& argc, char** argv) :
                QApplication(argc, argv) {}
            bool notify(QObject* receiver, QEvent* event);
    };

    /*!
     * \brief The Language class
     */
    class Language
    {
        public:
            //! Creates new instance of language
            //! param name Name of language
            Language(QString name);
            //! This is a short language name which is used by system
            QString LanguageName;
            //! Long identifier of language that is seen by user
            QString LanguageID;
            QMap<QString, QString> Messages;
    };

    /*!
     * \brief Miscelanceous system functions, all of these functions are static
     *
     * Making any instance of this class is nonsense don't do it :D
     */
    class Core
    {
        public:
            static Core *HuggleCore;
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
            //! Languages D:
            QList<Language*> LocalizationData;
            //! Pointer to AIV page
            WikiPage *AIVP;
            //! Pointer to UAA page
            WikiPage *UAAP;
            //! Change this to false when you want to terminate all threads properly (you will need to wait few ms)
            bool Running;

#ifdef PYTHONENGINE
            PythonEngine *Python;
#endif

            //! Function which is called as one of first when huggle is loaded
            void Init();
            //! Write text to terminal as well as ring log
            /*!
             * \param Message Message to log
             */
            void Log(QString Message);
            //! Load extensions (libraries as well as python)
            void ExtensionLoad();
            /*!
             * \brief VersionRead - read the version from embedded git file
             *
             * This function may be called also from terminal parser
             */
            void VersionRead();
            //! This log is only shown if verbosity is same or larger than requested verbosity
            void DebugLog(QString Message, unsigned int Verbosity = 1);
            //! Helper function that will return URL of project in question
            /*!
             * \param Project Site
             * \return String with url
             */
            static QString GetProjectURL(WikiSite Project);
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
            void ProcessEdit(WikiEdit *e);
            //! Terminate the process, call this after you release all resources and finish all queries
            void Shutdown();
            //! Return a ring log represented as 1 huge string
            QString RingLogToText();
            /*!
             * \brief Return a ring log as qstring list
             * \return QStringList
             */
            QStringList RingLogToQStringList();
            void InsertToRingLog(QString text);
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
            //! Remove leading and finishing space of string
            QString Trim(QString text);
            //! Remove edit in proper manner
            void DeleteEdit(WikiEdit *edit);
            // the mess bellow exist because of a way how huggle config stores the lists
            static QString GetSummaryOfWarningTypeFromWarningKey(QString key);
            static QString GetNameOfWarningTypeFromWarningKey(QString key);
            static QString GetKeyOfWarningTypeFromWarningName(QString id);
            //! Parse a part patterns for score words
            void ParsePats(QString text);
            //! Load a definitions of problematic users, see WikiUser::ProblematicUsers for details
            void LoadDefs();
            //! Store a definitions of problematic users, see WikiUser::ProblematicUsers for details
            void SaveDefs();
            static QString GetValueFromKey(QString item);
            static QString GetKeyFromValue(QString item);
            void ParseWords(QString text);
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
            Message *MessageUser(WikiUser *user, QString message, QString title, QString summary, bool section = true, Query *dependency = NULL);
            void FinalizeMessages();
            QString RetrieveTemplateToWarn(QString type);
            EditQuery *EditPage(WikiPage *page, QString text, QString summary = "Edited using huggle", bool minor = false);
            /*!
             * \brief Insert a query to internal list of running queries, so that they can be watched
             * This will insert it to a process list in main form
             * \param item Query that is about to be inserted to list of running queries
             */
            void AppendQuery(Query* item);
            static void ExceptionHandler(Exception *exception);
            QString Localize(QString key);
            QString Localize(QString key, QStringList parameters);
            QString Localize(QString key, QString parameters);
            void LoadLocalizations();
            bool ReportPreFlightCheck();
            int RunningQueriesGetCount();
            Core();
            ~Core();
        private:
            //! List of all running queries
            QList<Query*> RunningQueries;
            //! Ring log is a buffer that contains system messages
            QStringList RingLog;
            //! This is a post-processor for edits
            ProcessorThread * Processor;
            //! List of all messages that are being sent
            QList<Message*> Messages;
            /*!
             * \brief Initializes a localization with given name
             *
             * This function will create a new localization object using built-in localization file
             * using Core::MakeLanguage() and insert that to language list
             * \param name Name of a localization that is a name of language without txt suffix in localization folder
             */
            static void LocalInit(QString name);
            static Language *MakeLanguage(QString text, QString name);
    };
}

#endif // CORE_H
