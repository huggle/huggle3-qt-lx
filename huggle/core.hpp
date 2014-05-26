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

#include "definitions.hpp"
// now we need to ensure that python is included first, because it simply suck :P
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QApplication>
#include <QNetworkAccessManager>
#include <QList>
#include <QPluginLoader>
#include <QFile>
#include <QMap>
#include "query.hpp"
#include "login.hpp"
#include "wikiedit.hpp"
#include "mainwindow.hpp"
#include "exceptionwindow.hpp"
#include "iextension.hpp"
#include "hugglequeuefilter.hpp"
#include "editquery.hpp"
#include "resources.hpp"
#include "history.hpp"
#include "apiquery.hpp"
#include "querypool.hpp"
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
    namespace Python
    {
        class PythonEngine;
    }
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
    class Syslog;
    class QueryPool;
    class iExtension;
    class Configuration;
    class Localizations;

    //! Override of qapplication so that we can reimplement notify
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
     *
     * You may figure out that some pointers here are duplicates of static pointers that exist somewhere else but point
     * to exactly same memory area as these, that is made so that extension which explicitly request memory area for
     * core (in order to hook up into huggle internals) would get these memory addresses and can update these
     * static pointers which it has inside of its own domain, there is no other better way I know how to handle that
     */
    class Core
    {
        public:
            static void ExceptionHandler(Exception *exception);
            /*!
             * \brief VersionRead - read the version from embedded git file
             *
             * This function may be called also from terminal parser
             */
            static void VersionRead();
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
            //! Terminate the process, call this after you release all resources and finish all queries
            void Shutdown();
            void TestLanguages();
            void LoadDB();
            //! Load a definitions of problematic users, see WikiUser::ProblematicUsers for details
            void LoadDefs();
            //! Store a definitions of problematic users, see WikiUser::ProblematicUsers for details
            void SaveDefs();
            double GetUptimeInSeconds();
            void LoadLocalizations();
            QueryPool *HGQP;
            // Global variables
            QDateTime StartupTime;
            //! Pointer to main
            MainWindow *Main;
            //! Login form
            Login *fLogin;
            Syslog *HuggleSyslog;
            //! Pointer to primary feed provider
            HuggleFeed *PrimaryFeedProvider;
            //! Pointer to secondary feed provider
            HuggleFeed *SecondaryFeedProvider;
            //! List of extensions loaded in huggle
            QList<iExtension*> Extensions;
            QList<HuggleQueueFilter *> FilterDB;
            //! Change this to false when you want to terminate all threads properly (you will need to wait few ms)
            bool Running;
            Localizations *HuggleLocalizations;
            //! Garbage collector
            GC *gc;
#ifdef PYTHONENGINE
            Python::PythonEngine *Python;
#endif
        private:
            //! This is a post-processor for edits
            ProcessorThread *Processor;
    };
}

#endif // CORE_H
