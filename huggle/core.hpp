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

#include <QApplication>
#include <QList>
#include <QDateTime>
#include <QFile>
#include "pythonengine.hpp"

namespace Huggle
{
    // Predeclaring some types
#ifdef HUGGLE_PYTHON
    namespace Python
    {
        class PythonEngine;
    }
#endif
    class Login;
    class MainWindow;
    class Exception;
    class GC;
    class ProcessorThread;
    class HuggleQueueFilter;
    class Syslog;
    class QueryPool;
    class iExtension;

    //! Override of qapplication so that we can reimplement notify
    class HUGGLE_EX HgApplication : public QApplication
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
    class HUGGLE_EX Core
    {
        public:
            static void ExceptionHandler(Exception *exception);
            /*!
             * \brief VersionRead - read the version from embedded git file
             *
             * This function may be called also from terminal parser
             */
            static void VersionRead();
            //! Pointer to core
            //! When we compile an extension that links against the huggle core, it loads into a different stack,
            //! which is pretty much like a separate application domain, so I will call it so in this
            //! documentation. Because we want to have only 1 core, we create a dynamic instance here and let
            //! modules that are loaded by huggle read a pointer to it, so that they can access the correct one
            //! instead of creating own instance in different block of memory. There should be only 1 core for
            //! whole application and this is that one. If you are running extension you need to update this pointer
            //! with that one you receive using iExtension::HuggleCore()
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
            //! List of extensions loaded in huggle
            QList<iExtension*> Extensions;
            QList<HuggleQueueFilter *> FilterDB;
            //! Change this to false when you want to terminate all threads properly (you will need to wait few ms)
            bool Running;
            //! Garbage collector
            Huggle::GC *gc;
#ifdef HUGGLE_PYTHON
            Python::PythonEngine *Python;
#endif
        private:
            //! This is a post-processor for edits
            ProcessorThread *Processor;
    };
}

#endif // CORE_H
