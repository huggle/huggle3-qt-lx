//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef EXCEPTION_H
#define EXCEPTION_H

#include "definitions.hpp"
// now we need to ensure that python is included first, because it
// simply suck :P
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <iostream>
#include <QString>
#include <QDir>

//////////////////////////////////////////////////////////////////////////
// Breakpad init
//////////////////////////////////////////////////////////////////////////
#ifndef DISABLE_BREAKPAD
    #ifdef __linux__
        //linux code goes here
    #define HUGGLE_BREAKPAD 0
    #include "client/linux/handler/exception_handler.h"
    #elif _WIN32
        // windows code goes here
    #define HUGGLE_BREAKPAD 1
    // This fixes the qdatetime bug which produces error with compiler on windows
    #define NOMINMAX
    // Ensure that NOMINMAX is there before including this file
    #include "client/windows/handler/exception_handler.h"
    #endif
#endif
//////////////////////////////////////////////////////////////////////////
// remaining code must be surrounded with directives
//////////////////////////////////////////////////////////////////////////


namespace Huggle
{
    //! Every exception raised by huggle is defined by this class
    class Exception
    {
        public:
            /*!
             * \brief ThrowSoftException Soft exceptions that crashes the application only in debugging mode
             *This can be used in case you want to throw exception only when debugging the application because
             *the exception itself is not critical enough to crash whole application to regular users
             * \param Text
             * \param Source
             */
            static void ThrowSoftException(QString Text, QString Source);
            static void InitBreakpad();
            static void ExitBreakpad();

            //! Error code
            int ErrorCode;
            //! Source
            QString Source;
            //! Reason for crash
            QString Message;
            //! ctor
            Exception(QString Text, bool __IsRecoverable = true);
            Exception(QString Text, QString _Source, bool __IsRecoverable = true);
            bool IsRecoverable() const;
        private:
#ifdef HUGGLE_BREAKPAD
#if HUGGLE_BREAKPAD == 0
            static google_breakpad::MinidumpDescriptor *GoogleBP_descriptor;
#endif
            static google_breakpad::ExceptionHandler   *GoogleBP_handler;
#endif
            bool _IsRecoverable;
    };
}

#endif // EXCEPTION_H
