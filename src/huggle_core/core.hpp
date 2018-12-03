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
#include <QList>
#include <QDateTime>

namespace Huggle
{
    // Predeclaring some types
    class Configuration;
    class Exception;
    class ExceptionHandler;
    class GC;
    class WikiEdit_ProcessorThread;
    class HuggleQueueFilter;
    class Syslog;
    class QueryPool;
    class iExtension;

    /*!
     * \brief Miscelanceous system functions and application control class
     *
     * This class exists mostly for historical reasons, it's typically instantiated during launch of application and destroyed
     * during its shutdown.
     *
     * It controls various components of Huggle internals:
     * * Garbage collector
     * * Localization subsystem
     * * Exception handling
     * * Configuration load / write
     * * Extension control
     *
     * It should be globally accessible through instance Core::HuggleCore
     *
     * You may figure out that some pointers here are duplicates of static pointers that exist somewhere else but point
     * to exactly same memory area as these, that is made so that extension which explicitly request memory area for
     * core (in order to hook up into huggle internals) would get these memory addresses and can update these
     * static pointers which it has inside of its own domain, there is no other better way I know how to handle that
     */
    class HUGGLE_EX_CORE Core
    {
        public:
            static void HandleException(Exception *exception);
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
            qint64 GetUptimeInSeconds();
            void LoadLocalizations();
            void InstallNewExceptionHandler(ExceptionHandler *eh);
            void WriteProfilerDataIntoSyslog();
            QueryPool *HGQP;
            // Global variables
            QDateTime StartupTime;
            Syslog *HuggleSyslog;
            //! List of extensions loaded in huggle
            QList<iExtension*> Extensions;
            QList<HuggleQueueFilter *> FilterDB;
            //! Change this to false when you want to terminate all threads properly (you will need to wait few ms)
            bool Running;
            //! Garbage collector
            Huggle::GC *gc;
        private:
            //! This is a post-processor for edits
            WikiEdit_ProcessorThread *processorThread;
            ExceptionHandler *exceptionHandler;
            bool loaded = false;
    };
}

#endif // CORE_H
